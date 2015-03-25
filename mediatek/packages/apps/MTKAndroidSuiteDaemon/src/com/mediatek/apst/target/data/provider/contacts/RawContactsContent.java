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
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;

import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.FeatureOptionControl;
import com.mediatek.apst.util.entity.DataStoreLocations;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.contacts.RawContact;


import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

public class RawContactsContent {
    // ==============================================================
    // Constants
    // ==============================================================
    // For MTK SIM Contacts feature.
    /**
     * Column name of indicate_phone_or_sim_contact. This is the simId column.
     */
    public static final String COLUMN_SOURCE_LOCATION = "indicate_phone_or_sim_contact";
    /**
     * Column name of timestamp. Added by Shaoying Han
     */
    public static final String COLUMN_MODIFY_TIME = "timestamp";

    public static final String COLUMN_INDEX_IN_SIM = "index_in_sim";

    public static final int INSERT_FAIL = -1001;

    /**
     * @param cursor
     *            The cursor about RawContact.
     * @return A RawContact or null.
     */
    public static RawContact cursorToRawContact(final Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            Debugger.logW(new Object[] { cursor }, "Cursor is null.");
            return null;
        }

        // Create a new raw contact object
        final RawContact contact = new RawContact();
        contact.setStoreLocation(DataStoreLocations.PHONE);

        try {
            // Read basic info fields of the raw contact -----------------------
            int colId;
            // id
            colId = cursor.getColumnIndex(RawContacts._ID);
            if (-1 != colId) {
                contact.setId(cursor.getLong(colId));
            }
            // display name
            colId = cursor.getColumnIndex(Contacts.DISPLAY_NAME);
            if (-1 != colId) {
                contact.setDisplayName(cursor.getString(colId));
            }
            // starred
            colId = cursor.getColumnIndex(RawContacts.STARRED);
            if (-1 != colId) {
                contact
                        .setStarred(cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // send to voicemail
            colId = cursor.getColumnIndex(RawContacts.SEND_TO_VOICEMAIL);
            if (-1 != colId) {
                contact
                        .setSendToVoicemail(cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // times contacted
            /*
             * colId = c.getColumnIndex(RawContacts.TIMES_CONTACTED); if (-1 !=
             * colId){ contact.setTimesContacted(c.getInt(colId)); }
             */
            // last time contacted
            /*
             * colId = c.getColumnIndex(RawContacts.LAST_TIME_CONTACTED); if (-1
             * != colId){ contact.setLastTimeContacted(c.getLong(colId)); }
             */
            // custom ringtone
            /*
             * colId = c.getColumnIndex(RawContacts.CUSTOM_RINGTONE); if (-1 !=
             * colId){ contact.setCustomRingtone(c.getString(colId)); }
             */
            // version
            colId = cursor.getColumnIndex(RawContacts.VERSION);
            if (-1 != colId) {
                contact.setVersion(cursor.getInt(colId));
            }
            // dirty
            colId = cursor.getColumnIndex(RawContacts.DIRTY);
            if (-1 != colId) {
                contact
                        .setDirty(cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }

            // For MTK SIM Contacts feature.
            // indicate_phone_or_sim_contact
            colId = cursor.getColumnIndex(COLUMN_SOURCE_LOCATION);
            if (-1 != colId) {
                final int indicateSimOrPhone = cursor.getInt(colId);
                // For MTK SIM Contacts feature.
                // sourceLocation
                contact.setSourceLocation(Global
                        .getSourceLocationById(indicateSimOrPhone));
                // Sim Id. Added by Shaoying Han
                contact.setSimId(indicateSimOrPhone);
                // Sim Name. Added by Shaoying Han
                contact.setSimName(Global.getSimName(cursor.getInt(colId)));
            }
            // Modify time. Added by Shaoying Han
            colId = cursor.getColumnIndex(COLUMN_MODIFY_TIME);
            if (-1 != colId) {
                contact.setModifyTime(cursor.getLong(colId));
            }

            if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0) {
                colId = cursor.getColumnIndex(COLUMN_INDEX_IN_SIM);
                if (-1 != colId) {
                    contact.setSimIndex(cursor.getInt(colId));
                    Debugger.logI(new Object[] { cursor },
                            "contact.getSimIndex()" + contact.getSimIndex());
                } else {
                    contact.setSimIndex(-1);
                }
            }
            // contact.setIccid(Global.getIccid(contact.getSourceLocation()));
            // Debugger.logE(new
            // Object[]{},"SourceLocation="+contact.getSourceLocation());
            // Debugger.logE(new Object[]{},"Iccid="+contact.getIccid());
            // -----------------------------------------------------------------
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
        }

        return contact;
    }

    /**
     * 
     * @param cursor
     *            The cursor to parse.
     * @param buffer
     *            The buffer to store info about this entity.
     * @return The result of this operation.
     */
    public static int cursorToRaw(final Cursor cursor, final ByteBuffer buffer) {
        if (null == cursor) {
            Debugger.logW(new Object[] { cursor, buffer }, "Cursor is null.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            Debugger.logW(new Object[] { cursor, buffer },
                    "Cursor has moved to the end.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (null == buffer) {
            Debugger.logW(new Object[] { cursor, buffer }, "Buffer is null.");
            return IRawBufferWritable.RESULT_FAIL;
        }
        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(RawContacts._ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // store location
            buffer.putInt(DataStoreLocations.PHONE);
            // display name
            colId = cursor.getColumnIndex(Contacts.DISPLAY_NAME);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // primary number
            RawTransUtil.putString(buffer, null);
            // group memberships
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // sim name Changed by Shaoying Han
            colId = cursor.getColumnIndex(COLUMN_SOURCE_LOCATION);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, Global.getSimName(cursor
                        .getInt(colId)));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // Modify time. Added by Shaoying Han
            colId = cursor.getColumnIndex(COLUMN_MODIFY_TIME);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }

            // starred
            colId = cursor.getColumnIndex(RawContacts.STARRED);
            if (-1 != colId) {
                RawTransUtil.putBoolean(buffer,
                        (cursor.getInt(colId) == DatabaseRecordEntity.TRUE));
            } else {
                RawTransUtil.putBoolean(buffer, false);
            }
            // send to voicemail
            colId = cursor.getColumnIndex(RawContacts.SEND_TO_VOICEMAIL);
            if (-1 != colId) {
                RawTransUtil.putBoolean(buffer,
                        (cursor.getInt(colId) == DatabaseRecordEntity.TRUE));
            } else {
                RawTransUtil.putBoolean(buffer, false);
            }
            // times contacted
            // last time contacted
            // custom ringtone
            // version
            colId = cursor.getColumnIndex(RawContacts.VERSION);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
            } else {
                buffer.putInt(-1);
            }
            // dirty
            // names
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // phones
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // photos
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // emails
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // ims
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // postals
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // organizations
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // notes
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // nicknames
            buffer.putInt(0); // Only put a int to tell that list size is 0
            // websites
            buffer.putInt(0); // Only put a int to tell that list size is 0

            colId = cursor.getColumnIndex(COLUMN_SOURCE_LOCATION);
            if (-1 != colId) {
                final int indicateSimOrPhone = cursor.getInt(colId);
                // For MTK SIM Contacts feature.
                // indicate_phone_or_sim_contact
                buffer.putInt(Global.getSourceLocationById(indicateSimOrPhone));
                // simId
                buffer.putInt(indicateSimOrPhone);
                // RawTransUtil.putString(buffer, Global.getIccid(Global
                // .getSourceLocationById(indicateSimOrPhone)));
                // Debugger.logE(new
                // Object[]{},"SourceLocation="+Global.getSourceLocationById(indicateSimOrPhone));
                // Debugger.logE(new Object[]{},"Iccid="+Global.getIccid(Global
                // .getSourceLocationById(indicateSimOrPhone)));

            } else {
                buffer.putInt(RawContact.SOURCE_PHONE);
                // simId. -1 means can not get sim id.
                buffer.putInt(-1);

                // RawTransUtil.putString(buffer, null);
            }

            if (FeatureOptionControl.CONTACT_N_USIMGROUP != 0) {
                colId = cursor.getColumnIndex(COLUMN_INDEX_IN_SIM);
                if (-1 != colId) {
                    buffer.putInt(cursor.getInt(colId));
                    Debugger.logI(new Object[] { cursor },
                            "c.getColumnIndex(COLUMN_INDEX_IN_SIM): "
                                    + cursor.getInt(colId));
                } else {
                    buffer.putInt(-1);
                }
            }

        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor, buffer }, null, e);
            buffer.reset();
            return IRawBufferWritable.RESULT_FAIL;
        } catch (final BufferOverflowException e) {
            /*
             * DebugHelper.logW("[RawContactsContent] " + "cursorToRaw(" + c +
             * "): Not enough space left in " + "buffer. ", e);
             */
            buffer.reset();
            return IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER;
        }
        return IRawBufferWritable.RESULT_SUCCESS;
    }
}
