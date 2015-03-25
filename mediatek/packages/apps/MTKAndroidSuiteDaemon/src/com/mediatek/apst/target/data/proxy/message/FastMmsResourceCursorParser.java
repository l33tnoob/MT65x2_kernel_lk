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
import com.mediatek.apst.util.entity.message.MmsPart;

import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

/**
 * Class Name: FastMmsResourceCursorParser
 * <p>
 * Package: com.mediatek.apst.target.data.proxy.message
 * <p>
 * <p>
 * Description:
 * <p>
 * Parse MMS resource
 * <p>
 * 
 * @author mtk54043 Yu.Chen
 * @version V1.0
 */
public class FastMmsResourceCursorParser extends FastCursorParser {

    private MessageProxy mMessageProxy;

    /**
     * @param c
     * @param consumer
     */
    public FastMmsResourceCursorParser(Cursor c, IRawBlockConsumer consumer) {
        super(c, consumer);

    }

    /**
     * @param c
     * @param consumer
     * @param buffer
     * @param messageProxy
     * @param id
     */
    public FastMmsResourceCursorParser(Cursor c, IRawBlockConsumer consumer,
            ByteBuffer buffer, MessageProxy messageProxy, long id) {
        super(c, consumer, buffer);
        mMessageProxy = messageProxy;
    }

    /**
     * @param c
     * @param consumer
     * @param buffer
     * @param messageProxy
     */
    public FastMmsResourceCursorParser(Cursor c, IRawBlockConsumer consumer,
            ByteBuffer buffer, MessageProxy messageProxy) {
        super(c, consumer, buffer);
        mMessageProxy = messageProxy;
    }

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
        // get MMS parts and resource
        MmsPart mmsPart = MmsContent.cursorToMmsPart(c);
        try {
            if (mmsPart.getDataPath() != null) {
                Uri uri = Uri.parse("content://mms/part/" + mmsPart.getId());
                InputStream is = mMessageProxy.getContentResolver()
                        .openInputStream(uri);

                try {
                    /**
                     * Reads bytes from this stream and stores them in the byte
                     * array bufferPart
                     */
                    if (null != is && is.available() != 0) {
                        ByteArrayOutputStream baos = new ByteArrayOutputStream();
                        byte[] bufferTemp = new byte[256];
                        int len = is.read(bufferTemp);
                        while (len != -1) {
                            baos.write(bufferTemp, 0, len);
                            len = is.read(bufferTemp);
                        }
                        mmsPart.setByteArray(baos.toByteArray());
                        baos.flush();
                        baos.close();
                    } else {
                        mmsPart.setByteArray(null);
                    }
                    // release resource
                    if (null != is) {
                        is.close();
                        is = null;
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        } catch (FileNotFoundException e) {
            mmsPart.setByteArray(null);
            e.printStackTrace();
        }
        buffer.mark();
        try {
            mmsPart.writeRawWithVersion(buffer, Config.VERSION_CODE);
        } catch (NullPointerException e) {
            Debugger.logE(new Object[] { c, buffer }, null, e);
            buffer.reset();
            return IRawBufferWritable.RESULT_FAIL;
        } catch (BufferOverflowException e) {
            buffer.reset();
            return IRawBufferWritable.RESULT_NOT_ENOUGH_BUFFER;
        }
        return IRawBufferWritable.RESULT_SUCCESS;
    }
}
