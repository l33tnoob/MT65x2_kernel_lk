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

import com.mediatek.apst.util.entity.contacts.Contact;
import com.mediatek.apst.util.entity.contacts.RawContact;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

@SuppressWarnings("deprecation")
public class ContactTest extends AndroidTestCase {
    @SuppressWarnings("deprecation")
    private Contact mContact;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContact = new Contact(11);

    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    public void test01_getDisplayName() {
        mContact.setDisplayName("display name");
        assertEquals("display name", mContact.getDisplayName());
    }

    public void test02_getPhotoId() {
        mContact.setPhotoId(101);
        assertEquals(101, mContact.getPhotoId());
    }

    public void test03_getCustomRingtone() {
        mContact.setCustomRingtone("ringtone");
        assertEquals("ringtone", mContact.getCustomRingtone());
    }

    public void test04_isSendToVoicemail() {
        mContact.setSendToVoicemail(false);
        assertEquals(false, mContact.isSendToVoicemail());
    }

    public void test05_getTimesContacted() {
        mContact.setTimesContacted(10);
        assertEquals(10, mContact.getTimesContacted());
    }

    public void test06_getLastTimeContacted() {
        mContact.setLastTimeContacted(222);
        assertEquals(222, mContact.getLastTimeContacted());
    }

    public void test07_isStarred() {
        mContact.setStarred(true);
        assertEquals(true, mContact.isStarred());
    }

    public void test08_isInVisibleGroup() {
        mContact.setInVisibleGroup(false);
        assertEquals(false, mContact.isInVisibleGroup());
    }

    public void test09_isHasPhoneNumber() {
        mContact.setHasPhoneNumber(false);
        assertEquals(false, mContact.isHasPhoneNumber());
    }

    public void test10_getLookup() {
        mContact.setLookup("lookup");
        assertEquals("lookup", mContact.getLookup());
    }

    public void test11_getRawContacts() {
        ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
        mContact.setRawContacts(rawContacts);
        assertSame(rawContacts, mContact.getRawContacts());
    }

    public void test12_isThreadSafe() {
        assertFalse(mContact.isThreadSafe());
    }

    public void test13_isThreadSafe() {
        Contact contact = new Contact(11, true);
        assertTrue(contact.isThreadSafe());
    }

    /**
     * Test "setRawContacts(Vector<RawContact> rawContacts)". The rawContacts is
     * null;
     */
    public void test14_setRawContacts() {
        ArrayList<RawContact> rawContacts = null;
        assertFalse(mContact.setRawContacts(rawContacts));
    }

    /**
     * Test "setRawContacts(ArrayList<RawContact> rawContacts)". The rawContacts
     * is not null, the isThreadSafe is false;
     */
    public void test15_setRawContacts() {
        ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
        assertTrue(mContact.setRawContacts(rawContacts));
    }

    /**
     * Test "setRawContacts(ArrayList<RawContact> rawContacts)". The rawContacts
     * is not null, the isThreadSafe is true;
     */
    public void test16_setRawContacts() {
        Contact contact = new Contact(11, true);
        ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
        assertFalse(contact.setRawContacts(rawContacts));
    }

    /**
     * Test "setRawContacts(Vector<RawContact> rawContacts)". The rawContacts is
     * null;
     */
    public void test17_setRawContacts() {
        Vector<RawContact> rawContacts = null;
        assertFalse(mContact.setRawContacts(rawContacts));
    }

    /**
     * Test "setRawContacts(Vector<RawContact> rawContacts)". The rawContacts is
     * not null, the isThreadSafe is false;
     */
    public void test18_setRawContacts() {
        Vector<RawContact> rawContacts = new Vector<RawContact>();
        assertFalse(mContact.setRawContacts(rawContacts));
    }

    /**
     * Test "setRawContacts(Vector<RawContact> rawContacts)". The rawContacts is
     * not null, the isThreadSafe is true;
     */
    public void test19_setRawContacts() {
        Contact contact = new Contact(11, true);
        Vector<RawContact> rawContacts = new Vector<RawContact>();
        assertTrue(contact.setRawContacts(rawContacts));
    }

    /**
     * threadSafe is false;
     */
    public void test20_clone() {
        RawContact rawContact = organizeRawContact();
        mContact.getRawContacts().add(rawContact);
        try {
            mContact.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test21_shallowClone() {
        RawContact rawContact = organizeRawContact();
        mContact.getRawContacts().add(rawContact);
        try {
            mContact.shallowClone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test22_getRawContactsCount() {
        RawContact rawContact = organizeRawContact();
        mContact.getRawContacts().add(rawContact);
        assertEquals(1, mContact.getRawContactsCount());
    }

    public void test23_addRawContact() {
        RawContact rawContact = organizeRawContact();
        mContact.addRawContact(rawContact);
        assertEquals(1, mContact.getRawContactsCount());
    }

    /**
     * The addAll method in class Contact is error, but this class is
     * deprecated.
     */
    // public void test24_addAll() {
    // ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
    // rawContacts.add(organizeRawContact());
    // mContact.addAll(rawContacts);
    // assertEquals(1, mContact.getRawContactsCount());
    // }

    public void test25_removeRawContact() {
        RawContact rawContact = organizeRawContact();
        mContact.addRawContact(rawContact);
        assertEquals(1, mContact.getRawContactsCount());
        mContact.removeRawContact(rawContact);
        assertEquals(0, mContact.getRawContactsCount());
    }

    public void test26_removeRawContact() {
        RawContact rawContact = organizeRawContact();
        mContact.addRawContact(rawContact);
        assertEquals(1, mContact.getRawContactsCount());
        mContact.removeRawContact(0);
        assertEquals(0, mContact.getRawContactsCount());
    }

    /**
     * The addAll method in class Contact is error, but this class is
     * deprecated.
     */
    // public void test27_removeAll() {
    // ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
    // rawContacts.add(organizeRawContact());
    // mContact.addAll(rawContacts);
    // assertEquals(1, mContact.getRawContactsCount());
    // assertTrue(mContact.removeAll(rawContacts));
    // assertEquals(0, mContact.getRawContactsCount());
    // }

    public void test28_clear() {
        ArrayList<RawContact> rawContacts = new ArrayList<RawContact>();
        rawContacts.add(organizeRawContact());
        mContact.setRawContacts(rawContacts);
        assertEquals(1, mContact.getRawContactsCount());
        mContact.clear();
        assertEquals(0, mContact.getRawContactsCount());
    }

    private RawContact organizeRawContact() {
        RawContact rawContact = new RawContact();
        rawContact.setCustomRingtone("ringtone");
        rawContact.setDirty(false);
        rawContact.setDisplayName("display name");
        rawContact.setLastTimeContacted(1233);
        rawContact.setVersion(2);
        rawContact.setLastTimeContacted(3344);
        rawContact.setModifyTime(111);
        return rawContact;
    }
}
