package com.mediatek.engineermode;

import android.app.Service;
import android.content.Intent;
import android.media.AudioSystem;
import android.os.Bundle;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Settings;
import android.widget.Toast;

import com.mediatek.xlog.Xlog;

public class DailyUseSettingService extends Service {

    private static final String TAG = "EM/SettingService";
    private static final String KEY_QUICK_BOOT = "quick_boot";
    private static final String KEY_IVSR = "ivsr";
    private static final String KEY_VM_LOG = "vm_log";
    private static final String SYSTEM_PROP_NAME = "ril.em.dailyuse";
    private static final String VALUE_0 = "0";
    private static final String VALUE_1 = "1";
    private static final int IPO_ENABLE = 1;
    private static final int IPO_DISABLE = 0;
    private static final int DATA_SIZE = 1444;
    private static final int VM_LOG_POS = 1440;
    private static final int SET_SPEECH_VM_ENABLE = 0x60;
    private static final int VM_ENABLE = 0;
    private static final int VM_EPL_ENABLE = 1;
    private boolean mQuickBoot = true;
    private boolean mIvsr = true;
    private boolean mVmLog = true;

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Bundle bundle = intent.getExtras();
        if (null != bundle) {
            mQuickBoot = bundle.getBoolean(KEY_QUICK_BOOT, mQuickBoot);
            mIvsr = bundle.getBoolean(KEY_IVSR, mIvsr);
            mVmLog = bundle.getBoolean(KEY_VM_LOG, mVmLog);
        }
        doSettings();
        stopSelf();
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void doSettings() {
        Xlog.v(TAG, String.format("Quick boot: %1$s, IVSR: %2$s, VM Log: %3$s",
                mQuickBoot, mIvsr, mVmLog));
        boolean ipoSetOk = true;
        boolean ivsrSetOk = true;
        boolean vmSetOk = true;
        // IPO Setting
        boolean ipoSettingEnabled = Settings.System.getInt(
                getContentResolver(), Settings.System.IPO_SETTING, IPO_DISABLE) == IPO_ENABLE;
        if (ipoSettingEnabled == mQuickBoot) {
            Xlog.v(TAG, "Skip quick boot setting");
        } else {
            ipoSetOk = Settings.System.putInt(getContentResolver(),
                    Settings.System.IPO_SETTING, mQuickBoot ? IPO_ENABLE
                            : IPO_DISABLE);
        }
        // IVSR Setting
        boolean ivsrEnabled = Settings.System.getLong(getContentResolver(),
                Settings.System.IVSR_SETTING,
                Settings.System.IVSR_SETTING_DISABLE) == Settings.System.IVSR_SETTING_ENABLE;
        if (ivsrEnabled == mIvsr) {
            Xlog.v(TAG, "Skip IVSR setting");
        } else {
            ivsrSetOk = Settings.System.putLong(getContentResolver(),
                    Settings.System.IVSR_SETTING,
                    mIvsr ? Settings.System.IVSR_SETTING_ENABLE
                            : Settings.System.IVSR_SETTING_DISABLE);
        }
        // VM Log
        byte[] audioData = new byte[DATA_SIZE];
        int ret = AudioSystem.getEmParameter(audioData, DATA_SIZE);
        if (0 == ret) {
            if (mVmLog) {
                audioData[VM_LOG_POS] |= 0x01;
            } else {
                audioData[VM_LOG_POS] &= ~(0x01);
            }
            ret = AudioSystem.setEmParameter(audioData, DATA_SIZE);
            if (0 == ret && mVmLog) {
                ret = AudioSystem.setAudioCommand(SET_SPEECH_VM_ENABLE,
                        VM_ENABLE);
            } else {
                Xlog.v(TAG, "Set EM parameter fail");
            }
        } else {
            Xlog.v(TAG, "Get EM parameter fail");
        }
        vmSetOk = ret == 0;
        Toast.makeText(
                this,
                vmSetOk && ivsrSetOk && ipoSetOk ? "Daily Use Setting Succeed"
                        : "Daily Use Setting Fail", Toast.LENGTH_SHORT).show();
        Xlog.v(TAG, String.format(
                "Quick boot set: %1$s, IVSR set: %2$s, VM Log set: %3$s",
                ipoSetOk, ivsrSetOk, vmSetOk));
        SystemProperties.set(SYSTEM_PROP_NAME,
                vmSetOk && ivsrSetOk && ipoSetOk ? VALUE_1 : VALUE_0);
    }

}
