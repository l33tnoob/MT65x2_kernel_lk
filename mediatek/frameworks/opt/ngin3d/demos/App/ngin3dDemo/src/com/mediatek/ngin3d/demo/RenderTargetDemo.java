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
import android.animation.ValueAnimator;
import android.animation.ValueAnimator.AnimatorUpdateListener;

import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Sphere;
import com.mediatek.ngin3d.Layer;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.demo.R;

/**
 * A demo to show how to render into an image.
 * Two layers are created. The first one contains a sphere object and has
 * an Image object set as its render target. This image is added to the second
 * layer.
 */
public class RenderTargetDemo extends StageActivity {
    protected static final String TAG = "RenderTargetDemo";

    private static final Point GLOBE_POS = new Point(0f, 0f, -6f);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Layer layer1 = new Layer();

        // Create an Image object and use it as the render target for layer1.
        final Image image = Image.createEmptyImage(512, 256);
        layer1.setTargetImage(image);

        // Set layer1's camera to point at the position of the globe
        //layer1.setCameraPosition(new Point(0, 0, 0));
        layer1.setCameraLookAt(GLOBE_POS);
        layer1.setProjectionMode(Layer.PERSPECTIVE);
        layer1.setCameraFov(45);

        // Create sphere and add it to the first layer.
        final Sphere globe1 = Sphere.createFromResource(getResources(),
            R.drawable.green_grid);
        globe1.setPosition(GLOBE_POS);
        globe1.setScale(new Scale(5, 5, 5));
        layer1.add(globe1);



        // Create a second layer for the image object.
        Layer layer2 = new Layer();

        image.setPosition(new Point(0, 0, 0));
        image.setMaterial("rendertarget.mat");
        image.setDoubleSided(true);
        layer2.add(image);

        layer2.setProjectionMode(Layer.PERSPECTIVE);
        layer2.setCameraPosition(new Point(0, 0, 300));
        layer2.setCameraLookAt(new Point(0, 0, 0), new Point(0, 1, 0));
        layer2.setCameraNear(10);
        layer2.setCameraFar(1000);
        layer2.setCameraFov(45);


        // Because layer1 is drawn into "image", it is added first.

        mStage.add(layer1);
        mStage.add(layer2);

        // Create a ValueAnimator to rotate both the globe and the image object.

        ValueAnimator anim = ValueAnimator.ofFloat(0f, 360);
        anim.setDuration(4000);
        anim.setRepeatCount(ValueAnimator.INFINITE);
        anim.addUpdateListener(new AnimatorUpdateListener() {
            public void onAnimationUpdate(ValueAnimator ani) {

                // Shift the background depending on the animation progress
                Float t = (Float) ani.getAnimatedValue();
                image.setRotation(new Rotation(0, t, 0));
                globe1.setRotation(new Rotation(0, -t, 0));
            }
        });
        anim.start();
    }

}
