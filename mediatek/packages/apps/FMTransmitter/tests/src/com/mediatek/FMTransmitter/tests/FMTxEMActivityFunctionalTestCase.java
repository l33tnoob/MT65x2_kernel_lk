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

package com.mediatek.FMTransmitter.tests;

import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import com.mediatek.FMTransmitter.FMTxEMActivity;
import com.mediatek.FMTransmitter.FMTransmitterAdvanced;
import com.mediatek.FMTransmitter.FMTransmitterStation;
import com.mediatek.FMTransmitter.FMTxEMActivity;
import com.mediatek.FMTransmitter.FMTxEMActivity.TxDeviceStateEnum;
import com.mediatek.FMTransmitter.R;
import com.mediatek.common.featureoption.FeatureOption;

public class FMTxEMActivityFunctionalTestCase extends
        ActivityInstrumentationTestCase2<FMTxEMActivity> {

    private static final int WAIT_UI_STATE_CHANGE = 10000;//wait for advance search 
    private static final int WAIT_INITIAL = 5000;// wait for ui change time
    private static final int WAIT_SEARCH_UPDATE = 3000;// wait for search update
    private static final int WAIT_TEAR_DOWN = 1000;// wait for tear down complete
    private static final int WAIT_ADVANCED_ACTIVITY = 5000;// wait for advanced activity to set up
    private static final float SEARCH_GAP = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 0.05f : 0.1f;
    private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;// use as to get the frequency
    private static final int GET_STATUS = 500;// get status of device
    
    private Instrumentation mInstrumentation = null;
    private ActivityMonitor mActivityMonitor = null;
    private FMTxEMActivity mFMTxEMActivity = null;

    EditText    mInputMaxChannels = null;
    Button         mButtonSearch = null;
    //ListView    mSpinnerList = null;
    Spinner     mSpinnerList = null;
    ImageButton    mButtonMinus = null;
    ImageButton    mButtonPlus = null;
    EditText    mInputFrequency = null;
    //Button     gCheckFrequency = null;
    Button         mButtonSetFrequency = null;
    EditText    mMp3Path = null;
    Button         mButtonPlay = null;
    EditText    mInputRdsText = null;
    CheckBox     mSwitchRds = null;
    Button        mButtonPowerOn = null;
    
    ArrayAdapter<String> mChannelsAdapter = null;

    public FMTxEMActivityFunctionalTestCase(
            Class<FMTxEMActivity> activityClass) {
        super(activityClass);
    }

    public FMTxEMActivityFunctionalTestCase() {
        super(FMTxEMActivity.class);
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        setActivityInitialTouchMode(false);
        mFMTxEMActivity = getActivity();
        assertNotNull(mFMTxEMActivity);
        mInstrumentation = getInstrumentation();
        
        mInputMaxChannels = (EditText) mFMTxEMActivity.findViewById(R.id.inputMaxChannels);    
        mButtonSearch = (Button) mFMTxEMActivity.findViewById(R.id.searchUnoccupiedChannels);            
        mSpinnerList = (Spinner)mFMTxEMActivity.findViewById(R.id.channelsList);
        mButtonMinus = (ImageButton) mFMTxEMActivity.findViewById(R.id.minusFrequency);
        mButtonPlus = (ImageButton) mFMTxEMActivity.findViewById(R.id.plusFrequency);
        mInputFrequency = (EditText) mFMTxEMActivity.findViewById(R.id.inputFrequency);
        /*
        gCheckFrequency = (Button) findViewById(R.id.checkFrequency);
        gCheckFrequency.setOnClickListener(this);
        */
        mButtonSetFrequency = (Button) mFMTxEMActivity.findViewById(R.id.setFrequency);
        mMp3Path = (EditText) mFMTxEMActivity.findViewById(R.id.inputMp3Path);
        //mMp3Path.setEnabled(false);            
        mButtonPlay =(Button) mFMTxEMActivity.findViewById(R.id.musicAction);
        mInputRdsText = (EditText) mFMTxEMActivity.findViewById(R.id.inputRdsText);
        //mInputRdsText.setEnabled(false);            
        mSwitchRds = (CheckBox) mFMTxEMActivity.findViewById(R.id.switchRds);
        mButtonPowerOn = (Button) mFMTxEMActivity.findViewById(R.id.powerFMTx);
        
    }

    @Override
    public void tearDown() throws Exception {
        if (null != mFMTxEMActivity) {
            mFMTxEMActivity.finish();
        }
        if ((null != mActivityMonitor) && (null != mInstrumentation)) {
            mInstrumentation.removeMonitor(mActivityMonitor);
            mActivityMonitor = null;
        }
        sleep(WAIT_TEAR_DOWN);
        super.tearDown();  
    }
    
    // Test mainUIState right
    public void testcase01_MainUIStateAfterLaunch() {
        boolean playStopState = false;
        playStopState = (mFMTxEMActivity.getTxStatus() == TxDeviceStateEnum.TXPOWERUP);
        sleep(GET_STATUS);
        if (playStopState) {
            assertFalse(mButtonSearch.isEnabled());
            assertFalse(mInputMaxChannels.isEnabled());
            assertTrue(mButtonMinus.isEnabled());
            assertTrue(mButtonPlus.isEnabled());
            assertTrue(mInputFrequency.isEnabled());
            assertTrue(mButtonSetFrequency.isEnabled());
            assertTrue(mSwitchRds.isEnabled());
        } else {
            assertTrue(mButtonSearch.isEnabled());
            assertTrue(mInputMaxChannels.isEnabled());
            assertFalse(mButtonMinus.isEnabled());
            assertFalse(mButtonPlus.isEnabled());
            assertFalse(mInputFrequency.isEnabled());
            assertFalse(mButtonSetFrequency.isEnabled());
            assertFalse(mSwitchRds.isEnabled());
        }
    }
    public void testcase02_ButtonPowerOnClick() {
        boolean playStopState = false;
        playStopState = (mFMTxEMActivity.getTxStatus() == TxDeviceStateEnum.TXPOWERUP);
        sleep(GET_STATUS);
        if (playStopState) {
            buttonPowerOnClick();
            assertTrue(mButtonSearch.isEnabled());
            assertTrue(mInputMaxChannels.isEnabled());
            assertFalse(mButtonMinus.isEnabled());
            assertFalse(mButtonPlus.isEnabled());
            assertFalse(mInputFrequency.isEnabled());
            assertFalse(mButtonSetFrequency.isEnabled());
            assertFalse(mSwitchRds.isEnabled());
        } else {
           buttonPowerOnClick();
           assertFalse(mInputMaxChannels.isEnabled());
           assertTrue(mButtonMinus.isEnabled());
           assertTrue(mButtonPlus.isEnabled());
           assertTrue(mInputFrequency.isEnabled());
           assertTrue(mButtonSetFrequency.isEnabled());
           assertTrue(mSwitchRds.isEnabled());
        }
    }
    
    public void testcase03_SetMaxChannelsToSearch() {
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MOVE_END);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_5);
        searchButtonClick();
        int searchcount = FMTransmitterStation.getStationCount(
                mFMTxEMActivity,
                FMTransmitterStation.STATION_TYPE_SEARCHED);
        assertEquals(5, mSpinnerList.getCount());
    }
    
    public void testcase04_TestInputMaxChannelValidation() {
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MOVE_END);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        searchButtonClick();
        assertEquals(0, mSpinnerList.getCount());
        
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_0);
        searchButtonClick();
        assertEquals(0, mSpinnerList.getCount());
        
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_1);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_1);
        searchButtonClick();
        assertEquals(0, mSpinnerList.getCount());
    }
    
    public void testcase05_PlusAndMinusToSet() {
        boolean playStopState = false;
        float previousStationValue = Float.parseFloat(mInputFrequency.getText().toString());
        previousStationValue = Math.round(previousStationValue * BASE_NUMBER);
        previousStationValue = previousStationValue / BASE_NUMBER;
        float currentStationValue = 0;
        String [] mStrStation = null;
        playStopState = (mFMTxEMActivity.getTxStatus() == TxDeviceStateEnum.TXPOWERUP);
        sleep(GET_STATUS);
        if (!playStopState) {
            buttonPowerOnClick();
        } 
        
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonMinus.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        sleep(GET_STATUS);
        currentStationValue = Float.parseFloat(mInputFrequency.getText().toString());
        currentStationValue = Math.round(currentStationValue * BASE_NUMBER);
        currentStationValue = currentStationValue / BASE_NUMBER;
        previousStationValue = previousStationValue-SEARCH_GAP;
        if (previousStationValue < 87.5) {
            previousStationValue = 108;
        }
        assertEquals(previousStationValue,currentStationValue, 0.001);
        setButtonClick();
        mStrStation = mButtonPowerOn.getText().toString().split("    Turn ");
        assertEquals(currentStationValue, Float.parseFloat(mStrStation[0]),0.001);
        
        previousStationValue = currentStationValue;
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonPlus.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        sleep(GET_STATUS);
        currentStationValue = Float.parseFloat(mInputFrequency.getText().toString());
        currentStationValue = Math.round(currentStationValue * BASE_NUMBER);
        currentStationValue = currentStationValue / BASE_NUMBER;
        previousStationValue = previousStationValue+SEARCH_GAP;
        if (previousStationValue >108) {
            previousStationValue = (float)87.5;
        }
        assertEquals(previousStationValue,currentStationValue, 0.001);
        setButtonClick();
        mStrStation = mButtonPowerOn.getText().toString().split("    Turn ");
        assertEquals(currentStationValue, Float.parseFloat(mStrStation[0]),0.001);
    }
    
    public void testcase06_MusicPlayStatus() {
        boolean playStopState = false;
        mp3PlayButtonClick();
        playStopState = mButtonPlay.getText().toString().equals("Stop");
        if (playStopState) {
            assertFalse(mMp3Path.isEnabled());
        } else {
            assertTrue(mMp3Path.isEnabled());
        }
        sleep(WAIT_INITIAL);
    }
    
    private void sleep(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    
    private void searchButtonClick() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonSearch.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }        
        sleep(WAIT_UI_STATE_CHANGE);
    }
    
    private void buttonPowerOnClick() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonPowerOn.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        mInstrumentation.waitForIdleSync();
        sleep(WAIT_INITIAL);
    }
    
    private void setButtonClick() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonSetFrequency.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        mInstrumentation.waitForIdleSync();
        sleep(WAIT_INITIAL);
    }
    
    private void mp3PlayButtonClick() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonPlay.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        mInstrumentation.waitForIdleSync();
        sleep(WAIT_INITIAL);
    }
}
