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

package com.android.stk2;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.view.ContextMenu;
import android.view.Gravity;
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
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;

import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Menu;

/**
 * ListActivity used for displaying STK menus. These can be SET UP MENU and
 * SELECT ITEM menus. This activity is started multiple times with different
 * menu content.
 */
public class StkMenuActivity extends ListActivity {
    private Context mContext;
    private Menu mStkMenu = null;
    private int mState = STATE_MAIN;
    private boolean mAcceptUsersInput = true;

    private TextView mTitleTextView = null;
    private ImageView mTitleIconView = null;
    private ProgressBar mProgressView = null;
    private static final String LOGTAG = "Stk2-MA ";

    StkAppService appService = StkAppService.getInstance();

    private final IntentFilter mSIMStateChangeFilter = new IntentFilter(
            TelephonyIntents.ACTION_SIM_STATE_CHANGED);

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCard.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(
                        com.android.internal.telephony.Phone.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - simId[" + simId + "]  state["
                        + simState + "]");
                if ((simId == com.android.internal.telephony.Phone.GEMINI_SIM_2) &&
                        (IccCard.INTENT_VALUE_ICC_NOT_READY.equals(simState))) {
                    CatLog.d(LOGTAG, "StkMenuActivity.finish()");
                    StkMenuActivity.this.cancelTimeOut();
                    StkMenuActivity.this.finish();
                }
            }
        }
    };

    // Internal state values
    static final int STATE_MAIN = 1;
    static final int STATE_SECONDARY = 2;
    static final int STATE_END = 3;

    // message id for time out
    private static final int MSG_ID_TIMEOUT = 1;

    Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ID_TIMEOUT:
                mAcceptUsersInput = false;
                sendResponse(StkAppService.RES_ID_TIMEOUT);
                break;
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
        mContext = getBaseContext();
        getListView().setOnCreateContextMenuListener(mOnCreateContextMenuListener);
        initFromIntent(getIntent());
        mAcceptUsersInput = true;

        /*
         * // Change content background color to support theme mananger. if
         * (FeatureOption.MTK_THEMEMANAGER_APP) { View contentView =
         * this.findViewById(android.R.id.content);
         * contentView.setThemeContentBgColor(Color.TRANSPARENT); }
         */

        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
        CatLog.d(LOGTAG, "onCreate-");
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        CatLog.d(LOGTAG, "onNewIntent");
        initFromIntent(intent);
        mAcceptUsersInput = true;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        if (!mAcceptUsersInput) {
            return;
        }

        Item item = getSelectedItem(position);
        if (item == null) {
            return;
        }
        sendResponse(StkAppService.RES_ID_MENU_SELECTION, item.id, false);
        mAcceptUsersInput = false;
        mProgressView.setVisibility(View.VISIBLE);
        mProgressView.setIndeterminate(true);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        if (!mAcceptUsersInput) {
            return true;
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                CatLog.d(LOGTAG, "onKeyDown - KEYCODE_BACK - mState[" + mState + "]");
                switch (mState) {
                    case STATE_SECONDARY:
                        cancelTimeOut();
                        mAcceptUsersInput = false;
                        CatLog.d(LOGTAG, "onKeyDown - KEYCODE_BACK - STATE_SECONDARY");
                        sendResponse(StkAppService.RES_ID_BACKWARD);
                        return true;
                    case STATE_MAIN:
                        break;
                }
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onResume() {
        super.onResume();

        if (isOnFlightMode() == true) {
            CatLog.d(LOGTAG, "don't make stk be visible");
            showTextToast(getString(R.string.lable_on_flight_mode));

            finish();
            return;
        }

        if (appService == null) {
            CatLog.d(LOGTAG, "can not launch stk menu 'cause null StkAppService");
            finish();
            return;
        }

        appService.indicateMenuVisibility(true);
        mStkMenu = appService.getMenu();
        if (mStkMenu == null) {
            showTextToast(getString(R.string.main_menu_not_initialized));
            finish();
            return;
        }
        displayMenu();
        startTimeOut();
        // whenever this activity is resumed after a sub activity was invoked
        // (Browser, In call screen) switch back to main state and enable
        // user's input;
        if (!mAcceptUsersInput) {
            mState = STATE_MAIN;
            mAcceptUsersInput = true;
        }
        // make sure the progress bar is not shown.
        mProgressView.setIndeterminate(false);
        mProgressView.setVisibility(View.GONE);
    }

    @Override
    public void onPause() {
        super.onPause();

        appService.indicateMenuVisibility(false);
        cancelTimeOut();
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
            if (mStkMenu != null) {
                helpVisible = mStkMenu.helpAvailable;
            }
            if (helpVisible == true) {
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
                cancelTimeOut();
                mAcceptUsersInput = false;
                Item stkItem = getSelectedItem(info.position);
                if (stkItem == null) {
                    break;
                }
                // send help needed response.
                sendResponse(StkAppService.RES_ID_MENU_SELECTION, stkItem.id, true);
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
        boolean helpVisible = false;
        boolean mainVisible = false;

        if (mState == STATE_SECONDARY) {
            mainVisible = true;
        }
        if (mStkMenu != null) {
            helpVisible = mStkMenu.helpAvailable;
        }

        if (mainVisible) {
            menu.findItem(StkApp.MENU_ID_END_SESSION).setTitle(R.string.menu_end_session);
        }
        menu.findItem(StkApp.MENU_ID_END_SESSION).setVisible(mainVisible);

        if (helpVisible) {
            menu.findItem(StkApp.MENU_ID_HELP).setTitle(R.string.help);
        }
        menu.findItem(StkApp.MENU_ID_HELP).setVisible(helpVisible);
        // for defaut item
        if (mStkMenu != null) {
            Item item = mStkMenu.items.get(mStkMenu.defaultItem);
            if (item == null || item.text == null || item.text.length() == 0) {
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setVisible(false);
            } else {
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setTitle(item.text);
                menu.findItem(StkApp.MENU_ID_DEFAULT_ITEM).setVisible(true);
            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!mAcceptUsersInput) {
            return true;
        }
        switch (item.getItemId()) {
            case StkApp.MENU_ID_END_SESSION:
                cancelTimeOut();
                mAcceptUsersInput = false;
                // send session end response.
                sendResponse(StkAppService.RES_ID_END_SESSION);
                return true;
            case StkApp.MENU_ID_HELP:
                cancelTimeOut();
                mAcceptUsersInput = false;
                // Cannot get the current position, just consider as 0.
                int position = 0;
                Item stkItem = getSelectedItem(position);
                if (stkItem == null) {
                    break;
                }
                // send help needed response.
                sendResponse(StkAppService.RES_ID_MENU_SELECTION, stkItem.id, true);
                return true;
            case StkApp.MENU_ID_DEFAULT_ITEM:
                if (mStkMenu != null) {
                    Item defaultItem = mStkMenu.items.get(mStkMenu.defaultItem);
                    if (defaultItem == null) {
                        return true;
                    }
                    sendResponse(StkAppService.RES_ID_MENU_SELECTION, defaultItem.id,
                            false);
                    mAcceptUsersInput = false;
                    mProgressView.setVisibility(View.VISIBLE);
                    mProgressView.setIndeterminate(true);
                }
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putInt("STATE", mState);
        outState.putParcelable("MENU", mStkMenu);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        mState = savedInstanceState.getInt("STATE");
        mStkMenu = savedInstanceState.getParcelable("MENU");
    }

    private void cancelTimeOut() {
        mTimeoutHandler.removeMessages(MSG_ID_TIMEOUT);
    }

    private void startTimeOut() {
        if (mState == STATE_SECONDARY) {
            // Reset timeout.
            cancelTimeOut();
            mTimeoutHandler.sendMessageDelayed(mTimeoutHandler
                    .obtainMessage(MSG_ID_TIMEOUT), StkApp.UI_TIMEOUT);
        }
    }

    // Bind list adapter to the items list.
    private void displayMenu() {

        if (mStkMenu != null) {
            // Display title & title icon
            if (mStkMenu.titleIcon != null) {
                mTitleIconView.setImageBitmap(mStkMenu.titleIcon);
                mTitleIconView.setVisibility(View.VISIBLE);
            } else {
                mTitleIconView.setVisibility(View.GONE);
            }
            if (!mStkMenu.titleIconSelfExplanatory) {
                mTitleTextView.setVisibility(View.VISIBLE);
                if (mStkMenu.title == null) {
                    mTitleTextView.setText(R.string.app_name);
                } else {
                    mTitleTextView.setText(mStkMenu.title);
                }
            } else {
                mTitleTextView.setVisibility(View.INVISIBLE);
            }
            // create an array adapter for the menu list
            StkMenuAdapter adapter = new StkMenuAdapter(this,
                    mStkMenu.items, mStkMenu.nextActionIndicator, mStkMenu.itemsIconSelfExplanatory);
            // Bind menu list to the new adapter.
            setListAdapter(adapter);
            // Set default item
            setSelection(mStkMenu.defaultItem);
        }
    }

    private void initFromIntent(Intent intent) {

        if (intent != null) {
            mState = intent.getIntExtra("STATE", STATE_MAIN);
            if (mState == STATE_END) {
                finish();
            }
        } else {
            finish();
        }
    }

    private Item getSelectedItem(int position) {
        Item item = null;
        if (mStkMenu != null) {
            try {
                item = mStkMenu.items.get(position);
            } catch (IndexOutOfBoundsException e) {
                if (StkApp.DBG) {
                    CatLog.d(LOGTAG, "Invalid menu");
                }
            } catch (NullPointerException e) {
                if (StkApp.DBG) {
                    CatLog.d(LOGTAG, "Invalid menu");
                }
            }
        }
        return item;
    }

    private void sendResponse(int resId) {
        sendResponse(resId, 0, false);
    }

    private void sendResponse(int resId, int itemId, boolean help) {
        // if(appService.haveEndSession()) {
        // // ignore current command
        // StkLog.d(LOGTAG, "Ignore response, id is " + resId);
        // return;
        // }

        if ((STATE_SECONDARY != mState) && (StkAppService.RES_ID_END_SESSION == resId)) {
            CatLog.d(LOGTAG, "Ignore response of End Session in mState[" + mState + "]");
            return;
        }

        CatLog.d(LOGTAG, "sendResponse resID[" + resId + "] itemId[" + itemId + "] help[" + help
                + "]");
        Bundle args = new Bundle();
        args.putInt(StkAppService.OPCODE, StkAppService.OP_RESPONSE);
        args.putInt(StkAppService.RES_ID, resId);
        args.putInt(StkAppService.MENU_SELECTION, itemId);
        args.putBoolean(StkAppService.HELP, help);
        mContext.startService(new Intent(mContext, StkAppService.class)
                .putExtras(args));
    }

    private void showTextToast(String msg) {
        Toast toast = Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.AIRPLANE_MODE_ON);
        } catch (SettingNotFoundException e) {
            CatLog.d(LOGTAG, "fail to get airlane mode");
            mode = 0;
        }

        CatLog.d(LOGTAG, "airlane mode is " + mode);
        return (mode != 0);
    }
}
