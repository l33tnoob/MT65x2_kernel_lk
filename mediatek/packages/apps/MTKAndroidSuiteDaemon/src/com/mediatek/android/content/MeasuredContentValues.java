/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.android.content;

import android.content.ContentValues;

public class MeasuredContentValues {
    /** Actual content values */
    private final ContentValues mContentValues;
    /** Parcel size of the content values */
    private int mParcelSize;

    /**
     * constructor.
     */
    public MeasuredContentValues() {
        // 8 is preferred initiate size of ContentValues by default
        this(8);
    }

    /**
     * @param initSize
     *            The size of the ContentValues.
     */
    public MeasuredContentValues(final int initSize) {
        this.mContentValues = new ContentValues(initSize);
        // An integer will be written to tell the map size in content values
        // in parcel
        this.mParcelSize = 4;
    }

    /**
     * @return The values.
     */
    public ContentValues getValues() {
        return this.mContentValues;
    }

    /**
     * @return The size of the ContentValues.
     */
    public int measure() {
        return this.mParcelSize;
    }

    /**
     * @param value
     *            The value to be measured.
     * @return length of the String.
     */
    public int measureValue(final String value) {
        /*
         * A String value will be written in parcel as: 1 Integer(4 bytes):
         * Value type 1 Integer(4 bytes): String length Every 2 char takes 4
         * bytes (both 1 and 2 chars takes 4 bytes) Always a bonus 4 bytes
         * (maybe for '/0'?)
         * 
         * So cost 4 + 4 + 4 * (value.length() / 2 + 1) bytes
         */
        int result;
        if (null == value) {
            result = 4;
        } else {
            result = 4 * (value.length() / 2 + 3);
        }
        return result;
    }

    /**
     * @param value
     *            The value to be valued.
     * @return the length of byte[].
     */
    public int measureValue(final byte[] value) {
        /*
         * A byte[] will be written as: 1 Integer(4 bytes): Value type 1
         * Integer(4 bytes): byte array length Every 1 byte in array takes 1
         * byte
         * 
         * So cost 4 + 4 + value.length bytes
         */
        int result;
        if (null == value) {
            result = 4;
        } else {
            result = 8 + value.length;
        }
        return result;
    }

    /**
     * @param value
     *            The value to be measured.
     * @return the length of the Integer value.
     */
    public int measureValue(final Integer value) {
        /*
         * A Integer will be written as: 1 Integer(4 bytes): Value type 1
         * Integer(4 bytes): Value
         * 
         * So cost 8 bytes
         */
        int result;
        if (null == value) {
            result = 4;
        } else {
            result = 8;
        }
        return result;
    }

    /**
     * @param value
     *            The value to be measured.
     * @return The length of the Long value.
     */
    public int measureValue(final Long value) {
        /*
         * A Long will be written as: 1 Integer(4 bytes): Value type 1 Long(8
         * bytes): Value
         * 
         * So cost 12 bytes
         */
        int result;
        if (null == value) {
            result = 4;
        } else {
            result = 12;
        }
        return result;
    }

    /**
     * clear the ContentValues and set mParcelSize as 4.
     */
    public void clear() {
        mContentValues.clear();
        // An integer will be written to tell the map size in content values
        // in parcel
        mParcelSize = 4;
    }

    /**
     * @param key
     *            The key of the ContentValues to put.
     * @param value
     *            The Integer value of the ContentValues to put.
     */
    public void put(final String key, final Integer value) {
        if (!mContentValues.containsKey(key)) {
            mParcelSize += measureValue(key) + measureValue(value);
        }
        // Put actual content values
        mContentValues.put(key, value);
    }

    /**
     * @param key
     *            The key of the ContentValues to put.
     * @param value
     *            he Long value of the ContentValues to put.
     */
    public void put(final String key, final Long value) {
        if (!mContentValues.containsKey(key)) {
            mParcelSize += measureValue(key) + measureValue(value);
        }
        // Put actual content values
        mContentValues.put(key, value);
    }

    /**
     * @param key
     *            The key of the ContentValues to put.
     * @param value
     *            he String value of the ContentValues to put.
     */
    public void put(final String key, final String value) {
        if (mContentValues.containsKey(key)) {
            final String old = mContentValues.getAsString(key);
            mParcelSize += measureValue(value) - measureValue(old);
        } else {
            mParcelSize += measureValue(key) + measureValue(value);
        }
        // Put actual content values
        mContentValues.put(key, value);
    }

    /**
     * @param key
     *            The key of the ContentValues to put.
     * @param value
     *            he byte[] value of the ContentValues to put.
     */
    public void put(final String key, final byte[] value) {
        if (mContentValues.containsKey(key)) {
            final byte[] old = mContentValues.getAsByteArray(key);
            mParcelSize += measureValue(value) - measureValue(old);
        } else {
            mParcelSize += measureValue(key) + measureValue(value);
        }
        // Put actual content values
        mContentValues.put(key, value);
    }
}
