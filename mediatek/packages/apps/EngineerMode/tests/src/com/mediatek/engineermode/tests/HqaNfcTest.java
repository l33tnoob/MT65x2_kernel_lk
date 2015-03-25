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
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;
import com.mediatek.engineermode.R;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.hqanfc.NfcMainPage;

import java.util.ArrayList;

public class HqaNfcTest extends ActivityInstrumentationTestCase2<NfcMainPage> {

    private Solo mSolo;
    private Context mContext;
    private Instrumentation mIns;
    private Activity mActivity;
    private static boolean sFinished = false;
    
    public HqaNfcTest() {
        super("com.mediatek.engineermode", NfcMainPage.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        
    }
    public void testCase_preCheck() {
        boolean nfc_enabled = mSolo.searchText(mActivity.getString(R.string.hqa_nfc_dialog_warn_message));
        
        if(nfc_enabled) {
            sFinished = false;
            mSolo.clickOnText(mActivity.getString(R.string.ok));
        }
    }

    public void testCase01_TestReaderMode() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Always in reader mode");      

        mSolo.clickOnText("Clear All");
        mSolo.clickOnText("Default");
        mSolo.clickOnText("Return");
    }

    public void testCase02_TestP2pMode() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Always in peer to peer mode");
        mSolo.clickOnText("TypeA");
        mSolo.clickOnCheckBox(0);
        mSolo.clickOnText("TypeF");
        mSolo.clickOnText("Clear All");
        mSolo.clickOnText("Default");
        mSolo.clickOnText("Return");
    }

    public void testCase03_TestCardEmul() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Always in card emulation mode");
        mSolo.clickOnText("Clear All");
        mSolo.clickOnText("Default");
        mSolo.clickOnText("Return");
    }

    public void testCase04_TestTxSignal() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("TX carrier signal always on");
        mSolo.clickOnText("Return");
    }

    public void testCase05_TestVirtualCard() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Virtual card function");
        mSolo.clickOnText("TypeF");
        mSolo.clickOnText("Clear All");
        mSolo.clickOnText("Default");
        mSolo.clickOnText("Return");
    }

    public void testCase06_TestPnfcCommand() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("PNFC command");
//        mSolo.enterText(mSolo.getCurrentEditTexts().get(0), "0x00");
        mSolo.clickOnText("Return");
    }

    public void testCase07_TestPollingLoopMode() {
        if(sFinished == false){
            return;
        }
        verifyPreconditions();
        ArrayList<ListView> views = mSolo.getCurrentViews(ListView.class);
        assertEquals(1, views.size());
        mSolo.clickOnText("Polling loop mode");
//        assertEquals("500", mSolo.getCurrentEditTexts().get(0).getText());
//        mSolo.enterText(mSolo.getCurrentEditTexts().get(0), "100");

        mSolo.clickOnText("Pause");
        mSolo.clickOnText("Listen");
        mSolo.clickOnText("Reader Mode");
        mSolo.clickOnText("dual subcarrier");
        mSolo.clickOnText("subcarrier");
        mSolo.clickOnText("P2P Mode");

        mSolo.clickOnText("Card Emulation Mode");

        mSolo.clickOnText("Clear All");
        mSolo.clickOnText("Default");
        mSolo.clickOnText("Return");
    }

    private void verifyPreconditions() {
        assertTrue(mIns != null);
        assertTrue(mActivity != null);
        assertTrue(mContext != null);
        assertTrue(mSolo != null);
    }
}
