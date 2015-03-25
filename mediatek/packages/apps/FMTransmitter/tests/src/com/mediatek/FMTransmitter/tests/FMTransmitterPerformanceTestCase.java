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
import android.widget.ListView;
import com.mediatek.FMTransmitter.FMTransmitterActivity;
import com.mediatek.FMTransmitter.FMTransmitterAdvanced;
import com.mediatek.FMTransmitter.FMTransmitterStation;
import com.mediatek.FMTransmitter.FMTxLogUtils;
import com.mediatek.FMTransmitter.R;
import com.jayway.android.robotium.solo.Solo;
import com.mediatek.common.featureoption.FeatureOption;

public class FMTransmitterPerformanceTestCase extends
        ActivityInstrumentationTestCase2<FMTransmitterActivity> {

    private static final String TAG = "FMTxPerformanceTest";
    private static final int WAIT_UI_STATE_CHANGE = 10000;//wait for advance search 
    private static final int WAIT_SEARCH_UPDATE = 5000;// wait for search update
    private static final int WAIT_TEAR_DOWN = 1000;// wait for tear down complete
    private static final int WAIT_ADVANCED_ACTIVITY = 5000;// wait for advanced activity to set up
    private static final int GET_STATUS = 500;// get status of device
    private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;// use as to get the frequency
    private Instrumentation mInstrumentation = null;
    private ActivityMonitor mActivityMonitor = null;
    
    private Context mContext;
    
    private FMTransmitterActivity mFMTransmitterActivity = null;
    private FMTransmitterAdvanced mFMTransmitterAdvanced = null;
    private ImageButton mPlayStop = null;
    private ImageButton mButtonSeek = null;
    private ImageButton mButtonAdvanced = null;
    private Solo mSolo;
    private long mStartTime = 0;
    private long mEndTime = 0;
    
    public FMTransmitterPerformanceTestCase(
            Class<FMTransmitterActivity> activityClass) {
        super(activityClass);
    }

    public FMTransmitterPerformanceTestCase() {
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
        mSolo = new Solo(mInstrumentation, mFMTransmitterActivity);
        mPlayStop = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_play_stop);
        mButtonSeek = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_seek);
        mButtonAdvanced = (ImageButton) mFMTransmitterActivity
                .findViewById(R.id.button_advanced);
    }

    @Override
    public void tearDown() throws Exception {
        
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        mSolo.finishOpenedActivities();
        sleep(WAIT_TEAR_DOWN);
        super.tearDown();
        
        
    }

    // Test POWERON time
    public void testcase01_TestPowerOnTime() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        //if initial is powerup,first power down 
        if (playStopState) {
            mSolo.clickOnView(mPlayStop);
            sleep(WAIT_UI_STATE_CHANGE);
        }
        mStartTime = System.currentTimeMillis();
        FMTxLogUtils.i(TAG, "[Performance test][FMTransmitter] Test FM Tx Power on start ["+ mStartTime +"]");
        mSolo.clickOnView(mPlayStop);
        //back to initial state
        sleep(WAIT_UI_STATE_CHANGE);
        mSolo.clickOnView(mPlayStop);
        sleep(WAIT_UI_STATE_CHANGE);
    }

    // Test POWERDOWN time
    public void testcase02_TestPowerDownTime() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        //if initial is powerdown,then power up 
        if (!playStopState) {
            mSolo.clickOnView(mPlayStop);
            sleep(WAIT_UI_STATE_CHANGE);
        }
        mStartTime = System.currentTimeMillis();
        FMTxLogUtils.i(TAG, "[Performance test][FMTransmitter] Test FM Tx Power down start ["+ mStartTime +"]");
        mSolo.clickOnView(mPlayStop);
        sleep(WAIT_UI_STATE_CHANGE);
    }
    
    // Test Tune time
    public void testcase03_TestTuneTime() {
        boolean playStopState = false;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        //if initial is powerdown,then power up 
        if (!playStopState) {
            mSolo.clickOnView(mPlayStop);
            sleep(WAIT_UI_STATE_CHANGE);
        }
        advancedButtonClick();
        //back to initial state
        mSolo.clickOnView(mPlayStop);
        sleep(WAIT_UI_STATE_CHANGE);
    }

    public void testcase04_TestSeekTime() {
        boolean playStopState = false;
        int stationGap = 0;
        playStopState = getPlayStopState();
        sleep(GET_STATUS);
        //if initial is powerdown,then power up 
        if (!playStopState) {
            mSolo.clickOnView(mPlayStop);
            sleep(WAIT_UI_STATE_CHANGE);
        }
        // get the current station
        int currentStation = FMTransmitterStation.getCurrentStation(mContext);
        mStartTime = System.currentTimeMillis();
        FMTxLogUtils.i(TAG, "[Performance test][FMTransmitter] Test FM Tx total seek time start ["+ mStartTime +"]");
        mSolo.clickOnView(mButtonSeek);
        sleep(WAIT_SEARCH_UPDATE);
        int searchStation = FMTransmitterStation.getCurrentStation(mContext);
        if (searchStation > currentStation) {
            stationGap = searchStation-currentStation;
            FMTxLogUtils.i(TAG,"[Performance test][FMTransmitter] Test FM Tx total seek time stationGap ["+ (float)stationGap/BASE_NUMBER +"]" );
        } else if (searchStation < currentStation) {
            stationGap = 1080-currentStation + searchStation-875;
            FMTxLogUtils.i(TAG,"[Performance test][FMTransmitter] Test FM Tx total seek time stationGap ["+ (float)stationGap/BASE_NUMBER +"]" );
        } else {
            FMTxLogUtils.e(TAG, "SearchStation Unchanged");
        }
        
    }
 
    private void advancedButtonClick() {
        
        mSolo.clickOnView(mButtonAdvanced);
        mActivityMonitor = new ActivityMonitor(
                "com.mediatek.FMTransmitter.FMTransmitterAdvanced", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mFMTransmitterAdvanced = (FMTransmitterAdvanced) mActivityMonitor
                .waitForActivityWithTimeout(WAIT_ADVANCED_ACTIVITY);
        assertNotNull(mFMTransmitterAdvanced);
        View searchButton = mFMTransmitterAdvanced.findViewById(R.id.button_search);
        mSolo.clickOnView(searchButton);
        sleep(WAIT_UI_STATE_CHANGE);
        ListView listView = (ListView)mFMTransmitterAdvanced.findViewById(R.id.station_list);
        mSolo.clickOnView(listView.getChildAt(0));
        mStartTime = System.currentTimeMillis();
        FMTxLogUtils.i(TAG, "[Performance test][FMTransmitter] Test FM Tx Tune start ["+ mStartTime +"]");
        sleep(WAIT_UI_STATE_CHANGE);
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
