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

package com.mediatek.security.ui;

import android.app.Fragment;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.UserHandle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.security.R;
import com.mediatek.security.datamanager.DatabaseManager;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.xlog.Xlog;

import java.text.Collator;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class AppsFragment extends Fragment implements OnItemClickListener {
    private static final String TAG = "AppsFragment";
    private Context mCxt;
    // layout inflater object used to inflate views
    private LayoutInflater mInflater;
    private View mContentView;
    // ListView used to display list
    private ListView mListView;
    private AppMatchPermAdapter mApdater;
    private static int sRetrieveFlags;
    private DataAsyncLoader mAsyncTask;

    private final BroadcastReceiver mPkgChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PermControlUtils.PERM_CONTROL_DATA_UPDATE)) {
                Xlog.d(TAG, "receiver ,re-loading......");
                load();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mCxt = getActivity().getApplicationContext();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        mInflater = inflater;
        mContentView = inflater.inflate(R.layout.app_match_permission, null);
        View emptyView = mContentView
                .findViewById(com.android.internal.R.id.empty);
        ListView lv = (ListView) mContentView.findViewById(android.R.id.list);
        if (emptyView != null) {
            lv.setEmptyView(emptyView);
        }
        lv.setOnItemClickListener(this);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);
        mListView = lv;
        mApdater = new AppMatchPermAdapter();
        mListView.setAdapter(mApdater);
        return mContentView;
    }

    @Override
    public void onResume() {
        Xlog.i(TAG, "onResume");
        super.onResume();
        load();
        IntentFilter filter = new IntentFilter();
        filter.addAction(PermControlUtils.PERM_CONTROL_DATA_UPDATE);
        getActivity().registerReceiver(mPkgChangeReceiver, filter);
    }

    @Override
    public void onPause() {
        Xlog.i(TAG, "onPause");
        super.onPause();
        getActivity().unregisterReceiver(mPkgChangeReceiver);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mAsyncTask != null) {
            mAsyncTask.cancel(true);
            Xlog.d(TAG,"cancel task in onDestory()");
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        Xlog.d(TAG, "request Code=" + requestCode + ",result Code="
                + resultCode);
        if (UiUtils.RESULT_START_ACTIVITY == requestCode) {
            if (UiUtils.RESULT_FINISH_ITSELF == resultCode) {
                Xlog.d(TAG, "finish AppsFragment activity");
                getActivity().finish();
            }
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ListView l = (ListView) parent;
        AppInfo app = (AppInfo) l.getAdapter().getItem(position);
        // must refer to the package name , not app label
        String pkgName = app.mPkgName;

        Xlog.d(TAG, "onItemClick ,pkgName " + pkgName);

        Intent intent = new Intent();
        intent.setAction("com.mediatek.security.EACH_PERMISSION_CONTROL");
        intent.putExtra("pkgName", pkgName);
        try {
            startActivityForResult(intent, UiUtils.RESULT_START_ACTIVITY);
        } catch (ActivityNotFoundException e) {
            Xlog.d(TAG, "ActivityNotFoundException for " + pkgName);
        }
    }

    class AppMatchPermAdapter extends BaseAdapter {
        private List<AppInfo> mPkgList = new ArrayList<AppInfo>();

        public AppMatchPermAdapter() {

        }

        public void setDataAndNotify(List<AppInfo> pkgList) {
            mPkgList = pkgList;
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            if (mPkgList != null) {
                return mPkgList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return mPkgList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            AppViewHolder holder;

            // When convertView is not null, we can reuse it directly, there is
            // no need to reinflate it. We only inflate a new View when the convertView
            // supplied by ListView is null.
            if (convertView == null) {
                convertView = mInflater.inflate(
                        R.layout.manage_applications_item, null);

                // Creates a ViewHolder and store references to the two children
                // views we want to bind data to.
                holder = new AppViewHolder();
                holder.mAppName = (TextView) convertView
                        .findViewById(R.id.app_name);
                holder.mAppIcon = (ImageView) convertView
                        .findViewById(R.id.app_icon);
                holder.mAppSize = (TextView) convertView
                        .findViewById(R.id.app_size);
                convertView.setTag(holder);
            } else {
                // Get the ViewHolder back to get fast access to the TextView
                // and the ImageView.
                holder = (AppViewHolder) convertView.getTag();
            }

            // Bind the data efficiently with the holder
            AppInfo appItem = mPkgList.get(position);
            holder.mAppName.setText(appItem.mAppName);
            holder.mAppIcon.setImageDrawable(appItem.mAppIcon);
            holder.mAppSize.setText(appItem.mAppPermSize);

            return convertView;
        }
    }

    private class DataAsyncLoader extends
            AsyncTask<Void, Integer, List<AppInfo>> {
        @Override
        protected List<AppInfo> doInBackground(Void... params) {
            Xlog.d(TAG, "doInBackground......");
            // get the origin data
            List<String> list = (List<String>) DatabaseManager
                    .getPackageNames();
            if (list == null || list.size() == 0) {
                Xlog.d(TAG, "get origin list is null ");
                return null;
            }

            // encrypt the data to get view more fast
            List<AppInfo> pkgList = new ArrayList<AppInfo>();
            for (String pkgName : list) {
                if (isCancelled()) {
                   Xlog.d(TAG, "the Async Task is cancled");
                   return null;
                }
                String appLabel = PermControlUtils.getApplicationName(mCxt,
                        pkgName);
                if (appLabel != null) {
                    Drawable appIcon = PermControlUtils.getApplicationIcon(
                            mCxt, pkgName);

                    List<PermissionRecord> permList = DatabaseManager
                            .getPermRecordListByPkgName(pkgName);
                    int count = 0;
                    if (permList == null) {
                        Xlog.w(TAG,"error ,the app doesn't have any control permission");
                    } else {
                        count = permList.size();
                    }
                    String countUnit = count > 1 ? getString(R.string.perm_count_unit_plural)
                            : getString(R.string.perm_count_unit_single);
                    String permCount = String.valueOf(count) + " " + countUnit;

                    //Xlog.d(TAG, "appLabel = " + appLabel + " , permCount = "
                      //      + permCount);

                    AppInfo appItem = new AppInfo();
                    appItem.mPkgName = pkgName;
                    appItem.mAppName = appLabel;
                    appItem.mAppIcon = appIcon;
                    appItem.mAppPermSize = permCount;

                    pkgList.add(appItem);
                }
            }

            // sort list by the package label
            Collections.sort(pkgList, ALPHA_COMPARATOR);
            return pkgList;
        }

        @Override
        protected void onPostExecute(List<AppInfo> list) {
            Xlog.d(TAG, "onPostExecute......");
            mApdater.setDataAndNotify(list);
        }
    }

    private void load() {
        mAsyncTask = (DataAsyncLoader)new DataAsyncLoader().execute();
    }

    // comparator for sorting the app list
    public static final Comparator<AppInfo> ALPHA_COMPARATOR = new Comparator<AppInfo>() {
        private final Collator mCollator = Collator.getInstance();

        @Override
        public int compare(AppInfo object1, AppInfo object2) {
            return mCollator.compare(object1.mAppName, object2.mAppName);
        }
    };

    // View Holder used when displaying views
    static class AppViewHolder {
        TextView mAppName;
        ImageView mAppIcon;
        TextView mAppSize;
    }

    class AppInfo {
        private String mPkgName;
        private String mAppName;
        private Drawable mAppIcon;
        private String mAppPermSize;

        public AppInfo() {
        }
    }
}
