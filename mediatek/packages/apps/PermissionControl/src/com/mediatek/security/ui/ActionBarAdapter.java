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

package com.mediatek.security.ui;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.FragmentTransaction;
import android.content.Context;
import android.os.Bundle;

import com.mediatek.security.R;
import com.mediatek.xlog.Xlog;

/**
 * Adapter for the action bar at the top of the PermissionControlPageActivity.
 */
public class ActionBarAdapter {
    private static final String TAG = "PermControl/ActionBarAdapter";
    
    private final Context mContext;
    private final ActionBar mActionBar;

    private static final String EXTRA_KEY_SELECTED_TAB = "navBar.selectedTab";
    private static final TabState DEFAULT_TAB = TabState.PERMISSIONS_INFO;
    private TabState mCurrentTab = DEFAULT_TAB;

    private Listener mListener;
    private final MyTabListener mTabListener = new MyTabListener();
    
    public interface Listener {
        void onSelectedTabChanged();
    }
    
    public enum TabState {
        PERMISSIONS_INFO, APPS_INFO;

        public static TabState fromInt(int value) {

            if (PERMISSIONS_INFO.ordinal() == value) {
                return PERMISSIONS_INFO;
            }
            if (APPS_INFO.ordinal() == value) {
                return APPS_INFO;
            }
            throw new IllegalArgumentException("Invalid value: " + value);
        }
    }

    public ActionBarAdapter(Context context, Listener listener,
            ActionBar actionBar) {
        mContext = context;
        mListener = listener;
        mActionBar = actionBar;
    }

    public void addUpdateTab(Bundle savedState) {
        // Set up tabs
        addTab(TabState.PERMISSIONS_INFO, R.string.permissions_label);
        addTab(TabState.APPS_INFO, R.string.apps);
        Xlog.d(TAG, "initialize() .....");
        if (savedState != null) {
            mCurrentTab = TabState.fromInt(savedState
                    .getInt(EXTRA_KEY_SELECTED_TAB));
            Xlog.d(TAG, "get saved tab  " + mCurrentTab);
        }
        update();
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    private void addTab(TabState tabState, int description) {
        final Tab tab = mActionBar.newTab();
        tab.setTag(tabState);
        tab.setTabListener(mTabListener);
        tab.setText(description);
        mActionBar.addTab(tab);
    }

    public void removeAllTab() {
        mActionBar.removeAllTabs();
        // must set the navigation mode to standard ,
        // or will show the tab background
        mTabListener.mIgnoreTabSelected = true;
        mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
        mTabListener.mIgnoreTabSelected = false;
    }

    private class MyTabListener implements ActionBar.TabListener {
        /**
         * If true, it won't call {@link #setCurrentTab} in
         * {@link #onTabSelected}. This flag is used when we want to
         * programmatically update the current tab without
         * {@link #onTabSelected} getting called.
         */
        public boolean mIgnoreTabSelected;

        @Override
        public void onTabReselected(Tab tab, FragmentTransaction ft) {
        }

        @Override
        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        }

        @Override
        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            Xlog.d(TAG, "onTabSelected() " + " ignore ? " + mIgnoreTabSelected);
            if (!mIgnoreTabSelected) {
                Xlog.d(TAG, "setCurrentTab()");
                setCurrentTab((TabState) tab.getTag(),true);
            }
        }
    }


    /*
     * change the current tab
     * @param tab TabState, tab will be set to the current
     * @param notifyListener boolean , if true , 
     * will initiative update fragment visibility 
     * */
    
    public void setCurrentTab(TabState tab, boolean notifyListener) {
        if (tab == null) {
            throw new NullPointerException();
        }
        if (tab == mCurrentTab) {
            Xlog.d(TAG, "tab == mCurrentTab ,directly return " + tab);
            return;
        }
        mCurrentTab = tab;
        Xlog.d(TAG, "mCurrentTab = " + mCurrentTab + " notifyListner "
                + notifyListener);
        int index = mCurrentTab.ordinal();
        if ((mActionBar.getNavigationMode() == ActionBar.NAVIGATION_MODE_TABS)
                && (index != mActionBar.getSelectedNavigationIndex())) {
            mActionBar.setSelectedNavigationItem(index);
        }

        if (notifyListener && mListener != null) {
            mListener.onSelectedTabChanged();
        }
    }

    public TabState getCurrentTab() {
        return mCurrentTab;
    }

    private void update() {
        if (mActionBar.getNavigationMode() != ActionBar.NAVIGATION_MODE_TABS) {
            Xlog.d(TAG, "update() " + mCurrentTab.ordinal() + mCurrentTab);
            
            /* setNavigationMode will trigger onTabSelected() with the tab which
             was previously  selected.
             The issue is that when we're first switching to the tab
             navigation mode after
             screen orientation changes, onTabSelected() will get called with
             the first tab
             (i.e. "Apps"), which would results in mCurrentTab getting set to
             "Apps" and  we'd lose restored tab.
             So let's just disable the callback here temporarily. We'll notify
             the listener after this anyway.
             * */
            mTabListener.mIgnoreTabSelected = true;
            mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
            mActionBar.setSelectedNavigationItem(mCurrentTab.ordinal());
            mTabListener.mIgnoreTabSelected = false;
        }

    }

    public void onSaveInstanceState(Bundle outState) {
        Xlog.d(TAG, "onSaveInstanceState() " + mCurrentTab.ordinal());
        outState.putInt(EXTRA_KEY_SELECTED_TAB, mCurrentTab.ordinal());
    }
}

