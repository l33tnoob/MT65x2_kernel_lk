/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.oobe.basic;


import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.StatusBarManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.view.Surface;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.oobe.R;
import com.mediatek.oobe.ext.IOobeMiscExt;
import com.mediatek.oobe.qsg.QuickStartGuideMain;
import com.mediatek.oobe.utils.PDebug;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.oobe.WizardActivity;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.common.featureoption.FeatureOption;
import android.os.UserHandle;
public class MainActivity extends Activity {
    private static final String TAG = "OOBE.MainActivity";
    private static final String BOOT_MODE_INFO_FILE = "/sys/class/BOOT/BOOT/boot/boot_mode";
    private static final String EXTRA_IS_OOBE = "extra_is_oobe";
    private static final String EXTRA_OOBE_SETTINGS = "extra_oobe_settings";

    private static final String ACTION_SETTINGS_SETUP = "com.mediatek.settings.SETTINGS_SETUP_FOR_OOBE";
    private static final String ACTION_LANGUAGE_SETTINGS = "com.mediatek.oobe.basic.OOBE_LANGUAGE_SETTING";
    private static final String ACTION_IMPORT_CONTACTS = "com.mediatek.oobe.basic.OOBE_IMPORT_CONTACTS";
    private static final String ACTION_INTERNET_CONNECTION = "com.mediatek.oobe.basic.OOBE_INTERNET_CONNECTION";
    private static final String ACTION_ADVANCED_SETTINGS = "com.mediatek.oobe.advanced.AdvanceSettings";
    private static final String ACTION_QUICK_START_GUIDE = "com.mediatek.oobe.QUICK_START_GUIDE";

    private static final int ID_DATE_TIME_SETTINGS = 2;
    private static final int ID_SIM_INFO_SETTINGS = 3;
    private static final int ID_DEFAULT_SIM_SETTINGS = 4;
    private static final int ID_WIFI_SETTINGS = 7;

    private static final int REQUEST_CODE_ADVANCED_SETTINGS = 1001;
    private static final int QUICK_START_GUIDE_CODE = 4001;
    private static final int EVENT_MONITOR_SIM_TIME_OUT = 2001;
    private static final int EVENT_SIM_DETECTING_READY = 2002;
    private static final int DIALOG_WAITING_SIM = 3001;
    // max time to wait before SIM card is ready
    private static final int TIME_MONITOR_SIM = 30000;

    // advance meta model
    private static final int BOOT_MODE_NORMAL = 0;
    private static final int BOOT_MODE_ADV_META = 5;
    private int mBootMode = BOOT_MODE_NORMAL;

    private boolean mIsStepInitiated = false;
    private boolean mIsFirstRun;

    private int mTotalStep = 0;
    private int mCurrentStep = -1;
    private boolean mSimExist = false;
    private IOobeMiscExt mExt;

    private IntentFilter mSIMIntentFilter;
    private List<OobeActivityInfo> mStepActivityies = new ArrayList<OobeActivityInfo>();
            
    private OobeActivityInfo mLanguageSettings = 
                new OobeActivityInfo(ACTION_LANGUAGE_SETTINGS, null);
    private OobeActivityInfo mDateTimeSettings = 
                new OobeActivityInfo(ACTION_SETTINGS_SETUP, ID_DATE_TIME_SETTINGS);
    private OobeActivityInfo mSimInfoSettings = 
                new OobeActivityInfo(ACTION_SETTINGS_SETUP, ID_SIM_INFO_SETTINGS);
    private OobeActivityInfo mDefaultSimSettings = 
                new OobeActivityInfo(ACTION_SETTINGS_SETUP, ID_DEFAULT_SIM_SETTINGS);
    private OobeActivityInfo mContactsSettings = 
                new OobeActivityInfo(ACTION_IMPORT_CONTACTS, null);
    private OobeActivityInfo mInternetSettings = 
                new OobeActivityInfo(ACTION_INTERNET_CONNECTION, null);
    private OobeActivityInfo mWifiSettings = 
                new OobeActivityInfo(ACTION_SETTINGS_SETUP, ID_WIFI_SETTINGS);

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(android.os.Message msg) {
            switch (msg.what) {
            case EVENT_MONITOR_SIM_TIME_OUT:
                Xlog.v(TAG, "handler wait SIM time out");
                break;
            case EVENT_SIM_DETECTING_READY:
                Xlog.v(TAG, "handler SIM initialization finish");
                mHandler.removeMessages(EVENT_MONITOR_SIM_TIME_OUT);
                break;
            default:
                break;
            }
            initStep();
            removeDialog(DIALOG_WAITING_SIM);
        };
    };

    BroadcastReceiver mSIMStateReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction(); 
            Xlog.d(TAG, "receive action:" + action);
            if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
                List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(MainActivity.this);
                if (simList != null) {
                    mSimExist = simList.size() > 0;
                }
                mHandler.sendEmptyMessage(EVENT_SIM_DETECTING_READY);
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        PDebug.Start("MainActivity.onCreate");
        super.onCreate(savedInstanceState);

        mSimExist = isSimExist();
        Xlog.d(TAG, "onCreate mSimExist = " + mSimExist);

        if (savedInstanceState != null) {
            mCurrentStep = savedInstanceState.getInt("currentStep");
            mTotalStep = savedInstanceState.getInt("totalSteps");
            Xlog.d(TAG, "restore saved instance state mCurrentStep=" + mCurrentStep + "mTotalStep="
                    + mTotalStep);
            setActivityList();
        }
        PDebug.Start("disableQSG");        
        //disable quick start guide for tablet
        disableQSG();
        PDebug.EndAndStart("disableQSG", "getOobeMiscPlugin");
        mIsFirstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0) == 0;
        if (mIsFirstRun) {
            Settings.System.putInt(getContentResolver(),
                Settings.System.OOBE_DISPLAY, Settings.System.OOBE_DISPLAY_ON);
            if (!isTablet() || UserHandle.myUserId() == UserHandle.USER_OWNER) {
                Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 0);
            }
            enableStatusBar(false);
        }
        mExt = Utils.getOobeMiscPlugin(this);
        PDebug.End("getOobeMiscPlugin");
        mBootMode = getBootMode();       
        if (mIsFirstRun && mBootMode != BOOT_MODE_ADV_META && !Utils.isWifiOnly(this)) {
            mHandler.sendEmptyMessageDelayed(EVENT_MONITOR_SIM_TIME_OUT, TIME_MONITOR_SIM);
            showDialog(DIALOG_WAITING_SIM);
        } else {
            mHandler.sendEmptyMessage(EVENT_SIM_DETECTING_READY);
        }
        
        mSIMIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_INFO_UPDATE);
        registerReceiver(mSIMStateReceiver, mSIMIntentFilter);
        PDebug.End("MainActivity.onCreate");
    }

    /**
     * Init step at the very begining, depending on SIM card status
     */
    private void initStep() {
        if (mIsStepInitiated) {
            return;
        }
        mIsStepInitiated = true;
        setActivityList();

        if (mCurrentStep == -1) {
            nextActivity(true);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == DIALOG_WAITING_SIM) {
            ProgressDialog dialog = new ProgressDialog(this);
            String msg = mExt.replaceSimToSimUim(getString(R.string.oobe_waiting_sim));
            dialog.setMessage(msg);
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        }
        return null;
    }

    @Override
    protected void onDestroy() {
        Xlog.i(TAG, "onDestroy()");
        unregisterReceiver(mSIMStateReceiver);

        if (mIsFirstRun) {
            enableStatusBar(true);
        }

        super.onDestroy();
    }

    private boolean isSimExist() {
        PDebug.Start("isSimExist");
        boolean simExist = false;
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(this);
        if (simList != null) {
            simExist = simList.size() > 0;
        }
        PDebug.End("isSimExist");
        return simExist;
    }

    // Get the step list
    protected void setActivityList() {
        Xlog.d(TAG, "setActivityList()");
        mStepActivityies.clear();
        mStepActivityies.add(mLanguageSettings);
        mStepActivityies.add(mDateTimeSettings);
        if (mSimExist && isUserOwner()) {
            mStepActivityies.add(mSimInfoSettings);
            mStepActivityies.add(mDefaultSimSettings);
            mStepActivityies.add(mContactsSettings);
        }
        if (PhoneConstants.GEMINI_SIM_NUM == 1 && !Utils.isWifiOnly(this)) {
            mStepActivityies.add(mInternetSettings);
        }
        mStepActivityies.add(mWifiSettings);
        mTotalStep = mStepActivityies.size();
    }

    /// M: For MTK multiuser in 3gdatasms @{
    /**
     * For MTK multiuser in 3gdatasms
     * @return true if user id is USER_OWNER, false if user id is not USER_OWNER.
     */
    private boolean isUserOwner() {
        boolean isOwner = true;
        if (FeatureOption.MTK_ONLY_OWNER_SIM_SUPPORT &&
                UserHandle.myUserId() != UserHandle.USER_OWNER) {
            isOwner = false;
        }
        return isOwner;		
    }
    /// @}

    /**
     * start next step activity
     * @param nextStep true to start next step activity, false to start last step activity.
     */
    protected void nextActivity(boolean nextStep) {
        Xlog.i(TAG, "mCurrentStep:" + mCurrentStep + "mTotalStep:" + mTotalStep);

        if (nextStep) {
            mCurrentStep++;
        } else {
            mCurrentStep--;
        }
        if (mCurrentStep == mTotalStep) {
            startAdvancedSettings();
            return;
        }

        if (mCurrentStep >= 0 && mCurrentStep < mStepActivityies.size()) {

            Intent intent = mStepActivityies.get(mCurrentStep).getIntent();
            intent.putExtra(Utils.OOBE_BASIC_STEP_TOTAL, mTotalStep);
            intent.putExtra(Utils.OOBE_BASIC_STEP_INDEX, mCurrentStep + 1);
            intent.putExtra(EXTRA_IS_OOBE, true);
            startActivityForResult(intent, mCurrentStep);

            if (nextStep) {
                overridePendingTransition(R.anim.slide_right_in, R.anim.slide_left_out);
            } else {
                overridePendingTransition(R.anim.slide_left_in, R.anim.slide_right_out);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        Xlog.d(TAG, "onActivityResult resultCode = " + resultCode);
        switch (resultCode) {
            case Utils.RESULT_CODE_BACK:
                if (0 == mCurrentStep) {
                    finish();
                } else {
                    nextActivity(false);
                }
                break;
            case Utils.RESULT_CODE_NEXT:
                nextActivity(true);
                break;
            case Utils.RESULT_CODE_FINISH:
                finishOOBE();
                break;
            default:
                break;
        }
    }

    private void finishOOBE() {
        //disable WizardActivity
        PackageManager pm = getPackageManager();
        ComponentName name = new ComponentName(this, WizardActivity.class);
        int state = pm.getComponentEnabledSetting(name);
        if (state != PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
            pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                    PackageManager.DONT_KILL_APP);
        }

        if (mIsFirstRun) {
            if (isTablet()) {
                Utils.startLauncher(this);
            } else {
                // start quick start guide if OOBE is first run
                Intent intent = new Intent(ACTION_QUICK_START_GUIDE);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                intent.putExtra("mIsFirstRun", true);
                startActivity(intent);
            }

            //set device provisioned, oobe has run.
            Settings.Global.putInt(getContentResolver(), Settings.Global.DEVICE_PROVISIONED, 1);
            Settings.Secure.putInt(getContentResolver(), Settings.Secure.USER_SETUP_COMPLETE, 1);
            Settings.System.putInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 1);
            //set oobe display finish
            Settings.System.putInt(getContentResolver(),
                Settings.System.OOBE_DISPLAY, Settings.System.OOBE_DISPLAY_DEFAULT);
        }
        finish();
    }

    private void startAdvancedSettings() {
        Intent intent = new Intent(ACTION_ADVANCED_SETTINGS);
        startActivityForResult(intent, REQUEST_CODE_ADVANCED_SETTINGS);
        overridePendingTransition(R.anim.slide_right_in, R.anim.slide_left_out);
    }

    /**
     * Enable or disable the status bar 
     * @param enable true to enable, false to disable.
     */
    private void enableStatusBar(boolean enable) {
        Xlog.i(TAG, "enable status bar " + enable);
        StatusBarManager statusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);
        if (statusBarManager != null) {
            statusBarManager.disable(enable ? StatusBarManager.DISABLE_NONE : StatusBarManager.DISABLE_EXPAND);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Xlog.d(TAG, "onSaveInstanceState, mCurrentStep = " + mCurrentStep);
        outState.putInt("currentStep", mCurrentStep);
        outState.putInt("totalSteps", mTotalStep);
    }

    /**
     * Whether the device is tablet
     * @return true if the device is tablet, false if the device is phone.
     */
    private boolean isTablet() {
        String sDeviceInfo = SystemProperties.get("ro.build.characteristics");
        Xlog.d(TAG, "devide info: " + sDeviceInfo);
        return "tablet".equals(sDeviceInfo);
    }

    /**
     * disable quickstart guide in tablet
     */
    private void disableQSG() {
        if (isTablet()) {
            PackageManager pm = getPackageManager();
            ComponentName name = new ComponentName(this, QuickStartGuideMain.class);
            pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                    PackageManager.DONT_KILL_APP);
        }
    }
    /**
     * get boot mode
     * @return the content in the boot mode info file.
     */
    private int getBootMode() {
        int mode = -1;
        FileReader fr = null;
        BufferedReader br = null;
        try {
            fr = new FileReader(BOOT_MODE_INFO_FILE);
            br = new BufferedReader(fr);
            String readMode = br.readLine();
            if (readMode != null) {
                mode = Integer.parseInt(readMode);
            }
        } catch (FileNotFoundException e) {
            Xlog.d(TAG, "file not found; " + BOOT_MODE_INFO_FILE);
        } catch (IOException e) {
                Xlog.d(TAG, "read file error; " + e);
        } catch (NumberFormatException e) {
            Xlog.d(TAG, "NumberFormatException e =" + e);
        } finally {
            try {
                if (br != null) {
                    br.close();
                }
            } catch (IOException e) {
                Xlog.d(TAG, "br file close error; " + BOOT_MODE_INFO_FILE);
            }
            try {
                if (fr != null) {
                    fr.close();
                }
            } catch (IOException e) {
                Xlog.d(TAG, "fr file close error; " + BOOT_MODE_INFO_FILE);
            }
        }
        Xlog.d(TAG, "read mode;" + mode);
        return mode;
    }

    private class OobeActivityInfo {

        private Intent mIntent;

        public OobeActivityInfo(String actionName, String extraName) {
            mIntent = new Intent();
            mIntent.setAction(actionName);
            mIntent.putExtra(extraName, true);
        }

        public OobeActivityInfo(String actionName, int extraId) {
            mIntent = new Intent();
            mIntent.setAction(actionName);
            mIntent.putExtra(EXTRA_OOBE_SETTINGS, extraId);
        }

        public Intent getIntent() {
            return mIntent;
        }
    }
}
