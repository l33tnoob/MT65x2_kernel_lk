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

import android.database.Cursor;
import android.net.Uri;

import com.mediatek.apst.target.data.provider.message.MmsContent;
import com.mediatek.apst.target.data.proxy.FastCursorParser;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.message.Mms;
import com.mediatek.apst.util.entity.message.MmsPart;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

/**
 * Class Name: FastMmsBackupCursorParser
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.message
 * <p>
 * <p>
 * Description:
 * <p>
 * Transfer whole MMS information to PC for backup
 * <p>
 * 
 * @author mtk54043 Yu.Chen
 * @version V1.0
 */
public class FastMmsBackupCursorParser extends FastCursorParser {

    private MessageProxy mMessageProxy;

    /**
     * @param cursor
     *            The cursor of message.
     * @param consumer
     *            Set a consumer to handle the asynchronous blocks.
     * @param buffer
     *            The buffer to save the message.
     * @param messageProxy
     *            A message proxy.
     */
    public FastMmsBackupCursorParser(Cursor cursor, IRawBlockConsumer consumer,
            ByteBuffer buffer, MessageProxy messageProxy) {
        super(cursor, consumer, buffer);
        mMessageProxy = messageProxy;
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

        Mms mms = MmsContent.cursorToMms(c);

        // A new way to get address 2012-5-16 mtk54043 start
        mms.setTarget(mMessageProxy.getMmsAddress(mms));
        // A new way to get address 2012-5-16 mtk54043 end

        /*
         * Cursor c_addr = null; Uri uri_addr = Uri.parse("content://mms/" +
         * mms.getId() + "/addr"); c_addr =
         * mMessageProxy.getContentResolver().query(uri_addr, null, null, null,
         * null); TargetAddress mmsAddress; if (null != c_addr) { int c_count =
         * c_addr.getCount(); if (c_count != 0) { c_addr.moveToFirst(); if
         * (mms.getBox() == Mms.BOX_SENT && c_count > 1) { c_addr.moveToNext();
         * } mmsAddress = new TargetAddress(c_addr.getString(c_addr
         * .getColumnIndex(MmsContent.COLUMN_ADDR_ADDRESS)));
         * mmsAddress.setMmsId(c_addr.getLong(c_addr
         * .getColumnIndex(MmsContent.COLUMN_ADDR_MSG_ID)));
         * mms.setTarget(mmsAddress); // Debugger.logI("mmsAddress is " +
         * mmsAddress.getAddress()); if (!c_addr.isClosed()) { // Release
         * resources c_addr.close(); c_addr = null; } } else {
         * mms.setTarget(null); } }
         */
        Cursor cPart = mMessageProxy.getContentResolver().query(
                MmsContent.CONTENT_URI_PART,
                new String[] { MmsContent.COLUMN_PART_ID,
                        MmsContent.COLUMN_PART_MID, MmsContent.COLUMN_PART_SEQ,
                        MmsContent.COLUMN_PART_CONTENTTYPE,
                        MmsContent.COLUMN_PART_NAME,
                        MmsContent.COLUMN_PART_CHARSET,
                        MmsContent.COLUMN_PART_CID, MmsContent.COLUMN_PART_CL,
                        MmsContent.COLUMN_PART_DATAPATH,
                        MmsContent.COLUMN_PART_TEXT },
                new String(MmsContent.COLUMN_PART_MID + "=" + mms.getId()),
                null, MmsContent.COLUMN_PART_ID);
        ArrayList<MmsPart> parts = new ArrayList<MmsPart>();
        if (null != cPart) {
            while (cPart.moveToNext()) {
                MmsPart part = MmsContent.cursorToMmsPart(cPart);
                if (null != part.getDataPath()) {
                    Uri uri = Uri.parse("content://mms/part/" + part.getId());
                    InputStream is;
                    try {

                        /**
                         * Reads bytes from this stream and stores them in the
                         * byte array bufferPart
                         */

                        is = mMessageProxy.getContentResolver()
                                .openInputStream(uri);
                        if (null != is && is.available() != 0) {
                            ByteArrayOutputStream baos = new ByteArrayOutputStream();
                            byte[] bufferTemp = new byte[256];
                            int len = is.read(bufferTemp);
                            while (len != -1) {
                                baos.write(bufferTemp, 0, len);
                                len = is.read(bufferTemp);
                            }
                            part.setByteArray(baos.toByteArray());
                            baos.flush();
                            baos.close();
                        } else {
                            part.setByteArray(null);
                        }
                        // release resource
                        if (null != is) {
                            is.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
                parts.add(part);
            }
            if (!cPart.isClosed()) {
                // Release resources
                cPart.close();
                cPart = null;
            }
        }
        mms.setParts(parts);

        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            mms.writeAllWithVersion(buffer, Config.VERSION_CODE);
        } catch (NullPointerException e) {
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

    @Override
    protected void onBlockReady() {
        try {
            super.onBlockReady();
        } catch (OutOfMemoryError e) {
            Debugger.logE(new Object[] {}, " --->Catch OutOfMemoryError");
            e.printStackTrace();
        }
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Debugger.logD(e.getMessage());
        }
    }
}
