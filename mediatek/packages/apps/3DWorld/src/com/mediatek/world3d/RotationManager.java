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
import android.util.Log;

import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;

import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedList;

public class RotationManager {
    private final Activity mHost;
    private final Stage mStage;
    private Container mRoot;
    private ArrayList<Actor> mActors;
    private final Hashtable<String, AnimationGroup>  mAnimationMap = new Hashtable<String, AnimationGroup>();
    private final LinkedList<Actor>  mFocuses = new LinkedList<Actor>();
    private final LinkedList<AnimationGroup>  mRotationAnimation = new LinkedList<AnimationGroup>();

    private static double sRADIUS = 550;
    private static double sFloatRadius = 575;
    private static double sAngleRadia = 1.0472;
    private static Point[] sPOS = {
            new Point(0.0f, 0.0f, (float)-sRADIUS, false),
            new Point(-(float)(Math.sin(sAngleRadia) * sRADIUS), 0.0f,
                    (float)(Math.cos(sAngleRadia) * sRADIUS), false),
            new Point((float)(Math.sin(sAngleRadia) * sRADIUS), 0.0f,
                    (float)(Math.cos(sAngleRadia) * sRADIUS), false) };

    private static Point[] sFloatPos = {
            new Point(0.0f, 0.0f, -(float) sFloatRadius, false),
            new Point(-(float)(Math.sin(sAngleRadia) * sFloatRadius), 0.0f,
                    (float)(Math.cos(sAngleRadia) * sFloatRadius), false),
            new Point((float)(Math.sin(sAngleRadia) * sFloatRadius), 0.0f,
                    (float)(Math.cos(sAngleRadia) * sFloatRadius), false) };

    private static Scale[] sSCALE = {
        new Scale(1.0f, 1.0f, 1.0f),    // Front
        new Scale(0.7f, 0.7f, 1.0f),    // Back
        new Scale(0.7f, 0.7f, 1.0f) };  // Back

    public RotationManager(Activity activity, Stage stage) {
        mHost = activity;
        mStage = stage;
    }

    public void init() {
        mActors = new ArrayList<Actor>();
        mActors.add(new CameraItem(mHost).init());
        mActors.add(new GalleryItem(mHost).init());
        mActors.add(new VideoItem(mHost).init());

        mRoot = new Container();
        mRoot.setPosition(new Point(0.5f, 0.5f, 0, true));
        for (int i = 0; i < mActors.size(); ++i) {
            mRoot.add(mActors.get(i));
            mFocuses.add(mActors.get(i));
        }
        mStage.add(mRoot);

        setupPositionAndScale();

        AnimationGroup animation = generateYRotationAnimation("rotate120", 0, 120, 500, Mode.LINEAR);
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(0)).getTitle(), "opacity", 255, 0).setDuration(500));
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(2)).getTitle(), "opacity", 0, 255).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(0), "scale", sSCALE[0], sSCALE[2]).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(2), "scale", sSCALE[2], sSCALE[0]).setDuration(500));

        mRotationAnimation.add(animation);

        animation = generateYRotationAnimation("rotate240", 120, 240, 500, Mode.LINEAR);
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(2)).getTitle(), "opacity", 255, 0).setDuration(500));
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(1)).getTitle(), "opacity", 0, 255).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(2), "scale", sSCALE[0], sSCALE[1]).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(1), "scale", sSCALE[1], sSCALE[0]).setDuration(500));
        mRotationAnimation.add(animation);

        animation = generateYRotationAnimation("rotate360", 240, 360, 500, Mode.LINEAR);
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(1)).getTitle(), "opacity", 255, 0).setDuration(500));
        animation.add(new PropertyAnimation(
                ((RotateItem)mActors.get(0)).getTitle(), "opacity", 0, 255).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(1), "scale", sSCALE[0], sSCALE[1]).setDuration(500));
        animation.add(new PropertyAnimation(
                mActors.get(0), "scale", sSCALE[1], sSCALE[0]).setDuration(500));
        mRotationAnimation.add(animation);

        setupTitleAnimation();
        setupFloatingAnimation();
    }

    private void setupPositionAndScale() {
        for (int i = 0; i < mActors.size(); ++i) {
            mActors.get(i).setPosition(sPOS[i]);
            mActors.get(i).setScale(sSCALE[i]);
        }
    }

    public boolean hit(Actor hit, Point point) {
        if (hit == null) {
            return false;
        }
        int index = mActors.indexOf(hit);
        if (index != -1) {
            if (mActors.get(index) == mFocuses.peek()) {
                ((RotateItem)mActors.get(index)).onClick(point);
            }
        }
        return true;
    }

    public BasicAnimation getAnim(String name) {
        Log.v("World3D", "getAnim : " + name);
        return mAnimationMap.get(name);
    }

    public void startTitle() {
        getAnim("title").start();
    }

    public RotateItem getFocus() {
        return (RotateItem)mFocuses.peek();
    }

    public Animation updateFocusAndGetRotationAnimation(boolean isLeft) {
        AnimationGroup play = isLeft ? mRotationAnimation.poll() : mRotationAnimation.pollLast();
        if (isLeft) {
            mFocuses.addFirst(mFocuses.pollLast());
            mRotationAnimation.add(play);
            play.setDirection(Animation.FORWARD);
        } else {
            mFocuses.add(mFocuses.poll());
            mRotationAnimation.addFirst(play);
            play.setDirection(Animation.BACKWARD);
        }
        return play;
    }

    public boolean rotate(boolean isLeft) {
        getFocus().onDefocus();
        updateFocusAndGetRotationAnimation(isLeft).start();
        return true;
    }

    private AnimationGroup generateYRotationAnimation(
            String name, int fromAngle, int toAngle, int duration, Mode mode) {
        AnimationGroup rotate = new AnimationGroup();
        Rotation fromRot = new Rotation(0, fromAngle, 0);
        Rotation fromReverseRot = new Rotation(0, -fromAngle, 0);
        Rotation toRot = new Rotation(0, toAngle, 0);
        Rotation toReverseRot = new Rotation(0, -toAngle, 0);

        for (int i = 0; i < mActors.size(); ++i) {
            Log.v("World3D", "Object :" +
                    mActors.get(i) + " from : " + fromReverseRot + ", to :" + toReverseRot);
            rotate.add(new PropertyAnimation(
                    mActors.get(i), "rotation", fromReverseRot, toReverseRot)
                    .setDuration(duration).setMode(mode));
        }

        Log.v("World3D", "root from : " + fromRot + ", to :" + toRot);
        rotate.add(new PropertyAnimation(mRoot, "rotation", fromRot, toRot).setDuration(duration));
        rotate.setName(name);
        rotate.addListener(mAnimationListener);
        return rotate;
    }

    private void setupFloatingAnimation() {
        AnimationGroup floating = new AnimationGroup();
        for (int i = 0; i < mActors.size(); ++i) {
            floating.add(new PropertyAnimation(mActors.get(i), "position", sPOS[i], sFloatPos[i]));
        }

        floating.setLoop(true).setAutoReverse(true).setName("floating");
        floating.addListener(mAnimationListener);
        mAnimationMap.put("floating", floating);
    }

    private void setupTitleAnimation() {
        int duration = 1000;
        int rotation = 720;
        AnimationGroup title = new AnimationGroup();
        title.add(new PropertyAnimation(
                mRoot, "position", new Point(0.5f, 0, 0, true), new Point(0.5f,0.5f,0,true))
                .setDuration(duration).setMode(Mode.EASE_OUT_EXPO));
        title.add(new PropertyAnimation(
                mActors.get(0), "rotation", new Rotation(0, 0, 0), new Rotation(0, -rotation, 0))
                .setDuration(duration).setMode(Mode.EASE_OUT_EXPO));
        title.add(new PropertyAnimation(
                mActors.get(1), "rotation", new Rotation(0, 0, 0), new Rotation(0, -rotation, 0))
                .setDuration(duration).setMode(Mode.EASE_OUT_EXPO));
        title.add(new PropertyAnimation(
                mActors.get(2), "rotation", new Rotation(0, 0, 0), new Rotation(0, -rotation, 0))
                .setDuration(duration).setMode(Mode.EASE_OUT_EXPO));
        title.add(new PropertyAnimation(
                mRoot, "rotation", new Rotation(0, 0, 0), new Rotation(0, rotation, 0))
                .setDuration(duration).setMode(Mode.EASE_OUT_EXPO));
        title.setName("title");
        title.addListener(mAnimationListener);
        mAnimationMap.put("title", title);

    }

    private void handleAnimationComplete(final Animation animation) {
        String name = animation.getName();
        if (name.equalsIgnoreCase("title")) {
            getAnim("floating").start();
        }

        if (name.startsWith("rotate")) {
            Log.v("World3D", "rotate : " + getFocus());
            getFocus().onFocus();
        }
    }

    private final Animation.Listener mAnimationListener = new Animation.Listener() {
        public void onCompleted(final Animation animation) {
            handleAnimationComplete(animation);
        }
    };
}