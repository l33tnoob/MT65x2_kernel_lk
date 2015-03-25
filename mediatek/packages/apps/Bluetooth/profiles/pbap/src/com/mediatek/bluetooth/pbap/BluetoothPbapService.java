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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.pbap;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.IBluetooth;
import android.bluetooth.IBluetoothPbap;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.ContactsContract;
import android.util.Log;

import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.util.NotificationFactory;

public class BluetoothPbapService extends Service {

    private static final String TAG = "BluetoothPbapService";

    /* Flag for debug messages */
    private static final boolean DEBUG = true;

    /* Constant data */
    public static final int PBAP_NOTIFICATION_AUTHORIZE = 1;

    public static final int PBAP_NOTIFICATION_AUTH_CHALL = 2;

    public static final String PBAP_AUTHORIZE_CANCEL_ACTION = "com.android.bluetooth.pbap.authorize.cancel";

    public static final String PBAP_AUTHORIZE_RETURN_ACTION = "com.android.bluetooth.pbap.authorize.return";

    public static final String PBAP_AUTHENTICATE_CANCEL_ACTION = "com.android.bluetooth.pbap.authenticate.cancel";

    public static final String PBAP_AUTHENTICATE_RETURN_ACTION = "com.android.bluetooth.pbap.authenticate.return";

    public static final int PBAP_AUTHORIZE_TIMEOUT = 15000;

    public static final int PBAP_AUTHENTICATE_TIMEOUT = 30000;

    /* Timeout msg for authorize and authenticate */
    public static final int PBAP_AUTH_TIMEOUT_IND = 1001;
    public static final int PBAP_USER_CANCEL_AUTH = 1002;
    /* The object is only for Auto UT*/
    private static BluetoothPbapService sPbapServiceInstenceForUT;

    public static boolean sUtState = false;

    /* keep the Bluetooth default adapter */
    BluetoothAdapter mAdapter = null;

    /* Notification manager service */
    private NotificationManager mNM = null;

    private int mPid = -1;

    private boolean mIsConnected = false;

    /* Keep the connected remote device from authorize ind received */
    BluetoothDevice mRemoteDevice = null;

    private IBluetooth mBluetoothService;

    public BluetoothPbapService() {
        // mPbapServer = new BluetoothPbapServer(mServiceHandler);
        sPbapServiceInstenceForUT = this;
    }
    
    public static BluetoothPbapService getPbapServiceInstence() {
        return sPbapServiceInstenceForUT;
    }

    /* Receive Authorize or Authenticate return */
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            printLog("onReceive(" + intent.getAction() + ")");
            String action = intent.getAction();
            long res;
            if (action.equals(PBAP_AUTHORIZE_RETURN_ACTION)) {
                boolean alwaysaccept = intent.getBooleanExtra(BluetoothServerAuthorize.AUTHORIZE_ALWAYS_ALLOWED, false);
                res = intent.getIntExtra(BluetoothServerAuthorize.AUTHORIZE_RESULT, BluetoothServerAuthorize.RESULT_OTHERS);
                printLog("PBAP_AUTHORIZE_RETURN_ACTION : result == " + res);
                if (res == BluetoothServerAuthorize.RESULT_USER_ACCEPT && mRemoteDevice != null) {
                    printLog(mRemoteDevice.toString() + " : setTrust(" + String.valueOf(alwaysaccept) + ")");
                    mRemoteDevice.setTrust(alwaysaccept);
                }
                mPbapServer.accept(res == BluetoothServerAuthorize.RESULT_USER_ACCEPT);
                /* unregister receiver */
                // unregisterReceiver(mReceiver);
            } else if (action.equals(PBAP_AUTHENTICATE_RETURN_ACTION)) {
                res = intent.getIntExtra(BluetoothAuthenticating.AUTHENTICATE_RESULT, BluetoothAuthenticating.RESULT_OTHERS);
                printLog("PBAP_AUTHENTICATE_RETURN_ACTION : result == " + res);
                if (res == BluetoothAuthenticating.RESULT_USER_ACCEPT) {
                    BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
                    String passcode = intent.getStringExtra(BluetoothAuthenticating.AUTHETICATE_CODE);
                    String userid = (adapter == null) ? null : adapter.getName();
                    // TODO: response passcode to client
                    mPbapServer.authChallRsp(false, passcode, userid);
                } else {
                    // TODO: Cancel authentication
                    mPbapServer.authChallRsp(true, null, null);
                }
                /* unregister receiver */
                // unregisterReceiver(mReceiver);
            }
            /* Cancel notification */
            cancelServerNotification();
        }
    };

    private ContentObserver mContactsChangedObserver = new ContentObserver(new Handler()) {

        public void onChange(boolean selfChange) {
            printLog("mContactsChangedObserver changed");
        }

        public void onChange(boolean selfChange, Uri uri) {
            printLog("mContactsChangedObserver changed2");
            if (mPbapServer != null) {
                mPbapServer.resetPbapListQueryData();
            }
        }
    };

    /* Handler associated with the main thread */
    private Handler mServiceHandler = new Handler() {
        public void handleMessage(Message msg) {
            printLog("[MSG] handleMessage(" + msg.what + ")");
            switch (msg.what) {
                case BluetoothPbapServer.PBAP_AUTHORIZE_IND:
                    // TODO:
                    printLog("Handling: PBAP_AUTHORIZE_IND");
                    mRemoteDevice = (BluetoothDevice) msg.obj;
                    printLog("mRemoteDevice=" + mRemoteDevice.toString());
                    printLog("getTrustState=" + String.valueOf(mRemoteDevice.getTrustState()));
                    if (mRemoteDevice == null || !mRemoteDevice.getTrustState()) {
                        setServerNotification(PBAP_NOTIFICATION_AUTHORIZE);
                        mServerState = BluetoothAdapter.STATE_CONNECTING;
                    } else {
                        mPbapServer.accept(true);
                    }
                    break;
                case BluetoothPbapServer.PBAP_AUTH_CHALL_IND:
                    // TODO:
                    printLog("Handling: PBAP_AUTH_CHALL_IND");
                    setServerNotification(PBAP_NOTIFICATION_AUTH_CHALL);
                    mServerState = BluetoothAdapter.STATE_CONNECTING;
                    break;
                case BluetoothPbapServer.PBAP_SESSION_ESTABLISHED:
                    try {
                        mBluetoothService.sendConnectionStateChange(mRemoteDevice, BluetoothProfile.ID_PBAP,
                                BluetoothAdapter.STATE_CONNECTED, BluetoothAdapter.STATE_DISCONNECTED);
                    } catch (RemoteException e) {
                        Log.e(TAG, "", e);
                    }

                    mIsConnected = true;
                    mServerState = BluetoothAdapter.STATE_CONNECTED;
                    printLog("mIsConnected changed2=" + String.valueOf(mIsConnected));    
                    // TODO:
                    printLog("Handling: PBAP_SESSION_ESTABLISHED");
                    break;
                case BluetoothPbapServer.PBAP_SESSION_DISCONNECTED:
                    try {
                        if ( mIsConnected ) {
                        mBluetoothService.sendConnectionStateChange(mRemoteDevice, BluetoothProfile.ID_PBAP,
                                BluetoothAdapter.STATE_DISCONNECTED, BluetoothAdapter.STATE_CONNECTED);
                            mIsConnected = false;
                            printLog("mIsConnected changed1=" + String.valueOf(mIsConnected));    
                            }
                    } catch (RemoteException e) {
                        Log.e(TAG, "", e);
                    }
                    // TODO:
                    mServerState = BluetoothAdapter.STATE_DISCONNECTED;
                    /* cancel notification if any exists */
                    cancelServerNotification();
                    printLog("Handling: PBAP_SESSION_DISCONNECTED");
                    break;
                case PBAP_AUTH_TIMEOUT_IND:
                case PBAP_USER_CANCEL_AUTH:
                    /* response to remote device */
                    if (mPid == NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_PBAP,
                            PBAP_NOTIFICATION_AUTHORIZE)) {
                        mPbapServer.accept(false);
                    } else if (mPid == NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_PBAP,
                            PBAP_NOTIFICATION_AUTH_CHALL)) {
                        mPbapServer.authChallRsp(true, null, null);
                    }
                    cancelServerNotification();
                    break;
                default:
                    errorLog("Unsupported indication");
                    break;
            }
        }
    };

    /* Server state */
    private int mServerState = BluetoothAdapter.STATE_DISCONNECTED;

    /* A thread that handling PBAP request */
    private BluetoothPbapServer mPbapServer = null;

    /* Proxy binder API */
    private final IBluetoothPbap.Stub mServer = new IBluetoothPbap.Stub() {
        public int getState() {
            printLog("PBAP getState()=(" + mServerState + ")");
            return mServerState;
        }

        public BluetoothDevice getClient() {
            return null;
        }

        public boolean connect(BluetoothDevice device) {
            return false;
        }

        public void disconnect() {
            printLog("PBAP disconnect(), state =("+ mServerState +")");
            if ( mServerState == BluetoothAdapter.STATE_CONNECTED ) {
                mPbapServer.disconnect();
            }
            else if ( mServerState == BluetoothAdapter.STATE_CONNECTING ) {
                if ( mServiceHandler != null ) {
                    mServiceHandler.sendMessage(mServiceHandler.obtainMessage(PBAP_USER_CANCEL_AUTH));
                }
                else {
                    Log.e(TAG, "mServiceHandler is null");
                }
            }
        }

        public boolean isConnected(BluetoothDevice device) {
            return mIsConnected;
        }
    };

    public IBinder onBind(Intent intent) {
        Log.i(TAG, "Enter onBind()");
        if (IBluetoothPbap.class.getName().equals(intent.getAction())) {
            return mServer;
        }
        return null;
    }

    // For power on / off modification
    private void broadcastPbapState(int state) {
        BluetoothProfileManager.Profile profile;
        Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
        profile = BluetoothProfileManager.Profile.Bluetooth_PBAP;
        intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, profile);
        intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
        sendBroadcastAsUser(intent, UserHandle.ALL, android.Manifest.permission.BLUETOOTH);
    }

    // For power on / off modification

    @Override
    public void onCreate() {
        printLog("Enter onCreate()");

        // For power on / off modification
        broadcastPbapState(BluetoothProfileManager.STATE_ENABLING);
        // For power on / off modification

        String ns = Context.NOTIFICATION_SERVICE;
        mNM = (NotificationManager) getSystemService(ns);
        getContentResolver().registerContentObserver(ContactsContract.AUTHORITY_URI, true, mContactsChangedObserver);
        if (mNM == null) {
            Log.e(TAG, "Get Notification-Manager failed, stop PBAP service.");
            stopSelf();
            // For power on / off modification
            broadcastPbapState(BluetoothProfileManager.STATE_ABNORMAL);
            // For power on / off modification
        } else {
            /* get default adapter */
            mAdapter = BluetoothAdapter.getDefaultAdapter();
            if (mAdapter != null && mAdapter.isEnabled()) {
                mPbapServer = new BluetoothPbapServer(mServiceHandler, getApplicationContext());
                if (!mPbapServer.enable()) {
                    errorLog("[ERR] Pbap server enable failed");
                    mPbapServer = null;
                    mAdapter = null;
                    stopSelf();
                    broadcastPbapState(BluetoothProfileManager.STATE_ABNORMAL);
                } else {
                    broadcastPbapState(BluetoothProfileManager.STATE_ENABLED);
                }
            } else {
                mAdapter = null;
                errorLog("[ERR] Pbap server enable failed");
                broadcastPbapState(BluetoothProfileManager.STATE_ABNORMAL);
                stopSelf();
            }
        }
        IBinder b = ServiceManager.getService(BluetoothAdapter.BLUETOOTH_SERVICE);
        if (b == null) {
            throw new RuntimeException("Bluetooth service not available");
        }

        mBluetoothService = IBluetooth.Stub.asInterface(b);
    }

    @Override
    public void onDestroy() {
        printLog("onDestroy()");
        printLog("Before stop listening to socket.");
        cancelServerNotification();
        if (mPbapServer != null) {
            // broadcastPbapState(BluetoothProfileManager.STATE_DISABLING);
            mPbapServer.disable();
            broadcastPbapState(BluetoothProfileManager.STATE_DISABLED);
        }
        getContentResolver().unregisterContentObserver(mContactsChangedObserver);
        mPbapServer = null;
        mAdapter = null;
        sPbapServiceInstenceForUT = null;
    }

    /* Utility function */
    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private void errorLog(String msg) {
        Log.e(TAG, msg);
    }

    /* Utility function: sendServiceMsg */
    private void sendServiceMsg(int what, Object arg) {
        Message msg = null;
        printLog("[API] sendServiceMsg(" + what + ")");
        if (mServiceHandler != null) {
            msg = mServiceHandler.obtainMessage(what);
            msg.what = what;
            msg.obj = arg;
            mServiceHandler.sendMessage(msg);
        } else {
            printLog("mServiceHandler is null");
        }
    }

    /* Set notification for authorize and authenticate challenge */
    private void setServerNotification(int mode) {
        Context context = null;
        Intent intent = null;
        Notification noti = null;
        PendingIntent contentIntent = null;
        String returnAction = null;
        int timeout = -1;

        printLog("setServerNotification(" + mode + ")");
        context = getApplicationContext();
        intent = new Intent();
        /* Prepare the notification */
        if (mode == PBAP_NOTIFICATION_AUTHORIZE) {
            noti = new Notification(android.R.drawable.stat_sys_data_bluetooth,
                    getString(R.string.bluetooth_pbap_server_authorize_notify_ticker), System.currentTimeMillis());
            intent.setClassName(getPackageName(), BluetoothServerAuthorize.class.getName()).setFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK);
            printLog("Remote device name is " + mRemoteDevice.getName());
            intent.putExtra(BluetoothServerAuthorize.DEVICE_NAME, mRemoteDevice.getName()).putExtra(
                    BluetoothServerAuthorize.ACTION_CANCEL, PBAP_AUTHORIZE_CANCEL_ACTION).putExtra(
                    BluetoothServerAuthorize.ACTION_RETURN, PBAP_AUTHORIZE_RETURN_ACTION);
            // .putExtra(BluetoothServerAuthorize.TIMEOUT_VALUE,
            // PBAP_AUTHORIZE_TIMEOUT);
            returnAction = PBAP_AUTHORIZE_RETURN_ACTION;
            contentIntent = PendingIntent.getActivityAsUser(BluetoothPbapService.this, 0, intent, 0, null, UserHandle.CURRENT);
            noti.setLatestEventInfo(context, getString(R.string.bluetooth_pbap_server_authorize_notify_title),
                    getString(R.string.bluetooth_pbap_server_authorize_notify_message), contentIntent);
            timeout = PBAP_AUTHORIZE_TIMEOUT;
        } else if (mode == PBAP_NOTIFICATION_AUTH_CHALL) {
            noti = new Notification(android.R.drawable.stat_sys_data_bluetooth,
                    getString(R.string.bluetooth_pbap_server_auth_chall_notify_ticker), System.currentTimeMillis());
            intent.setClassName(getPackageName(), BluetoothAuthenticating.class.getName()).setFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.putExtra(BluetoothAuthenticating.DEVICE_NAME, mRemoteDevice.getName()).putExtra(
                    BluetoothAuthenticating.ACTION_CANCEL, PBAP_AUTHENTICATE_CANCEL_ACTION).putExtra(
                    BluetoothAuthenticating.ACTION_RETURN, PBAP_AUTHENTICATE_RETURN_ACTION);
            // .putExtra(BluetoothAuthenticating.TIMEOUT_VALUE,
            // PBAP_AUTHENTICATE_TIMEOUT);
            returnAction = PBAP_AUTHENTICATE_RETURN_ACTION;
            contentIntent = PendingIntent.getActivityAsUser(BluetoothPbapService.this, 0, intent, 0, null, UserHandle.CURRENT);
            noti.setLatestEventInfo(context, getString(R.string.bluetooth_pbap_server_auth_chall_notify_title), getString(
                    R.string.bluetooth_pbap_server_auth_chall_message, mRemoteDevice.getName()), contentIntent);
            timeout = PBAP_AUTHENTICATE_TIMEOUT;
        } else {
            errorLog("[ERR] unsupported mode");
            return;
        }

        /* Add vibrate */
        long[] vibrate = {
                0, 100, 200, 300
        };
        noti.vibrate = vibrate;
        noti.defaults |= Notification.DEFAULT_VIBRATE;
        noti.defaults |= Notification.DEFAULT_SOUND;
        noti.flags |= Notification.FLAG_NO_CLEAR;

        mPid = NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_PBAP, mode);
        mNM.notifyAsUser(null, mPid, noti, UserHandle.ALL);
        /* Register intent filter */
        IntentFilter returnFilter = new IntentFilter(returnAction);
        printLog("registerReceiver");
        registerReceiver(mReceiver, returnFilter);
        if (timeout > 0) {
            mServiceHandler.sendMessageDelayed(mServiceHandler.obtainMessage(PBAP_AUTH_TIMEOUT_IND), timeout);
        }
    }

    private void cancelServerNotification() {
        /* Stop timeout message */
        mServiceHandler.removeMessages(PBAP_AUTH_TIMEOUT_IND);
        /* Cancel notification */
        if (mPid > 0) {
            /* don't wanna receive intent form authorize & authenticate dialog */
            printLog("unregisterReceiver");
            unregisterReceiver(mReceiver);
            /* Send intent to stop dialog */
            if (mPid == NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_PBAP,
                    PBAP_NOTIFICATION_AUTHORIZE)) {
                printLog("Send authorize cancel intent");
                Intent intent = new Intent(PBAP_AUTHORIZE_CANCEL_ACTION);
                sendBroadcastAsUser(intent, UserHandle.ALL);
            } else if (mPid == NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_PBAP,
                    PBAP_NOTIFICATION_AUTH_CHALL)) {
                printLog("Send authenticate cancel intent");
                Intent intent = new Intent(PBAP_AUTHENTICATE_CANCEL_ACTION);
                sendBroadcastAsUser(intent, UserHandle.ALL);
            } else {
                errorLog("[ERR] invalid pid : " + mPid);
            }
            mNM.cancel(mPid);
            mPid = -1;
        }
    }
}
