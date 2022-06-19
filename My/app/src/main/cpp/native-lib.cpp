#include <jni.h>
#include <string>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>



extern "C" JNIEXPORT jstring JNICALL Java_com_example_my_MainActivity_strin(
        JNIEnv* env,
        jobject /* this */, jlong day ,jlong day2 ) {
   std::string hello = "Hello from C++";
   /*  hello = std::to_string(y)+ "/"+std::to_string(m)+ "/"
            +std::to_string(d)+ "/"+std::to_string(y1)+ "/"
            +std::to_string(m1)+ "/"+std::to_string(d1);

    char *serverIp = "192.168.1.11";
    int port = 8080;
    //create a message buffer
    char msg[1500];
    //setup a socket and connection tools
    struct hostent* host = gethostbyname(serverIp);
    sockaddr_in sendSockAddr;
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    connect(clientSd,(sockaddr*) &sendSockAddr, sizeof(sendSockAddr));

    memset(&msg, 0, sizeof(msg));
    strcpy(msg,hello.c_str());
    //send code
    send(clientSd,"d", 1, 0);
    send(clientSd, (char*)&msg, strlen(msg), 0);

    close(clientSd);

    int bytesRead, bytesWritten = 0;*/
    return env->NewStringUTF(hello.c_str());
}