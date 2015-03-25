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

package com.mediatek.oobe.advanced;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.xlog.Xlog;

public class AdvanceSettingsEntrance extends Activity implements Button.OnClickListener {
    private static final String TAG = "OOBE";
    
    public static final String OOBE_STEP_TOTAL = "oobe_step_total";
    public static final String OOBE_STEP_INDEX = "oobe_step_index";
    public static final String OOBE_STEP_NEXT = "oobe_step_next";
    public static final String ADVANCE_SETTING_END_ACTIVITY = "com.mediatek.android.oobe.advanced.AdvanceSettingsEnd";
    
    public static final int REQUEST_CODE = 10;

    private ImageView mAdvanceSettingsIcon;
    private Button mBackBtn;
    private Button mNextBtn;

    private String[] mActivityList = new String[] { 
                "com.mediatek.oobe.advanced.SyncSettings",
                "com.mediatek.oobe.advanced.AccountSettings"
            };

    private int mCurrentIndex = 0;
    private int mTotalStep = 2;
    private boolean mFirstRun = false;

    private View.OnClickListener mAdvancedListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            goToNextSettings(true);
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.d(TAG, "AdvanceSettingsEntrance, oncreate()");
        setContentView(R.layout.advance_settings_entrance);

        TextView title = (TextView) findViewById(R.id.settings_title);
        title.setText(R.string.basic_congratulation_title);

        mAdvanceSettingsIcon = (ImageView) findViewById(R.id.advance_settings_entrance);
        mAdvanceSettingsIcon.setOnClickListener(mAdvancedListener);

        mBackBtn = (Button) findViewById(R.id.panel_button_back);
        mBackBtn.setOnClickListener(this);

        mNextBtn = (Button) findViewById(R.id.panel_button_next);
        mNextBtn.setText(R.string.oobe_btn_text_finish);
        mNextBtn.setOnClickListener(this);

        mFirstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0) == 0;
    }

    @Override
    public void onClick(View v) {
        Xlog.v(TAG, "onClick()");
        if (v == mNextBtn) {
            finishActivityByResult(Utils.RESULT_CODE_FINISH);
        } else if (v == mBackBtn) {
            finishActivityByResult(Utils.RESULT_CODE_BACK);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == REQUEST_CODE) {
            Xlog.i(TAG, "AdvanceSettingsEntrance, request code=" 
                    + resultCode + ", current index=" + mCurrentIndex);
            switch (resultCode) {
            case Utils.RESULT_CODE_NEXT:
                goToNextSettings(true);
                break;
            case Utils.RESULT_CODE_BACK:
                goToNextSettings(false);
                break;
            case Utils.RESULT_CODE_FINISH:
                finishActivityByResult(Utils.RESULT_CODE_FINISH);
                break;
            default:
                finish();
                break;
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mFirstRun) {
                return true;
            }
            setResult(Utils.RESULT_CODE_BACK);
            finish();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void finishActivityByResult(int resultCode) {
        Xlog.v(TAG, "finishActivityByResult,resultCode: " + resultCode);
        setResult(resultCode);
        finish();
    }

    /**
     * Activity flow go back to this main activity, decide which activity will be the next one
     */
    private void goToNextSettings(boolean isNextStep) {

        if (isNextStep) {
            mCurrentIndex++;
        } else {
            mCurrentIndex--;
        }
        Xlog.v(TAG, "current index=" + mCurrentIndex);
        if (mCurrentIndex <= 0) {
            finishActivityByResult(Utils.RESULT_CODE_FINISH);
            return;
        } 

        Intent intent = new Intent();
        if (mCurrentIndex > mTotalStep) {
            //go to last step
            intent.putExtra(OOBE_STEP_TOTAL, mTotalStep);
            intent.putExtra(OOBE_STEP_INDEX, mTotalStep);
            intent.setClass(AdvanceSettingsEntrance.this, AdvanceSettingsEnd.class);
        } else {
            // go to sync or accoutn step
            String activityStr = mActivityList[mCurrentIndex - 1];
            intent.setAction(activityStr);
            intent.putExtra(AdvanceSettingsEntrance.OOBE_STEP_TOTAL, mTotalStep);
            intent.putExtra(AdvanceSettingsEntrance.OOBE_STEP_INDEX, mCurrentIndex);
        }
        startActivityForResult(intent, REQUEST_CODE);

        if (isNextStep) {
            overridePendingTransition(R.anim.slide_right_in, R.anim.slide_left_out);
        } else {
            overridePendingTransition(R.anim.slide_left_in, R.anim.slide_right_out);
        }
    }
}
