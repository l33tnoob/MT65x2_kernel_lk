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

package com.mediatek.apst.target.data.provider.message;

import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

import com.mediatek.android.content.MeasuredContentValues;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;
import com.mediatek.apst.util.entity.message.Message;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

public class SmsContent {
    // ==============================================================
    // Constants
    // ==============================================================
    /** All SMS, support DELETE */
    public static final Uri CONTENT_URI = Uri.parse("content://sms");
    /** Inbox. type = 1 */
    public static final Uri CONTENT_URI_INBOX = Uri
            .parse("content://sms/inbox");
    /** Sent. type = 2 */
    public static final Uri CONTENT_URI_SENT = Uri.parse("content://sms/sent");
    /** Draft. type = 3 */
    public static final Uri CONTENT_URI_DRAFT = Uri
            .parse("content://sms/draft");
    /** Outbox. type = 4 */
    public static final Uri CONTENT_URI_OUTBOX = Uri
            .parse("content://sms/outbox");
    /** Failed. type = 5 */
    public static final Uri CONTENT_URI_FAILED = Uri
            .parse("content://sms/failed");
    /** Queued. type = 6. CANNOT QUERY */
    /*
     * public static final Uri CONTENT_URI_QUEUED = Uri.parse(
     * "content://sms/quequed");
     */
    /** Undelivered. type = (3?)4,5,6 */
    public static final Uri CONTENT_URI_UNDELIVERED = Uri
            .parse("content://sms/undelivered");
    /** SMS conversations */
    public static final Uri CONTENT_URI_CONVERSATIONS = Uri
            .parse("content://sms/conversations");

    /** Id. INTEGER(long). */
    public static final String COLUMN_ID = "_id";
    /**
     * The thread ID of the message
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String COLUMN_THREAD_ID = "thread_id";
    /**
     * The address of the other party
     * <P>
     * Type: TEXT (String)
     * </P>
     */
    public static final String COLUMN_ADDRESS = "address";
    /**
     * The person ID of the sender
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String COLUMN_PERSON = "person";
    /**
     * The date the message was sent(in timestamp)
     * <P>
     * Type: INTEGER (long)
     * </P>
     */
    public static final String COLUMN_DATE = "date";
    /**
     * The protocol identifier code
     * <P>
     * Type: INTEGER (int)
     * </P>
     */
    public static final String COLUMN_PROTOCOL = "protocol";
    /**
     * Has the message been read
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String COLUMN_READ = "read";
    /**
     * The TP-Status value for the message, or -1 if no status has been received
     * <P>
     * Type: INTEGER (int)
     * </P>
     */
    public static final String COLUMN_STATUS = "status";
    /**
     * The type of the message
     * <P>
     * Type: INTEGER (int)
     * </P>
     */
    public static final String COLUMN_TYPE = "type";
    /**
     * Whether the <code>TP-Reply-Path</code> bit was set on this message
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String COLUMN_REPLY_PATH_PRESENT = "reply_path_present";
    /**
     * The subject of the message, if present
     * <P>
     * Type: TEXT (String)
     * </P>
     */
    public static final String COLUMN_SUBJECT = "subject";
    /**
     * The body of the message
     * <P>
     * Type: TEXT (String)
     * </P>
     */
    public static final String COLUMN_BODY = "body";
    /**
     * The service center (SC) through which to send the message, if present
     * <P>
     * Type: TEXT (String)
     * </P>
     */
    public static final String COLUMN_SERVICE_CENTER = "service_center";
    /**
     * Has the message been locked?
     * <P>
     * Type: INTEGER (boolean)
     * </P>
     */
    public static final String COLUMN_LOCKED = "locked";

    public static final String COLUMN_DATE_SENT = "date_sent";

    /**
     * ID indicates from which SIM the SMS comes.
     * <P>
     * Type: INTEGER (int)
     * </P>
     */
    public static final String COLUMN_SIM_ID = "sim_id";

    public static final int STATUS_NONE = -1;
    public static final int STATUS_COMPLETE = 0;
    public static final int STATUS_PENDING = 64;
    public static final int STATUS_FAILED = 128;

    public static final int SIM_ID_MIM = 1;
    public static final int SIM_ID_MAX = 100;

    /**
     * @param cursor
     *            The cursor to Sms.
     * @return A Sms or null.
     */
    public static Sms cursorToSms(Cursor cursor) {
        if (null == cursor || cursor.getPosition() == -1
                || cursor.getPosition() == cursor.getCount()) {
            return null;
        }

        // Create a new SMS object
        Sms sms = new Sms();

        try {
            int colId;
            // id
            colId = cursor.getColumnIndex(COLUMN_ID);
            Debugger.logW("----------------COLUMN_ID colId = " + colId);
            if (-1 != colId) {
                sms.setId(cursor.getLong(colId));
                Debugger.logW("----------------sms id is "
                        + cursor.getLong(colId));
            }
            // thread id
            colId = cursor.getColumnIndex(COLUMN_THREAD_ID);
            if (-1 != colId) {
                sms.setThreadId(cursor.getLong(colId));
            }
            // address
            colId = cursor.getColumnIndex(COLUMN_ADDRESS);
            if (-1 != colId) {
                String address = cursor.getString(colId);
                sms.setTarget(new TargetAddress(address));
            }
            // date
            colId = cursor.getColumnIndex(COLUMN_DATE);
            if (-1 != colId) {
                sms.setDate(cursor.getLong(colId));
            }
            // box type
            colId = cursor.getColumnIndex(COLUMN_TYPE);
            if (-1 != colId) {
                sms.setBox(cursor.getInt(colId));
            }
            // read
            colId = cursor.getColumnIndex(COLUMN_READ);
            if (-1 != colId) {
                sms.setRead(cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // subject
            colId = cursor.getColumnIndex(COLUMN_SUBJECT);
            if (-1 != colId) {
                sms.setSubject(cursor.getString(colId));
            }
            // locked
            colId = cursor.getColumnIndex(COLUMN_LOCKED);
            if (-1 != colId) {
                sms
                        .setLocked(cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            }
            // body
            colId = cursor.getColumnIndex(COLUMN_BODY);
            if (-1 != colId) {
                sms.setBody(cursor.getString(colId));
            }
            // service center
            colId = cursor.getColumnIndex(COLUMN_SERVICE_CENTER);
            if (-1 != colId) {
                sms.setServiceCenter(cursor.getString(colId));
            }

            // date_sent added by Yu for ics
            colId = cursor.getColumnIndex(COLUMN_DATE_SENT);
            if (-1 != colId) {
                sms.setDate_sent(cursor.getInt(colId));
            }

            // For MTK DUAL-SIM feature.
            // sim id
            colId = cursor.getColumnIndex(COLUMN_SIM_ID);
            Debugger.logW("----------------colId = " + colId);
            if (-1 != colId) {
                int simId = cursor.getInt(colId);
                Debugger.logW("*************The simId = " + simId);
                sms.setSimId(simId);
                // sim name. Added by Shaoying Han
                SimDetailInfo info = Global.getSimInfoById(simId);
                sms.setSimName(info.getDisplayName());
                // sim number
                sms.setSimNumber(info.getNumber());
                // sim ICCId
                sms.setSimICCId(info.getICCId());
                Debugger.logW("----------------The sim id form db is "
                        + cursor.getInt(colId));
            }
        } catch (IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor }, null, e);
        }

        return sms;
    }

    /**
     * @param cursor
     *            The cursor about Mms.
     * @param buffer
     *            The buffer to store info about Sms.
     * @return The result of this operation.
     */
    public static int cursorToRaw(Cursor cursor, ByteBuffer buffer) {
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
            colId = cursor.getColumnIndex(COLUMN_ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // thread id
            colId = cursor.getColumnIndex(COLUMN_THREAD_ID);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(DatabaseRecordEntity.ID_NULL);
            }
            // address
            colId = cursor.getColumnIndex(COLUMN_ADDRESS);
            if (-1 != colId) {
                new TargetAddress(cursor.getString(colId)).writeRaw(buffer);
            } else {
                new TargetAddress(null).writeRaw(buffer);
            }
            // date
            colId = cursor.getColumnIndex(COLUMN_DATE);
            if (-1 != colId) {
                buffer.putLong(cursor.getLong(colId));
            } else {
                buffer.putLong(0);
            }
            // box type
            colId = cursor.getColumnIndex(COLUMN_TYPE);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
            } else {
                buffer.putInt(Sms.BOX_NONE);
            }
            // read
            colId = cursor.getColumnIndex(COLUMN_READ);
            if (-1 != colId) {
                RawTransUtil.putBoolean(buffer,
                        cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            } else {
                RawTransUtil.putBoolean(buffer, false);
            }
            // subject
            colId = cursor.getColumnIndex(COLUMN_SUBJECT);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // locked
            colId = cursor.getColumnIndex(COLUMN_LOCKED);
            if (-1 != colId) {
                RawTransUtil.putBoolean(buffer,
                        cursor.getInt(colId) == DatabaseRecordEntity.TRUE);
            } else {
                RawTransUtil.putBoolean(buffer, false);
            }
            // body
            colId = cursor.getColumnIndex(COLUMN_BODY);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // service center
            colId = cursor.getColumnIndex(COLUMN_SERVICE_CENTER);
            if (-1 != colId) {
                RawTransUtil.putString(buffer, cursor.getString(colId));
            } else {
                RawTransUtil.putString(buffer, null);
            }
            // date_sent added by Yu for ics
            colId = cursor.getColumnIndex(COLUMN_DATE_SENT);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
            } else {
                buffer.putInt(-1);
            }
            // For MTK DUAL-SIM feature.
            // sim id
            colId = cursor.getColumnIndex(COLUMN_SIM_ID);
            Debugger.logW("[cursorToRaw]----------------The COLUMN_SIM_ID is  "
                    + colId);
            if (-1 != colId) {
                buffer.putInt(cursor.getInt(colId));
                // sim name. Added by Shaoying Han
                SimDetailInfo info = Global
                        .getSimInfoById(cursor.getInt(colId));
                RawTransUtil.putString(buffer, info.getDisplayName());
                // sim number
                RawTransUtil.putString(buffer, info.getNumber());
                // sim ICCId
                RawTransUtil.putString(buffer, info.getICCId());
                Debugger
                        .logW("[cursorToRaw]----------------The sim id form db is "
                                + cursor.getInt(colId));
            } else {
                buffer.putInt(Message.SIM_ID);
                // sim name. Added by Shaoying Han
                SimDetailInfo info = Global.getSimInfoById(Message.SIM_ID);
                RawTransUtil.putString(buffer, info.getDisplayName());
                // sim number
                RawTransUtil.putString(buffer, info.getNumber());
                // sim ICCId
                RawTransUtil.putString(buffer, info.getICCId());
            }
        } catch (IllegalArgumentException e) {
            Debugger.logE(new Object[] { cursor, buffer }, null, e);
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

    /**
     * @param sms
     *            The data to create ContentValues.
     * @param setId
     *            Whether to set id.
     * @param setThreadId
     *            Whether to set thread id.
     * @return A ContentValues or null.
     */
    public static ContentValues createContentValues(Sms sms, boolean setId,
            boolean setThreadId) {
        if (null == sms) {
            return null;
        }

        int fieldsCount = 8;
        if (setId) {
            ++fieldsCount;
        }
        if (setThreadId) {
            ++fieldsCount;
        }
        if (null != sms.getTarget()) {
            ++fieldsCount;
        }
        ContentValues values = new ContentValues(fieldsCount);
        values.put(COLUMN_BODY, sms.getBody());
        values.put(COLUMN_SERVICE_CENTER, sms.getServiceCenter());
        values.put(COLUMN_SUBJECT, sms.getSubject());
        values.put(COLUMN_TYPE, sms.getBox());
        values.put(COLUMN_DATE, sms.getDate());
        values.put(COLUMN_READ, sms.isRead() ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);
        values.put(COLUMN_LOCKED, sms.isLocked() ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);
        // For MTK DUAL-SIM feature.
        values.put(COLUMN_SIM_ID, sms.getSimId());
        if (setId) {
            values.put(COLUMN_ID, sms.getId());
        }
        if (setThreadId) {
            values.put(COLUMN_THREAD_ID, sms.getThreadId());
        }
        if (null != sms.getTarget()) {
            values.put(COLUMN_ADDRESS, sms.getTarget().getAddress());
        }

        return values;
    }

    /**
     * @param sms
     *            The data to create MeasuredContentValues.
     * @param setId
     *            Whether to set id.
     * @param setThreadId
     *            Whether to set thread id.
     * @return A MeasuredContentValues or null.
     */
    public static MeasuredContentValues createMeasuredContentValues(Sms sms,
            boolean setId, boolean setThreadId) {
        if (null == sms) {
            return null;
        }

        MeasuredContentValues values;
        int fieldsCount = 8;
        if (setId) {
            ++fieldsCount;
        }
        if (setThreadId) {
            ++fieldsCount;
        }
        if (null != sms.getTarget()) {
            ++fieldsCount;
        }
        values = new MeasuredContentValues(fieldsCount);
        values.put(COLUMN_BODY, sms.getBody());
        values.put(COLUMN_SERVICE_CENTER, sms.getServiceCenter());
        values.put(COLUMN_SUBJECT, sms.getSubject());
        values.put(COLUMN_TYPE, sms.getBox());
        values.put(COLUMN_DATE, sms.getDate());
        values.put(COLUMN_READ, sms.isRead() ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);
        values.put(COLUMN_LOCKED, sms.isLocked() ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);
        // For MTK DUAL-SIM feature.
        values.put(COLUMN_SIM_ID, sms.getSimId());
        if (setId) {
            values.put(COLUMN_ID, sms.getId());
        }
        if (setThreadId) {
            values.put(COLUMN_THREAD_ID, sms.getThreadId());
        }
        if (null != sms.getTarget() && sms.getBox() != Sms.BOX_DRAFT) {
            values.put(COLUMN_ADDRESS, sms.getTarget().getAddress());
        }

        return values;
    }
}
