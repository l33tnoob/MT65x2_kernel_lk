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

package com.mediatek.ngin3d.demo;

import android.os.Bundle;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.demo.R;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Rotation;
import android.animation.ValueAnimator;
import android.animation.ValueAnimator.AnimatorUpdateListener;

/**
 * This is a simple demo to demonstrate a bump mapped surface. The quad is flat
 * but textured not only with a colour texture but also with a 'normal' texture.
 * Each pixel of the latter contains the description of the normal vector at
 * that point, and this affects the lighting. The effect is that the flat quad
 * appears bumpy.
 */

public class BumpmapDemo extends StageActivity {

    // Floor panel is 1x1, how much to scale it up?
    private static final float MODEL_SCALE = 7.f;

    // How far to set Floor panel behind origin
    private static final float MODEL_Z = -1.f;

    // Radius of light rotation
    private static final float LIGHT_R = 3.f;

    // actor to hold light at an offset
    private Container lightArm = new Container();

    /*
     * Place the camera in 3D space. Putting it on the +Z axis means that the
     * other axes in the visible scene are 'where you expect' i.e. X axis is
     * left-to-right and Y is up.
     */
    private static final float CAMERA_Z = 10.f;

    // Set the camera's Field of View (angle in degrees)
    private static final float CAMERA_FOV = 40.f;

    /*
     * Set the far clip distance to twice the distance of camera-to-scene and
     * the near clip distance to 'nearby'. The important factor is that FAR/NEAR
     * is not a large number.
     */
    private static final float Z_NEAR = 1f;
    private static final float Z_FAR = 20.f;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /*
         * Load in the GLO models. Locate the models relative to the 3d origin
         * which equates to the centre of the screen in PERSPECTIVE set-up (Y
         * up, X right, Z to user)
         */

        final Glo3D glowBall = Glo3D.createFromAsset("pointlight.glo");
        lightArm.add(glowBall);
        glowBall.setPosition(new Point(LIGHT_R, 0, 0));

        final Glo3D floorSurface = Glo3D
                                   .createFromAsset("bumpfloor_unit_unlit.glo");
        // Rotate to face camera
        floorSurface.setRotation(new Rotation(90, 0, 0));
        floorSurface.setPosition(new Point(0, 0, -2));
        floorSurface.setScale(new Scale(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));

        /*
         * Set up the camera. Field of view, location and aim, and projection.
         */
        mStage.setCameraFov(CAMERA_FOV);
        mStage.setCamera(new Point(0, 0, CAMERA_Z), new Point(0, 0, 0));
        mStage.setProjection(Stage.PERSPECTIVE, Z_NEAR, Z_FAR, CAMERA_Z);

        // Add the whole scene to the stage
        mStage.add(floorSurface, lightArm);

        /*
         * Animate the light
         */
        ValueAnimator drive = ValueAnimator.ofFloat(360f, 0f);
        drive.setDuration(3000);
        drive.setInterpolator(null); // = linear
        drive.setRepeatCount(ValueAnimator.INFINITE);

        drive.addUpdateListener(new AnimatorUpdateListener() {
            public void onAnimationUpdate(ValueAnimator ani) {
                lightArm.setRotation(new Rotation(0, 0, (Float) ani
                                                  .getAnimatedValue()));
            }
        });
        drive.start();
    }
}
