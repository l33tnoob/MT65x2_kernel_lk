package com.mediatek.voicecommand.service;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class VoiceCommandManagerService extends Service {

    //public static String TAG = "VCmdAppService";
    private VoiceCommandManagerStub mServiceStub ;

    @Override
    public void onCreate() {
        Log.i(VoiceCommandManagerStub.TAG, "VoiceCommandNativeService onCreate");
        mServiceStub = new VoiceCommandManagerStub(this);
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO Auto-generated method stub
        // return serviceStub;
        Log.i(VoiceCommandManagerStub.TAG, "VoiceCommandNativeService onBind");
        return mServiceStub;
    }

    @Override
    public void onDestroy() {
        // must release native memory
        Log.i(VoiceCommandManagerStub.TAG, "VoiceCommandNativeService onDestroy");
        mServiceStub.release();
    }

}
