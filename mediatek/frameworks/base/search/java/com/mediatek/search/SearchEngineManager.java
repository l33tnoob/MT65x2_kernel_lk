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

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Slog;

import com.mediatek.common.search.ISearchEngineManager;
import com.mediatek.common.search.ISearchEngineManagerService;
import com.mediatek.common.search.SearchEngineInfo;

import java.util.List;

/**
 * This class provides access to the system search engine services.
 *
 * If you do require direct access to the SearchEngineManager, do not instantiate
 * this class directly. Instead, retrieve it through
 * {@link android.content.Context#getSystemService
 * context.getSystemService(Context.SEARCH_ENGINE_SERVICE)}.
 * @hide
 */
public class SearchEngineManager implements ISearchEngineManager
{

    private static final boolean DBG = false;
    private static final String TAG = "SearchEngineManager";
    /**
     * Reference to the shared system search service.
     */
    private static ISearchEngineManagerService mService;

    private final Context mContext;

    public SearchEngineManager(Context context) {
        mContext = context;
        mService = ISearchEngineManagerService.Stub.asInterface(
                ServiceManager.getService(Context.SEARCH_ENGINE_SERVICE));
    }

    /**
     * Returns a list of SearchEngineInfo that can be used by all applications to do web search
     * @return list of all SearchEngineInfo in current locale
     */
    public List<SearchEngineInfo> getAvailableSearchEngines() {
        try {
            return mService.getAvailableSearchEngines();
        } catch (RemoteException e) {
            Slog.e(TAG, "getSearchEngineInfos() failed: " + e);
            return null;
        }
    }

    /**
     * Get search engine by name or favicon. If could not find search engine by name,
     * then find search engine by favicon.
     * @param name the search engine name
     * @param favicon the search engine favicon
     * @return if found then return the search engine, else return null
     */
    public SearchEngineInfo getBestMatchSearchEngine(String name, String favicon) {
        try {
            return mService.getBestMatchSearchEngine(name, favicon);
        } catch (RemoteException e) {
            Slog.e(TAG, "getBestMatchSearchEngine() failed: " + e);
            return null;
        }
    }

    /**
     * Get search engine by favicon uri.
     * @param favicon the search engine favicon
     * @return the search engine
     */
    public SearchEngineInfo getSearchEngineByFavicon(String favicon) {
        return getSearchEngine(SearchEngineInfo.FAVICON, favicon);
    }

    /**
     * Get search engine by name.
     * @param name the search engine name
     * @return the search engine
     */
    public SearchEngineInfo getSearchEngineByName(String name) {
        return getSearchEngine(SearchEngineInfo.NAME, name);
    }

    /**
     * Get search engine through specified field and value.
     * @param field the field of SearchEngineInfo
     * @param value the value of the field
     * @return the search engine
     */
    public SearchEngineInfo getSearchEngine(int field, String value) {
        try {
            return mService.getSearchEngine(field, value);
        } catch (RemoteException e) {
            Slog.e(TAG, "getSearchEngine(int field, String value) failed: " + e);
            return null;
        }
    }

    /**
     * Get system default search engine.
     * @return the search engine
     */
    public SearchEngineInfo getDefaultSearchEngine() {
        try {
            return mService.getDefaultSearchEngine();
        } catch (RemoteException e) {
            Slog.e(TAG, "getSystemDefaultSearchEngine() failed: " + e);
            return null;
        }
    }

    /**
     * Set default search engine for system.
     * @param engine the search engine to set
     * @return if set success then return true, else return false
     */
    public boolean setDefaultSearchEngine(SearchEngineInfo engine) {
        try {
            return mService.setDefaultSearchEngine(engine);
        } catch (RemoteException e) {
            Slog.e(TAG, "getSystemDefaultSearchEngine() failed: " + e);
            return false;
        }
    }
}
