/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.oobe.advanced;

import android.accounts.Account;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.preference.Preference;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.oobe.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

/**
 * AccountPreference is used to display a username, status and provider icon for an account on the device.
 */
public class AccountPreference extends Preference {
    private static final String TAG = "AccountPreference";
    public static final int SYNC_ENABLED = 0; // all know sync adapters are enabled and OK
    public static final int SYNC_DISABLED = 1; // no sync adapters are enabled
    public static final int SYNC_ERROR = 2; // one or more sync adapters have a problem
    private int mStatus;
    private Account mAccount;
    private ArrayList<String> mAuthorities;
    private Drawable mProviderIcon;
    private ImageView mSyncStatusIcon;
    private ImageView mProviderIconView;

    /**
     * constructor of AccountPreference
     * 
     * @param context Context of class
     * @param account user account
     * @param icon an icon
     * @param authorities a string array list.
     */
    public AccountPreference(Context context, Account account, Drawable icon, ArrayList<String> authorities) {
        super(context);
        mAccount = account;
        mAuthorities = authorities;
        mProviderIcon = icon;
        setLayoutResource(R.layout.account_preference);
        // setTitle(mAccount.name);
        setTitle(mAccount.type);
        setSummary(mAccount.name);
        // Add account info to the intent for AccountSyncSettings
        Intent intent = new Intent("android.settings.ACCOUNT_SYNC_SETTINGS");
        intent.putExtra("account", mAccount);
        setIntent(intent);
        setPersistent(false);
        setSyncStatus(SYNC_DISABLED);
    }

    /**
     * get user account
     * @return Account object
     */
    public Account getAccount() {
        return mAccount;
    }

    /**
     * get authorities list
     * @return an array list of Authorities
     */
    public ArrayList<String> getAuthorities() {
        return mAuthorities;
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        // setSummary(getSyncStatusMessage(mStatus));
        mProviderIconView = (ImageView) view.findViewById(R.id.providerIcon);
        mProviderIconView.setImageDrawable(mProviderIcon);
        mSyncStatusIcon = (ImageView) view.findViewById(R.id.syncStatusIcon);
        mSyncStatusIcon.setImageResource(getSyncStatusIcon(mStatus));
    }

    /**
     * set provider icon
     * @param icon drawable icon
     */
    public void setProviderIcon(Drawable icon) {
        mProviderIcon = icon;
        if (mProviderIconView != null) {
            mProviderIconView.setImageDrawable(icon);
        }
    }

    /**
     * set sync status
     * @param status int
     */
    public void setSyncStatus(int status) {
        mStatus = status;
        if (mSyncStatusIcon != null) {
            mSyncStatusIcon.setImageResource(getSyncStatusIcon(status));
        }
        // setSummary(getSyncStatusMessage(status));
    }

    private int getSyncStatusIcon(int status) {
        int res;
        switch (status) {
        case SYNC_ENABLED:
            res = R.drawable.ic_sync_green;
            break;
        case SYNC_DISABLED:
            res = R.drawable.ic_sync_grey;
            break;
        case SYNC_ERROR:
            res = R.drawable.ic_sync_red;
            break;
        default:
            res = R.drawable.ic_sync_red;
            Xlog.w(TAG, "Unknown sync status: " + status);
        }
        return res;
    }

    @Override
    public int compareTo(Preference other) {
        if (!(other instanceof AccountPreference)) {
            // Put other preference types above us
            return 1;
        }
        return mAccount.name.compareTo(((AccountPreference) other).mAccount.name);
    }
}
