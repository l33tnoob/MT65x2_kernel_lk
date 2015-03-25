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

import android.content.ContentProviderResult;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Groups;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.RawContactsEntity;
import android.telephony.PhoneNumberUtils;

import com.mediatek.android.content.ContentProviderOperationBatch;
import com.mediatek.android.content.DefaultBulkInsertHelper;
import com.mediatek.android.content.DefaultDeleteBatchHelper;
import com.mediatek.android.content.DefaultInsertBatchHelper;
import com.mediatek.android.content.MeasuredContentValues;
import com.mediatek.apst.target.data.provider.contacts.ContactDataContent;
import com.mediatek.apst.target.data.provider.contacts.GroupContent;
import com.mediatek.apst.target.data.provider.contacts.RawContactsContent;
import com.mediatek.apst.target.data.provider.contacts.RawContactsEntityContent;
import com.mediatek.apst.target.data.provider.contacts.SimContactsContent;
import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.contacts.USIMUtils.USIMGroupException;
import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.target.util.SharedPrefs;
import com.mediatek.apst.util.FeatureOptionControl;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.DataStoreLocations;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
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
import com.mediatek.apst.util.entity.contacts.RawContact.UnknownContactDataTypeException;
import com.mediatek.apst.util.entity.contacts.StructuredName;
import com.mediatek.apst.util.entity.contacts.StructuredPostal;
import com.mediatek.apst.util.entity.contacts.Website;
import com.mediatek.apst.util.entity.message.Message;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Class Name: ContactsProxy
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.contacts
 * <p>
 * Created on: 2010-8-11
 * <p>
 * <p>
 * Description:
 * <p>
 * Facade of the sub system of contacts related operations.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public final class ContactsProxy extends ContextBasedProxy {
    /** Singleton instance. */
    private static ContactsProxy sInstance = null;

    private final ContactsOperationBatch mOpBatch;

    /**
     * @param context
     *            The context for the ContactsProxy.
     */
    private ContactsProxy(final Context context) {
        super(context);
        setProxyName("ContactsProxy");
        mOpBatch = new ContactsOperationBatch(getObservedContentResolver());
    }

    /**
     * @param context
     *            The current context.
     * @return A instance of the contact proxy.
     */
    public static synchronized ContactsProxy getInstance(final Context context) {
        if (null == sInstance) {
            sInstance = new ContactsProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    // // Sim Contacts
    // ------------------------------------------------------------
    // public static Uri getSimUri(int slotId) { // int simId. Modified by
    // Shaoying Han
    // switch (slotId) {
    // case DataStoreLocations.SIM:
    // return SimContactsContent.CONTENT_URI;
    //
    // case DataStoreLocations.SIM1:
    // return SimContactsContent.CONTENT_URI_SIM1;
    //
    // case DataStoreLocations.SIM2:
    // return SimContactsContent.CONTENT_URI_SIM2;
    //
    // default:
    // Debugger.logE(new Object[] { slotId }, "Invalid Slot id " + slotId +
    // ".");
    // return Uri.parse("");
    // }
    // }

    // Sim Contacts ------------------------------------------------------------
    /**
     * @param sourceLocation
     *            The sourceLocation.
     * @return The Uri of the contact in sim.
     */
    public static Uri getSimUri(final int sourceLocation) { // int simId. Modified by Shaoying
        // Han
        final boolean isUsim = USIMUtils.isSimUsimType(sourceLocation);
        switch (sourceLocation) {
        case DataStoreLocations.SIM:
            return isUsim ? SimContactsContent.ICCUSIMURI
                    : SimContactsContent.CONTENT_URI;

        case DataStoreLocations.SIM1:
            return isUsim ? SimContactsContent.ICCUSIM1URI
                    : SimContactsContent.CONTENT_URI_SIM1;

        case DataStoreLocations.SIM2:
            return isUsim ? SimContactsContent.ICCUSIM2URI
                    : SimContactsContent.CONTENT_URI_SIM2;
            
        case DataStoreLocations.SIM3:
            return isUsim ? SimContactsContent.ICCUSIM3URI
                    : SimContactsContent.CONTENT_URI_SIM3;
            
        case DataStoreLocations.SIM4:
            return isUsim ? SimContactsContent.ICCUSIM4URI
                    : SimContactsContent.CONTENT_URI_SIM4;

        default:
            Debugger.logE(new Object[] { sourceLocation }, "Invalid Slot id " + sourceLocation
                    + ".");
            return Uri.parse("");
        }
    }

    // //from iccprovider.java mtk54043 2012-4-19
    // public static Uri getSimUri(int slotId) {
    // boolean isUsim = isSimUsimType(slotId);
    // if (FeatureOption.MTK_GEMINI_SUPPORT) {
    // if (slotId == 0) {
    // return isUsim ? mIccUsim1Uri : mIccUri1;
    // } else {
    // return isUsim ? mIccUsim2Uri : mIccUri2;
    // }
    // } else {
    // return isUsim ? mIccUsimUri : mIccUri;
    // }
    // }
    // }

    // Only for the old design of RawContact
    /**
     * @param sourceLocation
     *            The location of the contact.
     * @return The id of the slot.
     */
    public static int getSlotId(final int sourceLocation) {
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
            return DataStoreLocations.SIM;

        case RawContact.SOURCE_SIM1:
            return DataStoreLocations.SIM1;

        case RawContact.SOURCE_SIM2:
            return DataStoreLocations.SIM2;
            
        case RawContact.SOURCE_SIM3:
            return DataStoreLocations.SIM3;

        case RawContact.SOURCE_SIM4:
            return DataStoreLocations.SIM4;

        default:
            Debugger.logE(new Object[] { sourceLocation },
                    "Invalid source location " + sourceLocation + " for SIM.");
            return DataStoreLocations.NONE;
        }
    }

    /**
     * @param sourceLocation
     *            The location of the contact.
     * @return The real slot id.
     */
    public static int getRealSlotId(final int sourceLocation) {
        if (sourceLocation < 0 || sourceLocation > 4) {
        	Debugger.logE(new Object[] { sourceLocation },
                  "Invalid source location " + sourceLocation + " for SIM.");
            return -1;
        }
        if (Config.MTK_GEMINI_SUPPORT) {
            return sourceLocation - 1;
        } else {
            return sourceLocation;
        }

    }

    /**
     * Add a contact on SIM with the specified name and number. Should specify
     * both the name and the number.
     * 
     * @param name
     *            Name of the contact to insert.
     * @param num
     *            Number of the contact to insert.
     * @param sourceLocation
     *            The id of the slot.
     * @return Id of the inserted row. This might be useless currently, for the
     *         ContentProvider of current version of Android SDK always return
     *         1. Return DatabaseRecordEntity.ID_NULL when insertion fail.
     * @see DatabaseRecordEntity#ID_NULL
     */
    public long insertSimContact(final String name, final String num,
            final int sourceLocation) { // int
        // simId.
        // Shaoying
        String number = num;
        if (null == name || null == number) {
            Debugger.logE(new Object[] { name, number, sourceLocation },
                    "Name and number should be specified but not null.");
            return DatabaseRecordEntity.ID_NULL;
        }

        Debugger.logE(new Object[] {}, "insertSimContact,name" + name
                + "number" + number);

        if (null != number) { //
            number = PhoneNumberUtils.stripSeparators(number);
        }

        long result = SimContactsContent.ERR_ICC_UNKNOWN;

        final ContentValues values = new ContentValues(2);
        values.put(SimContactsContent.COLUMN_TAG, name);
        values.put(SimContactsContent.COLUMN_NUMBER, number);
        // FIXME specify emails won't work
        // values.put("emails", "apst@mediatek.com");
        // FIXME specify _id won't work
        // values.put("_id", 999);

        try {
            final Uri uri = getObservedContentResolver().insert(
                    getSimUri(sourceLocation), values);
            if (uri != null) {
                // result = Long.parseLong(uri.getLastPathSegment());
                result = Long.parseLong(uri.getPathSegments().get(1));
                Debugger.logI(new Object[] { name, number, sourceLocation },
                        "Inserted SIM contact, result is " + result);
                if (result < 0) {
                    Debugger.logE(new Object[] { name, number, sourceLocation },
                                    "Failed to insert SIM contact, result is "
                                            + result);
                } else {
                    result = ContentUris.parseId(uri);
                    Debugger.logI(new Object[] { name, number, sourceLocation }, "Successed to insert SIM contact, index is "
                            + result);
                }
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { name, number, sourceLocation }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { name, number, sourceLocation }, null, e);
        }

        return result;
    }

    /**
     * Add a contact on SIM with the specified name and number. Should specify
     * both the name and the number.
     * 
     * @param name
     *            Name of the contact to insert.
     * @param num
     *            Number of the contact to insert.
     * @param email
     *            The email of the contact.
     * @param sourceLocation
     *            The sourceLocation.
     * @return Id of the inserted row. This might be useless currently, for the
     *         ContentProvider of current version of Android SDK always return
     *         1. Return DatabaseRecordEntity.ID_NULL when insertion fail.
     * @see DatabaseRecordEntity#ID_NULL
     */
    public long insertSimContact(final String name, final String num,
            final String email, final int sourceLocation) { // int simId. Shaoying
        String number = num;
        if (null == name || null == number) {
            Debugger.logE(new Object[] { name, number, sourceLocation },
                    "Name and number should be specified but not null.");
            return DatabaseRecordEntity.ID_NULL;
        }

        Debugger.logI(new Object[] {}, "insertSimContact,name" + name
                + "number" + number);

        if (null != number) {
            number = PhoneNumberUtils.stripSeparators(number);
        }

        long result = SimContactsContent.ERR_ICC_UNKNOWN;

        final ContentValues values = new ContentValues(3);
        values.put(SimContactsContent.COLUMN_TAG, name);
        values.put(SimContactsContent.COLUMN_NUMBER, number);
        if (null != email) {
            values.put(SimContactsContent.COLUMN_EMAILS, email);
            Debugger.logI(new Object[] {}, "insertSimContact,email" + email);
        } else {
            Debugger.logE(new Object[] {}, "insertSimContact , email is null");
        }
        // FIXME specify emails won't work
        // values.put("emails", "apst@mediatek.com");
        // FIXME specify _id won't work
        // values.put("_id", 999);

        try {
            final Uri uri = getObservedContentResolver().insert(
                    getSimUri(sourceLocation), values);
            if (uri != null) {
                // result = Long.parseLong(uri.getLastPathSegment());
                result = Long.parseLong(uri.getPathSegments().get(1));
                Debugger.logI(new Object[] { name, number, email, sourceLocation },
                        "Inserted SIM contact, result is " + result);
                if (result < 0) {
                    Debugger
                            .logE(new Object[] { name, number, sourceLocation },
                                    "Failed to insert SIM contact, result is "
                                            + result);
                } else {
                    // get sim index 
                    result = ContentUris.parseId(uri);
                    Debugger.logI(new Object[] { name, number, sourceLocation }, "Successed to insert SIM contact, index is "
                            + result);
                }
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { name, number, sourceLocation }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { name, number, sourceLocation }, null, e);
        }

        return result;
    }

    /**
     * Update contact on SIM with the new name and number. Should specify both
     * the old name and the old number.
     * 
     * @param oldName
     *            Old name of the contact to update. Must not be null.
     * @param oldNumber
     *            Old number of the contact to update. Must not be null.
     * @param newName
     *            New name of the contact to update. Pass null if this field
     *            does not need change.
     * @param newNumber
     *            New number of the contact to update. Pass null if this field
     *            does not need change.
     * @param sourceLocation
     *            The sourceLocation.
     * @return How many contacts are updated.
     */
    public int updateSimContact(final String oldName, final String oldNumber,
            final String newName, final String newNumber, final int sourceLocation) {
        final String oName = oldName;
        String oNumber = oldNumber;
        String nName = newName;
        String nNumber = newNumber;
        if (oName == null || oNumber == null) {
            // Old name and old number are needed for finding the proper record
            // to update and must not be null. So if any of them is null, do
            // nothing and return 0.
            Debugger.logE(
                    new Object[] { oName, oNumber, nName, nNumber, sourceLocation },
                    "'oldName' and 'oldNumber' must not be null.");
            return 0;
        }

        if ((nName == null && nNumber == null)
                || (nName.equals(oName) && nNumber.equals(oNumber))) {
            // If both new name and new number are the same with old ones,
            // or both of them are null(which means the same), do nothing and
            // return 0.
            Debugger.logE(
                    new Object[] { oName, oNumber, nName, nNumber, sourceLocation },
                    "must not be equal.");
            return 0;
        } else if (nName == null) {
            // Name does not need change
            nName = oName;
        } else if (nNumber == null) {
            // Number does not need change
            nNumber = oNumber;
        }

        if (null != oNumber) {
            oNumber = PhoneNumberUtils.stripSeparators(oNumber);
        }

        if (null != nNumber) {
            nNumber = PhoneNumberUtils.stripSeparators(nNumber);
        }

        int result = 0;
        final ContentValues values = new ContentValues(4);
        values.put(SimContactsContent.COLUMN_TAG, oName);
        values.put(SimContactsContent.COLUMN_NUMBER, oNumber);
        values.put(SimContactsContent.COLUMN_NEW_TAG, nName);
        values.put(SimContactsContent.COLUMN_NEW_NUMBER, nNumber);

        result = getObservedContentResolver().update(getSimUri(sourceLocation), values,
                null, null);
        if (result < 0) {
            Debugger.logE(
                    new Object[] { oName, oNumber, nName, nNumber, sourceLocation },
                    "Failed to update SIM contact, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(
                    new Object[] { oName, oNumber, nName, nNumber, sourceLocation },
                    "Updated several SIM contacts in one time, "
                            + "please check if it is normal.");
        }

        return result;
    }

    /**
     * Update contact on SIM with the new name and number. Should specify both
     * the old name and the old number.
     * 
     * @param oldName
     *            Old name of the contact to update. Must not be null.
     * @param oldNumber
     *            Old number of the contact to update. Must not be null.
     * @param oEmail
     *            The old email to update.
     * @param newName
     *            New name of the contact to update. Pass null if this field
     *            does not need change.
     * @param newNumber
     *            New number of the contact to update. Pass null if this field
     *            does not need change.
     * @param nEmail
     *            The new Email used to update.
     * @param sourceLocation
     *            The sourceLocation.
     * @return How many contacts are updated.
     */
    public int updateSimContact(final String oldName, final String oldNumber,
            final String oEmail, final String newName, final String newNumber,
            final String nEmail, final int sourceLocation) {
        final String oName = oldName;
        String oNumber = oldNumber;
        String nName = newName;
        String nNumber = newNumber;
        if (oName == null || oNumber == null) {
            // Old name and old number are needed for finding the proper record
            // to update and must not be null. So if any of them is null, do
            // nothing and return 0.
            Debugger.logE(new Object[] { oName, oNumber, oEmail, nName,
                    nNumber, nEmail, sourceLocation },
                    "'oldName' and 'oldNumber' must not be null.");
            return 0;
        }

        if ((nName == null && nNumber == null)
                || (nName.equals(oName) && nNumber.equals(oNumber) && nEmail
                        .equals(oEmail))) {
            // If both new name and new number are the same with old ones,
            // or both of them are null(which means the same), do nothing and
            // return 0.
            return 0;
        } else if (nName == null) {
            // Name does not need change
            nName = oName;
        } else if (nNumber == null) {
            // Number does not need change
            nNumber = oNumber;
        }

        if (null != oNumber) {
            oNumber = PhoneNumberUtils.stripSeparators(oNumber);
        }

        if (null != nNumber) {
            nNumber = PhoneNumberUtils.stripSeparators(nNumber);
        }

        final ContentValues values = new ContentValues(6);
        values.put(SimContactsContent.COLUMN_TAG, oName);
        values.put(SimContactsContent.COLUMN_NUMBER, oNumber);
        values.put(SimContactsContent.COLUMN_EMAILS, oEmail);
        values.put(SimContactsContent.COLUMN_NEW_TAG, nName);
        values.put(SimContactsContent.COLUMN_NEW_NUMBER, nNumber);
        values.put(SimContactsContent.COLUMN_NEW_EMAILS, nEmail);

        final int result = getObservedContentResolver().update(
                getSimUri(sourceLocation), values, null, null);
        if (result < 0) {
            Debugger.logE(new Object[] { oName, oNumber, oEmail, nName,
                    nNumber, nEmail, sourceLocation },
                    "Failed to update SIM contact, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { oName, oNumber, oEmail, nName,
                    nNumber, nEmail, sourceLocation },
                    "Updated several SIM contacts in one time, "
                            + "please check if it is normal.");
        }

        return result;
    }

    /**
     * Delete contact on SIM with the specified name and number. Should specify
     * both the name and the number.
     * 
     * @param name
     *            Name of the contact to delete.
     * @param deleteNumber
     *            Number of the contact to delete.
     * @param sourceLocation
     *            The sourceLocation.
     * @return How many rows are deleted.
     */
    public int deleteSimContact(final String name, final String deleteNumber,
            final int sourceLocation) {
        // int simId. Modified by Shaoying Han
        String number = deleteNumber;
        if (null == name) {
            Debugger.logE(new Object[] { name, number, sourceLocation },
                    "Name and number should be specified but not null.");
            return 0;
        }
        final StringBuffer selection = new StringBuffer();
        selection.append(SimContactsContent.COLUMN_TAG + "='" + name + "'");
        if (null != number) {
            number = PhoneNumberUtils.stripSeparators(number);
            selection.append("AND" + SimContactsContent.COLUMN_NUMBER + "='"
                    + number + "'");
        } else {
            Debugger.logW("DeleteSimCOntact:" + name + "  number is null");
        }

        Debugger.logI("DeleteSimCOntact:" + selection.toString());
        final int result = getObservedContentResolver().delete(
                getSimUri(sourceLocation), selection.toString(), null);

        if (result < 0) {
            Debugger.logE(new Object[] { name, number, sourceLocation },
                    "Failed to delete SIM contact, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { name, number, sourceLocation },
                    "Deleted several SIM contacts in one time, "
                            + "please check if it is normal.");
        }

        return result;

    }

    /**
     * Delete contact on SIM with the specified name and number. Should specify
     * both the name and the number.
     * 
     * @param name
     *            Name of the contact to delete.
     * @param deleteNumber
     *            Number of the contact to delete.
     * @param email
     *            The email of the contact to be deleted.
     * @param sourceLocation
     *            The sourceLocation.
     * @return How many rows are deleted.
     */
    public int deleteSimContact(final String name, final String deleteNumber,
            final String email, final int sourceLocation) {
        String number = deleteNumber;
        // int simId. Modified by Shaoying Han
        if (null == name) {
            Debugger.logE(new Object[] { name, number, email, sourceLocation },
                    "Name and number should be specified but not null.");
            return 0;
        }

        final StringBuffer selection = new StringBuffer();
        selection.append(SimContactsContent.COLUMN_TAG + "='" + name + "'");
        if (null != number) {
            number = PhoneNumberUtils.stripSeparators(number);
            selection.append("AND" + SimContactsContent.COLUMN_NUMBER + "='"
                    + number + "'");
        } else {
            Debugger.logW("DeleteSimCOntact:" + name + "  number is null");
        }

        if (null != email) {
            selection.append("AND" + SimContactsContent.COLUMN_EMAILS + "='"
                    + email + "'");
        } else {
            Debugger.logW("DeleteSimCOntact:" + name + "  email is null");
        }

        Debugger.logI("DeleteSimCOntact:" + selection.toString());
        final int result = getObservedContentResolver().delete(
                getSimUri(sourceLocation), selection.toString(), null);

        if (result < 0) {
            Debugger.logE(new Object[] { name, number, email, sourceLocation },
                    "Failed to delete SIM contact, result is " + result);
        } else if (result > 1) {
            // Update on multiple rows is not supposed to occur
            Debugger.logW(new Object[] { name, number, email, sourceLocation },
                    "Deleted several SIM contacts in one time, "
                            + "please check if it is normal.");
        }

        return result;
    }

    /**
     * Delete contact on SIM with the specified name and number. Should specify
     * both the name and the number.
     * 
     * @param names
     *            List of names of the contact to delete.
     * @param numbers
     *            List of numbers of the contact to delete.
     * @param sourceLocation
     *            The id of the slot.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteSimContacts(final String[] names,
            final String[] numbers, final int sourceLocation) {
        // int simId. Modified by Shaoying Han
        if (null == names || null == numbers) {
            Debugger.logE(new Object[] { names, numbers, sourceLocation },
                    "List is null.");
            return null;
        }
        if (names.length != numbers.length) {
            Debugger.logE(new Object[] { names, numbers, sourceLocation },
                    "Size of name list does not match size of number list.");
            return null;
        }

        final boolean[] results = new boolean[names.length];
        for (int i = 0; i < names.length; i++) {
            final int count = deleteSimContact(names[i], numbers[i], sourceLocation);
            if (count >= 1) {
                results[i] = true;
            }
        }
        return results;
    }

    // Raw Contacts ------------------------------------------------------------
    /**
     * Get a list of all contacts stored on phone. Only basic info is contained.
     * 
     * @return A list of contacts possess only basic info.
     * @deprecated
     */
    /*
     * public List<BaseContact> getAllSimpleContacts(){ List<BaseContact> result
     * = new ArrayList<BaseContact>(); Map<Long, BaseContact> mapIdToBaseContact
     * = new HashMap<Long, BaseContact>();
     * 
     * Cursor cContacts; Cursor cData;
     * 
     * // Query all raw contatcs on phone cContacts =
     * getContentResolver().query( RawContacts.CONTENT_URI, new String[]{
     * RawContacts._ID, // This column is returned in query on RawContacts after
     * // Android 2.2, but is not supported and will cause // exception in early
     * version of Android Contacts.DISPLAY_NAME }, RawContacts.DELETED + "<>" +
     * DatabaseRecordEntity.TRUE, null, Contacts.DISPLAY_NAME + " ASC");
     * 
     * if (cContacts != null){ long cId; while (cContacts.moveToNext()){ cId =
     * cContacts.getLong(0); // Construct a new contact object with the contact
     * id BaseContact contact = new BaseContact(cId); // Set store location
     * field to phone contact.setStoreLocation(DataStoreLocations.PHONE); // Set
     * display name contact.setDisplayName(cContacts.getString(1)); // Add the
     * contact to result list result.add(contact); // Record the mapping of the
     * id to the contact mapIdToBaseContact.put(cId, contact); } // Release
     * resources cContacts.close(); } else{ Debugger.logE("cContacts is NULL!");
     * }
     * 
     * // Query all basic contact data on phone cData =
     * getContentResolver().query( Data.CONTENT_URI, new String[]{ Data._ID,
     * Data.RAW_CONTACT_ID, Data.DATA1 }, Data.MIMETYPE + "='" +
     * CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE + "'", null,
     * Data.RAW_CONTACT_ID + " ASC");
     * 
     * if (cData != null){ // Data id long dId; // Raw contact id long cId; //
     * Raw contact id of current contact object. // Just initialize it with a
     * value that is impossible to occur. long currentRawContactId = -127;
     * BaseContact currentBaseContact = null;
     * 
     * while (cData.moveToNext()){ cId = cData.getLong(1); if (cId !=
     * currentRawContactId){ if (!mapIdToBaseContact.containsKey(cId)){ // A
     * data record belongs to no contact occurred. // It's not supposed to
     * happen. Debugger.logE("A data record belongs to no raw " +
     * "contact occurred. Its raw contact id is "+cId); continue; }
     * currentBaseContact = mapIdToBaseContact.get(cId); currentRawContactId =
     * cId; } if (currentBaseContact == null){ continue; }
     * 
     * dId = cData.getLong(0);
     * 
     * // Fill the proper raw contact data fields according to the // data's
     * MIME type GroupMembership groupMembership = new GroupMembership(dId); //
     * Set the group id of this group membership
     * groupMembership.setGroupId(cData.getLong(2)); // Add the group membership
     * to current contact
     * currentBaseContact.getGroupMemberships().add(groupMembership); } //
     * Release resources cData.close(); }
     * 
     * return result; }
     */
    /**
     * Get a specified contact on phone with all its detailed contact data.
     * 
     * @param id
     *            The unique database id of the contact to get.
     * @param withData
     *            If true, all corresponding contact data will also be queried
     *            and returned.
     * @return A detailed contact contains all data fields.
     */
    public RawContact getContact(final long id, final boolean withData) {
        RawContact contact = null;

        Cursor cContact;
        Cursor cData;

        // Query the raw contact record first
        cContact = getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts.TIMES_CONTACTED,
                        RawContacts.LAST_TIME_CONTACTED, RawContacts.STARRED,
                        RawContacts.CUSTOM_RINGTONE,
                        RawContacts.SEND_TO_VOICEMAIL, RawContacts.VERSION,
                        Contacts.DISPLAY_NAME,
                        // For MTK SIM Contacts feature
                        RawContactsContent.COLUMN_SOURCE_LOCATION,
                        // Modify time. Added by Shaoying Han
                        RawContactsContent.COLUMN_MODIFY_TIME,
                        RawContactsContent.COLUMN_INDEX_IN_SIM },
                RawContacts._ID + "=" + id + " AND " + RawContacts.DELETED
                        + "<>" + DatabaseRecordEntity.TRUE, null, null);

        if (cContact != null) {
            if (cContact.moveToNext()) {
                // Raw contact with the specified id exists
                contact = RawContactsContent.cursorToRawContact(cContact);
                // Because cursor does not contain id, we should set it
                contact.setId(id);
                if (!withData) {
                    // If do not query PIM data, return by here
                    return contact;
                }
                // Query the raw contact's PIM data records
                cData = getContentResolver().query(
                        RawContactsEntity.CONTENT_URI,
                        new String[] { RawContactsEntity.DATA_ID,
                                RawContactsEntity.MIMETYPE,
                                RawContactsEntity.DATA1,
                                RawContactsEntity.DATA2,
                                RawContactsEntity.DATA3,
                                RawContactsEntity.DATA4,
                                RawContactsEntity.DATA5,
                                RawContactsEntity.DATA6,
                                RawContactsEntity.DATA7,
                                RawContactsEntity.DATA8,
                                RawContactsEntity.DATA9,
                                RawContactsEntity.DATA10,
                                RawContactsEntity.DATA15,
                                // Added by Shaoying Han
                                ContactDataContent.COLUMN_BINDING_SIM_ID },
                        RawContactsEntity._ID + "=" + id + " AND "
                                + RawContactsEntity.DELETED + "<>"
                                + DatabaseRecordEntity.TRUE
                        /*
                         * + " AND " + RawContactsEntity.MIMETYPE + "<>'" +
                         * CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE +
                         * "'"
                         */, null, null);

                if (cData != null) {
                    // Parse all data rows
                    while (cData.moveToNext()) {
                        if (cData.isNull(0)) {
                            // Data row ID will be null only if the raw contact
                            // has no data rows
                            break;
                        }

                        final ContactData data = RawContactsEntityContent
                                .cursorToContactData(cData);
                        try {
                            if (null != data) {
                                contact.addContactData(data);
                            }
                        } catch (final UnknownContactDataTypeException e) {
                            Debugger.logE(new Object[] { id, withData }, null,
                                    e);
                        }
                    }
                    // Release resources
                    cData.close();
                }
            }
            // Release resources
            cContact.close();
        }
        // DO NOT manually regenerate display name, for it's auto-generated in
        // Froyo
        // genDisplayName(contact, false);
        // Debugger.logI(new Object[] { },"name : "+
        // contact.getDisplayName()+"id : "+contact.getId());
        return contact;
    }

    // For MTK SIM Contacts feature
    /**
     * @param id
     *            The id of the raw contact.
     * @return The location of the raw contact.
     */
    public int getContactSourceLocation(final long id) {
        int sourceLocation = RawContact.SOURCE_NONE;
        final Cursor c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContactsContent.COLUMN_SOURCE_LOCATION },
                RawContacts._ID + "=" + id, null, null);

        if (c != null) { //
            if (c.moveToNext()) {
                // sourceLocation = c.getInt(0);
                // Modified by Shaoying Han
                sourceLocation = Global.getSourceLocationById(c.getInt(0));
            } else {
                Debugger.logE(new Object[] { id },
                        "Fail to find the raw contact.");
            }
            c.close();
        } else {
            Debugger.logE(new Object[] { id },
                    "Cursor is null. Fail to find the raw contact.");
        }

        return sourceLocation;
    }

    /**
     * Add a contact on phone. Also add its corresponding data records.
     * 
     * @param newContact
     *            The contact to add. This object should encapsulate all needed
     *            data including phone numbers, E-mails, organizations, etc.
     * @param withData
     *            If true, all corresponding contact data will also be inserted.
     * @return The id after the contact has been inserted. Return -1 when
     *         insertion fail. <b>NOTE:<b> New id of all data will be set in the
     *         RawContact object.
     */
    public long insertContact(final RawContact newContact,
            final boolean withData) {
        if (newContact == null) {
            Debugger.logW(new Object[] { newContact, withData },
                    "Contact passed in is null.");
            return RawContactsContent.INSERT_FAIL;
        }

        long cId = RawContactsContent.INSERT_FAIL;

        final ContentValues values = new ContentValues(7);

        // Add content values for the raw contact insertion
        // Disable aggregation
        values.put(RawContacts.AGGREGATION_MODE,
                RawContacts.AGGREGATION_MODE_DISABLED);
        values.put(RawContacts.STARRED, newContact.isStarred() ? 1 : 0); // Modify
        // by
        // Yu
        // 2011-12-20
        // Set new ringtone
        /*
         * values.put(RawContacts.CUSTOM_RINGTONE,
         * newContact.getCustomRingtone());
         */
        values.put(RawContacts.SEND_TO_VOICEMAIL, newContact
                .isSendToVoicemail());
        // For MTK SIM Contacts feature
        // **********************************************
        final int realSlotId = getRealSlotId(newContact.getSourceLocation());
        if (realSlotId == -1) {
            values.put(RawContactsContent.COLUMN_SOURCE_LOCATION,
                    RawContact.SOURCE_PHONE);
            values.put(RawContacts.ACCOUNT_NAME,
                    USIMUtils.ACCOUNT_NAME_LOCAL_PHONE);
            values.put(RawContacts.ACCOUNT_TYPE,
                    USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE);
        } else {
            final SimDetailInfo info = Global.getSimInfoBySlot(realSlotId);
            values.put(RawContactsContent.COLUMN_SOURCE_LOCATION, info
                    .getSimId());
            // Added by mtk54043 2012-3-9
            values.put(RawContacts.ACCOUNT_NAME, USIMUtils
                    .getSimAccountNameBySlot(realSlotId));
            values.put(RawContacts.ACCOUNT_TYPE, USIMUtils
                    .getAccountTypeBySlot(realSlotId));
        }
        values.put(RawContacts.SEND_TO_VOICEMAIL, newContact
                .isSendToVoicemail());
        switch (newContact.getSourceLocation()) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            // This contact should be inserted into SIM before into database
            long simInsertionResult = SimContactsContent.ERR_ICC_UNKNOWN;
            String newSimName = "";
            String newSimNumber = "";
            String newSimEmail = "";
            if (newContact.getNames() != null
                    && newContact.getNames().size() > 0
                    && newContact.getNames().get(0).getDisplayName() != null) {
                newSimName = newContact.getNames().get(0).getDisplayName();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM name");
            }

            if (newContact.getPhones() != null
                    && newContact.getPhones().size() > 0
                    && newContact.getPhones().get(0).getNumber() != null) {
                newSimNumber = newContact.getPhones().get(0).getNumber();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM number");
            }

            if (newContact.getEmails() != null
                    && newContact.getEmails().size() > 0
                    && newContact.getEmails().get(0).getData() != null) {
                newSimEmail = newContact.getEmails().get(0).getData();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM email");
            }

            int sourceLocation = newContact.getSourceLocation();
            boolean isUSIM = USIMUtils.isSimUsimType(sourceLocation);
            if (isUSIM) {
                simInsertionResult = insertSimContact(newSimName, newSimNumber,
                        newSimEmail, sourceLocation);
            } else {
                simInsertionResult = insertSimContact(newSimName, newSimNumber,
                        sourceLocation);
            }

            if (simInsertionResult < 0) {
                // Insertion in SIM failed, so cancel insertion in database
                Debugger.logE(new Object[] { newContact, withData },
                        "Failed to insert contact into SIM.");
                newContact.setId(DatabaseRecordEntity.ID_NULL);
                // return DatabaseRecordEntity.ID_NULL;
                return simInsertionResult;
            } else {
                // insert sim index to db.
                values.put(RawContactsContent.COLUMN_INDEX_IN_SIM, simInsertionResult);
            }
            break;
        case RawContact.SOURCE_PHONE:
            break;
        default:
            Debugger.logW(new Object[] { newContact, withData },
                    "Invalid source location " + newContact.getSourceLocation()
                            + ".");
            return RawContactsContent.INSERT_FAIL;
        }
        // *********************************************************************

        try {
            final Uri uri = getObservedContentResolver().insert(
                    RawContacts.CONTENT_URI, values);
            if (uri != null) {
                cId = Long.parseLong(uri.getLastPathSegment());
                Debugger
                        .logE(new Object[] {}, "insert raw_contacts id :" + cId);
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { newContact, withData }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { newContact, withData }, null, e);
        }

        // Set the new id after insertion
        newContact.setId(cId);

        if (!withData) {
            // If do not insert PIM data, return by here
            return cId;
        }
        if (cId <= 0) {
            // Illegal id, possibly means the insertion fails, return
            return RawContactsContent.INSERT_FAIL;
        }

        // User batch mode to add all data of the contact
        mOpBatch.clear();
        final List<ContactData> allData = newContact.getAllContactData();
        int contactDataIndex = -1;
        // Has no data to insert
        if (allData == null) {
            return cId;
        }
        // Has data to insert
        int i = 0;
        for (final ContactData data : allData) {
            ++i;
            // Ensure the rawContactId in data is correct
            data.setRawContactId(cId);
            mOpBatch.appendContactDataInsert(data);
            // Operations batch has reached the max count,
            // or all data have been read,
            // We need to apply the batch immediately and then release memory
            if (mOpBatch.isFull() || i == allData.size()) {
                ContentProviderResult[] dataResults = null;
                try {
                    dataResults = mOpBatch.apply();
                } catch (final RemoteException e) {
                    Debugger.logE(new Object[] { newContact, withData }, null,
                            e);
                    return cId;
                } catch (final OperationApplicationException e) {
                    Debugger.logE(new Object[] { newContact, withData }, null,
                            e);
                    return cId;
                } finally {
                    // Clear operations
                    mOpBatch.clear();
                }

                if (dataResults != null) {
                    for (final ContentProviderResult result : dataResults) {
                        try {
                            ++contactDataIndex;
                            if (result == null) {
                                continue;
                            }
                            // The _id of the record in Data table
                            final long dataId = Long.parseLong(result.uri
                                    .getLastPathSegment());
                            final ContactData currentData = allData
                                    .get(contactDataIndex);
                            if (currentData != null) {
                                // Set the returned id to contact data object,
                                // as the insertion of the data is successful
                                currentData.setId(dataId);
                            }
                        } catch (final NumberFormatException e) {
                            Debugger.logE(
                                    new Object[] { newContact, withData },
                                    null, e);
                        }
                    }
                }
            }

        }

        // Refresh the display name of the new inserted raw contact
        /*
         * Cursor c = getContentResolver().query( RawContacts.CONTENT_URI, new
         * String[]{ Contacts.DISPLAY_NAME }, null, null, null); if (c != null){
         * if (c.moveToNext()){ newContact.setDisplayName(c.getString(0)); }
         * c.close(); }
         */
        // DO NOT manually regenerate display name, for it's auto-generated in
        // Froyo
        // genDisplayName(newContact, false);

        return cId;
    }

    /**
     * Update a contact on phone. Update will follow 3 steps:
     * <p>
     * 1. Update the base info columns of raw contact table
     * <p>
     * 2. Delete all current data of the specified contact object (If is
     * required by invoker)
     * <p>
     * 3. Insert all new data passed in the specified contact object (If is
     * required by invoker)
     * 
     * @param id
     *            The unique database id of the contact to update.
     * @param sourceLocation
     *            the location of the contact.
     * @param simName
     *            The name of sim.
     * @param simNumber
     *            The number of sim.
     * @param newContact
     *            The contact object encapsulating all new data of the contact.
     * @param updatePIMData
     *            If true, all corresponding PIM data will also be updated.
     * @return How many contacts are updated.
     */
    public int updateContact(final long id, final int sourceLocation,
            final String simName, final String simNumber,
            final RawContact newContact, final boolean updatePIMData) {
        if (newContact == null) {
            Debugger.logW(new Object[] { id, sourceLocation, simName,
                    simNumber, newContact, updatePIMData },
                    "New contact passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(2);

        // 1. Update raw contact base columns
        // Add content values for the raw contact update
        values.put(RawContacts.STARRED, newContact.isStarred() ? 1 : 0); // Modify
        // by
        // Yu
        // 2011-12-20
        values.put(RawContacts.SEND_TO_VOICEMAIL, newContact
                .isSendToVoicemail());
        /*
         * values.put(RawContacts.CUSTOM_RINGTONE,
         * newContact.getCustomRingtone());
         */

        updateCount = getObservedContentResolver().update(
                RawContacts.CONTENT_URI,
                values,
                RawContacts._ID + "=" + id + " AND " + RawContacts.DELETED
                        + "<>" + DatabaseRecordEntity.TRUE, null);

        // If do not update PIM data or update RawContact failed, return by here
        if (!updatePIMData || updateCount < 1) {
            return updateCount;
        }

        // 2. Insert all new data
        // For MTK SIM Contacts feature ****************************************
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            // This contact should be updated in SIM before updated in database
            int simUpdateResult = 0;

            String newSimName = "";
            String newSimNumber = "";
            if (newContact.getNames() != null
                    && newContact.getNames().size() > 0
                    && newContact.getNames().get(0).getDisplayName() != null) {
                /** provider changed 2012-12-5*/
//                newSimName = newContact.getNames().get(0).getGivenName();
                newSimName = newContact.getNames().get(0).getDisplayName();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM name");
            }

            if (newContact.getPhones() != null
                    && newContact.getPhones().size() > 0
                    && newContact.getPhones().get(0).getNumber() != null) {
                newSimNumber = newContact.getPhones().get(0).getNumber();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM number");
            }

            simUpdateResult = updateSimContact(simName, simNumber, newSimName,
                    newSimNumber, sourceLocation);// Modified by
            // Shaoying Han

            if (simUpdateResult < 1) {
                // Update in SIM failed, so cancel update in database
                Debugger.logE(new Object[] { id, sourceLocation, simName,
                        simNumber, newContact, updatePIMData },
                        "Failed to update contact in SIM.");
                newContact.setId(DatabaseRecordEntity.ID_NULL);
                return 0;
            }
            break;
        case RawContact.SOURCE_PHONE:
            break;
        default:
            Debugger.logW(new Object[] { id, sourceLocation, simName,
                    simNumber, newContact, updatePIMData },
                    "Contact source location can not be changed currently.");
            return 0;
        }
        // *********************************************************************
        // User batch mode to add all data of the contact
        mOpBatch.clear();
        final List<ContactData> allData = newContact.getAllContactData();
        final StringBuffer newIdSet = new StringBuffer();
        int contactDataIndex = -1;
        // Has no data to insert
        if (allData == null) {
            return updateCount;
        }
        // Has data to insert
        int i = 0;
        for (final ContactData data : allData) {
            ++i;
            // Ensure the rawContactId in data is correct
            data.setRawContactId(id);
//            // / For Contact provider's change, clear all names except DisplayName
              // before update structuredName {@ 2012-12-5
            switch (sourceLocation) {
            case RawContact.SOURCE_SIM:
            case RawContact.SOURCE_SIM1:
            case RawContact.SOURCE_SIM2:
            case RawContact.SOURCE_SIM3:
            case RawContact.SOURCE_SIM4:
                Debugger.logW(new Object[] { id, sourceLocation, simName, }, "update SIM contact data");
                mOpBatch.appendSimContactDataInsert(data);
                break;
            case RawContact.SOURCE_PHONE:
                mOpBatch.appendContactDataInsert(data);
                break;
            default:
                Debugger.logW(new Object[] { id, sourceLocation, simName, simNumber, newContact, updatePIMData },
                        "Contact source location can not be changed currently.");
            }
            // // / @}
            // Operations batch has reached the max count,
            // or all data have been read,
            // We need to apply the batch immediately and then release memory
            if (mOpBatch.isFull() || i == allData.size()) {
                ContentProviderResult[] dataResults = null;
                try {
                    dataResults = mOpBatch.apply();
                } catch (final RemoteException e) {
                    Debugger.logE(new Object[] { id, sourceLocation, simName,
                            simNumber, newContact, updatePIMData }, null, e);
                    return updateCount;
                } catch (final OperationApplicationException e) {
                    Debugger.logE(new Object[] { id, sourceLocation, simName,
                            simNumber, newContact, updatePIMData }, null, e);
                    return updateCount;
                } finally {
                    // Clear operations
                    mOpBatch.clear();
                }

                if (dataResults != null) {
                    for (final ContentProviderResult result : dataResults) {
                        try {
                            ++contactDataIndex;
                            if (result == null) {
                                continue;
                            }
                            // The _id of the record in Data table
                            final long dataId = Long.parseLong(result.uri
                                    .getLastPathSegment());
                            newIdSet.append(dataId + ",");
                            final ContactData currentData = allData
                                    .get(contactDataIndex);
                            if (currentData != null) {
                                // Set the returned id to contact data object,
                                // as the insertion of the data is successful
                                currentData.setId(dataId);
                            }
                        } catch (final NumberFormatException e) {
                            Debugger.logE(new Object[] { id, sourceLocation,
                                    simName, simNumber, newContact,
                                    updatePIMData },
                                    "Exception occurs when trying to get "
                                            + "inserted id of contact data", e);
                        }
                    }
                }
            }

        }

        // 3. Delete all old data
        final StringBuffer selection = new StringBuffer();
        selection.append(Data.RAW_CONTACT_ID + "=" + id);
        newIdSet.deleteCharAt(newIdSet.length() - 1);
        if (newIdSet.length() > 0) {
            selection.append(" AND " + Data._ID + " NOT IN(" + newIdSet + ")");
        }
        int deleteCount = getObservedContentResolver().delete(Data.CONTENT_URI
        /*
         * // Delete physically, not just set DELETED=1 .buildUpon()
         * .appendQueryParameter(ContactsContract.CALLER_IS_SYNCADAPTER, "true")
         * .build()
         */, selection.toString(), null);
        Debugger.logI(new Object[] { id, sourceLocation, simName, simNumber, newContact, updatePIMData },
                "Delete data count" + deleteCount);
        return updateCount;
    }

    /**
     * Update a contact on phone. Update will follow 3 steps:
     * <p>
     * 1. Update the base info columns of raw contact table
     * <p>
     * 2. Delete all current data of the specified contact object (If is
     * required by invoker)
     * <p>
     * 3. Insert all new data passed in the specified contact object (If is
     * required by invoker)
     * 
     * @param id
     *            The unique database id of the contact to update.
     * @param sourceLocation
     *            The location of the contact.
     * @param simName
     *            The name of the sim.
     * @param simNumber
     *            The number of the sim.
     * @param simEmail
     *            The email used to update.
     * @param newContact
     *            The contact object encapsulating all new data of the contact.
     * @param updatePIMData
     *            If true, all corresponding PIM data will also be updated.
     * @return How many contacts are updated.
     */
    public int updateContact(final long id, final int sourceLocation,
            final String simName, final String simNumber,
            final String simEmail, final RawContact newContact,
            final boolean updatePIMData) {
        if (newContact == null) {
            Debugger.logW(new Object[] { id, sourceLocation, simName,
                    simNumber, simEmail, newContact, updatePIMData },
                    "New contact passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(2);

        // 1. Update raw contact base columns
        // Add content values for the raw contact update
        values.put(RawContacts.STARRED, newContact.isStarred() ? 1 : 0); // Modify
        // by
        // Yu
        // 2011-12-20
        values.put(RawContacts.SEND_TO_VOICEMAIL, newContact
                .isSendToVoicemail());
        /*
         * values.put(RawContacts.CUSTOM_RINGTONE,
         * newContact.getCustomRingtone());
         */

        updateCount = getObservedContentResolver().update(
                RawContacts.CONTENT_URI,
                values,
                RawContacts._ID + "=" + id + " AND " + RawContacts.DELETED
                        + "<>" + DatabaseRecordEntity.TRUE, null);

        // If do not update PIM data or update RawContact failed, return by here
        if (!updatePIMData || updateCount < 1) {
            return updateCount;
        }

        // 2. Insert all new data
        // For MTK SIM Contacts feature ****************************************
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            // This contact should be updated in SIM before updated in database
            int simUpdateResult = 0;

            String newSimName = "";
            String newSimNumber = "";
            String newSimEmail = "";
            if (newContact.getNames() != null
                    && newContact.getNames().size() > 0
                    && newContact.getNames().get(0).getDisplayName() != null) {
                //newSimName = newContact.getNames().get(0).getGivenName();
                /** provider changed 2012-12-5*/
              newSimName = newContact.getNames().get(0).getDisplayName();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM name");
            }

            if (newContact.getPhones() != null
                    && newContact.getPhones().size() > 0
                    && newContact.getPhones().get(0).getNumber() != null) {
                newSimNumber = newContact.getPhones().get(0).getNumber();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM number");
            }

            if (newContact.getEmails() != null
                    && newContact.getEmails().size() > 0
                    && newContact.getEmails().get(0).getData() != null) {
                newSimEmail = newContact.getEmails().get(0).getData();
            } else {
                Debugger.logW(new Object[] { newSimName }, "No new SIM email");
            }

            //int slotId = getSlotId(sourceLocation);
            boolean isUSIM = USIMUtils.isSimUsimType(sourceLocation);
            if (newContact.getEmails().size() > 0 && isUSIM) {
                simUpdateResult = updateSimContact(simName, simNumber,
                        simEmail, newSimName, newSimNumber, newSimEmail, sourceLocation);
            } else {
                simUpdateResult = updateSimContact(simName, simNumber,
                        newSimName, newSimNumber, sourceLocation);
            }

            if (simUpdateResult < 1) {
                // Update in SIM failed, so cancel update in database
                Debugger.logE(new Object[] { id, sourceLocation, simName,
                        simNumber, newSimName, newSimNumber, updatePIMData },
                        "Failed to update contact in SIM.");
                newContact.setId(DatabaseRecordEntity.ID_NULL);
                return 0;
            }
            break;
        case RawContact.SOURCE_PHONE:
            break;
        default:
            Debugger.logW(new Object[] { id, sourceLocation, simName,
                    simNumber, newContact, updatePIMData },
                    "Contact source location can not be changed currently.");
            return 0;
        }
        // *********************************************************************
        // User batch mode to add all data of the contact
        mOpBatch.clear();
        final List<ContactData> allData = newContact.getAllContactData();
        final StringBuffer newIdSet = new StringBuffer();
        int contactDataIndex = -1;
        // Has no data to insert
        if (allData == null) {
            return updateCount;
        }
        // Has data to insert
        int i = 0;
        for (final ContactData data : allData) {
            ++i;
            // Ensure the rawContactId in data is correct
            data.setRawContactId(id);
            // / For Contact provider's change, clear all names except DisplayName before update structuredName {@ 2012-12-5
            
            switch (sourceLocation) {
            case RawContact.SOURCE_SIM:
            case RawContact.SOURCE_SIM1:
            case RawContact.SOURCE_SIM2:
            case RawContact.SOURCE_SIM3:
            case RawContact.SOURCE_SIM4:
                Debugger.logW(new Object[] { id, sourceLocation, simName, }, "update SIM contact data");
                mOpBatch.appendSimContactDataInsert(data);
                break;
            case RawContact.SOURCE_PHONE:
                mOpBatch.appendContactDataInsert(data);
                break;
            default:
                Debugger.logW(new Object[] { id, sourceLocation, simName, simNumber, newContact, updatePIMData },
                        "Contact source location can not be changed currently.");
            }
            // / @}
            // Operations batch has reached the max count,
            // or all data have been read,
            // We need to apply the batch immediately and then release memory
            if (mOpBatch.isFull() || i == allData.size()) {
                ContentProviderResult[] dataResults = null;
                try {
                    dataResults = mOpBatch.apply();
                } catch (final RemoteException e) {
                    Debugger.logE(new Object[] { id, sourceLocation, simName,
                            simNumber, newContact, updatePIMData }, null, e);
                    return updateCount;
                } catch (final OperationApplicationException e) {
                    Debugger.logE(new Object[] { id, sourceLocation, simName,
                            simNumber, newContact, updatePIMData }, null, e);
                    return updateCount;
                } finally {
                    // Clear operations
                    mOpBatch.clear();
                }

                if (dataResults != null) {
                    for (final ContentProviderResult result : dataResults) {
                        try {
                            ++contactDataIndex;
                            if (result == null) {
                                continue;
                            }
                            // The _id of the record in Data table
                            final long dataId = Long.parseLong(result.uri
                                    .getLastPathSegment());
                            newIdSet.append(dataId + ",");
                            final ContactData currentData = allData
                                    .get(contactDataIndex);
                            if (currentData != null) {
                                // Set the returned id to contact data object,
                                // as the insertion of the data is successful
                                currentData.setId(dataId);
                            }
                        } catch (final NumberFormatException e) {
                            Debugger.logE(new Object[] { id, sourceLocation,
                                    simName, simNumber, newContact,
                                    updatePIMData },
                                    "Exception occurs when trying to get "
                                            + "inserted id of contact data", e);
                        }
                    }
                }
            }

        }

        // 3. Delete all old data
        final StringBuffer selection = new StringBuffer();
        selection.append(Data.RAW_CONTACT_ID + "=" + id);
        newIdSet.deleteCharAt(newIdSet.length() - 1);
        if (newIdSet.length() > 0) {
            selection.append(" AND " + Data._ID + " NOT IN(" + newIdSet + ")");
        }
        int deleteCount = getObservedContentResolver().delete(Data.CONTENT_URI
        /*
         * // Delete physically, not just set DELETED=1 .buildUpon()
         * .appendQueryParameter(ContactsContract.CALLER_IS_SYNCADAPTER, "true") .build()
         */, selection.toString(), null);

        Debugger.logI(new Object[] { id, sourceLocation, simName, simNumber, newContact, updatePIMData },
                "Delete old data count" + deleteCount);
        return updateCount;
    }

    /**
     * Delete a contact on phone. Also delete its corresponding data records.
     * 
     * @param id
     *            The unique database id of the contact to delete.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @param sourceLocation
     *            The location of the contact.
     * @param simName
     *            The name of the sim.
     * @param simNumber
     *            The number of the sim.
     * @return How many contacts are deleted.
     */
    public int deleteContact(final long id, final boolean permanently,
            final int sourceLocation, final String simName,
            final String simNumber) {
        int deleteCount = 0;
        Uri deleteUri;
        // For MTK SIM Contacts feature
        // **********************************************
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            // This contact should be deleted in SIM before deleted in database
            int simDeletionResult = 0;
            try {
                simDeletionResult = deleteSimContact(simName, simNumber, sourceLocation);
                // Modified
                // by
                // Shaoying
                // Han
                // getSimId(sourceLocation));
            } catch (final NullPointerException e) {
                Debugger.logE(new Object[] { id, permanently, sourceLocation,
                        simName, simNumber }, null, e);
            }
            if (simDeletionResult < 1) {
                // Deletion in SIM failed, so cancel deletion in database
                Debugger.logE(new Object[] { id, permanently, sourceLocation,
                        simName, simNumber },
                        "Failed to delete contact in SIM.");
                return 0;
            }
            break;
        case RawContact.SOURCE_PHONE:
            break;
        default:
            Debugger.logE(new Object[] { id, permanently, sourceLocation,
                    simName, simNumber }, "Invalid source location.");
            return 0;
        }
        // *********************************************************************
        if (permanently) {
            deleteUri = RawContacts.CONTENT_URI
                    // Delete physically, not just set DELETED=1
                    .buildUpon().appendQueryParameter(
                            ContactsContract.CALLER_IS_SYNCADAPTER, "true")
                    .build();
        } else {
            deleteUri = RawContacts.CONTENT_URI;
        }

        deleteCount = getObservedContentResolver().delete(deleteUri,
                RawContacts._ID + "=" + id, null);

        return deleteCount;
    }

    /**
     * @param ids
     *            The array of the id which to be deleted.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @param sourceLocation
     *            The location of the contact.
     * @param simNames
     *            The name of the sim.
     * @param simNumbers
     *            the number of the sim.
     * @return A array of whether success to delete the contact with id.
     */
    public boolean[] deleteContacts(final long[] ids,
            final boolean permanently, final int sourceLocation,
            final String[] simNames, final String[] simNumbers) {
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            return this.deleteContactsSourcedOnSim(ids, permanently,
                    sourceLocation, simNames, simNumbers);
        case RawContact.SOURCE_PHONE:
            return this.deleteContactsSourcedOnPhone(ids, permanently);
        default:
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers }, "Invalid source location "
                    + sourceLocation + ".");
            return null;
        }
    }

    /**
     * @param ids
     *            The array of the id which to be deleted.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @param sourceLocation
     *            The location of the contact.
     * @param simNames
     *            The array of the sim name.
     * @param simNumbers
     *            The array of the sim number.
     * @param emails
     *            The email The array of the email to delete.
     * @return List of deletion results(successful or not) or null.
     */
    public boolean[] deleteContacts(final long[] ids,
            final boolean permanently, final int sourceLocation,
            final String[] simNames, final String[] simNumbers,
            final String[] emails) {
        switch (sourceLocation) {
        case RawContact.SOURCE_SIM:
        case RawContact.SOURCE_SIM1:
        case RawContact.SOURCE_SIM2:
        case RawContact.SOURCE_SIM3:
        case RawContact.SOURCE_SIM4:
            //final int slotId = getSlotId(sourceLocation);
            final boolean isUSIM = USIMUtils.isSimUsimType(sourceLocation);
            if (null != emails && emails.length > 0 && isUSIM) {
                return this.deleteContactsSourcedOnSim(ids, permanently,
                        sourceLocation, simNames, simNumbers, emails);
            } else {
                return this.deleteContactsSourcedOnSim(ids, permanently,
                        sourceLocation, simNames, simNumbers);
            }
        case RawContact.SOURCE_PHONE:
            return this.deleteContactsSourcedOnPhone(ids, permanently);
        default:
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers, emails }, "Invalid source location "
                    + sourceLocation + ".");
            return null;
        }
    }

    /**
     * @param ids
     *            The array of the id of contact which to be deleted in sim.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @param sourceLocation
     *            The location of the contact.
     * @param simNames
     *            The name of the sim.
     * @param simNumbers
     *            The number of the sim.
     * @return A array of whether success to delete the contact with id in sim.
     */
    public boolean[] deleteContactsSourcedOnSim(final long[] ids,
            final boolean permanently, final int sourceLocation,
            final String[] simNames, final String[] simNumbers) {
        if (null == ids || null == simNames || null == simNumbers) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers }, "List is null.");
            return null;
        }
        if ((ids.length != simNames.length)
                || (ids.length != simNumbers.length)) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers },
                    "List size does not match each other.");
            return null;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers }, "List is empty.");
            return null;
        }

        final boolean[] results = new boolean[ids.length];
        final ArrayList<Long> simOKIds = new ArrayList<Long>(ids.length);
        final ArrayList<Integer> simOKPositions = new ArrayList<Integer>(
                ids.length);
        // Delete contacts in SIM first
        for (int i = 0; i < ids.length; i++) {
            final int count = deleteSimContact(simNames[i], simNumbers[i], sourceLocation); // Modified
            // by
            // Shaoying
            // Han
            // getSimId(sourceLocation));
            if (count >= 1) {
                // Record the contact id if it's successfully deleted in SIM
                simOKIds.add(ids[i]);
                // Also record the position in id array
                simOKPositions.add(i);
            }
        }
        // Next delete those contacts deleted in SIM in database
        if (simOKIds.size() > 0) {
            final long[] phoneDeletionIds = new long[simOKIds.size()];
            int i = -1;
            for (final Long id : simOKIds) {
                phoneDeletionIds[++i] = id;
            }
            // Do deletion in database
            final boolean[] phoneResults = deleteContactsSourcedOnPhone(
                    phoneDeletionIds, permanently);
            for (int j = 0; j < phoneResults.length; j++) {
                if (phoneResults[j]) {
                    // Set true in the final result array to return
                    results[simOKPositions.get(j)] = true;
                }
            }
        }

        return results;
    }

    /**
     * @param ids
     *            The array of contact id.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @param sourceLocation
     *            The location of the contact.
     * @param simNames
     *            The array of the sim name.
     * @param simNumbers
     *            The array of the sim number.
     * @param emails
     *            The array of the email to delete.
     * @return A array of whether success to delete the contact with id in sim
     *         or null.
     */
    public boolean[] deleteContactsSourcedOnSim(final long[] ids,
            final boolean permanently, final int sourceLocation,
            final String[] simNames, final String[] simNumbers,
            final String[] emails) {
        if (null == ids || null == simNames || null == simNumbers
                || null == emails) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers, emails }, "List is null.");
            return null;
        }
        if ((ids.length != simNames.length)
                || (ids.length != simNumbers.length)
                || (ids.length != emails.length)) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers, emails },
                    "List size does not match each other.");
            return null;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, permanently, sourceLocation,
                    simNames, simNumbers, emails }, "List is empty.");
            return null;
        }

        final boolean[] results = new boolean[ids.length];
        final ArrayList<Long> simOKIds = new ArrayList<Long>(ids.length);
        final ArrayList<Integer> simOKPositions = new ArrayList<Integer>(
                ids.length);
        // Delete contacts in SIM first
        for (int i = 0; i < ids.length; i++) {
            final int count = deleteSimContact(simNames[i], simNumbers[i],
                    emails[i], sourceLocation); // Modified
            // by
            // Shaoying
            // Han
            // getSimId(sourceLocation));
            if (count >= 1) {
                // Record the contact id if it's successfully deleted in SIM
                simOKIds.add(ids[i]);
                // Also record the position in id array
                simOKPositions.add(i);
            }
        }
        // Next delete those contacts deleted in SIM in database
        if (simOKIds.size() > 0) {
            final long[] phoneDeletionIds = new long[simOKIds.size()];
            int i = -1;
            for (final Long id : simOKIds) {
                phoneDeletionIds[++i] = id;
            }
            // Do deletion in database
            final boolean[] phoneResults = deleteContactsSourcedOnPhone(
                    phoneDeletionIds, permanently);
            for (int j = 0; j < phoneResults.length; j++) {
                if (phoneResults[j]) {
                    // Set true in the final result array to return
                    results[simOKPositions.get(j)] = true;
                }
            }
        }

        return results;
    }

    /**
     * Delete contacts on phone. Also delete their corresponding data records.
     * This multiple-records delete implementation use batch SQL apply way.
     * 
     * @param ids
     *            List of the database id of the contacts to delete.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteContactsSourcedOnPhone(final long[] ids,
            final boolean permanently) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, permanently }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple deletion
        mOpBatch.clear();
        final DefaultDeleteBatchHelper batchHelper = new DefaultDeleteBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.deleteContact$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendRawContactDelete(
                        ids[appendPosition], permanently);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(ids.length);

        return batchHelper.getResults();
    }

    /**
     * Delete contacts on phone. Also delete their corresponding data records.
     * This multiple-records delete implementation use single SQL with 'IN'.
     * <p>
     * NOTE: Delete via this way is faster, however, it is atomic, which means
     * it only returns the actual deleted rows count. Caller cannot know which
     * contacts are deleted, which ones failed exactly.
     * 
     * @param ids
     *            List of the database id of the contacts to delete.
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @return List of deletion results(successful or not).
     */
    public int fastDeleteContactsSourcedOnPhone(final long[] ids,
            final boolean permanently) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, permanently }, "List is null.");
            return 0;
        }

        int deleteCount = 0;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < ids.length; i++){ strBuf.append(RawContacts._ID +
         * "=" + ids[i] + " OR "); } if (strBuf.length() > 4){ selection =
         * strBuf.substring(0, strBuf.length() - 4); }
         */
        // Expression tree depth won't beyond 1000 in this way
        strBuf.append(RawContacts._ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();

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

        deleteCount = getObservedContentResolver().delete(deleteUri, selection,
                null);

        return deleteCount;
    }

    /**
     * Delete all contacts on phone.
     * 
     * @param permanently
     *            If true, contacts will be deleted permanently. Otherwise, it
     *            will just be updated with 'DELETED=1'.
     * @return How many contacts are deleted.
     */
    public int deleteAllContacts(final boolean permanently) {
        int deleteCount = 0;
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
        deleteCount = getObservedContentResolver().delete(
                deleteUri,
                // For MTK SIM Contacts feature
                RawContactsContent.COLUMN_SOURCE_LOCATION + " = "
                        + RawContact.SOURCE_PHONE, null);
        Debugger.logI(new Object[] {}, "deleteCount >>:" + deleteCount);
        return deleteCount;
    }

    // for backup
    /**
     * @return the numbers the rows updated.
     */
    public int updateAllContactForBackup() {

        int updateCount = 0;
        final ContentValues values = new ContentValues();
        values.put(RawContacts.DELETED, 1);
        updateCount = getObservedContentResolver().update(
                RawContacts.CONTENT_URI,
                values,
                RawContactsContent.COLUMN_SOURCE_LOCATION + " = "
                        + RawContact.SOURCE_PHONE + " AND "
                        + RawContacts.DELETED + " = " + "0", null);
        Debugger.logE(new Object[] {}, "updateCount >>:" + updateCount);
        return updateCount;

    }

    /**
     * @return List of deletion results(successful or not).
     */
    public int deleteContactForBackup() {
        // int deleteCount = 0;
        int temp = 0;
        final Cursor c = getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID },
                RawContactsContent.COLUMN_SOURCE_LOCATION + " = "
                        + RawContact.SOURCE_PHONE + " AND "
                        + RawContacts.DELETED + " = " + "0", null, null);
        long[] ids = null;
        if (null != c) {
            ids = new long[c.getCount()];
            while (c.moveToNext()) {
                // getObservedContentResolver().delete(RawContacts.CONTENT_URI,
                // RawContacts._ID + " = " + c.getLong(0), null);
                // deleteCount++;
                ids[temp] = c.getLong(0);
                temp++;
            }
            c.close();
        }

        deleteContactsSourcedOnPhone(ids, false);

        return temp;
    }

    // Faster, but not so safe
    /**
     * @param raw
     *            The array of the raw bytes of the contact.
     * @param rawContactsConsumer
     *            Set a rawContactsConsumer to handle asynchronous blocks.
     * @param contactDataConsumer
     *            Set a contactDataConsumer to handle asynchronous blocks.
     * @param outBuffer
     *            The buffer to save the contact.
     */
    public void fastImportDetailedContacts(final byte[] raw,
            final IRawBlockConsumer rawContactsConsumer,
            final IRawBlockConsumer contactDataConsumer,
            final ByteBuffer outBuffer) {
        if ((null == rawContactsConsumer) || (null == contactDataConsumer)) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == raw) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer }, "Raw data is null.");
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        final ByteBuffer inBuffer = ByteBuffer.wrap(raw);
        // Contacts count in the raw data
        int count;
        long beginId;
        try {
            // The first 4 bytes tell contacts count in the raw data.
            count = inBuffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Can not get the contacts count in raw data", e);
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        if (count < 0) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer }, "Invalid contacts count "
                    + count);
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // beginId = getMaxRawContactsId() + 1;
        final DefaultBulkInsertHelper contactDataInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(Data.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert contact data failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper rawContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(RawContacts.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        final DefaultBulkInsertHelper simContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim1ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM1), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim2ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM2), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim3ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM3), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        
        final DefaultBulkInsertHelper sim4ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM4), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "fastImportDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        boolean shouldExit = false;

        final int[] contactsSimId = new int[count];
        beginId = getMaxRawContactsIdByQuery() + 1;

        final long[] contactsSimIndex = new long[count];
        // Insert contact data -------------------------------------------------
        for (int i = 0; i < count; i++) {
            // Read a raw contact(with contact data) from raw bytes
            final RawContact newContact = new RawContact();
            // newContact.readRaw(inBuffer); Changed by Shaoying Han
            newContact.readRawWithVersion(inBuffer, Config.VERSION_CODE);
            Debugger.logI("``````````````````newContact.getSourceLocation()"
                    + newContact.getSourceLocation());
            if (newContact.getSourceLocation() == RawContact.SOURCE_PHONE) {
                contactsSimId[i] = newContact.getSourceLocation();
            } else {
                contactsSimId[i] = Global.getSimInfoBySlot(
                        getRealSlotId(newContact.getSourceLocation()))
                        .getSimId();
            }

            String simName = null;
            String simNumber = null;
            if (newContact.getNames() != null
                    && newContact.getNames().size() > 0
                    && newContact.getNames().get(0).getDisplayName() != null) {
                simName = newContact.getNames().get(0).getDisplayName();
            } else {
                simName = "";
                Debugger.logW(new Object[] {}, "No SIM contact name");
            }

            if (newContact.getPhones() != null
                    && newContact.getPhones().size() > 0
                    && newContact.getPhones().get(0).getNumber() != null) {
                simNumber = newContact.getPhones().get(0).getNumber();
                simNumber = PhoneNumberUtils.stripSeparators(simNumber);
            } else {
                simNumber = "";
                Debugger.logW(new Object[] {}, "No SIM contact number");
            }
            MeasuredContentValues values = new MeasuredContentValues(2);
            values.put(SimContactsContent.COLUMN_TAG, simName);
            values.put(SimContactsContent.COLUMN_NUMBER, simNumber);

            /*if (newContact.getSourceLocation() == RawContact.SOURCE_SIM) {
                shouldExit = !simContactsInserter.append(values);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM1) {
                shouldExit = !sim1ContactsInserter.append(values);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM2) {
                shouldExit = !sim2ContactsInserter.append(values);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM3) {
                shouldExit = !sim3ContactsInserter.append(values);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM4) {
                shouldExit = !sim4ContactsInserter.append(values);
            } else */
            if (newContact.getSourceLocation() == RawContact.SOURCE_PHONE) {
                // Phone contact
                Debugger.logI("source location " + RawContact.SOURCE_PHONE);
            } else {
            	long simInsertionResult = SimContactsContent.ERR_ICC_UNKNOWN;
                String newSimName = "";
                String newSimNumber = "";
                String newSimEmail = "";
                if (newContact.getNames() != null
                        && newContact.getNames().size() > 0
                        && newContact.getNames().get(0).getDisplayName() != null) {
                    newSimName = newContact.getNames().get(0).getDisplayName();
                } else {
                    Debugger.logW(new Object[] { newSimName }, "No new SIM name");
                }

                if (newContact.getPhones() != null
                        && newContact.getPhones().size() > 0
                        && newContact.getPhones().get(0).getNumber() != null) {
                    newSimNumber = newContact.getPhones().get(0).getNumber();
                } else {
                    Debugger.logW(new Object[] { newSimName }, "No new SIM number");
                }

                if (newContact.getEmails() != null
                        && newContact.getEmails().size() > 0
                        && newContact.getEmails().get(0).getData() != null) {
                    newSimEmail = newContact.getEmails().get(0).getData();
                } else {
                    Debugger.logW(new Object[] { newSimName }, "No new SIM email");
                }

                int sourceLocation = newContact.getSourceLocation();
                boolean isUSIM = USIMUtils.isSimUsimType(sourceLocation);
                if (isUSIM) {
                    simInsertionResult = insertSimContact(newSimName, newSimNumber,
                            newSimEmail, sourceLocation);
                } else {
                    simInsertionResult = insertSimContact(newSimName, newSimNumber,
                            sourceLocation);
                }

                if (simInsertionResult < 0) {
                    // Insertion in SIM failed, so cancel insertion in database
                    Debugger.logE(new Object[] { newContact},
                            "Failed to insert contact into SIM.");
                    newContact.setId(DatabaseRecordEntity.ID_NULL);
                    // return DatabaseRecordEntity.ID_NULL;
                    return;
                } else {
                    // insert sim index to db.
                    //values.put(RawContactsContent.COLUMN_INDEX_IN_SIM, simInsertionResult);
                	contactsSimIndex[i] = simInsertionResult;
                }
                Debugger.logE(new Object[] {}, "SourceLocation is :"
                        + newContact.getSourceLocation());
            }

            for (final ContactData data : newContact.getAllContactData()) {
                data.setRawContactId(beginId + i);
                shouldExit = !contactDataInserter.append(ContactDataContent
                        .createMeasuredContentValues(data, false));
                if (shouldExit) {
                    Debugger.logE(new Object[] { raw, rawContactsConsumer,
                            contactDataConsumer, outBuffer },
                            "Error in bulk inserting contact data, "
                                    + "statusCode: "
                                    + contactDataInserter.getStatusCode());
                    rawContactsConsumer.consume(null, 0, 0);
                    contactDataConsumer.consume(null, 0, 0);
                    return;
                }
            }
        }

        // Bulk insert sim contacts left
        shouldExit = !simContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert sim1 contacts left
        shouldExit = !sim1ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim1 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert sim2 contacts left
        shouldExit = !sim2ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim2 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }
        // Bulk insert sim3 contacts left
        shouldExit = !sim3ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim3 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }
        
        // Bulk insert sim4 contacts left
        shouldExit = !sim4ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim4 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Insert raw contacts -------------------------------------------------
        for (int i = 0; i < count; i++) {
            // Add content values for the raw contact insertion
            //final MeasuredContentValues values = new MeasuredContentValues(5);
            final MeasuredContentValues values = new MeasuredContentValues(6);
            values.put(RawContacts._ID, beginId + i);
            // Disable aggregation
            values.put(RawContacts.AGGREGATION_MODE,
                    RawContacts.AGGREGATION_MODE_DISABLED);

            int sourceLocation = contactsSimId[i];
            // SourceLocation
            values.put(RawContactsContent.COLUMN_SOURCE_LOCATION,
                    sourceLocation);

            String accountName = USIMUtils.ACCOUNT_NAME_LOCAL_PHONE;
            String accountType = USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE;
            if (sourceLocation == RawContact.SOURCE_PHONE) {
                accountName = USIMUtils.ACCOUNT_NAME_LOCAL_PHONE;
                accountType = USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE;
            } else if (sourceLocation == RawContact.SOURCE_SIM) {
                accountName = USIMUtils
                        .getSimAccountNameBySlot(USIMUtils.SINGLE_SLOT);
                accountType = USIMUtils
                        .getAccountTypeBySlot(USIMUtils.SINGLE_SLOT);
            } else if (sourceLocation == RawContact.SOURCE_SIM1) {
                accountName = USIMUtils
                        .getSimAccountNameBySlot(USIMUtils.GEMINI_SLOT1);
                accountType = USIMUtils
                        .getAccountTypeBySlot(USIMUtils.GEMINI_SLOT1);
            } else if (sourceLocation == RawContact.SOURCE_SIM2) {
                accountName = USIMUtils
                        .getSimAccountNameBySlot(USIMUtils.GEMINI_SLOT2);
                accountType = USIMUtils
                        .getAccountTypeBySlot(USIMUtils.GEMINI_SLOT2);
            }  else if (sourceLocation == RawContact.SOURCE_SIM3) {
                accountName = USIMUtils
                	.getSimAccountNameBySlot(USIMUtils.GEMINI_SLOT3);
                accountType = USIMUtils
                	.getAccountTypeBySlot(USIMUtils.GEMINI_SLOT3);
            }  else if (sourceLocation == RawContact.SOURCE_SIM4) {
                accountName = USIMUtils
                	.getSimAccountNameBySlot(USIMUtils.GEMINI_SLOT4);
                accountType = USIMUtils
                	.getAccountTypeBySlot(USIMUtils.GEMINI_SLOT4);
            } else {
                Debugger.logE("source location is unkown");
            }
            values.put(RawContacts.ACCOUNT_NAME, accountName);
            values.put(RawContacts.ACCOUNT_TYPE, accountType);
            values.put(RawContactsContent.COLUMN_INDEX_IN_SIM, contactsSimIndex[i]);

            shouldExit = !rawContactsInserter.append(values);
            if (shouldExit) {
                Debugger.logE(new Object[] { raw, rawContactsConsumer,
                        contactDataConsumer, outBuffer },
                        "Error in bulk inserting raw contacts, "
                                + "statusCode: "
                                + rawContactsInserter.getStatusCode());
                rawContactsConsumer.consume(null, 0, 0);
                contactDataConsumer.consume(null, 0, 0);
                return;
            }
        }
        // Bulk insert all ContentValues left
        shouldExit = !rawContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting raw contacts, " + "statusCode: "
                            + rawContactsInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert all ContentValues left
        shouldExit = !contactDataInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting contact data, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        asyncGetAllRawContacts(rawContactsConsumer, outBuffer, beginId, beginId
                + count);
        asyncGetAllContactData(null, contactDataConsumer, outBuffer, null,
                null, beginId, beginId + count);
    }

    /**
     * @param raw
     *            The array of raw byte representing the contact.
     * @param rawContactsConsumer
     *            Set a rawContactsConsumer to handle asynchronous blocks.
     * @param contactDataConsumer
     *            Set a contactDataConsumer to handle asynchronous blocks.
     * @param outBuffer
     *            The buffer to save the contact.
     */
    public void restoreDetailedContacts(final byte[] raw,
            final IRawBlockConsumer rawContactsConsumer,
            final IRawBlockConsumer contactDataConsumer,
            final ByteBuffer outBuffer) {
        if ((null == rawContactsConsumer) || (null == contactDataConsumer)) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == raw) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer }, "Raw data is null.");
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        final ByteBuffer inBuffer = ByteBuffer.wrap(raw);
        // Contacts count in the raw data
        int count;
        long beginId;
        try {
            // The first 4 bytes tell contacts count in the raw data.
            count = inBuffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Can not get the contacts count in raw data", e);
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        if (count < 0) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer }, "Invalid contacts count "
                    + count);
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        beginId = getMaxRawContactsId() + 1;
        final DefaultBulkInsertHelper contactDataInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(Data.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert contact data failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper rawContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(RawContacts.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper simContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim1ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM1), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim2ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM2), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper sim3ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM3), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        
        final DefaultBulkInsertHelper sim4ContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(getSimUri(DataStoreLocations.SIM4), values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "restoreDetailedContacts",
                            new Object[] { raw, rawContactsConsumer,
                                    contactDataConsumer, outBuffer },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        boolean shouldExit = false;

        final int[] contactsSimId = new int[count];

        // Insert contact data -------------------------------------------------
        for (int i = 0; i < count; i++) {
            // Read a raw contact(with contact data) from raw bytes
            final RawContact newContact = new RawContact();
            // newContact.readRaw(inBuffer); Changed by Shaoying Han
            newContact.readRawWithVersion(inBuffer, Config.VERSION_CODE);
            Debugger.logI("restore contact sourceLocation :"
                    + newContact.getSourceLocation());
            if (newContact.getSourceLocation() == RawContact.SOURCE_PHONE) {
                contactsSimId[i] = newContact.getSourceLocation();
            } else {
                contactsSimId[i] = Global.getSimInfoBySlot(
                        getRealSlotId(newContact.getSourceLocation()))
                        .getSimId();
            }

            String simName = null;
            String simNumber = null;
            if (newContact.getNames() != null
                    && newContact.getNames().size() > 0
                    && newContact.getNames().get(0).getDisplayName() != null) {
                simName = newContact.getNames().get(0).getDisplayName();
            } else {
                simName = "";
                Debugger.logW(new Object[] {}, "No SIM contact name");
            }

            if (newContact.getPhones() != null
                    && newContact.getPhones().size() > 0
                    && newContact.getPhones().get(0).getNumber() != null) {
                simNumber = newContact.getPhones().get(0).getNumber();
                simNumber = PhoneNumberUtils.stripSeparators(simNumber);
            } else {
                simNumber = "";
                Debugger.logW(new Object[] {}, "No SIM contact number");
            }
            MeasuredContentValues simValues = new MeasuredContentValues(2);
            simValues.put(SimContactsContent.COLUMN_TAG, simName);
            simValues.put(SimContactsContent.COLUMN_NUMBER, simNumber);

            if (newContact.getSourceLocation() == RawContact.SOURCE_SIM) {
                shouldExit = !simContactsInserter.append(simValues);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM1) {
                shouldExit = !sim1ContactsInserter.append(simValues);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM2) {
                shouldExit = !sim2ContactsInserter.append(simValues);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM3) {
                shouldExit = !sim3ContactsInserter.append(simValues);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_SIM4) {
                shouldExit = !sim4ContactsInserter.append(simValues);
            } else if (newContact.getSourceLocation() == RawContact.SOURCE_PHONE) {
                // Phone contact
                Debugger.logI("source location " + RawContact.SOURCE_PHONE);
            } else {
                Debugger.logE(new Object[] {}, "SourceLocation is :"
                        + newContact.getSourceLocation());
            }

            for (final ContactData data : newContact.getAllContactData()) {
                data.setRawContactId(beginId + i);
                shouldExit = !contactDataInserter.append(ContactDataContent
                        .createMeasuredContentValues(data, false));
                if (shouldExit) {
                    Debugger.logE(new Object[] { raw, rawContactsConsumer,
                            contactDataConsumer, outBuffer },
                            "Error in bulk inserting contact data, "
                                    + "statusCode: "
                                    + contactDataInserter.getStatusCode());
                    rawContactsConsumer.consume(null, 0, 0);
                    contactDataConsumer.consume(null, 0, 0);
                    return;
                }
            }

            // modify by Yu
            final MeasuredContentValues values = new MeasuredContentValues(10);
            values.put(RawContacts._ID, beginId + i);
            // Disable aggregation
            values.put(RawContacts.AGGREGATION_MODE,
                    RawContacts.AGGREGATION_MODE_DISABLED);
            // SourceLocation
            values.put(RawContactsContent.COLUMN_SOURCE_LOCATION,
                    contactsSimId[i]);

            values.put(RawContacts.STARRED, newContact.isStarred() ? 1 : 0);
            // values.put(RawContacts.CONTACT_ID, beginId + i);
            values.put(RawContacts.SEND_TO_VOICEMAIL, newContact
                    .isSendToVoicemail() ? 1 : 0);
            values.put(RawContacts.LAST_TIME_CONTACTED, newContact
                    .getLastTimeContacted());
            values.put(RawContacts.TIMES_CONTACTED, newContact
                    .getTimesContacted());
            values.put(RawContacts.CUSTOM_RINGTONE, newContact
                    .getCustomRingtone());
            // values.put(RawContacts.DIRTY, newContact.isDirty() ? 1 : 0);
            // values.put(RawContacts.VERSION, newContact.getVersion());
            // add by Yu

            values.put(RawContacts.ACCOUNT_NAME,
                    USIMUtils.ACCOUNT_NAME_LOCAL_PHONE);
            values.put(RawContacts.ACCOUNT_TYPE,
                    USIMUtils.ACCOUNT_TYPE_LOCAL_PHONE);

            shouldExit = !rawContactsInserter.append(values);
            if (shouldExit) {
                Debugger.logE(new Object[] { raw, rawContactsConsumer,
                        contactDataConsumer, outBuffer },
                        "Error in bulk inserting raw contacts, "
                                + "statusCode: "
                                + rawContactsInserter.getStatusCode());
                rawContactsConsumer.consume(null, 0, 0);
                contactDataConsumer.consume(null, 0, 0);
                return;
            }
        }
        // Bulk insert sim contacts left
        shouldExit = !simContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert sim1 contacts left
        shouldExit = !sim1ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim1 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert sim2 contacts left
        shouldExit = !sim2ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim2 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }
        // Bulk insert sim3 contacts left
        shouldExit = !sim3ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim3 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }
        
        // Bulk insert sim4 contacts left
        shouldExit = !sim4ContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting sim4 contacts, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        // Bulk insert all ContentValues left
        shouldExit = !contactDataInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting contact data, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }
        // Bulk insert all ContentValues left
        shouldExit = !rawContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, rawContactsConsumer,
                    contactDataConsumer, outBuffer },
                    "Error in bulk inserting raw contacts, " + "statusCode: "
                            + rawContactsInserter.getStatusCode());
            rawContactsConsumer.consume(null, 0, 0);
            contactDataConsumer.consume(null, 0, 0);
            return;
        }

        asyncGetAllRawContacts(rawContactsConsumer, outBuffer, beginId, beginId
                + count);
        asyncGetAllContactData(null, contactDataConsumer, outBuffer, null,
                null, beginId, beginId + count);
    }

    /**
     * Asynchronously query all groups stored on phone.
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the content of the group.
     */
    public void asyncGetAllGroups(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;

        try {
            // Query all groups on phone
            c = getContentResolver().query(Groups.CONTENT_URI,
            /*
             * new String[] { Groups._ID, Groups.TITLE, Groups.NOTES,
             * Groups.SYSTEM_ID }
             */
            null, Groups.DELETED + "<>" + DatabaseRecordEntity.TRUE, null,
                    Groups._ID + " ASC");

            final FastGroupCursorParser parser = new FastGroupCursorParser(c,
                    consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * Asynchronously query all contacts stored on SIM.
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the contacts in sim.
     * @param sourceLocation
     *            The sourceLocation.
     */
    public void asyncGetAllSimContacts(final IRawBlockConsumer consumer,
            final ByteBuffer buffer, final int sourceLocation) {
        Cursor c = null;

        try {
            // Query all contacts on SIM
            c = getContentResolver().query(
                    getSimUri(sourceLocation),
                    new String[] { SimContactsContent.COLUMN_ID,
                            SimContactsContent.COLUMN_NAME,
                            SimContactsContent.COLUMN_NUMBER }, null, null,
                    SimContactsContent.COLUMN_NAME + " ASC");

            final FastSimContactsCursorParser parser = new FastSimContactsCursorParser(
                    c, consumer, buffer, sourceLocation);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * Asynchronously query all contacts stored on SIM.
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the contacts in sim.
     */
    public void asyncGetAllSimContacts(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        if (Config.MTK_GEMINI_SUPPORT) {
            this.asyncGetAllSimContacts(consumer, buffer,
                    DataStoreLocations.SIM1);
            this.asyncGetAllSimContacts(consumer, buffer,
                    DataStoreLocations.SIM2);
            if (Config.MTK_3SIM_SUPPORT) {
            	this.asyncGetAllSimContacts(consumer, buffer,
                        DataStoreLocations.SIM3);
            }
            if (Config.MTK_4SIM_SUPPORT) {
            	this.asyncGetAllSimContacts(consumer, buffer,
                        DataStoreLocations.SIM4);
            }
        } else {
            this.asyncGetAllSimContacts(consumer, buffer,
                    DataStoreLocations.SIM);
        }
    }

    /**
     * Asynchronously query all raw contacts(without detailed data) stored on
     * phone.
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks
     * @param buffer
     *            The buffer to save the raw contact.
     * @param idFrom
     *            The started id of the raw contact.
     * @param idTo
     *            The ended id of the raw contact.
     */
    public void asyncGetAllRawContacts(final IRawBlockConsumer consumer,
            final ByteBuffer buffer, final Long idFrom, final Long idTo) {
        Cursor c = null;
        // Query all by default
        String selection = null;
        // Construct query selection string according to required data types
        final StringBuffer strBuf = new StringBuffer();
        if (null != idFrom) {
            strBuf
                    .append(RawContacts._ID + ">=" + idFrom.longValue()
                            + " AND ");
        }
        if (null != idTo) {
            strBuf.append(RawContacts._ID + "<=" + idTo.longValue() + " AND ");
        }
        strBuf.append(RawContacts.DELETED + "<>" + DatabaseRecordEntity.TRUE);
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID,
                            Contacts.DISPLAY_NAME,
                            RawContactsContent.COLUMN_MODIFY_TIME, // Shaoying
                            // Han
                            RawContacts.STARRED, RawContacts.SEND_TO_VOICEMAIL,
                            /*
                             * RawContacts.TIMES_CONTACTED,
                             * RawContacts.LAST_TIME_CONTACTED,
                             * RawContacts.CUSTOM_RINGTONE,
                             */
                            RawContacts.VERSION,
                            // For MTK SIM Contacts feature
                            RawContactsContent.COLUMN_SOURCE_LOCATION,
                            RawContactsContent.COLUMN_INDEX_IN_SIM },
                    selection.toString(), null, null);

            final FastRawContactsCursorParser parser = new FastRawContactsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the raw contact.
     * @param idFrom
     *            The started id.
     * @param idTo
     *            The ended id.
     */
    public void asyncGetAllRawContactsForBackup(
            final IRawBlockConsumer consumer, final ByteBuffer buffer,
            final Long idFrom, final Long idTo) {
        Cursor c = null;
        // Query all by default
        String selection = null;
        // Construct query selection string according to required data types
        final StringBuffer strBuf = new StringBuffer();
        if (null != idFrom) {
            strBuf
                    .append(RawContacts._ID + ">=" + idFrom.longValue()
                            + " AND ");
        }
        if (null != idTo) {
            strBuf.append(RawContacts._ID + "<=" + idTo.longValue() + " AND ");
        }
        strBuf.append(RawContacts.DELETED + "<>" + DatabaseRecordEntity.TRUE);
        strBuf.append(" AND " + RawContactsContent.COLUMN_SOURCE_LOCATION + "="
                + "-1");
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID, Contacts.DISPLAY_NAME,
                            RawContactsContent.COLUMN_MODIFY_TIME, // Shaoying
                            // Han
                            RawContacts.STARRED, RawContacts.SEND_TO_VOICEMAIL,
                            /*
                             * RawContacts.TIMES_CONTACTED,
                             * RawContacts.LAST_TIME_CONTACTED,
                             * RawContacts.CUSTOM_RINGTONE,
                             */
                            RawContacts.VERSION,
                            // For MTK SIM Contacts feature
                            RawContactsContent.COLUMN_SOURCE_LOCATION },
                    selection.toString(), null, null);

            final FastRawContactsCursorParser parser = new FastRawContactsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the raw contact.
     */
    public void asyncGetAllRawContacts(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        this.asyncGetAllRawContacts(consumer, buffer, null, null);
    }

    /**
     * Asynchronously query all contact data stored on phone.
     * 
     * @param requiredMimeTypes
     *            The list of the mime types of the contact.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the contact data.
     * @param idFrom
     *            The started id of raw contact.
     * @param idTo
     *            The ended id of raw contact.
     * @param contactIdFrom
     *            The started id of the contact.
     * @param contactIdTo
     *            The ended id of the contact.
     */
    public void asyncGetAllContactData(final List<Integer> requiredMimeTypes,
            final IRawBlockConsumer consumer, final ByteBuffer buffer,
            final Long idFrom, final Long idTo, final Long contactIdFrom,
            final Long contactIdTo) {
        Cursor c = null;
        // Query all by default
        String selection = null;
        // Construct query selection string according to required data types
        final StringBuffer strBuf = new StringBuffer();
        // Append MIME Type selection
        if (null != requiredMimeTypes) {
            strBuf.append('(');
            boolean mimeAppended = false;
            for (final int mimeType : requiredMimeTypes) {
                switch (mimeType) {
                case StructuredName.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE
                            + "' OR ");
                    mimeAppended = true;
                    break;

                case Phone.MIME_TYPE:
                    strBuf
                            .append(Data.MIMETYPE + "='"
                                    + CommonDataKinds.Phone.CONTENT_ITEM_TYPE
                                    + "' OR ");
                    mimeAppended = true;
                    break;

                case Photo.MIME_TYPE:
                    strBuf
                            .append(Data.MIMETYPE + "='"
                                    + CommonDataKinds.Photo.CONTENT_ITEM_TYPE
                                    + "' OR ");
                    mimeAppended = true;
                    break;

                case Email.MIME_TYPE:
                    strBuf
                            .append(Data.MIMETYPE + "='"
                                    + CommonDataKinds.Email.CONTENT_ITEM_TYPE
                                    + "' OR ");
                    mimeAppended = true;
                    break;

                case Im.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.Im.CONTENT_ITEM_TYPE + "' OR ");
                    mimeAppended = true;
                    break;

                case StructuredPostal.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE
                                    + "='"
                                    + CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE
                                    + "' OR ");
                    mimeAppended = true;
                    break;

                case Organization.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.Organization.CONTENT_ITEM_TYPE
                            + "' OR ");
                    mimeAppended = true;
                    break;

                case Nickname.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.Nickname.CONTENT_ITEM_TYPE
                            + "' OR ");
                    mimeAppended = true;
                    break;

                case Note.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.Note.CONTENT_ITEM_TYPE + "' OR ");
                    mimeAppended = true;
                    break;

                case Website.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.Website.CONTENT_ITEM_TYPE
                            + "' OR ");
                    mimeAppended = true;
                    break;

                case GroupMembership.MIME_TYPE:
                    strBuf.append(Data.MIMETYPE + "='"
                            + CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE
                            + "' OR ");
                    mimeAppended = true;
                    break;

                default:
                    break;
                }
            }
            if (mimeAppended) {
                // Delete the last "' OR "
                strBuf.delete(strBuf.length() - 5, strBuf.length());
                strBuf.append(')');
            } else {
                // Delete "("
                strBuf.delete(strBuf.length() - 1, strBuf.length());
            }
        }
        // Append ID range selection
        if (null != idFrom) {
            if (strBuf.length() > 0) {
                strBuf.append(" AND ");
            }
            strBuf.append(Data._ID + ">=" + idFrom.longValue());
        }
        if (null != idTo) {
            if (strBuf.length() > 0) {
                strBuf.append(" AND ");
            }
            strBuf.append(Data._ID + "<=" + idTo.longValue());
        }
        if (null != contactIdFrom) {
            if (strBuf.length() > 0) {
                strBuf.append(" AND ");
            }
            strBuf.append(Data.RAW_CONTACT_ID + ">="
                    + contactIdFrom.longValue());
        }
        if (null != contactIdTo) {
            if (strBuf.length() > 0) {
                strBuf.append(" AND ");
            }
            strBuf.append(Data.RAW_CONTACT_ID + "<=" + contactIdTo.longValue());
        }

        if (strBuf.length() > 0) {
            selection = strBuf.toString();
        }

        try {
            c = getContentResolver().query(
                    Data.CONTENT_URI,
                    new String[] {
                            Data._ID,
                            Data.RAW_CONTACT_ID,
                            // Data.IS_PRIMARY,
                            // Data.IS_SUPER_PRIMARY,
                            Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DATA3,
                            Data.DATA4, Data.DATA5, Data.DATA6, Data.DATA7,
                            Data.DATA8, Data.DATA9, Data.DATA10, Data.DATA15,
                            // Added by Shaoying Han
                            ContactDataContent.COLUMN_BINDING_SIM_ID },
                    selection, null, Data.RAW_CONTACT_ID + " ASC");

            final FastContactDataCursorParser parser = new FastContactDataCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requiredMimeTypes
     *            The list of mime types of the contact.
     * @param consumer
     *            Set a consumer to handle asynchronous blocks.
     * @param buffer
     *            The buffer to save the contact data.
     */
    public void asyncGetAllContactData(final List<Integer> requiredMimeTypes,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        this.asyncGetAllContactData(requiredMimeTypes, consumer, buffer, null,
                null, null, null);
    }

    // Groups ------------------------------------------------------------------
    /**
     * Get a list of all groups on phone.
     * 
     * @return A list of groups.
     * @deprecated Use fastGetAllGroups instead.
     */
    /*
     * public List<Group> getAllGroups(){ List<Group> result = new
     * ArrayList<Group>(); Cursor cGroups;
     * 
     * cGroups = getContentResolver().query( Groups.CONTENT_URI, new String[]{
     * Groups._ID, Groups.TITLE, Groups.NOTES, Groups.SYSTEM_ID },
     * Groups.DELETED + "<>" + DatabaseRecordEntity.TRUE, null, Groups.TITLE +
     * " ASC");
     * 
     * if (cGroups != null){ long gId; while(cGroups.moveToNext()){ gId =
     * cGroups.getLong(0);
     * 
     * Group group = new Group(gId); group.setTitle(cGroups.getString(1));
     * group.setNotes(cGroups.getString(2));
     * group.setSystemId(cGroups.getString(3)); // Add the group into list
     * result.add(group); } // Release resources cGroups.close(); }
     * 
     * return result; }
     */
    /**
     * Add a group on phone. This insertion has nothing to do with contact
     * members and memberships in the group.
     * 
     * @param group
     *            The group to add.
     * @return The id after the group has been inserted. Return ID_NULL when
     *         insertion fail.
     * @see DatabaseRecordEntity#ID_NULL
     */
    public long insertGroup(final Group group) {
        if (group == null) {
            Debugger.logW(new Object[] { group }, "Group passed in is null.");
            return DatabaseRecordEntity.ID_NULL;
        }

        long insertedId = DatabaseRecordEntity.ID_NULL;

        String accountType = group.getAccount_type();
        String accountName = group.getAccount_name();

        final ContentValues values = new ContentValues(10);
        values.put(Groups.TITLE, group.getTitle());
        values.put(Groups.NOTES, group.getNotes());

        /*
         * added by Yu
         */
        if (null != group.getSystemId()) {
            values.put(Groups.SYSTEM_ID, group.getSystemId());
        } else {
            values.put(Groups.SYSTEM_ID, 0);
        }
        if (null != group.getDeleted()) {
            values.put(Groups.DELETED, group.getDeleted());
        }
        /**
         * Jelly Bean account_name account_type fields don't exist in group
         * table
         */
        if (null != accountName) {
            values.put(Groups.ACCOUNT_NAME, accountName);
            Debugger.logI(new Object[] {}, "accountName :" + accountName);
        }
        if (null != accountType) {
            values.put(Groups.ACCOUNT_TYPE, accountType);
        }
        if (null != group.getVersion()) {
            values.put(Groups.VERSION, group.getVersion());
        }
        if (null != group.getDirty()) {
            values.put(Groups.DIRTY, group.getDirty());
        }
        if (null != group.getGroup_visible()) {
            values.put(Groups.GROUP_VISIBLE, group.getGroup_visible());
        }
        if (null != group.getShould_sync()) {
            values.put(Groups.SHOULD_SYNC, group.getShould_sync());
        }

        if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0
                && accountType != null
                && accountType.equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            insertGroupInUSIM(group.getAccount_type(), accountName, group
                    .getTitle());
        }

        try {
            final Uri uri = getObservedContentResolver().insert(
                    Groups.CONTENT_URI, values);
            if (uri != null) {
                insertedId = Long.parseLong(uri.getLastPathSegment());
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { group }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { group }, null, e);
        }

        // Set the new id after insertion
        group.setId(insertedId);

        return insertedId;
    }

    /**
     * @param groups
     *            The list of group to be insert.
     * @return the number of newly created rows.
     */
    public int insertGroups(final List<Group> groups) {
        int count = 0;
        try {
            count = getObservedContentResolver().bulkInsert(Groups.CONTENT_URI,
                    GroupContent.groupsToValues(groups));
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { groups }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { groups }, null, e);
        }
        return count;
    }

    /**
     * Insert the group to USIM card by Iccprovider
     * 
     * @param accountType
     * @param accountName
     * @param groupTitle
     */
    private void insertGroupInUSIM(String accountType, String accountName,
            String groupTitle) {
        if (groupTitle == null) {
            groupTitle = "";
        }
        if (accountType == null || accountName == null) {
            return;
        }
        try {
            if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM)) {
                final int idInUSIM = USIMUtils.createUSIMGroup(0, groupTitle);
                Debugger.logI(new Object[] {}, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM + "Group id is "
                        + idInUSIM);
            } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM2)) {
                final int idInUSIM = USIMUtils.createUSIMGroup(1, groupTitle);
                Debugger.logI(new Object[] {}, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM2 + "Group id is "
                        + idInUSIM);
            } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM3)) {
                final int idInUSIM = USIMUtils.createUSIMGroup(2, groupTitle);
                Debugger.logI(new Object[] {}, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM3 + "Group id is "
                        + idInUSIM);
            } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM4)) {
                final int idInUSIM = USIMUtils.createUSIMGroup(3, groupTitle);
                Debugger.logI(new Object[] {}, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM4 + "Group id is "
                        + idInUSIM);
            } else {
                Debugger.logW(new Object[] {}, "Account name is " + accountName);
            }
        } catch (RemoteException e) {
            Debugger.logE(new Object[] {}, "Catch RemoteException!!");
        } catch (USIMGroupException e) {
            Debugger.logE(new Object[] {}, "Catch USIMGroupException!!");
        }
    }

    /**
     * Update a group on phone. This update has nothing to do with contact
     * members and memberships in the group.
     * 
     * @param id
     *            The unique database id of the group to update.
     * @param newGroup
     *            The group object encapsulating all new data of the group.
     * @return How many groups are updated.
     */
    public int updateGroup(final long id, final Group newGroup) {
        if (newGroup == null) {
            Debugger.logW(new Object[] { id, newGroup },
                    "New group passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(2);
        values.put(Groups.TITLE, newGroup.getTitle());
        values.put(Groups.NOTES, newGroup.getNotes());

        updateCount = getObservedContentResolver().update(
                Groups.CONTENT_URI,
                values,
                Groups._ID + "=" + id + " AND " + Groups.DELETED + "<>"
                        + DatabaseRecordEntity.TRUE, null);

        return updateCount;
    }

    /**
     * Update a group on phone. This update has nothing to do with contact
     * members and memberships in the group.
     * 
     * @param id
     *            The unique database id of the group to update.
     * @param newGroup
     *            The group object encapsulating all new data of the group.
     * @param oldName
     *            The old name of the group.
     * @return How many groups are updated.
     */
    public int updateGroup(final long id, final Group newGroup,
            final String oldName) {
        if (newGroup == null) {
            Debugger.logW(new Object[] { id, newGroup },
                    "New group passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final ContentValues values = new ContentValues(2);
        values.put(Groups.TITLE, newGroup.getTitle());
        values.put(Groups.NOTES, newGroup.getNotes());

        // USIM:
        final String accountType = newGroup.getAccount_type();
        final String accountName = newGroup.getAccount_name();

        if (accountType != null
                && accountType.equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            int simGroupId = -1;
            try {
            	int slotId = USIMUtils.getSlotIdByAccountName(accountName);
                simGroupId = USIMUtils.hasExistGroup(slotId, oldName);
            } catch (final RemoteException e1) {
                e1.printStackTrace();
            }
            try {
                if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM)) {
                    final int idInUSIM = USIMUtils.updateUSIMGroup(0,
                            simGroupId, newGroup.getTitle());
                    Debugger.logI(new Object[] { newGroup }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM + "; Group id is "
                            + idInUSIM);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM2)) {
                    final int idInUSIM = USIMUtils.updateUSIMGroup(1,
                            simGroupId, newGroup.getTitle());
                    Debugger.logI(new Object[] { newGroup }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM2 + "Group id is "
                            + idInUSIM);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM3)) {
                    final int idInUSIM = USIMUtils.updateUSIMGroup(2,
                            simGroupId, newGroup.getTitle());
                    Debugger.logI(new Object[] { newGroup }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM3 + "Group id is "
                            + idInUSIM);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM4)) {
                    final int idInUSIM = USIMUtils.updateUSIMGroup(3,
                            simGroupId, newGroup.getTitle());
                    Debugger.logI(new Object[] { newGroup }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM3 + "Group id is "
                            + idInUSIM);
                } else {
                    Debugger.logW(new Object[] { newGroup }, "Account name is "
                            + accountName);
                }
            } catch (final RemoteException e) {
                e.printStackTrace();
            } catch (final USIMGroupException e) {
                e.printStackTrace();
            }
        }

        updateCount = getObservedContentResolver().update(
                Groups.CONTENT_URI,
                values,
                Groups._ID + "=" + id + " AND " + Groups.DELETED + "<>"
                        + DatabaseRecordEntity.TRUE, null);

        return updateCount;
    }

    /**
     * @param groups
     *            The groups to update.
     * @return The number of the update group.
     */
    public int updateGroupForRestore(final List<Group> groups) {
        for (final Group group : groups) {
            final ContentValues values = GroupContent.groupToValues(group);
            if (null != values) {
                final int count = getObservedContentResolver().update(
                        Groups.CONTENT_URI, values,
                        Groups._ID + "=" + group.getId(), null);
                Debugger.logW(new Object[] {}, "update count :" + count);
                if (count == 0) {
                    getObservedContentResolver().insert(Groups.CONTENT_URI,
                            GroupContent.groupToValues(group));
                }
            }
        }
        return groups.size();
    }

    /**
     * Delete a group on phone. The group members(contact) in the group will not
     * be deleted, however, all its group memberships will be deleted.
     * 
     * @param id
     *            The unique database id of the group to delete.
     * @return How many groups are deleted.
     */
    public int deleteGroup(final long id) {
        int deleteCount = 0;
        deleteCount = getObservedContentResolver().delete(Groups.CONTENT_URI
        /*
         * // Delete physically, not just set DELETED=1 .buildUpon()
         * .appendQueryParameter(ContactsContract.CALLER_IS_SYNCADAPTER, "true")
         * .build()
         */, Groups._ID + "=" + id, null);

        return deleteCount;
    }

    /**
     * @param id
     *            The row id.
     * @param group
     *            The group to be delete.
     * @return The number of rows deleted.
     */
    public int deleteGroup(final long id, final Group group) {

        if (null != group.getAccount_type()
                && group.getAccount_type().equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            if (null != group.getAccount_name()
                    && group.getAccount_name().endsWith(
                            USIMUtils.ACCOUNT_NAME_USIM)) {
                final int idInUSIM = USIMUtils.deleteUSIMGroup(0, group
                        .getTitle());
                Debugger.logI(new Object[] { group }, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM + "Group id is "
                        + idInUSIM);
            } else if (null != group.getAccount_name()
                    && group.getAccount_name().endsWith(
                            USIMUtils.ACCOUNT_NAME_USIM2)) {
                final int idInUSIM = USIMUtils.deleteUSIMGroup(1, group
                        .getTitle());
                Debugger.logI(new Object[] { group }, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM2 + "Group id is "
                        + idInUSIM);
            } else if (null != group.getAccount_name()
                    && group.getAccount_name().endsWith(
                            USIMUtils.ACCOUNT_NAME_USIM3)) {
                final int idInUSIM = USIMUtils.deleteUSIMGroup(2, group
                        .getTitle());
                Debugger.logI(new Object[] { group }, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM3 + "Group id is "
                        + idInUSIM);
            } else if (null != group.getAccount_name()
                    && group.getAccount_name().endsWith(
                            USIMUtils.ACCOUNT_NAME_USIM4)) {
                final int idInUSIM = USIMUtils.deleteUSIMGroup(3, group
                        .getTitle());
                Debugger.logI(new Object[] { group }, "Account name is "
                        + USIMUtils.ACCOUNT_NAME_USIM4 + "Group id is "
                        + idInUSIM);
            } else {
                Debugger.logW(new Object[] { group }, "Account name is "
                        + group.getAccount_name());
            }
        }
        int deleteCount = 0;
        deleteCount = getObservedContentResolver().delete(Groups.CONTENT_URI
        /*
         * // Delete physically, not just set DELETED=1 .buildUpon()
         * .appendQueryParameter(ContactsContract.CALLER_IS_SYNCADAPTER, "true")
         * .build()
         */, Groups._ID + "=" + id, null);

        return deleteCount;
    }

    /**
     * Delete groups on phone. The group members(contact) in the groups will not
     * be deleted, however, all their group memberships will be deleted.
     * 
     * @param ids
     *            List of the database id of the groups to delete.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteGroup(final long[] ids) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple deletion
        mOpBatch.clear();
        final DefaultDeleteBatchHelper batchHelper = new DefaultDeleteBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.deleteGroup$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendGroupDelete(
                        ids[appendPosition], false);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(ids.length);

        return batchHelper.getResults();
    }

    /**
     * Delete groups on phone. The group members(contact) in the groups will not
     * be deleted, however, all their group memberships will be deleted.
     * 
     * @param ids
     *            List of the database id of the groups to delete.
     * @param groups
     *            The groups to be deleted.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteGroup(final long[] ids, final ArrayList<Group> groups) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple deletion
        mOpBatch.clear();
        final DefaultDeleteBatchHelper batchHelper = new DefaultDeleteBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.deleteGroup$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendGroupDelete(
                        ids[appendPosition], false);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(ids.length);

        // ==================================
        // USIM:===========================================//
        for (final Group group : groups) {

            final String accountType = group.getAccount_type();
            if (accountType != null
                    && accountType.equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
                Debugger.logI(new Object[] {}, "group name is :"
                        + group.getTitle());
                USIMUtils.syncUSIMGroupDeleteDualSim(group.getTitle());

            }
        }
        // ===================================================================================//

        return batchHelper.getResults();
    }

    /**
     * Delete all groups on phone. The group members(contact) in the groups will
     * not be deleted, however, all their group memberships will be deleted.
     * 
     * @return How many groups are deleted.
     */
    public int deleteAllGroups() {
        int deleteCount = 0;

        // deleteCount = getObservedContentResolver().delete(Groups.CONTENT_URI
        //        
        // // Delete physically, not just set DELETED=1
        // .buildUpon()
        // .appendQueryParameter(ContactsContract.CALLER_IS_SYNCADAPTER, "true")
        // .build()
        // , Groups._ID + " > " + 5
        // // ,null
        // , null);

        deleteCount = getObservedContentResolver().delete(Groups.CONTENT_URI,
                Groups._ID + " > " + 5
                // ,null
                , null);

        return deleteCount;
    }

    // Contact data ------------------------------------------------------------
    /**
     * Add one row of contact data on phone.
     * 
     * @param data
     *            Contact data to add.
     * @param validateContactId
     *            If true, it will check whether the contact id specified in the
     *            contact data is valid, and if the contact id is invalid, data
     *            will not be inserted.
     * @return The id after the group has been inserted. Return ID_NULL when
     *         insertion fail.
     * @see DatabaseRecordEntity#ID_NULL
     */
    public long insertContactData(final ContactData data,
            final boolean validateContactId) {
        if (data == null) {
            Debugger.logW(new Object[] { data, validateContactId },
                    "Contact data passed in is null.");
            return DatabaseRecordEntity.ID_NULL;
        }

        long insertedId = DatabaseRecordEntity.ID_NULL;
        final long cId = data.getRawContactId();

        if (validateContactId) {
            final Cursor c = getContentResolver().query(
                    RawContacts.CONTENT_URI, new String[] { RawContacts._ID },
                    RawContacts._ID + "=" + cId, null, null);
            if (null == c) {
                Debugger.logE(new Object[] { data, validateContactId },
                        "Cursor is null. Failed to find raw contact with id "
                                + "of " + cId + ".");
                return DatabaseRecordEntity.ID_NULL;
            } else if (c.getCount() < 1) {
                Debugger.logE(new Object[] { data, validateContactId },
                        "Raw contact id " + cId + " does not exist.");
                c.close();
                return DatabaseRecordEntity.ID_NULL;
            }
            c.close();
        }

        mOpBatch.clear();
        data.setRawContactId(cId);
        if (mOpBatch.appendContactDataInsert(data)) {
            // Do the insertion
            try {
                final ContentProviderResult[] results = mOpBatch.apply();
                if (results != null) {
                    insertedId = Long.parseLong(results[0].uri
                            .getLastPathSegment());
                } else {
                    insertedId = DatabaseRecordEntity.ID_NULL;
                }
            } catch (final NumberFormatException e) {
                Debugger
                        .logE(new Object[] { data, validateContactId }, null, e);
                e.printStackTrace();
            } catch (final IllegalArgumentException e) {
                Debugger
                        .logE(new Object[] { data, validateContactId }, null, e);
                e.printStackTrace();
            } catch (final RemoteException e) {
                Debugger
                        .logE(new Object[] { data, validateContactId }, null, e);
                e.printStackTrace();
            } catch (final OperationApplicationException e) {
                Debugger
                        .logE(new Object[] { data, validateContactId }, null, e);
                e.printStackTrace();
            } finally {
                mOpBatch.clear();
            }
        } else {
            insertedId = DatabaseRecordEntity.ID_NULL;
        }

        // Set the new id after insertion
        data.setId(insertedId);

        return insertedId;
    }

    /**
     * Update one row of contact data on phone.
     * 
     * @param dataId
     *            The unique database id of the contact data to update.
     * @param newData
     *            The contact data object encapsulating all new data.
     * @param validateContactId
     *            If true, it will check whether the new contact id specified in
     *            the contact data is valid, and if the new contact id is
     *            invalid, data will not be updated.
     * @return How many data rows are updated.
     */
    public int updateContactData(final long dataId, final ContactData newData,
            final boolean validateContactId) {
        if (newData == null) {
            Debugger.logW(new Object[] { dataId, newData, validateContactId },
                    "New contact data passed in is null.");
            return 0;
        }

        int updateCount = 0;

        final long cId = newData.getRawContactId();
        if (validateContactId) {
            final Cursor c = getContentResolver().query(
                    RawContacts.CONTENT_URI, new String[] { RawContacts._ID },
                    RawContacts._ID + "=" + cId, null, null);
            if (null == c) {
                Debugger.logE(
                        new Object[] { dataId, newData, validateContactId },
                        "Cursor is null. Failed to find raw contact with "
                                + "id of " + cId + ". ");
                return 0;
            } else if (c.getCount() < 1) {
                Debugger.logE(
                        new Object[] { dataId, newData, validateContactId },
                        "Raw contact id " + cId + " does not exist.");
                c.close();
                return 0;
            }
            c.close();
        }

        mOpBatch.clear();
        if (mOpBatch.appendContactDataUpdate(dataId, newData)) {
            // Do the update
            try {
                final ContentProviderResult[] results = mOpBatch.apply();
                updateCount = results[0].count;
            } catch (final RemoteException e) {
                Debugger.logE(
                        new Object[] { dataId, newData, validateContactId },
                        null, e);
                e.printStackTrace();
            } catch (final OperationApplicationException e) {
                Debugger.logE(
                        new Object[] { dataId, newData, validateContactId },
                        null, e);
                e.printStackTrace();
            } finally {
                mOpBatch.clear();
            }
        } else {
            updateCount = 0;
        }

        return updateCount;
    }

    /**
     * Delete one row of contact data on phone.
     * 
     * @param ids
     *            List of the unique database id of the contact data to delete.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteContactData(final long[] ids) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple deletion
        mOpBatch.clear();
        final DefaultDeleteBatchHelper batchHelper = new DefaultDeleteBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.deleteContactData$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendContactDataDelete(
                        ids[appendPosition], false);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(ids.length);

        return batchHelper.getResults();
    }

    /**
     * Delete one row of contact data on phone.
     * 
     * @param ids
     *            List of the unique database id of the contact data to delete.
     * @param groupId
     *            The id of the group.
     * @param group
     *            The group to delete.
     * @param simIndexes
     *            The array of the sim index.
     * @return List of deletion results(successful or not).
     */
    public boolean[] deleteContactData(final long[] ids, final long groupId,
            final Group group, final int[] simIndexes) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple deletion
        mOpBatch.clear();
        final DefaultDeleteBatchHelper batchHelper = new DefaultDeleteBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.deleteContactData$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendContactDataDelete(
                        ids[appendPosition], false);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(ids.length);

        // =================================USIM:=================================
        // //

        if (group == null || simIndexes == null) {
            return batchHelper.getResults();
        }

        final String accountType = group.getAccount_type();
        final String accountName = group.getAccount_name();

        if (accountType != null
                && accountType.equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            // USIMUtils.addUSIMGroupMember(slotId, simIndex, grpId);
            int simGroupId = -1;
            try {
            	
            	int slotId = USIMUtils.getSlotIdByAccountName(accountName);
                simGroupId = USIMUtils.hasExistGroup(slotId, group.getTitle()); 
            } catch (final RemoteException e1) {
                e1.printStackTrace();
            }
            for (final int simIndex : simIndexes) {
                if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM)) {
                    USIMUtils.deleteUSIMGroupMember(0, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM2)) {
                    USIMUtils.deleteUSIMGroupMember(1, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM3)) {
                    USIMUtils.deleteUSIMGroupMember(2, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM4)) {
                    USIMUtils.deleteUSIMGroupMember(3, simIndex, simGroupId);
                } else {
                    Debugger.logW(new Object[] { group }, "Account name is "
                            + accountName);
                }
            }
        }
        return batchHelper.getResults();
    }

    /**
     * @param contactIds
     *            The array of the contact id.
     * @param groupId
     *            The id of the group.
     * @return the array of id for inserted rows.
     */
    public long[] insertGroupMembership(final long[] contactIds,
            final long groupId) {
        if (null == contactIds) {
            Debugger
                    .logE(new Object[] { contactIds, groupId }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple update
        mOpBatch.clear();
        final DefaultInsertBatchHelper batchHelper = new DefaultInsertBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.insertGroupMembership$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendGroupMembershipInsert(
                        contactIds[appendPosition], groupId);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(contactIds.length);

        return batchHelper.getResults();
    }

    /**
     * @param contactIds
     *            The array of the contact id.
     * @param groupId
     *            The id of the group.
     * @param group
     *            The group to insert.
     * @param simIndexes
     *            The array of index of the sim.
     * @return The array of id for inserted rows.
     */
    public long[] insertGroupMembership(final long[] contactIds,
            final long groupId, final Group group, final int[] simIndexes) {
        if (null == contactIds) {
            Debugger
                    .logE(new Object[] { contactIds, groupId }, "List is null.");
            return null;
        }
        // Use batch mode to do multiple update
        mOpBatch.clear();
        final DefaultInsertBatchHelper batchHelper = new DefaultInsertBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.insertGroupMembership$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendGroupMembershipInsert(
                        contactIds[appendPosition], groupId);
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(contactIds.length);

        // ==================================
        // USIM:===========================================//
        if (group == null || simIndexes == null) {
            return batchHelper.getResults();
        }

        final String accountType = group.getAccount_type();
        final String accountName = group.getAccount_name();

        if (accountType != null
                && accountType.equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            // USIMUtils.addUSIMGroupMember(slotId, simIndex, grpId);
            int simGroupId = -1;
            try {
            	int slotId = USIMUtils.getSlotIdByAccountName(accountName);
                simGroupId = USIMUtils.hasExistGroup(slotId, group.getTitle());
            } catch (final RemoteException e1) {
                e1.printStackTrace();
            }
            for (final int simIndex : simIndexes) {
                if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM)) {
                    USIMUtils.addUSIMGroupMember(0, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM2)) {
                    USIMUtils.addUSIMGroupMember(1, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM3)) {
                    USIMUtils.addUSIMGroupMember(2, simIndex, simGroupId);
                } else if (accountName.endsWith(USIMUtils.ACCOUNT_NAME_USIM4)) {
                    USIMUtils.addUSIMGroupMember(3, simIndex, simGroupId);
                } else {
                    Debugger.logW(new Object[] { group }, "Account name is "
                            + accountName);
                }
            }
        }
        // ===================================================================================//

        return batchHelper.getResults();
    }

    /**
     * @param contactIds
     *            The array of the contact is which to be inserted.
     * @param group
     *            The group used to insert.
     * @return The array of id for inserted rows.
     */
    public long[] insertGroupMembership(final long[] contactIds,
            final Group group) {
        if (null == contactIds) {
            Debugger.logE(new Object[] { contactIds, group }, "List is null.");
            return null;
        }

        if (null != group.getAccount_type()
                && group.getAccount_type().equals(USIMUtils.ACCOUNT_TYPE_USIM)) {
            try {
                if (group.getAccount_name().endsWith(
                        USIMUtils.ACCOUNT_NAME_USIM)) {
                	int slotId = USIMUtils.getSlotIdByAccountName(group.getAccount_name());
                    final int groupId = USIMUtils.hasExistGroup(slotId, group
                            .getTitle());
                    USIMUtils.addUSIMGroupMember(0, 0, groupId);
                } else if (group.getAccount_name().endsWith(
                        USIMUtils.ACCOUNT_NAME_USIM2)) {
                    final int idInUSIM = USIMUtils.createUSIMGroup(1, group
                            .getTitle());
                    Debugger.logI(new Object[] { group }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM2 + "Group id is "
                            + idInUSIM);
                } else if (group.getAccount_name().endsWith(
                        USIMUtils.ACCOUNT_NAME_USIM3)) {
                    final int idInUSIM = USIMUtils.createUSIMGroup(2, group
                            .getTitle());
                    Debugger.logI(new Object[] { group }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM3 + "Group id is "
                            + idInUSIM);
                } else if (group.getAccount_name().endsWith(
                        USIMUtils.ACCOUNT_NAME_USIM4)) {
                    final int idInUSIM = USIMUtils.createUSIMGroup(3, group
                            .getTitle());
                    Debugger.logI(new Object[] { group }, "Account name is "
                            + USIMUtils.ACCOUNT_NAME_USIM4 + "Group id is "
                            + idInUSIM);
                } else {
                    Debugger.logW(new Object[] { group }, "Account name is "
                            + group.getAccount_name());
                }
            } catch (final RemoteException e) {
                e.printStackTrace();
            } catch (final USIMGroupException e) {
                e.printStackTrace();
            }
        }

        // Use batch mode to do multiple update
        mOpBatch.clear();
        final DefaultInsertBatchHelper batchHelper = new DefaultInsertBatchHelper(
                mOpBatch) {

            // @Override
            public String getName() {
                return "ContactsProxy.insertGroupMembership$" + super.getName();
            };

            // @Override
            public void onAppend(final ContentProviderOperationBatch opBatch,
                    final int appendPosition) {
                ((ContactsOperationBatch) opBatch).appendGroupMembershipInsert(
                        contactIds[appendPosition], group.getId());
            }

            // @Override
            public ContentProviderResult[] onApply(
                    final ContentProviderOperationBatch opBatch)
                    throws RemoteException, OperationApplicationException {
                return ((ContactsOperationBatch) opBatch).apply();
            }

        };
        batchHelper.run(contactIds.length);

        return batchHelper.getResults();
    }

    // Calls -------------------------------------------------------------------
    /**
     * Get time of the last call.
     * 
     * @return Time of the last call.
     */
    public long getLastCallTime() {
        long lastCallTime = 0;
        Cursor c;

        /*
         * c = getContentResolver().query( Calls.CONTENT_URI, new
         * String[]{Calls.DATE}, null, null, Calls.DATE + " DESC");
         */
        // RawContacts.LAST_TIME_CONTACTED will change at the very first time,
        // while Calls.DATE will change much later
        c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContacts.LAST_TIME_CONTACTED }, null, null,
                RawContacts.LAST_TIME_CONTACTED + " DESC");

        if (c != null) {
            if (c.moveToNext()) {
                lastCallTime = c.getLong(0);
            }
            c.close();
        }

        return lastCallTime;
    }

    // Sync --------------------------------------------------------------------
    /**
     * @return Whether need to re-initial.
     */
    public boolean isSyncNeedReinit() {
        boolean syncNeedReinit = true;
        try {
            syncNeedReinit = SharedPrefs.open(getContext()).getBoolean(
                    SharedPrefs.SYNC_NEED_REINIT, true);
        } catch (final ClassCastException e) {
            Debugger.logE(e);
        }
        return syncNeedReinit;
    }

    /**
     * @return The date of the last synchronization.
     */
    public long getLastSyncDate() {
        long lastSyncDate = 0L;
        try {
            lastSyncDate = SharedPrefs.open(getContext()).getLong(
                    SharedPrefs.LAST_SYNC_DATE, 0L);
        } catch (final ClassCastException e) {
            Debugger.logE(e);
        }
        return lastSyncDate;
    }

    /**
     * @param lastSyncDate
     *            The date of the last synchronization.
     * @return Whether success to update date in synchronization.
     */
    public boolean updateSyncDate(final long lastSyncDate) {
        SharedPrefs.open(getContext()).edit().putLong(
                SharedPrefs.LAST_SYNC_DATE, lastSyncDate).putBoolean(
                SharedPrefs.SYNC_NEED_REINIT, false).commit();
        return true;
    }

    /**
     * @return The max raw contact id.
     */
    public long getMaxRawContactsId() {
        long maxId = 0L;
        final Uri uri = getObservedContentResolver().insert(
                RawContacts.CONTENT_URI, new ContentValues(1));
        try {
            // Get the max ID in sqlite_sequence table
            maxId = Long.parseLong(uri.getLastPathSegment());
            // Delete the temporary record
            deleteContact(maxId, true, RawContact.SOURCE_PHONE, null, null);
        } catch (final NumberFormatException e) {
            Debugger.logE(e);
        } catch (final NullPointerException e) {
            Debugger.logE(e);
        }

        return maxId;
    }

    /**
     * 
     * @return The max raw contact id by query
     */

    public long getMaxRawContactsIdByQuery() {
        long maxId = 0L;
        Cursor c;

        c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID }, null, null,
                RawContacts._ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        Debugger.logI(new Object[] {}, "MaxRawContactsId is :" + maxId);
        return maxId;
    }

    /**
     * 
     * @param idSet
     *            <b>MUST</b> be in ascending order.
     * @return The array of raw bytes representing the flag of synchronization.
     */
    public byte[] getSyncFlags(final long[] idSet) {
        if (null == idSet) {
            Debugger.logE(new Object[] { idSet }, "Target ID list is null.");
            // Sync flags count is 0 by default
            return new byte[4];
        }
        if (idSet.length <= 0) {
            Debugger.logE(new Object[] { idSet }, "Target ID list is empty.");
            // Sync flags count is 0 by default
            return new byte[4];
        }

        // Build selection
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append("(");
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < idSet.length; i++){ if (idSet[i] > 0){
         * strBuf.append(RawContacts._ID + "=" + idSet[i] + " OR "); } } // Drop
         * the last ' OR ' strBuf.delete(strBuf.length() - 4, strBuf.length());
         */
        // Expression tree depth won't beyond 1000 in this way
        strBuf.append(RawContacts._ID + " IN(");
        for (int i = 0; i < idSet.length; i++) {
            strBuf.append(idSet[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")) AND " + RawContacts.DELETED + "<>"
                + DatabaseRecordEntity.TRUE
                // For MTK SIM Contacts feature.
                + " AND " + RawContactsContent.COLUMN_SOURCE_LOCATION + "="
                + RawContact.SOURCE_PHONE);
        selection = strBuf.toString();

        final ByteBuffer buffer = Global.getByteBuffer();
        // Sync flags count is 0 by default
        byte[] syncResultsRaw = new byte[4];
        Cursor c;

        c = getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID, RawContacts.VERSION,
                        Contacts.DISPLAY_NAME,
                        // Modify time. Added by Shaoying Han
                        RawContactsContent.COLUMN_MODIFY_TIME }, selection,
                null, RawContacts._ID + " ASC");

        buffer.clear();
        buffer.putInt(idSet.length);
        int index = 0;
        if (null != c) {
            outer: while (c.moveToNext()) {
                final long id = c.getLong(0);
                for (; index <= idSet.length; index++) {
                    if (index == idSet.length) {
                        break outer;
                    }

                    if (0 >= idSet[index]) {
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        buffer.putInt(-1);
                        RawTransUtil.putString(buffer, null);
                        // Modify time. Added by Shaoying Han
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        continue;
                    } else if (id == idSet[index]) {
                        buffer.putLong(id);
                        buffer.putInt(c.getInt(1));
                        RawTransUtil.putString(buffer, c.getString(2));
                        // Modify time. Added by Shaoying Han
                        buffer.putLong(c.getLong(3));
                    } else if (id > idSet[index]) {
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        buffer.putInt(-1);
                        RawTransUtil.putString(buffer, null);
                        // Modify time. Added by Shaoying Han
                        buffer.putLong(DatabaseRecordEntity.ID_NULL);
                        continue;
                    } else if (id < idSet[index]) {
                        break;
                    }
                }
            }
            c.close();
        }
        for (; index < idSet.length; index++) {
            buffer.putLong(DatabaseRecordEntity.ID_NULL);
            buffer.putInt(-1);
            RawTransUtil.putString(buffer, null);
            // Modify time. Added by Shaoying Han
            buffer.putLong(DatabaseRecordEntity.ID_NULL);
        }
        buffer.flip();
        syncResultsRaw = new byte[buffer.limit()];
        buffer.get(syncResultsRaw);

        return syncResultsRaw;
    }

    /**
     * 
     * @param idFrom
     *            The started id.
     * @param idTo
     *            The ended id.
     * @return A array of byte representing the result of synchronization.
     */
    public byte[] getSyncFlags(final long idFrom, final long idTo) {
        final ByteBuffer buffer = Global.getByteBuffer();
        // Sync flags count is 0 by default
        byte[] syncResultsRaw = new byte[4];
        Cursor c;

        c = getContentResolver().query(
                RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID, RawContacts.VERSION,
                        Contacts.DISPLAY_NAME,
                        // Modify time. Added by Shaoying Han
                        RawContactsContent.COLUMN_MODIFY_TIME },
                RawContacts._ID + ">=" + idFrom + " AND " + RawContacts._ID
                        + "<=" + idTo + " AND " + RawContacts.DELETED + "<>"
                        + DatabaseRecordEntity.TRUE
                        // For MTK SIM Contacts feature.
                        + " AND " + RawContactsContent.COLUMN_SOURCE_LOCATION
                        + "=" + RawContact.SOURCE_PHONE, null,
                RawContacts._ID + " ASC");

        buffer.clear();
        if (null != c) {
            buffer.putInt(c.getCount());
            while (c.moveToNext()) {
                // _id
                buffer.putLong(c.getLong(0));
                // version
                buffer.putInt(c.getInt(1));
                // displayname
                RawTransUtil.putString(buffer, c.getString(2));
                // Modify time. Added by Shaoying Han
                buffer.putLong(c.getLong(3));
            }
            c.close();
        } else {
            buffer.putInt(0);
        }
        buffer.flip();
        syncResultsRaw = new byte[buffer.limit()];
        buffer.get(syncResultsRaw);

        return syncResultsRaw;
    }

    // Faster, but not safe
    /**
     * @param raw
     *            The array of byte representing the contact to be added.
     * @return A array of byte representing the result of add synchronization.
     */
    public byte[] slowSyncAddDetailedContacts(final byte[] raw) {
        if (null == raw) {
            Debugger.logE(new Object[] { raw }, "Raw data is null.");
            return null;
        }

        final ByteBuffer buffer = ByteBuffer.wrap(raw);
        // Contacts count in the raw data
        int count;
        long beginId;
        try {
            // The first 4 bytes tell contacts count in the raw data.
            count = buffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw },
                    "Can not get the contacts count in raw data ", e);
            return null;
        }

        if (count < 0) {
            Debugger.logE(new Object[] { raw }, "Invalid contacts count "
                    + count);
            return null;
        }

        beginId = getMaxRawContactsId() + 1;
        final DefaultBulkInsertHelper contactDataInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(Data.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(),
                            "slowSyncAddDetailedContacts",
                            new Object[] { raw },
                            "Bulk insert contact data failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };
        final DefaultBulkInsertHelper rawContactsInserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(RawContacts.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(),
                            "slowSyncAddDetailedContacts",
                            new Object[] { raw },
                            "Bulk insert raw contacts failed, inserted "
                                    + insertedCount + ", expected "
                                    + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }

        };

        boolean shouldExit = false;
        // Insert contact data -------------------------------------------------
        for (int i = 0; i < count; i++) {
            // Read a raw contact(with contact data) from raw bytes
            final RawContact newContact = new RawContact();
            // newContact.readRaw(buffer); Changed by Shaoying Han
            newContact.readRawWithVersion(buffer, Config.VERSION_CODE);

            // Add content values for the contact data insertion
            for (final ContactData data : newContact.getAllContactData()) {
                data.setRawContactId(beginId + i);
                shouldExit = !contactDataInserter.append(ContactDataContent
                        .createMeasuredContentValues(data, false));
                if (shouldExit) {
                    Debugger.logE(new Object[] { raw },
                            "Error in bulk inserting contact data, "
                                    + "statusCode: "
                                    + contactDataInserter.getStatusCode());
                    return null;
                }
            }
        }
        // Bulk insert all ContentValues left
        shouldExit = !contactDataInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw },
                    "Error in bulk inserting contact data, " + "statusCode: "
                            + contactDataInserter.getStatusCode());
            return null;
        }

        // Insert raw contacts
        for (int i = 0; i < count; i++) {
            // Add content values for the raw contact insertion
            final MeasuredContentValues values = new MeasuredContentValues(2);
            values.put(RawContacts._ID, beginId + i);
            // Disable aggregation
            values.put(RawContacts.AGGREGATION_MODE,
                    RawContacts.AGGREGATION_MODE_DISABLED);

            shouldExit = !rawContactsInserter.append(values);
            if (shouldExit) {
                Debugger.logE(new Object[] { raw },
                        "Error in bulk inserting raw contacts, "
                                + "statusCode: "
                                + contactDataInserter.getStatusCode());
                return null;
            }
        }
        // Bulk insert all ContentValues left
        shouldExit = !rawContactsInserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw },
                    "Error in bulk inserting raw contacts, " + "statusCode: "
                            + rawContactsInserter.getStatusCode());
            return null;
        }

        // 2. Get needed info and set sync flags with them
        return getSyncFlags(beginId, beginId + count);
    }

    /**
     * @param contactIdTo
     *            The contact id in the last.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the raw contacts.
     */
    public void slowSyncGetAllRawContacts(final long contactIdTo,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { contactIdTo, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID, Contacts.DISPLAY_NAME,
                            // Modify time. Added by Shaoying Han
                            RawContactsContent.COLUMN_MODIFY_TIME,
                            RawContacts.VERSION },
                    RawContacts._ID + "<=" + contactIdTo + " AND "
                            + RawContacts.DELETED
                            + "<>"
                            + DatabaseRecordEntity.TRUE
                            // For MTK SIM Contacts feature.
                            + " AND "
                            + RawContactsContent.COLUMN_SOURCE_LOCATION + "="
                            + RawContact.SOURCE_PHONE, null, null);

            final FastRawContactsCursorParser parser = new FastRawContactsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param contactIdTo
     *            The contact id in the last.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the contact data.
     */
    public void slowSyncGetAllContactData(final long contactIdTo,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { contactIdTo, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    Data.CONTENT_URI,
                    new String[] {
                            Data._ID,
                            Data.RAW_CONTACT_ID,
                            // Data.IS_PRIMARY,
                            // Data.IS_SUPER_PRIMARY,
                            Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DATA3,
                            Data.DATA4, Data.DATA5, Data.DATA6, Data.DATA7,
                            Data.DATA8, Data.DATA9, Data.DATA10, Data.DATA15,
                            // Added by Shaoying Han
                            ContactDataContent.COLUMN_BINDING_SIM_ID },
                    Data.RAW_CONTACT_ID + "<=" + contactIdTo + " AND "
                            + Data.MIMETYPE + "<>'"
                            + CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE
                            + "'", null, Data.RAW_CONTACT_ID + " ASC");

            final FastContactDataCursorParser parser = new FastContactDataCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the flags of the synchronization.
     */
    public void fastSyncGetAllSyncFlags(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(new Object[] { consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        Cursor c = null;

        try {
            c = getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID, RawContacts.VERSION,
                            Contacts.DISPLAY_NAME,
                            RawContactsContent.COLUMN_MODIFY_TIME },
                    RawContacts.DELETED + "<>"
                            + DatabaseRecordEntity.TRUE
                            // For MTK SIM Contacts feature.
                            + " AND "
                            + RawContactsContent.COLUMN_SOURCE_LOCATION + "="
                            + RawContact.SOURCE_PHONE, null, null);

            final FastContactsSyncFlagsCursorParser parser = new FastContactsSyncFlagsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requestedContactIds
     *            The array of raw contact id.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the raw contact.
     */
    public void fastSyncGetRawContacts(final long[] requestedContactIds,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == requestedContactIds) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Requested contacts id list should not be null.");
            consumer.consume(null, 0, 0);
        } else if (0 >= requestedContactIds.length) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Requested contacts id list is empty.");
            consumer.consume(null, 0, 0);
        }

        Cursor c = null;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // Build selection string
        strBuf.append("(" + RawContacts._ID + " IN(");
        for (int i = 0; i < requestedContactIds.length; i++) {
            strBuf.append(requestedContactIds[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")) AND " + RawContacts.DELETED + "<>"
                + DatabaseRecordEntity.TRUE);
        // For MTK SIM Contacts feature.
        strBuf.append(" AND " + RawContactsContent.COLUMN_SOURCE_LOCATION + "="
                + RawContact.SOURCE_PHONE);
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID, Contacts.DISPLAY_NAME,
                            // Modify time. Added by Shaoying Han
                            RawContactsContent.COLUMN_MODIFY_TIME,
                            RawContacts.VERSION }, selection, null, null);

            final FastRawContactsCursorParser parser = new FastRawContactsCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param requestedContactIds
     *            The array of raw contact id.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the contact data.
     */
    public void fastSyncGetContactData(final long[] requestedContactIds,
            final IRawBlockConsumer consumer, final ByteBuffer buffer) {
        if (null == consumer) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Block consumer should not be null.");
            return;
        }
        if (null == requestedContactIds) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Requested contacts id list should not be null.");
            consumer.consume(null, 0, 0);
        }
        if (0 >= requestedContactIds.length) {
            Debugger.logE(
                    new Object[] { requestedContactIds, consumer, buffer },
                    "Requested contacts id list is empty.");
            consumer.consume(null, 0, 0);
        }

        Cursor c = null;
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // Build selection string
        strBuf.append("(" + Data.RAW_CONTACT_ID + " IN(");
        for (int i = 0; i < requestedContactIds.length; i++) {
            strBuf.append(requestedContactIds[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")) AND " + Data.MIMETYPE + "<>'"
                + CommonDataKinds.GroupMembership.CONTENT_ITEM_TYPE + "'");
        selection = strBuf.toString();

        try {
            c = getContentResolver().query(
                    Data.CONTENT_URI,
                    new String[] {
                            Data._ID,
                            Data.RAW_CONTACT_ID,
                            // Data.IS_PRIMARY,
                            // Data.IS_SUPER_PRIMARY,
                            Data.MIMETYPE, Data.DATA1, Data.DATA2, Data.DATA3,
                            Data.DATA4, Data.DATA5, Data.DATA6, Data.DATA7,
                            Data.DATA8, Data.DATA9, Data.DATA10, Data.DATA15,
                            // Added by Shaoying Han
                            ContactDataContent.COLUMN_BINDING_SIM_ID },
                    selection, null, Data.RAW_CONTACT_ID + " ASC");

            final FastContactDataCursorParser parser = new FastContactDataCursorParser(
                    c, consumer, buffer);
            parser.parse();
        } finally {
            // Release resources
            if (null != c && !c.isClosed()) {
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param raw
     *            The raw bytes of the contact.
     * @return A array of byte representing the result of add synchronization.
     */
    public byte[] fastSyncAddDetailedContacts(final byte[] raw) {
        return slowSyncAddDetailedContacts(raw);
    }

    /**
     * @param raw
     *            The raw bytes of the contacts.
     * @return The raw bytes representing result of the update.
     */
    public byte[] fastSyncUpdateDetailedContacts(final byte[] raw) {
        if (null == raw) {
            Debugger.logE(new Object[] { raw }, "Raw data is null.");
            return null;
        }

        ByteBuffer buffer = ByteBuffer.wrap(raw);
        // Contacts count in the raw data
        int count;
        long[] updatedIds;
        try {
            // The first 4 bytes tell contacts count in the raw data.
            count = buffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw },
                    "Can not get the contacts count in raw data ", e);
            return null;
        }

        if (count < 0) {
            Debugger.logE(new Object[] { raw }, "Invalid contacts count "
                    + count);
            return null;
        }

        updatedIds = new long[count];
        final ContentValues rawContactsValues = new ContentValues(1);
        ContentValues[] contactDataValues;

        long updateId;
        Cursor c;
        int insertedCount;
        for (int i = 0; i < count; i++) {
            // Read a raw contact(with contact data) from raw bytes
            final RawContact newContact = new RawContact();
            // newContact.readRaw(buffer); Changed by Shaoying Han
            newContact.readRawWithVersion(buffer, Config.VERSION_CODE);
            // Get the ID of the raw contact to update
            updateId = newContact.getId();
            // 1. Guarantee the raw contact to update exist
            c = getContentResolver().query(RawContacts.CONTENT_URI,
                    new String[] { RawContacts.DELETED },
                    RawContacts._ID + "=" + updateId, null, null);
            if (null == c) {
                Debugger.logE(new Object[] { raw },
                        "Cursor is null. Failed to find the raw contact "
                                + "to update.");
                return null;
            } else {
                if (c.getCount() <= 0) {
                    // Raw contact to update does not exist, insert it
                    rawContactsValues.clear();
                    rawContactsValues.put(RawContacts._ID, updateId);
                    getObservedContentResolver().insert(
                            RawContacts.CONTENT_URI, rawContactsValues);
                } else {
                    c.moveToFirst();
                    if (DatabaseRecordEntity.TRUE == c.getInt(0)) {
                        // Raw contact to update is set to deleted
                        // Delete it permanently first
                        deleteContact(updateId, true, RawContact.SOURCE_PHONE,
                                null, null);
                        // Then insert it back for further update
                        rawContactsValues.clear();
                        rawContactsValues.put(RawContacts._ID, updateId);
                        getObservedContentResolver().insert(
                                RawContacts.CONTENT_URI, rawContactsValues);
                    }
                }
                c.close();
            }

            // 2. Delete all old data
            getObservedContentResolver().delete(Data.CONTENT_URI
            /*
             * // Delete physically, not just set DELETED=1 .buildUpon()
             * .appendQueryParameter( ContactsContract.CALLER_IS_SYNCADAPTER,
             * "true") .build()
             */, Data.RAW_CONTACT_ID + "=" + updateId, null);

            // 3. Insert all new data
            contactDataValues = ContactDataContent.createContentValuesArray(
                    newContact, false);
            if (contactDataValues != null && contactDataValues.length > 0) {
                // Has data to insert
                insertedCount = getObservedContentResolver().bulkInsert(
                        Data.CONTENT_URI, contactDataValues);
                if (insertedCount != contactDataValues.length) {
                    // ERROR
                    buffer = null;
                    // System.gc();
                    return null;
                }
            }
            // 4. Update one contact finished
            updatedIds[i] = updateId;
        }

        // Get sync flags of updated raw contacts
        return getSyncFlags(updatedIds);
    }

    /**
     * @param ids
     *            List of the database id of the contacts to delete.
     * @return List of deletion results(successful or not).
     */
    public int fastSyncDeleteContacts(final long[] ids) {
        return fastDeleteContactsSourcedOnPhone(ids, false);
    }

    // Statistic ---------------------------------------------------------------
    /**
     * Get count of raw contacts.
     * 
     * @return Count of raw contacts in total.
     */
    public int getRawContactsCount() {
        int count;
        Cursor c;

        c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID },
                RawContacts.DELETED + "<>" + DatabaseRecordEntity.TRUE, null,
                null);
        if (c != null) {
            count = c.getCount();
            c.close();
        } else {
            count = 0;
        }
        return count;
    }

    /**
     * Get count of SIM contacts.
     * 
     * @param sourceLocation
     *            The id of the slot.
     * @return Count of SIM contacts in total.
     */
    public int getSimContactsCount(final int sourceLocation) { // int simId . Modified
        // by
        // Shaoying Han
        int count;
        Cursor c;

        c = getContentResolver()
                .query(getSimUri(sourceLocation),
                        new String[] { SimContactsContent.COLUMN_ID }, null,
                        null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        } else {
            count = 0;
        }
        return count;
    }

    /**
     * Get count of SIM contacts.
     * 
     * @return Count of SIM contacts in total.
     */
    public int getSimContactsCount() {
    	int count = 0;
        if (Config.MTK_GEMINI_SUPPORT) {
        	if (SystemInfoProxy.isSim1Accessible()) {
        		count += this.getSimContactsCount(DataStoreLocations.SIM1);
        	}
        	if (SystemInfoProxy.isSim2Accessible()) {
        		count += this.getSimContactsCount(DataStoreLocations.SIM2);
        	}
        	if (SystemInfoProxy.isSim3Accessible()) {
        		count += this.getSimContactsCount(DataStoreLocations.SIM3);
        	}
        	if (SystemInfoProxy.isSim4Accessible()) {
        		count += this.getSimContactsCount(DataStoreLocations.SIM4);
        	}
        } else {
        	if (SystemInfoProxy.isSimAccessible(SystemInfoProxy.getSimState(Message.SIM_ID))) {
        		count = this.getSimContactsCount(DataStoreLocations.SIM);        		
        	}
        }
        
        return count;
    }

    /**
     * @return
     */
    public int getAvailableContactsCount2() {
        int count = 0;
        Cursor c;
        String selection = null;
        final StringBuffer sourceLocationIn = new StringBuffer();
        // Contacts from phone
        sourceLocationIn.append('(');
        sourceLocationIn.append(RawContact.SOURCE_PHONE);
        // Contacts copied from SIM
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                // SIM1 is accessible
                // sourceLocationIn.append(", " + RawContact.SOURCE_SIM1);
                sourceLocationIn.append(", "
                        + Global.getSimIdBySlot(SimDetailInfo.SLOT_ID_ONE));
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                // SIM2 is accessible
                // sourceLocationIn.append(", " + RawContact.SOURCE_SIM2);
                sourceLocationIn.append(", "
                        + Global.getSimIdBySlot(SimDetailInfo.SLOT_ID_TWO));
            }
            if (SystemInfoProxy.isSim3Accessible()) {
                
                sourceLocationIn.append(", "
                        + Global.getSimIdBySlot(SimDetailInfo.SLOT_ID_THREE));
            }
            if (SystemInfoProxy.isSim4Accessible()) {
                
                sourceLocationIn.append(", "
                        + Global.getSimIdBySlot(SimDetailInfo.SLOT_ID_FOUR));
            }
        } else {
            if (SystemInfoProxy.isSimAccessible(SystemInfoProxy
                    .getSimState(Message.SIM_ID))) {
                // SIM is accessible
                // sourceLocationIn.append(", " + RawContact.SOURCE_SIM);
                sourceLocationIn.append(", "
                        + Global.getSimIdBySlot(SimDetailInfo.SLOT_ID_SINGLE));
            }
        }
        sourceLocationIn.append(')');
        // Only calculate contacts that are not deleted
        selection = RawContactsContent.COLUMN_SOURCE_LOCATION + " IN"
                + sourceLocationIn.toString() + " AND " + RawContacts.DELETED
                + "<>" + DatabaseRecordEntity.TRUE;
        c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID }, selection, null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        } else {
            count = 0;
        }
        Debugger.logD("getAvailableContactsCount : " + selection + "count : "
                + count);
        return count;
    }

    /**
     * @return The count of the available contacts.
     */
    public int getAvailableContactsCount() {
        int count = 0;
        Cursor c;
        String selection = null;
        // StringBuffer sourceLocationIn = new StringBuffer();
        // // Contacts from phone
        // sourceLocationIn.append('(');
        // sourceLocationIn.append(RawContact.SOURCE_PHONE);
        // sourceLocationIn.append(')');
        // Only calculate contacts that are not deleted

        //selection = null; // Jelly Bean
        selection = RawContactsContent.COLUMN_SOURCE_LOCATION + " = "
        + RawContact.SOURCE_PHONE + " AND " + RawContacts.DELETED
        + "<>" + DatabaseRecordEntity.TRUE;
        c = getContentResolver().query(RawContacts.CONTENT_URI,
                new String[] { RawContacts._ID }, selection, null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        } else {
            count = 0;
        }
        final int simCount = getSimContactsCount();
        Debugger.logD("getAvailableContactsCount from phone : " + count);
        Debugger.logD("getAvailableContactsCount from SIM : " + simCount);
        return count + simCount;
    }

    // // The following lines are provided and maintained by Mediatek inc.
    // // Added Local Account Type
    // public static final String ACCOUNT_TYPE_SIM = "SIM Account";
    // public static final String ACCOUNT_TYPE_USIM = "USIM Account";
    // public static final String ACCOUNT_TYPE_LOCAL_PHONE =
    // "Local Phone Account";
    //
    // // Added Local Account Name - For Sim/Usim Only
    // public static final String ACCOUNT_NAME_SIM = "SIM" + SimSlot.SLOT_ID1;
    // public static final String ACCOUNT_NAME_SIM2 = "SIM" + SimSlot.SLOT_ID2;
    // public static final String ACCOUNT_NAME_USIM = "USIM" + SimSlot.SLOT_ID1;
    // public static final String ACCOUNT_NAME_USIM2 = "USIM" +
    // SimSlot.SLOT_ID2;
    // public static final String ACCOUNT_NAME_LOCAL_PHONE = "Phone";
    //    
    // public static interface SimType {
    // public static final String SIM_TYPE_USIM_TAG = "USIM";
    //
    // public static final int SIM_TYPE_SIM = 0;
    // public static final int SIM_TYPE_USIM = 1;
    // }
    //     
    // public static interface SimSlot {
    // public static final int SLOT_NONE = -1;
    // public static final int SLOT_SINGLE = 0;
    // public static final int SLOT_ID1 =
    // com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1;
    // public static final int SLOT_ID2 =
    // com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2;
    // }
    //    
    // public static int getSimTypeBySlot(int slotId) {
    // final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
    // .getService(Context.TELEPHONY_SERVICE));
    // int simType = SimType.SIM_TYPE_SIM;
    // try {
    // if (Config.MTK_GEMINI_SUPPORT) {
    // if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardTypeGemini(slotId)))
    // simType = SimType.SIM_TYPE_USIM;
    // } else {
    // if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType()))
    // simType = SimType.SIM_TYPE_USIM;
    // }
    // } catch (Exception e) {
    // e.printStackTrace();
    // }
    // Debugger.logI(new Object[] { slotId }, "simType : " + simType);
    // return simType;
    // }
    //    
    // public static boolean isSimInserted(int slotId) {
    // final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
    // .getService(Context.TELEPHONY_SERVICE));
    // boolean isSimInsert = false;
    // try {
    // if (iTel != null) {
    // if (Config.MTK_GEMINI_SUPPORT) {
    // isSimInsert = iTel.isSimInsert(slotId);
    // } else {
    // isSimInsert = iTel.isSimInsert(0);
    // }
    // }
    // } catch (RemoteException e) {
    // e.printStackTrace();
    // isSimInsert = false;
    // }
    // return isSimInsert;
    // }
    //    
    //    
    // // The following lines are provided and maintained by Mediatek inc.
    // public static String getAccountTypeBySlot(int slotId) {
    // Debugger.logI("getAccountTypeBySlot()+ - slotId:" + slotId);
    // if (slotId < SimSlot.SLOT_ID1 || slotId > SimSlot.SLOT_ID2) {
    // Debugger.logE("Error! - slot id error. slotid:" + slotId);
    // return null;
    // }
    // int simtype = SimType.SIM_TYPE_SIM;
    // String simAccountType = ACCOUNT_TYPE_SIM;
    //
    // if (isSimInserted(slotId)) {
    // simtype = getSimTypeBySlot(slotId);
    // if (SimType.SIM_TYPE_USIM == simtype) {
    // simAccountType = ACCOUNT_TYPE_USIM;
    // }
    // } else {
    // Debugger.logE("Error! getAccountTypeBySlot - slotId:" + slotId +
    // " no sim inserted!");
    // simAccountType = null;
    // }
    // Debugger.logI("getAccountTypeBySlot()- - slotId:" + slotId +
    // " AccountType:" + simAccountType);
    // return simAccountType;
    // }
    //
    // public static String getSimAccountNameBySlot(int slotId) {
    // String retSimName = null;
    // int simType = SimType.SIM_TYPE_SIM;
    //
    // Debugger.logI("getSimAccountNameBySlot()+ slotId:" + slotId);
    // if (!isSimInserted(slotId)) {
    // Debugger.logE("getSimAccountNameBySlot Error! - SIM not inserted!");
    // return retSimName;
    // }
    //
    // simType = getSimTypeBySlot(slotId);
    // Debugger.logI("getSimAccountNameBySlot() slotId:" + slotId +
    // " simType(0-SIM/1-USIM):" + simType);
    //
    // if (SimType.SIM_TYPE_SIM == simType) {
    // retSimName = ACCOUNT_NAME_SIM;
    // if (SimSlot.SLOT_ID2 == slotId) {
    // retSimName = ACCOUNT_NAME_SIM2;
    // }
    // } else if (SimType.SIM_TYPE_USIM == simType) {
    // retSimName = ACCOUNT_NAME_USIM;
    // if (SimSlot.SLOT_ID2 == slotId) {
    // retSimName = ACCOUNT_NAME_USIM2;
    // }
    // } else {
    // Debugger.logE("getSimAccountNameBySlot() Error!  get SIM Type error! simType:"
    // + simType);
    // }
    //
    // Debugger.logI("getSimAccountNameBySlot()- slotId:" + slotId + " SimName:"
    // + retSimName);
    // return retSimName;
    // }
    //    
    // /*
    // * There are some differences with iccprovider
    // */
    // public static boolean isSimUsimType(int slotId) {
    // final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
    // .getService(Context.TELEPHONY_SERVICE));
    // boolean isUsim = false;
    // try {
    // if(slotId == 0){
    // if (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardType()))
    // isUsim = true;
    // } else if (slotId > 0){
    // // this slotId in deamon equals real slotId + 1
    // if
    // (SimType.SIM_TYPE_USIM_TAG.equals(iTel.getIccCardTypeGemini(slotId-1)))
    // isUsim = true;
    // } else {
    // Debugger.logE("slotId < 0");
    // }
    // } catch (Exception e) {
    // Debugger.logE("catched exception.");
    // e.printStackTrace();
    // }
    // Debugger.logI(new Object[] { slotId }, "isUsim : " + isUsim);
    // return isUsim;
    // }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================
}
