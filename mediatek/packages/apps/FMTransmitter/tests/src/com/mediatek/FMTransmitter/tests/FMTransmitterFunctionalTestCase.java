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
import android.content.Context;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.Button;
import android.widget.TextView;
import com.mediatek.FMTransmitter.FMTransmitterActivity;
import com.mediatek.FMTransmitter.FMTransmitterAdvanced;
import com.mediatek.FMTransmitter.FMTransmitterStation;
import com.mediatek.FMTransmitter.R;
import com.mediatek.FMTransmitter.FMTxLogUtils;
import com.mediatek.common.featureoption.FeatureOption;

public class FMTransmitterFunctionalTestCase extends
        ActivityInstrumentationTestCase2<FMTransmitterActivity> {

	public static final String TAG = "FMTx/FunctionalTestCase";
    private static final int WAIT_UI_STATE_CHANGE = 10000;//wait for advance search 
    private static final int WAIT_INITIAL = 5000;// wait for ui change time
    private static final int WAIT_SEARCH_UPDATE = 5000;// wait for search update
    private static final int WAIT_TEAR_DOWN = 1000;// wait for tear down complete
    private static final int WAIT_ADVANCED_ACTIVITY = 5000;// wait for advanced activity to set up
    private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;// use as to get the frequency
    private static final int GET_STATUS = 500;// get status of device
    private Instrumentation mInstrumentation = null;
    private ActivityMonitor mActivityMonitor = null;
    
    private Context mContext;
    
    private FMTransmitterActivity mFMTransmitterActivity = null;
    private FMTransmitterAdvanced mFMTransmitterAdvanced = null;
    private ImageButton mPlayStop = null;
    private ImageButton mButtonSeek = null;
    private ImageButton mButtonAdvanced = null;
    private TextView stationValue = null;

    public FMTransmitterFunctionalTestCase(
            Class<FMTransmitterActivity> activityClass) {
        super(activityClass);
    }

    public FMTransmitterFunctionalTestCase() {
        super(FMTransmitterActivity.class);
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        
        setActivityInitialTouchMode(false);
        mFMTransmitterActivity = getActivity();
        assertNotNull(mFMTransmitterActivity);
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getContext();
        assertNotNull(mInstrumentation);
        mPlayStop = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_play_stop);
        mButtonSeek = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_seek);
        mButtonAdvanced = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_advanced);
        stationValue = (TextView) mFMTransmitterActivity
                .findViewById(R.id.station_value);

    }

    @Override
    public void tearDown() throws Exception {
        if (null != mFMTransmitterActivity) {
            mFMTransmitterActivity.finish();
        }
        if (null != mFMTransmitterAdvanced) {
            mFMTransmitterAdvanced.finish();
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
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        if (playStopState) {
            assertTrue(mButtonAdvanced.isEnabled());
        } else {
            assertFalse(mButtonAdvanced.isEnabled());
        }
    }

    // Test POWERON/POWERDOWN
    public void testcase02_PlayStopButtonClick() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        if (playStopState) {
            try {
                runTestOnUiThread(new Runnable() {
                    public void run() {
                        mPlayStop.performClick();
                    }
                });
            } catch (Throwable e) {
                e.printStackTrace();
            }
            mInstrumentation.waitForIdleSync();
            sleep(WAIT_INITIAL);
            assertFalse(mButtonSeek.isEnabled());
            assertFalse(mButtonAdvanced.isEnabled());
        } else {
            try {
                runTestOnUiThread(new Runnable() {
                    public void run() {
                        mPlayStop.performClick();
                    }
                });
            } catch (Throwable e) {
                e.printStackTrace();
            }
            mInstrumentation.waitForIdleSync();
            sleep(WAIT_INITIAL);
            assertTrue(mButtonSeek.isEnabled());
            assertTrue(mButtonAdvanced.isEnabled());
            // power down FM Tx
            try {
                runTestOnUiThread(new Runnable() {
                    public void run() {
                        mPlayStop.performClick();
                    }
                });
            } catch (Throwable e) {
                e.printStackTrace();
            }
            sleep(WAIT_INITIAL);
        }
    }

    // Test search one channel
    public void testcase03_SeekOneChannel() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        float currentStation = FMTransmitterStation
                .getCurrentStation(mContext);
        currentStation = currentStation / BASE_NUMBER;
        
        // turn on FMTx if not open
        powerOnFMTx(playStopState);
        if (null != mFMTransmitterActivity) {
            try {
                runTestOnUiThread(new Runnable() {
                    public void run() {
                        mButtonSeek.performClick();
                    }
                });
            } catch (Throwable e) {
                e.printStackTrace();
            }
            mInstrumentation.waitForIdleSync();
            sleep(WAIT_SEARCH_UPDATE);
            
            float searchStation = FMTransmitterStation.getCurrentStation(mFMTransmitterActivity);
            searchStation = searchStation / BASE_NUMBER;
            
            float currentStationValue = 0;
            currentStationValue = Float.parseFloat(stationValue.getText().toString());
            currentStationValue = Math.round(currentStationValue * BASE_NUMBER);
            currentStationValue = currentStationValue / BASE_NUMBER;
            
            assertEquals(searchStation,currentStationValue,0.001);
            assertTrue(currentStation != searchStation);
        }
    }

    // Test AdvancedSearch
    public void testcase04_AdvancedSearchAndTune() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        // turn on FMTx if not open
        powerOnFMTx(playStopState);
        advancedButtonClick();
        searchButtonClick();
        sleep(WAIT_UI_STATE_CHANGE);
        int searchcount = FMTransmitterStation.getStationCount(
        		mContext,
                FMTransmitterStation.STATION_TYPE_SEARCHED);
        assertTrue(searchcount > 0);
        // test tune channel
        CharSequence previousStationValue = stationValue.getText();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        sleep(WAIT_INITIAL);
        CharSequence currentStationValue = stationValue.getText();
        assertFalse(previousStationValue.equals(currentStationValue));
    }

    public void testcase05_InputFrequencyAndTune() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonAdvanced.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }

        mActivityMonitor = new ActivityMonitor(
                "com.mediatek.FMTransmitter.FMTransmitterAdvanced", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mFMTransmitterAdvanced = (FMTransmitterAdvanced) mActivityMonitor
                .waitForActivityWithTimeout(WAIT_ADVANCED_ACTIVITY);
        assertNotNull(mFMTransmitterAdvanced);
        
        final EditText mEditTextFrequency =(EditText)mFMTransmitterAdvanced.findViewById(R.id.edittext_frequency);
        final Button mButtonOK =(Button)mFMTransmitterAdvanced.findViewById(R.id.button_ok);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MOVE_END);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DEL);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_8);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_8);
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mEditTextFrequency.setText(String.valueOf(88));
                    mButtonOK.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }        
        sleep(WAIT_INITIAL);        
        float currentStationValue = 0;
        currentStationValue = Float.parseFloat(stationValue.getText().toString());
        currentStationValue = Math.round(currentStationValue * BASE_NUMBER);
        currentStationValue = currentStationValue / BASE_NUMBER;
        assertEquals(88,currentStationValue,0.001);
    }
        
    private void searchButtonClick() {
        final View searchButton = mFMTransmitterAdvanced
                .findViewById(R.id.button_search);
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    searchButton.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }
        mInstrumentation.waitForIdleSync();
    }

    private void advancedButtonClick() {
        try {
            runTestOnUiThread(new Runnable() {
                public void run() {
                    mButtonAdvanced.performClick();
                }
            });
        } catch (Throwable e) {
            e.printStackTrace();
        }

        mActivityMonitor = new ActivityMonitor(
                "com.mediatek.FMTransmitter.FMTransmitterAdvanced", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mFMTransmitterAdvanced = (FMTransmitterAdvanced) mActivityMonitor
                .waitForActivityWithTimeout(WAIT_ADVANCED_ACTIVITY);
        assertNotNull(mFMTransmitterAdvanced);
        searchButtonClick();
        sleep(WAIT_UI_STATE_CHANGE);
        int searchcount = FMTransmitterStation.getStationCount(
        		mContext,
                FMTransmitterStation.STATION_TYPE_SEARCHED);
        assertTrue(searchcount > 0);
        // test tune channel
        TextView stationValue = (TextView) mFMTransmitterActivity
                .findViewById(R.id.station_value);
        CharSequence previousStationValue = stationValue.getText();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        sleep(WAIT_INITIAL);
        CharSequence currentStationValue = stationValue.getText();
        assertFalse(previousStationValue.equals(currentStationValue));
    }

    private void powerOnFMTx(boolean isplaying) {
        if (!isplaying) {
            try {
                runTestOnUiThread(new Runnable() {
                    public void run() {
                        mPlayStop.performClick();
                    }
                });
            } catch (Throwable e) {
                e.printStackTrace();
            }
            mInstrumentation.waitForIdleSync();
            sleep(WAIT_INITIAL);
        } 
    }

    private boolean getPlayStopState() {
        return (mButtonSeek.isEnabled() && mFMTransmitterActivity.isTxPowerUp());
    }

    private void sleep(long time) {
        try {
            Thread.sleep(time);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
