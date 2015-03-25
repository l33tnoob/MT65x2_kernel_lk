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
import android.util.Log;

import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Sphere;
import com.mediatek.ngin3d.Layer;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.Timeline;
import com.mediatek.ngin3d.demo.R;

/**
 * A demo to show the use of multiple layers.
 * Two sphere objects are created, and each is added to its own layer.
 * The second sphere appears in front of the first even though it is
 * further from the camera because its layer is added to the stage after the
 * first.
 */
public class LayerDemo extends StageActivity {
    protected static final String TAG = "LayerDemo";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Create two spheres (one for each layer).
        final Sphere globe1 = Sphere.createFromResource(getResources(),
            R.drawable.green_grid);
        globe1.setPosition(new Point(0, 0, -6));
        globe1.setScale(new Scale(5, 5, 5));

        final Sphere globe2 = Sphere.createFromResource(getResources(),
                R.drawable.green_grid);
        globe2.setPosition(new Point(0, 0, -10));
        globe2.setScale(new Scale(5, 5, 5));


        Layer layer1 = new Layer();
        layer1.add(globe1);
        layer1.setCameraPosition(new Point(0, 0, 0));
        layer1.setCameraLookAt(new Point(0, 0, -10), new Point(0, 1, 0));

        // Use orthographic projection for this layer and set the width
        // to exactly fit the globe inside the camera's view.
        layer1.setProjectionMode(Layer.ORTHOGRAPHIC);
        layer1.setCameraWidth(5);


        Layer layer2 = new Layer();
        layer2.add(globe2);
        layer2.setCameraPosition(new Point(0, 0, 0));
        layer2.setCameraLookAt(new Point(0, 0, -10), new Point(0, 1, 0));

        // Use perspective projection for this layer and set the camera's FOV
        // (field of view) to 45 degrees. The width setting has no effect on a
        // perspective projection so we don't set it.
        layer2.setProjectionMode(Layer.PERSPECTIVE);
        layer2.setCameraFov(45);

        // Add the layers to the stage. The first layer is added first and so
        // will be drawn first. This means that the second globe will appear in
        // front of the first even though the second layer's camera is
        // positioned further away from its globe.
        mStage.add(layer1);
        mStage.add(layer2);
    }

}
