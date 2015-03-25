/* Copyright Statement:

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

package com.mediatek.bluetooth.bip;


import com.mediatek.bluetooth.R;

import java.io.File;

import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.PatternMatcher;
import android.os.Environment;

import android.content.Intent;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.database.ContentObserver;
import android.database.Cursor;

import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;

import android.widget.RemoteViews;
import android.widget.Toast;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;

import android.net.Uri;

import android.util.Log;

import android.os.RemoteException;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.IBluetoothBipi;
import android.bluetooth.IBluetoothBipr;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothAdapter;

import android.Manifest.permission;

import com.mediatek.bluetooth.util.SystemUtils;
import com.mediatek.bluetooth.util.NotificationFactory;

import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;
import com.mediatek.bluetooth.share.BluetoothShareNotification;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

public class BipService extends Service {

    private static final String TAG = "BipService";


    public static final int
        MSG_ON_BIPI_ENABLE = 1,
        MSG_ON_BIPI_DISABLE = 2,
        MSG_ON_BIPI_OBEX_AUTHREQ = 3,
        MSG_ON_BIPI_CONNECT = 4,
        MSG_ON_BIPI_GET_CAPABILITY = 5,
        MSG_ON_BIPI_IMAGE_PUSH_START = 6,
        MSG_ON_BIPI_PROGRESS = 7,
        MSG_ON_BIPI_PUSH = 8,
        MSG_ON_BIPI_THUMBNAIL_REQ = 9,
        MSG_ON_BIPI_THUMBNAIL_PUSH_START = 10,
        MSG_ON_BIPI_THUMBNAIL_PUSH = 11,
        MSG_ON_BIPI_DISCONNECT = 12,
        MSG_ON_BIPI_CANCEL = 13,

        MSG_ON_BIPR_ENABLE = 21,
        MSG_ON_BIPR_DISABLE = 22,
        MSG_ON_BIPR_AUTH_REQ = 23,
        MSG_ON_BIPR_OBEX_AUTHREQ = 24,
        MSG_ON_BIPR_CONNECT = 25,
        MSG_ON_BIPR_GET_CAPABILITY_REQ = 26,
        MSG_ON_BIPR_CAPABILITY_RES = 27,
        MSG_ON_BIPR_ACCESS_REQ = 28,
        MSG_ON_BIPR_IMAGE_RECEIVE_START = 29,
        MSG_ON_BIPR_PROGRESS = 30,
        MSG_ON_BIPR_RECEIVE = 31,
        MSG_ON_BIPR_THUMBNAIL_RECEIVE_START = 32,
        MSG_ON_BIPR_THUMBNAIL_RECEIVE = 33,
        MSG_ON_BIPR_DISCONNECT = 34,
	MSG_ON_BIPR_ALWAYS_ACCEPT = 35;




    public static final int
        BIPI_ERROR = -1,
        BIPI_ENABLING = 1,
        BIPI_ENABLED = 2,
        BIPI_CONNECTED = 3,
        BIPI_PUSH_IMG = 4,
        BIPI_PUSH_THUMBNAIL = 5,
        BIPI_DISCONNECTING = 6,
        BIPI_DISCONNECTED = 7,
        BIPI_DISABLING = 8,
        BIPI_DISABLED = 9;


    public static final int
        BIPR_ERROR = -1,
        BIPR_ENABLING = 1,
        BIPR_ENABLED = 2,
        BIPR_CONNECTING = 3,
        BIPR_CONNECTED = 4,
        BIPR_PUSH_IMG = 5,
        BIPR_PUSH_THUMBNAIL = 6,
        BIPR_DISCONNECTING = 7,
        BIPR_DISCONNECTED = 8,
        BIPR_DISABLING = 9,
        BIPR_DISABLED = 10;



    public static final String
        ACTION_BIP_DISABLE = "com.mediatek.bluetooth.bipiservice.action.BIP_DISABLE",

        ACTION_SEND = "com.mediatek.bluetooth.bipiservice.action.SEND",
        ACTION_BIPI_AUTH_INFO = "com.mediatek.bluetooth.bppmanager.action.BIPI_AUTH_INFO",
        ACTION_CANCEL_PENDING = "com.mediatek.bluetooth.bipiservice.action.CANCEL_PENDING",
        ACTION_BIPI_CANCEL = "com.mediatek.bluetooth.bipiservice.action.BIPI_CANCEL",

        ACTION_BIPR_AUTH_INFO = "com.mediatek.bluetooth.bppmanager.action.BIPR_AUTH_INFO",
        ACTION_RECEIVE_ACCEPT = "com.mediatek.bluetooth.bipiservice.action.RECEIVE_ACCEPT",
        ACTION_RECEIVE_REJECT = "com.mediatek.bluetooth.bipiservice.action.RECEIVE_REJECT",
        ACTION_RECEIVE_RESTORE = "com.mediatek.bluetooth.bipiservice.action.RECEIVE_RESTORE",
        ACTION_BIPR_CANCEL = "com.mediatek.bluetooth.bipiservice.action.BIPR_CANCEL",
	ACTION_SEND_BIP_FILES = "com.mediatek.bluetooth.sharegateway.action.ACTION_SEND_BIP_FILES",

        EXTRA_AUTH_PASSWD = "com.mediatek.bluetooth.bipiservice.extra.EXTRA_AUTH_PASSWD",
        EXTRA_AUTH_USERID = "com.mediatek.bluetooth.bipiservice.extra.EXTRA_AUTH_USERID";

//sync with BipTransmitting.java
    public static final int
        NOTIFICATION_INCOMING_REQ = 1,
        NOTIFICATION_TRANSMIT_PROCESSING = 11,
        NOTIFICATION_TRANSMIT_SUCCESS = 12,
        NOTIFICATION_TRANSMIT_FAIL = 13,
        NOTIFICATION_PENDING_JOB = 20,
        NOTIFICATION_RECEIVE_PROCESSING = 21,
        NOTIFICATION_RECEIVE_SUCCESS = 22,
        NOTIFICATION_RECEIVE_FAIL = 23;


//sync with BluetoothProfile.java
    public static final int
        NOTIFICATION_ID_BIPI = 30000000,
        NOTIFICATION_ID_BIPI_PENDING = 32500000,
        NOTIFICATION_ID_BIPR = 35000000,
        NOTIFICATION_ID_BIP_END = 39999999;

    private static final int
        NOTIFICATION_TYPE_DEFAULT = 1,
        NOTIFICATION_TYPE_SILENCE = 2;
   
    private static final int mRequiredThumb = 0;

    private static int mBipiNotificationId =  NOTIFICATION_ID_BIPI;
    private static int mBiprNotificationId =  NOTIFICATION_ID_BIPR;

    private static int mStartId = -1;
    private static boolean mHasStarted = false;
    private static boolean mAlwaysAccept = false;
    private static int  mBipiState = BIPI_DISABLED;
    private static int  mBiprState = BIPR_DISABLED;
 


    private Context mContext;
    private ContentResolver mContentResolver;
    private Cursor mCursor;
    private BipiContentObserver mObserver;
    private NotificationManager mNotificationMgr;
    private BluetoothBipServer mBipServer;
	
    private PowerManager powerManager;
    private WakeLock initiatorWakeLock;
    private WakeLock responderWakeLock;



    private Uri mUri;
    private String mInitiatorObjectPath;
    private String mInitiatorObjectName;
    private String mInitiatorObjectMime;
    private String mInitiatorObjectSize;
    private BipImage mInitiatorImageObject;
    private String mInitiatorRemoteBtAddr;
    private String mInitiatorRemoteDevName;
    private ContentValues mInitiatorValues;
    private Uri mInitiatorJobUri;
    private String mPendingName;

    private String mResponderRemoteBtAddr;
    private String mResponderRemoteDevName;
    private String mResponderObjectName;
    private String mResponderObjectPath;
    private int mResponderObjectSize;
    private int mResponderReceivingSize;
    private ContentValues mResponderValues;
    private Uri mResponderJobUri;
	
    private DatabaseQueryThread mDatabaseQueryThread = null;

    private class DatabaseQueryThread extends Thread {
	public DatabaseQueryThread(){
		}
		
	@Override
	public void run() {		
	        Xlog.v(TAG, "Database Query Thread");
	        mCursor = mContentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI,
	                                         null,
	                                         BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND " +
	                                         BluetoothShareTaskMetaData.TASK_STATE + " = ?",
	                                         new String[]{ Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_START ),
	                                                       Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_END ),
	                                                       Integer.toString( BluetoothShareTask.STATE_PENDING ) },
	                                         BluetoothShareTaskMetaData.TASK_CREATION_DATE);			
	}
    }


    private final IBluetoothBipi.Stub mBluetoothBipiStub = new IBluetoothBipi.Stub() {
        public int getState() throws RemoteException {
            if ( (mBipiState > BIPI_ENABLED && mBipiState < BIPI_DISCONNECTING) ) {
                return BluetoothProfileManager.STATE_CONNECTED;
            }
            else {
                return BluetoothProfileManager.STATE_DISCONNECTED;
            } 
        }

        public BluetoothDevice getConnectedDevice() throws RemoteException {
            BluetoothDevice remoteDevice = null;
            if ( mBipiState > BIPI_ENABLED && mBipiState < BIPI_DISCONNECTING ) {
                remoteDevice  = BluetoothAdapter.getDefaultAdapter().getRemoteDevice( mInitiatorRemoteBtAddr );
            }
            return remoteDevice;
        }

        public boolean disconnect(BluetoothDevice device) throws RemoteException {
 

            Uri deletion = null;
            String id = null;
            String objectName = null;           
            ContentValues values = null;           
            String[] proj={ BluetoothShareTaskMetaData._ID, BluetoothShareTaskMetaData.TASK_OBJECT_FILE };
            Cursor c = mContentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI,
                                             proj,
                                             BluetoothShareTaskMetaData.TASK_PEER_ADDR + "=" + mInitiatorRemoteBtAddr,
                                             null,
                                             BluetoothShareTaskMetaData.TASK_CREATION_DATE);

            if ( null == c ) {
                Xlog.e(TAG, "cusor is null" );
            }
            else {
                if ( c.moveToFirst() ) {
                    do {
                       id = c.getString( c.getColumnIndexOrThrow(BluetoothShareTaskMetaData._ID) );
                       deletion = Uri.withAppendedPath(BluetoothShareTaskMetaData.CONTENT_URI, id );

                       values = extractContentValues(c);
                       values.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_FAILURE);
                       mContentResolver.update( deletion, values, null, null );

                       //TODO: create fail notification
                    } while( c.moveToNext() );
                }
                c.close();
            }           

            if (mBipiState != BIPI_PUSH_IMG) {
                mCallbackHandler.removeMessages(MSG_ON_BIPI_PUSH);

                Intent bIntent = new Intent( BipInitPushConfirmation.ACTION_TIMEOUT );
                sendBroadcast(bIntent);
            }
            mBipiState = BIPI_DISCONNECTING;
            mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);

            return true;
        }

    };
    

    private final IBluetoothBipr.Stub mBluetoothBiprStub = new IBluetoothBipr.Stub() {
        public int getState() throws RemoteException {
            if ( (mBiprState > BIPR_ENABLED && mBipiState < BIPR_DISCONNECTING) ) {
                return BluetoothProfileManager.STATE_CONNECTED;
            }
            else {
                return BluetoothProfileManager.STATE_DISCONNECTED;
            }
        }

        public BluetoothDevice getConnectedDevice() throws RemoteException {
            BluetoothDevice remoteDevice = null;
            if ( mBiprState > BIPR_ENABLED && mBiprState < BIPR_DISCONNECTING ) {
                remoteDevice  = BluetoothAdapter.getDefaultAdapter().getRemoteDevice( mResponderRemoteBtAddr );
            }
            return remoteDevice;
        }

        public boolean disconnect(BluetoothDevice device) throws RemoteException {
            if ( mBiprState != BIPR_PUSH_IMG ) {
                 mCallbackHandler.removeMessages(MSG_ON_BIPR_RECEIVE);

                 mNotificationMgr.cancel(NOTIFICATION_ID_BIP_END);     
                 Intent bIntent = new Intent( BipRespPushConfirmation.ACTION_CANCEL_BY_PEER );
                 sendBroadcast(bIntent);
            }
            mBiprState = BIPR_DISCONNECTING;
            mBipServer.biprDisconnect();
            return true;
        }

    };



    private IntentFilter mFilter = new IntentFilter(Intent.ACTION_MEDIA_EJECT);
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");
            String action = intent.getAction();
            Uri path = null;
            File curStorage = null;

            if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
                path = intent.getData();
                curStorage = new File(SystemUtils.getReceivedFilePath(mContext));
		Xlog.v(TAG, "ACTION_MEDIA_EJECT: curStorage = "+curStorage.getAbsolutePath()+"path = "+path.getPath());
                if( curStorage != null && curStorage.getAbsolutePath().contains(path.getPath()) ){
			mContentResolver.delete(
				BluetoothShareTaskMetaData.CONTENT_URI,
				BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND " + 
				BluetoothShareTaskMetaData.TASK_STATE + " = ?",
						new String[]{
								Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_START ),
								Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_END ),
								Integer.toString( BluetoothShareTask.STATE_PENDING ) 
			} );
                    if ( mBipiState < BIPI_DISCONNECTING && mBipiState > BIPI_ERROR) {
                        //mBipiState = BIPI_ERROR;
                        //mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
                        mCallbackHandler.sendMessage(mCallbackHandler.obtainMessage(MSG_ON_BIPI_PUSH, -1, 0));
                        mBipiState = BIPI_ERROR;
                    } //TODO: delete pending job, notificaiton
                    if ( mBiprState < BIPR_DISCONNECTING && mBiprState > BIPR_ERROR) {
                        mBiprState = BIPR_ERROR;
                        mBipServer.biprDisconnect();
                    }
                }
            }
        }
    };

    private IntentFilter nFilter = new IntentFilter(ACTION_SEND_BIP_FILES);
    private BroadcastReceiver nReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");
            String action = intent.getAction();

            if(action.equals(ACTION_SEND_BIP_FILES)){
		Xlog.v(TAG, "Receive image task");
		Bundle bundle = intent.getExtras();
		Intent mIntent = bundle.getParcelable("Intent");
		if(mIntent == null){
			Xlog.v(TAG, "mIntent == null");
		}
		BipInitEntry mBipInitEntry = new BipInitEntry(mContext, mIntent);
	    }
        }
    };


    private MediaScannerConnection mMediaScannerConnection = null;
/*
    private MediaScannerConnectionClient mMediaScannerClient = new MediaScannerConnectionClient() {
        public void onMediaScannerConnected() {}
        public void onScanCompleted(String path, Uri uri) {}
    };
*/

    private class BipiContentObserver extends ContentObserver {
        public BipiContentObserver() {
            super(new Handler());
        }

        @Override
        public void onChange(boolean selfChange) {
            Xlog.v(TAG, "BipiContentObserver received notification: bipi state=" + mBipiState );
            if (mBipiState == BIPI_DISABLED || mBipiState == BIPI_DISABLING) {
		if(!mHasStarted){
			Xlog.w(TAG, "Service does not start");
			return;
		}
                mBipiState = BIPI_ENABLING;
                mBipServer.bipiEnable();

            }
            //TODO: handle BIPI_DISABLING state
        }
    } //BipiContentObserver end


    private final Handler mCallbackHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Xlog.v(TAG, "Handler(): got msg=" + msg.what);
			if(!mHasStarted){
				Xlog.w(TAG, "Service does not start");
				return;
			}

            switch (msg.what) {
                case MSG_ON_BIPI_ENABLE:
                {
                    Xlog.v(TAG, "BIPI_ENABLE_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "enable error, disable bipi");
                        mBipiState = BIPI_DISABLING;
                        mBipServer.bipiDisable();
                    }
                    else {
                        mBipiState = BIPI_ENABLED;
						acquireWakeLock(initiatorWakeLock);
                        executeJob();
                    }
                }
                break;
                case MSG_ON_BIPI_DISABLE:
                {
                    Xlog.v(TAG, "BIPI_DISABLE_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "disable error");
                    }
                    mBipiState = BIPI_DISABLED;
		    releaseWakeLock(initiatorWakeLock);
                    //TODO: call disable if bipr doesn't enable
                    //mBipServer.disable();
                    //TODO: only stop it when all jobs are done
                    //stopService(new Intent(mContext, BipService.class));
                }
                break;
                case MSG_ON_BIPI_OBEX_AUTHREQ:
                {
                  Xlog.v(TAG, "MSG_ON_BIPI_OBEX_AUTHREQ:" + msg.arg2);

                  Intent intent = new Intent(mContext, BipAuthentication.class);
                  intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
                  intent.putExtra(BipAuthentication.EXTRA_FROM, "BIPI");
                  if ( 1 == msg.arg2 ) {
                      intent.putExtra(BipAuthentication.EXTRA_NEED_USERID, true);
                  }
                  else {
                      intent.putExtra(BipAuthentication.EXTRA_NEED_USERID, false);
                  }
                  startActivity(intent);
                }
                break;
                case MSG_ON_BIPI_CONNECT:
                {
                    Xlog.v(TAG, "BIPI_CONNECT_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "connect error, disable bipi");
                        mBipiState = BIPI_DISABLING;
                        mBipServer.bipiDisable();
                    }
                }
                break;
                case MSG_ON_BIPI_GET_CAPABILITY:
                {
                    Xlog.v(TAG, "BIPI_GET_CAPABILITYT_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "get capability error, disconnect bipi");
                        Toast.makeText(mContext, R.string.bt_bip_toast_connect_fail, Toast.LENGTH_LONG).show();
                        createBipNotification( 0, BipTransmitting.DIALOG_TRANSMISSION_FAIL);
                        mBipiState = BIPI_DISCONNECTING;
                        mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
                    }
                    else {
                        Xlog.v(TAG, "get capability success");

                        
                        Capability remoteCapaObj = (Capability)msg.obj;

                        Xlog.v(TAG, "Version: " + remoteCapaObj.PreferFormat.Version +
                                   "\tEncoding: " + remoteCapaObj.PreferFormat.Encoding +
                                   "\tWidth: " + remoteCapaObj.PreferFormat.Width + 
                                   "\tHeight: " + remoteCapaObj.PreferFormat.Height +
                                   "\tWidth2: " + remoteCapaObj.PreferFormat.Width2 +
                                   "\tHeight2: " + remoteCapaObj.PreferFormat.Height2 +
                                   "\tSize: " + remoteCapaObj.PreferFormat.Size + 
                                   "\tTransform: " + remoteCapaObj.PreferFormat.Transform +
                                   "\tNumFormats: " + remoteCapaObj.NumImageFormats );

                        for(int i=0; i<remoteCapaObj.NumImageFormats; i++)
                            Xlog.v(TAG, "SupEncoding: " + remoteCapaObj.ImageFormats[i].Encoding +
                                       "\tSupWidth: " + remoteCapaObj.ImageFormats[i].Width +
                                       "\tSupHeight: " + remoteCapaObj.ImageFormats[i].Height +
                                       "\tSupWidth2: " + remoteCapaObj.ImageFormats[i].Width2 + 
                                       "\tSupHeight2: " + remoteCapaObj.ImageFormats[i].Height2 +
                                       "\tSupSize: " + remoteCapaObj.ImageFormats[i].Size);


                        //mInitiatorImageObject = new BipImage(mUri, mContext);
                        mInitiatorImageObject = new BipImage(mUri, mInitiatorObjectPath, mInitiatorObjectSize, mInitiatorObjectMime, mContext);



                        if (compareImgFormat(remoteCapaObj, mInitiatorImageObject)) {
                            mBipiState = BIPI_PUSH_IMG;
                            sendStateChangedBroadcast(mContext, false, BIPI_CONNECTED);
                            mBipServer.bipiPushImage(mInitiatorRemoteBtAddr, mInitiatorImageObject);
                        }
                        else {
                            Xlog.w(TAG, "Responder dosen't support this format");

                            Intent intent = new Intent(mContext, BipInitPushConfirmation.class);
                            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
                            startActivity(intent);

                            //timeout 
                            this.sendMessageDelayed(this.obtainMessage(MSG_ON_BIPI_PUSH, -1, 0), 20000);
                        }
                    }
                }
                break;
                case MSG_ON_BIPI_IMAGE_PUSH_START:
                {
                    Toast.makeText( mContext, getString(R.string.bt_bip_toast_sending_object, mInitiatorObjectName), Toast.LENGTH_LONG ).show();
                    Xlog.v(TAG, "BIPI_IMAGE_PUSH_START");
                }
                break;
                case MSG_ON_BIPI_PROGRESS:
                {
                    Xlog.v(TAG, "BIPI_PROGRESS_IND: " + msg.arg1);
                    Xlog.v(TAG, "progress: " + msg.arg2);
                    if (mBipiState == BIPI_PUSH_IMG) {
                        createBipNotification( msg.arg2, BipTransmitting.DIALOG_TRANSMISSION_PROCESSING); 
                    }
                }
                break;
                case MSG_ON_BIPI_PUSH:
                {
                    Xlog.v(TAG, "BIPI_PUSH_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "push error, disconnect bipi");
                        if (mBipiState != BIPI_PUSH_IMG) {
                            Intent bIntent = new Intent( BipInitPushConfirmation.ACTION_TIMEOUT );
                            sendBroadcast(bIntent);
                        }
                        //for unsupporting format, it retruns fail even the transmission is success.
                        createBipNotification( 0, BipTransmitting.DIALOG_TRANSMISSION_FAIL); 
                    }
                    else{
                        //image handler should be parsed back to string, some reponders don't return the image handle
                        Xlog.v(TAG, "image handler: " + msg.arg2);
                        createBipNotification( mInitiatorImageObject.Size, BipTransmitting.DIALOG_TRANSMISSION_SUCCESS); 
                    }

                    //For PTS test:
                    mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
                    mBipiState = BIPI_DISCONNECTING;
                }
                break;
                case MSG_ON_BIPI_THUMBNAIL_REQ:
                {
                    Xlog.v(TAG, "BIPI_THUMBNAIL_REQ");
                    Xlog.v(TAG, "image handler: " + msg.arg2);
                    //parsing the handler to string and push thumbnail according to the handle
                    mBipServer.bipiPushThumbnail(mInitiatorRemoteBtAddr, mInitiatorImageObject);
                    mBipiState = BIPI_PUSH_THUMBNAIL;
                }
                break;
                case MSG_ON_BIPI_THUMBNAIL_PUSH_START:
                {
                    Xlog.v(TAG, "BIPI_THUMBNAIL_PUSH_START");
                }
                break;
                case MSG_ON_BIPI_THUMBNAIL_PUSH:
                {
                    Xlog.v(TAG, "BIPI_THUMBNAIL_PUSH_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "push thumbnail error, disconnect bipi");
                        //for unsupporting format, it retruns fail even the transmission is success.
                        createBipNotification( 0, BipTransmitting.DIALOG_TRANSMISSION_FAIL);
                    }
                    else{
                        //image handler should be parsed back to string, some reponders don't return the image handle
                        Xlog.v(TAG, "image handler: " + msg.arg2);
                        createBipNotification( mInitiatorImageObject.Size, BipTransmitting.DIALOG_TRANSMISSION_SUCCESS);
                    }

                    mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
                    mBipiState = BIPI_DISCONNECTING;
                }
                break;
                case MSG_ON_BIPI_DISCONNECT:
                {
                    Xlog.v(TAG, "BIPI_DISCONNECT_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "disconnect error, disable bipi");
                    }
                    //mBipServer.bipiDisable();
                    //mBipiState = BIPI_DISABLING;
                    sendStateChangedBroadcast(mContext, false, BIPI_DISCONNECTED);
                    try{
                        Thread.sleep(660);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    executeJob(); 
                }
                break;
                case MSG_ON_BIPI_CANCEL:
                {
                    Xlog.v(TAG, "BIPI_CANCEL_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "cancel error, disconnecting...");
                    }
                }
                break;
                //BIPI end

                //BIPR start
                case MSG_ON_BIPR_ENABLE:
                {
                    Xlog.v(TAG, "BIPR_ENABLE_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "enable error, disable bipr");
                        mBipServer.biprDisable();
                        mBiprState = BIPR_DISABLING;
                    }
                    else {
                        mBiprState = BIPR_ENABLED;
                        sendStateChangedBroadcast(mContext, true, BIPR_ENABLED);
                    }
                }
                break;
                case MSG_ON_BIPR_DISABLE:
                {
                    Xlog.v(TAG, "BIPR_DISABLE_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "disable error");
                    }
                    mBiprState = BIPR_DISABLED;
                    //TODO: change the operation to bipr enable if the disable isn't caused by bt off
                    stopService(new Intent(mContext, BipService.class));
                }
                break;
                case MSG_ON_BIPR_AUTH_REQ:
                {
                    Xlog.v(TAG, "BIPR_AUTH_REQ");
                    String[] Str = (String[])msg.obj;
                    mResponderRemoteDevName = Str[0];
                    mResponderRemoteBtAddr = Str[1];
                    if ( null == mResponderRemoteDevName ) {
                        mResponderRemoteDevName = "UNKNOWN DEVICE";
                    }
		    mBiprState = BIPR_CONNECTING;

                    //TODO: issue a notification and let user decides whether it is authorized, call biprAuthorizeRsp(int)
                    mBipServer.biprAuthorizeRsp(1);
                }
                break;
                case MSG_ON_BIPR_OBEX_AUTHREQ:
                {
                    Xlog.v(TAG, "MSG_ON_BIPI_OBEX_AUTHREQ:" + msg.arg2);

                    Intent intent = new Intent(mContext, BipAuthentication.class);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
                    intent.putExtra(BipAuthentication.EXTRA_FROM, "BIPR");
                    if ( 1 == msg.arg2 ) {
                        intent.putExtra(BipAuthentication.EXTRA_NEED_USERID, true);
                    }
                    else {
                        intent.putExtra(BipAuthentication.EXTRA_NEED_USERID, false);
                    }
                    startActivity(intent);
                }
                break;
                case MSG_ON_BIPR_CONNECT://not used
                {
                    Xlog.v(TAG, "BIPR_CONNECT_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "connect error, disable bipi");
                        mBipServer.biprDisable();
                        mBiprState = BIPR_DISABLING;
                    }
                }
                break;
                case MSG_ON_BIPR_GET_CAPABILITY_REQ:
                {
                    //for widcomm start
                    // IOT issue, it may send serveral images consecutivly.
                    this.removeMessages(MSG_ON_BIPR_THUMBNAIL_RECEIVE);
                    if (mBiprNotificationId < NOTIFICATION_ID_BIP_END - 1) {
                        mBiprNotificationId++;
                    }
                    else {
                        mBiprNotificationId = NOTIFICATION_ID_BIPR + 1;
                    }
                    //for widcomm end

                    Xlog.v(TAG, "MSG_ON_BIPR_GET_CAPABILITY");
                    Capability capaObj = new Capability(4);
                    
                    //test code
                    capaObj.PreferFormat.Version = "1.0";
                    capaObj.PreferFormat.Encoding = "JPEG";
                    capaObj.PreferFormat.Width = 1;
                    capaObj.PreferFormat.Height = 1;
                    capaObj.PreferFormat.Width2 = 65535;
                    capaObj.PreferFormat.Height2 = 65535;
                    capaObj.PreferFormat.Size = 20000000;
                    capaObj.PreferFormat.Transform = 0;

                    capaObj.ImageFormats[0].Encoding = "JPEG";
                    capaObj.ImageFormats[0].Width = 1;
                    capaObj.ImageFormats[0].Height = 1;
                    capaObj.ImageFormats[0].Width2 = 65535;
                    capaObj.ImageFormats[0].Height2 = 65535;
                    capaObj.ImageFormats[0].Size = 20000000;
          
                    capaObj.ImageFormats[1].Encoding = "PNG";
                    capaObj.ImageFormats[1].Width = 1;
                    capaObj.ImageFormats[1].Height = 1;
                    capaObj.ImageFormats[1].Width2 = 65535;
                    capaObj.ImageFormats[1].Height2 = 65535;
                    capaObj.ImageFormats[1].Size = 20000000;

                    capaObj.ImageFormats[2].Encoding = "BMP";
                    capaObj.ImageFormats[2].Width = 1;
                    capaObj.ImageFormats[2].Height = 1;
                    capaObj.ImageFormats[2].Width2 = 2000;
                    capaObj.ImageFormats[2].Height2 = 2000;
                    capaObj.ImageFormats[2].Size = 20000000;

                    capaObj.ImageFormats[3].Encoding = "GIF";
                    capaObj.ImageFormats[3].Width = 1;
                    capaObj.ImageFormats[3].Height = 1;
                    capaObj.ImageFormats[3].Width2 = 2000;
                    capaObj.ImageFormats[3].Height2 = 2000;
                    capaObj.ImageFormats[3].Size = 20000000;


                    mBipServer.biprGetCapabilityRsp(1, capaObj);
                }
                break;
                case MSG_ON_BIPR_CAPABILITY_RES:
                {
                    Xlog.v(TAG, "MSG_ON_BIPR_CAPABILITY_RES");
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "issue capability response error");
                    }
                    else {


                    }
                }
                break;
                case MSG_ON_BIPR_ACCESS_REQ:
                {
                    Xlog.v(TAG, "MSG_ON_BIPR_ACCESS_REQ");

                    mResponderObjectName = (String)msg.obj;
                    mResponderObjectSize = msg.arg2; //PTS 4 and IVT return 0

                    if( false == Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED) ){
                        Toast.makeText( mContext, R.string.bt_bip_toast_storage_unmounted, Toast.LENGTH_LONG ).show();
                        mBiprState = BIPR_DISCONNECTING;
                        mBipServer.biprDisconnect();
                    }
                    else
                    {
                    	String recvPath; 
            			File recvDirectory;
						
						recvPath = SystemUtils.getReceivedFilePath(mContext);
			            recvDirectory = new File( recvPath );
			            if( !recvDirectory.exists() )
			            {
			                recvDirectory.mkdirs();
			            }
						mResponderObjectPath = recvPath;
						
                        mResponderValues = new ContentValues();
 
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_TYPE, BluetoothShareTask.TYPE_BIPR_PUSH );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_PENDING );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_RESULT, 0 );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_OBJECT_NAME, mResponderObjectName );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_OBJECT_URI, "xxx" );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_OBJECT_FILE, mResponderObjectPath + "/" +
                                                                                mResponderObjectName);
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_MIMETYPE, "image/*" );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_PEER_NAME, mResponderRemoteDevName );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_PEER_ADDR, mResponderRemoteBtAddr );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_TOTAL_BYTES, mResponderObjectSize );
                        mResponderValues.put( BluetoothShareTaskMetaData.TASK_DONE_BYTES, 0 );

                        mResponderJobUri = mContentResolver.insert( BluetoothShareTaskMetaData.CONTENT_URI, mResponderValues );

			if(mAlwaysAccept){
				biprAccept();
			}else{
	                        createBipNotification( msg.arg2/1024, NOTIFICATION_INCOMING_REQ );
	                        this.sendMessageDelayed(this.obtainMessage(MSG_ON_BIPR_RECEIVE, -1, 0), 20000);
			}
                    }
                }
                break;
                case MSG_ON_BIPR_IMAGE_RECEIVE_START:
                {
                    Xlog.v(TAG, "MSG_ON_BIPR_IMAGE_RECEIVE_START");
                    mMediaScannerConnection.connect(); 
					acquireWakeLock(responderWakeLock);
                }
                break;
                case MSG_ON_BIPR_PROGRESS:
                {
                    Xlog.v(TAG, "BIPR_PROGRESS_IND");
                    Xlog.v(TAG, "object length: " + msg.arg1);
                    Xlog.v(TAG, "progress: " + msg.arg2);
                    mResponderObjectSize = msg.arg1;
                    mResponderReceivingSize = msg.arg2;
                    if (mBiprState == BIPR_PUSH_IMG) {
                        createBipNotification( msg.arg2, BipTransmitting.DIALOG_RECEIVE_PROCESSING);
                    }
                }
                break;
                case MSG_ON_BIPR_RECEIVE:
                {
                    Xlog.v(TAG, "BIPR_RECEIVE_CNF: " + msg.arg1);
		    mCallbackHandler.removeMessages(MSG_ON_BIPR_RECEIVE);
		    releaseWakeLock(responderWakeLock);
		    mCallbackHandler.sendEmptyMessageDelayed(MSG_ON_BIPR_ALWAYS_ACCEPT, 20000);
                    if ( msg.arg1 != 0 ){
                        //timeout or cancel by peer 
                        if ( mBiprState != BIPR_PUSH_IMG ) {
                            mNotificationMgr.cancel(NOTIFICATION_ID_BIP_END);

                            Intent bIntent = new Intent( BipRespPushConfirmation.ACTION_CANCEL_BY_PEER );
                            sendBroadcast(bIntent);
                        }
                        createBipNotification( 0, BipTransmitting.DIALOG_RECEIVE_FAIL); 
                        Xlog.e(TAG, "receive error, disconnect bipr");
                        mBiprState = BIPR_DISCONNECTING;
                        mBipServer.biprDisconnect();
                    }
                    else{
                        if ( mResponderReceivingSize == mResponderObjectSize || mResponderReceivingSize == 0) {
                            createBipNotification( mResponderObjectSize, BipTransmitting.DIALOG_RECEIVE_SUCCESS);
                        }
                        else {
                            createBipNotification( 0, BipTransmitting.DIALOG_RECEIVE_FAIL);
                        }
                    }
                    //if thumbnail is requested, it can't disconnect here
                    if ( mRequiredThumb == 0 ) {
                    //For PTS test: wait 5secs
                    	if(mResponderRemoteDevName.contains("PTS"))
                         	this.sendEmptyMessageDelayed(MSG_ON_BIPR_THUMBNAIL_RECEIVE, 5000);
                    //    mBipServer.biprDisconnect();
                    //    mBiprState = BIPR_DISCONNECTING;
                    }

                    if ( mMediaScannerConnection.isConnected() ){
                        mMediaScannerConnection.scanFile(
                        //Environment.getExternalStorageDirectory().getPath()+ "/" +  mResponderObjectName , null); 
                        SystemUtils.getReceivedFilePath(mContext)+ "/" +  mResponderObjectName , null); 
                    }
                   // if ( mMediaScannerConnection.isConnected() ){
                   //     mMediaScannerConnection.disconnect();
                   // }

                }
                break;
                case MSG_ON_BIPR_THUMBNAIL_RECEIVE_START:
                {
                    //For PTS test: wait 5secs
                    this.removeMessages(MSG_ON_BIPR_THUMBNAIL_RECEIVE);
                    mBiprState = BIPR_PUSH_THUMBNAIL;

                    Xlog.v(TAG, "MSG_ON_BIPR_THUMBNAIL_RECEIVE_START");
                }
                break;
                case MSG_ON_BIPR_THUMBNAIL_RECEIVE:
                {
                    Xlog.v(TAG, "BIPR_THUMBNAIL_RECEIVE_CNF: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "receive error, disconnect bipr");
                    }

                    mBiprState = BIPR_DISCONNECTING;
                    mBipServer.biprDisconnect();
                }
                break;
                case MSG_ON_BIPR_DISCONNECT:
                {
                    Xlog.v(TAG, "BIPR_DISCONNECT_RESULT: " + msg.arg1);
                    if ( msg.arg1 != 0 ){
                        Xlog.e(TAG, "disconnect error, disable bipr");
                    }
                    //mBipServer.biprDisable();
                    //mBiprState = BIPR_DISABLING;
                    if (mBiprNotificationId < NOTIFICATION_ID_BIP_END - 1) {
                        mBiprNotificationId++;
                    }
                    else {
                        mBiprNotificationId = NOTIFICATION_ID_BIPR + 1;
                    }
                    sendStateChangedBroadcast(mContext, true, BIPR_DISCONNECTED);
                    mBiprState = BIPR_DISCONNECTED;
		    releaseWakeLock(responderWakeLock);
	      	    mCallbackHandler.sendEmptyMessageDelayed(MSG_ON_BIPR_ALWAYS_ACCEPT, 20000);
                }
                break;
                case MSG_ON_BIPR_ALWAYS_ACCEPT:
                {
                    Xlog.v(TAG, "MSG_ON_BIPR_ALWAYS_ACCEPT");
		    mAlwaysAccept = false;
                }
                break;
            }//switch end
        }//handleMessage function end
    };//new Handler() end


    @Override
    public void onCreate() {
        super.onCreate();
        Xlog.v(TAG, "BipService onCreate");

        BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();

        if (btAdapter != null && btAdapter.isEnabled())
        {

                

            String recvPath; 
            File recvDirectory;

            if (!mHasStarted) {

            Xlog.v(TAG, "Creating BIPI Service......");
            mHasStarted = true;
            mBipiState = BIPI_DISABLED;
            mBiprState = BIPR_DISABLED;


            mContext = getApplicationContext();
            mContentResolver = getContentResolver();
            mObserver = new BipiContentObserver();
            mContentResolver.registerContentObserver(BluetoothShareTaskMetaData.CONTENT_URI, true, mObserver);

	    releaseWakeLock(initiatorWakeLock);
	    releaseWakeLock(responderWakeLock);
	    powerManager = (PowerManager)mContext.getSystemService(Context.POWER_SERVICE);

            initiatorWakeLock = powerManager.newWakeLock( PowerManager.PARTIAL_WAKE_LOCK, TAG );
            responderWakeLock = powerManager.newWakeLock( PowerManager.PARTIAL_WAKE_LOCK, TAG );
            //mWakeLock.setReferenceCounted(true);


            mNotificationMgr = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);


            mBipServer = new BluetoothBipServer( mCallbackHandler );
            mBipServer.enable();
            //mBipServer.biprEnable("/mnt/sdcard");
            //mBipServer.biprEnable(Environment.getExternalStorageDirectory().getPath());

            //recvPath = SystemUtils.getReceivedFilePath(mContext);
            //recvDirectory = new File( recvPath );
            //if( !recvDirectory.exists() )
            //{
            //    recvDirectory.mkdirs();
            //}
            //mBipServer.biprEnable(recvPath);


            mMediaScannerConnection = new MediaScannerConnection(this,
                                                                 new MediaScannerConnectionClient() {
                                                                     public void onMediaScannerConnected() {}
                                                                     public void onScanCompleted(String path, Uri uri)
                                                                     {mMediaScannerConnection.disconnect();}
                                                                 } ); 


            mFilter.addDataScheme("file");
            //mFilter.addDataAuthority("*", null);
            //mFilter.addDataPath(Environment.getExternalStorageDirectory().getPath(), PatternMatcher.PATTERN_LITERAL);
            //mFilter.addDataPath(SystemUtils.getReceivedFilePath(mContext), PatternMatcher.PATTERN_LITERAL);
            registerReceiver(mReceiver, mFilter);
            registerReceiver(nReceiver, nFilter);
            //mResponderObjectPath = recvPath;
            //mBipServer.biprEnable(recvPath);
            mBipServer.biprEnable("recvPath");
mBipServer.setRecvPath(SystemUtils.getReceivedFilePath(mContext)); // 20120626 added by mtk71255 for avoiding duplicated name file override
            }
            else {
               Xlog.e(TAG, "ERROR, BIPI has already created");
               stopSelf();
            }
        }
        else
        {

            Xlog.e(TAG, "DISABLE before ENABLE");
            stopSelf();
        }
    }


    @Override
    public IBinder onBind(Intent intent) {
        Xlog.v(TAG, "BipService onBind");

        String action = intent.getAction();
        Xlog.v(TAG, " onBind() action = " + action);

        if (IBluetoothBipi.class.getName().equals(action)) {
            return mBluetoothBipiStub;
        }
        else if (IBluetoothBipr.class.getName().equals(action)) {
            return mBluetoothBiprStub;
        }
        else {
            return null;
        }
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Xlog.v(TAG, "BipService onStartCommand");
        int retCode = super.onStartCommand(intent, flags, startId);

        if (retCode == START_STICKY) {
            mStartId = startId;

           if (intent != null) {
               parseIntent(intent);
           }
        }
        return retCode;
    }



    @Override
    public void onDestroy() {
        Xlog.v(TAG, "BipService onDestroy");
		releaseWakeLock(initiatorWakeLock);
		releaseWakeLock(responderWakeLock);

        if ( mHasStarted )
        {
            mContentResolver.unregisterContentObserver(mObserver);
            unregisterReceiver(mReceiver);
            unregisterReceiver(nReceiver);
            mBipServer.disable();
            sendStateChangedBroadcast(mContext, true, BIPR_DISABLED);
        }
        super.onDestroy();
        closeService();

    }

	private void releaseWakeLock(WakeLock mWakeLock){
		if(mWakeLock != null){
			if (mWakeLock.isHeld()) {
				Xlog.v(TAG, "releaseWakeLock");
				mWakeLock.release();
			}
		}
	}

	private void acquireWakeLock(WakeLock mWakeLock){
		if(mWakeLock != null){
			if (!mWakeLock.isHeld()) {
				Xlog.v(TAG, "acquireWakeLock");
				mWakeLock.acquire();
			}
		}
	}

    private void parseIntent(final Intent intent) {

        String action = intent.getStringExtra("action");
        Xlog.v(TAG, "parseIntent GET action: " + action);

        if ( null == action ) {
            return;
        }
        else if (action.equals(ACTION_SEND)) {
            Xlog.v(TAG, "ACTION_SEND");
            mCallbackHandler.removeMessages(MSG_ON_BIPI_PUSH);
            mBipiState = BIPI_PUSH_IMG;
            sendStateChangedBroadcast(mContext, false, BIPI_CONNECTED);
            mBipServer.bipiPushImage(mInitiatorRemoteBtAddr, mInitiatorImageObject);
        } 
        else if (action.equals(ACTION_BIPI_AUTH_INFO)) {
            Xlog.v(TAG, "ACTION_BIPI_AUTH_INFO");
            AuthInfo aobj = new AuthInfo(false,  intent.getStringExtra(EXTRA_AUTH_USERID), intent.getStringExtra(EXTRA_AUTH_PASSWD));
            mBipServer.bipAuthRsp(aobj, false);
        }
        else if (action.equals(ACTION_BIPI_CANCEL)) {
            Xlog.v(TAG, "ACTION_BIPI_CANCEL");
            mCallbackHandler.removeMessages(MSG_ON_BIPI_PUSH);
            mBipiState = BIPI_DISCONNECTING;
            mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
            //mJobCanceled = true;
        }
        else if (action.equals(ACTION_CANCEL_PENDING)) {
           Xlog.e(TAG, "ACTION_CANCEL_PENDING");
        }
        else if (action.equals(ACTION_BIP_DISABLE)) {
            Xlog.v(TAG, "ACTION_BIP_DISABLE");
            
            if ( mHasStarted )
            {
		mContentResolver.delete(
			BluetoothShareTaskMetaData.CONTENT_URI,
			BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND " + 
			BluetoothShareTaskMetaData.TASK_STATE + " = ?",
					new String[]{
							Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_START ),
							Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_END ),
							Integer.toString( BluetoothShareTask.STATE_PENDING ) 
		} );
		
                if ( mBipiState < BIPI_DISCONNECTING ) {
                    mBipiState = BIPI_ERROR;
                    mBipServer.bipiDisconnect(mInitiatorRemoteBtAddr);
                    sendStateChangedBroadcast(mContext, false, BIPI_CONNECTED);
                } 
                if ( mBiprState < BIPR_DISCONNECTING ) {
                    mBiprState = BIPR_ERROR;
                    mBipServer.biprDisconnect();
                    sendStateChangedBroadcast(mContext, true, BIPR_CONNECTED);
                }
                mBiprState = BIPR_DISABLING;
                mBipServer.biprDisable();
            }
            else
            {
                Xlog.e(TAG, "Disable BIPR before it was enabled");
            }
        }
        else if (action.equals(ACTION_BIPR_AUTH_INFO)) {
            Xlog.v(TAG, "ACTION_BIPR_AUTH_INFO");
            AuthInfo aobj = new AuthInfo(false,  intent.getStringExtra(EXTRA_AUTH_USERID), intent.getStringExtra(EXTRA_AUTH_PASSWD));
            mBipServer.bipAuthRsp(aobj, true);
        }
        else if (action.equals(ACTION_BIPR_CANCEL)) {
            Xlog.v(TAG, "ACTION_BIPR_CANCEL");
            mBiprState = BIPR_DISCONNECTING;
            mBipServer.biprDisconnect();
            //mJobCanceled = true;
        }
        else if (action.equals(ACTION_RECEIVE_ACCEPT)) {
	    mAlwaysAccept = intent.getBooleanExtra("alwaysAccept",false);
            Xlog.v(TAG, "ACTION_RECEIVE_ACCEPT and always Accept is "+ mAlwaysAccept);
			/*
            mCallbackHandler.removeMessages(MSG_ON_BIPR_RECEIVE);
            mBiprState = BIPR_PUSH_IMG;
            sendStateChangedBroadcast(mContext, true, BIPR_CONNECTED);
            mBipServer.biprAccessRsp(1, mRequiredThumb, mResponderObjectPath);
            */
            biprAccept();
        }
        else if (action.equals(ACTION_RECEIVE_REJECT)) {
            Xlog.v(TAG, "ACTION_RECEIVE_REJECT");
            mCallbackHandler.removeMessages(MSG_ON_BIPR_RECEIVE);
            mBiprState = BIPR_DISCONNECTING;
            mBipServer.biprDisconnect();
        } 
        else if (action.equals(ACTION_RECEIVE_RESTORE)){
            Xlog.v(TAG, "ACTION_RECEIVE_REJECT");
            createBipNotificationT( mResponderObjectSize/1024, NOTIFICATION_INCOMING_REQ, NOTIFICATION_TYPE_SILENCE);
        }
    }
	
	private void biprAccept(){
		Xlog.v(TAG, "biprAccept");
		mCallbackHandler.removeMessages(MSG_ON_BIPR_ALWAYS_ACCEPT);
		
		mCallbackHandler.removeMessages(MSG_ON_BIPR_RECEIVE);
		mBiprState = BIPR_PUSH_IMG;
		sendStateChangedBroadcast(mContext, true, BIPR_CONNECTED);
		mBipServer.biprAccessRsp(1, mRequiredThumb, mResponderObjectPath);	    
	}

    private final void closeService() {
        Xlog.v(TAG, "BipiService closeService");
        mHasStarted = false;
        if (stopSelfResult(mStartId)) {
            Xlog.v(TAG, "successfully stopped Bipi service");
        }
    }


    private boolean compareImgFormat(Capability capa, BipImage img) {

        for(int i=0; i<capa.NumImageFormats; i++) {
            if ( capa.ImageFormats[i].Encoding.equals(img.Encoding) ) {
                if ( capa.ImageFormats[i].Width2 == 0 ?
                         (capa.ImageFormats[i].Width == img.Width) :
                         (capa.ImageFormats[i].Width <= img.Width && 
                              (capa.ImageFormats[i].Width2 == 65535 ? true : img.Width <= capa.ImageFormats[i].Width2) ) ) {
                    if( capa.ImageFormats[i].Height2 == 0 ?
                         (capa.ImageFormats[i].Height == img.Height) :
                         (capa.ImageFormats[i].Height < img.Height &&
                              (capa.ImageFormats[i].Height2 == 65535 ? true : img.Height <= capa.ImageFormats[i].Height2) ) ) {
                        return true;  
                    }
                    else {
                        continue;
                    }
                }
                else {
                    continue;
                }
            }
            else {
                continue;
            }
        }
        return false;
    }


    private void executeJob() {

        Xlog.v(TAG, "enter executeJob,Environment.getExternalStorageState() = "+Environment.getExternalStorageState());
	
	if( false == Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED) ){
		Xlog.v(TAG, "SD card unmounted");
		Toast.makeText( mContext, R.string.bt_bip_toast_storage_unmounted, Toast.LENGTH_LONG ).show();
		
		mBipiState = BIPI_DISABLING;
		mBipServer.bipiDisable();
		return;
	}
/*
        mCursor = mContentResolver.query(BluetoothShareTaskMetaData.CONTENT_URI,
                                         null,
                                         BluetoothShareTaskMetaData.TASK_TYPE + " between ? and ? AND " +
                                         BluetoothShareTaskMetaData.TASK_STATE + " = ?",
                                         new String[]{ Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_START ),
                                                       Integer.toString( BluetoothShareTask.TYPE_BIPI_GROUP_END ),
                                                       Integer.toString( BluetoothShareTask.STATE_PENDING ) },
                                         BluetoothShareTaskMetaData.TASK_CREATION_DATE);
*/
	mDatabaseQueryThread = new DatabaseQueryThread();
	mDatabaseQueryThread.start();
	
	try {
		mDatabaseQueryThread.join();
		mDatabaseQueryThread = null;
	} catch (InterruptedException e) {
		Xlog.e(TAG, "mDatabaseQueryThread close error.");
	}


        if ( null == mCursor ) {
            Xlog.e(TAG, "mCursor is null");
        } 
        else if( mCursor.moveToFirst() && mBipiState != BIPI_ERROR ){

            if (mBipiNotificationId < NOTIFICATION_ID_BIPI_PENDING) {
                mBipiNotificationId++;
            }
            else {
                mBipiNotificationId = NOTIFICATION_ID_BIPI + 1;
            }
            int Id = mCursor.getInt( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData._ID) );

            mUri = Uri.parse( mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_URI) ));
            mInitiatorObjectPath = mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_FILE) );
            mInitiatorObjectMime = mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_MIMETYPE) );
            mInitiatorObjectSize = Integer.toString(mCursor.getInt( mCursor.getColumnIndexOrThrow(
                                                                                BluetoothShareTaskMetaData.TASK_TOTAL_BYTES) ));

            mInitiatorObjectName = mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_FILE) );
            mInitiatorObjectName = mInitiatorObjectName.substring(mInitiatorObjectName.lastIndexOf('/')+1);

            mInitiatorRemoteBtAddr = mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_ADDR) ); 
            mInitiatorRemoteDevName = mCursor.getString( mCursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_NAME) );


            mInitiatorJobUri = Uri.withAppendedPath( BluetoothShareTaskMetaData.CONTENT_URI, Integer.toString(Id) );
            mInitiatorValues = extractContentValues(mCursor);

            mInitiatorValues.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_ONGOING);
            mContentResolver.update( mInitiatorJobUri, mInitiatorValues, null, null );


            mCursor.close();

            Xlog.v(TAG, "ID: " + Id );
            Xlog.v(TAG, "Uri: " + mUri );
            Xlog.v(TAG, "Addr: " + mInitiatorRemoteBtAddr );
            Xlog.v(TAG, "Name: " + mInitiatorRemoteDevName );


            Intent bIntent = new Intent( BipTransmitting.ACTION_TRANSMISSION_UPDATE);
            bIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, NOTIFICATION_ID_BIPI_PENDING + Id);
            sendBroadcast(bIntent);


            //temp solution
            //the first job doesn't issue pending notification and the cancel operation fails
            mNotificationMgr.cancel( NOTIFICATION_ID_BIPI_PENDING + Id);
            mBipServer.bipiGetCapabilityReq(mInitiatorRemoteBtAddr);
        }
        else {
            Xlog.w(TAG, "Job table is empty");
            mCursor.close();
            mBipiState = BIPI_DISABLING;
            mBipServer.bipiDisable();
			//releaseWakeLock();
        }
        Xlog.v(TAG, "leave executeJob");

        return;
    }
    
    private void createBipNotification(long sent, int descriptionType) {
        createBipNotificationT(sent, descriptionType, NOTIFICATION_TYPE_DEFAULT );
    }


    private void createBipNotificationT(long sent, int descriptionType, int type) {

        Xlog.v(TAG, "createBipNotification");

        if ( descriptionType == NOTIFICATION_INCOMING_REQ ) {

            Xlog.v(TAG, "NOTIFICATION_INCOMING_REQ");
            String title = null;
            String description = null;

            Notification n = new Notification();


            Intent pIntent =  new Intent( mContext, BipRespPushConfirmation.class );
            pIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
            pIntent.putExtra(BipRespPushConfirmation.EXTRA_DEVICE_NAME, mResponderRemoteDevName );
            pIntent.putExtra(BipRespPushConfirmation.EXTRA_FILE_NAME, mResponderObjectName);
            pIntent.putExtra(BipRespPushConfirmation.EXTRA_FILE_SIZE, Long.toString(sent));

            Xlog.v(TAG, "RemoteDevName:" +  mResponderRemoteDevName + "\tfileName:" + mResponderObjectName + "\tfileSize:" + sent );

            title = getString(R.string.bt_bipr_incoming_request);
            description = getString(R.string.bt_bipr_incoming_request_description, mResponderObjectName);
            n.tickerText = title;
            
            if ( type == NOTIFICATION_TYPE_DEFAULT ) {
                n.defaults |= Notification.DEFAULT_SOUND;
                n.defaults |= Notification.DEFAULT_VIBRATE;
            }
            n.flags |= Notification.FLAG_ONGOING_EVENT;
            n.flags |= Notification.FLAG_AUTO_CANCEL;
            n.icon = android.R.drawable.stat_sys_data_bluetooth;
            n.setLatestEventInfo(mContext, title, description,
                                 PendingIntent.getActivity(mContext,
                                                           NOTIFICATION_ID_BIP_END,
                                                           pIntent,
                                                           PendingIntent.FLAG_ONE_SHOT |PendingIntent.FLAG_UPDATE_CURRENT));

            mNotificationMgr.notify(NOTIFICATION_ID_BIP_END, n);

            return;
        }

        if ( descriptionType == NOTIFICATION_PENDING_JOB ) {

            Xlog.v(TAG, "NOTIFICATION_PENDING_JOB");
            int nId = (int)sent;
            String fileName = mPendingName;
            String title = null;
            String description = null;

            Notification n = new Notification();

            Intent pIntent =  new Intent( mContext, BipTransmitting.class );
            pIntent.setFlags(Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP );
            pIntent.putExtra("action", BipTransmitting.ACTION_TRANSMISSION_UPDATE);
            pIntent.putExtra(BipTransmitting.EXTRA_FILE_NAME, fileName );
            pIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, nId );
            pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_PENDING_JOB);

            Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_PENDING_JOB + "\tfileName:" + fileName);

            title = getString(R.string.bt_bip_app_name);
            description = getString(R.string.bt_bip_pending_job, fileName);

            n.flags |= Notification.FLAG_ONGOING_EVENT;
            n.icon = android.R.drawable.stat_sys_upload_done;
            n.setLatestEventInfo(mContext, title, description,
                                 PendingIntent.getActivity(mContext, nId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT));

            mNotificationMgr.notify(nId , n);

            return; 
        }


        String fileName = null;
        int total = 0;
        Intent bIntent = null;
        if ( descriptionType < NOTIFICATION_PENDING_JOB ) {
            fileName = mInitiatorObjectName;
	    if(mInitiatorObjectSize == null){
            	Xlog.w(TAG, "mInitiatorObjectSize is null");
		total = 0;
	    }else
            	total = Integer.parseInt(mInitiatorObjectSize);
            bIntent = new Intent( BipTransmitting.ACTION_TRANSMISSION_UPDATE);
        }    
        else if ( descriptionType > NOTIFICATION_PENDING_JOB ) {
            fileName = mResponderObjectName;
            total = mResponderObjectSize;
            bIntent = new Intent( BipTransmitting.ACTION_RECEIVING_UPDATE);
        }

        if ( null == bIntent ) {
            Xlog.e(TAG, "broadcast intent is null");
            return;
        }
   

        
         
        int percentage = 0;
        if ( 0 != total ) {
            percentage = (int)(sent *100/ total);
        }
        
        String progress = Integer.toString(percentage).toString() + "%";
        String title = null;
        String description = null;

        bIntent.putExtra(BipTransmitting.EXTRA_FILE_NAME, fileName );
        bIntent.putExtra(BipTransmitting.EXTRA_PERCENTAGE, percentage );

        Notification n = new Notification();
        Notification mgmt_n = null; 
        ContentValues values = null;


        Intent pIntent =  new Intent( mContext, BipTransmitting.class );
        pIntent.setFlags(Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP );
        pIntent.putExtra("action", BipTransmitting.ACTION_TRANSMISSION_UPDATE);

        pIntent.putExtra(BipTransmitting.EXTRA_PERCENTAGE, percentage);
        pIntent.putExtra(BipTransmitting.EXTRA_FILE_NAME, fileName );


        switch ( descriptionType ) {
            case NOTIFICATION_TRANSMIT_PROCESSING:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_PROCESSING);

                Xlog.v(TAG, "NOTIFICATION_TRANSMIT_PROCESSING");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_TRANSMISSION_PROCESSING +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);

                description = getString(R.string.bt_bip_transmitting, fileName);
                pIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, mBipiNotificationId );
                pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_PROCESSING);
/*
                RemoteViews expandedView = new RemoteViews( "com.mediatek.bluetooth",
                                                    R.layout.bt_bip_notification_progress);

                expandedView.setImageViewResource(R.id.appIcon, android.R.drawable.stat_sys_upload);
                expandedView.setTextViewText(R.id.description, description);
                expandedView.setTextViewText(R.id.progress_text, progress);
                expandedView.setProgressBar(R.id.progress_bar, total, sent, false);

                n.icon = android.R.drawable.stat_sys_upload;
                n.contentView = expandedView;
                n.flags |= Notification.FLAG_ONGOING_EVENT;

                pIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, mBipiNotificationId );
                pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_PROCESSING);

                n.contentIntent = PendingIntent.getActivity(mContext, mBipiNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT);
*/
				
				Notification.Builder b = new Notification.Builder( mContext );
				b.setSmallIcon( android.R.drawable.stat_sys_upload );
				b.setWhen( System.currentTimeMillis() );
				b.setTicker( description );
				b.setContentInfo( progress );
				b.setContentTitle( description );
				b.setContentIntent( PendingIntent.getActivity(mContext, mBipiNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT) );
				b.setProgress( 100, percentage, (100<1) );
				b.setOngoing( true );
				n = b.getNotification();
                n.flags |= Notification.FLAG_ONGOING_EVENT;
            } 
            break;
            case NOTIFICATION_TRANSMIT_SUCCESS:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_SUCCESS);

                Xlog.v(TAG, "NOTIFICATION_TRANSMIT_SUCCESS");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_TRANSMISSION_SUCCESS +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);

                mInitiatorValues.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_SUCCESS);
                mContentResolver.update( mInitiatorJobUri, mInitiatorValues, null, null );

                mNotificationMgr.cancel(mBipiNotificationId );

                mgmt_n = BluetoothShareNotification.getShareManagementNotification( mContext );
                mNotificationMgr.notify( NotificationFactory.NID_SHARE_MGMT_NOTIFICATION, mgmt_n );
				
				pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_SUCCESS);
				startActivity(pIntent);
            }
            break;
            case NOTIFICATION_TRANSMIT_FAIL:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_FAIL);

                Xlog.v(TAG, "NNOTIFICATION_TRANSMIT_FAIL");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_TRANSMISSION_FAIL +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);

                mInitiatorValues.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_FAILURE);
                mContentResolver.update( mInitiatorJobUri, mInitiatorValues, null, null );

                mNotificationMgr.cancel(mBipiNotificationId );

                mgmt_n = BluetoothShareNotification.getShareManagementNotification( mContext );
                mNotificationMgr.notify( NotificationFactory.NID_SHARE_MGMT_NOTIFICATION, mgmt_n );
				
				pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_TRANSMISSION_FAIL);
				startActivity(pIntent);
            }
            break;
            case NOTIFICATION_RECEIVE_PROCESSING:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_PROCESSING);

                Xlog.v(TAG, "NOTIFICATION_RECEIVE_PROCESSING");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_RECEIVE_PROCESSING +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);

                description = getString(R.string.bt_bip_receiving, fileName);
                pIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, mBiprNotificationId );
                pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_PROCESSING);
	/*
                RemoteViews expandedView = new RemoteViews( "com.mediatek.bluetooth",
                                                    R.layout.bt_bip_notification_progress);

                expandedView.setImageViewResource(R.id.appIcon, android.R.drawable.stat_sys_download);
                expandedView.setTextViewText(R.id.description, description);
                expandedView.setTextViewText(R.id.progress_text, progress);
                expandedView.setProgressBar(R.id.progress_bar, total, sent, false);

                n.icon = android.R.drawable.stat_sys_download;
                n.contentView = expandedView;
                n.flags |= Notification.FLAG_ONGOING_EVENT;

                pIntent.putExtra(BipTransmitting.EXTRA_NOTIFICATION_ID, mBiprNotificationId );
                pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_PROCESSING);

                n.contentIntent = PendingIntent.getActivity(mContext, mBiprNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT);
*/
				
				Notification.Builder b = new Notification.Builder( mContext );
				b.setSmallIcon( android.R.drawable.stat_sys_download );
				b.setWhen( System.currentTimeMillis() );
				b.setTicker( description );
				b.setContentInfo( progress );
				b.setContentTitle( description );
				b.setContentIntent( PendingIntent.getActivity(mContext, mBiprNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT) );
				b.setProgress( 100, percentage, (100<1) );
				b.setOngoing( true );
				n = b.getNotification();
                n.flags |= Notification.FLAG_ONGOING_EVENT;
            }
            break; 
            case NOTIFICATION_RECEIVE_SUCCESS:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_SUCCESS);

                Xlog.v(TAG, "NOTIFICATION_RECEIVE_SUCCESS");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_RECEIVE_SUCCESS +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);


                mResponderValues.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_SUCCESS);
                mContentResolver.update( mResponderJobUri, mResponderValues, null, null );

                mNotificationMgr.cancel(mBiprNotificationId );

                mgmt_n = BluetoothShareNotification.getShareManagementNotification( mContext );
                mNotificationMgr.notify( NotificationFactory.NID_SHARE_MGMT_NOTIFICATION, mgmt_n );
				
				pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_SUCCESS);
				startActivity(pIntent);


            }
            break;
            case NOTIFICATION_RECEIVE_FAIL:
            {
                bIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_FAIL);

                Xlog.v(TAG, "NNOTIFICATION_RECEIVE_FAIL");
                Xlog.v(TAG, "mDialogType:" + BipTransmitting.DIALOG_RECEIVE_FAIL +
                       "\tmPercentage:" + percentage + "\tfileName:" + fileName);
                mResponderValues.put(BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_FAILURE);
                mContentResolver.update( mResponderJobUri, mResponderValues, null, null );

                mNotificationMgr.cancel(mBiprNotificationId );

                mgmt_n = BluetoothShareNotification.getShareManagementNotification( mContext );
                mNotificationMgr.notify( NotificationFactory.NID_SHARE_MGMT_NOTIFICATION, mgmt_n );
				
				pIntent.putExtra(BipTransmitting.EXTRA_DIALOG_TYPE, BipTransmitting.DIALOG_RECEIVE_FAIL);
				startActivity(pIntent);

            }
            break;
        } //switch (descriptionType) end

        Xlog.v(TAG, "SEND broadcast to DIALOG");
        sendBroadcast(bIntent);
        if ( descriptionType == NOTIFICATION_RECEIVE_PROCESSING ) {
            mNotificationMgr.notify(mBiprNotificationId, n);
        }
        else if ( descriptionType == NOTIFICATION_TRANSMIT_PROCESSING ) {        
            mNotificationMgr.notify(mBipiNotificationId, n);
        }
    } // createBipNotification end


    private void sendStateChangedBroadcast( Context context, boolean isResponder , int state ){
       
        Intent intent = null;

        switch (state) {
            case BIPR_CONNECTED:
            { 
                intent = new Intent(BluetoothProfileManager.ACTION_STATE_CHANGED);
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_CONNECTED);
                intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_DISCONNECTED);
                break;
            }
            case BIPR_DISCONNECTED:
            {
                intent = new Intent(BluetoothProfileManager.ACTION_STATE_CHANGED);
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_DISCONNECTED);
                intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_CONNECTED);
                break;
            }
            case BIPR_ENABLED:
            {
                intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_ENABLED);
                intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_ENABLING);
                break;
            }
            case BIPR_DISABLED:
            {
                intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_DISABLED);
                intent.putExtra(BluetoothProfileManager.EXTRA_PREVIOUS_STATE, BluetoothProfileManager.STATE_DISABLING);
                break;
            }
        }
        
        if ( null != intent ) {
     
            if( isResponder ){
                intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, BluetoothProfileManager.Profile.Bluetooth_BIP_Responder);
                intent.putExtra(BluetoothDevice.EXTRA_DEVICE,
                            BluetoothAdapter.checkBluetoothAddress(mResponderRemoteBtAddr) ?
                            BluetoothAdapter.getDefaultAdapter().getRemoteDevice(mResponderRemoteBtAddr) : null);
            }   
            else {
                intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, BluetoothProfileManager.Profile.Bluetooth_BIP_Initiator);
                intent.putExtra(BluetoothDevice.EXTRA_DEVICE,
                            BluetoothAdapter.checkBluetoothAddress(mInitiatorRemoteBtAddr) ?
                            BluetoothAdapter.getDefaultAdapter().getRemoteDevice(mInitiatorRemoteBtAddr) : null);
            }  

            context.sendBroadcast(intent, permission.BLUETOOTH);
        }
    }


    private ContentValues extractContentValues( Cursor cursor ){

        ContentValues values = new ContentValues();
 
        values.put(BluetoothShareTaskMetaData._ID,
                   cursor.getInt(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData._ID)));
        values.put(BluetoothShareTaskMetaData.TASK_TYPE,
                   cursor.getInt(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_TYPE)));
        values.put(BluetoothShareTaskMetaData.TASK_STATE,
                   cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_STATE)));
        values.put(BluetoothShareTaskMetaData.TASK_RESULT,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_RESULT) ));
        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_NAME,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_NAME)));
        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_URI,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_URI)));
        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_FILE,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_FILE)));
        values.put(BluetoothShareTaskMetaData.TASK_MIMETYPE,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_MIMETYPE)));
        values.put(BluetoothShareTaskMetaData.TASK_PEER_NAME,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_NAME)));
        values.put(BluetoothShareTaskMetaData.TASK_PEER_ADDR,
                   cursor.getString( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_ADDR)));
        values.put(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES,
                   cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES)));
        values.put(BluetoothShareTaskMetaData.TASK_DONE_BYTES,
                   cursor.getInt( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_DONE_BYTES)));
        values.put(BluetoothShareTaskMetaData.TASK_CREATION_DATE,
                   cursor.getLong( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_CREATION_DATE)));
        values.put(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE,
                   cursor.getLong( cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE)));

        return values;
    }



}
