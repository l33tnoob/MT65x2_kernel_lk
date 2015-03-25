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
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.provider.contacts.RawContactsContent;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.contacts.RawContact;

import java.nio.ByteBuffer;

public class RawContactsContentTest extends AndroidTestCase {

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
        result = RawContactsContent.cursorToRawContact(null);
        assertNull(result);
        Cursor cursor = mContext.getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID, Contacts.DISPLAY_NAME,
                        RawContactsContent.COLUMN_MODIFY_TIME,
                        RawContacts.STARRED, RawContacts.SEND_TO_VOICEMAIL,
                        RawContacts.VERSION,
                        RawContactsContent.COLUMN_SOURCE_LOCATION,
                        RawContactsContent.COLUMN_INDEX_IN_SIM }, null, null,
                null);
        assertTrue(cursor.moveToNext());
        result = RawContactsContent.cursorToRawContact(cursor);
        assertNotNull(result);
        cursor.close();
    }

    public void test02_cursorToRaw() {
        int result;
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        Cursor cursor = mContext.getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID, Contacts.DISPLAY_NAME,
                        RawContactsContent.COLUMN_MODIFY_TIME,
                        RawContacts.STARRED, RawContacts.SEND_TO_VOICEMAIL,
                        RawContacts.VERSION,
                        RawContactsContent.COLUMN_SOURCE_LOCATION,
                        RawContactsContent.COLUMN_INDEX_IN_SIM }, null, null,
                null);
        result = RawContactsContent.cursorToRaw(null, buffer);
        assertTrue(result == 0);
        result = RawContactsContent.cursorToRaw(cursor, null);
        assertTrue(result == 0);
        result = RawContactsContent.cursorToRaw(cursor, buffer);
        assertTrue(result >= 0);
    }
}
