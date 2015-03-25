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

package com.mediatek.apst.target.data.proxy.message;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.RawContactsEntity;

import com.mediatek.android.content.DefaultBulkInsertHelper;
import com.mediatek.apst.target.data.provider.message.ConversationsContent;
import com.mediatek.apst.target.data.provider.message.MmsContent;
import com.mediatek.apst.target.data.provider.message.SmsContent;
import com.mediatek.apst.target.data.proxy.ContextBasedProxy;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.service.SmsSender;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.message.Mms;
import com.mediatek.apst.util.entity.message.MmsPart;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Set;

/**
 * Class Name: MessageProxy
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.message
 * <p>
 * Created on: 2010-8-12
 * <p>
 * <p>
 * Description:
 * <p>
 * Facade of the sub system of message related operations.
 * <p>
 * 
 * @author mtk80734 Siyang.Miao
 * @version V1.0
 */
public final class MessageProxy extends ContextBasedProxy {
    private static final int COLUMN_ID = 0;
    private static final int COLUMN_DATE = 1;
    private static final int COLUMN_LOCK = 2;
    private static final int COLUMN_READ = 3;
    private static final int COLUMN_SERVICECENTER = 4;
    private static final int COLUMN_THREADID = 5;
    private static final int COLUMN_SUBJECT = 6;
    private static final int COLUMN_SIMID = 6;

    /** Singleton instance. */
    private static MessageProxy sInstance = null;

    /**
     * @param context
     *            The context for message proxy.
     */
    private MessageProxy(final Context context) {
        super(context);
        setProxyName("MessageProxy");
    }

    /**
     * @param context
     *            The current context.
     * @return An instance of the MessageProxy.
     */
    public static synchronized MessageProxy getInstance(final Context context) {
        if (null == sInstance) {
            sInstance = new MessageProxy(context);
        } else {
            sInstance.setContext(context);
        }
        return sInstance;
    }

    /**
     * Asynchronously query all SMS messages. Faster due to transmission in raw
     * bytes.
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks
     * @param buffer
     *            buffer to write raw bytes.
     */
    public void asyncGetAllSms(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;
        try {
            c = getContentResolver().query(
                    SmsContent.CONTENT_URI,
                    new String[] { SmsContent.COLUMN_ID,
                            SmsContent.COLUMN_THREAD_ID,
                            SmsContent.COLUMN_ADDRESS, SmsContent.COLUMN_DATE,
                            SmsContent.COLUMN_TYPE, SmsContent.COLUMN_READ,
                            SmsContent.COLUMN_SUBJECT,
                            SmsContent.COLUMN_LOCKED, SmsContent.COLUMN_BODY,
                            SmsContent.COLUMN_SERVICE_CENTER,
                            SmsContent.COLUMN_SIM_ID },
                    SmsContent.COLUMN_THREAD_ID + ">0", null,
                    SmsContent.COLUMN_DATE + " DESC");

            final FastSmsCursorParser paser = new FastSmsCursorParser(c,
                    consumer, buffer, false, this);
            paser.parse();
        } finally {
            if (null != c && !c.isClosed()) {
                // Release resources
                c.close();
                c = null;
            }
        }
    }

    /**
     * get all MMS basic information from pdu table
     * 
     * @param consumer
     *            Set a consumer to handle asynchronous blocks
     * @param buffer
     *            buffer to write raw bytes.
     */
    public void asyncGetAllMms(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;
        try {
            c = getContentResolver()
                    .query(
                            MmsContent.CONTENT_URI,
                            new String[] { MmsContent.COLUMN_ID,
                                    MmsContent.COLUMN_THREAD_ID,
                                    MmsContent.COLUMN_DATE,
                                    MmsContent.COLUMN_M_TYPE,
                                    MmsContent.COLUMN_READ,
                                    MmsContent.COLUMN_SUBJECT,
                                    MmsContent.COLUMN_LOCKED,
                                    MmsContent.COLUMN_MSG_BOX,
                                    MmsContent.COLUMN_SIM_ID },
                            MmsContent.COLUMN_THREAD_ID + ">0", null,
                            MmsContent.COLUMN_DATE);

            final FastMmsCursorParser paser = new FastMmsCursorParser(c,
                    consumer, buffer, false, this);
            paser.parse();
        } finally {
            if (null != c && !c.isClosed()) {
                // Release resources
                c.close();
                c = null;
            }
        }
    }

    /**
     * get one MMS resource
     * 
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the mms.
     * @param id
     *            The id of the mms.
     */
    public void getOneMmsResource(final IRawBlockConsumer consumer,
            final ByteBuffer buffer, final long id) {
        Cursor c = null;
        try {
            c = getContentResolver().query(
                    MmsContent.CONTENT_URI_PART,
                    new String[] { MmsContent.COLUMN_PART_ID,
                            MmsContent.COLUMN_PART_MID,
                            MmsContent.COLUMN_PART_SEQ,
                            MmsContent.COLUMN_PART_CONTENTTYPE,
                            MmsContent.COLUMN_PART_NAME,
                            MmsContent.COLUMN_PART_CHARSET,
                            MmsContent.COLUMN_PART_CID,
                            MmsContent.COLUMN_PART_CL,
                            MmsContent.COLUMN_PART_DATAPATH,
                            MmsContent.COLUMN_PART_TEXT },
                    new String(MmsContent.COLUMN_PART_MID + "=" + id), null,
                    MmsContent.COLUMN_PART_ID);
            final FastMmsResourceCursorParser paser = new FastMmsResourceCursorParser(
                    c, consumer, buffer, this, id);
            paser.parse();
        } finally {
            if (null != c && !c.isClosed()) {
                // Release resources
                c.close();
                c = null;
            }
        }
    }

    /**
     * get all MMS information(include resource) for backup
     * 
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the mms data.
     * @param isBackup
     *            Whether to backup.
     * @param list
     *            The list of the message id.
     */
    public void getMmsData(final IRawBlockConsumer consumer,
            final ByteBuffer buffer, final boolean isBackup,
            final LinkedList<Long> list) {

        final StringBuffer selectionBuffer = new StringBuffer();
        String selection = null;
        if (!isBackup) {
            selectionBuffer.append(MmsContent.COLUMN_ID + " IN(");
            for (final long id : list) {
                selectionBuffer.append(id + ",");
            }
            selectionBuffer.deleteCharAt(selectionBuffer.length() - 1);
            selectionBuffer.append(")");
            selection = selectionBuffer.toString();
            Debugger.logI(new Object[] { list }, "Selection is :" + selection);
        } else {
            // support draft box backup/restore 2012-5-15 mtk54043
            // selection = MmsContent.COLUMN_MSG_BOX + "<>" + Message.BOX_DRAFT;
            Debugger.logI("isBack" + isBackup);
        }

        Cursor c = null;
        try {
            c = getContentResolver().query(MmsContent.CONTENT_URI,
            /*
             * new String[] { MmsContent.COLUMN_ID, MmsContent.COLUMN_THREAD_ID,
             * MmsContent.COLUMN_DATE, MmsContent.COLUMN_M_TYPE,
             * MmsContent.COLUMN_READ, MmsContent.COLUMN_SUBJECT,
             * MmsContent.COLUMN_LOCKED, MmsContent.COLUMN_MSG_BOX,
             * MmsContent.COLUMN_SIM_ID }
             */
            null, selection, null, MmsContent.COLUMN_DATE);
            final FastMmsBackupCursorParser paser = new FastMmsBackupCursorParser(
                    c, consumer, buffer, this);
            paser.parse();
        } finally {
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
     *            The buffer to save the phone list.
     */
    public void asyncGetPhoneList(final IRawBlockConsumer consumer,
            final ByteBuffer buffer) {
        Cursor c = null;
        try {
            // First query raw contacts content to build a map of id to name
            c = getContentResolver().query(RawContacts.CONTENT_URI,
                    new String[] { RawContacts._ID, Contacts.DISPLAY_NAME },
                    RawContacts.DELETED + "<>" + DatabaseRecordEntity.TRUE,
                    null, null);

            HashMap<Long, String> mapIdToName;
            if (null != c) {
                mapIdToName = new HashMap<Long, String>(c.getCount());
                while (c.moveToNext()) {
                    mapIdToName.put(c.getLong(0), c.getString(1));
                }
                c.close();
            } else {
                mapIdToName = new HashMap<Long, String>(0);
            }
            // Then query data content to get all phones
            c = getContentResolver().query(
                    RawContactsEntity.CONTENT_URI,
                    new String[] { RawContactsEntity._ID,
                            CommonDataKinds.Phone.NUMBER },
                    RawContactsEntity.MIMETYPE + "='"
                            + CommonDataKinds.Phone.CONTENT_ITEM_TYPE
                            + "' AND " + RawContactsEntity.DELETED + "<>"
                            + DatabaseRecordEntity.TRUE, null, null);
            final FastPhoneListCursorParser paser = new FastPhoneListCursorParser(
                    c, consumer, buffer, mapIdToName);
            paser.parse();
        } finally {
            if (null != c && !c.isClosed()) {
                // Release resources
                c.close();
                c = null;
            }
        }
    }

    /**
     * @param raw
     *            The byte array of the sms to import.
     * @param threadIdsToReturn
     *            The thread id of the sms.
     * @return The insert ids of the sms.
     */
    public long[] importSms(final byte[] raw,
            final ArrayList<Long> threadIdsToReturn) {
        threadIdsToReturn.clear();
        if (null == raw) {
            Debugger.logE(new Object[] { raw, threadIdsToReturn },
                    "List is null.");
            return new long[0];
        }

        ByteBuffer inBuffer = ByteBuffer.wrap(raw);
        // SMS count in the raw data
        int count;
        long[] insertedIds;
        long beginId;
        final HashMap<String, Long> mapAddress2ThreadId = new HashMap<String, Long>();
        try {
            // The first 4 bytes tell SMS count in the raw data.
            count = inBuffer.getInt();
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { raw, threadIdsToReturn },
                    "Can not get the SMS count in raw data ", e);
            return new long[0];
        }

        if (count < 0) {
            Debugger.logE(new Object[] { raw, threadIdsToReturn },
                    "Invalid SMS count " + count);
            return new long[0];
        }

        beginId = getMaxSmsId() + 1;
        // Prepare content values
        final DefaultBulkInsertHelper inserter = new DefaultBulkInsertHelper() {

            @Override
            public boolean onExecute(final ContentValues[] values) {
                final int expectedCount = values.length;
                final int insertedCount = getObservedContentResolver()
                        .bulkInsert(SmsContent.CONTENT_URI, values);
                if (insertedCount != expectedCount) {
                    // ERROR
                    Debugger.logE(getProxyName(), "importSms", new Object[] {
                            raw, threadIdsToReturn },
                            "Bulk insert SMS failed, inserted " + insertedCount
                                    + ", expected " + expectedCount);
                    return false;
                } else {
                    return true;
                }
            }
        };
        Long threadId;
        String address;
        boolean shouldExit;
        for (int i = 0; i < count; i++) {
            // Read a SMS message from raw bytes
            final Sms newSms = new Sms();
            // newSms.readRaw(inBuffer); Changed by Shaoying Han
            newSms.readRawWithVersion(inBuffer, Config.VERSION_CODE);

            // Add content values for the SMS insertion
            newSms.setId(beginId + i);
            address = newSms.getTarget().getAddress();
            if (null != address) {
                final String[] addressList = address.split(",");
                if (addressList.length > 1) {
                    threadId = getOrCreateThreadId2(addressList);
                } else {
                    threadId = mapAddress2ThreadId.get(address);
                    if (null == threadId) {
                        threadId = getOrCreateThreadId(address);
                        mapAddress2ThreadId.put(address, threadId);
                    }
                }
                newSms.setThreadId(threadId);
                // Record the thread ID in return list
                threadIdsToReturn.add(threadId);
                Debugger.logI(new Object[] { raw }, "SMS getOrCreateThreadId: "
                        + threadId);
            } else {
                // No address?
                newSms.setThreadId(DatabaseRecordEntity.ID_NULL);
                threadIdsToReturn.add(DatabaseRecordEntity.ID_NULL);
                Debugger.logE(new Object[] { newSms }, "SMS havn't address!");
            }

            shouldExit = !inserter.append(SmsContent
                    .createMeasuredContentValues(newSms, true, true));
            if (shouldExit) {
                Debugger.logE(new Object[] { raw, threadIdsToReturn },
                        "Error in bulk inserting SMS, statusCode: "
                                + inserter.getStatusCode());
                inBuffer = null;
                mapAddress2ThreadId.clear();
                threadIdsToReturn.clear();
                return new long[0];
            }
        }
        // Bulk insert all ContentValues left
        shouldExit = !inserter.execute();
        if (shouldExit) {
            Debugger.logE(new Object[] { raw, threadIdsToReturn },
                    "Error in bulk inserting SMS, statusCode: "
                            + inserter.getStatusCode());
            inBuffer = null;
            mapAddress2ThreadId.clear();
            threadIdsToReturn.clear();
            return new long[0];
        }

        // Set inserted id values and return
        insertedIds = new long[count];
        for (int i = 0; i < count; i++) {
            insertedIds[i] = beginId + i;
        }

        inBuffer = null;
        mapAddress2ThreadId.clear();
        return insertedIds;
    }

    /**
     * Import MMS read from raw
     * 
     * @param raw
     *            The byte array of the import mms.
     * @param threadIdsToReturn
     *            The thread id of the mms.
     * @param isBackup
     *            Whether to backup.
     * @return The inserted ids of the mms.
     */
    public long[] importMms(final byte[] raw,
            final ArrayList<Long> threadIdsToReturn) {
        byte[] rawBytes = raw;
        threadIdsToReturn.clear();
        if (null == rawBytes) {
            Debugger.logE(new Object[] { rawBytes, threadIdsToReturn },
                    "List is null.");
            return null;
        }
        Debugger.logI(new Object[] { rawBytes }, "Receive rawBytes[]:"
                + rawBytes.length);
        ByteBuffer inBuffer = ByteBuffer.wrap(rawBytes);
        rawBytes = null;
        // MMS count in the raw data
        int count = 0;
        int countTotal = 0;
        long[] insertedIds;
        long beginId2;
        final HashMap<String, Long> mapAddress2ThreadId = new HashMap<String, Long>();
        beginId2 = getMaxMmsId() + 1;

        try {
            // The first 4 bytes tell MMS count in the raw data.
            count = inBuffer.getInt();
            Debugger.logI(new Object[] { rawBytes }, "MMS count is " + count);
            Debugger.logI(new Object[] { rawBytes }, "inBuffer position is "
                    + inBuffer.position());
        } catch (final BufferUnderflowException e) {
            Debugger.logE(new Object[] { rawBytes, threadIdsToReturn },
                    "Can not get the MMS count in raw data ", e);
            return null;
        }

        if (count < 0) {
            Debugger.logE(new Object[] { rawBytes, threadIdsToReturn },
                    "Invalid MMS count " + count);
            return null;
        }
        Long threadId;
        String address;
        for (int i = 0; i < count; i++) {
            // Read a MMS message from raw bytes
            Mms newMms = new Mms();
            try {
                newMms.readAllWithVersion(inBuffer, Config.VERSION_CODE);
                Debugger.logI(new Object[] { rawBytes },
                        "readAllWithVersion over");
            } catch (final NullPointerException e) {
                e.printStackTrace();
                return null;
            } catch (final BufferUnderflowException e) {
                e.printStackTrace();
                return null;
            }
            Debugger.logI(new Object[] { rawBytes }, "newMms id is "
                    + newMms.getId());
            address = newMms.getTarget().getAddress();

            if (null != address) {
                final String[] addressList = address.split(",");
                if (addressList.length > 1) {
                    threadId = getOrCreateThreadId2(addressList);
                } else {
                    threadId = mapAddress2ThreadId.get(address);
                    if (null == threadId) {
                        Debugger.logI(new Object[] {}, "MMS address: "
                                + address);
                        threadId = getOrCreateThreadId(address);
                        mapAddress2ThreadId.put(address, threadId);
                    }
                }
                newMms.setThreadId(threadId);
                // Record the thread ID in return list
                threadIdsToReturn.add(threadId);
                Debugger.logI(new Object[] { rawBytes },
                        "SMS getOrCreateThreadId: " + threadId);
            } else {
                // No address?
                Debugger.logE(new Object[] {}, "MMS no address");
                newMms.setThreadId(DatabaseRecordEntity.ID_NULL);
                threadIdsToReturn.add(DatabaseRecordEntity.ID_NULL);
            }
            if (!insertMms(newMms)) {
                newMms = null;
                count = i + 1;
                break;
            };
            newMms = null;
        }
        countTotal += count;
        // Set inserted id values and return
        insertedIds = new long[countTotal];
        for (int i = 0; i < countTotal; i++) {
            insertedIds[i] = beginId2 + i;
        }
        Debugger.logI(new Object[] {}, "Received MMS buffer size: "
                + inBuffer.capacity());
        inBuffer.clear();
        inBuffer = null;
        mapAddress2ThreadId.clear();
        return insertedIds;
    }

    /**
     * @return The max id of the sms.
     */
    public long getMaxSmsId() {
        long maxId = 0L;
        Cursor c;
        Debugger.logI("ANR Test1");
        c = getContentResolver().query(SmsContent.CONTENT_URI,
                new String[] { SmsContent.COLUMN_ID }, null, null,
                SmsContent.COLUMN_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        Debugger.logI("ANR Test2");
        return maxId;
    }

    /**
     * @return The max id of the mms.
     */
    public long getMaxMmsId() {
        long maxId = 0L;
        Cursor c;

        c = getContentResolver().query(MmsContent.CONTENT_URI,
                new String[] { MmsContent.COLUMN_ID }, null, null,
                MmsContent.COLUMN_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        return maxId;
    }

    /**
     * @return The max id of the mms part.
     */
    public long getMaxMmsPartId() {
        long maxId = 0L;
        Cursor c;

        c = getContentResolver().query(MmsContent.CONTENT_URI_PART,
                new String[] { MmsContent.COLUMN_PART_ID }, null, null,
                MmsContent.COLUMN_PART_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        return maxId;
    }
    
    /**
     * @return The max mms id.
     */
    public long getMaxMmsIdByInsert() {
        Debugger.logI("-->getMaxMmsIdByInsert");
        long maxId = 0L;
        // ContentValues valuesPdu = new ContentValues(5);
        // valuesPdu.put(MmsContent.COLUMN_SUBJECT, "");
        // valuesPdu.put(MmsContent.COLUMN_MSG_BOX, Mms.BOX_DRAFT);
        // valuesPdu.put(MmsContent.COLUMN_READ, DatabaseRecordEntity.TRUE);
        // valuesPdu.put(MmsContent.COLUMN_LOCKED, DatabaseRecordEntity.FALSE);
        // valuesPdu.put(MmsContent.COLUMN_SIM_ID, -1);
        final Uri uri = getObservedContentResolver().insert(MmsContent.CONTENT_URI, new ContentValues(1));
        Debugger.logI("getMaxMmsIdByInsert: insert test mms done!");
        try {
            // Get the max ID in sqlite_sequence table
            maxId = Long.parseLong(uri.getLastPathSegment());
            // Delete the temporary record
            deleteMms(maxId, false, 0l);
        } catch (final NumberFormatException e) {
            Debugger.logE(e);
        } catch (final NullPointerException e) {
            Debugger.logE(e);
        }
        Debugger.logW("getMaxMmsIdByInsert max id: " + maxId);
        return maxId;
    }

    /**
     * @param afterTimeOf
     *            After what time of the sms which you want to find.
     * @param address
     *            The address of the sms which you want to find.
     * @param smsBody
     *            The body of the sms which you want to find.
     * @param box
     *            The sms in which box you want to find.
     * @return The sms you want to find.
     */
    public Sms findSms(final long afterTimeOf, final String address,
            final String smsBody, final int box) {
        Sms result = null;
        Cursor c;
        String body = null;
        if (null != smsBody) {
            body = smsBody.trim();
            body = body.replaceAll("'", "''");
        }

        c = getContentResolver().query(
                SmsContent.CONTENT_URI,
                new String[] { SmsContent.COLUMN_ID, SmsContent.COLUMN_DATE,
                        SmsContent.COLUMN_LOCKED, SmsContent.COLUMN_READ,
                        SmsContent.COLUMN_SERVICE_CENTER,
                        SmsContent.COLUMN_THREAD_ID, SmsContent.COLUMN_SUBJECT,
                        // For MTK Dual-SIM feature.
                        SmsContent.COLUMN_SIM_ID },
                SmsContent.COLUMN_DATE + ">" + afterTimeOf + " AND "
                        + SmsContent.COLUMN_ADDRESS + "='" + address + "' AND "
                        + SmsContent.COLUMN_BODY + "='" + body + "' AND "
                        + SmsContent.COLUMN_TYPE + "=" + box, null,
                SmsContent.COLUMN_ID + " ASC");
        if (null != c) {
            if (c.moveToNext()) {
                result = new Sms();
                result.setBody(body);
                result.setBox(box);
                result.setTarget(new TargetAddress(address));
                // modify magic number .
                result.setId(c.getLong(COLUMN_ID));
                result.setDate(c.getLong(COLUMN_DATE));
                result
                        .setLocked(c.getInt(COLUMN_LOCK) == DatabaseRecordEntity.TRUE);
                result
                        .setRead(c.getInt(COLUMN_READ) == DatabaseRecordEntity.TRUE);
                result.setServiceCenter(c.getString(COLUMN_SERVICECENTER));
                result.setThreadId(c.getLong(COLUMN_THREADID));
                result.setSubject(c.getString(COLUMN_SUBJECT));
                int colId = c.getColumnIndex(SmsContent.COLUMN_SIM_ID);
                // For MTK Dual-SIM feature.
                final int simId = c.getInt(colId); //COLUMN_SIMID
                result.setSimId(simId);
                // sim name. Added by Shaoying Han
                final SimDetailInfo info = Global.getSimInfoById(simId);
                result.setSimName(info.getDisplayName());
                // sim number
                result.setSimNumber(info.getNumber());
                // sim ICCId
                result.setSimICCId(info.getICCId());
            }
            c.close();
        }
        return result;
    }

    /**
     * @param number
     *            The number of contact which you want to find.
     * @return The target address or null.
     */
    public TargetAddress lookupContact(final String number) {
        if (null == number) {
            return null;
        }

        final TargetAddress target = new TargetAddress(number);
        Cursor c;
        // Try to find the contact id
        c = getContentResolver().query(
                Data.CONTENT_URI,
                new String[] { Data.RAW_CONTACT_ID },
                CommonDataKinds.Phone.NUMBER + "='" + number + "'" + " AND "
                        + Data.MIMETYPE + "='"
                        + CommonDataKinds.Phone.CONTENT_ITEM_TYPE + "'", null,
                Data.RAW_CONTACT_ID);
        if (null != c) {
            if (c.moveToNext()) {
                // Number belongs to known contact
                final long rawContactId = c.getLong(0);
                target.setContactId(rawContactId);
                // Next get the display name
                c.close();
                c = getContentResolver().query(RawContacts.CONTENT_URI,
                        new String[] { Contacts.DISPLAY_NAME },
                        RawContacts._ID + "=" + rawContactId, null, null);
                if (null != c) {
                    if (c.moveToNext()) {
                        target.setName(c.getString(0));
                    } else {
                        // Cannot find a raw contact with the _id value of this
                        // phone
                        // number data's raw_contact_id. This is not supposed to
                        // happen.
                        Debugger
                                .logE(
                                        new Object[] { number },
                                        "Cannot find a raw contact with the _id value "
                                                + "of this phone number data's raw_contact_id.");
                    }
                    c.close();
                } else {
                    Debugger
                            .logE(
                                    new Object[] { number },
                                    "Cursor is null. Failed to find a raw contact "
                                            + "with the _id value of this phone number data's "
                                            + "raw_contact_id.");
                }
            }
        }
        return target;
    }

    /**
     * @param box
     *            Which box to clear the message(such as inbox, outbox).
     * @param keepLockedMessage
     *            Whether to lock the message.
     * @return The counts of the messages cleared.
     */
    public int clearMessageBox(final int box, final boolean keepLockedMessage) {
        int deleteCount;
        int deleteSmsCount = 0;
        int deleteMmsCount = 0;
        final StringBuffer selection = new StringBuffer();

        selection.append(SmsContent.COLUMN_TYPE + "=" + box);
        if (keepLockedMessage) {
            selection.append(" AND ");
            selection.append(SmsContent.COLUMN_LOCKED + "<>"
                    + DatabaseRecordEntity.TRUE);
        }
        deleteSmsCount = getObservedContentResolver().delete(
                SmsContent.CONTENT_URI, selection.toString(), null);

        // TODO Disable MMS currently

        selection.append(MmsContent.COLUMN_MSG_BOX + "=" + box);
        if (keepLockedMessage) {
            selection.append(" AND ");
            selection.append(MmsContent.COLUMN_LOCKED + "<>"
                    + DatabaseRecordEntity.TRUE);
        }
        deleteMmsCount = getObservedContentResolver().delete(
                MmsContent.CONTENT_URI, MmsContent.COLUMN_MSG_BOX + "=" + box,
                null);

        deleteCount = deleteSmsCount + deleteMmsCount;

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param keepLockedMessage
     *            Whether to lock the message.
     * @return The counts of the messages deleted.
     */
    public int deleteAllMessages(final boolean keepLockedMessage) {
        int deleteCount;
        int deleteSmsCount = 0;
        int deleteMmsCount = 0;
        String selection = null;
        String selection2 = null; // changed by Yu

        if (keepLockedMessage) {
            selection = SmsContent.COLUMN_LOCKED + "<>"
                    + DatabaseRecordEntity.TRUE;
            selection2 = MmsContent.COLUMN_LOCKED + "<>"
                    + DatabaseRecordEntity.TRUE;
        }
        deleteSmsCount = getObservedContentResolver().delete(
                SmsContent.CONTENT_URI, selection, null);

        // TODO Disable MMS currently
        // if (keepLockedMessage){
        // selection = MmsContent.COLUMN_LOCKED + "<>" +
        // DatabaseRecordEntity.TRUE;
        // }
        deleteMmsCount = getObservedContentResolver().delete(
                MmsContent.CONTENT_URI, selection2, null);

        deleteCount = deleteSmsCount + deleteMmsCount;

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param id
     *            The id of sms which to delete.
     * @param checkDate
     *            Whether to check data.
     * @param date
     *            The date of the sms.
     * @return The counts of the deleted sms.
     */
    public int deleteSms(final long id, final boolean checkDate, final long date) {
        int deleteCount = 0;
        String selection = null;
        if (checkDate) {
            selection = SmsContent.COLUMN_ID + "=" + id + " AND "
                    + SmsContent.COLUMN_DATE + "=" + date;
        } else {
            selection = SmsContent.COLUMN_ID + "=" + id;
        }

        deleteCount = getObservedContentResolver().delete(
                SmsContent.CONTENT_URI, selection, null);

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param id
     *            The id of the mms which to delete.
     * @param checkDate
     *            Whether to check date.
     * @param date
     *            The date of the mms.
     * @return The Counts of the deleted mms.
     */
    public int deleteMms(final long id, final boolean checkDate, final long date) {
        int deleteCount = 0;
        String selection = null;
        if (checkDate) {
            selection = MmsContent.COLUMN_ID + "=" + id + " AND "
                    + MmsContent.COLUMN_DATE + "=" + date;
        } else {
            selection = MmsContent.COLUMN_ID + "=" + id;
        }

        deleteCount = getObservedContentResolver().delete(
                MmsContent.CONTENT_URI, selection, null);

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param ids
     *            The ids which to delete.
     * @param checkDates
     *            Whether to check date.
     * @param dates
     *            The date of the sms.
     * @return The counts of the deleted mms.
     */
    public int deleteSms(final long[] ids, final boolean checkDates,
            final long[] dates) {
        if (null == ids) {
            Debugger.logW(new Object[] { ids, checkDates, dates },
                    "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, checkDates, dates },
                    "List is empty.");
            return 0;
        }
        if (checkDates && (ids.length != dates.length)) {
            Debugger.logE(new Object[] { ids, checkDates, dates },
                    "Size of ID list does not match size of date list.");
            return 0;
        }

        int deleteCount;
        // Set selection
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < ids.length; i++){ strBuf.append("(" +
         * SmsContent.COLUMN_ID + "=" + ids[i]); if (checkDates){
         * strBuf.append(" AND " + SmsContent.COLUMN_DATE + "=" + dates[i]); }
         * strBuf.append(") OR "); } if (strBuf.length() > 4){ // Drop the last
         * ' OR ' selection = strBuf.substring(0, strBuf.length() - 4); }
         */
        // Expression tree depth won't beyond 1000 in this way, but it's
        // not precise logically
        strBuf.append(SmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        /*
         * strBuf.append(" AND " + SmsContent.COLUMN_DATE + " IN("); for (int i
         * = 0; i < dates.length; i++){ strBuf.append(dates[i] + ","); } // Drop
         * the last ',' strBuf.deleteCharAt(strBuf.length() - 1);
         * strBuf.append(")");
         */
        selection = strBuf.toString();

        // Do the deletion
        deleteCount = getObservedContentResolver().delete(
                SmsContent.CONTENT_URI, selection, null);

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param ids
     *            The ids of the mms which to delete.
     * @param checkDates
     *            Whether to check date.
     * @param dates
     *            The date of the mms.
     * @return The counts of the deleted mms.
     */
    public int deleteMms(final long[] ids, final boolean checkDates,
            final long[] dates) {
        if (null == ids) {
            Debugger.logW(new Object[] { ids, checkDates, dates },
                    "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, checkDates, dates },
                    "List is empty.");
            return 0;
        }
        if (checkDates && (ids.length != dates.length)) {
            Debugger.logE(new Object[] { ids, checkDates, dates },
                    "Size of ID list does not match size of date list.");
            return 0;
        }

        int deleteCount;
        // Set selection
        String selection = null;
        final StringBuffer strBuf = new StringBuffer();
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < ids.length; i++){ strBuf.append("(" +
         * MmsContent.COLUMN_ID + "=" + ids[i]); if (checkDates){
         * strBuf.append(" AND " + MmsContent.COLUMN_DATE + "=" + dates[i]); }
         * strBuf.append(") OR "); } if (strBuf.length() > 4){ // Drop the last
         * ' OR ' selection = strBuf.substring(0, strBuf.length() - 4); }
         */
        // Expression tree depth won't beyond 1000 in this way, but it's
        // not precise logically
        strBuf.append(MmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        /*
         * strBuf.append(" AND " + MmsContent.COLUMN_DATE + " IN("); for (int i
         * = 0; i < dates.length; i++){ strBuf.append(dates[i] + ","); } // Drop
         * the last ',' strBuf.deleteCharAt(strBuf.length() - 1);
         * strBuf.append(")");
         */
        selection = strBuf.toString();

        // Do the deletion
        deleteCount = getObservedContentResolver().delete(
                MmsContent.CONTENT_URI, selection, null);

        updateThread(-1L);
        // Update Mms notifications
        updateMmsNotifications();

        return deleteCount;
    }

    /**
     * @param ids
     *            The ids of the sms which to move.
     * @param checkDates
     *            Whether to check date.
     * @param dates
     *            The date of the sms.
     * @param box
     *            Which box to move to.
     * @return The counts of the moved sms.
     */
    public int moveSmsToBox(final long ids[], final boolean checkDates,
            final long[] dates, final int box) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "List is empty.");
            return 0;
        }
        if (checkDates && (ids.length != dates.length)) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "Size of ID list does not match size of date list.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(SmsContent.COLUMN_TYPE, box);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < ids.length; i++){ strBuf.append("(" +
         * SmsContent.COLUMN_ID + "=" + ids[i]); if (checkDates){
         * strBuf.append(" AND " + SmsContent.COLUMN_DATE + "=" + dates[i]); }
         * strBuf.append(") OR "); } if (strBuf.length() > 4){ // Drop the last
         * ' OR ' selection = strBuf.substring(0, strBuf.length() - 4); }
         */
        // Expression tree depth won't beyond 1000 in this way, but it's
        // not precise logically
        strBuf.append(SmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        /*
         * strBuf.append(" AND " + SmsContent.COLUMN_DATE + " IN("); for (int i
         * = 0; i < dates.length; i++){ strBuf.append(dates[i] + ","); } // Drop
         * the last ',' strBuf.deleteCharAt(strBuf.length() - 1);
         * strBuf.append(")");
         */
        selection = strBuf.toString();

        // Do the update
        updateCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values, selection, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param ids
     *            The id of the mms which to move.
     * @param checkDates
     *            Whether to check date.
     * @param dates
     *            The date of the mms.
     * @param box
     *            Which box to move mms to.
     * @return The counts of the moved mms.
     */
    public int moveMmsToBox(final long ids[], final boolean checkDates,
            final long[] dates, final int box) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "List is empty.");
            return 0;
        }
        if (checkDates && (ids.length != dates.length)) {
            Debugger.logE(new Object[] { ids, checkDates, dates, box },
                    "Size of ID list does not match size of date list.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(MmsContent.COLUMN_MSG_BOX, box);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        // TODO
        /*
         * // Expression tree depth may beyond 1000 in this way, so it may fail
         * for (int i = 0; i < ids.length; i++){ strBuf.append("(" +
         * MmsContent.COLUMN_ID + "=" + ids[i]); if (checkDates){
         * strBuf.append(" AND " + MmsContent.COLUMN_DATE + "=" + dates[i]); }
         * strBuf.append(") OR "); } if (strBuf.length() > 4){ // Drop the last
         * ' OR ' selection = strBuf.substring(0, strBuf.length() - 4); }
         */
        // Expression tree depth won't beyond 1000 in this way, but it's
        // not precise logically
        strBuf.append(MmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        /*
         * strBuf.append(" AND " + MmsContent.COLUMN_DATE + " IN("); for (int i
         * = 0; i < dates.length; i++){ strBuf.append(dates[i] + ","); } // Drop
         * the last ',' strBuf.deleteCharAt(strBuf.length() - 1);
         * strBuf.append(")");
         */
        selection = strBuf.toString();

        // Do the update
        updateCount = getObservedContentResolver().update(
                MmsContent.CONTENT_URI, values, selection, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param id
     *            The id of the sms which to lock.
     * @param state
     *            The state to set.
     * @return The counts of the locked sms.
     */
    public int lockSms(final long id, final boolean state) {
        int updateCount = 0;

        final ContentValues values = new ContentValues(1);
        values.put(SmsContent.COLUMN_LOCKED, state ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);

        updateCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values,
                SmsContent.COLUMN_ID + "=" + id, null);

        return updateCount;
    }

    /**
     * @param id
     *            The id of the mms which to lock.
     * @param state
     *            The state to set.
     * @return The counts of the locked mms.
     */
    public int lockMms(final long id, final boolean state) {
        int updateCount = 0;

        final ContentValues values = new ContentValues(1);
        values.put(MmsContent.COLUMN_LOCKED, state ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);

        updateCount = getObservedContentResolver().update(
                MmsContent.CONTENT_URI, values,
                MmsContent.COLUMN_ID + "=" + id, null);

        return updateCount;
    }

    /**
     * @param ids
     *            The id of the sms which to lock.
     * @param state
     *            The state to set.
     * @return The counts of the locked sms.
     */
    public int lockSms(final long[] ids, final boolean state) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, state }, "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, state }, "List is empty.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(SmsContent.COLUMN_LOCKED, state);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append(SmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();
        // Do the update
        updateCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values, selection, null);

        return updateCount;
    }

    /**
     * @param ids
     *            The id of the mms which to lock.
     * @param state
     *            The state to set.
     * @return The counts of the locked mms.
     */
    public int lockMms(final long[] ids, final boolean state) {
        if (null == ids) {
            Debugger.logE(new Object[] { ids, state }, "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, state }, "List is empty.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(MmsContent.COLUMN_LOCKED, state);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append(MmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();
        // Do the update
        updateCount = getObservedContentResolver().update(
                MmsContent.CONTENT_URI, values, selection, null);

        return updateCount;
    }

    /**
     * @param id
     *            The id of the sms which to mark as read.
     * @param state
     *            The state the set, true is read , false is not read.
     * @return The count of marked sms.
     */
    public int markSmsAsRead(final long id, final boolean state) {
        int updateCount = 0;

        final ContentValues values = new ContentValues(1);
        values.put(SmsContent.COLUMN_READ, state ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);

        updateCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values,
                SmsContent.COLUMN_ID + "=" + id, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param id
     *            The id of the mms which to mark as read.
     * @param state
     *            The state the set, true is read , false is not read.
     * @return The count of marked mms.
     */
    public int markMmsAsRead(final long id, final boolean state) {
        int updateCount = 0;

        final ContentValues values = new ContentValues(1);
        values.put(MmsContent.COLUMN_READ, state ? DatabaseRecordEntity.TRUE
                : DatabaseRecordEntity.FALSE);

        updateCount = getObservedContentResolver().update(
                MmsContent.CONTENT_URI, values,
                MmsContent.COLUMN_ID + "=" + id, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param ids
     *            The id of the sms which to mark as read.
     * @param state
     *            The state the set, true is read , false is not read.
     * @return The count of marked sms.
     */
    public int markSmsAsRead(final long[] ids, final boolean state) {
        if (null == ids) {
            Debugger.logW(new Object[] { ids, state }, "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, state }, "List is empty.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(SmsContent.COLUMN_READ, state);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append(SmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();
        // Do the update
        updateCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values, selection, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param ids
     *            The id of the mms which to mark as read.
     * @param state
     *            The state the set, true is read , false is not read.
     * @return The count of marked mms.
     */
    public int markMmsAsRead(final long[] ids, final boolean state) {
        if (null == ids) {
            Debugger.logW(new Object[] { ids, state }, "List is null.");
            return 0;
        }
        if (0 >= ids.length) {
            Debugger.logE(new Object[] { ids, state }, "List is empty.");
            return 0;
        }

        int updateCount = 0;
        String selection = null;
        // Set update values
        final ContentValues values = new ContentValues(1);
        values.put(MmsContent.COLUMN_READ, state);
        // Set selection
        final StringBuffer strBuf = new StringBuffer();
        strBuf.append(MmsContent.COLUMN_ID + " IN(");
        for (int i = 0; i < ids.length; i++) {
            strBuf.append(ids[i] + ",");
        }
        // Drop the last ','
        strBuf.deleteCharAt(strBuf.length() - 1);
        strBuf.append(")");
        selection = strBuf.toString();
        // Do the update
        updateCount = getObservedContentResolver().update(
                MmsContent.CONTENT_URI, values, selection, null);

        // Update Mms notifications
        updateMmsNotifications();

        return updateCount;
    }

    /**
     * @param body
     *            The body of the sms.
     * @param recipients
     *            The recipients of the sms.
     * @return The the saved sms or null.
     */
    public Sms saveSmsDraft(final String body, final String[] recipients) {
        if (null == body) {
            Debugger.logW(new Object[] { body, recipients },
                    "Sms body should not be null.");
            return null;
        }

        final Sms draft = new Sms();
        ContentValues values;
        long insertedId = DatabaseRecordEntity.ID_NULL;
        // Set body
        draft.setBody(body);
        // Set thread id
        final Set<String> address = new HashSet<String>(recipients.length);
        for (int i = 0; i < recipients.length; i++) {
            if (null != recipients[i]) {
                address.add(recipients[i]);
            }
        }
        draft.setThreadId(getOrCreateThreadId(address));
        // Set date
        draft.setDate(System.currentTimeMillis());

        values = new ContentValues(4);
        values.put(SmsContent.COLUMN_THREAD_ID, draft.getThreadId());
        // FIXME It seems that drafts and sent items do not possess individual
        // addresses, for their addresses is specified by their threads
        // values.put(SmsContent.COLUMN_ADDRESS, sms.getTarget().getAddress());
        values.put(SmsContent.COLUMN_DATE, draft.getDate());
        values.put(SmsContent.COLUMN_TYPE, Sms.BOX_DRAFT);
        values.put(SmsContent.COLUMN_BODY, draft.getBody());

        try {
            final Uri uri = getObservedContentResolver().insert(
                    SmsContent.CONTENT_URI, values);
            if (null != uri) {
                insertedId = Long.parseLong(uri.getLastPathSegment());
            }
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { body, recipients }, null, e);
        } catch (final IllegalArgumentException e) {
            Debugger.logE(new Object[] { body, recipients }, null, e);
        }

        // Set the new id after insertion
        draft.setId(insertedId);

        // Delete all other drafts in this thread
        getObservedContentResolver().delete(
                SmsContent.CONTENT_URI,
                SmsContent.COLUMN_THREAD_ID + "=" + draft.getThreadId()
                        + " AND " + SmsContent.COLUMN_TYPE + "="
                        + Sms.BOX_DRAFT + " AND " + SmsContent.COLUMN_ID + "<>"
                        + insertedId, null);
        getObservedContentResolver().delete(MmsContent.CONTENT_URI_DRAFT,
                MmsContent.COLUMN_THREAD_ID + "=" + draft.getThreadId(), null);

        // TODO Update thread
        // updateThread(draft.getThreadId());

        return draft;
    }

    /**
     * @param body
     *            The body of the sms which to send.
     * @param recipients
     *            The recipients of the sms.
     * @param smsSender
     *            An instance of the SmsSender.
     * @param simId
     *            The simid of the sms.
     * @return The array of the sent sms.
     */
    public Sms[] sendSms(final String body, final String[] recipients,
            final SmsSender smsSender, final int simId) {
        if (null == body) {
            Debugger.logE(new Object[] { body, recipients, smsSender, simId },
                    "Sms body should not be null.");
            return null;
        }
        if (0 == body.length()) {
            Debugger.logE(new Object[] { body, recipients, smsSender, simId },
                    "Sms body should not be empty.");
            return null;
        }
        if (null == recipients || 0 >= recipients.length) {
            Debugger.logE(new Object[] { body, recipients, smsSender, simId },
                    "At least one recipient should be specified.");
            return null;
        }
        if (null == smsSender) {
            Debugger.logE(new Object[] { body, recipients, smsSender, simId },
                    "Sms sender is null. Sms will not be sent.");
        }

        final Sms[] results = new Sms[recipients.length];
        Sms outgoingSms;
        String address;
        // Get thread id
        final Set<String> threadAddress = new HashSet<String>(recipients.length);
        for (int i = 0; i < recipients.length; i++) {
            if (null != recipients[i]) {
                threadAddress.add(recipients[i]);
            }
        }
        final long threadId = getOrCreateThreadId(threadAddress);
        // Create SMS and save it in outbox first
        for (int i = 0; i < recipients.length; i++) {
            outgoingSms = new Sms();
            // Set body
            outgoingSms.setBody(body);
            // Set address
            address = recipients[i];
            if (null == address) {
                Debugger.logW(
                        new Object[] { body, recipients, smsSender, simId },
                        "Address is null. Sms will not be sent.");
                // Send fail
                outgoingSms.setId(DatabaseRecordEntity.ID_NULL);
                continue;
            } else {
                outgoingSms.setTarget(new TargetAddress(address));
            }
            // Set date
            outgoingSms.setDate(System.currentTimeMillis());
            // Put it in outbox temporarily
            outgoingSms.setBox(Sms.BOX_OUTBOX);
            // Set thread ID
            outgoingSms.setThreadId(threadId);
            // For MTK Dual-SIM feature.
            // Set SIM ID
            // outgoingSms.setSimId(simId); Modified by Shaoying Han
            final SimDetailInfo info = Global.getSimInfoBySlot(simId);
            outgoingSms.setSimId(info.getSimId());
            outgoingSms.setSimName(info.getDisplayName());
            outgoingSms.setSimNumber(info.getNumber());
            outgoingSms.setSimICCId(info.getICCId());

            Debugger.logD(new Object[] { body, recipients, smsSender, simId },
                    "Do not insert send out sms for KK default sms app");
//            insertSms(outgoingSms, false);
            
            // After insertion, SMS ID will be set. Now add it to the results.
            results[i] = outgoingSms;

            // Append the SMS message in SMS sender's task queue.
            smsSender.appendTask(outgoingSms, getContext(), simId);
        }
        return results;
    }

    /**
     * @param id
     *            The id the sms for resending.
     * @param date
     *            The date of the sms.
     * @param body
     *            The body of the sms.
     * @param recipient
     *            The recipient of the sms.
     * @param smsSender
     *            An instance of the SmsSender for sending.
     * @param simId
     *            The sim id .
     * @return The resend sms or null.
     */
    public Sms resendSms(final long id, final long date, final String body,
            final String recipient, final SmsSender smsSender, final int simId) {
        if (null == body) {
            Debugger.logE(
                    new Object[] { id, date, body, recipient, smsSender },
                    "Sms body should not be null.");
            return null;
        } else if (0 == body.length()) {
            Debugger.logE(
                    new Object[] { id, date, body, recipient, smsSender },
                    "Sms body should not be empty.");
            return null;
        }
        if (null == recipient) {
            Debugger.logE(
                    new Object[] { id, date, body, recipient, smsSender },
                    "Sms recipient should not be null.");
            return null;
        }

        final Sms outgoingSms;
        final long newDate = System.currentTimeMillis();
        final ContentValues values = new ContentValues(3);
        values.put(SmsContent.COLUMN_DATE, newDate);
        values.put(SmsContent.COLUMN_TYPE, Sms.BOX_OUTBOX);
        // For MTK Dual-SIM feature. Modified by Shaoying Han
        final SimDetailInfo simInfo = Global.getSimInfoBySlot(simId);
        values.put(SmsContent.COLUMN_SIM_ID, simInfo.getSimId());
        // Update the box type and date of the SMS to resend
        final int updatedCount = getContentResolver().update(
                SmsContent.CONTENT_URI,
                values,
                SmsContent.COLUMN_ID + "=" + id + " AND "
                        + SmsContent.COLUMN_DATE + "=" + date + " AND "
                        + SmsContent.COLUMN_TYPE + "=" + Sms.BOX_FAILED
                        + " AND " + SmsContent.COLUMN_ADDRESS + "='"
                        + recipient + "' AND " + SmsContent.COLUMN_BODY + "='"
                        + body + "'", null);

        if (updatedCount == 1) {
            // SMS to resend found and updated
            outgoingSms = new Sms();
            outgoingSms.setId(id);
            outgoingSms.setDate(newDate);
            outgoingSms.setTarget(new TargetAddress(recipient));
            outgoingSms.setBody(body);
            // Put it in outbox temporarily
            outgoingSms.setBox(Sms.BOX_OUTBOX);
            // For MTK Dual-SIM feature.
            // outgoingSms.setSimId(simId); Modified by Shaoying Han
            final SimDetailInfo info = Global.getSimInfoBySlot(simId);
            outgoingSms.setSimId(info.getSimId());
            outgoingSms.setSimName(info.getDisplayName());
            outgoingSms.setSimNumber(info.getNumber());
            outgoingSms.setSimICCId(info.getICCId());

            // Append the SMS message in SMS sender's task queue.
            smsSender.appendTask(outgoingSms, getContext(), simId);
        } else {
            // SMS to resend does not exist
            return null;
        }

        return outgoingSms;
    }

    /**
     * @param sms
     *            The sms to insert.
     * @param regenThreadId
     *            The thread id which to insert.
     * @return The insert id of the sms.
     */
    public long insertSms(final Sms sms, final boolean regenThreadId) {
        if (null == sms) {
            Debugger.logE(new Object[] { sms, regenThreadId }, "Sms is null.");
            return DatabaseRecordEntity.ID_NULL;
        }
        long insertedId = DatabaseRecordEntity.ID_NULL;
        // Set thread ID
        if (regenThreadId) {
            if (null != sms.getTarget()) {
                final String address = sms.getTarget().getAddress();
                if (null != address) {
                    sms.setThreadId(getOrCreateThreadId(address));
                }
            }
        }

        final ContentValues values = SmsContent.createContentValues(sms, false,
                true);
        final Uri uri = getObservedContentResolver().insert(
                SmsContent.CONTENT_URI, values);
        try {
            insertedId = Long.parseLong(uri.getLastPathSegment());
        } catch (final NumberFormatException e) {
            Debugger.logE(new Object[] { sms, regenThreadId }, null, e);
        } catch (final NullPointerException e) {
            Debugger.logE(new Object[] { sms, regenThreadId }, null, e);
        }
        // Set the new id after insertion
        sms.setId(insertedId);

        return insertedId;
    }

    /**
     * Get the mms threadId through insert a sms
     * 
     * @param address
     *            The address of sms to insert.
     * @return The thread id of the insert sms.
     */
    public long insertSms(final String address) {
        Debugger.logI(new Object[] { address }, ">>insertSms");
        final ContentValues values = new ContentValues();
        values.put("address", address);
        getObservedContentResolver().insert(SmsContent.CONTENT_URI, values);
        final Cursor cursor = getContentResolver().query(
                SmsContent.CONTENT_URI, null, null, null, null);
        cursor.moveToLast();
        final long threadId = cursor.getLong(cursor
                .getColumnIndex(SmsContent.COLUMN_THREAD_ID));
        final long smsId = cursor.getLong(cursor.getColumnIndex("_id"));
        cursor.close();
        getObservedContentResolver().delete(SmsContent.CONTENT_URI,
                "_id=" + smsId, null);
        return threadId;
    }

    /**
     * @param mms
     *            The mms to insert.
     * @param isBackup
     *            Whether to backup.
     */
    public void insertMmsOld(final Mms mms, final boolean isBackup) {
        if (null != mms) {
            // -- insert pdu table begin
            Debugger.logI(new Object[] { mms }, ">>insertMms begin");
            Debugger.logI(new Object[] { mms }, "The Mms id is" + mms.getId());
            final ContentValues valuesPdu = new ContentValues(19);
            valuesPdu.put(MmsContent.COLUMN_ID, mms.getId());
            valuesPdu.put(MmsContent.COLUMN_THREAD_ID, mms.getThreadId());
            valuesPdu.put(MmsContent.COLUMN_SUBJECT, mms.getSubject());
            valuesPdu.put(MmsContent.COLUMN_CT_T, mms.getContentType());
            valuesPdu.put(MmsContent.COLUMN_MSG_BOX, mms.getBox());
            valuesPdu.put(MmsContent.COLUMN_DATE, mms.getDate() / 1000);
            valuesPdu.put(MmsContent.COLUMN_READ,
                    mms.isRead() ? DatabaseRecordEntity.TRUE
                            : DatabaseRecordEntity.FALSE);
            valuesPdu.put(MmsContent.COLUMN_LOCKED,
                    mms.isLocked() ? DatabaseRecordEntity.TRUE
                            : DatabaseRecordEntity.FALSE);

            valuesPdu.put(MmsContent.COLUMN_M_ID, mms.getM_id());
            valuesPdu.put(MmsContent.COLUMN_SUBJECT_CHAR_SET, mms.getSub_cs());
            valuesPdu.put(MmsContent.COLUMN_M_CLS, mms.getM_cls());
            valuesPdu.put(MmsContent.COLUMN_M_TYPE, mms.getM_type());
            valuesPdu.put(MmsContent.COLUMN_V, mms.getV());
            valuesPdu.put(MmsContent.COLUMN_M_SIZE, mms.getM_size());
            valuesPdu.put(MmsContent.COLUMN_TR_ID, mms.getTr_id());
            valuesPdu.put(MmsContent.COLUMN_D_RPT, mms.getD_rpt());
            valuesPdu.put(MmsContent.COLUMN_SEEN, mms.getSeen());
            valuesPdu.put(MmsContent.COLUMN_SIM_ID, mms.getSimId());

            valuesPdu.put(MmsContent.COLUMN_DATE_SENT, mms.getDate_sent());

            getContentResolver().insert(MmsContent.CONTENT_URI, valuesPdu);
            Debugger.logI(new Object[] { mms }, ">>insertMmsPdu end");
            // -- insert pdu table end

            // -- new way to handle draft box 2012-5-17 mtk54043
            // -- insert addr table begin
            ContentValues valuesAddr[] = null;
            if (mms.getBox() == Mms.BOX_DRAFT) {
                final String address = mms.getTarget().getAddress();
                final String[] addressList = address.split(",");
                valuesAddr = new ContentValues[addressList.length + 1];
                for (int i = 0; i < addressList.length + 1; i++) {
                    valuesAddr[i] = new ContentValues(4);
                    if (i == 0) {
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_TYPE,
                                MmsContent.ADDR_TYPE_RECEIVE);
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_ADDRESS,
                                "insert-address-token");
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_MSG_ID, mms
                                .getId());
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_CHARSET,
                                MmsContent.ADDR_CHARSET_VALUE);
                    } else {
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_TYPE,
                                MmsContent.ADDR_TYPE_SENT);
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_ADDRESS,
                                addressList[i - 1].trim());
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_MSG_ID, mms
                                .getId());
                        valuesAddr[i].put(MmsContent.COLUMN_ADDR_CHARSET,
                                MmsContent.ADDR_CHARSET_VALUE);
                    }
                }
            } else {
                valuesAddr = new ContentValues[2];
                valuesAddr[0] = new ContentValues(4);
                valuesAddr[1] = new ContentValues(4);
                if (mms.getBox() == Mms.BOX_SENT) {
                    valuesAddr[0].put(MmsContent.COLUMN_ADDR_TYPE,
                            MmsContent.ADDR_TYPE_SENT);
                    valuesAddr[1].put(MmsContent.COLUMN_ADDR_TYPE,
                            MmsContent.ADDR_TYPE_RECEIVE);
                } else {
                    valuesAddr[0].put(MmsContent.COLUMN_ADDR_TYPE,
                            MmsContent.ADDR_TYPE_RECEIVE);
                    valuesAddr[1].put(MmsContent.COLUMN_ADDR_TYPE,
                            MmsContent.ADDR_TYPE_SENT);
                }
                valuesAddr[0].put(MmsContent.COLUMN_ADDR_ADDRESS, mms
                        .getTarget().getAddress());
                valuesAddr[0].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
                valuesAddr[0].put(MmsContent.COLUMN_ADDR_CHARSET,
                        MmsContent.ADDR_CHARSET_VALUE);
                valuesAddr[1].put(MmsContent.COLUMN_ADDR_ADDRESS, mms
                        .getTarget().getAddress());
                valuesAddr[1].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
                valuesAddr[1].put(MmsContent.COLUMN_ADDR_CHARSET,
                        MmsContent.ADDR_CHARSET_VALUE);
            }
            final Uri uriAddr = Uri.parse("content://mms/" + mms.getId()
                    + "/addr");
            getContentResolver().bulkInsert(uriAddr, valuesAddr);
            Debugger.logI(new Object[] { mms }, ">>insertMmsAddr end");
            // -- insert addr table end
            // -- insert parts table begin
            final ArrayList<MmsPart> mmsParts = (ArrayList<MmsPart>) mms
                    .getParts();
            final long beginPartId = getMaxMmsPartId() + 1;
            long j = 0;
            for (final MmsPart part : mmsParts) {
                final ContentValues valuesPart = new ContentValues(8);
                valuesPart.put(MmsContent.COLUMN_PART_MID, mms.getId());
                // Debugger.logI(new Object[] { mms }, "mmspart>> mmsId is "
                // + mms.getId());
                valuesPart.put(MmsContent.COLUMN_PART_SEQ, part.getSequence());
                valuesPart.put(MmsContent.COLUMN_PART_CONTENTTYPE, part
                        .getContentType());
                valuesPart.put(MmsContent.COLUMN_PART_NAME, part.getName());
                valuesPart.put(MmsContent.COLUMN_PART_CHARSET, part
                        .getCharset());
                valuesPart.put(MmsContent.COLUMN_PART_CID, part.getCid());
                valuesPart.put(MmsContent.COLUMN_PART_CL, part.getCl());
                valuesPart.put(MmsContent.COLUMN_PART_TEXT, part.getText());
                final Uri uri = Uri.parse("content://mms/" + mms.getId()
                        + "/part");

                try {
                    getContentResolver().insert(uri, valuesPart);
                } catch (final IllegalStateException e) {
                    Debugger.logE(new Object[] { mms.getId() },
                            "Catch IllegalStateException ,"
                                    + " maybe this part doesn't insert to db");
                    e.printStackTrace();
                }
                // -- insert parts table end

                // -- write parts
                if (part.getDataPath() != null) {
                    try {
                        final byte[] buffer = part.getByteArray();
                        if (null != buffer) {
                            Debugger.logI(new Object[] { part },
                                    "DataPath is :" + part.getDataPath());
                            final OutputStream os = getContentResolver()
                                    .openOutputStream(
                                            Uri.parse("content://mms/part/"
                                                    + (beginPartId + j)));
                            os.write(part.getByteArray());
                            os.flush();
                            os.close();
                        }
                    } catch (final FileNotFoundException e) {
                        e.printStackTrace();
                    } catch (final IOException e) {
                        e.printStackTrace();
                    }
                }
                j++;
            }
            Debugger.logI(new Object[] { mms }, ">>WriteMmsPart end");
        }
    }
    
    /**
     * 
     * @param mms
     *            the mms to insert
     * @param isBackup
     *            is backup or import
     */
    public boolean insertMms(Mms mms) {
        if (mms == null) {
            return false;
        }
        /** A mms have three parts,separate it and insert */
        if (-1 == insertPdu(mms)) {
            return false;
        }
        insertAddr(mms);
        insertPart(mms);
        
        return true;
    }

    /**
     * 
     * @param mms
     *            the mms to insert
     * @return
     */
    public long insertPdu(Mms mms) {
        long insertId = 0l;
        Debugger.logI(new Object[] { mms }, ">>insertMms begin");
        final ContentValues valuesPdu = new ContentValues(18);
        valuesPdu.put(MmsContent.COLUMN_THREAD_ID, mms.getThreadId());
        valuesPdu.put(MmsContent.COLUMN_SUBJECT, mms.getSubject());
        valuesPdu.put(MmsContent.COLUMN_CT_T, mms.getContentType());
        valuesPdu.put(MmsContent.COLUMN_MSG_BOX, mms.getBox());
        valuesPdu.put(MmsContent.COLUMN_DATE, mms.getDate() / 1000);
        valuesPdu.put(MmsContent.COLUMN_READ, mms.isRead() ? DatabaseRecordEntity.TRUE : DatabaseRecordEntity.FALSE);
        valuesPdu.put(MmsContent.COLUMN_LOCKED, mms.isLocked() ? DatabaseRecordEntity.TRUE : DatabaseRecordEntity.FALSE);
        valuesPdu.put(MmsContent.COLUMN_M_ID, mms.getM_id());
        valuesPdu.put(MmsContent.COLUMN_SUBJECT_CHAR_SET, mms.getSub_cs());
        valuesPdu.put(MmsContent.COLUMN_M_CLS, mms.getM_cls());
        valuesPdu.put(MmsContent.COLUMN_M_TYPE, mms.getM_type());
        valuesPdu.put(MmsContent.COLUMN_V, mms.getV());
        valuesPdu.put(MmsContent.COLUMN_M_SIZE, mms.getM_size());
        valuesPdu.put(MmsContent.COLUMN_TR_ID, mms.getTr_id());
        valuesPdu.put(MmsContent.COLUMN_D_RPT, mms.getD_rpt());
        valuesPdu.put(MmsContent.COLUMN_SEEN, mms.getSeen());
        valuesPdu.put(MmsContent.COLUMN_SIM_ID, mms.getSimId());
        valuesPdu.put(MmsContent.COLUMN_DATE_SENT, mms.getDate_sent());

        try {
            Uri uriPdu = getContentResolver().insert(MmsContent.CONTENT_URI, valuesPdu);
            insertId = Long.parseLong(uriPdu.getLastPathSegment());
            /** Set mms id */
            mms.setId(insertId);

            Debugger.logI(new Object[] { mms }, ">>insertMmsPdu end mms id is " + insertId);
        } catch(Exception e) {
            Debugger.logI(new Object[] { mms }, ">>insertMmsPdu exception !");
            insertId = -1;
        }
        
        return insertId;
    }

    /**
     * 
     * @param mms
     *            the mms to insert
     */
    public void insertAddr(Mms mms) {
        // -- insert addr table begin
        Debugger.logI(new Object[] { mms }, ">>insertMmsAddr begin");
        Debugger.logI(new Object[] { mms }, ">>insertMmsAddr mms id = " + mms.getId());
        ContentValues valuesAddr[] = null;
        if (mms.getBox() == Mms.BOX_DRAFT) {
            final String address = mms.getTarget().getAddress();
            final String[] addressList = address.split(",");
            valuesAddr = new ContentValues[addressList.length + 1];
            for (int i = 0; i < addressList.length + 1; i++) {
                valuesAddr[i] = new ContentValues(4);
                if (i == 0) {
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_RECEIVE);
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_ADDRESS, "insert-address-token");
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_CHARSET, MmsContent.ADDR_CHARSET_VALUE);
                } else {
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_SENT);
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_ADDRESS, addressList[i - 1].trim());
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
                    valuesAddr[i].put(MmsContent.COLUMN_ADDR_CHARSET, MmsContent.ADDR_CHARSET_VALUE);
                }
            }
        } else {
            valuesAddr = new ContentValues[2];
            valuesAddr[0] = new ContentValues(4);
            valuesAddr[1] = new ContentValues(4);
            if (mms.getBox() == Mms.BOX_SENT) {
                valuesAddr[0].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_SENT);
                valuesAddr[1].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_RECEIVE);
            } else {
                valuesAddr[0].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_RECEIVE);
                valuesAddr[1].put(MmsContent.COLUMN_ADDR_TYPE, MmsContent.ADDR_TYPE_SENT);
            }
            valuesAddr[0].put(MmsContent.COLUMN_ADDR_ADDRESS, mms.getTarget().getAddress());
            valuesAddr[0].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
            valuesAddr[0].put(MmsContent.COLUMN_ADDR_CHARSET, MmsContent.ADDR_CHARSET_VALUE);
            valuesAddr[1].put(MmsContent.COLUMN_ADDR_ADDRESS, mms.getTarget().getAddress());
            valuesAddr[1].put(MmsContent.COLUMN_ADDR_MSG_ID, mms.getId());
            valuesAddr[1].put(MmsContent.COLUMN_ADDR_CHARSET, MmsContent.ADDR_CHARSET_VALUE);
        }
        final Uri uriAddr = Uri.parse("content://mms/" + mms.getId() + "/addr");
        getContentResolver().bulkInsert(uriAddr, valuesAddr);
        Debugger.logI(new Object[] { mms }, ">>insertMmsAddr end");
        // -- insert addr table end
    }

    /**
     * 
     * @param mms
     *            the mms to insert
     */
    public void insertPart(Mms mms) {
        // -- insert parts table begin
        long partId = 0l;
        final ArrayList<MmsPart> mmsParts = (ArrayList<MmsPart>) mms.getParts();
        for (final MmsPart part : mmsParts) {
            final ContentValues valuesPart = new ContentValues(8);
            valuesPart.put(MmsContent.COLUMN_PART_MID, mms.getId());
            Debugger.logI(new Object[] { mms }, "mmspart>> mmsId is " + mms.getId());
            valuesPart.put(MmsContent.COLUMN_PART_SEQ, part.getSequence());
            valuesPart.put(MmsContent.COLUMN_PART_CONTENTTYPE, part.getContentType());
            valuesPart.put(MmsContent.COLUMN_PART_NAME, part.getName());
            valuesPart.put(MmsContent.COLUMN_PART_CHARSET, part.getCharset());
            valuesPart.put(MmsContent.COLUMN_PART_CID, part.getCid());
            valuesPart.put(MmsContent.COLUMN_PART_CL, part.getCl());
            valuesPart.put(MmsContent.COLUMN_PART_TEXT, part.getText());
            final Uri uri = Uri.parse("content://mms/" + mms.getId() + "/part");

            try {
                Uri uriPart = getContentResolver().insert(uri, valuesPart);
                partId = Long.parseLong(uriPart.getLastPathSegment());
            } catch (final IllegalStateException e) {
                Debugger.logE(new Object[] { mms.getId() }, "Catch IllegalStateException ,"
                        + " maybe this part doesn't insert to db");
                e.printStackTrace();
            }
            // -- insert parts table end
            // -- write parts
            if (part.getDataPath() != null) {
                try {
                    final byte[] buffer = part.getByteArray();
                    if (null != buffer) {
                        Debugger.logI(new Object[] { part }, "DataPath is :" + part.getDataPath());
                        final OutputStream os = getContentResolver().openOutputStream(
                                Uri.parse("content://mms/part/" + partId));
                        os.write(part.getByteArray());
                        os.flush();
                        os.close();
                    }
                } catch (final FileNotFoundException e) {
                    e.printStackTrace();
                } catch (final IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * @param id
     *            The id of the sms to update.
     * @param newOne
     *            The newSms for updating.
     * @param updateId
     *            Whether to set update id.
     * @param regenThreadId
     *            Whether to set thread id.
     * @return The count of the updated sms.
     */
    public long updateSms(final long id, final Sms newOne,
            final boolean updateId, final boolean regenThreadId) {
        if (null == newOne) {
            Debugger.logE(new Object[] { id, newOne, updateId, regenThreadId },
                    "New sms is null.");
            return 0;
        }
        int updatedCount = 0;
        // Set thread ID
        if (regenThreadId) {
            if (null != newOne.getTarget()) {
                final String address = newOne.getTarget().getAddress();
                if (null != address) {
                    newOne.setThreadId(getOrCreateThreadId(address));
                }
            }
        }

        final ContentValues values = SmsContent.createContentValues(newOne,
                updateId, true);
        updatedCount = getObservedContentResolver().update(
                SmsContent.CONTENT_URI, values,
                SmsContent.COLUMN_ID + "=" + id, null);

        return updatedCount;
    }

    /**
     * This is a single-recipient version of getOrCreateThreadId. It's
     * convenient for use with SMS messages.
     * 
     * @param recipient
     *            The recipient of the message.
     * @return The thread id.
     */
    public long getOrCreateThreadId(final String recipient) {
        final Set<String> recipients = new HashSet<String>();

        recipients.add(recipient);
        return getOrCreateThreadId(recipients);
    }

    /**
     * For mul-recipient
     * 
     * @param recipientList
     *            The recipient list of the message.
     * @return The thread id.
     */
    public long getOrCreateThreadId2(final String[] recipientList) {
        final Set<String> recipients = new HashSet<String>();
        for (final String recipient : recipientList) {
            recipients.add(recipient);
        }
        return getOrCreateThreadId(recipients);
    }

    /**
     * Given the recipients list and subject of an unsaved message, return its
     * thread ID. If the message starts a new thread, allocate a new thread ID.
     * Otherwise, use the appropriate existing thread ID.
     * 
     * Find the thread ID of the same set of recipients (in any order, without
     * any additions). If one is found, return it. Otherwise, return a unique
     * thread ID.
     * 
     * @param recipients
     *            The recipients of the message.
     * @return The thread id of the message.
     */
    public long getOrCreateThreadId(final Set<String> recipients) {
        long threadId = DatabaseRecordEntity.ID_NULL;
        final Uri.Builder uriBuilder = ConversationsContent.THREAD_ID_CONTENT_URI
                .buildUpon();

        for (String recipient : recipients) {
            if (MmsContent.isEmailAddress(recipient)) {
                recipient = MmsContent.extractAddrSpec(recipient);
            }
            uriBuilder.appendQueryParameter("recipient", recipient);
        }

        final Uri uri = uriBuilder.build();
        final Cursor cursor = getObservedContentResolver().query(uri,
                new String[] { SmsContent.COLUMN_ID }, null, null, null);

        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    threadId = cursor.getLong(0);
                    Debugger.logI(new Object[] { recipients },
                            "Thread ID allocated: " + threadId);
                } else {
                    Debugger.logE(new Object[] { recipients },
                            "Fail to get or allocate a thread ID.");
                }
            } finally {
                cursor.close();
            }
        } else {
            Debugger.logE(new Object[] { recipients }, "Cursor is null.");
        }
        return threadId;
    }

    // Statistic ---------------------------------------------------------------
    /**
     * Get count of messages.
     * 
     * @return Count of messages in total.
     */
    public int getMessagesCount() {
        int count;

        count = getSmsCount() + getMmsCount();

        return count;
    }

    /**
     * Get count of SMS messages.
     * 
     * @return Count of SMS messages in total.
     */
    public int getSmsCount() {
        int count = 0;
        Cursor c;

        c = getContentResolver().query(SmsContent.CONTENT_URI,
                new String[] { SmsContent.COLUMN_ID },
                SmsContent.COLUMN_THREAD_ID + ">0"
                /*
                 * + " AND (" + SmsContent.COLUMN_TYPE + "=" + Sms.BOX_INBOX +
                 * " OR " + SmsContent.COLUMN_TYPE + "=" + Sms.BOX_SENT + " OR "
                 * + SmsContent.COLUMN_TYPE + "=" + Sms.BOX_DRAFT + " OR " +
                 * SmsContent.COLUMN_TYPE + "=" + Sms.BOX_FAILED + ")"
                 */, null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        }

        return count;
    }

    /**
     * Get count of MMS messages.
     * 
     * @return Count of MMS messages in total.
     */
    public int getMmsCount() {
        int count = 0;
        Cursor c;

        c = getContentResolver().query(MmsContent.CONTENT_URI,
                new String[] { MmsContent.COLUMN_ID },
                MmsContent.COLUMN_THREAD_ID + ">0"
                /*
                 * + " AND (" + MmsContent.COLUMN_MSG_BOX + "=" + Mms.BOX_INBOX
                 * + " OR " + MmsContent.COLUMN_MSG_BOX + "=" + Mms.BOX_SENT +
                 * " OR " + MmsContent.COLUMN_MSG_BOX + "=" + Mms.BOX_DRAFT +
                 * ")"
                 */, null, null);
        if (c != null) {
            count = c.getCount();
            c.close();
        }

        return count;
    }

    // Sync Mms application on phone
    private void updateMmsNotifications() {
        // FIXME Is there any way to do this?
        // MessagingNotification.blockingUpdateAllNotifications(getContext());
    }

    private void updateThread(final long threadId) {
        final Cursor c = getContentResolver().query(
                Uri.withAppendedPath(SmsContent.CONTENT_URI_CONVERSATIONS,
                        String.valueOf(threadId)), null, null, null, null);
        if (null != c && !c.isClosed()) {
            c.close();
        }
    }

    /**
     * @param box
     *            Which box to get max sms id.
     * @return The max sms id .
     */
    public long getMaxSmsId(final int box) {
        long maxId = 0L;
        Cursor c;

        c = getContentResolver().query(SmsContent.CONTENT_URI,
                new String[] { SmsContent.COLUMN_ID },
                SmsContent.COLUMN_TYPE + "=" + box, null,
                SmsContent.COLUMN_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        return maxId;
    }

    /**
     * get the max id of pdu table
     * 
     * @param box
     *            Which box to get the max mms id.
     * @return The max mms id in the box.
     */
    public long getMaxMmsId(final int box) {
        long maxId = 0L;
        Cursor c;

        c = getContentResolver().query(MmsContent.CONTENT_URI,
                new String[] { MmsContent.COLUMN_ID },
                MmsContent.COLUMN_MSG_BOX + "=" + box, null,
                MmsContent.COLUMN_ID + " DESC");
        if (null != c) {
            if (c.moveToNext()) {
                maxId = c.getLong(0);
            }
            c.close();
        }
        Debugger.logI(new Object[] { box }, "MaxMmsId is :" + maxId);
        return maxId;
    }

    /**
     * @param id
     *            The id of the sms.
     * @return The sms according to the id.
     */
    public Sms getSms(final long id) {
        Sms result = null;
        Cursor c;

        c = getContentResolver().query(
                SmsContent.CONTENT_URI,
                new String[] { SmsContent.COLUMN_ID,
                        SmsContent.COLUMN_THREAD_ID, SmsContent.COLUMN_ADDRESS,
                        SmsContent.COLUMN_DATE, SmsContent.COLUMN_TYPE,
                        SmsContent.COLUMN_READ, SmsContent.COLUMN_SUBJECT,
                        SmsContent.COLUMN_LOCKED, SmsContent.COLUMN_BODY,
                        SmsContent.COLUMN_SERVICE_CENTER,
                        // For MTK Dual-SIM feature.
                        SmsContent.COLUMN_SIM_ID },
                SmsContent.COLUMN_ID + "=" + id, null, null);
        if (null != c) {
            if (c.moveToNext()) {
                result = SmsContent.cursorToSms(c);
            }
            c.close();
        }
        return result;
    }

    /**
     * Get a MMS information from pdu,threads,canonical_addresses tables
     * 
     * @param id
     *            The id of the mms.
     * @return A mms or null.
     */

    public Mms getMms(final long id) {
        Mms result = null;
        Cursor c = null;
        c = getContentResolver().query(
                MmsContent.CONTENT_URI,
                new String[] { MmsContent.COLUMN_ID,
                        MmsContent.COLUMN_THREAD_ID, MmsContent.COLUMN_DATE,
                        MmsContent.COLUMN_M_TYPE, MmsContent.COLUMN_READ,
                        MmsContent.COLUMN_SUBJECT, MmsContent.COLUMN_LOCKED,
                        MmsContent.COLUMN_MSG_BOX, MmsContent.COLUMN_SIM_ID },
                MmsContent.COLUMN_ID + "=" + id, null, null);

        if (null != c) {
            if (c.moveToNext()) {
                result = MmsContent.cursorToMms(c);
            }
            // -- A new way to get address 2012-5-16 mtk54043
            // if m_type = 130, this MMS is a notify
            if (null != result && null != result.getM_type()
                    && !MmsContent.NOTIFY_MMS.equals(result.getM_type())) {
                result.setTarget(getMmsAddress(result));
            } else {
                c.close();
                return null;
            }
            // --
        } else {
            Debugger.logW(new Object[] { id }, "Cursor is null");
        }
        if (null != c) {
            c.close();
        }
        return result;
    }

    /**
     * Get SMS address from sms,threads,canonical_addresses tables by join query
     * 
     * @param threadId
     *            The thread id of the sms.
     * @return The sms's address or null.
     */
    public String getSmsAddress(final long threadId) {
        final StringBuffer draftAddress = new StringBuffer();
        Cursor draftCursorRecipient = null;
        try {
            draftCursorRecipient = getContentResolver()
                    .query(
                            SmsContent.CONTENT_URI,
                            new String[] { "threads.recipient_ids "
                                    + "from sms,threads "
                                    + "where sms.thread_id=threads._id and sms.thread_id ="
                                    + threadId + " --" }, null, null, null);
        } catch (final SQLiteException e) {
            Debugger.logE("Catch SQLiteException");
            e.printStackTrace();
            if (null != draftCursorRecipient) {
                draftCursorRecipient.close();
                draftCursorRecipient = null;
            }
            return null;
        }

        if (draftCursorRecipient != null && draftCursorRecipient.moveToFirst()) {
            final String recipientIds = draftCursorRecipient
                    .getString(draftCursorRecipient
                            .getColumnIndex("recipient_ids"));
            Debugger.logI(new Object[] {}, "recipient_ids = " + recipientIds);
            if (null == recipientIds || recipientIds.equals("")) {
                draftCursorRecipient.close();
                draftCursorRecipient = null;
                return null;
            }
            final String[] recipientIdList = recipientIds.split(" ");
            if (null == recipientIdList || recipientIdList.length <= 0) {
                draftCursorRecipient.close();
                draftCursorRecipient = null;
                return null;
            }
            for (final String recipientId : recipientIdList) {
                Debugger.logI("recipient_id: " + recipientId);
            }

            if (null != draftCursorRecipient) {
                draftCursorRecipient.close();
                draftCursorRecipient = null;
            }

            // -- get address from canonical_addresses table
            String selection = null;
            final StringBuffer strBuf = new StringBuffer();
            strBuf.append("canonical_addresses.address "
                    + "from canonical_addresses "
                    + "where canonical_addresses._id in(");
            for (int i = 0; i < recipientIdList.length; i++) {
                strBuf.append(recipientIdList[i] + ",");
            }
            strBuf.deleteCharAt(strBuf.length() - 1);
            strBuf.append(")");
            strBuf.append(" --");
            selection = strBuf.toString();
            Debugger.logI("selection: " + selection);
            Cursor draftCursor = null;
            try {
                draftCursor = getContentResolver().query(
                        SmsContent.CONTENT_URI, new String[] { selection },
                        null, null, null);
            } catch (final SQLiteException e) {
                Debugger.logE("Catch SQLiteException");
                e.printStackTrace();
                if (null != draftCursor) {
                    draftCursor.close();
                    draftCursor = null;
                }
                return null;
            }
            if (draftCursor != null) {
                while (draftCursor.moveToNext()) {
                    final String address = draftCursor.getString(draftCursor
                            .getColumnIndex("address"));
                    draftAddress.append(address + ",");
                }
                if (draftAddress.length() > 0) {
                    draftAddress.deleteCharAt(draftAddress.length() - 1);
                    Debugger.logI(new Object[] {}, "draftAddress = "
                            + draftAddress);
                } else {
                    if (null != draftCursor) {
                        draftCursor.close();
                        draftCursor = null;
                    }
                    return null;
                }
            }
            if (null != draftCursor) {
                draftCursor.close();
                draftCursor = null;
            }
        }
        return draftAddress.toString();
    }

    /**
     * Get MMS address from pdu,threads,canonical_addresses tables by join query
     * 
     * @param mms
     *            The mms to get the mms address.
     * @return A target address.
     */
    public TargetAddress getMmsAddress(final Mms mms) {
        final long threadId = mms.getThreadId();
        final TargetAddress mmsAddress = new TargetAddress("");
        mmsAddress.setMmsId(mms.getId());
        if (mms.getBox() == Mms.BOX_DRAFT) {
            Cursor draftCursorRecipient = null;
            try {
                draftCursorRecipient = getContentResolver()
                        .query(
                                MmsContent.CONTENT_URI,
                                new String[] { "threads.recipient_ids "
                                        + "from pdu,threads "
                                        + "where pdu.thread_id=threads._id and pdu.thread_id ="
                                        + threadId + " --" }, null, null, null);
            } catch (final SQLiteException e) {
                Debugger.logE("Catch SQLiteException");
                e.printStackTrace();
                if (null != draftCursorRecipient) {
                    draftCursorRecipient.close();
                    draftCursorRecipient = null;
                }
                return mmsAddress;
            }

            if (draftCursorRecipient != null
                    && draftCursorRecipient.moveToFirst()) {
                final String recipientIds = draftCursorRecipient
                        .getString(draftCursorRecipient
                                .getColumnIndex("recipient_ids"));
                Debugger.logI(new Object[] {}, "recipient_ids = "
                        + recipientIds + " mms id = " + mms.getId());

                if (null == recipientIds || recipientIds.equals("")) {
                    draftCursorRecipient.close();
                    draftCursorRecipient = null;
                    return mmsAddress;
                }

                final String[] recipientIdList = recipientIds.split(" ");

                if (null == recipientIdList || recipientIdList.length <= 0) {
                    draftCursorRecipient.close();
                    draftCursorRecipient = null;
                    return mmsAddress;
                }

                for (final String recipientId : recipientIdList) {
                    Debugger.logI("recipientId: " + recipientId);
                }

                // -- get address from canonical_addresses table
                String selection = null;
                final StringBuffer strBuf = new StringBuffer();
                strBuf.append("canonical_addresses.address "
                        + "from canonical_addresses " + "where ");
                for (int i = 0; i < recipientIdList.length; i++) {
                    strBuf.append("canonical_addresses._id ="
                            + recipientIdList[i] + " or ");
                }
                strBuf.deleteCharAt(strBuf.length() - 1);
                strBuf.delete(strBuf.length() - 3, strBuf.length());
                strBuf.append(" --");
                selection = strBuf.toString();
                Debugger.logI("selection: " + selection);
                Cursor draftCursor = null;
                try {
                    draftCursor = getContentResolver().query(
                            MmsContent.CONTENT_URI, new String[] { selection },
                            null, null, null);
                } catch (final SQLiteException e) {
                    Debugger.logE("Catch SQLiteException");
                    e.printStackTrace();
                    draftCursorRecipient.close();
                    if (null != draftCursor) {
                        draftCursor.close();
                        draftCursor = null;
                    }
                    return mmsAddress;
                }

                final StringBuffer draftAddress = new StringBuffer();

                while (draftCursor != null && draftCursor.moveToNext()) {
                    final String address = draftCursor.getString(draftCursor
                            .getColumnIndex(SmsContent.COLUMN_ADDRESS));
                    draftAddress.append(address + ",");
                }

                if (draftAddress.length() > 0) {
                    draftAddress.deleteCharAt(draftAddress.length() - 1);
                    Debugger.logI(new Object[] {}, "draftAddress = "
                            + draftAddress + " mms id = " + mms.getId());
                    mmsAddress.setAddress(draftAddress.toString());
                }
                // close draftCursor
                if (null != draftCursor) {
                    draftCursor.close();
                    draftCursor = null;
                }
            }
            // close draftCursor_recipient
            if (null != draftCursorRecipient) {
                draftCursorRecipient.close();
                draftCursorRecipient = null;
            }
        } else {
            Cursor cursorAddress = null;
            try {
                cursorAddress = getContentResolver()
                        .query(
                                MmsContent.CONTENT_URI,
                                new String[] { "canonical_addresses.address "
                                        + "from pdu,threads,canonical_addresses "
                                        + "where pdu.thread_id=threads._id and "
                                        + "threads.recipient_ids=canonical_addresses._id "
                                        + "and pdu.thread_id =" + threadId
                                        + " --" }, null, null, null);
            } catch (final SQLiteException e) {
                Debugger.logE("Catch SQLiteException");
                e.printStackTrace();
                if (null != cursorAddress) {
                    cursorAddress.close();
                    cursorAddress = null;
                }
                return mmsAddress;
            }

            if (cursorAddress != null && cursorAddress.moveToFirst()) {
                final String draftAddress = cursorAddress
                        .getString(cursorAddress
                                .getColumnIndex(MmsContent.COLUMN_ADDR_ADDRESS));
                Debugger.logI(new Object[] {}, "draftAddress = " + draftAddress
                        + " mms id = " + mms.getId());
                mmsAddress.setAddress(draftAddress);
            } else {
                Debugger.logE(new Object[] {},
                        "No address in canonical_addresses" + " mms id = "
                                + mms.getId());
            }
            // close cursor_address
            if (null != cursorAddress) {
                cursorAddress.close();
                cursorAddress = null;
            }
        }

        return mmsAddress;
    }

    // ==============================================================
    // Inner & Nested classes
    // ==============================================================
}
