package com.st.st25nfc.type5.st25dv;

import android.os.AsyncTask;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Arrays;


public class Client2 extends AsyncTask<byte[],Void,Void>
{
    Socket s;

    PrintWriter p;
    DataOutputStream dd3;

    @Override
    protected Void doInBackground(byte[]... voids) {
        byte[] mes = voids[0];
        try{
            byte[] values = new byte[8];

            for(int i=0;i<8;i++)
                values[i] = 0;

            values[0] =8;


            s = new Socket("192.168.1.11", 8080);
            DataOutputStream p = new DataOutputStream(s.getOutputStream());
            p.write(values);
            p.write(mes);
            p.flush();
            p.close();
            s.close();

        }catch (IOException e){

            e.printStackTrace();

        }

        return null;
    }


}