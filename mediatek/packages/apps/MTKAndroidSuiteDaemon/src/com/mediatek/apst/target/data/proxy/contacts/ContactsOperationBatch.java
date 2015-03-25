/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.apst.target.data.proxy.contacts;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.OperationApplicationException;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;

import com.mediatek.android.content.ContentProviderOperationBatch;
import com.mediatek.apst.target.data.provider.contacts.ContactDataContent;
import com.mediatek.apst.target.data.proxy.ObservedContentResolver;
import com.mediatek.apst.target.util.Debugger;
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

/**
 * Class Name: ContactsOperationBatch
 * <p>
 * Package: com.mediatek.apst.target.proxy.contacts
 * <p>
 * Created on: 2010-8-6
 * <p>
 * <p>
 * Description:
 * <p>
 * Used to build and apply contacts related content operation batch.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public class ContactsOperationBatch extends ContentProviderOperationBatch {

    /**
     * @param ocr
     *            The content resolver.
     */
    public ContactsOperationBatch(ObservedContentResolver ocr) {
        super(ocr);
    }

    /**
     * @return A array contains the result of the application of a
     *         ContentProviderOperation. It is guaranteed to have exactly one of
     *         uri or count set.
     * @throws RemoteException
     *             thrown if a RemoteException is encountered while attempting
     *             to communicate with a remote provider.
     * @throws OperationApplicationException
     *             thrown if an application fails.
     */
    public ContentProviderResult[] apply() throws RemoteException,
            OperationApplicationException {
        return super.apply(ContactsContract.AUTHORITY);
    }

    /**
     * @param data
     *            The contact data.
     * @return Whether success to insert the contact data.
     */
    public boolean appendContactDataInsert(ContactData data) {
        if (data == null) {
            return false;
        }

        boolean b = false;
        long cId = data.getRawContactId();
        int mimeType = data.getMimeType();
        if (data instanceof StructuredName) {
            // Name --------------------------------------------------------
            StructuredName name = (StructuredName) data;
            b = append(ContentProviderOperation
                    .newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId)
                    .withValue(Data.MIMETYPE,
                            CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
                    /*
                     * .withValue(CommonDataKinds.StructuredName.DISPLAY_NAME,
                     * name.getDisplayName())
                     */
                    .withValue(CommonDataKinds.StructuredName.GIVEN_NAME,
                            name.getGivenName())
                    .withValue(CommonDataKinds.StructuredName.FAMILY_NAME,
                            name.getFamilyName())
                    .withValue(CommonDataKinds.StructuredName.MIDDLE_NAME,
                            name.getMiddleName())
                     
                    .withValue(CommonDataKinds.StructuredName.PREFIX,
                            name.getPrefix())
                    .withValue(CommonDataKinds.StructuredName.SUFFIX,
                            name.getSuffix())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_FAMILY_NAME,
                            name.getPhoneticFamilyName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_GIVEN_NAME,
                            name.getPhoneticGivenName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_MIDDLE_NAME,
                            name.getPhoneticMiddleName()).build());
        } else if (data instanceof Phone) {
            // Phone -------------------------------------------------------
            Phone phone = (Phone) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Phone.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Phone.NUMBER, phone.getNumber())
                    .withValue(CommonDataKinds.Phone.TYPE, phone.getType())
                    .withValue(CommonDataKinds.Phone.LABEL, phone.getLabel())
                    // Added by Shaoying Han
                    .withValue(ContactDataContent.COLUMN_BINDING_SIM_ID,
                            phone.getBindingSimId()).build());
        } else if (data instanceof Photo) {
            // Photo -------------------------------------------------------
            Photo photo = (Photo) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Photo.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Photo.PHOTO, photo.getPhotoBytes())
                    .build());
        } else if (data instanceof Im) {
            // IM ----------------------------------------------------------
            Im im = (Im) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId)
                    .withValue(Data.MIMETYPE,
                            CommonDataKinds.Im.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Im.DATA, im.getData()).withValue(
                            CommonDataKinds.Im.TYPE, im.getType()).withValue(
                            CommonDataKinds.Im.LABEL, im.getLabel()).withValue(
                            CommonDataKinds.Im.PROTOCOL, im.getProtocol())
                    .withValue(CommonDataKinds.Im.CUSTOM_PROTOCOL,
                            im.getCustomProtocol()).build());
        } else if (data instanceof Email) {
            // Email -------------------------------------------------------
            Email email = (Email) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Email.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Email.DATA, email.getData())
                    .withValue(CommonDataKinds.Email.TYPE, email.getType())
                    .withValue(CommonDataKinds.Email.LABEL, email.getLabel())
                    .build());
        } else if (data instanceof StructuredPostal) {
            // Postal ------------------------------------------------------
            StructuredPostal postal = (StructuredPostal) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE)
                    /*
                     * .withValue(CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS
                     * , postal.getFormattedAddress())
                     */
                    .withValue(CommonDataKinds.StructuredPostal.TYPE,
                            postal.getType()).withValue(
                            CommonDataKinds.StructuredPostal.LABEL,
                            postal.getLabel()).withValue(
                            CommonDataKinds.StructuredPostal.STREET,
                            postal.getStreet()).withValue(
                            CommonDataKinds.StructuredPostal.POBOX,
                            postal.getPobox()).withValue(
                            CommonDataKinds.StructuredPostal.NEIGHBORHOOD,
                            postal.getNeighborhood()).withValue(
                            CommonDataKinds.StructuredPostal.CITY,
                            postal.getCity()).withValue(
                            CommonDataKinds.StructuredPostal.REGION,
                            postal.getRegion()).withValue(
                            CommonDataKinds.StructuredPostal.POSTCODE,
                            postal.getPostcode()).withValue(
                            CommonDataKinds.StructuredPostal.COUNTRY,
                            postal.getCountry()).build());
        } else if (data instanceof Organization) {
            // Organization ------------------------------------------------
            Organization organization = (Organization) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Organization.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Organization.COMPANY,
                            organization.getCompany()).withValue(
                            CommonDataKinds.Organization.TYPE,
                            organization.getType()).withValue(
                            CommonDataKinds.Organization.LABEL,
                            organization.getLabel()).withValue(
                            CommonDataKinds.Organization.TITLE,
                            organization.getTitle()).withValue(
                            CommonDataKinds.Organization.DEPARTMENT,
                            organization.getDepartment()).withValue(
                            CommonDataKinds.Organization.JOB_DESCRIPTION,
                            organization.getJobDescription()).withValue(
                            CommonDataKinds.Organization.SYMBOL,
                            organization.getSymbol()).withValue(
                            CommonDataKinds.Organization.PHONETIC_NAME,
                            organization.getPhoneticName()).withValue(
                            CommonDataKinds.Organization.OFFICE_LOCATION,
                            organization.getOfficeLocation()).build());
        } else if (data instanceof Nickname) {
            // Nickname ----------------------------------------------------
            Nickname nickname = (Nickname) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Nickname.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Nickname.NAME,
                            nickname.getName()).withValue(
                            CommonDataKinds.Nickname.TYPE, nickname.getType())
                    .withValue(CommonDataKinds.Nickname.LABEL,
                            nickname.getLabel()).build());
        } else if (data instanceof Website) {
            // Website -----------------------------------------------------
            Website website = (Website) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Website.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Website.URL, website.getUrl())
                    .withValue(CommonDataKinds.Website.TYPE, website.getType())
                    .withValue(CommonDataKinds.Website.LABEL,
                            website.getLabel()).build());
        } else if (data instanceof Note) {
            // Note --------------------------------------------------------
            Note note = (Note) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Note.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Note.NOTE, note.getNote()).build());
        } else if (data instanceof GroupMembership) {
            // Group membership --------------------------------------------
            GroupMembership groupMembership = (GroupMembership) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.GroupMembership.GROUP_ROW_ID,
                            groupMembership.getGroupId()).build());
        } else {
            Debugger.logE(new Object[] { data }, "Illegal mime type: "
                    + mimeType);
            return false;
        }

        return b;
    }
    
    /**
     * @param data
     *            The contact data. Add this function just by contact provider's change.
     * @return Whether success to insert the contact data.
     */
    public boolean appendSimContactDataInsert(ContactData data) {
        if (data == null) {
            return false;
        }

        boolean b = false;
        long cId = data.getRawContactId();
        int mimeType = data.getMimeType();
        if (data instanceof StructuredName) {
            // Name --------------------------------------------------------
            StructuredName name = (StructuredName) data;
            b = append(ContentProviderOperation
                    .newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId)
                    .withValue(Data.MIMETYPE,
                            CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
                    
                    .withValue(CommonDataKinds.StructuredName.DISPLAY_NAME,
                      name.getDisplayName())               
                    .withValue(CommonDataKinds.StructuredName.GIVEN_NAME,
                            null)
                    .withValue(CommonDataKinds.StructuredName.FAMILY_NAME,
                            null)
                    .withValue(CommonDataKinds.StructuredName.MIDDLE_NAME,
                            null)
                    .withValue(CommonDataKinds.StructuredName.PREFIX,
                            null)
                    .withValue(CommonDataKinds.StructuredName.SUFFIX,
                            null)
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_FAMILY_NAME,
                            name.getPhoneticFamilyName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_GIVEN_NAME,
                            name.getPhoneticGivenName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_MIDDLE_NAME,
                            name.getPhoneticMiddleName()).build());
        } else if (data instanceof Phone) {
            // Phone -------------------------------------------------------
            Phone phone = (Phone) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Phone.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Phone.NUMBER, phone.getNumber())
                    .withValue(CommonDataKinds.Phone.TYPE, phone.getType())
                    .withValue(CommonDataKinds.Phone.LABEL, phone.getLabel())
                    // Added by Shaoying Han
                    .withValue(ContactDataContent.COLUMN_BINDING_SIM_ID,
                            phone.getBindingSimId()).build());
        } else if (data instanceof Photo) {
            // Photo -------------------------------------------------------
            Photo photo = (Photo) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Photo.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Photo.PHOTO, photo.getPhotoBytes())
                    .build());
        } else if (data instanceof Im) {
            // IM ----------------------------------------------------------
            Im im = (Im) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId)
                    .withValue(Data.MIMETYPE,
                            CommonDataKinds.Im.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Im.DATA, im.getData()).withValue(
                            CommonDataKinds.Im.TYPE, im.getType()).withValue(
                            CommonDataKinds.Im.LABEL, im.getLabel()).withValue(
                            CommonDataKinds.Im.PROTOCOL, im.getProtocol())
                    .withValue(CommonDataKinds.Im.CUSTOM_PROTOCOL,
                            im.getCustomProtocol()).build());
        } else if (data instanceof Email) {
            // Email -------------------------------------------------------
            Email email = (Email) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Email.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Email.DATA, email.getData())
                    .withValue(CommonDataKinds.Email.TYPE, email.getType())
                    .withValue(CommonDataKinds.Email.LABEL, email.getLabel())
                    .build());
        } else if (data instanceof StructuredPostal) {
            // Postal ------------------------------------------------------
            StructuredPostal postal = (StructuredPostal) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE)
                    /*
                     * .withValue(CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS
                     * , postal.getFormattedAddress())
                     */
                    .withValue(CommonDataKinds.StructuredPostal.TYPE,
                            postal.getType()).withValue(
                            CommonDataKinds.StructuredPostal.LABEL,
                            postal.getLabel()).withValue(
                            CommonDataKinds.StructuredPostal.STREET,
                            postal.getStreet()).withValue(
                            CommonDataKinds.StructuredPostal.POBOX,
                            postal.getPobox()).withValue(
                            CommonDataKinds.StructuredPostal.NEIGHBORHOOD,
                            postal.getNeighborhood()).withValue(
                            CommonDataKinds.StructuredPostal.CITY,
                            postal.getCity()).withValue(
                            CommonDataKinds.StructuredPostal.REGION,
                            postal.getRegion()).withValue(
                            CommonDataKinds.StructuredPostal.POSTCODE,
                            postal.getPostcode()).withValue(
                            CommonDataKinds.StructuredPostal.COUNTRY,
                            postal.getCountry()).build());
        } else if (data instanceof Organization) {
            // Organization ------------------------------------------------
            Organization organization = (Organization) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Organization.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Organization.COMPANY,
                            organization.getCompany()).withValue(
                            CommonDataKinds.Organization.TYPE,
                            organization.getType()).withValue(
                            CommonDataKinds.Organization.LABEL,
                            organization.getLabel()).withValue(
                            CommonDataKinds.Organization.TITLE,
                            organization.getTitle()).withValue(
                            CommonDataKinds.Organization.DEPARTMENT,
                            organization.getDepartment()).withValue(
                            CommonDataKinds.Organization.JOB_DESCRIPTION,
                            organization.getJobDescription()).withValue(
                            CommonDataKinds.Organization.SYMBOL,
                            organization.getSymbol()).withValue(
                            CommonDataKinds.Organization.PHONETIC_NAME,
                            organization.getPhoneticName()).withValue(
                            CommonDataKinds.Organization.OFFICE_LOCATION,
                            organization.getOfficeLocation()).build());
        } else if (data instanceof Nickname) {
            // Nickname ----------------------------------------------------
            Nickname nickname = (Nickname) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Nickname.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Nickname.NAME,
                            nickname.getName()).withValue(
                            CommonDataKinds.Nickname.TYPE, nickname.getType())
                    .withValue(CommonDataKinds.Nickname.LABEL,
                            nickname.getLabel()).build());
        } else if (data instanceof Website) {
            // Website -----------------------------------------------------
            Website website = (Website) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Website.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Website.URL, website.getUrl())
                    .withValue(CommonDataKinds.Website.TYPE, website.getType())
                    .withValue(CommonDataKinds.Website.LABEL,
                            website.getLabel()).build());
        } else if (data instanceof Note) {
            // Note --------------------------------------------------------
            Note note = (Note) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.Note.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Note.NOTE, note.getNote()).build());
        } else if (data instanceof GroupMembership) {
            // Group membership --------------------------------------------
            GroupMembership groupMembership = (GroupMembership) data;
            b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                    .withValue(Data.RAW_CONTACT_ID, cId).withValue(
                            Data.MIMETYPE,
                            CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.GroupMembership.GROUP_ROW_ID,
                            groupMembership.getGroupId()).build());
        } else {
            Debugger.logE(new Object[] { data }, "Illegal mime type: "
                    + mimeType);
            return false;
        }

        return b;
    }

    /**
     * @param contactId
     *            The id of the contact.
     * @param groupId
     *            The id of the group.
     * @return True upon success, false if the batch is full.
     */
    public boolean appendGroupMembershipInsert(long contactId, long groupId) {
        boolean b = append(ContentProviderOperation.newInsert(Data.CONTENT_URI)
                .withValue(Data.RAW_CONTACT_ID, contactId).withValue(
                        Data.MIMETYPE,
                        CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE)
                .withValue(CommonDataKinds.GroupMembership.GROUP_ROW_ID,
                        groupId).build());
        return b;
    }

    /**
     * @param id
     *            The id of the contact.
     * @param newData
     *            The data used to insert.
     * @return True upon success, false if the batch is full.
     */
    public boolean appendContactDataUpdate(long id, ContactData newData) {
        if (newData == null) {
            return false;
        }

        boolean b = false;
        long cId = newData.getRawContactId();
        int mimeType = newData.getMimeType();
        if (newData instanceof StructuredName) {
            // Name --------------------------------------------------------
            StructuredName name = (StructuredName) newData;
            b = append(ContentProviderOperation
                    .newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) })
                    .withValue(Data.RAW_CONTACT_ID, cId)
                    .withValue(Data.MIMETYPE,
                            CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
                    /*
                     * .withValue(CommonDataKinds.StructuredName.DISPLAY_NAME,
                     * name.getDisplayName())
                     */
                    .withValue(CommonDataKinds.StructuredName.GIVEN_NAME,
                            name.getGivenName())
                    .withValue(CommonDataKinds.StructuredName.FAMILY_NAME,
                            name.getFamilyName())
                    .withValue(CommonDataKinds.StructuredName.MIDDLE_NAME,
                            name.getMiddleName())
                    .withValue(CommonDataKinds.StructuredName.PREFIX,
                            name.getPrefix())
                    .withValue(CommonDataKinds.StructuredName.SUFFIX,
                            name.getSuffix())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_FAMILY_NAME,
                            name.getPhoneticFamilyName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_GIVEN_NAME,
                            name.getPhoneticGivenName())
                    .withValue(
                            CommonDataKinds.StructuredName.PHONETIC_MIDDLE_NAME,
                            name.getPhoneticMiddleName()).build());
        } else if (newData instanceof Phone) {
            // Phone -------------------------------------------------------
            Phone phone = (Phone) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Phone.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Phone.NUMBER, phone.getNumber())
                    .withValue(CommonDataKinds.Phone.TYPE, phone.getType())
                    .withValue(CommonDataKinds.Phone.LABEL, phone.getLabel())
                    // Added by Shaoying Han
                    .withValue(ContactDataContent.COLUMN_BINDING_SIM_ID,
                            phone.getBindingSimId()).build());
        } else if (newData instanceof Photo) {
            // Photo -------------------------------------------------------
            Photo photo = (Photo) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Photo.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Photo.PHOTO, photo.getPhotoBytes())
                    .build());
        } else if (newData instanceof Im) {
            // IM ----------------------------------------------------------
            Im im = (Im) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Im.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Im.DATA, im.getData()).withValue(
                            CommonDataKinds.Im.TYPE, im.getType()).withValue(
                            CommonDataKinds.Im.LABEL, im.getLabel()).withValue(
                            CommonDataKinds.Im.PROTOCOL, im.getProtocol())
                    .withValue(CommonDataKinds.Im.CUSTOM_PROTOCOL,
                            im.getCustomProtocol()).build());
        } else if (newData instanceof Email) {
            // Email -------------------------------------------------------
            Email email = (Email) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Email.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Email.DATA, email.getData())
                    .withValue(CommonDataKinds.Email.TYPE, email.getType())
                    .withValue(CommonDataKinds.Email.LABEL, email.getLabel())
                    .build());
        } else if (newData instanceof StructuredPostal) {
            // Postal ------------------------------------------------------
            StructuredPostal postal = (StructuredPostal) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE)
                    /*
                     * .withValue(CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS
                     * , postal.getFormattedAddress())
                     */
                    .withValue(CommonDataKinds.StructuredPostal.TYPE,
                            postal.getType()).withValue(
                            CommonDataKinds.StructuredPostal.LABEL,
                            postal.getLabel()).withValue(
                            CommonDataKinds.StructuredPostal.STREET,
                            postal.getStreet()).withValue(
                            CommonDataKinds.StructuredPostal.POBOX,
                            postal.getPobox()).withValue(
                            CommonDataKinds.StructuredPostal.NEIGHBORHOOD,
                            postal.getNeighborhood()).withValue(
                            CommonDataKinds.StructuredPostal.CITY,
                            postal.getCity()).withValue(
                            CommonDataKinds.StructuredPostal.REGION,
                            postal.getRegion()).withValue(
                            CommonDataKinds.StructuredPostal.POSTCODE,
                            postal.getPostcode()).withValue(
                            CommonDataKinds.StructuredPostal.COUNTRY,
                            postal.getCountry()).build());
        } else if (newData instanceof Organization) {
            // Organization ------------------------------------------------
            Organization organization = (Organization) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Organization.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Organization.COMPANY,
                            organization.getCompany()).withValue(
                            CommonDataKinds.Organization.TYPE,
                            organization.getType()).withValue(
                            CommonDataKinds.Organization.LABEL,
                            organization.getLabel()).withValue(
                            CommonDataKinds.Organization.TITLE,
                            organization.getTitle()).withValue(
                            CommonDataKinds.Organization.DEPARTMENT,
                            organization.getDepartment()).withValue(
                            CommonDataKinds.Organization.JOB_DESCRIPTION,
                            organization.getJobDescription()).withValue(
                            CommonDataKinds.Organization.SYMBOL,
                            organization.getSymbol()).withValue(
                            CommonDataKinds.Organization.PHONETIC_NAME,
                            organization.getPhoneticName()).withValue(
                            CommonDataKinds.Organization.OFFICE_LOCATION,
                            organization.getOfficeLocation()).build());
        } else if (newData instanceof Nickname) {
            // Nickname ----------------------------------------------------
            Nickname nickname = (Nickname) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Nickname.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Nickname.NAME,
                            nickname.getName()).withValue(
                            CommonDataKinds.Nickname.TYPE, nickname.getType())
                    .withValue(CommonDataKinds.Nickname.LABEL,
                            nickname.getLabel()).build());
        } else if (newData instanceof Website) {
            // Website -----------------------------------------------------
            Website website = (Website) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Website.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.Website.URL, website.getUrl())
                    .withValue(CommonDataKinds.Website.TYPE, website.getType())
                    .withValue(CommonDataKinds.Website.LABEL,
                            website.getLabel()).build());
        } else if (newData instanceof Note) {
            // Note --------------------------------------------------------
            Note note = (Note) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.Note.CONTENT_ITEM_TYPE).withValue(
                            CommonDataKinds.Note.NOTE, note.getNote()).build());
        } else if (newData instanceof GroupMembership) {
            // Group membership --------------------------------------------
            GroupMembership groupMembership = (GroupMembership) newData;
            b = append(ContentProviderOperation.newUpdate(Data.CONTENT_URI)
                    .withSelection(Data._ID + "=?",
                            new String[] { String.valueOf(id) }).withValue(
                            Data.RAW_CONTACT_ID, cId).withValue(Data.MIMETYPE,
                            CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE)
                    .withValue(CommonDataKinds.GroupMembership.GROUP_ROW_ID,
                            groupMembership.getGroupId()).build());
        } else {
            Debugger.logE(new Object[] { id, newData }, "Illegal mime type: "
                    + mimeType);
            return false;
        }

        return b;
    }

    /**
     * @param id
     *            The id of the contact.
     * @param permanently
     *            Whether to delete permanently.
     * @return True upon success, false if the batch is full.
     */
    public boolean appendContactDataDelete(long id, boolean permanently) {
        Uri deleteUri;
        if (permanently) {
            deleteUri = Data.CONTENT_URI
                    // Delete physically, not just set DELETED=1
                    .buildUpon().appendQueryParameter(
                            ContactsContract.CALLER_IS_SYNCADAPTER, "true")
                    .build();
        } else {
            deleteUri = Data.CONTENT_URI;
        }
        boolean b = append(ContentProviderOperation.newDelete(deleteUri)
                .withSelection(Data._ID + "=?",
                        new String[] { String.valueOf(id) }).build());

        return b;
    }

    /**
     * @param rawContact
     *            The raw contact before aggregation.
     * @return Whether success to insert the raw contact.
     */
    public boolean appendRawContactInsert(RawContact rawContact) {
        boolean b = append(ContentProviderOperation.newInsert(
                RawContacts.CONTENT_URI)
        // Disable aggregation
                .withValue(RawContacts.AGGREGATION_MODE,
                        RawContacts.AGGREGATION_MODE_DISABLED).withValue(
                        RawContacts.STARRED, rawContact.isStarred())
                // Set new ringtone
                /*
                 * .withValue(RawContacts.CUSTOM_RINGTONE,
                 * rawContact.getCustomRingtone())
                 */
                .withValue(RawContacts.SEND_TO_VOICEMAIL,
                        rawContact.isSendToVoicemail()).build());

        return b;
    }

    /**
     * @param id
     *            The raw contact id.
     * @param rawContact
     *            The raw contact used to update.
     * @return Whether success to update the raw contact.
     */
    public boolean appendRawContactUpdate(long id, RawContact rawContact) {
        boolean b = append(ContentProviderOperation.newUpdate(
                RawContacts.CONTENT_URI).withSelection(RawContacts._ID + "=?",
                new String[] { String.valueOf(id) }).withValue(
                RawContacts.STARRED, rawContact.isStarred())
        // Set new ringtone
                /*
                 * .withValue(RawContacts.CUSTOM_RINGTONE,
                 * rawContact.getCustomRingtone())
                 */
                .withValue(RawContacts.SEND_TO_VOICEMAIL,
                        rawContact.isSendToVoicemail()).build());

        return b;
    }

    /**
     * @param id
     *            The if of the raw contact.
     * @param permanently
     *            Whether delete permanently.
     * @return Whether success to delete the raw contact.
     */
    public boolean appendRawContactDelete(long id, boolean permanently) {
        Uri deleteUri;
        if (permanently) {
            deleteUri = RawContacts.CONTENT_URI
                    // Delete physically, not just set DELETED=1
                    .buildUpon().appendQueryParameter(
                            ContactsContract.CALLER_IS_SYNCADAPTER, "true")
                    .build();
        } else {
            deleteUri = RawContacts.CONTENT_URI;
        }
        boolean b = append(ContentProviderOperation.newDelete(deleteUri)
                .withSelection(RawContacts._ID + "=?",
                        new String[] { String.valueOf(id) }).build());

        return b;
    }

    /*
     * public boolean appendSimContactDelete(String name, String number){
     * boolean b = append(
     * ContentProviderOperation.newDelete(SimContactsContent.CONTENT_URI)
     * .withSelection(SimContactsContent.COLUMN_TAG + "='?'", new
     * String[]{name}) .withSelection(SimContactsContent.COLUMN_NUMBER + "='?'",
     * new String[]{number}) .build());
     * 
     * return b; }
     */
    /**
     * @param group
     *            The group to be inserted.
     * @return Whether success to insert the group.
     */
    public boolean appendGroupInsert(Group group) {
        boolean b = append(ContentProviderOperation.newInsert(
                Groups.CONTENT_URI).withValue(Groups.TITLE, group.getTitle())
                .withValue(Groups.NOTES, group.getNotes()).build());

        return b;
    }

    /**
     * @param id
     *            The row id.
     * @param group
     *            The group used to update.
     * @return Whether success to update the group.
     */
    public boolean appendGroupUpdate(long id, Group group) {
        boolean b = append(ContentProviderOperation.newUpdate(
                Groups.CONTENT_URI).withSelection(Groups._ID + "=?",
                new String[] { String.valueOf(id) }).withValue(Groups.TITLE,
                group.getTitle()).withValue(Groups.NOTES, group.getNotes())
                .build());

        return b;
    }

    /**
     * @param id
     *            The row id.
     * @param permanently
     *            Whether delete permanently.
     * @return Whether success to delete the group.
     */
    public boolean appendGroupDelete(long id, boolean permanently) {
        Uri deleteUri;
        if (permanently) {
            deleteUri = Groups.CONTENT_URI
                    // Delete physically, not just set DELETED=1
                    .buildUpon().appendQueryParameter(
                            ContactsContract.CALLER_IS_SYNCADAPTER, "true")
                    .build();
        } else {
            deleteUri = Groups.CONTENT_URI;
        }
        boolean b = append(ContentProviderOperation.newDelete(deleteUri)
                .withSelection(Groups._ID + "=?",
                        new String[] { String.valueOf(id) }).build());

        return b;
    }
}
