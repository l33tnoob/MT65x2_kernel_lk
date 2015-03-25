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

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import com.mediatek.ngin3d.*;
import com.mediatek.ngin3d.android.StageTextureView;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.demo.R;

/**
 * A demo for usage of Object3D.
 */
public class Glo3DTextureDemo extends Activity {

    Animation mBendGail;
    Animation mBendGentle;
    Animation mBendModerate;

    Animation mBlowGail;
    Animation mBlowGentle;
    Animation mBlowModerate;

    Animation mSheepEat;
    Animation mSheepWalk;
    Animation mSheepSleep;

    Animation mRainFall;
    Animation mStarTwinkle;
    Animation mHeavyCloud;
    Animation mLightCloud;

    Animation mDayToNight;
    Animation mNightToDay;
    Animation mShowHide;
    Animation mSunShowHide;
    Animation mMoonShowHide;

    private StageTextureView mTextureView;
    private Stage mStage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mTextureView = new StageTextureView(this);
        setContentView(mTextureView);

        mStage = mTextureView.getStage();
        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
        // tree
        final Container tree = new Container();
        final Glo3D tree_bend_gale = Glo3D.createFromAsset("tree_bend_gail.glo");
        final Glo3D tree_bend_gentle = Glo3D.createFromAsset("tree_bend_gentle.glo");
        final Glo3D tree_bend_moderate = Glo3D.createFromAsset("tree_bend_moderate.glo");
        tree.add(tree_bend_gale, tree_bend_gentle, tree_bend_moderate);

        // sheep
        final Container sheep = new Container();
        final Glo3D sheep_walk = Glo3D.createFromAsset("sheep_walk.glo");
        final Glo3D sheep_eat = Glo3D.createFromAsset("sheep_eat.glo");
        final Glo3D sheep_sleep = Glo3D.createFromAsset("sheep_sleep.glo");
        sheep.add(sheep_eat, sheep_walk, sheep_sleep);

        // cloud
        final Container cloud = new Container();
        final Glo3D clouds_dark = Glo3D.createFromAsset("clouds_show_hide_heavy_dark.glo");
        final Glo3D clouds_bright = Glo3D.createFromAsset("clouds_show_hide_heavy_bright.glo");
        cloud.add(clouds_dark, clouds_bright);

        // sunmoon
        final Container sun_moon = new Container();
        final Glo3D sunmoon = Glo3D.createFromAsset("sunmoon.glo");
        final Glo3D sunmoon_day_to_night = Glo3D.createFromAsset("sunmoon_day_to_night.glo");
        final Glo3D sunmoon_night_to_day = Glo3D.createFromAsset("sunmoon_night_to_day.glo");
        final Glo3D sunmoon_show_hide = Glo3D.createFromAsset("sunmoon_show_hide.glo");
        final Glo3D sun_show_hide = Glo3D.createFromAsset("sun_show_hide.glo");
        final Glo3D moon_show_hide = Glo3D.createFromAsset("moon_show_hide.glo");
        sun_moon.add(sunmoon, sunmoon_day_to_night, sunmoon_night_to_day, sunmoon_show_hide, sun_show_hide, moon_show_hide);

        // leaves
        final Container leaves = new Container();
        final Glo3D leaves_blow_gale = Glo3D.createFromAsset("leaves_blow_gail.glo");
        final Glo3D leaves_blow_gentle = Glo3D.createFromAsset("leaves_blow_gentle.glo");
        final Glo3D leaves_blow_moderate = Glo3D.createFromAsset("leaves_blow_moderate.glo");
        leaves.add(leaves_blow_gale, leaves_blow_gentle, leaves_blow_moderate);

        final Glo3D stars = Glo3D.createFromAsset("stars_twinkle.glo");
        final Glo3D rain = Glo3D.createFromAsset("rain_fall.glo");

        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D.createFromAsset("weak_direct_light.glo");
        direct_light.setRotation(new Rotation (70, 40, 0));

        Container scenario = new Container();
        scenario.add(landscape, tree, sheep, cloud, leaves, sun_moon, stars, rain, direct_light);
        scenario.setPosition(new Point(0.5f, 0.5f, -1085, true));
        scenario.setRotation(new Rotation(190, 30, 0));

        mStage.add(scenario);

        // Get animations
        mBendGail = tree_bend_gale.getAnimation();
        mBendGentle = tree_bend_gentle.getAnimation();
        mBendModerate = tree_bend_moderate.getAnimation();
        mSheepWalk = sheep_walk.getAnimation();
        mSheepEat = sheep_eat.getAnimation();
        mSheepSleep = sheep_sleep.getAnimation();
        mStarTwinkle = stars.getAnimation();
        mHeavyCloud = clouds_dark.getAnimation();
        mLightCloud = clouds_bright.getAnimation();
        mRainFall = rain.getAnimation();
        mBlowGail = leaves_blow_gale.getAnimation();
        mBlowGentle = leaves_blow_gentle.getAnimation();
        mBlowModerate = leaves_blow_moderate.getAnimation();
        mDayToNight = sunmoon_day_to_night.getAnimation();
        mNightToDay = sunmoon_night_to_day.getAnimation();
        mShowHide = sunmoon_show_hide.getAnimation();
        mSunShowHide = sun_show_hide.getAnimation();
        mMoonShowHide = moon_show_hide.getAnimation();

        // Start the sun animation to scale full size and start rotating
        mSunShowHide.start();
    }

    private void toggleAnimation(Animation aniToStart, Animation... aniToStop) {
        if (aniToStart.isStarted()) {
            aniToStart.stop();
        } else {
            ((BasicAnimation)aniToStart).setLoop(true).start();
            for (Animation ani : aniToStop) {
                ani.stop();
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.glo_demo_option_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int itemId = item.getItemId();

        switch (itemId){
        case R.id.eat:
            toggleAnimation(mSheepEat, mSheepWalk, mSheepSleep);
            break;
        case R.id.walk:
            toggleAnimation(mSheepWalk, mSheepEat, mSheepSleep);
            break;
        case R.id.sleep:
            toggleAnimation(mSheepSleep, mSheepWalk, mSheepEat);
            break;
        case R.id.bend_gale:
            toggleAnimation(mBendGail, mBendGentle, mBendModerate);
            break;
        case R.id.bend_gentle:
            toggleAnimation(mBendGentle, mBendGail, mBendModerate);
            break;
        case R.id.bend_moderate:
            toggleAnimation(mBendModerate, mBendGentle, mBendGail);
            break;
        case R.id.blow_gale:
            toggleAnimation(mBlowGail, mBlowModerate, mBlowGentle);
            break;
        case R.id.blow_gentle:
            toggleAnimation(mBlowGentle, mBlowGail, mBlowModerate);
            break;
        case R.id.blow_moderate:
            toggleAnimation(mBlowModerate, mBlowGentle, mBlowGail);
            break;
        case R.id.stars_twinkle:
            toggleAnimation(mStarTwinkle);
            break;
        case R.id.rain_fall:
            toggleAnimation(mRainFall);
            break;
        case R.id.light_clouds_show_hide:
            toggleAnimation(mLightCloud, mHeavyCloud);
            break;
        case R.id.heavy_clouds_show_hide:
            toggleAnimation(mHeavyCloud, mLightCloud);
            break;
        case R.id.day_to_night:
            mMoonShowHide.start();
            mSunShowHide.stop();
            mDayToNight.start();
            break;
        case R.id.night_to_day:
            mSunShowHide.start();
            mMoonShowHide.stop();
            mNightToDay.start();
            break;
        case R.id.show_hide:
            mShowHide.start();
            break;
        default:
            return false;
        }
        return true;
    }
}
