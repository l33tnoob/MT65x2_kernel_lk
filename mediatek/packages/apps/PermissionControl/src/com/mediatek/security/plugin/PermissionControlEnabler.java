/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.security.plugin;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.preference.Preference;
import android.provider.Settings;

import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.ui.UiUtils;
import com.mediatek.xlog.Xlog;
/**
 * BluetoothEnabler is a helper to manage the Bluetooth on/off checkbox
 * preference. It turns on/off Bluetooth and ensures the summary of the
 * preference reflects the current state.
 */
public final class PermissionControlEnabler implements Preference.OnPreferenceChangeListener {
    private static final String TAG = "PermControlEnabler";
    
    private final Context mContext;
    private CustomizedSwitchPreference mSwitchPreference;
    /**
     * add content obsever to connect with other permission management app
     * */
    private final UiUtils.SwitchContentObserver mSwitchContentObserver = new UiUtils.SwitchContentObserver(
            new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            boolean enable = PermControlUtils.isInHouseEnabled(mContext);
            Xlog.d(TAG, "onChange(), update switch " + enable);
            mSwitchPreference.setEnabled(enable);
        }
    };    

    public PermissionControlEnabler(Context context, CustomizedSwitchPreference preference) {
        mContext = context;
        mSwitchPreference = preference;
    }
    
    public void resume() {
        Xlog.d(TAG, "resume()");
        if (mSwitchPreference != null) {
            mSwitchPreference.setOnPreferenceChangeListener(this);
        }
        boolean isCheck = PermControlUtils.isPermControlOn(mContext);
        Xlog.d(TAG, "isPermissionControlOn = " + isCheck);
        setSwitchChecked(isCheck);

        // register observer to enable/disable the switch
        // for the case: other permssion manage apk is installed or
        // uninstalled(Tecent)
        mSwitchContentObserver.register(mContext.getContentResolver());
    }

    public void pause() {
        Xlog.d(TAG, "pause()");
        if (mSwitchPreference != null) {
            mSwitchPreference.setOnPreferenceChangeListener(null);
        }
        // unregister observer
        mSwitchContentObserver.unregister(mContext.getContentResolver());
    }

    private void setSwitchChecked(boolean checked) {
        if (ActivityManager.isUserAMonkey()) {
            Xlog.d(TAG, "Monkey is running");
            return;
        }
        if (mSwitchPreference != null) {
            mSwitchPreference.setChecked(checked);
        }
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object value) {
        if (ActivityManager.isUserAMonkey()) {
            Xlog.d(TAG, "Monkey is running");
            return false;
        }

        if (preference == mSwitchPreference) {
            boolean isChecked = (Boolean) value;
            Xlog.d(TAG, "onPreferenceChange, isChecked = " + isChecked);
            if (isChecked) {
                setSwitchChecked(isChecked);
                PermControlUtils.enablePermissionControl(isChecked, mContext);
            } else {
                // get the value from provider , it's 0 by default
                boolean isShowDlg = Settings.System.getInt(
                        mContext.getContentResolver(),
                        PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, 0) == 0;
                Xlog.d(TAG, "onPreferenceChange, isShowDlg = " + isShowDlg);

                if (isShowDlg) {
                    Intent intent = new Intent();
                    intent.setAction(UiUtils.ACTION_SWITCH_OFF_CONTROL_FROM_SECURITY);
                    /*
                     * must add the flag , or will have the exception: Calling
                     * startActivity() from outside of an Activity context
                     * requires the FLAG_ACTIVITY_NEW_TASK flag.
                     */
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(intent);
                } else {
                    setSwitchChecked(isChecked);
                    PermControlUtils.enablePermissionControl(isChecked,
                            mContext);
                }
            }
        }
        return false;
    }

}
