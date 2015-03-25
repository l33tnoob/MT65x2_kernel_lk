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

import com.mediatek.apst.util.entity.contacts.Im;

import java.nio.ByteBuffer;

public class ImTest extends AndroidTestCase {
    private Im mIm;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIm = new Im();

    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    public void test01_getData() {
        mIm.setData("123456");
        assertEquals("123456", mIm.getData());
    }

    public void test02_getType() {
        mIm.setType(Im.TYPE_HOME);
        assertEquals(Im.TYPE_HOME, mIm.getType());
    }

    public void test03_getLabel() {
        mIm.setLabel("QQ");
        assertEquals("QQ", mIm.getLabel());
    }

    public void test04_getProtocol() {
        mIm.setProtocol(Im.PROTOCOL_QQ);
        assertEquals(Im.PROTOCOL_QQ, mIm.getProtocol());
    }

    public void test05_getCustomProtocol() {
        mIm.setCustomProtocol("custom");
        assertEquals("custom", mIm.getCustomProtocol());
    }

    public void test06_getMimeTypeString() {
        assertEquals(Im.MIME_TYPE_STRING, mIm.getMimeTypeString());
    }

    public void test07_clone() {
        try {
            mIm.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test08_readRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(100);
        Im im = new Im();
        im.setCustomProtocol("custom");
        im.setData("123456");
        im.setLabel("label");
        im.setProtocol(Im.PROTOCOL_QQ);
        im.setType(Im.TYPE_WORK);
        im.writeRaw(buffer);
        buffer.position(0);
        mIm.readRaw(buffer);
        assertEquals("custom", mIm.getCustomProtocol());
        assertEquals("123456", mIm.getData());
        assertEquals("label", mIm.getLabel());
        assertEquals(Im.PROTOCOL_QQ, mIm.getProtocol());
        assertEquals(Im.TYPE_WORK, mIm.getType());

    }

}
