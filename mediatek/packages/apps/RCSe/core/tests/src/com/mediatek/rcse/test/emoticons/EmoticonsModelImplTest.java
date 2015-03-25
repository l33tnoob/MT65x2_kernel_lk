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

package com.mediatek.rcse.test.emoticons;

import android.content.res.TypedArray;
import android.test.AndroidTestCase;
import android.text.SpannableStringBuilder;

import com.mediatek.rcse.emoticons.EmoticonsModelImpl;

import com.orangelabs.rcs.R;

import java.util.ArrayList;

/**
 * Defined to test the function of EmoticonsModelImpl
 */
public class EmoticonsModelImplTest extends AndroidTestCase {
    private static final int INVALID_EMOTION_POSITION = -1;
    private static final int VALID_EMOTION_POSITION = 0;
    private static final String VALID_EMOTION_TEXT = ":-)";
    private static final String TEXT = "abc- -cde";

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        EmoticonsModelImpl.init(mContext);
    }

    /**
     * Test the getInstance() method
     */
    public void testCase1_getInstance() {
        assertNotNull(EmoticonsModelImpl.getInstance());
    }

    /**
     * Test case for the function that used to format a message containing icon
     * text to a spannable string.
     */
    public void testCase2_FormatMessage() {
        Object message = EmoticonsModelImpl.getInstance().formatMessage(TEXT);
        assertTrue(message instanceof SpannableStringBuilder);
        assertEquals(message.toString(), TEXT);
    }

    /**
     * Test the getEmotionCode() method with invalid position.
     */
    public void testCase3_getEmotionCode_invalidPosition() {
        assertNull(EmoticonsModelImpl.getInstance().getEmotionCode(INVALID_EMOTION_POSITION));
    }

    /**
     * Test the getEmotionCode() method with valid position.
     */
    public void testCase4_getEmotionCode_validPosition() {
        assertEquals(EmoticonsModelImpl.getInstance().getEmotionCode(VALID_EMOTION_POSITION),
                VALID_EMOTION_TEXT);
    }

    /**
     * Test the getResourceIdArray() method.
     */
    public void testCase5_getResourceIdArray() {
        String[] emotionCodes = mContext.getResources().getStringArray(R.array.emotion_codes);
        TypedArray resIdArray = mContext.getResources().obtainTypedArray(R.array.emotion_resid);
        assertNotNull(emotionCodes);
        assertNotNull(resIdArray);
        assertEquals(emotionCodes.length, resIdArray.length());
        ArrayList<Integer> resIds = EmoticonsModelImpl.getInstance().getResourceIdArray();
        int resIdArrsySize = resIdArray.length();
        assertEquals(resIds.size(), resIdArrsySize);
        for (int i = 0; i < resIdArrsySize; i++) {
            assertTrue(resIds.contains(resIdArray.getResourceId(i, 0)));
        }
    }
}
