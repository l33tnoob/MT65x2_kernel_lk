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

import com.mediatek.apst.util.entity.contacts.Photo;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class PhotoTest extends AndroidTestCase {
    private Photo mPhoto;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mPhoto = new Photo();

    }

    @Override
    protected void tearDown() throws Exception {

        mPhoto = null;
        super.tearDown();
    }

    public void test01_getPhotoBytes() {
        byte[] photoBytes = { 0x01, 0x02, 0x03, 0x04, 0x05 };
        mPhoto.setPhotoBytes(photoBytes);
        assertTrue(Arrays.equals(photoBytes, mPhoto.getPhotoBytes()));
    }

    public void test02_getMimeTypeString() {
        assertEquals(Photo.MIME_TYPE_STRING, mPhoto.getMimeTypeString());
    }

    public void test03_clone() {
        try {
            mPhoto.clone();
        } catch (CloneNotSupportedException e) {
            fail(e.getMessage());
        }
    }

    public void test04_readRaw() {
        ByteBuffer buffer = ByteBuffer.allocate(50);
        byte[] photoBytes = { 0x01, 0x02, 0x03, 0x04, 0x05 };
        Photo photo = new Photo();
        photo.setPhotoBytes(photoBytes);
        photo.writeRaw(buffer);
        buffer.position(0);
        mPhoto.readRaw(buffer);
        assertTrue(Arrays.equals(photoBytes, mPhoto.getPhotoBytes()));
    }

}
