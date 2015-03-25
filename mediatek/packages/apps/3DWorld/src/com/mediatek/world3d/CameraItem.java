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

public class CameraItem extends Container implements RotateItem {

    private final Activity mHost;
    private Text mTitle;
    private AnimationGroup mPictureTaker;

    public CameraItem(Activity activity) {
        mHost = activity;
    }

    public CameraItem init() {
        Image body = Image.createFromResource(mHost.getResources(), R.drawable.camera_body);
        body.setPosition(new Point(0, 0, -1));
        body.setReactive(false);
        add(body);

        Image aperture = Image.createFromResource(mHost.getResources(), R.drawable.camera_aperture);
        aperture.setPosition(new Point(27, -12, -5));
        aperture.setReactive(false);
        add(aperture);

        Image lens = Image.createFromResource(mHost.getResources(), R.drawable.camera_lens);
        lens.setPosition(new Point(27, -12, -2));
        lens.setReactive(false);
        add(lens);

        Image lenszoom = Image.createFromResource(mHost.getResources(), R.drawable.camera_lenszoom);
        lenszoom.setPosition(new Point(27, -12, -2));
        lenszoom.setReactive(false);
        lenszoom.setVisible(false);
        add(lenszoom);

        Image strobe = Image.createFromResource(mHost.getResources(), R.drawable.camera_strobe);
        strobe.setPosition(new Point(0, 20, 2));
        strobe.setReactive(false);
        add(strobe);

        Image radiance = Image.createFromResource(mHost.getResources(), R.drawable.camera_radiance);
        radiance.setPosition(new Point(25, -13, -10));
        radiance.setReactive(false);
        radiance.setScale(new Scale(0, 0, 0));
        add(radiance);

        Image shine = Image.createFromResource(mHost.getResources(), R.drawable.camera_shine);
        shine.setPosition(new Point(25, -13, -20));
        shine.setReactive(false);
        shine.setScale(new Scale(0, 0, 0));
        add(shine);

        Image light = Image.createFromResource(mHost.getResources(), R.drawable.camera_light);
        light.setPosition(new Point(83, -85, -30));
        light.setReactive(false);
        light.setScale(new Scale(0, 0, 0));
        add(light);

        mTitle = new Text(mHost.getResources().getString(R.string.camera_text));
        mTitle.setPosition(new Point(0, 130, -411));
        mTitle.setReactive(false);
        mTitle.setOpacity(255);
        add(mTitle);
        setName("camera");

        mPictureTaker = new AnimationGroup();
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_aperture).setTarget(aperture));
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_strobe).setTarget(strobe));
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_radiance).setTarget(radiance));
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_shine).setTarget(shine));
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_light).setTarget(light));
        mPictureTaker.add(AnimationLoader.loadAnimation(mHost, R.raw.camera_lenszoom).
            setTarget(lenszoom).enableOptions(Animation.SHOW_TARGET_DURING_ANIMATION));
        mPictureTaker.setName("PictureTaker");
        return this;
    }

    private void start() {
        mPictureTaker.start();
    }

    private void stop() {
        if (mPictureTaker.isStarted()) {
            mPictureTaker.complete();
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
        Intent intent = new Intent("android.media.action.IMAGE_CAPTURE_3D");

        try  {
            Log.v("World3D", "Sending intent : " + intent);
            mHost.startActivityForResult(intent, 0);
        } catch (ActivityNotFoundException e) {
            Log.v("World3D", "exception :" + e);
        }
    }
}