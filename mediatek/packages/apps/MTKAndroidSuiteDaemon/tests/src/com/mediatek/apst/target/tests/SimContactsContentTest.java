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

import android.database.Cursor;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.provider.contacts.SimContactsContent;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.DataStoreLocations;
import com.mediatek.apst.util.entity.contacts.RawContact;

import java.nio.ByteBuffer;

public class SimContactsContentTest extends AndroidTestCase {

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_cursorToRawContact() {
        RawContact result = null;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                ContactsProxy.getInstance(mContext).insertSimContact("test",
                        "10086", DataStoreLocations.SIM1);
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(1),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRawContact(null, 1);
                assertNull(result);
                result = SimContactsContent.cursorToRawContact(cursor, 1);
                cursor.close();
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                ContactsProxy.getInstance(mContext).insertSimContact("test",
                        "10086", DataStoreLocations.SIM2);
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(2),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRawContact(null, 2);
                assertNull(result);
                SimContactsContent.cursorToRawContact(cursor, 2);
                cursor.close();
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                ContactsProxy.getInstance(mContext).insertSimContact("test",
                        "10086", DataStoreLocations.SIM);
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(0),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRawContact(null, 0);
                assertNull(result);
                SimContactsContent.cursorToRawContact(cursor, 0);
                cursor.close();
            }
        }
    }

    public void test02_cursorToRawContact() {
        Cursor cursor = null;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(1),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                ContactsProxy.getInstance(mContext).insertSimContact("test",
                        "10086", DataStoreLocations.SIM2);
                cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(2),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                ContactsProxy.getInstance(mContext).insertSimContact("test",
                        "10086", DataStoreLocations.SIM);
                cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(0),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
            }
        }
        if (cursor != null) {
            cursor.moveToFirst();
            SimContactsContent.cursorToRawContact(cursor);
            cursor.close();
        } else {
            SimContactsContent.cursorToRawContact(cursor);
        }
    }

    public void test03_cursorToRaw() {
        int result;
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(1),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer, 1);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null, 1);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, buffer, 1);
                cursor.close();
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(2),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer, 2);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null, 2);
                assertTrue(result == 0);
                SimContactsContent.cursorToRaw(cursor, buffer, 2);
                cursor.close();
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(0),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer, 0);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null, 0);
                assertTrue(result == 0);
                SimContactsContent.cursorToRaw(cursor, buffer, 0);
                cursor.close();
            }
        }
    }

    public void test04_cursorToRaw() {
        int result;
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(1),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, buffer);
                cursor.close();
            }

            if (SystemInfoProxy.isSim2Accessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(2),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null);
                assertTrue(result == 0);
                SimContactsContent.cursorToRaw(cursor, buffer);
                cursor.close();
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                Cursor cursor = mContext.getContentResolver().query(
                        ContactsProxy.getSimUri(0),
                        new String[] { SimContactsContent.COLUMN_ID,
                                SimContactsContent.COLUMN_NAME,
                                SimContactsContent.COLUMN_NUMBER }, null, null,
                        SimContactsContent.COLUMN_NAME + " ASC");
                cursor.moveToFirst();
                result = SimContactsContent.cursorToRaw(null, buffer);
                assertTrue(result == 0);
                result = SimContactsContent.cursorToRaw(cursor, null);
                assertTrue(result == 0);
                SimContactsContent.cursorToRaw(cursor, buffer);
                cursor.close();
            }
        }
    }
}
