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
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVERI129
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

package com.mediatek.oobe.utils;

import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.PreferenceActivity;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.oobe.R;
import com.mediatek.xlog.Xlog;

public class OOBEStepPreferenceActivity extends PreferenceActivity implements Button.OnClickListener {
    protected static final String TAG = "OOBEStepPreferenceActivity";
    protected LinearLayout mProgressbarLayout;
    protected Button mBackBtn;
    protected Button mNextBtn;
    // add for gesture operation
    // private View mRootView;

    protected int mTotalStep;
    protected int mStepIndex;
    protected boolean mLastStep = false;
    private String mStepSpecialTag;
    private TextView mSettingTitle;
    private WindowManager.LayoutParams mWindowLayoutParams;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.frame_layout);
        initLayout();
    }

    /**
     * set the screen mode, according table or mobile devices.
     */
    public void setScreenOrientation() {
        mWindowLayoutParams = new WindowManager.LayoutParams();
        boolean enableScreenRotation = SystemProperties.getBoolean("lockscreen.rot_override", false)
                || getResources().getBoolean(com.android.internal.R.bool.config_enableLockScreenRotation);

        if (enableScreenRotation || FeatureOption.MTK_TB_APP_LANDSCAPE_SUPPORT) {
            Xlog.d(TAG, "Rotation sensor for lock screen On!");
            mWindowLayoutParams.screenOrientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR;
        } else {
            Xlog.d(TAG, "Rotation sensor for lock screen Off!");
            mWindowLayoutParams.screenOrientation = ActivityInfo.SCREEN_ORIENTATION_NOSENSOR;
        }
    }

    /**
     * this method must be called in sub-class onCreate() method
     */
    protected void initLayout() {
        mStepSpecialTag = getStepSpecialTag();
        mTotalStep = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_TOTAL, 1);
        mStepIndex = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_INDEX, 0);
        Xlog.i(TAG, mStepSpecialTag + " initLayout(), step index = " + mStepIndex + "/" + mTotalStep);

        mSettingTitle = (TextView) findViewById(R.id.settings_title);
        mProgressbarLayout = (LinearLayout) findViewById(R.id.progressbar_layout);
        mBackBtn = (Button) findViewById(R.id.panel_button_back);
        mNextBtn = (Button) findViewById(R.id.panel_button_next);

        if (mStepIndex == 1) {
            mBackBtn.setVisibility(View.INVISIBLE);
        }
        mBackBtn.setOnClickListener(this);
        mNextBtn.setOnClickListener(this);

        for (int i = 0; i < mTotalStep; i++) {
            ImageView child = (ImageView) mProgressbarLayout.getChildAt(i);
            if (i == mStepIndex - 1) {
                child.setImageResource(R.drawable.progress_radio_on);
            }
            child.setVisibility(View.VISIBLE);
        }

        if (mTotalStep == mStepIndex) {
            Xlog.i(TAG, "Get to last settings step");
            mLastStep = true;
            mNextBtn.setText(R.string.oobe_btn_text_next);
        }
    }

    /**
     * If you want to implement your special layout in some step, like step title, inherit this and do it
     */
    public void setTitle(int titleRes) {
        mSettingTitle.setText(titleRes);
    }

    @Override
    public void onClick(View v) {
        if (v == mBackBtn) {
            if (mStepIndex == 1) {
                finishActivityByResult(Utils.RESULT_CODE_FINISH);
            } else {
                onNextStep(false);
            }
        } else if (v == mNextBtn) {
            onNextStep(true);
        }
    }

    /**
     * process to next step 
     * @param isNext boolean
     */
    public void onNextStep(boolean isNext) {
        int result = isNext ? Utils.RESULT_CODE_NEXT : Utils.RESULT_CODE_BACK;
        finishActivityByResult(result);
    }

    protected void finishActivityByResult(int resultCode) {
        Xlog.i(TAG, "Finish " + getStepSpecialTag() + ", resultCode: " + resultCode);
        Intent intent = new Intent();
        setResult(resultCode, intent);
        finish();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            int firstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0);
            if ((firstRun == 0)) {
                return true;
            }
            finishActivityByResult(Utils.RESULT_CODE_BACK);
        }
        return super.onKeyDown(keyCode, event);
    }

    protected String getStepSpecialTag() {
        return "OOBEStepPreferenceActivity";
    }

}
