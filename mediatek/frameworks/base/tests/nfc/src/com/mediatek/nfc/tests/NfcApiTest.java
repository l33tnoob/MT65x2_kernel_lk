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

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.nfc.tests;

import android.nfc.NfcAdapter;
import android.content.Context;
import android.test.InstrumentationTestCase;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Bundle;
import java.util.concurrent.Semaphore;

public class NfcApiTest extends InstrumentationTestCase {
    private static final String TAG  = "NfcApiTest";
	private boolean mHasNfcFeature = false;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
		Context context = getInstrumentation().getContext();
        NfcAdapter adapter = NfcAdapter.getDefaultAdapter(context);
		mHasNfcFeature = adapter != null ? true : false;
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void testEnable() {
		if (!mHasNfcFeature)
			return;

		Context context = getInstrumentation().getContext();
        NfcAdapter adapter = NfcAdapter.getDefaultAdapter(context);
        assertNotNull(adapter);
        adapter.isEnabled();
    }    
    
    static final int setNdefPushMessage = 1;
    static final int setNdefPushMessageCallback = 2;
    static final int setOnNdefPushCompleteCallback = 3;
    static final int enableForegroundDispatch = 4;
    static final int disableForegroundDispatch = 5;
    
    static private Semaphore mSem = new Semaphore(0);
    static private boolean mIsOk = false;
    
    static public boolean waitForResult() {
        try {
            mSem.acquire();
        } catch (InterruptedException e) {
            return false;
        }
        return mIsOk;
    }
    
    static public void onTestFinished(boolean isOk) {
        mIsOk = isOk;
        mSem.release();
    }
    
    private void launchTestCase(int id) {
		if (!mHasNfcFeature)
			return;
        Context context = getInstrumentation().getContext();
        Intent i = new Intent(context, NfcTestActivity.class);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK); 
        i.putExtra("NfcTestCase", id);        
        context.startActivity(i);
    }
    
    public void testSetNdefPushMessage() {
		if (!mHasNfcFeature)
			return;
        launchTestCase(NfcApiTest.setNdefPushMessage);
        if (!waitForResult())
            assertNotNull(null);
    }
    
    public void testSetNdefPushMessageCallback() {
		if (!mHasNfcFeature)
			return;
        launchTestCase(NfcApiTest.setNdefPushMessageCallback);
        if (!waitForResult())
            assertNotNull(null);
    }
    
    public void testSetOnNdefPushCompleteCallback() {
		if (!mHasNfcFeature)
			return;
        launchTestCase(NfcApiTest.setOnNdefPushCompleteCallback);
        if (!waitForResult())
            assertNotNull(null);
    }
    
    public void testEnableForegroundDispatch() {    
		if (!mHasNfcFeature)
			return;
        launchTestCase(NfcApiTest.enableForegroundDispatch);
        if (!waitForResult())
            assertNotNull(null);
    }

    public void testDisableForegroundDispatch() {    
		if (!mHasNfcFeature)
			return;
        launchTestCase(NfcApiTest.disableForegroundDispatch);
        if (!waitForResult())
            assertNotNull(null);
    }

}

