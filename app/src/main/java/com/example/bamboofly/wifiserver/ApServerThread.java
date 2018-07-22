package com.example.bamboofly.wifiserver;

import android.os.SystemClock;
import android.util.Log;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

public class ApServerThread extends Thread {

    public static final int MAIN_PORT = 12345;
    private ServerSocket mServerSocket;

    private int mHeartJumpCount;
    private boolean mRunFlag;

    public ApServerThread(){

    }


    @Override
    public void run() {
        Log.e("lianghuan","server start");
        while (mRunFlag && mServerSocket == null){
            try {
                mServerSocket = new ServerSocket(MAIN_PORT);
            } catch (IOException e) {
                e.printStackTrace();
                Log.e("lianghuan","e = "+e.getMessage());
            }
            SystemClock.sleep(1000);
        }

        try {
            while (mRunFlag){
                Log.e("lianghaun","--------socket ok-----------");
                Socket accept = mServerSocket.accept();

                Log.e("lianghuan","accept socket");
//                new HeartJumpThread(accept).start();
                new HandAndHeartJumpThread(accept).start();

//                mHeartJumpCount++;
//                SystemClock.sleep(1000);

            }
            mServerSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
            Log.e("lianghuan","e = "+e.getMessage());
        }
    }

    @Override
    public synchronized void start() {
        mRunFlag = true;
        super.start();

    }

    public void stopServer(){
        mRunFlag = false;
    }
}
