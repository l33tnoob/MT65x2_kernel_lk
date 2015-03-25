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

package com.mediatek.ngin3d.animation;

import com.mediatek.ngin3d.EulerOrder;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

/**
 * The interpolator of keyframe.
 * @hide
 */
public class KeyframeInterpolator {
    private final int mSampleType;
    private Object mValue;

    private boolean mNormalized;
    private int mLen;
    private final int[] mCurveType;
    private final float[] mKfTime;
    private float[] mValX;
    private float[] mValY;
    private float[] mValZ;
    private float[] mInTanX;
    private float[] mInTanY;
    private float[] mInTanZ;
    private float[] mOutTanX;
    private float[] mOutTanY;
    private float[] mOutTanZ;

    private static final int FORMULA_NONE = 0x00;
    private static final int FORMULA_TEMPORAL_CONTINUOUS = 0x01;
    private static final int FORMULA_TEMPORAL_AUTOBEZIER = 0x02;
    private static final int FORMULA_SPATIAL_CONTINUOUS = 0x04;
    private static final int FORMULA_SPATIAL_AUTOBEZIER = 0x08;
    private static final int FORMULA_ROVING = 0x10;

    Rotation mEulerStart = new Rotation();
    Rotation mEulerEnd = new Rotation();

    public KeyframeInterpolator(Samples samples) {
        mSampleType = samples.getType();
        mCurveType = samples.getInt(Samples.CURVE_TYPE);
        mKfTime = samples.get(Samples.KEYFRAME_TIME);
        if (mSampleType == Samples.ALPHA) {
            mValX = samples.get(Samples.VALUE);
            mInTanX = samples.get(Samples.IN_TANVAL);
            mOutTanX = samples.get(Samples.OUT_TANVAL);
        } else {
            mValX = samples.get(Samples.X_AXIS);
            mValY = samples.get(Samples.Y_AXIS);
            mValZ = samples.get(Samples.Z_AXIS);
            mInTanX = samples.get(Samples.IN_TANX);
            mInTanY = samples.get(Samples.IN_TANY);
            mInTanZ = samples.get(Samples.IN_TANZ);
            mOutTanX = samples.get(Samples.OUT_TANX);
            mOutTanY = samples.get(Samples.OUT_TANY);
            mOutTanZ = samples.get(Samples.OUT_TANZ);
        }

        if (mSampleType == Samples.ALPHA) {
            mValue = new Float(mValX[0]);
        } else if (mSampleType == Samples.ROTATE || mSampleType == Samples.X_ROTATE
                || mSampleType == Samples.Y_ROTATE || mSampleType == Samples.Z_ROTATE) {
            mValue = new Rotation(mValX[0], mValY[0], mValZ[0]);
        } else if (mSampleType == Samples.TRANSLATE || mSampleType == Samples.ANCHOR_POINT) {
            mValue = new Point(mValX[0], mValY[0], mValZ[0]);
        } else if (mSampleType == Samples.SCALE) {
            mValue = new Scale(mValX[0] / 100, mValY[0] / 100, mValZ[0] / 100);
        } else {
            throw new RuntimeException("Not excepted Sample type");
        }

        if (mKfTime == null) {
            mLen = 0;
        } else {
            mLen = mKfTime.length;
        }
    }

    public Object getValue(float currTime) {
        int nKf = 0;
        if (mLen == 0) {
            return mValue;
        }
        for (; nKf < mLen; nKf++) {
            if (currTime <= mKfTime[nKf]) {
                break;
            }
        }

        if (currTime <= mKfTime[0]) {
            if (mSampleType == Samples.ALPHA) {
                mValue = mValX[0];
            } else {
                setValue(mValX[0], mValY[0], mValZ[0]);
            }
            return mValue;
        } else if (currTime >= mKfTime[mLen - 1]) {
            if (mSampleType == Samples.ALPHA) {
                mValue = mValX[mLen - 1];
            } else {
                setValue(mValX[mLen - 1], mValY[mLen - 1], mValZ[mLen - 1]);
            }
            return mValue;
        } else if (nKf != 0 && nKf < mLen) {
            if (mCurveType[nKf] == FORMULA_SPATIAL_CONTINUOUS && mCurveType[nKf - 1] == FORMULA_SPATIAL_CONTINUOUS) {
                bezier(currTime, nKf);
            } else {
                linear(currTime, nKf);
            }
            return mValue;
        }
        return null;
    }

    private void bezier(float currTime, int nKf) {
        // Cubic bezier curves
        // (1-t)^3 * P0 + 3 * (1-t)^2 * P1 + 3* (1-t) * t^2 * P2 + t^3 * P3
        float t = (currTime - mKfTime[nKf - 1]) / ((mKfTime[nKf] - mKfTime[nKf - 1]));
        float t2 = t * t;
        float t3 = t2 * t;
        float tc = (1 - t);
        float tc2 = tc * tc;
        float tc3 = tc2 * tc;
        float valX = 0;
        float valY = 0;
        float valZ = 0;
        float p0;
        float p1;
        float p2;
        float p3;

        p0 = mValX[nKf - 1];
        p1 = mValX[nKf - 1] + mOutTanX[nKf - 1];
        p2 = mValX[nKf] + mInTanX[nKf];
        p3 = mValX[nKf];
        valX = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;

        if (mSampleType != Samples.ALPHA) {
            p0 = mValY[nKf - 1];
            p1 = mValY[nKf - 1] + mOutTanY[nKf - 1];
            p2 = mValY[nKf] + mInTanY[nKf];
            p3 = mValY[nKf];
            valY = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;

            p0 = mValZ[nKf - 1];
            p1 = mValZ[nKf - 1] + mOutTanZ[nKf - 1];
            p2 = mValZ[nKf] + mInTanZ[nKf];
            p3 = mValZ[nKf];
            valZ = tc3 * p0 + 3 * tc2 * t * p1 + 3 * tc * t2 * p2 + t3 * p3;
        }
        setValue(valX, valY, valZ);
    }

    private void linear(float currTime, int nKf) {
        float px0;
        float py0;
        float pz0;
        float px1;
        float py1;
        float pz1;
        float valX = 0;
        float valY = 0;
        float valZ = 0;
        float t = (currTime - mKfTime[nKf - 1]) / ((mKfTime[nKf] - mKfTime[nKf - 1]));

        if (mSampleType == Samples.ROTATE) {
            px0 = mValX[nKf - 1];
            py0 = mValY[nKf - 1];
            pz0 = mValZ[nKf - 1];
            px1 = mValX[nKf];
            py1 = mValY[nKf];
            pz1 = mValZ[nKf];

            Rotation rot = (Rotation) mValue;
            mEulerStart.set(EulerOrder.ZYX, px0, py0, pz0);
            mEulerEnd.set(EulerOrder.ZYX, px1, py1, pz1);
            Quaternion linearQ = mEulerStart.getQuaternion().slerp(mEulerEnd.getQuaternion(), t);
            rot.set(linearQ.getQ0(), linearQ.getQ1(), linearQ.getQ2(), linearQ.getQ3(), true);
            return;
        } else {
            px0 = mValX[nKf - 1];
            px1 = mValX[nKf];
            valX = px0 * (1 - t) + px1 * t;

            if (mSampleType != Samples.ALPHA) {
                py0 = mValY[nKf - 1];
                py1 = mValY[nKf];
                valY = py0 * (1 - t) + py1 * t;

                pz0 = mValZ[nKf - 1];
                pz1 = mValZ[nKf];
                valZ = pz0 * (1 - t) + pz1 * t;
            }
        }
        setValue(valX, valY, valZ);
    }

    private void setValue(float x, float y, float z) {
        if (mSampleType == Samples.ALPHA) {
            mValue = x;
        } else if (mSampleType == Samples.X_ROTATE
            || mSampleType == Samples.Y_ROTATE
            || mSampleType == Samples.Z_ROTATE
            || mSampleType == Samples.ROTATE) {
            Rotation rot = (Rotation)mValue;
            rot.set(EulerOrder.ZYX, x, y, z);
            // AE Keyframe works on Quaternion, switch to Quaternion mode.
            Quaternion q = rot.getQuaternion();
            rot.set(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3(), true);
        } else if (mSampleType == Samples.TRANSLATE || mSampleType == Samples.ANCHOR_POINT) {
            Point point = (Point)mValue;
            point.x = x;
            point.y = y;
            point.z = z;
        } else if (mSampleType == Samples.SCALE) {
            Scale scale = (Scale)mValue;
            scale.x = x / 100;
            scale.y = y / 100;
            scale.z = z / 100;
        }
    }

    public int getType() {
        return mSampleType;
    }

    public int getDuration() {
        if (mLen == 0)
            return 0;

        return (int)(mKfTime[mLen - 1] * 1000);
    }

    public void setNormalized(boolean isNormalized) {
        mNormalized = isNormalized;
    }

    public boolean isNormalized() {
        return mNormalized;
    }
}
