package com.mediatek.wifisdiotest;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiConfiguration.AuthAlgorithm;
import android.net.wifi.WifiConfiguration.GroupCipher;
import android.net.wifi.WifiConfiguration.KeyMgmt;
import android.net.wifi.WifiConfiguration.PairwiseCipher;
import android.net.wifi.WifiConfiguration.Protocol;
import android.net.wifi.WifiEnterpriseConfig;
import android.net.wifi.WifiEnterpriseConfig.Eap;
import android.net.wifi.WifiEnterpriseConfig.Phase2;
import android.net.wifi.WifiManager;
import android.os.SystemClock;

import com.mediatek.xlog.Xlog;

import java.util.List;

public class WifiAdmService extends IntentService {

    private static final String TAG = "EM/WifiAdmService";
    private static final String WORKER_THREAD_NAME = "WifiAdminService";
    private static final int WIFI_OPT_MAX_SLEEP = 200;
    private static final int WIFI_OPT_MAX_LOOP = 50;
    public static final String WIFI_ADM_ACTION = "com.mediatek.wifisdiotest.wifi_adm";
    public static final String KEY_OPT = "wifi_optcode";
    public static final String KEY_WIFI_CONFIG_SSID = "SSID";
    public static final String KEY_WIFI_CONFIG_SECTYPE = "security_type";
    public static final String KEY_WIFI_CONFIG_PASSWORD = "password";
    public static final String KEY_WIFI_CONFIG_IDENTITY = "identity";
    public static final String KEY_FORCE_CONNECT = "force_connect";

    public static final int WIFI_OPT_NOTHING = 0;
    public static final int WIFI_OPT_ENABLE = 1;
    public static final int WIFI_OPT_SCAN = 2;
    public static final int WIFI_OPT_CONNECT = 3;
    public static final int WIFI_OPT_DISCONNECT = 4;
    public static final int WIFI_OPT_DISABLE = 5;

    public static final int SECURITY_NONE = 0;
    public static final int SECURITY_WEP = 1;
    public static final int SECURITY_PSK = 2;
    public static final int SECURITY_WPA_PSK = 3;
    public static final int SECURITY_WPA2_PSK = 4;
    public static final int SECURITY_EAP = 5;
    public static final int SECURITY_WAPI_PSK = 6;
    public static final int SECURITY_WAPI_CERT = 7;

    private static WifiManager mWifiManager = null;
    private WifiConfiguration mConfig = null;

    public WifiAdmService() {
        super(WORKER_THREAD_NAME);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        if(null == mWifiManager){
            mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        }
        mConfig = new WifiConfiguration();
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (WIFI_ADM_ACTION.equals(intent.getAction()) && null != mWifiManager) {
            int optCode = intent.getIntExtra(KEY_OPT, WIFI_OPT_NOTHING);
            Xlog.v(TAG, "Wifi opt code: " + optCode);
            switch (optCode) {
            case WIFI_OPT_NOTHING:
                break;
            case WIFI_OPT_ENABLE:
                enableWifi();
                break;
            case WIFI_OPT_SCAN:
                doScan();
                break;
            case WIFI_OPT_CONNECT:
                doConnect(intent);
                break;
            case WIFI_OPT_DISCONNECT:
                doDisconnect();
                break;
            case WIFI_OPT_DISABLE:
                disableWifi();
                break;
            default:
                break;
            }
        }
    }

    private void enableWifi() {
        if (!mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(true);
            Xlog.i(TAG, "Enabling Wifi");
            int i = 0;
            while (!mWifiManager.isWifiEnabled() && i < WIFI_OPT_MAX_LOOP) {
                SystemClock.sleep(WIFI_OPT_MAX_SLEEP);
                i++;
            }
            if (WIFI_OPT_MAX_LOOP == i) {
                Xlog.w(TAG, "Enable Wifi failed");
            } else {
                Xlog.i(TAG, "Enable Wifi success: " + i);
            }
        }
    }

    private void doScan() {
        Xlog.i(TAG, "doScan");
        mWifiManager.startScan();
    }

    private void doConnect(Intent intent) {
        Xlog.i(TAG, "connect AP");
        doDisconnect();
        mConfig.SSID = convertToQuotedString(intent
                .getStringExtra(KEY_WIFI_CONFIG_SSID));
        if (isConfigured(mConfig)) {
            connectNetwork(mConfig.networkId, true);
        } else {
            setSecurityType(intent);
            int networkId = mWifiManager.addNetwork(mConfig);
            if (-1 != networkId) {
                mWifiManager.enableNetwork(networkId, false);
                connectNetwork(networkId, true);
            }
        }
    }

    private void doDisconnect() {
        // int netId = mWifiManager.getConnectionInfo().getNetworkId();
        // mWifiManager.disableNetwork(netId);
        // mWifiManager.removeNetwork(netId);
        // Xlog.i(TAG, netId + "");
        mWifiManager.disconnect();
        // if (mWifiManager.isWifiEnabled()) {
        // mWifiManager.setWifiEnabled(false);
        // }
    }

    private void disableWifi() {
        if (mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(false);
            Xlog.i(TAG, "Disabling Wifi");
            int i = 0;
            while (mWifiManager.isWifiEnabled() && i < WIFI_OPT_MAX_LOOP) {
                SystemClock.sleep(WIFI_OPT_MAX_SLEEP);
                i++;
            }
            if (WIFI_OPT_MAX_LOOP == i) {
                Xlog.w(TAG, "Disable Wifi failed");
            } else {
                Xlog.i(TAG, "Disable Wifi success: " + i);
            }
        }
    }

    private void connectNetwork(int id, boolean updateConfig) {
        if (updateConfig) {
            WifiConfiguration config = new WifiConfiguration();
            config.networkId = id;
            config.priority = getMaxPriority() + 1;
            mWifiManager.updateNetwork(config);
            mWifiManager.saveConfiguration();
            mWifiManager.enableNetwork(id, true);
            mWifiManager.reconnect();
        } else {
            mWifiManager.enableNetwork(id, true);
        }
    }

    private boolean isConfigured(WifiConfiguration wifiConfig) {
        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        for (WifiConfiguration config : configs) {
            if (config != null && config.SSID.equals(wifiConfig.SSID)) {
                Xlog.v(TAG, "Wifi configured: SSID: " + config.SSID
                        + " networkID: " + config.networkId);
                wifiConfig.networkId = config.networkId;
                return true;
            }
        }
        return false;
    }

    private int getMaxPriority() {
        int max = 0;
        List<WifiConfiguration> configs = mWifiManager.getConfiguredNetworks();
        for (WifiConfiguration config : configs) {
            if (config != null) {
                if (config.priority > max) {
                    max = config.priority;
                }
            }
        }
        return max;
    }

    private String convertToQuotedString(String string) {
        return "\"" + string + "\"";
    }

    private void setSecurityType(Intent intent) {
        int securityType = intent.getIntExtra(KEY_WIFI_CONFIG_SECTYPE,
                SECURITY_NONE);
        String password = intent.getStringExtra(KEY_WIFI_CONFIG_PASSWORD);
        switch (securityType) {
        case SECURITY_NONE:
            mConfig.allowedKeyManagement.set(KeyMgmt.NONE);
            break;
        case SECURITY_WEP:
            mConfig.allowedKeyManagement.set(KeyMgmt.NONE);
            mConfig.allowedAuthAlgorithms.set(AuthAlgorithm.OPEN);
            mConfig.allowedAuthAlgorithms.set(AuthAlgorithm.SHARED);
            int length = password.length();
            if (length != 0) {
                // get selected WEP key index
                int keyIndex = 0;// selected password index, 0~3
                // WEP-40, WEP-104, and 256-bit WEP (WEP-232?)
                mConfig.wepKeys[keyIndex] = password;
                mConfig.wepTxKeyIndex = keyIndex;
            }
            break;
        case SECURITY_WPA_PSK:
        case SECURITY_WPA2_PSK:
        case SECURITY_PSK:
            mConfig.allowedKeyManagement.set(KeyMgmt.WPA_PSK);
            if (password.length() != 0) {
                if (password.matches("[0-9A-Fa-f]{64}")) {
                    mConfig.preSharedKey = password;
                } else {
                    mConfig.preSharedKey = '"' + password + '"';
                }
            }
            if (securityType == SECURITY_WPA_PSK) {
                mConfig.allowedPairwiseCiphers.set(PairwiseCipher.TKIP);
            } else if (securityType == SECURITY_WPA2_PSK) {
                mConfig.allowedPairwiseCiphers.set(PairwiseCipher.CCMP);
            }
            break;
        case SECURITY_EAP:
            String identity = intent.getStringExtra(KEY_WIFI_CONFIG_IDENTITY);
            mConfig.allowedKeyManagement.set(KeyMgmt.WPA_EAP);
            mConfig.allowedKeyManagement.set(KeyMgmt.IEEE8021X);
            mConfig.enterpriseConfig = new WifiEnterpriseConfig();
            mConfig.enterpriseConfig.setEapMethod(Eap.PEAP);  //PEAP: 0, TLS: 1, TTLS: 2, PWD: 3, NONE: -1
            mConfig.enterpriseConfig.setPhase2Method(Phase2.NONE);
            mConfig.enterpriseConfig.setClientCertificateAlias("");
            mConfig.enterpriseConfig.setCaCertificateAlias("");
            mConfig.enterpriseConfig.setIdentity(identity);
            mConfig.enterpriseConfig.setAnonymousIdentity("");
           
            // mConfig.enterpriseFields[0];
            //mConfig.eap.setValue("PEAP");
            //mConfig.phase2.setValue("");
            //mConfig.ca_cert.setValue("");
           //mConfig.client_cert.setValue("");
            // mConfig.private_key.setValue("");
           // mConfig.identity.setValue(identity);
            //mConfig.anonymous_identity.setValue("");
            if (password.length() != 0) {
                //mConfig.password.setValue(password);
                mConfig.enterpriseConfig.setPassword(password);
            }
            break;
        case SECURITY_WAPI_PSK:
            mConfig.allowedKeyManagement.set(KeyMgmt.WAPI_PSK);
            mConfig.allowedProtocols.set(Protocol.WAPI);
            mConfig.allowedPairwiseCiphers.set(PairwiseCipher.SMS4);
            mConfig.allowedGroupCiphers.set(GroupCipher.SMS4);
            if (password.length() != 0) {
                mConfig.preSharedKey = '"' + password + '"';
            }
            break;
        case SECURITY_WAPI_CERT:
            mConfig.allowedKeyManagement.set(KeyMgmt.WAPI_CERT);
            mConfig.allowedProtocols.set(Protocol.WAPI);
            mConfig.allowedPairwiseCiphers.set(PairwiseCipher.SMS4);
            mConfig.allowedGroupCiphers.set(GroupCipher.SMS4);
            mConfig.enterpriseConfig.setClientCertificateAlias("");
            mConfig.enterpriseConfig.setCaCertificateAlias("");
            //mConfig.ca_cert2.setValue("");
            //mConfig.client_cert.setValue("");
            // mConfig.private_key.setValue("");
            break;
        default:
            break;
        }
    }
}
