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
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.animation.Timeline;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.demo.R;

/**
 * Add description here.
 */
public class PlaneDemo extends StageActivity {

    private static final int[] ANIMATIONS = new int[] {
        R.raw.photo_swap_enter_photo1_ani, R.raw.photo_swap_enter_photo2_ani,
        R.raw.photo_swap_enter_photo3_ani, R.raw.photo_swap_enter_photo4_ani
    };

    private static final int PLANE_PER_PAGE = 4;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Container c = new Container();
        mStage.add(c);

        for (int i = 0; i < PLANE_PER_PAGE; i++) {
            final Plane plane = Plane.create(new Dimension(300, 200));
            c.add(plane);
            if(i == 0) {
                plane.setColor(new Color(255, 0, 0, 255));
            }
            else if (i == 1) {
                plane.setColor(new Color(0, 255, 0, 191));
            }
            else if (i == 2) {
                plane.setColor(new Color(0, 0, 255, 127));
            }
            else if (i == 3) {
                plane.setColor(new Color(0, 255, 255, 63));
            }

            Animation animation = AnimationLoader.loadAnimation(this, ANIMATIONS[i]);
            animation.setTarget(plane);
            animation.start();
        }

        Timeline timeline = new Timeline(5000);
        timeline.addListener(new Timeline.Listener() {
            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                int opacity = (int)(255 * timeline.getProgress());
                c.setOpacity(opacity);
            }

            public void onStarted(Timeline timeline) {
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs, String marker, int direction) {
            }

            public void onPaused(Timeline timeline) {
            }

            public void onCompleted(Timeline timeline) {
            }

            public void onLooped(Timeline timeline) {
            }
        });

        timeline.setLoop(true);
        timeline.start();

    }
}
