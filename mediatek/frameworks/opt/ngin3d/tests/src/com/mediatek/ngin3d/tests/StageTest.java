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

import android.test.suitebuilder.annotation.SmallTest;
import android.view.Display;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.TextureAtlas;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

public class StageTest extends Ngin3dInstrumentationTestCase {
    @SmallTest
    public void testDefaultStage() {
        assertNotNull(mStage);
        assertEquals(0, mStage.getChildrenCount());
    }

    @SmallTest
    public void testStageProperties() {
        mStage.setName("stage");
        assertEquals("stage", mStage.getName());

        // Check projection mode parameters pass through OK
        mStage.setProjection(0, 1.0f, 2.0f, 3.0f);
        Stage.ProjectionConfig config = (Stage.ProjectionConfig) mStage.getProjection();
        assertEquals(0, config.mode);
        assertEquals(1.f, config.zNear);
        assertEquals(2.f, config.zFar);
        assertEquals(3.f, config.zStage);

        // Check legal mode range 0-2
        mStage.setProjection(1, 1.0f, 2.0f, 3.0f);
        config = (Stage.ProjectionConfig) mStage.getProjection();
        assertEquals(1, config.mode);
        mStage.setProjection(2, 1.0f, 2.0f, 3.0f);
        config = (Stage.ProjectionConfig) mStage.getProjection();
        assertEquals(2, config.mode);

        Color bkgColor = new Color(1, 1, 1);
        mStage.setBackgroundColor(bkgColor);
        assertThat(mStage.getBackgroundColor(), is(equalTo(bkgColor)));

        mStage.applyChanges(mPresentationEngine);
    }

    @SmallTest
    public void testRealize() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                mStage.realize(mPresentationEngine);
                mStage.unrealize();
                mStage.realize(mPresentationEngine);
                return true;
            }
        });
        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    @SmallTest
    public void testAddingActor() {
        for (int i = 0; i < 10; i++) {
            mStage.add(new Empty());
        }
        assertEquals(10, mStage.getChildrenCount());
        mStage.realize(mPresentationEngine);
    }

    @SmallTest
    public void testCamera() {
        Point position = new Point(0, 0, 2);
        Point lookAt = new Point(1, 1, 1);
        mStage.setCamera(position, lookAt);
        Stage.Camera camera = mStage.getCamera();
        assertThat(camera.position, is(equalTo(position)));
        assertThat(camera.lookAt, is(equalTo(lookAt)));
    }

    public void testAddTextureAtlas() {
        mStage.addTextureAtlas(getInstrumentation().getContext().getResources(), R.raw.media3d_altas, R.raw.media3d_atlas_res);
        TextureAtlas.getDefault().cleanup();
        assertTrue(TextureAtlas.getDefault().isEmpty());
    }

    public void testFPS() {
        mStage.setMaxFPS(10);
        assertThat(mStage.getMaxFPS(), is(10));
    }

    public void testSize() {
        Display display = getActivity().getWindowManager().getDefaultDisplay();
        android.graphics.Point size = new android.graphics.Point();
        display.getSize(size);
        assertThat(mStage.getHeight(), is(size.y));
        assertThat(mStage.getWidth(), is(size.x));
    }

    public void testInnerClass() {
        float[] f = {0f, 0f, 0f};

        Stage.ProjectionConfig con1 = new Stage.ProjectionConfig(2, f[0], f[1], f[2]);
        Stage.ProjectionConfig con2 = new Stage.ProjectionConfig(2, f[0], f[1], f[2]);
        assertTrue(con1.equals(con2));
    }

}
