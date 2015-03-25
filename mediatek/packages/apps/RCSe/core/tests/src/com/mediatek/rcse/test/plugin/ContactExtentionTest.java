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

import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.test.AndroidTestCase;

import com.mediatek.rcse.api.Logger;

import com.mediatek.rcse.plugin.contacts.ContactExtention;
import com.mediatek.rcse.plugin.contacts.ContactExtention.Action;
import com.mediatek.rcse.service.PluginApiManager;

import com.orangelabs.rcs.R;

import java.lang.reflect.Field;

/**
 * Test case for ContactExtention.
 */
public class ContactExtentionTest extends AndroidTestCase {

    private final static String IM_ACTION = "com.mediatek.rcse.action.CHAT_INSTANCE_MESSAGE";
    private final static String FT_ACTION = "com.mediatek.rcse.action.CHAT_FILE_TRANSFER";
    private final static String PARTICIPANT_NAME = "VF Bruce";
    private final static String PARTICIPANT_NUMBER = "+34200000247";
    private final static String CONTACT_NAME = "rcs_display_name";
    private final static String CONTACT_NUMBER = "rcs_phone_number";
    private final static String FILE_PATH = "rcs_file_path";
    private final static String PROXY_ACTION = "com.mediatek.rcse.action.PROXY";
    private static final String FIELD_INSTANCE = "mInstance";
    private ContactExtention mContactExt = null;
    private Resources mResources = null;
    private final static int CONTACT_ID = 1;
    private static final int ACTION_COUNT = 2;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mResources = mContext.getResources();
        mContactExt = new ContactExtention(mContext);
        Field field = ContactExtention.class.getDeclaredField(FIELD_INSTANCE);
        field.setAccessible(true);
        field.set(mContactExt, new MockPluginApiManager());
    }

    /**
     * Test the method of getAppTitle()
     */
    public void testCase1_getAppTitle() {
        String title = mContactExt.getAppTitle();
        String titleExpected = mResources.getString(R.string.joyn_title);
        assertEquals("app title is not as expected", titleExpected, title);
    }

    /**
     * Test the method of isEnabled()
     */
    public void testCase2_isEnabled() {
        boolean isEnable = mContactExt.isEnabled();
        assertEquals(isEnable, true);
    }

    /**
     * Test the method of getAppIcon()
     */
    public void testCase3_getAppIcon() {
        Drawable drawable = mContactExt.getAppIcon();
        assertNotNull(drawable);
    }

    /**
     * Test the method of getPresenceIcon()
     */
    public void testCase4_getPresenceIcon() {
        Drawable drawable = mContactExt.getContactPresence(CONTACT_ID);
        assertNotNull(drawable);
    }

    /**
     * Test the method of getContactActions()
     */
    public void testCase5_getContactActions() {
        Action[] actions = mContactExt.getContactActions();
        assertNotNull(actions);
        assertEquals(actions.length, ACTION_COUNT);
        Intent imIntent = actions[0].intentAction;
        Intent ftIntent = actions[1].intentAction;
        assertEquals(imIntent.getAction(), PROXY_ACTION);
        assertEquals(ftIntent.getAction(), PROXY_ACTION);
        assertEquals(imIntent.getBooleanExtra(IM_ACTION, false), true);
        assertEquals(ftIntent.getBooleanExtra(FT_ACTION, false), true);
    }

    private final class MockPluginApiManager extends PluginApiManager {
        private static final int RCS_PRESENCE = 1;

        public MockPluginApiManager() {
            super();
        }

        @Override
        public boolean getRegistrationStatus() {
            return true;
        }

        @Override
        public int getContactPresence(long contactId) {
            return RCS_PRESENCE;
        }
    }
}
