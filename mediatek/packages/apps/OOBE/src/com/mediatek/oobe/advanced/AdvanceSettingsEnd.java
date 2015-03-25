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
import android.os.Bundle;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.xlog.Xlog;

public class AdvanceSettingsEnd extends Activity implements Button.OnClickListener {
    private static final String TAG = "OOBE";
    private LinearLayout mProgressbarLayout;
    private Button mBackBtn;
    private Button mNextBtn;
    protected int mTotalStep;
    protected int mStepIndex;
    private String mStepSpecialTag;
    protected boolean mLastStep = false;
    private boolean mFirstRun = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.advance_settings_end_layout);

        initLayout();
        mFirstRun = Settings.System.getInt(getContentResolver(), Utils.OOBE_HAS_RUN_KEY, 0) == 0;
    }

    /**
     * this method must be called in sub-class onCreate() method
     */
    protected void initLayout() {
        mStepSpecialTag = getStepSpecialTag();

        mTotalStep = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_TOTAL, 1);
        mStepIndex = getIntent().getIntExtra(Utils.OOBE_BASIC_STEP_INDEX, 0);
        Xlog.i(TAG, mStepSpecialTag + "initLayout(), step index = " + mStepIndex + "/" + mTotalStep);

        mBackBtn = (Button) findViewById(R.id.panel_button_back);
        mBackBtn.setOnClickListener(this);

        mNextBtn = (Button) findViewById(R.id.panel_button_next);
        mNextBtn.setOnClickListener(this);

        TextView title = (TextView) findViewById(R.id.settings_title);
        title.setText(R.string.basic_congratulation_title);

        if (mTotalStep == mStepIndex) {
            Xlog.i(TAG, "Get to last settings step");
            mLastStep = true;
            mNextBtn.setText(R.string.oobe_btn_text_finish);
        }

    }

    @Override
    public void onClick(View v) {
        if (v == mNextBtn) {
            Xlog.d(TAG, "Finish advance settings, exit now");
            setResult(Utils.RESULT_CODE_FINISH);
            finish();
        } else if (v == mBackBtn) {
            Xlog.d(TAG, "bact to previous OOBE");
            setResult(Utils.RESULT_CODE_BACK);
            finish();
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mFirstRun) {
                return true;
            }
            setResult(Utils.RESULT_CODE_FINISH);
            finish();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    protected String getStepSpecialTag() {
        return "AdvanceSettingsEnd";
    }
}
