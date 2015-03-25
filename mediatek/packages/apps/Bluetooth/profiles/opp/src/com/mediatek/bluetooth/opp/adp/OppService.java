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

package com.mediatek.bluetooth.opp.adp;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.sqlite.SQLiteFullException;
import android.net.Uri;
import android.os.IBinder;
import android.os.IUserManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserManager;
import android.os.Handler;
import android.os.Message;
import android.widget.Toast;

import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.opp.mmi.OppLog;
import com.mediatek.bluetooth.opp.mmi.UriDataUtils;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.SystemUtils;

import java.io.File;
import java.util.Collections;
import java.util.List;

/**
 * 1. ContentProvider consumer 2. Thread management
 */
public class OppService extends OppServiceNative {

    public static final String OPP_DIALOG_RETURN_TYPE = "com.android.bluetooth.opp.dialog.returnType";
    
    public static final String OPP_DIALOG_OPPS_SUBMIT_TASK_ACTION = "com.android.bluetooth.opp.opps.submitTask";

    public static final String OPP_DIALOG_OPPS_RESEND_NOTIFICATION = "com.android.bluetooth.opp.opps.resendNoti";

    public static final String OPP_DIALOG_OPPS_CANCEL_TASK = "com.android.bluetooth.opp.opps.cancelTask";

    public static final String OPP_DIALOG_RECEIVER_RETURNS = "com.android.bluetooth.opp.returns";

    public static final String OPPS_DIALOG_RESULT = "com.mediatek.bluetooth.opps.dialog_result";

    public static final int TOAST_DATABASE_FULL = 100;

    private OppManager mOppManager;

    private IUserManager mUserManagerService;

    private static int sCurrentUserId = 0;

    private OppTaskWorkerThread mOppcWorker;

    private OppTaskWorkerThread mOppsWorker;

    // For ALPS00118268 & ALPS00235236, task thread can't interuppt properly
    private boolean mIsTaskWorkThreadInterrupted = false;

    // For ALPS00231774, contentProvider operations remove out from OPPService
    // to oppcWorkerThread
    private boolean mIsOppcResetTask = false;

    // For ALPS0026, contentProvider operations remove out from OPPService to
    // oppsWorkerThread
    private boolean mIsOppsResetTask = false;

    private String mOppcCurrentStoragePath = null;

    private String mOppsCurrentStoragePath = null;

    private BroadcastReceiver mSdcardBroadcastReceiver = null;

    private BroadcastReceiver mDialogCallbackReceiver = null;

    private void registerSdcardBroadcastReceiver() {

        OppLog.i("OppService.registerSdcardBroadcastReceiver()[+]");

        this.mSdcardBroadcastReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {

                OppLog.i("OppService.BroadcastReceiver.onReceive()[+]");

                Uri path = intent.getData();
                if (path != null) {

                    OppLog.d("OppService: path[" + path.getPath() + "], oppc["
                            + mOppcCurrentStoragePath + "], opps[" + mOppsCurrentStoragePath + "]");

                    File oppcCurStorage = SystemUtils.getExternalStorageDirectory(context,
                            mOppcCurrentStoragePath);
                    if (oppcCurStorage != null
                            && oppcCurStorage.getAbsolutePath().equals(path.getPath())) {
                        oppcDisconnectNative();
                    }
                    File oppsCurStorage = SystemUtils.getExternalStorageDirectory(context,
                            mOppsCurrentStoragePath);
                    if (oppsCurStorage != null
                            && oppsCurStorage.getAbsolutePath().equals(path.getPath())) {
                        oppsDisconnectNative();
                    }
                }
            }
        };

        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addDataScheme("file");
        this.registerReceiver(this.mSdcardBroadcastReceiver, intentFilter);
    }

    private void unregisterSdCardBroadcastReceiver() {

        OppLog.i("OppService.unregisterSdCardBroadcastReceiver()[+]");

        if (this.mSdcardBroadcastReceiver != null) {

            this.unregisterReceiver(this.mSdcardBroadcastReceiver);
        }
    }

    private void registerDialogCallbackBroadcastReceiver() {

        OppLog.i("OppService.registerDialogCallbackBroadcastReceiver()[+]");

        mDialogCallbackReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {

                OppLog.i("OppService.mDialogCallbackBroadcastReceiver.onReceive()[+]");

                String returnType = intent.getStringExtra(OPP_DIALOG_RETURN_TYPE);
                if (returnType.equals(OPP_DIALOG_OPPS_SUBMIT_TASK_ACTION)) {
                    int state = intent.getIntExtra(OPPS_DIALOG_RESULT, BluetoothShareTask.STATE_REJECTING);
                    if (mOppManager != null) {
                        mOppManager.oppsSubmitTask(state);
                    }
                } else if (returnType.equals(OPP_DIALOG_OPPS_RESEND_NOTIFICATION)) {
                    if (mOppManager != null) {
                        mOppManager.oppsSendCurrentIncomingNotification();
                    }
                } else if (returnType.equals(OPP_DIALOG_OPPS_CANCEL_TASK)) {
                    Uri task = Uri.parse(intent.getStringExtra(OPPS_DIALOG_RESULT));
                    if ((mOppManager != null) && (task != null)) {
                        mOppManager.oppAbortTask(task);
                    }
                    
                }
                
            }

        
        };
        IntentFilter intentFilter = new IntentFilter(OPP_DIALOG_RECEIVER_RETURNS);
        this.registerReceiver(mDialogCallbackReceiver, intentFilter);
    }

    private void unregisterDialogCallbackBroadcastReceiver() {
        if (mDialogCallbackReceiver != null) {
            unregisterReceiver(mDialogCallbackReceiver);
        }
    }
    
    private Handler mServiceHandler = new Handler() {
        public void handleMessage(Message msg) {
            OppLog.i("[MSG] handleMessage(" + msg.what + ")");
            switch (msg.what) {
                case TOAST_DATABASE_FULL:
                    Toast.makeText(OppService.this, getString(R.string.bt_oppc_toast_disk_full), Toast.LENGTH_LONG).show();
                    break;

                default:
                    OppLog.e("Unsupported indication");
                    break;
            }
        }
    };

    @Override
    public void onCreate() {

        OppLog.i("OppService.onCreate()[+]");

        if (BluetoothAdapter.getDefaultAdapter().getState() != BluetoothAdapter.STATE_ON) {
            OppLog.d("OppService.onCreate(): BluetoothAdapter is not STATE_ON, stop OppService");
            BluetoothOppService.sendActivationBroadcast(this, true,
                    BluetoothOppService.STATE_ABNORMAL);
            BluetoothOppService.sendActivationBroadcast(this, false,
                    BluetoothOppService.STATE_ABNORMAL);
            this.stopSelf();
            return;
        }

        // required for object initialization and check the result (or ANR:
        // ALPS00092662)
        super.onCreate();
        if (!this.mIsServiceNativeEnabled) {

            OppLog.w("OppService native onCreate failed.");
            // service int failed -> stop self
            BluetoothOppService.sendActivationBroadcast(this, true,
                    BluetoothOppService.STATE_ABNORMAL);
            BluetoothOppService.sendActivationBroadcast(this, false,
                    BluetoothOppService.STATE_ABNORMAL);
            this.stopSelf();
            return;
        }

        // initialization
        this.mOppManager = OppManager.getInstance(this);
        this.mOppManager.setOppService(this);
        this.mOppManager.cancelAllNotification();
        
        IBinder b = ServiceManager.getService(Context.USER_SERVICE);
        mUserManagerService = IUserManager.Stub.asInterface(b);
        UserManager userManager = new UserManager(this, mUserManagerService);
        sCurrentUserId = userManager.getSwitchedUserId();
        OppLog.d("OppService get UserId is " + sCurrentUserId);

        // register sdcard broadcast receiver
        this.registerSdcardBroadcastReceiver();
        registerDialogCallbackBroadcastReceiver();

        // enable oppc ( context init ) => must be called before worker-thread
        // start
        BluetoothOppService.sendActivationBroadcast(this, true, BluetoothOppService.STATE_ENABLING);
        if (this.oppcEnable()) {

            BluetoothOppService.sendActivationBroadcast(this, true,
                    BluetoothOppService.STATE_ENABLED);
        } else {
            BluetoothOppService.sendActivationBroadcast(this, true,
                    BluetoothOppService.STATE_ABNORMAL);
        }

        BluetoothOppService
                .sendActivationBroadcast(this, false, BluetoothOppService.STATE_ENABLING);
        if (this.oppsEnable()) {

            BluetoothOppService.sendActivationBroadcast(this, false,
                    BluetoothOppService.STATE_ENABLED);
        } else {
            BluetoothOppService.sendActivationBroadcast(this, false,
                    BluetoothOppService.STATE_ABNORMAL);
        }

        // start worker thread
        this.mOppcWorker = new OppTaskWorkerThread("BtOppc", new OppcTaskHandler());
        this.mOppcWorker.start();
        this.mOppsWorker = new OppTaskWorkerThread("BtOpps", new OppsTaskHandler());
        this.mOppsWorker.start();

        // reset task
        // If pengding too much task in content provider, ANR will happen
        // this.oppManager.oppcResetTaskState();
        // this.oppManager.oppsResetTaskState();

        // process pending oppc task(s)
        // this.oppcWorker.notifyNewTask();

        OppLog.i("OppService.onCreate()[-]");
    }

    @Override
    public void onDestroy() {

        OppLog.i("OppService.onDestroy()[+]");

        if (this.mIsServiceNativeEnabled) {

            // cancel all notification (user action entry point)
            this.mOppManager.oppOnServiceStop();

            // stop worker thread => must be called before diable (or event
            // maybe received)
            OppLog.d("OppService.onDestroy() interrupt OppTaskWorkerThread...");
            this.mOppcWorker.interrupt();
            this.mOppsWorker.interrupt();

            this.mIsTaskWorkThreadInterrupted = true;

            // disable opp service
            OppLog.d("OppService.onDestroy() disable oppc/opps native service...");
            if (this.oppcDisable()) {

                BluetoothOppService.sendActivationBroadcast(this, true,
                        BluetoothOppService.STATE_DISABLED);
            } else {
                BluetoothOppService.sendActivationBroadcast(this, true,
                        BluetoothOppService.STATE_ABNORMAL);
            }
            if (this.oppsDisable()) {

                BluetoothOppService.sendActivationBroadcast(this, false,
                        BluetoothOppService.STATE_DISABLED);
            } else {
                BluetoothOppService.sendActivationBroadcast(this, false,
                        BluetoothOppService.STATE_ABNORMAL);
            }

            // register sdcard broadcast receiver
            this.unregisterSdCardBroadcastReceiver();
            unregisterDialogCallbackBroadcastReceiver();
        }

        // required for object destroy
        OppLog.d("OppService.onDestroy() call native destroy()...");
        super.onDestroy(); // OppServiceNative.onDestroy()

        // reset OppService
        if (this.mOppManager != null) {

            this.mOppManager.setOppService(null);
        }

        OppLog.i("OppService.onDestroy()[-]");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        OppLog.i("OppService.onStartCommand()[+]");

        // action
        if (this.mIsServiceNativeEnabled && intent != null) {

            String action = intent.getAction();
            OppLog.d("OppService.onStartCommand() action: " + action);

            // oppc
            if (OppConstants.OppService.ACTION_OPPC_START.equals(action)) {

                this.mOppcWorker.notifyNewTask();
            } else if (OppConstants.OppService.ACTION_OPPS_START.equals(action)) { // opps

                // stop listen before handle opps task
                this.oppsStopListenDisconnect();
                this.mOppsWorker.notifyNewTask();
            } else {
                // OppLog.e( "OppService.onStartCommand unsupported action: " +
                // action );
                // default start opps: from BluetoothReceiver
                this.mOppsWorker.notifyNewTask();
            }
        } else {
            OppLog.w("OppService.onStartCommand() warn: isServiceNativeEnabled["
                    + this.mIsServiceNativeEnabled + "] or null Intent");
        }

        OppLog.i("OppService.onStartCommand()[-]");
        return super.onStartCommand(intent, flags, startId);
    }

    public static int getCurrentUserId() {
        return sCurrentUserId;
    }

    /******************************************************************************************************
     * OPPC Service Implementation
     ******************************************************************************************************/
    class OppcTaskHandler implements OppTaskHandler {

        public boolean beforeWait() throws InterruptedException {

            // oppc task is from UI (user)

            // check pending task will remove to oppc thread to avoid ANR in OPP
            // Service (ANR: ALPS00231774)
            if (!OppService.this.mIsOppcResetTask) {

                OppService.this.mIsOppcResetTask = true;

                OppLog.d("oppc beforeWait() - oppcResetTaskState() ");
                OppService.this.mOppManager.oppcResetTaskState();

                // begain to process oppc pending task
                ContentResolver contentResolver = OppService.this.getContentResolver();

                // query all pending tasks
                Cursor cursor = contentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI,
                        new String[] {
                            BluetoothShareTaskMetaData._ID
                        }, BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                                + BluetoothShareTaskMetaData.TASK_STATE + " = ?", new String[] {
                                Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_START),
                                Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_END),
                                Integer.toString(BluetoothShareTask.STATE_PENDING)
                        }, BluetoothShareTaskMetaData._ID + " ASC");

                // construct result list
                List<Uri> newTaskList = Collections.emptyList();
                try {
                    newTaskList = OppService.this.mOppManager.getOppTaskList(cursor);
                    OppLog.d("oppc beforeWait() - task count: " + newTaskList.size());
                } finally {
                    if (cursor != null) {
                        cursor.close();
                        cursor = null;
                    }
                }

                // acquire wake-lock
                OppService.this.mOppManager.acquireWakeLock();
                try {
                    // loop for all tasks
                    for (Uri newTask : newTaskList) {

                        OppLog.d(" oppc beforeWait() processing task: " + newTask);
                        this.processBatchPush(newTask);
                    }
                } finally {
                    // release wake-lock
                    OppService.this.mOppManager.releaeWakeLock();
                }
            }

            return true;
        }

        public void afterWait() throws InterruptedException {

            ContentResolver contentResolver = OppService.this.getContentResolver();

            // query all pending tasks
            Cursor cursor = contentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI,
                    new String[] {
                        BluetoothShareTaskMetaData._ID
                    }, BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                            + BluetoothShareTaskMetaData.TASK_STATE + " = ?", new String[] {
                            Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_START),
                            Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_END),
                            Integer.toString(BluetoothShareTask.STATE_PENDING)
                    }, BluetoothShareTaskMetaData._ID + " ASC");

            // construct result list
            List<Uri> newTaskList = Collections.emptyList();
            try {
                newTaskList = OppService.this.mOppManager.getOppTaskList(cursor);
                OppLog.d("oppc afterWait() - task count: " + newTaskList.size());
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }

            // acquire wake-lock
            OppService.this.mOppManager.acquireWakeLock();
            try {
                // loop for all tasks
                for (Uri newTask : newTaskList) {

                    OppLog.d(" oppc afterWait() processing task: " + newTask);
                    this.processBatchPush(newTask);
                }
            } finally {
                // release wake-lock
                OppService.this.mOppManager.releaeWakeLock();
            }
        }

        private void processBatchPush(Uri taskUri) throws InterruptedException {

            // attributes for content-provider operations
            ContentResolver contentResolver = OppService.this.getContentResolver();
            String pendingTaskWhere = BluetoothShareTaskMetaData.TASK_STATE + "="
                    + BluetoothShareTask.STATE_PENDING;

            // query current task and get batch peerAddr and timestamp
            BluetoothShareTask initTask = null;
            Cursor cursor = contentResolver.query(taskUri, null, pendingTaskWhere, null, null);
            try {
                if (cursor == null || !cursor.moveToFirst()) {

                    return; // e.g. aborted by user
                }
                initTask = new BluetoothShareTask(cursor);
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }

            // query all tasks in the same batch
            cursor = contentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI, new String[] {
                    BluetoothShareTaskMetaData._ID
            }, BluetoothShareTaskMetaData.TASK_TYPE + " = ? AND "
                    + BluetoothShareTaskMetaData.TASK_STATE + " = ? AND "
                    + BluetoothShareTaskMetaData.TASK_PEER_ADDR + " = ? AND "
                    + BluetoothShareTaskMetaData.TASK_CREATION_DATE + " = ?", new String[] {
                    Integer.toString(BluetoothShareTask.TYPE_OPPC_PUSH),
                    Integer.toString(BluetoothShareTask.STATE_PENDING), initTask.getPeerAddr(),
                    Long.toString(initTask.getCreationDate())
            }, BluetoothShareTaskMetaData._ID + " ASC");

            // construct result list
            List<Uri> batchTaskList = Collections.emptyList();
            try {
                batchTaskList = OppService.this.mOppManager.getOppTaskList(cursor);
                OppLog.d("oppc processBatchPush() - task count: " + batchTaskList.size());
            } finally {
                if (cursor != null) {
                    cursor.close();
                    cursor = null;
                }
            }

            // no task in this batch (shouldn't happen)
            if (batchTaskList.size() < 1) {
                return;
            }

            // connect to peer device
            boolean isConnected = OppService.this.oppcConnect(initTask.getPeerAddr());
            boolean isDisconnected = false;

            // loop all batch tasks
            try {
                // send state changed broadcast: connected
                if (isConnected) {
                    BluetoothOppService.sendStateChangedBroadcast(OppService.this, initTask, true);
                }

                // constants for task update
                ContentValues values = new ContentValues();
                values.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_ONGOING);

                for (Uri newTask : batchTaskList) {

                    if (OppService.this.mIsTaskWorkThreadInterrupted) {
                        OppLog.i("OppTaskWorkerThread had been interuppted, stop current task.");

                        // after disable oppservice, need not disconnect by
                        // upper layer
                        isDisconnected = true;
                        break;
                    }

                    OppLog.d(" oppc processBatchPush() processing task: " + newTask);

                    // update content provider: state = ongoing
                    int count = contentResolver.update(newTask, values, pendingTaskWhere, null);
                    if (count != 1) {
                        OppLog.i("skip non-pending task: " + newTask);
                        continue;
                    }

                    // lock current task ( can accept abort request )
                    OppService.this.oppcSetCurrentTask(newTask);

                    // query new state of task
                    cursor = contentResolver.query(newTask, null, // all columns
                            null, null, null);

                    BluetoothShareTask task = null;
                    try {
                        if (cursor == null || !cursor.moveToFirst()) {

                            // e.g. aborted by user
                            continue;
                        }
                        task = new BluetoothShareTask(cursor);

                        // task maybe canceled after oppcSetCurrentTask() but
                        // before task query()
                        if (task.getState() == BluetoothShareTask.STATE_ABORTING) {

                            BtLog.i("handle aborting task before push it.");
                            task.setState(BluetoothShareTask.STATE_ABORTED);
                            this.onObjectChange(task);
                            continue;
                        }

                        // start monitor eject broadcast first
                        mOppcCurrentStoragePath = task.getData();

                        // isStorageMounted = InternalStorage ||
                        // MountedExternalStorage
                        boolean isStorageMounted = (SystemUtils.getExternalStorageDirectory(
                                OppService.this, mOppcCurrentStoragePath) == null)
                                || SystemUtils.isExternalStorageMounted(OppService.this,
                                        mOppcCurrentStoragePath);

                        if (isConnected && !isDisconnected && isStorageMounted) {

                            // open uri data for task
                            UriDataUtils.openUriData(OppService.this, Uri
                                    .parse(task.getObjectUri()), task.getData());
                            // call push api
                            isDisconnected = !OppService.this.oppcPush(task, this);
                            mOppcCurrentStoragePath = null;
                            // close uri data
                            UriDataUtils.closeUriData(OppService.this, Uri.parse(task
                                    .getObjectUri()), task.getData());
                        } else {
                            // batch connection failed => all task fail
                            task.setState(BluetoothShareTask.STATE_FAILURE);
                            this.onObjectChange(task);
                        }
                    } finally {
                        // reset current task
                        mOppcCurrentStoragePath = null;
                        OppService.this.oppcSetCurrentTask(null);
                        if (cursor != null) {
                            cursor.close();
                            cursor = null;
                        }
                    }
                }
            } finally {
                // send state changed broadcast: disconnected
                if (isConnected) {
                    if ((!isDisconnected) && (!OppService.this.mIsTaskWorkThreadInterrupted)) {
                        OppService.this.oppcDisconnect();
                    }
                    BluetoothOppService.sendStateChangedBroadcast(OppService.this, initTask, false);
                    // sleep and let bttask can handle disconnect properly
                    Thread.sleep(660);
                }
            }

        }

        public void onObjectChange(BluetoothShareTask task) {

            if (Options.LL_DEBUG) {

                OppLog.d("oppc onObjectChange() for taskId[" + task.getId() + "], state["
                        + task.getState() + "]");
            }

            task.setModifiedDate(System.currentTimeMillis());

            // notify
            OppService.this.mOppManager.notifyOppTask(task);

            // update content provider
            if (task.getState() == BluetoothShareTask.STATE_ONGOING && task.getDoneBytes() != 0) {
                OppLog.d("onObjectChange,task is STATE_ONGOING");
                // skip progress update event => don't update db, update ui only
                // do db update for every progress update will cause:
                // 1. MMI thread (OppTaskWorkerThread) slower than EXT Thread
                // (MessageListener)
                // 2. UI progress display is out of sync.
            } else {
                try {
                    OppService.this.getContentResolver().update(task.getTaskUri(),
                            task.getContentValues(), null, null);
                    } catch (Exception e) {
                        OppLog.e("onObjectChange::update db error");
                        e.printStackTrace();
                    }
            }
        }
    }

    /******************************************************************************************************
     * OPPS Service Implementation
     ******************************************************************************************************/
    class OppsTaskHandler implements OppTaskHandler {

        // used to verify current access request is processed
        boolean mIsBusy = false;

        /**
         * before waiting for user confirmation
         */
        public boolean beforeWait() throws InterruptedException {

            if (!OppService.this.mIsOppsResetTask) {

                OppService.this.mIsOppsResetTask = true;

                // reset task(delete DB record) will remove to oppsWorkThread to
                // avoid ANR in OPP Service (ANR: ALPS00268876)
                OppLog.d("opps beforeWait() - oppsResetTaskState() ");
                OppService.this.mOppManager.oppsResetTaskState();
            }

            if (this.mIsBusy) {
                return true;
            }

            // waiting for incoming request
            OppEvent ind = OppService.this.oppsWaitForAccessRequest();
            String[] args = ind.parameters;

            // start activity to handle request confirmation
            if (ind.event == OppEvent.BT_OPPS_PUSH_ACCESS_REQUEST) {

                // pupup notification & dialog to confirm request
                boolean isConfirmed = OppService.this.mOppManager.oppsStartPushActivity(
                        OppService.this.oppsIsAuthorized(), args[0], args[1], args[2], Long
                                .parseLong(args[3]));
                this.mIsBusy = true;

                // listen disconnect event from stack
                if (!isConfirmed) {

                    boolean isDisconnected = OppService.this.oppsListenDisconnect();
                    if (isDisconnected) {
                        // connection timeout or canceled by peer device
                        mIsBusy = false; // request done
                        OppService.this.mOppManager.oppsCancelPushActivity();
                        return false;
                    }
                } else {
                	
                	// set state as CLEARED while database insert error              	
                	if (OppService.this.mOppManager.mOppsTask != null ) {
                		
                		if (OppService.this.mOppManager.mOppsTask.getState() == BluetoothShareTask.STATE_CLEARED) {
                			
                			OppService.this.oppsAccessResponseNative(OppConstants.GOEP.DATABASE_FULL, 
                			        new String[] {
                			        	    "0", ""
                			        });  
                			// peer should launch disconnect after received our un-success push rsp
                			boolean isDisconnected = OppService.this.oppsListenDisconnect();
                			if (isDisconnected) {
                				
                				mIsBusy = false; // request done
                				return false;
                			}	
                		}
                	}     			
                }     			
                
                // user confirmed the request (accept or reject)
                return true;
            } else {
                OppLog.e("opps beforeWait(): get unsupported event(oppsRequestIndication)");
                OppService.this.oppsAccessResponseNative(OppConstants.GOEP.NOT_IMPLEMENTED,
                        new String[] {
                                "0", ""
                        });
                this.mIsBusy = false;
                return false; // wait for next indication
            }
        }

        public void afterWait() throws InterruptedException {

            // query all pending tasks
            Cursor cursor = OppService.this.getContentResolver().query(
                    BluetoothShareTaskMetaData.CONTENT_URI,
                    new String[] {
                            BluetoothShareTaskMetaData._ID
                    },
                    BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                            + BluetoothShareTaskMetaData.TASK_STATE + " in ( ?, ? )",
                    new String[] {
                            Integer.toString(BluetoothShareTask.TYPE_OPPS_GROUP_START),
                            Integer.toString(BluetoothShareTask.TYPE_OPPS_GROUP_END),
                            Integer.toString(BluetoothShareTask.STATE_PENDING),
                            Integer.toString(BluetoothShareTask.STATE_REJECTING)
                    }, BluetoothShareTaskMetaData._ID + " ASC");

            // construct result list
            List<Uri> newTaskList = Collections.emptyList();
            try {
                newTaskList = OppService.this.mOppManager.getOppTaskList(cursor);
                OppLog.d("opps afterWait(): task count: " + newTaskList.size());
            } finally {

                if (cursor != null) {

                    cursor.close();
                    cursor = null;
                }
            }

            // acquire wake-lock
            OppService.this.mOppManager.acquireWakeLock();
            try {
                for (Uri newTask : newTaskList) {

                    OppLog.d("opps afterWait() processing task:" + newTask);

                    // lock cuurent task
                    OppService.this.oppsSetCurrentTask(newTask);

                    // query new state of task
                    cursor = OppService.this.getContentResolver().query(newTask,
                            null, // all columns
                            BluetoothShareTaskMetaData.TASK_STATE + "="
                                    + BluetoothShareTask.STATE_PENDING + " OR "
                                    + BluetoothShareTaskMetaData.TASK_STATE + "="
                                    + BluetoothShareTask.STATE_REJECTING, null, null);

                    BluetoothShareTask task = null;
                    try {
                        if (cursor == null || !cursor.moveToFirst()) {

                            // maybe canceled by user
                            continue;
                        }
                        task = new BluetoothShareTask(cursor);

                        // send state changed broadcast: connected
                        BluetoothOppService.sendStateChangedBroadcast(OppService.this, task, true);
                        // call push api
                        mOppsCurrentStoragePath = task.getData();
                        OppService.this.oppsAccessResponse(task, this);
                        OppService.this.oppsSetCurrentTask(null);
                    } finally {
                        mOppsCurrentStoragePath = null;
                        if (task != null) {

                            // send state changed broadcast: disconnected
                            BluetoothOppService.sendStateChangedBroadcast(OppService.this, task,
                                    false);
                        }

                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                }
            } finally {
                // release wake-lock
                OppService.this.mOppManager.releaeWakeLock();

                // request done
                this.mIsBusy = false;
            }
        }

        public void onObjectChange(BluetoothShareTask task) {

            if (Options.LL_DEBUG) {

                OppLog.d("opps onObjectChange() for taskId=" + task.getId() + ",state="
                        + task.getState());
            }

            task.setModifiedDate(System.currentTimeMillis());

            OppService.this.mOppManager.notifyOppTask(task);

            // update content provider
            if (task.getState() == BluetoothShareTask.STATE_ONGOING && task.getDoneBytes() != 0) {
                OppLog.d("onObjectChange,task state is STATE_ONGOING");
                // skip progress update event => don't update db, update ui only
                // do db update for every progress update will cause:
                // 1. MMI thread (OppTaskWorkerThread) slower than EXT Thread
                // (MessageListener)
                // 2. UI progress display is out of sync.
            } else {
                try {
                    OppService.this.getContentResolver().update(task.getTaskUri(),
                            task.getContentValues(), null, null);
                } catch (SQLiteFullException e) {
                    Message msg = null;
                    OppLog.d("[API] sendServiceMsg(" + TOAST_DATABASE_FULL + ")");
                    if (mServiceHandler != null) {
                        msg = mServiceHandler.obtainMessage(TOAST_DATABASE_FULL);
                        msg.what = TOAST_DATABASE_FULL;
                        mServiceHandler.sendMessage(msg);
                    }
                }
            }
        }
    }
}
