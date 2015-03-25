/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERfETO. RECEIVER EXPRESSLY ACKNOWLEDGES
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

package com.mediatek.engineermode.modemdebug;

import android.app.Activity;
import android.view.View;
import android.view.View.OnClickListener;
import android.os.Bundle;
import android.os.SystemProperties;
import android.widget.Button;
import android.widget.EditText;
import android.widget.CheckBox;
import android.widget.Toast;
import android.provider.Settings;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Arrays;

public class ModemDebug extends Activity implements OnClickListener {
    private static final String TAG = "EM/MODEM_DEBUG";
    private static final String KEY_MODEM_RESET = "mediatek.debug.md.reset.wait";
    
    private EditText mEtResetTime = null;
    private Button  mBtDone = null;
    private int mDelayTime = 0;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.modem_debug);
        
        mEtResetTime = (EditText) findViewById(R.id.Modem_Reset_Time_Edit);

        mBtDone = (Button) findViewById(R.id.Modem_Reset_Done);
        mBtDone.setOnClickListener(this);
        mDelayTime = SystemProperties.getInt(KEY_MODEM_RESET, 0);
        
        mEtResetTime.setText(String.valueOf(mDelayTime)); 
    }

    @Override
    public void onClick(View view) {
        Xlog.d(TAG, "view_id = " + view.getId());
        if (view.getId() == mBtDone.getId()) {
            if(checkInputValue() == true){
                onClickBtnDone();
            }
        } 
    }
    private boolean checkInputValue(){
        String str = mEtResetTime.getText().toString();
        if((null == str) || str.equals("")){
            Toast.makeText(this, R.string.modem_debug_input_error,
                    Toast.LENGTH_LONG).show();
            return false;
        }
        return true;
    }
    private void onClickBtnDone() {
        try {
            mDelayTime = Integer.valueOf(mEtResetTime.getText().toString());
            SystemProperties.set(KEY_MODEM_RESET, mEtResetTime.getText().toString());   
            Xlog.v(TAG, "Set modem debug delay time : " + mDelayTime);
            Toast.makeText(this, R.string.modem_debug_sucess,
                    Toast.LENGTH_LONG).show();
        } catch (NumberFormatException e) {
            Xlog.v(TAG, "NumberFormatException: " + e.getMessage());
            Toast.makeText(this, R.string.modem_debug_input_error,
                    Toast.LENGTH_LONG).show();
        }
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

}
