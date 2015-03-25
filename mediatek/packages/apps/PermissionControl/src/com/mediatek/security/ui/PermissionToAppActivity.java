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
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
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


public class PermissionToAppActivity extends Activity implements OnItemClickListener {

    private static final String TAG = "PermToAppActivity";

    private LayoutInflater mInflater;
    private Context mCxt;

    private ListView mListView;
    private PermToAppAdapter mApdater;

    private String mPermName;
    private String[] mEntries;

    private MyPermissionRecord mSelectPermInfo;

    // Dialog identifiers used in showDialog
    private static final int DLG_BASE = 0;
    private static final int DLG_CHANEG_PERM_STATUS = DLG_BASE + 1;
    AlertDialog mAlertDlg;
    private int mSavedSelectedIndex;
    
    private PermToAppAsyncLoader mAsyncTask;

    private final BroadcastReceiver mPkgChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PermControlUtils.PERM_CONTROL_DATA_UPDATE)) {
                Xlog.d(TAG, "receiver ,re-loading....... ");
                load();
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
        mPermName = bundle != null ? bundle.getString("permName") : null;
        Xlog.d(TAG, "get perm name " + mPermName);
        if (mPermName == null) {
            Xlog.w(TAG, "finish itself because of pkgName is null");
            finish();
        }

        mEntries = getResources().getStringArray(R.array.perm_status_entry);

        mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCxt = getApplicationContext();

        setContentView(R.layout.manage_permission_app_details);

        ListView lv = (ListView) findViewById(android.R.id.list);
        lv.setOnItemClickListener(this);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setTextFilterEnabled(true);
        mListView = lv;
        mApdater = new PermToAppAdapter();
        mListView.setAdapter(mApdater);

        TextView permTxt = (TextView) findViewById(R.id.perm_name);
        String[] permLabelArray = getResources().getStringArray(
                R.array.permission_label);
        permTxt.setText(PermControlUtils.getPermissionLabel(mPermName,
                permLabelArray));
    }

    @Override
    public void onResume() {
        Xlog.i(TAG, "onResume");
        super.onResume();
        
        // firstly check enabled or not
        checkUiEnabled();
        
        // loading the list data
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
        mSelectPermInfo = (MyPermissionRecord) l.getAdapter().getItem(position);
        showDialog(DLG_CHANEG_PERM_STATUS);
    }

    @Override
    public Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
        case DLG_CHANEG_PERM_STATUS:
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            Xlog.d(TAG, "onCreateDialog , get status = " + mSelectPermInfo.getStatus());
            mSavedSelectedIndex =  UiUtils.getSelectIndex(mSelectPermInfo.getStatus());
            builder.setTitle(mSelectPermInfo.mPkgLabel);
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
                    Xlog.d(TAG,"dialog dismiss, remove it");
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

    class PermToAppAdapter extends BaseAdapter {
        List<MyPermissionRecord> mPermRecordList = new ArrayList<MyPermissionRecord>();

        public PermToAppAdapter() {

        }

        public void setDataAndNotify(List<MyPermissionRecord> permRecordList) {
            mPermRecordList = permRecordList;
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
                holder.mAppIcon = (ImageView) convertView
                        .findViewById(R.id.app_icon);
                holder.mPkgLabel = (TextView) convertView
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
            MyPermissionRecord perminfo = mPermRecordList.get(position);
            String status = mEntries[UiUtils.getSelectIndex(perminfo
                    .getStatus())];

            holder.mAppIcon.setImageDrawable(perminfo.mAppIcon);
            holder.mPkgLabel.setText(perminfo.mPkgLabel);
            holder.mPermStatus.setText(status);

            Xlog.d(TAG,
                    "getView for perm name = " + mPermName + ", pkg = "
                            + perminfo.mPkgLabel + " perm status "
                            + perminfo.getStatus() + ", dialog status = "
                            + status);
            return convertView;
        }

    }

    private class PermToAppAsyncLoader extends
            AsyncTask<Void, Integer, List<MyPermissionRecord>> {
        @Override
        protected List<MyPermissionRecord> doInBackground(Void... params) {
            Xlog.d(TAG, "doInBackground......");
            // get the origin data
            List<PermissionRecord> recordList = DatabaseManager
                    .getPermRecordListByPermName(mPermName);
            if (recordList == null) {
                Xlog.d(TAG, "no app has the perm");
                return null;
            }
            // encrypt the data to get view quickly
            List<MyPermissionRecord> appsList = new ArrayList<MyPermissionRecord>();

            for (PermissionRecord record : recordList) {
                if (isCancelled()) {
                    Xlog.d(TAG, "the Async Task is cancled");
                    return null;
                }
                String pkgName = record.mPackageName;
                int status = record.getStatus();
                MyPermissionRecord myRecord = new MyPermissionRecord(pkgName,
                        record.mPermissionName, status);
                myRecord.mPkgLabel = PermControlUtils.getApplicationName(mCxt,
                        pkgName);
                Xlog.d(TAG,"get  label " + pkgName + "," + myRecord.mPkgLabel);
                if (myRecord.mPkgLabel != null) {
                    myRecord.mAppIcon = PermControlUtils.getApplicationIcon(mCxt,
                            pkgName);
                    appsList.add(myRecord);
                    // get the mSavedSelectedIndex if mSelectPermInfo != null 
                    if (mSelectPermInfo != null 
                        && myRecord.mPkgLabel.equals(mSelectPermInfo.mPkgLabel)
                        && myRecord.mPermissionName.equals(mSelectPermInfo.mPermissionName)) {
                           mSavedSelectedIndex =  UiUtils.getSelectIndex(status);
                    }
                }
            }
            // sort list by the package label
            Collections.sort(appsList, ALPHA_COMPARATOR);
            return appsList;
        }

        @Override
        protected void onPostExecute(List<MyPermissionRecord> permsList) {
            Xlog.d(TAG, "onPostExecute......");
            // as the alert dialog is on the top of listview ,so refresh it firstly
            updateAlertDialog();
            // if the data list is null , set the empty notify text
            if (permsList == null || permsList.size() == 0) {
                View emptyView = (View) findViewById(R.id.empty);
                emptyView.setVisibility(View.VISIBLE);
            }
            mApdater.setDataAndNotify(permsList);
        }
    }

    private void load() {
         mAsyncTask = (PermToAppAsyncLoader)new PermToAppAsyncLoader().execute();
    }

    // View Holder used when displaying views
    static class AppViewHolder {
        TextView mPkgLabel;
        ImageView mAppIcon;
        TextView mPermStatus;
    }

    class MyPermissionRecord extends PermissionRecord {
        private String mPkgLabel;
        private Drawable mAppIcon;

        public MyPermissionRecord(String packageName, String permissionName,
                int status) {
            super(packageName, permissionName, status);
        }
    }

    // comparator for sorting the app list
    public static final Comparator<MyPermissionRecord> ALPHA_COMPARATOR = new Comparator<MyPermissionRecord>() {
        private final Collator mCollator = Collator.getInstance();

        @Override
        public int compare(MyPermissionRecord object1,
                MyPermissionRecord object2) {
            return mCollator.compare(object1.mPkgLabel, object2.mPkgLabel);
        }
    };
    
    
    private void checkUiEnabled() {
        boolean enable = PermControlUtils
                .isInHouseEnabled(PermissionToAppActivity.this);
        Xlog.d(TAG, "onChange(), update current interface, enable = " + enable);
        if (!enable) {
            // set result for previous activity:PermissionControlPageActivity
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
