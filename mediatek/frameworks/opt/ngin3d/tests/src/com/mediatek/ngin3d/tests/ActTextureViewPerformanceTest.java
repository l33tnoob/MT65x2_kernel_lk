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

package com.mediatek.ngin3d.tests;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.StrictMode;
import android.os.SystemClock;
import android.util.Log;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.lessThanOrEqualTo;

public class ActTextureViewPerformanceTest extends Ngin3dTextureViewInstrumentationTestCase {
    private static final String TAG = "ActPerformanceTest";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (Ngin3dTestCaseProvider.ENGINE_INITIALIZATION_TIME_CRITERIA == 0) {
            Ngin3dTestCaseProvider.setupCriteriaValue(getActivity().getResources());
        }
    }

    public void test01_EngineInitialTime() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getEngineInitialTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test02_ImageLoadingFromResourceTime() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromResourceTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test03_ImageLoadingFromAssetTime() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromAssetTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test04_ImageLoadingFromBitmapTime() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromBitmapTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test05_ImageLoadingFromFile() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getImageLoadingFromFileTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test06_UpdateSystemTextContent() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getUpdateSystemTextTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test07_UpdateBitmapTextContent() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getUpdateBitmapTextTask(mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test08_Render50Actor() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getRender50ActorsTask(mStage, mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test09_Render100Actor() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getRender100ActorsTask(mStage, mPresentationEngine, getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test10_Start50Animation() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getStart50AnimationsTask(getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test11_Start100Animation() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = Ngin3dTestCaseProvider.getStart100AnimationsTask(getActivity());
        mStageTextureView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void test13_Render25Landscapes() {
        mStageTextureView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        Ngin3dTestCaseProvider.render25LandscapesTest(mStage, mPresentationEngine, getActivity());
        mStageTextureView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    /**
     * To detect slow calls such as disk read/write and network access during engine
     * initialization and rendering.
     */
    public void test14_detectSlowCalls() throws ExecutionException, InterruptedException {
        final StrictMode.ThreadPolicy oldPolicy = StrictMode.getThreadPolicy();
        try {
            StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder().detectAll().penaltyDeath().build());
            FutureTask<Boolean> task = Ngin3dTestCaseProvider.getDetectSlowCallsTask(mStage, mPresentationEngine);
            mStageTextureView.runInGLThread(task);
            assertTrue("The test runs successfully", task.get());
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    private void writePerformanceData(Activity activity, String name, Object data) {
        File dataFile = new File(activity.getDir("perf", Context.MODE_PRIVATE), name);
        dataFile.delete();
        try {
            FileWriter writer = new FileWriter(dataFile);
            writer.write("YVALUE=" + data);
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
