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

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.util.entity.contacts.ContactData;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.GroupMembership;
import com.mediatek.apst.util.entity.contacts.Im;
import com.mediatek.apst.util.entity.contacts.Nickname;
import com.mediatek.apst.util.entity.contacts.Note;
import com.mediatek.apst.util.entity.contacts.Organization;
import com.mediatek.apst.util.entity.contacts.Phone;
import com.mediatek.apst.util.entity.contacts.Photo;
import com.mediatek.apst.util.entity.contacts.RawContact;
import com.mediatek.apst.util.entity.contacts.StructuredName;
import com.mediatek.apst.util.entity.contacts.StructuredPostal;
import com.mediatek.apst.util.entity.contacts.Website;
import com.mediatek.apst.util.entity.contacts.RawContact.UnknownContactDataTypeException;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Vector;

public class RawContactTest extends AndroidTestCase {
    private RawContact mRawContact;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mRawContact = new RawContact();
    }

    @Override
    protected void tearDown() throws Exception {
        mRawContact = null;
        super.tearDown();
    }

    public void test01_getContactId() {
        mRawContact.setContactId(100);
        assertEquals(100, mRawContact.getContactId());
    }

    public void test02_getLastTimeContacted() {
        mRawContact.setLastTimeContacted(557668675);
        assertEquals(557668675, mRawContact.getLastTimeContacted());
    }

    public void test03_isStarred() {
        mRawContact.setStarred(true);
        assertEquals(true, mRawContact.isStarred());
    }

    public void test04_getCustomRingtone() {
        mRawContact.setCustomRingtone("custom ringtone");
        assertEquals("custom ringtone", mRawContact.getCustomRingtone());
    }

    public void test05_isSendToVoicemail() {
        mRawContact.setSendToVoicemail(false);
        assertEquals(false, mRawContact.isSendToVoicemail());
    }

    public void test06_getVersion() {
        mRawContact.setVersion(2);
        assertEquals(2, mRawContact.getVersion());
    }

    public void test07_isDirty() {
        mRawContact.setDirty(true);
        assertEquals(true, mRawContact.isDirty());
    }

    public void test08_getNames() {
        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        mRawContact.setNames(names);
        assertSame(names, mRawContact.getNames());
    }

    public void test09_getNames() {
        Vector<StructuredName> names = new Vector<StructuredName>();
        mRawContact.setNames(names);
        assertSame(names, mRawContact.getNames());
    }

    public void test10_getPhones() {
        ArrayList<Phone> phones = new ArrayList<Phone>();
        mRawContact.setPhones(phones);
        assertSame(phones, mRawContact.getPhones());
    }

    public void test11_getPhones() {
        Vector<Phone> phones = new Vector<Phone>();
        mRawContact.setPhones(phones);
        assertSame(phones, mRawContact.getPhones());
    }

    public void test12_getEmails() {
        ArrayList<Email> emails = new ArrayList<Email>();
        mRawContact.setEmails(emails);
        assertSame(emails, mRawContact.getEmails());
    }

    public void test13_getEmails() {
        Vector<Email> emails = new Vector<Email>();
        mRawContact.setEmails(emails);
        assertSame(emails, mRawContact.getEmails());
    }

    public void test14_getIms() {
        ArrayList<Im> ims = new ArrayList<Im>();
        mRawContact.setIms(ims);
        assertSame(ims, mRawContact.getIms());
    }

    public void test15_getIms() {
        Vector<Im> ims = new Vector<Im>();
        mRawContact.setIms(ims);
        assertSame(ims, mRawContact.getIms());
    }

    public void test16_getPostals() {
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        mRawContact.setPostals(postals);
        assertSame(postals, mRawContact.getPostals());
    }

    public void test17_getPostals() {
        Vector<StructuredPostal> postals = new Vector<StructuredPostal>();
        mRawContact.setPostals(postals);
        assertSame(postals, mRawContact.getPostals());
    }

    public void test18_getOrganizations() {
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        mRawContact.setOrganizations(organizations);
        assertSame(organizations, mRawContact.getOrganizations());
    }

    public void test19_getOrganizations() {
        Vector<Organization> organizations = new Vector<Organization>();
        mRawContact.setOrganizations(organizations);
        assertSame(organizations, mRawContact.getOrganizations());
    }

    public void test20_getNotes() {
        ArrayList<Note> notes = new ArrayList<Note>();
        mRawContact.setNotes(notes);
        assertSame(notes, mRawContact.getNotes());
    }

    public void test21_getNotes() {
        Vector<Note> notes = new Vector<Note>();
        mRawContact.setNotes(notes);
        assertSame(notes, mRawContact.getNotes());
    }

    public void test22_getNicknames() {
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        mRawContact.setNicknames(nicknames);
        assertSame(nicknames, mRawContact.getNicknames());
    }

    public void test23_getNicknames() {
        Vector<Nickname> nicknames = new Vector<Nickname>();
        mRawContact.setNicknames(nicknames);
        assertSame(nicknames, mRawContact.getNicknames());
    }

    public void test24_getWebsites() {
        ArrayList<Website> websites = new ArrayList<Website>();
        mRawContact.setWebsites(websites);
        assertSame(websites, mRawContact.getWebsites());
    }

    public void test25_getWebsites() {
        Vector<Website> websites = new Vector<Website>();
        mRawContact.setWebsites(websites);
        assertSame(websites, mRawContact.getWebsites());
    }

    public void test17_getSourceLocation() {
        mRawContact.setSourceLocation(1);
        assertEquals(1, mRawContact.getSourceLocation());
    }

    public void test18_getSimId() {
        mRawContact.setSimId(2);
        assertEquals(2, mRawContact.getSimId());
    }

    public void test19_getSimIndex() {
        mRawContact.setSimIndex(0);
        assertEquals(0, mRawContact.getSimIndex());
    }

    public void test20_clone() {
        try {
            mRawContact.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test21_shallowClone() {
        try {
            mRawContact.shallowClone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test22_getAllContactData() {
        assertNotNull(mRawContact.getAllContactData());
    }

    public void test23_getContactDataCount() {
        assertNotNull(mRawContact.getContactDataCount());
    }

    public void test24_addContactData() {
        ContactData data1 = new StructuredName();
        ContactData data2 = new Phone();
        ContactData data3 = new Photo();
        ContactData data4 = new Im();
        ContactData data5 = new Email();
        ContactData data6 = new StructuredPostal();
        ContactData data7 = new Organization();
        ContactData data8 = new Nickname();
        ContactData data9 = new Website();
        ContactData data10 = new Note();
        ContactData data11 = new GroupMembership();
        try {
            mRawContact.addContactData(data1);
            mRawContact.addContactData(data2);
            mRawContact.addContactData(data3);
            mRawContact.addContactData(data4);
            mRawContact.addContactData(data5);
            mRawContact.addContactData(data6);
            mRawContact.addContactData(data7);
            mRawContact.addContactData(data8);
            mRawContact.addContactData(data9);
            mRawContact.addContactData(data10);
            mRawContact.addContactData(data11);
        } catch (UnknownContactDataTypeException e) {
            fail(e.getMessage());
        }
    }

    public void test24_clearAllData() {
        mRawContact.clearAllData();
        assertEquals(0, mRawContact.getNames().size());
        assertEquals(0, mRawContact.getPhones().size());
        assertEquals(0, mRawContact.getPhotos().size());
        assertEquals(0, mRawContact.getIms().size());
        assertEquals(0, mRawContact.getEmails().size());
        assertEquals(0, mRawContact.getPostals().size());
        assertEquals(0, mRawContact.getOrganizations().size());
        assertEquals(0, mRawContact.getNicknames().size());
        assertEquals(0, mRawContact.getWebsites().size());
        assertEquals(0, mRawContact.getNotes().size());
        assertEquals(0, mRawContact.getGroupMemberships().size());
    }

    public void test25_writeRaw() {
        RawContact rawContact = new RawContact();
        ByteBuffer buffer = ByteBuffer.allocate(1000);
        mRawContact.setContactId(100);
        mRawContact.setLastTimeContacted(557668675);
        mRawContact.setStarred(true);
        mRawContact.setCustomRingtone("custom ringtone");
        mRawContact.setSendToVoicemail(false);
        mRawContact.setVersion(2);
        mRawContact.setDirty(true);
        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        mRawContact.setNames(names);
        ArrayList<Phone> phones = new ArrayList<Phone>();
        mRawContact.setPhones(phones);
        ArrayList<Photo> photos = new ArrayList<Photo>();
        mRawContact.setPhotos(photos);
        ArrayList<Email> emails = new ArrayList<Email>();
        mRawContact.setEmails(emails);
        ArrayList<Im> ims = new ArrayList<Im>();
        mRawContact.setIms(ims);
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        mRawContact.setPostals(postals);
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        mRawContact.setOrganizations(organizations);
        ArrayList<Note> notes = new ArrayList<Note>();
        mRawContact.setNotes(notes);
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        mRawContact.setNicknames(nicknames);
        ArrayList<Website> websites = new ArrayList<Website>();
        mRawContact.setWebsites(websites);
        mRawContact.setSourceLocation(1);
        mRawContact.setSimId(2);
        mRawContact.setSimIndex(0);
        mRawContact.writeRaw(buffer);
        buffer.position(0);
        rawContact.readRaw(buffer);
    }

    public void test26_writeRawWithVersion() {
        RawContact rawContact = new RawContact();
        ByteBuffer buffer = ByteBuffer.allocate(1000);
        mRawContact.setContactId(100);
        mRawContact.setLastTimeContacted(557668675);
        mRawContact.setStarred(true);
        mRawContact.setCustomRingtone("custom ringtone");
        mRawContact.setSendToVoicemail(false);
        mRawContact.setVersion(2);
        mRawContact.setDirty(true);
        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        mRawContact.setNames(names);
        ArrayList<Phone> phones = new ArrayList<Phone>();
        mRawContact.setPhones(phones);
        ArrayList<Photo> photos = new ArrayList<Photo>();
        mRawContact.setPhotos(photos);
        ArrayList<Email> emails = new ArrayList<Email>();
        mRawContact.setEmails(emails);
        ArrayList<Im> ims = new ArrayList<Im>();
        mRawContact.setIms(ims);
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        mRawContact.setPostals(postals);
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        mRawContact.setOrganizations(organizations);
        ArrayList<Note> notes = new ArrayList<Note>();
        mRawContact.setNotes(notes);
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        mRawContact.setNicknames(nicknames);
        ArrayList<Website> websites = new ArrayList<Website>();
        mRawContact.setWebsites(websites);
        mRawContact.setSourceLocation(1);
        mRawContact.setSimId(2);
        mRawContact.setSimIndex(0);
        mRawContact.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        rawContact.readRawWithVersion(buffer, Config.VERSION_CODE);
    }

    /**
     * The inner class UnknownContactDataTypeException.
     */
    public void test27_toString() {
        UnknownContactDataTypeException dataTypeException = mRawContact.new UnknownContactDataTypeException(
                "Class name", 3);
        assertEquals("Unknown type of contact data, its class name is "
                + "Class name" + ", MIME type is " + 3, dataTypeException
                .toString());
    }
}
