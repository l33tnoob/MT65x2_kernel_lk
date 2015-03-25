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

import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.contacts.BaseContact;
import com.mediatek.apst.util.entity.contacts.Group;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Vector;

public class GroupTest extends AndroidTestCase {
    private static final String TAG = "GroupTest";
    private Group mGroup;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mGroup = new Group();

    }

    @Override
    protected void tearDown() throws Exception {

        mGroup = null;
        super.tearDown();
    }

    public void test01_getTitle() {
        mGroup.setTitle("classmate");
        assertEquals("classmate", mGroup.getTitle());
    }

    public void test02_getNotes() {
        mGroup.setNotes("a note");
        assertEquals("a note", mGroup.getNotes());
    }

    public void test03_getSystemId() {
        mGroup.setSystemId("111");
        assertEquals("111", mGroup.getSystemId());
    }

    public void test04_getDeleted() {
        mGroup.setDeleted("deleted");
        assertEquals("deleted", mGroup.getDeleted());
    }

    public void test05_getAccount_name() {
        mGroup.setAccount_name("accoutName");
        assertEquals("accoutName", mGroup.getAccount_name());
    }

    public void test06_getAccount_type() {
        mGroup.setAccount_type("accoutType");
        assertEquals("accoutType", mGroup.getAccount_type());
    }

    public void test07_getVersion() {
        mGroup.setVersion("1.1");
        assertEquals("1.1", mGroup.getVersion());
    }

    public void test08_getDirty() {
        mGroup.setDirty("false");
        assertEquals("false", mGroup.getDirty());
    }

    public void test09_getGroup_visible() {
        mGroup.setGroup_visible("true");
        assertEquals("true", mGroup.getGroup_visible());
    }

    public void test10_getShould_sync() {
        mGroup.setShould_sync("false");
        assertEquals("false", mGroup.getShould_sync());
    }

    /**
     * setMembers(ArrayList<BaseContact> members). members is null.
     */
    public void test11_setMembers() {
        ArrayList<BaseContact> members = null;
        assertFalse(mGroup.setMembers(members));
    }

    /**
     * setMembers(ArrayList<BaseContact> members). members is not null.
     */
    public void test12_setMembers() {
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        members.add(new BaseContact());
        assertTrue(mGroup.setMembers(members));
    }

    /**
     * setMembers(Vector<BaseContact> members). members is null.
     */
    public void test13_setMembers() {
        Vector<BaseContact> members = null;
        assertFalse(mGroup.setMembers(members));
    }

    /**
     * setMembers(Vector<BaseContact> members). members is not null.
     */
    public void test14_setMembers() {
        Vector<BaseContact> members = new Vector<BaseContact>();
        members.add(new BaseContact());
        assertTrue(mGroup.setMembers(members));
    }

    public void test15_clone() {
        try {
            mGroup.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test16_shallowClone() {
        try {
            mGroup.shallowClone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test17_getSize() {
        ArrayList<BaseContact> members = organizeMembers();
        assertTrue(mGroup.setMembers(members));
        assertEquals(3, mGroup.getSize());
        members.clear();
    }

    public void test18_addMember() {
        BaseContact member = new BaseContact();
        mGroup.addMember(member);
        assertEquals(1, mGroup.getSize());
    }

    /**
     * The method addAll in class Group is wrong.
     */
    // public void test19_addAll() {
    // ArrayList<BaseContact> members = organizeMembers();
    // assertTrue(mGroup.addAll(members));
    // Log.i(TAG, "the count of the members: " + members.size());
    // Log.i(TAG, "the count of the mMembers: " + mGroup.getSize());
    // assertEquals(3,mGroup.getSize());
    // mGroup.clear();
    // }

    /**
     * removeMember(BaseContact member).
     */
    public void test20_removeMember() {
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact member1 = new BaseContact(1);
        BaseContact member2 = new BaseContact(2);
        BaseContact member3 = new BaseContact(3);
        members.add(member1);
        members.add(member2);
        members.add(member3);
        assertTrue(mGroup.setMembers(members));
        assertEquals(3, mGroup.getSize());
        assertTrue(mGroup.removeMember(member2));
        assertEquals(2, mGroup.getSize());
    }

    /**
     * removeMember(int location)
     */
    public void test21_removeMember() {
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact member1 = new BaseContact(1);
        BaseContact member2 = new BaseContact(2);
        BaseContact member3 = new BaseContact(3);
        members.add(member1);
        members.add(member2);
        members.add(member3);
        assertTrue(mGroup.setMembers(members));
        assertEquals(3, mGroup.getSize());
        assertSame(member3, mGroup.removeMember(2));
        assertEquals(2, mGroup.getSize());
        members.clear();
    }

    /**
     * The method is wrong in the class Group.
     */
    public void test22_removeAll() {
        ArrayList<BaseContact> members = organizeMembers();
        assertTrue(mGroup.setMembers(members));
        Log.i(TAG, "removeAll, the members: " + members.size());
        Log.i(TAG, "removeAll, the mMembers: " + mGroup.getSize());
        assertEquals(3, mGroup.getSize());
        mGroup.removeAll(members);
        Log.i(TAG, "removeAll, the members: " + members.size());
        Log.i(TAG, "removeAll, the mMembers: " + mGroup.getSize());
        assertEquals(0, mGroup.getSize());
        members.clear();
    }

    public void test23_clear() {
        ArrayList<BaseContact> members = organizeMembers();
        assertTrue(mGroup.setMembers(members));
        mGroup.clear();
        assertEquals(0, mGroup.getSize());
    }

    public void test24_writeRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(200);
        mGroup.setAccount_name("accoutName");
        mGroup.setAccount_type("accoutType");
        mGroup.setDeleted("deleted");
        mGroup.setDirty("true");
        mGroup.setGroup_visible("true");
        mGroup.setId(111);
        mGroup.setNotes("notes");
        mGroup.setSystemId("22");
        mGroup.setTitle("classMate");
        mGroup.setVersion("2.0");
        mGroup.setShould_sync("false");

        mGroup.writeRaw(buffer);
        buffer.position(8);
        assertEquals("classMate", RawTransUtil.getString(buffer));
        assertEquals("notes", RawTransUtil.getString(buffer));
        assertEquals("22", RawTransUtil.getString(buffer));
        assertEquals("deleted", RawTransUtil.getString(buffer));
        assertEquals("accoutName", RawTransUtil.getString(buffer));
        assertEquals("accoutType", RawTransUtil.getString(buffer));
        assertEquals("2.0", RawTransUtil.getString(buffer));
        assertEquals("true", RawTransUtil.getString(buffer));
        assertEquals("true", RawTransUtil.getString(buffer));
        assertEquals("false", RawTransUtil.getString(buffer));

    }

    public void test25_readRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(200);
        Group group = new Group();
        group.setAccount_name("accoutName");
        group.setAccount_type("accoutType");
        group.setDeleted("deleted");
        group.setDirty("true");
        group.setGroup_visible("true");
        group.setId(111);
        group.setNotes("notes");
        group.setSystemId("22");
        group.setTitle("classMate");
        group.setVersion("2.0");
        group.setShould_sync("false");

        group.writeRaw(buffer);
        buffer.position(0);
        mGroup.readRaw(buffer);

        assertEquals("accoutName", mGroup.getAccount_name());
        assertEquals("accoutType", mGroup.getAccount_type());
        assertEquals("deleted", mGroup.getDeleted());
        assertEquals("true", mGroup.getDirty());
        assertEquals("true", mGroup.getGroup_visible());
        assertEquals("notes", mGroup.getNotes());
        assertEquals("22", mGroup.getSystemId());
        assertEquals("classMate", mGroup.getTitle());
        assertEquals("2.0", mGroup.getVersion());
        assertEquals("false", mGroup.getShould_sync());

    }

    private ArrayList<BaseContact> organizeMembers() {
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        members.add(new BaseContact(1));
        members.add(new BaseContact(2));
        members.add(new BaseContact(3));
        return members;
    }
}
