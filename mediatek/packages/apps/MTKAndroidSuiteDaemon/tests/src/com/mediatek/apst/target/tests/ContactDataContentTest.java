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

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContactsEntity;
import android.test.AndroidTestCase;
import android.util.Log;

import com.mediatek.apst.target.data.provider.contacts.ContactDataContent;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.util.Global;
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

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class ContactDataContentTest extends AndroidTestCase {
    private static final String TAG = "ContactDataContentTest";
    private Context mContext;
    private static final String AUTHORITY = "com.android.contacts";
    /** A content:// style uri to the authority for the contacts provider */
    private static final Uri AUTHORITY_URI = Uri
            .parse("content://" + AUTHORITY);
    private Uri CONTENT_URI = Uri.withAppendedPath(AUTHORITY_URI,
            "raw_contacts");

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test02_cursorToRaw() {
        ContactsProxy.getInstance(getContext()).insertContact(
                ContactsUtils.getRawContact(), true);
        Cursor cursor = mContext.getContentResolver().query(
                RawContactsEntity.CONTENT_URI,
                new String[] { RawContactsEntity.DATA_ID,
                        RawContactsEntity.MIMETYPE, RawContactsEntity.DATA1,
                        RawContactsEntity.DATA2, RawContactsEntity.DATA3,
                        RawContactsEntity.DATA4, RawContactsEntity.DATA5,
                        RawContactsEntity.DATA6, RawContactsEntity.DATA7,
                        RawContactsEntity.DATA8, RawContactsEntity.DATA9,
                        RawContactsEntity.DATA10, RawContactsEntity.DATA15,
                        ContactDataContent.COLUMN_BINDING_SIM_ID }, null, null,
                null);
        ByteBuffer buffer = Global.getByteBuffer();
        Log.i(TAG, "cursor position: " + cursor.getPosition());
        Log.i(TAG, "cursor count: " + cursor.getCount());
        cursor.moveToFirst();
        Log.i(TAG, "cursor position: " + cursor.getPosition());
        assertNotNull(ContactDataContent.cursorToRaw(cursor, buffer));
        cursor.close();
        Cursor cursor2 = mContext.getContentResolver().query(CONTENT_URI, null,
                null, null, null);
        cursor2.moveToFirst();
        assertNotNull(ContactDataContent.cursorToRaw(cursor2, buffer));
        cursor2.close();

        Cursor cursor_data = mContext.getContentResolver().query(
                Data.CONTENT_URI,
                new String[] { Data._ID, Data.RAW_CONTACT_ID, Data.MIMETYPE,
                        Data.DATA1, Data.DATA2, Data.DATA3, Data.DATA4,
                        Data.DATA5, Data.DATA6, Data.DATA7, Data.DATA8,
                        Data.DATA9, Data.DATA10, Data.DATA15,
                        ContactDataContent.COLUMN_BINDING_SIM_ID },
                Data.MIMETYPE + "<>'"
                        + CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE
                        + "'", null, Data.RAW_CONTACT_ID + " ASC");
        buffer.clear();
        while (cursor_data.moveToNext()) {
            assertNotNull(ContactDataContent.cursorToRaw(cursor_data, buffer));
        }
        cursor_data.close();
    }

    /**
     * Contact data is structure name.
     */
    public void test03_createContentValues() {
        StructuredName data = new StructuredName();
        data.setMimeType(StructuredName.MIME_TYPE);
        data.setPrimary(false);
        data.setDisplayName("Jim Green");
        data.setFamilyName("Green");
        data.setGivenName("Jim");
        data.setSuperPrimary(false);
        data.setMiddleName("middleName");
        data.setPhoneticFamilyName("phoneticFamilyName");
        data.setPhoneticGivenName("phoneticGivenName");
        data.setPrefix("prefix");
        data.setSuffix("suffix");
        assertNotNull(ContactDataContent.createContentValues(data, true));
    }

    /**
     * Contact data is Phone.
     */
    public void test04_createContentValues() {
        Phone phone = new Phone();
        phone.setLabel("home");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setNumber("121464646");
        phone.setType(Phone.TYPE_HOME);
        assertNotNull(ContactDataContent.createContentValues(phone, true));
    }

    /**
     * Contact data is im.
     */
    public void test05_createContentValues() {
        Im im = new Im();
        im.setData("464614616");
        im.setLabel("QQ");
        im.setMimeType(Im.MIME_TYPE);
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_NONE);
        assertNotNull(ContactDataContent.createContentValues(im, true));
    }

    /**
     * Contact data is email.
     */
    public void test07_createContentValues() {
        Email email = new Email();
        email.setData("test@qq.com");
        email.setLabel("my Email");
        email.setMimeType(Email.MIME_TYPE);
        email.setType(Email.TYPE_WORK);
        assertNotNull(ContactDataContent.createContentValues(email, true));
    }

    /**
     * Contact data is structured postal.
     */
    public void test08_createContentValues() {
        StructuredPostal postal = new StructuredPostal();
        postal.setCity("chengdu");
        postal.setCountry("china");
        postal.setFormattedAddress("sichuan chengdu");
        postal.setLabel("my postal");
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setPostcode("1456465");
        postal.setType(StructuredPostal.TYPE_HOME);
        assertNotNull(ContactDataContent.createContentValues(postal, true));
    }

    /**
     * Contact data is organization.
     */
    public void test09_createContentValues() {
        Organization organization = new Organization();
        organization.setCompany("company");
        organization.setDepartment("department");
        organization.setJobDescription("jobDescription");
        organization.setLabel("label");
        organization.setMimeType(Organization.MIME_TYPE);
        organization.setOfficeLocation("officeLocation");
        organization.setPhoneticName("phoneticName");
        organization.setPhoneticNameStyle("phoneticNameStyle");
        organization.setTitle("my organization");
        organization.setType(Organization.TYPE_WORK);
        assertNotNull(ContactDataContent
                .createContentValues(organization, true));
    }

    /**
     * Contact data is nickname.
     */
    public void test10_createContentValues() {
        Nickname nickName = new Nickname();
        nickName.setLabel("my nickName");
        nickName.setMimeType(Nickname.MIME_TYPE);
        nickName.setName("name");
        nickName.setType(Nickname.TYPE_INITIALS);
        assertNotNull(ContactDataContent.createContentValues(nickName, true));
    }

    /**
     * Contact data is website.
     */
    public void test11_createContentValues() {
        Website website = new Website();
        website.setLabel("my website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.baidu.com");
        assertNotNull(ContactDataContent.createContentValues(website, true));
    }

    /**
     * Contact data is note.
     */
    public void test12_createContentValues() {
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("a note");
        assertNotNull(ContactDataContent.createContentValues(note, true));
    }

    /**
     * Contact data is groupmembership.
     */
    public void test13_createContentValues() {
        GroupMembership membership = new GroupMembership();
        membership.setMimeType(GroupMembership.MIME_TYPE);
        membership.setGroupId(166);
        assertNotNull(ContactDataContent.createContentValues(membership, true));
    }

    public void test14_createContentValuesArray() {
        RawContact rawContact = new RawContact();
        // newContact.setContactId(100);
        rawContact.setLastTimeContacted(557668675);
        rawContact.setStarred(true);
        rawContact.setCustomRingtone("custom ringtone");
        rawContact.setSendToVoicemail(false);
        rawContact.setVersion(2);
        rawContact.setDirty(true);
        ArrayList<StructuredName> names = new ArrayList<StructuredName>();
        StructuredName name = new StructuredName();
        name.setMimeType(StructuredName.MIME_TYPE);
        name.setDisplayName("displayName");
        name.setFamilyName("familyName");
        name.setGivenName("givenName");
        names.add(name);
        rawContact.setNames(names);
        ArrayList<Phone> phones = new ArrayList<Phone>();
        Phone phone = new Phone();
        phone.setNumber("15898565847");
        phone.setLabel("a phone num");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setType(Phone.TYPE_HOME);
        phones.add(phone);
        rawContact.setPhones(phones);
        ArrayList<Photo> photos = new ArrayList<Photo>();
        rawContact.setPhotos(photos);
        ArrayList<Email> emails = new ArrayList<Email>();
        Email email = new Email();
        email.setMimeType(Email.MIME_TYPE);
        email.setData("there is a meeting");
        email.setLabel("meeting");
        email.setType(Email.TYPE_WORK);
        emails.add(email);
        rawContact.setEmails(emails);
        ArrayList<Im> ims = new ArrayList<Im>();
        Im im = new Im();
        im.setMimeType(Im.MIME_TYPE);
        im.setData("5459655616");
        im.setLabel("QQ");
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_HOME);
        im.setCustomProtocol(Im.MIME_TYPE_STRING);
        ims.add(im);
        rawContact.setIms(ims);
        ArrayList<StructuredPostal> postals = new ArrayList<StructuredPostal>();
        StructuredPostal postal = new StructuredPostal();
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setCity("chengdu");
        postal.setCountry("China");
        postal.setLabel("Home");
        postal.setFormattedAddress(StructuredPostal.MIME_TYPE_STRING);
        postal.setNeighborhood("neighborhood");
        postal.setPostcode("11655");
        postal.setRegion("sichuang");
        postal.setStreet("xinhua street");
        postal.setType(StructuredPostal.TYPE_WORK);
        postals.add(postal);
        rawContact.setPostals(postals);
        ArrayList<Organization> organizations = new ArrayList<Organization>();
        rawContact.setOrganizations(organizations);
        ArrayList<Note> notes = new ArrayList<Note>();
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("it's a note");
        notes.add(note);
        rawContact.setNotes(notes);
        ArrayList<Nickname> nicknames = new ArrayList<Nickname>();
        Nickname nickname = new Nickname();
        nickname.setLabel("a nickname");
        nickname.setMimeType(Nickname.MIME_TYPE);
        nickname.setName("nickname");
        nickname.setType(Nickname.TYPE_SHORT_NAME);
        nicknames.add(nickname);
        rawContact.setNicknames(nicknames);
        ArrayList<Website> websites = new ArrayList<Website>();
        Website website = new Website();
        website.setLabel("a website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_WORK);
        website.setUrl("www.mediatek.com");
        websites.add(website);
        rawContact.setWebsites(websites);
        rawContact.setSourceLocation(RawContact.SOURCE_PHONE);
        assertNotNull(ContactDataContent.createContentValuesArray(rawContact,
                true));
    }

    public void test15_createMeasuredContentValues() {
        StructuredName data = new StructuredName();
        data.setMimeType(StructuredName.MIME_TYPE);
        data.setPrimary(false);
        data.setDisplayName("Jim Green");
        data.setFamilyName("Green");
        data.setGivenName("Jim");
        data.setSuperPrimary(false);
        data.setMiddleName("middleName");
        data.setPhoneticFamilyName("phoneticFamilyName");
        data.setPhoneticGivenName("phoneticGivenName");
        data.setPrefix("prefix");
        data.setSuffix("suffix");
        assertNotNull(ContactDataContent
                .createMeasuredContentValues(data, true));
    }

    public void test17_createMeasuredContentValues() {
        Phone phone = new Phone();
        phone.setLabel("home");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setNumber("121464646");
        phone.setType(Phone.TYPE_HOME);
        assertNotNull(ContactDataContent.createMeasuredContentValues(phone,
                true));
    }

    public void test18_createMeasuredContentValues() {
        Im im = new Im();
        im.setData("464614616");
        im.setLabel("QQ");
        im.setMimeType(Im.MIME_TYPE);
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_NONE);
        assertNotNull(ContactDataContent.createMeasuredContentValues(im, true));
    }

    public void test19_createMeasuredContentValues() {
        Email email = new Email();
        email.setData("test@qq.com");
        email.setLabel("my Email");
        email.setMimeType(Email.MIME_TYPE);
        email.setType(Email.TYPE_WORK);
        assertNotNull(ContactDataContent.createMeasuredContentValues(email,
                true));
    }

    public void test20_createMeasuredContentValues() {
        StructuredPostal postal = new StructuredPostal();
        postal.setCity("chengdu");
        postal.setCountry("china");
        postal.setFormattedAddress("sichuan chengdu");
        postal.setLabel("my postal");
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setPostcode("1456465");
        postal.setType(StructuredPostal.TYPE_HOME);
        assertNotNull(ContactDataContent.createMeasuredContentValues(postal,
                true));
    }

    public void test21_createMeasuredContentValues() {
        Organization organization = new Organization();
        organization.setCompany("company");
        organization.setDepartment("department");
        organization.setJobDescription("jobDescription");
        organization.setLabel("label");
        organization.setMimeType(Organization.MIME_TYPE);
        organization.setOfficeLocation("officeLocation");
        organization.setPhoneticName("phoneticName");
        organization.setPhoneticNameStyle("phoneticNameStyle");
        organization.setTitle("my organization");
        organization.setType(Organization.TYPE_WORK);
        assertNotNull(ContactDataContent
                .createContentValues(organization, true));
    }

    public void test22_createMeasuredContentValues() {
        Nickname nickName = new Nickname();
        nickName.setLabel("my nickName");
        nickName.setMimeType(Nickname.MIME_TYPE);
        nickName.setName("name");
        nickName.setType(Nickname.TYPE_INITIALS);
        assertNotNull(ContactDataContent.createMeasuredContentValues(nickName,
                true));
    }

    public void test23_createMeasuredContentValues() {
        Website website = new Website();
        website.setLabel("my website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.baidu.com");
        assertNotNull(ContactDataContent.createMeasuredContentValues(website,
                true));
    }

    public void test24_createMeasuredContentValues() {
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("a note");
        assertNotNull(ContactDataContent
                .createMeasuredContentValues(note, true));
    }

    public void test25_createMeasuredContentValues() {
        GroupMembership membership = new GroupMembership();
        membership.setMimeType(GroupMembership.MIME_TYPE);
        membership.setGroupId(166);
        assertNotNull(ContactDataContent.createMeasuredContentValues(
                membership, true));
    }
}