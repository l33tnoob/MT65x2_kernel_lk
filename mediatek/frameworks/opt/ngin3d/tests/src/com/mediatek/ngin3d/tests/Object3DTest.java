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

import android.opengl.GLSurfaceView;
import android.test.ActivityInstrumentationTestCase2;
import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.utils.Ngin3dException;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.is;

public class Object3DTest extends ActivityInstrumentationTestCase2<PresentationStubActivity> {

    protected Stage mStage;
    private StageView mStageView;
    protected PresentationEngine mPresentationEngine;

    public Object3DTest() {
        super(PresentationStubActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        mStageView = getActivity().getStageView();
        mStageView.waitSurfaceReady();
        mPresentationEngine = mStageView.getPresentationEngine();
    }

    @SmallTest
    public void testObject3DAnimation() throws InterruptedException, ExecutionException {
        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
        final Glo3D tree_bend_gail = Glo3D.createFromAsset("tree_bend_gail.glo");

        getActivity().getStage().add(landscape);
        getActivity().getStage().add(tree_bend_gail);

        BasicAnimation treeGail = tree_bend_gail.getAnimation();
        assertThat(treeGail.getDuration(), is(0));
        assertEquals(tree_bend_gail, treeGail.getTarget());

        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                landscape.realize(mPresentationEngine);
                tree_bend_gail.realize(mPresentationEngine);
                return true;
            }
        });

        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());

        treeGail = tree_bend_gail.getAnimation();
        assertThat(treeGail.getDuration(), is(greaterThan(0)));

        treeGail.setLoop(false);

        treeGail.start();
        Thread.sleep(treeGail.getDuration() / 2);
        assertTrue(treeGail.isStarted());

        treeGail.waitForCompletion();
        assertFalse(treeGail.isStarted());

        try {
            treeGail.setTarget(landscape);
            fail("Should throw exception because Object3DAnimation can not change target.");
        } catch (Ngin3dException e) {
            // expected
        }
        assertEquals(tree_bend_gail, treeGail.getTarget());

        treeGail.setLoop(true);

        treeGail.start();
        Thread.sleep(treeGail.getDuration() * 2);
        assertTrue(treeGail.isStarted());

        treeGail.stop();
        assertFalse(treeGail.isStarted());
    }
}
