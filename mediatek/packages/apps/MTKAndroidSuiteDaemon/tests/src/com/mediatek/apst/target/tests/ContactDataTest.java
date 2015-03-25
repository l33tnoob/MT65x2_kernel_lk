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

import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.contacts.ContactData;

public class ContactDataTest extends AndroidTestCase {
    private ContactData mContactData;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContactData = new ContactData(2);

    }

    @Override
    protected void tearDown() throws Exception {

        mContactData = null;
        super.tearDown();
    }

    public void test01_getRawContactId() {
        assertEquals(DatabaseRecordEntity.ID_NULL, mContactData
                .getRawContactId());
    }

    public void test02_getRawContactId() {
        ContactData contactData = new ContactData(1, 2, 3, false, false, 4);
        assertEquals(3, contactData.getRawContactId());
    }

    public void test03_isPrimary() {
        assertFalse(mContactData.isPrimary());
    }

    public void test04_isPrimary() {
        ContactData contactData = new ContactData(1, 2, 3, true, false, 4);
        assertTrue(contactData.isPrimary());
    }

    public void test05_isSuperPrimary() {
        assertFalse(mContactData.isSuperPrimary());
    }

    public void test06_isSuperPrimary() {
        ContactData contactData = new ContactData(1, 2, 3, false, true, 4);
        assertTrue(contactData.isSuperPrimary());
    }

    public void test07_getMimeType() {
        assertEquals(2, mContactData.getMimeType());
    }

    public void test08_setRawContactId() {
        mContactData.setRawContactId(11);
        assertEquals(11, mContactData.getRawContactId());
    }

    public void test09_setPrimary() {
        assertFalse(mContactData.isPrimary());
        mContactData.setPrimary(true);
        assertTrue(mContactData.isPrimary());
    }

    public void test10_setSuperPrimary() {
        assertFalse(mContactData.isSuperPrimary());
        mContactData.setSuperPrimary(true);
        assertTrue(mContactData.isSuperPrimary());
    }

    public void test11_setMimeType() {
        assertEquals(2, mContactData.getMimeType());
        mContactData.setMimeType(3);
        assertEquals(3, mContactData.getMimeType());
    }

    public void test12_getMimeTypeString() {
        assertEquals("", mContactData.getMimeTypeString());
    }

    public void test13_clone() {
        try {
            mContactData.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }
}
