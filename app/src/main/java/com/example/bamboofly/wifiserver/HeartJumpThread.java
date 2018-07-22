package com.example.bamboofly.wifiserver;

import android.os.SystemClock;
import android.util.Log;

import java.io.IOException;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class HeartJumpThread extends Thread {

    private ServerSocket mServerSocket;
    private int mHeartJumpCount;
    private Socket mSocket;

    public HeartJumpThread(Socket socket){
        mSocket = socket;
    }

    @Override
    public void run() {
        Log.e("lianghuan","-----run-----");

        if (mSocket != null){
            Log.e("lianghuan","-----run----3-");
            try {
                OutputStream outputStream = mSocket.getOutputStream();
                Log.e("lianghuan","-----run----4-");
                while (mSocket.isConnected()){
                    outputStream.write(mHeartJumpCount);
                    Log.e("lianghuan",""+this+" mHeartJumpCount = "+mHeartJumpCount);
                    mHeartJumpCount++;
                    SystemClock.sleep(1000);
                }

                mSocket.close();
                Log.e("lianghuan","this = "+this+" socket close");

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
