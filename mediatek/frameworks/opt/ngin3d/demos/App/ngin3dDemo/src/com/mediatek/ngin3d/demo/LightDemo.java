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
 * Example to demonstrate adding lights to the scene.
 */

package com.mediatek.ngin3d.demo;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Light;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.android.StageView;
import com.mediatek.ngin3d.demo.R;

import android.animation.ValueAnimator;
import android.animation.ValueAnimator.AnimatorUpdateListener;


import javax.microedition.khronos.opengles.GL10;

/** \ingroup ngin3dDemos

Illustration of ngin3D user defined lights.

 */
public class LightDemo extends StageActivity {
    /** Tag to identify log messages */
    private static final String TAG = "LightDemo";

    /* Camera position in world */
    private static final Point CAM_POS = new Point(2f, 3f, 2f);
    private static final float CAM_Z = -1111f;

    /* Object position in world */
    private static final Point OBJ_POS = new Point(0f, 0f, 0f);

    /* Light positions */
    private static final Point POINT_LIGHT_POS = new Point(-3f, 3f, 3f);
    private static final Point SPOT_LIGHT_POS = new Point(0f, 5f, 0f);

    /* Point light attenuations */
    private static final float POINT_ATTN_NEAR = 0f;
    private static final float POINT_ATTN_FAR = 5f;

    /* Spot light colour and intesity */
    private static final Color SPOT_LIGHT_COL = new Color(0, 0, 255, 0);
    private static final float SPOT_LIGHT_INTENSITY = 2f;

    /* Field-of-view of the camera in degrees */
    private static final float CAM_FOV = 50.0f;

    /* Clipping distances for camera */
    private static final float Z_NEAR = 1.f;
    private static final float Z_FAR = 100.f;

    /* Animation durations in milliseconds */
    private static final int ANIM_DURATION1 = 700;
    private static final int ANIM_DURATION2 = 1000;
    private static final int ANIM_DURATION3 = 1900;

    /**
     * This method creates the scene on start-up.
     * @param savedInstanceState Standard parameter for android activities
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");

        mStage.setProjection(Stage.PERSPECTIVE, Z_NEAR, Z_FAR, CAM_Z);
        mStage.setCamera(CAM_POS, OBJ_POS);
        mStage.setCameraFov(CAM_FOV);

        // The first light is directional
        final Light light1 = new Light();
        light1.setTypeDirectional();
        // Light will initially point down the -ve z axis. Rotating by
        // -90 degrees around the x axis will make it point downwards.
        light1.setRotation(new Rotation (-90, 0, 0));

        // The second light is a point light (the default type)
        final Light light2 = new Light();
        light2.setIsAttenuated(true);
        light2.setAttenuationNear(POINT_ATTN_NEAR);
        light2.setAttenuationFar(POINT_ATTN_FAR);
        light2.setPosition(POINT_LIGHT_POS);

        final Light light3 = new Light();
        light3.setTypeSpot();
        light3.setRotation(new Rotation (-90, 0, 0));
        light3.setPosition(SPOT_LIGHT_POS);
        light3.setIntensity(SPOT_LIGHT_INTENSITY);
        light3.setDiffuseColor(SPOT_LIGHT_COL);

        final Container scenario = new Container();
        scenario.add(landscape, light1, light2, light3);

        /* Add the assembled 'scene' to the stage */
        mStage.add(scenario);

        ValueAnimator light1ColourAnimation;
        light1ColourAnimation = ValueAnimator.ofInt(0, 255, 0);
        light1ColourAnimation.setRepeatCount(ValueAnimator.INFINITE);
        light1ColourAnimation.setDuration(ANIM_DURATION1);
        light1ColourAnimation.addUpdateListener(new AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator ani) {
                Integer red = (Integer) ani.getAnimatedValue();
                light1.setDiffuseColor(new Color(red, 255-red, 0, 0));
            }
        });
        light1ColourAnimation.start();

        ValueAnimator light2IntensityAnimation;
        light2IntensityAnimation = ValueAnimator.ofFloat(0f, 2f, 0f);
        light2IntensityAnimation.setRepeatCount(ValueAnimator.INFINITE);
        light2IntensityAnimation.setDuration(ANIM_DURATION2);
        light2IntensityAnimation.addUpdateListener(new AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator ani) {
                Float intensity = (Float) ani.getAnimatedValue();
                light2.setIntensity( intensity );
                if (intensity > 1f) {
                    light2.setIsAttenuated(false);
                } else {
                    light2.setIsAttenuated(true);
                }
            }
        });
        light2IntensityAnimation.start();

        ValueAnimator light3SpotAngleAnimation;
        light3SpotAngleAnimation = ValueAnimator.ofFloat(10f, 90f, 10f);
        light3SpotAngleAnimation.setRepeatCount(ValueAnimator.INFINITE);
        light3SpotAngleAnimation.setDuration(ANIM_DURATION3);
        light3SpotAngleAnimation.addUpdateListener(new AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator ani) {

                Float outerAngle = (Float) ani.getAnimatedValue();
                light3.setSpotOuterAngle( outerAngle );
                light3.setSpotInnerAngle( outerAngle * 0.5f );
            }
        });
        light3SpotAngleAnimation.start();

    }

}
