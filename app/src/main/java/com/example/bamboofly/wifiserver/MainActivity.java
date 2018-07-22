package com.example.bamboofly.wifiserver;

import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    private Handler mHandler = new Handler();
    private ApServerThread mServer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mApMonitor.register(this,mHandler);
        mServer = new ApServerThread();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        ApUtils.closeAp(this);
        mApMonitor.unregister();
        mServer.stopServer();
    }

    public void openAp(View view) {
        boolean res = ApUtils.openAp(this,"lianghuan","lianghuan");
        Log.e("lianghuan","res =- "+res);

    }

    private ApMonitor mApMonitor = new ApMonitor() {
        @Override
        public void onApOpening() {
            Log.e("lianghuan","opening");
        }

        @Override
        public void onApOpened() {
            Log.e("lianghuan","opened");
            SystemClock.sleep(1000);
//            String hotspotLocalIpAddress = ApUtils.getHotspotLocalIpAddress(MainActivity.this);
//            Log.e("lianghuan","hotspotLocalIpAddress = "+hotspotLocalIpAddress);
            mServer.start();
        }

        @Override
        public void onApCloseing() {
            Log.e("lianghuan","closeing");
        }

        @Override
        public void onApClosed() {
            Log.e("lianghuan","closed");
        }

        @Override
        public void onApIpAddress(String ip) {
            Log.e("lianghuan","ip = "+ip);
        }
    };
}
