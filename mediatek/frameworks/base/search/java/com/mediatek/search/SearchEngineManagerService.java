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
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.search;

import com.android.internal.content.PackageMonitor;

import android.app.SearchableInfo;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;
import android.os.Handler;
import android.os.Process;
import android.provider.Settings;
import android.util.Log;

import com.mediatek.common.regionalphone.RegionalPhone;
import com.mediatek.common.search.ISearchEngineManagerService;
import com.mediatek.common.search.SearchEngineInfo;
import com.mediatek.common.regionalphone.RegionalPhone;
import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.search.IRegionalPhoneSearchEngineExt;

import java.util.ArrayList;
import java.util.List;

/**
 * The search engine manager service handles the search UI, and maintains a registry of search engines.
 * @hide
 */
public class SearchEngineManagerService extends ISearchEngineManagerService.Stub {

    // general debugging support
    private static final String TAG = "SearchEngineManagerService";

    // Context that the service is running in.
    private final Context mContext;

    // This list saved all the search engines supported by search framework.
    private List<SearchEngineInfo> mSearchEngineInfos;

    private SearchEngineInfo mDefaultSearchEngine;

    /**
     * Initializes the Search Manager service in the provided system context.
     * Only one instance of this object should be created!
     *
     * @param context to use for accessing DB, window manager, etc.
     */
    public SearchEngineManagerService(Context context)  {
        mContext = context;
        mContext.registerReceiver(new BootCompletedReceiver(),
                new IntentFilter(Intent.ACTION_BOOT_COMPLETED));
        // register observer for search engines.
        mContext.getContentResolver().registerContentObserver(RegionalPhone.SEARCHENGINE_URI,
                true, mSearchEngineObserver);
    }

    private ContentObserver mSearchEngineObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            initSearchEngineInfos();
        }
    };

    /**
     * Creates the initial searchables list after boot.
     */
    private final class BootCompletedReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            new Thread() {
                @Override
                public void run() {
                    Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
                    mContext.unregisterReceiver(BootCompletedReceiver.this);
                    initSearchEngineInfos();
                    mContext.registerReceiver(new LocaleChangeReceiver(),
                            new IntentFilter(Intent.ACTION_LOCALE_CHANGED));
                }
            }.start();
        }
    }

    /**
     * Returns a list of SearchEngineInfo that can be used by all applications to do web search
     * @return list of all SearchEngineInfo in current locale
     * @hide
     */
    public synchronized List<SearchEngineInfo> getAvailableSearchEngines() {
        Log.i(TAG, "get avilable search engines");
        if (mSearchEngineInfos == null) {
            initSearchEngineInfos();
        }
        return mSearchEngineInfos;
    }

    private void initSearchEngineInfos() throws IllegalArgumentException{
        IRegionalPhoneSearchEngineExt regionalPhoneSearchEngineExt = MediatekClassFactory.createInstance(IRegionalPhoneSearchEngineExt.class);
        mSearchEngineInfos = regionalPhoneSearchEngineExt.initSearchEngineInfosFromRpm(mContext);
        if ( mSearchEngineInfos != null){
            mDefaultSearchEngine = mSearchEngineInfos.get(0);
            Log.d(TAG, "RegionalPhone Search engine init");
            return;
        }else{
        mSearchEngineInfos = new ArrayList<SearchEngineInfo>();
        }

        Resources res = mContext.getResources();
        String[] searchEngines = res.getStringArray(com.mediatek.internal.R.array.new_search_engines);
        if (null == searchEngines || 1 >= searchEngines.length) {
            // todo: throws an exception in this case.
            throw new IllegalArgumentException("No data found for ");
        }
        String sp = searchEngines[0];
        for (int i = 1; i < searchEngines.length; i++) {
            String configInfo = searchEngines[i];
            SearchEngineInfo info = SearchEngineInfo.parseFrom(configInfo, sp);
            mSearchEngineInfos.add(info);
        }

        // keep old setting.
        if (mDefaultSearchEngine != null) {
            mDefaultSearchEngine = getBestMatchSearchEngine(mDefaultSearchEngine.getName(),
                            mDefaultSearchEngine.getFaviconUri());
        }

        if (mDefaultSearchEngine == null) {
            mDefaultSearchEngine = mSearchEngineInfos.get(0);
        }
        // tell search widget that search engine changed.
        broadcastSearchEngineChangedInternal(mContext);
    }

    /**
     * Creates the initial searchables list after boot.
     */
    private final class LocaleChangeReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            initSearchEngineInfos();
        }
    }

    /**
     * Sent a broadcast without extral data.
     */
    private void broadcastSearchEngineChangedInternal(Context context) {
        Intent intent = new Intent(SearchEngineManager.ACTION_SEARCH_ENGINE_CHANGED);
        context.sendBroadcast(intent);
        Log.d(TAG, "broadcast serach engine changed");
    }

    /**
     * Get search engine by name or favicon. If could not find search engine by name,
     * then find search engine by favicon.
     * @param name the search engine name
     * @param favicon the search engine favicon
     * @return if found then return the search engine, else return null
     */
    public SearchEngineInfo getBestMatchSearchEngine(String name, String favicon) {
        SearchEngineInfo engine = getSearchEngineByName(name);
        return (engine != null) ? engine : getSearchEngineByFavicon(favicon);
    }

    /**
     * Get search engine by favicon uri.
     */
    private SearchEngineInfo getSearchEngineByFavicon(String favicon) {
        List<SearchEngineInfo> engines = getAvailableSearchEngines();
        for (SearchEngineInfo engine : engines) {
            if (favicon.equals(engine.getFaviconUri())) {
                return engine;
            }
        }
        return null;
    }

    /**
     * Get search engine by name.
     */
    private SearchEngineInfo getSearchEngineByName(String name) {
        List<SearchEngineInfo> engines = getAvailableSearchEngines();
        for (SearchEngineInfo engine : engines) {
            if (name.equals(engine.getName())) {
                return engine;
            }
        }
        return null;
    }

    /**
     * Get search engine through specified field and value.
     * @param field the field of SearchEngineInfo
     * @param value the value of the field
     * @return the search engine
     */
    public SearchEngineInfo getSearchEngine(int field, String value) {
        switch (field) {
            case SearchEngineInfo.NAME:
                return getSearchEngineByName(value);
            case SearchEngineInfo.FAVICON:
                return getSearchEngineByFavicon(value);
            default:
                return null;
        }
    }

    /**
     * Get system default search engine.
     * @return the first item in config file as system default search engine
     */
    public SearchEngineInfo getDefaultSearchEngine() {
        return mDefaultSearchEngine;
    }

    /**
     * Set default search engine for system.
     * @param engine the search engine to set
     * @return if set success then return true, else return false
     */
    public boolean setDefaultSearchEngine(SearchEngineInfo engine) {
        List<SearchEngineInfo> engines = getAvailableSearchEngines();
        for (SearchEngineInfo eng : engines) {
            if (eng.getName().equals(engine.getName())) {
                mDefaultSearchEngine = engine;
                return true;
            }
        }
        return false;
    }
}
