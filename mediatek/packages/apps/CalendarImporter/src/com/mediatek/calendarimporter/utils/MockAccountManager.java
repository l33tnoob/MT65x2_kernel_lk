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
package com.mediatek.calendarimporter.utils;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.AccountManagerCallback;
import android.accounts.AccountManagerFuture;
import android.content.Context;
import android.os.Handler;

/**
 * A mock {@link AccountManager} class, just for test.
 * 
 * Where ever use this class, must use
 * AccountManager.get(getApplicationContext()), but not AccountManager.get(this)
 * or AccountManager.get(context) and so on, because just this, the
 * AccountManager.get will call back to CalendarApplication.class
 * 
 * @param context this context must is ApplicationContext! Get by
 *                getApplicationContext() system function.
 */
public class MockAccountManager extends AccountManager{

    public Account[] mAccounts;

    public MockAccountManager(Context context, Account[] accounts) {
        super(context, null);
        this.mAccounts = accounts;
    }

    /**
     * Use this like AccountManager.get(getApplicationContext()).getAccounts()
     * to get accounts.
     */
    public Account[] getAccountsByType(String type) {
        if (mAccounts == null) {
            mAccounts = new Account[0];
        }
        return mAccounts;
    }

    public Account[] getAccounts() {
        if (mAccounts == null) {
            mAccounts = new Account[0];
        }
        return mAccounts;
    }
    /**
     * For test to clear the test accounts.
     */
    public void removeAccount() {
        mAccounts = null;
    }
}
