/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.android.stk;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnCreateContextMenuListener;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.CatLog;

import android.widget.AdapterView.AdapterContextMenuInfo;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Context;
import android.provider.Settings.System;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import android.graphics.Color;
import com.mediatek.common.featureoption.FeatureOption;

import android.view.Gravity;
import android.widget.Toast;

import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

/**
 * ListActivity used for displaying STK menus. These can be SET UP MENU and
 * SELECT ITEM menus. This activity is started multiple times with different
 * menu content.
 *
 */
public class StkMenuActivityII extends ListActivity {
    private StkMenuInstance mMenuInstance = new StkMenuInstance();
    private TextView mTitleTextView = null;
    private ImageView mTitleIconView = null;
    private ProgressBar mProgressView = null;
    private static final String LOGTAG = "Stk2-MA ";
    
    
    private final IntentFilter mSIMStateChangeFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - simId[" + simId + "]  state[" + simState + "]");
                if ((simId == com.android.internal.telephony.PhoneConstants.GEMINI_SIM_2) && 
                    (IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(simState) ||
                     IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simState))) {
                    CatLog.d(LOGTAG, "mSendResp: " + mMenuInstance.mSendResp);
                    if (!mMenuInstance.mSendResp)
                    {
                        StkMenuActivityII.this.mMenuInstance.sendResponse(StkAppService.RES_ID_END_SESSION);
                    }
                    StkMenuActivityII.this.mMenuInstance.cancelTimeOut();
                    StkMenuActivityII.this.finish();
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        CatLog.d(LOGTAG, "onCreate+");
        // Remove the default title, customized one is used.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        // Set the layout for this activity.
        setContentView(R.layout.stk_menu_list);

        mTitleTextView = (TextView) findViewById(R.id.title_text);
        mTitleIconView = (ImageView) findViewById(R.id.title_icon);
        mProgressView = (ProgressBar) findViewById(R.id.progress_bar);
        getListView().setOnCreateContextMenuListener(mOnCreateContextMenuListener);
        if (!mMenuInstance.handleOnCreate(getBaseContext(), getIntent(), true))
    	{
    	    CatLog.d(LOGTAG, "finish!");
            finish();
    	}

                /*
        // Change content background color to support theme mananger.
        if (FeatureOption.MTK_THEMEMANAGER_APP) {
            View contentView = this.findViewById(android.R.id.content);
            contentView.setThemeContentBgColor(Color.TRANSPARENT);
        }
        */
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
        CatLog.d(LOGTAG, "onCreate-");
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        CatLog.d(LOGTAG, "onNewIntent");
        if (!mMenuInstance.handleNewIntent(intent, true))
        {
    	    CatLog.d(LOGTAG, "finish!");
            finish();
    	}
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        mMenuInstance.handleListItemClick(position, mProgressView);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        boolean result = mMenuInstance.handleKeyDown(keyCode, event);
		if (result)
            return result;
		else
            return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onResume() {
        super.onResume();

        CatLog.d(LOGTAG, "onResume, sim id: " + mMenuInstance.mSimId);
        int res = mMenuInstance.handleResume(mTitleIconView, mTitleTextView, this, mProgressView);
        switch(res)
        {
            case StkMenuInstance.FINISH_CAUSE_FLIGHT_MODE:
                mMenuInstance.showTextToast(getApplicationContext(), getString(R.string.lable_on_flight_mode));
                finish();
                break;
            case StkMenuInstance.FINISH_CAUSE_NULL_MENU:
                mMenuInstance.showTextToast(getApplicationContext(), getString(R.string.main_menu_not_initialized));
                finish();
                break;
            case StkMenuInstance.FINISH_CAUSE_NULL_SERVICE:
                finish();
                break;
            case StkMenuInstance.FINISH_CAUSE_NOT_AVAILABLE:
                mMenuInstance.showTextToast(getApplicationContext(), getString(R.string.lable_sim_not_ready));
                finish();
                break;
            case StkMenuInstance.FINISH_CAUSE_SIM_REMOVED:
                mMenuInstance.showTextToast(getApplicationContext(), getString(R.string.no_sim_card_inserted));
                finish();
                break;
        }
    }

    @Override
    public void onPause() {
        super.onPause();
		CatLog.d(LOGTAG, "onPause, sim id: " + mMenuInstance.mSimId);
        mMenuInstance.handlePause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mSIMStateChangeReceiver);
        CatLog.d(LOGTAG, "onDestroy");
    }
    // For long click menu
    private final OnCreateContextMenuListener mOnCreateContextMenuListener =
        new OnCreateContextMenuListener() {
        public void onCreateContextMenu(ContextMenu menu, View v,
                ContextMenuInfo menuInfo) {
            boolean helpVisible = false;
            if (mMenuInstance.mStkMenu != null) {
                helpVisible = mMenuInstance.mStkMenu.helpAvailable;
            }
            if(helpVisible == true) {
                menu.add(0, StkApp.MENU_ID_HELP, 0, R.string.help);
            }
        }
    };

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        final AdapterContextMenuInfo info =
            (AdapterContextMenuInfo) item.getMenuInfo();
        switch (item.getItemId()) {
        case StkApp.MENU_ID_HELP:
            mMenuInstance.cancelTimeOut();
            mMenuInstance.mAcceptUsersInput = false;
            Item stkItem = mMenuInstance.getSelectedItem(info.position);
            if (stkItem == null) {
                break;
            }
            // send help needed response.
            mMenuInstance.sendResponse(StkAppService.RES_ID_MENU_SELECTION, stkItem.id, true);
            return true;
        default:
            break;
        }
        return super.onContextItemSelected(item);
    }

    @Override
    public boolean onCreateOptionsMenu(android.view.Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.add(0, StkApp.MENU_ID_END_SESSION, 1, R.string.menu_end_session);
        menu.add(0, StkApp.MENU_ID_HELP, 2, R.string.help);
        menu.add(0, StkApp.MENU_ID_DEFAULT_ITEM, 3, R.string.help);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(android.view.Menu menu) {
        super.onPrepareOptionsMenu(menu);
        
        return mMenuInstance.handlePrepareOptionMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        boolean result = mMenuInstance.handleOptionItemSelected(item, mProgressView);
		CatLog.d(LOGTAG, "onOptionsItemSelected, result: " + result);
        if (result)
            return result;
		else
            return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putInt("STATE", mMenuInstance.mState);
        outState.putParcelable("MENU", mMenuInstance.mStkMenu);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        mMenuInstance.mState = savedInstanceState.getInt("STATE");
        mMenuInstance.mStkMenu = savedInstanceState.getParcelable("MENU");
    }
}
