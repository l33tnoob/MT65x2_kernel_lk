/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

package com.mediatek.security.tests.functional;

import com.mediatek.security.service.PermControlUtils;
import com.mediatek.security.ui.PermissionControlPageActivity;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.text.TextUtils;
import android.webkit.WebView;
import android.widget.FrameLayout;
import android.widget.GridLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

import com.jayway.android.robotium.solo.Solo;

import com.mediatek.security.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class PermControlMainUiTest extends ActivityInstrumentationTestCase2<PermissionControlPageActivity> {
    
    private static final String TAG = "PermControlMainUiTest";
    private static final int TIMES_CLICK_SWITCH = 2;
    
    private Solo mSolo;
    private Activity mActivity;
    private Context mContext;
    private Instrumentation mIns;
    private ContentResolver mCr;
    private Switch mSwitch;

    public PermControlMainUiTest(){
       super("com.mediatek.security.ui",PermissionControlPageActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mIns = getInstrumentation();
        mContext = mIns.getTargetContext();
        mActivity = getActivity();
        mSolo = new Solo(mIns, mActivity);
        mCr = mContext.getContentResolver();
        mSwitch = (Switch) mActivity.getActionBar().getCustomView();
    }

    public void test01_SwitchPermissionControl() throws InterruptedException {
        if(mSwitch.isEnabled() && mSwitch.isChecked()) {
            //Turn off permission control 
            if(setPermissionSwitchOffDialogStateClosed()) {
                mSolo.clickOnView(mSwitch);
                mSolo.sleep(5000);
                assertEquals(mSwitch.isChecked(), PermControlUtils.isPermControlOn(mContext));
            }
            //Turn on permission control
            mSolo.clickOnView(mSwitch);
            mSolo.sleep(5000);
            assertEquals(mSwitch.isChecked(), PermControlUtils.isPermControlOn(mContext));
        } else if(mSwitch.isEnabled() && !(mSwitch.isChecked())) {
            //Turn on permission control
            mSolo.clickOnView(mSwitch);
            mSolo.sleep(5000);
            assertEquals(mSwitch.isChecked(), PermControlUtils.isPermControlOn(mContext));
            //Turn off permission control
            if(setPermissionSwitchOffDialogStateClosed()) {
                mSolo.clickOnView(mSwitch);
                mSolo.sleep(5000);
                assertEquals(mSwitch.isChecked(), PermControlUtils.isPermControlOn(mContext));
            }
        }
    }
    
    private boolean setPermissionSwitchOffDialogStateClosed() {
        boolean isShowDlg = Settings.System.getInt(mCr,
                PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, 0) == 0;
        Xlog.d(TAG, "isShowDlg: " + isShowDlg);
        if (isShowDlg) {
            Settings.System.putInt(mCr,
                    PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, 1);
            mSolo.sleep(1000);
        }
        return Settings.System.getInt(mCr,
                PermControlUtils.PERMISSION_SWITCH_OFF_DLG_STATE, 0) == 1;
    }
    
    public void test02_PermissionsTab() {
        if(turnOnPermissionControl()) {
            boolean zeroAppPermissionItemClicked = false;
            boolean multipleAppPermissionItemClicked = false;
            String permissionLabelArray[] = mActivity.getResources().getStringArray(R.array.permission_label);
            for (int i = 0; i < permissionLabelArray.length; i++) {
                if (mSolo.searchText(permissionLabelArray[i])) {
                    Xlog.d(TAG, "start to check " + permissionLabelArray[i]);
                    //start to find the app count view 
                    TextView label = getTextView(permissionLabelArray[i]);
                    if(label == null) {
                        Xlog.d(TAG, "Can not find the permission label textview");
                        continue;
                    }
                    GridLayout parent = (GridLayout)label.getParent();
                    if(parent == null) {
                        Xlog.d(TAG, "Can not find the permission label parent GridLayout");
                        continue;
                    }
                    TextView appCountView = (TextView)parent.getChildAt(3);
                    if(appCountView == null) {
                        Xlog.d(TAG, "Can not find the app count text view");
                        continue;
                    }
                    //Find app count view end
                    String ZERO_APP_COUNT_TEXT_SUMMARY = "0 " + mActivity.getString(R.string.app_count_unit_single);
                    Xlog.d(TAG, "Const zero app count is " + ZERO_APP_COUNT_TEXT_SUMMARY + ", summary is " + appCountView.getText());
                    if(ZERO_APP_COUNT_TEXT_SUMMARY.equals(appCountView.getText())) {
                        //no app permission item
                        if(zeroAppPermissionItemClicked) {
                            continue;
                        }
                        mSolo.clickOnText(permissionLabelArray[i]);
                        mSolo.sleep(1000);
                        if(mSolo.searchText(mActivity.getString(R.string.no_app_use_perm))) {
                            Xlog.d(TAG, "Success to find the empty view text");
                        }
                        zeroAppPermissionItemClicked = true;
                        mSolo.goBack();
                    } else {
                        if(multipleAppPermissionItemClicked) {
                            continue;
                        }
                        //one or multiple apps permission summary
                        mSolo.clickOnText(permissionLabelArray[i]);
                        mSolo.sleep(1000);
                        //start to find the view of Permission to APP item
                        if (mSolo.searchText(permissionLabelArray[i])) {
                            TextView tv = (TextView)getTextView(permissionLabelArray[i]);
                            if(tv == null) {
                                Xlog.d(TAG, "Can not find the permission label text view");
                                continue;
                            }
                            LinearLayout layout = (LinearLayout)tv.getParent();
                            if(layout == null) {
                                Xlog.d(TAG, "Can not find the permission label parent, return");
                                continue;
                            }
                            FrameLayout framelayout = (FrameLayout)layout.getChildAt(1);
                            if(framelayout == null) {
                                Xlog.d(TAG, "Can not find permission to app framelayout, return");
                                continue;
                            }
                            ListView listview = (ListView)framelayout.getChildAt(0);
                            if(listview == null) {
                                Xlog.d(TAG, "Can not find permission to app listview, return");
                                continue;
                            }
                            
                            GridLayout gridlayout = (GridLayout)listview.getChildAt(0);
                            if(gridlayout == null) {
                                Xlog.d(TAG, "Can not find the item 0 grid layout");
                                continue;
                            }
                            //Find the view of Permission to APP item end
                            String entries[] = mActivity.getResources().getStringArray(R.array.perm_status_entry);
                            final ArrayList<String> entryList = new ArrayList<String>(Arrays.asList(entries));
                            //start to select ""Always allow/Always ask/Always deny" for app
                            for(int j = 0; j < entries.length; j ++) {
                                mSolo.clickOnView(gridlayout);
                                int index = -1;
                                ArrayList<TextView> mTextviewList = mSolo.clickInList(j + 1);
                                for(TextView tv1 : mTextviewList) {
                                    Xlog.d(TAG, "Text view array list is " + tv1.getText());
                                }
                                if(mTextviewList.size() != 1) {
                                    Xlog.d(TAG, "click view size > 1, return");
                                    continue;
                                } else {
                                    String title = mTextviewList.get(0).getText().toString();
                                    Xlog.d(TAG, "title is " + title);
                                    index = entryList.indexOf(title);
                                    Xlog.d(TAG, "index is " + index);
                                }
                                
                                if(index == -1) {
                                    Xlog.d(TAG, "can not find the click index, return");
                                    continue;
                                }
                                mSolo.sleep(1000);
                                //Todo: Add equals
                            }
                            //Select ""Always allow/Always ask/Always deny" for app end
                        }
                        multipleAppPermissionItemClicked = true;
                        mSolo.goBack();
                    }
                }
                if(zeroAppPermissionItemClicked && multipleAppPermissionItemClicked) {
                    Xlog.d(TAG, "Already click on the one app permission item and multiple app permission item, break");
                    break;
                }
            }
        }
    }
    
    private boolean turnOnPermissionControl() {
        if(mSwitch.isChecked()) {
            Xlog.d(TAG, "permission control is on, return");
            return true;
        } else if (mSwitch.isEnabled() && !(mSwitch.isChecked())) {
            mSolo.clickOnView(mSwitch);
            mSolo.sleep(5000);
            return mSwitch.isChecked();
        } else {
            Xlog.d(TAG, "permission control can not trun on, return");
           return false;
        }
    }
    
    private TextView getTextView(String text) {
        ArrayList<TextView> views = mSolo.getCurrentViews(TextView.class);
        TextView viewToReturn = null;
        for(TextView view : views) {
            if(view.getText().toString().equals(text)) {
                viewToReturn = view;
            }
        }
        return viewToReturn;
    }
    
    @Override
    protected void tearDown() throws Exception {
        //turn off permission control
        PermControlUtils.enablePermissionControl(false, mContext);
        try{
            mSolo.finishOpenedActivities();
        }catch(Exception e){
            // ignore
        }
        super.tearDown();
    }
}
