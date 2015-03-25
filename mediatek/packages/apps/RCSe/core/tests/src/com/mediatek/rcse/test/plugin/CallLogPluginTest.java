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

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.test.AndroidTestCase;
import android.util.LruCache;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.contacts.CallLogExtention;
import com.mediatek.rcse.plugin.contacts.CallLogExtention.Action;
import com.mediatek.rcse.plugin.contacts.CallLogExtention.OnPresenceChangedListener;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.test.Utils;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.lang.reflect.Field;
import java.util.HashMap;

/**
 * Test case for CallLogExtention.
 */
public class CallLogPluginTest extends AndroidTestCase {
    private static final String TAG = "CallLogPluginTest";
    private CallLogExtention mCallLogExt = null;
    private final static String NUMBER = "+34200000254";
    private static final String IM_CHAT = "IM";
    private static final String FIELD_INSTANCE = "mInstance";
    private static final int ACTION_COUNT = 2;
    private final static String IM_ACTION = "com.mediatek.rcse.action.CHAT_INSTANCE_MESSAGE";
    private final static String FT_ACTION = "com.mediatek.rcse.action.CHAT_FILE_TRANSFER";
    private final static String PROXY_ACTION = "com.mediatek.rcse.action.PROXY";
    private MockPluginApiManager mInstance;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mCallLogExt = new CallLogExtention(mContext);
        Field field = CallLogExtention.class.getDeclaredField(FIELD_INSTANCE);
        field.setAccessible(true);
        mInstance = new MockPluginApiManager();
        field.set(mCallLogExt, mInstance);
    }

    /**
     * Test the method of getAppTitle()
     */
    public void testCase01_getChatString() {
        String chat = mCallLogExt.getChatString();
        assertEquals(chat, IM_CHAT);
    }

    /**
     * Test the method of isEnabled()
     */
    public void testCase02_isEnabled() {
        boolean isEnable = mCallLogExt.isEnabled();
        assertEquals(isEnable, true);
    }

    /**
     * Test the method of getAppIcon()
     */
    public void testCase03_getAppIcon() {
        Drawable drawable = mCallLogExt.getAppIcon();
        assertNotNull(drawable);
    }

    /**
     * Test the method of getPresenceIcon()
     */
    public void testCase04_getPresenceIcon() {
        Logger.d(TAG, "testCase04_getPresenceIcon");
        Drawable drawable = mCallLogExt.getContactPresence(NUMBER);
        assertNotNull(drawable);
        assertNull( mCallLogExt.getContactPresence(null));
    }

    /**
     * Test the method of getContactActions()
     */
    @SuppressWarnings("unchecked")
    public void testCase5_getContactActions() throws NoSuchFieldException, IllegalAccessException {
        Field field = PluginApiManager.class.getDeclaredField("mContactsCache");
        assertTrue(field != null);
        field.setAccessible(true);
        LruCache<String, ContactInformation> contactCache = (LruCache<String, ContactInformation>) field
                .get(mInstance);
        ContactInformation info = new ContactInformation();
        info.isRcsContact = 1;
        info.isImSupported = true;
        info.isFtSupported = true;
        contactCache.put(NUMBER, info);
        Action[] actions = mCallLogExt.getContactActions(NUMBER);
        assertNotNull(actions);
        assertEquals(actions.length, ACTION_COUNT);
        Intent imIntent = actions[0].intentAction;
        Intent ftIntent = actions[1].intentAction;
        assertEquals(imIntent.getAction(), PROXY_ACTION);
        assertEquals(imIntent.getBooleanExtra(IM_ACTION, false), true);
        assertEquals(ftIntent.getAction(), PROXY_ACTION);
        assertEquals(ftIntent.getBooleanExtra(FT_ACTION, false), true);
    }

    /**
     * Test onReceive
     * 
     * @throws NoSuchFieldException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase06_onReceive() throws NoSuchFieldException, IllegalArgumentException,
            IllegalAccessException {
        Logger.d(TAG, "testCase06_onReceive");
        Field fieldBroadcastReceiver = Utils.getPrivateField(mCallLogExt.getClass(),
                "mBroadcastReceiver");
        BroadcastReceiver broadcastReceiver = (BroadcastReceiver) fieldBroadcastReceiver
                .get(mCallLogExt);
        Intent intent = new Intent();
        broadcastReceiver.onReceive(mContext, intent);
        intent.putExtra("contact", NUMBER);
        Capabilities capabilities = new Capabilities();
        capabilities.setRcseContact(true);
        intent.putExtra("capabilities", capabilities);
        getOnPresenceChangedListenerList().clear();
        broadcastReceiver.onReceive(mContext, intent);
        MockOnPresenceChangedListener listener = new MockOnPresenceChangedListener();
        getOnPresenceChangedListenerList().put(NUMBER, listener);
        broadcastReceiver.onReceive(mContext, intent);
        assertEquals(NUMBER, listener.mNumber);
        assertEquals(1, listener.mPresence);
        getOnPresenceChangedListenerList().clear();
    }

    /**
     * Test addOnPresenceChangedListener
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws NoSuchFieldException
     */
    public void testCase07_addOnPresenceChangedListener() throws IllegalArgumentException,
            IllegalAccessException, NoSuchFieldException {
        Logger.d(TAG, "testCase07_addOnPresenceChangedListener");
        MockOnPresenceChangedListener listener = new MockOnPresenceChangedListener();
        getOnPresenceChangedListenerList().clear();
        mCallLogExt.addOnPresenceChangedListener(listener, NUMBER);
        assertEquals(listener, getOnPresenceChangedListenerList().get(NUMBER));
    }

    @SuppressWarnings("unchecked")
    HashMap<String, Object> getOnPresenceChangedListenerList() throws IllegalArgumentException,
            IllegalAccessException, NoSuchFieldException {
        Field fieldnPresenceChangedListenerList = Utils.getPrivateField(mCallLogExt.getClass(),
                "mOnPresenceChangedListenerList");
        return (HashMap<String, Object>) fieldnPresenceChangedListenerList.get(mCallLogExt);
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
        public int getContactPresence(String number) {
            return RCS_PRESENCE;
        }
    }

    private class MockOnPresenceChangedListener implements OnPresenceChangedListener {
        private String mNumber = null;
        private int mPresence = 0;
        @Override
        public void onPresenceChanged(String number, int presence) {
            Logger.d(TAG, "onPresenceChanged(): number = " + number + ", presence = " + presence);
            mNumber = number;
            mPresence = presence;
        }
    }
}
