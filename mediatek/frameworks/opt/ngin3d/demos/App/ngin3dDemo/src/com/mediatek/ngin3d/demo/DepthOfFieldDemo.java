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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

/**
 * \file
 * Example to demonstrate depth of field effect.
 */

package com.mediatek.ngin3d.demo;

import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Layer;
import com.mediatek.ngin3d.Light;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.demo.R;

import android.os.Bundle;
import android.view.Display;
import android.view.MotionEvent;


import javax.microedition.khronos.opengles.GL10;

/** \ingroup ngin3dDemos

Illustration of ngin3D depth of field effect.

 */
public class DepthOfFieldDemo extends StageActivity {
    /** Tag to identify log messages */
    private static final String TAG = "DepthOfFieldDemo";

    /* Camera position in world */
    private static final Point CAM_POS = new Point(2f, 3f, 2f);

    /* Object position in world */
    private static final Point OBJ_POS = new Point(0f, 0f, 0f);

    /* Field-of-view of the camera in degrees */
    private static final float CAM_FOV = 50.0f;

    /* Clipping distances for camera */
    private static final float Z_NEAR = 1.f;
    private static final float Z_FAR = 8.f;

    private final Layer mLayer = new Layer();

    private boolean mDepthEnabled = true;

    /**
     * This method creates the scene on start-up.
     * @param savedInstanceState Standard parameter for android activities
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");

        final Light light = new Light();
        light.setRotation(new Rotation (-90, 0, 0));
        light.setTypeDirectional();

        mLayer.add(landscape, light);
        mLayer.enableDepthOfField(true);
        mLayer.setCameraPosition(CAM_POS);
        mLayer.setCameraLookAt(OBJ_POS);
        mLayer.setCameraNear(Z_NEAR);
        mLayer.setCameraFar(Z_FAR);
        mLayer.setCameraFov(CAM_FOV);

        mLayer.setFocusDistance(5f);
        mLayer.setFocusRange(1f);

        /* Add the assembled 'scene' to the stage */
        mStage.add(mLayer);

        getStageView().setRenderMode(StageView.RENDERMODE_CONTINUOUSLY);

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_DOWN) {
            mDepthEnabled = !mDepthEnabled;
            mLayer.enableDepthOfField(mDepthEnabled);
        }

        Display display = getWindowManager().getDefaultDisplay();

        float y = 1.f - (event.getY() / display.getHeight());
        mLayer.setFocusDistance(Z_NEAR + y * (Z_FAR - Z_NEAR));

        float x = event.getX() / display.getHeight();
        mLayer.setFocusRange(x * (Z_FAR - Z_NEAR));

        return true;
    }

}
