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
import android.accounts.AccountManager;
import android.accounts.AccountManagerCallback;
import android.accounts.AccountManagerFuture;
import android.accounts.AuthenticatorDescription;
import android.accounts.AuthenticatorException;
import android.accounts.OnAccountsUpdateListener;
import android.accounts.OperationCanceledException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.SyncAdapterType;
import android.content.SyncStatusObserver;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.oobe.R;
import com.mediatek.oobe.utils.OOBEStepPreferenceActivity;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;

class AccountPreferenceBase extends OOBEStepPreferenceActivity implements OnAccountsUpdateListener {
    protected static final String TAG = "AccountPreferenceBase";
    public static final String AUTHORITIES_FILTER_KEY = "authorities";

    private Map<String, AuthenticatorDescription> mTypeToAuthDescription = new HashMap<String, AuthenticatorDescription>();

    protected Context mContext;
    protected AuthenticatorDescription[] mAuthDescs;
    private final Handler mHandler = new Handler();
    private Object mStatusChangeListenerHandle;
    private HashMap<String, ArrayList<String>> mAccountTypeToAuthorities = null;
    // authority passed in by intent as a filter
    protected String[] mAuthorities;
    protected String[] mSupportedType;
    protected String[] mUnSupportedType;

    protected PreferenceCategory mAccountListCategory;
    private Account[] mAccounts;
    private ArrayList<String> mUnSNSType = new ArrayList<String>();

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mUnSNSType.add("com.android.exchange");
    }

    /**
     * Overload to handle account updates.
     */
    @Override
    public void onAccountsUpdated(Account[] accounts) {
        Xlog.v(TAG, "onAccountsUpdated()");
        mAccounts = accounts;
        if (accounts == null) {
            Xlog.w(TAG, "No account to show");
            return;
        }

        updatePage();
    }

    /**
     * Overload to handle authenticator description updates
     */
    protected void onAuthDescriptionsUpdated() {
        updatePage();
    }

    /**
     * Overload to handle sync state updates.
     */
    protected void onSyncStateUpdated() {
        Xlog.v(TAG, "onSyncStateUpdated()");
        // Set background connection state
        ConnectivityManager connManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        boolean backgroundDataSetting = connManager.getBackgroundDataSetting();
        boolean masterSyncAutomatically = ContentResolver.getMasterSyncAutomatically();

        // // iterate over all the preferences, setting the state properly for each
        // SyncInfo currentSync= ContentResolver.getCurrentSync();
        //
        // boolean anySyncFailed = false; // true if sync on any account failed

        // only track userfacing sync adapters when deciding if account is synced or not
        final SyncAdapterType[] syncAdapters = ContentResolver.getSyncAdapterTypes();
        HashSet<String> userFacing = new HashSet<String>();
        for (int k = 0, n = syncAdapters.length; k < n; k++) {
            final SyncAdapterType sa = syncAdapters[k];
            if (sa.isUserVisible()) {
                userFacing.add(sa.authority);
            }
        }
        if (mAccountListCategory == null) {
            Xlog.w(TAG, "No account till now, no need to update, return");
            return;
        }
        for (int i = 0, count = mAccountListCategory.getPreferenceCount(); i < count; i++) {
            Preference pref = mAccountListCategory.getPreference(i);
            if (!(pref instanceof AccountPreference)) {
                continue;
            }

            AccountPreference accountPref = (AccountPreference) pref;
            Account account = accountPref.getAccount();
            int syncCount = 0;
            boolean syncIsFailing = false;
            final ArrayList<String> authorities = accountPref.getAuthorities();
            if (authorities != null) {
                for (String authority : authorities) {
                    // SyncStatusInfo status = ContentResolver.getSyncStatus(account, authority);
                    boolean syncEnabled = ContentResolver.getSyncAutomatically(account, authority)
                            && masterSyncAutomatically && backgroundDataSetting
                            && (ContentResolver.getIsSyncable(account, authority) > 0);

                    syncCount += syncEnabled && userFacing.contains(authority) ? 1 : 0;
                }
            } else {
                if (Log.isLoggable(TAG, Log.VERBOSE)) {
                    Xlog.v(TAG, "no syncadapters found for " + account);
                }
            }
            int syncStatus = AccountPreference.SYNC_DISABLED;
            if (syncIsFailing) {
                syncStatus = AccountPreference.SYNC_ERROR;
            } else if (syncCount == 0) {
                syncStatus = AccountPreference.SYNC_DISABLED;
            } else if (syncCount > 0) {
                syncStatus = AccountPreference.SYNC_ENABLED;
            }
            accountPref.setSyncStatus(syncStatus);
        }

    }

    /**
     * when authenticator provider or accounts change, refresh this page
     */
    private void updatePage() {
        Xlog.v(TAG, "updatePage()");
        mAccountListCategory = null;
        getPreferenceScreen().removeAll();
        addProvider();
        addAccountPreference();
    }

    private void addProvider() {
        Xlog.v(TAG, getStepSpecialTag() + " --> addProvider()");
        // Create list of providers to show on preference screen
        for (int i = 0; i < mAuthDescs.length; i++) {
            String accountType = mAuthDescs[i].type;
            boolean addAccountPref = true;
            // if this is a SNS account, and such account has already be added, then just hide this account
            if (!mUnSNSType.contains(accountType) && mAccounts != null) {
                for (int j = 0, n = mAccounts.length; j < n; j++) {
                    final Account account = mAccounts[j];
                    if (account != null && account.type.equals(accountType)) {
                        Xlog.v(TAG, "Provider type[" + accountType
                                + "] already have an account added, just hide this provider");
                        addAccountPref = false;
                        break;
                    }
                }
            }
            if (!addAccountPref) {
                continue;
            }
            CharSequence providerName = getLabelForType(accountType);
            // Skip preferences for authorities not specified. If no authorities specified,
            // then include them all.
            ArrayList<String> accountAuths = getAuthoritiesForAccountType(accountType);
            Xlog.v(TAG, getStepSpecialTag() + " ###  type=" + accountType);
            if (mAuthorities != null && mAuthorities.length > 0 && accountAuths != null) {
                addAccountPref = false;
                for (int k = 0; k < mAuthorities.length; k++) {
                    if (accountAuths.contains(mAuthorities[k])) {
                        addAccountPref = true;
                        break;
                    }
                }
            }
            if (addAccountPref && !shouldShowType(accountType)) {
                addAccountPref = false;
            }
            if (addAccountPref) {
                Drawable drawable = getDrawableForType(accountType);
                ProviderPreference pref = new ProviderPreference(this, accountType, drawable, providerName);
                getPreferenceScreen().addPreference(pref);
            }
        }
    }

    private void addAccountPreference() {
        Xlog.v(TAG, "addAccountPreference()");

        if (mAccounts == null) {
            Xlog.w(TAG, "No account till now, return");
            return;
        }

        Xlog.w(TAG, "mAccounts=" + mAccounts.length);
        if (mAccountListCategory == null) {
            mAccountListCategory = new PreferenceCategory(this);
            mAccountListCategory.setKey("account_list_category");
            mAccountListCategory.setTitle(R.string.oobe_account_list_title);
        }

        if (mAccounts.length == 0) {
            PreferenceScreen prefScreen = getPreferenceScreen();
            if (prefScreen != null) {
                prefScreen.removePreference(mAccountListCategory);
            }
        } else {
            PreferenceScreen prefScreen = getPreferenceScreen();
            if (prefScreen != null) {
                prefScreen.addPreference(mAccountListCategory);
            }
        }
        // re-fill account category
        mAccountListCategory.removeAll();

        for (int i = 0, n = mAccounts.length; i < n; i++) {
            final Account account = mAccounts[i];
            final ArrayList<String> auths = getAuthoritiesForAccountType(account.type);

            boolean showAccount = true;
            if (mAuthorities != null && auths != null) {
                showAccount = false;
                for (String requestedAuthority : mAuthorities) {
                    if (auths.contains(requestedAuthority)) {
                        showAccount = true;
                        break;
                    }
                }
            }
            if (showAccount && !shouldShowType(account.type)) {
                showAccount = false;
            }

            if (showAccount) {
                Drawable icon = getDrawableForType(account.type);
                AccountPreference preference = new AccountPreference(this, account, icon, auths);
                mAccountListCategory.addPreference(preference);
            }
        }
        onSyncStateUpdated();
    }

    /**
     * Judge whether we should show a type, if supported list is not null and this type is among it, show else if this type
     * is contained in unsupported list, exclude this type
     * 
     * @param currType
     * @return
     */
    private boolean shouldShowType(String currType) {
        if (currType == null || currType.equals("")) {
            return false;
        }
        boolean result = true;
        if (mSupportedType != null) {
            result = false;
            // whether it's among supported type array
            for (String type : mSupportedType) {
                if (currType.equals(type)) {
                    result = true;
                    break;
                }
            }
        } else if (mUnSupportedType != null) {
            for (String type : mUnSupportedType) {
                if (currType.equals(type)) {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    @Override
    protected void onResume() {
        super.onResume();
        mStatusChangeListenerHandle = ContentResolver.addStatusChangeListener(ContentResolver.SYNC_OBSERVER_TYPE_ACTIVE
        // | ContentResolver.SYNC_OBSERVER_TYPE_STATUS
                | ContentResolver.SYNC_OBSERVER_TYPE_SETTINGS, mSyncStatusObserver);
        onSyncStateUpdated();
    }

    @Override
    protected void onPause() {
        super.onPause();
        ContentResolver.removeStatusChangeListener(mStatusChangeListenerHandle);
    }

    private SyncStatusObserver mSyncStatusObserver = new SyncStatusObserver() {
        @Override
        public void onStatusChanged(int which) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    onSyncStateUpdated();
                }
            });
        }
    };

    public ArrayList<String> getAuthoritiesForAccountType(String type) {
        if (mAccountTypeToAuthorities == null) {
            // mAccountTypeToAuthorities = Maps.newHashMap();
            mAccountTypeToAuthorities = new HashMap<String, ArrayList<String>>();
            SyncAdapterType[] syncAdapters = ContentResolver.getSyncAdapterTypes();
            for (int i = 0, n = syncAdapters.length; i < n; i++) {
                final SyncAdapterType sa = syncAdapters[i];
                ArrayList<String> authorities = mAccountTypeToAuthorities.get(sa.accountType);
                if (authorities == null) {
                    authorities = new ArrayList<String>();
                    mAccountTypeToAuthorities.put(sa.accountType, authorities);
                }
                Xlog.d(TAG, "added authority " + sa.authority + " to accountType " + sa.accountType);

                authorities.add(sa.authority);
            }
        }
        return mAccountTypeToAuthorities.get(type);
    }

    /**
     * Gets an icon associated with a particular account type. If none found, return null.
     * 
     * @param accountType
     *            the type of account
     * @return a drawable for the icon or null if one cannot be found.
     */
    protected Drawable getDrawableForType(final String accountType) {
        Drawable icon = null;
        if (mTypeToAuthDescription.containsKey(accountType)) {
            try {
                AuthenticatorDescription desc = mTypeToAuthDescription.get(accountType);
                Context authContext = createPackageContext(desc.packageName, 0);
                icon = authContext.getResources().getDrawable(desc.iconId);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.w(TAG, "No icon for account type " + accountType);
            }
        }
        return icon;
    }

    /**
     * Gets the label associated with a particular account type. If none found, return null.
     * 
     * @param accountType
     *            the type of account
     * @return a CharSequence for the label or null if one cannot be found.
     */
    protected CharSequence getLabelForType(final String accountType) {
        CharSequence label = null;
        if (mTypeToAuthDescription.containsKey(accountType)) {
            try {
                AuthenticatorDescription desc = mTypeToAuthDescription.get(accountType);
                Context authContext = createPackageContext(desc.packageName, 0);
                label = authContext.getResources().getText(desc.labelId);
            } catch (PackageManager.NameNotFoundException e) {
                Xlog.w(TAG, "No label for account type " + ", type " + accountType);
            }
        }
        return label;
    }

    /**
     * Gets the preferences.xml file associated with a particular account type.
     * 
     * @param accountType
     *            the type of account
     * @return a PreferenceScreen inflated from accountPreferenceId.
     */
    // protected PreferenceScreen addPreferencesForType(final String accountType) {
    // PreferenceScreen prefs = null;
    // if (mTypeToAuthDescription.containsKey(accountType)) {
    // AuthenticatorDescription desc = null;
    // try {
    // desc = (AuthenticatorDescription) mTypeToAuthDescription.get(accountType);
    // if (desc != null && desc.accountPreferencesId != 0) {
    // Context authContext = createPackageContext(desc.packageName, 0);
    // prefs = getPreferenceManager().inflateFromResource(authContext,
    // desc.accountPreferencesId, getPreferenceScreen());
    // }
    // } catch (PackageManager.NameNotFoundException e) {
    // Xlog.w(TAG, "Couldn't load preferences.xml file from " + desc.packageName);
    // }
    // }
    // return prefs;
    // }

    /**
     * Updates provider icons. Subclasses should call this in onCreate() and update any UI that depends on
     * AuthenticatorDescriptions in onAuthDescriptionsUpdated().
     */
    protected void updateAuthDescriptions() {
        mAuthDescs = AccountManager.get(this).getAuthenticatorTypes();
        for (int i = 0; i < mAuthDescs.length; i++) {
            mTypeToAuthDescription.put(mAuthDescs[i].type, mAuthDescs[i]);
        }
        onAuthDescriptionsUpdated();
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Xlog.d(TAG, "$$$ preference is clicked");
        if (preference instanceof ProviderPreference) {
            ProviderPreference pref = (ProviderPreference) preference;
            Xlog.v(TAG, "Attempting to add account of type " + pref.getAccountType());
            addAccount(pref.getAccountType());
        }
        return false;
    }

    /**
     * click on a provider type will add a corresponding account
     * 
     * @param accountType
     */
    protected void addAccount(String accountType) {
        if (mContext != null) {
            AccountManager.get(mContext).addAccount(accountType, null, /* authTokenType */
            null, /* requiredFeatures */
            null, /* addAccountOptions */
            null, mCallback, null /* handler */);
        } else {
            Xlog.w(TAG, "Click provider, try to add an account, but context is null");
        }
    }

    private AccountManagerCallback<Bundle> mCallback = new AccountManagerCallback<Bundle>() {
        @Override
        public void run(AccountManagerFuture<Bundle> future) {
            try {
                Bundle bundle = future.getResult();
                Intent intent = bundle.getParcelable(AccountManager.KEY_INTENT);
                if (intent != null) {
                    mContext.startActivity(intent);

                } else {
                    String errMsg = bundle.getString(AccountManager.KEY_ERROR_MESSAGE);
                    if (errMsg != null) {
                        Toast.makeText(mContext, errMsg, Toast.LENGTH_SHORT).show();
                    }
                }
                Xlog.v(TAG, "account added: " + bundle);
            } catch (OperationCanceledException e) {
                Xlog.e(TAG, "addAccount was canceled");
            } catch (IOException e) {
                Xlog.e(TAG, "addAccount failed: " + e);
            } catch (AuthenticatorException e) {
                Xlog.e(TAG, "addAccount failed: " + e);
            }
        }
    };

}
