package com.mediatek.deviceregister.test;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.test.mock.MockContext;

public class MockReceiverContext extends MockContext {
    private Context mContext;

    public MockReceiverContext(Context context) {
        mContext = context;
    }

    private Intent receivedIntent;

    @Override
    public ComponentName startService(Intent intent) {
        receivedIntent = intent;
        return null;
    }

    public Intent getReceivedIntent() {
        return receivedIntent;
    }

    @Override
    public SharedPreferences getSharedPreferences(String name, int mode) {
        return mContext.getSharedPreferences(name, mode);
    }
    
    @Override
    public String getPackageName() {        
        return mContext.getPackageName();
    }
    
}
