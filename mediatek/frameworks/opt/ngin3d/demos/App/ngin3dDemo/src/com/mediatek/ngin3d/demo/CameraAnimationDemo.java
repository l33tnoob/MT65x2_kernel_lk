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
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.PropertyAnimation;

/**
 * A demo for usage of Camera Animation.
 */
public class CameraAnimationDemo extends StageActivity {

    private static final float Z_NEAR = 2f;
    private static final float Z_FAR = 3000f;
    private static final float CAMERA_Z_FROM = 900f;
    private static final float CAMERA_Z_TO = 820f;

    BasicAnimation mCameraMove;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
        // tree
        final Container tree = new Container();
        final Glo3D tree_bend_gale = Glo3D.createFromAsset("tree_bend_gail.glo");
        tree.add(tree_bend_gale);

        // sheep
        final Container sheep = new Container();
        final Glo3D sheep_walk = Glo3D.createFromAsset("sheep_walk.glo");
        sheep.add(sheep_walk);

        // sunmoon
        final Container sun_moon = new Container();
        final Glo3D sunmoon = Glo3D.createFromAsset("sunmoon.glo");
        final Glo3D sun_show_hide = Glo3D.createFromAsset("sun_show_hide.glo");
        sun_moon.add(sunmoon, sun_show_hide);

        // leaves
        final Container leaves = new Container();
        final Glo3D leaves_blow_gale = Glo3D.createFromAsset("leaves_blow_gail.glo");
        leaves.add(leaves_blow_gale);

        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D.createFromAsset("weak_direct_light.glo");
        direct_light.setRotation(new Rotation (70, 40, 0));

        Container scenario = new Container();
        scenario.add(landscape, tree, sheep, sun_moon, leaves, direct_light);
        scenario.setPosition(new Point(0, 0, 800));
        scenario.setRotation(new Rotation(10, 0, 0));
        scenario.setScale(new Scale(10f, 10f, 10f));
        mStage.add(scenario);

        // Start Glo animations
        sheep_walk.play();
        tree_bend_gale.play();
        leaves_blow_gale.play();

        // Start the sun animation to scale full size and start rotating
        sun_show_hide.play();

        // Set up perspective projection
        mStage.setProjection(Stage.PERSPECTIVE, Z_NEAR, Z_FAR, CAMERA_Z_FROM);

        Point cameraPos = new Point(0, 0, CAMERA_Z_FROM);
        Point cameraLookAt = new Point(0, 0, 0);
        Stage.Camera cameraFrom = new Stage.Camera(cameraPos, cameraLookAt);
        Stage.Camera cameraTo = new Stage.Camera(new Point(0, 0, CAMERA_Z_TO), cameraLookAt);
        mCameraMove = new PropertyAnimation(mStage, "camera", cameraFrom, cameraTo)
            .setDuration(3000)
            .setLoop(true)
            .setAutoReverse(true);
        mCameraMove.start();

    }

}
