package com.st.st25nfc.type5.st25dv;

import android.os.AsyncTask;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.Arrays;


public class Client extends AsyncTask<byte[],Void,Void>
{
    Socket s;

    PrintWriter p;
    DataOutputStream dd3;

    @Override
    protected Void doInBackground(byte[]... voids) {
        byte[] mes = voids[0];

        try{

            s = new Socket("192.168.1.11", 8080);
            s.setSendBufferSize(1024);
            DataOutputStream p = new DataOutputStream(s.getOutputStream());
            p.write('2');
            p.flush();
            p.write(Arrays.copyOfRange(mes, 0, 1024 ));

            p.flush();
            p.close();
            s.close();

        }catch (IOException e){

            e.printStackTrace();

        }

        return null;
    }


}