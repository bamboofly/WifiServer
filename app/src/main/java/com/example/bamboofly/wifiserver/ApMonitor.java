package com.example.bamboofly.wifiserver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;


public abstract class ApMonitor extends BroadcastReceiver implements ApListener {

    public static final String DEFAULT_UNKNOW_IP = "0.0.0.0";

    public static final String ACTION_AP_STATE_CHANGED = "android.net.wifi.WIFI_AP_STATE_CHANGED";
    private Context mContext;
    private Handler mHandler;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (ACTION_AP_STATE_CHANGED.equals(action)) {
            //便携式热点的状态为：10---正在关闭；11---已关闭；12---正在开启；13---已开启
            int state = intent.getIntExtra("wifi_state",  0);

            if (state == WIFI_AP_OPENING){
                onApOpening();
            }else if (state == WIFI_AP_OPEN_SUCCESS){
                onApOpened();
                if (mHandler != null){
                    mHandler.post(mGetApIpAddressRunnable);
                }
            }else if (state == WIFI_AP_CLOSEING){
                onApCloseing();
            }else if (state == WIFI_AP_CLOSE_SUCCESS){
                onApClosed();
            }
        }
    }

    public void register(Context context,Handler handler){
        mContext = context;
        mHandler = handler;
        IntentFilter filter = new IntentFilter(ACTION_AP_STATE_CHANGED);
        context.registerReceiver(this,filter);
    }

    public void unregister(){
        mContext.unregisterReceiver(this);
    }

    private Runnable mGetApIpAddressRunnable = new Runnable() {
        @Override
        public void run() {
//            String hotspotLocalIpAddress = ApUtils.getHotspotLocalIpAddress(mContext);
            String hotspotLocalIpAddress = ApUtils.getWifiApIpAddress();
            if (DEFAULT_UNKNOW_IP.equals(hotspotLocalIpAddress)){
                mHandler.postDelayed(this,1000);
            }else {
                onApIpAddress(hotspotLocalIpAddress);
            }

        }
    };

}
