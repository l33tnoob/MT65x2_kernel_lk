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

import android.app.Instrumentation;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityUnitTestCase;
import android.widget.ListView;

import com.mediatek.engineermode.R;
import com.mediatek.engineermode.dualtalknetworkinfo.NetworkInfoSimType;

public class NetworkDualTest extends ActivityUnitTestCase<NetworkInfoSimType> {
    private static final int LIST_ITEM = 2;
    private int mListitem;
    private Instrumentation mInst;
    private Context mContext;
    private Intent mIntent;

    public NetworkDualTest() {
    super(NetworkInfoSimType.class);
    }

    protected void setUp() throws Exception {
    super.setUp();
    mInst = this.getInstrumentation();
    mContext = mInst.getContext();
    mIntent = new Intent(Intent.ACTION_MAIN);
    mIntent.setComponent(new ComponentName(mContext, NetworkInfoSimType.class
            .getName()));
    }

    public void test01_Precondition() {
    assertNotNull(mInst);
    assertNotNull(mContext);
    NetworkInfoSimType activity = startActivity(mIntent, null, null);
    assertNotNull(activity);
    ListView listView = (ListView) activity
            .findViewById(R.id.ListView_dualtalk_networkinfo);
    assertNotNull(listView);
    }

    public void test02_CheckLisItem() {
    NetworkInfoSimType activity = startActivity(mIntent, null, null);
    ListView listView = (ListView) activity
            .findViewById(R.id.ListView_dualtalk_networkinfo);
    int actual = listView.getAdapter().getCount();
    assertEquals(LIST_ITEM, actual);
    }

    protected void tearDown() throws Exception {
    super.tearDown();
    }

}
