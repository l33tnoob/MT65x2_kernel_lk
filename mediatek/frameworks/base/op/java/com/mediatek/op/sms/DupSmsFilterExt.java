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

package com.mediatek.op.sms;

import com.mediatek.common.sms.IDupSmsFilterExt;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import com.android.internal.telephony.PhoneConstants;

public class DupSmsFilterExt implements IDupSmsFilterExt {

    private static String TAG = "DupSmsFilterExt";

    protected static final String KEY_DUP_SMS_KEEP_PERIOD = "dev.dup_sms_keep_period";
    protected static final long DEFAULT_DUP_SMS_KEEP_PERIOD = 5 * 60 * 1000;
    protected static final int EVENT_CLEAR_SMS_LIST = 0x01;
    
    protected Context mContext = null;
    protected int mSimId = -1;

    protected HashMap<Long, byte[]> mSmsMap = null;

    public DupSmsFilterExt(Context context, int simId) {
        Xlog.d(TAG, "call constructor");
        if(context == null) {
            Xlog.d(TAG, "FAIL! context is null");
            return;
        }
        
        mContext = context;
        mSimId = simId;
        mSmsMap = new HashMap<Long, byte[]>();

    }

    public boolean containDupSms(byte[] pdu) {
        Xlog.d(TAG, "call containDupSms");

        /* Test SIM card should not use the duplicate mechanism */
        if (isTestIccCard()) {
            return false;
        }

        removeExpiredItem();
        Iterator<Map.Entry<Long, byte[]>> iter = mSmsMap.entrySet().iterator();
        while(iter.hasNext()) {
            Map.Entry<Long, byte[]> entry = (Map.Entry<Long, byte[]>)iter.next();
            if (isDupSms(pdu, entry.getValue())) {
                return true;
            }
        }
        synchronized(mSmsMap) {
            mSmsMap.put(System.currentTimeMillis(), pdu);
        }
        return false;
    }

    protected boolean isDupSms(byte[] newPdu, byte[] oldPdu) {
        Xlog.d(TAG, "call isDupSms");
        if (Arrays.equals(newPdu, oldPdu)) {
            Xlog.d(TAG, "find a duplicated sms");
            return true;
        }

        return false;
    }

    synchronized private void removeExpiredItem() {
        Xlog.d(TAG, "call removeExpiredItem");
        long delayedPeriod = SystemProperties.getLong(
                KEY_DUP_SMS_KEEP_PERIOD, 
                DEFAULT_DUP_SMS_KEEP_PERIOD);
        Iterator<Map.Entry<Long, byte[]>> iter = mSmsMap.entrySet().iterator();
        while(iter.hasNext()) {
            Map.Entry<Long, byte[]> entry = (Map.Entry<Long, byte[]>)iter.next();
            if (entry.getKey() < System.currentTimeMillis() - delayedPeriod) {
                iter.remove();
            }
        }
        Xlog.d(TAG, "mSmsMap has " + mSmsMap.size() + " items after removeExpiredItem");
    }

    private boolean isTestIccCard() {
        int ret = -1;
        if (mSimId == PhoneConstants.GEMINI_SIM_1) {
            ret = SystemProperties.getInt("gsm.sim.ril.testsim", -1);
        } else if (mSimId == PhoneConstants.GEMINI_SIM_2) {
            ret = SystemProperties.getInt("gsm.sim.ril.testsim.2", -1);
        } else if (mSimId == PhoneConstants.GEMINI_SIM_3) {
            ret = SystemProperties.getInt("gsm.sim.ril.testsim.3", -1);
        } else if (mSimId == PhoneConstants.GEMINI_SIM_4) {
            ret = SystemProperties.getInt("gsm.sim.ril.testsim.4", -1);
        }
        Xlog.d(TAG, "Sim id: " + mSimId + "isTestIccCard: " + ret);
        return (ret == 1);
    }
}