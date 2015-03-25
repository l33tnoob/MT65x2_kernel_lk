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

import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;

public class HitTest extends Ngin3dInstrumentationTestCase {
    protected Image mImage1;
    protected Image mImage2;
    protected Image mImage3;
    protected Image mImage4;
    protected Container mContainer;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        // R.drawable.photo_01~04: 320 * 314
        mImage1 = Image.createFromResource(getActivity().getResources(), R.drawable.photo_02);
        mImage1.setPosition(new Point(96, 160));
        mImage1.setScale(new Scale(0.5f, 0.5f));
        mStage.add(mImage1);

        // Image
        mImage2 = Image.createFromResource(getActivity().getResources(), R.drawable.photo_02);
        mImage2.setPosition(new Point(192, 320));
        mImage2.setScale(new Scale(0.5f, 0.5f));
        mStage.add(mImage2);

        // Container
        mContainer = new Container();
        mImage3 = Image.createFromResource(getActivity().getResources(), R.drawable.photo_03);
        mImage3.setPosition(new Point(288, 480));
        mImage3.setScale(new Scale(0.5f, 0.5f, 1.0f));
        mContainer.add(mImage3);

        mImage4 = Image.createFromResource(getActivity().getResources(), R.drawable.photo_04);
        mImage4.setPosition(new Point(384, 640));
        mImage4.setScale(new Scale(0.5f, 0.5f, 1.0f));
        mContainer.add(mImage4);

        mStage.add(mContainer);
        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                mStage.realize(mPresentationEngine);
                return true;
            }
        });
        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
    }

    public void testHit() {
        Actor actor;
        actor = mImage1.hitTest(new Point(0, 0));
        assertNull(actor);
        actor = mImage2.hitTest(new Point(0, 0));
        assertNull(actor);
        actor = mImage3.hitTest(new Point(0, 0));
        assertNull(actor);
        actor = mImage4.hitTest(new Point(0, 0));
        assertNull(actor);
        actor = mContainer.hitTest(new Point(0, 0));
        assertNull(actor);

        // Hit center of actor
        actor = mImage1.hitTest(new Point(96, 160));
        assertEquals(mImage1, actor);
        actor = mImage2.hitTest(new Point(192, 320));
        assertEquals(mImage2, actor);
        actor = mImage3.hitTest(new Point(288, 480));
        assertEquals(mImage3, actor);
        actor = mImage4.hitTest(new Point(384, 640));
        assertEquals(mImage4, actor);

        actor = mContainer.hitTest(new Point(288, 480));
        assertEquals(mImage3, actor);

        actor = mContainer.hitTest(new Point(384, 640));
        assertEquals(mImage4, actor);

        // Hit edge + 1
        actor = mImage1.hitTest(new Point(96 + 81, 160));
        assertNull(actor);
        actor = mImage2.hitTest(new Point(288 + 81, 480));
        assertNull(actor);

        actor = mImage3.hitTest(new Point(192 + 81, 320));
        assertNull(actor);
        actor = mImage4.hitTest(new Point(384 + 81, 640));
        assertNull(actor);

        actor = mContainer.hitTest(new Point(288 + 81, 480));
        assertNull(actor);
        actor = mContainer.hitTest(new Point(384 + 81, 640));
        assertNull(actor);
    }
}
