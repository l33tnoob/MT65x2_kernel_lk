/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.test.model;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.test.AndroidTestCase;

import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.service.ApiManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;

import java.lang.InterruptedException;
import java.lang.reflect.Method;

/**
 * The class defined to test the clear all history function in ModelImpl part
 */
public class ClearAllHistoryTest extends AndroidTestCase {
   
    private ContentResolver mContentResolver = null;
    private final static String NAME_A = "TesterA";
    private final static String NAME_B = "TesterB";
    private final static int LOOP_COUNT = 5;
    private final static int EXPECT_MESSAGE_COUNTS = 0;
    private final static int THREAD_SLEEP_DURATION = 5000;
    
    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.mContext);
        ApiManager apiManager = ApiManager.getInstance();
        assertNotNull(apiManager);
        Context context = apiManager.getContext();
        assertNotNull(context);
        mContentResolver = context.getContentResolver();
        assertNotNull(mContentResolver);
        // Insert some chat message
        for (int i = 0; i < LOOP_COUNT; i++) {
            ContentValues valuesA = new ContentValues();
            valuesA.put(RichMessagingData.KEY_NAME, NAME_A);
            valuesA.put(RichMessagingData.KEY_DATA, i);
            mContentResolver.insert(RichMessagingData.CONTENT_URI, valuesA);

            ContentValues valuesB = new ContentValues();
            valuesB.put(RichMessagingData.KEY_NAME, NAME_B);
            valuesB.put(RichMessagingData.KEY_DATA, i);
            mContentResolver.insert(RichMessagingData.CONTENT_URI, valuesB);
        }
    }

    /**
     * Used to test the clear all chat history function in the model part
     */
    public void testCase1_clearAllHistory() throws InterruptedException {
        assertTrue(((ModelImpl)ModelImpl.getInstance()).clearAllHistory());
        Thread.currentThread().sleep(THREAD_SLEEP_DURATION);
        Cursor cursor = mContentResolver.query(RichMessagingData.CONTENT_URI, null, null, null,
                null);
        if (null != cursor) {
            assertTrue(cursor.getCount() == EXPECT_MESSAGE_COUNTS);
            cursor.close();
            cursor = null;
        }
    }
}
