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
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, {
 * 
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.test.model;

import android.content.Context;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelUuid;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment.ChatEventInformation;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation.Information;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used to test ViewImpl.java
 */
public class ViewImplTest extends InstrumentationTestCase {
    private static final String TAG = "ViewImplTest";
    private static final String CONTACT_NUMBER1 = "+341";
    private static final String CONTACT_NUMBER2 = "+342";
    private static final String CONTACT_NUMBER3 = "+343";
    private static final String OLD_TAG = "oldTag";
    private static final String NEW_TAG = "newTag";
    private static final String ONEONE_WINDOW = "One2OneChatWindowDispatcher";
    private static final String GROUP_WINDOW = "GroupChatWindowDispatcher";
    private static final String METHOD_SETCOMPOSING = "setIsComposing";
    private static final String FIELD_CHATWINDOWMAP = "mChatWindowMap";
    private ViewImpl mViewImpl = ViewImpl.getInstance();
    private MockOneOneChatWindow mOneOneWindow = null;
    private MockGroupChatWindow mGroupWindow = null;
    private IChatEventInformation mChatEvent = null;
    public One2OneChatFragment.SentFileTransfer mSentFileTransfer = null;
    private One2OneChatFragment.ReceivedFileTransfer mReceivedFileTransfer = null;
    private SentChatMessage mSentChatMessage = null;
    private One2OneChatFragment mOne2OneChatFragment = new One2OneChatFragment();
    private boolean mIsRemoved = false;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        mOneOneWindow = null;
        mGroupWindow = null;
        super.tearDown();
    }

    /**
     * Test case to setIsComposing().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase01_setIsComposing() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException, NoSuchFieldException,
            InterruptedException, InstantiationException {
        UUID uuid1 = UUID.randomUUID();
        ParcelUuid tag1 = new ParcelUuid(uuid1);
        UUID uuid2 = UUID.randomUUID();
        ParcelUuid tag2 = new ParcelUuid(uuid2);
        Participant participant1 = new Participant(CONTACT_NUMBER1, "");
        Participant participant2 = new Participant(CONTACT_NUMBER2, "");
        ParticipantInfo info1 = new ParticipantInfo(participant1, User.STATE_PENDING);
        ParticipantInfo info2 = new ParticipantInfo(participant2, User.STATE_PENDING);
        MockChatWindowManager chatWindowManager = new MockChatWindowManager();
        CopyOnWriteArrayList<ParticipantInfo> list = new CopyOnWriteArrayList<ParticipantInfo>();
        list.add(info1);
        list.add(info2);
        mViewImpl.addChatWindowManager(chatWindowManager, true);
        mViewImpl.addOne2OneChatWindow(tag1, participant1);
        mViewImpl.addGroupChatWindow(tag2, list);

        mOneOneWindow = new MockOneOneChatWindow();
        mGroupWindow = new MockGroupChatWindow();
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> oneoneCtr = null;
        Constructor<?> groupCtr = null;
        for (Class<?> classz : classes) {
            if (ONEONE_WINDOW.equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        oneoneCtr = ctrz;
                    }
                }
            } else if (GROUP_WINDOW.equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        groupCtr = ctrz;
                    }
                }
            }
        }
        assertNotNull(groupCtr);

        Object groupDispatcher = groupCtr.newInstance(tag1, list);
        Field chatWindowMapField = Utils.getPrivateField(
                groupDispatcher.getClass().getSuperclass(), FIELD_CHATWINDOWMAP);
        Object chatWindowMap = chatWindowMapField.get(groupDispatcher);
        ((ConcurrentHashMap<IChatWindowManager, IChatWindow>) chatWindowMap).put(chatWindowManager,
                mGroupWindow);
        Method method = Utils.getPrivateMethod(groupDispatcher.getClass(), METHOD_SETCOMPOSING,
                boolean.class, Participant.class);
        method.invoke(groupDispatcher, true, participant2);
        assertTrue(mGroupWindow.isComposing());

        assertNotNull(oneoneCtr);
        Object oneoneDispatcher = oneoneCtr.newInstance(tag1, participant1);
        chatWindowMap = chatWindowMapField.get(oneoneDispatcher);
        ((ConcurrentHashMap<IChatWindowManager, IChatWindow>) chatWindowMap).put(chatWindowManager,
                mOneOneWindow);
        method = Utils.getPrivateMethod(oneoneDispatcher.getClass(), METHOD_SETCOMPOSING,
                boolean.class);
        method.invoke(oneoneDispatcher, true);
        assertTrue(mOneOneWindow.isComposing());

        mViewImpl.removeChatWindowManager(chatWindowManager);
    }

    /**
     * Test case to ChatEventInformationDispatcher().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase02_ChatEventInformationDispatcher() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException {
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> chatEventCtr = null;
        for (Class<?> classz : classes) {
            if ("ChatEventInformationDispatcher".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        chatEventCtr = ctrz;
                    }
                }
            }
        }

        // constructor
        ChatEventStruct struct = new ChatEventStruct(Information.INVITE, CONTACT_NUMBER1,
                new Date());
        Object object = chatEventCtr.newInstance(struct);
        Field field = Utils.getPrivateField(object.getClass(), "mChatEventStruct");
        assertNotNull(field.get(object));

        // onAddChatWindow
        Field fieldChatWindowMap = Utils.getPrivateField(object.getClass(), "mChatWindowMap");
        Map<?, ?> map = (Map<?, ?>) fieldChatWindowMap.get(object);
        Method methodAddWindow = Utils.getPrivateMethod(object.getClass(), "onAddChatWindow",
                IChatWindow.class);
        MockGroupChatWindow window = new MockGroupChatWindow();

        methodAddWindow.invoke(object, (IChatWindow) null);
        assertTrue(map.size() == 0);

        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 0);

        mChatEvent = new ChatEventInformation(new ChatEventStruct(Information.INVITE,
                CONTACT_NUMBER1, new Date()));
        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 1);

        // onRemoveChatWindow
        Method methodRemoveWindow = Utils.getPrivateMethod(object.getClass(), "onRemoveChatWindow",
                IChatWindow.class);
        methodRemoveWindow.invoke(object, window);
        assertTrue(map.size() == 0);
    }

    /**
     * Test case to ReceivedFileTransferDispatcher().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase03_ReceivedFileTransferDispatcher() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException {
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> chatEventCtr = null;
        for (Class<?> classz : classes) {
            if ("ReceivedFileTransferDispatcher".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        chatEventCtr = ctrz;
                    }
                }
            }
        }

        // constructor
        FileStruct struct = new FileStruct("", "", 1L, "", new Date());
        Object object = chatEventCtr.newInstance(struct);
        Field field = Utils.getPrivateField(object.getClass().getSuperclass(), "mFileStruct");
        assertNotNull(field.get(object));

        // onAddChatWindow
        Field fieldChatWindowMap = Utils.getPrivateField(object.getClass().getSuperclass(),
                "mChatWindowMap");
        Map<?, ?> map = (Map<?, ?>) fieldChatWindowMap.get(object);
        Method methodAddWindow = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onAddChatWindow", IChatWindow.class);
        MockOneOneChatWindow window = new MockOneOneChatWindow();

        methodAddWindow.invoke(object, (IChatWindow) null);
        assertTrue(map.size() == 0);

        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 0);

        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
        mOne2OneChatFragment = oneChatWindow.getFragment();
        mReceivedFileTransfer = mOne2OneChatFragment.new ReceivedFileTransfer(struct, mIsRemoved);
        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 1);

        Method methodSetProgress = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setProgress", long.class);
        methodSetProgress.invoke(object, 1L);
        Field fieldProgress = Utils.getPrivateField(object.getClass().getSuperclass(), "mProgress");
        assertEquals(fieldProgress.get(object), 1L);

        Method methodSetStatus = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setStatus", Status.class);
        methodSetStatus.invoke(object, Status.PENDING);
        Field fieldStatus = Utils.getPrivateField(object.getClass().getSuperclass(), "mStatus");
        assertEquals(fieldStatus.get(object), Status.PENDING);

        methodSetStatus.invoke(object, Status.PENDING);
        assertEquals(fieldStatus.get(object), Status.PENDING);

        Method methodSetFilePath = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setFilePath", String.class);
        methodSetFilePath.invoke(object, "abc");
        assertEquals(struct.mFilePath, "abc");

        Method methodUpdateTag = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "updateTag", String.class, long.class);
        methodUpdateTag.invoke(object, "abc", 1L);
        assertEquals(struct.mFileTransferTag, "abc");
        assertEquals(mReceivedFileTransfer.getFileStruct().mSize, 1L);

        // onRemoveChatWindow
        Method methodRemoveWindow = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onRemoveChatWindow", IChatWindow.class);
        methodRemoveWindow.invoke(object, window);
        assertTrue(map.size() == 0);

        // getFileTransfer
        Method methodGetFileTransfer = Utils.getPrivateMethod(object.getClass(), "getFileTransfer",
                IChatWindow.class, FileStruct.class);
        Object result = methodGetFileTransfer.invoke(object, window, struct);
        assertEquals(mReceivedFileTransfer, result);
    }

    /**
     * Test case to SentFileTransferDispatcher().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase04_SentFileTransferDispatcher() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException {
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> chatEventCtr = null;
        for (Class<?> classz : classes) {
            if ("SentFileTransferDispatcher".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        chatEventCtr = ctrz;
                    }
                }
            }
        }

        // constructor
        FileStruct struct = new FileStruct("", "", 1L, "", new Date());
        Object object = chatEventCtr.newInstance(struct);
        Field field = Utils.getPrivateField(object.getClass().getSuperclass(), "mFileStruct");
        assertNotNull(field.get(object));

        // onAddChatWindow
        Field fieldChatWindowMap = Utils.getPrivateField(object.getClass().getSuperclass(),
                "mChatWindowMap");
        Map<?, ?> map = (Map<?, ?>) fieldChatWindowMap.get(object);
        Method methodAddWindow = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onAddChatWindow", IChatWindow.class);
        MockOneOneChatWindow window = new MockOneOneChatWindow();

        methodAddWindow.invoke(object, (IChatWindow) null);
        assertTrue(map.size() == 0);

        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 0);

        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
        mOne2OneChatFragment = oneChatWindow.getFragment();
        mSentFileTransfer = mOne2OneChatFragment.new SentFileTransfer(struct);
        methodAddWindow.invoke(object, window);
        assertTrue(map.size() == 1);

        // setProgress
        Method methodSetProgress = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setProgress", long.class);
        methodSetProgress.invoke(object, 1L);
        Field fieldProgress = Utils.getPrivateField(object.getClass().getSuperclass(), "mProgress");
        assertEquals(fieldProgress.get(object), 1L);

        // setStatus
        Method methodSetStatus = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setStatus", Status.class);
        methodSetStatus.invoke(object, Status.PENDING);
        Field fieldStatus = Utils.getPrivateField(object.getClass().getSuperclass(), "mStatus");
        assertEquals(fieldStatus.get(object), Status.PENDING);

        methodSetStatus.invoke(object, Status.PENDING);
        assertEquals(fieldStatus.get(object), Status.PENDING);

        // setFilePath
        Method methodSetFilePath = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "setFilePath", String.class);
        methodSetFilePath.invoke(object, "abc");
        assertEquals(struct.mFilePath, "abc");

        // updateTag
        Method methodUpdateTag = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "updateTag", String.class, long.class);
        methodUpdateTag.invoke(object, "abc", 1L);
        assertEquals(struct.mFileTransferTag, "abc");
        assertEquals(mSentFileTransfer.getFileStruct().mSize, 1L);

        // onRemoveChatWindow
        Method methodRemoveWindow = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onRemoveChatWindow", IChatWindow.class);
        methodRemoveWindow.invoke(object, window);
        assertTrue(map.size() == 0);

        // getFileTransfer
        Method methodGetFileTransfer = Utils.getPrivateMethod(object.getClass(), "getFileTransfer",
                IChatWindow.class, FileStruct.class);
        Object result = methodGetFileTransfer.invoke(object, window, struct);
        assertEquals(mSentFileTransfer, result);
    }

    /**
     * Test case to One2OneChatWindowDispatcher().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase05_One2OneChatWindowDispatcher() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException {
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> chatWindowCtr = null;
        for (Class<?> classz : classes) {
            if ("One2OneChatWindowDispatcher".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        chatWindowCtr = ctrz;
                    }
                }
            }
        }

        // constructor
        FileStruct struct = new FileStruct("", "", 1L, "", new Date());
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        Object object = chatWindowCtr.newInstance(parcelUuid, participant);
        Field field = Utils.getPrivateField(object.getClass(), "mParticipant");
        assertEquals(field.get(object), participant);

        // onAddChatWindowManager
        Field fieldChatWindowMap = Utils.getPrivateField(object.getClass().getSuperclass(),
                "mChatWindowMap");
        Map<?, ?> map = (Map<?, ?>) fieldChatWindowMap.get(object);
        Method methodAddWM = Utils.getPrivateMethod(object.getClass(), "onAddChatWindowManager",
                IChatWindowManager.class);
        MockChatWindowManager windowManager = new MockChatWindowManager();

        methodAddWM.invoke(object, windowManager);
        assertTrue(map.size() == 0);

        mOneOneWindow = new MockOneOneChatWindow();
        methodAddWM.invoke(object, windowManager);
        assertTrue(map.size() == 1);

        // setFileTransferEnable
        Method method = Utils.getPrivateMethod(object.getClass(), "setFileTransferEnable",
                int.class);
        method.invoke(object, 1);
        field = Utils.getPrivateField(object.getClass(), "mFileTransferDisableReason");
        assertEquals(field.get(object), 1);

        // setIsComposing
        method = Utils.getPrivateMethod(object.getClass(), "setIsComposing", boolean.class);
        method.invoke(object, true);
        field = Utils.getPrivateField(object.getClass(), "mIsComposing");
        assertEquals(field.get(object), true);

        // setRemoteOfflineReminder
        method = Utils.getPrivateMethod(object.getClass(), "setRemoteOfflineReminder",
                boolean.class);
        method.invoke(object, true);
        assertEquals(field.get(object), true);

        // addReceivedFileTransfer
        method = Utils.getPrivateMethod(object.getClass(), "addReceivedFileTransfer",
                FileStruct.class);
        field = Utils.getPrivateField(object.getClass().getSuperclass(), "mChatItemList");
        List itemList = (List) field.get(object);
        int size = itemList.size();
        method.invoke(object, struct);
        assertEquals(itemList.size(), size + 1);

        // addReceivedFileTransfer
        method = Utils.getPrivateMethod(object.getClass(), "addSentFileTransfer", FileStruct.class);
        size = itemList.size();
        method.invoke(object, struct);
        assertEquals(itemList.size(), size + 1);

        // updateAllMsgAsRead
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "updateAllMsgAsRead");
        method.invoke(object);
        assertTrue(mOneOneWindow.isUpdated());

        // addReceivedMessage
        InstantMessage msg = new InstantMessage("-1", "", "", true);
        InstantMessage msg2 = new InstantMessage("-2", "", "", true);
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "addReceivedMessage",
                InstantMessage.class, boolean.class);
        size = itemList.size();
        Object result = method.invoke(object, msg, true);
        assertEquals(itemList.size(), size + 1);

        // getSentChatMessage
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "getSentChatMessage",
                String.class);
        result = method.invoke(object, "-2");
        assertNull(result);

        // addSentMessage
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "addSentMessage",
                InstantMessage.class, int.class);
        size = itemList.size();
        result = method.invoke(object, msg2, -2);
        assertEquals(itemList.size(), size + 1);

        // getSentChatMessage
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "getSentChatMessage",
                String.class);
        mSentChatMessage = new SentChatMessage(msg, -2);
        result = method.invoke(object, "-2");
        assertNotNull(result);

        // onRemoveChatWindowManager
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onRemoveChatWindowManager", IChatWindowManager.class);
        method.invoke(object, windowManager);
        assertTrue(map.size() == 0);

        // onAddChatWindowManager
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(),
                "onAddChatWindowManager", IChatWindowManager.class);
        method.invoke(object, windowManager);
        assertTrue(map.size() == 1);

        // removeAllMessages
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "removeAllMessages");
        method.invoke(object);
        assertTrue(itemList.size() == 0);

        // addLoadHistoryHeader
        method = Utils.getPrivateMethod(object.getClass().getSuperclass(), "addLoadHistoryHeader",
                boolean.class);
        method.invoke(object, true);
        assertTrue(mOneOneWindow.showLoader());

        // onDestroy
        method = Utils.getPrivateMethod(object.getClass(), "onDestroy");
        method.invoke(object);
        assertTrue(mIsRemoved);
    }

    /**
     * Test case to GroupChatWindowDispatcher().
     * 
     * @throws NoSuchMethodException
     * @throws IllegalArgumentException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws NoSuchFieldException
     * @throws InterruptedException
     */
    public void testCase06_GroupChatWindowDispatcher() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            NoSuchFieldException, InterruptedException, InstantiationException {
        Class<?>[] classes = ViewImpl.class.getDeclaredClasses();
        Constructor<?> chatWindowCtr = null;
        for (Class<?> classz : classes) {
            if ("GroupChatWindowDispatcher".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        chatWindowCtr = ctrz;
                    }
                }
            }
        }

        // constructor
        FileStruct struct = new FileStruct("", "", 1L, "", new Date());
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(CONTACT_NUMBER1, CONTACT_NUMBER1);
        Participant participant2 = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER2);
        CopyOnWriteArrayList<ParticipantInfo> list = new CopyOnWriteArrayList<ParticipantInfo>();
        list.add(new ParticipantInfo(participant, User.STATE_PENDING));
        list.add(new ParticipantInfo(participant2, User.STATE_PENDING));
        Object object = chatWindowCtr.newInstance(parcelUuid, list);
        Field field = Utils.getPrivateField(object.getClass().getSuperclass(), "mTag");
        assertEquals(field.get(object), parcelUuid);

        // onAddChatWindowManager
        Field fieldChatWindowMap = Utils.getPrivateField(object.getClass().getSuperclass(),
                "mChatWindowMap");
        Map<?, ?> map = (Map<?, ?>) fieldChatWindowMap.get(object);
        Method methodAddWM = Utils.getPrivateMethod(object.getClass(), "onAddChatWindowManager",
                IChatWindowManager.class);
        MockChatWindowManager windowManager = new MockChatWindowManager();

        methodAddWM.invoke(object, windowManager);
        assertTrue(map.size() == 0);

        mGroupWindow = new MockGroupChatWindow();
        methodAddWM.invoke(object, windowManager);
        assertTrue(map.size() == 1);

        // updateParticipants
        Method method = Utils.getPrivateMethod(object.getClass(), "updateParticipants", List.class);
        method.invoke(object, list);
        field = Utils.getPrivateField(object.getClass(), "mParticipantInfos");
        assertEquals(field.get(object), list);

        // setIsComposing
        method = Utils.getPrivateMethod(object.getClass(), "setIsComposing", boolean.class,
                Participant.class);
        method.invoke(object, true, participant);
        assertEquals(mGroupWindow.isComposing(), true);

        // setIsRejoining
        method = Utils.getPrivateMethod(object.getClass(), "setIsRejoining", boolean.class);
        method.invoke(object, true);
        field = Utils.getPrivateField(object.getClass(), "mIsRejoining");
        assertEquals(field.get(object), true);

        // addChatEventInformation
        method = Utils.getPrivateMethod(object.getClass(), "addChatEventInformation",
                ChatEventStruct.class);
        ChatEventStruct chatEventStruct = new ChatEventStruct(Information.INVITE, "", new Date());
        field = Utils.getPrivateField(object.getClass().getSuperclass(), "mChatItemList");
        List itemList = (List) field.get(object);
        int size = itemList.size();
        method.invoke(object, chatEventStruct);
        assertEquals(itemList.size(), size + 1);

        // onDestroy
        method = Utils.getPrivateMethod(object.getClass(), "onDestroy");
        method.invoke(object);
        assertTrue(mIsRemoved);
    }

    private class MockChatWindowManager implements IChatWindowManager {
        private String mCurrentTag = OLD_TAG;

        public String getCurrentTag() {
            return mCurrentTag;
        }

        public IOne2OneChatWindow addOne2OneChatWindow(Object tag, Participant participant) {
            return mOneOneWindow;
        }

        public IGroupChatWindow addGroupChatWindow(Object tag, List<ParticipantInfo> participantList) {
            return mGroupWindow;
        }

        public boolean removeChatWindow(IChatWindow chatWindow) {
            mIsRemoved = true;
            return false;
        }

        public void switchChatWindowByTag(ParcelUuid uuidTag) {

        }
    }

    private class MockGroupChatWindow implements IGroupChatWindow {
        private boolean mMessagesRemoved = false;
        private boolean mIsComposing = false;

        public boolean isComposing() {
            return mIsComposing;
        }

        public void updateParticipants(List<ParticipantInfo> participants) {

        }

        public void setIsComposing(boolean isComposing, Participant participant) {
            Logger.d(TAG, "setIsComposing() isComposing: " + isComposing);
            mIsComposing = isComposing;
        }

        public void setIsRejoining(boolean isRejoining) {

        }

        public IChatEventInformation addChatEventInformation(ChatEventStruct chatEventStruct) {
            return mChatEvent;
        }

        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            return null;
        }

        public void removeAllMessages() {
            mMessagesRemoved = true;
        }

        public IChatWindowMessage getSentChatMessage(String messageId) {
            return mSentChatMessage;
        }

        public void addLoadHistoryHeader(boolean showLoader) {

        }

        public void updateAllMsgAsRead() {

        }

        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

		@Override
		public void removeChatMessage(String messageId) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void setFileTransferEnable(int reason) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void updateChatStatus(int status) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public void addgroupSubject(String subject) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IFileTransfer addSentFileTransfer(FileStruct file) {
			// TODO Auto-generated method stub
			return null;
		}

		@Override
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}

    }

    /**
     * Mock chat window for test
     */
    private class MockOneOneChatWindow implements IOne2OneChatWindow {
        private boolean mUpdated = false;
        private boolean mMessagesRemoved = false;
        private boolean mIsComposing = false;
        private boolean mShowLoader = false;

        public boolean isUpdated() {
            return mUpdated;
        }

        public boolean showLoader() {
            return mShowLoader;
        }

        public boolean isMessagesRemoved() {
            return mMessagesRemoved;
        }

        public boolean isComposing() {
            return mIsComposing;
        }

        public void setFileTransferEnable(int reason) {

        }

        public void setIsComposing(boolean isComposing) {
            Logger.d(TAG, "setIsComposing() isComposing: " + isComposing);
            mIsComposing = isComposing;
        }

        public void setRemoteOfflineReminder(boolean isOffline) {

        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return mSentFileTransfer;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return mReceivedFileTransfer;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
            mShowLoader = showLoader;
        }

        @Override
        public IReceivedChatMessage addReceivedMessage(InstantMessage message, boolean isRead) {
            return null;
        }

        @Override
        public ISentChatMessage addSentMessage(InstantMessage message, int messageTag) {
            return null;
        }

        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages() entry");
            mMessagesRemoved = true;
        }

        @Override
        public void updateAllMsgAsRead() {
            Logger.d(TAG, "updateAllMsgAsRead() entry");
            mUpdated = true;
        }

        public void updateAllMsgAsReadForContact(Participant participant){
        	
        }

		@Override
		public void removeChatMessage(String messageId) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public IFileTransfer addReceivedFileTransfer(FileStruct file,
				boolean isAutoAccept) {
			// TODO Auto-generated method stub
			return null;
		}
    }
}
