/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.appguide.plugin.contacts;

import android.app.Activity;
import android.app.Dialog;
import android.app.StatusBarManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import com.mediatek.appguide.plugin.R;
import com.mediatek.contacts.ext.ContactAccountExtension;
import com.mediatek.contacts.ext.ContactAccountExtension.OnGuideFinishListener;
import com.mediatek.contacts.ext.ContactPluginDefault;
import com.mediatek.pluginmanager.PluginLayoutInflater;
import com.mediatek.telephony.SimInfoManager;
import com.mediatek.telephony.SimInfoManager.SimInfoRecord;
import com.mediatek.xlog.Xlog;

import java.util.List;

public class SwitchSimContactsExt extends ContactAccountExtension {
    private static final String TAG = "SwitchSimContactsExt";
    private static final String DIALTACTS = "DialtactsActivity";
    private static final String PEOPLE = "activities.PeopleActivity";
    private static final String PHONE = "PHONE";
    private static final String CONTACTS = "CONTACTS";
    private static final int MULTI_SIM = 2;

    private static final String SHARED_PREFERENCE_NAME = "application_guide";
    private static final String KEY_VCS_GUIDE = "vcs_guide";
    private SharedPreferences mSharedPrefs;
    private Context mContext;
    private Dialog mAppGuideDialog;
    private OnGuideFinishListener mFinishListener;

    public SwitchSimContactsExt(Context context) {
        mContext = context;
    }

    /**
     * Called when the app want to show application guide
     * 
     * @param activity
     *            : The parent activity
     * @param type
     *            : The app type, such as "CONTACTS"
     */
    public void switchSimGuide(Activity activity, String type, String commd) {
        String name = activity.getLocalClassName();
        Xlog.d(TAG, "activity name:" + name + ",commd:" + commd);
        if (!ContactPluginDefault.COMMD_FOR_AppGuideExt.equals(commd)) {
            return;
        }
        if (DIALTACTS.equals(name)) {
            type = PHONE;
        } else if (PEOPLE.equals(name)) {
            type = CONTACTS;
        } else {
            return;
        }
        Xlog.d(TAG, "type:" + type);
        List<SimInfoRecord> simList = SimInfoManager.getInsertedSimInfoList(activity);
        if (simList.size() < MULTI_SIM) {
            Xlog.d(TAG, "sim card number is : " + simList.size());
            return;
        }
        StatusBarManager manager = (StatusBarManager) activity.getSystemService(Context.STATUS_BAR_SERVICE);
        manager.showApplicationGuide(type);
    }

    /**
     * Called when the app want to show VCS application guide
     * 
     * @param activity
     *            The parent activity
     * @param commd
     *            The commd fotrwhich Plugin Implements will run
     */
    public boolean setVcsAppGuideVisibility(Activity activity, boolean isShow, OnGuideFinishListener onFinishListener,
            String commd) {
        if (!ContactPluginDefault.COMMD_FOR_AppGuideExt.equals(commd)) {
            return false;
        }
        if (isShow) {
            mFinishListener = onFinishListener;
            mSharedPrefs = activity.getSharedPreferences(SHARED_PREFERENCE_NAME, Context.MODE_WORLD_WRITEABLE);
            if (mSharedPrefs.getBoolean(KEY_VCS_GUIDE, false)) {
                Xlog.d(TAG, "already show VCS guide, return");
                return false;
            }
            Xlog.d(TAG, "showVcsAppGuide");
            if (mAppGuideDialog == null) {
                Xlog.d(TAG, "mAppGuideDialog == null");
                mAppGuideDialog = new AppGuideDialog(activity);
                mAppGuideDialog.getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,
                        WindowManager.LayoutParams.MATCH_PARENT);
            }
            mAppGuideDialog.show();
            return true;
        } else {
            dismissVcsAppGuide();
            return false;
        }
    }

    private void dismissVcsAppGuide() {
        if (mAppGuideDialog != null) {
            Xlog.d(TAG, "dismissVcsAppGuide");
            mAppGuideDialog.dismiss();
            mAppGuideDialog = null;
        }
    }

    private void onGuideFinish() {
        if (mFinishListener != null) {
            Xlog.d(TAG, "onGuideFinish");
            mFinishListener.onGuideFinish();
        }
    }
    class AppGuideDialog extends Dialog {

        private Activity mActivity;
        private Button mOkBtn;

        /**
         * ok button listner, finish app guide.
         */
        private View.OnClickListener mOkListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSharedPrefs.edit().putBoolean(KEY_VCS_GUIDE, true).commit();
                onGuideFinish();
                onBackPressed();
            }
        };

        public AppGuideDialog(Activity activity) {
            super(activity, android.R.style.Theme_Translucent_NoTitleBar);
            mActivity = activity;
        }

        @Override
        public void onBackPressed() {
            dismissVcsAppGuide();
            onGuideFinish();
            super.onBackPressed();
        }

        @Override
        protected void onCreate(Bundle savedInstanceState) {
            PluginLayoutInflater inflater = new PluginLayoutInflater(mContext);
            View view = inflater.inflate(R.layout.vcs_guide_layout, null);
            mOkBtn = (Button) view.findViewById(R.id.ok_btn);
            mOkBtn.setText(android.R.string.ok);
            mOkBtn.setOnClickListener(mOkListener);
            setContentView(view);
        }
    }
}
