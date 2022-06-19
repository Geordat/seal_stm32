
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "config.h"
#include "convert.h"
#include "fileops.h"
#include "generate.h"
#include "key.h"
#include "seal/seal.h"
#include "utils.h"

using namespace std;
using namespace seal;
using namespace seal::util;

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::filesystem::recursive_directory_iterator;

long milis_of_day = 86400000;

vector<double> norm_min = {24.940000,  24.750000, 24.440000, 24.940000, 0.000000, 0.000000,
                           0.000000,   0.000000,  0.060000,  0.040000,  0.040000, 0.050000,
                           345.000000, -6.296154, 0.000000,  0.000000};

vector<double> norm_max = {26.380000,   29.000000, 26.190000, 26.560000, 165.000000, 258.000000,
                           280.000000,  74.000000, 3.880000,  3.440000,  3.670000,   3.400000,
                           1270.000000, 8.980769,  1.000000,  1.000000};

vector<double> input    = {24.94, 24.75, 24.56, 25.38, 121, 34,       53, 40,
                        0.08,  0.19,  0.06,  0.06,  390, 0.769231, 0,  0};

double sigmoid(double x)
{
    double result;
    result = 1 / (1 + exp(-x));
    return result;
}

Ciphertext mul_vmat(Ciphertext mat, vector<vector<double>> mat1, EncryptionParameters params,
                    int cols, int rows, int col2, int scale, GaloisKeys gal_keys)

{
    Ciphertext res;
    Plaintext pt;
    SEALContext context(params);
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);

    vector<Ciphertext> ctA_result(cols);
    vector<Ciphertext> ctB_result(cols);

    vector<double> p(col2 * rows);

    for (int y = 0; y < col2 * rows; y++) p[y] = 0.0001;
    for (int y = 0; y < rows; y++) p[y] = 1;
    encoder.encode(p, scale, pt);

    parms_id_type last_parms_id = mat.parms_id();
    evaluator.mod_switch_to_inplace(pt, last_parms_id);

    for (int i = 0; i < cols; i++)
    {
        evaluator.multiply_plain(mat, pt, ctA_result[i]);
        evaluator.rotate_vector(mat, rows, gal_keys, mat);

        ctB_result[i] = ctA_result[i];

        for (int ii = 0; ii < cols - 1; ii++)
        {
            evaluator.rotate_vector(ctA_result[i], -rows, gal_keys, ctA_result[i]);
            evaluator.add(ctB_result[i], ctA_result[i], ctB_result[i]);
        }
    }

    vector<vector<double>> a(cols, vector<double>(rows * col2));

    for (int i = 0; i < cols; i++)
    {
        int cnt = 0;
        for (int ii = 0; ii < rows * col2;)
        {
            a[i][ii] = mat1[i][cnt];
            ii++;
            if (ii % rows == 0) cnt++;
        }
    }

    for (int i = 0; i < cols; i++)
    {
        encoder.encode(a[i], scale, pt);
        parms_id_type last_parms_id = ctB_result[i].parms_id();
        evaluator.mod_switch_to_inplace(pt, last_parms_id);
        evaluator.multiply_plain(ctB_result[i], pt, ctB_result[i]);
    }

    evaluator.add_many(ctB_result, res);

    return res;
}

void print_ct(Ciphertext ct, seal::SEALContext context)
{
    cout << " size of encrypted " << ct.size() << endl;
    cout << " scale " << log2(ct.scale()) << endl;
    cout << " Modulus chain index " << context.get_context_data(ct.parms_id())->chain_index()
         << endl;
}

vector<vector<double>> fl_w = {
    {-0.05, 0.723, -0.044, 0.749, -0.818, 0.138, -0.076, 0.87, 0.611, 0.234, -0.59, 0.649},
    {0.17, 0.585, -0.075, 0.589, -0.202, -0.065, 0.249, 0.274, 0.353, -0.24, -0.257, 0.716},
    {-0.686, 0.799, 0.966, 0.334, -0.967, 0.612, 0.179, 0.616, -0.297, -0.429, -0.213, 0.425},
    {-0.091, -0.161, 0.13, 0.08, 0.229, 0.025, -0.975, -0.176, 1.266, 0.654, -0.05, -0.354},
    {-0.29, 1.082, 0.794, 1.03, -0.859, 0.892, 0.642, 1.189, -0.536, -0.777, -0.634, 0.715},
    {-0.611, -0.106, 0.713, 0.778, 0.005, -0.002, 0.303, 0.501, 0.06, -0.183, -0.501, 0.788},
    {-0.177, -0.01, 0.082, 0.016, -0.271, 0.064, -0.135, -0.073, 0.361, -0.417, -0.234, -0.05},
    {-0.112, 0.355, -0.086, -0.007, 0.141, -0.059, -0.203, 0.034, 0.401, -0.199, 0.176, 0.114},
    {-0.655, 0.776, -0.002, 0.226, -0.291, 0.51, 0.496, 0.281, -0.077, -0.399, -0.954, 1.308},
    {-0.376, 0.449, 0.723, 0.506, -0.319, 0.209, 0.465, 0.337, -0.602, -0.253, -1.184, 0.632},
    {-0.238, 0.297, 0.006, 0.518, -0.133, 0.454, 0.285, 0.183, 0.144, 0.066, -0.18, 0.52},
    {-0.163, 0.669, -0.079, 0.212, -0.28, -0.188, -0.675, 0.384, -0.148, 0.309, 0.302, 0.445},
    {-0.118, 0.03, 0.506, -0.085, 0.045, 0.049, -0.102, -0.215, 0.687, 0.427, -0.514, 1.04},
    {-0.928, 1.167, 0.769, 1.014, -0.851, 0.455, -0.264, 0.579, -0.305, 0.441, 0.146, -0.4},
    {-0.358, 0.281, 0.137, 0.003, -0.314, 0.484, -0.218, 0.187, 0.078, 0.025, 0.072, 0.691},
    {0.037, 0.145, 0.309, -0.442, -0.354, 0.371, 0.076, -0.199, -0.163, -0.175, -0.433, 0.514}};

vector<double> bias1 = {-0.099, 0.078, 0.168, 0.052, -0.234, 0.19,
                        -0.895, 0.25,  0.866, 1.,    0.625,  -0.844};

vector<vector<double>> sl_w = {

    {-0.479, 0.773},  {-1.074, 0.175}, {-0.878, 0.819}, {-0.828, 0.501}, {-0.87, 0.227},
    {-0.808, -0.278}, {0.717, -0.895}, {-0.825, 0.47},  {0.947, -1.366}, {1.439, -0.764},
    {0.379, -1.13},   {1.556, -1.564}

};

vector<double> bias2 = {1.036, -1.036};

Ciphertext infer(Ciphertext input, EncryptionParameters params, double scale, GaloisKeys gal_keys,
                 seal::SEALContext context, RelinKeys relin_keys)
{
    Ciphertext ct;
    Plaintext pt;
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);

    vector<double> tr;
    vector<double> sub;

    for (int i = 0; i < 16; i++) tr.push_back(1 / (norm_max[i % 16] - norm_min[i % 16]));

    for (int i = 0; i < 4096; i++) sub.push_back(-1 * norm_min[i % 16] + 0.0001);

    encoder.encode(sub, scale, pt);
    evaluator.add_plain(input, pt, ct);
    encoder.encode(tr, scale, pt);

    parms_id_type last_parms_id = ct.parms_id();
    evaluator.mod_switch_to_inplace(pt, last_parms_id);
    evaluator.multiply_plain(ct, pt, ct);
    evaluator.rescale_to_next_inplace(ct);

    ct = mul_vmat(ct, fl_w, params, 16, 1, 12, ct.scale(), gal_keys);

    evaluator.rescale_to_next_inplace(ct);

    encoder.encode(bias1, ct.scale(), pt);
    last_parms_id = ct.parms_id();
    evaluator.mod_switch_to_inplace(pt, last_parms_id);
    evaluator.add_plain(ct, pt, ct);

    evaluator.square_inplace(ct);

    evaluator.relinearize_inplace(ct, relin_keys);
    evaluator.rescale_to_next_inplace(ct);

    ct = mul_vmat(ct, sl_w, params, 12, 1, 2, scale, gal_keys);

    encoder.encode(bias2, ct.scale(), pt);
    last_parms_id = ct.parms_id();
    evaluator.mod_switch_to_inplace(pt, last_parms_id);
    evaluator.add_plain(ct, pt, ct);

    return ct;
}

void saveCiphertext(Ciphertext encrypted, string filename)
{
    ofstream ct;
    ct.open(filename, ios::binary);
    encrypted.save(ct);
};

Ciphertext loadCiphertext(string filename, seal::SEALContext context)
{
    ifstream ct;
    ct.open(filename, ios::binary);
    Ciphertext result;
    result.load(context, ct);

    return result;
};

vector<unsigned char> intToBytes(int paramInt)
{
    vector<unsigned char> arrayOfByte(4);
    for (int i = 0; i < 4; i++) arrayOfByte[3 - i] = (paramInt >> (i * 8));
    return arrayOfByte;
}

void sk_seal_load(string fpath, const SEALContext &context, SecretKey &sk)
{
    stringstream parms_stream;

    char buffer[4];
    for (int i = 0; i < 114711; i++)
    {
        buffer[0] = intToBytes(key[i])[0];
        buffer[1] = intToBytes(key[i])[1];
        buffer[2] = intToBytes(key[i])[2];
        buffer[3] = intToBytes(key[i])[3];

        parms_stream.write(buffer, 4);
    }
    sk.load(context, parms_stream);
}
void gal_seal_load(string fpath, const SEALContext &context, GaloisKeys &gal_keys);

void pm_seal_load(string fpath, EncryptionParameters &parms);
void pm_seal_save(string fpath, EncryptionParameters &parms, bool compress);
void re_seal_save(string fpath, RelinKeys &relin_keys, bool compress);
void re_seal_load(string fpath, const SEALContext &context, RelinKeys &relin_keys);

int main(int argc, char *argv[])
{
    size_t degree = 8192;
    double scale  = pow(2, 25);
    long dayss[3];

    EncryptionParameters parms(scheme_type::ckks);
    seal::SEALContext context = setup_seale_prime_default(degree, parms);

    KeyGenerator keygen(context);
    Evaluator evaluator(context);
    RelinKeys rlk_big;
    keygen.create_relin_keys(rlk_big);
    GaloisKeys gal_keys;
    keygen.create_galois_keys(gal_keys);

    gal_seal_load("gal_r", context, gal_keys);
    re_seal_load("rel_r", context, rlk_big);

    int port          = 8080;
    streampos filepos = 0;

    char msg[1500];

    sockaddr_in servAddr;
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port        = htons(port);

    int serverSd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr, sizeof(servAddr));

    if (bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }

    listen(serverSd, 5);
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd                 = 0;
    long days[3];
    byte f[65536 * 6];
   

    while (1)
    {
        cout<<"Waiting for a request"<<endl;

        newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);

        cout << recv(newSd, days, 8, 0) << endl;

        if (days[0] == 7)
        {
            recv(newSd, days + 1, 16, 0);
            vector<string> chipetexts;

            cout << days[1] << "  ";
            chipetexts.push_back(to_string(days[1]));

            while (days[2] != days[1])
            {
                days[1] += 86400000;
                chipetexts.push_back(to_string(days[1]));
            }

            Ciphertext ct_in, ct, ct2;

            string path = "./ciphertext";

            for (int i = 0; i < chipetexts.size(); i++)
            {
                ct_in = loadCiphertext("exam", context);

                cout<<"now processing day "<<chipetexts[i]<<endl;

                ct_string_file_load(path + "/" + chipetexts[i], context, evaluator, ct_in, filepos);
                print_ct(ct_in, context);

                for (int ii = 0; ii < 3; ii++)
                {
                    cout << "cipher " << ii << endl;

                    evaluator.rotate_vector(ct_in, 16, gal_keys, ct_in);

                    if (i == 0) { ct = infer(ct_in, parms, scale, gal_keys, context, rlk_big); }
                    else
                    {
                        ct2 = infer(ct_in, parms, scale, gal_keys, context, rlk_big);
                        evaluator.rotate_vector_inplace(ct2, -2 * ii - 256 * i, gal_keys);
                        evaluator.add(ct, ct2, ct);
                    }
                }
            }

            evaluator.rescale_to_next_inplace(ct);
            evaluator.rescale_to_next_inplace(ct);

            saveCiphertext(ct, "ct1");

            memset(&msg, 0, sizeof(msg));
            int bytesRead, bytesWritten = 0;

            char buffer;
            int cnt = 0;

            fstream file("ct1", fstream::binary | fstream::in);
            fstream file2("ct1", fstream::binary | fstream::in);

            while (!file2.eof())
            {
                file2.get(buffer);
                cnt += 1;
            }

            send(newSd, &cnt, 4, 0);
            cnt = 0;

            while (!file.eof())
            {
                file.get(buffer);
                cnt += send(newSd, (char *)&buffer, 1, 0);
            }

            cout<<"end of processing"<<endl;
        }
        else if (days[0] == 8)
        {
            int u = 0;

            while (u != 65536 * 6) u += recv(newSd, f + u, 65536 * 6, 0);

            unsigned int t[98304];

            memcpy(t, f, 65536 * 6);
            string path = "./ciphertext/";
            vector<string> paths;

            for (const auto &file : recursive_directory_iterator(path))
            {
                paths.push_back(file.path());
            }

            sort(paths.begin(), paths.end());

            long int number;
            string last_d = paths[paths.size() - 1].c_str();
            last_d        = last_d.substr(13, 26);
            cout << "{" << atol(last_d.c_str()) << "}" << endl;
            long i = atol(last_d.c_str()) + milis_of_day;

            ofstream MyFile(path + "/" + to_string(i));

            for (int i = 0; i < 6; i++)
            {
                MyFile << "c0:  : { ";

                for (int y = 0; y < 8192; y++) MyFile << t[y + i * 8192 * 2] << ", ";

                MyFile << "} c1:  : { ";

                for (int y = 0; y < 8192; y++) MyFile << t[y + i * 8192 * 2 + 8192] << ", ";

                MyFile << "} ";
            }
        }
    }

    close(newSd);
    close(serverSd);

    return 0;
}

/*
SecretKey sk = keygen.secret_key();
sk_bin_file_load(
    "/home/george/Documents/SEAL-Embedded-main/device/adapter_output_data/sk_8192.dat", context,
    sk);
stringstream sk_stream;
sk.save(sk_stream);
char buffer[4];
stringstream parms_stream;
ofstream M("gee");
int cnt=0;
while (!sk_stream.eof())
{
    sk_stream.get(buffer[0]);
    sk_stream.get(buffer[1]);
    sk_stream.get(buffer[2]);
    sk_stream.get(buffer[3]);
    int a = int((unsigned char)(buffer[0]) << 24 | (unsigned char)(buffer[1]) << 16 |
                (unsigned char)(buffer[2]) << 8 | (unsigned char)(buffer[3]));
      M<<a<<" , "<<endl;
     cnt++;
    buffer[0] = intToBytes(a)[0];
    buffer[1] = intToBytes(a)[1];
    buffer[2] = intToBytes(a)[2];
    buffer[3] = intToBytes(a)[3];

    parms_stream.write(buffer, 4);
}

cout<<" t "<<cnt<<endl;
sk.load(context, parms_stream);
Ciphertext ctt;
streampos filepos   = 0;
vector<double> test = {1.0, 2.0, 3.0};
Plaintext pt;
CKKSEncoder encoder(context);
encoder.encode(test, scale, pt);

Encryptor encryptor(context, sk);
encryptor.encrypt_symmetric(pt, ctt);
ct_string_file_load(
    "/home/george/Documents/SEAL-Embedded-main/build/bin/ciphertext/1648760400000", context,
    evaluator, ctt, filepos);
Decryptor decryptor(context, sk);


vector<double> re;
decryptor.decrypt(ctt, pt);
encoder.decode(pt, re) ;
cout<<"{"<<re[0]<<endl; */