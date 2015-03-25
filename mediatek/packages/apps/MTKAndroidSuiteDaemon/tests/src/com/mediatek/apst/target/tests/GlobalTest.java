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

import android.content.Context;
import android.test.AndroidTestCase;

import com.mediatek.apst.target.data.proxy.sysinfo.SystemInfoProxy;
import com.mediatek.apst.target.util.Config;
import com.mediatek.apst.target.util.Global;
import com.mediatek.apst.util.command.sysinfo.SimDetailInfo;
import com.mediatek.apst.util.entity.contacts.RawContact;

import java.nio.ByteBuffer;
import java.util.List;

public class GlobalTest extends AndroidTestCase {

    Context mContext;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getContext();
        Global.sContext = mContext;

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test01_getByteBuffer() {
        ByteBuffer buffer = Global.getByteBuffer();
        assertNotNull(buffer);
    }

    public void test02_getSimName() {
        String result = null;
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = Global.getSimName(Global.getSimIdBySlot(0));
                assertTrue(result.length() > 0);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                result = Global.getSimName(Global.getSimIdBySlot(1));
                assertTrue(result.length() > 0);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = Global.getSimName(Global.getSimIdBySlot(0));
                assertTrue(result.length() > 0);
            }
        }

        Global.sContext = null;
        result = Global.getSimName(1);
        assertTrue(result.equals(""));
    }

    public void test03_getSourceLocationById() {
        int result;
        result = Global.getSourceLocationById(RawContact.SOURCE_NONE);
        assertTrue(result == RawContact.SOURCE_NONE);
        result = Global.getSourceLocationById(RawContact.SOURCE_PHONE);
        assertTrue(result == RawContact.SOURCE_PHONE);
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = Global.getSourceLocationById(Global.getSimIdBySlot(0));
                assertTrue(result == RawContact.SOURCE_SIM1);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                result = Global.getSourceLocationById(Global.getSimIdBySlot(1));
                assertTrue(result == RawContact.SOURCE_SIM2);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = Global.getSourceLocationById(Global.getSimIdBySlot(0));
                assertTrue(result == RawContact.SOURCE_SIM);
            }
        }

        Global.sContext = null;
        result = Global.getSourceLocationById(1);
        assertTrue(result == RawContact.SOURCE_NONE);
    }

    public void test04_getSimInfoById() {
        SimDetailInfo result = null;
        result = Global.getSimInfoById(RawContact.SOURCE_NONE);
        assertNotNull(result);
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = Global.getSimInfoById(Global.getSimIdBySlot(0));
                assertNotNull(result);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                result = Global.getSimInfoById(Global.getSimIdBySlot(1));
                assertNotNull(result);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = Global.getSimInfoById(Global.getSimIdBySlot(0));
                assertNotNull(result);
            }
        }

        Global.sContext = null;
        result = Global.getSimInfoById(1);
    }

    public void test05_getSimInfoBySlot() {

        SimDetailInfo result = null;
        result = Global.getSimInfoBySlot(RawContact.SOURCE_NONE);
        assertNotNull(result);
        if (Config.MTK_GEMINI_SUPPORT) {
            if (SystemInfoProxy.isSim1Accessible()) {
                result = Global.getSimInfoBySlot(0);
                assertNotNull(result);
            }
            if (SystemInfoProxy.isSim2Accessible()) {
                result = Global.getSimInfoBySlot(1);
                assertNotNull(result);
            }
        } else {
            if (SystemInfoProxy.getInstance(mContext).isSimAccessible()) {
                result = Global.getSimInfoBySlot(0);
                assertNotNull(result);
            }
        }

        Global.sContext = null;
        result = Global.getSimInfoBySlot(1);
        assertNotNull(result);
    }

    public void test06_getAllSIMList() {
        List<SimDetailInfo> result = null;
        result = Global.getAllSIMList();
        assertNotNull(result);
        Global.sContext = null;
        result = Global.getAllSIMList();
        assertNull(result);
    }
}
