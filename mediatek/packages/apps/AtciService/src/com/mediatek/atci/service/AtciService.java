/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 *
 */
package com.mediatek.atci.service;

import android.app.Service;

import android.bluetooth.AtCommandHandler;
import android.bluetooth.AtCommandResult;
import android.bluetooth.AtParser;
import android.bluetooth.BluetoothAdapter;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;

import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.location.LocationManager;
import android.media.AudioManager;

import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.net.Uri;
import android.net.wifi.WifiManager;

import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

//import android.os.SystemProperties;

import android.provider.Settings;
import android.util.Log;

import com.android.internal.os.storage.ExternalStorageFormatter;
import com.android.internal.telephony.ITelephony;
import com.mediatek.common.featureoption.FeatureOption;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.nio.channels.ClosedByInterruptException;
import java.io.InputStream;
import java.io.OutputStream;

import java.util.Arrays;
import java.util.Locale;


/**
 * @author mtk03594
 *
 */
public class AtciService extends Service {
    static final String LOG_TAG = "ATCIJ";
    static final String SOCKET_NAME_ATCI = "atci-serv-fw";
    private static final String EMERGENCY_CALL_ACTION = "android.location.agps.EMERGENCY_CALL";
    private static final boolean DBG = true;

    private static final char[] HEX_EX_STRING = new char[] {0x02,0x03};

    static final int SOCKET_OPEN_RETRY_MILLIS = 4 * 1000;
    static final int ATCI_MAX_BUFFER_BYTES = 1024;

    static final int NUM_ZERO = 0;
    static final int NUM_ONE = 1;
    static final int NUM_TWO = 2;
    static final int NUM_THREE = 3;
    static final int NUM_FOUR = 4;
    static final int NUM_FIVE = 5;
    static final int NUM_SIX = 6;
    static final int NUM_SEVEN = 7;
    static final int NUM_TEN = 10;
    static final int NUM_ELEVEN = 11;
    static final int NUM_TWELVE = 12;
    static final int NUM_SIXTEEN = 16;
    static final int NUM_FORTY_TWO = 42;
    static final int THREAD_SLEEP_ONE = 2000;
    static final int THREAD_SLEEP_TWO = 4000;
    
    static final String ECALL_NUMBER = "112";

    LocalSocket mSocket;
    Thread mReceiverThread;
    AtciReceiver mReceiver;
    AtParser mAtParser;
    Context mContext;
    private WakeLock mWakeLock;  // held while processing an AT command    
    private WakeLock mFullWakeLock;
    private int mEcallState = 0; //The ECALL is in Idle state
    private char mMp3PlayerMode = '0';

    private IFMRadioService mFMRadioService = null;
    private boolean mIsFMRadioPlaying = false;
    public static final int FIRST_FREQUENCY = 901;
    public static final int SECOND_FREQUENCY = 1063;
    public static final int LOWEST_STATION = 875;
    public static final int HIGHEST_STATION = 1080;
    public static final int DEFAULT_FREQUENCY = 1000;
    private int mCurrentStation = DEFAULT_FREQUENCY;
    private boolean mIsFMRadioServiceStarted = false;
    private boolean mIsFMRadioServiceBinded = false;
    private AudioManager mAudioManager = null;
    private boolean mIsFMRadioSeeking = false;
    private boolean mIsFMRadioDoSeek = false;
    private boolean mIsFMRadioSeekUp = false;
    private AtCommandResult mFMRadioAtCommandResult = null;
    private Thread mFMRadioSeekThread = null;
    public static final String TYPE_MSGID = "MSGID";
    public static final String TYPE_SEEK_STATION = "SEEK_STATION";
    public static final int MSGID_SEEK_FINISH = 6;
    public static final int MSGID_SEEK_FAIL = 7;
    private static final int FMRADIO_POWER_OFF = 0;
    private static final int FMRADIO_FIRST_STATION = 1;
    private static final int FMRADIO_SECOND_STATION = 2;
    private static final int FMRADIO_SEEK_UP = 3;
    private static final int FMRADIO_SEEK_DOWN = 4; 
 // frequency is float,in database as int, use to convert
    private static final int FREQUENCY_CONVERT_RATE = 10;  
    private static final int WAIT_TIME = 1000; 
    static {
        System.loadLibrary("atciserv_jni");
    }

    /* (non-Javadoc)
     * @see android.app.Service#onBind(android.content.Intent)
     */
    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        // TODO Auto-generated method stub
        super.onCreate();
        mReceiver = new AtciReceiver();
        mReceiverThread = new Thread(mReceiver, "AtciReceiver");
        mReceiverThread.start();

        mContext = getApplicationContext();
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "AtciService");
        mWakeLock.setReferenceCounted(false);
        mFullWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK |
                        PowerManager.ACQUIRE_CAUSES_WAKEUP, "AtciService");

        //Construct one At command parser
        mAtParser = new AtParser();

        initializeAtParser();
        
        IntentFilter atciIntentFilter = new IntentFilter();
        atciIntentFilter.addAction(EMERGENCY_CALL_ACTION);
        mContext.registerReceiver(mEccStateReceiver, atciIntentFilter);
    }

    private String formatResponse(String res) {
        return String.format("%c%s%c", HEX_EX_STRING[0],res,HEX_EX_STRING[1]);
    }

    protected void initializeAtDB() {
        //AT% Commands Name : INITDB 
        //Description : Apply to all the model used in DB Creation
        //Commands Syntax :  AT%INITDB / AT% INITDB?
        mAtParser.register("%INITDB", new AtCommandHandler() {
            private SQLiteDatabase mDb;
            private static final String AT_DB_TABLE = "at_table";
            private static final String STR_ID = "_id";
            private static final String NAME = "name";
            private static final String DESCRIPTION = "description";

            private AtCommandResult initDB() {
                try {
                    mDb = SQLiteDatabase.create(null);
                    mDb.execSQL("CREATE TABLE IF NOT EXISTS " + AT_DB_TABLE + " (" +
                        STR_ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                        NAME + " TEXT COLLATE LOCALIZED," +
                        DESCRIPTION + " description TEXT" +
                        ");");
                } catch (SQLException e) {
                    Log.e(LOG_TAG, "error when initDB! Exception = " + e);
                }
                
                if (mDb.isOpen()) {
                    mDb.close();
                    return new AtCommandResult("1");
                } else {
                    return new AtCommandResult("0");
                }                
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return initDB();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return initDB();
            }
        });

        //AT% Commands Name : DBCHK 
        //Description : DB Check sum Check
        //Commands Syntax :  AT%DBCHK / AT% DBCHK?
        mAtParser.register("%DBCHK", new AtCommandHandler() {
            private AtCommandResult checkDB() {
                return new AtCommandResult("1");               
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return checkDB();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return checkDB();
            }
        });
    }

    protected void initializeAtOSVer() {
        //AT% Commands Name : OSVER 
        //Description : Smart phone OS version Check
        //Commands Syntax :  AT%OSVER / AT% OSVER?
        mAtParser.register("%OSVER", new AtCommandHandler() {
            private AtCommandResult checkOSVer() {
                String oSVersion = "ERROR";
                try {
                    oSVersion = 
                        SystemProperties.get("ro.build.version.release", "ERROR");
                } catch (IllegalArgumentException e) {
                    Log.e(LOG_TAG, "error when get os version! Exception = " + e);
                }

                return new AtCommandResult(oSVersion);               
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return checkOSVer();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return checkOSVer();
            }
        });
    }

    protected void initializeAtTelephony() {

        //AT% Commands Name : FLIGHT Description : Enter FLIGHT Mode of Phone
        //Commands Syntax :  AT%FLIGHT / AT% FLIGHT? / AT% FLIGHT =? / AT% FLIGHT =
        mAtParser.register("%FLIGHT", new AtCommandHandler() {
            private AtCommandResult isAirplaneModeOn() {
                int mode = Settings.System.getInt(
                               mContext.getContentResolver(), 
                               Settings.System.AIRPLANE_MODE_ON, 
                               0);
                Log.d(LOG_TAG, "Get airplane mode:" + mode);
                String response = formatResponse("" + mode);
                return new AtCommandResult(response);
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return isAirplaneModeOn();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                
                Log.d(LOG_TAG, "args:" + args.length);
                Log.d(LOG_TAG, "args string:" + args[0]);
                
                if (args.length == 1) {
                    char c = 0;
                    boolean enabling;
                    String response = null;

                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "c is " + c);

                    if (c == '0' || c == '1') {
                        Log.d(LOG_TAG, "Set airplane mode:" + c);
                        enabling = ((c == '0') ? false : true);
                        
                        // Change the system setting
                        Settings.System.putInt(
                                       mContext.getContentResolver(), 
                                       Settings.System.AIRPLANE_MODE_ON, 
                                       enabling ? 1 : 0);

                        // Post the intent
                        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
                        intent.putExtra("state", enabling);
                        mContext.sendBroadcast(intent);
                        
                        if (c == '0') {
                            response = formatResponse("[0] FLIGHT Mode OFF");
                        } else {
                            response = formatResponse("[1] FLIGHT Mode ON");
                        }
                        return new AtCommandResult(response);
                    }
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
            });

        //3.4.2 AT%ECALL
        //AT% Commands Name : Emergency CALL
        //Description : do Emergency call from the Mobile to a Measuring
        
        mAtParser.register("%ECALL", new AtCommandHandler() {
            private AtCommandResult isEmgencyCallOn() {
                String response = formatResponse("" + mEcallState);
                return new AtCommandResult(response);
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return isEmgencyCallOn();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return isEmgencyCallOn();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return isEmgencyCallOn();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                String response = null;
                if (args.length == 1) {
                    int callId = -1;
                    boolean ret = true;

                    if (args[0] instanceof Integer) {
                        callId = Integer.parseInt(args[0].toString());
                    }

                    if (callId >= 0 && callId <= 2) {
                        Log.d(LOG_TAG, "Make ECC call with " + callId);
                       
                        //if(FeatureOption.MTK_GEMINI_SUPPORT) {
                            Log.d(LOG_TAG, "FeatureOption.MTK_GEMINI_SUPPORT ");
/*
                            try{
                                if(callId == 0){
                                    final ITelephony iTelephony = ITelephony.Stub.asInterface(
                                        ServiceManager.getService(Context.TELEPHONY_SERVICE));
                                    if(null != iTelephony 
                                        && !iTelephony.isIdleGemini(Phone.GEMINI_SIM_1)){
                                        iTelephony.endCallGemini(Phone.GEMINI_SIM_1);
                                    }else if(null != iTelephony 
                                        && !iTelephony.isIdleGemini(Phone.GEMINI_SIM_2)){
                                        iTelephony.endCallGemini(Phone.GEMINI_SIM_2);
                                    }
                                }else if(callId == 1 || callId == 2){
                                    int simId = 
                                        (callId == 1) ? Phone.GEMINI_SIM_1 : Phone.GEMINI_SIM_2;
                                    Intent intent = new Intent(Intent.ACTION_CALL_EMERGENCY);
                                    intent.setData(Uri.fromParts("tel", "112", null));
                                    intent.putExtra(Phone.GEMINI_SIM_ID_KEY, simId);
                                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                                    mContext.startActivity(intent);
                                }else{
                                     ret = false;
                                }
                            }catch(Exception e){
                                e.printStackTrace();
                                ret = false;
                            }
*/
                        //}else{
                        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
                            try {
                                ITelephony iTelephony = ITelephony.Stub.asInterface(
                                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
                                if (iTelephony != null) {
                                    switch (callId) {
                                    case 0:
                                        if (!iTelephony.isIdle()) {
                                            iTelephony.endCall();
                                        }
                                        break;
                                    case 1:
                                        Intent intent = new Intent(Intent.ACTION_CALL_EMERGENCY);
                                        intent.setData(Uri.fromParts("tel", "112", null));
                                        mContext.startActivity(intent);
                                        break;
                                    default:
                                        ret = false;
                                        break;
                                    }
/*
                                    if (callId == 0) {
                                       if(!iTelephony.isIdle()){
                                            iTelephony.endCall();
                                       }
                                    } else if (callId == 1) {
                                        Intent intent = new Intent(Intent.ACTION_CALL_EMERGENCY);
                                        intent.setData(Uri.fromParts("tel", "112", null));
                                        mContext.startActivity(intent);
                                    } else {
                                       ret = false;
                                    }
*/
                                } else {
                                    ret = false;
                                }
                            } catch (RemoteException er) {
                                er.printStackTrace();
                                ret = false;
                            } catch (NullPointerException enp) {
                                enp.printStackTrace();
                                ret = false;
                            }
                        }
                        
                        //Dial the emergency call
                        if (ret) {
                            if (callId == 0) {
                                response = formatResponse("[0]ECALL OFF");
                            } else if (callId == 1) {
                                response = formatResponse("[1]ECALL ON");
                            } else if (callId == 2) {
                                response = formatResponse("[2]ECALL ON");
                            }                       
                            return new AtCommandResult(response);
                        }
                    }
                }
                
                response = formatResponse("ECALL ERROR");
                return new AtCommandResult(response);
            }
            });

            //3.5.1 AT%BTAD
            //AT% Commands Name : Read/Write BD Address
            //Description : command to read and write BD address
            mAtParser.register("%BTAD", new AtCommandHandler() {
                @Override
                public AtCommandResult handleActionCommand() {
                    return handleReadCommand();
                }
    
                @Override
                public AtCommandResult handleReadCommand() {
                    AtCommandResult result;
                    String addr = readBTAddressNative();
                    if (addr == null) {
                        addr = "BLUETOOTH ADDRESS READ FAIL";
                        result = new AtCommandResult(AtCommandResult.UNSOLICITED);
                        result.addResponse(formatResponse(addr));
                    } else {
                        result = new AtCommandResult(formatResponse(addr));
                    }
                    return result;
                }
    
                @Override
                public AtCommandResult handleTestCommand() {
                    return new AtCommandResult(formatResponse(
                        "AT%BTAD=[BD ADDR : 12 HEX nibble => 6 Bytes]"));
                }
    
                @Override
                public AtCommandResult handleSetCommand(Object[] args) {
                    int ret = 1;
                    AtCommandResult response = null;
                    if (args.length == NUM_TWELVE) {
                        StringBuilder addr = new StringBuilder(NUM_TWELVE);
                        String strAddr;
                        for (Object obj : args) {
                            addr.append(obj);
                        }
                        strAddr = addr.toString().substring(0, NUM_TWELVE);
                        Log.d(LOG_TAG, "Addr=" + strAddr);
                        if (writeBTAddressNative(strAddr)) {
                            ret = -1;
                        }
                    } else {
                        ret = -1;
                    }
                    response = new AtCommandResult(AtCommandResult.UNSOLICITED);
                    if (ret < 0) {
                        response.addResponse(formatResponse("BLUETOOTH ADDRESS WRITE FAIL"));
                    } else {
                        response.addResponse(formatResponse("BLUETOOTH ADDRESS WRITE OK"));
                    }
                    return response;
                }
            });
            //3.5.2 AT%BTTM
            //AT% Commands Name : Bluetooth Test Mode
            //Description : Enter / leave Bluetooth Test Mode
            mAtParser.register("%BTTM", new AtCommandHandler() {
                @Override
                public AtCommandResult handleActionCommand() {
                    return handleReadCommand();
                }
    
                @Override
                public AtCommandResult handleReadCommand() {
                    return new AtCommandResult(formatResponse(
                        queryBTTestModeNative() ? "1" : "2"));
                }
    
                @Override
                public AtCommandResult handleTestCommand() {
                    return handleReadCommand();
                }
    
                @Override
                public AtCommandResult handleSetCommand(Object[] args) {
                    boolean ret = false;
                    AtCommandResult response = null;
                    if (args.length == 1 && (args[0] instanceof Integer)) {
                        int mode = Integer.parseInt(args[0].toString());
                        Log.d(LOG_TAG, "AT%BTTM=" + args[0].toString());
                        if (mode == NUM_THREE) {
                            ret = leaveBTTestModeNative();
                            AudioManager audioManager = 
                                (AudioManager) mContext.getSystemService(
                                Context.AUDIO_SERVICE);
                            audioManager.setParameters("SET_LOOPBACK_TYPE=0");
                            //audioManager.setBluetoothScoOn(false);
                        } else {
                            int offset = -1;
                            String bdaddr;
                            if (mode == 2) {
                                offset = 0;
                            } else if (mode >= NUM_ELEVEN && mode <= NUM_FORTY_TWO) {
                                offset = mode - NUM_TEN;
                            }
                            if (offset >= 0) {
                                bdaddr = String.format("0005c90000%02d",offset);
                                if (bdaddr != null) {
                                    /*
                                    BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
                                    if(adapter.isEnabled()){
                                        Intent intent = new Intent(
                                            BluetoothAdapter.ACTION_REQUEST_ENABLE );
                                        mContext.sendBroadcast(intent, 
                                             android.Manifest.permission.BLUETOOTH);
                                        while(1){
                                            sleep()
                                        }
                                    }
                                    */
                                    ret = enterBTTestModeNative(bdaddr);
                                    if (ret) {
                                        AudioManager audioManager = 
                                            (AudioManager) mContext.getSystemService(
                                             Context.AUDIO_SERVICE);
                                        //audioManager.setBluetoothScoOn(true);
                                        audioManager.setParameters("SET_LOOPBACK_TYPE=31");
                                    }
                                }
                            }
                        }
                    } else {
                        Log.e(LOG_TAG, "arg length is not 1 or arg[0] is not integer");
                    }
                    response = new AtCommandResult(AtCommandResult.UNSOLICITED);
                    if (ret) {
                        response.addResponse(formatResponse("BTTM ERROR"));
                    } else {
                        response.addResponse(formatResponse("BTTM OK"));
                    }
                    return response;
                }
            });
    }
        // 3.4.8 AT%FMRadio
        // AT% Commands Name : FMRadio
        // Description : Enter FM Reception mode

    protected void initializeAtFMRadio() {
        mAtParser.register("%FMR", new AtCommandHandler() {
            private AtCommandResult getState() {
                Log.d("Green", "getFMRadioState");
                String response = formatResponse("" + getFMRadioState());
                return new AtCommandResult(response);
            }

            private int getFMRadioState() {
                if ((null == mFMRadioService) || mIsFMRadioServiceBinded) {
                    Log.i(LOG_TAG, "FMRadio Service is null");
                    return FMRADIO_POWER_OFF;
                } else {
                    if (!isPowerUp()) {
                        Log.i(LOG_TAG, "FMRadio is not power up");
                        return FMRADIO_POWER_OFF;
                    } else {
                        if (mIsFMRadioDoSeek || mIsFMRadioSeeking) {
                            if (mIsFMRadioSeekUp) {
                                return FMRADIO_SEEK_UP;
                            } else {
                                return FMRADIO_SEEK_DOWN;
                            }
                        } else {
                            int frequency = getFrequency();
                            Log.i(LOG_TAG, "FMRadio frequency=" + frequency);
                            if (FIRST_FREQUENCY == frequency) {
                                return FMRADIO_FIRST_STATION;
                            } else if (SECOND_FREQUENCY == frequency) {
                                return FMRADIO_SECOND_STATION;
                            } else {
                                return FMRADIO_POWER_OFF;
                            }
                        }
                    }
                }
            }

            @Override
            public AtCommandResult handleActionCommand() {
                Log.d("Green", "handleActionCommand");
                return getState();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                Log.d("Green", "handleReadCommand");
                return getState();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                Log.d("Green", "handleTestCommand");
                return getState();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                String response = null;
                if (args.length == 1) {
                    int fmradioId = -1;
                    boolean ret = true;

                    if (args[0] instanceof Integer) {
                        fmradioId = Integer.parseInt(args[0].toString());
                    }

                    if (fmradioId >= 0 && fmradioId <= NUM_FOUR) {
                        Log.d(LOG_TAG, "Make ECC call with " + fmradioId);
                         ComponentName componentName = startService(
                             new Intent(AtciService.this,FMRadioService.class));                        
                          if (null == componentName) { 
                              mIsFMRadioPlaying = false; 
                              Log.e(LOG_TAG, "FMRadio: can't start FMRadio service"); 
                              } else {
                                      bindService(new Intent(AtciService.this,FMRadioService.class),
                                              mConnection, Context.BIND_AUTO_CREATE);
                                      
                                      
                            } 
                                  if (ret) {
                                      switch (fmradioId) {
                                      case NUM_ZERO:
                                          mIsFMRadioDoSeek = false;
                                          if (isPowerUp()) {
                                              powerDown();
                                          } else {
                                              Log.i(LOG_TAG, "FMRadio has power down");
                                          }
                                          response = formatResponse("[0]FMR OFF");
                                          break;
                                      case NUM_ONE:
                                          Log.i(LOG_TAG, "FMRadio want to tune 90.1");
                                          mIsFMRadioDoSeek = false;
                                          if (!isPowerUp()) {
                                              powerUp(
                                                  (float)FIRST_FREQUENCY / FREQUENCY_CONVERT_RATE);
                                          } else {
                                              int frequency = getFrequency();
                                              Log.i(LOG_TAG, "FMRadio frequency=" + frequency);
                                              if (FIRST_FREQUENCY != frequency) {
                                                  tune((float) FIRST_FREQUENCY / 
                                                      FREQUENCY_CONVERT_RATE);
                                              } else {
                                                  Log.i(LOG_TAG, "FMRadio has tune to 90.1");
                                              }
                                          }
                                          response = formatResponse("[1]FMR 90.1");
                                          break;
                                      case NUM_TWO:
                                          Log.i(LOG_TAG, "FMRadio want to tune 106.3");
                                          mIsFMRadioDoSeek = false;
                                          if (!isPowerUp()) {
                                              powerUp((float) SECOND_FREQUENCY / 
                                                  FREQUENCY_CONVERT_RATE);
                                          } else {
                                              int frequency = getFrequency();
                                              Log.i(LOG_TAG, "FMRadio frequency=" + frequency);
                                              if (SECOND_FREQUENCY != frequency) {
                                                  tune((float) SECOND_FREQUENCY / 
                                                      FREQUENCY_CONVERT_RATE);
                                              } else {
                                                  Log.i(LOG_TAG, "FMRadio has tune to 106.3");
                                              }
                                          }
                                          response = formatResponse("[2]FMR 106.3");
                                          break;
                                      case NUM_THREE:
                                          mIsFMRadioSeekUp = true;
                                          if (!isPowerUp()) {
                                              int frequency = getFrequency();
                                              powerUp((float) frequency / FREQUENCY_CONVERT_RATE);
                                              seekStation(false, frequency, true);
                                          } else {
                                              int frequency = getFrequency();
                                              seekStation(false, frequency, true);
                                          }
                                          response = formatResponse("[3]FMR SEARCH UP");
                                          break;
                                      case NUM_FOUR:
                                          mIsFMRadioSeekUp = false;
                                          if (!isPowerUp()) {
                                              int frequency = getFrequency();
                                              powerUp((float) frequency / FREQUENCY_CONVERT_RATE);
                                              seekStation(false, frequency, false);
                                          } else {
                                              int frequency = getFrequency();
                                              seekStation(false, frequency, false);
                                          }
                                          response = formatResponse("[4]FMR SEARCH DOWN");
                                          break;
                                      default:
                                          break;
                                      }
                                      return new AtCommandResult(response);
                                       
                                      
                                  }                                                
                         }                        
                    }

                response = formatResponse("FMRX ERROR");
                return new AtCommandResult(response);
            }

        });
     
    }
    /*
         * 3.10.1 AT%LANG
         *    Print the number of Languages(containing dialects) and Strings.
         *    Commands Syntax :  AT%LANG / AT%LANG? / AT%LANG=? / AT%LANG=
         *
         */
    protected void initializeAtLanguage() {
        if (DBG) {
            Log.d(LOG_TAG, "initializeAtLanguage");
        }
        mAtParser.register("%LANG", new AtCommandHandler() {
            /*
                       * Action Commands : 
                       * User types    =>  AT%LANG
                       * Return String :  digit   or   ERROR 
                       * Returns the number of Language(only the number)
                       * -   In case that there is no Language menu or list :  0 
                       * -   In case of the model in which AT%LANG command is 
                       * not implemented :  ERROR 
                       */            
            @Override
            public AtCommandResult handleActionCommand() {
                String[] locales = getAssets().getLocales();
                final int origSize = locales.length;
                
                int num = 0;
                for (int i = 0 ; i < origSize; i++) {
                    String s = locales[i];
                    int len = s.length();
                    if (len == NUM_FIVE) {
                        num++;
                    }
                }                
                if (DBG) {
                    Log.d(LOG_TAG, "AT%LANG Action Commands result=" + num);
                }

                return new AtCommandResult("" + num);
            }
            /*
                       * Query Commands : 
                       * User types    =>  AT%LANG
                       * Returns String of Language.(there is distinction of capital/minuscule)
                       * You should obey the standard of Language name managed by GSM.(Specify
                       * loaded Language to the PDM-requesting report.)
                       * Return String :   "Deutsch", "English"
                       */
            @Override
            public AtCommandResult handleReadCommand() {
                String result = "";
                String[] locales = getAssets().getLocales();
                Arrays.sort(locales);
                
                String lang = "";
                final int origSize = locales.length;
                for (int i = 0 ; i < origSize; i++) {
                    String s = locales[i];
                    int len = s.length();
                    if (len == NUM_FIVE) {
                        String language = s.substring(NUM_ZERO, NUM_TWO);
                        String country = s.substring(NUM_THREE, NUM_FIVE);
                        Locale l = new Locale(language, country);
                        
                        if ("".equals(result)) {
                            result += String.format("\"%s\"", getLangDisplayName(l));
                            lang = language;
                        } else {
                            /* A new language */
                            if (language.compareTo(lang) != 0) {
                                result += String.format(", \"%s\"", getLangDisplayName(l));
                                lang = language;
                            } else if (s.equals("zz_ZZ"))  {                                
                                result += String.format(", \"%s\"", "Pseudo...");
                            }
                        }
                    }
                }
                if (DBG) {
                    Log.d(LOG_TAG, "AT%LANG Query Commands result=" + result);
                }
                return new AtCommandResult(result);
            }            
        });
    
    }    

    private String getLangDisplayName(Locale loc) {
        String s = loc.getDisplayLanguage(loc);
        if (s.length() == 0) {
            return s;
        }

        return s.substring(0, 1).toUpperCase(loc) + s.substring(1);
    }

    protected void initializeAtMp3Play() {
        //private static final String EMERGENCY_CALL_ACTION="android.location.agps.EMERGENCY_CALL";
        //private static final boolean DBG = true;
        //AT% Commands Name : MPT Description : Measuring of MP3 output level & quality through 
        //  Phone MP3 DSP(including Stereo)
        //Commands Syntax :  AT%MPT / AT% MPT? / AT% MPT =? / AT% MPT =
        mAtParser.register("%MPT", new AtCommandHandler() {
            private AtCommandResult queryTestMode() {                
                Mp3Player mp3player = Mp3Player.getInstanceOfMp3Player();
                if ((mMp3PlayerMode > '0' && mMp3PlayerMode <= '5') && mp3player.isIdle()) {
                    mMp3PlayerMode = '0';
                }
                Log.d(LOG_TAG, "Get MPT mMp3PlayerMode:" + mMp3PlayerMode);
                String response = "" + mMp3PlayerMode;
                //formatResponse("" + mMp3PlayerMode + "\nOK!");
                return new AtCommandResult(response);
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return queryTestMode();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return queryTestMode();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return queryTestMode();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                
                Log.d(LOG_TAG, "args:" + args.length);
                Log.d(LOG_TAG, "args string:" + args[0]);
                Mp3Player mp3player = Mp3Player.getInstanceOfMp3Player();
                if (args.length == 1) {
                    char c = 0;
                    String response = null;

                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "c is " + c);
                    
                    if (c >= '0' && c <= '5') {
                        mMp3PlayerMode = c;
                    }
                    
                    if (c  == '0') {
                        mp3player.stopPlayer();    
                    } else {
                        mp3player.startPlayer(c);
                    }    
                    
                    switch(c) {
                    case '0':
                        response = "[0]MP3 OFF";
                        break;
                    case '1':
                        response = "[1]NO SIGNAL";
                        break;
                    case '2':
                        response = "[2]LR";
                        break;
                    case '3':
                        response = "[3]L";
                        break;
                    case '4':
                        response = "[4]R";
                        break;
                    case '5':
                        response = "[5]MULTI LR";
                        break;
                    default:
                        response = "[5]MP3 ERROR";
                    }                     
                    //return new AtCommandResult(formatResponse(response));
                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
            });
           
    }

    /*
     *    3.3.6 AT%NOSLEEP
     *    AT%SLEEP
     *    Set screen always on.
     *    Commands Syntax :  AT%SLEEP / AT%SLEEP? / AT%SLEEP=? / AT%SLEEP=
     */
    protected void initializeAtNoSleep() {
        if (DBG) {
            Log.d(LOG_TAG, "initializeAtSleep");
        }
        mAtParser.register("%NOSLEEP", new AtCommandHandler() {
            private AtCommandResult checkFullWakelcok() {
                if (mFullWakeLock.isHeld()) {
                    return new AtCommandResult("1");
                } else {
                    return new AtCommandResult("0");
                }
            }

            /*   Action Commands :
             *   User types => AT% NOSLEEP
             *   Return String =>0 or 1
             *   If it is possible to enter Sleep Mode, returns"0", 
             *   If it is No Sleep Mode, returns"1".
             */
            @Override
            public AtCommandResult handleActionCommand() {
                return checkFullWakelcok();
            }

            /*   Query Commands :
             *   User types    => AT% NOSLEEP?
             *   Return String => Same as "Action"
             */
            @Override
            public AtCommandResult handleReadCommand() {
                return checkFullWakelcok();
            }

            /*   Range  Commands :
             *   User types => AT% NOSLEEP =?
             *   Return String => Same as "Action"
             */
            @Override
            public AtCommandResult handleTestCommand() {
                return checkFullWakelcok();
            }

            /*   Assign  Commands :
             *   User types    =>  AT% NOSLEEP =0 or AT% NOSLEEP =1
             *   Return String => [0] Sleep Mode or [1] No Sleep Mode 
             */
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "args:" + args.length);
                Log.d(LOG_TAG, "args string:" + args[0]);

                if (args.length == 1) {
                    char c = 0;
                    String response = null;

                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "c is " + c);

                    switch(c) {
                    case '0':
                        if(mFullWakeLock != null && true == mFullWakeLock.isHeld()) {
                            try{
                            mFullWakeLock.release();
                            }catch(RuntimeException re){
                                Log.e(LOG_TAG, "Release full wake lock exception.");
                            }
                        }
                        response = "[0]Sleep Mode";
                        break;
                    case '1':
                        if(false == mFullWakeLock.isHeld()) {
                            mFullWakeLock.acquire();
                        }
                        response = "[1]No Sleep Mode";
                        break;
                    default:
                        response = "NOSLEEP ERROR";
                    }                     

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });
    }    

    private int mCameraOn = 0;
    private int mCameraAVROn = 0;

    protected void initializeAtSystemCall() {
        mAtParser.register("+SN", new AtCommandHandler() {
            @Override
            public AtCommandResult handleActionCommand() {
                String sn = "";
                String response = "";
                //try{ 
                    // sn = ((TelephonyManager)mContext.getSystemService(
                    //        Context.TELEPHONY_SERVICE)).getSN();
                    response = "SN: " + sn + "\r\nOK\r\n";
                //} catch (Exception e) {
                //    response = "ERROR\r\n";
                //}
                return new AtCommandResult(response);
            }
        });

        mAtParser.register("+SHUTDOWN", new AtCommandHandler() {
            @Override
            public AtCommandResult handleActionCommand() {
                String response = "";
                //try {
                    Intent intent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
                    intent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(intent);
                    response = "SHUTDOWN NOW\r\nOK\r\n";
                //} catch (Exception e) {
                //    response = "ERROR\r\n";
                //}
                return new AtCommandResult(response);
            }
        });

        mAtParser.register("+MODEL", new AtCommandHandler() {
            @Override
            public AtCommandResult handleActionCommand() {
                String response = "MODEL: " + Build.MODEL + "\r\nOK\r\n";
                return new AtCommandResult(response);
            }
        });

        mAtParser.register("+POWERKEY", new AtCommandHandler() {
            @Override
            public AtCommandResult handleActionCommand() {
                String response = "OK\r\n";
                //try {
                    BluetoothAdapter.getDefaultAdapter().disable();
                    Settings.Secure.setLocationProviderEnabled(
                             getContentResolver(), 
                             LocationManager.GPS_PROVIDER, 
                             false);
                    WifiManager mWifiManager = 
                        (WifiManager)getSystemService(Context.WIFI_SERVICE);
                    mWifiManager.setWifiEnabled(false);
                //} catch (Exception e) {
                //    response = "ERROR\r\n";
                //}
                return new AtCommandResult(response);
            }
        });

        final StorageEventListener mStorageListener = new StorageEventListener() {
            @Override
            public void onStorageStateChanged(String path, String oldState, String newState) {
                Log.i(LOG_TAG, "Received storage state changed notification that " + path +
                    " changed state from " + oldState + " to " + newState);

                if (path.equals("/mnt/sdcard")) {
                    if (oldState.equals("checking") && newState.equals("mounted")) {
                        mContext.sendBroadcast(new Intent(
                            "android.intent.action.MASTER_CLEAR"));
                    }
                }
            }
        };

        mAtParser.register("+FACTORYRESET", new AtCommandHandler() {
            @Override
            public AtCommandResult handleActionCommand() {
                String response = "OK\r\n";
                //try {
                    Intent intent = new Intent(ExternalStorageFormatter.FORMAT_ONLY);
                    intent.setComponent(ExternalStorageFormatter.COMPONENT_NAME);
                    // Transfer the storage volume to the new intent
                    StorageManager storageManager = 
                        (StorageManager) getSystemService(Context.STORAGE_SERVICE);
                    storageManager.registerListener(mStorageListener);
                    StorageVolume[] volumes = storageManager.getVolumeList();
                    int i = 0;
                    StorageVolume storageVolume = null;
                    for (i = 0; i < volumes.length; i++) {
                        if (volumes[i].getPath().equals("/mnt/sdcard")) {
                            storageVolume = volumes[i];
                            break;
                        }
                    }
                    intent.putExtra(StorageVolume.EXTRA_STORAGE_VOLUME, storageVolume);
                    startService(intent);
                //} catch (Exception e) {
                //    e.printStackTrace();
                //    response = "ERROR\r\n";
                //}
                return new AtCommandResult(response);
            }
        });

        mAtParser.register("+WITOF", new AtCommandHandler() {
            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                Log.d(LOG_TAG, "args:" + args.length);
                Log.d(LOG_TAG, "args string:" + args[0]);
                String imei = "";
                String response = "OK\r\n";
                if (args.length == 1) {
                    char c = 0;
                    if (args[0] instanceof Integer) {
                        c = ((Integer) args[0]).toString().charAt(0);
                    }
                    Log.d(LOG_TAG, "c is " + c);

                    WifiManager mWifiManager = 
                       (WifiManager)getSystemService(Context.WIFI_SERVICE);

                    switch(c) {
                    case '1':
                        mWifiManager.setWifiEnabled(false);
                        try {
                            Thread.sleep(THREAD_SLEEP_ONE);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        if (mWifiManager.isWifiEnabled()) {
                            response = "FAILED\r\n";
                        }
                        break;
                    case '2':
                        mWifiManager.setWifiEnabled(true);
                        try {
                            Thread.sleep(THREAD_SLEEP_TWO);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        if (!mWifiManager.isWifiEnabled()) {
                            response = "FAILED\r\n";
                        }
                        break;
                    default:
                        response = "IMEI PARAMETER ERROR";
                    }                     

                    return new AtCommandResult(response);
                }
                return new AtCommandResult(AtCommandResult.ERROR);
            }
        });
    }

    protected void initializeAtComCamera() {
        //3.6.1 AT%CAM
        //AT% Commands Name : CAM
        //Description : Camera performance Test. Commands were
        //    modified accordding to variety of camera functions
        mAtParser.register("%CAM", new AtCommandHandler() {
            private AtCommandResult notSupport() {
                String response = formatResponse("NOT SUPPORT");
                return new AtCommandResult(response);
            }
            private AtCommandResult cameraOk() {
                String response = formatResponse("CAMERA OK");
                return new AtCommandResult(response);
            }
            private AtCommandResult isCameraOn() {
                String response = formatResponse("" + mCameraOn);
                return new AtCommandResult(response);
            }


            private boolean startCameraCapturePicture() {
                Intent intent = new Intent(
                    android.provider.MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA);
//                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                try {
                    startActivity(intent);
                } catch (android.content.ActivityNotFoundException e) {
                    return false;
                }
                return true;
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                int setNumber = -1;
                if (null == args || !(args[0] instanceof Integer)) {
                    Log.e("AtciService", "handleSetCommand:Invalid args " + args);
                    return notSupport();
                }

                setNumber = Integer.parseInt(args[0].toString());
                Log.v("AtciService","handleSetCommand:get setNumber=" + setNumber);

                if (setNumber < NUM_ZERO || setNumber > NUM_TWELVE) {
                    Log.e("AtciService", "Error set number " + setNumber);
                    return new AtCommandResult(formatResponse("CAMERA ERROR"));
                }

                if (setNumber >= NUM_SIX && setNumber <= NUM_TEN) {
                    Log.w("AtciService", "not supported function tested!");
                    return notSupport();
                }

                //turn off AVR
                mCameraAVROn = 0;

                if (1 == setNumber) {
                    mCameraOn = 1;
                    if (startCameraCapturePicture()) {
                        return cameraOk();
                    } else {
                        return new AtCommandResult(formatResponse("CAMERA INITIAL FAILED"));
                    }
                }

                Intent intent = new Intent("com.mediatek.AtciService.AT_CAM");
                intent.putExtra("setNumber", setNumber);

                sendBroadcast(intent);
                Log.v("AtciService","handleSetCommand:sent AT_CAM broadcast intent:" + intent);

                String response = formatResponse("CAM set " + setNumber + " OK");
                return new AtCommandResult(response);
            }
        });

        //3.3.1 AT%AVR
        //AT% Commands Name : AVR
        //Description : Test Movie
        mAtParser.register("%AVR", new AtCommandHandler() {
            private AtCommandResult notSupport() {
                String response = formatResponse("NOT SUPPORT");
                return new AtCommandResult(response);
            }
            private AtCommandResult cameraOk() {
                String response = formatResponse("AVR OK");
                return new AtCommandResult(response);
            }
            private AtCommandResult isCameraOn() {
                String response = formatResponse("" + mCameraAVROn);
                return new AtCommandResult(response);
            }


            private boolean startCameraCaptureVideo() {
                Intent intent = new Intent(android.provider.MediaStore.INTENT_ACTION_VIDEO_CAMERA);
//                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                try {
                    startActivity(intent);
                } catch (android.content.ActivityNotFoundException e) {
                    return false;
                }
                return true;
            }

            @Override
            public AtCommandResult handleActionCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleReadCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleTestCommand() {
                return isCameraOn();
            }

            @Override
            public AtCommandResult handleSetCommand(Object[] args) {
                int setNumber = -1;
                if (null == args || !(args[0] instanceof Integer)) {
                    Log.e("AtciService", "handleSetCommand:Invalid args " + args);
                    return notSupport();
                }

                setNumber = Integer.parseInt(args[0].toString());
                Log.v("AtciService","handleSetCommand:get setNumber=" + setNumber);

                if (setNumber < NUM_ZERO || setNumber > NUM_SEVEN) {
                    Log.e("AtciService", "Error set number " + setNumber);
                    return new AtCommandResult(formatResponse("AVR ERROR"));
                }

                if (setNumber >= NUM_SIX && setNumber <= NUM_SEVEN) {
                    Log.w("AtciService", "not supported function tested!");
                    return notSupport();
                }

                //turn off Camera
                mCameraOn = 0;

                if (1 == setNumber) {
                    mCameraAVROn = 1;
                    if (startCameraCaptureVideo()) {
                        return cameraOk();
                    } else {
                        return new AtCommandResult(formatResponse("CAMERA INITIAL FAILED"));
                    }
                }

                Intent intent = new Intent("com.mediatek.AtciService.AT_AVR");
                intent.putExtra("setNumber", setNumber);

                sendBroadcast(intent);
                Log.v("AtciService","handleSetCommand:sent AT_CAM broadcast intent:" + intent);

                String response = formatResponse("AVR set " + setNumber + " OK");
                return new AtCommandResult(response);
            }
        });
    }

    protected void initializeAtParser() {
        Log.i(LOG_TAG, "initializeAtParser");

        initializeAtTelephony();
        initializeAtFMRadio();
        initializeAtMp3Play();
        initializeAtDB();
        initializeAtOSVer();
        initializeAtNoSleep(); 

        /* For AT%LANG */
        initializeAtLanguage();
        initializeAtComCamera();

        initializeAtSystemCall();
    }

    /* Process an incoming AT command line
     */
    protected void handleInput(String input) {
        acquireWakeLock();
        long timestamp;

        if (DBG) {
            timestamp = System.currentTimeMillis();
        }

        AtCommandResult result = mAtParser.process(input);
        if (DBG) {
            Log.d(LOG_TAG, "Processing " + input + " took " +
                   (System.currentTimeMillis() - timestamp) + " ms");
        }

        if (result.getResultCode() == AtCommandResult.ERROR) {
            Log.i(LOG_TAG, "Error processing <" + input + 
                  "> with result <" + result.toString() + ">");
        }

        sendURC("\r\n" + result.toString() + "\r\n");

        releaseWakeLock();
    }

    public AtParser getAtParser() {
        return mAtParser;
    }

    @Override
    public void onDestroy() {
        // TODO Auto-generated method stub
        Log.d(LOG_TAG, " onDestroy");
        
        try {
            InputStream is = mSocket.getInputStream();
            if (is != null) {
                is.close();
            }
            mReceiver.requestStop();
            mReceiverThread.interrupt();
        } catch (IOException e) {
            Log.d(LOG_TAG, " IOException");
        } catch (NullPointerException npe) {
            Log.d(LOG_TAG, " NullPointerException");
        }
        super.onDestroy();
    }

    @Override
    public void onStart(Intent intent, int startId) {
        // TODO Auto-generated method stub
        super.onStart(intent, startId);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // TODO Auto-generated method stub
        return super.onStartCommand(intent, flags, startId);
    }

    public synchronized boolean sendURC(String urc) {
        boolean ret = true;

        if (urc.length() > 0) {
            Log.d(LOG_TAG, "URC Processing:" + urc + ">");
            OutputStream os = null;
            try {
                os = mSocket.getOutputStream();
                os.write(urc.getBytes());
            } catch (IOException e) {
                e.printStackTrace();
                ret = false;
            } //catch(Exception ex) {
             //   ex.printStackTrace();
             //   ret = false;
            //}
            return ret;
        }
        return true;
    }

    private synchronized void acquireWakeLock() {
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        }
    }

    private synchronized void releaseWakeLock() {
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

    class AtciReceiver implements Runnable {
        byte[] mBuffer;
        private volatile boolean mStopThread = false;

        protected synchronized void requestStop() {
            mStopThread = true;
            Log.i(LOG_TAG, "stop AtciReceiver thread.");
        }

        AtciReceiver() {
            mBuffer = new byte[ATCI_MAX_BUFFER_BYTES];
        }

        public void run() {
            int retryCount = 0;
            String socketAtci = SOCKET_NAME_ATCI;

            for (;;) {
                LocalSocket s = null;
                LocalSocketAddress l;

                if(false == mStopThread) {
                    try {
                        s = new LocalSocket();
                        l = new LocalSocketAddress(socketAtci, LocalSocketAddress.Namespace.RESERVED);
                        s.connect(l);
                    } catch (IOException ex) {
                        try {
                            if (s != null) {
                                s.close();
                            }
                        } catch (IOException ex2) {
                            //ignore failure to close after failure to connect
                            Log.e(LOG_TAG, "NullPointerException ex2");
                        }

                        if (retryCount == NUM_SIXTEEN) {
                            Log.e(LOG_TAG,
                                   "Couldn't find '" + socketAtci
                                   + "' socket after " + retryCount
                                   + " times, continuing to retry silently");
                        } else if (retryCount > NUM_ZERO && retryCount < NUM_SIXTEEN) {
                            Log.i(LOG_TAG,
                                   "Couldn't find '" + socketAtci
                                   + "' socket; retrying after timeout");
                        }

                        try {
                            Thread.sleep(SOCKET_OPEN_RETRY_MILLIS);
                        } catch (InterruptedException er) {
                            Log.e(LOG_TAG, "InterruptedException er");
                        }

                        retryCount++;
                        continue;
                    }

                    retryCount = 0;
                    mSocket = s;
                    Log.i(LOG_TAG, "Connected to '" + socketAtci + "' socket");

                    int length = 0;
                    InputStream is = null;
                    try {
                        int countRead = 0;
                        is = mSocket.getInputStream();

                        for (;;) {
                            Log.i(LOG_TAG, "Wait to read command from ATCI generic service");
                            countRead = is.read(mBuffer);
                            if (countRead < 0) {
                                Log.e(LOG_TAG, "Hit EOS while reading message");
                                break;
                            }
                            if (countRead > 0) {
                                handleInput(new String(mBuffer, 0, countRead));
                            }
                        }
                    } catch (Exception ex) {
                        //} catch (Throwable tr) {
                         //   tr.printStackTrace();
                         //   Log.e(LOG_TAG, "Uncaught exception read length=" + 
                         //         length + "Exception:" + tr.toString());                    
                        if(ex instanceof InterruptedException || 
                            ex instanceof InterruptedIOException ||
                            ex instanceof ClosedByInterruptException){
                            Log.i(LOG_TAG, "'" + socketAtci + "' Interrupt : ", ex);
                        } else if ( ex instanceof IOException) {
                            ex.printStackTrace();
                            Log.i(LOG_TAG, "'" + socketAtci + "' socket closed", ex);                        
                        } else {
                            throw new RuntimeException(ex);
                        }
                    } finally {
                        try {
                            if (is != null) {
                                is.close();
                                is = null;
                            }
                        } catch (IOException ex) {
                            Log.e(LOG_TAG, "IOException ex");
                        }
                    }

                    Log.i(LOG_TAG, "Disconnected from '" + socketAtci + "' socket");
                    try {
                        mSocket.close();
                    } catch (IOException ex) {
                        Log.e(LOG_TAG, "IOException ex2");
                    }
                    mSocket = null;
                }
            }
        }
    }
    
    private BroadcastReceiver mEccStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();            
            if (EMERGENCY_CALL_ACTION.equals(action)) {
                int state = intent.getIntExtra("EM_Call_State", 0);//0=call hang up;1= call dialed
                String callNumber = intent.getStringExtra("Call_Number");
                
                Log.i(LOG_TAG, "Receive ECC intent call state:" + state +
                     " with number:" + callNumber);
                if (callNumber != null) {
                    mEcallState = state; //0=call hang up;1= call dialed
                }
            }
        }
    };   

    /****************/
    /* Native functions */
    /****************/
    private native boolean writeBTAddressNative(String bdaddr);
    private native String readBTAddressNative();
    private native boolean queryBTTestModeNative();
    private native boolean enterBTTestModeNative(String bdaddr);
    private native boolean leaveBTTestModeNative();
    private void playFM() {
        Log.i(LOG_TAG, ">>> PlayFM");

        setMute(true);

        boolean bRes = powerUp((float) mCurrentStation / FREQUENCY_CONVERT_RATE);
        if (bRes) {
            mIsFMRadioPlaying = true;
            setMute(false);

            if (!isAntennaAvailable()) {
                switchAntenna(1); // short antenna.
            }
            // Hold the wake lock.
            // mWakeLock.acquire();
        } else {
            setMute(true);
            powerDown();
            mIsFMRadioPlaying = false;
            Log.e(LOG_TAG, "Error: Can not power up.");
        }
        Log.i(LOG_TAG, "<<< PlayFM");
    }

    private boolean isAntennaAvailable() {
        mAudioManager = (AudioManager) getSystemService(mContext.AUDIO_SERVICE);
        if (!FeatureOption.MTK_MT519X_FM_SUPPORT) {
            return mAudioManager.isWiredHeadsetOn();
        } else {
            return true;
        }
    }

    // Wrap service functions.
    private boolean openDevice() {
        Log.i(LOG_TAG, ">>> FMRadio.openDevice");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.openDevice();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.openDevice: " + bRet);
        return bRet;
    }

    private boolean isDeviceOpen() {
        Log.i(LOG_TAG, ">>> FMRadio.isDeviceOpen");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.isDeviceOpen();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.isDeviceOpen: " + bRet);
        return bRet;
    }

    private boolean powerUp(float frequency) {
        Log.i(LOG_TAG, ">>> FMRadio.powerUp");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.powerUp(frequency);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.powerUp: " + bRet);
        return bRet;
    }

    private boolean powerDown() {
        Log.i(LOG_TAG, ">>> FMRadio.powerDown");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.powerDown();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.powerDown: " + bRet);
        return bRet;
    }

    private boolean isPowerUp() {
        Log.i(LOG_TAG, ">>> FMRadio.isPowerUp");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.isPowerUp();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.isPowerUp: " + bRet);
        return bRet;
    }
    private boolean tune(float frequency) {
        Log.i(LOG_TAG, ">>> FMRadio.tune");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.tune(frequency);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.tune: " + bRet);
        return bRet;
    }

    private float seek(float frequency, boolean isUp) {
        Log.i(LOG_TAG, ">>> FMRadio.seek");
        float fRet = 0;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                fRet = mFMRadioService.seek(frequency, isUp);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.seek: " + fRet);
        return fRet;
    }

    private int setMute(boolean mute) {
        Log.i(LOG_TAG, ">>> FMRadio.setMute");
        int iRet = -1;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                iRet = mFMRadioService.setMute(mute);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.setMute: " + iRet);
        return iRet;
    }
    private void initService(int iCurrentStation) {
        Log.i(LOG_TAG, ">>> FMRadio.initService: " + iCurrentStation);
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                mFMRadioService.initService(iCurrentStation);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.initService");
    }
    private boolean isServiceInit() {
        Log.i(LOG_TAG, ">>> FMRadio.isServiceInit");
        boolean bRet = false;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                bRet = mFMRadioService.isServiceInit();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.isServiceInit: " + bRet);
        return bRet;
    }

    private int getFrequency() {
        Log.i(LOG_TAG, ">>> FMRadio.getFrequency");
        int iRet = 0;
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                iRet = mFMRadioService.getFrequency();
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.getFrequency: " + iRet);
        return iRet;
    }

    /** 0 success, 1 fail, 2 not support.*/
    public int switchAntenna(int type) {
        Log.i(LOG_TAG, ">>> FMRadio.switchAntenna");
        int iRet = 2; // not supported short antenna.
        if (null == mFMRadioService) {
            Log.e(LOG_TAG, "Error: No service interface.");
        } else {
            try {
                iRet = mFMRadioService.switchAntenna(type);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "FMRadio Exception: Cannot call service function.");
            }
        }
        Log.i(LOG_TAG, "<<< FMRadio.switchAntenna: " + iRet);
        return iRet;
    }

    private void seekStation(boolean enable, int station, boolean direction) {
        if (!enable) {
            // Start seek. We should detect if there is another seek procedure.
            if (mIsFMRadioSeeking) {
                Log.w(LOG_TAG, "Warning: already seeking");
            } else {
                mIsFMRadioSeeking = true;
                mFMRadioSeekThread = new SeekThread(station, direction);
                mFMRadioSeekThread.start();
            }
        } else {
            mFMRadioSeekThread = null;
            mIsFMRadioSeeking = false;
        }
    }

    class InitialThread extends Thread {

        public InitialThread() {
        }

        public void run() {
            if (!openDevice()) {
                // ... If failed, exit?
                Log.e(LOG_TAG, "Error: opendev failed.");
            } else {
                Log.i(LOG_TAG, "opendev succeed.");
            }
            // The app maybe killed at the previous time. So after opendev, get the power state.
            mIsFMRadioPlaying = isPowerUp();

            if (!isPowerUp()) {
                playFM();
            }

            Log.e(LOG_TAG, "InitialThread terminated.");
        }
    }

    class SeekThread extends Thread {
        public int mCrntStation = 0;
        public boolean mSeekDirection = false;

        public SeekThread(int station, boolean direction) {
            mCrntStation = station;
            mSeekDirection = direction;
        }

        public void run() {

            float fStation = seek((float) mCrntStation / FREQUENCY_CONVERT_RATE, mSeekDirection);
            int iStation = (int) (fStation * FREQUENCY_CONVERT_RATE);

            if (iStation >= HIGHEST_STATION || iStation <= LOWEST_STATION) {
                // Loop to the highest frequency and continue to search valid station.
                if (!mSeekDirection) {
                    fStation = seek((float) (HIGHEST_STATION) / FREQUENCY_CONVERT_RATE, 
                                       mSeekDirection);
                    iStation = (int) (fStation * FREQUENCY_CONVERT_RATE);
                } else {
                    fStation = seek((float) (LOWEST_STATION) / FREQUENCY_CONVERT_RATE, 
                                       mSeekDirection);
                    iStation = (int) (fStation * FREQUENCY_CONVERT_RATE);
                }
            }
            if (iStation >= HIGHEST_STATION || iStation <= LOWEST_STATION) {
                // Can not find a valid station.
                Log.e(LOG_TAG, "Error: Can not search previous station.");
                Message msg = new Message();
                msg.setTarget(mFMRadioHandler);
                Bundle bundle = new Bundle();
                bundle.putInt(TYPE_MSGID, MSGID_SEEK_FAIL);
                msg.setData(bundle);
                msg.sendToTarget();
            } else {
                Log.i(LOG_TAG, "Send message to tune to recently seeked station: " + fStation);
                Message msg = new Message();
                msg.setTarget(mFMRadioHandler);
                Bundle bundle = new Bundle();
                bundle.putInt(TYPE_MSGID, MSGID_SEEK_FINISH);
                bundle.putInt(TYPE_SEEK_STATION, iStation);
                msg.setData(bundle);
                msg.sendToTarget();
                Log.i(LOG_TAG, "Send message to tune to recently seeked station: " + fStation);
            }
        }
    }
    Handler mFMRadioHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (MSGID_SEEK_FINISH == msg.getData().getInt(TYPE_MSGID)) {
                int station = msg.getData().getInt(TYPE_SEEK_STATION);
                tune((float) station / FREQUENCY_CONVERT_RATE);
                seekStation(true, 0, true);
                mIsFMRadioDoSeek = true;
            } else if (MSGID_SEEK_FAIL == msg.getData().getInt(TYPE_MSGID)) {
                seekStation(true, 0, true);
                mIsFMRadioDoSeek = false;
            }
        }
    };
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.i(LOG_TAG, ">>> FMRadio.onServiceConnected");
            mFMRadioService = IFMRadioService.Stub.asInterface(service);
            if (null == mFMRadioService) {
                Log.e(LOG_TAG, "Error: null interface");
            } else {
                if (!isServiceInit()) {
                    Log.i(LOG_TAG, "FMRadio service is not init.");
                    initService(DEFAULT_FREQUENCY);
                    InitialThread thread = new InitialThread();
                    thread.start();
                    if (FeatureOption.MTK_MT519X_FM_SUPPORT) {
                        try {
                            Thread.sleep(WAIT_TIME);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                            Log.e(LOG_TAG, "FMRadio Exception: Thread.sleep.");
                        }
                    }
                } else {
                    Log.i(LOG_TAG, "FMRadio service is already init.");
                    if (isDeviceOpen()) {
                        // Get the current frequency in service and save it into database.
                        int iFreq = getFrequency();
                        if (iFreq > HIGHEST_STATION || iFreq < LOWEST_STATION) {
                            Log.e(LOG_TAG, "FMRadio Error: invalid frequency in service.");
                        } else {
                            Log.i(LOG_TAG, "The frequency in FM service is same as in database.");
                        }

                    } else {
                        // This is theoretically never happen.
                        Log.e(LOG_TAG, "Error: FMRadio device is not open");
                    }
                }
            }
            mIsFMRadioServiceBinded = true;
            Log.i(LOG_TAG, "<<< FMRadio.onServiceConnected");

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.i(LOG_TAG, ">>> FMRadioEMActivity.onServiceDisconnected");
            mFMRadioService = null;
            Log.i(LOG_TAG, "<<< FMRadioEMActivity.onServiceDisconnected");

        }
    };
}
