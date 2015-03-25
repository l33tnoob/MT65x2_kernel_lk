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

import com.mediatek.apst.target.data.proxy.contacts.USIMUtils;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.Group;
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

public class ContactsUtils {

    public static RawContact getRawContact() {
        RawContact rawContact = new RawContact();
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
        Photo photo = new Photo();
        photo.setPrimary(false);
        photos.add(photo);
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
        return rawContact;
    }

    public static Group getGroup() {
        Group group = new Group();
        group.setAccount_name(USIMUtils.ACCOUNT_NAME_LOCAL_PHONE);
        group.setAccount_type(USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE);
        group.setVersion(Config.VERSION_STRING);
        group.setDeleted("false");
        group.setDirty("false");
        group.setGroup_visible("true");
        group.setNotes("notes");
        group.setShould_sync("true");
        group.setTitle("classmates");
        return group;
    }

}
