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

package com.mediatek.security.ui;


import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
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

public class AppToPermissionActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "AppToPermActivity";

    private LayoutInflater mInflater;
    private PackageManager mPm;
    private Context mCxt;
    private ApplicationInfo mAppInfo;
    private PackageInfo mPackageInfo;
    private int mRetrieveFlags;

    private ListView mListView;
    private PermAdapter mApdater;

    private String[] mEntries;
    PermissionRecord mSelectPermInfo;

    private String mPkgName;
    
    // get permission label array
    String[] mPermLabelArray;
    
    // Dialog identifiers used in showDialog
    private static final int DLG_BASE = 0;
    private static final int DLG_CHANEG_PERM_STATUS = DLG_BASE + 1;
    AlertDialog mAlertDlg;
    private int mSavedSelectedIndex;

    private AppToPermAsyncLoader mAsyncTask;

    private final BroadcastReceiver mPkgChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PermControlUtils.PERM_CONTROL_DATA_UPDATE)) {
                Bundle bundle = intent.getExtras();
                String updatePkg = bundle
                        .getString(PermControlUtils.PACKAGE_NAME);
                if (updatePkg != null && updatePkg.equals(mPkgName)) {
                    Xlog.d(TAG, "receiver ,re-loading for pkgname: " + mPkgName);
                    load();
                }
            }
        }
    };

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
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Bundle bundle = getIntent().getExtras();
        mPkgName = bundle != null ? bundle.getString("pkgName") : null;
        Xlog.d(TAG, "get package name " + mPkgName);
        if (mPkgName == null) {
            Xlog.w(TAG, "finish itself because of pkgName is null");
            finish();
        }

        mPm = getPackageManager();
        mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCxt = getApplicationContext();

        // Only the owner can see all apps.
        if (UserHandle.myUserId() == 0) {
            mRetrieveFlags = PackageManager.GET_UNINSTALLED_PACKAGES
                    | PackageManager.GET_DISABLED_COMPONENTS;
        } else {
            mRetrieveFlags = PackageManager.GET_DISABLED_COMPONENTS;
        }

        try {
            mAppInfo = mPm.getApplicationInfo(mPkgName, mRetrieveFlags);
            mPackageInfo = mPm.getPackageInfo(mPkgName, mRetrieveFlags);
        } catch (NameNotFoundException ex) {
            Xlog.w(TAG,
                    "ApplicationInfo cannot be found because of pkgName is null");
            return;
        }

        mEntries = getResources().getStringArray(R.array.perm_status_entry);
        mPermLabelArray = getResources().getStringArray(
                R.array.permission_label);
        
        setContentView(R.layout.manage_app_permission_details);

        View emptyView = (View) findViewById(com.android.internal.R.id.empty);
        ListView lv = (ListView) findViewById(android.R.id.list);
        if (emptyView != null) {
            lv.setEmptyView(emptyView);
        }
        lv.setOnItemClickListener(this);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);
        mListView = lv;

        mApdater = new PermAdapter();
        mListView.setAdapter(mApdater);

        setAppLabelAndIcon(mPkgName);
    }

    @Override
    public void onResume() {
        Xlog.i(TAG, "onResume");
        super.onResume();
        // firstly check enabled or not
        checkUiEnabled();
        // update the UI in time
        load();
        // register the receiver about the pkg changed
        IntentFilter filter = new IntentFilter();
        filter.addAction(PermControlUtils.PERM_CONTROL_DATA_UPDATE);
        registerReceiver(mPkgChangeReceiver, filter);

        // register observer to enable/disable the switch
        // for the case: other permssion manage apk is installed or
        // uninstalled(Tecent)
        mSwitchContentObserver.register(getContentResolver());
    }

    @Override
    public void onPause() {
        Xlog.i(TAG, "onPause");
        super.onPause();
        unregisterReceiver(mPkgChangeReceiver);
        // unregister observer
        mSwitchContentObserver.unregister(getContentResolver());
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
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        ListView l = (ListView) parent;
        mSelectPermInfo = (PermissionRecord) l.getAdapter().getItem(position);
        Xlog.d(TAG, "onItemClick  " + mSelectPermInfo.mPermissionName);
        showDialog(DLG_CHANEG_PERM_STATUS);
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
        case DLG_CHANEG_PERM_STATUS:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            Xlog.d(TAG, "onCreateDialog ,get status = " + mSelectPermInfo.getStatus());
            mSavedSelectedIndex =  UiUtils.getSelectIndex(mSelectPermInfo.getStatus());
            builder.setTitle(PermControlUtils.getPermissionLabel(
                    mSelectPermInfo.mPermissionName, mPermLabelArray));
            builder.setSingleChoiceItems(mEntries,mSavedSelectedIndex,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Xlog.d(TAG, "selected which " + which);
                            mSavedSelectedIndex = which;
                            mSelectPermInfo.setStatus(UiUtils.PERM_STATUS_ARRAY[which]);
                            PermControlUtils.changePermission(mSelectPermInfo,mCxt);
                            mApdater.notifyDataSetChanged();
                            removeDialog(DLG_CHANEG_PERM_STATUS);
                        }
                    });
            builder.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    Xlog.d(TAG, "dialog dismiss, remove it");
                    removeDialog(DLG_CHANEG_PERM_STATUS);
                }
            });
            mAlertDlg = builder.create();
            return mAlertDlg;
        default:
            break;
        }
        return null;
    }

    // Utility method to set applicaiton label and icon.
    private void setAppLabelAndIcon(String pkgName) {
        final View appSnippet = findViewById(R.id.app_snippet);
        appSnippet.setPadding(0, appSnippet.getPaddingTop(), 0,
                appSnippet.getPaddingBottom());
        // Set application name.
        TextView label = (TextView) appSnippet.findViewById(R.id.app_name);
        String appLabel = mPm.getApplicationLabel(mAppInfo).toString();
        label.setText(appLabel);
        // Set application icon
        ImageView icon = (ImageView) appSnippet.findViewById(R.id.app_icon);
        Drawable appIcon = mPm.getApplicationIcon(mAppInfo);
        icon.setImageDrawable(appIcon);
        // Version number of application
        TextView appVersion = (TextView) appSnippet.findViewById(R.id.app_size);
        if (mPackageInfo != null && mPackageInfo.versionName != null) {
            appVersion.setVisibility(View.VISIBLE);
            appVersion.setText(getString(R.string.version_text,
                    String.valueOf(mPackageInfo.versionName)));
        } else {
            appVersion.setVisibility(View.INVISIBLE);
        }
    }

    class PermAdapter extends BaseAdapter {
        List<PermissionRecord> mPermRecordList = new ArrayList<PermissionRecord>();

        public PermAdapter() {

        }

        public void setDataAndNotify(List<PermissionRecord> permList) {
            mPermRecordList = permList;
            notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            if (mPermRecordList != null) {
                return mPermRecordList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return mPermRecordList.get(position);
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
                holder.mPermIcon = (ImageView) convertView
                        .findViewById(R.id.app_icon);
                holder.mPermName = (TextView) convertView
                        .findViewById(R.id.app_name);
                holder.mPermStatus = (TextView) convertView
                        .findViewById(R.id.app_size);
                convertView.setTag(holder);
            } else {
                // Get the ViewHolder back to get fast access to the TextView
                // and the ImageView.
                holder = (AppViewHolder) convertView.getTag();
            }

            // Bind the data efficiently with the holder
            PermissionRecord perminfo = mPermRecordList.get(position);
            String permName = perminfo.mPermissionName;
            String permLabel = PermControlUtils.getPermissionLabel(permName,
                    mPermLabelArray);
            String status = mEntries[UiUtils.getSelectIndex(perminfo
                    .getStatus())];
            holder.mPermIcon.setImageResource(PermControlUtils
                    .getPermissionIcon(permName));
            holder.mPermName.setText(permLabel);
            holder.mPermStatus.setText(status);

            Xlog.d(TAG, "getView for pka name = " + mPkgName + " ,permLabel = "
                    + permLabel + " ,perm status " + perminfo.getStatus()
                    + " , dialog status = " + status);
            return convertView;
        }
    }

    private class AppToPermAsyncLoader extends
            AsyncTask<Void, Integer, List<PermissionRecord>> {
        @Override
        protected List<PermissionRecord> doInBackground(Void... params) {
            List<PermissionRecord> permsList = DatabaseManager
                    .getPermRecordListByPkgName(mPkgName);
            Xlog.d(TAG, "doInBackground......");
            if (permsList == null) {
                Xlog.d(TAG, "permsList == null");
                return null;
            }
            if (isCancelled()) {
                Xlog.d(TAG, "the Async Task is cancled");
                return null;
           }
            // get the mSavedSelectedIndex if mSelectPermInfo != null 
            if (mSelectPermInfo != null) {
                for (PermissionRecord record : permsList) {
                    if (record.mPermissionName.equals(mSelectPermInfo.mPermissionName)) {
                           mSavedSelectedIndex =  UiUtils.getSelectIndex(record.getStatus());
                           break;
                    }
                 }
            }
            
            // sort list by the defined perm list
            Collections.sort(permsList, DEFINED_PERM_COMPARATOR);
            return permsList;
        }

        @Override
        protected void onPostExecute(List<PermissionRecord> permsList) {
            Xlog.d(TAG, "onPostExecute......");
            // as the alert dialog is on the top of listview ,so refresh it firstly
            updateAlertDialog();
            mApdater.setDataAndNotify(permsList);
        }
    }

    private void load() {
         mAsyncTask = (AppToPermAsyncLoader)new AppToPermAsyncLoader().execute();
    }

    // comparator for sort the app's perm list by the defined controlled perm list
    public static final Comparator<PermissionRecord> DEFINED_PERM_COMPARATOR = new Comparator<PermissionRecord>() {
        private final Collator mCollator = Collator.getInstance();

        @Override
        public int compare(PermissionRecord obj1, PermissionRecord obj2) {
            int obj1Index = PermControlUtils
                    .getPermissionIndex(obj1.mPermissionName);
            int obj2Index = PermControlUtils
                    .getPermissionIndex(obj2.mPermissionName);
            return mCollator.compare(String.valueOf(obj1Index),
                    String.valueOf(obj2Index));
        }
    };

    // View Holder used when displaying views
    static class AppViewHolder {
        TextView mPermName;
        ImageView mPermIcon;
        TextView mPermStatus;
    }

    private void checkUiEnabled() {
        boolean enable = PermControlUtils
                .isInHouseEnabled(AppToPermissionActivity.this);
        Xlog.d(TAG, "checkUiEnabled(), update current interface, enable = "
                + enable);
        if (!enable) {
            // set result for previous activity: PermissionControlPageActivity
            setResult(UiUtils.RESULT_FINISH_ITSELF);
            Xlog.d(TAG, "finish itself");
            finish();
        }
    }

    /***to fix the issue: 
     * 1) show the dialog of change permission ,must keep it showing
     * 2) press home key to exit
     * 3) enter the app requested this permission to change the permission status
     * 4) enter this interface again 
     * error result: the dialog select index is not right.
     * So must refresh dialog status.
    ****/
    private void updateAlertDialog() {
        if (mAlertDlg == null || !mAlertDlg.isShowing()
                || mSelectPermInfo == null) {
            Xlog.d(TAG, "mAlertDlg = " + mAlertDlg + " ,mSelectPermInfo = "
                    + mSelectPermInfo);
            return;
        }
        Xlog.d(TAG, "set alertDialog select mSavedSelectedIndex = "
                + mSavedSelectedIndex);
        ListView listview = mAlertDlg.getListView();
        listview.setItemChecked(mSavedSelectedIndex, true);
        listview.setSelection(mSavedSelectedIndex);
    }
}