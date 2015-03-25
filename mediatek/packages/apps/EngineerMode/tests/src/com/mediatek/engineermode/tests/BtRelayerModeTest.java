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

package com.mediatek.engineermode.tests;


import android.app.Activity;
import android.app.Instrumentation;
import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Spinner;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.bluetooth.BtRelayerModeActivity;



public class BtRelayerModeTest extends
        ActivityInstrumentationTestCase2<BtRelayerModeActivity> {

    private static final String TAG = "BtChipInfoActivity";
    private static final String EM_BAKC_FAIL_MSG = "Back to BtList fail";
    private Solo mSolo = null;
    private Activity mActivity = null;
    private Context mContext = null;
    private Instrumentation mInst = null;   
    private BluetoothAdapter adapter = null;
//    private ContentResolver mCr = null;
 //   private ListView mListView = null;

    public BtRelayerModeTest() {
        super(BtRelayerModeActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        
        adapter = BluetoothAdapter.getDefaultAdapter();
        if (BluetoothAdapter.STATE_OFF != adapter.getState()) {
            adapter.disable();
            int jumpCount = 0;
            while ((BluetoothAdapter.STATE_OFF != adapter.getState()) && (jumpCount++ < 10)) {
                EmOperate.waitSomeTime(EmOperate.TIME_SUPER_LONG);                
            }
            return;
        }
        mInst = getInstrumentation();
        mContext = mInst.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);
 //       mListView = (ListView) mActivity.findViewById(R.id.ListView_BtList);
    }


    public void test01_Precondition() {
        assertNotNull(mInst);
        assertNotNull(mContext);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
      //  assertNotNull(mListView);
      //  mSolo.sleep(EmOperate.TIME_LONG);
    }

    public void test02_ChipInfo() {
        Activity testActivity = mSolo.getCurrentActivity();
        mSolo.assertCurrentActivity("Not BtRelayerModeActivity Class", BtRelayerModeActivity.class);

        mSolo.sleep(EmOperate.TIME_SUPER_LONG);

        mSolo.pressSpinnerItem(0, 1);
        mSolo.pressSpinnerItem(1, 2);
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);
        mSolo.clickOnButton(0);
        
        mSolo.sleep(EmOperate.TIME_SUPER_LONG);
        
        mSolo.goBack();

    }
    
   
}
