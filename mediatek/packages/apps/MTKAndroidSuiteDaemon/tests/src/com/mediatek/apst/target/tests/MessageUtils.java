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

package com.mediatek.apst.target.tests;

import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.entity.message.Message;
import com.mediatek.apst.util.entity.message.Mms;
import com.mediatek.apst.util.entity.message.MmsPart;
import com.mediatek.apst.util.entity.message.Sms;
import com.mediatek.apst.util.entity.message.TargetAddress;

import java.nio.ByteBuffer;
import java.util.ArrayList;

public class MessageUtils {

    public static Sms getAsms() {
        Sms sms = new Sms();
        sms.setBody("a short message");
        sms.setBox(Message.BOX_INBOX);
        sms.setLocked(false);
        sms.setRead(false);
        sms.setDate_sent(0);
        sms.setDate(1266752658);
        sms.setSimId(1);
        TargetAddress target = new TargetAddress();
        target.setAddress("15965232514");
        target.setName("Lisi");
        sms.setTarget(target);
        return sms;
    }

    public static Mms getAmms() {
        Mms mms = new Mms();
        ArrayList<MmsPart> mmsParts = new ArrayList<MmsPart>();
        MmsPart mmsPart = new MmsPart();
        mmsPart.setCharset("mmsPart");
        mmsPart.setContentType(MmsPart.CT_TXT);
        mmsPart.setDataPath(null);
        mmsPart.setName("Mms");
        mmsPart.setText("It's a mmsPart");
        mmsParts.add(mmsPart);
        mms.setBox(Message.BOX_INBOX);
        mms.setContentType("2");
        mms.setDate(1266752658);
        mms.setLocked(false);
        mms.setSubject("subject");
        mms.setDate_sent(0);
        mms.setParts(mmsParts);
        mms.setId(Mms.ID_NULL);
        mms.setD_rpt("128");
        mms.setM_type("132");
        TargetAddress target = new TargetAddress();
        target.setAddress("15865232541");
        target.setName("Zhangsan");
        mms.setTarget(target);
        return mms;
    }

    public static byte[] getMmsRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        byte[] raw = null;
        Mms mms = getAmms();
        buffer.putInt(1);
        mms.writeAllWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        try {
            raw = buffer.array();
        } catch (UnsupportedOperationException e) {
        }
        return raw;
    }
    
    public static byte[] getSmsRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(Global.DEFAULT_BUFFER_SIZE);
        byte[] raw = null;
        Sms sms = getAsms();
        buffer.putInt(1);
        sms.writeRawWithVersion(buffer, Config.VERSION_CODE);
        buffer.position(0);
        try {
            raw = buffer.array();
        } catch (UnsupportedOperationException e) {
        }
        return raw;
    }

    public static byte[] SMS_RAW = { 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 44, -1, -1, -1, -1, -1, -1,
            -1, -1, 0, 0, 0, 17, 0, 49, 0, 56, 0, 50, 0, 50, 0, 55, 0, 54, 0, 52, 0, 48, 0, 51, 0, 54, 0, 53, 0, 44, 0, 49,
            0, 48, 0, 48, 0, 56, 0, 54, 0, 0, 0, 10, 0, 97, -128, 1, 81, 108, 0, 44, 0, 97, 0, 44, 0, 32, 0, 79, 0, 75, 0,
            44, 0, 0, 1, 57, -102, 101, 117, 89, 0, 0, 0, 3, 1, -1, -1, -1, -1, 0, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 5, 89, 125, 84, -81, 89, 125, 89, 125, 84, -81, 0, 0, 0, 0, 0, 0,
            0, 20, 0, 0, 0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 5, 110, 0, 0, 0, 5, 0, 49, 0, 48, 0, 48, 0, 56, 0, 54, 0, 0,
            0, 5, 0, 97, 0, 44, 0, 32, 0, 79, 0, 75, 0, 0, 1, 57, -103, 42, 12, -119, 0, 0, 0, 1, 1, -1, -1, -1, -1, 0, 0,
            0, 0, 3, 0, 0, 0, 7, 78, 45, 86, -3, 121, -5, 82, -88, 0, 32, 0, 48, 0, 49, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, 0, 0, 0, 81, 92, 10, 101, 108, 118, -124, 82, -88, 97, 31, 87, 48, 94, 38, 91, -94, 98, 55, 0, 58,
            98, 42, -127, -13, 0, 48, 0, 57, 103, 8, 0, 48, 0, 54, 101, -27, 0, 48, 0, 57, 101, -10, 0, 44, 96, -88, 118,
            -124, -117, -35, -115, 57, 79, 89, -104, -99, 78, 58, 0, 57, 0, 46, 0, 53, 0, 57, 81, 67, 0, 44, 78, 58, -112,
            127, 81, 77, 80, 92, 103, 58, 0, 44, -117, -9, 96, -88, 83, -54, 101, -10, 81, 69, 80, 60, 48, 2, 127, 81, 78,
            10, 127, 52, -115, 57, 102, -12, 101, -71, 79, -65, 48, 1, 102, -12, 79, 24, 96, -32, -1, 12, -117, -9, 117, 53,
            -127, 17, 118, 123, 95, 85, 0, 119, 0, 119, 0, 119, 0, 46, 0, 115, 0, 99, 0, 46, 0, 49, 0, 48, 0, 48, 0, 56, 0,
            54, 0, 46, 0, 99, 0, 110, 48, 2, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 41, -1, -1, -1, -1, -1, -1, -1,
            -1, 0, 0, 0, 5, 0, 49, 0, 48, 0, 48, 0, 49, 0, 48, -1, -1, -1, -1, 0, 0, 1, 57, -107, 28, 18, 124, 0, 0, 0, 1,
            1, -1, -1, -1, -1, 0, 0, 0, 0, 2, 0, 0, 0, 4, 78, 45, 86, -3, -128, 84, -112, 26, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, 0, 0, 0, 34, 92, 10, 101, 108, 118, -124, 117, 40, 98, 55, -1, 12, 96, -88, 107, 99, 87, 40,
            78, 45, 86, -3, -128, 84, -112, 26, 127, 81, 78, 10, -124, 37, 78, 26, 83, -123, 82, -98, 116, 6, 98, 75, 103,
            58, 78, 26, 82, -95, -1, 12, -106, -113, 103, 58, 91, -58, 120, 1, 78, 58, 0, 55, 0, 50, 0, 49, 0, 54 };

    public static byte[] MMS_RAW = { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 46, 0, 0, 0, 0, 0, 0, 0, 52, -1, -1, -1, -1, -1, -1,
            -1, -1, 0, 0, 0, 11, 0, 48, 0, 50, 0, 56, 0, 56, 0, 53, 0, 57, 0, 51, 0, 57, 0, 48, 0, 48, 0, 48, -1, -1, -1,
            -1, 0, 0, 1, 57, -102, 118, 73, -104, 0, 0, 0, 3, 1, 0, 0, 0, 9, 0, -27, 0, -109, 0, -120, 0, -27, 0, -109, 0,
            -120, 0, -27, 0, -109, 0, -120, 0, -1, -1, -1, -1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 37, 0,
            97, 0, 112, 0, 112, 0, 108, 0, 105, 0, 99, 0, 97, 0, 116, 0, 105, 0, 111, 0, 110, 0, 47, 0, 118, 0, 110, 0, 100,
            0, 46, 0, 119, 0, 97, 0, 112, 0, 46, 0, 109, 0, 117, 0, 108, 0, 116, 0, 105, 0, 112, 0, 97, 0, 114, 0, 116, 0,
            46, 0, 114, 0, 101, 0, 108, 0, 97, 0, 116, 0, 101, 0, 100, -1, -1, -1, -1, 0, 0, 0, 3, 0, 49, 0, 48, 0, 54, -1,
            -1, -1, -1, 0, 0, 0, 3, 0, 49, 0, 50, 0, 56, 0, 0, 0, 2, 0, 49, 0, 56, -1, -1, -1, -1, 0, 0, 0, 12, 0, 84, 0,
            49, 0, 51, 0, 57, 0, 57, 0, 97, 0, 55, 0, 54, 0, 52, 0, 98, 0, 101, 0, 97, -1, -1, -1, -1, 0, 0, 0, 1, 0, 48, 0,
            0, 0, 2, 0, 0, 0, 0, 0, 0, 0, -79, 0, 0, 0, 0, 0, 0, 0, 46, -1, -1, -1, -1, 0, 0, 0, 16, 0, 97, 0, 112, 0, 112,
            0, 108, 0, 105, 0, 99, 0, 97, 0, 116, 0, 105, 0, 111, 0, 110, 0, 47, 0, 115, 0, 109, 0, 105, 0, 108, -1, -1, -1,
            -1, -1, -1, -1, -1, 0, 0, 0, 6, 0, 60, 0, 115, 0, 109, 0, 105, 0, 108, 0, 62, 0, 0, 0, 8, 0, 115, 0, 109, 0,
            105, 0, 108, 0, 46, 0, 120, 0, 109, 0, 108, -1, -1, -1, -1, 0, 0, 0, -18, 0, 60, 0, 115, 0, 109, 0, 105, 0, 108,
            0, 62, 0, 60, 0, 104, 0, 101, 0, 97, 0, 100, 0, 62, 0, 60, 0, 108, 0, 97, 0, 121, 0, 111, 0, 117, 0, 116, 0, 62,
            0, 60, 0, 114, 0, 111, 0, 111, 0, 116, 0, 45, 0, 108, 0, 97, 0, 121, 0, 111, 0, 117, 0, 116, 0, 32, 0, 119, 0,
            105, 0, 100, 0, 116, 0, 104, 0, 61, 0, 34, 0, 55, 0, 54, 0, 50, 0, 112, 0, 120, 0, 34, 0, 32, 0, 104, 0, 101, 0,
            105, 0, 103, 0, 104, 0, 116, 0, 61, 0, 34, 0, 52, 0, 56, 0, 48, 0, 112, 0, 120, 0, 34, 0, 47, 0, 62, 0, 60, 0,
            114, 0, 101, 0, 103, 0, 105, 0, 111, 0, 110, 0, 32, 0, 105, 0, 100, 0, 61, 0, 34, 0, 84, 0, 101, 0, 120, 0, 116,
            0, 34, 0, 32, 0, 108, 0, 101, 0, 102, 0, 116, 0, 61, 0, 34, 0, 48, 0, 34, 0, 32, 0, 116, 0, 111, 0, 112, 0, 61,
            0, 34, 0, 52, 0, 51, 0, 50, 0, 34, 0, 32, 0, 119, 0, 105, 0, 100, 0, 116, 0, 104, 0, 61, 0, 34, 0, 55, 0, 54, 0,
            50, 0, 112, 0, 120, 0, 34, 0, 32, 0, 104, 0, 101, 0, 105, 0, 103, 0, 104, 0, 116, 0, 61, 0, 34, 0, 52, 0, 56, 0,
            112, 0, 120, 0, 34, 0, 32, 0, 102, 0, 105, 0, 116, 0, 61, 0, 34, 0, 109, 0, 101, 0, 101, 0, 116, 0, 34, 0, 47,
            0, 62, 0, 60, 0, 47, 0, 108, 0, 97, 0, 121, 0, 111, 0, 117, 0, 116, 0, 62, 0, 60, 0, 47, 0, 104, 0, 101, 0, 97,
            0, 100, 0, 62, 0, 60, 0, 98, 0, 111, 0, 100, 0, 121, 0, 62, 0, 60, 0, 112, 0, 97, 0, 114, 0, 32, 0, 100, 0, 117,
            0, 114, 0, 61, 0, 34, 0, 53, 0, 48, 0, 48, 0, 48, 0, 109, 0, 115, 0, 34, 0, 62, 0, 60, 0, 116, 0, 101, 0, 120,
            0, 116, 0, 32, 0, 115, 0, 114, 0, 99, 0, 61, 0, 34, 0, 116, 0, 101, 0, 120, 0, 116, 0, 95, 0, 48, 0, 46, 0, 116,
            0, 120, 0, 116, 0, 34, 0, 32, 0, 114, 0, 101, 0, 103, 0, 105, 0, 111, 0, 110, 0, 61, 0, 34, 0, 84, 0, 101, 0,
            120, 0, 116, 0, 34, 0, 47, 0, 62, 0, 60, 0, 47, 0, 112, 0, 97, 0, 114, 0, 62, 0, 60, 0, 47, 0, 98, 0, 111, 0,
            100, 0, 121, 0, 62, 0, 60, 0, 47, 0, 115, 0, 109, 0, 105, 0, 108, 0, 62, 0, 0, 0, 0, 0, 0, 0, -78, 0, 0, 0, 0,
            0, 0, 0, 46, 0, 0, 0, 0, 0, 0, 0, 10, 0, 116, 0, 101, 0, 120, 0, 116, 0, 47, 0, 112, 0, 108, 0, 97, 0, 105, 0,
            110, -1, -1, -1, -1, 0, 0, 0, 3, 0, 49, 0, 48, 0, 54, 0, 0, 0, 8, 0, 60, 0, 116, 0, 101, 0, 120, 0, 116, 0, 95,
            0, 48, 0, 62, 0, 0, 0, 10, 0, 116, 0, 101, 0, 120, 0, 116, 0, 95, 0, 48, 0, 46, 0, 116, 0, 120, 0, 116, -1, -1,
            -1, -1, 0, 0, 0, 20, 89, -45, 84, 13, 0, 58, 0, 32, 0, 97, 0, 44, 0, 32, 0, 79, 0, 75, 0, 10, 117, 53, -117,
            -35, 0, 58, 0, 32, 0, 49, 0, 48, 0, 48, 0, 56, 0, 54, 0, 10 };

}
