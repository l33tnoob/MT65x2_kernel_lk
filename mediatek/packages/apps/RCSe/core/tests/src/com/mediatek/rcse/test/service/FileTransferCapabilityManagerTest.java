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

package com.mediatek.rcse.test.service;

import android.content.Context;
import android.os.Environment;
import android.test.InstrumentationTestCase;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.FileTransferCapabilityManager;

import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.lang.InterruptedException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;


/**
 * This class is used to test FileTransferCapabilityManager.java
 */
public class FileTransferCapabilityManagerTest extends InstrumentationTestCase {

    private static final String TAG = "FileTransferCapabilityManagerTest";
    private static final int THREAD_SLEEP_PERIOD = 20;
    private static final long TIME_OUT = 1000;
    private Context mContext = null;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp()");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        ApiManager.initialize(mContext);
    }

    /**
     * Test case for isFileTransferCapabilitySupported()
     */
    public void testCase1_IsFileTransferCapabilitySupported() {
        Logger.d(TAG, "testCase1_IsFileTransferCapabilitySupported() entry");
        boolean testResult = FileTransferCapabilityManager.isFileTransferCapabilitySupported();
        boolean expectResult = Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED);
        if (expectResult) {
            assertTrue(testResult);
        } else {
            assertFalse(testResult);
        }
        Logger.d(TAG, "testCase1_IsFileTransferCapabilitySupported() exit");
    }
    
    /**
     * Test case for setFileTransferCapability()
     * 
     * @throws InterruptedException
     */
    public void testCase2_SetFileTransferCapability() throws InterruptedException {
        Logger.d(TAG, "testCase2_SetFileTransferCapability() entry");
        waitToWriteDatabase(true);
        waitToWriteDatabase(false);
        Logger.d(TAG, "testCase2_SetFileTransferCapability() exit");
    }
    
    private void waitToWriteDatabase(boolean isFtCapabilityEnable) throws InterruptedException {
        FileTransferCapabilityManager.setFileTransferCapability(mContext, isFtCapabilityEnable);
        long startTime = System.currentTimeMillis();
        while (true) {
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail();
            } else {
                if (RcsSettings.getInstance().isFileTransferSupported() == isFtCapabilityEnable) {
                    break;
                } else {
                    Thread.sleep(THREAD_SLEEP_PERIOD);
                }
            }
        }
    }

    /**
     * Tetst constructor
     * @throws NoSuchMethodException
     * @throws InvocationTargetException 
     * @throws IllegalAccessException 
     * @throws InstantiationException 
     * @throws IllegalArgumentException 
     */
    public void testCase3_constructor() throws NoSuchMethodException,
            IllegalArgumentException, InstantiationException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase3_constructor() entry");
        FileTransferCapabilityManager fileTransferCapabilityManager = 
                new FileTransferCapabilityManager();
        assertNotNull(fileTransferCapabilityManager);
        Class<?>[] classes = fileTransferCapabilityManager.getClass()
                .getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> clazz : classes) {
            if ("SdcardHelper".equals(clazz.getSimpleName())) {
                ctr = clazz.getDeclaredConstructor();
                ctr.setAccessible(true);
            }
        }
        assertNotNull(ctr.newInstance());
    }
}
