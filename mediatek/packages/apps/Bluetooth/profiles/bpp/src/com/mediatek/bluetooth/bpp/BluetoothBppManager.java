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

package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;

import java.io.File;

import android.os.IBinder;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.PatternMatcher;
import android.os.Environment;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;

import android.bluetooth.BluetoothDevice;

import android.widget.Toast;
import android.widget.RemoteViews;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import android.net.Uri;

import android.os.RemoteException;
import android.bluetooth.IBluetoothBpp;
import android.bluetooth.BluetoothProfileManager;

import com.mediatek.bluetooth.util.SystemUtils;

//import java.io.IOException;
//import java.util.ArrayList;


public class BluetoothBppManager extends Service {
    private static final String TAG = "BluetoothBppManager";

    public static final String EXTRA_FILE_NAME = "com.mediatek.bluetooth.bppmanager.extra.FILE_NAME";
    public static final String EXTRA_FILE_PATH = "com.mediatek.bluetooth.bppmanager.extra.FILE_PATH";
    public static final String EXTRA_MIME_TYPE = "com.mediatek.bluetooth.bppmanager.extra.MIME_TYPE";
    public static final String EXTRA_FILE_SIZE = "com.mediatek.bluetooth.bppmanager.extra.FILE_SIZE";
    public static final String EXTRA_AUTH_PASSWD = "com.mediatek.bluetooth.bppmanager.extra.AUTH_PASSWD";

    public static final String ACTION_PASS_OBJECT = "com.mediatek.bluetooth.bppmanager.action.PASS_OBJECT";
    public static final String ACTION_GET_PRINTER_ATTR = "com.mediatek.bluetooth.bppmanager.action.GET_PRINTER_ATTR";
    public static final String ACTION_PRINT = "com.mediatek.bluetooth.bppmanager.action.PRINT";
    public static final String ACTION_GET_DEFAULT_VALUE = "com.mediatek.bluetooth.bppmanager.action.GET_DEFAULT_VALUE";
    public static final String ACTION_CANCEL = "com.mediatek.bluetooth.bppmanager.action.CANCEL";
    public static final String ACTION_AUTH_INFO = "com.mediatek.bluetooth.bppmanager.action.AUTH_INFO";

    public static final int MSG_ON_BPP_ENABLE = 1;
    public static final int MSG_ON_BPP_CONNECT_CNF = 2;
    public static final int MSG_ON_BPP_GET_PRINTER_ATTR_CNF = 3;
    public static final int MSG_ON_BPP_CANCEL_CNF = 4;
    public static final int MSG_ON_BPP_PROGRESS_IND = 5;
    public static final int MSG_ON_BPP_JOBSTATUS_IND = 6;
    public static final int MSG_ON_BPP_AUTH_IND = 7;
    public static final int MSG_ON_BPP_PRINT_CNF = 8;
    public static final int MSG_ON_BPP_DISCONNECT_CNF = 9;
    public static final int MSG_ON_BPP_DISCONNECT_IND = 10;
    public static final int MSG_ON_BPP_DISABLE = 11;

    public static final int NOTIFICATION_PRINT_PROCESSING = 1;
    public static final int NOTIFICATION_PRINT_SUCCESS = 2;
    public static final int NOTIFICATION_PRINT_FAIL = 3;


    public static final int
        BPPS_ERROR = -1,
        BPPS_ENABLED = 1,
        BPPS_DISABLED = 2;


    public static final int NOTIFICATION_ID_BPP = 40000000;
    private static int mNotificationId =  NOTIFICATION_ID_BPP; 

    private int mStartId = -1;
    private boolean mHasStarted = false;

    private boolean mJobCanceled = false;
    private boolean mTransContinue = false;
    private boolean mConnected = false;

    private static String mEntryError[]; 


    private static String mFileName = null;
    private static String mFilePath = null;
    private static String mMimeType = null;
    private static String mFileSize = null;

    private static int mProgress = 0;

    private static int mCopies = -1;
    private static String mPaperSize = null;
    private static String mSidesSetting = null;
    private static String mSheetSetting = null;
    private static String mOrientation = null;
    private static String mQuality = null;

    private static Context mContext;
    private static BluetoothDevice mRemoteDev;
    private BluetoothBppServer mBppServer;
    private NotificationManager mNotificationMgr;
    private WakeLock mWakeLock;



    private final IBluetoothBpp.Stub mBluetoothBppStub = new IBluetoothBpp.Stub() {
        public int getState() throws RemoteException {
            return BluetoothProfileManager.STATE_UNKNOWN;
        }

        public BluetoothDevice getConnectedDevice() throws RemoteException {
            return mRemoteDev;
        }

        public boolean disconnect(BluetoothDevice device) throws RemoteException {
            if (device.equals(mRemoteDev)) {
                mBppServer.bppDisconnect();
            }
            return true;
        }

    };



    private void sendStateChangedBroadcast( Context context, int state ){

        Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);

        intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, BluetoothProfileManager.Profile.Bluetooth_BPP_Sender);

        switch (state) {
            case BPPS_ENABLED:
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_ENABLED);
                break;
            case BPPS_DISABLED:
                intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, BluetoothProfileManager.STATE_DISABLED);
                break;
        }
    }






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
                //curStorage = SystemUtils.getExternalStorageDirectory(mFilePath);
                curStorage = new File(SystemUtils.getReceivedFilePath(mContext));

                if( curStorage != null && curStorage.getAbsolutePath().contains(path.getPath()) ){
                    if ( mCopies == -1 ) {
                        Intent intent_update = new Intent(BluetoothBppPrintJobSettings.ACTION_ATTR_UPDATE);
                        intent_update.putExtra(BluetoothBppPrintJobSettings.EXTRA_EXCEPTION, -1);
                        sendBroadcast(intent_update);
                    }
                    else
                    {
                        mCallbackHandler.sendMessage(mCallbackHandler.obtainMessage(MSG_ON_BPP_PRINT_CNF, -1, 0));
                    } 
                    mBppServer.bppDisconnect();
                    mJobCanceled = true;
                }
            }


        }
    };


    private final Handler mCallbackHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
          Xlog.v(TAG, "Handler(): got msg=" + msg.what);

          switch (msg.what) {
              case MSG_ON_BPP_ENABLE:
              {
                  Xlog.v(TAG, "BPP_ENABLE_RESULT: " + msg.arg1);
                  if ( msg.arg1 != 0 ){
                      Xlog.e(TAG, "enable error, stop bpp server");
                      Toast.makeText(mContext, R.string.toast_bpp_enable_fail, Toast.LENGTH_LONG).show();
                      mBppServer.bppDisable();
                      //stopService(new Intent(mContext, BluetoothBppManager.class));
                  } 
                  else {
                      sendStateChangedBroadcast (mContext, BPPS_ENABLED);
                  }
              }
              break;

              case MSG_ON_BPP_DISABLE:
              {
                  Xlog.v(TAG, "BPP_DISABLE_RESULT: " + msg.arg1);
                  if ( msg.arg1 != 0 ){
                      Xlog.e(TAG, "disable error");
                  }
                  mBppServer.disableService();
                  sendStateChangedBroadcast (mContext, BPPS_DISABLED);
                  stopService(new Intent(mContext, BluetoothBppManager.class));
              }
              break;

              case MSG_ON_BPP_CONNECT_CNF:
              {
                  Xlog.v(TAG, "CONNECT_CNF: " + msg.arg1);

                  if ( msg.arg1 != 0 ){
                      Xlog.e(TAG, "Connect fail, stop bpp server");
                      Toast.makeText(mContext, R.string.toast_connect_fail, Toast.LENGTH_LONG).show();
                      mBppServer.bppDisable();
                     // stopService(new Intent(mContext, BluetoothBppManager.class));
                  }
                  else
                  {
                      mConnected = true;
                  }
              }
              break;

         case MSG_ON_BPP_GET_PRINTER_ATTR_CNF:
              {
                  Xlog.v(TAG, "GET_PRINTER_ATTR_CNF: " + msg.arg1);
                  mCopies = -1;
                  if ( msg.arg1 == -1 )
                  {
                      Xlog.e(TAG, "get attributes error");
                      Toast.makeText(mContext, R.string.toast_get_printer_attr_fail, Toast.LENGTH_LONG).show();

                      if ( mConnected == true )
                      {
                          mBppServer.bppDisconnect();
                      }
                  }
                  else if ( msg.arg1 == 1 && !mMimeType.equals("image/jpeg") && !mMimeType.equals("text/x-vcard:3.0") )
                  {
                      Xlog.e(TAG, "special printer: HP Photosmart D7200 series ");
                      Toast.makeText(mContext, mEntryError[2], Toast.LENGTH_LONG).show();

                      if ( mConnected == true )
                      {
                          mBppServer.bppDisconnect();
                      }
                  } 
                  else {
                      //String [] paperSize = {"4 x 6", "A3", "A4"};
                      //String [] sidesSetting = {"One-sided", "Two-sided-long-edge", "Two-sided_short-dege"};
                      //String [] paperPerSheet = {"1", "2", "4", "8"};
                      //String [] orientation = {"Portrait", "Landscape", "reverse-portrait", "reverse-landscape"};
                      //String [] qualitySetting = {"DSS", "Normal", "High"};
                      PrinterAttr printerAttrObj = (PrinterAttr)msg.obj; 
 
                 

                      Intent intent = new Intent(mContext, BluetoothBppPrintJobSettings.class);
                     // intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
                      intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_PAPER_SIZE , printerAttrObj.MediaSize);
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_NUMBER_COPIES, printerAttrObj.MaxCopies);
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_SIDES_SETTING ,printerAttrObj.Sides);
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_SHEET_SETTING , printerAttrObj.MaxNumberup );
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_ORIENTATION_SETTING ,printerAttrObj.Orientations );
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_QUALITY_SETTING , printerAttrObj.Qualities );

                       
                      intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_FILE_NAME , mFileName );
                      startActivity(intent);

                      Intent intent_update = new Intent(BluetoothBppPrintJobSettings.ACTION_ATTR_UPDATE);
                      sendBroadcast(intent_update);
                  }
              }
              break;

              case MSG_ON_BPP_PRINT_CNF:
              {
                  Xlog.v(TAG, "PRINT_DOC_CNF: " + msg.arg1);
                  mTransContinue = false;

                  if ( msg.arg1 != 0 ) {
                      Xlog.e(TAG, "print doc error");

                      Xlog.e(TAG, "print doc error - SEND broadcast to DIALOG");
                      Intent intent = new Intent(BluetoothBppPrinting.ACTION_PRINTING_UPDATE);
                      intent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_FAIL);
                      intent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, 0 );
                      intent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );
                      intent.putExtra(BluetoothBppPrinting.EXTRA_REASON, getString(R.string.reason_nondefine) );
                      sendBroadcast(intent);

                      //temporary release
                      createBppNotification(0,100, 
                                            getString(R.string.reason_nondefine), NOTIFICATION_PRINT_FAIL);
                      //TODO: error handle
                  }
                  else {
                      Xlog.v(TAG, "print doc success");

                      Xlog.v(TAG, "print doc success - SEND broadcast to DIALOG");
                      Intent intent = new Intent(BluetoothBppPrinting.ACTION_PRINTING_UPDATE);
                      intent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_SUCCESS);
                      intent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, 100 );
                      intent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );
                      intent.putExtra(BluetoothBppPrinting.EXTRA_REASON, getString(R.string.reason_nondefine) );
                      sendBroadcast(intent);
                      
                      //temporary release
                      createBppNotification(100,100,
                                            getString(R.string.reason_nondefine), NOTIFICATION_PRINT_SUCCESS);
                  }
                  mBppServer.bppDisconnect();
              }
              break;

              case MSG_ON_BPP_CANCEL_CNF:
              {
                  Xlog.v(TAG, "CANCEL_CNF:" + msg.arg1);

                  if ( msg.arg1 != 0 ) {
                      Xlog.v(TAG, "cancel error");
                      //Xlog.v(TAG, "cancel error, processing disconnect");
                      //TODO: error handle
                  }
                  else {
                      Xlog.v(TAG, "cancel success");
                      //Xlog.v(TAG, "cancel success, processing disconnect");

                  }
                  //disconnect should proceed when cancel operation returns success
                  //mBppServer.bppDisconnect();
              }
              break;

              case MSG_ON_BPP_DISCONNECT_CNF:
              case MSG_ON_BPP_DISCONNECT_IND:
              {
                  Xlog.v(TAG, "DISCONNECT_CNF:" + msg.arg1);

                  if ( msg.arg1 != 0 ) {
                      Xlog.e(TAG, "disconnect error, stop bpp server");
                      //TODO: error handle
                  }
                  else {
                      Xlog.v(TAG, "disconnect success, stop bpp server");
                  }
                  //disable should proceed when disconnect operation returns success
                  mConnected = false;
                  mBppServer.bppDisable();
                  //stopService(new Intent(mContext, BluetoothBppManager.class));
              }
              break;

              case MSG_ON_BPP_PROGRESS_IND:
              {
                  Xlog.v(TAG, "sentdatalength: " + msg.arg1 + "\ttotaldatalength:" + msg.arg2);
                  mTransContinue = true;

                  mProgress = msg.arg1*100/msg.arg2;
                  if (mProgress == 100 )
                      mProgress = 99;

                  Xlog.v(TAG, "print progress ind - SEND broadcast to DIALOG");
                  Intent intent = new Intent(BluetoothBppPrinting.ACTION_PRINTING_UPDATE);
                  //intent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_PROCESSING);
                  intent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );
                  intent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, mProgress );
                  intent.putExtra(BluetoothBppPrinting.EXTRA_REASON, getString(R.string.reason_nondefine) );
                  sendBroadcast(intent);

                  //temporary release
                  
                  if (mProgress == 99) {
                      if ( mJobCanceled == false )
                      {
                          createBppNotification(99, 100,
                                                getString(R.string.reason_nondefine), NOTIFICATION_PRINT_PROCESSING);
                      }
                  }
                  else {
                      if ( mJobCanceled == false )
                      {
                          createBppNotification(msg.arg1, msg.arg2, 
                                                getString(R.string.reason_nondefine), NOTIFICATION_PRINT_PROCESSING);
                      }
                  }
                  

              }
              break;


              case MSG_ON_BPP_JOBSTATUS_IND:
              {
                  Xlog.v(TAG, "jobstatus: " + msg.arg1 );
                  Xlog.v(TAG, "job status indpr ind - SEND broadcast to DIALOG");
                  Intent intent = new Intent(BluetoothBppPrinting.ACTION_PRINTING_UPDATE);
                  intent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, mProgress );
                  //intent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_PROCESSING);
                  intent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );

                  String reason = null;

                  switch (msg.arg1) {
                      case 1:
                          reason = getString(R.string.reason_attention_required);
                      break;
                      case 2:
                          reason = getString(R.string.reason_media_jam);
                      break;
                      case 3:
                          reason = getString(R.string.reason_paused);
                      break;
                      case 4:
                          reason = getString(R.string.reason_door_open);
                      break;
                      case 5:
                          reason = getString(R.string.reason_media_low);
                      break;
                      case 6:
                          reason = getString(R.string.reason_media_empty);
                      break;
                      case 7:
                          reason = getString(R.string.reason_output_area_almost_full);
                      break;
                      case 8:
                          reason = getString(R.string.reason_output_area_full);
                      break;
                      case 9:
                          reason = getString(R.string.reason_marker_supply_low);
                      break;
                      case 10:
                          reason = getString(R.string.reason_marker_supply_empty);
                      break;
                      case 11:
                          reason = getString(R.string.reason_marker_failure);
                      break;
                      default:
                          reason = getString(R.string.reason_nondefine);
                  }

                  intent.putExtra(BluetoothBppPrinting.EXTRA_REASON, reason);
                  sendBroadcast(intent);

                  if ( mJobCanceled == false ) {
                       createBppNotification(mProgress, 100, reason, NOTIFICATION_PRINT_PROCESSING);
                  }


              }
              break;

              case MSG_ON_BPP_AUTH_IND:
              {
                  Xlog.v(TAG, "MSG_ON_BPP_AUTH_IND:" + msg.arg1);

                  Intent intent = new Intent(mContext, BluetoothBppAuthenticating.class);
                  intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_HISTORY);
                  startActivity(intent);
              }
              break;
          }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        Xlog.v(TAG, "BppManager onCreate");

        if (!mHasStarted) {
            Xlog.v(TAG, "Starting BPP mamanger......");
            mHasStarted = true;
            if (mNotificationId < NOTIFICATION_ID_BPP+10000000) {
                mNotificationId++;
            }
            else {
                mNotificationId = NOTIFICATION_ID_BPP;
            }

            mContext = getApplicationContext();

            mNotificationMgr = (NotificationManager)mContext.getSystemService(Context.NOTIFICATION_SERVICE);

            mWakeLock = ((PowerManager)mContext.getSystemService(Context.POWER_SERVICE)).
                                       newWakeLock( PowerManager.PARTIAL_WAKE_LOCK, TAG );
            mWakeLock.acquire();

            mEntryError = mContext.getResources().getStringArray(R.array.bt_bpp_entry_error);

            mFilter.addDataScheme("file");
            mFilter.addDataAuthority("*", null);
            //mFilter.addDataPath("/sdcard", PatternMatcher.PATTERN_LITERAL);
            //mFilter.addDataPath(Environment.getExternalStorageDirectory().getPath(), PatternMatcher.PATTERN_LITERAL);
            mFilter.addDataPath(SystemUtils.getReceivedFilePath(mContext), PatternMatcher.PATTERN_LITERAL);
            registerReceiver(mReceiver, mFilter);

            mBppServer = new BluetoothBppServer(mContext ,mCallbackHandler);

            if (mBppServer.enable() != true) {
                 Xlog.e(TAG, " Bpp Server enable error 1");
                 stopSelf();
            }
        }
        else {

            Xlog.e(TAG, " Bpp Server enable error 2");
            mBppServer.bppDisable();
        }
    }



    @Override
    public IBinder onBind(Intent intent) {
        //Xlog.v(TAG, "BppManager onBind: not support");
        //throw new UnsupportedOperationException("Cannot bind to BluetoothBppManager");
        //return null;
        Xlog.v(TAG, "BppManager onBind");

        return mBluetoothBppStub;
    }






    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Xlog.v(TAG, "BppManager onStartCommand");

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
        Xlog.v(TAG, "BppManager onDestroy");
        if (mTransContinue == true) {
            mNotificationMgr.cancel( mNotificationId );
        }

        if(mWakeLock.isHeld()) {
            mWakeLock.release();
        }

        unregisterReceiver(mReceiver);
        closeService();
        super.onDestroy();
    }


    private void parseIntent(final Intent intent) {
        String action = intent.getStringExtra("action");
        Xlog.v(TAG, "parseIntent GET action: " + action);

        if ( null == action ) {
           Xlog.e(TAG, "action in null" );
        }
        else if (action.equals(ACTION_PASS_OBJECT)) {
            Xlog.v(TAG, "ACTION_PASS_OBJECT");

            mFilePath = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_PATH);
            mMimeType = intent.getStringExtra(BluetoothBppManager.EXTRA_MIME_TYPE);
            mFileSize = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_SIZE);

            if ( null == mFilePath || null == mMimeType || null == mFileSize ) {
                Xlog.e(TAG, "file info error" );
            }
            else {
                mFileName = mFilePath.substring(mFilePath.lastIndexOf('/')+1);
                mFilePath = mFilePath.substring(0, mFilePath.lastIndexOf('/'));
            }

            Xlog.v(TAG, " filePath: " + mFilePath + "\tfileName: " + mFileName + "\tfileSize: " + mFileSize);
            Xlog.v(TAG, " mimeType: " + mMimeType);
        }
        else if (action.equals(ACTION_GET_PRINTER_ATTR)) {
            Xlog.v(TAG, "ACTION_GET_PEINTER_ATTR");

            mFilePath = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_PATH);
            mMimeType = intent.getStringExtra(BluetoothBppManager.EXTRA_MIME_TYPE);
            mFileSize = intent.getStringExtra(BluetoothBppManager.EXTRA_FILE_SIZE);

            mRemoteDev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

            if ( null == mFilePath || null == mMimeType || null == mFileSize ) {
                Xlog.e(TAG, "file info error" );
            }
            else if ( null == mRemoteDev ) {
                Xlog.e(TAG, "mRemoteDev is null" );
            }
            else {

                mFileName = mFilePath.substring(mFilePath.lastIndexOf('/')+1);
                mFilePath = mFilePath.substring(0, mFilePath.lastIndexOf('/'));

                Xlog.v(TAG, " filePath: " + mFilePath + "\tfileName: " + mFileName + "\tfileSize: " + mFileSize);
                Xlog.v(TAG, " mimeType: " + mMimeType);

                Toast.makeText(this, R.string.toast_get_printer_attr, Toast.LENGTH_LONG).show();

                mBppServer.bppGetPrinterAttr(mRemoteDev.getAddress() ,0);
            }
        }
        else if (action.equals(ACTION_PRINT)) {
            Xlog.v(TAG, "ACTION_PRINT");

            mPaperSize =  intent.getStringExtra( BluetoothBppPrintJobSettings.EXTRA_PAPER_SIZE);
            mCopies = intent.getIntExtra( BluetoothBppPrintJobSettings.EXTRA_NUMBER_COPIES, 1);
            mSidesSetting = intent.getStringExtra( BluetoothBppPrintJobSettings.EXTRA_SIDES_SETTING);
            mSheetSetting = intent.getStringExtra(BluetoothBppPrintJobSettings.EXTRA_SHEET_SETTING);
            mOrientation = intent.getStringExtra(BluetoothBppPrintJobSettings.EXTRA_ORIENTATION_SETTING);
            mQuality = intent.getStringExtra(BluetoothBppPrintJobSettings.EXTRA_QUALITY_SETTING);


            Xlog.v(TAG, " paperSize: " + mPaperSize +
                       "\tcopies: " + mCopies +
                       "\tsideSetting: " + mSidesSetting +
                       "\tsheetSetting: " + mSheetSetting +
                       "\torientation: " + mOrientation +
                       "\tquality: " + mQuality );            

            if ( null == mPaperSize || null == mSidesSetting || null == mSheetSetting ||
                 null == mOrientation || null == mQuality ) {
                Xlog.e(TAG, "print settings error" );

                mBppServer.bppDisconnect();
            }
            else {
                PrintObject pobj = new PrintObject(mFilePath, mFileName, mMimeType, mFileSize, true, mCopies, 
                                               mSheetSetting, mSidesSetting, mOrientation, mQuality, mPaperSize);

                mBppServer.bppPrint(mRemoteDev.getAddress(), pobj);
            }
        }
        else if (action.equals(ACTION_GET_DEFAULT_VALUE)) {
            Toast.makeText(this, R.string.toast_get_default_value, Toast.LENGTH_SHORT).show();
            mBppServer.bppGetPrinterAttr(mRemoteDev.getAddress() ,0);
        }
        else if (action.equals(ACTION_AUTH_INFO)) { 
            Xlog.v(TAG, "ACTION_AUTH");
             AuthInfo aobj = new AuthInfo(false, "UserId", intent.getStringExtra(EXTRA_AUTH_PASSWD));
             mBppServer.bppAuthRsp(aobj);
        }
        else if (action.equals(ACTION_CANCEL)) {
            Xlog.v(TAG, "ACTION_CANCEL");
            mBppServer.bppDisconnect();
            //mBppServer.bppCancel();
            mJobCanceled = true;
        }
    }


    private final void closeService() {
        Xlog.v(TAG, "BppManager closeService");
        mHasStarted = false;
        if (stopSelfResult(mStartId)) {
            Xlog.v(TAG, "successfully stopped bpp manager");
        }
    }


    private void createBppNotification(int sent, int total, String reason, int descriptionType) {

        Xlog.v(TAG, "createBppNotification");
      
        int percentage = sent*100 / total;

        String progress = Integer.toString(percentage).toString() + "%";
        String title = null;
        String description = null; 

        Notification n = new Notification();

        Intent pIntent =  new Intent( mContext, BluetoothBppPrinting.class );
        pIntent.setFlags(Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP );
        pIntent.putExtra("action", BluetoothBppPrinting.ACTION_PRINTING_UPDATE);

        pIntent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, percentage);
        pIntent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );
        pIntent.putExtra(BluetoothBppPrinting.EXTRA_REASON, reason );
        pIntent.putExtra(BluetoothBppPrinting.EXTRA_NOTIFICATION_ID, mNotificationId );

        if ( descriptionType == NOTIFICATION_PRINT_PROCESSING ){

            Xlog.v(TAG, "NOTIFICATION_PRINT_PROCESSING");
            Xlog.v(TAG, "mDialogType:" + BluetoothBppPrinting.DIALOG_PRINT_PROCESSING +
                       "\tmPercentage:" + percentage + "\tmFileName:" + mFileName);

            description = getString(R.string.bluetooth_printing, mFileName);
            pIntent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_PROCESSING);
	/*
            RemoteViews expandedView = new RemoteViews( "com.mediatek.bluetooth",
                                                    R.layout.bt_bpp_notification_prompt_dialog);

            expandedView.setImageViewResource(R.id.appIcon, android.R.drawable.stat_sys_upload);
            expandedView.setTextViewText(R.id.description, description);
            expandedView.setTextViewText(R.id.progress_text, progress);
            expandedView.setProgressBar(R.id.progress_bar, total, sent, false);

            n.icon = android.R.drawable.stat_sys_upload;
            n.contentView = expandedView;
            //n.flags |= Notification.FLAG_NO_CLEAR ;
            n.flags |= Notification.FLAG_ONGOING_EVENT;

            pIntent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_PROCESSING);

            n.contentIntent = PendingIntent.getActivity(mContext, mNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT);
*/
			
			Notification.Builder b = new Notification.Builder( mContext );
			b.setSmallIcon( android.R.drawable.stat_sys_upload );
			b.setWhen( System.currentTimeMillis() );
			b.setTicker( description );
			b.setContentInfo( progress );
			b.setContentTitle( description );
			b.setContentIntent(PendingIntent.getActivity(mContext, mNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT));
			b.setProgress( 100, percentage, (100<1) );
			b.setOngoing( true );
			n = b.getNotification();
			n.flags |= Notification.FLAG_ONGOING_EVENT;
        }
        else if ( descriptionType == NOTIFICATION_PRINT_SUCCESS ) {

            Xlog.v(TAG, "NOTIFICATION_PRINT_SUCCESS");
            Xlog.v(TAG, "mDialogType:" + BluetoothBppPrinting.DIALOG_PRINT_SUCCESS +
                       "\tmPercentage:" + percentage + "\tmFileName:" + mFileName);

            title = getString(R.string.app_name);
            description = getString(R.string.printing_successful, mFileName);

            n.icon = android.R.drawable.stat_sys_upload_done;

            pIntent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_SUCCESS);

            n.setLatestEventInfo(mContext, title, description,
                                 PendingIntent.getActivity(mContext, mNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT));
			mNotificationMgr.cancel(mNotificationId);
        }
        else if ( descriptionType == NOTIFICATION_PRINT_FAIL ) {

            Xlog.v(TAG, "NNOTIFICATION_PRINT_FAIL");
            Xlog.v(TAG, "mDialogType:" + BluetoothBppPrinting.DIALOG_PRINT_FAIL +
                       "\tmPercentage:" + percentage + "\tmFileName:" + mFileName);

            title = getString(R.string.app_name);
            description = getString(R.string.printing_fail, mFileName);

            n.icon = android.R.drawable.stat_notify_error;

            pIntent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_FAIL);

            n.setLatestEventInfo(mContext, title, description,
                                 PendingIntent.getActivity(mContext, mNotificationId, pIntent, PendingIntent.FLAG_UPDATE_CURRENT));

			mNotificationMgr.cancel(mNotificationId);
        }

        mNotificationMgr.notify(mNotificationId, n);
    }
}

