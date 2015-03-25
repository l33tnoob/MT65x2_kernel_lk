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

package com.mediatek.bluetooth.avrcp;

import android.Manifest.permission;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfileManager;
import android.bluetooth.BluetoothProfileManager.Profile;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.mediatek.common.featureoption.FeatureOption;

import java.lang.ref.WeakReference;

public class BluetoothAvrcpService extends Service implements Runnable {
    public static final String TAG = "EXT_AVRCP";

    private boolean mStopped = false;

    private int mNativeObject;

    private Thread mThread;

    BTAvrcpMusicAdapter mAdapter;

    BTAvrcpBrowserAdapter mBrowserAdapter;

    private short mCurPlayerId = 1;

    private short mCurBrowserId = 0;

    private boolean mTrackAvailableChangedFlag = false;

    private boolean mTrackAddressedPlayerChangedFlag = false;

    public static int sPTSDebugMode = 0;

    private static final byte OK = BTAvrcpProfile.OK; // 0

    private static final byte FAIL = BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM; // 1

    private static final short UTF8_CHARSET = BTAvrcpProfile.UTF8_CHARSET; // 0x06a

    // AVRCP status definition
    public static final byte STATUS_OK = BTAvrcpProfile.STATUS_OK;

    public static final int FILENAME_ATTR_ID = 0x01; // Use title as file name

    public static final  short UTF8_ARRAY[] = {
            UTF8_CHARSET, UTF8_CHARSET, UTF8_CHARSET, UTF8_CHARSET, UTF8_CHARSET, UTF8_CHARSET, UTF8_CHARSET
    };

    // debug thread for BTAvrcpMusicAdapter
    public static boolean sSupportMusicUI = false;

    // debug thread for BTAvrcpBrowseAdapter
    public static boolean sSupportBrowse = false;

    public static boolean sDebugMsg = false;

/********These code is for UT******************************/
    public static boolean sUtState = false;

    private static final String UT_TAG = "[BT][AVRCP_UT][BluetoothAvrcpService]";
/**********************************************************/

/*    static {
        System.loadLibrary("extavrcp_jni");
        classInitNative(); // register down API to alloc a object
    }
*/
    private int mStartId = -1;

    private Context mContext;

    private byte mVersion = 10;

    public BluetoothAvrcpService() {
        Log.v(TAG, "[BT][AVRCP] BluetoothAvrcpService Constructor enable");
        // set up the flage before create Browse and Music adapter
        // Use BTAvrcpProfile to get prefer comile Version
        mVersion = BTAvrcpProfile.getPreferVersion();
        Log.v(TAG, "[BT][AVRCP] getPreferVersion:" + Integer.toString(mVersion));
        if( mVersion >= 14 ){
        	sSupportBrowse = true;
        }
        if( mVersion >= 13 ){
        	sSupportMusicUI = true;
        }

        mBrowserAdapter = new BTAvrcpBrowserAdapter(this);

        /************These code is for UT*****************************/
        if (!sUtState) {
            Log.d(UT_TAG, "[API: service constructor] load library : extavrcp_jni");
            System.loadLibrary("extavrcp_jni");
        } else {
            Log.d(UT_TAG, "[API: service constructor] load library : extavrcp_ut");
            System.loadLibrary("extavrcp_ut");
        }
        classInitNative();
    }

    public static boolean isSupportBrowse() {
        return sSupportBrowse;
    }

    private void testBrowser() {
        mBrowserAdapter.search("es");
    }

    // @Override
    public IBinder onBind(Intent intent) {
        Log.v(TAG, "[BT][AVRCP] AVRCP Service onBind");
        // throw new
        // UnsupportedOperationException("Cannot bind to Bluetooth Avrcp Service");
        return mBinder;
    }

    // @Override
    public void onCreate() {
        Log.v(TAG, "[BT][AVRCP] AVRCP Service onCreate");
        mContext = getApplicationContext();

        if (mAdapter == null) {
            Log.v(TAG, "AVRCP initializeNativeObjectNative");
            initializeNativeObjectNative(); // create a context of jni
            mAdapter = new BTAvrcpMusicAdapter(this, this);
            this.enable(); // create indication thread

            mAdapter.init();
        }
//        else {
//            // already has a adapter
//        }

        BluetoothAvrcpReceiver.sAvrcpServer = this;
    }

    // @Override
    public void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "[BT][AVRCP] AVRCP Service onDestroy");
        this.disable(); // close indication thread
        cleanupNativeObjectNative(); // free a context of jni
        if (mAdapter != null) {
            mAdapter.deinit();
            mAdapter = null;
        }

        if (null != mBrowserAdapter) {
            mBrowserAdapter.deinit();
            mBrowserAdapter = null;
        }

        BluetoothAvrcpReceiver.sAvrcpServer = null;
    }

    // @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.v(TAG, "[BT][AVRCP] AVRCP Service onStartCommand flags:" + flags + " startId:" + startId);
        int retCode = super.onStartCommand(intent, flags, startId);

        mStartId = startId;
        if (retCode == START_STICKY) {
            if (null != intent) {
                parseIntent(intent);
            } else {
                Log.v(TAG, "[BT][AVRCP] onStartCommand null intent");
            }
        } else {
            Log.v(TAG, "[BT][AVRCP] onStartCommand retCode " + retCode);
        }

        return retCode;
    }

    // process the intent from receiver
    private void parseIntent(final Intent intent) {
        String action = intent.getStringExtra("action");
        Log.v(TAG, "action: " + action);

        int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            if (state == BluetoothAdapter.STATE_ON) {
                if (mAdapter == null) {
                    mAdapter = new BTAvrcpMusicAdapter(this, this);
                    mAdapter.init();
                }
                if (mAdapter != null) {
                    enable();
                }
            } else if (state == BluetoothAdapter.STATE_OFF) {
                // Release all resources
                disable();
                stopSelf(mStartId);
            } else {
                Log.v(TAG, "[BT][AVRCP] parseIntent state " + state);
            }
        } else {
            Log.v(TAG, "[BT][AVRCP] intent without action");
            if (mAdapter == null) {
                mAdapter = new BTAvrcpMusicAdapter(this, this);
                mAdapter.init();
            }
            if (mAdapter != null) {
                enable();
            }
        }
    }

    /**
     * @brief enable the servcie Send the activate request ilm
     */
    public boolean enable() {
        byte musicVersion = 10;
    	
        Log.i(TAG, "[BT][AVRCP] enable - ignore test");
        if (mThread == null) {
            if (!enableNative()) {
                Log.e(TAG, "[BT][AVRCP] Could not init BluetoothDunService");
                notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
                return false;
            }

            // switchable version
    	    mVersion = BTAvrcpProfile.getPreferVersion();
            musicVersion = mAdapter.getSupportVersion();
			
            // show version
            Log.v(TAG, "[BT][AVRCP] mVersion:" + Integer.toString(mVersion) + " music:" + Integer.toString(musicVersion));
            if( mVersion > musicVersion ){
                mVersion = musicVersion;
            }
    	    switch(mVersion){
    	    	case 14:
    	    	sSupportMusicUI = true;
    	    	sSupportBrowse = true;
    	    	Log.v(TAG, "[BT][AVRCP] Support AVRCP1.4");
    	    	break;
    	    	case 13:
    	    	sSupportMusicUI = true;
    	    	sSupportBrowse = false;
    	    	Log.v(TAG, "[BT][AVRCP] Support AVRCP1.3");
    	    	break;
    	    	default:
    	    	sSupportMusicUI = false;
    	    	sSupportBrowse = false;    	    	
    	    	Log.v(TAG, "[BT][AVRCP] Support AVRCP1.0");
    	    	mVersion = 10;
    	    	break;
            }
    	    
    	    activateConfigNative((byte)mVersion, (byte)0x01, (byte)0); /*14 is v1.4*/
    	    
            activateReqNative((byte) OK);

            mThread = new Thread(this);
            mThread.setName("BluetoothAvrcpServiceThread");
            mThread.start();
        } else {
            Log.i(TAG, "enable - ignore");
        }

        notifyProfileState(BluetoothProfileManager.STATE_ENABLING);
        return true;
    }

    /**
     * @brief disable the service Doesn't real send the disable request ilm
     */
    public void disable() {
        Log.i(TAG, "[BT][AVRCP] +disable");
        Log.v(TAG, "[BT][AVRCP] AVRCP disable start");
        deactivateReqNative((byte) 0);

        if (null != mThread) {

            shutdownIndThread(); // flag up the stopped boolean
            try {
                mThread.join(10000); // wait for mills to die
            } catch (InterruptedException e) {
                Log.w(TAG, "[BT][AVRCP] Excpetion ".concat(e.toString()));
            }
            mThread = null;
        }
        Log.v(TAG, "[BT][AVRCP] Invoke AVRCP cleanupNativeObjectNative");
        disableNative();

        Log.i(TAG, "[BT][AVRCP] -disable");
    }

    /**
     * @brief Peek the indication response by native code
     */
    // @Override
    public void run() {
        Log.v(TAG, "[BT][AVRCP] AVRCP run!!!");

        while (!mStopped) {
            if (!listenerNativeEventLoop(false)) {
                mStopped = true;
            }
        }
        Log.v(TAG, "[BT][AVRCP] AVRCP Int Thread stop!!!");
    }

    /**
     * @brief stop the indication thread
     */
    void shutdownIndThread() {
        Log.v(TAG, "[BT][AVRCP] +shutdownIndThread start");
        mStopped = true;
        wakeupListenerNative();
//        if (mThread != null) {
//            // mThread.interrupt();
//        }
        Log.v(TAG, "[BT][AVRCP] +shutdownIndThread end");
    }

    private void notifyProfileState(int state) {
        Log.v(TAG, "[BT][AVRCP] notifyProfileState: " + state);

        Intent intent = new Intent(BluetoothProfileManager.ACTION_PROFILE_STATE_UPDATE);
        intent.putExtra(BluetoothProfileManager.EXTRA_PROFILE, Profile.Bluetooth_AVRCP);
        intent.putExtra(BluetoothProfileManager.EXTRA_NEW_STATE, state);
        // mContext.sendBroadcast(intent, permission.BLUETOOTH); // no need this by gap profile
    }

    /* JNI related method */
    private static native boolean classInitNative();

    private native boolean enableNative(); // /< create sockets

    private native void disableNative();

    private native void initializeNativeObjectNative(); // /< create a context

    // of jni

    private native void cleanupNativeObjectNative();

    public native boolean connectNative(String s); // /< only connect control

    // channel

    public native boolean disconnectNative();

    public native boolean connectBrowseNative();

    public native boolean disconnectBrowseNative();

    private native void wakeupListenerNative();

    /* AVRCP essential function */
    private native boolean listenerNativeEventLoop(boolean notWait);

    public native int getPlayerIdNative();

    public static native int getMaxPlayerNumNative();

    public native boolean testparmnum(byte i1, byte i2, byte i3, byte i4, byte i5, byte i6, byte i7, byte i8, byte i9,
            byte i10, byte i11, byte i12);

    /* AVRCP 1.0 Req */
    public native boolean activateConfigNative(byte version, byte sdpfeature, byte reserved); // /< config the acitvate
                                                                                              // version

    public native boolean activateReqNative(byte index);

    public native boolean deactivateReqNative(byte index);

    public native boolean connectReqNative(String addrStr); // /< connect

    // control & browse
    // channel

    public native boolean disconnectReqNative();

    /* AVRCP 1.0 Cnf */
    public void activateCnf(byte index, int result) {
        Log.v(TAG, String.format("[BT][AVRCP] activate_cnf index:%d result:%d", index, result));// ,
        // index,
        // result);
        if (0x1000 == result || 0x0 == result) {
            notifyProfileState(BluetoothProfileManager.STATE_ENABLED);
        } else {
            notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }
    }

    public void deactivateCnf(byte index, int result) {
        Log.v(TAG, "[BT][AVRCP] deactivate_cnf index:%d result:%d");// , index,
        // result);
        if (0x1000 == result || 0x0 == result) {
            notifyProfileState(BluetoothProfileManager.STATE_DISABLED);
        } else {
            notifyProfileState(BluetoothProfileManager.STATE_ABNORMAL);
        }
    }

    public void connectCnf(int result) {
        Log.v(TAG, "[BT][AVRCP] connect_cnf result:%d");// , result);
        mAdapter.onConnect();
    }

    public void connectInd(byte addr[], String deviceName, int psmValue) {
        Log.v(TAG, "[BT][AVRCP] connect_ind");
        mAdapter.onConnect();
        if (null != mBrowserAdapter) {
            mBrowserAdapter.onConnect();
        }

        // reset data
        mTrackAvailableChangedFlag = false;
        mTrackAddressedPlayerChangedFlag = false;
        mCurPlayerId = 1;
    }

    public void disconnectInd() {
        Log.v(TAG, "[BT][AVRCP] disconnect_ind");
        mAdapter.onDisconnect();
        if (null != mBrowserAdapter) {
            mBrowserAdapter.onDisconnect();
        }

        sPTSDebugMode = 0; // reset to normal
    }

    /**
     * @brief pass key to player
     */
    public void passThroughKeyInd(int keyvalue, byte isPress) {
        Log.v(TAG, String.format("[BT][AVRCP] pass_through_key key:%d isPress:%d", keyvalue, isPress));
        mAdapter.passThroughKeyInd(keyvalue, isPress); // only show information
    }

    /**
     * @brief pass key to MMI manager When receiving a pass through(key), AVRCP server could pass it to the foreground
     *        application (sendKeyEvent API) or pass it directly to Music player (pass_throughKey API)
     * @see pass_throughKey
     */
    public void sendAvrcpKeyEventInd(int keyvalue, byte isPress) {
        Log.v(TAG, String.format("[BT][AVRCP] sendAvrcpKeyEventInd key:%d isPress:%d", keyvalue, isPress));
        mAdapter.sendAvrcpKeyEvent(keyvalue, isPress);
    }

    /* AVRCP 1.0 CT Req */
    public native boolean sendKeyReqNative(int key);

    /* AVRCP 1.3 TG Rsp */
    public native boolean getCapabilitiesRspNative(byte error, byte count, byte capabilities[]);

    public native boolean listPlayerappAttributeRspNative(byte error, byte count, byte attributeIds[]);

    public native boolean listPlayerappValuesRspNative(byte error, byte attributeId, byte count, byte attributeValues[]);

    public native boolean getCurplayerappValueRspNative(byte error, byte count, byte attributeIds[],
            byte attributeValues[]);

    public native boolean setPlayerappValueRspNative(byte error);

    public native boolean getPlayerappAttributeTextRspNative(byte error, byte item, byte total, byte attributeId,
            int charset, String attributeString);

    public native boolean getPlayerappValueTextRspNative(byte error, byte item, byte total, byte attributeId,
            byte valueId, String valueString);

    public native boolean informBatteryStatusRspNative(byte error);

    public native boolean informDisplayCharsetRspNative(byte error);

    public native boolean getElementAttributesRspNative(byte error, byte item, byte total, byte attributeId, short charset,
            String value);

    public native boolean getPlayerstatusRspNative(byte index, int length, int position, byte status);

    public native boolean notificationPlayStatusChangedNative(byte index, byte isinterim, byte status);

    public native boolean notificationTrackChangedNative(byte index, byte isinterim, long id);

    public native boolean notificationTrackReachedEndNative(byte index, byte isinterim);

    public native boolean notificationTrackReachedStartNative(byte index, byte isinterim);

    public native boolean notificationPlayPosChangedNative(byte error, byte isinterim, int position);

    public native boolean notificationBatteryStatusChangedNative(byte error, byte isinterim, byte status);

    public native boolean notificationSystemStatusChangedNative(byte error, byte isinterim, byte status);

    public native boolean notificationApplicationSettingChangedNative(byte error, byte isinterim, byte count,
            byte attrIds[], byte values[]);

    public native boolean setAbsoluteVolumeRspNative(byte error, byte status, byte volume);

    public native boolean setAddressedplayerRspNative(byte index, byte playerId);

    /* AVRCP 1.3 TG Ind */
    public void playerAppCapabilitiesInd(int type) {
        byte[] ca;
        Log.v(TAG, String.format("[BT][AVRCP] playerAppCapabilitiesInd type:%d", type));

        ca = mAdapter.playerAppCapabilities();
        if (ca != null) {
            getCapabilitiesRspNative((byte) 0, (byte) ca.length, ca); // test
            // not to
            // send
            // ca[3]
            // (4th)
        } else {
            getCapabilitiesRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INTERNAL_ERROR, (byte) 0, new byte[1]);
        }
    }

    public void listPlayerAppAttributeInd() {
        byte[] ca;
        Log.v(TAG, "[BT][AVRCP] listPlayerAppAttributeInd");

        ca = mAdapter.listPlayerAppAttribute();
        if (ca.length > 0) {
            listPlayerappAttributeRspNative((byte) 0, (byte) ca.length, ca);
        } else {
            listPlayerappAttributeRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INTERNAL_ERROR, (byte) 0, new byte[1]);
        }
    }

    public void listPlayerAppValueInd(byte attrId) {
        byte[] ca;
        Log.v(TAG, String.format("[BT][AVRCP] listPlayerAppValueInd attrId:%d", attrId));

        ca = mAdapter.listPlayerAppValue(attrId);
        if (ca != null && ca.length > 0) {
            listPlayerappValuesRspNative((byte) 0, attrId, (byte) ca.length, ca);
        } else {
            listPlayerappValuesRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, attrId, (byte) 0, new byte[1]);
        }
    }

    public void getCurPlayerAppValueInd(byte count, byte attrIds[]) {
        Log.v(TAG, String.format("[BT][AVRCP] getCurPlayerAppValueInd count:%d", count));

        byte dataValue[];
        byte dataAttr[];
        byte attrId;
        byte k;
        if (count == 0) {
            // all attributes 0x01~0x04
            count = BTAvrcpProfile.MAX_ATTRIBUTE_NUM;
            Log.v(TAG, String.format("[BT][AVRCP] getCurPlayerAppValueInd Change to All count:%d", count));
            attrIds = new byte[4];
            for (byte i = 0; i < 4; i++) {
                attrIds[i] = (byte) (i + 1);
            }
        }
        dataAttr = new byte[count];
        dataValue = new byte[count];

        // Only return valid attributes
        k = 0;
        for (byte i = 0; i < count; i++) {
            if (attrIds == null || attrIds.length < i) {
                attrId = i;
            } else {
                attrId = attrIds[i];
            }
            dataAttr[k] = attrId;
            dataValue[k] = mAdapter.getCurPlayerAppValue(attrId);

            // next item
            if (dataValue[k] != 0) {
                k++;
            } else {
                Log.w(TAG, String.format("[BT][AVRCP] getCurPlayerAppValueInd attr_id:%d ret:%d", attrId, dataValue[k]));
            }
        }

        // / k is the real data number. data[] is the value
        if (k != 0) {
            getCurplayerappValueRspNative((byte) 0, k, dataAttr, dataValue);
        } else {
            // PTS TC_TG_PAS_BI_04_C
            getCurplayerappValueRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, k, dataAttr, dataValue); 
        }
    }

    public void setPlayerAppValueInd(byte count, byte attrIds[], byte values[]) {
        boolean ok = false;
        Log.v(TAG, String.format("[BT][AVRCP] setPlayerAppValueInd count:%d length:%d", count, attrIds.length));

        for (byte i = 0; i < count && i < attrIds.length && i < values.length; i++) {
            Log.v(TAG, String.format("[BT][AVRCP] setPlayerAppValueInd i:%d id:%d value:%d", i, attrIds[i], values[i]));
            ok = mAdapter.setPlayerAppValue(attrIds[i], values[i]);
        }
        if (ok) {
            setPlayerappValueRspNative((byte) 0); // success
        } else {
            setPlayerappValueRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM); // PTS
            // TC_TG_PAS_BI_05_c
        }
    }

    public void getPlayerAppAttrTextInd(byte count, byte attrIds[]) {
        Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppAttrTextInd count:%d", count));
        String sText;
        byte i = 0;
        if (count > attrIds.length) {
            count = (byte) (attrIds.length);
        }
        if (0 == count) {
            sText = "empty";
            getPlayerappAttributeTextRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, i, count, attrIds[i],
                    0x6a, sText);
        } else if (1 == count) {
            sText = mAdapter.getPlayerAppAttrText(attrIds[0]);
            if (null == sText) {
                // don't support attrId. return invalid parameter
                sText = "null"; // avoid java-to-jni exception
                Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppAttrTextInd attr_id:%d  is null", attrIds[0]));
                getPlayerappAttributeTextRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, i, count, attrIds[i],
                        0x6a, sText);
            } else {
                //
                i = 0;
                getPlayerappAttributeTextRspNative((byte) 0, i, count, attrIds[i], 0x6a, sText);
            }
        } else {
            for (i = 0; i < count; i++) {
                sText = mAdapter.getPlayerAppAttrText(attrIds[i]);
                if (null == sText) {
                    sText = "";
                }
                getPlayerappAttributeTextRspNative((byte) 0, i, count, attrIds[i], 0x6a, sText);
            }
        }

        Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppAttrTextInd done"));
    }

    public void getPlayerAppValueTextInd(byte attrId, byte count, byte values[]) {
        Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppValueTextInd attrId:%d count:%d", attrId, count));
        String sText = "";
        byte i = 0;
        for (i = 0; i < count; i++) {
            Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppValueTextInd attrId:%d i:%d count:%d", attrId, i, values[i]));
        }

        if (count > values.length) {
            count = (byte) (values.length);
        }
        if (0 == count) { // BI_03_C
            getPlayerappValueTextRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, i, count, attrId, values[i],
                    sText);
        } else if (1 == count) {
            i = 0;
            sText = mAdapter.getPlayerAppValueText(attrId, values[i]);
            if (null == sText) {
                // PTS TC_TG_PAS_BI_03_C
                getPlayerappValueTextRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PARAM, i, count, attrId,
                        values[i], sText);
            } else {
                getPlayerappValueTextRspNative((byte) 0, i, count, attrId, values[i], sText);
            }
        } else {
            for (i = 0; i < count; i++) {
                sText = mAdapter.getPlayerAppValueText(attrId, values[i]);
                if (null == sText) {
                    sText = "";
                }
                getPlayerappValueTextRspNative((byte) 0, i, count, attrId, values[i], sText);
            }
        }

        Log.v(TAG, String.format("[BT][AVRCP] getPlayerAppAttrTextInd done"));
    }

    public void informDisplayCharsetInd(byte count, short charset[]) {
        boolean ret;
        StringBuffer sb = new StringBuffer();
        Log.v(TAG, String.format("[BT][AVRCP] informDisplayCharsetInd count:%d", count));

        for (short i = 0; i < count; i++) {
            sb.append(String.format("[BT][AVRCP]  [%d]:%x", i, charset[i]));
        }
        Log.v(TAG, sb.toString());

        ret = mAdapter.informDisplayCharset(count, charset);
        if (ret) {
            informDisplayCharsetRspNative((byte) 0);
        } else {
            informDisplayCharsetRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_INTERNAL_ERROR);
        }
    }

    public void informBatteryStatusofctInd(byte status) {
        Log.v(TAG, String.format("[BT][AVRCP] informBatteryStatusofctInd  status:%d", status));

        mAdapter.informBatteryStatus(status);

        informBatteryStatusRspNative((byte) 0);
    }

    public void getElementAttributesInd(long identifier, byte count, int attributeIds[]) {
        String sText;
        StringBuffer sb = new StringBuffer();
        int bPTS = 0;
        int ptsAttirbuteIds[] = attributeIds;

        Log.v(TAG, String.format("[BT][AVRCP] getElementAttributesInd id:%d count:%d ", identifier, count));

        for (int i = 0; i < count; i++) {
            sb.append(String.format(" [%d]:%x", i, attributeIds[i]));
        }
        Log.v(TAG, sb.toString());

        if (count == 0 || null == attributeIds) {
            // list all attributes !!!
            count = 3;
            attributeIds = new int[3];
            for (byte i = 0; i < count; i++) {
                attributeIds[i] = i + 1;
            }
            Log.v(TAG, String.format("[BT][AVRCP] getElementAttributesInd Create all attrs count:%d ", count));
            ptsAttirbuteIds = attributeIds;
        }

        bPTS = sPTSDebugMode;
        /* PTS generate debug test */

        // generate fake data
        if (0 == count) {
            count = 3;
            ptsAttirbuteIds = new int[count];
            for (byte i = 0; i < count; i++) {
                ptsAttirbuteIds[i] = i + 1;
            }
        } else {
            ptsAttirbuteIds = new int[attributeIds.length];
            for (byte i = 0; i < count; i++) {
                ptsAttirbuteIds[i] = attributeIds[i];
            }
        }
        if (null != ptsAttirbuteIds) {
            Log.v(TAG, String.format("[BT][AVRCP] getElementAttributesInd pts_attirbute_ids.length:%d mPTSDebugMode:%d",
                    ptsAttirbuteIds.length, sPTSDebugMode));
        }
        for (byte i = 0; i < count; i++) {
            if (0 == identifier) {
                // 0 is the current play
                sText = mAdapter.getElementAttribute(identifier, ptsAttirbuteIds[i]);
            } else {
                sText = mBrowserAdapter.getItemAttribute((byte) 1, identifier, (short) 0, ptsAttirbuteIds[i]);
            }

            /* start of PTS test mode */
            if (0 != bPTS) {
                StringBuffer sbdebug = new StringBuffer();
                // mPTSDebugMode = 0; // reset back to normal when disconnected
                // !

                Log.e(TAG, "[BT][AVRCP] PTS test mode !!! generate a 512 byte data cur mode:" + sPTSDebugMode);
                sText = "";
                sbdebug.append("[BT][AVRCP] PTS debug mode Start:");
                for (int debug = 0; debug < 512; debug++) {
                    sbdebug.append(String.format("[%d]=%d", debug, debug));
                }
                sText = sbdebug.toString();
            }
            /* end of PTS test mode */

            if (null == sText) {
                // error or empty data: use empty string to avoid rejection.
                Log.v(TAG, String.format(
                        "[BT][AVRCP] Fail to getElementAttributes from adapter id:%d i:%d attr_id:%d. Use empty data",
                        identifier, i, attributeIds[i]));
                sText = "";
            }

            if (0 != bPTS) {
                Log.v(TAG, String.format("[BT][AVRCP] PTS %d %d", i, count));
                getElementAttributesRspNative((byte) OK, (byte) i, (byte) count, (byte) ptsAttirbuteIds[i], (short) 0x6a,
                        sText);
            } else {
                getElementAttributesRspNative((byte) OK, (byte) i, (byte) count, (byte) attributeIds[i], (short) 0x6a,
                        sText);
            }
        }

    }

    public void getPlayerstatusInd() {
        byte status = 2;
        int songLength;
        int songPosition;
        Log.v(TAG, String.format("[BT][AVRCP] get_playerstatus_ind "));

        status = mAdapter.getPlayerstatus();
        songLength = mAdapter.getPlayerstatusSongLength();
        songPosition = mAdapter.getPlayerstatusSongPos();
        if (status != 0xff) {
            getPlayerstatusRspNative((byte) 0, songLength, songPosition, status);
        } else {
            Log.v(TAG, String.format("[BT][AVRCP] get_playerstatus_ind error:%d", status));
            // no select file
            songLength = 0;
            songPosition = 0;
            status = 0;
            getPlayerstatusRspNative((byte) 0, songLength, songPosition, status);
        }
    }

    public void registerNotificationInd(byte eventId, int interval) {
        Log.v(TAG, "[BT][AVRCP](test1) registerNotificationInd eventId:" + eventId + " interval:" + interval);
        boolean ret = false;
        int ievent = 0;

        if (eventId == BTAvrcpProfile.EVENT_AVAILABLE_PLAYERS_CHANGED
                || eventId == BTAvrcpProfile.EVENT_ADDRESSED_PLAYER_CHANGED) {
            // service handle
            switch (eventId) {
                case BTAvrcpProfile.EVENT_AVAILABLE_PLAYERS_CHANGED:
                    /*
                     * // only support one player now ret = true; notificationAvailPlayersChangedNative( OK, (byte)1);
                     * bTrackAvailableChangedFlag = true;
                     */
                    ret = false;
                    break;
                case BTAvrcpProfile.EVENT_ADDRESSED_PLAYER_CHANGED:
                    /*
                     * // only support one player now ret = true; notificationAddressedPlayerChangedNative( OK, (byte)1,
                     * (short) 1, (short)browserAdapter.getUidCounter() ); bTrackAddressedPlayerChangedFlag = true;
                     */
                    ret = false;
                    break;
                default:
                    break;
            }
        } else if (BTAvrcpProfile.EVENT_UIDS_CHANGED == eventId) {
            // UID counter is controlled by browser
            ret = mBrowserAdapter.registerNotification(eventId, interval);
        } else {
            // others are player's
            ret = mAdapter.registerNotification(eventId, interval);
        }

        if (!ret) {
            Log.v(TAG, "[BT][AVRCP] registerNotificationInd fail and reject it");
            // player dones't support the event
            ievent = eventId;
            switch (ievent) {
                case BTAvrcpProfile.EVENT_PLAYBACK_STATUS_CHANGED:
                    notificationPlayStatusChangedNative((byte) 0x03, (byte) 1, (byte) 1);
                    break;
                case BTAvrcpProfile.EVENT_TRACK_CHANGED:
                    notificationTrackChangedNative((byte) 0x03, (byte) 1, (long) 0x12345678);
                    break;
                case BTAvrcpProfile.EVENT_TRACK_REACHED_END:
                    notificationTrackReachedEndNative((byte) 0x03, (byte) 1);
                    break;
                case BTAvrcpProfile.EVENT_TRACK_REACHED_START:
                    notificationTrackReachedStartNative((byte) 0x03, (byte) 1);
                    break;
                case BTAvrcpProfile.EVENT_PLAYBACK_POS_CHANGED:
                    notificationPlayPosChangedNative((byte) 0x03, (byte) 1, 123);
                    break;
                case BTAvrcpProfile.EVENT_BATT_STATUS_CHANGED:
                    notificationBatteryStatusChangedNative((byte) 0x03, (byte) 1, (byte) 2);
                    break;
                case BTAvrcpProfile.EVENT_SYSTEM_STATUS_CHANGED:
                    notificationSystemStatusChangedNative((byte) 0x03, (byte) 1, (byte) 2);
                    break;
                case BTAvrcpProfile.EVENT_PLAYER_APPLICATION_SETTING_CHANGED:
                    byte a[];
                    byte b[];
                    a = new byte[2];
                    b = new byte[2];
                    a[0] = 2;
                    a[1] = 3;
                    b[0] = 1;
                    b[1] = 1;
                    // for PTS Test
                    notificationApplicationSettingChangedNative((byte) OK, (byte) 1, (byte) 2, a, b);
                    break;
                case BTAvrcpProfile.EVENT_NOW_PLAYING_CONTENT_CHANGED:
                    notificationNowPlayingChangedNative((byte) 0x03, (byte) 3);
                    break;
                case BTAvrcpProfile.EVENT_AVAILABLE_PLAYERS_CHANGED:
                    notificationAvailPlayersChangedNative((byte) 0x03, (byte) 3);
                    break;
                case BTAvrcpProfile.EVENT_ADDRESSED_PLAYER_CHANGED:
                    notificationAddressedPlayerChangedNative((byte) 0x03, (byte) 3, (short) 0, (short) 0);
                    break;
                case BTAvrcpProfile.EVENT_UIDS_CHANGED:
                    // PTS test TC_TG_MCN_CB_BV_09_C return a 0 uid-counter
                    notificationUIDSChangedNative((byte) OK, (byte) 1, (short) 0);
                    break;
                case BTAvrcpProfile.EVENT_VOLUME_CHANGED:
                    notificationVolumeChangedNative((byte) 0x03, (byte) 3, (byte) 0);
                    break;
                default:
                    Log.e(TAG, "[BT][AVRCP] register fail but don't know how to reject it");
                    break;

            }
        }
//        else {
//        }
    }

    public void notificationBatteryStatusChanged(byte error, byte isinterim, byte status) {
        notificationBatteryStatusChangedNative(error, isinterim, status);
    }

    public void notificationSystemStatusChanged(byte error, byte isinterim, byte status) {
        notificationSystemStatusChangedNative(error, isinterim, status);
    }

    public void notificationVolumeChanged(byte error, byte isinterim, byte volume) {
        notificationVolumeChangedNative(error, isinterim, volume);
    }

    /*
     * pos change - no track currently select, then return 0xfffffff in interim repsonse
     */

    public void abortContinueInd() {
        Log.v(TAG, "[BT][AVRCP] abortContinueInd");
        if (mAdapter != null) {
            mAdapter.abortContinueInd();
        }
    }

    /* AVRCP 1.4 TG Ind */
    public void setAbsoluteVolumeInd(byte index, byte volume) {
        byte lVol;
        byte newVolume;
        newVolume = (byte) (volume & 0x7f);
        Log.v(TAG, "[BT][AVRCP] set_absolute_volume_ind idx:" + index + " volume:" + volume + "newvol:" + newVolume);

        if (mAdapter.setAbsoluteVolume(newVolume)) {
            // PTS
            lVol = mAdapter.getAbsoluteVolume();
            setAbsoluteVolumeRspNative((byte) 0, OK, (byte) volume);
            return;
        }
        setAbsoluteVolumeRspNative((byte) FAIL, FAIL, (byte) 0);
    }

    public void setAddressedplayerInd(short playerId) {
        Log.v(TAG, String.format("[BT][AVRCP] set_addressedplayer_ind playerId:%d", playerId));
        if (playerId == 1) {
            mCurPlayerId = playerId;
            setAddressedplayerRspNative((byte) 0, BTAvrcpProfile.AVRCP_ERRCODE_OPERATION_COMPLETE);
        } else {
            // PTS - TC_TG_MPS_BI_01_C
            setAddressedplayerRspNative(BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PLAYER_ID,
                    BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS);
        }
    }

    public void getMediaPlayerList(int start, int end) {
        byte mask[];
        byte type;
        byte subtype;
        short uidCounter;
        short playerId;
        byte status = 0x0;

        Log.v(TAG, String.format("[BT][AVRCP] getMediaPlayerList start:%d end:%d  [test:1]", start, end));
        // Compose a list and return it
        mask = new byte[16]; // PTS default 0000 0000 0000 0007 FFF0 0070 0000
        // 00000
        mask[0] = 0x00;
        mask[1] = 0x00;
        mask[2] = 0x00;
        mask[3] = 0x00;

        mask[4] = (byte) 0x00;
        mask[5] = (byte) 0x87; // if support 0x87 -> B7
        mask[6] = (byte) 0x01;
        mask[7] = (byte) 0x38;

        mask[8] = (byte) 0xFB;
        mask[9] = (byte) 0xF0;
        mask[10] = 0x00;
        mask[11] = 0x70;

        mask[12] = 0x00;
        mask[13] = 0x00;
        mask[14] = 0x00;
        mask[15] = 0x00;

        type = 0;
        subtype = 0;
        uidCounter = 0;
        playerId = 1; /*
                       * player Id beginns with 1, not 0 (some carkit cannot handle 0. They treat 0 as default invalid one)
                       */
        status = mAdapter.getPlayerstatus();

        if (start > 1) {
            // error
            getMediaPlayerListRspNative((byte) 0x16, (byte) 0, (byte) 1, status, uidCounter, playerId, type, subtype, mask,
                    new String("Error"));
        } else if (start >= 0 && end >= 1) {
            // ok return success
            if ((end - start) > 1) {
                getMediaPlayerListRspNative(OK, (byte) 0, (byte) 1, status, uidCounter, playerId, type, subtype, mask,
                        new String("Player1"));
                // playerId++;
                // getMediaPlayerListRspNative( OK, (byte) 1, (byte) 2, status,
                // uid_counter, playerId, type, subtype, mask, new
                // String("Player2") );
            } else {
                getMediaPlayerListRspNative(OK, (byte) 0, (byte) 1, status, uidCounter, playerId, type, subtype, mask,
                        new String("Player0"));
            }
        } else {
            // error bounds
            getMediaPlayerListRspNative((byte) 0x16, (byte) 0, (byte) 1, OK, uidCounter, playerId, type, subtype, mask,
                    new String("Error"));
        }
    }

    public void getFileSystemitemsList(int start, int end, byte count, int attrIds[]) {
        byte i = 0;
        short charset = 0x65;
        long uid = 0;
        String sSample = "";

        int attrId = 0;
        byte mediatype = 0;
        short len = 0;

        if (start > end) {
            // PTS TC_TG_MCN_CB_BI_01_C
            Log.w(TAG, String.format("[BT][AVRCP] getFileSystemitemsList Wrong range start:%d end:%d", start, end));
            getFileSystemItemEndRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS,
                    BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
            return;
        }

        /* query the */
        if (null != mBrowserAdapter) {
            mBrowserAdapter.getFileSystemitemsList(start, end, count, attrIds);
        } else {
            getFileSystemItemEndRspNative((byte) 0x03, FAIL, (short) 0);
        }

    }

    public void getSearchResultitemsList(int start, int end, byte count, int attrIds[]) {
        int i = 0;
        int j = 0;
        int total = 0;
        long searchedList[];
        String sSongAttrText;
        String sSongNameText;
        int numAttr;

        Log.v(TAG, String.format("[BT][AVRCP] getSearchResultitemsList start:%d end:%d count:%d", start, end, count));
        for (i = 0; i < count; i++) {
            Log.v(TAG, "[BT][AVRCP] i:" + i + " " + attrIds[i]);
        }

        if (mBrowserAdapter == null) {
            // No any item could be provide
            getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_PLAYER_NOT_ADDRESSED, (short) 0);
            return;
        }

        if (start > end) {
            // No any item could be provide
            getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
            return;
        }

        searchedList = mBrowserAdapter.getSearchedList();

        if (searchedList != null && start >= searchedList.length) {
            // No any item could be provide
            Log
                    .w(TAG, "[BT][AVRCP] getSearchResultitemsList start:" + start + " Searchedlist.length:"
                            + searchedList.length);
            getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
            return;
        }

        if (searchedList != null && 0 == searchedList.length) {
            // No any item could be provide
            getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
            return;
        }

        getSearchedItemStartRspNative();
        if (searchedList != null) {
            if (end >= searchedList.length) {
                end = searchedList.length - 1;
            }

            // (playlist.length-start) the max available number
            if ((end - start + 1) > ((int) searchedList.length - start)) {
                total = ((int) searchedList.length - start);
            } else {
                total = (end - start + 1);
            }

            if (0 == total) {
                // no thing to return
                Log.w(TAG, "[BT][AVRCP] No data to return. list.length" + searchedList.length + " start:" + start + " end:"
                        + end);
                getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
                return;
            }
            Log.w(TAG, "[BT][AVRCP] Searchedlist.length" + searchedList.length + " start:" + start + " end:" + end);
            for (i = start; i <= end && i < searchedList.length; i++) {
                numAttr = 0;
                for (j = 0; j < attrIds.length; j++) {
                    sSongAttrText = mBrowserAdapter.getSearchedItemAttribute(i, attrIds[j]);
                    if (sSongAttrText != null) {
                        getSearchedItemFileAttrRspNative(OK, (byte) i, (byte) numAttr, attrIds[j], UTF8_CHARSET,
                                (short) sSongAttrText.length(), sSongAttrText);
                        numAttr++;
                    }
                }

                sSongNameText = mBrowserAdapter.getSearchedItemAttribute(i, FILENAME_ATTR_ID); // use
                if (null == sSongNameText) {
                    sSongNameText = "";
                }
                Log.v(TAG, "[BT][AVRCP] got i:" + i + " uid:" + searchedList[i] + " SongName:" + sSongNameText);
                getSearchedItemFileRspNative(OK, (byte) i, (byte) total, searchedList[i], BTAvrcpProfile.ITEM_TYPE_ELEMENT,
                        UTF8_CHARSET, (short) sSongNameText.length(), sSongNameText);
            }
            getSearchedItemRspNative(OK, OK, (short) 0);
        } else {
            Log.w(TAG, "[BT][AVRCP] No Searchedlist !");
            getSearchedItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
        }
    }

    public void getNowPlayingitemsList(int start, int end, byte count, int attrIds[]) {
        int i = 0;
        int j = 0;
        int total = 0;
        long playlist[];
        String songName;
        String attrValue;
        int num;
        Log.v(TAG, String.format("[BT][AVRCP] getNowPlayingitemsList start:%d end:%d count:%d", start, end, count));
        for (i = 0; i < count; i++) {
            Log.v(TAG, " i:" + i + " " + attrIds[i]);
        }

        playlist = mAdapter.getNowPlaying();
        if (playlist == null || start > end) {
            // No any item could be provide
            Log.w(TAG, String.format("[BT][AVRCP] getNowPlayingitemsList no playlist !"));
            getNowPlayingItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
            return;
        }

        getNowPlayingItemStartRspNative();
        if (start > playlist.length) {
            // No any item could be provide
            Log.w(TAG, String.format("[BT][AVRCP] getNowPlayingitemsList start:%d playlist:%d", start, playlist.length));
            getNowPlayingItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
        } else {
            if (playlist.length == 0) {
                songName = "empty";
                getNowPlayingItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
                return;
            }

            // (playlist.length-start) the max available number
            if ((end - start + 1) > ((int) playlist.length - start)) {
                total = ((int) playlist.length - start);
            } else {
                total = (end - start + 1);
            }

            if (0 == total) {
                // no thing to return
                Log.w(TAG, "[BT][AVRCP] No data to return. list.length" + playlist.length + " start:" + start + " end:"
                        + end);
                getNowPlayingItemRspNative(FAIL, BTAvrcpProfile.AVRCP_ERRCODE_RANGE_OUT_OF_BOUNDS, (short) 0);
                return;
            }

            Log.w(TAG, "[BT][AVRCP] list.length" + playlist.length + " start:" + start + " end:" + end);

            short uidCounter = mBrowserAdapter.getUidCounter();
            for (i = start; i <= end && i < playlist.length; i++) {
                // step1.configure the attribute list
                num = 0;
                for (j = 0; j < attrIds.length; j++) {
                    // the i-th Song ID and its the j-th attribute
                    attrValue = mBrowserAdapter.getItemAttribute(BTAvrcpProfile.AVRCP_SCOPE_NOW_PLAYING, playlist[i],
                            uidCounter, attrIds[j]);
                    if (null != attrValue) {
                        getNowPlayingItemFileAttrRspNative(OK, (byte) i, (byte) num, attrIds[j], UTF8_CHARSET,
                                (short) attrValue.length(), attrValue);
                        num++;
                    }
                }

                // step2. return the item
                attrValue = mBrowserAdapter.getItemAttribute(BTAvrcpProfile.AVRCP_SCOPE_NOW_PLAYING, playlist[i],
                        uidCounter, 1);
                songName = attrValue;
                if (null == songName) {
                    Log.w(TAG, "[BT][AVRCP] Use 'ID:<id>' as filename");
                    songName = "ID:" + i;
                }
                getNowPlayingItemFileRspNative(OK, (byte) i, (byte) total, playlist[i], BTAvrcpProfile.MEDIA_TYPE_AUDIO,
                        UTF8_CHARSET, (short) songName.length(), songName);
            }

            getNowPlayingItemRspNative(OK, OK, (short) 0);
        }
    }

    public void setBrowsedplayerInd(short playerId) {
        String[] paths = null;
        int depth = 0;
        byte status = 0;
        short uidCounter = 0;
        int num = 0;
        Log.v(TAG, String.format("[BT][AVRCP] setBrowsedplayerInd player_id:%d", playerId));

        if (playerId != 1) {
            // PTS TC_TG_MPS_BI_02_C
            Log.w(TAG, "[BT][AVRCP] invalid player id:" + playerId);
            setBrowsedplayerRspNative(BTAvrcpProfile.AVRCP_ERRCODE_INVALID_PLAYER_ID, (byte) OK, (short) 0, (int) 0,
                    (short) 0x6a, (byte) 0, new String[1]);
            return;
        }

        if (playerId != mCurPlayerId && null != mBrowserAdapter) {
            mBrowserAdapter.onSelect();
            // reject it when browser and player are not the same it
            // invalid player id - PTS test case!
            setBrowsedplayerRspNative(BTAvrcpProfile.AVRCP_ERRCODE_PLAYER_NOT_ADDRESSED, (byte) OK, (short) 0, (int) 0,
                    (short) 0x6a, (byte) 0, new String[1]);
        } else {
            if (null != mBrowserAdapter) {

                uidCounter = mBrowserAdapter.getUidCounter();
                num = mBrowserAdapter.getCurPathItems();
                depth = mBrowserAdapter.getCurPathDepth();
                paths = mBrowserAdapter.getCurPaths();
                Log.w(TAG, String.format("[BT][AVRCP] browserAdapter uid_counter:%d num:%d depth:%d", uidCounter, num,
                        depth));
                setBrowsedplayerRspNative((byte) OK, (byte) OK, (short) uidCounter, (int) num, (short) 0x6a, (byte) depth,
                        paths);
            } else {
                // no selected browser
                setBrowsedplayerRspNative(BTAvrcpProfile.AVRCP_ERRCODE_PLAYER_NOT_ADDRESSED, (byte) OK, (short) 0, (int) 0,
                        (short) 0x6a, (byte) 0, new String[1]);
            }
        }
    }

    public void changePathInd(int uidCounter, byte direction, long uid) {
        int num = 0;
        byte status = 0;
        Log.v(TAG, String.format("[BT][AVRCP] changePathInd uid_counter:%d dir:%d uid:%d", uidCounter, direction, uid));
        if (mBrowserAdapter.changePath((short) uidCounter, direction, uid)) {
            // num = browserAdapter.getCurPathItems();
            // changePathRspNative((byte)OK, status, num);
            Log.v(TAG, "Wait for broserAdapter to call changePathRspNavtive");
            return;
        }
        changePathRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_UID_IS_DIRECTORY, status, num);
    }

    public void getItemAttributesInd(byte scope, long identifier, short uidCounter, byte count, int attributeIds[]) {
        byte i = 0;
        byte error = 1;
        byte status = 0;
        String sAttrArray[];
        String sAttrText;
        int iAvailAttrId[];
        int num = 0;

        Log.v(TAG, String.format("[BT][AVRCP] getItemAttributesInd scope:%d id:%d uid_counter:%d", scope, identifier,
                uidCounter));

        for (i = 0; i < count & count != 0xff; i++) {
            Log.v(TAG, " i:" + i + " " + attributeIds[i]);
        }

        /* Only AP know all the */
        if (count == 0) {
            count = 3;
            attributeIds = new int[3];
            attributeIds[0] = 1;
            attributeIds[1] = 2;
            attributeIds[2] = 3;
        }

        if (identifier == 0) {
            // 0 is current playing
            sAttrArray = new String[count];
            iAvailAttrId = new int[count];
            for (i = 0; i < count; i++) {
                sAttrText = mAdapter.getElementAttribute(identifier, attributeIds[i]);
                if (null != sAttrText) {
                    iAvailAttrId[num] = attributeIds[i];
                    sAttrArray[num] = sAttrText;
                    num++;
                }
            }
            Log.v(TAG, "[BT][AVRCP] getItemAttributesInd 0(playing) num:" + num);
            for (byte loop = 0; loop < num; loop++) {
                Log.v(TAG, String.format("[BT][AVRCP] getItemAttributesInd id:%d array:%d ", iAvailAttrId[loop],
                        UTF8_ARRAY[loop])
                        + " s:" + sAttrArray[loop]);
            }
            getItemattributesRspNative(OK, STATUS_OK, (byte) num, iAvailAttrId, UTF8_ARRAY, sAttrArray);
            return;
        }

        if (scope <= BTAvrcpProfile.AVRCP_SCOPE_NOW_PLAYING) {
            sAttrArray = new String[count];
            iAvailAttrId = new int[count];
            if (mBrowserAdapter.isItemExist(scope, identifier, uidCounter)) {
                num = 0;
                for (i = 0; i < count; i++) {
                    // bypass to browser to get all item attributes
                    sAttrText = mBrowserAdapter.getItemAttribute(scope, identifier, uidCounter, attributeIds[i]);
                    if (null != sAttrText) {
                        iAvailAttrId[num] = attributeIds[i];
                        sAttrArray[num] = sAttrText;
                        num++;
                    }
                }
                error = OK;
                Log.v(TAG, "[BT][AVRCP] getItemAttributesInd num:" + num);
                for (byte loop = 0; loop < num; loop++) {
                    Log.v(TAG, String.format("[BT][AVRCP] getItemAttributesInd id:%d array:%d ", iAvailAttrId[loop],
                            UTF8_ARRAY[loop])
                            + " s:" + sAttrArray[loop]);
                }
                getItemattributesRspNative(OK, STATUS_OK, (byte) num, iAvailAttrId, UTF8_ARRAY, sAttrArray);
            } else {
                error = 1;
                status = BTAvrcpProfile.AVRCP_ERRCODE_NOT_EXIST;
                getItemattributesRspNative(FAIL, status, (byte) 0, new int[0], new short[0], new String[0]);
                Log.v(TAG, "[BT][AVRCP] getItemAttributesInd Item not exist");
            }
        } else {
            error = 1;
            status = BTAvrcpProfile.AVRCP_ERRCODE_INVALID_SCOPE;
            getItemattributesRspNative(FAIL, status, (byte) 0, new int[0], new short[0], new String[0]);
        }
    }

    public void playItemsInd(byte scope, long uid, short uidCounter) {
        byte status = 0;
        Log.v(TAG, String.format("[BT][AVRCP] playItemsInd idx:%d uid:%d player_id:%d", scope, uid, uidCounter));

        if (uidCounter == mBrowserAdapter.getUidCounter()) {
            if (scope == 0x01 && !mBrowserAdapter.checkSongIdExisted(uid)) {
                Log.w(TAG, "[BT][AVRCP] playItemsInd Warning try to add a not exist id:" + uid);
                playItemsRspNative((byte) 9, (byte) 9); // AVRCP_ERRCODE_NOT_EXIST
                return;
            } else {
                if (mAdapter.playItems(uid)) {
                    playItemsRspNative((byte) OK, OK);
                    return;
                } else {
                    playItemsRspNative((byte) FAIL, BTAvrcpProfile.AVRCP_ERRCODE_NOW_PLAYING_FULL);
                }
            }
        } else {
            // uid not match
            playItemsRspNative((byte) BTAvrcpProfile.AVRCP_ERRCODE_UID_CHANGED, status);
        }

    }

    public void searchInd(short charset, String text) {

        Log.v(TAG, String.format("[BT][AVRCP] searchInd charset:0x%x text:'%s'", charset, text));

        if (!mBrowserAdapter.search(text)) {
            searchRspNative((byte) 0x10, (byte) 0, (short) 0, 0); // AVRCP_ERRCODE_SEARCH_IN_PROGRESS
        }

        // if it is true. BrowserAdapter will invoke response
    }

    public void addTonowplayingInd(byte scope, long uid, short uidCounter) {
        Log.v(TAG, String.format("[BT][AVRCP] addTonowplayingInd scope:%d  uid:%d uid_counter:%d", scope, uid, uidCounter));

        if (uidCounter == mBrowserAdapter.getUidCounter()) {
            if (scope == 0x01 && !mBrowserAdapter.checkSongIdExisted(uid)) {
                // item is not exist
                Log.w(TAG, "[BT][AVRCP] addTonowplayingInd Warning try to add a not exist id:" + uid);
                addTonowplayingRspNative((byte) 9, (byte) 9); // AVRCP_ERRCODE_NOT_EXIST
                return;
            } else {
                if (mAdapter.addToNowPlaying(uid)) {
                    addTonowplayingRspNative((byte) OK, (byte) 4);
                    return;
                }
            }
            addTonowplayingRspNative((byte) 0x0e, (byte) 0x0e); // AVRCP_ERRCODE_NOW_PLAYING_FULL
            return;
        } else {
            Log.w(TAG, "[BT][AVRCP] addTonowplayingInd Warning try to add a not exist id:" + uid);
        }

        addTonowplayingRspNative((byte) 1, (byte) 1);
    }

    /* AVRCP 1.4 TG Rsp */
    /* get player list */
    /* get folder list */
    /* get search result list */
    public native boolean setBrowsedplayerRspNative(byte error, byte status, short uidCounter, int num, short charset,
            byte depth, String foldername[]);

    public native boolean changePathRspNative(byte error, byte status, int numOfItems);

    public native boolean getItemattributesRspNative(byte error, byte status, byte count, int attributeIds[],
            short charsets[], String attributeValues[]);

    public native boolean playItemsRspNative(byte error, byte status);

    public native boolean searchRspNative(byte error, byte status, short uidCounter, int numOfItems);

    public native boolean addTonowplayingRspNative(byte error, byte status);

    public native boolean notificationVolumeChangedNative(byte error, byte isinterim, byte volume);

    public native boolean notificationAddressedPlayerChangedNative(byte error, byte isinterim, short playerId,
            short uidCounter);

    public native boolean notificationAvailPlayersChangedNative(byte error, byte isinterim);

    public native boolean notificationUIDSChangedNative(byte error, byte isinterim, short uidCounter);

    public native boolean notificationNowPlayingChangedNative(byte error, byte isinterim);

    /* Media Player list */
    public native boolean getMediaPlayerListRspNative(byte error, byte status, byte item, byte total, short uidCounter,
            short plauerId, byte type, byte subtype, byte mask[], String playerName);

    /* File System List */
    public native boolean getFileSystemItemStartRspNative();

    public native boolean getFileSystemItemFolderRspNative(byte error, byte item, byte total, long uid, byte foldertype,
            byte playable, short charset, short len, String foldername);

    public native boolean getFileSystemItemFileAttrRspNative(byte error, byte item, byte attrIndex, int attrId,
            short charset, short len, String attrValue);

    public native boolean getFileSystemItemFileRspNative(byte error, byte item, byte total, long uid, byte mediatype,
            short charset, short len, String filename);

    public native boolean getFileSystemItemEndRspNative(byte error, byte status, short uidCounter);

    /* Search Result List */
    public boolean getSearchedItemStartRspNative() {
        return getFileSystemItemStartRspNative();
    }

    public boolean getSearchedItemFileAttrRspNative(byte error, byte item, byte attrIndex, int attrId, short charset,
            short len, String attrValue) {
        return getFileSystemItemFileAttrRspNative(error, item, attrIndex, attrId, charset, len, attrValue);
    }

    public boolean getSearchedItemFileRspNative(byte error, byte item, byte total, long uid, byte mediatype, short charset,
            short len, String filename) {
        // use the same function
        // if( BTAvrcpProfile.ITEM_TYPE_ELEMENT != mediatype){
        // Log.v(TAG,
        // "[BT][AVRCP] getNowPlayingItemFileRspNative wrong mediatype:" +
        // mediatype ); // Search result should only element type
        // }
        return getFileSystemItemFileRspNative(error, item, total, uid, mediatype, charset, len, filename);
    }

    /**
     * @param error return success or fail
     * @param status fail response's status field
     */
    public boolean getSearchedItemRspNative(byte error, byte status, short uidCounter) {
        return getFileSystemItemEndRspNative(error, status, uidCounter);
    }

    /* Now Playing List */
    public boolean getNowPlayingItemStartRspNative() {
        return getFileSystemItemStartRspNative();
    }

    public boolean getNowPlayingItemFileAttrRspNative(byte error, byte item, byte attrIndex, int attrId, short charset,
            short len, String attrValue) {
        return getFileSystemItemFileAttrRspNative(error, item, attrIndex, attrId, charset, len, attrValue);
    }

    public boolean getNowPlayingItemFileRspNative(byte error, byte item, byte total, long uid, byte mediatype,
            short charset, short len, String filename) {
        // use the same function
        // if( BTAvrcpProfile.ITEM_TYPE_ELEMENT != mediatype){
        // Log.v(TAG,
        // "[BT][AVRCP] getNowPlayingItemFileRspNative wrong mediatype:" +
        // mediatype ); // Now playing should only element type
        // }
        if (mediatype != 0 && mediatype != 1) {
            Log.v(TAG, "[BT][AVRCP] wrong mediatype:" + mediatype); // only
            // audio or
            // video
            // type
            mediatype = 0;
        }
        return getFileSystemItemFileRspNative(error, item, total, uid, mediatype, charset, len, filename);
    }

    public boolean getNowPlayingItemRspNative(byte error, byte status, short uidCounter) {
        return getFileSystemItemEndRspNative(error, status, uidCounter);
    }

    /**/
    private final IBinder mBinder = new ServiceStub(this);

    static class ServiceStub extends IBTAvrcpService.Stub {
        WeakReference<BluetoothAvrcpService> mService;

        ServiceStub(BluetoothAvrcpService service) {
            mService = new WeakReference<BluetoothAvrcpService>(service);
        }

        public byte getStatus() {
            return OK;
        }

        public boolean connect(String sAddr) {
            if (sAddr.length() == 17) {
                mService.get().connectNative(sAddr);
                return true;
            }
            return false;
        }

        public boolean disconnect() {
            if (true) {
                mService.get().disconnectNative();
                return true;
            }
            return false;
        }

        public boolean connectBrowse() {
            if (true) {
                mService.get().connectBrowseNative();
                return true;
            }
            return false;
        }

        public boolean disconnectBrowse() {
            if (true) {
                mService.get().disconnectBrowseNative();
                return true;
            }
            return false;
        }

        public boolean setDebugElementAttribute() {
            // test the browser
            int[] attrIds;
            byte start;
            byte end;
            byte count;

            attrIds = new int[3];
            attrIds[0] = 0x01;
            attrIds[1] = 0x05;
            attrIds[2] = 0x02;

            mService.get().connectInd(new byte[0], "Test", 0);

            start = 0;
            end = 10;
            count = 3;
            // mService.get().getSearchResultitemsList(start, end, count,
            // attr_ids);
            // mService.get().informBatteryStatusofctInd((byte)3);
            // mService.get().getNowPlayingitemsList(start, end, count,
            // attr_ids);

            start = 5;
            end = 10;
            count = 3;
            // mService.get().getSearchResultitemsList(start, end, count,
            // attr_ids);
            // mService.get().informBatteryStatusofctInd((byte)3);
            // mService.get().getNowPlayingitemsList(start, end, count,
            // attr_ids);

            start = 0;
            end = 10;
            count = 3;
            mService.get().setBrowsedplayerInd((short) 1);

            mService.get().getItemAttributesInd((byte) 0, (long) 1, (short) 0, count, attrIds);
            mService.get().getItemAttributesInd((byte) 1, (long) 1, (short) 0, count, attrIds);
            mService.get().getItemAttributesInd((byte) 2, (long) 1, (short) 0, count, attrIds);
            mService.get().getItemAttributesInd((byte) 3, (long) 1, (short) 0, count, attrIds);
            mService.get().getItemAttributesInd((byte) 4, (long) 1, (short) 0, count, attrIds);

            long uid1 = Long.MAX_VALUE;
            long uid2 = Long.MIN_VALUE;
            Log.v(TAG, "[BT][AVRCP] uid1 " + uid1);
            mService.get().changePathInd((short) 0, (byte) 1, uid1); // 1 is
            // down
            Log.v(TAG, "[BT][AVRCP] uid2 " + uid2);
            mService.get().changePathInd((short) 0, (byte) 1, uid2); // 1 is
            // down

            debugPTSAttributes(3);
            return true;
        }

        public boolean selectPlayerId(int playerid) {
            short playerId = (short) playerid;
            if (null != mService.get()) {
                if (playerId == 1 || playerId == 2) {
                    // m
                    mService.get().mCurPlayerId = playerId;
                    Log.v(TAG, "[BT][AVRCP] player_id:" + playerId + " availFalg:"
                            + mService.get().mTrackAvailableChangedFlag + " addresflag:"
                            + mService.get().mTrackAddressedPlayerChangedFlag);
                    if (mService.get().mTrackAvailableChangedFlag) {
                        //
                        mService.get().mTrackAvailableChangedFlag = false;
                        mService.get().notificationAvailPlayersChangedNative(OK, (byte) 0); // 0
                        // final
                        // response
                    }

                    if (mService.get().mTrackAddressedPlayerChangedFlag) {
                        // send it back
                        mService.get().mTrackAddressedPlayerChangedFlag = false;
                        mService.get().notificationAddressedPlayerChangedNative(OK, (byte) 0, (short) playerId, (short) 0);
                    }
                } else {
                    Log.e(TAG, "[BT][AVRCP] Out of range player_id:" + playerId);
                }
            } else {
                Log.v(TAG, "[BT][AVRCP] mService not exist ");
            }
            return false;
        }

        public boolean debugPTSAttributes(int mode) {

            if (mode == 0xAA) {
                Log.e(TAG, "[BT][AVRCP] stop server self !");
                mService.get().disable();
                mService.get().stopSelf();
                return true;
            }

            if (null != mService.get()) {
                Log.e(TAG, "[BT][AVRCP] debugPTSAttributes mode to " + mode);
                mService.get().sPTSDebugMode = mode;
                return true;
            } else {
                Log.v(TAG, "[BT][AVRCP] mService not exist ");
            }
            return false;
        }
    }
}
