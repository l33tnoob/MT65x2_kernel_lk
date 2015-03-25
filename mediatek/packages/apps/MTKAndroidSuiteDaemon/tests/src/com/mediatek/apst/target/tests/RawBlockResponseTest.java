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

import com.mediatek.apst.util.command.BaseCommand;
import com.mediatek.apst.util.command.RawBlockResponse;
import com.mediatek.apst.util.command.RawBlockResponse.Builder;

public class RawBlockResponseTest extends AndroidTestCase {
    private RawBlockResponse mRawBkResponse;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mRawBkResponse = new RawBlockResponse(BaseCommand.FEATURE_APPLICATION,
                1);

    }

    @Override
    protected void tearDown() throws Exception {

        super.tearDown();
    }

    /**
     * The raw is null.
     */
    public void test01_getRaw() {
        mRawBkResponse.setRaw(null);
        byte[] raw = mRawBkResponse.getRaw();
        assertEquals(4, raw.length);
    }

    /**
     * The raw is not null.
     */
    public void test02_getRaw() {
        byte[] raw1 = { 1, 2, 3, 4, 5, 6 };
        mRawBkResponse.setRaw(raw1);
        byte[] raw2 = mRawBkResponse.getRaw();
        assertEquals(raw1.length, raw2.length);
    }

    public void test03_getProgress() {
        mRawBkResponse.setProgress(100);
        assertEquals(100, mRawBkResponse.getProgress());
    }

    public void test04_getTotal() {
        mRawBkResponse.setTotal(123);
        assertEquals(123, mRawBkResponse.getTotal());
    }

    /**
     * Test builder(int featureId).
     */
    public void test03_builder() {
        Builder builder = RawBlockResponse
                .builder(BaseCommand.FEATURE_CONTACTS);
        assertNotNull(builder);
    }

    /**
     * Test builder(int rawBlockSize, int featureId). The rawBlockSize > 0.
     */
    public void test04_builder() {
        Builder builder = RawBlockResponse.builder(10,
                BaseCommand.FEATURE_CONTACTS);
        assertNotNull(builder);
    }

    /**
     * Test builder(int rawBlockSize, int featureId). The rawBlockSize < 0.
     */
    public void test05_builder() {
        Builder builder = RawBlockResponse.builder(-1,
                BaseCommand.FEATURE_CONTACTS);
        assertNotNull(builder);
    }

}
