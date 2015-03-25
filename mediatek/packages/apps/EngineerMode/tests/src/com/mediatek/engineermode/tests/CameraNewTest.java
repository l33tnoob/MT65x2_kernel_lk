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
import android.os.StatFs;
import android.test.ActivityInstrumentationTestCase2;
import android.content.Context;
import android.util.Log;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemClickListener;
import android.os.PowerManager;

import java.io.File;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.IOException;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;
import com.mediatek.engineermode.cameranew.AutoCalibration;
import com.mediatek.engineermode.cameranew.Camera;
import com.mediatek.storage.StorageManagerEx;

public class CameraNewTest extends ActivityInstrumentationTestCase2<AutoCalibration> {
    private static final String TAG = "CameraNewTest";
    private Instrumentation mInst = null;
    private Activity mActivity = null;
    private Solo mSolo = null;
    private ListView mLvStartPreview = null;
    private static final long MIN_STORAGE = 10 * 1024 * 1024;
    private PowerManager.WakeLock mWakeLock = null;
    private Spinner mSensorSp = null;
    
    public CameraNewTest() {
        super("com.mediatek.engineermode", AutoCalibration.class);
    }
    
    protected void setUp() throws Exception {
        super.setUp();
        mInst = getInstrumentation();
        mActivity = getActivity();
        mSolo = new Solo(mInst, mActivity);
        
    }
    
    public void testCase01Precondition() {
        assertNotNull(mInst);
        assertNotNull(mActivity);
        assertNotNull(mSolo);
        mSolo.assertCurrentActivity("current should be AutoCalibration activity", AutoCalibration.class);
    }
    
    public void testCase02CameraPreview() {
        mInst.waitForIdleSync();
        while(mSolo.scrollDown());
        mLvStartPreview = (ListView)mActivity.findViewById(R.id.listview_capture);
        assertNotNull(mLvStartPreview);
        String startPreview = mLvStartPreview.getAdapter().getItem(0).toString();
        mSolo.clickOnText(startPreview);
        mInst.waitForIdleSync();
        mSolo.sleep(3000);
        mSolo.assertCurrentActivity("current should be Camara activity", Camera.class);
        mSolo.goBack();
    }
    
    public void testCase03CameraCapture() {
        mInst.waitForIdleSync();
        while(mSolo.scrollDown());
        mLvStartPreview = (ListView)mActivity.findViewById(R.id.listview_capture);
        assertNotNull(mLvStartPreview);
        String startPreview = mLvStartPreview.getAdapter().getItem(0).toString();
        mSolo.clickOnText(startPreview);
        mInst.waitForIdleSync();
        mSolo.sleep(3000);
        mSolo.clickOnButton(mActivity.getString(R.string.capture_picture));
        mSolo.sleep(200);
        Button btn = (Button)mSolo.getCurrentActivity().findViewById(R.id.capture_btn);
        while (!btn.isEnabled()) {
            mSolo.sleep(500);
        }
        mSolo.sleep(500);
        mSolo.goBack();
    }
    
    public void testCase04EvCalibration() {
        mSolo.assertCurrentActivity("current should be AutoCalibration activity", AutoCalibration.class);
        String targetTag = "EV Calibration";
        if (!mSolo.searchText(targetTag)) {
            return;
        }
        assertTrue(mSolo.searchText(targetTag, 1, true));
        // to visible EV Calibration Text
        while(mSolo.scrollDown());
        mSolo.sendKey(Solo.UP);
        mSolo.clickOnText(targetTag);
        mInst.waitForIdleSync();
        mSolo.sleep(500);
        mSolo.assertCurrentActivity("current should be Camera activity", Camera.class);
        assertTrue(mSolo.getButton(0) != null);
        mSolo.clickOnButton(0);
        mSolo.sleep(200);
        assertTrue(mSolo.getEditText(0) != null);
        mSolo.clearEditText(0);
        mSolo.enterText(0, "abc");
        //mSolo.goBack();
        mSolo.clickOnButton(0);
        mSolo.sleep(200);
        assertTrue(mSolo.getEditText(0) != null);
        mSolo.clearEditText(0);
        mSolo.enterText(0, "40");
        //mSolo.goBack();
        mSolo.clickOnButton(0);
        mSolo.clickOnButton(0);
        mSolo.assertCurrentActivity("current should be Camera activity", Camera.class);
        mSolo.goBack();
    }
    
    public void testCase05SubSensor() {
        mSolo.assertCurrentActivity("current should be AutoCalibration activity", AutoCalibration.class);
        Activity activity = mSolo.getCurrentActivity();
        mSensorSp = (Spinner)activity.findViewById(R.id.auto_clibr_camera_sensor_spnr);
        int count = mSensorSp.getAdapter().getCount();
        assertTrue(count > 0);
        if (count == 1) {
            return;
        }
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mSensorSp.setSelection(1);
            }
        });
        mSolo.sleep(200);
        testCase02CameraPreview();
        mSolo.sleep(1000);
        testCase03CameraCapture();
        mSolo.goBack();
    }
    
    public void testCase06CameraNoAvailStorage() {
        String str = "for em camera auto test string!" 
            + "Copyright Statement:This software/firmware"
            + " and related documentation MediaTek Softwareare* protected under relevant "
            + "copyright laws. The information contained herein* is confidential and proprietary"
            + " to MediaTek Inc. and/or its licensors.* Without the prior written permission of "
            + "MediaTek inc. and/or its licensors,* any reproduction, modification, use or "
            + "disclosure of MediaTek Software,* and information contained herein, in whole "
            + "or in part, shall be strictly prohibited. MediaTek Inc. (C) 2010. All rights "
            + "reserved** BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES "
            + "AND AGREES* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (MEDIATEK SOFTWARE)"
            + "* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON* "
            + "AN AS-IS BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,* EXPRESS"
            + " OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED OF* MERCHANTABILITY,"
            + " FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.* NEITHER DOES MEDIATEK PROVIDE"
            + " ANY WARRANTY WHATSOEVER WITH RESPECT TO THE* SOFTWARE OF ANY THIRD PARTY WHICH MAY "
            + "BE USED BY, INCORPORATED IN, OR* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER "
            + "AGREES TO LOOK ONLY TO SUCH* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. "
            + "RECEIVER EXPRESSLY ACKNOWLEDGES* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN"
            + " FROM ANY THIRD PARTY ALL PROPER LICENSES* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK "
            + "SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEKSOFTWARE RELEASES MADE TO RECEIVER'S "
            + "SPECIFICATION OR TO CONFORM TO A PARTICULARSTANDARD OR OPEN FORUM. RECEIVER'S SOLE "
            + "AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE ANCUMULATIVE LIABILITY WITH RESPECT TO "
            + "THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, TO REVISE OR "
            + "REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY LICENSE FEES OR SERVICE"
            + " CHARGE PAID BY RECEIVER TOMEDIATEK FOR SUCH MEDIATEK  AT ISSUE.The following"
            + " software/firmware and/or related documentation have been modified"
            + " by MediaTek Inc. All revisions are subject to any receiver'sapplicable license "
            + "agreements with MediaTek Inc.";

        mWakeLock = ((PowerManager)mInst.getContext().getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.FULL_WAKE_LOCK | 
                PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
        mSolo.sleep(100);
        mWakeLock.acquire();
        String sdPath = StorageManagerEx.getDefaultPath();
        File file = new File(sdPath + "/EMCameraAutoTest.dat");
        BufferedWriter writer = null;
        try {
            writer = new BufferedWriter(new FileWriter(file));
            while (getSdAvailableSpace(sdPath) > MIN_STORAGE) {
                writer.write(str);
            }
        } catch (IOException e) {
            Xlog.d("IOException:", e.getMessage());
        } finally {
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException e) {
                    Xlog.d("IOException:", e.getMessage());
                }
            }
        }
        testCase03CameraCapture();
        mWakeLock.release();
        file.delete();
    }
    
    private long getSdAvailableSpace(String sdPath) {
        StatFs stat = new StatFs(sdPath);
        return stat.getAvailableBlocks() * (long) stat.getBlockSize();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

}
