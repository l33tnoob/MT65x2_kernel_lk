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

package com.mediatek.engineermode.bluetooth;

import android.app.Activity;

import android.content.Intent;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;

import android.text.Html;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.Button;

import com.mediatek.engineermode.EngineerMode;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * Do BT RX ADC Clock selection
 * @author mtk80137
 * 
 */
public class BtClockSelectionActivity extends Activity implements OnClickListener {

    private static final String TAG = "BtClockSelectionFeature";
    
    private BtTest mBtTest = null;
    private boolean mStateOn = false;  // Default the feature is off.
    
    // UI component
    private Button mSetButton;

    private boolean mHasInit = false;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        

        setContentView(R.layout.bt_clock_selection);

        TextView tv = (TextView) findViewById(R.id.ClockSelectionDesTxv);
        tv.setText(Html.fromHtml(getString(R.string.ClockSelectionDes)));
        mSetButton = (Button) findViewById(R.id.clock_select_btn);
        if(mStateOn) {
            mSetButton.setText(R.string.clock_select_turn_off);
        }else{
            mSetButton.setText(R.string.clock_select_turn_on);
        }
        mSetButton.setOnClickListener(this);
    }
    
    @Override
    protected void onResume() {
        Xlog.v(TAG, "-->onResume");
        super.onResume();
        
       // showDialog(OPEN_BT);
      //  mWorkHandler.sendEmptyMessage(OP_OPEN_BT);
    }
    @Override
    protected void onStop() {
        Xlog.v(TAG, "-->onStop");
        uninitBtTestOjbect();
        super.onStop();
    }
    
    @Override
    public void onClick(final View arg0) {

        if (arg0 == mSetButton) {
            if (mStateOn) {
                mStateOn = false;
                mSetButton.setText(R.string.clock_select_turn_on);
            } else {
                mStateOn = true;
                mSetButton.setText(R.string.clock_select_turn_off);
            }
            runHCICommand(mStateOn);
        }
    }
    
    private void runHCICommand(boolean state) {
        initBtTestOjbect();
        int cmdLen = 12;
        char[] cmd = new char[cmdLen];
        char[] response = null;
        
        Xlog.i(TAG, "-->runHCICommand");
        cmd[0] = 0x01;
        cmd[1] = 0x20;
        cmd[2] = 0xFC;
        cmd[3] = 0x08;
        cmd[4] = 0x0;
        cmd[5] = 0x0;
        cmd[6] = 0x0;
        cmd[7] = 0x0;
        cmd[8] = 0x0;
        cmd[9] = 0x0;
        cmd[10] = 0x0;
        cmd[11] = (char)(state == true ? 0x01 : 0x0);
        
        if (mBtTest == null) {
            mBtTest = new BtTest();
        }
        response = mBtTest.hciCommandRun(cmd, cmdLen);
        if (response != null) {
            String s = null;
            for (int i = 0; i < response.length; i++) {
                s = String.format("response[%d] = 0x%x", i, (long) response[i]);
                Xlog.i(TAG, s);
            }
        } else {
            Xlog.i(TAG, "HCICommandRun failed");
        }
        response = null;
           
    }
    
    // init BtTest -call init function of BtTest
    private boolean initBtTestOjbect() {
        Xlog.i(TAG, "-->initBtTestOjbect");

        if (mHasInit) {
            return mHasInit;
        }
        if (mBtTest == null) {
            mBtTest = new BtTest();
        }
        if (mBtTest != null) {
            if (mBtTest.init() != 0) {
                mHasInit = false;
                Xlog.i(TAG, "mBtTest initialization failed");
            } else {
                mHasInit = true;
            }
        }
        return mHasInit;
    }
    
    private boolean uninitBtTestOjbect() {
        Xlog.i(TAG, "-->uninitBtTestOjbect");
        if (mBtTest != null && mHasInit) {
            //runHCIResetCmd();
            if (mBtTest.unInit() != 0) {
                Xlog.i(TAG, "mBtTest un-initialization failed");
            }
        }
        mBtTest = null;
        mHasInit = false;
        return true;
    }
}
