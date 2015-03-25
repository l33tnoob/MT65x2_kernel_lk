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
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.Timeline;
import com.mediatek.ngin3d.demo.R;

/**
 * Demonstrate mipmap/filter quality
 */
public class MipmapDemo extends StageActivity {

    // 'standard' position of camera for Ngin demos
    private static final float Z_CAM = -1111.f;
    // half-size of screen. Screen fixed at portrait in manifest XML.
    private static final float HWIDTH = 240.f;
    private static final float HHEIGHT = 400.f;

    private static final float SLABX = 0.f; // HWIDTH/2;
    private static final float SLABY = 0.f; // HHEIGHT/2;
    private static final float SLABZ = 0.f;

    private int mQuality = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        Container myWorld = new Container();

        final Image slab1 = makeImage( new Point(SLABX, -HHEIGHT/2, SLABZ));
        final Image slab2 = makeImage( new Point(SLABX, HHEIGHT/2, SLABZ));

        slab2.setSourceRect( new Box( 240, 45, 258, 56 ) );

        slab1.enableMipmap( false );
        slab2.setFilterQuality( 0 );

        // Scale Y -1 as the UI-Perspective is Y-down
        mStage.setCamera( new Point(HWIDTH, -HHEIGHT, Z_CAM),
                          new Point(SLABX, SLABY, SLABZ) );

        myWorld.add(slab1, slab2);

        mStage.add( myWorld );

        //Rotate the image showing minification so that 'sparkle' can be seen.
        Timeline timeline = new Timeline(10000);
        timeline.addListener(new Timeline.Listener() {
            private Rotation mRotation = new Rotation();

            public void onNewFrame(Timeline timeline, int elapsedMsecs) {
                mRotation.set(0, 0, 1, 36.f * timeline.getProgress());
                slab1.setRotation(mRotation);
            }

            public void onStarted(Timeline timeline) {
            }

            public void onMarkerReached(Timeline timeline, int elapsedMsecs,
                                        String marker, int direction) {
            }

            public void onPaused(Timeline timeline) {
            }

            public void onCompleted(Timeline timeline) {
            }

            public void onLooped(Timeline timeline) {
              mQuality = 1 - mQuality;
              if( mQuality == 1 )
              {
                slab1.enableMipmap( true );
                slab2.setFilterQuality( 2 );
              }
              else
              {
                slab1.enableMipmap( false );
                slab2.setFilterQuality( 0 );
              }
            }
        });
        timeline.setLoop(true);
        timeline.start();

    }

    private Image makeImage( Point position ) {
        Image image = Image.createFromResource(getResources(), R.drawable.earth);
        image.setScale(new Scale(0.7f, 1.0f, 1.f));
        image.setPosition(position);

        return image;
    }
}
