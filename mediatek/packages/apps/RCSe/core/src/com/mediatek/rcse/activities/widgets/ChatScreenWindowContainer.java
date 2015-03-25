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

package com.mediatek.rcse.activities.widgets;

import android.os.ParcelUuid;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;

import java.util.HashMap;
import java.util.List;

/**
 * The container of windows used in ChatScreenActivity.
 */
public final class ChatScreenWindowContainer {
    private static final String TAG = "ChatScreenWindowContainer";
    private static ChatScreenWindowContainer sChatScreenWindowContainer = null;
    private ChatScreenWindow mCurrentWindow = null;
    private ParcelUuid mCurrentTag = null;
    private List<Participant> mCurrentParticipants = null;
    private HashMap<ParcelUuid, ChatScreenWindow> mWindows =
            new HashMap<ParcelUuid, ChatScreenWindow>();

    private ChatScreenWindowContainer() {

    }

    /**
     * Get the instance of ChatScreenWindowContainer.
     * 
     * @return The instance.
     */
    public static ChatScreenWindowContainer getInstance() {
        if (sChatScreenWindowContainer == null) {
            sChatScreenWindowContainer = new ChatScreenWindowContainer();
        }
        return sChatScreenWindowContainer;
    }

    /**
     * Set focus window.
     * 
     * @param window The window to be focused.
     */
    public void setFocusWindow(ChatScreenWindow window) {
        mCurrentWindow = window;
    }

    /**
     * Focus one window with participants. This method is used when window is
     * not already been created, and since that time we only know participants.
     * 
     * @param participants The participants of the window to be focused.
     */
    public void focus(List<Participant> participants) {
        Logger.d(TAG, "focus participants: " + participants);
        mCurrentParticipants = participants;
    }

    /**
     * Focus one window with tag. This method is used when window is not already
     * been created, and since that time we only know tag.
     * 
     * @param tag The tag of the window to be focused.
     */
    public void focus(ParcelUuid tag) {
        Logger.d(TAG, "focus tag: " + tag);
        mCurrentTag = tag;
    }

    /**
     * Get the focused window.
     * 
     * @return The focused window
     */
    public ChatScreenWindow getFocusWindow() {
        return mCurrentWindow;
    }

    /**
     * Get the tag of the focused window.
     * 
     * @return The tag of the focused window.
     */
    public ParcelUuid getCurrentTag() {
        return mCurrentTag;
    }

    /**
     * Get the participants of the focused window.
     * 
     * @return The participants of the focused window.
     */
    public List<Participant> getCurrentParticipants() {
        return mCurrentParticipants;
    }

    /**
     * Add one window with tag.
     * 
     * @param tag The tag of the window.
     * @param window The window associated with the tag.
     */
    public void addWindow(ParcelUuid tag, ChatScreenWindow window) {
        mWindows.put(tag, window);
    }

    /**
     * Get the window with tag.
     * 
     * @param tag The tag that can identify one window.
     * @return The window with the tag.
     */
    public ChatScreenWindow getWindow(ParcelUuid tag) {
        return mWindows.get(tag);
    }

    /**
     * Clear current window's status.
     */
    public void clearCurrentStatus() {
        Logger.v(TAG, "clearCurrentStatus entry, the tag is: " + mCurrentTag);
        mCurrentWindow = null;
        mCurrentTag = null;
        mCurrentParticipants = null;
        Logger.v(TAG, "clearCurrentStatus exit");
    }
}
