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

import java.util.ArrayList;

public abstract class DefaultBulkInsertHelper {
    public static final int MAX_PARCEL_SIZE = 1000000;

    public static final int DEFAULT_BULK_LIMIT = 499;

    public static final int SC_OK = 0x00000001;
    public static final int SC_FAILED = 0x00000000;
    public static final int SC_EXECUTED = 0x10000001;
    public static final int SC_APPENDED = 0x00100001;
    public static final int SC_BUFFER_EMPTY = 0x01000001;
    public static final int SC_SINGLE_VALUE_EXCEED = 0x00100000;

    // ==============================================================
    // Fields
    // ==============================================================
    private final ArrayList<ContentValues> mBuffer;

    private int mParcelSize;

    private final int mBulkLimit;

    private int mStatusCode;

    /**
     * constructor.
     */
    public DefaultBulkInsertHelper() {
        this.mBulkLimit = DEFAULT_BULK_LIMIT;
        this.mBuffer = new ArrayList<ContentValues>(this.mBulkLimit);
        this.mParcelSize = 0;
    }

    /**
     * @param bulkLimit
     *            The max number of bulk insert.
     */
    public DefaultBulkInsertHelper(final int bulkLimit) {
        this.mBulkLimit = bulkLimit;
        this.mBuffer = new ArrayList<ContentValues>(this.mBulkLimit);
        this.mParcelSize = 0;
    }

    /**
     * @return the StatusCode.
     */
    public int getStatusCode() {
        return this.mStatusCode;
    }

    /**
     * @param values The ContentValues to execute.
     * @return whether execute succeeded.
     */
    public abstract boolean onExecute(ContentValues[] values);

    /**
     * @param measuredValues The measuredValues used to measure the size of object.
     * @return whether appended succeeded.
     */
    public boolean append(final MeasuredContentValues measuredValues) {
        if (null == measuredValues) {
            mStatusCode = SC_APPENDED;
            return true;
        }
        if (measuredValues.measure() > MAX_PARCEL_SIZE) {
            // Single ContentValues already exceeds parcel size limit
            this.mStatusCode = SC_SINGLE_VALUE_EXCEED;
            return false;
        } else if (mParcelSize + measuredValues.measure() > MAX_PARCEL_SIZE) {
            // Buffer exceeds parcel size limit, bulk insert current
            // ContentValues array, and free buffer for new ContentValues
            if (!execute()) {
                return false;
            }
        } else if (mBuffer.size() >= mBulkLimit) {
            // Insert 'bulkLimit' records at most at one time, bulk insert
            // current
            // ContentValues array, and free buffer for new ContentValues
            if (!execute()) {
                return false;
            }
        }
        mParcelSize += measuredValues.measure();
        mBuffer.add(measuredValues.getValues());
        mStatusCode = SC_APPENDED;
        return true;
    }

    /**
     * @return the length of list "ContentValues".
     */
    public int size() {
        return this.mBuffer.size();
    }

    /**
     * clear the list and reset the mParcelSize as 0.
     */
    public void clear() {
        mParcelSize = 0;
        mBuffer.clear();
    }

    /**
     * @return whether executed succeeded.
     */
    public boolean execute() {
        boolean result;
        if (mBuffer.size() > 0) {
            final ContentValues[] values = new ContentValues[mBuffer.size()];
            mBuffer.toArray(values);
            // Reset
            clear();
            // Execute insert
            if (onExecute(values)) {
                mStatusCode = SC_EXECUTED;
                result = true;
            } else {
                mStatusCode = SC_FAILED;
                result = false;
            }
        } else {
            mStatusCode = SC_BUFFER_EMPTY;
            result = true;
        }
        return result;
    }

    /**
     * @return true, if mStatusCode is not 0.
     */
    public boolean isStatusOK() {
        return ((this.mStatusCode & SC_OK) != 0);
    }
}
