package com.example.bamboofly.wifiserver;

import android.os.SystemClock;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.HashMap;

public class HandAndHeartJumpThread extends Thread {

    private final Socket mSocket;
    private DatagramPacket mDatagramPacket;
    private DatagramSocket mDatagramSocket;
    private byte[] mPacketBytes = new byte[64];
    private String mUdpIp;
    private int mUdpPort;

    public HandAndHeartJumpThread(Socket socket){
        mSocket = socket;

    }


    int count;
    @Override
    public void run() {
        try {
            InputStream inputStream = mSocket.getInputStream();
            OutputStream outputStream = mSocket.getOutputStream();

            byte[] by = new byte[1024];

            outputStream.write(1);
            int read = inputStream.read(by);
            String ip = new String(by,0,read);
            mUdpIp = ip;
            Log.e("lianghuan","ip = "+ip);
            outputStream.write(2);
            int read_port_size = inputStream.read(by);
            String port = new String(by,0,read_port_size);
            Log.e("lianghuan","port = "+port);

            int port_int = Integer.valueOf(port);
            mUdpPort = port_int;
            Log.e("lianghuan","port_int = "+port_int);
            Flv2Yuv.setCallback(mYuvCallback);
            Flv2Yuv.setFlvPath("/storage/sdcard0/a_video/aa.flv");
            Flv2Yuv.startDecode();

            SystemClock.sleep(500);
            byte[] extraData = Flv2Yuv.getExtraData();
            outputStream.write(extraData);
            int send_extradata = inputStream.read();
            if (send_extradata == 1){

            }


            mDatagramSocket = new DatagramSocket(port_int);
            InetAddress inetAddress = InetAddress.getByName(ip);
            mDatagramPacket = new DatagramPacket(by,by.length,inetAddress,port_int);

            mUdpSendFlag = true;
            Log.e("lianghuan","isConnected = "+mSocket.isConnected());
            while (mSocket.isConnected()){
                outputStream.write(0x15);
                int heart = inputStream.read();
                if (heart == 0){
                    break;
                }
                SystemClock.sleep(2000);
            }

            Log.e("lianghuan","close socket");
            mSocket.close();
            mUdpSendFlag = false;
            Flv2Yuv.stopDecode();
        } catch (IOException e) {
            e.printStackTrace();
            Flv2Yuv.stopDecode();
            Log.e("lianghuan","e = "+e.getMessage());
        }
    }

    private HashMap<Integer,DatagramPacket> mPacketMap = new HashMap<>();
    private DatagramPacket getDatagramPacket(int size){
        int maxSize = size + 1024;
        int key = maxSize / 1024;
        DatagramPacket datagramPacket = mPacketMap.get(key);
        if (datagramPacket == null){
            byte[] by = new byte[key * 1024];
            try {
                InetAddress inetAddress = InetAddress.getByName(mUdpIp);
                datagramPacket = new DatagramPacket(by,by.length,inetAddress,mUdpPort);
                mPacketMap.put(key,datagramPacket);
                return datagramPacket;
            } catch (UnknownHostException e) {
                e.printStackTrace();
            }
            return null;
        }
        return datagramPacket;
    }

    private volatile boolean mUdpSendFlag = false;
    private YuvCallback mYuvCallback = new YuvCallback() {
        @Override
        public void onYuvFrame(ByteBuffer buffer, int timestamp) {
            if (mUdpSendFlag){
                int size = buffer.capacity();
                Log.e("lianghuan","buffer size = "+size);
                DatagramPacket datagramPacket = getDatagramPacket(size);
                if (datagramPacket != null){
                    byte[] data = datagramPacket.getData();
                    buffer.clear();
                    buffer.get(data,0,size);
                    datagramPacket.setLength(size);
                    try {
                        mDatagramSocket.send(datagramPacket);
                        Log.e("lianghuan","send success");
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    };
}
