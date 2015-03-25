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

import com.mediatek.apst.util.entity.contacts.Phone;

import java.nio.ByteBuffer;

public class PhoneTest extends AndroidTestCase {
    private Phone mPhone;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mPhone = new Phone();

    }

    @Override
    protected void tearDown() throws Exception {

        mPhone = null;
        super.tearDown();
    }

    public void test01_getNumber() {
        mPhone.setNumber("13952366452147");
        assertEquals("13952366452147", mPhone.getNumber());
    }

    public void test02_getType() {
        mPhone.setType(Phone.TYPE_HOME);
        assertEquals(Phone.TYPE_HOME, mPhone.getType());
    }

    public void test03_getLabel() {
        mPhone.setLabel("label");
        assertEquals("label", mPhone.getLabel());
    }

    public void test04_getBindingSimId() {
        mPhone.setBindingSimId(1);
        assertEquals(1, mPhone.getBindingSimId());
    }

    public void test05_getMimeTypeString() {
        assertEquals(Phone.MIME_TYPE_STRING, mPhone.getMimeTypeString());
    }

    /**
     * versionCode < 0x00000002.
     */
    public void test06_readRawWithVersion() {
        ByteBuffer buffer = ByteBuffer.allocate(100);
        int versionCode = 1;
        Phone phone = new Phone();
        phone.setBindingSimId(1);
        phone.setLabel("label");
        phone.setNumber("15987452536");
        phone.setType(Phone.TYPE_HOME);
        phone.writeRawWithVersion(buffer, versionCode);
        buffer.position(0);
        mPhone.readRawWithVersion(buffer, versionCode);
        assertFalse(mPhone.getBindingSimId() == 1);
        assertEquals("label", mPhone.getLabel());
        assertEquals("15987452536", mPhone.getNumber());
        assertEquals(Phone.TYPE_HOME, mPhone.getType());
    }

    /**
     * versionCode >= 0x00000002.
     */
    public void test07_readRawWithVersion() {
        ByteBuffer buffer = ByteBuffer.allocate(100);
        int versionCode = 2;
        Phone phone = new Phone();
        phone.setBindingSimId(1);
        phone.setLabel("label");
        phone.setNumber("15987452536");
        phone.setType(Phone.TYPE_HOME);
        phone.writeRawWithVersion(buffer, versionCode);
        buffer.position(0);
        mPhone.readRawWithVersion(buffer, versionCode);
        assertEquals(1, mPhone.getBindingSimId());
        assertEquals("label", mPhone.getLabel());
        assertEquals("15987452536", mPhone.getNumber());
        assertEquals(Phone.TYPE_HOME, mPhone.getType());
    }
}
