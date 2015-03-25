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
import android.content.ComponentName;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Text;

import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.animation.BasicAnimation;

import java.util.ArrayList;

public class GalleryItem extends Container implements RotateItem {

    private final Activity mHost;
    private Text mTitle;
    private final Container mHitTestArea = new Container();

    public GalleryItem(Activity activity) {
        mHost = activity;
    }

    private ArrayList<Animation> mMarch;
    private ArrayList<Image> mMarchTargets;
    private Animation mLightAnimation;
    private static int[] sRES = {
            R.drawable.photo_01,
            R.drawable.photo_02,
            R.drawable.photo_03,
            R.drawable.photo_04,
            R.drawable.photo_05,
            R.drawable.photo_06,
            R.drawable.gallery_light };
    
    public GalleryItem init() {
        setupTargets();
        setupAnimation();
        
        Image icon = Image.createFromResource(mHost.getResources(), R.drawable.ic_gallery);
        icon.setReactive(false);
        mHitTestArea.add(icon);
        mHitTestArea.setReactive(false);
        add(mHitTestArea);
        
        mTitle = new Text(mHost.getResources().getString(R.string.gallery_text));
        mTitle.setPosition(new Point(0, 130, -411));
        mTitle.setReactive(false);
        mTitle.setOpacity(0);
        add(mTitle);
        return this;
    }
    
    private void setupTargets() {
        mMarchTargets = new ArrayList<Image>();

        Point position = new Point(0.5f, 0.5f, 0, true);
        for (int i = 0; i < sRES.length; ++i) {
            Image target = Image.createFromResource(mHost.getResources(), sRES[i]);
            target.setPosition(position);
            target.setVisible(false);
            target.setDoubleSided(true);
            target.setReactive(false);
            mMarchTargets.add(target);
            add(target);
        }
    }
    
    private void setupAnimation() {
        mMarch = new ArrayList<Animation>();
        
        BasicAnimation flight = AnimationLoader.loadAnimation(mHost, R.raw.gallery_flightcurve);
        flight.enableOptions(Animation.SHOW_TARGET_DURING_ANIMATION);
        mMarch.add(flight.setTarget(mMarchTargets.get(0)));
        mMarch.add(flight.clone().setTarget(mMarchTargets.get(1)));
        mMarch.add(flight.clone().setTarget(mMarchTargets.get(2)));
        mMarch.add(flight.clone().setTarget(mMarchTargets.get(3)));
        mMarch.add(flight.clone().setTarget(mMarchTargets.get(4)));
        mMarch.add(flight.clone().setTarget(mMarchTargets.get(5)));
        mLightAnimation = AnimationLoader.loadAnimation(mHost, R.raw.gallery_light).setTarget(mMarchTargets.get(6));
    }

    public Actor getTitle() {
        return mTitle;
    }

    @SuppressWarnings("PMD.UncommentedEmptyMethod")
    public void onRotate() {}

    @SuppressWarnings("PMD.UncommentedEmptyMethod")
    public void onIdle() {}

    public void onFocus() {
        start(0);
    }
    
    public void onDefocus() {
        stop();
    }

    public void onClick(Point point) {
        Point hit = new Point(point);
        mHitTestArea.setReactive(true);
        if (mHitTestArea.hitTest(hit) == mHitTestArea) {
            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);
            intent.setComponent(new ComponentName("com.android.gallery3d", "com.android.gallery3d.app.Gallery"));
            intent.putExtra("onlyStereoMedia", true);

            try  {
                Log.v("World3D", "Sending intent : " + intent);
                mHost.startActivityForResult(intent, 0);
            } catch (ActivityNotFoundException e) {
                Log.v("World3D", "exception :" + e);
            }
        }
        mHitTestArea.setReactive(false);
    }

    private static int sStartAnimation = 1;
    private void start(int fromIndex) {
        if (fromIndex >= mMarch.size()) {
            return;
        }

        if (fromIndex == 0) {
            mLightAnimation.start();
        }

        mMarch.get(fromIndex).start();
        Message msg = Message.obtain();
        msg.what = sStartAnimation;
        msg.arg1 = fromIndex + 1;
        mHandler.sendMessageDelayed(msg, 250);
    }

    private void stop() {
        for (int i = 0; i < mMarch.size(); ++i) {
            if (mMarch.get(i).isStarted()) {
                mMarch.get(i).complete();
            }
        }

        if (mLightAnimation.isStarted()) {
            mLightAnimation.complete();
        }
    }

    TinyHandler mHandler = new TinyHandler();
    private class TinyHandler extends Handler {
        public void handleMessage(Message msg) {
            if (msg.what == sStartAnimation) {
                start(msg.arg1);
                return;
            }
            super.handleMessage(msg);
        }
    }
}