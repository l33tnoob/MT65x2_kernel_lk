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

package com.mediatek.rcse.test.fragments;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ViewImpl;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.plugin.contacts.ContactListExtensionForRCS;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment.ReceivedMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.DateMessage;
import com.mediatek.rcse.fragments.One2OneChatFragment.SentFileTransfer;
import com.mediatek.rcse.interfaces.ChatModel.IChat;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.RcsContact;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.AttachmentTypeSelectorAdapter;
import com.mediatek.rcse.activities.widgets.ChatAdapter;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.DateLabelItemBinder;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.activities.widgets.ChatAdapter.AbsItemBinder;
import com.mediatek.rcse.test.Utils;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.api.client.messaging.InstantMessage;
import com.orangelabs.rcs.service.api.client.capability.Capabilities;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import java.util.UUID;

/**
 * This class is used to test the functions of ComposingManager in Model part
 */
public class One2OneChatFragmentTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {

    private static final String TAG = "One2OneChatFragmentTest";
    private One2OneChatFragment mOne2OneChatFragment = null;
    private static final String SCONTACT = "+8618980928189@voiceservice.homeip.net";
    private static final String CONTACT_NUMBER2 = "250";
    private static final String SDISPLAYNAME = "free";
    private static final int WAIT_TIME = 5000;
    private static final String METHOD_CLEAR_TOP_REMINDER = "handleClearTopReminder";
    private static final String FIELD_REMOTE_STRANGER = "mRemoteStrangerText";
    private static final String METHOD_FT_INVITE = "handleFileTransferInvite";
    private static final String METHOD_FTSIZE_INVITE = "handleFileSizeWhenInvite";
    private static final String FIELD_WARNING_SIZE = "sWarningFileSize";
    private static final String FIELD_MAX_SIZE = "sMaxFileSize";
    private static final String FIELD_WARNING_DIALOG = "mWarningDialog";
    private static final String FIELD_REPICK_DIALOG = "mRepickDialog";
    private static final String FIELD_FT_ENABLE_STATUS = "mFiletransferEnableStatus";
    private static final String FIELD_PREFT_MAP = "mPreFileTransferMap";
    private static final String FIELD_PREMSG_MAP = "mPreMessageMap";
    private static final String METHOD_SHOW_TOP_REMINDER = "handleShowTopReminder";
    private static final String METHOD_ON_SEND = "onSend";
    private static final String METHOD_QUERY_CAPABILITY = "queryCapablility";
    private static final String CHAT_MAP = "mChatMap";
    private static final String FIELD_PARTICIPANT = "mParticipant";
    private static final String FIELD_TMP_CAMERA_URI = "mCameraTempFileUri";
    private static final String METHOD_ONCLICK = "onClick";
    private static final String FIELD_REQUEST_CODE = "mRequestCode";
    private static final String FIELD_CHECK_REMIND = "mCheckRemind";
    private static final String METHOD_ADD_ATTACHMENT = "addAttachment";
    private static final String METHOD_ADD_CONTACTS_ONEONE = "addContactsToOne2OneChat";
    private static final String METHOD_ADD_RECEIVED_FT = "addReceivedFileTransfer";
    private static final String FIELD_MESSAGE_LIST = "mMessageList";
    private static final String FIELD_IS_RCSE = "mRemoteIsRcse";
    private static final String FIELD_IS_BOTTOM = "mIsBottom";
    private static final String FIELD_MAINTHREAD_ID = "THREAD_ID_MAIN";
    private static final String METHOD_ADD_SENTMESSAGE = "addSentMessage";
    private static final String FIELD_PREMESSAGE_MAP = "mPreMessageMap";
    private static final String METHOD_ADD_RECDMESSAGE = "addReceivedMessage";
    private static final String FIELD_MESSAGE_ADAPTER = "mMessageAdapter";
    private static final String METHOD_ADD_SENTFT = "addSentFileTransfer";
    private static final String FIELD_FILE_NAME = "mFileName";
    private static final String MOCKED_SESSION_ID = "123";
    private static final int TIME_OUT = 10000;
    private static final String TEST_MESSAGE = "test message";

    public One2OneChatFragmentTest() {
        super(ChatScreenActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Intent intent = new Intent();
        intent.putExtra("", "");
        setActivityIntent(intent);
        Method initializeMethod = ApiManager.class.getDeclaredMethod("initialize", Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), getInstrumentation().getTargetContext());
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(SCONTACT, SDISPLAYNAME);
        final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
        mOne2OneChatFragment = oneChatWindow.getFragment();
        mOne2OneChatFragment.setTag(parcelUuid);
        mOne2OneChatFragment.setParticipant(participant);
        ContactsListManager.initialize(getInstrumentation().getTargetContext());
    }

    /**
     * Test the participant's number
     */
    public void testCase01_getParticipantsNum() {
        Logger.v(TAG, "testCase01_getParticipantsNum");
        if (mOne2OneChatFragment != null) {
            int num = mOne2OneChatFragment.getParticipantsNum();
            assertEquals(num, 1);
        } else {
            Logger.w(TAG, "testCase2_getParticipantsNum mOne2OneChatFragment is null.");
        }
    }

    /**
     * Test whether compress image work when compress image check box is
     * checked.
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     * @throws InterruptedException
     * @throws NoSuchFieldException
     */
    public void testCase02_compressImageByDefault() throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Logger.d(TAG, "testCase02_compressImageByDefault()");
        RcsSettings.getInstance().setCompressingImage(true);
        assertEquals(null, prepareCompress(false));
    }

    /**
     * Test whether compress function reduce the image size
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase03_compressResult() throws NoSuchMethodException, IllegalArgumentException,
            IllegalAccessException, InvocationTargetException {
        assertNotNull("mOne2OneChatFragment is null", mOne2OneChatFragment);
        FragmentTransaction fragmentTransaction = getActivity().getFragmentManager()
                .beginTransaction();
        fragmentTransaction.add(mOne2OneChatFragment, TAG);
        Method methodDoCompress = Utils.getPrivateMethod(mOne2OneChatFragment.getClass(),
                "doCompress", String.class);
        methodDoCompress.setAccessible(true);
        String originFileName = getFilePath();
        assertNotNull(originFileName);
        String compressedFilename = (String) methodDoCompress.invoke(mOne2OneChatFragment,
                originFileName);
        assertNotNull(compressedFilename);
        File originFile = new File(originFileName);
        File compressedFile = new File(compressedFilename);
        assertNotNull(originFile);
        assertNotNull(compressedFile);
        Logger.d(TAG, "originFile.length() = " + originFile.length()
                + ", compressedFile.length() = " + compressedFile.length());
        assertTrue("origin file is not larger than compressed file ",
                originFile.length() >= compressedFile.length());
    }

    /**
     * Test the getEmotionsVisibility() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase04_getEmotionsVisibility() throws Throwable, IllegalAccessException,
            NoSuchMethodException, IllegalArgumentException, InvocationTargetException {
        Logger.d(TAG, "testCase04_getEmotionsVisibility()");
        assertNotNull("mOne2OneChatFragment is null", mOne2OneChatFragment);
        final Method methodShowImm = mOne2OneChatFragment.getClass().getSuperclass()
                .getDeclaredMethod("showImm", Boolean.class);
        methodShowImm.setAccessible(true);
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addOne2OneChatUi(mOne2OneChatFragment);
                    methodShowImm.invoke(mOne2OneChatFragment, false);
                    assertTrue(mOne2OneChatFragment.getEmotionsVisibility() == View.VISIBLE);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test the hideEmotions() method
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase05_hideEmotions() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Logger.d(TAG, "testCase05_hideEmotions()");
        assertNotNull("mOne2OneChatFragment is null", mOne2OneChatFragment);
        final Method methodShowImm = mOne2OneChatFragment.getClass().getSuperclass()
                .getDeclaredMethod("showImm", Boolean.class);
        methodShowImm.setAccessible(true);
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addOne2OneChatUi(mOne2OneChatFragment);
                    methodShowImm.invoke(mOne2OneChatFragment, false);
                    mOne2OneChatFragment.hideEmotions();
                    assertTrue((mOne2OneChatFragment).getEmotionsVisibility() == View.GONE);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test the handleClearTopReminder() method
     */
    public void testCase06_handleClearTopReminder() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                try {
                    activity.addOne2OneChatUi(mOne2OneChatFragment);
                    final Method methodClearTopReminder = Utils.getPrivateMethod(
                            One2OneChatFragment.class, METHOD_CLEAR_TOP_REMINDER);
                    methodClearTopReminder.invoke(mOne2OneChatFragment);
                    Field field = Utils.getPrivateField(One2OneChatFragment.class,
                            FIELD_REMOTE_STRANGER);
                    TextView view = (TextView) field.get(mOne2OneChatFragment);
                    assertTrue(view.getVisibility() == View.GONE);
                } catch (Throwable e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test the handleFileSizeWhenInvite/handleFileTransferInvite method
     */
    public void testCase07_handleFileTransferInvite() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                try {
                    testFtInvitation(activity);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

    }

    private void testFtInvitation(ChatScreenActivity activity) throws InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException, NoSuchFieldException {
        activity.addOne2OneChatUi(mOne2OneChatFragment);
        Field fieldWarningSize = Utils.getPrivateField(ApiManager.class, FIELD_WARNING_SIZE);
        Field fieldMaxSize = Utils.getPrivateField(ApiManager.class, FIELD_MAX_SIZE);
        SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(activity);
        prefer.edit().putBoolean(SettingsFragment.RCS_REMIND, true).commit();
        fieldWarningSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        fieldMaxSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        final Method methodFtSize = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_FTSIZE_INVITE, String.class);
        String filePath = getFilePath();

        /*
         * Warning
         */
        Field fieldWarningDialog = Utils.getPrivateField(One2OneChatFragment.class,
                FIELD_WARNING_DIALOG);
        DialogFragment dialog = (DialogFragment) fieldWarningDialog.get(mOne2OneChatFragment);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        assertFalse(dialog.isHidden());
        Method method = Utils.getPrivateMethod(dialog.getClass(), "onClick", DialogInterface.class,
                int.class);
        method.invoke(dialog, (DialogInterface) null, DialogInterface.BUTTON_POSITIVE);

        /*
         * file size < warning size, handleFileTransferInvite()
         */
        fieldWarningSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        fieldMaxSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        Field fieldPreFtMap = Utils.getPrivateField(One2OneChatFragment.class, FIELD_PREFT_MAP);
        Map<?, ?> preFtMap = (Map<?, ?>) fieldPreFtMap.get(mOne2OneChatFragment);
        int sizeBefore = preFtMap.size();
        Field fieldStatus = Utils
                .getPrivateField(One2OneChatFragment.class, FIELD_FT_ENABLE_STATUS);
        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_ENABLE_OK);
        int sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        fieldStatus.set(mOne2OneChatFragment,
                One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        fieldStatus.set(mOne2OneChatFragment, 100);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);

        sizeBefore = preFtMap.size();
        fieldStatus.set(mOne2OneChatFragment,
                One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore);

        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore);

        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore);

        fieldStatus.set(mOne2OneChatFragment, -1);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore);

        /*
         * handleFileTransferInvite()
         */
        fieldWarningSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        fieldMaxSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        prefer.edit().putBoolean(SettingsFragment.RCS_REMIND, false).commit();
        sizeBefore = preFtMap.size();
        fieldStatus.set(mOne2OneChatFragment, One2OneChat.FILETRANSFER_ENABLE_OK);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        /*
         * Repick
         */
        fieldMaxSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        Field fieldRepickDialog = Utils.getPrivateField(One2OneChatFragment.class,
                FIELD_REPICK_DIALOG);
        dialog = (DialogFragment) fieldRepickDialog.get(mOne2OneChatFragment);
        Bundle arguments = new Bundle();
        arguments.putInt(com.mediatek.rcse.service.Utils.MESSAGE, 1);
        dialog.setArguments(arguments);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        assertFalse(dialog.isHidden());
        dialog.dismissAllowingStateLoss();
    }

    /**
     * Test method: handleShowTopReminder().
     */
    public void testCase08_handleShowTopReminder() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Field fieldStranger = Utils.getPrivateField(One2OneChatFragment.class,
                FIELD_REMOTE_STRANGER);
        final Method methodShowTopReminder = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_SHOW_TOP_REMINDER, String.class);
        final TextView view = (TextView) fieldStranger.get(mOne2OneChatFragment);

        methodShowTopReminder
                .invoke(mOne2OneChatFragment, One2OneChatFragment.SHOW_REMOTE_STRANGER);
        assertTrue(view.getVisibility() == View.VISIBLE);

        methodShowTopReminder.invoke(mOne2OneChatFragment,
                One2OneChatFragment.SHOW_REMOTE_OFFLINE_REMINDER);
        assertTrue(view.getVisibility() == View.GONE);
    }

    /**
     * Test method: onSend().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    /* 
    public void testCase09_onSend() throws Throwable, InterruptedException, NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Field fieldPreMsgMap = Utils.getPrivateField(One2OneChatFragment.class.getSuperclass(),
                FIELD_PREMSG_MAP);
        Map<?, ?> preFtMap = (Map<?, ?>) fieldPreMsgMap.get(mOne2OneChatFragment);
        int sizeBefore = preFtMap.size();
        final Method methodSend = Utils.getPrivateMethod(One2OneChatFragment.class, METHOD_ON_SEND,
                String.class);
        methodSend.invoke(mOne2OneChatFragment, TEST_MESSAGE);
        int sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        sizeBefore = preFtMap.size();
        methodSend.invoke(mOne2OneChatFragment, TEST_MESSAGE);
        sizeAfter = preFtMap.size();
        assertEquals(sizeAfter, sizeBefore + 1);

        methodSend.invoke(mOne2OneChatFragment, new String());
    }
*/
    /**
     * Test method: queryCapablility().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase10_queryCapablility() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Participant participant = new Participant(SCONTACT, SDISPLAYNAME);
        UUID uuid = UUID.randomUUID();
        ParcelUuid tag = new ParcelUuid(uuid);
        mOne2OneChatFragment.setTag(tag);
        Field chatMapField = Utils.getPrivateField(ModelImpl.class, CHAT_MAP);
        ModelImpl modle = (ModelImpl) ModelImpl.getInstance();
        Map<Object, IChat> chatMap = (Map<Object, IChat>) chatMapField.get(modle);
        MockOneOneChat oneoneChat = new MockOneOneChat(modle, new MockChatWindow(), participant,
                tag);
        chatMap.put(tag, oneoneChat);
        final Method methodQueryCapability = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_QUERY_CAPABILITY);
        methodQueryCapability.invoke(mOne2OneChatFragment);
        boolean success = false;
        long beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (oneoneChat.isCapabilityCheked()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test method: Repick().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase20_onClickRepick() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();

        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Field fieldWarningSize = Utils.getPrivateField(ApiManager.class, FIELD_WARNING_SIZE);
        Field fieldMaxSize = Utils.getPrivateField(ApiManager.class, FIELD_MAX_SIZE);
        SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(activity);
        prefer.edit().putBoolean(SettingsFragment.RCS_REMIND, true).commit();
        fieldWarningSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        final Method methodFtSize = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_FTSIZE_INVITE, String.class);
        String filePath = getFilePath();

        /*
         * Repick
         */
        fieldMaxSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        Field fieldRepickDialog = Utils.getPrivateField(One2OneChatFragment.class,
                FIELD_REPICK_DIALOG);
        DialogFragment dialog = (DialogFragment) fieldRepickDialog.get(mOne2OneChatFragment);
        Field fieldRequestCode = Utils.getPrivateField(dialog.getClass(), FIELD_REQUEST_CODE);
        fieldRequestCode.set(dialog, One2OneChatFragment.RESULT_CODE_CAMERA);
        Bundle bundle = new Bundle();
        bundle.putInt(com.mediatek.rcse.service.Utils.MESSAGE,
                One2OneChatFragment.RESULT_CODE_CAMERA);
        dialog.setArguments(bundle);
        methodFtSize.invoke(mOne2OneChatFragment, filePath);
        assertFalse(dialog.isHidden());
        dialog.dismissAllowingStateLoss();

        final Method methodClick = Utils.getPrivateMethod(dialog.getClass(), METHOD_ONCLICK,
                DialogInterface.class, int.class);
        methodClick.invoke(dialog, (DialogInterface) null, DialogInterface.BUTTON_NEGATIVE);
        // /*
        // * From camera
        // */
        // final Method methodClick = Utils.getPrivateMethod(dialog.getClass(),
        // METHOD_ONCLICK,
        // DialogInterface.class, int.class);
        // methodClick.invoke(dialog, null, DialogInterface.BUTTON_POSITIVE);
        // Field fieldtmpCameraUri =
        // Utils.getPrivateField(One2OneChatFragment.class,
        // FIELD_TMP_CAMERA_URI);
        // assertNotNull(fieldtmpCameraUri.get(mOne2OneChatFragment));
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_CAMERA);
        // getInstrumentation().waitForIdleSync();
        //
        // /*
        // * From file manager.
        // */
        // fieldRequestCode.set(dialog,
        // One2OneChatFragment.RESULT_CODE_FILE_MANAGER);
        // methodFtSize.invoke(mOne2OneChatFragment, filePath);
        // assertFalse(dialog.isHidden());
        // methodClick.invoke(dialog, null, DialogInterface.BUTTON_POSITIVE);
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_FILE_MANAGER);
        // getInstrumentation().waitForIdleSync();
        //
        // /*
        // * From gallery.
        // */
        // fieldRequestCode.set(dialog,
        // One2OneChatFragment.RESULT_CODE_GALLERY);
        // methodFtSize.invoke(mOne2OneChatFragment, filePath);
        // assertFalse(dialog.isHidden());
        // methodClick.invoke(dialog, null, DialogInterface.BUTTON_POSITIVE);
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_GALLERY);
        // getInstrumentation().waitForIdleSync();
    }

    /**
     * Test method: onClickWarningDialog
     */
    public void testCase18_onClickWarningDialog() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Field fieldWarningSize = Utils.getPrivateField(ApiManager.class, FIELD_WARNING_SIZE);
        Field fieldMaxSize = Utils.getPrivateField(ApiManager.class, FIELD_MAX_SIZE);
        SharedPreferences prefer = PreferenceManager.getDefaultSharedPreferences(activity);
        prefer.edit().putBoolean(SettingsFragment.RCS_REMIND, false).commit();
        fieldWarningSize.set(ApiManager.getInstance(), Long.MIN_VALUE);
        fieldMaxSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        final Method methodFtSize = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_FTSIZE_INVITE, String.class);
        String filePath = getFilePath();

        Field fieldFileName = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_FILE_NAME);
        fieldFileName.set(mOne2OneChatFragment, filePath);

        /*
         * Warning
         */
        Field fieldWarningDialog = Utils.getPrivateField(One2OneChatFragment.class,
                FIELD_WARNING_DIALOG);
        DialogFragment dialog = (DialogFragment) fieldWarningDialog.get(mOne2OneChatFragment);
        dialog.show(getActivity().getFragmentManager(), TAG);
        assertFalse(dialog.isHidden());

        /*
         * Click
         */
        final Method methodClick = Utils.getPrivateMethod(dialog.getClass(), METHOD_ONCLICK,
                DialogInterface.class, int.class);
        methodClick.invoke(dialog, (DialogInterface) null, DialogInterface.BUTTON_POSITIVE);
        boolean success = false;
        long beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (!dialog.isAdded()) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test method: addAttachment
     */
    public void testCase15_addAttachment() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method methodAddAttachment = Utils.getPrivateMethod(
                One2OneChatFragment.class.getSuperclass(), METHOD_ADD_ATTACHMENT, int.class);
        // /*
        // * Camera
        // */
        // methodAddAttachment.invoke(mOne2OneChatFragment,
        // AttachmentTypeSelectorAdapter.ADD_FILE_FROM_CAMERA);
        // Field fieldtmpCameraUri =
        // Utils.getPrivateField(One2OneChatFragment.class,
        // FIELD_TMP_CAMERA_URI);
        // assertNotNull(fieldtmpCameraUri.get(mOne2OneChatFragment));
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_CAMERA);
        // getInstrumentation().waitForIdleSync();
        // /*
        // * Gallery
        // */
        // methodAddAttachment.invoke(mOne2OneChatFragment,
        // AttachmentTypeSelectorAdapter.ADD_FILE_FROM_GALLERY);
        // getInstrumentation().waitForIdleSync();
        // assertNotNull(fieldtmpCameraUri.get(mOne2OneChatFragment));
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_GALLERY);
        // getInstrumentation().waitForIdleSync();
        // /*
        // * File manager
        // */
        // methodAddAttachment.invoke(mOne2OneChatFragment,
        // AttachmentTypeSelectorAdapter.ADD_FILE_FROM_FILE_MANAGER);
        // getInstrumentation().waitForIdleSync();
        // assertNotNull(fieldtmpCameraUri.get(mOne2OneChatFragment));
        // getInstrumentation().waitForIdleSync();
        // activity.finishActivity(One2OneChatFragment.RESULT_CODE_FILE_MANAGER);
        // getInstrumentation().waitForIdleSync();
        /*
         * Default
         */
        methodAddAttachment.invoke(mOne2OneChatFragment, -1);
        // assertNotNull(fieldtmpCameraUri.get(mOne2OneChatFragment));
    }

    /**
     * Test method: addContactsToOneOneChat
     */
    public void testCase11_addContactsToOneOneChat() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_ADD_CONTACTS_ONEONE, ArrayList.class);

        ArrayList<Participant> list = new ArrayList<Participant>();
        Participant participant = new Participant(SCONTACT, SDISPLAYNAME);
        list.add(participant);
        ChatScreenWindowContainer.getInstance().clearCurrentStatus();
        method.invoke(mOne2OneChatFragment, list);
        assertEquals(list, ChatScreenWindowContainer.getInstance().getCurrentParticipants());
    }

    /**
     * Test method: addReceivedFileTransfer
     */
    public void testCase12_addReceivedFileTransfer() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_ADD_RECEIVED_FT, FileStruct.class);
        Field fieldRcse = Utils.getPrivateField(mOne2OneChatFragment.getClass(), FIELD_IS_RCSE);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        Field fieldBottom = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_IS_BOTTOM);
        fieldBottom.setBoolean(mOne2OneChatFragment, false);

        Field fieldMessageList = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        List<?> messageList = (List<?>) fieldMessageList.get(mOne2OneChatFragment);
        String filePath = getFilePath();
        final FileStruct fs = new FileStruct(filePath,
                filePath.substring(filePath.lastIndexOf(".")), new File(filePath).length(),
                MOCKED_SESSION_ID, new Date());
        assertNull(method.invoke(mOne2OneChatFragment, (FileStruct) null));

        int sizeBefore = messageList.size();
        method.invoke(mOne2OneChatFragment, fs);
        getInstrumentation().waitForIdleSync();
        int sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 2);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        Field fieldThreadId = Utils.getPrivateField(
                mOne2OneChatFragment.getClass().getSuperclass(), FIELD_MAINTHREAD_ID);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId() + 1);

        // Not THREAD_ID_MAIN
        sizeBefore = messageList.size();
        method.invoke(mOne2OneChatFragment, fs);
        getInstrumentation().waitForIdleSync();
        sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // file is null
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, (FileStruct) null);
        getInstrumentation().waitForIdleSync();
        assertFalse(fieldRcse.getBoolean(mOne2OneChatFragment));

        // main thread: show as stranger
        RcsContact rcsContact = new RcsContact(SDISPLAYNAME, SCONTACT);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().STRANGER_LIST.add(SCONTACT);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());

        sizeBefore = messageList.size();
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, fs);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // not main thread: show as stranger
        ContactsListManager.getInstance().STRANGER_LIST.add(SCONTACT);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId() + 1);

        sizeBefore = messageList.size();
        method.invoke(mOne2OneChatFragment, fs);
        getInstrumentation().waitForIdleSync();
        sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // main thread: show as local
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        sizeBefore = messageList.size();
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, fs);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // not main thread: show as local
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId() + 1);

        sizeBefore = messageList.size();
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, fs);
        getInstrumentation().waitForIdleSync();
        sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 1);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));
        ContactsListManager.getInstance().STRANGER_LIST.remove(SCONTACT);
        ContactsListManager.getInstance().CONTACTS_LIST.remove(rcsContact);
    }

    /**
     * Test method: addSentMessage
     */
    public void testCase13_addSentMessage() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_ADD_SENTMESSAGE, InstantMessage.class, int.class);
        Field fieldPreMessageMap = Utils.getPrivateField(mOne2OneChatFragment.getClass()
                .getSuperclass(), FIELD_PREMESSAGE_MAP);
        Map<?, ?> preMessageMap = (Map<?, ?>) fieldPreMessageMap.get(mOne2OneChatFragment);
        preMessageMap.clear();
        Field fieldMessageList = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        List<?> messageList = (List<?>) fieldMessageList.get(mOne2OneChatFragment);
        int sizeBefore = messageList.size();
        InstantMessage msg = new InstantMessage("", "", "", true);

        // not find in preMessageMap
        method.invoke(mOne2OneChatFragment, msg, -1);
        int sizeAfter = messageList.size();
        assertEquals(sizeAfter, sizeBefore + 2);

        // find in preMessageMap
        method.invoke(mOne2OneChatFragment, msg, -1);
        assertFalse(preMessageMap.containsKey(-1));

        Field fieldPreMsgMap = Utils.getPrivateField(One2OneChatFragment.class.getSuperclass(),
                FIELD_PREMSG_MAP);
        Map<Integer, ISentChatMessage> preFtMap = (Map<Integer, ISentChatMessage>) fieldPreMsgMap
                .get(mOne2OneChatFragment);
        preFtMap.put(-1, (ISentChatMessage) method.invoke(mOne2OneChatFragment, msg, -1));
        assertNotNull(method.invoke(mOne2OneChatFragment, msg, -1));

        // null msg.
        Object result = method.invoke(mOne2OneChatFragment, (InstantMessage) null, -1);
        assertNull(result);

        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);
        fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);

        // null adapter
        result = method.invoke(mOne2OneChatFragment, msg, -2);
        assertNull(result);

    }

    /**
     * Test method: addReceivedMessage
     */
    public void testCase14_addReceivedMessage() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                METHOD_ADD_RECDMESSAGE, InstantMessage.class, boolean.class);
        final InstantMessage msg = new InstantMessage("", "", "", true);

        // null msg
        Object result = method.invoke(mOne2OneChatFragment, (InstantMessage) null, false);
        assertNull(result);

        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);

        Field fieldRcse = Utils.getPrivateField(mOne2OneChatFragment.getClass(), FIELD_IS_RCSE);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        Field fieldBottom = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_IS_BOTTOM);
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        Field fieldMessageList = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        List<?> messageList = (List<?>) fieldMessageList.get(mOne2OneChatFragment);
        messageList.clear();

        int sizeBefore = messageList.size();
        Field fieldThreadId = Utils.getPrivateField(
                mOne2OneChatFragment.getClass().getSuperclass(), FIELD_MAINTHREAD_ID);

        RcsContact rcsContact = new RcsContact(SDISPLAYNAME, SCONTACT);
        // main thread.
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, msg, false);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));
        assertEquals(messageList.size(), sizeBefore + 2);

        // not main thread.
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId() + 1);
        method.invoke(mOne2OneChatFragment, msg, false);
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // main thread: show as stranger
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().STRANGER_LIST.add(SCONTACT);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        sizeBefore = messageList.size();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, msg, false);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // main thread: show as local
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        sizeBefore = messageList.size();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, msg, false);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        getInstrumentation().waitForIdleSync();
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));

        // main thread: show as local
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        method.invoke(mOne2OneChatFragment, msg, false);
        getInstrumentation().waitForIdleSync();

        // not main thread: show as local
        fieldBottom.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId() + 1);
        method.invoke(mOne2OneChatFragment, msg, false);
        getInstrumentation().waitForIdleSync();
        ContactsListManager.getInstance().STRANGER_LIST.remove(SCONTACT);
        ContactsListManager.getInstance().CONTACTS_LIST.remove(rcsContact);
    }

    /**
     * Test method: onActivityResult
     */
    public void testCase15_onActivityResult() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Field fieldWarningSize = Utils.getPrivateField(ApiManager.class, FIELD_WARNING_SIZE);
        Field fieldMaxSize = Utils.getPrivateField(ApiManager.class, FIELD_MAX_SIZE);
        fieldWarningSize.set(ApiManager.getInstance(), 0L);
        fieldMaxSize.set(ApiManager.getInstance(), 0L);
        Method methodOnActivityResult = Utils.getPrivateMethod(mOne2OneChatFragment.getClass(),
                "onActivityResult", int.class, int.class, Intent.class);
        Participant participant = new Participant(CONTACT_NUMBER2, CONTACT_NUMBER2);
        ArrayList<Participant> participants = new ArrayList<Participant>();
        participants.add(participant);
        Intent intent = new Intent();
        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST, participants);
        methodOnActivityResult.invoke(mOne2OneChatFragment, ChatFragment.RESULT_CODE_ADD_CONTACTS,
                Activity.RESULT_OK, intent);
        getInstrumentation().waitForIdleSync();
        assertEquals(ChatScreenWindowContainer.getInstance().getCurrentParticipants().size(), 2);

        // result_cancel
        Field fieldFileName = Utils.getPrivateField(mOne2OneChatFragment.getClass(), "mFileName");
        fieldFileName.set(mOne2OneChatFragment, "");
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_CANCELED, intent);
        boolean success = false;
        long beginTime = System.currentTimeMillis();
        // just wait
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT / 3) {
        }
        assertTrue(fieldFileName.get(mOne2OneChatFragment).equals(""));

        // null data in intent
        fieldFileName.set(mOne2OneChatFragment, "");
        intent = new Intent();
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        // just wait
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT / 3) {
        }
        assertTrue(fieldFileName.get(mOne2OneChatFragment).equals(""));

        // gallery, enable
        RcsSettings.getInstance().setCompressingImage(true);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // gallery, not enable, remind, click ok
        RcsSettings.getInstance().setCompressingImage(false);
        RcsSettings.getInstance().saveRemindCompressFlag(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        Field fieldCompressDialog = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                "mCompressDialog");
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldCompressDialog.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // just wait
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT / 3) {
        }
        Object dialog = fieldCompressDialog.get(mOne2OneChatFragment);
        Method methodOnClick = Utils.getPrivateMethod(dialog.getClass(), "onClick",
                DialogInterface.class, int.class);
        // click ok
        methodOnClick.invoke(dialog, (DialogInterface) null, DialogInterface.BUTTON_POSITIVE);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // gallery, enable, click cancel
        RcsSettings.getInstance().setCompressingImage(false);
        RcsSettings.getInstance().saveRemindCompressFlag(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldCompressDialog.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        dialog = fieldCompressDialog.get(mOne2OneChatFragment);
        // just wait
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT / 3) {
        }
        // click cancel
        methodOnClick.invoke(dialog, (DialogInterface) null, DialogInterface.BUTTON_NEGATIVE);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // gallery, not enable, not remind
        RcsSettings.getInstance().setCompressingImage(true);
        RcsSettings.getInstance().saveRemindCompressFlag(true);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // gallery, file:\\
        RcsSettings.getInstance().setCompressingImage(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse("file://" + getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // camera, not null
        RcsSettings.getInstance().setCompressingImage(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        Field fieldCamera = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                "mCameraTempFileUri");
        fieldCamera.set(mOne2OneChatFragment, Uri.fromFile(new File(getImageUrl())));
        methodOnActivityResult.invoke(mOne2OneChatFragment, One2OneChatFragment.RESULT_CODE_CAMERA,
                Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // camera, null
        RcsSettings.getInstance().setCompressingImage(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        fieldCamera.set(mOne2OneChatFragment, Uri.fromFile(new File(getImageUrl())));
        methodOnActivityResult.invoke(mOne2OneChatFragment, One2OneChatFragment.RESULT_CODE_CAMERA,
                Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) != null) {
                success = true;
                break;
            }
        }
        assertTrue(success);

        // file manager, not null
        RcsSettings.getInstance().setCompressingImage(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        intent.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_FILE_MANAGER, Activity.RESULT_OK, intent);
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT / 3) {
        }

        // file manager, null
        RcsSettings.getInstance().setCompressingImage(false);
        fieldFileName.set(mOne2OneChatFragment, (String) null);
        intent = new Intent();
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_FILE_MANAGER, Activity.RESULT_OK, intent);
        success = false;
        beginTime = System.currentTimeMillis();
        while ((System.currentTimeMillis() - beginTime) < TIME_OUT) {
            if (fieldFileName.get(mOne2OneChatFragment) == null) {
                success = true;
                break;
            }
        }
        assertTrue(success);
    }

    /**
     * Test method: onCapabilityChanged
     */
    public void testCase16_onCapabilityChanged() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                "onCapabilityChanged", String.class, Capabilities.class);
        Field fieldRcse = Utils.getPrivateField(mOne2OneChatFragment.getClass(), FIELD_IS_RCSE);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        Capabilities capabilities = new Capabilities();
        method.invoke(mOne2OneChatFragment, CONTACT_NUMBER2, capabilities);
        getInstrumentation().waitForIdleSync();
        assertFalse(fieldRcse.getBoolean(mOne2OneChatFragment));

        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        RcsContact rcsContact = new RcsContact(SDISPLAYNAME, SCONTACT);
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        method.invoke(mOne2OneChatFragment, SCONTACT, capabilities);
        getInstrumentation().waitForIdleSync();
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));
        for (RcsContact contact : ContactsListManager.getInstance().CONTACTS_LIST) {
            if (SCONTACT.equals(contact.mNumber)) {
                ContactsListManager.getInstance().CONTACTS_LIST.remove(contact);
            }
        }

        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        ContactsListManager.getInstance().STRANGER_LIST.add(SCONTACT);
        method.invoke(mOne2OneChatFragment, SCONTACT, capabilities);
        getInstrumentation().waitForIdleSync();
        for (String contact : ContactsListManager.getInstance().STRANGER_LIST) {
            if (contact.equals(SCONTACT)) {
                ContactsListManager.getInstance().STRANGER_LIST.remove(contact);
            }
        }

        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, SCONTACT, capabilities);
        getInstrumentation().waitForIdleSync();
        assertFalse(fieldRcse.getBoolean(mOne2OneChatFragment));
        for (String contact : ContactsListManager.getInstance().STRANGER_LIST) {
            if (contact.equals(SCONTACT)) {
                ContactsListManager.getInstance().STRANGER_LIST.remove(contact);
            }
        }

        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        capabilities.setRcseContact(true);
        method.invoke(mOne2OneChatFragment, SCONTACT, capabilities);
        getInstrumentation().waitForIdleSync();
        assertTrue(fieldRcse.getBoolean(mOne2OneChatFragment));
    }

    /**
     * Test method: setFileTransferEnable
     */
    public void testCase17_setFileTransferEnable() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                "setFileTransferEnable", int.class);
        Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                "mFiletransferEnableStatus");
        method.invoke(mOne2OneChatFragment,
                One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);
        getInstrumentation().waitForIdleSync();
        assertEquals(field.getInt(mOne2OneChatFragment),
                One2OneChat.FILETRANSFER_DISABLE_REASON_CAPABILITY_FAILED);

        method.invoke(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);
        getInstrumentation().waitForIdleSync();
        assertEquals(field.getInt(mOne2OneChatFragment),
                One2OneChat.FILETRANSFER_DISABLE_REASON_NOT_REGISTER);

        method.invoke(mOne2OneChatFragment, One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);
        getInstrumentation().waitForIdleSync();
        assertEquals(field.getInt(mOne2OneChatFragment),
                One2OneChat.FILETRANSFER_DISABLE_REASON_REMOTE);

        method.invoke(mOne2OneChatFragment, 4);
        getInstrumentation().waitForIdleSync();
        assertEquals(field.getInt(mOne2OneChatFragment), 4);

        method.invoke(mOne2OneChatFragment, One2OneChat.FILETRANSFER_ENABLE_OK);
        getInstrumentation().waitForIdleSync();
        assertEquals(field.getInt(mOne2OneChatFragment), One2OneChat.FILETRANSFER_ENABLE_OK);

        Field fieldContentView = Utils.getPrivateField(mOne2OneChatFragment.getClass()
                .getSuperclass(), "mContentView");
        final View view = (View) fieldContentView.get(mOne2OneChatFragment);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                ImageButton btnAddView = (ImageButton) view.findViewById(R.id.btn_chat_add);
                btnAddView.performClick();
            }
        });
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                "mAttachmentTypeSelectorAdapter");
        assertNotNull(fieldAdapter.get(mOne2OneChatFragment));
    }

    /**
     * Test method: addLoadHistoryHeader
     */
    public void testCase18_addLoadHistoryHeader() throws Throwable {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                "addLoadHistoryHeader", boolean.class);
        Field fieldRcse = Utils.getPrivateField(mOne2OneChatFragment.getClass(), FIELD_IS_RCSE);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();

        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();

        // main thread: show as stranger
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        RcsContact rcsContact = new RcsContact(SDISPLAYNAME, SCONTACT);
        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();

        // main thread: show as local
        ContactsListManager.getInstance().CONTACTS_LIST.add(rcsContact);
        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();

        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);
        fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);
        fieldRcse.setBoolean(mOne2OneChatFragment, false);
        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();

        ContactsListManager.getInstance().CONTACTS_LIST.remove(rcsContact);
    }

    private Object prepareCompress(boolean wait) throws NoSuchMethodException,
            IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            InterruptedException, NoSuchFieldException {
        Logger.d(TAG, "prepareCompress(), wait = " + wait);
        assertNotNull("mOne2OneChatFragment is null", mOne2OneChatFragment);
        Method methodOnActivityResult = Utils.getPrivateMethod(mOne2OneChatFragment.getClass(),
                "onActivityResult", int.class, int.class, Intent.class);
        Intent data = new Intent();
        data.setData(Uri.parse(getImageUrl()));
        methodOnActivityResult.invoke(mOne2OneChatFragment,
                One2OneChatFragment.RESULT_CODE_GALLERY, Activity.RESULT_OK, data);
        getInstrumentation().waitForIdleSync();
        getActivity().finishActivity(One2OneChatFragment.RESULT_CODE_GALLERY);
        Field fieldCompressDialog = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                "mCompressDialog");
        Object dialog = fieldCompressDialog.get(mOne2OneChatFragment);
        if (wait) {
            long beginTime = System.currentTimeMillis();
            while (true) {
                dialog = fieldCompressDialog.get(mOne2OneChatFragment);
                if (System.currentTimeMillis() - beginTime > WAIT_TIME || dialog != null) {
                    Logger.d(TAG, "dialog = " + dialog + ":"
                            + (System.currentTimeMillis() - beginTime));
                    break;
                }
                Thread.sleep(200);
            }
        }
        return dialog;
    }

    @Override
    protected void tearDown() throws Exception {
        Logger.v(TAG, "tearDown() enter");
        Utils.clearAllStatus();
        Field fieldWarningSize = Utils.getPrivateField(ApiManager.class, FIELD_WARNING_SIZE);
        Field fieldMaxSize = Utils.getPrivateField(ApiManager.class, FIELD_MAX_SIZE);
        fieldWarningSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        fieldMaxSize.set(ApiManager.getInstance(), Long.MAX_VALUE);
        ContactsListManager.getInstance().CONTACTS_LIST.clear();
        ContactsListManager.getInstance().STRANGER_LIST.clear();
        Logger.setIsIntegrationMode(false);
        if (getActivity() != null) {
            getActivity().finish();
        }
        super.tearDown();
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
        Logger.v(TAG, "tearDown() exit");
    }

    /**
     * Get a image url
     */
    private String getImageUrl() {
        Logger.v(TAG, "getImageUrl()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String id = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                id = cursor.getString(cursor.getColumnIndex(MediaStore.Images.ImageColumns._ID));
            } else {
                fail("Cannot find an image in sdcard");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getImageUrl() out, id is " + id);
        return MediaStore.Images.Media.EXTERNAL_CONTENT_URI + "/" + id;
    }

    /**
     * Get a image file path
     */
    private String getFilePath() {
        Logger.v(TAG, "getFilePath()");
        Context context = getInstrumentation().getTargetContext();
        Cursor cursor = null;
        String filePath = null;
        try {
            cursor = context.getContentResolver().query(
                    MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null, null);
            if (null != cursor && cursor.getCount() > 0) {
                cursor.moveToFirst();
                filePath = cursor.getString(cursor
                        .getColumnIndex(MediaStore.Images.ImageColumns.DATA));
            } else {
                fail("testCase1_BindView() Cannot find image in sdcard");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        Logger.v(TAG, "getFilePath() out, filePath is " + filePath);
        return filePath;
    }

    /**
     * Mock 1-2-1 chat.
     */
    private class MockOneOneChat extends One2OneChat {
        private boolean mCapabilityCheked = false;

        public MockOneOneChat(ModelImpl modelImpl, IOne2OneChatWindow chatWindow,
                Participant participant, Object tag) {
            super(modelImpl, chatWindow, participant, tag);
        }

        public boolean isCapabilityCheked() {
            return mCapabilityCheked;
        }

        protected void checkAllCapability() {
            super.checkAllCapability();
            mCapabilityCheked = true;
        }
    }

    /**
     * Mock chat window for test
     */
    private class MockChatWindow implements IOne2OneChatWindow {

        public void setFileTransferEnable(int reason) {
        }

        public void setIsComposing(boolean isComposing) {
        }

        public void setRemoteOfflineReminder(boolean isOffline) {
        }

        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return null;
        }

        public IFileTransfer addReceivedFileTransfer(FileStruct file) {
            return null;
        }

        @Override
        public void addLoadHistoryHeader(boolean showLoader) {
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
        }

        @Override
        public void updateAllMsgAsRead() {
        }

		@Override
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

    private ChatAdapter getChatAdapter() throws Exception {
        assertNotNull("mOne2OneChatFragment is null", mOne2OneChatFragment);

        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addOne2OneChatUi(mOne2OneChatFragment);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);
        ChatAdapter chatAdapter = (ChatAdapter) fieldAdapter.get(mOne2OneChatFragment);

        return chatAdapter;
    }

    /**
     * Test case for getSections, getPositionForSection and
     * getSectionForPosition method in class ChatAdapter.
     * 
     * @throws Exception
     */
    public void testCase21_getSections() throws Exception {
        ChatAdapter chatAdapter = getChatAdapter();
        Field fieldItemBinderList = Utils.getPrivateField(ChatAdapter.class, "mItemBinderList");

        int mockPosition1 = 0;
        int mockPosition2 = 1;
        int mockSection1 = 0;
        int mockSection2 = 1;
        DateLabelItemBinder dateLabelItemBinder1 = new DateLabelItemBinder(new Date(2012, 12, 01),
                mockPosition1);
        DateLabelItemBinder dateLabelItemBinder2 = new DateLabelItemBinder(new Date(2012, 12, 02),
                mockPosition2);
        List<AbsItemBinder> itemBinderList = new ArrayList<AbsItemBinder>();
        itemBinderList.add(mockPosition1, dateLabelItemBinder1);
        itemBinderList.add(mockPosition2, dateLabelItemBinder2);
        fieldItemBinderList.set(chatAdapter, itemBinderList);

        // Test case for getSections method in class ChatAdapter.
        Object[] sections = chatAdapter.getSections();
        assertEquals(2, sections.length);

        // Test case for getPositionForSection method in class ChatAdapter.
        int position1 = chatAdapter.getPositionForSection(mockSection1);
        assertEquals(mockPosition1, position1);

        int position2 = chatAdapter.getPositionForSection(mockSection2);
        assertEquals(mockPosition2, position2);

        // Test case for getPositionForSection method in class ChatAdapter.
        int section1 = chatAdapter.getSectionForPosition(mockPosition1);
        assertEquals(0, section1);

        int section2 = chatAdapter.getSectionForPosition(mockPosition2);
        assertEquals(1, section2);
    }

    /**
     * Test case for the rest of methods in class ChatAdapter.
     * 
     * @throws Exception
     */
    public void testCase22_otherMethods() throws Throwable, Exception {
        final ChatAdapter chatAdapter = getChatAdapter();
        final Field fieldItemBinderList = Utils.getPrivateField(ChatAdapter.class,
                "mItemBinderList");

        int mockPosition1 = 0;
        int mockPosition2 = 1;
        DateLabelItemBinder dateLabelItemBinder1 = new DateLabelItemBinder(new Date(2012, 12, 01),
                mockPosition1);
        DateLabelItemBinder dateLabelItemBinder2 = new DateLabelItemBinder(new Date(2012, 12, 02),
                mockPosition2);
        List<AbsItemBinder> itemBinderList = new ArrayList<AbsItemBinder>();
        itemBinderList.add(mockPosition1, dateLabelItemBinder1);
        itemBinderList.add(mockPosition2, dateLabelItemBinder2);
        fieldItemBinderList.set(chatAdapter, itemBinderList);
        // test getItem method
        Object item1 = chatAdapter.getItem(0);
        assertEquals(dateLabelItemBinder1, item1);

        // test removeMessage method
        chatAdapter.removeMessage(1);
        getInstrumentation().waitForIdleSync();

        assertEquals(1, ((List) fieldItemBinderList.get(chatAdapter)).size());

        new Thread(new Runnable() {

            @Override
            public void run() {
                chatAdapter.removeMessage(0);
                getInstrumentation().waitForIdleSync();
                try {
                    assertEquals(0, ((List) fieldItemBinderList.get(chatAdapter)).size());
                } catch (Exception e) {
                    e.printStackTrace();
                    fail(e.getMessage());
                }
            }
        }).start();

        getInstrumentation().waitForIdleSync();

        // test showHeaderView method
        Field fieldHeaderView = ChatAdapter.class.getDeclaredField("mHeaderView");
        fieldHeaderView.setAccessible(true);
        View headerView = (View) fieldHeaderView.get(chatAdapter);
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                chatAdapter.showHeaderView(true);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertTrue(headerView.isShown());

        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                chatAdapter.showHeaderView(false);
            }
        });
        getInstrumentation().waitForIdleSync();
        assertFalse(headerView.isShown());
    }

    /**
     * Test method: SentMessage().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase23_SentMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Class<?>[] classes = One2OneChatFragment.class.getDeclaredClasses();
        Constructor<?> sentMessageCtr = null;
        for (Class<?> classz : classes) {
            if ("SentMessage".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        sentMessageCtr = ctrz;
                        break;
                    }
                }
            }
        }
        Date date = new Date();
        final Date date1 = new Date();
        final Date date2 = new Date();
        final Date date3 = new Date();
        final Date date4 = new Date();
        InstantMessage msg = new InstantMessage("", CONTACT_NUMBER2, "", true, date);
        InstantMessage msg2 = new InstantMessage(null, null, null, false, null);
        InstantMessage msg3 = new InstantMessage("", CONTACT_NUMBER2, "", true, date3);
        InstantMessage msg4 = new InstantMessage("", CONTACT_NUMBER2, "", true, date4);
        final SentMessage sentMessage = (SentMessage) sentMessageCtr.newInstance(
                mOne2OneChatFragment, msg);
        final SentMessage sentMessage2 = (SentMessage) sentMessageCtr.newInstance(
                mOne2OneChatFragment, msg2);
        final SentMessage sentMessage3 = (SentMessage) sentMessageCtr.newInstance(
                mOne2OneChatFragment, msg3);
        final SentMessage sentMessage4 = (SentMessage) sentMessageCtr.newInstance(
                mOne2OneChatFragment, msg4);
        assertEquals(sentMessage.getId(), "");
        sentMessage.updateStatus(Status.DELIVERED);
        getInstrumentation().waitForIdleSync();
        assertEquals(sentMessage.getStatus(), Status.DELIVERED);
        assertEquals(sentMessage.getMessageText(), "");
        assertEquals(sentMessage.getMessageDate(), date);
        assertNotNull(sentMessage.getChatTag());
        Method method = Utils.getPrivateMethod(sentMessage.getClass(), "updateMessage",
                InstantMessage.class);
        method.invoke(sentMessage, (InstantMessage) null);
        sentMessage.updateDate(new Date());
        getInstrumentation().waitForIdleSync();
        assertNull(sentMessage.getId());
        assertNull(sentMessage.getMessageText());

        Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass(), "mMessageList");
        List<IChatWindowMessage> list = (List<IChatWindowMessage>) field.get(mOne2OneChatFragment);
        list.add(0, (IChatWindowMessage) sentMessage);

        Field fieldThreadId = Utils.getPrivateField(
                mOne2OneChatFragment.getClass().getSuperclass(), FIELD_MAINTHREAD_ID);
        fieldThreadId.setLong(mOne2OneChatFragment, Thread.currentThread().getId());
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                sentMessage.updateDate(new Date());
            }
        });
        getInstrumentation().waitForIdleSync();
        assertNull(sentMessage.getId());
        assertNull(sentMessage.getMessageText());

        method.invoke(sentMessage2, msg2);
        method.invoke(sentMessage2, msg3);
        list.add(sentMessage);
        list.add(sentMessage2);
        list.add(sentMessage3);
        list.add(sentMessage4);
        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);
        ChatAdapter adapter = (ChatAdapter) fieldAdapter.get(mOne2OneChatFragment);
        adapter.addMessage(sentMessage);
        adapter.addMessage(sentMessage2);
        adapter.addMessage(sentMessage3);
        adapter.addMessage(sentMessage4);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                try {
                    Method method2 = Utils.getPrivateMethod(mOne2OneChatFragment.getClass(),
                            "onMessageDateUpdated", SentMessage.class, Date.class, Date.class);
                    method2.invoke(mOne2OneChatFragment, sentMessage3, date1, date2);

                    // null message adapter
                    Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass()
                            .getSuperclass(), FIELD_MESSAGE_ADAPTER);
                    fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);
                    mOne2OneChatFragment.addMessageDate(new Date());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        Field field3 = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                "DEFAULT_PRE_SEND_MESSAGE_DATE");
        field3.set(mOne2OneChatFragment, date1);
        getInstrumentation().runOnMainSync(new Runnable() {

            @Override
            public void run() {
                try {
                    Method method2 = Utils.getPrivateMethod(mOne2OneChatFragment.getClass(),
                            "onMessageDateUpdated", SentMessage.class, Date.class, Date.class);
                    method2.invoke(mOne2OneChatFragment, sentMessage2, date1, date4);

                    // null message adapter
                    Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass()
                            .getSuperclass(), FIELD_MESSAGE_ADAPTER);
                    fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);
                    mOne2OneChatFragment.addMessageDate(new Date());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test method: ReceivedMessage().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase24_ReceivedMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        Class<?>[] classes = One2OneChatFragment.class.getDeclaredClasses();
        Constructor<?> receivedMessageCtr = null;
        for (Class<?> classz : classes) {
            if ("ReceivedMessage".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        receivedMessageCtr = ctrz;
                        break;
                    }
                }
            }
        }
        Date date = new Date();
        InstantMessage msg = new InstantMessage("", CONTACT_NUMBER2, "", true, date);
        InstantMessage msg2 = new InstantMessage(null, null, null, false, null);
        ReceivedMessage receivedMessage = (ReceivedMessage) receivedMessageCtr.newInstance(
                mOne2OneChatFragment, msg);
        assertEquals(receivedMessage.getId(), "");
        getInstrumentation().waitForIdleSync();
        assertEquals(receivedMessage.getMessageText(), "");
        assertEquals(receivedMessage.getMessageDate(), date);
        assertEquals(receivedMessage.getMessageSender(), CONTACT_NUMBER2);
        assertEquals(receivedMessage.getDisplayName(), SDISPLAYNAME);

        receivedMessage = (ReceivedMessage) receivedMessageCtr.newInstance(mOne2OneChatFragment,
                (InstantMessage) null);
        assertNull(receivedMessage.getId());
        assertNull(receivedMessage.getMessageText());
        assertNull(receivedMessage.getMessageSender());
        assertNull(receivedMessage.getMessageDate());
        Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass(), "mParticipant");
        field.set(mOne2OneChatFragment, (Participant) null);
        assertNull(receivedMessage.getDisplayName());
    }

    /**
     * Test method: addSentFileTransfer().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase24_addSentFileTransfer() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class, METHOD_ADD_SENTFT,
                FileStruct.class);
        final Field fieldPreFtMap = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_PREFT_MAP);
        final Field fieldMsgList = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                FIELD_MESSAGE_LIST);
        Map<?, ?> preMsgMap = (Map<?, ?>) fieldPreFtMap.get(mOne2OneChatFragment);
        preMsgMap.clear();
        List msgList = (List) fieldMsgList.get(mOne2OneChatFragment);
        msgList.clear();
        String filePath = getFilePath();
        final FileStruct fs = new FileStruct(filePath,
                filePath.substring(filePath.lastIndexOf(".")), new File(filePath).length(),
                MOCKED_SESSION_ID, new Date());
        int sizeBefore = msgList.size();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, fs);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        int sizeAfter = msgList.size();

        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, fs);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        sizeAfter = preMsgMap.size();
        assertEquals(sizeAfter, 0);

        final Method methodSentFile = Utils.getPrivateMethod(One2OneChatFragment.class,
                "onSentFile", FileStruct.class);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    assertNull(methodSentFile.invoke(mOne2OneChatFragment, (FileStruct) null));
                    assertNull(methodSentFile.invoke(mOne2OneChatFragment, fs));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });

        sizeBefore = preMsgMap.size();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, fs);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        sizeAfter = preMsgMap.size();
        assertEquals(sizeAfter, sizeBefore - 1);

        // null message adapter
        Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                FIELD_MESSAGE_ADAPTER);
        fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);
        preMsgMap.clear();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    assertNotNull(method.invoke(mOne2OneChatFragment, fs));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        preMsgMap.clear();
    }

    /**
     * Test method: setRemoteOfflineReminder().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase25_setRemoteOfflineReminder() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                "setRemoteOfflineReminder", boolean.class);
        Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                "mTextReminderSortedSet");
        TreeSet<String> treeSet = (TreeSet<String>) field.get(mOne2OneChatFragment);
        method.invoke(mOne2OneChatFragment, false);
        getInstrumentation().waitForIdleSync();
        assertFalse(treeSet.contains(ChatFragment.SHOW_REMOTE_OFFLINE_REMINDER));

        method.invoke(mOne2OneChatFragment, true);
        getInstrumentation().waitForIdleSync();
        assertTrue(treeSet.contains(ChatFragment.SHOW_REMOTE_OFFLINE_REMINDER));

        method.invoke(mOne2OneChatFragment, false);
        getInstrumentation().waitForIdleSync();
        assertFalse(treeSet.contains(ChatFragment.SHOW_REMOTE_OFFLINE_REMINDER));
    }

    /**
     * Test method: updateSendButtonState().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase25_updateSendButtonState() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                activity.addOne2OneChatUi(mOne2OneChatFragment);
            }
        });
        final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                "updateSendButtonState", String.class);
        final Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass().getSuperclass(),
                "mBtnSend");
        final ImageButton btn = (ImageButton) field.get(mOne2OneChatFragment);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, new String());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        assertFalse(btn.isEnabled());
        Field fieldRcse = Utils.getPrivateField(mOne2OneChatFragment.getClass(), FIELD_IS_RCSE);
        fieldRcse.setBoolean(mOne2OneChatFragment, true);
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    method.invoke(mOne2OneChatFragment, new String("abc"));
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        assertTrue(btn.isEnabled());
    }

    /**
     * Test method: onDisplayNameChanged().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase26_onDisplayNameChanged() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        final ChatScreenActivity activity = getActivity();
        getInstrumentation().runOnMainSync(new Runnable() {
            public void run() {
                try {
                    activity.addOne2OneChatUi(mOne2OneChatFragment);
                    final Method method = Utils.getPrivateMethod(One2OneChatFragment.class,
                            "onDisplayNameChanged");
                    Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass(),
                            "mParticipant");

                    field.set(mOne2OneChatFragment, (Participant) null);
                    method.invoke(mOne2OneChatFragment);

                    // null message adapter
                    Field fieldAdapter = Utils.getPrivateField(mOne2OneChatFragment.getClass()
                            .getSuperclass(), FIELD_MESSAGE_ADAPTER);
                    fieldAdapter.set(mOne2OneChatFragment, (ChatAdapter) null);
                    method.invoke(mOne2OneChatFragment);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * Test method: DateMessage().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase27_DateMessage() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Class<?>[] classes = One2OneChatFragment.class.getDeclaredClasses();
        Constructor<?> dateMessageCtr = null;
        for (Class<?> classz : classes) {
            if ("DateMessage".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 1) {
                        dateMessageCtr = ctrz;
                        break;
                    }
                }
            }
        }
        Date date = new Date();
        Date date2 = new Date(1000l);
        DateMessage msg = (DateMessage) dateMessageCtr.newInstance(date);
        DateMessage msg2 = (DateMessage) dateMessageCtr.newInstance(date);
        DateMessage msg3 = (DateMessage) dateMessageCtr.newInstance(date2);
        assertEquals(msg.getMessageDate(), date);
        assertTrue(msg.equals(msg2));
        assertFalse(msg.equals(msg3));
        assertFalse(msg.equals(""));
        assertNotNull(msg.hashCode());
        assertNull(msg.getId());
    }

    /**
     * Test method: SentFileTransfer().
     * 
     * @throws NoSuchMethodException
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     * @throws IllegalArgumentException
     */
    public void testCase28_SentFileTransfer() throws Throwable, InterruptedException,
            NoSuchMethodException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        Class<?>[] classes = One2OneChatFragment.class.getDeclaredClasses();
        Constructor<?> ctr = null;
        for (Class<?> classz : classes) {
            if ("SentFileTransfer".equals(classz.getSimpleName())) {
                Constructor<?>[] ctrs = classz.getDeclaredConstructors();
                for (Constructor<?> ctrz : ctrs) {
                    Class<?>[] paras = ctrz.getParameterTypes();
                    if (paras.length == 2) {
                        ctr = ctrz;
                        break;
                    }
                }
            }
        }
        String filePath = getFilePath();
        final FileStruct fs = new FileStruct(filePath,
                filePath.substring(filePath.lastIndexOf(".")), new File(filePath).length(),
                MOCKED_SESSION_ID, new Date());
        Field field = Utils.getPrivateField(mOne2OneChatFragment.getClass(), "mParticipant");
        field.set(mOne2OneChatFragment, (Participant) null);
        SentFileTransfer sentFt = (SentFileTransfer) ctr.newInstance(mOne2OneChatFragment, fs);
        assertNull(sentFt.getContactName());
        assertNull(sentFt.getContactNum());
        sentFt.setProgress(10L);
        getInstrumentation().waitForIdleSync();
        sentFt.setStatus(com.mediatek.rcse.interfaces.ChatView.IFileTransfer.Status.CANCELED);
        getInstrumentation().waitForIdleSync();
        assertNull(sentFt.getTag());
        assertNull(sentFt.getId());
        SentFileTransfer sentFt2 = (SentFileTransfer) ctr.newInstance(mOne2OneChatFragment,
                (FileStruct) null);
        assertNull(sentFt2.getMessageDate());
    }
}
