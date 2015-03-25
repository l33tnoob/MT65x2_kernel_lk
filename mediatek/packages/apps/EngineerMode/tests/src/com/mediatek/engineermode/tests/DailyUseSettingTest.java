/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.engineermode.tests;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;
import android.test.ServiceTestCase;

import com.mediatek.engineermode.DailyUseSettingService;


public class DailyUseSettingTest extends ServiceTestCase<DailyUseSettingService> {

    private static final String KEY_QUICK_BOOT = "quick_boot";
    private static final String KEY_IVSR = "ivsr";
    private static final String KEY_VM_LOG = "vm_log";
    private static final String VALUE_0 = "0";
    private static final String VALUE_1 = "1";
    private static final int IPO_ENABLE = 1;
    private static final int IPO_DISABLE = 0;
    
    private Context mContext;

    public DailyUseSettingTest() {
        super(DailyUseSettingService.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContext = getSystemContext();
    }

    @Override
    protected void tearDown() throws Exception {
        mContext = null;
        super.tearDown();
    }

    /**
     * Test whether the service start correctly.
     */
    public void test01_Start() {
        boolean ipoSetOk = true;
        boolean ivsrSetOk = true;
        boolean vmSetOk = false;
        ipoSetOk = Settings.System.getInt(
                mContext.getContentResolver(), Settings.System.IPO_SETTING, IPO_DISABLE) == IPO_ENABLE;
        ivsrSetOk = Settings.System.getLong(mContext.getContentResolver(),
                        Settings.System.IVSR_SETTING,
                        Settings.System.IVSR_SETTING_DISABLE) == Settings.System.IVSR_SETTING_ENABLE;
        Intent intent = new Intent();
        intent.setClass(mContext, DailyUseSettingService.class);
        intent.putExtra(KEY_QUICK_BOOT, !ipoSetOk);
        intent.putExtra(KEY_IVSR, !ivsrSetOk);
        intent.putExtra(KEY_VM_LOG, !vmSetOk);
        startService(intent);
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
        intent = new Intent();
        intent.setClass(mContext, DailyUseSettingService.class);
        intent.putExtra(KEY_QUICK_BOOT, ipoSetOk);
        intent.putExtra(KEY_IVSR, ivsrSetOk);
        intent.putExtra(KEY_VM_LOG, vmSetOk);
        startService(intent);
        EmOperate.waitSomeTime(EmOperate.TIME_MID);
    }
}
