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

import android.graphics.Bitmap;
import android.test.ActivityInstrumentationTestCase2;
import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Cube;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageView;

/**
 * Add description here.
 */
public class CubeTest extends ActivityInstrumentationTestCase2<PresentationStubActivity> {
    @SuppressWarnings("deprecation")
    public CubeTest() {
        super("com.mediatek.ngin3d.tests", PresentationStubActivity.class);
    }

    private Stage mStage;
    private StageView mStageView;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mStageView = getActivity().getStageView();
        mStage = getActivity().getStage();
    }

    // Clipping distances for camera
    private static final float Z_NEAR = 2.f;
    private static final float Z_FAR = 2000.f;

    // Note app uses PERSPECTIVE mode so coord system is Right-handed
    private static final float CAM_Z = 1111.f;// 1111.f;
    private static final float CAM_X = 240.f;
    private static final float CAM_Y = 400.f;

    // Narrow FOV required because camera is very far away
    private static final float CAM_FOV = 0.435f;

    private static final float SLABX = 0.f;
    private static final float SLABY = 0.f;
    private static final float SLABZ = 0.f;

    private Cube makeCube(Scale size) {

        Cube newCube = Cube.createFromResource(getActivity().getResources(),
            R.drawable.fourcolour);
        newCube.setPosition(new Point(0.f, 0.f, 0.f));
        newCube.setScale(size);

        return newCube;
    }

    @SmallTest
    public void testScreenShotChangeAfterRotationScale() {
        Container myWorld = new Container();

        // The unit cuboid is sized to match the size of the weather field.
        final Cube slab = makeCube(new Scale(292.3f, 55.82f, 185.6f));

        // 3 long thin cuboids are used to create crude visual axes.
        final Cube xaxis = makeCube(new Scale(2000.f, 1f, 1f));
        final Cube yaxis = makeCube(new Scale(1f, 2000.f, 1f));
        final Cube zaxis = makeCube(new Scale(1f, 1f, 2000.f));

        slab.setPosition(new Point(SLABX, SLABY, SLABZ));

        myWorld.add(xaxis, yaxis, zaxis, slab);

        mStage.setProjection(Stage.PERSPECTIVE, Z_NEAR, Z_FAR, CAM_Z);
        mStage.setCameraFov(CAM_FOV);

        mStage.setCamera(
            new Point(CAM_X, CAM_Y, CAM_Z),
            // Aim the camera at the centre of the cuboid
            new Point(SLABX, SLABY, SLABZ));

        mStage.add(myWorld);
        Bitmap bitmap = mStageView.getScreenShot();

        int oldSize = bitmap.getRowBytes() * bitmap.getHeight();
        slab.setScale(new Scale(100, 25, 90));
        xaxis.setScale(new Scale(1000, 1, 1));
        yaxis.setScale(new Scale(1, 1000, 1));
        zaxis.setScale(new Scale(1, 1, 1000));

        slab.setRotation(new Rotation(5, 5, 5));
        xaxis.setRotation(new Rotation(5, 5, 5));
        yaxis.setRotation(new Rotation(5, 5, 5));
        zaxis.setRotation(new Rotation(5, 5, 5));

        bitmap = mStageView.getScreenShot();
        int newSize = bitmap.getRowBytes() * bitmap.getHeight();
        assertNotSame(newSize, oldSize);

    }

}
