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

import android.content.ContentProviderOperation;
import android.content.Context;
import android.provider.ContactsContract.RawContacts;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.android.content.ContentProviderOperationBatch;
import com.mediatek.apst.target.data.proxy.ObservedContentResolver;

import java.lang.reflect.Field;
import java.util.ArrayList;

public class ContentProviderOperationBatchTest extends AndroidTestCase {
    private static final String TAG = "ContentProviderOperationBatchTest";
    private ContentProviderOperationBatch mOperBatch;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        ObservedContentResolver oCR = new ObservedContentResolver(mContext
                .getContentResolver());
        mOperBatch = new ContentProviderOperationBatch(oCR);
    }

    @Override
    protected void tearDown() throws Exception {
        mOperBatch.clear();
        mOperBatch = null;
        mContext = null;
        super.tearDown();
    }

    /**
     * Test append, when the batch is not full.
     */
    public void test01_append01() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        assertEquals(1, mOperBatch.size());
    }

    /**
     * Test append, when the batch is full.
     */
//    public void test02_append02() {
//        ContentProviderOperation operation = ContentProviderOperation
//                .newInsert(RawContacts.CONTENT_URI).withValue(
//                        RawContacts.ACCOUNT_TYPE, 0).withValue(
//                        RawContacts.ACCOUNT_NAME, "test").build();
//        int capacity = mOperBatch.getCapacity();
//        for (int i = 0; i < capacity; i++) {
//            mOperBatch.append(operation);
//        }
//        mOperBatch.append(operation);
//        Log.i(TAG, "The operation size: " + mOperBatch.size());
//        assertEquals(capacity, mOperBatch.size());
//    }

    public void test03_getCapacity() {
        assertEquals(499, mOperBatch.getCapacity());
    }

    /**
     * Test save().
     * 
     */
    public void test04_save() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        mOperBatch.save();
        Class<ContentProviderOperationBatch> clazz = ContentProviderOperationBatch.class;
        Field field;
        Integer mark = null;
        try {
            field = clazz.getDeclaredField("mMark");
            field.setAccessible(true);
            mark = (Integer) field.get(mOperBatch);
        } catch (SecurityException e) {
            fail(e.getMessage());
        } catch (NoSuchFieldException e) {
            fail(e.getMessage());
        } catch (IllegalArgumentException e) {
            fail(e.getMessage());
        } catch (IllegalAccessException e) {
            fail(e.getMessage());
        }

        assertEquals(mOperBatch.size(), mark.intValue());
    }

    /**
     * Test size().
     * 
     */
    @SuppressWarnings("unchecked")
    public void test05_size() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        Class<ContentProviderOperationBatch> clazz = ContentProviderOperationBatch.class;
        Field field;
        int size = 0;
        try {
            field = clazz.getDeclaredField("mOps");
            field.setAccessible(true);
            ArrayList<ContentProviderOperation> operationList = (ArrayList<ContentProviderOperation>) field
                    .get(mOperBatch);
            size = operationList.size();
        } catch (SecurityException e) {
            fail(e.getMessage());
        } catch (NoSuchFieldException e) {
            fail(e.getMessage());
        } catch (IllegalArgumentException e) {
            fail(e.getMessage());
        } catch (IllegalAccessException e) {
            fail(e.getMessage());
        }

        assertEquals(size, mOperBatch.size());
    }

    public void test06_remaining() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        int remaining = mOperBatch.remaining();
        assertEquals(498, remaining);
    }

    public void test07_clear() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        mOperBatch.clear();
        assertEquals(0, mOperBatch.size());
    }

    /**
     * Test isFull(), when it is not full.
     */
    public void test08_isFull() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        assertFalse(mOperBatch.isFull());
    }

    /**
     * Test isFull(), when it is full.
     */
//    public void test09_isFull02() {
//        ContentProviderOperation operation = ContentProviderOperation
//                .newInsert(RawContacts.CONTENT_URI).withValue(
//                        RawContacts.ACCOUNT_TYPE, 0).withValue(
//                        RawContacts.ACCOUNT_NAME, "test").build();
//        int capacity = mOperBatch.getCapacity();
//        for (int i = 0; i < capacity; i++) {
//            mOperBatch.append(operation);
//        }
//        assertTrue(mOperBatch.isFull());
//    }

    public void test10_rollback() {
        ContentProviderOperation operation = ContentProviderOperation
                .newInsert(RawContacts.CONTENT_URI).withValue(
                        RawContacts.ACCOUNT_TYPE, 0).withValue(
                        RawContacts.ACCOUNT_NAME, "test").build();
        mOperBatch.append(operation);
        mOperBatch.save();
        mOperBatch.append(operation);
        mOperBatch.rollback();
        assertEquals(1, mOperBatch.size());
    }
}
