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

package com.mediatek.rcse.test.plugin;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import android.test.AndroidTestCase;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.mediatek.rcse.plugin.contacts.CallListExtensionForRCS;
import com.mediatek.rcse.test.Utils;

import com.mediatek.rcse.api.Logger;

/**
 * Test case for CallListExtensionForRCS.
 */
public class CallListExtensionForRCSTest extends AndroidTestCase {
    private static final String TAG = "CallListExtensionForRCSTest";
    private CallListExtensionForRCS mCallListExtensionForRCS = null;

    // private CallLogExtention mCallLogPlugin = null;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCallListExtensionForRCS = new CallListExtensionForRCS(mContext);
    }

    /**
     * Test layoutExtentionIcon
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase01_layoutExtentionIcon() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase01_layoutExtentionIcon entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setVisibility(View.VISIBLE);
        Field fieldmRCSIconViewWidth = Utils.getPrivateField(mCallListExtensionForRCS.getClass(),
                "mRCSIconViewWidth");
        int RCSIconViewWidth = fieldmRCSIconViewWidth.getInt(mCallListExtensionForRCS);
        assertEquals(20 - (1 + RCSIconViewWidth), mCallListExtensionForRCS.layoutExtentionIcon(1,
                1, 20, 20, 1, imageView, CallListExtensionForRCS.COMMD_FOR_RCS));
    }

    /**
     * Test isVisible
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase02_isVisible() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        Logger.d(TAG, "testCase02_isVisible entry");
        View view = new TextView(mContext);
        view.setVisibility(View.VISIBLE);
        Method methodIsVisible = Utils.getPrivateMethod(mCallListExtensionForRCS.getClass(),
                "isVisible", View.class);
        assertEquals(true, methodIsVisible.invoke(mCallListExtensionForRCS, view));
        view = null;

    }

    /**
     * Test measureExtention
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_measureExtention() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase03_measureExtention entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setVisibility(View.VISIBLE);
        Field fieldmRCSIconViewWidthAndHeightAreReady = Utils.getPrivateField(
                mCallListExtensionForRCS.getClass(), "mRCSIconViewWidthAndHeightAreReady");
        fieldmRCSIconViewWidthAndHeightAreReady.set(mCallListExtensionForRCS, false);
        mCallListExtensionForRCS.measureExtention(imageView, CallListExtensionForRCS.COMMD_FOR_RCS);
        assertTrue(fieldmRCSIconViewWidthAndHeightAreReady.getBoolean(mCallListExtensionForRCS));

    }

    /**
     * Test setExtentionIcon
     */
    public void testCase04_setExtentionIcon() {
        Logger.d(TAG, "testCase04_setExtentionIcon entry");
        assertEquals(false, mCallListExtensionForRCS.setExtentionIcon(null, null));
        assertEquals(false, mCallListExtensionForRCS.setExtentionIcon("+34200000246", null));
    }

    /**
     * Test setExtentionImageView
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase05_setExtentionImageView() throws NoSuchFieldException,
            IllegalArgumentException, IllegalAccessException {
        Logger.d(TAG, "testCase05_setExtentionImageView entry");
        ImageView imageView = new ImageView(mContext);
        imageView.setImageDrawable(null);
        mCallListExtensionForRCS.setExtentionImageView(imageView,
                CallListExtensionForRCS.COMMD_FOR_RCS + "00");
        assertNull(imageView.getDrawable());

        mCallListExtensionForRCS.setExtentionImageView(imageView,
                CallListExtensionForRCS.COMMD_FOR_RCS);
        Field fieldmExtenstionIcon = Utils.getPrivateField(mCallListExtensionForRCS.getClass(),
                "mExtenstionIcon");
        assertEquals(imageView.getDrawable(), fieldmExtenstionIcon.get(mCallListExtensionForRCS));

    }

    /**
     * Test checkPluginSupport
     */
    public void testCase06_checkPluginSupport() {
        Logger.d(TAG, "checkPluginSupport entry");
        assertTrue(mCallListExtensionForRCS
                .checkPluginSupport(CallListExtensionForRCS.COMMD_FOR_RCS));
        assertFalse(mCallListExtensionForRCS
                .checkPluginSupport(CallListExtensionForRCS.COMMD_FOR_RCS + "000"));
    }
}
