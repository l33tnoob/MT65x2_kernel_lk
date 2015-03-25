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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ListView; 
import android.widget.TextView;

import com.mediatek.common.mom.IMobileManager;
import com.mediatek.common.mom.ReceiverRecord;
import com.mediatek.security.R;
import com.mediatek.security.service.PermControlUtils;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;


public class AutoBootAppManageActivity extends Activity implements OnItemClickListener {

    private static final String TAG = "AutoBootAppActivity";

    private LayoutInflater mInflater;
    private Context mCxt;
    private IMobileManager mMoMService;
    
    private ListView mListView;
    private TextView mSummaryText;
    private TextView mEmptyView;
    private AutoBootAdapter mApdater;
    private AutoBootAysncLoader mAsyncTask;
    // flag: the checkbox changed is by user click or not
    private boolean mUserCheckedFlag;
    
    // Receiver to handle package update broadcast
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent != null) {
                String action = intent.getAction();
                Xlog.d(TAG, "onReceive with action = " + action);
                if (IMobileManager.ACTION_PACKAGE_CHANGE.equals(action)) {
                    load();
                }
            }
        }
    };
    
    private void registerReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(IMobileManager.ACTION_PACKAGE_CHANGE);
        registerReceiver(mReceiver,intentFilter);
    }
    
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (mMoMService == null) {
            mMoMService = (IMobileManager)getSystemService(Context.MOBILE_SERVICE);
        }
        mInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mCxt = getApplicationContext();

        setContentView(R.layout.auto_boot_app_overview);

        ListView lv = (ListView) findViewById(android.R.id.list);
        lv.setSaveEnabled(true);
        lv.setItemsCanFocus(true);
        lv.setOnItemClickListener(this);
        lv.setTextFilterEnabled(true);
        
        mSummaryText = (TextView)findViewById(R.id.autoboot_app_count_txt);
        mEmptyView = (TextView) findViewById(R.id.empty);
        mListView = lv;
        mApdater = new AutoBootAdapter();
        mListView.setAdapter(mApdater);
    }

    @Override
    public void onResume() {
        Xlog.i(TAG, "onResume");
        super.onResume();   
        // loading the list data
        load();
        registerReceiver();
    }

    @Override
    public void onPause() {
        Xlog.i(TAG, "onPause");
        super.onPause();
        unregisterReceiver(mReceiver);
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
        // get object
        ListView l = (ListView) parent;
        AutoBootAdapter adapter = (AutoBootAdapter) l.getAdapter();
        ReceiverRecord info = (ReceiverRecord) adapter.getItem(position);
        String pkgName = info.packageName;
        
        // get current status , change to opp
        boolean status = mMoMService.getBootReceiverEnabledSetting(pkgName);
        Xlog.d(TAG, "onItemClick , " + pkgName + " current status = "
                 + status + ", will change to " + !status);
        
        // just change checkbox status , then will trigger checkbox's checkedChangeListener
        CheckBox statusBox = (CheckBox) view.findViewById(R.id.status);
        statusBox.setChecked(!status);
    }

    private void load() {
         mAsyncTask = (AutoBootAysncLoader)new AutoBootAysncLoader().execute();
    }
    
    void refreshUi(boolean dataChanged) {
        AutoBootAdapter adapter = (AutoBootAdapter)(mListView.getAdapter());
        if (dataChanged) {
            adapter.notifyDataSetChanged();
        }
        // if the data list is null , set the empty notify text
        if (adapter.mReceiverList == null || adapter.mReceiverList.size() == 0) {
            mEmptyView.setVisibility(View.VISIBLE);
            mSummaryText.setVisibility(View.GONE);
        } else {
            mEmptyView.setVisibility(View.GONE);
            mSummaryText.setVisibility(View.VISIBLE);
        }
        
        int enableCount = 0;
        int disableCount = 0;
        int size = 0;
        if (adapter.mReceiverList != null) {
             size = adapter.mReceiverList.size();
        }
        
        for (int i = 0 ; i < size ; i++) {
            String pkg = adapter.mReceiverList.get(i).packageName;
            // must get status again according to the package name ,
            // don't use  adapter.mReceiverList.get(i).enableadd directly
            boolean status = mMoMService.getBootReceiverEnabledSetting(pkg);
            if (status) {
                enableCount++;
            } else {
                disableCount++;
            }
        }
        Xlog.d(TAG,"enableCount = " + enableCount + ",disableCount = " + disableCount);       
        mSummaryText.setText(getResources().getQuantityString(R.plurals.autoboot_app_desc_allow,
                enableCount,enableCount) 
                + getResources().getQuantityString(R.plurals.autoboot_app_desc_deny, 
                 disableCount,disableCount)) ;
    }

    
    class AutoBootAdapter extends BaseAdapter {
        List<ReceiverRecord> mReceiverList = new ArrayList<ReceiverRecord>();

        public AutoBootAdapter() {

        }

        public void setDataAndNotify(List<ReceiverRecord> receiverList) {
            mReceiverList = receiverList;
            refreshUi(true);
        }

        @Override
        public int getCount() {
            if (mReceiverList != null) {
                return mReceiverList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int position) {
            return mReceiverList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View v;
            if (convertView == null) {
                v = newView(parent,position);
            } else {
                v = convertView;
            }
            bindView(v, position);
            return v;
        }

        public View newView(ViewGroup parent,int pos) {
            View v = mInflater.inflate(R.layout.auto_boot_app_item, parent, false);
            new AppViewHolder(v);
            return v;
        }
        
        public void bindView(View view, int position) {
            ReceiverRecord infoItem = mReceiverList.get(position);
            AppViewHolder vh = (AppViewHolder) view.getTag();
            vh.bind(infoItem); 
        }
        
    }
    

    
    private class AutoBootAysncLoader extends
            AsyncTask<Void, Integer, List<ReceiverRecord>> {
        @Override
        protected List<ReceiverRecord> doInBackground(Void... params) {
            Xlog.d(TAG, "doInBackground......");
            // get the origin data
            if (isCancelled()) {
                Xlog.d(TAG, "the Async Task is cancled");
                return null;
            }
            
            List<ReceiverRecord> recordList = mMoMService.getBootReceiverList();
            if (recordList == null) {
                Xlog.d(TAG, "no 3rd party app will auto boot");
                return null;
            }
            Xlog.d(TAG,"recordList size = " + recordList.size());
            return recordList;
        }

        @Override
        protected void onPostExecute(List<ReceiverRecord> receiverList) {
            Xlog.d(TAG, "onPostExecute......");
            mApdater.setDataAndNotify(receiverList);
        }
    }


    // View Holder used when displaying views
    public class AppViewHolder {
        public View rootView;
        public TextView mPkgLabel;
        public ImageView mAppIcon;
        public CheckBox mBootEnable;
        
        public AppViewHolder(View v) {
            rootView = v;
            mAppIcon = (ImageView) v.findViewById(R.id.app_icon);
            mPkgLabel = (TextView) v.findViewById(R.id.app_name);
            mBootEnable = (CheckBox) v.findViewById(R.id.status);
            v.setTag(this);
        }
        
        public void bind(ReceiverRecord infoItem) {
            String pkgName = infoItem.packageName;
            String label = PermControlUtils.getApplicationName(mCxt,
                    pkgName);
            if (label != null) {
                Drawable icon = PermControlUtils.getApplicationIcon(mCxt,
                        pkgName);
                mAppIcon.setImageDrawable(icon);
                mPkgLabel.setText(label);
                // get the pks's current status , CR: ALPS01414339
                boolean status = mMoMService.getBootReceiverEnabledSetting(pkgName);
                Xlog.d(TAG,"bindView , check " + label + " status to " + status);
                mUserCheckedFlag = false;
                mBootEnable.setChecked(status);
                mUserCheckedFlag = true;
            }
            
            if (mBootEnable != null) {
                final ReceiverRecord info = infoItem;
                final String pkg = info.packageName;
                mBootEnable.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                    public void onCheckedChanged(CompoundButton buttonView , boolean isChecked) {
                    	// CR: ALPS01414339 {@
                    	if (!mUserCheckedFlag) {
                    	    Xlog.d(TAG,"mUserCheckedFlag is false , return ");
                    	    return;
                    	}
                    	// @}
                        final Boolean status = mMoMService.getBootReceiverEnabledSetting(pkg);
                        // enable or disable auto boot permission
                        Xlog.d(TAG, "click checkbox , set " + pkg + " to " + isChecked);
                        mMoMService.setBootReceiverEnabledSetting(pkg, isChecked);
                        AutoBootAppManageActivity.this.refreshUi(false);
                    }
                });
            }
        }
    }

}
