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

import static android.provider.Telephony.Intents.ACTION_REMOVE_IDLE_TEXT;
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
import android.os.SystemProperties;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Telephony;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;
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
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
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
import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.bip.BearerDesc;;
import com.mediatek.op.telephony.cat.CatOpAppInterfaceImp;
import android.provider.Settings;

import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.telephony.ITelephonyEx;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.gemini.MTKCallManager;

import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

/**
 * SIM toolkit application level service. Interacts with Telephopny messages,
 * application's launch and user input from STK UI elements.
 *
 */
public class StkAppService extends Service implements Runnable {

    // members
    protected class StkContext {
        protected CatCmdMessage mMainCmd = null;
        protected CatCmdMessage mCurrentCmd = null;
        protected CatCmdMessage mCurrentMenuCmd = null;
        protected Menu mCurrentMenu = null;
        protected String lastSelectedItem = null;
        protected boolean mMenuIsVisible = false;
        protected boolean mInputIsVisible = false;
        protected boolean mDialogIsVisible = false;
        protected boolean responseNeeded = true;
        protected boolean launchBrowser = false;
        protected BrowserSettings mBrowserSettings = null;
        protected boolean mSetupMenuCalled = false; 
        protected boolean mSetUpMenuHandled = false;
        protected boolean mNotified = false;
        protected boolean isUserAccessed = false;
        protected boolean mSetupCallInProcess = false; // true means in process.
        protected int mAvailable = STK_AVAIL_INIT;
        protected LinkedList<DelayedCmd> mCmdsQ = null;
        protected boolean mCmdInProgress = false;
    }

    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private Context mContext = null;
    /* TODO: GEMINI+ */
    static int STK_GEMINI_SIM_NUM = 4;
    static int STK_GEMINI_BROADCAST_ALL = 99;
    private AppInterface[] mStkService = new AppInterface[STK_GEMINI_SIM_NUM];
    private StkContext[] mStkContext = new StkContext[STK_GEMINI_SIM_NUM];
    private NotificationManager mNotificationManager = null;
    private static final int PHONE_DISCONNECT = 1001;
    private static final int PHONE_DISCONNECT2 = 1002;
    private static final int PHONE_DISCONNECT3 = 1003;
    private static final int PHONE_DISCONNECT4 = 1004;    
    private Object mCallManager = null;
    private static final int[] PHONE_DISCONNECT_GEMINI = new int[] { PHONE_DISCONNECT, PHONE_DISCONNECT2,
            PHONE_DISCONNECT3, PHONE_DISCONNECT4 };
    private static final int[] GEMINI_SLOT = new int[] {PhoneConstants.GEMINI_SIM_1,PhoneConstants.GEMINI_SIM_2,
        PhoneConstants.GEMINI_SIM_3,PhoneConstants.GEMINI_SIM_4}; 
    private int mEvdlCallObj = 0;
    private LinkedList<Integer> mEvdlCallObjQ = new LinkedList();
    
    static StkAppService sInstance = null;
//    static private boolean mFirstCmdSetupMenu = false;//whether the first command is set up menu
//    static private String mstrbackDisplayTitle = null;
    public Phone mPhone = null;
//    static public final String NOTIFICATION_KEY = "notification_message";
//    static public final String NOTIFICATION_TITLE = "notification_title";
    // Used for setting FLAG_ACTIVITY_NO_USER_ACTION when
    // creating an intent.
    private Call.State mPreCallState = Call.State.IDLE;
    private Call.State mPreCallState2 = Call.State.IDLE;
    private Call.State mPreCallState3 = Call.State.IDLE;
    private Call.State mPreCallState4 = Call.State.IDLE;
    private enum InitiatedByUserAction {
        yes,            // The action was started via a user initiated action
        unknown,        // Not known for sure if user initated the action
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
    static final String CMD_SIM_ID = "sim id";

    // operations ids for different service functionality.
    static final int OP_CMD = 1;
    static final int OP_RESPONSE = 2;
    static final int OP_LAUNCH_APP = 3;
    static final int OP_END_SESSION = 4;
    static final int OP_BOOT_COMPLETED = 5;
    static final int OP_EVENT_DOWNLOAD = 6;
    private static final int OP_DELAYED_MSG = 7;

    private static final int OP_RESPONSE_IDLE_TEXT = 8;
    static final int OP_REMOVE_STM = 9;
    private static final int OP_EVDL_CALL_DISCONN_TIMEOUT = 10;
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
    static final int EVDL_ID_CALL_CONNECTED = 0X01;        
    static final int EVDL_ID_CALL_DISCONNECTED = 0X02;    
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
    
    private static final String PACKAGE_NAME = "com.android.stk";
    /* TODO: GEMINI+ begin */
    private static final String STK1_MENU_ACTIVITY_NAME =
                                        PACKAGE_NAME + ".StkMenuActivity";
    /*private static final String STK2_MENU_ACTIVITY_NAME =
                                        PACKAGE_NAME + ".AliasStkMenuActivity";*/
    private static final String STK2_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityII";
    private static final String STK3_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityIII";
    private static final String STK4_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityIV";

    private static final String STK1_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivity";

    private static final String STK2_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityII";
    private static final String STK3_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityIII";
    private static final String STK4_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityIV";

    // Notification id used to display Idle Mode text in NotificationManager.
    private static final int STK1_NOTIFICATION_ID = 333;
    private static final int STK2_NOTIFICATION_ID = 334;
    private static final int STK3_NOTIFICATION_ID = 335;
    private static final int STK4_NOTIFICATION_ID = 336;
    /* TODO: GEMINI+ end */
    
    private static final int PHONE_STATE_CHANGED = 101;
    private static final int SUPP_SERVICE_FAILED = 102;

    private static final int miSIMid = 0;  // Gemini SIM1  - com.android.internal.telephony.Phone.GEMINI_SIM_1

    private static final String LOGTAG = "Stk-SAS ";

    public static final int STK_AVAIL_INIT = -1;
    public static final int STK_AVAIL_NOT_AVAILABLE = 0;
    public static final int STK_AVAIL_AVAILABLE = 1;

    private static boolean mPhoneStateChangeReg = false;
    private static final int AP_EVDL_TIMEOUT = 8*1000;
    
    Thread serviceThread = null;
    // Inner class used for queuing telephony messages (proactive commands,
    // session end) while the service is busy processing a previous message.
    private class DelayedCmd {
        // members
        int id;
        CatCmdMessage msg;
        int sim_id;

        DelayedCmd(int id, CatCmdMessage msg, int sim_id) {
            this.id = id;
            this.msg = msg;
            this.sim_id = sim_id;
        }
    }

    static boolean isSetupMenuCalled(int SIMID) {
        CatLog.d("StkAppService", "isSetupMenuCalled, sim id: " + SIMID + ",[" + sInstance + "]");
        if (sInstance != null && (SIMID >= 0 && SIMID < StkAppService.STK_GEMINI_SIM_NUM))
        {
            CatLog.d("StkAppService", "isSetupMenuCalled, Stk context: " + sInstance.mStkContext[SIMID]);
            if (sInstance.mStkContext[SIMID] != null)
                return sInstance.mStkContext[SIMID].mSetupMenuCalled;
            else
                return false;
        }
        else
            return false;
    }

    @Override
    public void onCreate() {
            CatLog.d(this, " StkAppService Oncreate");
        CatLog.d(LOGTAG, " onCreate()+");
        // Initialize members
        int i = 0;
        int sim_id = PhoneConstants.GEMINI_SIM_1;
        for (i = 0; i < STK_GEMINI_SIM_NUM; i++)
        {
            switch (i)
            {
                case 1:
                    sim_id = PhoneConstants.GEMINI_SIM_2;
                    break;
                case 2:
                    sim_id = PhoneConstants.GEMINI_SIM_3;
                    break;
                case 3:
                    sim_id = PhoneConstants.GEMINI_SIM_4;
                    break;
                default:
                    break;
            }
            mStkService[i] = com.android.internal.telephony.cat.CatService.getInstance(sim_id);

            mStkContext[i] = new StkContext();
            mStkContext[i].mAvailable = STK_AVAIL_INIT;
            mStkContext[i].mCmdsQ = new LinkedList<DelayedCmd>();
        }
        
        serviceThread = new Thread(null, this, "Stk App Service");
        serviceThread.start();
        mContext = getBaseContext();
        mNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        sInstance = this;
        
        initNotify();
        
        IntentFilter mSIMStateChangeFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mSIMStateChangeFilter.addAction("android.intent.action.ACTION_SHUTDOWN_IPO");
        mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT);
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            registerReceiver(mEventDownloadCallReceiver, mEventDownloadCallFilter);        
        }
        CatLog.d(LOGTAG, " onCreate()-");
    }

    /**
     * @param intent The intent with action {@link Telephony.Intents#SPN_STRINGS_UPDATED_ACTION}
     * @return The string to use for the plmn, or null if it should not be shown.
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
        int i = 0;
        for (i = 0; i < STK_GEMINI_SIM_NUM; i++)
            StkApp.mPLMN[i] = getDefaultPlmn();
        final IntentFilter filter = new IntentFilter();
        filter.addAction(SPN_STRINGS_UPDATED_ACTION);
        registerReceiver(new BroadcastReceiver() {
            public void onReceive(Context context, Intent intent) {
                final String action = intent.getAction();
                if (SPN_STRINGS_UPDATED_ACTION.equals(action)) {
                    int j = 0;
                    for (j = 0; j < StkAppService.STK_GEMINI_SIM_NUM; j++)
                        StkApp.mPLMN[j] = getTelephonyPlmnFrom(intent);
                }
            }
        }, filter); 
        return;
    }
    
    @Override
    public void onStart(Intent intent, int startId) {
        if (intent == null) {
            CatLog.d(this, "StkAppService onStart intent is null so return");
            return;
        }

        Bundle args = intent.getExtras();
        if (args == null) {
            CatLog.d(this, "StkAppService onStart args is null so return");
            return;
        }

        int[] op = args.getIntArray(OPCODE);
        if (op == null)
        {
            CatLog.d(this, "StkAppService onStart op is null  return. args: " + args);
            return;
        }
        int sim_id = op[1];
        CatLog.d(this, "StkAppService onStart sim id: " + sim_id + ", op: " + op[0] + ", " + args);
        if ((sim_id >= 0 && sim_id < STK_GEMINI_SIM_NUM) && mStkService[sim_id] == null)
        {
            mStkService[sim_id] = com.android.internal.telephony.cat.CatService.getInstance(sim_id);
            if (mStkService[sim_id] == null) {
                CatLog.d(this, "StkAppService onStart mStkService is null  return, please check op code. Make sure it did not come from CatService");
                if (op[0] == OP_CMD) {
                    stopSelf();
                }
                return;
            }
        }

        if (mPhone == null)
        {
            mPhone = PhoneFactory.getDefaultPhone();
        }

        /* TODO: Gemini and non-Gemini are different begine */
        CatLog.d(this, "StkAppService onStart mPhone: " + ((mPhone != null)? 1 : 0) + ", mPhoneStateChangeReg: " + mPhoneStateChangeReg);
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
        {
            if (mPhone != null && !mPhoneStateChangeReg)
            {
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).registerForPreciseCallStateChanged(mCallHandler, 
                    PHONE_STATE_CHANGED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).registerForPreciseCallStateChanged(mCallHandler2, 
                    PHONE_STATE_CHANGED, null);
                if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForPreciseCallStateChanged(mCallHandler3, PHONE_STATE_CHANGED, null);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForSuppServiceFailed(mCallHandler3, SUPP_SERVICE_FAILED, null);
                }
                if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForPreciseCallStateChanged(mCallHandler3, PHONE_STATE_CHANGED, null);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForSuppServiceFailed(mCallHandler3, SUPP_SERVICE_FAILED, null);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).registerForPreciseCallStateChanged(mCallHandler4, PHONE_STATE_CHANGED, null);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).registerForSuppServiceFailed(mCallHandler4, SUPP_SERVICE_FAILED, null);
                }
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).registerForSuppServiceFailed(mCallHandler, SUPP_SERVICE_FAILED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).registerForSuppServiceFailed(mCallHandler2, SUPP_SERVICE_FAILED, null);
                if (false == FeatureOption.MTK_BSP_PACKAGE) {
                    if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
                        mCallManager = MTKCallManager.getInstance();
                    } else {
                        mCallManager = CallManager.getInstance();
                    }
                    registerForDisconnect(mCallManager,mCallDisConnHandler,PHONE_DISCONNECT_GEMINI);
                }                
                mPhoneStateChangeReg = true;
            }
        }
        else
        {
            if (mPhone != null && !mPhoneStateChangeReg)
            {
                mPhone.registerForPreciseCallStateChanged(mCallHandler, PHONE_STATE_CHANGED, null);
                mPhone.registerForSuppServiceFailed(mCallHandler, SUPP_SERVICE_FAILED, null);
                mPhoneStateChangeReg = true;
            }
        }
        /* TODO: Gemini and non-Gemini are different end */
        waitForLooper();

        // onStart() method can be passed a null intent
        // TODO: replace onStart() with onStartCommand()
        Message msg = mServiceHandler.obtainMessage();

        msg.arg1 = op[0];
        msg.arg2 = sim_id;
        switch(msg.arg1) {
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
        case OP_REMOVE_STM:
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
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            unregisterReceiver(mEventDownloadCallReceiver);    
            unregisterForDisconnect(mCallManager,mCallDisConnHandler);        
        }
        
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
        {
            /* TODO: Gemini and non-Gemini are different begine */
            if (mPhone != null) {
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).unregisterForPreciseCallStateChanged(mCallHandler);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).unregisterForPreciseCallStateChanged(mCallHandler2);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).unregisterForSuppServiceFailed(mCallHandler);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).unregisterForSuppServiceFailed(mCallHandler2);
                if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).unregisterForPreciseCallStateChanged(mCallHandler3);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).unregisterForSuppServiceFailed(mCallHandler3);
                }
                if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).unregisterForPreciseCallStateChanged(mCallHandler3);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).unregisterForSuppServiceFailed(mCallHandler3);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).unregisterForPreciseCallStateChanged(mCallHandler4);
                    ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).unregisterForSuppServiceFailed(mCallHandler4);
                }
            } else {
                CatLog.d(LOGTAG, "mPhone is null so don't need to unregister");
            }
            /* TODO: Gemini and non-Gemini are different end */
        }
        else
        {
            if (mPhone != null) {
                mPhone.registerForPreciseCallStateChanged(mCallHandler, PHONE_STATE_CHANGED, null);
                mPhone.registerForSuppServiceFailed(mCallHandler, SUPP_SERVICE_FAILED, null);
            } else {
                CatLog.d(LOGTAG, "mPhone is null so don't need to unregister");
            }
        }
        
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
    void indicateMenuVisibility(boolean visibility, int sim_id) {
        if (sim_id >=0 && sim_id < STK_GEMINI_SIM_NUM) {
            mStkContext[sim_id].mMenuIsVisible = visibility;
        }
    }
    
    /*
     * Package api used by StkInputActivity to indicate if its on the foreground.
     */
    void indicateInputVisibility(boolean visibility, int sim_id) {
        if (sim_id >=0 && sim_id < STK_GEMINI_SIM_NUM) {
            mStkContext[sim_id].mInputIsVisible = visibility;
        }
    }

    /*
     * Package api used by StkDialogActivity to indicate if its on the foreground.
     */
    void indicateDialogVisibility(boolean visibility, int sim_id) {
        if (sim_id >=0 && sim_id < STK_GEMINI_SIM_NUM) {
            mStkContext[sim_id].mDialogIsVisible = visibility;
        }
    }
    
    /*
     * Package api used by StkMenuActivity to get its Menu parameter.
     */
    Menu getMenu(int sim_id) {
        CatLog.d(LOGTAG, "StkAppService, getMenu, sim id: " + sim_id);
        if (sim_id >=0 && sim_id < STK_GEMINI_SIM_NUM)
            return mStkContext[sim_id].mCurrentMenu;
        else
            return null;
    }

    boolean isCurCmdSetupCall(int sim_id) {
        if (sim_id < 0 || sim_id >= STK_GEMINI_SIM_NUM) {
            CatLog.d(LOGTAG, "[isCurCmdSetupCall] sim id is out of range");
            return false;
        }
        else if (mStkContext[sim_id].mCurrentCmd == null) {
            CatLog.d(LOGTAG, "[isCurCmdSetupCall][mCurrentCmd]:null");
            return false;
        } else if(mStkContext[sim_id].mCurrentCmd.getCmdType() == null){
            CatLog.d(LOGTAG, "[isCurCmdSetupCall][mCurrentCmd.getCmdType()]:null");
            return false;
        } else {
            CatLog.d(LOGTAG, "SET UP CALL Cmd Check["  + mStkContext[sim_id].mCurrentCmd.getCmdType().value() + "]");
            return (AppInterface.CommandType.SET_UP_CALL.value() == mStkContext[sim_id].mCurrentCmd.getCmdType().value());
        }
     }

    boolean isCurCmdDisPlayText(int sim_id) {
        if (sim_id < 0 || sim_id >= STK_GEMINI_SIM_NUM) {
            CatLog.d(LOGTAG, "[isCurCmdDisPlayText] sim id is out of range");
            return false;
        }
        else if (mStkContext[sim_id].mCurrentCmd == null) {
            CatLog.d(LOGTAG, "[isCurCmdDisPlayText][mCurrentCmd]:null");
            return false;
        } else if(mStkContext[sim_id].mCurrentCmd.getCmdType() == null) {
            CatLog.d(LOGTAG, "[isCurCmdDisPlayText][mCurrentCmd.getCmdType()]:null");
            return false;
        } else {
            CatLog.d(LOGTAG, "DISPLAY TEXT Cmd Check["  + mStkContext[sim_id].mCurrentCmd.getCmdType().value() + "]");
            return (AppInterface.CommandType.DISPLAY_TEXT.value() == mStkContext[sim_id].mCurrentCmd.getCmdType().value());
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
            if(serviceThread == null || serviceThread.isAlive() == false) {
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
            if(null == msg)
            {
                CatLog.d(LOGTAG, "ServiceHandler handleMessage msg is null");
                return;
            }
            int opcode = msg.arg1;
            int sim_id = msg.arg2;

            CatLog.d(LOGTAG, "handleMessage opcode[" + opcode + "], sim id[" + sim_id + "]");
            if (opcode == OP_CMD && msg.obj != null && ((CatCmdMessage)msg.obj).getCmdType() != null) {
                CatLog.d(LOGTAG, "handleMessage cmdName[" + ((CatCmdMessage)msg.obj).getCmdType().name() + "]");
            }

            switch (opcode) {
            case OP_LAUNCH_APP:
                if (mStkContext[sim_id].mMainCmd == null) {
                    // nothing todo when no SET UP MENU command didn't arrive.
                    return;
                }
                if(isBusyOnCall() == true) {
                    Toast toast = Toast.makeText(mContext.getApplicationContext(), R.string.lable_busy_on_call, Toast.LENGTH_LONG);
                    toast.setGravity(Gravity.BOTTOM, 0, 0);
                    toast.show();
                    return;
                }
                if (mStkContext[sim_id].mAvailable != STK_AVAIL_AVAILABLE)
                {
                    Toast toast = Toast.makeText(mContext.getApplicationContext(), R.string.lable_busy_on_call, Toast.LENGTH_LONG);
                    toast.setGravity(Gravity.BOTTOM, 0, 0);
                    toast.show();
                    return;
                }
                CatLog.d(LOGTAG, "handleMessage OP_LAUNCH_APP - mCmdInProgress[" + mStkContext[sim_id].mCmdInProgress + "]");
                if(mStkContext[sim_id].mCurrentMenu == mStkContext[sim_id].mMainCmd.getMenu() ||  mStkContext[sim_id].mCurrentMenu == null) {
                    launchMenuActivity(null, sim_id);
                } else {
                    launchMenuActivity(mStkContext[sim_id].mCurrentMenu, sim_id);
                }
                
                setUserAccessState(true, sim_id);
                break;
            case OP_CMD:
                CatLog.d(LOGTAG, "[OP_CMD]");
//                PackageManager pm = mContext.getPackageManager();
//                // check that STK app package is known to the PackageManager
//                ComponentName cName = new ComponentName("com.android.stk",
//                        "com.android.stk.StkLauncherActivity");
//                boolean bpkgNotInstalled = pm.getComponentEnabledSetting(cName) == PackageManager.COMPONENT_ENABLED_STATE_DISABLED;
//                //Log.d(LOGTAG, "findbug Installed:" + bpkgNotInstalled + " cmdInter:" + isCmdInteractive((StkCmdMessage) msg.obj) + " process: " + mCmdInProgress);
                CatCmdMessage cmdMsg = (CatCmdMessage) msg.obj;
//                if(bpkgNotInstalled){
//                  CatLog.d(LOGTAG, "[prepare to start STKAPPSERVICE]");                
//               switch (cmdMsg.getCmdType()){
//               case SET_UP_MENU:
//                   CatLog.d(LOGTAG, "[OP_CMD][SETUPMENU delayed]");  
//                   mFirstCmdSetupMenu = true;
//                   if(cmdMsg.getMenu() != null){
//                       mstrbackDisplayTitle = cmdMsg.getMenu().title;
//                       CatLog.d(LOGTAG, "[OP_CMD][SETUPMENU delayed][Menu Title] : " + mstrbackDisplayTitle);
//                   }                   
//                   break;
//                   }
//                  StkAppInstaller.install(mContext);
//                  Message msgNew = mServiceHandler.obtainMessage();
//                  msgNew.obj = msg.obj;
//                  msgNew.arg1 = OP_CMD;
//                  mServiceHandler.sendMessageDelayed(msgNew, 50);
//              }else{
//                  CatLog.d(LOGTAG, "[OP_CMD][Normal]");                    
//                             // There are two types of commands:
                    // 1. Interactive - user's response is required.
                    // 2. Informative - display a message, no interaction with the user.
                    //
                    // Informative commands can be handled immediately without any delay.
                    // Interactive commands can't override each other. So if a command
                    // is already in progress, we need to queue the next command until
                    // the user has responded or a timeout expired.
                    if (cmdMsg == null) {
                        /* In EMMA test case, cmdMsg may be null */
                        return;
                    }
                    if (!isCmdInteractive(cmdMsg)) {
                                 CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Not Interactive]");
                        handleCmd(cmdMsg, sim_id);
                    } else {
                                 CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive]");
                        if (!mStkContext[sim_id].mCmdInProgress) {
                                     CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive][not in progress]");
                            mStkContext[sim_id].mCmdInProgress = true;
                            handleCmd((CatCmdMessage) msg.obj, sim_id);
                        } else {
                                     CatLog.d(LOGTAG, "[OP_CMD][Normal][Not DISPLAY_TEXT][Interactive][in progress]");
                            mStkContext[sim_id].mCmdsQ.addLast(new DelayedCmd(OP_CMD,
                                    (CatCmdMessage) msg.obj, sim_id));
                        }
                    }                   
                
//              }
                break;
            case OP_RESPONSE:
                CatLog.d(LOGTAG, " [OP_RESPONSE][responseNeeded] : " + mStkContext[sim_id].responseNeeded);
                if (mStkContext[sim_id].responseNeeded) {
                    handleCmdResponse((Bundle) msg.obj, sim_id);
                }
                // call delayed commands if needed.
                if (mStkContext[sim_id].mCmdsQ.size() != 0) {
                    callDelayedMsg(sim_id);
                } else {
                    mStkContext[sim_id].mCmdInProgress = false;
                }
                // reset response needed state var to its original value.
                mStkContext[sim_id].responseNeeded = true;
                break;
            case OP_END_SESSION:
                if (!mStkContext[sim_id].mCmdInProgress) {
                    mStkContext[sim_id].mCmdInProgress = true;
                    handleSessionEnd(sim_id);
                } else {
                    mStkContext[sim_id].mCmdsQ.addLast(new DelayedCmd(OP_END_SESSION, null, sim_id));
                }
                break;
            case OP_BOOT_COMPLETED:
                CatLog.d(LOGTAG, " OP_BOOT_COMPLETED");
//                if (mMainCmd == null) {
//                    CatLog.d(LOGTAG, "OP_BOOT_COMPLETED - unInstall");
//                    StkAppInstaller.unInstall(mContext);
//                }
                break;
            case OP_REMOVE_STM:
                {
                    CatLog.d(LOGTAG, "OP_REMOVE_STM");
                    if (mStkContext[sim_id] != null)
                    {
                        mStkContext[sim_id].mCurrentMenu = null;
                        mStkContext[sim_id].mSetupMenuCalled = false;
                    }
                    StkAppInstaller appInstaller = StkAppInstaller.getInstance();
                    appInstaller.unInstall(mContext, sim_id);
                    if (mStkService[sim_id] != null)
                    {
                        mStkService[sim_id].onDBHandler(sim_id);
                    }
                }
                break;
            case OP_EVENT_DOWNLOAD:
                CatLog.d(LOGTAG, "OP_EVENT_DOWNLOAD");
                handleEventDownload((Bundle) msg.obj, sim_id);
                break;
            case OP_DELAYED_MSG:
                handleDelayedCmd(sim_id);
                break;
            case OP_RESPONSE_IDLE_TEXT:
                handleIdleTextResponse(sim_id);
                // End the process.
                mStkContext[sim_id].mCmdInProgress = false;
                break;
            case OP_EVDL_CALL_DISCONN_TIMEOUT:
                if (false == FeatureOption.MTK_BSP_PACKAGE) {
                    CatLog.d(LOGTAG, "OP_EVDL_CALL_DISCONN_TIMEOUT() No disconn intent received."); 
                    if(mStkService[sim_id] != null && true == mStkService[sim_id].isCallDisConnReceived()) {
                        SendEventDownloadMsg(EVDL_ID_CALL_DISCONNECTED, sim_id);
                    }
                }
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

    private void handleDelayedCmd(int sim_id) {
        CatLog.d(LOGTAG, "handleDelayedCmd, sim_id: " + sim_id);
        if (mStkContext[sim_id].mCmdsQ.size() != 0) {
            DelayedCmd cmd = mStkContext[sim_id].mCmdsQ.poll();
            if (cmd != null)
            {
                CatLog.d(LOGTAG, "handleDelayedCmd - queue size: " + mStkContext[sim_id].mCmdsQ.size() + " id: " + cmd.id + "sim id: " + cmd.sim_id);
                switch (cmd.id) {
                case OP_CMD:
                    handleCmd(cmd.msg, cmd.sim_id);
                    break;
                case OP_END_SESSION:
                    handleSessionEnd(cmd.sim_id);
                    break;
                }
            }
        }
    }

    private void callDelayedMsg(int sim_id) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_DELAYED_MSG;
        msg.arg2 = sim_id;
        mServiceHandler.sendMessage(msg);
    }

    private void handleSessionEnd(int sim_id) {
        mStkContext[sim_id].mCurrentCmd = mStkContext[sim_id].mMainCmd;
        CatLog.d(LOGTAG, "handleSessionEnd - mCurrentCmd changed to mMainCmd!");
        mStkContext[sim_id].mCurrentMenuCmd = mStkContext[sim_id].mMainCmd;
        if (mStkContext[sim_id].mMainCmd == null){
            CatLog.d(LOGTAG, "[handleSessionEnd][mCurrentCmd is null!]");
        }
        mStkContext[sim_id].lastSelectedItem = null;
        // In case of SET UP MENU command which removed the app, don't
        // update the current menu member.
        if (mStkContext[sim_id].mCurrentMenu != null && mStkContext[sim_id].mMainCmd != null) {
            mStkContext[sim_id].mCurrentMenu = mStkContext[sim_id].mMainCmd.getMenu();
        }
        if (mStkContext[sim_id].mMenuIsVisible) {
            if(mStkContext[sim_id].mSetupMenuCalled == true) {
                launchMenuActivity(null, sim_id);
            } else {
                CatLog.d(LOGTAG, "[handleSessionEnd][To finish menu activity]");
                finishMenuActivity(sim_id);
            }
        }
        if (mStkContext[sim_id].mCmdsQ.size() != 0) {
            callDelayedMsg(sim_id);
        } else {
            mStkContext[sim_id].mCmdInProgress = false;
        }
        // In case a launch browser command was just confirmed, launch that url.
        if (mStkContext[sim_id].launchBrowser) {
            mStkContext[sim_id].launchBrowser = false;
            launchBrowser(mStkContext[sim_id].mBrowserSettings);
        }
    }

    private void handleCmd(CatCmdMessage cmdMsg, int sim_id) {
        StkAppInstaller appInstaller = StkAppInstaller.getInstance();
        if (cmdMsg == null) {
            return;
        }
        // save local reference for state tracking.
        mStkContext[sim_id].mCurrentCmd = cmdMsg;
        boolean waitForUsersResponse = true;
        byte[] additionalInfo = null;

        if(cmdMsg.getCmdType() != null) {
            CatLog.d(LOGTAG, "handleCmd cmdName[" + cmdMsg.getCmdType().name() + "]  mCurrentCmd=cmdMsg");
        }
        
        switch (cmdMsg.getCmdType()) {
        case DISPLAY_TEXT:
            CatLog.d(LOGTAG, "[handleCmd][DISPLAY_TEXT] +");
            
            if(isBusyOnCall() == true) {
                CatLog.d(LOGTAG, "[Handle Command][DISPLAY_TEXT][Can not handle currently]");
                CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                additionalInfo = new byte[1];
                additionalInfo[0] = (byte)0x02;
                resMsg.setAdditionalInfo(additionalInfo);
                mStkService[sim_id].onCmdResponse(resMsg);
                return;
            }
            
            TextMessage msg = cmdMsg.geTextMessage();
            mStkContext[sim_id].responseNeeded = msg.responseNeeded;
            waitForUsersResponse = msg.responseNeeded;
            if(mStkContext[sim_id].responseNeeded == false) {
                waitForUsersResponse = false;
                //Immediate response
                CatLog.d(LOGTAG, "[Handle Command][DISPLAY_TEXT][Should immediatly response]");
                CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                  resMsg.setResultCode(ResultCode.OK);
                mStkService[sim_id].onCmdResponse(resMsg);
            }else{
         
            }
            if (mStkContext[sim_id].lastSelectedItem != null) {
                msg.title = mStkContext[sim_id].lastSelectedItem;
            } else if (mStkContext[sim_id].mMainCmd != null){
                msg.title = mStkContext[sim_id].mMainCmd.getMenu().title;
//            }else if(mstrbackDisplayTitle != null){
//              msg.title = mstrbackDisplayTitle;
            } else {
                // TODO: get the carrier name from the SIM
                msg.title = "";
            }
            
            byte[] target = {0x0d,0x0a};
            String strTarget = new String(target);
            String strLine = System.getProperty("line.separator");
            
            String strText = msg.text.replaceAll(strTarget, strLine);
            msg.text = strText;
            launchTextDialog(sim_id);
            break;
        case SELECT_ITEM:
            CatLog.d(LOGTAG, "[handleCmd][Select_Item] +");
            mStkContext[sim_id].mCurrentMenuCmd = mStkContext[sim_id].mCurrentCmd;
            mStkContext[sim_id].mCurrentMenu = cmdMsg.getMenu();
            if(isBusyOnCall() == true) {
                CatLog.d(LOGTAG, "[Handle Command][SELECT_ITEM][Can not handle currently]");
                CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                additionalInfo = new byte[1];
                additionalInfo[0] = (byte)0x02;
                resMsg.setAdditionalInfo(additionalInfo);
                mStkService[sim_id].onCmdResponse(resMsg);
                return;
            }
            if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            {
                final ITelephonyEx iTel = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
                if (iTel != null){
                    try{
                        /* TODO: Gemini and non-Gemini are different begine */
                        if(iTel.isRadioOn(sim_id) == true) {
                            launchMenuActivity(cmdMsg.getMenu(), sim_id);
                        } else {
                            CatLog.d(LOGTAG, "radio off, send TR directly.");
                            CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                            resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                            if(null != mStkService[sim_id])
                                mStkService[sim_id].onCmdResponse(resMsg);
                        }
                    }catch(RemoteException ex){
                            ex.getMessage();
                    }
                } else {
                    CatLog.d(LOGTAG, "ITelephonyEx is null.");
                }
            }
            else
            {
                final ITelephony iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
                if (iTel != null){
                    try{
                        if (iTel.isRadioOn() == true){
                            launchMenuActivity(cmdMsg.getMenu(), sim_id);
                        } else {
                            CatLog.d(LOGTAG, "single - radio off, send TR directly.");
                            CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                            resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                            if(null != mStkService[sim_id])
                                mStkService[sim_id].onCmdResponse(resMsg);
                        }
                    }catch(RemoteException ex){
                            ex.getMessage();
                    }
                } else {
                    CatLog.d(LOGTAG, "ITelephony is null.");
                }
            }
            break;
        case SET_UP_MENU:
            CatLog.d(LOGTAG, "[handleCmd][SET_UP_MENU] +, from modem: " + cmdMsg.getMenu().getSetUpMenuFlag());
            if (cmdMsg.getMenu().getSetUpMenuFlag() == 1)
            {
                CatLog.d(LOGTAG, "Got SET_UP_MENU from mode");
                if (mStkContext[sim_id].mCmdsQ.size() != 0) {
                    CatLog.d(LOGTAG, "Command queue size is not 0 so to remove all items in the queue, size: " + mStkContext[sim_id].mCmdsQ.size());
                    mStkContext[sim_id].mCmdsQ.clear();
                }
                mStkContext[sim_id].mCmdInProgress = false;
            }
            mStkContext[sim_id].mSetupMenuCalled = true;
            mStkContext[sim_id].mMainCmd = mStkContext[sim_id].mCurrentCmd;
            mStkContext[sim_id].mCurrentMenuCmd = mStkContext[sim_id].mCurrentCmd;
            mStkContext[sim_id].mCurrentMenu = cmdMsg.getMenu();
            CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU [" + removeMenu(sim_id) + "]");

            boolean radio_on = true;
            /* TODO: Gemini and non-Gemini are different begine */
            if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
                try {
                    ITelephonyEx phone = ITelephonyEx.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICEEX));
                    if (phone != null) { 
                        /*For OP02 spec v4.1 start*/
                        String optr = SystemProperties.get("ro.operator.optr");
                        if (optr != null && "OP02".equals(optr)) {   
                            radio_on = checkSimRadioState(mContext, sim_id);
                        }/*For OP02 spec v4.1 end*/
                        else 
                        {
                            radio_on = phone.isRadioOn(sim_id);
                        }
                        CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU radio_on[" + radio_on + "]");
                    }            
                } catch (RemoteException e) {
                    e.printStackTrace();
                    CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU Exception happen ====");
                }
            }
            /* TODO: Gemini and non-Gemini are different end */
            if (removeMenu(sim_id)) {
                CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - removeMenu() - Uninstall App");
                mStkContext[sim_id].mCurrentMenu = null;
                mStkContext[sim_id].mSetupMenuCalled = false;
                appInstaller.unInstall(mContext, sim_id);
                StkAvailable(sim_id, STK_AVAIL_NOT_AVAILABLE);
            } else if (!radio_on) {
                CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - install App - radio_on[" + radio_on +"]");
                appInstaller.unInstall(mContext, sim_id);
                StkAvailable(sim_id, STK_AVAIL_NOT_AVAILABLE);
            } else {
                CatLog.d(LOGTAG, "StkAppService - SET_UP_MENU - install App");
                appInstaller.install(mContext, sim_id);
                StkAvailable(sim_id, STK_AVAIL_AVAILABLE);
            }
            
            if (mStkContext[sim_id].mMenuIsVisible) {
                launchMenuActivity(null, sim_id);
            }
            // MTK_OP03_PROTECT_START
            if (sim_id == PhoneConstants.GEMINI_SIM_1)
            {
                CatOpAppInterfaceImp imp = new CatOpAppInterfaceImp((CatService)mStkService[sim_id]);
                if (imp != null && mStkContext[sim_id].mCurrentMenu != null)
                {
                    imp.updateMenuTitleFromEf(mStkContext[sim_id].mCurrentMenu.title);
                }
            }
            // MTK_OP03_PROTECT_END
            break;
        case GET_INPUT:
        case GET_INKEY:
            if(isBusyOnCall() == true) {
                CatLog.d(LOGTAG, "[Handle Command][GET_INPUT][Can not handle currently]");
                CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                additionalInfo = new byte[1];
                additionalInfo[0] = (byte)0x02;
                resMsg.setAdditionalInfo(additionalInfo);
                mStkService[sim_id].onCmdResponse(resMsg);
                return;
            }
            launchInputActivity(sim_id);
            break;
        case SET_UP_IDLE_MODE_TEXT:
            waitForUsersResponse = false;
            launchIdleText(sim_id);
            break;
        case SEND_DTMF:
        case SEND_SMS:
        case SEND_SS:
        case SEND_USSD:
            waitForUsersResponse = false;
            launchEventMessage(sim_id);
            break;
        case LAUNCH_BROWSER:
            CatLog.d(LOGTAG, "[Handle Command][LAUNCH_BROWSER]");
            mStkContext[sim_id].mBrowserSettings = mStkContext[sim_id].mCurrentCmd.getBrowserSettings();
            if ((mStkContext[sim_id].mBrowserSettings!=null)                
                && (isBrowserLaunched(getApplicationContext()) == true)) {
                switch(mStkContext[sim_id].mBrowserSettings.mode){
                    case LAUNCH_IF_NOT_ALREADY_LAUNCHED : 
                        CatLog.d(LOGTAG, "[Handle Command][LAUNCH_BROWSER][Should not launch browser]");
                        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
                        mStkContext[sim_id].launchBrowser = false;
                        resMsg.setResultCode(ResultCode.LAUNCH_BROWSER_ERROR);
                        mStkService[sim_id].onCmdResponse(resMsg);
                        break;
                    default:
                        launchConfirmationDialog(mStkContext[sim_id].mCurrentCmd.geTextMessage(), sim_id);
                        break;
                }
            
            }else{
                launchConfirmationDialog(mStkContext[sim_id].mCurrentCmd.geTextMessage(), sim_id);
            }           
            break;
        case SET_UP_CALL:
            processSetupCall(sim_id);
            break;
        case PLAY_TONE:
            launchToneDialog(sim_id);
            break;

        // TODO: 6573 supported
        case RUN_AT_COMMAND:
            break;
            
        case OPEN_CHANNEL:
            processOpenChannel(sim_id);
            break;
            
        case CLOSE_CHANNEL:
        case RECEIVE_DATA:
        case SEND_DATA:
        case GET_CHANNEL_STATUS:
            waitForUsersResponse = false;
            launchEventMessage(sim_id);
            break;
        }

        if (!waitForUsersResponse) {
            if (mStkContext[sim_id].mCmdsQ.size() != 0) {
                callDelayedMsg(sim_id);
            } else {
                mStkContext[sim_id].mCmdInProgress = false;
            }
        }
    }
    
    private void displayAlphaIcon(TextMessage msg, int sim_id) {
        
        if(msg == null)
        {
            CatLog.d(LOGTAG, "[displayAlphaIcon] msg is null");
            return;
        }

        CatLog.d(LOGTAG, "launchAlphaIcon - IconSelfExplanatory[" + msg.iconSelfExplanatory + "]"
                                          + "icon[" + msg.icon + "]" 
                                          + "text[" + msg.text + "]" );
        TextMessage dispTxt = msg;
        correctTextMessage(dispTxt, sim_id);
        if (msg.iconSelfExplanatory == true) {
            // only display Icon.
            if(msg.icon != null) {
                showIconToast(msg);
            } else {
                // do nothing.
                CatLog.d(LOGTAG, "launchAlphaIcon - null icon!");
                return; 
            }
        } else {
            // show text & icon.
            if(msg.icon != null) {
                if (msg.text == null || msg.text.length() == 0) {
                    // show Icon only.
                    showIconToast(msg);
                }        
                else {
                    showIconAndTextToast(msg);
                }
            } else {
                if (msg.text == null || msg.text.length() == 0) {
                    // do nothing
                    CatLog.d(LOGTAG, "launchAlphaIcon - null txt!");
                    return;
                } else {
                    showTextToast(msg, sim_id);
                }
            }
        }
    }
    
    private void processOpenChannel(int sim_id) {
        CatLog.d(LOGTAG, "processOpenChannel()+ " + sim_id);

        Call.State callState = Call.State.IDLE;
        TextMessage txtMsg = mStkContext[sim_id].mCurrentCmd.geTextMessage();
        ServiceState ss = null;
        
        if(FeatureOption.MTK_GEMINI_SUPPORT == true) {            
            ss = ((GeminiPhone)mPhone).getPhonebyId(sim_id).getServiceState();
        } else {
            ss = mPhone.getServiceState();
        }

        if(ss.getNetworkType() <= TelephonyManager.NETWORK_TYPE_EDGE)        
            callState = getCallState(sim_id);
        
        switch(callState) {
        case IDLE:
        case DISCONNECTED:
            if ((null != txtMsg.text) && (0 != txtMsg.text.length())) {
                /* Alpha identifier with data object */
                launchConfirmationDialog(txtMsg, sim_id);               
            } else {
                 /* Alpha identifier with null data object 
                Chap 6.4.27.1 ME should not give any information to the user or ask for user confirmation */
                processNormalOpenChannelResponse(sim_id);
            }
            break;
            
        default:
            CatLog.d(LOGTAG, "processOpenChannel() Abnormal OpenChannel Response");
            processAbnormalOpenChannelResponse(sim_id);
            break;
        }

        CatLog.d(LOGTAG, "processOpenChannel()-");
    }

    
    private void processOpenChannelResponse(int sim_id) {
        CatLog.d(LOGTAG, "processOpenChannelResponse()+ " + sim_id);
        int iChannelType = 0;
        BearerDesc iBearerDesc = mStkContext[sim_id].mCurrentCmd.getBearerDesc();
        if(iBearerDesc == null) {
            iChannelType = 2;
        }else{
            iChannelType = iBearerDesc.bearerType;
        }
        switch (iChannelType) {
            case 1: /* Open Channel related to CS Bearer */
                processNormalOpenChannelResponse(sim_id);
                break;
                
            case 2: /* Open Channel related to packet data service Bearer */
                processNormalOpenChannelResponse(sim_id);
                break;
                
            case 3: /* Open Channel related to local Bearer */
                processNormalOpenChannelResponse(sim_id);
                break;
                
            case 4: /* Open Channel related to default(Network) Bearer */
                processNormalOpenChannelResponse(sim_id);
                break;
                
            case 5: /* Open Channel related to UICC Server Mode */
                processNormalOpenChannelResponse(sim_id);
                break;
                
            default: /* Error! */
                CatLog.d(LOGTAG, "processOpenChannelResponse() Error channel type[" + iChannelType + "]");
                processAbnormalOpenChannelResponse(sim_id);  // TODO: To check 
                break;
        }
        CatLog.d(LOGTAG, "processOpenChannelResponse()-");

    }    

    private void processNormalResponse(int sim_id) {
        CatLog.d(LOGTAG, "Normal Response PROCESS Start, sim id: " + sim_id);
        mStkContext[sim_id].mCmdInProgress = false;
        if(mStkContext[sim_id].mSetupCallInProcess == false) {
            return;
        }
        mStkContext[sim_id].mSetupCallInProcess = false;
        if(mStkContext[sim_id].mCurrentCmd == null) {
            CatLog.d(LOGTAG, "Normal Response PROCESS mCurrentCmd changed to null!");
            return;
        }
        if(mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Normal Response PROCESS end! cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }       
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(true);
        launchCallMsg(sim_id);
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void processAbnormalResponse(int sim_id) {
        mStkContext[sim_id].mCmdInProgress = false;
        CatLog.d(LOGTAG, "Abnormal Response PROCESS Start");
        if(mStkContext[sim_id].mSetupCallInProcess == false) {
            return;
        }
        mStkContext[sim_id].mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal Response PROCESS");
        if(mStkContext[sim_id].mCurrentCmd == null) {
            return;
        }
        if(mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Abnormal Response PROCESS end! cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }       
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.NETWORK_CRNTLY_UNABLE_TO_PROCESS);
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void processAbnormalPhone2BusyResponse(int sim_id) {
        mStkContext[sim_id].mCmdInProgress = false;
        mStkContext[sim_id].mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS - SIM 2 Call Busy");
        if(mStkContext[sim_id].mCurrentCmd == null) {
            return;
        }
        if(mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS end - SIM 2 Call Busy! cmdName[" 
                + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }
        
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(false);
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void processAbnormalNoCallResponse(int sim_id) {
        mStkContext[sim_id].mCmdInProgress = false;
        if(mStkContext[sim_id].mSetupCallInProcess == false) {
            return;
        }
        mStkContext[sim_id].mSetupCallInProcess = false;
        CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS");
        if(mStkContext[sim_id].mCurrentCmd == null) {
            return;
        }
        if(mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Abnormal No Call Response PROCESS end! cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void processNormalOpenChannelResponse(int sim_id) {
        CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS Start");
        
        mStkContext[sim_id].mCmdInProgress = false;
        if(mStkContext[sim_id].mCurrentCmd == null) {
            CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS mCurrentCmd changed to null!");
            return;
        }

        TextMessage txtMsg = mStkContext[sim_id].mCurrentCmd.geTextMessage();
        if (mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Normal OpenChannel Response PROCESS end! cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }        
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(true);
        displayAlphaIcon(txtMsg, sim_id);
        mStkService[sim_id].onCmdResponse(resMsg);
    }

    private void processAbnormalOpenChannelResponse(int sim_id) {
        mStkContext[sim_id].mCmdInProgress = false;
        CatLog.d(LOGTAG, "Abnormal OpenChannel Response PROCESS");
        if(mStkContext[sim_id].mCurrentCmd == null) {
            return;
        }       
        if (mStkContext[sim_id].mCurrentCmd.getCmdType() != null){
            CatLog.d(LOGTAG, "Abnormal OpenChannel Response PROCESS end! cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }        
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void processNormalEndCallResponse(int sim_id) {
        CatLog.d(LOGTAG, "END CALL PROCESS");
        processNormalResponse(sim_id);
    }
    
    private void processNormalHoldCallResponse(int sim_id) {
        CatLog.d(LOGTAG, "HOLD CALL PROCESS");
        processNormalResponse(sim_id);
    }
    
    private void processAbnormalEndCallResponse(int sim_id) {
       CatLog.d(LOGTAG, "End Abnormal CALL PROCESS");
        processAbnormalResponse(sim_id);
    }
    
    private void processAbnormalHoldCallResponse(int sim_id) {
        CatLog.d(LOGTAG, "HOLD Abnormal CALL PROCESS");
        processAbnormalResponse(sim_id);
    }
    
    private boolean isReadyToCallConnected(Call.State state){
        boolean ret = false;
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            switch(state) {
                case IDLE:
                case DIALING:
                case ALERTING:
                case INCOMING:
                case WAITING:
                    ret = true;
                    break;
                default:
                    ret = false;
                    break;
            }
        }
        return ret;
    }
    private void processPhoneStateChanged(int sim_id) {
        CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: " + sim_id);
        /* TODO: Gemini and non-Gemini are different begine */
        Call fg;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            fg = ((GeminiPhone)mPhone).getPhonebyId(sim_id).getForegroundCall();
        else
            fg = mPhone.getForegroundCall();
        
        Call.State state = null;
        /* TODO: Gemini and non-Gemini are different end */
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            Call.State tmpPreCallState = null;
            
            if(PhoneConstants.GEMINI_SIM_1 == sim_id)
                tmpPreCallState = mPreCallState;
            else if(PhoneConstants.GEMINI_SIM_2 == sim_id)
                tmpPreCallState = mPreCallState2;
            else if(PhoneConstants.GEMINI_SIM_3 == sim_id)
                tmpPreCallState = mPreCallState3;
            else
                tmpPreCallState = mPreCallState4;
            
            if(fg != null) {
                state = fg.getState();
                CatLog.d(LOGTAG, "processPhoneStateChanged state -> "+state);            
                if(Call.State.ACTIVE == state && true == isReadyToCallConnected(tmpPreCallState)) {
                    CatLog.d(LOGTAG, "IDLE -> ACTIVE");
                    SendEventDownloadMsg(EVDL_ID_CALL_CONNECTED, sim_id);                
                }
                if(PhoneConstants.GEMINI_SIM_1 == sim_id)
                    mPreCallState = state;
                else if(PhoneConstants.GEMINI_SIM_2 == sim_id)
                    mPreCallState2 = state;
                else if(PhoneConstants.GEMINI_SIM_3 == sim_id)
                    mPreCallState3 = state;
                else
                    mPreCallState4 = state;                
            }
        }
        if(mStkContext[sim_id].mSetupCallInProcess == false) {
            CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: setup in process is false");
            return;
        }
        CatLog.d(LOGTAG, " PHONE_STATE_CHANGED: setup in process is true");
        // Setup call In Process.
        if(mStkContext[sim_id].mCurrentCmd != null ) {
            // Set up call
            switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) {
            case SET_UP_CALL: 
                int cmdQualifier = mStkContext[sim_id].mCurrentCmd.getCmdQualifier();
                // Call fg = mPhone.getForegroundCall();
                if(fg != null) {
                    state = fg.getState();
                    CatLog.d(LOGTAG, " PHONE_STATE_CHANGED to : " + state);
                    switch(state) {
                    case HOLDING:
                        if(cmdQualifier == SETUP_CALL_HOLD_CALL_1 || cmdQualifier == SETUP_CALL_HOLD_CALL_2) {
                            processNormalHoldCallResponse(sim_id);  
                        }
                        break;
                    case IDLE:
                        if(cmdQualifier == SETUP_CALL_HOLD_CALL_1 || 
                                cmdQualifier == SETUP_CALL_HOLD_CALL_2) {
                            // need process "end call" when hold
                            processNormalHoldCallResponse(sim_id);  
                        }
                        break;
                    }
                }
                PhoneConstants.State phoneState = null;
                if(null != mPhone) {
                    if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
                        phoneState = ((GeminiPhone)mPhone).getState();
                    } else {
                        phoneState = mPhone.getState();
                    }
                }
                CatLog.d(this, "phone state: " + phoneState);
                if(PhoneConstants.State.IDLE == phoneState) {
                    if (cmdQualifier == SETUP_CALL_END_CALL_1 || 
                        cmdQualifier == SETUP_CALL_END_CALL_2) {
                            processNormalEndCallResponse(sim_id);
                    }
                }
                break; 
            }
        }
        return;
    }
    
    private void processSuppServiceFailed(AsyncResult r, int sim_id) {
        Phone.SuppService service = (Phone.SuppService) r.result;
        CatLog.d(LOGTAG, "onSuppServiceFailed: " + service + ", sim id: " + sim_id);

        int errorMessageResId;
        switch (service) {
            case SWITCH:
                // Attempt to switch foreground and background/incoming calls failed
                // ("Failed to switch calls")
                CatLog.d(LOGTAG, "Switch failed");
                processAbnormalHoldCallResponse(sim_id);
                break;
        }
    }
    private Handler mCallDisConnHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (false == FeatureOption.MTK_BSP_PACKAGE) {
                int sim_id = 0;        
                CatLog.d(LOGTAG, "mCallDisConnHandler");                
                switch (msg.what) {
                    case PHONE_DISCONNECT:
                        sim_id = PhoneConstants.GEMINI_SIM_1;
                        break;
                    case PHONE_DISCONNECT2:
                        sim_id = PhoneConstants.GEMINI_SIM_2;
                        break;
                    case PHONE_DISCONNECT3:
                        sim_id = PhoneConstants.GEMINI_SIM_3;
                        break;                    
                    case PHONE_DISCONNECT4:
                        sim_id = PhoneConstants.GEMINI_SIM_4;
                        break;                    
                }        
                CatLog.d(LOGTAG, "Send OP_EVDL_CALL_DISCONN_TIMEOUT:"+msg.what);                
                //Send delay message 8 seconds to wait UI.
                Message msg1 = mServiceHandler.obtainMessage(OP_EVDL_CALL_DISCONN_TIMEOUT);
                msg1.arg1 = OP_EVDL_CALL_DISCONN_TIMEOUT;
                msg1.arg2 = sim_id;
                mServiceHandler.sendMessageDelayed(msg1,AP_EVDL_TIMEOUT);
            }
        }
    };   
    
    private Handler mCallHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case PHONE_STATE_CHANGED:
                processPhoneStateChanged(PhoneConstants.GEMINI_SIM_1);
                break;
            case SUPP_SERVICE_FAILED:
                processSuppServiceFailed((AsyncResult) msg.obj, PhoneConstants.GEMINI_SIM_1);
                break;
            }
        }
    };

    private Handler mCallHandler2 = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case PHONE_STATE_CHANGED:
                processPhoneStateChanged(PhoneConstants.GEMINI_SIM_2);
                break;
            case SUPP_SERVICE_FAILED:
                processSuppServiceFailed((AsyncResult) msg.obj, PhoneConstants.GEMINI_SIM_2);
                break;
            }
        }
    };

    private Handler mCallHandler3 = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case PHONE_STATE_CHANGED:
                processPhoneStateChanged(PhoneConstants.GEMINI_SIM_3);
                break;
            case SUPP_SERVICE_FAILED:
                processSuppServiceFailed((AsyncResult) msg.obj, PhoneConstants.GEMINI_SIM_3);
                break;
            }
        }
    };

    private Handler mCallHandler4 = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case PHONE_STATE_CHANGED:
                processPhoneStateChanged(PhoneConstants.GEMINI_SIM_4);
                break;
            case SUPP_SERVICE_FAILED:
                processSuppServiceFailed((AsyncResult) msg.obj, PhoneConstants.GEMINI_SIM_4);
                break;
            }
        }
    };

    private Call.State getCallState(int sim_id) {
        // Call fg = mPhone.getForegroundCall();
        CatLog.d(LOGTAG, "getCallState: " + sim_id);
        /* TODO: Gemini and non-Gemini are different begine */
        Call fg;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            fg = ((GeminiPhone)mPhone).getPhonebyId(sim_id).getForegroundCall();
        else
            fg = mPhone.getForegroundCall();
        /* TODO: Gemini and non-Gemini are different end */
        if(fg != null) {
            CatLog.d(LOGTAG, "ForegroundCall State: " + fg.getState());
            return fg.getState();
        }
        return Call.State.IDLE;
    }
    
    private Call.State getBackgroundCallState(int sim_id) {
        // Call bg = mPhone.getBackgroundCall();
        CatLog.d(LOGTAG, "getBackgroundCallState: " + sim_id);
        /* TODO: Gemini and non-Gemini are different begine */
        Call bg;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            bg = ((GeminiPhone)mPhone).getPhonebyId(sim_id).getBackgroundCall();
        else
            bg = mPhone.getBackgroundCall();
        /* TODO: Gemini and non-Gemini are different end */
        if(bg != null) {
            CatLog.d(LOGTAG, "BackgroundCall State: " + bg.getState());
            return bg.getState();
        }
        return Call.State.IDLE;
    }

    private boolean is1A1H(int sim_id) {
        Call.State fgState = getCallState(sim_id);
        Call.State bgState = getBackgroundCallState(sim_id);
        if(fgState != Call.State.IDLE && bgState != Call.State.IDLE) {
            CatLog.d(LOGTAG, "1A1H");
            return true;
        }
        return false;
    }

    private Call.State isPhoneIdle(int SIMid) {
        /* TODO: Gemini and non-Gemini are different begine */
        if (SIMid == PhoneConstants.GEMINI_SIM_3 && (FeatureOption.MTK_GEMINI_3SIM_SUPPORT != true || FeatureOption.MTK_GEMINI_4SIM_SUPPORT != true))
        {
            CatLog.d(LOGTAG, "isPhoneIdle(), Does not support SIM3");
            return Call.State.IDLE;
        }
        if (SIMid == PhoneConstants.GEMINI_SIM_4 && FeatureOption.MTK_GEMINI_4SIM_SUPPORT != true)
        {
            CatLog.d(LOGTAG, "isPhoneIdle(), Does not support SIM4");
            return Call.State.IDLE;
        }
        Call fg = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            fg = ((GeminiPhone)mPhone).getPhonebyId(SIMid).getForegroundCall();
        else
            fg = mPhone.getForegroundCall();
        /* TODO: Gemini and non-Gemini are different end */
        if(fg != null ) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " ForegroundCall State: " + fg.getState());
            if ((Call.State.IDLE != fg.getState()) && (Call.State.DISCONNECTED != fg.getState())) {
                return fg.getState();
            }
        }
        /* TODO: Gemini and non-Gemini are different begine */
        Call bg = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            bg = ((GeminiPhone)mPhone).getPhonebyId(SIMid).getBackgroundCall();
        else
            bg = mPhone.getBackgroundCall();
        /* TODO: Gemini and non-Gemini are different end */
        if(bg != null ) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " BackgroundCall State: " + bg.getState());
            if (Call.State.IDLE != bg.getState() && (Call.State.DISCONNECTED != bg.getState())) {
                return bg.getState();
            }
        }
        /* TODO: Gemini and non-Gemini are different begine */
        Call ring = null;
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
            ring = ((GeminiPhone)mPhone).getPhonebyId(SIMid).getRingingCall();
        else
            ring = mPhone.getRingingCall();
        /* TODO: Gemini and non-Gemini are different end */
        if(bg != null ) {
            CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " RingCall State: " + ring.getState());
            if (Call.State.IDLE != ring.getState() && (Call.State.DISCONNECTED != ring.getState())) {
                return ring.getState();
            }
        }
        
        CatLog.d(LOGTAG, "isPhoneIdle() Phone" + SIMid + " State: " + Call.State.IDLE);
        return Call.State.IDLE;
    }

    private void processNoCall(int sim_id) {
        // get Call State.
        Call.State callState = getCallState(sim_id);
        switch(callState) {
        case IDLE:
        case DISCONNECTED:
            launchConfirmationDialog(mStkContext[sim_id].mCurrentCmd.getCallSettings().confirmMsg, sim_id);
            break;
        default:
            CatLog.d(LOGTAG, "Call Abnormal No Call Response");
            processAbnormalNoCallResponse(sim_id);
            break;
        }
    }
    
    private void processHoldCall(int sim_id) {
        // Just show the confirm dialog, and add the process when user click OK.
        if(!is1A1H(sim_id)) { 
            launchConfirmationDialog(mStkContext[sim_id].mCurrentCmd.getCallSettings().confirmMsg, sim_id);
        } else {
            CatLog.d(LOGTAG, "Call Abnormal Hold Call Response(has 1A1H calls)");
            processAbnormalNoCallResponse(sim_id);
        }
    }
    
    private void processEndCall(int sim_id) {
        // Just show the confirm dialog, and add the process when user click OK.
        launchConfirmationDialog(mStkContext[sim_id].mCurrentCmd.getCallSettings().confirmMsg, sim_id);
    }
    
    private void processSetupCall(int sim_id) {
        CatLog.d(LOGTAG, "processSetupCall, sim id: " + sim_id);
        int i = 0;
        boolean state_idle = true;
        boolean isDualTalkMode = isSupportDualTalk();
        CatLog.d(LOGTAG, "isDualTalkMode: " + isDualTalkMode);
        if (true == FeatureOption.MTK_GEMINI_SUPPORT && !isDualTalkMode)
        {
            for (i = 0; i < STK_GEMINI_SIM_NUM; i++)
            {
                if ((i != sim_id) && (Call.State.IDLE != isPhoneIdle(i)))
                {
                    state_idle = false;
                    processAbnormalPhone2BusyResponse(sim_id);
                    CatLog.d(LOGTAG, "The other sim is not idle, sim id: " + i);
                    break;
                }
            }
        }
        else
        {
            CatLog.d(LOGTAG, "This is dual talk mode");
        }
        if (state_idle) {
            // get callback.
            mStkContext[sim_id].mSetupCallInProcess = true;
            int cmdQualifier = mStkContext[sim_id].mCurrentCmd.getCmdQualifier();
            CatLog.d(LOGTAG, "Qualifier code is " + cmdQualifier);
            switch(cmdQualifier) {
            case SETUP_CALL_NO_CALL_1:
            case SETUP_CALL_NO_CALL_2:
                processNoCall(sim_id);
                break;
            case SETUP_CALL_HOLD_CALL_1:
            case SETUP_CALL_HOLD_CALL_2:
                processHoldCall(sim_id);
                break;
            case SETUP_CALL_END_CALL_1:
            case SETUP_CALL_END_CALL_2:
                processEndCall(sim_id);
                break;
            }
        }
    }
    
    private void processHoldCallResponse(int sim_id) {
        // get Call State.
        Call.State callState = getCallState(sim_id);
        CatLog.d(LOGTAG, "processHoldCallResponse callState[" + callState + "], sim id: " + sim_id);
        
        switch(callState) {
        case IDLE:
        case HOLDING:
            processNormalResponse(sim_id);
            CatLog.d(LOGTAG, "processHoldCallResponse in Idle or HOLDING");
            break;
        case ACTIVE:
            CatLog.d(LOGTAG, "processHoldCallResponse in Active ");
            try {
                CatLog.d(LOGTAG, "switchHoldingAndActive");
                // mPhone.switchHoldingAndActive();
                /* TODO: Gemini and non-Gemini are different begine */
                if (FeatureOption.MTK_GEMINI_SUPPORT == true)
                    ((GeminiPhone)mPhone).getPhonebyId(sim_id).switchHoldingAndActive();
                else
                    mPhone.switchHoldingAndActive();
                /* TODO: Gemini and non-Gemini are different end */
            } catch (CallStateException ex) {
                CatLog.d(LOGTAG, " Error: switchHoldingAndActive: caught " + ex);
                processAbnormalResponse(sim_id);
            }
            break;
        default:
            CatLog.d(LOGTAG, "processHoldCallResponse in other state");
            processAbnormalResponse(sim_id);
            break;
        }
        return;
    }

    private void processEndCallResponse(int sim_id) {
        // get Call State.
        Call.State callState = getCallState(sim_id);
        CatLog.d(LOGTAG, "call State  = " + callState + " ,sim id" + sim_id);
        switch(callState) {
        case IDLE:
            processNormalResponse(sim_id);
            break;
            // other state
        default:
            // End call
            CatLog.d(LOGTAG, "End call");
            // 1A1H call
            if(is1A1H(sim_id)) {
                try {
                    // mPhone.hangupAll();
                    /* TODO: Gemini and non-Gemini are different begine */
                    if (FeatureOption.MTK_GEMINI_SUPPORT == true)
                        ((GeminiPhone)mPhone).getPhonebyId(sim_id).hangupAll();
                    else
                        mPhone.hangupAll();
                    /* TODO: Gemini and non-Gemini are different end */
                } catch (Exception ex) {
                    CatLog.d(LOGTAG, " Error: Call hangup: caught " + ex);
                    processAbnormalResponse(sim_id);
                }
            } else {
                // Call fg = mPhone.getForegroundCall();
                /* TODO: Gemini and non-Gemini are different begine */
                Call fg = null;
                if (FeatureOption.MTK_GEMINI_SUPPORT == true)
                    fg = ((GeminiPhone)mPhone).getPhonebyId(sim_id).getForegroundCall();
                else
                    fg = mPhone.getForegroundCall();
                /* TODO: Gemini and non-Gemini are different end */
                if(fg != null) {
                    try {
                        CatLog.d(LOGTAG, "End call  " + callState);
                        fg.hangup();
                    } catch (CallStateException ex){
                        CatLog.d(LOGTAG, " Error: Call hangup: caught " + ex);
                        // TODO
                        processAbnormalResponse(sim_id);
                    }
                }
            }
            CatLog.d(LOGTAG, "call Not IDLE  = " + callState);
            break;
        }
    }

    private void processSetupCallResponse(int sim_id) {
        CatLog.d(LOGTAG, "processSetupCallResponse(), sim id: " + sim_id);
        int cmdQualifier = mStkContext[sim_id].mCurrentCmd.getCmdQualifier();
        CatLog.d(LOGTAG, "processSetupCallResponse() - cmdQualifier[" + cmdQualifier + "]");
        
        switch (cmdQualifier) {
        case SETUP_CALL_NO_CALL_1:
        case SETUP_CALL_NO_CALL_2:
            //TODO
            processNormalResponse(sim_id);
            break;
        case SETUP_CALL_HOLD_CALL_1:
        case SETUP_CALL_HOLD_CALL_2:
            processHoldCallResponse(sim_id);
            break;
        case SETUP_CALL_END_CALL_1:
        case SETUP_CALL_END_CALL_2:
            processEndCallResponse(sim_id);
            break;
        }
    }
    // End Setup Call

    private void handleEventDownload(Bundle args, int sim_id) {     
        int eventId = args.getInt(EVDL_ID);
        int sourceId = 0;
        int destinationId = 0;
        byte[] additionalInfo = null;
        byte[] language;
        boolean oneShot = false;
        String languageInfo;
        
        CatResponseMessage resMsg = new CatResponseMessage(eventId);
        switch(eventId) {
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
            //language tag
            additionalInfo[0] = (byte) 0xAD;
            //language code, defined in ISO639,coded in GSM 7-bit ex. Emglish -> en -> 0x65 0x6E
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
            //browser termination cause tag
            additionalInfo = new byte[3];
            additionalInfo[0] = (byte) 0xB4;
            additionalInfo[1] = 0x01;
            additionalInfo[2] = 0x00;
            oneShot = false;
            break; 
        case EVDL_ID_CALL_DISCONNECTED:
            oneShot = false;
            break;
        default:
            break;
        }
        resMsg.setSourceId(sourceId);
        resMsg.setDestinationId(destinationId);
        resMsg.setAdditionalInfo(additionalInfo);
        resMsg.setOneShot(oneShot);
        CatLog.d(LOGTAG, "onEventDownload - eventId[" + eventId + "], sim id: " + sim_id);
        if (sim_id >= 0 && sim_id < STK_GEMINI_SIM_NUM) {
            try {
                mStkService[sim_id].onEventDownload(resMsg);
            } catch (NullPointerException e) {
                CatLog.d(LOGTAG, "mStkService is null, sim: " + sim_id);
            }
        }
        else if (sim_id == STK_GEMINI_BROADCAST_ALL)
        {
            int i = 0;
            for (i = 0; i < STK_GEMINI_SIM_NUM; i++)
            {
                if (mStkService[i] != null)
                    mStkService[i].onEventDownload(resMsg);
            }
        }
    }
    private void handleCmdResponse(Bundle args, int sim_id) {
        CatLog.d(LOGTAG, "handleCmdResponse, sim id: " + sim_id);
        if (mStkContext[sim_id].mCurrentCmd == null) {
            return;
        }

        if (mStkService[sim_id] == null) {
            mStkService[sim_id] = com.android.internal.telephony.cat.CatService.getInstance(sim_id);
            if (mStkService[sim_id] == null) {
                // This should never happen (we should be responding only to a message
                // that arrived from StkService). It has to exist by this time
                CatLog.d(LOGTAG, "handleCmdResponse exception! mStkService is null when we need to send response.");
                throw new RuntimeException("mStkService is null when we need to send response");
            }
        }

        boolean skip_timeout = false;
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        if (null != mStkContext[sim_id].mCurrentCmd && null != mStkContext[sim_id].mCurrentCmd.getCmdType()) {
            CatLog.d(LOGTAG, "handleCmdResponse+ cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
            if (mStkContext[sim_id].mCurrentCmd.getCmdType() == AppInterface.CommandType.SEND_DATA ||
                mStkContext[sim_id].mCurrentCmd.getCmdType() == AppInterface.CommandType.RECEIVE_DATA ||
                mStkContext[sim_id].mCurrentCmd.getCmdType() == AppInterface.CommandType.CLOSE_CHANNEL ||
                mStkContext[sim_id].mCurrentCmd.getCmdType() == AppInterface.CommandType.SET_UP_MENU)
            {
                skip_timeout = true;
            }
        }

        // set result code
        boolean helpRequired = args.getBoolean(HELP, false);

        switch(args.getInt(RES_ID)) {
        case RES_ID_MENU_SELECTION:
            CatLog.d(LOGTAG, "RES_ID_MENU_SELECTION");
            if(isBipCommand(mStkContext[sim_id].mCurrentCmd)) {
                Toast toast = Toast.makeText(mContext.getApplicationContext(), R.string.lable_busy_on_bip, Toast.LENGTH_LONG);
                toast.setGravity(Gravity.BOTTOM, 0, 0);
                toast.show();
                return;
            }
            
            int menuSelection = args.getInt(MENU_SELECTION);
            switch(mStkContext[sim_id].mCurrentMenuCmd.getCmdType()) {
            case SET_UP_MENU:
                //have already handled setup menu
                mStkContext[sim_id].mSetUpMenuHandled = true;
            case SELECT_ITEM:
                resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentMenuCmd);
                mStkContext[sim_id].lastSelectedItem = getItemName(menuSelection, sim_id);
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
            if (input != null && (null != mStkContext[sim_id].mCurrentCmd.geInput())&&(mStkContext[sim_id].mCurrentCmd.geInput().yesNo)) {
                boolean yesNoSelection = input
                        .equals(StkInputInstance.YES_STR_RESPONSE);
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
            switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) 
            {
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
                    mStkContext[sim_id].launchBrowser = true;
                    mStkContext[sim_id].mBrowserSettings = mStkContext[sim_id].mCurrentCmd.getBrowserSettings();
                }
                break;
            case SET_UP_CALL:
                if(confirmed) {
                    processSetupCallResponse(sim_id);
                    return;
                }
                // Cancel
                mStkContext[sim_id].mSetupCallInProcess = false;
                resMsg.setResultCode(ResultCode.OK);
                resMsg.setConfirmation(confirmed);
                break;
                
            case OPEN_CHANNEL:
                if(confirmed) {
                    processOpenChannelResponse(sim_id);
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
            switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) {
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
            CatLog.d(LOGTAG, "RES_ID_TIMEOUT, skip timout: " + skip_timeout);
            if (!skip_timeout)
            {
                resMsg.setResultCode(ResultCode.NO_RESPONSE_FROM_USER);
                // GCF test-case 27.22.4.1.1 Expected Sequence 1.5 (DISPLAY TEXT,
                // Clear message after delay, successful) expects result code OK.
                // If the command qualifier specifies no user response is required
                // then send OK instead of NO_RESPONSE_FROM_USER
                if ((mStkContext[sim_id].mCurrentCmd.getCmdType().value() == AppInterface.CommandType.DISPLAY_TEXT
                        .value())
                        && (mStkContext[sim_id].mCurrentCmd.geTextMessage().responseNeeded == true)) {
                    if (mStkContext[sim_id].mCurrentCmd.geTextMessage().userClear == false) {
                        resMsg.setResultCode(ResultCode.OK);
                    }
                }
            }
            else
            {
                CatLog.d(LOGTAG, "Skip timeout because the command is SEND_DATA");
            }
            break;
        default:
            CatLog.d(LOGTAG, "Unknown result id");
            return;
        }
        
        if (null != mStkContext[sim_id].mCurrentCmd && null != mStkContext[sim_id].mCurrentCmd.getCmdType()) {
            CatLog.d(LOGTAG, "handleCmdResponse- cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        }
        mStkService[sim_id].onCmdResponse(resMsg);
    }

    /**
     * Returns 0 or FLAG_ACTIVITY_NO_USER_ACTION, 0 means the user initiated the action.
     *
     * @param userAction If the userAction is yes then we always return 0 otherwise
     * mMenuIsVisible is used to determine what to return. If mMenuIsVisible is true
     * then we are the foreground app and we'll return 0 as from our perspective a
     * user action did cause. If it's false than we aren't the foreground app and
     * FLAG_ACTIVITY_NO_USER_ACTION is returned.
     *
     * @return 0 or FLAG_ACTIVITY_NO_USER_ACTION
     */
    private int getFlagActivityNoUserAction(InitiatedByUserAction userAction, int sim_id) {
        return ((userAction == InitiatedByUserAction.yes) | mStkContext[sim_id].mMenuIsVisible) ?
                                                    0 : Intent.FLAG_ACTIVITY_NO_USER_ACTION;
    }
    
    private void finishMenuActivity(int sim_id) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        /* TODO: GEMINI+ begin */
        String targetActivity = STK1_MENU_ACTIVITY_NAME;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                break;
            case PhoneConstants.GEMINI_SIM_2:
                targetActivity = STK2_MENU_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_3:
                targetActivity = STK3_MENU_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_4:
                targetActivity = STK4_MENU_ACTIVITY_NAME;
                break;
            default:
                break;
        }
        /* TODO: GEMINI+ end */
        CatLog.d(LOGTAG, "finishMenuActivity, target: " + targetActivity); 
        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TOP;

        intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id);
        newIntent.putExtra("STATE", StkMenuInstance.STATE_END); 
        newIntent.putExtra(CMD_SIM_ID, sim_id);
        newIntent.setFlags(intentFlags);
        mContext.startActivity(newIntent);
    }
    
    private void launchMenuActivity(Menu menu, int sim_id) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        /* TODO: GEMINI+ begin */
        String targetActivity = STK1_MENU_ACTIVITY_NAME;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                break;
            case PhoneConstants.GEMINI_SIM_2:
                targetActivity = STK2_MENU_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_3:
                targetActivity = STK3_MENU_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_4:
                targetActivity = STK4_MENU_ACTIVITY_NAME;
                break;
            default:
                break;
        }
        /* TODO: GEMINI+ end */
        CatLog.d(LOGTAG, "launchMenuActivity, target: " + targetActivity); 
        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_CLEAR_TOP;
        if (menu == null) {
            // We assume this was initiated by the user pressing the tool kit icon
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.yes, sim_id);

            newIntent.putExtra("STATE", StkMenuInstance.STATE_MAIN);
        } else {
            // We don't know and we'll let getFlagActivityNoUserAction decide.
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id);

            newIntent.putExtra("STATE", StkMenuInstance.STATE_SECONDARY);
        }
        newIntent.putExtra(CMD_SIM_ID, sim_id);
        newIntent.setFlags(intentFlags);
        mContext.startActivity(newIntent);
    }

    private void launchInputActivity(int sim_id) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        /* TODO: GEMINI+ begin */
        String targetActivity = STK1_INPUT_ACTIVITY_NAME;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                break;
            case PhoneConstants.GEMINI_SIM_2:
                targetActivity = STK2_INPUT_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_3:
                targetActivity = STK3_INPUT_ACTIVITY_NAME;
                break;
            case PhoneConstants.GEMINI_SIM_4:
                targetActivity = STK4_INPUT_ACTIVITY_NAME;
                break;
            default:
                break;
        }
        /* TODO: GEMINI+ end */
        CatLog.d(LOGTAG, "launchInputActivity, target: " + targetActivity);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                            | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id));
        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        newIntent.putExtra("INPUT", mStkContext[sim_id].mCurrentCmd.geInput());
        newIntent.putExtra(CMD_SIM_ID, sim_id);
        mContext.startActivity(newIntent);
    }

    private void launchTextDialog(int sim_id) {
        CatLog.d(LOGTAG, "launchTextDialog, sim id: " + sim_id);
        if(canShowTextDialog(mStkContext[sim_id].mCurrentCmd.geTextMessage(), sim_id) == false) {
            sendOkMessage(sim_id);
            return;
        }

        /* TODO: GEMINI+ begin */
        Intent newIntent = null;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                newIntent = new Intent(this, StkDialogActivity.class);
                break;
            case PhoneConstants.GEMINI_SIM_2:
                newIntent = new Intent(this, StkDialogActivityII.class);
                break;
            case PhoneConstants.GEMINI_SIM_3:
                newIntent = new Intent(this, StkDialogActivityIII.class);
                break;
            case PhoneConstants.GEMINI_SIM_4:
                newIntent = new Intent(this, StkDialogActivityIV.class);
                break;
            default:
                break;
        }
        /* TODO: GEMINI+ end */
        if (newIntent != null)
        {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP
                    | Intent.FLAG_ACTIVITY_NO_HISTORY
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id));
            TextMessage msg = mStkContext[sim_id].mCurrentCmd.geTextMessage();
            newIntent.putExtra("TEXT", mStkContext[sim_id].mCurrentCmd.geTextMessage());
            newIntent.putExtra(CMD_SIM_ID, sim_id);
            startActivity(newIntent);
        }
    }
    
    private boolean canShowTextDialog(TextMessage msg, int sim_id) {
        // can show whatever screen it is.
        if(msg == null) {
            // using normal flow.
            return true;
        }
        CatLog.d(LOGTAG, "canShowTextDialog? mMenuIsVisible = " + mStkContext[sim_id].mMenuIsVisible + " mInputIsVisible = " + " mDialogIsVisible = " + mStkContext[sim_id].mDialogIsVisible); 
        if(msg.isHighPriority == true) {
            return true;
        } else {
            // only show in idle screen.
            if(isIdleScreen(this.mContext) == true) 
            {
                return true;
            } 
            // if not in Idle Screen, but in Stk screen, will show the message.
            if(mStkContext[sim_id].mMenuIsVisible == true || mStkContext[sim_id].mInputIsVisible == true || mStkContext[sim_id].mDialogIsVisible == true) {
                return true;
            }
        }
        return false;
    }
    
    public boolean isIdleScreen(){
        final ActivityManager am = (ActivityManager)getApplicationContext().getSystemService(Context.ACTIVITY_SERVICE);
        List<RecentTaskInfo> taskInfo = am.getRecentTasks(16, ActivityManager.RECENT_WITH_EXCLUDED);

        String home = null;
                if (taskInfo != null){
            for(RecentTaskInfo task : taskInfo){
            if(true == task.baseIntent.hasCategory(Intent.CATEGORY_HOME)){
                home = task.baseIntent.getComponent().getPackageName();
                break;
            }
            }
                }

        boolean idle = false;
        List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
                if (runningAppInfo != null){
            for(RunningAppProcessInfo app : runningAppInfo){
            if(app.processName.equals(home) && app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND){
                            idle = true;
                    break;
            }
            }
                }

        CatLog.d(LOGTAG, "[isIdleScreen][idle] : " + idle);
        return idle;
    }
    
    public boolean isIdleScreen(Context context) 
    {
        String homePackage = null;
        String homeProcess = null;
        boolean idle = false;

        final ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<RecentTaskInfo> taskInfo = am.getRecentTasks(16, ActivityManager.RECENT_WITH_EXCLUDED);

        if (taskInfo != null){
            for (RecentTaskInfo task : taskInfo) {
                if ( true == task.baseIntent.hasCategory(Intent.CATEGORY_HOME) ) {
                    homePackage = task.baseIntent.getComponent().getPackageName();
                    break;
                }
            }
        }
        CatLog.d(LOGTAG, "[isIdleScreen] homePackage is: " + homePackage);
        

        try {
            ApplicationInfo appInfo = context.getPackageManager().getApplicationInfo(homePackage,0);
            homeProcess = appInfo.processName;
        } catch (NameNotFoundException e) {
            CatLog.d(LOGTAG, "[isIdleScreen] NameNotFoundException");
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        CatLog.d(LOGTAG, "home package = " + homePackage + ", home process = " + homeProcess);
        
        List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
        for (RunningAppProcessInfo app : runningAppInfo) {
            if ( app.processName.equals(homeProcess) && 
                    app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND ) {
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
            final ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
            
            boolean top = false;
            List<RunningAppProcessInfo> runningAppInfo = am.getRunningAppProcesses();
            if (runningAppInfo != null){
                for (RunningAppProcessInfo app : runningAppInfo) {
                    if ( app.processName.equals(BROWSER_PACKAGE_NAME) && (app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND) ) {
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
    private void  handleIdleTextResponse(int sim_id) {
        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[sim_id].mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        byte[] additionalInfo = new byte[1];
        additionalInfo[0] = (byte)0x01;
        resMsg.setAdditionalInfo(additionalInfo);
        CatLog.d(LOGTAG, "handleResponseOk ");
        if(null != mStkContext[sim_id].mCurrentCmd && null != mStkContext[sim_id].mCurrentCmd.getCmdType())
            CatLog.d(LOGTAG, "handleIdleTextResponse cmdName[" + mStkContext[sim_id].mCurrentCmd.getCmdType().name() + "]");
        mStkService[sim_id].onCmdResponse(resMsg);
    }
    
    private void sendOkMessage(int sim_id) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_RESPONSE_IDLE_TEXT;
        msg.arg2 = sim_id;
        mServiceHandler.sendMessage(msg);
    }
    
    private void launchEventMessage(int sim_id) {
        TextMessage msg = mStkContext[sim_id].mCurrentCmd.geTextMessage();
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
            if(msg.text == null){
                CatLog.d(LOGTAG, "aaaaa [msg.text == null] ");
                switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) {
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
            }
            else{
            tv.setText(msg.text);
        }
        }

        toast.setView(v);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private void launchConfirmationDialog(TextMessage msg, int sim_id) {
        msg.title = mStkContext[sim_id].lastSelectedItem;
        correctTextMessage(msg, sim_id);
        /* TODO: GEMINI+ begin */
        Intent newIntent = null;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                newIntent = new Intent(this, StkDialogActivity.class);
                break;
            case PhoneConstants.GEMINI_SIM_2:
                newIntent = new Intent(this, StkDialogActivityII.class);
                break;
            case PhoneConstants.GEMINI_SIM_3:
                newIntent = new Intent(this, StkDialogActivityIII.class);
                break;
            case PhoneConstants.GEMINI_SIM_4:
                newIntent = new Intent(this, StkDialogActivityIV.class);
                break;
            default:
                break;
        }
        /* TODO: GEMINI+ end */
        if (newIntent != null)
        {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_NO_HISTORY
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id));
            newIntent.putExtra("TEXT", msg);
            newIntent.putExtra(CMD_SIM_ID, sim_id);
            startActivity(newIntent);
        }
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
        if (data != null) {            
            intent = new Intent(Intent.ACTION_VIEW);            
            intent.setData(data);
        } else {
        // if the command did not contain a URL,
        // launch the browser to the default homepage.
            CatLog.d(this, "launch browser with default URL ");
            intent = Intent.makeMainSelectorActivity(Intent.ACTION_MAIN,
            Intent.CATEGORY_APP_BROWSER);
        }
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
        // a small delay, let the browser start, before processing the next command.
        // this is good for scenarios where a related DISPLAY TEXT command is
        // followed immediately.
        try {
            Thread.sleep(10000);
        } catch (InterruptedException e) {}
    }

    private void showIconToast(TextMessage msg) {
        Toast t = new Toast(this);
        ImageView v = new ImageView(this);
        v.setImageBitmap(msg.icon);           
        t.setView(v);
        t.setDuration(Toast.LENGTH_LONG);
        t.show();
    }

    private void showTextToast(TextMessage msg, int sim_id) {
        msg.title = mStkContext[sim_id].lastSelectedItem;

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

    private void launchCallMsg(int sim_id) {
        TextMessage msg = mStkContext[sim_id].mCurrentCmd.getCallSettings().callMsg;
        if (msg.iconSelfExplanatory == true) {
            // only display Icon.

            if(msg.icon != null) {
                showIconToast(msg);
            } else {
                // do nothing.
                return; 
            }
        } else {
            // show text & icon.
            if(msg.icon != null) {
                if (msg.text == null || msg.text.length() == 0) {
                    // show Icon only.
                    showIconToast(msg);
                }        
                else {
                    showIconAndTextToast(msg);
                }
            } else {
                if (msg.text == null || msg.text.length() == 0) {
                    // do nothing
                    return;
                } else {
                    showTextToast(msg, sim_id);
                }
                
            }
        }
    }

    private void launchIdleText(int sim_id) {
        TextMessage msg = mStkContext[sim_id].mCurrentCmd.geTextMessage();
        
        CatLog.d(LOGTAG, "launchIdleText - text[" + msg.text 
                         + "] iconSelfExplanatory[" + msg.iconSelfExplanatory
                         + "] icon[" + msg.icon + "], sim id: " + sim_id);
        if (msg.text == null) {
            CatLog.d(LOGTAG, "cancel IdleMode text");
            mNotificationManager.cancel(getNotificationId(sim_id));
        } else {
            CatLog.d(LOGTAG, "Add IdleMode text");
            
            mNotificationManager.cancel(getNotificationId(sim_id));
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
            // use mIdleMessage replace Intent parameter, because the extra seems do not update
            // even create a new notification with same ID.
            StkApp.mIdleMessage[sim_id] = msg.text;
//            notificationIntent.putExtra(NOTIFICATION_KEY, msg.text);
//            notificationIntent.putExtra(NOTIFICATION_TITLE, StkApp.mPLMN;
//            notification.setLatestEventInfo(this, StkApp.mPLMN, msg.text,
//                    PendingIntent.getActivity(mContext, 0, notificationIntent, 0));
            // setlatestEventInfo will create a new contentView, and replace icon with default icon,
            // so remove it and use our own contentView.
            notificationIntent.putExtra(CMD_SIM_ID, sim_id);
            contentView.setTextViewText(com.android.internal.R.id.title, StkApp.mPLMN[sim_id]);
            notification.contentView = contentView;
            notification.contentIntent = PendingIntent.getActivity(mContext, 0, notificationIntent, 0);
            mNotificationManager.notify(getNotificationId(sim_id), notification);
        }
    }

    private void launchToneDialog(int sim_id) {
        Intent newIntent = new Intent(this, ToneDialog.class);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, sim_id));
        newIntent.putExtra("TEXT", mStkContext[sim_id].mCurrentCmd.geTextMessage());
        newIntent.putExtra("TONE", mStkContext[sim_id].mCurrentCmd.getToneSettings());
        newIntent.putExtra(CMD_SIM_ID, sim_id);
        startActivity(newIntent);
    }

    private String getItemName(int itemId, int sim_id) {
        Menu menu = mStkContext[sim_id].mCurrentCmd.getMenu();
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

    private boolean removeMenu(int sim_id) {
        try {
            if (mStkContext[sim_id].mCurrentMenu.items.size() == 1 &&
                mStkContext[sim_id].mCurrentMenu.items.get(0) == null) {
                return true;
            }
        } catch (NullPointerException e) {
            CatLog.d(LOGTAG, "Unable to get Menu's items size");
            return true;
        }
        return false;
    }
    
    private void correctTextMessage(TextMessage msg, int sim_id) {
        switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) {
            case OPEN_CHANNEL:
                if(msg.text == null ) {
                    msg.text = getDefaultText(sim_id); 
                }
                break;
            default:
                if(msg.text == null || msg.text.length() == 0) {
                    msg.text = getDefaultText(sim_id); 
                }              
        }
        return;
    }
    
    private String getDefaultText(int sim_id) {
        String str = "";
        switch (mStkContext[sim_id].mCurrentCmd.getCmdType()) {
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
    
    public boolean haveEndSession(int sim_id) {
        CatLog.d(LOGTAG, "haveEndSession, query by sim id: " + sim_id);
        if(mStkContext[sim_id].mCmdsQ.size() == 0) 
            return false;
        for(int i = 0 ; i < mStkContext[sim_id].mCmdsQ.size() ; i++) {
            // if delay message involve OP_END_SESSION, return true;
            if(mStkContext[sim_id].mCmdsQ.get(i).id == OP_END_SESSION && mStkContext[sim_id].mCmdsQ.get(i).sim_id == sim_id) {
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
            int i = 0;
            for (i = 0; i < StkAppService.STK_GEMINI_SIM_NUM; i++)
            {
                SendEventDownloadMsg(evdl, i);
            }
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
                CatLog.d(LOGTAG, "mStkIdleScreenAvailableReceiver() - Received[IDLE_SCREEN_AVAILABLE]");
                evdl = EVDL_ID_IDLE_SCREEN_AVAILABLE;
            } else {
                CatLog.d(LOGTAG, "mStkIdleScreenAvailableReceiver() - Received needn't handle!");
                return;
            }
            int i = 0;
            for (i = 0; i < StkAppService.STK_GEMINI_SIM_NUM; i++)
            {
                SendEventDownloadMsg(evdl, i);
            }
        }
    }; 

    private final IntentFilter mEventDownloadCallFilter = 
        new IntentFilter("android.intent.action.stk.CALL_DISCONNECTED");

    private final BroadcastReceiver mEventDownloadCallReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (false == FeatureOption.MTK_BSP_PACKAGE) {
                final String evtAction = intent.getAction();
                int simId = PhoneConstants.GEMINI_SIM_1;
                
                CatLog.d(LOGTAG, "mEventDownloadCallReceiver() - evtAction[" + evtAction + "]");
                
                if (evtAction.equals("android.intent.action.stk.CALL_DISCONNECTED")) {
                    CatLog.d(LOGTAG, "mEventDownloadCallReceiver() - Received[CALL_DISCONNECTED]");
                    simId = intent.getIntExtra("sim_id", PhoneConstants.GEMINI_SIM_1);                
                } else {
                    CatLog.d(LOGTAG, "mEventDownloadCallReceiver() - Received needn't handle!");
                    return;
                }            
                mServiceHandler.removeMessages(OP_EVDL_CALL_DISCONN_TIMEOUT);//All calls are disconnected, remove all TIMEOUT message
                try{
                    mStkService[simId].setAllCallDisConn(true);
                    SendEventDownloadMsg(EVDL_ID_CALL_DISCONNECTED, simId);
                } catch (NullPointerException e) {
                    CatLog.d(LOGTAG, "mStkService is null, sim: " + simId);
                }
            }
        }
    };

    private void registerStkReceiver() {
        CatLog.d(LOGTAG, "registerStkReceiver()");
        //registerReceiver(mStkLocaleChangedReceiver, mLocaleChangedFilter);
        registerReceiver(mStkIdleScreenAvailableReceiver, mIdleScreenAvailableFilter);
    }

    private void unregisterStkReceiver() {
        CatLog.d(LOGTAG, "unregisterStkReceiver()");
        //unregisterReceiver(mStkLocaleChangedReceiver);
        unregisterReceiver(mStkIdleScreenAvailableReceiver);
    }

    
    private int getNotificationId(int sim_id)
    {
        int notify_id = STK1_NOTIFICATION_ID;
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_2:
                notify_id = STK2_NOTIFICATION_ID;
                break;
            case PhoneConstants.GEMINI_SIM_3:
                notify_id = STK3_NOTIFICATION_ID;
                break;
            case PhoneConstants.GEMINI_SIM_4:
                notify_id = STK4_NOTIFICATION_ID;
                break;
        }
        CatLog.d(LOGTAG, "getNotificationId, sim_id: " + sim_id + ", notify_id: " + notify_id);
        return notify_id;
    }
    private final BroadcastReceiver mSIMStateChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                int simId = intent.getIntExtra(com.android.internal.telephony.PhoneConstants.GEMINI_SIM_ID_KEY, -1);

                CatLog.d(LOGTAG, "mSIMStateChangeReceiver() - simId[" + simId + "]  state[" + simState 
                                 + "]");
                
                if (IccCardConstants.INTENT_VALUE_ICC_NOT_READY.equals(simState)) {
                    mNotificationManager.cancel(getNotificationId(simId));
                } else if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simState)) {
                    /* Remove all command in queue because SIM card was absent */
                    if (mStkContext[simId] != null && mStkContext[simId].mCmdsQ != null && mStkContext[simId].mCmdsQ.size() != 0)
                    {
                        //mStkContext[simId].mCmdsQ.clear();
                        CatLog.d(LOGTAG, "There are command in queue because SIM card was absent. size: " + mStkContext[simId].mCmdsQ.size());
                    }
                }
            } else if(intent.getAction().equals("android.intent.action.ACTION_SHUTDOWN_IPO")){
                int i;
                for (i = 0; i < STK_GEMINI_SIM_NUM; i++)
                {
                    CatLog.d(LOGTAG, "[IPO_SHUTDOWN][initial mMainCmd] : " + mStkContext[i].mMainCmd);
                    mStkContext[i].mMainCmd = null;
                    mStkContext[i].mSetUpMenuHandled = false;
                    mStkContext[i].mSetupMenuCalled = false;
                    CatLog.d(LOGTAG, "[IPO_SHUTDOWN][mMainCmd] : " + mStkContext[i].mMainCmd);
                }
            } else if (intent.getAction().equals(ACTION_REMOVE_IDLE_TEXT)) {
                int simId = intent.getIntExtra("SIM_ID", -1);
                CatLog.d(LOGTAG, "remove idle mode text by Refresh command for sim " + (simId+1));
                mNotificationManager.cancel(getNotificationId(simId));
            }
        }
    };

    private void SendEventDownloadMsg(int evdlId, int sim_id) {
        CatLog.d(LOGTAG, "SendEventDownloadMsg() - evdlId[" + evdlId + "], sim id: " + sim_id);
        Bundle args = new Bundle();
        int[] op = new int[2];
        op[0] = OP_EVENT_DOWNLOAD;
        op[1] = sim_id;
        args.putIntArray(OPCODE, op);
        args.putInt(EVDL_ID, evdlId);
        
        Message msg = mServiceHandler.obtainMessage();
        // msg.arg1 = EVDL_ID_IDLE_SCREEN_AVAILABLE;
        msg.arg1 = OP_EVENT_DOWNLOAD;
        msg.arg2 = sim_id;
        msg.obj =  args;
        
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
        PhoneConstants.State s;
        /* TODO: Gemini and non-Gemini are different begine */
        if(FeatureOption.MTK_GEMINI_SUPPORT == true) {
            s = ((GeminiPhone)mPhone).getState();
        } else {
            s = mPhone.getState();
        }
        /* TODO: Gemini and non-Gemini are different end */
        
        CatLog.d(this, "isBusyOnCall: " + s);
        return (s == PhoneConstants.State.RINGING);
    }
    
    private void changeMenuStateToMain(int sim_id) {
        CatLog.d(LOGTAG, "call changeMenuStateToMain");
        if(mStkContext[sim_id].mMainCmd == null) {
            CatLog.d(LOGTAG, "changeMenuStateToMain: mMainCmd is null");
        }
        
        mStkContext[sim_id].mCurrentCmd = mStkContext[sim_id].mMainCmd;
        mStkContext[sim_id].mCurrentMenuCmd = mStkContext[sim_id].mMainCmd;
        mStkContext[sim_id].lastSelectedItem = null;
        if(mStkContext[sim_id].mCurrentMenu != null && mStkContext[sim_id].mMainCmd != null) {
            mStkContext[sim_id].mCurrentMenu = mStkContext[sim_id].mMainCmd.getMenu();
        }
    }
    
    public void setUserAccessState(boolean state, int sim_id) {
        CatLog.d(LOGTAG, "setUserAccessState: state=" + state + ", sim id=" + sim_id);
        mStkContext[sim_id].isUserAccessed = state;
    }
    
    private void init() {
        CatLog.d(LOGTAG, "init()+ ");
        serviceThread = new Thread(null, this, "Stk App Service");
        serviceThread.start();
        mContext = getBaseContext();
        mNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        sInstance = this;
        
        mPhone = PhoneFactory.getDefaultPhone();
        if (FeatureOption.MTK_GEMINI_SUPPORT == true)
        {
            /* TODO: Gemini and non-Gemini are different begine */
            ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).registerForPreciseCallStateChanged(mCallHandler, 
                PHONE_STATE_CHANGED, null);
            ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).registerForPreciseCallStateChanged(mCallHandler2, 
                PHONE_STATE_CHANGED, null);
            ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_1).registerForSuppServiceFailed(mCallHandler, SUPP_SERVICE_FAILED, null);
            ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_2).registerForSuppServiceFailed(mCallHandler2, SUPP_SERVICE_FAILED, null);
            if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForPreciseCallStateChanged(mCallHandler3, PHONE_STATE_CHANGED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForSuppServiceFailed(mCallHandler3, SUPP_SERVICE_FAILED, null);
            }
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForPreciseCallStateChanged(mCallHandler3, PHONE_STATE_CHANGED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_3).registerForSuppServiceFailed(mCallHandler3, SUPP_SERVICE_FAILED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).registerForPreciseCallStateChanged(mCallHandler4, PHONE_STATE_CHANGED, null);
                ((GeminiPhone)mPhone).getPhonebyId(PhoneConstants.GEMINI_SIM_4).registerForSuppServiceFailed(mCallHandler4, SUPP_SERVICE_FAILED, null);
            }
            /* TODO: Gemini and non-Gemini are different end */
        }
        else
        {
            mPhone.registerForPreciseCallStateChanged(mCallHandler, PHONE_STATE_CHANGED, null);
            mPhone.registerForSuppServiceFailed(mCallHandler, SUPP_SERVICE_FAILED, null);
        }
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            registerForDisconnect(mCallManager,mCallDisConnHandler,PHONE_DISCONNECT_GEMINI);        
        }
        initNotify();
        
        IntentFilter mSIMStateChangeFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mSIMStateChangeFilter.addAction("android.intent.action.ACTION_SHUTDOWN_IPO");
        mSIMStateChangeFilter.addAction(ACTION_REMOVE_IDLE_TEXT);
        registerReceiver(mSIMStateChangeReceiver, mSIMStateChangeFilter);
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            registerReceiver(mEventDownloadCallReceiver, mEventDownloadCallFilter);
        }
    }
    
    public void sendMessageToServiceHandler(int opCode, Object obj, int sim_id) {
        CatLog.d(LOGTAG, "call sendMessageToServiceHandler: " + opCodeToString(opCode));
        if(mServiceHandler == null) {
            waitForLooper();
        }
        Message msg = mServiceHandler.obtainMessage(0, opCode, sim_id, obj);
        mServiceHandler.sendMessage(msg);
    }
    
    private String opCodeToString(int opCode) {
        switch(opCode) {
            case OP_CMD:                return "OP_CMD";
            case OP_RESPONSE:           return "OP_RESPONSE";
            case OP_LAUNCH_APP:         return "OP_LAUNCH_APP";
            case OP_END_SESSION:        return "OP_END_SESSION";
            case OP_BOOT_COMPLETED:     return "OP_BOOT_COMPLETED";
            case OP_EVENT_DOWNLOAD:     return "OP_EVENT_DOWNLOAD";
            case OP_DELAYED_MSG:        return "OP_DELAYED_MSG";
            case OP_RESPONSE_IDLE_TEXT: return "OP_RESPONSE_IDLE_TEXT";
            default:                    return "unknown op code";
        }
    }

    public void StkAvailable(int sim_id, int available)
    {
        if (mStkContext[sim_id] != null)
        {
            mStkContext[sim_id].mAvailable = available;
        }
        CatLog.d(LOGTAG, "sim_id: " + sim_id + ", available: " + available + ", StkAvailable: " + ((mStkContext[sim_id] != null)? mStkContext[sim_id].mAvailable : -1));
    }

    public int StkQueryAvailable(int sim_id)
    {
        int result = ((mStkContext[sim_id] != null)? mStkContext[sim_id].mAvailable : -1);
        
        CatLog.d(LOGTAG, "sim_id: " + sim_id + ", StkQueryAvailable: " + result);
        return result;
    }
    
    private boolean checkSimRadioState(Context context, int sim_id)
    {
        int dualSimMode = -1;
        boolean result = false;

        /* dualSimMode: 0 => both are off, 1 => SIM1 is on, 2 => SIM2 is on, 3 => both is on */
        dualSimMode = Settings.System.getInt(context.getContentResolver(), Settings.System.DUAL_SIM_MODE_SETTING, -1);

        CatLog.d(LOGTAG, "dualSimMode: " + dualSimMode + ", sim id: " + sim_id);
        switch (sim_id)
        {
            case PhoneConstants.GEMINI_SIM_1:
                if ((dualSimMode == 1) || (dualSimMode == 3))
                {
                    result = true;
                }
                break;
            case PhoneConstants.GEMINI_SIM_2:
                if ((dualSimMode == 2) || (dualSimMode == 3))
                {
                    result = true;
                }
                break;
        }
        return result;
    }
    private void registerForDisconnect(Object callManager, Handler handler, int[] whats) {  
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            if(null == callManager) {
                CatLog.d(LOGTAG, "registerForDisconnect null call manager");            
                return;
            }
            if (callManager instanceof MTKCallManager) {
                MTKCallManager mtkCm = (MTKCallManager) callManager;
    
                for (int i = 0; i < STK_GEMINI_SIM_NUM; i++) {
                    CallManager.getInstance().registerForDisconnectEx(handler, whats[i], null, GEMINI_SLOT[i]);
                }
            } else {
                ((CallManager) callManager).registerForDisconnect(handler, whats[0], null);
            }
        }
    }
    private void unregisterForDisconnect(Object callManager, Handler handler) {
        if (false == FeatureOption.MTK_BSP_PACKAGE) {
            if(null == callManager) {
                CatLog.d(LOGTAG, "unregisterForDisconnect null call manager");            
                return;
            }        
            if (callManager instanceof MTKCallManager) {
                MTKCallManager mtkCm = (MTKCallManager) callManager;
                
                for (int i = 0; i < STK_GEMINI_SIM_NUM; i++) {
                    CallManager.getInstance().unregisterForDisconnectEx(handler, GEMINI_SLOT[i]);
                }
            } else {
                ((CallManager) callManager).unregisterForDisconnect(handler);
            }
        }
    }

    private boolean isSupportDualTalk() {
        if (FeatureOption.EVDO_IR_SUPPORT == true) {
            return (SystemProperties.getInt("mediatek.evdo.mode.dualtalk", 1) == 1);
        } else {
            return FeatureOption.MTK_DT_SUPPORT;
        }
    }
}
