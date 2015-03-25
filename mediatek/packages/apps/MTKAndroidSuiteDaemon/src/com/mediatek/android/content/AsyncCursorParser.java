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

package com.mediatek.android.content;

import android.database.Cursor;

import com.mediatek.apst.target.util.Debugger;

public abstract class AsyncCursorParser {
    private Cursor mCursor;
    private int mCount;
    private int mPosition;

    /**
     * @param cursor
     *            the cursor to query.
     */
    public AsyncCursorParser(final Cursor cursor) {
        this.mCursor = cursor;
        this.mPosition = 0;
        this.mCount = (null == cursor) ? 0 : cursor.getCount();
    }

    /**
     * @return a cursor.
     */
    public Cursor getCursor() {
        return mCursor;
    }

    /**
     * @return the number of rows in the cursor.
     */
    public int getCount() {
        return mCount;
    }

    /**
     * @return the current cursor position.
     */
    public int getPosition() {
        return mPosition;
    }

    /**
     * @param cursor
     *            the cursor used to set.
     */
    protected void setCursor(final Cursor cursor) {
        this.mCursor = cursor;
    }

    /**
     * @return the current position in the cursor.
     */
    public int getCursorPosition() {
        int result;
        if (null == mCursor) {
            result = -1;
        } else {
            result = mCursor.getPosition();
        }
        return result;
    }

    /**
     * Move the cursor to the next row.
     * 
     * @return whether the move succeeded.
     */
    public boolean moveToNext() {
        boolean result;
        if (null == mCursor) {
            result = false;
        } else {
            result = mCursor.moveToNext();
        }
        return result;
    }

    /**
     * Move the cursor to the previous row.
     * 
     * @return whether the move succeeded.
     */
    public boolean moveToPrevious() {
        boolean result;
        if (null == mCursor) {
            result = false;
        } else {
            result = mCursor.moveToPrevious();
        }
        return result;
    }

    /**
     * @param cursor
     *            the cursor to reset.
     */
    public void resetCursor(final Cursor cursor) {
        if (null != mCursor && !mCursor.isClosed()) {
            mCursor.close();
            Debugger.logD(new Object[] { cursor }, "Cursor Closed!");
        }
        mCursor = cursor;
        mPosition = 0;
        mCount = (null == cursor) ? 0 : cursor.getCount();
    }

    /**
     * Call it to do the parse work.
     */
    public void parse() {
        Debugger.logI("Parse begin...");
        if (null == mCursor) {
            Debugger.logW("Curosr is null.");
            return;
        }

        onParseStart();

        try {
            while (mCursor.moveToNext()) {
                ++mPosition;
                onNewRow(mCursor);
                if (isBlockReady()) {
                    onBlockReady();
                }
            }
        } catch (final IllegalStateException e) {
            Debugger.logE(new Object[] {},
                    ">>>>>>>>>>Catched IllegalStateException!");
            onBlockReadyForEx();
        } finally {
            onParseOver();
            Debugger.logI("Parse finished.");
        }
    }

    /**
     * Override it.
     */
    protected abstract void onParseStart();

    /**
     * Override it.
     * 
     * @param cursor
     *            Cursor to parse
     */
    protected abstract void onNewRow(Cursor cursor);

    /**
     * Override it.
     * 
     * @return Is block ready.
     */
    public abstract boolean isBlockReady();

    /**
     * Override it.
     */
    protected abstract void onBlockReady();

    /**
     * Override it.
     *  Add by Yu, catch exception then stop parse.
     */
    protected abstract void onBlockReadyForEx();

    /**
     * Override it.
     */
    protected abstract void onParseOver();
}
