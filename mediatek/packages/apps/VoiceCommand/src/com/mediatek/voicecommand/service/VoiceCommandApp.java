package com.mediatek.voicecommand.service;

import android.app.Application;
import android.content.Intent;
import com.mediatek.common.voicecommand.VoiceCommandListener;

public class VoiceCommandApp extends Application {

    private Intent mVoiceServiceIntent;

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        mVoiceServiceIntent = new Intent();
        mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
        mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
        startService(mVoiceServiceIntent);
    }
}
