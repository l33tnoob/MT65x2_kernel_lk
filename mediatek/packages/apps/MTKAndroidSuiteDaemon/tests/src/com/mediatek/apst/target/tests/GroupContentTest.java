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
import android.database.Cursor;
import android.provider.ContactsContract.Groups;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.provider.contacts.GroupContent;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.contacts.Group;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class GroupContentTest extends AndroidTestCase {

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_cursorToGroup() {
        Group result = null;
        ContactsProxy.getInstance(mContext).insertGroup(
                ContactsUtils.getGroup());
        Cursor cursor = mContext.getContentResolver().query(Groups.CONTENT_URI,
                null, null, null, Groups._ID + " ASC");
        assertTrue(cursor.moveToNext());
        result = GroupContent.cursorToGroup(cursor);
        assertNotNull(result);
        result = GroupContent.cursorToGroup(null);
        assertNull(result);
    }

    public void test02_cursorToRaw() {
        int result;
        ContactsProxy.getInstance(mContext).insertGroup(
                ContactsUtils.getGroup());
        Cursor cursor = mContext.getContentResolver().query(Groups.CONTENT_URI,
                null, null, null, Groups._ID + " ASC");
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        assertTrue(cursor.moveToNext());
        result = GroupContent.cursorToRaw(cursor, buffer);
        assertTrue(result >= 0);
        result = GroupContent.cursorToRaw(null, buffer);
        assertTrue(result == 0);
        result = GroupContent.cursorToRaw(cursor, null);
        assertTrue(result == 0);
    }

    public void test03_groupsToValues() {
        ContentValues[] result = null;
        List<Group> groups = new ArrayList<Group>(3);
        Group group = ContactsUtils.getGroup();
        groups.add(group);
        groups.add(group);
        groups.add(group);
        result = GroupContent.groupsToValues(groups);
        assertNotNull(result);
        result = GroupContent.groupsToValues(null);
        assertNull(result);
    }

    public void test04_groupToValues() {
        ContentValues result = null;
        result = GroupContent.groupToValues(ContactsUtils.getGroup());
        assertNotNull(result);
    }
}
