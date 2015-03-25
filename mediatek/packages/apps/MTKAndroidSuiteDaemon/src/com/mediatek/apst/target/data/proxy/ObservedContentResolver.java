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

package com.mediatek.apst.target.data.proxy;

import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.os.RemoteException;

import com.mediatek.apst.target.util.Debugger;

import java.util.ArrayList;

public class ObservedContentResolver {

    private ContentResolver mCR;
    private ISelfChangeObserver mSelfChangeOb;

    /**
     * @param cr
     *            the ContentResolver.
     */
    public ObservedContentResolver(final ContentResolver cr) {
        mCR = cr;
    }

    /**
     * @return a ISelfChangeObserver or null.
     */
    public ISelfChangeObserver getSelfChangeObserver() {
        return mSelfChangeOb;
    }

    /**
     * @param ob
     *            the ISelfChangeObserver to register.
     */
    public void registerSelfChangeObserver(final ISelfChangeObserver ob) {
        mSelfChangeOb = ob;
    }

    /**
     * unregister SelfChangeObserver.
     */
    public void unregisterSelfChangeObserver() {
        mSelfChangeOb = null;
    }

    /**
     * @param cr
     *            the ContentResolver to set.
     */
    public void setInnerContentResolver(final ContentResolver cr) {
        mCR = cr;
    }

    /**
     * start the ISelfChangeObserver.
     */
    private void selfChangeStart() {
        if (mSelfChangeOb != null) {
            mSelfChangeOb.onSelfChangeStart();
        } else {
            Debugger.logW(new Object[] {}, "mSelfChangeOb is null");
        }
    }

    /**
     * stop the ISelfChangeObserver.
     */
    private void selfChangeDone() {
        if (mSelfChangeOb != null) {
            mSelfChangeOb.onSelfChangeDone();
        }
    }

    /**
     * @param uri
     *            The URI, using the content:// scheme, for the content to
     *            retrieve.
     * @param projection
     *            A list of which columns to return. Passing null will return
     *            all columns, which is inefficient.
     * @param selection
     *            A filter declaring which rows to return, formatted as an SQL
     *            WHERE clause (excluding the WHERE itself). Passing null will
     *            return all rows for the given URI.
     * @param selectionArgs
     *            You may include ?s in selection, which will be replaced by the
     *            values from selectionArgs, in the order that they appear in
     *            the selection. The values will be bound as Strings.
     * @param sortOrder
     *            How to order the rows, formatted as an SQL ORDER BY clause
     *            (excluding the ORDER BY itself). Passing null will use the
     *            default sort order, which may be unordered
     * @return A Cursor object, which is positioned before the first entry, or
     *         null.
     */
    public Cursor query(final Uri uri, final String[] projection,
            final String selection, final String[] selectionArgs,
            final String sortOrder) {
        Cursor rt;
        rt = mCR.query(uri, projection, selection, selectionArgs, sortOrder);
        return rt;
    }

    /**
     * @param uri
     *            The URL of the table to insert into.
     * @param values
     *            The initial values for the newly inserted row.
     * @return The URL of the newly created row.
     */
    public Uri insert(final Uri uri, final ContentValues values) {
        Uri rt = null;
        if (null != mSelfChangeOb) {
            synchronized (mSelfChangeOb) {
                selfChangeStart();
                try {
                    rt = mCR.insert(uri, values);
                } catch (final SQLException e) {
                    Debugger.logE(new Object[] { uri, values }, null, e);
                }
                try {
                    mSelfChangeOb.wait();
                } catch (final InterruptedException e) {
                    e.printStackTrace();
                }
                selfChangeDone();
            }
        } else {
            selfChangeStart();
            try {
                rt = mCR.insert(uri, values);
            } catch (final SQLException e) {
                Debugger.logE(new Object[] { uri, values }, null, e);
            }
            selfChangeDone();
        }
        return rt;
    }

    /**
     * @param uri
     *            The URL of the table to insert into.
     * @param values
     *            The initial values for newly inserted rows.
     * @return the number of newly created rows.
     */
    public int bulkInsert(final Uri uri, final ContentValues[] values) {
        int rt = 0;
        selfChangeStart();
        try {
            rt = mCR.bulkInsert(uri, values);
        } catch (final SQLException e) {
            Debugger.logE(new Object[] { uri, values }, null, e);
        }
        selfChangeDone();
        return rt;
    }

    /**
     * @param uri
     *            The URL of table to modify.
     * @param values
     *            The new field values.
     * @param where
     *            A filter to apply to rows before updating.
     * @param selectionArgs
     *            You may include ?s in selection, which will be replaced by the
     *            values from selectionArgs, in order that they appear in the
     *            selection. The values will be bound as Strings.
     * @return the numbers the rows updated.
     */
    public int update(final Uri uri, final ContentValues values,
            final String where, final String[] selectionArgs) {
        int rt = 0;
        selfChangeStart();
        try {
            rt = mCR.update(uri, values, where, selectionArgs);
        } catch (final SQLException e) {
            Debugger.logE(new Object[] { uri, values, where, selectionArgs },
                    null, e);
        }
        selfChangeDone();
        return rt;
    }

    /**
     * @param uri
     *            The URL of rows to delete.
     * @param where
     *            A filter to apply to rows before deleting.
     * @param selectionArgs
     *            You may include ?s in selection, which will be replaced by the
     *            values from selectionArgs, in order that they appear in the
     *            selection. The values will be bound as Strings.
     * @return the number of rows deleted.
     */
    public int delete(final Uri uri, final String where,
            final String[] selectionArgs) {
        int rt = 0;
        selfChangeStart();
        try {
            rt = mCR.delete(uri, where, selectionArgs);
        } catch (final SQLException e) {
            Debugger.logE(new Object[] { uri, where, selectionArgs }, null, e);
        }
        selfChangeDone();
        return rt;
    }

    /**
     * @param authority
     *            The authority of the ContentProvider to which this batch
     *            should be applied.
     * @param operations
     *            New content provider operation to append to this batch.
     * @return The results of the applications.
     * @throws RemoteException
     *             thrown if a RemoteException is encountered while attempting
     *             to communicate with a remote provider.
     * @throws OperationApplicationException
     *             thrown if an application fails.
     */
    public ContentProviderResult[] applyBatch(final String authority,
            final ArrayList<ContentProviderOperation> operations)
            throws RemoteException, OperationApplicationException {
        ContentProviderResult[] rt = null;
        selfChangeStart();
        try {
            rt = mCR.applyBatch(authority, operations);
        } catch (final SQLException e) {
            Debugger.logE(new Object[] { authority, operations }, null, e);
        }
        selfChangeDone();
        return rt;
    }
}
