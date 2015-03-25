package com.mediatek.ppl;

import com.mediatek.ppl.test.MockPlatformManager;

import android.app.Application;

public class PplApplication extends Application {
    private static PlatformManager sPlatformManager = null;

    @Override
    public void onCreate() {
        super.onCreate();
        sPlatformManager = new PlatformManager(this);
        // FIXME DEBUG ONLY
        // sPlatformManager = new MockPlatformManager(this);
    }

    public static PlatformManager getPlatformManager() {
        return sPlatformManager;
    }
}
