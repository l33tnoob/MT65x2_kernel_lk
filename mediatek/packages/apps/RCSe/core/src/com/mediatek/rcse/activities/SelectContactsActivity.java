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

package com.mediatek.rcse.activities;

import android.app.ActionBar;
import android.app.Activity;
import android.app.FragmentManager;
import android.content.Intent;
import android.os.Bundle;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.service.PluginApiManager;

import java.util.ArrayList;

/**
 * Demonstration of the use of a CursorLoader to load and display contacts data
 * in a fragment.
 */
public class SelectContactsActivity extends Activity {
    private static final String TAG = "SelectContactsActivity";

    public static final String KEY_IS_NEED_ORIGINAL_CONTACTS = "is_need_original_contacts";
    ContactsListFragment mContactsListFragment = null;
    String mCallBackActivity;
    ArrayList<Participant> mParticipantList = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActionBar().setDisplayOptions(ActionBar.NAVIGATION_MODE_STANDARD);
        FragmentManager fm = getFragmentManager();
        Intent intent = getIntent();
        mCallBackActivity = intent.getStringExtra(ChatMainActivity.KEY_ADD_CONTACTS);
        mParticipantList =
            intent.getParcelableArrayListExtra(
                        ChatScreenActivity.KEY_EXSITING_PARTICIPANTS);
        mContactsListFragment = new ContactsListFragment();
        if (mParticipantList != null) {
            Logger.d(TAG, "onCreate() the mParticipantlist size is " + mParticipantList.size());
            mContactsListFragment.setExistingContacts(mParticipantList);
            mContactsListFragment.setIsNeedOriginalContacts(intent.getBooleanExtra(KEY_IS_NEED_ORIGINAL_CONTACTS, false));
        } else {
            Logger.d(TAG, "onCreate() the mParticipantlist is null");
        }
        mContactsListFragment.startSelectMode(mCallBackActivity);
        String action = intent.getAction();
        if (null != action && action.equals(PluginApiManager.RcseAction.SELECT_CONTACT_ACTION)) {
            mContactsListFragment.setUsedForFileTransfer(true);
        } else {
            Logger.d(TAG, "onCreate() the action is null");
            mContactsListFragment.setUsedForFileTransfer(false);
        }

        fm.beginTransaction().add(android.R.id.content, mContactsListFragment).commit();
    }
}
