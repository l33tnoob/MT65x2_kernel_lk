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
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.RawContactsEntity;

import com.mediatek.apst.target.data.proxy.FastCursorParser;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.util.entity.DatabaseRecordEntity;
import com.mediatek.apst.util.entity.RawTransUtil;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.util.HashMap;

public class FastPhoneListCursorParser extends FastCursorParser {
    private HashMap<Long, String> mMapIdToName;

    /**
     * @param c
     * @param consumer
     * @param buffer
     * @param mapIdToName
     */
    public FastPhoneListCursorParser(Cursor c, IRawBlockConsumer consumer,
            ByteBuffer buffer, HashMap<Long, String> mapIdToName) {
        super(c, consumer, buffer);
        this.mMapIdToName = mapIdToName;
    }

    /**
     * @param c
     * @param consumer
     * @param mapIdToName
     */
    public FastPhoneListCursorParser(Cursor c, IRawBlockConsumer consumer,
            HashMap<Long, String> mapIdToName) {
        super(c, consumer);
        this.mMapIdToName = mapIdToName;
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

        int columnId;
        long rawContactId = DatabaseRecordEntity.ID_NULL;
        String number = null;
        String name = null;
        columnId = c.getColumnIndex(RawContactsEntity._ID);
        if (columnId != -1) {
            rawContactId = c.getLong(columnId);
            name = mMapIdToName.get(rawContactId);
        }
        columnId = c.getColumnIndex(CommonDataKinds.Phone.NUMBER);
        if (columnId != -1) {
            number = c.getString(columnId);
        }

        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            buffer.putLong(rawContactId);
            RawTransUtil.putString(buffer, number);
            RawTransUtil.putString(buffer, name);
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
}
