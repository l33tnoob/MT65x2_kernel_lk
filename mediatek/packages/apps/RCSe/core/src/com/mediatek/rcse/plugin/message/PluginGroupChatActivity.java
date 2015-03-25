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

package com.mediatek.rcse.plugin.message;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.ParcelUuid;
import android.view.View.OnClickListener;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.widgets.ChatScreenWindowContainer;
import com.mediatek.rcse.activities.widgets.PhotoLoaderManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.Utils;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;

import java.util.List;
import java.util.TreeSet;

/**
 * The activity is a container which contains many chat fragments.
 */
public class PluginGroupChatActivity extends ChatScreenActivity implements
        PhotoLoaderManager.OnPhotoChangedListener {
    public static final String TAG = "PluginGroupChatActivity";
    public static final String ACTION = "com.mediatek.rcse.action.PLUGIN_GROUP_CHAT";
    private TreeSet<String> mKey = new TreeSet<String>();
    private AsyncListener mCurrentListener = null;

    @Override
    protected void onCreate(Bundle savedInstance) {
        super.onCreate(savedInstance);
        PhotoLoaderManager.addListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        PhotoLoaderManager.removeListener(this);
    }

    private class AsyncListener implements OnLoadImageFinishListener {
        private Object mKey = null;

        AsyncListener(Object key) {
            mKey = key;
        }

        @Override
        public void onLoadImageFished(Bitmap image) {
            Logger.d(TAG, "onLoadImageFished() entry");
            getActionBar().setIcon(new BitmapDrawable(image));
            if (this == mCurrentListener) {
                mCurrentListener = null;
            }
        }

        public void destroy() {
            ImageLoader.interrupt(mKey);
        }
    }

    protected void handleIntentAction(Intent intent) {
        Logger.d(TAG, "handleIntentAction() entry with the intent is " + intent);
        Bundle data = intent.getExtras();
        Logger.d(TAG, "handleIntentAction() ,the data is " + data);
        boolean isInvitation = checkInvitation(intent);
        if (isInvitation) {
            boolean isGroup = intent.getBooleanExtra(Utils.IS_GROUP_CHAT, false);
            if (isGroup) {
                newChatTag(intent);
            }
            boolean result = ModelImpl.getInstance().handleInvitation(intent, false);
            if (result) {
                mChatWindowManager.connect(true);
            }
            Logger.d(TAG, "handleIntentAction(), isGroup: " + isGroup + " result: " + result);
        } else {
            if (data != null) {
                if (data.containsKey(ChatMainActivity.KEY_ADD_CONTACTS)
                        && data.containsKey(Participant.KEY_PARTICIPANT_LIST)) {
                    ParcelUuid usedChatTag = intent.getParcelableExtra(KEY_USED_CHATTAG);
                    Logger.v(TAG, "handleIntentAction() usedChatTag: " + usedChatTag);
                    if (usedChatTag == null) {
                        addChatWindow(intent);
                    } else {
                        focusOnChatByTag(usedChatTag);
                    }
                } else if (data.containsKey(ChatScreenActivity.KEY_CHAT_TAG)) {
                    // Open an exist chat window.
                    Logger.v(TAG, "handleIntentAction() Open an exist chat window");
                    focusOnChatByTag(data.get(ChatScreenActivity.KEY_CHAT_TAG));
                } else {
                    Logger.e(TAG, "handleIntentAction() data is null!");
                }
            }
        }
        Logger.d(TAG, "handleIntentAction() exit");
    }

    public void addGroupChatUi(GroupChatFragment chatFragment, OnClickListener clickListener) {
        super.addGroupChatUi(chatFragment, clickListener);
        List<Participant> paritcipants = chatFragment.getParticipants();
        if (paritcipants == null || paritcipants.isEmpty()) {
            paritcipants = ChatScreenWindowContainer.getInstance().getCurrentParticipants();
        }
        updateParticipants(paritcipants);
    }

    private Bitmap requestImage() {
        Logger.d(TAG, "requestImage() entry mKey is " + mKey);
        if (mCurrentListener == null) {
            mCurrentListener = new AsyncListener(mKey);
        }
        return ImageLoader.requestImage(mKey, mCurrentListener);
    }

    @Override
    public void onPhotoChanged() {
        Logger.d(TAG, "onPhotoChanged() entry");
        final Bitmap bitMap = requestImage();
        if (bitMap != null) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    getActionBar().setIcon(new BitmapDrawable(bitMap));
                }
            });
        } else {
            Logger.d(TAG, "onPhotoChanged bitMap is null");
        }
    }

    public void updateParticipants(List<Participant> participantList) {
        Logger.d(TAG, "updateParticipants() entry participantList is " + participantList);
        if (participantList == null) {
            Logger.d(TAG, "updateParticipants() participantList is null");
            return;
        }
        TreeSet<String> nums = new TreeSet<String>();
        for (Participant participant : participantList) {
            nums.add(participant.getContact());
        }
        mKey = nums;
        Bitmap bitMap = requestImage();
        if (bitMap != null) {
            getActionBar().setIcon(new BitmapDrawable(bitMap));
        } else {
            Logger.d(TAG, "updateParticipants() bitMap is null");
        }
    }
}
