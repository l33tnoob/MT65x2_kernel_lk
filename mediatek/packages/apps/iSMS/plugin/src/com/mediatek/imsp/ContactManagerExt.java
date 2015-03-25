/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.imsp;

import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.hissage.contact.NmsContact;
import com.hissage.api.NmsIpMessageApi;
import com.mediatek.encapsulation.MmsLog;
import com.mediatek.mms.ipmessage.ContactManager;
import com.mediatek.xlog.Xlog;

/**
 * Provide contact management related interface
 */
public class ContactManagerExt extends ContactManager {
    private static final String TAG = "imsp/ContactManagerExt";

    public ContactManagerExt(Context context) {
        super(context);
    }

    public String getNumberByEngineId(short engineId) {
        Xlog.d(TAG, "getNumberByEngineId, engine id = " + engineId);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaEngineId(
                engineId);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getNumber();
    }

    public String getNumberByMessageId(long messageId) {
        Xlog.d(TAG, "getNumberByMessageId, message id = " + messageId);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaMsgId(
                messageId);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getNumber();
    }

    public String getNumberByThreadId(long threadId) {
        Xlog.d(TAG, "getNumberByThreadId, thread id = " + threadId);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaThreadId(
                threadId);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getNumber();
    }

    public short getContactIdByNumber(String number) {
        Xlog.d(TAG, "getContactIdByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return -1;
        }
        return nmsContact.getId();
    }

    public int getTypeByNumber(String number) {
        Xlog.d(TAG, "getTypeByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return -1;
        }
        return nmsContact.getType();
    }

    public int getStatusByNumber(String number) {
        Xlog.d(TAG, "getStatusByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return -1;
        }
        return nmsContact.getStatus();
    }

    public int getOnlineTimeByNumber(String number) {
        Xlog.d(TAG, "getOnlineTimeByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return -1;
        }
        return nmsContact.getOnlineTime();
    }

    public String getNameByNumber(String number) {
        Xlog.d(TAG, "getNameByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getName();
    }

    public String getNameByThreadId(long threadId) {
        Xlog.d(TAG, "getNameByThreadId, thread id = " + threadId);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaThreadId(
                threadId);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getName();
    }

    public String getSignatureByNumber(String number) {
        Xlog.d(TAG, "getSignatureByNumber, number = " + number);
        NmsContact nmsContact = NmsIpMessageApi.getInstance(mContext).nmsGetContactInfoViaNumber(
                number);
        if (nmsContact == null) {
            Xlog.e(TAG, "isms interface return null");
            return null;
        }
        return nmsContact.getSignature();
    }

    public Bitmap getAvatarByNumber(String number) {
        Xlog.d(TAG, "getAvatarByNumber, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsGetContactAvatarViaNumber(number);
    }

    public Bitmap getAvatarByThreadId(long threadId) {
        Xlog.d(TAG, "getAvatarByThreadId, thread id = " + threadId);
        return NmsIpMessageApi.getInstance(mContext).nmsGetContactAvatarViaThreadId(threadId);
    }

    public boolean isIpMessageNumber(String number) {
        Xlog.d(TAG, "isIpMessageNumber, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsIsiSMSNumber(number);
    }

    public boolean addContactToSpamList(int[] contactIds) {
        Xlog.d(TAG, "addContactToSpamList contactIds = " + contactIds);
        return NmsIpMessageApi.getInstance(mContext).nmsAddContactToSpamList(contactIds);
    }

    public boolean deleteContactFromSpamList(int[] contactIds) {
        Xlog.d(TAG, "deleteContactFromSpamList contactIds = " + contactIds);
        return NmsIpMessageApi.getInstance(mContext).nmsDeleteContactFromSpamList(contactIds);
    }

    public boolean isIpMessageSrvNumber(String number) {
        Xlog.d(TAG, "isIpMessageSrvNumber, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsIsIpMessageSrvNumber(number);
    }

    public String getIpMessageSrvNumberName(String number) {
        Xlog.d(TAG, "getIpMessageSrvNumberName, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsGetIpMessageSrvNumberName(number);
    }

    public boolean isIpMsgSendable(String number) {
        Xlog.d(TAG, "isIpMsgSendable call, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsIsIpMsgSendable(number);
    }

    public String getUnifyPhoneNumber(String number) {
        Xlog.d(TAG, "getUnifyPhoneNumber call, number = " + number);
        return NmsIpMessageApi.getInstance(mContext).nmsGetUnifyPhoneNumber(number);
    }
}
