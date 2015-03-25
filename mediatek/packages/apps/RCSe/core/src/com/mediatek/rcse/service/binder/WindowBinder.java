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

package com.mediatek.rcse.service.binder;

import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.text.TextUtils;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ControllerImpl;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ViewImpl;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * This class is used for message plugin IPC
 */
public class WindowBinder extends IRemoteWindowBinder.Stub {

    public static final String TAG = "WindowBinder";
    private static final String REMOTE_CONTROLLER_THREAD_NAME = "Remote Controller";
    public static final String REMOTE_KEY_DATA = "data";
    public static final String REMOTE_KEY_TAG = "tag";
    public static final String REMOTE_KEY_CONTACT = "contact";
    List<ChatWindowManagerAdapter> mChatWindowManagerAdapters = new CopyOnWriteArrayList<ChatWindowManagerAdapter>();
    private Messenger mRemoteMessenger = null; 
    private HandlerThread mHandlerThread = null;

    @Override
    public void addChatWindowManager(IRemoteChatWindowManager chatWindowManager, boolean isAutoShow)
            throws RemoteException {
        Logger.d(TAG, "addChatWindowManager() entry! chatWindowManager = " + chatWindowManager);
        ChatWindowManagerAdapter chatWindowManagerAdapter = new ChatWindowManagerAdapter(chatWindowManager);
        ViewImpl.getInstance().addChatWindowManager(chatWindowManagerAdapter, isAutoShow);
        if (mChatWindowManagerAdapters != null) {
            mChatWindowManagerAdapters.add(chatWindowManagerAdapter);
        } else {
            Logger.w(TAG, "addChatWindowManager(), mChatWindowManagerAdapters is null or mChatWindowManagers is null!");
        }
    }

    @Override
    public void removeChatWindowManager(IRemoteChatWindowManager chatWindowManager) throws RemoteException {
        Logger.d(TAG, "removeChatWindowManager()");
        if (mChatWindowManagerAdapters != null) {
            for (ChatWindowManagerAdapter chatWindowManagerAdapter : mChatWindowManagerAdapters) {
                IRemoteChatWindowManager windowManager = chatWindowManagerAdapter.getChatWindowManager();
                if (windowManager.equals(chatWindowManager)) {
                    ViewImpl.getInstance().removeChatWindowManager(chatWindowManagerAdapter);
                    mChatWindowManagerAdapters.remove(chatWindowManagerAdapter);
                } else {
                    Logger.w(TAG, "removeChatWindowManager(), not the same, chatWindowManagerAdapter = "
                            + chatWindowManagerAdapter);
                }
            }
        } else {
            Logger.w(TAG, "addChatWindowManager(), mChatWindowManagerAdapters is null!");
        }
    }

    @Override
    public IBinder getController() {
        if (null == mRemoteMessenger) {
            mHandlerThread = new HandlerThread(REMOTE_CONTROLLER_THREAD_NAME);
            mHandlerThread.start();
            mRemoteMessenger = new Messenger(new MessageAdapter(mHandlerThread.getLooper()));
        }
        return mRemoteMessenger.getBinder();
    }

    /**
     * This class will help to translate a remote message to be a local message
     */
    private static class MessageAdapter extends Handler {
        private static final String TAG = MessageAdapter.class.getSimpleName();

        private MessageAdapter(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Logger.d(TAG, "handleMessage() receive message: " + msg);
            Bundle bundle = msg.getData();
            Object data = bundle.get(REMOTE_KEY_DATA);
            Logger.d(TAG, "handleMessage() will obtain a local message" +
                    " - eventType: " + msg.what + ", data: " + data);
            Message localMessage = null;
            if (bundle.containsKey(REMOTE_KEY_TAG)) {
                Logger.d(TAG, "handleMessage() remote tag string found");
                String remoteString = bundle.getString(REMOTE_KEY_TAG);
                if (TextUtils.isEmpty(remoteString)) {
                    Logger.w(TAG, "handleMessage() invalid remoteString: " + remoteString);
                    return;
                }
                Object tag = TagTranslater.translateTag(remoteString);
                if (tag != null) {
                    Logger.d(TAG, "handleMessage() tag for remoteString: " + remoteString + " found: " + tag);
                } else {
                    Logger.w(TAG, "handleMessage() unable to find tag for remoteString: " + remoteString);
                    tag = remoteString;
                }
                localMessage = ControllerImpl.getInstance().obtainMessage(msg.what, tag, data);
            } else if (bundle.containsKey(REMOTE_KEY_CONTACT)) {
                String contact = bundle.getString(REMOTE_KEY_CONTACT);
                Logger.d(TAG, "handleMessage() contact found: " + contact);
                localMessage = ControllerImpl.getInstance().obtainMessage(msg.what, contact, data);
            } else {
                Logger.d(TAG, "handleMessage() unable to find a tag or contact");
                localMessage = ControllerImpl.getInstance().obtainMessage(msg.what, null, data);
            }
            if (bundle.containsKey(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG)) {
                Logger.d(TAG, "handleMessage() add file transfer tag!");
                localMessage.getData().putParcelable(
                        ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG,
                        bundle.getParcelable(ModelImpl.SentFileTransfer.KEY_FILE_TRANSFER_TAG));
            }
            localMessage.arg1 = msg.arg1;
            localMessage.arg2 = msg.arg2;
            Logger.d(TAG, "handleMessage() generate local message: " + msg);
            localMessage.sendToTarget();
        }
    }
}
