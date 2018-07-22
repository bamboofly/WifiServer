package com.example.bamboofly.wifiserver;


import android.content.Context;
import android.net.DhcpInfo;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.text.TextUtils;
import android.util.Log;

import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

/**
 * Created by AA on 2017/3/22.
 */
public class ApUtils {

    /**
     * 便携热点是否开启
     * @param context 上下文
     * @return
     */
    public static boolean isApOn(Context context) {
        WifiManager wifimanager = (WifiManager) context.getSystemService(context.WIFI_SERVICE);
        try {
            Method method = wifimanager.getClass().getDeclaredMethod("isWifiApEnabled");
            method.setAccessible(true);
            return (Boolean) method.invoke(wifimanager);
        } catch (Throwable ignored) {}
        return false;
    }

    /**
     * 关闭Wi-Fi
     * @param context 上下文
     */
    public static void closeWifi(Context context) {
        WifiManager wifimanager = (WifiManager) context.getSystemService(context.WIFI_SERVICE);
        if (wifimanager.isWifiEnabled()) {
            wifimanager.setWifiEnabled(false);
        }
    }

    /**
     * 开启便携热点
     * @param context 上下文
     * @param SSID 便携热点SSID
     * @param password 便携热点密码
     * @return
     */
    public static boolean openAp(Context context, String SSID, String password) {
        if(TextUtils.isEmpty(SSID)) {
            return false;
        }

        WifiManager wifimanager = (WifiManager) context.getSystemService(context.WIFI_SERVICE);
        if (wifimanager.isWifiEnabled()) {
            wifimanager.setWifiEnabled(false);

        }

        WifiConfiguration wifiConfiguration = getApConfig(SSID, password);
        Log.e("lianghuan","wifiConfiguration = "+wifiConfiguration);
        try {
            Log.e("lianghuan","--------1---------");
            if(isApOn(context)) {
                wifimanager.setWifiEnabled(false);
                closeAp(context);
            }
            Log.e("lianghuan","--------2---------");
            //使用反射开启Wi-Fi热点
            Method method = wifimanager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
            Log.e("lianghuan","--------3---------method = "+method);
            method.invoke(wifimanager, wifiConfiguration, true);
            Log.e("lianghuan","--------4---------");
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e("lianghuan","--------5--------- e = "+e.getLocalizedMessage());
        }
        return false;
    }

    /**
     * 关闭便携热点
     * @param context 上下文
     */
    public static void closeAp(Context context) {
        WifiManager wifimanager = (WifiManager) context.getSystemService(context.WIFI_SERVICE);
        try {
            Method method = wifimanager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
            method.invoke(wifimanager, null, false);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 获取开启便携热点后自身热点IP地址
     * @param context
     * @return
     */
    public static String getHotspotLocalIpAddress(Context context) {
        WifiManager wifimanager = (WifiManager) context.getSystemService(context.WIFI_SERVICE);
        DhcpInfo dhcpInfo = wifimanager.getDhcpInfo();
        if(dhcpInfo != null) {
            int address = dhcpInfo.serverAddress;
            return ((address & 0xFF)
                    + "." + ((address >> 8) & 0xFF)
                    + "." + ((address >> 16) & 0xFF)
                    + "." + ((address >> 24) & 0xFF));
        }
        return null;
    }

    public static String getWifiApIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface .getNetworkInterfaces(); en.hasMoreElements(); ) { NetworkInterface intf = en.nextElement(); if (intf.getName().contains("wlan")) { for (Enumeration<InetAddress> enumIpAddr = intf .getInetAddresses(); enumIpAddr.hasMoreElements(); ) { InetAddress inetAddress = enumIpAddr.nextElement(); if (!inetAddress.isLoopbackAddress() && (inetAddress.getAddress().length == 4)) { Log.d("Main", inetAddress.getHostAddress()); return inetAddress.getHostAddress(); } } } } } catch (SocketException ex) { Log.e("Main", ex.toString()); } return null; }


    /**
     * 设置有密码的热点信息
     * @param SSID 便携热点SSID
     * @param pwd 便携热点密码
     * @return
     */
    private static WifiConfiguration getApConfig(String SSID, String pwd) {
        if(TextUtils.isEmpty(pwd)) {
            return null;
        }

        WifiConfiguration config = new WifiConfiguration();
        config.SSID = SSID;
        config.preSharedKey = pwd;
//        config.hiddenSSID = true;
        config.status = WifiConfiguration.Status.ENABLED;
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
        config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
        config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
        config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
        config.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
        return config;
    }
}
