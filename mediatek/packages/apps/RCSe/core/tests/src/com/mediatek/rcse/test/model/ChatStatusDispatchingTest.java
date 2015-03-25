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

import android.app.Instrumentation.ActivityMonitor;

import android.os.SystemClock;
import android.test.ActivityInstrumentationTestCase2;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatImpl;
import com.mediatek.rcse.test.Utils;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * This class is used to test the functions of pause status dispatching in Model
 * part
 */
public class ChatStatusDispatchingTest extends ActivityInstrumentationTestCase2<ChatMainActivity> {
    private static final String TAG = "ChatStatusDispatchingTest";
    private final Participant mParticipantA = new Participant("+3420000090", "TestUserA");

    private final Participant mParticipantB = new Participant("+3420000080", "TestUserB");

    private final List<Participant> mSingleParticipant = new ArrayList<Participant>();

    private final List<Participant> mMultiParticipants = new ArrayList<Participant>();

    private final static int CHAT_TAG_ARRAY_FIRST = 0;

    private final static int CHAT_TAG_ARRAY_LAST = 1;

    private final static int CHAT_TAG_ARRAY_SIZE = 2;
    
    private final static int WAIT_TIME_OUT = 3000;

    private final Object mChatTagArray[] = new Object[CHAT_TAG_ARRAY_SIZE];
    
    private ActivityMonitor mChatMainActivityMonitor = new ActivityMonitor(
            ChatScreenActivity.class.getName(), null, false);
    
    public ChatStatusDispatchingTest() {
        super(ChatMainActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        getInstrumentation().addMonitor(mChatMainActivityMonitor);
        SystemClock.sleep(WAIT_TIME_OUT);
        mSingleParticipant.add(mParticipantA);
        ChatImpl singleParticipantChat = (ChatImpl) ModelImpl.getInstance().addChat(
                mSingleParticipant, null, null);
        assertNotNull(singleParticipantChat);
        Object tagA = singleParticipantChat.getChatTag();
        assertNotNull(tagA);
        mChatTagArray[CHAT_TAG_ARRAY_FIRST] = tagA;

        mMultiParticipants.add(mParticipantA);
        mMultiParticipants.add(mParticipantB);
        ChatImpl multiParticipantChat = (ChatImpl) ModelImpl.getInstance().addChat(
                mMultiParticipants, null, null);
        assertNotNull(multiParticipantChat);
        Object tagB = multiParticipantChat.getChatTag();
        assertNotNull(tagB);
        mChatTagArray[CHAT_TAG_ARRAY_LAST] = tagB;
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() entry");
        super.tearDown();
        Utils.clearAllStatus();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Used to test the puase status dispatching in the model part
     * 
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase1_onPause() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InstantiationException, ClassNotFoundException,
            NoSuchMethodException, NoSuchFieldException {
        for (Object tag : mChatTagArray) {
            IChat chat = ModelImpl.getInstance().getChat(tag);
            assertNotNull(chat);
            Class clazz = Class.forName(ChatImpl.class.getName());
            Method method = clazz.getDeclaredMethod("onPause");
            method.setAccessible(true);
            method.invoke((ChatImpl) chat);
            Field field = clazz.getDeclaredField("mIsInBackground");
            field.setAccessible(true);
            assertTrue(field.getBoolean((ChatImpl) chat));
        }
    }

    /**
     * Used to test the resume status dispatching in the model part
     * 
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     */
    public void testCase2_onResume() throws IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, InstantiationException, ClassNotFoundException,
            NoSuchMethodException, NoSuchFieldException {
        for (Object tag : mChatTagArray) {
            IChat chat = ModelImpl.getInstance().getChat(tag);
            assertNotNull(chat);
            Class clazz = Class.forName(ChatImpl.class.getName());
            Method method = clazz.getDeclaredMethod("onResume");
            method.setAccessible(true);
            method.invoke((ChatImpl) chat);
            Field field = clazz.getDeclaredField("mIsInBackground");
            field.setAccessible(true);
            assertFalse(field.getBoolean((ChatImpl) chat));
        }
    }
}