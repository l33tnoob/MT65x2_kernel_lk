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

import static android.provider.Telephony.Intents.ACTION_REMOVE_IDLE_TEXT;
import static android.provider.Telephony.Intents.ACTION_REMOVE_IDLE_TEXT_2;
import static android.provider.Telephony.Intents.EXTRA_PLMN;
import static android.provider.Telephony.Intents.EXTRA_SHOW_PLMN;
import static android.provider.Telephony.Intents.SPN_STRINGS_UPDATED_ACTION;

import android.app.ActivityManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.ActivityManager.RecentTaskInfo;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.RemoteViews;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.CatResponseMessage;
import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.CatCmdMessage.BrowserSettings;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;

import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

/**
 * SIM toolkit application level service. Interacts with Telephopny messages,
 * application's launch and user input from STK UI elements.
 */
public class StkAppService extends Service implements Runnable {

    // members
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private AppInterface mStkService;
    private Context mContext = null;
    private CatCmdMessage mMainCmd = null;
    private CatCmdMessage mCurrentCmd = null;
    private CatCmdMessage mCurrentMenuCmd = null;
    private Menu mCurrentMenu = null;
    private String lastSelectedItem = null;
    private boolean mMenuIsVisible = false;
    private boolean mInputIsVisible = false;
    private boolean mDialogIsVisible = false;
    private boolean responseNeeded = true;
    private boolean mCmdInProgress = false;
    private NotificationManager mNotificationManager = null;
    private LinkedList<DelayedCmd> mCmdsQ = null;
    private boolean launchBrowser = false;
    private BrowserSettings mBrowserSettings = null;
    static StkAppService sInstance = null;
    static private boolean mSetupMenuCalled = false;
    static private boolean mSetUpMenuHandled = false;
    public Phone mPhone;

    // static public final String NOTIFICATION_KEY = "notification_message";
    // static public final String NOTIFICATION_TITLE = "notification_title";
    // Used for setting FLAG_ACTIVITY_NO_USER_ACTION when
    // creating an intent.
    private enum InitiatedByUserAction {
        yes, // The action was started via a user initiated action
        unknown, // Not known for sure if user initated the action
    }

    // constants
    static final String OPCODE = "op";
    static final String CMD_MSG = "cmd message";
    static final String RES_ID = "response id";
    static final String EVDL_ID = "downLoad event id";
    static final String MENU_SELECTION = "menu selection";
    static final String INPUT = "input";
    static final String HELP = "help";
    static final String CONFIRMATION = "confirm";

    // operations ids for different service functionality.
    static final int OP_CMD = 1;
    static final int OP_RESPONSE = 2;
    static final int OP_LAUNCH_APP = 3;
    static final int OP_END_SESSION = 4;
    static final int OP_BOOT_COMPLETED = 5;
    static final int OP_EVENT_DOWNLOAD = 6;
    private static final int OP_DELAYED_MSG = 7;

    private static final int OP_RESPONSE_IDLE_TEXT = 8;
    // Response ids
    static final int RES_ID_MENU_SELECTION = 11;
    static final int RES_ID_INPUT = 12;
    static final int RES_ID_CONFIRM = 13;
    static final int RES_ID_DONE = 14;

    static final int RES_ID_TIMEOUT = 20;
    static final int RES_ID_BACKWARD = 21;
    static final int RES_ID_END_SESSION = 22;
    static final int RES_ID_EXIT = 23;

    // DownLoad event ids
    static final int EVDL_ID_USER_ACTIVITY = 0x04;
    static final int EVDL_ID_IDLE_SCREEN_AVAILABLE = 0x05;
    static final int EVDL_ID_LANGUAGE_SELECT = 0x07;
    static final int EVDL_ID_BROWSER_TERMINATION = 0x08;

    static final int DEV_ID_KEYPAD = 0x01;
    static final int DEV_ID_DISPLAY = 0x02;
    static final int DEV_ID_EARPIECE = 0x03;
    static final int DEV_ID_UICC = 0x81;
    static final int DEV_ID_TERMINAL = 0x82;
    static final int DEV_ID_NETWORK = 0x83;

    static final int SETUP_CALL_NO_CALL_1 = 0x00;
    static final int SETUP_CALL_NO_CALL_2 = 0x01;
    static final int SETUP_CALL_HOLD_CALL_1 = 0x02;
    static final int SETUP_CALL_HOLD_CALL_2 = 0x03;
    static final int SETUP_CALL_END_CALL_1 = 0x04;
    static final int SETUP_CALL_END_CALL_2 = 0x05;

    private static final String PACKAGE_NAME = "com.android.stk2";
    private static final String MENU_ACTIVITY_NAME =
            PACKAGE_NAME + ".AliasStkMenuActivity";
    private static final String INPUT_ACTIVITY_NAME =
            PACKAGE_NAME + ".StkInputActivity";

    // Notification id used to display Idle Mode text in NotificationManager.
    private static final int STK_NOTIFICATION_ID = 333;

    private static final int PHONE_STATE_CHANGED = 101;
    private static final int SUPP_SERVICE_FAILED = 102;

    private static final int miSIMid = 1; // Gemini SIM2
    // //com.android.internal.telephony.Phone.GEMINI_SIM_2

    private boolean isUserAccessed = false;

    private static final String LOGTAG = "Stk2-SAS ";

    Thread serviceThread = null;

    // Inner class used for queuing telephony messages (proactive commands,
    // session end) while the service is busy processing a previous message.
    private class DelayedCmd {
        // members
        int id;
        CatCmdMessage msg;

        DelayedCmd(int id, CatCmdMessage msg) {
            this.id = id;
            this.msg = msg;
        }
    }

    static boolean isSetupMenuCalled() {
        return mSetupMenuCalled;
    }

    @Override
    public void onCreate() {

        CatLog.d(this, " StkAppService Oncreate");
        CatLog.d(LOGTAG, " onCreate()+");
        // Initialize members
        mStkService = com.android.internal.telephony.cat.CatService
                .getInstance(com.android.internal.telephony.cat.CatService.GEMINI_SIM_2);

        // NOTE mStkService is a singleton and continues to exist even if the
        // GSMPhone is disposed
        // after the radio technology change from GSM to CDMA so the
        // PHONE_TYPE_CDMA check is
        // needed. In case of switching back from CDMA to GSM the GSMPhone
        // constructor updates
        // the instance. (TODO: test).
        if ((mStkService == null)
                && (TelephonyManager.getDefault().getPhoneType()
                != TelephonyManager.PHONE_TYPE_CDMA)) {
            CatLog.d(LOGTAG, " Unable to get Service handle");
            return;
        }

        mCmdsQ = new LinkedList<DelayedCmd>();
        serviceThread = new Thread(null, this, "Stk2 App Service");
        serviceThread.start();
        mContext = getBaseContext();
        mNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        sInstance = this;

        mPhone = PhoneFactory.getDefaultPhone();
        ((GeminiPhone) mPhone).registerForPreciseCallStateChangedGemini(mCallHandler,
                PHONE_STATE_CHANGED, null, miSIMid);
        ((GeminiPhone) mPhone).registerForSuppServiceFailedGemini(mCallHandler,
                SUPP_SERVICE_FAILED, null, miSIMid);
        initNotify();

        IntentFilter mSIMStateChangeFilter = new IntentFilter(
                TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mSIMStateChangeFilter.addAction("android.intent.action.ACTION_SHUTDOWN_IPO");
        if (miSIMid == Phone.GEMINI_SIM_1) {
            mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT);
        } else {
            mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT_2);
        }
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
        CatLog.d(LOGTAG, " onCreate()-");
    }

    /**
     * @param intent The intent with action
     *            {@link Telephony.Intents#SPN_STRINGS_UPDATED_ACTION}
     * @return The string to use for the plmn, or null if it should not be
     *         shown.
     */
    private String getTelephonyPlmnFrom(Intent intent) {
        if (intent.getBooleanExtra(EXTRA_SHOW_PLMN, false)) {
            final String plmn = intent.getStringExtra(EXTRA_PLMN);
            if (plmn != null) {
                return plmn;
            }
        }
        return getDefaultPlmn();
    }

    /**
     * @return The default plmn (no service)
     */
    private String getDefaultPlmn() {
        return getResources().getString(
                com.android.internal.R.string.lockscreen_carrier_default);
    }

    public void initNotify() {
        StkApp.mPLMN = getDefaultPlmn();
        final IntentFilter filter = new IntentFilter();
        filter.addAction(SPN_STRINGS_UPDATED_ACTION);
        registerReceiver(new BroadcastReceiver() {
            public void onReceive(Context context, Intent intent) {
                final String action = intent.getAction();
                if (SPN_STRINGS_UPDATED_ACTION.equals(action)) {
                    StkApp.mPLMN = getTelephonyPlmnFrom(intent);
                }
            }
        }, filter);
        return;
    }

    @Override
    public void onStart(Intent intent, int startId) {
        if (mStkService == null) {
            CatLog.d(this, " StkAppService onStart mStkService is null  return");
            return;
        }
        waitForLooper();

        // onStart() method can be passed a null intent
        // TODO: replace onStart() with onStartCommand()
        if (intent == null) {
            return;
        }

        Bundle args = intent.getExtras();

        if (args == null) {
            return;
        }

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = args.getInt(OPCODE);
        switch (msg.arg1) {
            case OP_CMD:
                msg.obj = args.getParcelable(CMD_MSG);
                break;
            case OP_EVENT_DOWNLOAD:
                msg.obj = args;
                break;
            case OP_RESPONSE:
                msg.obj = args;
                /* falls through */
            case OP_LAUNCH_APP:
            case OP_END_SESSION:
            case OP_BOOT_COMPLETED:
                break;
            default:
                return;
        }
        mServiceHandler.sendMessage(msg);
    }

    @Override
    public void onDestroy() {
        CatLog.d(LOGTAG, " onDestroy()");
        unregisterReceiver(mSIMStateChangeReceiver);
        waitForLooper();
        mServiceLooper.quit();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void run() {
        Looper.prepare();

        mServiceLooper = Looper.myLooper();
        mServiceHandler = new ServiceHandler();

        Looper.loop();
    }

    /*
     * Package api used by StkMenuActivity to indicate if its on the foreground.
     */
    void indicateMenuVisibility(boolean visibility) {
        mMenuIsVisible = visibility;
    }

    /*
     * Package api used by StkInputActivity to indicate if its on the
     * foreground.
     */
    void indicateInputVisibility(boolean visibility) {
        mInputIsVisible = visibility;
    }

    /*
     * Package api used by StkDialogActivity to indicate if its on the
     * foreground.
     */
    void indicateDialogVisibility(boolean visibility) {
        mDialogIsVisible = visibility;
    }

    /*
     * Package api used by StkMenuActivity to get its Menu parameter.
     */
    Menu getMenu() {
        return mCurrentMenu;
    }

    boolean isCurCmdSetupCall() {
        if (mCurrentCmd == null) {
            CatLog.d(LOGTAG, "[isCurCmdSetupCall][mCurrentCmd]:null");
            return false;
        } else if (mCurrentCmd.getCmdType() == null) {
            CatLog.d(LOGTAG, "[isCurCmdSetupCall][mCurrentCmd.getCmdType()]:null");
            return false;

        } else {
            CatLog.d(LOGTAG, "SET UP CALL Cmd Check[" + mCurrentCmd.getCmdType().value() + "]");
            return (AppInterface.CommandType.SET_UP_CALL.value() == mCurrentCmd.getCmdType()
                    .value());

        }

    }

    /*
     * Package api used by UI Activities and Dialogs to communicate directly
     * with the service to deliver state information and parameters.
     */
    static StkAppService getInstance() {
        return sInstance;
    }

    private void waitForLooper() {
        while (mServiceHandler == null) {
            if (serviceThread == null || serviceThread.isAlive() == false) {
                CatLog.d(LOGTAG, "do re-init");
                init();
            }
            synchronized (this) {
                try {
                    wait(100);
                } catch (InterruptedException e) {
                }
            }
        }
    }

    private final class ServiceHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                CatLog.d(LOGTAG, "ServiceHandler handleMessage msg is null");
                return;
            }
            int opcode = msg.arg1;

            CatLog.d(LOGTAG, "handleMessage opcode[" + opcode + "]");
            if (opcode == OP_CMD && msg.obj != null
                    && ((CatCmdMessage) msg.obj).getCmdType() != null) {
                CatLog.d(LOGTAG, "handleMessage cmdName["
                        + ((CatCmdMessage) msg.obj).getCmdType().name() + "]");
            }

            switch (opcode) {
                case OP_LAUNCH_APP:
                    if (mMainCmd == null) {
                        // nothing todo when no SET UP MENU command didn't
                        // arrive.
                        return;
                    }
                    if (isBusyOnCall() == true) {
                        Toast toast = Toast.makeText(mContext.getApplicationContext(),
                                R.string.lable_busy_on_call, Toast.LENGTH_LONG);
                        toast.setGravity(Gravity.BOTTOM, 0, 0);
                        toast.show();
                        return;
                    }
                    CatLog.d(LOGTAG, "handleMessage OP_LAUNCH_APP - mCmdInProgress["
                            + mCmdInProgress + "]");

                    if (mCurrentMenu == mMainCmd.getMenu() || mCurrentMenu == null) {
                        launchMenuActivity(null);
                    } else {
                        launchMenuActivity(mCurrentMenu);
                    }

                    setUserAccessState(true);
                    break;
                case OP_CMD:
                    CatLog.d(LOGTAG, "[OP_CMD]");
                    // PackageManager pm = mContext.getPackageManager();
                    // // check that STK app package is known to the
                    // PackageManager
                    // ComponentName cName = new
                    // ComponentName("com.android.stk",
                    // "com.android.stk.StkLauncherActivity");
                    // boolean bpkgNotInstalled =
                    // pm.getComponentEnabledSetting(cName) ==
                    // PackageManager.COMPONENT_ENABLED_STATE_DISABLED;
                    // //Log.d(LOGTAG, "findbug Installed:" + bpkgNotInstalled +
                    // " cmdInter:" + isCmdInteractive((StkCmdMessage) msg.obj)
                    // + " process: " + mCmdInProgress);
                    CatCmdMessage cmdMsg = (CatCmdMessage) msg.obj;
                    // if(bpkgNotInstalled){
                    // CatLog.d(LOGTAG, "[prepare to start STKAPPSERVICE]");
                    // switch (cmdMsg.getCmdType()){
                    // case SET_UP_MENU:
                    // CatLog.d(LOGTAG, "[OP_CMD][SETUPMENU delayed]");
                    // mFirstCmdSetupMenu = true;
                    // if(cmdMsg.getMenu() != null){
                    // mstrbackDisplayTitle = cmdMsg.getMenu().title;
                    // CatLog.d(LOGTAG,
                    // "[OP_CMD][SETUPMENU delayed][Menu Title] : " +
                    // mstrbackDisplayTitle);
                    // }
                    // break;
                    // }
                    // StkAppInstaller.install(mContext);
                    // Message msgNew = mServiceHandler.obtainMessage();
                    // msgNew.obj = msg.obj;
                    // msgNew.arg1 = OP_CMD;
                    // mServiceHandler.sendMessageDelayed(msgNew, 50);
                    // }else{
                    // CatLog.d(LOGTAG, "[OP_CMD][Normal]");
                    // // There are two types of commands:
                    // 1. Interactive - user's response is required.
                    // 2. Informative - display a message, no interaction with
                    // the user.
                    //
                    // Informative commands can be handled immediately without
                    // any delay.
                    // Interactive commands can't override each other. So if a
                    // command
                    // is already in progress, we need to queue the next command
                    // until
                    // the user has responded or a timeout expired.
                    if (!isCmdInteractive(cmdMsg)) {
                        CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Not Interactive]");
                        handleCmd(cmdMsg);
                    } else {
                        CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive]");
                        if (!mCmdInProgress) {
                            CatLog
                                    .d(LOGTAG,
                                            "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive][not in progress]");
                            mCmdInProgress = true;
                            handleCmd((CatCmdMessage) msg.obj);
                        } else {
                            CatLog.d(LOGTAG,
                                    "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive][in progress]");
                            mCmdsQ.addLast(new DelayedCmd(OP_CMD,
                                    (CatCmdMessage) msg.obj));
                        }
                    }

                    // }
                    break;
                case OP_RESPONSE:
                    if (responseNeeded) {
                        handleCmdResponse((Bundle) msg.obj);
                    }
                    // call delayed commands if needed.
                    if (mCmdsQ.size() != 0) {
                        callDelayedMsg();
                    } else {
                        mCmdInProgress = false;
                    }
                    // reset response needed state var to its original value.
                    responseNeeded = true;
                    break;
                case OP_END_SESSION:
                    if (!mCmdInProgress) {
                        mCmdInProgress = true;
                        handleSessionEnd();
                    } else {
                        mCmdsQ.addLast(new DelayedCmd(OP_END_SESSION, null));
                    }
                    break;
                case OP_BOOT_COMPLETED:
                    CatLog.d(LOGTAG, " OP_BOOT_COMPLETED");
                    // if (mMainCmd == null) {
                    // CatLog.d(LOGTAG, "OP_BOOT_COMPLETED - unInstall");
                    // StkAppInstaller.unInstall(mContext);
                    // }
                    break;
                case OP_EVENT_DOWNLOAD:
                    CatLog.d(LOGTAG, "OP_EVENT_DOWNLOAD");
                    handleEventDownload((Bundle) msg.obj);
                    break;
                case OP_DELAYED_MSG:
                    handleDelayedCmd();
                    break;
                case OP_RESPONSE_IDLE_TEXT:
                    handleIdleTextResponse();
                    // End the process.
                    mCmdInProgress = false;
                    break;
            }
        }
    }

    private boolean isCmdInteractive(CatCmdMessage cmd) {
        switch (cmd.getCmdType()) {
            case SEND_DTMF:
            case SEND_SMS:
            case SEND_SS:
            case SEND_USSD:
            case SET_UP_IDLE_MODE_TEXT:
            case SET_UP_MENU:
                return false;
        }

        return true;
    }

    private void handleDelayedCmd() {
        if (mCmdsQ.size() != 0) {
            DelayedCmd cmd = mCmdsQ.poll();
            if (cmd != null) {
                switch (cmd.id) {
                    case OP_CMD:
                        handleCmd(cmd.msg);
                        break;
                    case OP_END_SESSION:
                        handleSessionEnd();
                        break;
                }
            } else {
                CatLog.d(LOGTAG, "handleDelayedCmd cmd is null");
            }

        }
    }

    private void callDelayedMsg() {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_DELAYED_MSG;
        mServiceHandler.sendMessage(msg);
    }

    private void handleSessionEnd() {
        mCurrentCmd = mMainCmd;
        CatLog.d(LOGTAG, "handleSessionEnd - mCurrentCmd changed to mMainCmd!");
        mCurrentMenuCmd = mMainCmd;
        lastSelectedItem = null;
        // In case of SET UP MENU command which removed the app, don't
        // update the current menu member.
        if (mCurrentMenu != null && mMainCmd != null) {
            mCurrentMenu = mMainCmd.getMenu();
        }
        if (mMenuIsVisible) {
            if (mSetupMenuCalled == true) {
                launchMenuActivity(null);
            } else {
                // Only called when pass FTA test (154.1.1)
                finishMenuActivity();
            }
        }
        if (mCmdsQ.size() != 0) {
            callDelayedMsg();
        } else {
            mCmdInProgress = false;
        }
        // In case a launch browser command was just confirmed, launch that url.
        if (launchBrowser) {
            launchBrowser = false;
            launchBrowser(mBrowserSettings);
        }
    }

    private void handleCmd(CatCmdMessage cmdMsg) {
        StkAppInstaller appInstaller = StkAppInstaller.getInstance();
        if (cmdMsg == null) {
            return;
        }
        // save local reference for state tracking.
        mCurrentCmd = cmdMsg;
        boolean waitForUsersResponse = true;
        byte[] additionalInfo = null;

        if (cmdMsg.getCmdType() != null) {
            CatLog.d(LOGTAG, "handleCmd cmdName[" + cmdMsg.getCmdType().name()
                    + "]  mCurrentCmd=cmdMsg");
        }

        switch (cmdMsg.getCmdType()) {
            case DISPLAY_TEXT:
                if (isBusyOnCall() == true) {
                    CatLog.d(LOGTAG, "[Handle Command][DISPLAY_TEXT][Can not handle currently]");
                    CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                    resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                    additionalInfo = new byte[1];
                    additionalInfo[0] = (byte) 0x02;
                    resMsg.setAdditionalInfo(additionalInfo);
                    mStkService.onCmdResponse(resMsg);
                    return;
                }

                TextMessage msg = cmdMsg.geTextMessage();
                responseNeeded = msg.responseNeeded;
                if (responseNeeded == false) {
                    waitForUsersResponse = false;
                    CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                    resMsg.setResultCode(ResultCode.OK);
                    mStkService.onCmdResponse(resMsg);
                }
                if (lastSelectedItem != null) {
                    msg.title = lastSelectedItem;
                } else if (mMainCmd != null) {
                    msg.title = mMainCmd.getMenu().title;
                } else {
                    // TODO: get the carrier name from the SIM
                    msg.title = "";
                }

                byte[] target = {
                        0x0d, 0x0a
                };
                String strTarget = new String(target);
                String strLine = System.getProperty("line.separator");

                String strText = msg.text.replaceAll(strTarget, strLine);
                msg.text = strText;
                launchTextDialog();
                break;
            case SELECT_ITEM:
                mCurrentMenuCmd = mCurrentCmd;
                mCurrentMenu = cmdMsg.getMenu();

                if (isBusyOnCall() == true) {
                    CatLog.d(LOGTAG, "[Handle Command][SELECT_ITEM][Can not handle currently]");
                    CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                    resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                    additionalInfo = new byte[1];
                    additionalInfo[0] = (byte) 0x02;
                    resMsg.setAdditionalInfo(additionalInfo);
                    mStkService.onCmdResponse(resMsg);
                    return;
                }

                final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager
                        .getService(Context.TELEPHONY_SERVICE));
                if (iTel != null) {
                    try {
                        if (iTel.isRadioOnGemini(Phone.GEMINI_SIM_2) == true) {
                            if (mMenuIsVisible == true || isUserAccessed == false) {
                                launchMenuActivity(cmdMsg.getMenu());
                            } else {
                                CatLog.d(LOGTAG, "can not show select_item now");
                                CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                                resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                                mStkService.onCmdResponse(resMsg);
                                changeMenuStateToMain();
                            }
                        }
                    } catch (RemoteException ex) {
                        ex.getMessage();
                    }
                }
                break;
            case SET_UP_MENU:
                mSetupMenuCalled = true;
                mMainCmd = mCurrentCmd;
                mCurrentMenuCmd = mCurrentCmd;
                mCurrentMenu = cmdMsg.getMenu();
                CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU [" + removeMenu() + "]");

                boolean radio_on = true;
                if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
                    try {
                        ITelephony phone = ITelephony.Stub.asInterface(ServiceManager
                                .checkService("phone"));
                        if (phone != null) {
                            radio_on = phone.isRadioOnGemini(Phone.GEMINI_SIM_2);
                            CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU radio_on[" + radio_on
                                    + "]");
                        }
                    } catch (RemoteException e) {
                        e.printStackTrace();
                        CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU Exception happen ====");
                    }
                }

                if (removeMenu()) {
                    CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - removeMenu() - Uninstall App");
                    mCurrentMenu = null;
                    appInstaller.unInstall(mContext);
                } else if (!radio_on) {
                    CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - install App - radio_on["
                            + radio_on + "]");
                    appInstaller.unInstall(mContext);
                } else {
                    CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - install App");
                    appInstaller.install(mContext);
                }

                if (mMenuIsVisible) {
                    launchMenuActivity(null);
                }
                break;
            case GET_INPUT:
            case GET_INKEY:
                if (isBusyOnCall() == true) {
                    CatLog.d(LOGTAG, "[Handle Command][GET_INPUT][Can not handle currently]");
                    CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                    resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                    additionalInfo = new byte[1];
                    additionalInfo[0] = (byte) 0x02;
                    resMsg.setAdditionalInfo(additionalInfo);
                    mStkService.onCmdResponse(resMsg);
                    return;
                }
                launchInputActivity();
                break;
            case SET_UP_IDLE_MODE_TEXT:
                waitForUsersResponse = false;
                launchIdleText();
                break;
            case SEND_DTMF:
            case SEND_SMS:
            case SEND_SS:
            case SEND_USSD:
                waitForUsersResponse = false;
                launchEventMessage();
                break;
            case LAUNCH_BROWSER:
                CatLog.d(LOGTAG, "[Handle Command][LAUNCH_BROWSER]");
                mBrowserSettings = mCurrentCmd.getBrowserSettings();
                if ((mBrowserSettings != null)
                        && (isBrowserLaunched(getApplicationContext()) == true)) {
                    switch (mBrowserSettings.mode) {
                        case LAUNCH_IF_NOT_ALREADY_LAUNCHED:
                            CatLog.d(LOGTAG,
                                    "[Handle Command][LAUNCH_BROWSER][Should not launch browser]");
                            CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
                            launchBrowser = false;
                            resMsg.setResultCode(ResultCode.LAUNCH_BROWSER_ERROR);
                            mStkService.onCmdResponse(resMsg);
                            break;
                        default:
                            launchConfirmationDialog(mCurrentCmd.geTextMessage());
                            break;
                    }

                } else {
                    launchConfirmationDialog(mCurrentCmd.geTextMessage());
                }
                break;
            case SET_UP_CALL:
                processSetupCall();
                break;
            case PLAY_TONE:
                launchToneDialog();
                break;

            // TODO: 6573 supported
            case RUN_AT_COMMAND:
                break;

            case OPEN_CHANNEL:
                processOpenChannel();
                break;

            case CLOSE_CHANNEL:
            case RECEIVE_DATA:
            case SEND_DATA:
            case GET_CHANNEL_STATUS:
                waitForUsersResponse = false;
                launchEventMessage();
                break;
        }

        if (!waitForUsersResponse) {
            if (mCmdsQ.size() != 0) {
                callDelayedMsg();
            } else {
                mCmdInProgress = false;
            }
        }
    }

    private void displayAlphaIcon(TextMessage msg) {

        CatLog.d(LOGTAG, "launchAlphaIcon - IconSelfExplanatory[" + msg.iconSelfExplanatory + "]"
                + "icon[" + msg.icon + "]"
                + "text[" + msg.text + "]");

        if (msg.iconSelfExplanatory == true) {
            // only display Icon.
            if (msg.icon != null) {
                showIconToast(msg);
            } else {
                // do nothing.
                CatLog.d(LOGTAG, "launchAlphaIcon - null icon!");
                return;
            }
        } else {
            // show text & icon.
            if (msg.icon != null) {
                if (msg.text == null || msg.text.length() == 0) {
                    // show Icon only.
                    showIconToast(msg);
                } else {
                    showIconAndTextToast(msg);
                }
            } else {
                if (msg.text == null || msg.text.length() == 0) {
                    // do nothing
                    CatLog.d(LOGTAG, "launchAlphaIcon - null txt!");
                    return;
                } else {
                    showTextToast(msg);
                }
            }
        }
    }

    private void processOpenChannel() {
        CatLog.d(LOGTAG, "processOpenChannel()+");

        Call.State callState = getCallState();
        TextMessage txtMsg = mCurrentCmd.geTextMessage();

        switch (callState) {
            case IDLE:
            case DISCONNECTED:
                if ((null != txtMsg.text) && (0 != txtMsg.text.length())) {
                    /* Alpha identifier with data object */
                    launchConfirmationDialog(txtMsg);
                } else {
                    /*
                     * Alpha identifier with null data object Chap 6.4.27.1 ME
                     * should not give any information to the user or ask for
                     * user confirmation
                     */
                    processNormalOpenChannelResponse();
                }
                break;

            default:
                CatLog.d(LOGTAG, "processOpenChannel() Abnormal OpenChannel Response");
                processAbnormalOpenChannelResponse();
                break;
        }

        CatLog.d(LOGTAG, "processOpenChannel()-");
    }

    private void processOpenChannelResponse() {
        CatLog.d(LOGTAG, "processOpenChannelResponse()+");
        int iChannelType = 0;
        if (mCurrentCmd.mBearerDesc == null) {
            iChannelType = 2;
        } else {
            iChannelType = mCurrentCmd.mBearerDesc.bearerType;
        }
        switch (iChannelType) {
            case 1: /* Open Channel related to CS Bearer */
                processNormalOpenChannelResponse();
                break;

            case 2: /* Open Channel related to packet data service Bearer */
                processNormalOpenChannelResponse();
                break;

            case 3: /* Open Channel related to local Bearer */
                processNormalOpenChannelResponse();
                break;

            case 4: /* Open Channel related to default(Network) Bearer */
                processNormalOpenChannelResponse();
                break;

            case 5: /* Open Channel related to UICC Server Mode */
                processNormalOpenChannelResponse();
                break;

            default: /* Error! */
                CatLog.d(LOGTAG, "processOpenChannelResponse() Error channel type[" + iChannelType
                        + "]");
                processAbnormalOpenChannelResponse(); // TODO: To check
                break;
        }
        CatLog.d(LOGTAG, "processOpenChannelResponse()-");

    }

    private void processNormalResponse() {
        CatLog.d(LOGTAG, "Normal Response PROCESS Start");
        mCmdInProgress = false;
        if (mSetupCallInProcess == false) {
            return;
        }
        mSetupCallInProcess = false;
        if (mCurrentCmd == null) {
            CatLog.d(LOGTAG, "Normal Response PROCESS mCurrentCmd changed to null!");
            return;
        }

        if (mCurrentCmd.getCmdType() != null) {
            CatLog.d(LOGTAG, "Normal Response PROCESS end! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");
        }
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(true);
        launchCallMsg();
        mStkService.onCmdResponse(resMsg);
    }

    private void processAbnormalResponse() {
        mCmdInProgress = false;
        CatLog.d(LOGTAG, "Abnormal Response PROCESS Start");
        if (mSetupCallInProcess == false) {
            return;
        }
        mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal Response PROCESS");
        if (mCurrentCmd == null) {
            return;
        }
        if (mCurrentCmd.getCmdType() != null)
            CatLog.d(LOGTAG, "Abnormal Response PROCESS end! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.NETWORK_CRNTLY_UNABLE_TO_PROCESS);
        mStkService.onCmdResponse(resMsg);
    }

    private void processAbnormalPhone1BusyResponse() {
        mCmdInProgress = false;
        mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS - SIM 1 Call Busy");
        if (mCurrentCmd == null) {
            return;
        }
        if (mCurrentCmd.getCmdType() != null)
            CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS end - SIM 1 Call Busy! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");

        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(false);
        mStkService.onCmdResponse(resMsg);
    }

    private void processAbnormalNoCallResponse() {
        mCmdInProgress = false;
        if (mSetupCallInProcess == false) {
            return;
        }
        mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS");
        if (mCurrentCmd == null) {
            return;
        }
        if (mCurrentCmd.getCmdType() != null)
            CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS end! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        mStkService.onCmdResponse(resMsg);
    }

    private void processNormalOpenChannelResponse() {
        CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS Start");

        mCmdInProgress = false;
        if (mCurrentCmd == null) {
            CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS mCurrentCmd changed to null!");
            return;
        }

        TextMessage txtMsg = mCurrentCmd.geTextMessage();
        if (mCurrentCmd.getCmdType() != null) {
            CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS end! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");
        }
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(true);
        displayAlphaIcon(txtMsg);
        mStkService.onCmdResponse(resMsg);
    }

    private void processAbnormalOpenChannelResponse() {
        mCmdInProgress = false;
        CatLog.d(LOGTAG, "Abnormal OpenChannel Response PROCESS");
        if (mCurrentCmd == null) {
            return;
        }
        if (mCurrentCmd.getCmdType() != null) {
            CatLog.d(LOGTAG, "Abnormal OpenChannel Response PROCESS end! cmdName["
                    + mCurrentCmd.getCmdType().name() + "]");
        }
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        mStkService.onCmdResponse(resMsg);
    }

    private void processNormalEndCallResponse() {
        CatLog.d(LOGTAG, "END CALL PROCESS");
        processNormalResponse();
    }

    private void processNormalHoldCallResponse() {
        CatLog.d(LOGTAG, "HOLD CALL PROCESS");
        processNormalResponse();
    }

    private void processAbnormalEndCallResponse() {
        CatLog.d(LOGTAG, "End Abnormal CALL PROCESS");
        processAbnormalResponse();
    }

    private void processAbnormalHoldCallResponse() {
        CatLog.d(LOGTAG, "HOLD Abnormal CALL PROCESS");
        processAbnormalResponse();
    }

    private void processPhoneStateChanged() {
        CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: ");
        if (mSetupCallInProcess == false) {
            CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: setup in process is false");
            return;
        }
        CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: setup in process is true");
        // Setup call In Process.
        if (mCurrentCmd != null) {
            // Set up call
            switch (mCurrentCmd.getCmdType()) {
                case SET_UP_CALL:
                    int cmdQualifier = mCurrentCmd.getCmdQualifier();
                    // Call fg = mPhone.getForegroundCall();
                    Call fg = ((GeminiPhone) mPhone).getForegroundCallGemini(miSIMid);
                    if (fg != null) {
                        Call.State state = fg.getState();
                        CatLog.d(LOGTAG, " PHONE_STATE_CHANGED to : " + state);
                        switch (state) {
                            case HOLDING:
                                if (cmdQualifier == SETUP_CALL_HOLD_CALL_1 ||
                                        cmdQualifier == SETUP_CALL_HOLD_CALL_2) {
                                    processNormalHoldCallResponse();
                                }
                                break;
                            case IDLE:
                                if (cmdQualifier == SETUP_CALL_HOLD_CALL_1 ||
                                        cmdQualifier == SETUP_CALL_HOLD_CALL_2) {
                                    // need process "end call" when hold
                                    processNormalHoldCallResponse();
                                } else if (cmdQualifier == SETUP_CALL_END_CALL_1 ||
                                        cmdQualifier == SETUP_CALL_END_CALL_2) {
                                    processNormalEndCallResponse();
                                }
                        }
                    }
                    break;
            }
        }
        return;
    }

    private void processSuppServiceFailed(AsyncResult r) {
        Phone.SuppService service = (Phone.SuppService) r.result;
        CatLog.d(LOGTAG, "onSuppServiceFailed: " + service);

        int errorMessageResId;
        switch (service) {
            case SWITCH:
                // Attempt to switch foreground and background/incoming calls
                // failed
                // ("Failed to switch calls")
                CatLog.d(LOGTAG, "Switch failed");
                processAbnormalHoldCallResponse();
                break;
        }
    }

    private Handler mCallHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case PHONE_STATE_CHANGED:
                                processPhoneStateChanged();
                    break;
                case SUPP_SERVICE_FAILED:
                                processSuppServiceFailed((AsyncResult) msg.obj);
                    break;
            }
        }
    };

    private Call.State getCallState() {
        // Call fg = mPhone.getForegroundCall();
        Call fg = ((GeminiPhone) mPhone).getForegroundCallGemini(miSIMid);
        if (fg != null) {
            CatLog.d(LOGTAG, "ForegroundCall State: " + fg.getState());
            return fg.getState();
        }
        return Call.State.IDLE;
    }

    private Call.State getBackgroundCallState() {
        // Call bg = mPhone.getBackgroundCall();
        Call bg = ((GeminiPhone) mPhone).getBackgroundCallGemini(miSIMid);
        if (bg != null) {
            CatLog.d(LOGTAG, "BackgroundCall State: " + bg.getState());
            return bg.getState();
        }
        return Call.State.IDLE;
    }

    private boolean is1A1H() {
        Call.State fgState = getCallState();
        Call.State bgState = getBackgroundCallState();
        if (fgState != Call.State.IDLE && bgState != Call.State.IDLE) {
            CatLog.d(LOGTAG, "1A1H");
            return true;
        }
        return false;
    }

    private Call.State isPhoneIdle(int SIMid) {
        Call fg = ((GeminiPhone) mPhone).getForegroundCallGemini(SIMid);
        if (fg != null) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " ForegroundCall State: "
                    + fg.getState());
            if ((Call.State.IDLE != fg.getState()) && (Call.State.DISCONNECTED != fg.getState())) {
                return fg.getState();
            }
        }

        Call bg = ((GeminiPhone) mPhone).getBackgroundCallGemini(SIMid);
        if (bg != null) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " BackgroundCall State: "
                    + bg.getState());
            if (Call.State.IDLE != bg.getState() && (Call.State.DISCONNECTED != bg.getState())) {
                return bg.getState();
            }
        }

        Call ring = ((GeminiPhone) mPhone).getRingingCallGemini(SIMid);
        if (bg != null) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " RingCall State: " + ring.getState());
            if (Call.State.IDLE != ring.getState() && (Call.State.DISCONNECTED != ring.getState())) {
                return ring.getState();
            }
        }

        CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " State: " + Call.State.IDLE);
        return Call.State.IDLE;
    }

    private void processNoCall() {
        // get Call State.
        Call.State callState = getCallState();
        switch (callState) {
            case IDLE:
            case DISCONNECTED:
                launchConfirmationDialog(mCurrentCmd.getCallSettings().confirmMsg);
                break;
            default:
                CatLog.d(LOGTAG, "Call Abnormal No Call Response");
                processAbnormalNoCallResponse();
                break;
        }
    }

    private void processHoldCall() {
        // Just show the confirm dialog, and add the process when user click OK.
        if (!is1A1H()) {
            launchConfirmationDialog(mCurrentCmd.getCallSettings().confirmMsg);
        } else {
            CatLog.d(LOGTAG, "Call Abnormal Hold Call Response(has 1A1H calls)");
            processAbnormalNoCallResponse();
        }
    }

    private void processEndCall() {
        // Just show the confirm dialog, and add the process when user click OK.
        launchConfirmationDialog(mCurrentCmd.getCallSettings().confirmMsg);
    }

    private void processSetupCall() {
        if (Call.State.IDLE != isPhoneIdle(0)) {
            processAbnormalPhone1BusyResponse();
        } else {
            // get callback.
            mSetupCallInProcess = true;
            int cmdQualifier = mCurrentCmd.getCmdQualifier();
            CatLog.d(LOGTAG, "Qualifier code is " + cmdQualifier);
            switch (cmdQualifier) {
                case SETUP_CALL_NO_CALL_1:
                case SETUP_CALL_NO_CALL_2:
                    processNoCall();
                    break;
                case SETUP_CALL_HOLD_CALL_1:
                case SETUP_CALL_HOLD_CALL_2:
                    processHoldCall();
                    break;
                case SETUP_CALL_END_CALL_1:
                case SETUP_CALL_END_CALL_2:
                    processEndCall();
                    break;
            }
        }
    }

    private void processHoldCallResponse() {
        // get Call State.
        Call.State callState = getCallState();
        CatLog.d(LOGTAG, "processHoldCallResponse callState[" + callState + "]");

        switch (callState) {
            case IDLE:
            case HOLDING:
                processNormalResponse();
                CatLog.d(LOGTAG, "processHoldCallResponse in Idle or HOLDING");
                break;
            case ACTIVE:
                CatLog.d(LOGTAG, "processHoldCallResponse in Active ");
                try {
                    CatLog.d(LOGTAG, "switchHoldingAndActive");
                    // mPhone.switchHoldingAndActive();
                    ((GeminiPhone) mPhone).switchHoldingAndActiveGemini(miSIMid);
                } catch (CallStateException ex) {
                    CatLog.d(LOGTAG, "Error: switchHoldingAndActive: caught " + ex);
                    processAbnormalResponse();
                }
                break;
            default:
                CatLog.d(LOGTAG, "processHoldCallResponse in other state");
                processAbnormalResponse();
                break;
        }
        return;
    }

    private boolean mSetupCallInProcess = false; // true means in process.

    private void processEndCallResponse() {
        // get Call State.
        Call.State callState = getCallState();
        CatLog.d(LOGTAG, "call State  = " + callState);
        switch (callState) {
            case IDLE:
                processNormalResponse();
                break;
            // other state
            default:
                // End call
                CatLog.d(LOGTAG, "End call");
                // 1A1H call
                if (is1A1H()) {
                    try {
                        // mPhone.hangupAll();
                        ((GeminiPhone) mPhone).hangupAllGemini(miSIMid);
                    } catch (Exception ex) {
                        CatLog.d(LOGTAG, "Error: Call hangup: caught " + ex);
                        processAbnormalResponse();
                    }
                } else {
                    // Call fg = mPhone.getForegroundCall();
                    Call fg = ((GeminiPhone) mPhone).getForegroundCallGemini(miSIMid);
                    if (fg != null) {
                        try {
                            CatLog.d(LOGTAG, "End call  " + callState);
                            fg.hangup();
                        } catch (CallStateException ex) {
                            CatLog.d(LOGTAG, "Error: Call hangup: caught " + ex);
                            // TODO
                            processAbnormalResponse();
                        }
                    }
                }
                CatLog.d(LOGTAG, "call Not IDLE  = " + callState);
                break;
        }
    }

    private void processSetupCallResponse() {
        int cmdQualifier = mCurrentCmd.getCmdQualifier();
        CatLog.d(LOGTAG, "processSetupCallResponse() - cmdQualifier[" + cmdQualifier + "]");

        switch (cmdQualifier) {
            case SETUP_CALL_NO_CALL_1:
            case SETUP_CALL_NO_CALL_2:
                // TODO
                processNormalResponse();
                break;
            case SETUP_CALL_HOLD_CALL_1:
            case SETUP_CALL_HOLD_CALL_2:
                processHoldCallResponse();
                break;
            case SETUP_CALL_END_CALL_1:
            case SETUP_CALL_END_CALL_2:
                processEndCallResponse();
                break;
        }
    }

    // End Setup Call

    private void handleEventDownload(Bundle args) {
        int eventId = args.getInt(EVDL_ID);
        int sourceId = 0;
        int destinationId = 0;
        byte[] additionalInfo = null;
        byte[] language;
        boolean oneShot = false;
        String languageInfo;

        CatResponseMessage resMsg = new CatResponseMessage(eventId);
        switch (eventId) {
            case EVDL_ID_USER_ACTIVITY:
                sourceId = DEV_ID_TERMINAL;
                destinationId = DEV_ID_UICC;
                oneShot = true;
                break;
            case EVDL_ID_IDLE_SCREEN_AVAILABLE:
                sourceId = DEV_ID_DISPLAY;
                destinationId = DEV_ID_UICC;
                oneShot = true;
                break;
            case EVDL_ID_LANGUAGE_SELECT:
                sourceId = DEV_ID_TERMINAL;
                destinationId = DEV_ID_UICC;
                additionalInfo = new byte[4];
                // language tag
                additionalInfo[0] = (byte) 0xAD;
                // language code, defined in ISO639,coded in GSM 7-bit ex.
                // Emglish -> en -> 0x65 0x6E
                languageInfo = Locale.getDefault().getLanguage();
                additionalInfo[1] = 0x02;
                language = languageInfo.getBytes();
                additionalInfo[2] = language[0];
                additionalInfo[3] = language[1];

                oneShot = false;
                break;
            case EVDL_ID_BROWSER_TERMINATION:
                sourceId = DEV_ID_TERMINAL;
                destinationId = DEV_ID_UICC;
                // browser termination cause tag
                additionalInfo = new byte[3];
                additionalInfo[0] = (byte) 0xB4;
                additionalInfo[1] = 0x01;
                additionalInfo[2] = 0x00;
                oneShot = false;
                break;
            default:
                break;
        }
        resMsg.setSourceId(sourceId);
        resMsg.setDestinationId(destinationId);
        resMsg.setAdditionalInfo(additionalInfo);
        resMsg.setOneShot(oneShot);
        CatLog.d(LOGTAG, "onEventDownload - eventId[" + eventId + "]");
        mStkService.onEventDownload(resMsg);
    }

    private void handleCmdResponse(Bundle args) {
        if (mCurrentCmd == null) {
            return;
        }
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        if (null != mCurrentCmd && mCurrentCmd.getCmdType() != null) {
            CatLog.d(LOGTAG, "handleCmdResponse+ cmdName[" + mCurrentCmd.getCmdType().name() + "]");
        }

        // set result code
        boolean helpRequired = args.getBoolean(HELP, false);

        switch (args.getInt(RES_ID)) {
            case RES_ID_MENU_SELECTION:
                CatLog.d(LOGTAG, "RES_ID_MENU_SELECTION");
                if (isBipCommand(mCurrentCmd)) {
                    Toast toast = Toast.makeText(mContext.getApplicationContext(),
                            R.string.lable_busy_on_bip, Toast.LENGTH_LONG);
                    toast.setGravity(Gravity.BOTTOM, 0, 0);
                    toast.show();
                    return;
                }

                int menuSelection = args.getInt(MENU_SELECTION);
                switch (mCurrentMenuCmd.getCmdType()) {
                    case SET_UP_MENU:
                        // have already handled setup menu
                        mSetUpMenuHandled = true;
                    case SELECT_ITEM:
                        resMsg = new CatResponseMessage(mCurrentMenuCmd);
                        lastSelectedItem = getItemName(menuSelection);
                        if (helpRequired) {
                            resMsg.setResultCode(ResultCode.HELP_INFO_REQUIRED);
                        } else {
                            resMsg.setResultCode(ResultCode.OK);
                        }
                        resMsg.setMenuSelection(menuSelection);
                        break;
                }
                break;
            case RES_ID_INPUT:
                CatLog.d(LOGTAG, "RES_ID_INPUT");
                String input = args.getString(INPUT);
                if (input != null && (null != mCurrentCmd.geInput())
                        && (mCurrentCmd.geInput().yesNo)) {
                    boolean yesNoSelection = input
                            .equals(StkInputActivity.YES_STR_RESPONSE);
                    resMsg.setYesNo(yesNoSelection);
                } else {
                    if (helpRequired) {
                        resMsg.setResultCode(ResultCode.HELP_INFO_REQUIRED);
                    } else {
                        resMsg.setResultCode(ResultCode.OK);
                        resMsg.setInput(input);
                    }
                }
                break;
            case RES_ID_CONFIRM:
                CatLog.d(LOGTAG, "RES_ID_CONFIRM");
                boolean confirmed = args.getBoolean(CONFIRMATION);
                switch (mCurrentCmd.getCmdType()) {
                    case SET_UP_MENU:
                        CatLog.d(LOGTAG, "RES_ID_CONFIRM SET_UP_MENU");
                        return;
                    case DISPLAY_TEXT:
                        resMsg.setResultCode(confirmed ? ResultCode.OK
                                : ResultCode.UICC_SESSION_TERM_BY_USER);
                        break;
                    case LAUNCH_BROWSER:
                        resMsg.setResultCode(confirmed ? ResultCode.OK
                                : ResultCode.UICC_SESSION_TERM_BY_USER);
                        if (confirmed) {
                            launchBrowser = true;
                            mBrowserSettings = mCurrentCmd.getBrowserSettings();
                        }
                        break;
                    case SET_UP_CALL:
                        if (confirmed) {
                            processSetupCallResponse();
                            return;
                        }
                        // Cancel
                        mSetupCallInProcess = false;
                        resMsg.setResultCode(ResultCode.OK);
                        resMsg.setConfirmation(confirmed);
                        break;

                    case OPEN_CHANNEL:
                        if (confirmed) {
                            processOpenChannelResponse();
                            return;
                        }

                        // Cancel
                        resMsg.setResultCode(ResultCode.USER_NOT_ACCEPT);
                        resMsg.setConfirmation(confirmed);
                        break;
                }
                break;
            case RES_ID_DONE:
                resMsg.setResultCode(ResultCode.OK);
                break;
            case RES_ID_BACKWARD:
                CatLog.d(LOGTAG, "RES_ID_BACKWARD");
                switch (mCurrentCmd.getCmdType()) {
                    case OPEN_CHANNEL:
                        CatLog.d(LOGTAG, "RES_ID_BACKWARD - OPEN_CHANNEL");
                        resMsg.setResultCode(ResultCode.UICC_SESSION_TERM_BY_USER);
                        break;

                    default:
                        CatLog.d(LOGTAG, "RES_ID_BACKWARD - not OPEN_CHANNEL");
                        resMsg.setResultCode(ResultCode.BACKWARD_MOVE_BY_USER);
                        break;
                }
                break;
            case RES_ID_END_SESSION:
                CatLog.d(LOGTAG, "RES_ID_END_SESSION");
                resMsg.setResultCode(ResultCode.UICC_SESSION_TERM_BY_USER);
                break;
            case RES_ID_TIMEOUT:
                CatLog.d(LOGTAG, "RES_ID_TIMEOUT");
                resMsg.setResultCode(ResultCode.NO_RESPONSE_FROM_USER);
                // GCF test-case 27.22.4.1.1 Expected Sequence 1.5 (DISPLAY
                // TEXT,
                // Clear message after delay, successful) expects result code
                // OK.
                // If the command qualifier specifies no user response is
                // required
                // then send OK instead of NO_RESPONSE_FROM_USER
                if ((mCurrentCmd.getCmdType() != null && mCurrentCmd.getCmdType().value() == AppInterface.CommandType.DISPLAY_TEXT
                        .value())
                        && (mCurrentCmd.geTextMessage() != null && mCurrentCmd.geTextMessage().responseNeeded == true)) {
                    if (mCurrentCmd.geTextMessage().userClear == false) {
                        resMsg.setResultCode(ResultCode.OK);
                    }
                }
                break;
            default:
                CatLog.d(LOGTAG, "Unknown result id");
                return;
        }

        if (null != mCurrentCmd && mCurrentCmd.getCmdType() != null) {
            CatLog.d(LOGTAG, "handleCmdResponse- cmdName[" + mCurrentCmd.getCmdType().name() + "]");
        }
        mStkService.onCmdResponse(resMsg);
    }

    /**
     * Returns 0 or FLAG_ACTIVITY_NO_USER_ACTION, 0 means the user initiated the
     * action.
     * 
     * @param userAction If the userAction is yes then we always return 0
     *            otherwise mMenuIsVisible is used to determine what to return.
     *            If mMenuIsVisible is true then we are the foreground app and
     *            we'll return 0 as from our perspective a user action did
     *            cause. If it's false than we aren't the foreground app and
     *            FLAG_ACTIVITY_NO_USER_ACTION is returned.
     * @return 0 or FLAG_ACTIVITY_NO_USER_ACTION
     */
    private int getFlagActivityNoUserAction(InitiatedByUserAction userAction) {
        return ((userAction == InitiatedByUserAction.yes) | mMenuIsVisible) ?
                0 : Intent.FLAG_ACTIVITY_NO_USER_ACTION;
    }

    private void finishMenuActivity() {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName(PACKAGE_NAME, MENU_ACTIVITY_NAME);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TOP;

        intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.unknown);
        newIntent.putExtra("STATE", StkMenuActivity.STATE_END);

        newIntent.setFlags(intentFlags);
        mContext.startActivity(newIntent);
    }

    private void launchMenuActivity(Menu menu) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setClassName(PACKAGE_NAME, MENU_ACTIVITY_NAME);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        if (menu == null) {
            // We assume this was initiated by the user pressing the tool kit
            // icon
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.yes);

            newIntent.putExtra("STATE", StkMenuActivity.STATE_MAIN);
        } else {
            // We don't know and we'll let getFlagActivityNoUserAction decide.
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.unknown);

            newIntent.putExtra("STATE", StkMenuActivity.STATE_SECONDARY);
        }
        newIntent.setFlags(intentFlags);
        mContext.startActivity(newIntent);
    }

    private void launchInputActivity() {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown));
        newIntent.setClassName(PACKAGE_NAME, INPUT_ACTIVITY_NAME);
        newIntent.putExtra("INPUT", mCurrentCmd.geInput());
        mContext.startActivity(newIntent);
    }

    private void launchTextDialog() {
        if (canShowTextDialog(mCurrentCmd.geTextMessage()) == false) {
            sendOkMessage();
            return;
        }
        Intent newIntent = new Intent(this, StkDialogActivity.class);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_MULTIPLE_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown));
        TextMessage msg = mCurrentCmd.geTextMessage();
        newIntent.putExtra("TEXT", mCurrentCmd.geTextMessage());
        startActivity(newIntent);
    }

    private boolean canShowTextDialog(TextMessage msg) {
        // can show whatever screen it is.
        if (msg == null) {
            // using normal flow.
            return true;
        }
        CatLog.d(LOGTAG, "canShowTextDialog? mMenuIsVisible = " + mMenuIsVisible
                + " mInputIsVisible = " + " mDialogIsVisible = " + mDialogIsVisible);
        if (msg.isHighPriority == true) {
            return true;
        } else {
            // only show in idle screen.
            if (isIdleScreen(this.mContext) == true) {
                return true;
            }
            // if not in Idle Screen, but in Stk screen, will show the message.
            if (mMenuIsVisible == true || mInputIsVisible == true || mDialogIsVisible == true) {
                return true;
            }
        }
        return false;
    }

    public boolean isIdleScreen() {
        final ActivityManager am = (ActivityManager) getApplicationContext().getSystemService(
                Context.ACTIVITY_SERVICE);
        List<RecentTaskInfo> taskInfo = am.getRecentTasks(16, ActivityManager.RECENT_WITH_EXCLUDED);

        String home = null;
        if (taskInfo != null) {
            for (RecentTaskInfo task : taskInfo) {
                if (true == task.baseIntent.hasCategory(Intent.CATEGORY_HOME)) {
                    home = task.baseIntent.getComponent().getPackageName();
                    break;
                }
            }
        }

        boolean idle = false;
        List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
        if (runningAppInfo != null) {
            for (RunningAppProcessInfo app : runningAppInfo) {
                if (app.processName.equals(home)
                        && app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
                    idle = true;
                    break;
                }
            }
        }

        CatLog.d(LOGTAG, "[isIdleScreen][idle] : " + idle);
        return idle;
    }

    public boolean isIdleScreen(Context context) {
        String homePackage = null;
        String homeProcess = null;
        boolean idle = false;

        final ActivityManager am = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        List<RecentTaskInfo> taskInfo = am.getRecentTasks(16, ActivityManager.RECENT_WITH_EXCLUDED);

        if (taskInfo != null) {
            for (RecentTaskInfo task : taskInfo) {
                if (true == task.baseIntent.hasCategory(Intent.CATEGORY_HOME)) {
                    homePackage = task.baseIntent.getComponent().getPackageName();
                    break;
                }
            }
        }
        CatLog.d(LOGTAG, "[isIdleScreen] homePackage is: " + homePackage);

        try {
            ApplicationInfo appInfo = context.getPackageManager()
                    .getApplicationInfo(homePackage, 0);
            homeProcess = appInfo.processName;
        } catch (NameNotFoundException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        CatLog.d(LOGTAG, "home package = " + homePackage + ", home process = " + homeProcess);

        List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
        for (RunningAppProcessInfo app : runningAppInfo) {
            if (app.processName.equals(homeProcess) &&
                    app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
                idle = true;
                break;
            }
        }

        CatLog.d(LOGTAG, "[isIdleScreen][idle] : " + idle);
        return idle;
    }

    static String BROWSER_PACKAGE_NAME = "com.android.browser";

    public boolean isBrowserLaunched(Context context) {
        CatLog.d(LOGTAG, "[isBrowserLaunched]+");
        final ActivityManager am = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);

        boolean top = false;
        List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
        if (runningAppInfo != null) {
            for (RunningAppProcessInfo app : runningAppInfo) {
                if (app.processName.equals(BROWSER_PACKAGE_NAME)
                        && (app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND)) {
                    top = true;
                    break;
                }
            }
        }

        CatLog.d(LOGTAG, "[isBrowserLaunched][top] : " + top);
        CatLog.d(LOGTAG, "[isBrowserLaunched]-");
        return top;
    }

    // just for idle Screen text response
    private void handleIdleTextResponse() {
        CatResponseMessage resMsg = new CatResponseMessage(mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        byte[] additionalInfo = new byte[1];
        additionalInfo[0] = (byte) 0x01;
        resMsg.setAdditionalInfo(additionalInfo);
        CatLog.d(LOGTAG, "handleResponseOk ");
        if (mCurrentCmd.getCmdType() != null)
            CatLog.d(LOGTAG, "handleIdleTextResponse cmdName[" + mCurrentCmd.getCmdType().name()
                    + "]");
        mStkService.onCmdResponse(resMsg);
    }

    private void sendOkMessage() {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_RESPONSE_IDLE_TEXT;
        mServiceHandler.sendMessage(msg);
    }

    private void launchEventMessage() {
        TextMessage msg = mCurrentCmd.geTextMessage();
        if (msg == null || (msg.text != null && msg.text.length() == 0)) {
            CatLog.d(LOGTAG, "aaaaa [return] ");
            return;
        }

        Toast toast = new Toast(mContext.getApplicationContext());
        LayoutInflater inflate = (LayoutInflater) mContext
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflate.inflate(R.layout.stk_event_msg, null);
        TextView tv = (TextView) v
                .findViewById(com.android.internal.R.id.message);
        ImageView iv = (ImageView) v
                .findViewById(com.android.internal.R.id.icon);
        if (msg.icon != null) {
            iv.setImageBitmap(msg.icon);
        } else {
            iv.setVisibility(View.GONE);
        }
        if (!msg.iconSelfExplanatory) {
            CatLog.d(LOGTAG, "aaaaa [msg.iconSelfExplanatory = null] ");
            if (msg.text == null) {
                CatLog.d(LOGTAG, "aaaaa [msg.text == null] ");
                switch (mCurrentCmd.getCmdType()) {
                    case SEND_DTMF:
                        tv.setText(R.string.lable_send_dtmf);
                        break;
                    case SEND_SMS:
                        CatLog.d(LOGTAG, "aaaaa [SEND_SMS] ");
                        tv.setText(R.string.lable_send_sms);
                        break;
                    case SEND_SS:
                        tv.setText(R.string.lable_send_ss);
                        break;
                    case SEND_USSD:
                        tv.setText(R.string.lable_send_ussd);
                        break;
                    case CLOSE_CHANNEL:
                        tv.setText(R.string.lable_close_channel);
                        break;
                    case RECEIVE_DATA:
                        tv.setText(R.string.lable_receive_data);
                        break;
                    case SEND_DATA:
                        tv.setText(R.string.lable_send_data);
                        break;
                    case GET_CHANNEL_STATUS:
                        tv.setText(R.string.lable_get_channel_status);
                        break;
                }
            } else {
                tv.setText(msg.text);
            }
        }

        toast.setView(v);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private void launchConfirmationDialog(TextMessage msg) {
        msg.title = lastSelectedItem;
        correctTextMessage(msg);
        Intent newIntent = new Intent(this, StkDialogActivity.class);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown));
        newIntent.putExtra("TEXT", msg);
        startActivity(newIntent);
    }

    private void launchBrowser(BrowserSettings settings) {
        if (settings == null) {
            return;
        }
        // Set browser launch mode
        Intent intent = new Intent();
        intent.setClassName("com.android.browser",
                "com.android.browser.BrowserActivity");

        // to launch home page, make sure that data Uri is null.
        Uri data = null;
        if (settings.url != null) {
            data = Uri.parse(settings.url);
        }
        intent.setData(data);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        switch (settings.mode) {
            case USE_EXISTING_BROWSER:
                intent.setAction(Intent.ACTION_VIEW);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                break;
            case LAUNCH_NEW_BROWSER:
                intent.setAction(Intent.ACTION_VIEW);
                intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
                break;
            case LAUNCH_IF_NOT_ALREADY_LAUNCHED:
                intent.setAction(Intent.ACTION_VIEW);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                break;
        }
        // start browser activity
        startActivity(intent);
        // a small delay, let the browser start, before processing the next
        // command.
        // this is good for scenarios where a related DISPLAY TEXT command is
        // followed immediately.
        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {
        }
    }

    private void showIconToast(TextMessage msg) {
        Toast t = new Toast(this);
        ImageView v = new ImageView(this);
        v.setImageBitmap(msg.icon);
        t.setView(v);
        t.setDuration(Toast.LENGTH_LONG);
        t.show();
    }

    private void showTextToast(TextMessage msg) {
        msg.title = lastSelectedItem;

        Toast toast = Toast.makeText(mContext.getApplicationContext(), msg.text,
                Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    // TODO should show text and Icon
    private void showIconAndTextToast(TextMessage msg) {
        Toast t = new Toast(this);
        ImageView v = new ImageView(this);
        v.setImageBitmap(msg.icon);
        t.setView(v);
        t.setDuration(Toast.LENGTH_LONG);
        t.show();
    }

    private void launchCallMsg() {
        TextMessage msg = mCurrentCmd.getCallSettings().callMsg;
        if (msg.iconSelfExplanatory == true) {
            // only display Icon.

            if (msg.icon != null) {
                showIconToast(msg);
            } else {
                // do nothing.
                return;
            }
        } else {
            // show text & icon.
            if (msg.icon != null) {
                if (msg.text == null || msg.text.length() == 0) {
                    // show Icon only.
                    showIconToast(msg);
                } else {
                    showIconAndTextToast(msg);
                }
            } else {
                if (msg.text == null || msg.text.length() == 0) {
                    // do nothing
                    return;
                } else {
                    showTextToast(msg);
                }

            }
        }
    }

    private void launchIdleText() {
        TextMessage msg = mCurrentCmd.geTextMessage();

        CatLog.d(LOGTAG, "launchIdleText - text[" + msg.text
                + "] iconSelfExplanatory[" + msg.iconSelfExplanatory
                + "] icon[" + msg.icon + "]");
        if (msg.text == null) {
            CatLog.d(LOGTAG, "cancel IdleMode text");
            mNotificationManager.cancel(STK_NOTIFICATION_ID);
        } else {
            CatLog.d(LOGTAG, "Add IdleMode text");

            mNotificationManager.cancel(STK_NOTIFICATION_ID);
            Notification notification = new Notification();
            RemoteViews contentView = new RemoteViews(
                    PACKAGE_NAME,
                    com.android.internal.R.layout.status_bar_latest_event_content);

            notification.flags |= Notification.FLAG_NO_CLEAR;
            notification.icon = com.android.internal.R.drawable.stat_notify_sim_toolkit;
            // Set text and icon for the status bar and notification body.
            if (!msg.iconSelfExplanatory) {
                notification.tickerText = msg.text;
                contentView.setTextViewText(com.android.internal.R.id.text,
                        msg.text);
            }
            if (msg.icon != null) {
                CatLog.d(LOGTAG, "Idle Mode Text with icon");
                contentView.setImageViewBitmap(com.android.internal.R.id.icon,
                        msg.icon);
            } else {
                CatLog.d(LOGTAG, "Idle Mode Text without icon");
                contentView
                        .setImageViewResource(
                        com.android.internal.R.id.icon,
                        com.android.internal.R.drawable.stat_notify_sim_toolkit);
            }
            Intent notificationIntent = new Intent(mContext,
                    NotificationAlertActivity.class);
            // use mIdleMessage replace Intent parameter, because the extra
            // seems do not update
            // even create a new notification with same ID.
            StkApp.mIdleMessage = msg.text;
            // notificationIntent.putExtra(NOTIFICATION_KEY, msg.text);
            // notificationIntent.putExtra(NOTIFICATION_TITLE, StkApp.mPLMN;
            // notification.setLatestEventInfo(this, StkApp.mPLMN, msg.text,
            // PendingIntent.getActivity(mContext, 0, notificationIntent, 0));
            // setlatestEventInfo will create a new contentView, and replace
            // icon with default icon,
            // so remove it and use our own contentView.
            contentView.setTextViewText(com.android.internal.R.id.title, StkApp.mPLMN);
            notification.contentView = contentView;
            notification.contentIntent = PendingIntent.getActivity(mContext, 0, notificationIntent,
                    0);
            mNotificationManager.notify(STK_NOTIFICATION_ID, notification);
        }
    }

    private void launchToneDialog() {
        Intent newIntent = new Intent(this, ToneDialog.class);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown));
        newIntent.putExtra("TEXT", mCurrentCmd.geTextMessage());
        newIntent.putExtra("TONE", mCurrentCmd.getToneSettings());
        startActivity(newIntent);
    }

    private String getItemName(int itemId) {
        Menu menu = mCurrentCmd.getMenu();
        if (menu == null) {
            return null;
        }
        for (Item item : menu.items) {
            if (item.id == itemId) {
                return item.text;
            }
        }
        return null;
    }

    private boolean removeMenu() {
        try {
            if (mCurrentMenu.items.size() == 1 &&
                    mCurrentMenu.items.get(0) == null) {
                return true;
            }
        } catch (NullPointerException e) {
            CatLog.d(LOGTAG, "Unable to get Menu's items size");
            return true;
        }
        return false;
    }

    private void correctTextMessage(TextMessage msg) {
        switch (mCurrentCmd.getCmdType()) {
            case OPEN_CHANNEL:
                if (msg.text == null) {
                    msg.text = getDefaultText();
                }
                break;
            default:
                if (msg.text == null || msg.text.length() == 0) {
                    msg.text = getDefaultText();
                }
        }
        return;
    }

    private String getDefaultText() {
        String str = "";
        switch (mCurrentCmd.getCmdType()) {
            case LAUNCH_BROWSER:
                str = getResources().getString(R.string.action_launch_browser);
                break;
            case SET_UP_CALL:
                str = getResources().getString(R.string.action_setup_call);
                break;
            case OPEN_CHANNEL:
                str = getResources().getString(R.string.lable_open_channel);
                break;
        }
        return str;
    }

    public boolean haveEndSession() {
        if (mCmdsQ.size() == 0)
            return false;
        for (int i = 0; i < mCmdsQ.size(); i++) {
            // if delay message involve OP_END_SESSION, return true;
            if (mCmdsQ.get(i).id == OP_END_SESSION) {
                CatLog.d(LOGTAG, "end Session a delay Message");
                return true;
            }
        }
        return false;
    }

    private final IntentFilter mLocaleChangedFilter =
            new IntentFilter("android.intent.action.LOCALE_CHANGED");

    private final BroadcastReceiver mStkLocaleChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String evtAction = intent.getAction();
            int evdl = EVDL_ID_USER_ACTIVITY;

            CatLog.d(LOGTAG, "mStkLocaleChangedReceiver() - evtAction[" + evtAction + "]");

            if (evtAction.equals("android.intent.action.LOCALE_CHANGED")) {
                CatLog.d(LOGTAG, "mStkLocaleChangedReceiver() - Received[LOCALE_CHANGED]");
                evdl = EVDL_ID_LANGUAGE_SELECT;
            } else {
                CatLog.d(LOGTAG, "mStkLocaleChangedReceiver() - Received needn't handle!");
                return;
            }
            SendEventDownloadMsg(evdl);
        }
    };

    private final IntentFilter mIdleScreenAvailableFilter =
            new IntentFilter("android.intent.action.stk.IDLE_SCREEN_AVAILABLE");

    private final BroadcastReceiver mStkIdleScreenAvailableReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String evtAction = intent.getAction();
            int evdl = EVDL_ID_USER_ACTIVITY;

            CatLog.d(LOGTAG, "mStkIdleScreenAvailableReceiver() - evtAction[" + evtAction + "]");

            if (evtAction.equals("android.intent.action.stk.IDLE_SCREEN_AVAILABLE")) {
                CatLog.d(LOGTAG,
                        "mStkIdleScreenAvailableReceiver() - Received[IDLE_SCREEN_AVAILABLE]");
                evdl = EVDL_ID_IDLE_SCREEN_AVAILABLE;
            } else {
                CatLog.d(LOGTAG, "mStkIdleScreenAvailableReceiver() - Received needn't handle!");
                return;
            }
            SendEventDownloadMsg(evdl);
        }
    };

    private void registerStkReceiver() {
        CatLog.d(LOGTAG, "registerStkReceiver()");
        // registerReceiver(mStkLocaleChangedReceiver, mLocaleChangedFilter);
        registerReceiver(mStkIdleScreenAvailableReceiver, mIdleScreenAvailableFilter);
    }

    private void unregisterStkReceiver() {
        CatLog.d(LOGTAG, "unregisterStkReceiver()");
        // unregisterReceiver(mStkLocaleChangedReceiver);
        unregisterReceiver(mStkIdleScreenAvailableReceiver);
    }

    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCard.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(
                        com.android.internal.telephony.Phone.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - simId[" + simId + "]  state["
                        + simState
                        + "] bStkEventReceiverReged[" + bStkEventReceiverReged + "]");

                if ((simId == miSIMid)) {
                    if (IccCard.INTENT_VALUE_ICC_NOT_READY.equals(simState)) {
                        if (bStkEventReceiverReged) {
                            CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - unReg stk Event EvDl");
                            unregisterStkReceiver();
                            bStkEventReceiverReged = false;
                        }
                        mNotificationManager.cancel(STK_NOTIFICATION_ID);
                    } else {
                        if (!bStkEventReceiverReged) {
                            CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - Reg stk Event EvDl");
                            registerStkReceiver();
                            bStkEventReceiverReged = true;
                        }
                    }
                }
            } else if (intent.getAction().equals("android.intent.action.ACTION_SHUTDOWN_IPO")) {
                CatLog.d(LOGTAG, "[IPO_SHUTDOWN][initial mMainCmd] : " + mMainCmd);
                mMainCmd = null;
                mSetUpMenuHandled = false;
                mSetupMenuCalled = false;
                CatLog.d(LOGTAG, "[IPO_SHUTDOWN][mMainCmd] : " + mMainCmd);
            } else if (intent.getAction().equals(ACTION_REMOVE_IDLE_TEXT)) {
                if (miSIMid == Phone.GEMINI_SIM_1) {
                    CatLog.d(LOGTAG, "remove Stk1 idle mode text by Refresh command");
                    mNotificationManager.cancel(STK_NOTIFICATION_ID);
                }
            } else if (intent.getAction().equals(ACTION_REMOVE_IDLE_TEXT_2)) {
                if (miSIMid == Phone.GEMINI_SIM_2) {
                    CatLog.d(LOGTAG, "remove Stk2 idle mode text by Refresh command");
                    mNotificationManager.cancel(STK_NOTIFICATION_ID);
                }
            }
        }
    };

    private boolean bStkEventReceiverReged = false;

    private void SendEventDownloadMsg(int evdlId) {
        CatLog.d(LOGTAG, "SendEventDownloadMsg() - evdlId[" + evdlId + "]");
        Bundle args = new Bundle();
        args.putInt(OPCODE, OP_EVENT_DOWNLOAD);
        args.putInt(EVDL_ID, evdlId);

        Message msg = mServiceHandler.obtainMessage();
        // msg.arg1 = EVDL_ID_IDLE_SCREEN_AVAILABLE;
        msg.arg1 = OP_EVENT_DOWNLOAD;
        msg.obj = args;

        mServiceHandler.sendMessage(msg);
    }

    private boolean isBipCommand(CatCmdMessage cmd) {
        switch (cmd.getCmdType()) {
            case OPEN_CHANNEL:
            case CLOSE_CHANNEL:
            case SEND_DATA:
            case RECEIVE_DATA:
            case GET_CHANNEL_STATUS:
                CatLog.d(this, "BIP command");
                return true;
        }

        CatLog.d(this, "non-BIP command");
        return false;
    }

    private boolean isBusyOnCall() {
        Phone.State s;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
            s = ((GeminiPhone) mPhone).getState();
        } else {
            s = mPhone.getState();
        }

        CatLog.d(this, "isBusyOnCall: " + s);
        return (s == Phone.State.RINGING);
    }

    private void changeMenuStateToMain() {
        CatLog.d(LOGTAG, "call changeMenuStateToMain");
        if (mMainCmd == null) {
            CatLog.d(LOGTAG, "changeMenuStateToMain: mMainCmd is null");
        }

        mCurrentCmd = mMainCmd;
        mCurrentMenuCmd = mMainCmd;
        lastSelectedItem = null;
        if (mCurrentMenu != null && mMainCmd != null) {
            mCurrentMenu = mMainCmd.getMenu();
        }
    }

    public void setUserAccessState(boolean state) {
        CatLog.d(LOGTAG, "setUserAccessState: state=" + state);
        isUserAccessed = state;
    }

    private void init() {
        mCmdsQ = new LinkedList<DelayedCmd>();
        serviceThread = new Thread(null, this, "Stk2 App Service");
        serviceThread.start();
        mContext = getBaseContext();
        mNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        sInstance = this;

        mPhone = PhoneFactory.getDefaultPhone();
        ((GeminiPhone) mPhone).registerForPreciseCallStateChangedGemini(mCallHandler,
                PHONE_STATE_CHANGED, null, miSIMid);
        ((GeminiPhone) mPhone).registerForSuppServiceFailedGemini(mCallHandler,
                SUPP_SERVICE_FAILED, null, miSIMid);
        initNotify();

        IntentFilter mSIMStateChangeFilter = new IntentFilter(
                TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mSIMStateChangeFilter.addAction("android.intent.action.ACTION_SHUTDOWN_IPO");
        if (miSIMid == Phone.GEMINI_SIM_1) {
            mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT);
        } else {
            mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT_2);
        }
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
    }

    public void sendMessageToServiceHandler(int opCode, Object obj) {
        CatLog.d(LOGTAG, "call sendMessageToServiceHandler: " + opCodeToString(opCode));
        if (mServiceHandler == null) {
            waitForLooper();
        }
        Message msg = mServiceHandler.obtainMessage(0, opCode, 0, obj);
        mServiceHandler.sendMessage(msg);
    }

    private String opCodeToString(int opCode) {
        switch (opCode) {
            case OP_CMD:
                return "OP_CMD";
            case OP_RESPONSE:
                return "OP_RESPONSE";
            case OP_LAUNCH_APP:
                return "OP_LAUNCH_APP";
            case OP_END_SESSION:
                return "OP_END_SESSION";
            case OP_BOOT_COMPLETED:
                return "OP_BOOT_COMPLETED";
            case OP_EVENT_DOWNLOAD:
                return "OP_EVENT_DOWNLOAD";
            case OP_DELAYED_MSG:
                return "OP_DELAYED_MSG";
            case OP_RESPONSE_IDLE_TEXT:
                return "OP_RESPONSE_IDLE_TEXT";
            default:
                return "unknown op code";
        }
    }
}
