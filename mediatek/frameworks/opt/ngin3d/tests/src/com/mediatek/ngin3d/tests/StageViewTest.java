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
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.animation.PropertyAnimation;

import static android.test.MoreAsserts.assertNotEqual;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.greaterThan;

public class StageViewTest extends Ngin3dInstrumentationTestCase {
    @SmallTest
    public void testScreenShot() {
        assertNotNull(mStageView.getScreenShot());
    }

    @SmallTest
    public void testRendering() throws InterruptedException {
        Integer start = new Integer(0);
        Integer end = new Integer(255);
        Plane plane = new Plane();
        getActivity().getStage().add(plane);
        PropertyAnimation ani = new PropertyAnimation(plane, "opacity", start, end);
        ani.start();
        mStageView.pauseRendering();
        Thread.sleep(500);
        float p1 = ani.getProgress();
        Thread.sleep(500);
        float p2 = ani.getProgress();
        assertEquals(p1, p2);
        assertTrue(ani.isStarted());
        assertEquals(mStageView.getRenderMode(), StageView.RENDERMODE_WHEN_DIRTY);

        mStageView.resumeRendering();
        assertTrue(ani.isStarted());
        Thread.sleep(1000);

        float p3 = ani.getProgress();
        assertNotEqual(p2, p3);
    }

    public void testFrameInterrval() {
        mStageView.setRenderMode(StageView.RENDERMODE_CONTINUOUSLY);
        mStage.getFrameInterval();
        assertThat(mStage.getFrameInterval(), greaterThan(0));
    }

}
