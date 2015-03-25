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
 * Copyright (C) 2011 The Android Open Source Project
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

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Bundle;
import android.nfc.NfcAdapter;
import android.nfc.NfcEvent;
import android.nfc.NdefRecord;
import android.nfc.NdefMessage;
import java.nio.charset.Charset;
import android.nfc.tech.NfcF;
import android.app.PendingIntent;
import android.content.IntentFilter;
import android.content.IntentFilter.MalformedMimeTypeException;

public class NfcTestActivity extends Activity 
    implements NfcAdapter.CreateNdefMessageCallback, NfcAdapter.OnNdefPushCompleteCallback {

    static final String EXTRA_LAUNCH_INTENT = "launchIntent";
    
    private NdefRecord createMimeRecord(String mimeType, byte[] payload) {        
        byte[] mimeBytes = mimeType.getBytes(Charset.forName("US-ASCII"));        
        NdefRecord mimeRecord = new NdefRecord(NdefRecord.TNF_MIME_MEDIA, 
                                                                  mimeBytes, 
                                                                  new byte[0], 
                                                                  payload);        
        return mimeRecord;    
    }
    
    private NdefMessage createTestNdefMessage() {
        String text = ("Beam me up, Android!\n\n" + "Beam Time: " + System.currentTimeMillis());
        NdefMessage msg = new NdefMessage (new NdefRecord[] 
            { createMimeRecord("application/com.example.android.beam", text.getBytes()) });
        return msg;
    }    
    
    @Override
    public NdefMessage createNdefMessage(NfcEvent e) {
        return createTestNdefMessage();
    }
    
    @Override
    public void onNdefPushComplete(NfcEvent e) {
        return;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        int testCase = getIntent().getIntExtra("NfcTestCase", -1);
        NfcAdapter adapter = NfcAdapter.getDefaultAdapter(this);
        if (adapter == null)
            testCase = -1;
        
        switch (testCase) {
            case NfcApiTest.setNdefPushMessage:
                NdefMessage msg = createTestNdefMessage();
                adapter.setNdefPushMessage(msg, this);
                NfcApiTest.onTestFinished(true);
                break;
            case NfcApiTest.setNdefPushMessageCallback:
                adapter.setNdefPushMessageCallback(this, this);
                NfcApiTest.onTestFinished(true);
                break;
            case NfcApiTest.setOnNdefPushCompleteCallback:
                adapter.setOnNdefPushCompleteCallback(this, this);
                NfcApiTest.onTestFinished(true);
                break;
            case NfcApiTest.enableForegroundDispatch:
                PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, 
                    new Intent(this, getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), 0);
                IntentFilter ndef = new IntentFilter(NfcAdapter.ACTION_NDEF_DISCOVERED);
                try {
                    ndef.addDataType("*/*");
                } catch (MalformedMimeTypeException e) {
                    throw new RuntimeException("fail", e);                    
                }
                IntentFilter[] filters = new IntentFilter[] { ndef, };
                String[][] techLists = new String[][] { new String[] { NfcF.class.getName()}};
                try {
                    adapter.enableForegroundDispatch(this, pendingIntent, filters, techLists);
                } catch (IllegalStateException e) {
                    // only allowed to be called when onResume state,
                    // currently bypass this exception    
                }
                NfcApiTest.onTestFinished(true);
                break;
            case NfcApiTest.disableForegroundDispatch:
                try {
                    adapter.disableForegroundDispatch(this);                
                } catch (IllegalStateException e) {
                    // only allowed to be called when onResume state,
                    // currently bypass this exception
                }
                NfcApiTest.onTestFinished(true);
                break;
            default:
                break;
        }
        
        finish();
    }
}
