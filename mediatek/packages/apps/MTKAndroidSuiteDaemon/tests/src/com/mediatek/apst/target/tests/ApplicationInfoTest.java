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

import android.test.AndroidTestCase;

import com.mediatek.apst.util.entity.app.ApplicationInfo;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class ApplicationInfoTest extends AndroidTestCase {
    private ApplicationInfo mApplicationInfo;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mApplicationInfo = new ApplicationInfo();
    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    public void test01_getPackageName() {
        mApplicationInfo.setPackageName("com.mediatek.apst");
        assertEquals("com.mediatek.apst", mApplicationInfo.getPackageName());
    }

    public void test02_getType() {
        mApplicationInfo.setType(ApplicationInfo.TYPE_DOWNLOADED);
        assertEquals(ApplicationInfo.TYPE_DOWNLOADED, mApplicationInfo
                .getType());
    }

    public void test03_getLabel() {
        mApplicationInfo.setLabel("label");
        assertEquals("label", mApplicationInfo.getLabel());
    }

    public void test04_getDescription() {
        mApplicationInfo.setDescription("description");
        assertEquals("description", mApplicationInfo.getDescription());
    }

    public void test05_getVersionName() {
        mApplicationInfo.setVersionName("versionName");
        assertEquals("versionName", mApplicationInfo.getVersionName());
    }

    public void test06_getApkSize() {
        mApplicationInfo.setApkSize(20);
        assertEquals(20, mApplicationInfo.getApkSize());
    }

    public void test07_getRequestedPermissions() {
        String[] permissions = { "permission1", "permission2" };
        mApplicationInfo.setRequestedPermissions(permissions);
        String[] newPermissions = mApplicationInfo.getRequestedPermissions();
        assertTrue(Arrays.equals(permissions, newPermissions));
    }

    public void test08_getSdkVersion() {
        mApplicationInfo.setSdkVersion(8);
        assertEquals(8, mApplicationInfo.getSdkVersion());
    }

    public void test09_getUid() {
        mApplicationInfo.setUid(12);
        assertEquals(12, mApplicationInfo.getUid());
    }

    public void test10_getIconBytes() {
        byte[] iconBytes = new byte[10];
        mApplicationInfo.setIconBytes(iconBytes);
        assertSame(iconBytes, mApplicationInfo.getIconBytes());
    }

    public void test11_getSourceDirectory() {
        mApplicationInfo.setSourceDirectory("sourceDirectory");
        assertEquals("sourceDirectory", mApplicationInfo.getSourceDirectory());
    }

    public void test12_getDataDirectory() {
        mApplicationInfo.setDataDirectory("dataDirectory");
        assertEquals("dataDirectory", mApplicationInfo.getDataDirectory());
    }

    public void test13_writeRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(500);
        String[] permissions = { "permission1", "permission2" };
        byte[] iconBytes = { 0x01, 0x02, 0x03 };
        ApplicationInfo appInfo = new ApplicationInfo();
        appInfo.setApkSize(10);
        appInfo.setDataDirectory("dataDirectory");
        appInfo.setDescription("description");
        appInfo.setIconBytes(iconBytes);
        appInfo.setLabel("label");
        appInfo.setPackageName("com.mediatek.apst");
        appInfo.setRequestedPermissions(permissions);
        appInfo.setSdkVersion(9);
        appInfo.setSourceDirectory("sourceDirectory");
        appInfo.setType(ApplicationInfo.TYPE_SYSTEM);
        appInfo.setUid(10);
        appInfo.setVersionName("2.0");
        appInfo.writeRaw(buffer);
        buffer.position(0);
        mApplicationInfo.readRaw(buffer);
        assertEquals(10, mApplicationInfo.getApkSize());
        assertEquals("dataDirectory", mApplicationInfo.getDataDirectory());
        assertEquals("description", mApplicationInfo.getDescription());
        assertTrue(Arrays.equals(iconBytes, mApplicationInfo.getIconBytes()));
        assertEquals("label", mApplicationInfo.getLabel());
        assertEquals("com.mediatek.apst", mApplicationInfo.getPackageName());
        assertTrue(Arrays.equals(permissions, mApplicationInfo
                .getRequestedPermissions()));
        assertEquals(9, mApplicationInfo.getSdkVersion());
        assertEquals("sourceDirectory", mApplicationInfo.getSourceDirectory());
        assertEquals(ApplicationInfo.TYPE_SYSTEM, mApplicationInfo.getType());
        assertEquals(10, mApplicationInfo.getUid());
        assertEquals("2.0", mApplicationInfo.getVersionName());
    }
}
