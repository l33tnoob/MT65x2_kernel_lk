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

package com.mediatek.ngin3d.demo;

import android.os.Bundle;
import android.view.GestureDetector.SimpleOnGestureListener;
import android.view.GestureDetector;
import android.view.MotionEvent;

import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;

/**
 * Demonstration of how to apply custom shader-based materials to Glo objects.
 */
public class CustomMaterials extends StageActivity {

    // Warning:
    // The following positioning numbers are model specific.
    // Change the glo model almost certain require re-tune these numbers.
    // Here is not intended to implement as reference how to view a glo file.
    private static final float Z_NEAR = 50.f;
    private static final float Z_FAR = 1000.f;
    private static final float MODEL_SCALE = 60;
    private static final int SCREEN_WIDTH = 480;
    private static final int SCREEN_HEIGHT = 800;

    private GestureDetector mGestureDetector;
    private boolean mInvert;
    private int mMultiplier = 7;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Set up a perspective camera, pointing at the robot.
        mStage.setProjection(Stage.PERSPECTIVE, Z_NEAR, Z_FAR, 0.0f);
        mStage.setCamera(new Point(200, 200, 300), new Point(0, 0, 0));
        mStage.setCameraFov(40.0f);

        Container scenario = new Container();
        mStage.add(scenario);

        // Load the robot model, and apply a custom material defined across
        // several files:
        //  - wave.mat:  specifies the shader program to use, and defines the
        //               initial properties of the material
        //  - wave.sp:   specifies the shader program source files, and defines
        //               the properties to which the shader uniforms correspond
        //  - wave.vert: the vertex shader code
        //  - wave.frag: the fragment shader code
        final Glo3D robot = Glo3D.createFromAsset("OptimusPrime.glo");
        robot.setScale(new Scale(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE));
        robot.setMaterial("wave.mat");

        // Programmatically set some material properties.  It doesn't matter
        // that they aren't defined in the material file: they will be created
        // when they are set.
        robot.setMaterialProperty("TOUCH_POSITION",
                new Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
        robot.setMaterialProperty("INVERT", mInvert);
        robot.setMaterialProperty("MULTIPLIER", mMultiplier);
        robot.setMaterialProperty("ASPECT_RATIO",
                (float) SCREEN_WIDTH / SCREEN_HEIGHT);
        robot.setMaterialProperty("TEXTURE", "onions.jpg");
        scenario.add(robot);

        // Simple Android touch-screen gesture detector.
        mGestureDetector = new GestureDetector(new SimpleOnGestureListener() {
            @Override
            public boolean onScroll(MotionEvent e1, MotionEvent e2,
                    float distanceX, float distanceY) {

                // Set the touch position property to wherever the user touched
                // the screen.
                Point touchPosition = new Point(
                    e2.getX(), SCREEN_HEIGHT - e2.getY());
                robot.setMaterialProperty("TOUCH_POSITION", touchPosition);
                return true;
            }

            @Override
            public boolean onSingleTapConfirmed(MotionEvent e) {
                ++mMultiplier;

                // With each single tap, the multiplier will change, causing
                // the wavey effect of the shader to change speed.
                if (mMultiplier > 14) {
                    mMultiplier = 1;
                }

                robot.setMaterialProperty("MULTIPLIER", mMultiplier);
                return true;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                // Double-tapping inverts the shader effect.
                mInvert = !mInvert;
                robot.setMaterialProperty("INVERT", mInvert);
                return true;
            }
        });

        // Start the robot animation.
        robot.setAnimationLoopEnabled(true);
        robot.play();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return mGestureDetector.onTouchEvent(event);
    }
}
