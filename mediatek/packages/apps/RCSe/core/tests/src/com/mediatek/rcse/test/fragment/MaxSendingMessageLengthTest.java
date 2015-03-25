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

package com.mediatek.rcse.test.fragment;

import android.app.Activity;
import android.content.Context;
import android.os.ParcelUuid;
import android.test.ActivityInstrumentationTestCase2;
import android.widget.EditText;

import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.test.Utils;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.One2OneChatFragment;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;

import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Random;
import java.util.UUID;

/**
 * This class is used to test the functions of sending editor in View part
 */
public class MaxSendingMessageLengthTest extends ActivityInstrumentationTestCase2<ChatScreenActivity> {

    private final static String TAG = "MaxSendingMessageLengthTest";
    private final static String FIELD_NAME_MESSAGE_EDITOR = "mMessageEditor";
    private final static String METHOD_NAME_INITIALIZE = "initialize";
    private final static String BASE = "abcdefghijklmnopqrstuvwxyz0123456789";
    private final static String MOCK_CONTACT_PHONE = "+861860";
    private final static String MOCK_CONTACT_NAME = "MOCK";
    private final static int  SIZE_OFFSET = 1;
    private int mMaxEditorSize;
    private EditText mEditText = null;

    private String getRandomString(int length) {
        Random random = new Random();
        StringBuffer stringBuffer = new StringBuffer();
        for (int i = 0; i < length; i++) {
            int number = random.nextInt(BASE.length());
            stringBuffer.append(BASE.charAt(number));
        }
        Logger.d(TAG, "getRandomString current random string is:" + stringBuffer.toString());
        return stringBuffer.toString();
    }

    private Field getEditTextFiled() throws NoSuchFieldException {
        Field editTextField = ChatFragment.class.getDeclaredField(FIELD_NAME_MESSAGE_EDITOR);
        editTextField.setAccessible(true);
        return editTextField;
    }

    private ChatFragment createChatFragment(final Activity activity) throws Throwable {
        UUID uuid = UUID.randomUUID();
        ParcelUuid parcelUuid = new ParcelUuid(uuid);
        Participant participant = new Participant(MOCK_CONTACT_PHONE, MOCK_CONTACT_NAME);
        final OneOneChatWindow oneChatWindow = new OneOneChatWindow(parcelUuid, participant);
        final One2OneChatFragment chatFragment = oneChatWindow.getFragment();
        this.runTestOnUiThread(new Runnable() {
            public void run() {
                ((ChatScreenActivity) activity).addOne2OneChatUi(chatFragment);
            }
        });
        return chatFragment;
    }

    /**
     * Constructor
     */
    public MaxSendingMessageLengthTest() {
        super("com.mediatek.rcse.activities", ChatScreenActivity.class);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        Logger.d(TAG, "tearDown()");
        //Wait Activity finish
        Thread.sleep(Utils.TEAR_DOWN_SLEEP_TIME);
    }

    @Override
    protected void setUp() throws Exception{
        super.setUp();
        Method initializeMethod = ApiManager.class.getDeclaredMethod(METHOD_NAME_INITIALIZE,
                Context.class);
        initializeMethod.setAccessible(true);
        initializeMethod.invoke(new Boolean(true), this.getActivity().getBaseContext());
        RcsSettings.createInstance(this.getActivity().getBaseContext());
        RcsSettings rcsSettings = RcsSettings.getInstance();
        if (rcsSettings != null) {
            mMaxEditorSize = rcsSettings.getMaxChatMessageLength();
        } else {
            Logger.d(TAG, "setUp the rcsSettings is null");
        }
        final Activity activity = this.getActivity();
        try{
            Object obj = getEditTextFiled().get(createChatFragment(activity));
            mEditText = (EditText) obj;
        }catch(Throwable t){
            Logger.d(TAG, "setUp get object from filed failed");
            t.printStackTrace();
        }finally{
            assertTrue(mEditText != null);
        }
    }

    /*
     * This test case is for the case that when the input size is less than the
     * max editor size
     */
    public void testCase1_lessThanMaxEditorSize() throws Throwable,InterruptedException{
        this.runTestOnUiThread(new Runnable(){
            public void run(){
                mEditText.setText(getRandomString(mMaxEditorSize-SIZE_OFFSET));
            }
        });
        getInstrumentation().waitForIdleSync();
        assertTrue(mEditText.getEditableText() != null);
    }

    /*
     * This test case is for the case that when the input size is equal the max
     * editor size
     */
    public void testCase2_equalWithMaxEditorSize() throws Throwable,InterruptedException{
        this.runTestOnUiThread(new Runnable(){
            public void run(){
                mEditText.setText(getRandomString(mMaxEditorSize));
            }
        });
        getInstrumentation().waitForIdleSync();
        assertFalse(mEditText.getEditableText() == null);
    }

    /*
     * This test case is for the case that when the input size is greater than
     * the max editor size
     */
    public void testCase3_greaterThanMaxEditorSize() throws Throwable,InterruptedException{
        this.runTestOnUiThread(new Runnable(){
            public void run(){
                mEditText.setText(getRandomString(mMaxEditorSize+SIZE_OFFSET));
            }
        });
        getInstrumentation().waitForIdleSync();
        assertFalse(mEditText.getEditableText() == null);
    }
}
