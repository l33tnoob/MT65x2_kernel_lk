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
import android.util.Log;
import android.view.Display;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.GestureDetector.SimpleOnGestureListener;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Glo3D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.android.StageActivity;

public class EularAngleDemo extends StageActivity {

    private static final float SURFZ = 0.f;
    private Container mScenario;
    private GestureDetector mGestureDetector;
    private float mYaw;
    private float mRoll;
    private float mPitch = 180.0f;
    private Text mYawText;
    private Text mRollText;
    private Text mPitchText;

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
                    mRoll = mRoll - distanceX / 10;
                    mRollText.setText("roll angle: " + mRoll);
                }

                if (distanceY > 10 || distanceY < 10) {
                    mPitch = mPitch - distanceY / 10;
                    mPitchText.setText("pitch angle: " + mPitch);
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
        mYawText.setText("yaw angle: " + mYaw);
        mScenario.setRotation(new Rotation(mPitch, mYaw, mRoll));
    }

    private float spacing(MotionEvent event) {
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return FloatMath.sqrt(x * x + y * y);
    }

    int mode = 0;
    float oldDist;

    public boolean onTouchEvent(MotionEvent event) {

        mYaw = mScenario.getRotation().getEulerAngles()[1];

        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN:
                mode = 1;
                break;
            case MotionEvent.ACTION_UP:
                mode = 0;

                break;
            case MotionEvent.ACTION_POINTER_UP:
                mode -= 1;
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                oldDist = spacing(event);
                mode += 1;
                break;

            case MotionEvent.ACTION_MOVE:

                if (mode >= 2) {
                    float newDist = spacing(event);
                    if (newDist > oldDist + 1) {
                        Log.e("zoom", ">>");
                        zoom(newDist / oldDist);
                        oldDist = newDist;
                    }
                    if (newDist < oldDist - 1) {
                        Log.e("zoom", "<<");
                        zoom(newDist / oldDist);
                        oldDist = newDist;
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

        mGestureDetector = new GestureDetector(new MyGestureDetector());
        mScenario = new Container();

        final Glo3D landscape = Glo3D.createFromAsset("landscape.glo");

        // add a directional light to the scene to illuminate the landscape
        final Glo3D direct_light = Glo3D.createFromAsset("direct_light.glo");
        direct_light.setRotation(new Rotation (30, 30, 0));

        mScenario.add(landscape, direct_light);
        mScenario.setPosition(new Point(0.5f, 0.56f, SURFZ, true));
        mScenario.setRotation(new Rotation(mPitch, mYaw, mRoll));
        mScenario.setScale(new Scale(30, 30, 30));

        mPitchText = new Text("pitch angle: " + mScenario.getRotation().getEulerAngles()[0]);
        mPitchText.setBackgroundColor(new Color(255, 0, 0, 128));
        mPitchText.setPosition(new Point(0.5f, 0.13f, true));

        mYawText = new Text("yaw angle: " + mScenario.getRotation().getEulerAngles()[1]);
        mYawText.setBackgroundColor(new Color(255, 0, 0, 128));
        mYawText.setPosition(new Point(0.5f, 0.19f, true));

        mRollText = new Text("roll angle: " + mScenario.getRotation().getEulerAngles()[2]);
        mRollText.setBackgroundColor(new Color(255, 0, 0, 128));
        mRollText.setPosition(new Point(0.5f, 0.25f, true));

        mStage.add(mScenario, mYawText, mRollText, mPitchText);
    }
}
