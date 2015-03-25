package com.mediatek.wifisdiotest;

import android.app.TabActivity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TabHost;
import android.widget.Toast;

import com.mediatek.wifisdiotest.R;

import java.io.IOException;

public class WifiSdioTestTab extends TabActivity {

    private static final String TAG = "EM/WifiSdioTab";

    private static final String IPERF = "iperf";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!checkIperf()) {
            Toast.makeText(this, R.string.wifi_sdio_toast_iperf_notready,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        TabHost tabHost = getTabHost();
        tabHost.addTab(tabHost.newTabSpec(
                getString(R.string.wifi_sdio_title_config)).setIndicator(
                getString(R.string.wifi_sdio_title_config)).setContent(
                new Intent(this, WifiSdioTestConfig.class)));
        tabHost.addTab(tabHost.newTabSpec(
                getString(R.string.wifi_sdio_title_result)).setIndicator(
                getString(R.string.wifi_sdio_title_result)).setContent(
                new Intent(this, WifiSdioTestResult.class)));
        tabHost.setCurrentTab(0);
    }

    private boolean checkIperf() {
        boolean result = false;
        try {
            Runtime.getRuntime().exec(IPERF);
            result = true;
        } catch (IOException e) {
            result = false;
        }
        return result;
    }

    // @Override
    // public void onConfigurationChanged(Configuration newConfig) {
    // super.onConfigurationChanged(newConfig);
    // }
}
