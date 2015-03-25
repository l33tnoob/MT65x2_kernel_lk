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

import android.content.ContentValues;
import android.database.Cursor;
import android.provider.ContactsContract.Groups;

import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.contacts.Group;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.util.List;

public class GroupContent {
    /**
     * @param c
     *            The group cursor.
     * @return The group or null.
     */
    public static Group cursorToGroup(Cursor c) {
        if (null == c || c.getPosition() == -1
                || c.getPosition() == c.getCount()) {
            Debugger.logW(new Object[] { c }, "Cursor is null.");
            return null;
        }

        // Create a new group object
        Group group = new Group();

        try {
            int colId;
            // id
            colId = c.getColumnIndex(Groups._ID);
            if (-1 != colId) {
                group.setId(c.getLong(colId));
            }
            // title
            colId = c.getColumnIndex(Groups.TITLE);
            if (-1 != colId) {
                group.setTitle(c.getString(colId));
            }
            // notes
            colId = c.getColumnIndex(Groups.NOTES);
            if (-1 != colId) {
                group.setNotes(c.getString(colId));
            }
            // system id
            colId = c.getColumnIndex(Groups.SYSTEM_ID);
            if (-1 != colId) {
                group.setSystemId(c.getString(colId));
            }
            // deleted
            colId = c.getColumnIndex(Groups.DELETED);
            if (-1 != colId) {
                group.setDeleted(c.getString(colId));
            }
            /**
             * Jelly Bean account_name account_type fields don't exist in group
             * table
             */
            // account_name
            colId = c.getColumnIndex(Groups.ACCOUNT_NAME);
            if (-1 != colId) {
                group.setAccount_name(c.getString(colId));
            }
            // account_type
            colId = c.getColumnIndex(Groups.ACCOUNT_TYPE);
            if (-1 != colId) {
                group.setAccount_type(c.getString(colId));
            }
            // version
            colId = c.getColumnIndex(Groups.VERSION);
            if (-1 != colId) {
                group.setVersion(c.getString(colId));
            }
            // dirty
            colId = c.getColumnIndex(Groups.DIRTY);
            if (-1 != colId) {
                group.setDirty(c.getString(colId));
            }
            // group_visible
            colId = c.getColumnIndex(Groups.GROUP_VISIBLE);
            if (-1 != colId) {
                group.setGroup_visible(c.getString(colId));
            }
            // should_sync
            colId = c.getColumnIndex(Groups.SHOULD_SYNC);
            if (-1 != colId) {
                group.setShould_sync(c.getString(colId));
            }
        } catch (IllegalArgumentException e) {
            Debugger.logE(new Object[] { c }, null, e);
        }

        return group;
    }

    /**
     * 
     * @param c
     *            The group cursor.
     * @param buffer
     *            The buffer to save the raw byte.
     * @return Whether succeed to get raw data of group.
     */
    public static int cursorToRaw(Cursor c, ByteBuffer buffer) {
        if (null == c) {
            Debugger.logW(new Object[] { c, buffer }, "Cursor is null.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (c.getPosition() == -1 || c.getPosition() == c.getCount()) {
            Debugger.logW(new Object[] { c, buffer },
                    "Cursor has moved to the end.");
            return IRawBufferWritable.RESULT_FAIL;
        } else if (null == buffer) {
            Debugger.logW(new Object[] { c, buffer }, "Buffer is null.");
            return IRawBufferWritable.RESULT_FAIL;
        }
        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            int colId;
            // id
            colId = c.getColumnIndex(Groups._ID);
            if (-1 != colId) {
                buffer.putLong(c.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // title
            colId = c.getColumnIndex(Groups.TITLE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // notes
            colId = c.getColumnIndex(Groups.NOTES);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // system id
            colId = c.getColumnIndex(Groups.SYSTEM_ID);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // deleted
            colId = c.getColumnIndex(Groups.DELETED);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            /**
             * Jelly Bean account_name account_type fields don't exist in group
             * table
             */
            // account_name
            colId = c.getColumnIndex(Groups.ACCOUNT_NAME);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // account_type
            colId = c.getColumnIndex(Groups.ACCOUNT_TYPE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // version
            colId = c.getColumnIndex(Groups.VERSION);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // dirty
            colId = c.getColumnIndex(Groups.DIRTY);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // group_visible
            colId = c.getColumnIndex(Groups.GROUP_VISIBLE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // should_sync
            colId = c.getColumnIndex(Groups.SHOULD_SYNC);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }

        } catch (IllegalArgumentException e) {
            Debugger.logE(new Object[] { c, buffer }, null, e);
            buffer.reset();
            return IRawBufferWritable.RESULT_FAIL;
        } catch (BufferOverflowException e) {
            /*
             * DebugHelper.logW("[GroupContent] cursorToRaw(" + c + "): " +
             * "Not enough space left in buffer. ", e);
             */
            buffer.reset();
            return IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER;
        }
        return IRawBufferWritable.RESULT_SUCCESS;
    }

    /*
     * added by Yu
     */
    /**
     * @param groups
     *            The group list.
     * @return The content values of the group.
     */
    public static ContentValues[] groupsToValues(List<Group> groups) {
        ContentValues[] values = null;
        if (null != groups) {
            // for (Group group : groups) {
            // if (group.getGroup_visible().equals("1")) {
            // groups.remove(group);
            // }
            // }
            values = new ContentValues[groups.size()];
            for (int i = 0; i < groups.size(); i++) {
                values[i] = new ContentValues();

                String title = groups.get(i).getTitle();
                if (null != title) {
                    values[i].put(Groups.TITLE, title);
                }
                String note = groups.get(i).getNotes();
                if (null != note) {
                    values[i].put(Groups.NOTES, note);
                }
                String delete = groups.get(i).getDeleted();
                if (null != delete) {
                    values[i].put(Groups.DELETED, delete);
                }
                String accountName = groups.get(i).getAccount_name();
                if (null != accountName) {
                    values[i].put(Groups.ACCOUNT_NAME, accountName);
                }
                String accountType = groups.get(i).getAccount_type();
                if (null != accountType) {
                    values[i].put(Groups.ACCOUNT_TYPE, accountType);
                }
                String version = groups.get(i).getVersion();
                if (null != version) {
                    values[i].put(Groups.VERSION, version);
                }
                String dirty = groups.get(i).getDirty();
                if (null != dirty) {
                    values[i].put(Groups.DIRTY, dirty);
                }
                String groupVisible = groups.get(i).getGroup_visible();
                if (null != groupVisible) {
                    values[i].put(Groups.GROUP_VISIBLE, groupVisible);
                }
                String shouldSync = groups.get(i).getShould_sync();
                if (null != shouldSync) {
                    values[i].put(Groups.SHOULD_SYNC, shouldSync);
                }
            }
        }
        return values;
    }

    /**
     * @param group
     *            The group to get content.
     * @return The content values of the group.
     */
    public static ContentValues groupToValues(Group group) {
        ContentValues values = null;
        if (null != group) {
            values = new ContentValues();

            String title = group.getTitle();
            if (null != title) {
                values.put(Groups.TITLE, title);
            }
            String note = group.getNotes();
            if (null != note) {
                values.put(Groups.NOTES, note);
            }
            String delete = group.getDeleted();
            if (null != delete) {
                values.put(Groups.DELETED, delete);
            }
            String accountName = group.getAccount_name();
            if (null != accountName) {
                values.put(Groups.ACCOUNT_NAME, accountName);
            }
            String accountType = group.getAccount_type();
            if (null != accountType) {
                values.put(Groups.ACCOUNT_TYPE, accountType);
            }
            String version = group.getVersion();
            if (null != version) {
                values.put(Groups.VERSION, version);
            }
            String dirty = group.getDirty();
            if (null != dirty) {
                values.put(Groups.DIRTY, dirty);
            }
            String groupVisible = group.getGroup_visible();
            if (null != groupVisible) {
                values.put(Groups.GROUP_VISIBLE, groupVisible);
            }
            String shouldSync = group.getShould_sync();
            if (null != shouldSync) {
                values.put(Groups.SHOULD_SYNC, shouldSync);
            }
        }
        return values;
    }
}
