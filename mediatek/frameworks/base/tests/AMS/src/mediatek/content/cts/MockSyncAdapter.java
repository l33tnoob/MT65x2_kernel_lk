/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package android.content.cts;

import android.accounts.Account;
import android.content.ContentResolver;
import android.content.ISyncAdapter;
import android.content.ISyncContext;
import android.os.Bundle;
import android.os.RemoteException;

import java.util.concurrent.CountDownLatch;

public class MockSyncAdapter extends ISyncAdapter.Stub {

    private static MockSyncAdapter sSyncAdapter = null;

    private Account mAccount;
    private String mAuthority;
    private Bundle mExtras;
    private boolean mInitialized;
    private boolean mStartSync;
    private boolean mCancelSync;
    private CountDownLatch mLatch;

    public Account getAccount() {
        return mAccount;
    }

    public String getAuthority() {
        return mAuthority;
    }

    public Bundle getExtras() {
        return mExtras;
    }

    public boolean isInitialized() {
        return mInitialized;
    }

    public boolean isStartSync() {
        return mStartSync;
    }

    public boolean isCancelSync() {
        return mCancelSync;
    }

    public void clearData() {
        mAccount = null;
        mAuthority = null;
        mExtras = null;
        mInitialized = false;
        mStartSync = false;
        mCancelSync = false;
        mLatch = null;
    }

    public void setLatch(CountDownLatch mLatch) {
        this.mLatch = mLatch;
    }

    public void startSync(ISyncContext syncContext, String authority, Account account,
            Bundle extras) throws RemoteException {

        mAccount = account;
        mAuthority = authority;
        mExtras = extras;

        if (null != extras && extras.getBoolean(ContentResolver.SYNC_EXTRAS_INITIALIZE)) {
            mInitialized = true;
            mStartSync = false;
            mCancelSync = false;
        } else {
            mInitialized = false;
            mStartSync = true;
            mCancelSync = false;
        }

        if (null != mLatch) {
            mLatch.countDown();
        }
    }

    public void cancelSync(ISyncContext syncContext) throws RemoteException {
        mAccount = null;
        mAuthority = null;
        mExtras = null;

        mInitialized = false;
        mStartSync = false;
        mCancelSync = true;

        if (null != mLatch) {
            mLatch.countDown();
        }
    }

    public void initialize(android.accounts.Account account, java.lang.String authority)
            throws android.os.RemoteException {

        mAccount = account;
        mAuthority = authority;

        mInitialized = true;
        mStartSync = false;
        mCancelSync = false;

        if (null != mLatch) {
            mLatch.countDown();
        }
    }

    public static MockSyncAdapter getMockSyncAdapter() {
        if (null == sSyncAdapter) {
            sSyncAdapter = new MockSyncAdapter();
        }
        return sSyncAdapter;
    }
}
