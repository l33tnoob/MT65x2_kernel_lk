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

package com.mediatek.apst.target.tests;

import android.content.ContentValues;
import android.test.AndroidTestCase;

import com.mediatek.android.content.MeasuredContentValues;

public class MeasuredContentValuesTest extends AndroidTestCase {
    private MeasuredContentValues mMvalues;
    private MeasuredContentValues mMvalues2;
    private final String mString = "a test string";
    private final byte[] mByte = { 1, 2, 3, 4, 5, 6 };
    private final Integer mInt = 100;
    private final Long mLong = (long) 100;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mMvalues = new MeasuredContentValues();
        mMvalues2 = new MeasuredContentValues(10);
    }

    @Override
    protected void tearDown() throws Exception {
        mMvalues.clear();
        mMvalues.clear();
        mMvalues = null;
        mMvalues2 = null;
        super.tearDown();
    }

    /**
     * Test get the real content value.
     */
    public void test01_getValues01() {
        ContentValues contentValues = mMvalues.getValues();
        assertNotNull(contentValues);
    }

    /**
     * Test get the real content value. Constructor with parameter.
     */
    public void test02_getValues02() {
        ContentValues contentValues = mMvalues2.getValues();
        assertNotNull(contentValues);
    }

    /**
     * Test get the Parcel size of the content values.
     */
    public void test03_measure01() {
        int size = mMvalues.measure();
        assertEquals(4, size);
    }

    /**
     * Test get the Parcel size of the content values. Constructor with
     * parameter.
     */
    public void test04_measure02() {
        int size = mMvalues2.measure();
        assertEquals(4, size);
    }

    /**
     * Test get the measure value of the String. The parameter is not null;
     */
    public void test05_measureValue01() {
        int measureValue = mMvalues.measureValue(mString);
        assertEquals(36, measureValue);
    }

    /**
     * Test get the measure value of the String. The parameter is null;
     */
    public void test06_measureValue02() {
        String testString = null;
        int measureValue = mMvalues.measureValue(testString);
        assertEquals(4, measureValue);
    }

    /**
     * Test get the measure value of the byte array. The parameter is not null;
     */
    public void test07_measureValue01() {
        int measureValue = mMvalues.measureValue(mByte);
        assertEquals(14, measureValue);
    }

    /**
     * Test get the measure value of the byte array. The parameter is null;
     */
    public void test08_measureValue02() {
        byte[] testByteArray = null;
        int measureValue = mMvalues.measureValue(testByteArray);
        assertEquals(4, measureValue);
    }

    /**
     * Test get the measure value of the Integer. The parameter is null;
     */
    public void test09_measureValue01() {
        int measureValue = mMvalues.measureValue(mInt);
        assertEquals(8, measureValue);
    }

    /**
     * Test get the measure value of the Integer. The parameter null;
     */
    public void test10_measureValue02() {
        Integer testInteger = null;
        int measureValue = mMvalues.measureValue(testInteger);
        assertEquals(4, measureValue);
    }

    /**
     * Test get the measure value of the Long. The parameter is not null.
     */
    public void test11_measureValue01() {
        int measureValue = mMvalues.measureValue(mLong);
        assertEquals(12, measureValue);
    }

    /**
     * Test get the measure value of the Long. The parameter is null.
     */
    public void test12_measureValue02() {
        Long testLong = null;
        int measureValue = mMvalues.measureValue(testLong);
        assertEquals(4, measureValue);
    }

    public void test13_clear() {
        mMvalues.clear();
        assertEquals(0, mMvalues.getValues().size());
        assertEquals(4, mMvalues.measure());
    }

    public void test14_put() {
        mMvalues.put("test", mByte);
        mMvalues.put("test", mByte);
        //
        mMvalues.put("testString", mString);
        mMvalues.put("testString", mString);
    }
}
