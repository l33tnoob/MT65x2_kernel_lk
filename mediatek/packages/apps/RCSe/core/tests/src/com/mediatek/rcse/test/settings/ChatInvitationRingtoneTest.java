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

package com.mediatek.rcse.test.settings;

import android.content.Context;
import android.net.Uri;
import android.test.InstrumentationTestCase;
import android.text.TextUtils;

import com.orangelabs.rcs.provider.settings.RcsSettings;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.settings.ChatInvitationRingtone;
import com.mediatek.rcse.test.Utils;

/**
 * The class is used to test ChatInvitationRingtone
 */
public class ChatInvitationRingtoneTest extends InstrumentationTestCase {
    private static final String TAG = "ChatInvitationRingtoneTest";
    private static final String URI = "android.resource://com.mediatek.rcse/R.raw.ringtone";
    private Context mContext = null;
    private ChatInvitationRingtone mRingtone = null;

    @Override
    protected void setUp() throws Exception {
        Logger.d(TAG, "setUp() entry");
        super.setUp();
        mContext = getInstrumentation().getTargetContext();
        mRingtone = new ChatInvitationRingtone(mContext, null);
        RcsSettings.getInstance().setChatInvitationRingtone(URI);
        Logger.d(TAG, "setUp() entry");
    }

    /**
     * Test case for onRestoreRingtone()
     */
    public void testCase1_OnRestoreRingtone() {
        Logger.d(TAG, "testCase1_OnRestoreRingtone() entry");
        RcsSettings.getInstance().setChatInvitationRingtone("");
        Uri retUri = mRingtone.onRestoreRingtone();
        assertNull(retUri);
        RcsSettings.getInstance().setChatInvitationRingtone(URI);
        retUri = mRingtone.onRestoreRingtone();
        assertNotNull(retUri);
        Logger.d(TAG, "testCase1_OnRestoreRingtone() exit");
    }

    /**
     * Test case for onSaveRingtone()
     */
    public void testCase2_OnSaveRingtone() {
        Logger.d(TAG, "testCase2_OnSaveRingtone() entry");
        mRingtone.onSaveRingtone(null);
        String retUri = RcsSettings.getInstance().getChatInvitationRingtone();
        assertTrue(TextUtils.isEmpty(retUri));
        mRingtone.onSaveRingtone(Uri.parse(URI));
        retUri = RcsSettings.getInstance().getChatInvitationRingtone();
        assertFalse(TextUtils.isEmpty(retUri));
        Logger.d(TAG, "testCase2_OnSaveRingtone() exit");
    }
}
