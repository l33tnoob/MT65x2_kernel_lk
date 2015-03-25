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
 * Copyright (C) 2006 The Android Open Source Project
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

package com.mediatek.security.ui;

import android.app.ActionBar;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Parcelable;
import android.provider.Settings;
import android.support.v4.view.PagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.FrameLayout;
import android.widget.Switch;

import com.mediatek.security.R;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.ui.ActionBarAdapter.TabState;
import com.mediatek.xlog.Xlog;

/**
 * Activity to pick an application that will be used to display installation information and
 * options to uninstall/delete user data for system applications. This activity
 * can be launched through Settings or via the ACTION_MANAGE_PACKAGE_STORAGE
 * intent.
 */
public class PermissionControlPageActivity extends Activity implements ActionBarAdapter.Listener,
        CompoundButton.OnCheckedChangeListener {

    static final String TAG = "PermControlPageActivity";

    private static final int PERMISSIONS_INFO = 0;
    private static final int APPS_INFO = 1;    
    private static final int NUM_TABS = 2;

    private ViewPager mTabPager;
    private TabPagerAdapter mTabPagerAdapter;
    private ActionBarAdapter mActionBarAdapter;
    private final TabPagerListener mTabPagerListener = new TabPagerListener();

    final String mPermissionsTag = "tab-pager-perms";
    final String mAppsTag = "tab-pager-apps";
    private PermissionsFragment mPermissionsFragment;
    private AppsFragment mAppsFragment;

    private FrameLayout mEmptyView;
    private Switch mSwitch;

    private Bundle mSavedInstanceState;
    
    private boolean mUserCheckedFlag;
    
    private class TabPagerAdapter extends PagerAdapter {
        private final FragmentManager mFragmentManager;
        private FragmentTransaction mCurTransaction = null;
        private Fragment mCurrentPrimaryItem;

        public TabPagerAdapter() {
            mFragmentManager = getFragmentManager();
        }

        @Override
        public int getCount() {
            return NUM_TABS;
        }

        /** Gets called when the number of items changes. */
        @Override
        public int getItemPosition(Object object) {
            if (object == mPermissionsFragment) {
                return PERMISSIONS_INFO;
            }

            if (object == mAppsFragment) {
                return APPS_INFO;
            }
            return POSITION_NONE;
        }

        @Override
        public void startUpdate(View container) {
        }

        private Fragment getFragment(int position) {
            if (position == PERMISSIONS_INFO) {
                return mPermissionsFragment;
            } else if (position == APPS_INFO) {
                return mAppsFragment;
            }

            throw new IllegalArgumentException("position: " + position);
        }

        @Override
        public Object instantiateItem(View container, int position) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
            Fragment f = getFragment(position);
            return f;
        }

        @Override
        public void destroyItem(View container, int position, Object object) {
            if (mCurTransaction == null) {
                mCurTransaction = mFragmentManager.beginTransaction();
            }
        }

        @Override
        public void finishUpdate(View container) {
            if (mCurTransaction != null) {
                mCurTransaction.commitAllowingStateLoss();
                mCurTransaction = null;
                mFragmentManager.executePendingTransactions();
            }
        }

        @Override
        public boolean isViewFromObject(View view, Object object) {
            return ((Fragment) object).getView() == view;
        }

        @Override
        public void setPrimaryItem(View container, int position, Object object) {
            Fragment fragment = (Fragment) object;
            if (mCurrentPrimaryItem != fragment) {
                if (mCurrentPrimaryItem != null) {
                    mCurrentPrimaryItem.setUserVisibleHint(false);
                }
                if (fragment != null) {
                    fragment.setUserVisibleHint(true);
                }
                mCurrentPrimaryItem = fragment;
            }
        }

        @Override
        public Parcelable saveState() {
            return null;
        }

        @Override
        public void restoreState(Parcelable state, ClassLoader loader) {
        }
    }


    private class TabPagerListener implements ViewPager.OnPageChangeListener {
        @Override
        public void onPageScrollStateChanged(int state) {
        }

        @Override
        public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        }

        @Override
        public void onPageSelected(int position) {
                // Make sure not in the search mode, in which case position != TabState.ordinal().
                TabState selectedTab = TabState.fromInt(position);
                mActionBarAdapter.setCurrentTab(selectedTab, false);
                PermissionControlPageActivity.this.updateFragmentsVisibility();
        }
    }
    
    /**
     * add content obsever to connect with other permission management app
     * */
    private final UiUtils.SwitchContentObserver mSwitchContentObserver = new UiUtils.SwitchContentObserver(
            new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            checkUiEnabled();
        }
    };
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.permission_control_pages);
        mEmptyView = (FrameLayout)findViewById(R.id.empty_view);
        mSavedInstanceState = savedInstanceState;
        // add the switch on Action bar
        LayoutInflater inflater = (LayoutInflater)
              getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mSwitch = (Switch) inflater.inflate(com.mediatek.internal.R.layout.imageswitch_layout, null);
        final int padding = getResources().getDimensionPixelSize(R.dimen.action_bar_switch_padding);
        mSwitch.setPadding(0, 0, padding, 0);
        getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM);
        getActionBar().setCustomView(mSwitch, new ActionBar.LayoutParams(
                ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT,
                Gravity.CENTER_VERTICAL | Gravity.END));
        mSwitch.setOnCheckedChangeListener(this);

        // hide fragment firstly , then update it in onResume() according to switch status
        final FragmentManager fragmentManager = getFragmentManager();
        FragmentTransaction transaction = fragmentManager.beginTransaction();

        mPermissionsFragment = (PermissionsFragment)fragmentManager.findFragmentByTag(mPermissionsTag);
        mAppsFragment = (AppsFragment) fragmentManager.findFragmentByTag(mAppsTag);     

        if (mPermissionsFragment == null) {
            mPermissionsFragment = new PermissionsFragment();
            mAppsFragment = new AppsFragment();
            transaction.add(R.id.tab_pager, mPermissionsFragment, mPermissionsTag);
            transaction.add(R.id.tab_pager, mAppsFragment, mAppsTag);
        }

        transaction.hide(mPermissionsFragment);
        transaction.hide(mAppsFragment);
        transaction.commit();
        fragmentManager.executePendingTransactions();
        
        // set page adapter
        mTabPager = (ViewPager) findViewById(R.id.tab_pager);
        mTabPagerAdapter = new TabPagerAdapter();
        mTabPager.setAdapter(mTabPagerAdapter);
        mTabPager.setOnPageChangeListener(mTabPagerListener);

        // Configure action bar
        mActionBarAdapter = new ActionBarAdapter(this, this, getActionBar());

         boolean isShow = PermControlUtils.isPermControlOn(this);
         Xlog.d(TAG, "oncreate(), isShow " + isShow);

     }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    @Override 
    protected void onResume() {
        super.onResume();
        
        // always  check firstly
        checkUiEnabled();
        
        // register observer to enable/disable the switch 
        // for the case: other permssion manage apk is installed or uninstalled(Tecent)
        mSwitchContentObserver.register(getContentResolver());
        
        boolean isShow = PermControlUtils.isPermControlOn(this);
        Xlog.d(TAG, "onResume() , isShow " + isShow);

        if (isShow) {
            addUI();
        } else {
            removeUI();
        }
        mUserCheckedFlag = false;
        mSwitch.setChecked(isShow);
        mUserCheckedFlag = true;

    }

    @Override
    protected void onPause() {
        super.onPause();
        // unregister observer
        mSwitchContentObserver.unregister(getContentResolver());
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // Some of variables will be null if this Activity redirects Intent.
        // See also onCreate() or other methods called during the Activity's
        // initialization.
        if (mActionBarAdapter != null) {
            mActionBarAdapter.setListener(null);
        }
    }

    @Override
    public void onSelectedTabChanged() {
        updateFragmentsVisibility();
    }

    private void updateFragmentsVisibility() {
        TabState tab = mActionBarAdapter.getCurrentTab();
        int tabIndex = tab.ordinal();
        if (mTabPager.getCurrentItem() != tabIndex) {
            Xlog.d(TAG,
                    "mTabPager.getCurrentItem() " + mTabPager.getCurrentItem()
                            + " tabIndex " + tabIndex);
            mTabPager.setCurrentItem(tabIndex);
        }

        invalidateOptionsMenu();

    }

    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (ActivityManager.isUserAMonkey()) {
            Xlog.d(TAG, "Monkey is running");
            return;
        }
        if (!mUserCheckedFlag) {
            Xlog.d(TAG, "mUserCheckedFlag is false , return ");
            return;
        }

        Xlog.d(TAG, "onCheckedChanged(),isChecked = " + isChecked);

        if (isChecked) {
            addUI();
            PermControlUtils.enablePermissionControl(true, this);
        } else {
            // get the value from provider , it's 0 by default
            boolean isShowDlg = Settings.System.getInt(getContentResolver(),
                    PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, 0) == 0;
            Xlog.d(TAG, "onCheckedChanged(), isShow alert Dlg = " + isShowDlg);
            if (isShowDlg) {
                // show alert dialog
                Intent intent = new Intent();
                intent.setAction(UiUtils.ACTION_SWITCH_OFF_CONTROL_FROM_APP_PERM);
                startActivity(intent);
            } else {
                removeUI();
                PermControlUtils.enablePermissionControl(false, this);
            }
        }
    }

    protected void addUI() {
        Xlog.d(TAG, "addUI()");
        // must get a new transaction each time
        FragmentTransaction transaction = getFragmentManager()
                .beginTransaction();
        // set empty view to gone
        mEmptyView.setVisibility(View.GONE);
        // add all the fragment
        mPermissionsFragment = (PermissionsFragment) getFragmentManager()
                .findFragmentByTag(mPermissionsTag);
        mAppsFragment = (AppsFragment) getFragmentManager().findFragmentByTag(
                mAppsTag);

        if (mPermissionsFragment == null) {
            mPermissionsFragment = new PermissionsFragment();
            mAppsFragment = new AppsFragment();
            transaction.add(R.id.tab_pager, mPermissionsFragment, mPermissionsTag);
            transaction.add(R.id.tab_pager, mAppsFragment, mAppsTag);
        }
        transaction.show(mPermissionsFragment);
        transaction.show(mAppsFragment);

        transaction.commit();

        getFragmentManager().executePendingTransactions();
        // firstly remove tabs ,then add tabs and update it
        mActionBarAdapter.removeAllTab();
        mActionBarAdapter.addUpdateTab(mSavedInstanceState);
    }

    protected void removeUI() {
        Xlog.d(TAG, "removeUI()");
        // must get a new transaction each time
        FragmentTransaction transaction = getFragmentManager()
                .beginTransaction();
        // set empty view to visible
        mEmptyView.setVisibility(View.VISIBLE);
        // remove all the fragment
        mPermissionsFragment = (PermissionsFragment) getFragmentManager()
                .findFragmentByTag(mPermissionsTag);

        if (mPermissionsFragment != null) {
            transaction.hide(mPermissionsFragment);
            transaction.hide(mAppsFragment);
        }
        transaction.commit();
        getFragmentManager().executePendingTransactions();
        // remove tabs on actionbar
        mActionBarAdapter.removeAllTab();
    }
    
    private void checkUiEnabled() {
        boolean enable = PermControlUtils
                .isInHouseEnabled(PermissionControlPageActivity.this);
        Xlog.d(TAG, "checkEnabled(), update current interface, enable = "
                + enable);
        if (!enable) {
            Xlog.d(TAG, "finish itself");
            finish();
        }
    }
}