package com.example.bamboofly.wifiserver;

public class Flv2Yuv {

    static {
//        System.loadLibrary("ffmpeg");
        System.loadLibrary("flv2yuv");
    }

    public static native void setFlvPath(String flvPath);

    public static native void setCallback(YuvCallback callback);

    public static native void startDecode();

    public static native void stopDecode();

    public static native void pauseTest();

    public static native void resumeTest();

    public static native void init(Object context);

    public static native byte[] getExtraData();
}
