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

package com.mediatek.world3d;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.util.Log;

import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Text;

import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.AnimationLoader;

public class VideoItem extends Container implements RotateItem  {

    private final Activity mHost;
    private Text mTitle;
    private AnimationGroup mActionAnimation;
    private AnimationGroup mButterFlyAnimation;

    public VideoItem(Activity activity) {
        mHost = activity;
    }

    public VideoItem init() {
        Image mBackground = Image.createFromResource(mHost.getResources(), R.drawable.video_content);
        mBackground.setPosition(new Point(2, -10, 2));
        mBackground.setReactive(false);
        add(mBackground);

        Image mFrame = Image.createFromResource(mHost.getResources(), R.drawable.video_frame);
        mFrame.setPosition(new Point(0, 0, -5));
        mFrame.setReactive(false);
        add(mFrame);

        Image mClapper = Image.createFromResource(mHost.getResources(), R.drawable.video_upperclapper);
        mClapper.setAnchorPoint(new Point(0.14f, 0.75f, 0, true));
        mClapper.setPosition(new Point(-108, -123, -1));
        mClapper.setReactive(false);
        add(mClapper);

        Image mLight1 = Image.createFromResource(mHost.getResources(), R.drawable.video_purplelight);
        mLight1.setPosition(new Point(-108, -123, -10));
        mLight1.setReactive(false);
        mLight1.setScale(new Scale(0, 0, 0));
        add(mLight1);

        Image mLight2 = Image.createFromResource(mHost.getResources(), R.drawable.video_greenlight);
        mLight2.setPosition(new Point(105, 50, -10));
        mLight2.setReactive(false);
        mLight2.setScale(new Scale(0, 0, 0));
        add(mLight2);

        mActionAnimation = new AnimationGroup();
        mActionAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.video_purplelight).setTarget(mLight1));
        mActionAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.video_greenlight).setTarget(mLight2));
        mActionAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.video_action).setTarget(mClapper));
        mActionAnimation.setName("video_action");
        mActionAnimation.addListener(mAnimationListener);

        mTitle = new Text(mHost.getResources().getString(R.string.video_text));
        mTitle.setPosition(new Point(0, 130, -411));
        mTitle.setReactive(false);
        mTitle.setOpacity(0);
        add(mTitle);

        setupButterFly();
        return this;
    }

    private void setupButterFly() {
        Container butterFly = new Container();
        butterFly.setReactive(false);
        butterFly.setVisible(false);

        Image rightWing = Image.createFromResource(mHost.getResources(), R.drawable.right);
        rightWing.setAnchorPoint(new Point(0f, 0.5f, 0f, true));
        rightWing.setReactive(false);
        rightWing.setDoubleSided(true);
        butterFly.add(rightWing);

        Image leftWing = Image.createFromResource(mHost.getResources(), R.drawable.left);
        leftWing.setAnchorPoint(new Point(1f, 0.5f, 0f, true));
        leftWing.setReactive(false);
        leftWing.setDoubleSided(true);
        butterFly.add(leftWing);

        add(butterFly);

        mButterFlyAnimation = new AnimationGroup();
        mButterFlyAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.butterfly_curve).setTarget(butterFly));
        mButterFlyAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.butterfly_rightwing).setTarget(rightWing));
        mButterFlyAnimation.add(AnimationLoader.loadAnimation(mHost, R.raw.butterfly_leftwing).setTarget(leftWing));
    }

    public void start() {
        mActionAnimation.start();
        mButterFlyAnimation.start();
    }

    private void stop() {
        if (mActionAnimation.isStarted()) {
            mActionAnimation.complete();
        }

        if (mButterFlyAnimation.isStarted()) {
            mButterFlyAnimation.complete();
        }
    }

    public Actor getTitle() {
        return mTitle;
    }

    @SuppressWarnings("PMD.UncommentedEmptyMethod")
    public void onIdle() {}

    @SuppressWarnings("PMD.UncommentedEmptyMethod")
    public void onRotate() {}

    public void onFocus() {
        start();
    }

    public void onDefocus() {
        stop();
    }

    public void onClick(Point point) {
        // Assumes N3D is supported by default.
        Intent intent = new Intent("android.media.action.VIDEO_CAPTURE_3D");

        // Otherwise, launch video player.
        if (!World3D.isN3DSupported()) {
            intent.setAction("android.intent.action.MAIN");
            intent.setPackage("com.mediatek.videoplayer");
        }

        try {
            Log.v("World3D", "Sending intent : " + intent);
            mHost.startActivityForResult(intent, 0);
        } catch (ActivityNotFoundException e) {
            Log.v("World3D", "exception :" + e);
        }
    }

    private final Animation.Listener mAnimationListener = new Animation.Listener() {
        @SuppressWarnings("PMD.UncommentedEmptyMethod")
        public void onStarted(Animation animation) {}

        @SuppressWarnings("PMD.UncommentedEmptyMethod")
        public void onPaused(Animation animation) {}

        public void onCompleted(final Animation animation) {
            if (animation.getName().equalsIgnoreCase("video_action")) {
                mButterFlyAnimation.start();
            }
        }
    };
}