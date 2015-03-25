/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.voicecommand.business;

import java.util.ArrayList;

import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.ContactCounts;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;

public class VoiceContactsObserver extends ContentObserver {
    private Context mContext;
    private Handler mainHandler;
    private HandlerThread mHandlerThread;
    private Handler mVoiceHandler;

    private static final int MSG_GET_CONTACTS_NAME = 1000;

    // public static final Uri CONTACTS_URI = Uri
    // .parse("content://com.android.contacts/contacts?address_book_index_extras=true&directory=0");
    public static final Uri CONTACTS_URI = Uri
            .parse(ContactsContract.Contacts.CONTENT_URI + "?"
                    + ContactCounts.ADDRESS_BOOK_INDEX_EXTRAS + "=true&"
                    + ContactsContract.DIRECTORY_PARAM_KEY + "=0");

//    private long mStartTime = -1L;
//    private long mEndTime = -1L;

    public VoiceContactsObserver(Context context, Handler handler) {
        super(handler);
        mContext = context;
        mainHandler = handler;
        mHandlerThread = new HandlerThread("VoiceHandlerThread");
        mHandlerThread.start();
        mVoiceHandler = new VoiceHandler(mHandlerThread.getLooper());
        mVoiceHandler.sendEmptyMessage(MSG_GET_CONTACTS_NAME);
//        mStartTime = System.currentTimeMillis();
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.d(VoiceCommandManagerStub.TAG, "onChange uri : " + uri);
        // TODO Auto-generated method stub
//        mStartTime = System.currentTimeMillis();
        if (mVoiceHandler.hasMessages(MSG_GET_CONTACTS_NAME)) {
            mVoiceHandler.removeMessages(MSG_GET_CONTACTS_NAME);
        }
        mVoiceHandler.sendEmptyMessage(MSG_GET_CONTACTS_NAME);
    }

    private class VoiceHandler extends Handler {
        public VoiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            switch (msg.what) {
            case MSG_GET_CONTACTS_NAME: {
                sendToMainHandler();
            }
                break;
            default:
                break;
            }
        }
    }

    /*
     * Query all contact name from contact table
     * 
     * @return
     */
    public String[] getContactsNames() {

        String columnsInbox[] = new String[] { Contacts.DISPLAY_NAME };
        Cursor cursor = null;
        try {
            ArrayList<String> contactsList = new ArrayList<String>();
            cursor = mContext.getContentResolver().query(CONTACTS_URI,
                    columnsInbox, null, null, Contacts.SORT_KEY_PRIMARY);
            if (cursor != null && cursor.moveToFirst()) {
                int count = cursor.getCount();
                Log.d(VoiceCommandManagerStub.TAG,
                        "getContactsNames cursor count = " + count);
                String contact;
                for (int i = 0; i < count; i++) {
                    contact = cursor.getString(cursor
                            .getColumnIndex(Contacts.DISPLAY_NAME));
                    if (!TextUtils.isEmpty(contact)) {
                        contactsList.add(contact);
                    }
                    cursor.moveToNext();
                }
            }
            return contactsList.toArray(new String[contactsList.size()]);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void sendToMainHandler() {

        if (mainHandler
                .hasMessages(VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME)) {
            mainHandler
                    .removeMessages(VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME);
        }
        // Query database after remove the contacts msg of main handler
        String[] contactsNames = getContactsNames();
        Message msg = mainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME;
        msg.obj = contactsNames;
        mainHandler.sendMessage(msg);
//        mEndTime = System.currentTimeMillis();
//        Log.i(VoiceCommandManagerStub.TAG, "query spends time : "
//                + (mEndTime - mStartTime) + "ms" + ", thread id : "
//                + Thread.currentThread().getId());
    }
}
