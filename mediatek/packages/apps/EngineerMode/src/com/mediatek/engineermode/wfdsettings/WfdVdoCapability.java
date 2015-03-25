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

package com.mediatek.engineermode.wfdsettings;

import android.app.Activity;
import android.view.View;
import android.view.View.OnClickListener;
import android.os.Bundle;
import android.widget.Button;

import android.widget.CheckBox;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.provider.Settings;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.Elog;
import java.util.Arrays;

public class WfdVdoCapability extends Activity implements OnClickListener {
    private static final String TAG = "EM/WFD_VDO_CAP";
    private static final int RADIO_RESOLUTION_720P_1 = 0;
    private static final int RADIO_RESOLUTION_720P_2 = 1;
    private static final int RADIO_RESOLUTION_1080P = 2;
    
    
    private Button  mBtDone = null;
    //private RadioGroup mRgResolution = null;
    
    private RadioButton[] mRdResoulution =  new RadioButton[3]; // 0: 720p(30p), 1: 720p(60p), 2: 1080p(30p)
    
    private RadioGroup mRg720pDefinition = null;
    private RadioGroup mRg1080pDefinition = null;
    
    private CheckBox mCb720pSettingMenu = null;
    private CheckBox mCb1080pSettingMenu = null;


    /* 0: 720p , setting hide
     * 1: 1080p, setting off, 
     * 2: 1080p, setting on, high,
     * 3: 1080p, setting on, general.
     */
    private int mResolutionInfo = 0;  
    
    
    private final RadioGroup.OnCheckedChangeListener mCheckedListener = new RadioGroup.OnCheckedChangeListener() {
        public void onCheckedChanged(RadioGroup group, int checkedId) {
            if(group.equals(mRg720pDefinition)) {
                if(checkedId == R.id.Wfd_Vdo_720p_def_h){
                    Elog.v(TAG, "check 720-720p(60p)");
                } else if (checkedId == R.id.Wfd_Vdo_720p_def_l) {
                    Elog.v(TAG, "check 720-720p(30p)");
                }
            }
            
            if(group.equals(mRg1080pDefinition)) {
                if(checkedId == R.id.Wfd_Vdo_def_f) {
                    Elog.v(TAG, "check 1080-full");
                } else if (checkedId == R.id.Wfd_Vdo_def_h) {
                    Elog.v(TAG, "check 1080-720p(60p)");
                } else if (checkedId == R.id.Wfd_Vdo_def_l) {
                    Elog.v(TAG, "check 1080-720p(30p)");
                }
            }
        }
    };
    
    private final View.OnClickListener mRdClickListener = new View.OnClickListener() {
        public void onClick(View v) {
                boolean newState = false;
                if(v.equals(mRdResoulution[RADIO_RESOLUTION_720P_1])) {
                    newState = mRdResoulution[RADIO_RESOLUTION_720P_1].isChecked();
                    if(newState == true) {
                        mRdResoulution[RADIO_RESOLUTION_720P_2].setChecked(false);
                        mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(false);
                        mCb720pSettingMenu.setVisibility(View.GONE);
                        mRg720pDefinition.setVisibility(View.GONE);
                        mCb1080pSettingMenu.setVisibility(View.GONE);
                        mRg1080pDefinition.setVisibility(View.GONE);
                    }
                } else if (v.equals(mRdResoulution[RADIO_RESOLUTION_720P_2])) {
                    newState = mRdResoulution[RADIO_RESOLUTION_720P_2].isChecked();
                    if(newState == true) {
                        mRdResoulution[RADIO_RESOLUTION_720P_1].setChecked(false);
                        mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(false);
                        mCb720pSettingMenu.setVisibility(View.VISIBLE);
                        mRg720pDefinition.setVisibility(View.VISIBLE);
                        if(mCb720pSettingMenu.isChecked() == false) {
                            for (int i = 0; i < mRg720pDefinition.getChildCount(); i++) {
                                mRg720pDefinition.getChildAt(i).setEnabled(false);
                            }
                        }
                        mCb1080pSettingMenu.setVisibility(View.GONE);
                        mRg1080pDefinition.setVisibility(View.GONE);
                    }
                } else if (v.equals(mRdResoulution[RADIO_RESOLUTION_1080P])) {
                    newState = mRdResoulution[RADIO_RESOLUTION_1080P].isChecked();
                    if(newState == true) {
                        mRdResoulution[RADIO_RESOLUTION_720P_1].setChecked(false);
                        mRdResoulution[RADIO_RESOLUTION_720P_2].setChecked(false);
                        mCb720pSettingMenu.setVisibility(View.GONE);
                        mRg720pDefinition.setVisibility(View.GONE);
                        mCb1080pSettingMenu.setVisibility(View.VISIBLE);
                        mRg1080pDefinition.setVisibility(View.VISIBLE);
                        if(mCb1080pSettingMenu.isChecked() == false) {
                            for (int i = 0; i < mRg1080pDefinition.getChildCount(); i++) {
                                mRg1080pDefinition.getChildAt(i).setEnabled(false);
                            }
                        }
                    }
                }
            }
    };
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_vdo_capability);
        
        //mRgResolution = (RadioGroup) findViewById(R.id.Wfd_Vdo_Max_Resolution);
        
        mRdResoulution[RADIO_RESOLUTION_720P_1] = (RadioButton) findViewById(R.id.Wfd_Vdo_720p_1);
        mRdResoulution[RADIO_RESOLUTION_720P_2] = (RadioButton) findViewById(R.id.Wfd_Vdo_720p_2);
        mRdResoulution[RADIO_RESOLUTION_1080P] = (RadioButton) findViewById(R.id.Wfd_Vdo_1080p);
        
        for(int i = 0; i < 3; i++) {
            mRdResoulution[i].setOnClickListener(mRdClickListener);
        }
        mCb720pSettingMenu = (CheckBox) findViewById(R.id.Wfd_Setting_Menu_720p);
        mRg720pDefinition = (RadioGroup)findViewById(R.id.Wfd_Vdo_Definition_720p);
        
        mCb1080pSettingMenu = (CheckBox) findViewById(R.id.Wfd_Setting_Menu_1080p);
        mRg1080pDefinition = (RadioGroup)findViewById(R.id.Wfd_Vdo_Definition_1080p);
        
        mBtDone = (Button) findViewById(R.id.Wfd_Done);
        mBtDone.setOnClickListener(this);
        
        mResolutionInfo = Settings.Global.getInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_RESOLUTION, 0);
        
        setInitState(mResolutionInfo);
        
        mRg720pDefinition.setOnCheckedChangeListener(mCheckedListener);
        mRg1080pDefinition.setOnCheckedChangeListener(mCheckedListener);
        
        mCb720pSettingMenu.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCb720pSettingMenu.isChecked();
                Elog.v(TAG, "Enable720pSettingMenu : " + newState);
                for (int i = 0; i < mRg720pDefinition.getChildCount(); i++) {
                    mRg720pDefinition.getChildAt(i).setEnabled(newState);
                }
                
                if(mRg720pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_720p_def_h &&
                    mRg720pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_720p_def_l) {
                    mRg720pDefinition.check(R.id.Wfd_Vdo_720p_def_h);
                }
            }
        });
        
        mCb1080pSettingMenu.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                boolean newState = mCb1080pSettingMenu.isChecked();
                Elog.v(TAG, "Enable1080pSettingMenu : " + newState);
                for (int i = 0; i < mRg1080pDefinition.getChildCount(); i++) {
                    mRg1080pDefinition.getChildAt(i).setEnabled(newState);
                }
                
                if(mRg1080pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_def_h &&
                    mRg1080pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_def_l && 
                    mRg1080pDefinition.getCheckedRadioButtonId() != R.id.Wfd_Vdo_def_f) {
                    mRg1080pDefinition.check(R.id.Wfd_Vdo_def_f);
                }
            }
        });
    }

    @Override
    public void onClick(View view) {
        Elog.d(TAG, "view_id = " + view.getId());
        if (view.getId() == mBtDone.getId()) {
            onClickBtnDone();
            finish();
        } 
    }
    private void setInitState(int state) {
        if(state == 0) {
            mRdResoulution[RADIO_RESOLUTION_720P_1].setChecked(true);

            mCb720pSettingMenu.setVisibility(View.GONE);
            mRg720pDefinition.setVisibility(View.GONE);
            
            mCb1080pSettingMenu.setVisibility(View.GONE);
            mRg1080pDefinition.setVisibility(View.GONE);
        } else if(state == 1) {
            mRdResoulution[RADIO_RESOLUTION_720P_2].setChecked(true);
            
            mCb720pSettingMenu.setVisibility(View.VISIBLE);
            mRg720pDefinition.setVisibility(View.VISIBLE);
            
            mCb720pSettingMenu.setChecked(false);
            mRg720pDefinition.setEnabled(false);
            for (int i = 0; i < mRg720pDefinition.getChildCount(); i++) {
                mRg720pDefinition.getChildAt(i).setEnabled(false);
            }
            
            mCb1080pSettingMenu.setVisibility(View.GONE);
            mRg1080pDefinition.setVisibility(View.GONE);
        } else if(state == 2) {
            mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(true);
            
            mCb720pSettingMenu.setVisibility(View.GONE);
            mRg720pDefinition.setVisibility(View.GONE);
            
            mCb1080pSettingMenu.setVisibility(View.VISIBLE);
            mRg1080pDefinition.setVisibility(View.VISIBLE);
            mCb1080pSettingMenu.setChecked(false);
            mRg1080pDefinition.setEnabled(true);
            for (int i = 0; i < mRg1080pDefinition.getChildCount(); i++) {
                mRg1080pDefinition.getChildAt(i).setEnabled(false);
            }

        } else if(state == 3 || state == 4) {
            mRdResoulution[RADIO_RESOLUTION_720P_2].setChecked(true);
            
            mCb720pSettingMenu.setVisibility(View.VISIBLE);
            mRg720pDefinition.setVisibility(View.VISIBLE);
            
            mCb720pSettingMenu.setChecked(true);
            mRg720pDefinition.setEnabled(true);
            if(state == 3) {
                mRg720pDefinition.check(R.id.Wfd_Vdo_720p_def_h);
            } else { //state == 4
                mRg720pDefinition.check(R.id.Wfd_Vdo_720p_def_l);
            }
            
            mCb1080pSettingMenu.setVisibility(View.GONE);
            mRg1080pDefinition.setVisibility(View.GONE);
        } else if(state == 5 || state == 6 || state == 7) {
            mRdResoulution[RADIO_RESOLUTION_1080P].setChecked(true);
            
            mCb720pSettingMenu.setVisibility(View.GONE);
            mRg720pDefinition.setVisibility(View.GONE);
            
            mCb1080pSettingMenu.setVisibility(View.VISIBLE);
            mRg1080pDefinition.setVisibility(View.VISIBLE);
            mCb1080pSettingMenu.setChecked(true);
            mRg1080pDefinition.setEnabled(true);
            if(state == 5) {
                mRg1080pDefinition.check(R.id.Wfd_Vdo_def_f);
            } else if(state == 6) {
                mRg1080pDefinition.check(R.id.Wfd_Vdo_def_h);
            } else { //state ==7
                mRg1080pDefinition.check(R.id.Wfd_Vdo_def_l);
            }
        } else {
            Elog.w(TAG, "Wrong input resolution info");
        }
    }
    private void onClickBtnDone() {
        int state = 0;
        
        if(mRdResoulution[RADIO_RESOLUTION_720P_1].isChecked()) {
            state = 0;
        } else if (mRdResoulution[RADIO_RESOLUTION_720P_2].isChecked()) {
            if(mCb720pSettingMenu.isChecked() == false){
                state = 1;
            } else {
                if(mRg720pDefinition.getCheckedRadioButtonId() == R.id.Wfd_Vdo_720p_def_h){
                    state = 3;
                } else {
                    state = 4;
                }
            }
        } else {
            if(mCb1080pSettingMenu.isChecked() == false){
                state = 2;
            } else {
                if(mRg1080pDefinition.getCheckedRadioButtonId() == R.id.Wfd_Vdo_def_f){
                    state = 5;
                } else if(mRg1080pDefinition.getCheckedRadioButtonId() == R.id.Wfd_Vdo_def_h){
                    state = 6;
                } else {
                    state = 7;
                }
            }
        }

        Settings.Global.putInt(getContentResolver(), Settings.Global.WIFI_DISPLAY_RESOLUTION, state);
        
        Elog.v(TAG, "Last solution = " + state);
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

}
