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

import com.mediatek.apst.util.entity.contacts.BaseContact;
import com.mediatek.apst.util.entity.contacts.GroupMembership;

import java.util.ArrayList;

public class BaseContactTest extends AndroidTestCase {
    private BaseContact mBaseContact;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mBaseContact = new BaseContact();

    }

    @Override
    protected void tearDown() throws Exception {

        mBaseContact = null;
        super.tearDown();
    }

    public void test01_getStoreLocation() {
        mBaseContact.setStoreLocation(1);
        assertEquals(1, mBaseContact.getStoreLocation());
    }

    public void test02_getDisplayName() {
        mBaseContact.setDisplayName("display name");
        assertEquals("display name", mBaseContact.getDisplayName());
    }

    public void test03_getPrimaryNumber() {
        mBaseContact.setPrimaryNumber("13914253696");
        assertEquals("13914253696", mBaseContact.getPrimaryNumber());
    }

    /**
     * groupMemberships = null.
     */
    public void test04_getGroupMemberships() {
        ArrayList<GroupMembership> groupMemberships = null;
        assertFalse(mBaseContact.setGroupMemberships(groupMemberships));
        assertNull(mBaseContact.getGroupMemberships());
    }

    /**
     * groupMemberships != null
     */
    public void test05_getGroupMemberships() {
        GroupMembership ship1 = new GroupMembership(1);
        GroupMembership ship2 = new GroupMembership(2);
        GroupMembership ship3 = new GroupMembership(3);
        ArrayList<GroupMembership> groupMemberships = new ArrayList<GroupMembership>();
        groupMemberships.add(ship1);
        groupMemberships.add(ship2);
        groupMemberships.add(ship3);
        assertTrue(mBaseContact.setGroupMemberships(groupMemberships));
        assertNotNull(mBaseContact.getGroupMemberships());
    }

    public void test06_getSimName() {
        mBaseContact.setSimName("sim1");
        assertEquals("sim1", mBaseContact.getSimName());
    }

    public void test07_getModifyTime() {
        mBaseContact.setModifyTime(12345);
        assertEquals(12345, mBaseContact.getModifyTime());
    }
}
