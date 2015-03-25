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
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.demo.R;

/**
 * A demo for usage of Object3D.
 */
public class Glo3DDemo extends StageActivity {
    private static final String TAG = "Glo3DDemo";

    private static final float MODEL_Z_POS = -1085;
    // for info CAMERA_Z_POS = -1111

    final Glo3D mBendGale = Glo3D.createFromAsset("tree_bend_gail.glo");
    final Glo3D mBendGentle = Glo3D.createFromAsset("tree_bend_gentle.glo");
    final Glo3D mBendModerate = Glo3D.createFromAsset("tree_bend_moderate.glo");

    final Glo3D mSheepWalk = Glo3D.createFromAsset("sheep_walk.glo");
    final Glo3D mSheepEat = Glo3D.createFromAsset("sheep_eat.glo");
    final Glo3D mSheepSleep = Glo3D.createFromAsset("sheep_sleep.glo");

    final Glo3D mHeavyCloud = Glo3D.createFromAsset("clouds_show_hide_heavy_dark.glo");
    final Glo3D mLightCloud = Glo3D.createFromAsset("clouds_show_hide_heavy_bright.glo");

    final Glo3D mBlowGale = Glo3D.createFromAsset("leaves_blow_gail.glo");
    final Glo3D mBlowGentle = Glo3D.createFromAsset("leaves_blow_gentle.glo");
    final Glo3D mBlowModerate = Glo3D.createFromAsset("leaves_blow_moderate.glo");

    final Glo3D mStarTwinkle = Glo3D.createFromAsset("stars_twinkle.glo");
    final Glo3D mRainFall = Glo3D.createFromAsset("rain_fall.glo");

    /*
     * For various reasons the naming and function of the Sun/Moon glo files is
     * confusing.
     *
     * sunmoon.glo Contains the geometry (no animation) for the sun and the
     * moon, co-located, and scaled to zero (so, invisible)
     *
     * sunmoon_day_to_night.glo contains the animation (only) to position the
     * moon and sun in the 'night' location - moon above and scaled up, sun out
     * of sight and scaled zero. night_to_day.glo is the converse.
     *
     * sunmoon_show_hide.glo contains animation (only) to scale both sun and
     * moon from zero to 1, without moving them. Stopping/rewinding this
     * animation hides the objects. There is no 'hide' animation.
     *
     * sun|moon_show_hide.glo contains animation (only) to scale the
     * corresponding object from 0 to 1, and in the case of the sun, starts a
     * loop for the sun's rotation. There is no 'hide' animation.
     */
    final Glo3D mSunMoonGeom = Glo3D.createFromAsset("sunmoon.glo");
    final Glo3D mMoonUpSunDown = Glo3D.createFromAsset("sunmoon_day_to_night.glo");
    final Glo3D mSunUpMoonDown = Glo3D.createFromAsset("sunmoon_night_to_day.glo");
    final Glo3D mSunAndMoon = Glo3D.createFromAsset("sunmoon_show_hide.glo");
    final Glo3D mSun = Glo3D.createFromAsset("sun_show_hide.glo");
    final Glo3D mMoon = Glo3D.createFromAsset("moon_show_hide.glo");

    Glo3D mPreviousGlo = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");

        final Container tree = new Container();
        tree.add(mBendGale, mBendGentle, mBendModerate);

        final Container sheep = new Container();
        sheep.add(mSheepEat, mSheepWalk, mSheepSleep);

        final Container cloud = new Container();
        cloud.add(mHeavyCloud, mLightCloud);

        final Container sun_moon = new Container();
        sun_moon.add(mSunMoonGeom, mMoonUpSunDown, mSunUpMoonDown, mSunAndMoon, mSun, mMoon);

        final Container leaves = new Container();
        leaves.add(mBlowGentle, mBlowModerate, mBlowGale);

        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D
                .createFromAsset("weak_direct_light.glo");
        direct_light.setRotation(new Rotation(70, 40, 0));

        Container scenario = new Container();
        scenario.add(landscape, tree, sheep, cloud, leaves, sun_moon,
                     mStarTwinkle, mRainFall, direct_light);

        /*
         * Using UI_PERSPECTIVE. Setting normalise 'true' converts 0.5's into
         * 'half screeen' for X and Y, but Z is still in pixels!
         */
        scenario.setPosition(new Point(0.5f, 0.5f, MODEL_Z_POS, true));


        // We are using the UI_PERSPECTIVE projection mode, where the y-axis
        // points downwards, so we must rotate Glo models by 180 additional
        // degrees to turn them the right way up.
        scenario.setRotation(new Rotation(190, 30, 0));

        mStage.add(scenario);

        // Start the sun animation to scale full size and start rotating
        mSun.play();
    }

    /*
     * A couple of crude utility functions to allow re-use of one menu option to
     * do multiple operations, e.g. start/stop. NOTE! The 'stop on second press'
     * only works if no other animation has been selected since the first press.
     * Example:
     *
     * Rain (starts rain);
     * Rain (stops rain)
     *
     * Rain (starts rain);
     * Eat  (starts sheep eating);
     * Rain (restarts rain, previous press was 'Eat' not 'Rain');
     * Rain (stops rain)
     */
    private void playOnFirstPressStopOnRepeatPresses(Glo3D ani) {
        if (mPreviousGlo == ani) {
            ani.stop();
        } else {
            ani.play();
        }
        mPreviousGlo = ani;
    }

    private void togglePlayThisAndStopOthers(Glo3D toStart, Glo3D... toStop) {
        toStart.togglePlaying();
        for (Glo3D glo : toStop) {
            glo.stop();
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

        switch (itemId) {
        case R.id.eat:
            // mSheepEat.setAnimationSpeed(-2f); // try these!
            togglePlayThisAndStopOthers(mSheepEat, mSheepWalk, mSheepSleep);
            break;
        case R.id.walk:
            togglePlayThisAndStopOthers(mSheepWalk, mSheepEat, mSheepSleep);
            break;
        case R.id.sleep:
            // mSheepSleep.setAnimationSpeed(0.5f);
            togglePlayThisAndStopOthers(mSheepSleep, mSheepWalk, mSheepEat);
            break;
        case R.id.bend_gale:
            // mBendGale.setAnimationSpeed(2f);
            togglePlayThisAndStopOthers(mBendGale, mBendGentle, mBendModerate);
            break;
        case R.id.bend_gentle:
            togglePlayThisAndStopOthers(mBendGentle, mBendGale, mBendModerate);
            break;
        case R.id.bend_moderate:
            togglePlayThisAndStopOthers(mBendModerate, mBendGentle, mBendGale);
            break;
        case R.id.blow_gale:
            togglePlayThisAndStopOthers(mBlowGale, mBlowModerate, mBlowGentle);
            break;
        case R.id.blow_gentle:
            togglePlayThisAndStopOthers(mBlowGentle, mBlowModerate, mBlowGale);
            break;
        case R.id.blow_moderate:
            togglePlayThisAndStopOthers(mBlowModerate, mBlowGentle, mBlowGale);
            break;
        case R.id.stars_twinkle:
            playOnFirstPressStopOnRepeatPresses(mStarTwinkle);
            break;
        case R.id.rain_fall:
            playOnFirstPressStopOnRepeatPresses(mRainFall);
            break;
        case R.id.heavy_clouds_show_hide:
            mLightCloud.stop();
            playOnFirstPressStopOnRepeatPresses(mHeavyCloud);
            break;
        case R.id.light_clouds_show_hide:
            mHeavyCloud.stop();
            playOnFirstPressStopOnRepeatPresses(mLightCloud);
            break;
        case R.id.day_to_night:
            mMoon.play();
            mSun.stop();
            mMoonUpSunDown.play();
            break;
        case R.id.night_to_day:
            mSun.play();
            mMoon.stop();
            mSunUpMoonDown.play();
            break;
        case R.id.show_hide:
            playOnFirstPressStopOnRepeatPresses(mSunAndMoon);
            break;
        default:
            return false;
        }
        return true;
    }
}
