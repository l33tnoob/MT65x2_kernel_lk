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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityUnitTestCase;
import android.widget.Button;

import com.mediatek.engineermode.networkinfo.NetworkInfoInfomation;
import com.mediatek.engineermode.tests.EmOperate;
import com.mediatek.engineermode.R;

public class NetworkInformationTest extends
        ActivityUnitTestCase<NetworkInfoInfomation> {

    private Instrumentation mInst;
    private Context mContext;
    private Intent mIntent;
    private Activity mActivity;
    private int mChecked[];

    public NetworkInformationTest() {
    super(NetworkInfoInfomation.class);
    }

    protected void setUp() throws Exception {
    super.setUp();
    mInst = this.getInstrumentation();
    mContext = mInst.getTargetContext();
    mIntent = new Intent(Intent.ACTION_MAIN);
    mIntent.setComponent(new ComponentName(mContext,
            NetworkInfoInfomation.class.getName()));

    }

    protected void tearDown() throws Exception {
    super.tearDown();
    }

    public void test01_Precodition() {
    assertNotNull(mInst);
    assertNotNull(mContext);
    mChecked[0] = 1;
    mChecked[1] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    assertNotNull(activity);

    }

    public void test02_PageUp() {
    mChecked[2] = 1;
    mChecked[3] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    final Button mPageUp = (Button) activity.findViewById(R.id.NetworkInfo_PageUp);
    assertNotNull(mPageUp);
    EmOperate.runOnUiThread(mInst, activity, new Runnable() {

        public void run() {
        mPageUp.performClick();
        }

    });
    }

    public void test03_PageDown() {
    mChecked[4] = 1;
    mChecked[5] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    final Button mPageDown = (Button) activity
            .findViewById(R.id.NetworkInfo_PageDown);
    assertNotNull(mPageDown);
    EmOperate.runOnUiThread(mInst, activity, new Runnable() {

        public void run() {
        mPageDown.performClick();
        }

    });
    }

    public void test04_mminfo() {
    mChecked[21] = 1;
    mChecked[27] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    }

    public void test05_serv() {
    mChecked[47] = 1;
    mChecked[48] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    }

    public void test06_bler() {
    mChecked[63] = 1;
    mIntent.putExtra("mChecked", mChecked);
    NetworkInfoInfomation activity = startActivity(mIntent, null, null);
    }
}
