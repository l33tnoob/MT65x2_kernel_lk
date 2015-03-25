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

package com.mediatek.engineermode.tests;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.Context;
import android.content.Intent;
import android.test.ActivityInstrumentationTestCase2;
import android.test.SingleLaunchActivityTestCase;
import android.util.Log;
import android.widget.ListView;

import com.jayway.android.robotium.solo.Solo;
import com.mediatek.engineermode.memory.EmiRegister;
import com.mediatek.engineermode.memory.Memory;
import com.mediatek.engineermode.memory.NandFlash;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.File;

public class MemoryTest extends SingleLaunchActivityTestCase<Memory> {

    private static final String TAG = "EMTest/memory";
    private static final int MEMORY_ITEM_COUNT = 2;
    private static final String EMMC_PROC_FILE = "/proc/emmc";
    private static final String EMMC_ID_HEADER = "emmc ID:";
    private static final String EMMC_PARTITION = "Part_Name";
    private static final String FLASH_FS_TAG = "rootfs";
    private static final String FLASH_PART_EMMC = "emmc_p";
    private static final String FLASH_PART_MTD = "mtd";
    private static final int SLEEP_TIME = 1000;
    private static Solo sSolo = null;
    private static Activity sActivity = null;
    private static Context sContext = null;
    private static Instrumentation sInst = null;
    private static ListView sListView = null;
    private static boolean sHasEmmc = true;
    private static boolean sFinished = false;

    public MemoryTest() {
        super("com.mediatek.engineermode", Memory.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (null == sInst) {
            sInst = getInstrumentation();
        }
        if (null == sContext) {
            sContext = sInst.getTargetContext();
        }
        if (null == sActivity) {
            sActivity = getActivity();
            if (sActivity.getClass() != Memory.class) {
                sActivity.finish();
                sActivity = launchActivity("com.mediatek.engineermode",
                        Memory.class, null);
            }
        }
        if (null == sSolo) {
            sSolo = new Solo(sInst, sActivity);
        }
        sHasEmmc = new File(EMMC_PROC_FILE).exists();
    }

    @Override
    protected void tearDown() throws Exception {
        if (sFinished) {
            sSolo.finishOpenedActivities();
        }
        super.tearDown();
    }

    public void test01_Precondition() {
        assertNotNull(sInst);
        assertNotNull(sContext);
        assertNotNull(sActivity);
        assertNotNull(sSolo);
        if (null == sListView) {
            sListView = (ListView) sActivity
                    .findViewById(R.id.list_memory_item);
        }
        assertNotNull(sListView);
        sSolo.sleep(EmOperate.TIME_LONG);
    }

    public void test02_ItemCount() {
        int expected = MEMORY_ITEM_COUNT;
        int actual = sListView.getAdapter().getCount();
        sSolo.sleep(EmOperate.TIME_LONG);
        assertEquals(expected, actual);
    }

    public void test03_Flashidenty() {
        String actual = sListView.getAdapter().getItem(0).toString();
        if (sHasEmmc) {
            assertEquals(sActivity.getString(R.string.memory_item_emmc), actual);
        } else {
            assertEquals(sActivity.getString(R.string.memory_item_nand), actual);
        }
        sSolo.sleep(SLEEP_TIME);
    }

    public void test04_FlashActivity() {
        sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
        sSolo.sleep(SLEEP_TIME);
        Activity activity = sSolo.getCurrentActivity();
        boolean actual = (activity instanceof NandFlash);
        assertEquals(true, actual);
        sSolo.sleep(SLEEP_TIME);
        sSolo.goBack();
        sSolo.sleep(SLEEP_TIME);
    }

    public void test05_CommonInfo() {
        sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
        Activity activity = sSolo.getCurrentActivity();
        if (sHasEmmc) {
            assertEquals(true, sSolo.searchText(EMMC_ID_HEADER)
                    && sSolo.searchText(EMMC_PARTITION));
        } else {
            assertEquals(true, sSolo.searchText("ID")
                    && sSolo.searchText("total size"));
        }
        sSolo.goBack();
        sSolo.sleep(SLEEP_TIME);
        Xlog.d(TAG, "[FOR_NATA_MEMORY_COMMON_INFO_PASS]");
    }

    public void test06_FsInfo() {
        sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
        Activity activity = sSolo.getCurrentActivity();
        sSolo.clickOnText(activity.getString(R.string.memory_file_sys_info));
        assertEquals(true, sSolo.searchText(FLASH_FS_TAG));
        sSolo.goBack();
        sSolo.sleep(SLEEP_TIME);
        Xlog.d(TAG, "[FOR_NATA_MEMORY_FS_INFO_PASS]");
    }

    public void test07_PartInfo() {
        sSolo.clickOnText(sListView.getAdapter().getItem(0).toString());
        Activity activity = sSolo.getCurrentActivity();
        sSolo.clickOnText(activity.getString(R.string.memory_partition_info));
        if (sHasEmmc) {
            assertEquals(true, sSolo.searchText(FLASH_PART_EMMC));
        } else {
            assertEquals(true, sSolo.searchText(FLASH_PART_MTD));
        }
        sSolo.goBack();
        sSolo.sleep(SLEEP_TIME);
        Xlog.d(TAG, "[FOR_NATA_MEMORY_PATITION_INFO_PASS]");
    }
    
    public void test08_Help() {
        sSolo.clickOnText(sListView.getAdapter().getItem(1).toString());
        sSolo.sleep(EmOperate.TIME_LONG);
        sSolo.goBack();
        sSolo.sleep(SLEEP_TIME);
    }

    public void test09_Emi() {
        sFinished = true;
        Intent intent = new Intent(sActivity, EmiRegister.class);
        sActivity.startActivity(intent);
        sSolo.sleep(EmOperate.TIME_MID);
        sSolo.clickOnButton(0);
        sSolo.sleep(EmOperate.TIME_MID);
    }
}
