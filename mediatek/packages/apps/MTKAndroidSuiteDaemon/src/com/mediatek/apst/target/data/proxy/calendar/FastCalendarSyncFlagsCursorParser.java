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

package com.mediatek.apst.target.data.proxy.calendar;

import android.database.Cursor;

import com.mediatek.apst.target.data.provider.calendar.CalendarEventContent;
import com.mediatek.apst.target.data.proxy.FastCursorParser;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

public class FastCalendarSyncFlagsCursorParser extends FastCursorParser {

    /**
     * @param cursor
     *            The cursor of the the fast calendar synchronization.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the flags of the fast calendar
     *            synchronization.
     */
    public FastCalendarSyncFlagsCursorParser(Cursor cursor,
            IRawBlockConsumer consumer, ByteBuffer buffer) {
        super(cursor, consumer, buffer);
    }

    /**
     * @param cursor
     *            The cursor of the the fast calendar synchronization.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     */
    public FastCalendarSyncFlagsCursorParser(Cursor cursor,
            IRawBlockConsumer consumer) {
        super(cursor, consumer);
    }

    /*
     * (non-Javadoc)
     * 
     * @see
     * com.mediatek.apst.target.data.proxy.FastCursorParser#onParseCursorToRaw
     * (android.database.Cursor, java.nio.ByteBuffer)
     */
    @Override
    public int onParseCursorToRaw(Cursor c, ByteBuffer buffer) {
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
            colId = c.getColumnIndex(CalendarEventContent.COLUMN_ID);
            if (-1 != colId) {
                buffer.putLong(c.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // modifyTime
            colId = c.getColumnIndex(CalendarEventContent.MODIFY_TIME);
            if (-1 != colId) {
                buffer.putLong(c.getLong(colId));
            } else {
                buffer.putLong(-1);
            }
            // calendarId
            colId = c.getColumnIndex(CalendarEventContent.CALENDAR_ID);
            if (-1 != colId) {
                buffer.putLong(c.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // title
            colId = c.getColumnIndex(CalendarEventContent.TITLE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // timeZone
            colId = c.getColumnIndex(CalendarEventContent.EVENT_TIMEZONE);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, c.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // timeFrom
            colId = c.getColumnIndex(CalendarEventContent.DTSTART);
            if (-1 != colId) {
                buffer.putLong(c.getLong(colId));
            } else {
                buffer.putLong(-1);
            }
        } catch (IllegalArgumentException e) {
            Debugger.logE(new Object[] { c, buffer }, null, e);
            buffer.reset();
            return IRawBufferWritable.RESULT_FAIL;
        } catch (BufferOverflowException e) {
            /*
             * Debugger.logW(new Object[]{c, buffer},
             * "Not enough space left in buffer. ", e);
             */
            buffer.reset();
            return IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER;
        }
        return IRawBufferWritable.RESULT_SUCCESS;
    }
}
