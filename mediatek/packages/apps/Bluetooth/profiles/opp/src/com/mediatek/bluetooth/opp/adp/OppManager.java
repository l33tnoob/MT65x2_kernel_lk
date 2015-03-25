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

import android.app.Notification;
import android.app.NotificationManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteFullException;
import android.net.Uri;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Process;
import android.os.SystemClock;
import android.os.UserHandle;
import android.util.Pair;
import android.widget.Toast;

import com.mediatek.activity.CancelableActivity;
import com.mediatek.bluetooth.BluetoothProfile;
import com.mediatek.bluetooth.Options;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.opp.adp.OppService;
import com.mediatek.bluetooth.opp.mmi.OppLog;
import com.mediatek.bluetooth.opp.mmi.UriData;
import com.mediatek.bluetooth.opp.mmi.UriDataUtils;
import com.mediatek.bluetooth.opp.mmi.Utils;
import com.mediatek.bluetooth.share.BluetoothShareNotification;
import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;
import com.mediatek.bluetooth.share.BluetoothShareTask.Direction;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.MediaScanner;
import com.mediatek.bluetooth.util.MimeUtils;
import com.mediatek.bluetooth.util.NotificationFactory;
import com.mediatek.bluetooth.util.SystemUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

/**
 * @author Jerry Hsu 1. implement as an "singleton" object.
 */
public class OppManager implements Callback {

    // singleton
    private static OppManager sInstance = null;

    public static final String PATH_FOR_STORAGE_FROM_SYSTEM = "/storage/emulated/";

    // bluetooth
    private BluetoothAdapter mBluetoothAdapter;

    // context objects
    private Context mApplicationContext;

    private ContentResolver mContentResolver;

    private NotificationManager mNotificationManager;

    private PowerManager mPowerManager;

    private WakeLock mWakeLock;

    // OppTask cache for OPPC ( keep request before device selected )
    private ArrayList<BluetoothShareTask> mOppcTaskCache = null;

    // keep current task for OPPS
    public BluetoothShareTask mOppsTask = null;

    // run job in another Thread to prevent ANR
    private LooperThread mBgRunner = new LooperThread("OppManagerExecuter", Process.THREAD_PRIORITY_BACKGROUND, this);

    // opp service
    private OppService mOppService;

    // A list of devices that may send files over OPP to this device
    // without user confirmation. Used for connection handover from forex NFC.
    private List<Pair<String, Long>> mWhitelist = new ArrayList<Pair<String, Long>>();

    // The time for which the whitelist entries remain valid.
    private static final int WHITELIST_DURATION_MS = 15000;

    // singleton
    public static synchronized OppManager getInstance(Context context) {

        if (sInstance == null) {

            sInstance = new OppManager();
            sInstance.init(context);
        }
        return sInstance;
    }

    /**
     * Private constructor
     */
    private OppManager() {
    }

    /**
     * initialize context objects
     * 
     * @param context
     */
    private void init(Context context) {

        this.mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (this.mBluetoothAdapter == null) {

            OppLog.w("Bluetooth is not supported in this hardware platform (null BluetoothAdapter).");
            return;
        }

        this.mApplicationContext = context.getApplicationContext();
        this.mContentResolver = this.mApplicationContext.getContentResolver();
        this.mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        this.mPowerManager = (PowerManager) this.mApplicationContext.getSystemService(Context.POWER_SERVICE);
        this.mWakeLock = this.mPowerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Bluetooth");
        this.mBgRunner.start();
    }

    private void cleanupWhitelist() {
        // Removes expired entries
        long curTime = SystemClock.elapsedRealtime();
        for (Iterator<Pair<String, Long>> iter = mWhitelist.iterator(); iter.hasNext();) {
            Pair<String, Long> entry = iter.next();
            if (curTime - entry.second > WHITELIST_DURATION_MS) {
                iter.remove();
            }
        }
    }

    public synchronized void addToWhitelist(String address) {
        if (address == null) {
            return;
        }
        // Remove any existing entries
        for (Iterator<Pair<String, Long>> iter = mWhitelist.iterator(); iter.hasNext();) {
            Pair<String, Long> entry = iter.next();
            if (entry.first.equals(address)) {
                iter.remove();
            }
        }
        mWhitelist.add(new Pair<String, Long>(address, SystemClock.elapsedRealtime()));
    }

    public synchronized boolean isWhitelisted(String address) {
        cleanupWhitelist();
        for (Pair<String, Long> entry : mWhitelist) {
            if (entry.first.equals(address)) {
                return true;
            }
        }
        return false;
    }

    ///M: For multiUser, we should change the storage path to user ID path
    ///M: /storage/emulated/0 change to /storage/emulated/10
    private String checkPathForMultiUser(String orignalPath) {
        int currentUserId = OppService.getCurrentUserId();
        if (currentUserId != 0 && orignalPath.startsWith(PATH_FOR_STORAGE_FROM_SYSTEM + "0")) {
            orignalPath = orignalPath.substring((PATH_FOR_STORAGE_FROM_SYSTEM + "0").length());
            orignalPath = PATH_FOR_STORAGE_FROM_SYSTEM + currentUserId + orignalPath;
        }

        return orignalPath;
    }

    /********************************************************************************************
     * OPPC API
     ********************************************************************************************/

    /**
     * add OppTask into taskCache (wait for device selected event to complete
     * this task)
     *
     * @param task
     */
    public synchronized void oppcCacheTask(final BluetoothShareTask task) {

        // OppLog.d( "OppManager.oppcCacheTask()[+]");

        if (this.mOppcTaskCache == null) {

            this.mOppcTaskCache = new ArrayList<BluetoothShareTask>(3);
        }

        // cache task
        this.mOppcTaskCache.add(task);
    }

    /**
     * use selected device to commit current OppTask (in taskCache)
     * 
     * @param device
     */
    public synchronized void oppcSubmitTask(final BluetoothDevice device) {

        // log
        if (Options.LL_DEBUG) {

            OppLog.d("oppcSubmitTask for device[" + device.getName() + "][" + device.getAddress() + "]");
            // for( BluetoothShareTask task : this.oppcTaskCache ){

            // OppLog.d( "oppcSubmitTask task objectUri:" + task.getObjectUri()
            // );
            // }
        }

        if (this.mOppcTaskCache != null) {

            // copy the cached object list (and then the next round of sharing
            // operation can continue without ConcurrentModificationException)
            Object[] param = new Object[] {
                    device, this.mOppcTaskCache
            };
            this.mOppcTaskCache = null;

            // run the job in another thread to prevent ANR
            this.mBgRunner.mHandler.sendMessage(this.mBgRunner.mHandler.obtainMessage(MSG_OPPC_SUBMIT_TASK, param));
        }
    }

    /**
     * should be called when service start and will: 1. remove finish tasks
     * (success, failure, aborted, aborting, rejecting) 2. reset the state of
     * tasks: 2.1. ongoing => pending 2.2. fail => pending (retry) 3. notify
     * worker-thread to process all pending tasks.
     */
    protected void oppcResetTaskState() {

        // remove finish tasks
        this.mContentResolver.delete(BluetoothShareTaskMetaData.CONTENT_URI,
                BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                        + BluetoothShareTaskMetaData.TASK_STATE + " in ( ?, ?, ?, ? )",
                new String[] {
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_START),
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_END),
                        Integer.toString(BluetoothShareTask.STATE_ABORTED),
                        Integer.toString(BluetoothShareTask.STATE_ABORTING),
                        Integer.toString(BluetoothShareTask.STATE_REJECTING),
                        Integer.toString(BluetoothShareTask.STATE_CLEARED)
                });

        // reset task: state / done-bytes / result
        ContentValues cv = new ContentValues();
        cv.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_PENDING);
        cv.put(BluetoothShareTaskMetaData.TASK_DONE_BYTES, 0);
        cv.put(BluetoothShareTaskMetaData.TASK_RESULT, "");
        this.mContentResolver.update(BluetoothShareTaskMetaData.CONTENT_URI, cv,
                BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                        + BluetoothShareTaskMetaData.TASK_STATE + " = ?", new String[] {
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_START),
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_END),
                        Integer.toString(BluetoothShareTask.STATE_ONGOING)
                });

        // notify user (via Android Notification)
        Cursor cursor = this.mContentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI, null,
                BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND "
                        + BluetoothShareTaskMetaData.TASK_STATE + " = ?", new String[] {
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_START),
                        Integer.toString(BluetoothShareTask.TYPE_OPPC_GROUP_END),
                        Integer.toString(BluetoothShareTask.STATE_PENDING)
                }, BluetoothShareTaskMetaData._ID + " ASC");

        try {
            if (cursor == null || !cursor.moveToFirst()) {

                BtLog.i("oppcResetTaskState() - can't find any OPPC pending task to restart.");
                return;
            }

            if (Options.LL_DEBUG) {

                OppLog.d("oppc found [" + cursor.getCount()
                        + "] pending tasks after reset (creating notification for them).");
            }
            for (; !cursor.isAfterLast(); cursor.moveToNext()) {

                this.notifyOppTask(new BluetoothShareTask(cursor));
            }
        } finally {

            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public Uri oppcGetCurrentTask() {

        return this.mOppService.oppcGetCurrentTask();
    }

    /********************************************************************************************
     * OPPS API
     ********************************************************************************************/

    /**
     * check the parameter and storage is valid
     */
    public String oppsCheckCurrentTask() {

        // check parameter
        if (this.mOppsTask == null || this.mOppsTask.getPeerAddr() == null
                || this.mOppsTask.getObjectName() == null || this.mOppsTask.getTotalBytes() < 0) {

            if (this.mOppsTask == null) {

                OppLog.e("current opps task is null => can't check it");
            } else {
                OppLog.i("invalid opps new task parameters: peerAddr[" + this.mOppsTask.getPeerAddr() + "], objectName["
                        + this.mOppsTask.getObjectName() + "], totalByte[" + this.mOppsTask.getTotalBytes() + "]");
            }
            return this.mApplicationContext.getString(R.string.bt_opps_push_toast_invalid_request);
        }

        // check storage writable size
        long as = SystemUtils.getReceivedFilePathAvailableSize(this.mApplicationContext);
        if (as <= this.mOppsTask.getTotalBytes()) {

            OppLog.i("storage is not available for opps new task: available[" + as + "] v.s. required["
                    + this.mOppsTask.getTotalBytes() + "]");
            return this.mApplicationContext.getString(R.string.bt_opps_toast_storage_unavailable);
        }

        // check ok
        return null;
    }

    /**
     * start Activity to process incoming Push request
     * 
     * @param peerAddr
     * @param objectName
     * @param mimeType
     * @param totalBytes
     */
    protected boolean oppsStartPushActivity(boolean isAuthorized, String peerAddr, String objectName, String mimeType,
            long totalBytes) {

        if (Options.LL_DEBUG) {

            OppLog.d("oppsStartPushActivity for: authorized[" + isAuthorized + "], device[" + peerAddr + "], object["
                    + objectName + "], mime[" + mimeType + "], size[" + totalBytes + "]");
        }

        if (MimeUtils.VCARD_TYPE.equals(mimeType)) {
            objectName = MimeUtils.applyVcardExt(objectName, 256);
        }

        // prepare task object
        String peerName = this.getDeviceName(peerAddr);

        // check mimeType
        String localMimeType = MimeUtils.getMimeType(objectName);
        OppLog.d("oppsStartPushActivity::localMimeType = " + localMimeType + " mimeType = " + mimeType);
        if (localMimeType == null && mimeType != null || mimeType.length() != 0 ) {
            localMimeType = mimeType;
        }

        // create task object
        this.mOppsTask = new BluetoothShareTask(BluetoothShareTask.TYPE_OPPS_PUSH);
        this.mOppsTask.setPeerAddr(peerAddr);
        this.mOppsTask.setPeerName(peerName);
        this.mOppsTask.setObjectName(objectName);
        this.mOppsTask.setMimeType(localMimeType);
        this.mOppsTask.setTotalBytes(totalBytes);
        //need to check filename valid
        String filePath = Utils.getValidStoragePath(this.mApplicationContext, this.mOppsTask.getObjectName());
        ///M: For multiUser, we should change the storage path to user ID path
        ///M: /storage/emulated/0 change to /storage/emulated/10
        filePath = checkPathForMultiUser(filePath);
        OppLog.d("checkPathForMultiUser" + filePath );
        this.mOppsTask.setData(filePath);

        boolean isHandover = isWhitelisted(mOppsTask.getPeerAddr());
        this.mOppsTask.setHandover(isHandover);    

        // insert content-provider and get id (for notification)
        Uri newUri = this.mContentResolver.insert(BluetoothShareTaskMetaData.CONTENT_URI, this.mOppsTask.getContentValues());
        
        // database exception, no storage resource or other reason
        if (newUri == null) {
        	
        	OppLog.d("opps newUri: " + newUri );
        	this.mBgRunner.mHandler.sendMessage(this.mBgRunner.mHandler.obtainMessage(MSG_SHOW_TOAST, 
        	         this.mApplicationContext.getString(R.string.bt_opps_toast_storage_unavailable))); 
        	this.mOppsTask.setState(BluetoothShareTask.STATE_CLEARED);
        	return true;
        }
        
        this.mOppsTask.setId(Integer.parseInt(newUri.getLastPathSegment()));

        // print debug message
        if (Options.LL_DEBUG) {

            OppLog.d("opps newTask: " + this.mOppsTask.getPrintableString());
        }

        // check task
        String errMessage = this.oppsCheckCurrentTask();
        if (errMessage != null) {

            this.mBgRunner.mHandler.sendMessage(this.mBgRunner.mHandler.obtainMessage(MSG_SHOW_TOAST, errMessage));
            // Toast.makeText( applicationContext, errMessage, Toast.LENGTH_LONG
            // ).show();
            this.oppsSubmitTask(BluetoothShareTask.STATE_REJECTING);
            return true; // result confirmed: task submitted
        }

        // check authorization and if the sender is contained in white list
        if (isAuthorized || isHandover) {

            this.oppsSubmitTask(BluetoothShareTask.STATE_PENDING);
            return true; // result confirmed: task submitted
        } else {

            // create and send notification
            int notificationId = NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_OPP, this.mOppsTask
                    .getId());
            Notification notification = OppNotificationFactory.getOppIncomingNotification(this.mApplicationContext,
                    this.mOppsTask);

            // send timeout notification
            this.mNotificationManager.notifyAsUser(null, notificationId, notification, UserHandle.ALL);
            return false; // result undetermined
        }
    }

    public void oppsSendCurrentIncomingNotification() {

        synchronized (this.mOppsTask) {

            // incoming request is submitted (accept or reject)
            if (this.mOppsTask == null) {
                return;
            }

            // re-sned incoming notification after activity stopped
            int notificationId = NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_OPP, this.mOppsTask
                    .getId());
            Notification n = OppNotificationFactory.getOppIncomingNotification(this.mApplicationContext, this.mOppsTask);
            n.defaults = 0; // cancel vibration / sound effects
            this.mNotificationManager.notifyAsUser(null, notificationId, n, UserHandle.ALL);
        }
    }

    protected void oppsCancelPushActivity() {

        if (this.mOppsTask == null) {
            return;
        }
        try {
        synchronized (this.mOppsTask) {

            // cancel Notification
            int notificationId = NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_OPP, this.mOppsTask
                    .getId());
            this.mNotificationManager.cancel(notificationId);

            // cancel Activity
            CancelableActivity.sendCancelActivityIntent(this.mApplicationContext, this.mOppsTask.getId());

            // reject the request => because it's disconnected, the state should
            // be rejected (not rejecting)
            // this.oppsSubmitTask( BluetoothShareTask.STATE_REJECTED );
            OppLog.d("oppsCancelPushActivity: STATE_REJECTED -> STATE_FAILURE");
            this.oppsSubmitTask(BluetoothShareTask.STATE_FAILURE);

        }

        } catch( Exception e ){

			OppLog.d( "oppsCancelPushActivity catch exception, this.oppstask:" + this.mOppsTask );
		}
    }

    /**
     * after user confirm the incoming request, this function will be called to
     * accept or reject the request.
     * 
     * @param isAccept
     */
    public void oppsSubmitTask(int taskSate) {

        if (Options.LL_DEBUG) {

            OppLog.d("oppsSubmitTask for task: " + this.mOppsTask);
        }

        if (this.mOppsTask == null) {

            OppLog.e("current opps task is null => can't submit it");
            return;
        }

        synchronized (this.mOppsTask) {

            if (this.mOppsTask == null) {

                OppLog.i("duplicated submit [Rejecting] opps task (timeout and user) => skip one");
                return;
            }

            // update filename for saving (rename if file already exists)
            // if user reject or miss incoming file request, needn't create
            // empty file in sdcard
            if ((taskSate == BluetoothShareTask.STATE_REJECTING) || (taskSate == BluetoothShareTask.STATE_REJECTED)
                    || (taskSate == BluetoothShareTask.STATE_FAILURE)) {
                OppLog.i("oppsSubmitTask,taskState is rejecting or rejected or failure");
                // this.oppsTask.setData("");
            } else {

                String filename = this.mOppsTask.getData();
                File file = SystemUtils.createNewFileForSaving(filename);
                if (file != null) {
                    String filePath = file.getAbsolutePath();
                    ///M: For multiUser, we should change the storage path to user ID path
                    ///M: /storage/emulated/0 change to /storage/emulated/10
                    filePath = checkPathForMultiUser(filePath);
                    OppLog.d("checkPathForMultiUser" + filePath );
                    this.mOppsTask.setData(filePath);
                }
            }

            // update content provider
            this.mOppsTask.setState(taskSate);
            int count = this.mContentResolver.update(Uri.withAppendedPath(BluetoothShareTaskMetaData.CONTENT_URI, Integer
                    .toString(this.mOppsTask.getId())), this.mOppsTask.getContentValues(), null, null);
            if (count != 1) {

                OppLog.w("oppsSubmitTask(): update task fail: count[" + count + "], id[" + this.mOppsTask.getId() + "]");
            }

            // notify user
            this.notifyOppTask(this.mOppsTask);

            // start service
            this.oppsStartService();

            // reset opps task
            this.mOppsTask = null;
        }
    }

    /**
     * start OPP(server-role) service - register OPP and accept incoming requests.
     */
    public void oppsStartService() {

        // start service to process request
        Intent intent = new Intent(this.mApplicationContext, OppService.class);
        intent.setAction(OppConstants.OppService.ACTION_OPPS_START);
        this.mApplicationContext.startService(intent);
    }

    public void oppsResetTaskState() {

        StringBuilder where = new StringBuilder();

        where.append("( ");
        where.append(BluetoothShareTaskMetaData.TASK_TYPE);
        where.append(" between ? and ?");
        where.append(" ) and ( ");
        where.append(BluetoothShareTaskMetaData.TASK_STATE);
        where.append(" not in ( ?, ? ))");

        final String[] selectionArgs = new String[] {
                Integer.toString(BluetoothShareTask.TYPE_OPPS_GROUP_START),
                Integer.toString(BluetoothShareTask.TYPE_OPPS_GROUP_END),
                Integer.toString(BluetoothShareTask.STATE_SUCCESS), Integer.toString(BluetoothShareTask.STATE_FAILURE)
        };

        mContentResolver.delete(BluetoothShareTaskMetaData.CONTENT_URI, where.toString(), selectionArgs);
    }

    public Uri oppsGetCurrentTask() {

        return this.mOppService.oppsGetCurrentTask();
    }

    /********************************************************************************************
     * Common API
     ********************************************************************************************/

    protected void oppOnServiceStop() {

        // cancel all notification (user action entry point)
        this.cancelAllNotification();

        // finish active Activity (popped dialog)
        CancelableActivity.sendCancelActivityIntent(this.mApplicationContext, CancelableActivity.NULL_CANCEL_ID);
    }

    public void oppAbortDeviceTasks(String bdAddr) {

        if (bdAddr == null) {
            return;
        }

        OppLog.i("oppAbortDeviceTasks(): " + bdAddr);

        Cursor cursor = this.mContentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI, new String[] {
            BluetoothShareTaskMetaData._ID
        }, BluetoothShareTaskMetaData.TASK_PEER_ADDR + " = ? and " + BluetoothShareTaskMetaData.TASK_STATE + " in ( ?, ? )",
                new String[] {
                        bdAddr, Integer.toString(BluetoothShareTask.STATE_ONGOING),
                        Integer.toString(BluetoothShareTask.STATE_PENDING)
                }, null);
        List<Uri> uriList = this.getOppTaskList(cursor);
        for (Uri uri : uriList) {

            OppLog.i("oppAbortDeviceTasks(): aborting task " + uri);
            this.oppAbortTask(uri);
        }
    }

    public BluetoothDevice oppQueryTaskDevice(Uri taskUri) {

        if (taskUri == null) {
            return null;
        }

        Cursor cursor = this.mContentResolver.query(taskUri, new String[] {
            BluetoothShareTaskMetaData.TASK_PEER_ADDR
        }, null, null, null);

        try {
            if (cursor == null || !cursor.moveToFirst()) {

                OppLog.e("oppQueryTask cannot find task for uri: " + taskUri);
                return null;
            }
            String peerAddr = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_ADDR));
            return this.mBluetoothAdapter.getRemoteDevice(peerAddr);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public void oppAbortTask(Uri taskUri) {

        // check service ready
        if (this.mOppService == null) {

            OppLog.e("oppService is null => can't abort task:[" + taskUri + "]");
            return;
        }

        // update pending and ongoing task only ( other states can't be aborted
        // )
        // update content provider: ongoing -> aborting / pending -> aborted
        ContentValues values = new ContentValues();
        values.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_ABORTING);
        int count = this.mContentResolver.update(taskUri, values, BluetoothShareTaskMetaData.TASK_STATE + "="
                + Integer.toString(BluetoothShareTask.STATE_ONGOING), null);
        if (count != 1) {
            values.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_ABORTED);
            count = this.mContentResolver.update(taskUri, values, BluetoothShareTaskMetaData.TASK_STATE + "="
                    + Integer.toString(BluetoothShareTask.STATE_PENDING), null);
        } else {
            OppLog.d("oppAbortTask() => STATE_ABORTING");
        }

        // only execute cancel when task is under proper state
        if (count == 1) {

            OppLog.d("oppAbortTask() => STATE_ABORTING or STATE_ABORTED");

            boolean isOngoing = true;
            BluetoothShareTask task = this.oppQueryTask(taskUri);
            if (task == null) {

                // this shouldn't happen => just update its state to
                // aborting/aborted
                OppLog.e("can't find task for uri[" + taskUri + "] => can't abort this task");
                return;
            } else if (task.isOppcTask()) {

                isOngoing = this.mOppService.oppcAbort(taskUri);
            } else if (task.isOppsTask()) {

                isOngoing = this.mOppService.oppsAbort(taskUri);
            }

            // notify task according to new state: aborting or aborted (or any
            // updated by other thread)
            OppLog.d("try to notify aborting/aborted task: isOngoing[" + isOngoing + "], state[" + task.getState() + "]");
            this.notifyOppTask(task);
        } else {
            // normal case: task finished and user can't cancel it
            OppLog.i("can't find proper task to cancel in db. found[" + count + "] task(s) for Uri[" + taskUri + "]");

            // abnormal case: can't find in db => cancel pending notification (
            // no content )
            BluetoothShareTask task = this.oppQueryTask(taskUri);
            if (task == null) {

                OppLog.w("can't find task to cancel for Uri[" + taskUri + "]");
                this.cancelNotification(Integer.parseInt(taskUri.getLastPathSegment()));
            }
        }
    }

    /**
     * query OppTask via specific Uri
     * 
     * @param taskUri
     * @return
     */
    protected BluetoothShareTask oppQueryTask(Uri taskUri) {

        Cursor cursor = this.mContentResolver.query(taskUri, null, null, null, null);
        try {
            if (cursor == null || !cursor.moveToFirst()) {

                OppLog.e("oppQueryTask cannot find task for uri: " + taskUri);
                return null;
            }
            return new BluetoothShareTask(cursor);
        } finally {

            if (cursor != null) {
                cursor.close();
            }
        }
    }

    public String getDeviceName(String address) {

        return this.mBluetoothAdapter.getRemoteDevice(address).getName();
    }

    public void acquireWakeLock() {

        this.mWakeLock.acquire();
    }

    public void releaeWakeLock() {

        if (this.mWakeLock.isHeld()) {

            this.mWakeLock.release();
        }
    }

    // for call back service from mmi (e.g. cancel)
    protected void setOppService(OppService oppService) {

        this.mOppService = oppService;
    }

    public List<Uri> getOppTaskList(Cursor cursor) {

        if (cursor == null) {
            return Collections.emptyList();
        }

        List<Uri> result = new ArrayList<Uri>(cursor.getCount());
        for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext()) {
            // current task uri
            int id = cursor.getInt(cursor.getColumnIndex(BluetoothShareTaskMetaData._ID));
            result.add(Uri.parse(BluetoothShareTaskMetaData.CONTENT_URI + "/" + id));
        }
        return result;
    }

    /**
     * @param context
     * @param task
     */
    protected void notifyOppTask(BluetoothShareTask task) {

        Notification n;
        int pid = NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_OPP, task.getId());

        // scan downloaded file
        if (task.getState() == BluetoothShareTask.STATE_SUCCESS && task.getDirection() != Direction.out && !task.isHandover()) {

            // request media scan
            OppLog.i("create MediaScanner for newly received file:" + task.getData() + "," + task.getMimeType());
            new MediaScanner(this.mApplicationContext, task.getData(), task.getMimeType(), null, 0);
        }

        switch (task.getState()) {
            case BluetoothShareTask.STATE_ONGOING:
                if (task.isHandover()) {
                    float progress = 0;
                    long totalBytes = task.getTotalBytes();

                    if (totalBytes == -1) {
                        progress = -1;
                    } else {
                        progress = (float) task.getDoneBytes() / totalBytes;
                    }

                    // Let NFC service deal with notifications for this transfer
                    Intent intent = new Intent(OppConstants.ACTION_BT_OPP_TRANSFER_PROGRESS);
                    if (task.getDirection() == Direction.in) {
                        intent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_DIRECTION,
                                OppConstants.DIRECTION_BLUETOOTH_INCOMING);
                    } else {
                        intent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_DIRECTION,
                                OppConstants.DIRECTION_BLUETOOTH_OUTGOING);
                    }
                    intent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_ID, task.getId());
                    intent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_PROGRESS, progress);
                    intent.putExtra(OppConstants.EXTRA_BT_OPP_ADDRESS, task.getPeerAddr());
                    this.mApplicationContext.sendBroadcastAsUser(intent, UserHandle.ALL, OppConstants.HANDOVER_STATUS_PERMISSION);
                } else {
                    n = OppNotificationFactory.getOppOngoingNotification(this.mApplicationContext, task);
                    this.mNotificationManager.notifyAsUser(null, pid, n, UserHandle.ALL);
                }
                break;
            case BluetoothShareTask.STATE_SUCCESS:
            case BluetoothShareTask.STATE_FAILURE:
                // For ICS usability - add toast message as transferring failed
                // via BT
                // Added by mtk04254
                if (task.getState() == BluetoothShareTask.STATE_FAILURE) {
                    String strPeerName = task.getPeerName();
                    if (null == strPeerName) {
                        //if peer name storaged in task structure is null, then get the name again from hashmap
                        String strRefreshName = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(task.getPeerAddr())
                                .getName();
                        if (null != strRefreshName) {
                            strPeerName = strRefreshName;
                        } else {
                            strPeerName = this.mApplicationContext.getString(R.string.bt_oppc_push_toast_unknown_device);
                        }
                    }
                    if (task.getDirection() == Direction.out) {
                        this.mBgRunner.mHandler.sendMessage(this.mBgRunner.mHandler.obtainMessage(MSG_SHOW_TOAST,
                                this.mApplicationContext.getString(R.string.bt_share_mgmt_tab_dialog_message_resend, task
                                        .getObjectName(), strPeerName)));
                    } else if (task.getDirection() == Direction.in) {
                        this.mBgRunner.mHandler.sendMessage(this.mBgRunner.mHandler.obtainMessage(MSG_SHOW_TOAST,
                                this.mApplicationContext.getString(R.string.bt_share_mgmt_tab_dialog_message_recfail, task
                                        .getObjectName(), strPeerName)));
                    }
                }
                // send out TransferPage Intent
                if (!task.isHandover()) {
                    n = BluetoothShareNotification.getShareManagementNotification(this.mApplicationContext);
                    this.mNotificationManager.notifyAsUser(null, NotificationFactory.NID_SHARE_MGMT_NOTIFICATION, n, UserHandle.ALL);
                    CancelableActivity.sendCancelActivityIntent(this.mApplicationContext, task.getId());
                }
                cancelOngoingNotification(task, pid);
                break;

            case BluetoothShareTask.STATE_ABORTING:
            case BluetoothShareTask.STATE_ABORTED:
                // states that need to cancel CancelableActivity
                // (OppCancelActivity)
                if (!task.isHandover()) {
                    CancelableActivity.sendCancelActivityIntent(this.mApplicationContext, task.getId());
                }
                cancelOngoingNotification(task, pid);
                break;

            case BluetoothShareTask.STATE_REJECTING:
            case BluetoothShareTask.STATE_REJECTED:
                cancelOngoingNotification(task, pid);
                break;

            case BluetoothShareTask.STATE_PENDING:
                // n = OppNotificationFactory.getOppPendingNotification(
                // this.applicationContext, task );
                // break;
            default:
                // no notification required
                OppLog.d("cancel notification for unhandled state[" + task.getState() + "] - id:" + pid);
                this.mNotificationManager.cancel(pid);
                return;
        }
    }

    private void cancelOngoingNotification(BluetoothShareTask task, int pid) {
        // states that need to cancel ongoing notification
        if (task.isHandover()) {
            // Deal with handover-initiated transfers separately
            Intent handoverIntent = new Intent(OppConstants.ACTION_BT_OPP_TRANSFER_DONE);
            if (task.getDirection() == Direction.in) {
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_DIRECTION,
                        OppConstants.DIRECTION_BLUETOOTH_INCOMING);
            } else {
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_DIRECTION,
                        OppConstants.DIRECTION_BLUETOOTH_OUTGOING);
            }
            handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_ID, task.getId());
            handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_ADDRESS, task.getPeerAddr());

            if (BluetoothShareTask.STATE_SUCCESS == task.getState()) {
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_STATUS,
                        OppConstants.HANDOVER_TRANSFER_STATUS_SUCCESS);
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_URI, task.getData());
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_MIMETYPE, task.getMimeType());
            } else {
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_STATUS,
                        OppConstants.HANDOVER_TRANSFER_STATUS_FAILURE);
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_MIMETYPE, task.getMimeType());
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_OBJECT_NAME, task.getObjectName());
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_FILE_SIZE, task.getTotalBytes());
                handoverIntent.putExtra(OppConstants.EXTRA_BT_OPP_TRANSFER_DONE_SIZE, task.getDoneBytes());                
            }
            this.mApplicationContext.sendBroadcastAsUser(handoverIntent, UserHandle.ALL, OppConstants.HANDOVER_STATUS_PERMISSION);
        } else {
            this.mNotificationManager.cancel(pid);
        }
    }

    /**
     * cancel notification
     * 
     * @param context
     * @param id
     */
    protected void cancelNotification(int id) {

        this.mNotificationManager.cancel(NotificationFactory.getProfileNotificationId(BluetoothProfile.ID_OPP, id));
    }

    /**
     * cancel all application notifications (cross-profile)
     * 
     * @param context
     */
    protected void cancelAllNotification() {

        this.mNotificationManager.cancelAll();
    }

    private static final int MSG_OPPC_SUBMIT_TASK = 1;

    private static final int MSG_SHOW_TOAST = 2;

    private OppcTaskTransferThread mOppcTaskTransfer;

    public boolean handleMessage(Message msg) {

        switch (msg.what) {
            case MSG_OPPC_SUBMIT_TASK:
                Object[] param = (Object[]) msg.obj;
                mOppcTaskTransfer = new OppcTaskTransferThread("BtOppcTaskTransferThread", new Object[] {
                        param[0], param[1]
                });
                mOppcTaskTransfer.start();
                break;
            case MSG_SHOW_TOAST:
                String message = (String) msg.obj;
                Toast.makeText(mApplicationContext, message, Toast.LENGTH_LONG).show();
                break;
            default:
                break;
        }
        return false;
    }

    static class LooperThread extends Thread {
        private int mThreadPriority;

        private Callback mCallback;

        public Handler mHandler;

        public LooperThread(String name, int threadPriority, Callback callback) {
            super(name);
            this.mThreadPriority = threadPriority;
            this.mCallback = callback;

            // For ALPS00117959
            mHandler = new Handler(this.mCallback);
        }

        @Override
        public void run() {
            Process.setThreadPriority(this.mThreadPriority);
            Looper.prepare();
            // mHandler = new Handler(this.callback);
            Looper.loop();
        }
    }

    public class OppcTaskTransferThread extends Thread {

        private Object[] mParam;

        public OppcTaskTransferThread(String name, Object[] param) {

            super(name);
            this.mParam = param;
        }

        @Override
        public void run() {

            OppLog.d("Oppc Task handler thread start: thread name - " + this.getName());
            Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

            this.oppcHandleTask();

            OppLog.d("Oppc Task handler thread end: thread name - " + this.getName());

        }

        public synchronized void oppcHandleTask() {
            // device info
            long batchTimestamp = System.currentTimeMillis();
            BluetoothDevice device = (BluetoothDevice) mParam[0];
            ArrayList<BluetoothShareTask> cachedTasks = (ArrayList<BluetoothShareTask>) mParam[1];
            String deviceName = device.getName();
            String deviceAddr = device.getAddress();
            for (BluetoothShareTask task : cachedTasks) {

                UriData ud = UriDataUtils.getUriData(mApplicationContext, Uri.parse(task.getObjectUri()));
                if (ud != null) {

                    task.setObjectName(ud.getName());
                    task.setData(ud.getData());
                    task.setTotalBytes(ud.getSize());
                    task.setState(BluetoothShareTask.STATE_PENDING);
                } else {
                    OppLog.w("oppcSubmitTask - invalid task object: " + task.getPrintableString());
                    // can't find object for given Uri (e.g. SDCard unmounted)
                    // task.setObjectName( "object not found" );
                    task.setState(BluetoothShareTask.STATE_FAILURE);
                }

                // insert into ContentProvider
                task.setPeerName(deviceName);
                task.setPeerAddr(deviceAddr);
                task.setCreationDate(batchTimestamp); // batch identifier
                try {
                    Uri newUri = mContentResolver.insert(BluetoothShareTaskMetaData.CONTENT_URI, task.getContentValues());
                    if(newUri != null) {
                        task.setId(Integer.parseInt(newUri.getLastPathSegment()));
                        // notify user
                        notifyOppTask(task);
                    } else {
                        OppLog.w("newUri is null");
                        return;
                    }
                } catch (SQLiteFullException e) {
                    // TODO: handle exception
                    mBgRunner.mHandler.sendMessage(mBgRunner.mHandler.obtainMessage(MSG_SHOW_TOAST,
                            mApplicationContext.getString(R.string.bt_oppc_toast_disk_full)));
                    task.setState(BluetoothShareTask.STATE_CLEARED);

                    OppLog.w("oppcHandleTask::insert to db exception" );
                    e.printStackTrace();

                    cachedTasks.clear();
                    cachedTasks = null;
                    return;
                }

                // if (Options.LL_DEBUG) {

                // OppLog.d( "oppcSubmitTask committed task: [" +
                // newUri.toString() + "]" );
                // }
            }

            // clear cache
            cachedTasks.clear();
            cachedTasks = null;

            // start service to process request
            Intent intent = new Intent(mApplicationContext, OppService.class);
            intent.setAction(OppConstants.OppService.ACTION_OPPC_START);
            mApplicationContext.startService(intent);

        }
    }

}
