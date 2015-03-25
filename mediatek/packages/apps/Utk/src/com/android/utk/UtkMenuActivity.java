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

package com.android.utk;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import android.content.BroadcastReceiver;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cdma.utk.Item;
import com.android.internal.telephony.cdma.utk.Menu;
import com.android.internal.telephony.cdma.utk.UtkLog;

import android.view.Gravity;
import android.widget.Toast;

import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;

/**
 * ListActivity used for displaying UTK menus. These can be SET UP MENU and
 * SELECT ITEM menus. This activity is started multiple times with different
 * menu content.
 *
 */
public class UtkMenuActivity extends ListActivity {
    private Context mContext;
    private Menu mUtkMenu = null;
    private int mState = STATE_MAIN;
    private boolean mAcceptUsersInput = true;

    private TextView mTitleTextView = null;
    private ImageView mTitleIconView = null;
    private ProgressBar mProgressView = null;

    UtkAppService appService = UtkAppService.getInstance();

    private final IntentFilter mSIMStateChangeFilter = new IntentFilter(
            TelephonyIntents.ACTION_SIM_STATE_CHANGED);

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(
                        com.android.internal.telephony.PhoneConstants.GEMINI_SIM_ID_KEY, -1);

                UtkLog.d(this, "mSIMStateChangeReceiver() - simId[" + simId + "]  state["
                        + simState + "]");
                if ((simId == com.android.internal.telephony.PhoneConstants.GEMINI_SIM_1) &&
                        ((IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(simState)) ||
                        (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simState)))) {
                    UtkLog.d(this, "UtkMenuActivity.finish()");
                    UtkMenuActivity.this.cancelTimeOut();
                    UtkMenuActivity.this.finish();
                }
            }
        }
    };

    // Internal state values
    static final int STATE_MAIN = 1;
    static final int STATE_SECONDARY = 2;

    // message id for time out
    private static final int MSG_ID_TIMEOUT = 1;

    Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch(msg.what) {
            case MSG_ID_TIMEOUT:
                mAcceptUsersInput = false;
                sendResponse(UtkAppService.RES_ID_TIMEOUT);
                break;
            }
        }
    };

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        UtkLog.d(this, "onCreate");
        // Remove the default title, customized one is used.
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        // Set the layout for this activity.
        setContentView(R.layout.utk_menu_list);

        mTitleTextView = (TextView) findViewById(R.id.title_text);
        mTitleIconView = (ImageView) findViewById(R.id.title_icon);
        mProgressView = (ProgressBar) findViewById(R.id.progress_bar);
        mContext = getBaseContext();

        initFromIntent(getIntent());
        mAcceptUsersInput = true;
        
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);

        UtkLog.d(this, "onNewIntent");
        initFromIntent(intent);
        mAcceptUsersInput = true;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        UtkLog.d(this, "onListItemClick");
        UtkLog.d(this, "mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);                
        if (!mAcceptUsersInput) {
            return;
        }

        Item item = getSelectedItem(position);
        UtkLog.d(this, "item :"+item + "position:" + position);                
        if (item == null) {
            return;
        }
        sendResponse(UtkAppService.RES_ID_MENU_SELECTION, item.id, false);
        mAcceptUsersInput = false;
        mProgressView.setVisibility(View.VISIBLE);
        mProgressView.setIndeterminate(true);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        UtkLog.d(this, "onKeyDown");
        UtkLog.d(this,"mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);        
        if (!mAcceptUsersInput) {
            return true;
        }
        UtkLog.d(this, "keyCode:"+ keyCode + "event:" + event);
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            switch (mState) {
            case STATE_SECONDARY:
                cancelTimeOut();
                mAcceptUsersInput = false;
                sendResponse(UtkAppService.RES_ID_BACKWARD);
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

        UtkLog.d(this, "onResume");
        if (isOnFlightMode() == true) {
            UtkLog.d(this, "Utk can't be launched in flight mode");
            showTextToast(getString(R.string.lable_on_flight_mode));

            finish();
            return;
        }
        UtkLog.d(this, "mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);
        appService.indicateMenuVisibility(true);
        mUtkMenu = appService.getMenu();
        if (mUtkMenu == null) {
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
        UtkLog.d(this, "onPause");
        appService.indicateMenuVisibility(false);
        cancelTimeOut();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mSIMStateChangeReceiver);
        UtkLog.d(this, "onDestroy");
    }

    @Override
    public boolean onCreateOptionsMenu(android.view.Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.add(0, UtkApp.MENU_ID_END_SESSION, 1, R.string.menu_end_session);
        menu.add(0, UtkApp.MENU_ID_HELP, 2, R.string.help);
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
        if (mUtkMenu != null) {
            helpVisible = mUtkMenu.helpAvailable;
        }

        menu.findItem(UtkApp.MENU_ID_END_SESSION).setVisible(mainVisible);
        menu.findItem(UtkApp.MENU_ID_HELP).setVisible(helpVisible);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!mAcceptUsersInput) {
            return true;
        }
        switch (item.getItemId()) {
        case UtkApp.MENU_ID_END_SESSION:
            cancelTimeOut();
            mAcceptUsersInput = false;
            // send session end response.
            sendResponse(UtkAppService.RES_ID_END_SESSION);
            return true;
        case UtkApp.MENU_ID_HELP:
            cancelTimeOut();
            mAcceptUsersInput = false;
            int position = getSelectedItemPosition();
            Item utkItem = getSelectedItem(position);
            if (utkItem == null) {
                break;
            }
            // send help needed response.
            sendResponse(UtkAppService.RES_ID_MENU_SELECTION, utkItem.id, true);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        UtkLog.d (this, "onSaveInstanceState");
        UtkLog.d(this, "mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);        
        outState.putInt("STATE", mState);
        outState.putParcelable("MENU", mUtkMenu);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        UtkLog.d (this, "onRestoreInstanceState");
        UtkLog.d(this, "mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);                
        mState = savedInstanceState.getInt("STATE");
        mUtkMenu = savedInstanceState.getParcelable("MENU");
    }

    private void cancelTimeOut() {
        mTimeoutHandler.removeMessages(MSG_ID_TIMEOUT);
    }

    private void startTimeOut() {
        if (mState == STATE_SECONDARY) {
            // Reset timeout.
            cancelTimeOut();
            mTimeoutHandler.sendMessageDelayed(mTimeoutHandler
                    .obtainMessage(MSG_ID_TIMEOUT), UtkApp.UI_TIMEOUT);
        }
    }

    // Bind list adapter to the items list.
    private void displayMenu() {

        if (mUtkMenu != null) {
            // Display title & title icon
            if (mUtkMenu.titleIcon != null) {
                mTitleIconView.setImageBitmap(mUtkMenu.titleIcon);
                mTitleIconView.setVisibility(View.VISIBLE);
            } else {
                mTitleIconView.setVisibility(View.GONE);
            }
            if (!mUtkMenu.titleIconSelfExplanatory) {
                mTitleTextView.setVisibility(View.VISIBLE);
                if (mUtkMenu.title == null) {
                    mTitleTextView.setText(R.string.app_name);
                } else {
                    mTitleTextView.setText(mUtkMenu.title);
                }
            } else {
                mTitleTextView.setVisibility(View.INVISIBLE);
            }
            // create an array adapter for the menu list
            UtkMenuAdapter adapter = new UtkMenuAdapter(this,
                    mUtkMenu.items, mUtkMenu.itemsIconSelfExplanatory);
            // Bind menu list to the new adapter.
            setListAdapter(adapter);
            // Set default item
            setSelection(mUtkMenu.defaultItem);
        }
    }

    private void initFromIntent(Intent intent) {
        UtkLog.d (this, "initFromIntent");
        if (intent != null) {
            mState = intent.getIntExtra("STATE", STATE_MAIN);
            UtkLog.d(this, "mState :"+mState + "mAcceptUsersInput:" + mAcceptUsersInput);
        } else {
            UtkLog.d (this, "initFromIntent null");        
            finish();
        }
    }

    private Item getSelectedItem(int position) {
        Item item = null;
        if (mUtkMenu != null) {
            try {
                item = mUtkMenu.items.get(position);
            } catch (IndexOutOfBoundsException e) {
                if (UtkApp.DBG) {
                    UtkLog.d(this, "Invalid menu");
                }
            } catch (NullPointerException e) {
                if (UtkApp.DBG) {
                    UtkLog.d(this, "Invalid menu");
                }
            }
        }
        return item;
    }

    private void sendResponse(int resId) {
        sendResponse(resId, 0, false);
    }

    private void sendResponse(int resId, int itemId, boolean help) {
        Bundle args = new Bundle();
        UtkLog.d (this, "sendResponse resId:" + resId);             
        args.putInt(UtkAppService.OPCODE, UtkAppService.OP_RESPONSE);
        args.putInt(UtkAppService.RES_ID, resId);
        args.putInt(UtkAppService.MENU_SELECTION, itemId);
        args.putBoolean(UtkAppService.HELP, help);
        mContext.startService(new Intent(mContext, UtkAppService.class)
                .putExtras(args));
    }

    boolean isOnFlightMode() {
        int mode = 0;
        try {
            mode = Settings.Global.getInt(mContext.getContentResolver(), Settings.Global.AIRPLANE_MODE_ON);
        } catch(SettingNotFoundException e) {
            UtkLog.d(this, "fail to get airlane mode");
            mode = 0;
        }
        
        UtkLog.d(this, "airlane mode is " + mode);
        return (mode != 0);
    }

    void showTextToast(String msg) {
        Toast toast = Toast.makeText(getApplicationContext(), msg, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }
}
