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

import java.util.HashMap;

/**
 * A helper class to store arrays of sample.
 *
 * @hide This level of detail should not be exposed in an abstract API
 */
public class Samples {
    // Types
    public static final int TRANSLATE = 1;
    public static final int ROTATE = 2;
    public static final int SCALE = 3;
    public static final int ALPHA = 4;
    public static final int X_ROTATE = 5;
    public static final int Y_ROTATE = 6;
    public static final int Z_ROTATE = 7;
    public static final int ANCHOR_POINT = 8;
    public static final int MARKER = 9;

    // Arrays
    public static final String VALUE = "v";
    public static final String X_AXIS = "x";
    public static final String Y_AXIS = "y";
    public static final String Z_AXIS = "z";
    public static final String CURVE_TYPE = "type";
    public static final String KEYFRAME_TIME = "time";
    public static final String IN_TANX = "itx";
    public static final String IN_TANY = "ity";
    public static final String IN_TANZ = "itz";
    public static final String OUT_TANX = "otx";
    public static final String OUT_TANY = "oty";
    public static final String OUT_TANZ = "otz";
    public static final String IN_TANVAL = IN_TANX;
    public static final String OUT_TANVAL = OUT_TANX;
    public static final String ACTION = "action";
    public static final String MARKER_TIME = "time";

    private final HashMap<String, float[]> mSampleArrays = new HashMap<String, float[]>();
    private final HashMap<String, int[]> mIntSampleArrays = new HashMap<String, int[]>();
    private final HashMap<String, String[]> mStringSampleArrays = new HashMap<String, String[]>();

    private final int mType;

    public Samples(int type) {
        mType = type;
    }

    public int getType() {
        return mType;
    }

    public Samples add(String name, float[] array) {
        mSampleArrays.put(name, array);
        return this;
    }

    public Samples remove(String name) {
        mSampleArrays.remove(name);
        mIntSampleArrays.remove(name);
        return this;
    }

    public float[] get(String name) {
        return mSampleArrays.get(name);
    }

    public Samples add(String name, int[] array) {
        mIntSampleArrays.put(name, array);
        return this;
    }

    public int[] getInt(String name) {
        return mIntSampleArrays.get(name);
    }

    public Samples add(String name, String[] array) {
        mStringSampleArrays.put(name, array);
        return this;
    }

    public String[] getString(String name) {
        return mStringSampleArrays.get(name);
    }
}