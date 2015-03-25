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

import android.util.FloatMath;
import android.view.Display;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.GestureDetector.SimpleOnGestureListener;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.android.StageActivity;

public class Glo3DScaleRotationDemo extends StageActivity {

    private Container mScenario;
    private GestureDetector mGestureDetector;
    private float mYaw;
    private float mRoll;
    private float mPitch = 180.0f;
    private int mMode = 0;
    private float mOldDist;
    private Scale mCurrentScale;
    private Image mUpbutton;
    private Image mDownbutton;

    public class MyGestureDetector extends SimpleOnGestureListener {

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
                float distanceY) {
            mPitch = mScenario.getRotation().getEulerAngles()[0];
            mYaw = mScenario.getRotation().getEulerAngles()[1];
            mRoll = mScenario.getRotation().getEulerAngles()[2];

            if (e1.getAction() != MotionEvent.ACTION_POINTER_DOWN
                    || e1.getAction() != MotionEvent.ACTION_POINTER_UP) {
                if (distanceX < 10 || distanceX > 10) {
                    mRoll = mRoll - distanceX / 5;
                }

                if (distanceY > 10 || distanceY < 10) {
                    mPitch = mPitch - distanceY / 5;
                }
            }

            mScenario.setRotation(new Rotation(mPitch, mYaw, mRoll));
            return true;
        }

        @Override
        public boolean onSingleTapUp(MotionEvent upEvent) {

            return true;
        }

    }

    private void zoom(float f) {
        mYaw = (mYaw + 0.1f) * f;
        mScenario.setRotation(new Rotation(mPitch, mYaw, mRoll));
    }

    private float spacing(MotionEvent event) {
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return FloatMath.sqrt(x * x + y * y);
    }

    public boolean onTouchEvent(MotionEvent event) {

        mYaw = mScenario.getRotation().getEulerAngles()[1];

        if (mUpbutton.hitTest(new Point(event.getX(), event.getY())) != null) {
            mCurrentScale = mScenario.getScale();
            if (mCurrentScale.x <= 30) {
                mCurrentScale.x *= 1.05;
                mCurrentScale.y *= 1.05;
                mCurrentScale.z *= 1.05;
            }

            mScenario.setScale(mCurrentScale);
        } else if (mDownbutton.hitTest(new Point(event.getX(), event.getY())) != null) {
            mCurrentScale = mScenario.getScale();
            if (mCurrentScale.x >= 10) {
                mCurrentScale.x *= 0.95;
                mCurrentScale.y *= 0.95;
                mCurrentScale.z *= 0.95;
            }
            mScenario.setScale(mCurrentScale);
        }
        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN:

                mMode = 1;
                break;
            case MotionEvent.ACTION_UP:
                mMode = 0;

                break;
            case MotionEvent.ACTION_POINTER_UP:
                mMode -= 1;

                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                mOldDist = spacing(event);
                mMode += 1;

                break;
            case MotionEvent.ACTION_MOVE:

                if (mMode >= 2) {
                    float newDist = spacing(event);
                    if (newDist > mOldDist + 1) {

                        zoom(newDist / mOldDist);
                        mOldDist = newDist;
                    } else if (newDist < mOldDist - 1) {

                        zoom(newDist / mOldDist);
                        mOldDist = newDist;
                    }
                    return true;
                }

        }

        if (mGestureDetector.onTouchEvent(event)) {
            return true;
        }

        return true;
    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mUpbutton = Image.createFromResource(getResources(), R.drawable.up_button);
        mDownbutton = Image.createFromResource(getResources(), R.drawable.down_button);

        final Container button = new Container();
        button.add(mUpbutton, mDownbutton);
        mUpbutton.setPosition(new Point(0.21f, 0.88f, 0, true));
        mDownbutton.setPosition(new Point(0.83f, 0.88f, 0, true));
        mGestureDetector = new GestureDetector(new MyGestureDetector());
        mScenario = new Container();

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");
        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D.createFromAsset("direct_light.glo");
        direct_light.setRotation(new Rotation (30, 30, 0));

        mScenario.add(landscape, direct_light);
        mScenario.setPosition(new Point(0.5f, 0.56f, 0, true));
        mScenario.setRotation(new Rotation(mPitch, mYaw, mRoll));
        mScenario.setScale(new Scale(10, 10, 10));
        mStage.add(mScenario, button);
    }
}
