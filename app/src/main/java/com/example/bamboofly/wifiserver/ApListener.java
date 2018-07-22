package com.example.bamboofly.wifiserver;

public interface ApListener {

    int WIFI_AP_CLOSEING 		= 10;  //wifi hot is closeing
    int WIFI_AP_CLOSE_SUCCESS 	= 11;  //wifi hot close success
    int WIFI_AP_OPENING 		= 12;  //WiFi hot is opening
    int WIFI_AP_OPEN_SUCCESS 	= 13;  //WiFi hot open success

    void onApOpening();
    void onApOpened();
    void onApCloseing();
    void onApClosed();
    void onApIpAddress(String ip);
}
