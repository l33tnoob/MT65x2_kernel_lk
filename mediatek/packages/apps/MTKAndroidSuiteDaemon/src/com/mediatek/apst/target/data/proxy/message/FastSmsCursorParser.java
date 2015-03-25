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

import com.mediatek.apst.target.data.provider.message.SmsContent;
import com.mediatek.apst.target.data.proxy.FastCursorParser;
import com.mediatek.apst.target.data.proxy.IRawBlockConsumer;
import com.mediatek.apst.target.data.proxy.IRawBufferWritable;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Debugger;
import com.mediatek.apst.target.util.StringUtils;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.util.HashMap;

public class FastSmsCursorParser extends FastCursorParser {

    private boolean mQueryContact;

    private MessageProxy mMessageProxy;

    private HashMap<String, TargetAddress> mMapAddressToContact;

    /**
     * @param c
     * @param consumer
     * @param buffer
     * @param queryContact
     * @param messageProxy
     */
    public FastSmsCursorParser(Cursor c, IRawBlockConsumer consumer,
            ByteBuffer buffer, boolean queryContact, MessageProxy messageProxy) {
        super(c, consumer, buffer);
        mQueryContact = queryContact;
        mMessageProxy = messageProxy;
        if (mQueryContact) {
            mMapAddressToContact = new HashMap<String, TargetAddress>();
        }
    }

    /**
     * @param c
     * @param consumer
     * @param queryContact
     * @param messageProxy
     */
    public FastSmsCursorParser(Cursor c, IRawBlockConsumer consumer,
            boolean queryContact, MessageProxy messageProxy) {
        super(c, consumer);
        mQueryContact = queryContact;
        mMessageProxy = messageProxy;
        if (mQueryContact) {
            mMapAddressToContact = new HashMap<String, TargetAddress>();
        }
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

        Sms sms = SmsContent.cursorToSms(c);
        if (null == sms) {
            return IRawBufferWritable.RESULT_FAIL;
        }

        // Get draft address 2012-5-16 mtk54043
        if (sms.getBox() == Sms.BOX_DRAFT) {
            sms.getTarget().setAddress(
                    mMessageProxy.getSmsAddress(sms.getThreadId()));
        }

        // Need to query contact
        if (mQueryContact) {
            String address = null;
            if (sms.getTarget() != null) {
                address = sms.getTarget().getAddress();
            }
            String number = StringUtils.dropServiceCenter(address);
            TargetAddress target;
            if (mMapAddressToContact.containsKey(number)) {
                // Address is already queried before, just get contact
                // info from map
                target = mMapAddressToContact.get(number);
            } else {
                // Address is not queried before
                target = mMessageProxy.lookupContact(number);
                // Store contact info in map
                mMapAddressToContact.put(number, target);
            }
            if (null != target) {
                sms.getTarget().setContactId(target.getContactId());
                sms.getTarget().setName(target.getName());
            }
        }
        // Mark the current start position of byte buffer in order to reset
        // later when there is not enough space left in buffer
        buffer.mark();
        try {
            // sms.writeRaw(buffer); Changed by Shaoying Han
            sms.writeRawWithVersion(buffer, Config.VERSION_CODE);
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
