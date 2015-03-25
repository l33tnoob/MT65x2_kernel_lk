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
import android.content.OperationApplicationException;
import android.os.RemoteException;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.ObservedContentResolver;
import com.mediatek.apst.target.data.proxy.contacts.ContactsOperationBatch;
import com.mediatek.apst.target.data.proxy.contacts.ContactsProxy;
import com.mediatek.apst.target.data.proxy.contacts.USIMUtils;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.util.entity.contacts.BaseContact;
import com.mediatek.apst.util.entity.contacts.ContactData;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.Group;
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

import java.util.ArrayList;

public class ContactsOperationBatchTest extends AndroidTestCase {
    private ContactsOperationBatch mBatch;
    private Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        mBatch = new ContactsOperationBatch(new ObservedContentResolver(
                mContext.getContentResolver()));
    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    public void test01_apply() {
        try {
            assertNotNull(mBatch.apply());
        } catch (RemoteException e) {
            fail(e.getMessage());
        } catch (OperationApplicationException e) {
            fail(e.getMessage());
        }
    }

    public void test02_appendContactDataInsert() {
        ContactData data = null;
        // data is null.
        assertFalse(mBatch.appendContactDataInsert(data));
        // data is StructureName.
        StructuredName name = new StructuredName();
        name.setMimeType(StructuredName.MIME_TYPE);
        name.setPrimary(false);
        name.setDisplayName("Jim Green");
        name.setFamilyName("Green");
        name.setGivenName("Jim");
        name.setSuperPrimary(false);
        name.setMiddleName("middleName");
        name.setPhoneticFamilyName("phoneticFamilyName");
        name.setPhoneticGivenName("phoneticGivenName");
        name.setPrefix("prefix");
        name.setSuffix("suffix");
        assertTrue(mBatch.appendContactDataInsert(name));
        // data is Phone.
        Phone phone = new Phone();
        phone.setLabel("home");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setNumber("121464646");
        phone.setType(Phone.TYPE_HOME);
        assertTrue(mBatch.appendContactDataInsert(phone));
        // data is Im.
        Im im = new Im();
        im.setData("464614616");
        im.setLabel("QQ");
        im.setMimeType(Im.MIME_TYPE);
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_NONE);
        assertTrue(mBatch.appendContactDataInsert(im));
        // data is Email.
        Email email = new Email();
        email.setData("test@qq.com");
        email.setLabel("my Email");
        email.setMimeType(Email.MIME_TYPE);
        email.setType(Email.TYPE_WORK);
        assertTrue(mBatch.appendContactDataInsert(email));
        // data is StructuredPostal.
        StructuredPostal postal = new StructuredPostal();
        postal.setCity("chengdu");
        postal.setCountry("china");
        postal.setFormattedAddress("sichuan chengdu");
        postal.setLabel("my postal");
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setPostcode("1456465");
        postal.setType(StructuredPostal.TYPE_HOME);
        assertTrue(mBatch.appendContactDataInsert(postal));
        // data is Organization.
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
        assertTrue(mBatch.appendContactDataInsert(organization));
        // data is nickName.
        Nickname nickName = new Nickname();
        nickName.setLabel("my nickName");
        nickName.setMimeType(Nickname.MIME_TYPE);
        nickName.setName("name");
        nickName.setType(Nickname.TYPE_INITIALS);
        assertTrue(mBatch.appendContactDataInsert(nickName));
        // data is Website.
        Website website = new Website();
        website.setLabel("my website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.baidu.com");
        assertTrue(mBatch.appendContactDataInsert(website));
        // data is Note.
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("a note");
        assertTrue(mBatch.appendContactDataInsert(note));
        // data is GroupMembership.
        GroupMembership membership = new GroupMembership();
        membership.setMimeType(GroupMembership.MIME_TYPE);
        membership.setGroupId(166);
        assertTrue(mBatch.appendContactDataInsert(membership));
    }

    public void test03_appendContactDataUpdate() {
        long oldId = ContactsProxy.getInstance(mContext).getMaxRawContactsId() - 1;
        long id = ContactsProxy.getInstance(mContext).getMaxRawContactsId();
        // data is null.
        ContactData newData = null;
        assertFalse(mBatch.appendContactDataUpdate(oldId, newData));

        // data is StructureName.
        StructuredName name = new StructuredName();
        name.setMimeType(StructuredName.MIME_TYPE);
        name.setPrimary(false);
        name.setDisplayName("Jim Green");
        name.setFamilyName("Green");
        name.setGivenName("Jim");
        name.setSuperPrimary(false);
        name.setMiddleName("middleName");
        name.setPhoneticFamilyName("phoneticFamilyName");
        name.setPhoneticGivenName("phoneticGivenName");
        name.setPrefix("prefix");
        name.setSuffix("suffix");
        name.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, name));
        // data is Phone.
        Phone phone = new Phone();
        phone.setLabel("home");
        phone.setMimeType(Phone.MIME_TYPE);
        phone.setNumber("121464646");
        phone.setType(Phone.TYPE_HOME);
        phone.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, phone));
        // data is Im.
        Im im = new Im();
        im.setData("464614616");
        im.setLabel("QQ");
        im.setMimeType(Im.MIME_TYPE);
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_NONE);
        im.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, im));
        // data is Email.
        Email email = new Email();
        email.setData("test@qq.com");
        email.setLabel("my Email");
        email.setMimeType(Email.MIME_TYPE);
        email.setType(Email.TYPE_WORK);
        email.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, email));
        // data is StructuredPostal.
        StructuredPostal postal = new StructuredPostal();
        postal.setCity("chengdu");
        postal.setCountry("china");
        postal.setFormattedAddress("sichuan chengdu");
        postal.setLabel("my postal");
        postal.setMimeType(StructuredPostal.MIME_TYPE);
        postal.setPostcode("1456465");
        postal.setType(StructuredPostal.TYPE_HOME);
        postal.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, postal));
        // data is Organization.
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
        organization.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, organization));
        // data is nickName.
        Nickname nickName = new Nickname();
        nickName.setLabel("my nickName");
        nickName.setMimeType(Nickname.MIME_TYPE);
        nickName.setName("name");
        nickName.setType(Nickname.TYPE_INITIALS);
        nickName.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, nickName));
        // data is Website.
        Website website = new Website();
        website.setLabel("my website");
        website.setMimeType(Website.MIME_TYPE);
        website.setType(Website.TYPE_HOME);
        website.setUrl("www.baidu.com");
        website.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, website));
        // data is Note.
        Note note = new Note();
        note.setMimeType(Note.MIME_TYPE);
        note.setNote("a note");
        note.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, note));
        // data is GroupMembership.
        GroupMembership membership = new GroupMembership();
        membership.setMimeType(GroupMembership.MIME_TYPE);
        membership.setGroupId(166);
        membership.setRawContactId(id);
        assertTrue(mBatch.appendContactDataUpdate(oldId, membership));
    }

    public void test04_appendGroupInsert() {
        Group group = new Group();
        group.setAccount_name("usim" + USIMUtils.ACCOUNT_NAME_USIM);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_USIM);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("true");
        group.setTitle("classmates");
        ArrayList<BaseContact> members = new ArrayList<BaseContact>();
        BaseContact bContact1 = new BaseContact();
        bContact1.setDisplayName("displayName");
        bContact1.setModifyTime(146541654);
        bContact1.setPrimaryNumber("15858474521");
        bContact1.setSimName("sim1");
        members.add(bContact1);
        group.setMembers(members);
        mBatch.appendGroupInsert(group);
    }

    public void test05_appendRawContactInsert() {
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
        phone.setNumber("13965251425");
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
        email.setData("test@112.com");
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
        rawContact.setSimName("test");
        ArrayList<GroupMembership> groupMemberships = new ArrayList<GroupMembership>();
        GroupMembership groupMembership = new GroupMembership();
        groupMembership.setGroupId(0);
        groupMemberships.add(groupMembership);
        rawContact.setGroupMemberships(groupMemberships);
        mBatch.appendRawContactInsert(rawContact);
    }

}
