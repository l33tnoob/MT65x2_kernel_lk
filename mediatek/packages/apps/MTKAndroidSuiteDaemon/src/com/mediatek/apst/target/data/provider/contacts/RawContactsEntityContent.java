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

package com.mediatek.apst.target.data.provider.contacts;

import android.database.Cursor;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.RawContactsEntity;

import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.contacts.ContactData;
import com.mediatek.apst.util.entity.contacts.Email;
import com.mediatek.apst.util.entity.contacts.GroupMembership;
import com.mediatek.apst.util.entity.contacts.Im;
import com.mediatek.apst.util.entity.contacts.Nickname;
import com.mediatek.apst.util.entity.contacts.Note;
import com.mediatek.apst.util.entity.contacts.Organization;
import com.mediatek.apst.util.entity.contacts.Phone;
import com.mediatek.apst.util.entity.contacts.Photo;
import com.mediatek.apst.util.entity.contacts.StructuredName;
import com.mediatek.apst.util.entity.contacts.StructuredPostal;
import com.mediatek.apst.util.entity.contacts.Website;

public class RawContactsEntityContent {

    /**
     * @param cursor
     *            The cursor about ContactData.
     * @return A ContactData or null.
     */
    public static ContactData cursorToContactData(final Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            return null;
        }

        int colId;
        ContactData data = null;
        long id = DatabaseRecordEntity.ID_NULL;
        long rawContactId = DatabaseRecordEntity.ID_NULL;
        boolean primary = false;
        boolean superPrimary = false;
        String strMimeType = null;

        try {
            // raw contact id
            colId = cursor.getColumnIndex(RawContactsEntity._ID);
            if (-1 != colId) {
                rawContactId = cursor.getLong(colId);
            }
            // data id
            colId = cursor.getColumnIndex(RawContactsEntity.DATA_ID);
            if (-1 != colId) {
                if (cursor.isNull(colId)) {
                    // Data row ID will be null only if the raw contact has no
                    // data rows, thus, we can return by here
                    data = new ContactData(DatabaseRecordEntity.ID_NULL, -1);
                    // Set the raw contact id before return, for this could tell
                    // the caller that this raw contact has no data
                    data.setRawContactId(rawContactId);
                    return data;
                } else {
                    id = cursor.getLong(colId);
                }
            }
            // is primary
            colId = cursor.getColumnIndex(RawContactsEntity.IS_PRIMARY);
            if (-1 != colId) {
                primary = (cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // is super primary
            colId = cursor.getColumnIndex(RawContactsEntity.IS_SUPER_PRIMARY);
            if (-1 != colId) {
                superPrimary = (cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // MIME type
            colId = cursor.getColumnIndex(RawContactsEntity.MIMETYPE);
            if (-1 != colId) {
                strMimeType = cursor.getString(colId);
            }

            // Create a new contact data object according to its MIME type
            if (null == strMimeType) {
                Debugger.logW(new Object[] { cursor },
                        "mimeType is absent in cursor.");
                return null;
            } else if (strMimeType.equals(StructuredName.MIME_TYPE_STRING)) {
                // Name --------------------------------------------
                data = cursorToStructuredName(cursor);
            } else if (strMimeType.equals(Phone.MIME_TYPE_STRING)) {
                // Phone -------------------------------------------
                data = cursorToPhone(cursor);
            } else if (strMimeType.equals(Photo.MIME_TYPE_STRING)) {
                // Photo -------------------------------------------
                data = cursorToPhoto(cursor);
            } else if (strMimeType.equals(Im.MIME_TYPE_STRING)) {
                // IM ----------------------------------------------
                data = cursorToIm(cursor);
            } else if (strMimeType.equals(Email.MIME_TYPE_STRING)) {
                // Email -------------------------------------------
                data = cursorToEmail(cursor);
            } else if (strMimeType.equals(StructuredPostal.MIME_TYPE_STRING)) {
                // Postal ------------------------------------------
                data = cursorToStructuredPostal(cursor);
            } else if (strMimeType.equals(Organization.MIME_TYPE_STRING)) {
                // Organization ------------------------------------
                data = cursorToOrganization(cursor);
            } else if (strMimeType.equals(Nickname.MIME_TYPE_STRING)) {
                // Nickname ----------------------------------------
                data = cursorToNickname(cursor);
            } else if (strMimeType.equals(Website.MIME_TYPE_STRING)) {
                // Website -----------------------------------------
                data = cursorToWebsite(cursor);
            } else if (strMimeType.equals(Note.MIME_TYPE_STRING)) {
                // Note --------------------------------------------
                data = cursorToNote(cursor);
            } else if (strMimeType.equals(GroupMembership.MIME_TYPE_STRING)) {
                // Group membership --------------------------------
                data = cursorToGroupMembership(cursor);
            } else {
                Debugger.logW(new Object[] { cursor },
                        "Ignored unknown mimeType: " + strMimeType);
                return null;
            }

            // At last, set common fields
            data.setId(id);
            data.setRawContactId(rawContactId);
            data.setPrimary(primary);
            data.setSuperPrimary(superPrimary);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }

        return data;
    }

    /**
     * @param cursor
     *            The cursor about StructuredName.
     * @return A StructuredName or null.
     */
    private static StructuredName cursorToStructuredName(final Cursor cursor) {
        final StructuredName data = new StructuredName();

        try {
            int colId;
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.DISPLAY_NAME);
            if (-1 != colId) {
                data.setDisplayName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.GIVEN_NAME);
            if (-1 != colId) {
                data.setGivenName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.FAMILY_NAME);
            if (-1 != colId) {
                data.setFamilyName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.PREFIX);
            if (-1 != colId) {
                data.setPrefix(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.MIDDLE_NAME);
            if (-1 != colId) {
                data.setMiddleName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.SUFFIX);
            if (-1 != colId) {
                data.setSuffix(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.PHONETIC_GIVEN_NAME);
            if (-1 != colId) {
                data.setPhoneticGivenName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.PHONETIC_MIDDLE_NAME);
            if (-1 != colId) {
                data.setPhoneticMiddleName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredName.PHONETIC_FAMILY_NAME);
            if (-1 != colId) {
                data.setPhoneticFamilyName(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Phone.
     * @return A Phone or null.
     */
    private static Phone cursorToPhone(final Cursor cursor) {
        final Phone data = new Phone();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Phone.NUMBER);
            if (-1 != colId) {
                data.setNumber(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Phone.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Phone.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
            // Added by Shaoying Han
            colId = cursor
                    .getColumnIndex(ContactDataContent.COLUMN_BINDING_SIM_ID);
            if (-1 != colId) {
                data.setBindingSimId(cursor.getInt(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Photo.
     * @return A Photo or null.
     */
    private static Photo cursorToPhoto(final Cursor cursor) {
        final Photo data = new Photo();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Photo.PHOTO);
            if (-1 != colId) {
                data.setPhotoBytes(cursor.getBlob(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Im.
     * @return A Im or null.
     */
    private static Im cursorToIm(final Cursor cursor) {
        final Im data = new Im();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Im.DATA);
            if (-1 != colId) {
                data.setData(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Im.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Im.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
            // Note that DATA4 is not used in Im data row
            colId = cursor.getColumnIndex(CommonDataKinds.Im.PROTOCOL);
            if (-1 != colId) {
                data.setProtocol(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Im.CUSTOM_PROTOCOL);
            if (-1 != colId) {
                data.setCustomProtocol(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about email.
     * @return A Email or null.
     */
    private static Email cursorToEmail(final Cursor cursor) {
        final Email data = new Email();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Email.DATA);
            if (-1 != colId) {
                data.setData(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Email.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Email.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about StructuredPostal.
     * @return A StructuredPostal or null.
     */
    private static StructuredPostal cursorToStructuredPostal(final Cursor cursor) {
        final StructuredPostal data = new StructuredPostal();

        try {
            int colId;
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS);
            if (-1 != colId) {
                data.setFormattedAddress(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.STREET);
            if (-1 != colId) {
                data.setStreet(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.POBOX);
            if (-1 != colId) {
                data.setPobox(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.NEIGHBORHOOD);
            if (-1 != colId) {
                data.setNeighborhood(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.CITY);
            if (-1 != colId) {
                data.setCity(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.REGION);
            if (-1 != colId) {
                data.setRegion(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.POSTCODE);
            if (-1 != colId) {
                data.setPostcode(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.StructuredPostal.COUNTRY);
            if (-1 != colId) {
                data.setCountry(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Organization.
     * @return A Organization or null.
     */
    private static Organization cursorToOrganization(final Cursor cursor) {
        final Organization data = new Organization();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Organization.COMPANY);
            if (-1 != colId) {
                data.setCompany(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Organization.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Organization.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Organization.TITLE);
            if (-1 != colId) {
                data.setTitle(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.Organization.DEPARTMENT);
            if (-1 != colId) {
                data.setDepartment(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.Organization.JOB_DESCRIPTION);
            if (-1 != colId) {
                data.setJobDescription(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Organization.SYMBOL);
            if (-1 != colId) {
                data.setSymbol(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.Organization.PHONETIC_NAME);
            if (-1 != colId) {
                data.setPhoneticName(cursor.getString(colId));
            }
            colId = cursor
                    .getColumnIndex(CommonDataKinds.Organization.OFFICE_LOCATION);
            if (-1 != colId) {
                data.setOfficeLocation(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Nickname.
     * @return A Nickname or null.
     */
    private static Nickname cursorToNickname(final Cursor cursor) {
        final Nickname data = new Nickname();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Nickname.NAME);
            if (-1 != colId) {
                data.setName(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Nickname.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Nickname.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Website.
     * @return A Website or null.
     */
    private static Website cursorToWebsite(final Cursor cursor) {
        final Website data = new Website();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Website.URL);
            if (-1 != colId) {
                data.setUrl(cursor.getString(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Website.TYPE);
            if (-1 != colId) {
                data.setType(cursor.getInt(colId));
            }
            colId = cursor.getColumnIndex(CommonDataKinds.Website.LABEL);
            if (-1 != colId) {
                data.setLabel(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about Note.
     * @return A Note or null.
     */
    private static Note cursorToNote(final Cursor cursor) {
        final Note data = new Note();

        try {
            int colId;
            colId = cursor.getColumnIndex(CommonDataKinds.Note.NOTE);
            if (-1 != colId) {
                data.setNote(cursor.getString(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }

    /**
     * @param cursor
     *            The cursor about GroupMembership.
     * @return A GroupMembership or null.
     */
    private static GroupMembership cursorToGroupMembership(final Cursor cursor) {
        final GroupMembership data = new GroupMembership();

        try {
            int colId;
            colId = cursor
                    .getColumnIndex(CommonDataKinds.GroupMembership.GROUP_ROW_ID);
            if (-1 != colId) {
                data.setGroupId(cursor.getLong(colId));
            }
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
            return null;
        }
        return data;
    }
}
