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
package com.mediatek.common.search;

import android.content.Context;
import android.content.res.Resources;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;
import android.util.Log;

import java.net.URLEncoder;
import java.util.Arrays;
import java.util.Locale;

/**
 * Loads and holds data for a given web search engine.
 * @hide
 */
public final class SearchEngineInfo implements Parcelable {

    private static String TAG = "SearchEngineInfo";
    private static final boolean DBG = true;

    /*
     * The fields of a search engine data array, defined in the same order
     * as they appear in the all_search_engines.xml file.
     * If you are adding/removing to this list, remember to update NUM_FIELDS below.
     */
    private static final int FIELD_LABEL = 0;
    private static final int FIELD_KEYWORD = 1;
    private static final int FIELD_FAVICON = 2;
    private static final int FIELD_SEARCH_URI = 3;
    private static final int FIELD_ENCODING = 4;
    private static final int FIELD_SUGGEST_URI = 5;
    private static final int NUM_FIELDS = 6;

    public static final int NAME = -1;
    public static final int FAVICON = 2;

    // The OpenSearch URI template parameters that we support.
    private static final String PARAMETER_LANGUAGE = "{language}";
    private static final String PARAMETER_SEARCH_TERMS = "{searchTerms}";
    private static final String PARAMETER_INPUT_ENCODING = "{inputEncoding}";
    private static final String DEFAULT_SP = "--";
    private static final String EMPTY = "nil";

    private final String mName;

    /*
     * The array of strings defining this search engine. The array values
     * are in the same order as the above enumeration definition.
     */
    private final String[] mSearchEngineData;

    public SearchEngineInfo(String name, String []data) {
        mName = name;
        mSearchEngineData = data;
    }

    /**
     * Get the name of current search engine.
     * @return the name of current search engine
     *
     * @internal
     */
    public String getName() {
        return mName;
    }

    /**
     * Get the label of current search engine to display.
     * @return the label of current search engine
     *
     * @internal
     */
    public String getLabel() {
        return mSearchEngineData[FIELD_LABEL];
    }

    /**
     * Returns the URI for launching a web search with the given query (or null if there was no
     * data available for this search engine).
     * @param query query string
     * @return uri with query string to search
     *
     * @internal
     */
    public String getSearchUriForQuery(String query) {
        return getFormattedUri(getSearchUri(), query);
    }

    /**
     * Returns the URI for retrieving web search suggestions for the given query (or null if there
     * was no data available for this search engine).
     * @param query query string which is input by user
     * @return uri with query string to get suggestions
     *
     * @internal
     */
    public String getSuggestUriForQuery(String query) {
        return getFormattedUri(getSuggestUri(), query);
    }

    /**
     * Whether this search engine supports suggestions or not.
     * @return if supports return true, else return false
     *
     * @internal
     */
    public boolean supportsSuggestions() {
        return !TextUtils.isEmpty(getSuggestUri());
    }

    /**
     * Get the key word of current search engine.
     * @return the key word of current search engine
     *
     * @internal
     */
    public String getKeyWord() {
        return mSearchEngineData[FIELD_KEYWORD];
    }

    /**
     * Get the favicon of current search engine.
     * @return the favicon of current search engine
     *
     * @internal
     */
    public String getFaviconUri() {
        return mSearchEngineData[FIELD_FAVICON];
    }

    private String getSuggestUri() {
        return mSearchEngineData[FIELD_SUGGEST_URI];
    }

    private String getSearchUri() {
        return mSearchEngineData[FIELD_SEARCH_URI];
    }

    /**
     * Formats a launchable uri out of the template uri by replacing the template parameters with
     * actual values.
     */
    private String getFormattedUri(String templateUri, String query) {
        if (TextUtils.isEmpty(templateUri)) {
            return null;
        }

        // Encode the query terms in the requested encoding (and fallback to UTF-8 if not).
        String enc = mSearchEngineData[FIELD_ENCODING];
        try {
            return templateUri.replace(PARAMETER_SEARCH_TERMS, URLEncoder.encode(query, enc));
        } catch (java.io.UnsupportedEncodingException e) {
            Log.e(TAG, "Exception occured when encoding query " + query + " to " + enc);
            return null;
        }
    }

    @Override
    public String toString() {
        return "SearchEngineInfo{" + Arrays.toString(mSearchEngineData) + "}";
    }

    /**
     * Support for parcelable and aidl operations.
     */
    public static final Parcelable.Creator<SearchEngineInfo> CREATOR = new Parcelable.Creator<SearchEngineInfo>() {
        public SearchEngineInfo createFromParcel(Parcel in) {
            return new SearchEngineInfo(in);
        }

        public SearchEngineInfo[] newArray(int size) {
            return new SearchEngineInfo[size];
        }
    };

    /**
     * Instantiates a new SearchableInfo from the data in a Parcel that was
     * previously written with {@link #writeToParcel(Parcel, int)}.
     *
     * @param in The Parcel containing the previously written SearchableInfo,
     * positioned at the location in the buffer where it was written.
     */
    SearchEngineInfo(Parcel in) {
        mName = in.readString();
        mSearchEngineData = new String[NUM_FIELDS];
        in.readStringArray(mSearchEngineData);
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        // TODO Auto-generated method stub
        dest.writeString(mName);
        dest.writeStringArray(mSearchEngineData);
    }

    public static SearchEngineInfo parseFrom(String configInfo, String sp) throws IllegalArgumentException {

        if(DBG) {
            Log.i(TAG, "Parse From config file " + configInfo);
        }

        if (null == configInfo || "".equals(configInfo)) {
            throw new IllegalArgumentException("Empty config info");
        }

        if (null == sp || "".equals(sp)) {
            sp = DEFAULT_SP;
        }

        String []configData = configInfo.split(sp);

        if (configData.length != (NUM_FIELDS + 1)) {
            throw new IllegalArgumentException("Field Missing");
        }

        String engineName = parseField(configData, -1);
        String engineLabel = parseField(configData, FIELD_LABEL);
        String engineKeyword = parseField(configData, FIELD_KEYWORD);
        String engineFavicon = parseField(configData, FIELD_FAVICON);
        String engineSearchUri = parseField(configData, FIELD_SEARCH_URI);
        String engineEncoding = parseField(configData, FIELD_ENCODING);
        String engineSuggestUri = parseField(configData,FIELD_SUGGEST_URI);

        if(DBG) {
            Log.i(TAG, "SearchEngineInfo consturctor called, search engine name is: " + engineName);
        }

        if (null == engineSearchUri) {
            throw new IllegalArgumentException(engineName + " has an empty search URI");
        }

        // Add the current language/country information to the URIs.
        //Locale locale = context.getResources().getConfiguration().locale;
        Locale locale = Locale.getDefault();
        StringBuilder language = new StringBuilder(locale.getLanguage());
        if (!TextUtils.isEmpty(locale.getCountry())) {
            language.append('-');
            language.append(locale.getCountry());
        }

        String language_str = language.toString();
        engineSearchUri = engineSearchUri.replace(PARAMETER_LANGUAGE, language_str);
        if (null != engineSuggestUri) {
            engineSuggestUri =engineSuggestUri.replace(PARAMETER_LANGUAGE, language_str);
        }

        // Default to UTF-8 if not specified.
        if (null == engineEncoding) {
            engineEncoding = "UTF-8";
        }

        // Add the input encoding method to the URI.
        engineSearchUri = engineSearchUri.replace(PARAMETER_INPUT_ENCODING, engineEncoding);
        if (null != engineSuggestUri) {
            engineSuggestUri = engineSuggestUri.replace(PARAMETER_INPUT_ENCODING, engineEncoding);
        }

        String []datas = {engineLabel, engineKeyword, engineFavicon,
            engineSearchUri, engineEncoding, engineSuggestUri};
        SearchEngineInfo newInstance = new SearchEngineInfo(engineName, datas);

        return newInstance;
    }

    /**
     * return null or a real string in data.
     */
    private static String parseField(String []data, int fieldIndex) {
        // we add name at data[0], so +1
        int realFieldIndex = fieldIndex + 1;

        if ((data.length - 1) < realFieldIndex ||
                TextUtils.isEmpty(data[realFieldIndex]) || EMPTY.equals(data[realFieldIndex])) {
            return null;
                }
        return data[realFieldIndex];
    }
}
