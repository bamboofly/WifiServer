package com.example.bamboofly.wifiserver;

import java.nio.ByteBuffer;

public interface YuvCallback {
    void onYuvFrame(ByteBuffer buffer, int timestamp);
}
