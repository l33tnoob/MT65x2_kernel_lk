package com.mediatek.ppl.test;

import android.app.PendingIntent;
import android.content.Context;
import android.util.Log;

import com.mediatek.common.ppl.IPplAgent;
import com.mediatek.ppl.PlatformManager;

public class MockPlatformManager extends PlatformManager {
    private IPplAgent mAgent;
    public boolean usbMassStorageEnabled = false;

    public MockPlatformManager(Context context) {
        super(context);
        mAgent = new MockPplAgent(context);
    }

    @Override
    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    @Override
    public boolean isUsbMassStorageEnabled() {
        return usbMassStorageEnabled;
    }
}
