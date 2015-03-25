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
import android.view.GestureDetector;
import android.view.MotionEvent;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.android.StageActivity;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.AnimationLoader;

/**
 * Put your finger on the screen, move your finger and the object
 * is following your finger's track. We call it as a drag animation.
 * This demo show you how to use drag animation to control an object
 * by your finger.
 */
public class DragAnimationDemo extends StageActivity {
    private static final String TAG = "DragAnimationDemo";

    // The progress of drag animation is 0~1, progress = (current movement) / (max distance)
    // The speed of drag animation is much slower if maximum distance of drag animation
    // is larger.
    private int mMaxDistance = 400;
    private int mCurrentMovement;

    private static final int[] PHOTO_FRAMES = new int[] {
        R.drawable.photo_01,
        R.drawable.photo_02,
        R.drawable.photo_03,
        R.drawable.photo_04,
    };

    private static final int[] WEATHER_FRAMES = new int[] {
        R.drawable.icon_moon,
        R.drawable.icon_sun,
        R.drawable.icon_nightcloud,
        R.drawable.icon_sun2,
    };

    private static final int[] LAST_ENTER = new int[] {
        R.raw.photo_last_enter_photo1_ani, R.raw.photo_last_enter_photo2_ani,
        R.raw.photo_last_enter_photo3_ani, R.raw.photo_last_enter_photo4_ani
    };

    private static final int[] LAST_EXIT = new int[] {
        R.raw.photo_last_exit_photo1_ani, R.raw.photo_last_exit_photo2_ani,
        R.raw.photo_last_exit_photo3_ani, R.raw.photo_last_exit_photo4_ani
    };

    private AnimationGroup mGroup = new AnimationGroup();
    private static final int PHOTO_PER_PAGE = 4;
    private GestureDetector mGestureDetector;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mGestureDetector = new GestureDetector(this, new MyGestureListener());
        Animation.Listener l = new Animation.Listener() {
            public void onStarted(Animation animation) {
                android.util.Log.v(TAG, "onStarted " + animation);
            }

            public void onCompleted(Animation animation) {
                android.util.Log.v(TAG, "onCompleted " + animation);
                if (animation.getDirection() == Animation.FORWARD) {
                    animation.setDirection(Animation.BACKWARD);
                } else {
                    animation.setDirection(Animation.FORWARD);
                }
            }

            public void onMarkerReached(Animation animation, int direction, String marker) {
                android.util.Log.v(TAG, "onMarkerReached " + marker + " Direction: " + direction);
            }
        };

        for (int i = 0; i < PHOTO_PER_PAGE; i++) {
            final Image photo = Image.createFromResource(getResources(), PHOTO_FRAMES[i]);
            final Image weather = Image.createFromResource(getResources(), WEATHER_FRAMES[i]);
            mStage.add(photo);
            mStage.add(weather);

            Animation photoAni = AnimationLoader.loadAnimation(this, LAST_ENTER[i]);
            Animation weatherAni = AnimationLoader.loadAnimation(this, LAST_EXIT[i]);
            photoAni.setTarget(photo);
            weatherAni.setTarget(weather);

            mGroup.add(photoAni);
            mGroup.add(weatherAni);
        }
        mGroup.addListener(l);
        mGroup.disableOptions(Animation.START_TARGET_WITH_INITIAL_VALUE);
        mGroup.start();
    }

    private class MyGestureListener extends GestureDetector.SimpleOnGestureListener {
        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            mCurrentMovement += distanceY;
            if (mGroup.getDirection() == Animation.BACKWARD) {
                // In backward case of this demo: the movement should less than 0 and the range should be -(maximum distance) ~ 0
                if (mCurrentMovement > 0) {
                    mCurrentMovement = 0;
                }
                if (mCurrentMovement < -mMaxDistance) {
                    mCurrentMovement = -mMaxDistance;
                }
                mGroup.setProgress(1 + (float) mCurrentMovement / mMaxDistance);
            } else {
                // In forward case of this demo: the movement should be 0 ~ (maximum distance)
                if (mCurrentMovement < 0) {
                    mCurrentMovement = 0;
                }
                mGroup.setProgress((float) mCurrentMovement / mMaxDistance);
            }

            Log.v(TAG, "disX:" + distanceX + "disY:" + distanceY);
            return true;
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_UP) {
            mGroup.start();
            float progress = (float) mCurrentMovement / mMaxDistance;
            if (mGroup.getDirection() == Animation.BACKWARD) {
                if ((1 + progress) > 0.5) {
                    mGroup.reverse();
                }
            } else {
                if (progress < 0.5) {
                    mGroup.reverse();
                }
            }
            mCurrentMovement = 0;
            return true;
        }

        return super.onTouchEvent(event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent m) {
        boolean handled = super.dispatchTouchEvent(m);
        if (!handled) {
            handled = mGestureDetector.onTouchEvent(m);
        }

        if (!handled) {
            handled = onTouchEvent(m);
        }
        return handled;
    }
}
