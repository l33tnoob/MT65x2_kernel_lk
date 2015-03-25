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

package com.mediatek.omacp.message;

import static android.content.Intent.ACTION_BOOT_COMPLETED;
import static android.provider.Telephony.Sms.Intents.WAP_PUSH_RECEIVED_ACTION;

import android.app.Service;
import android.content.ContentValues;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.provider.Telephony;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.omacp.R;
import com.mediatek.omacp.parser.ApplicationClass;
import com.mediatek.omacp.parser.NapdefClass;
import com.mediatek.omacp.parser.OmacpParser;
import com.mediatek.omacp.parser.ProxyClass;
import com.mediatek.omacp.provider.OmacpProviderDatabase;
import com.mediatek.omacp.utils.MTKlog;

import java.util.ArrayList;
import java.util.HashMap;

public class OmacpReceiverService extends Service {

    private static final String TAG = "Omacp/OmacpReceiverService";

    private static final boolean DEBUG = true;

    private ServiceHandler mServiceHandler;

    private Looper mServiceLooper;

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void onCreate() {
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService onCreate");
        }

        // Start up the thread running the service. Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
        HandlerThread thread = new HandlerThread("Omacp", Process.THREAD_PRIORITY_BACKGROUND);
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService onStartCommand");
        }

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
        return Service.START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService onDestroy");
        }

        mServiceLooper.quit();
    }

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        /**
         * Handle incoming transaction requests. The incoming requests are
         * initiated by the MMSC Server or by the MMS Client itself.
         */
        @Override
        public void handleMessage(Message msg) {
            int serviceId = msg.arg1;
            Intent intent = (Intent) msg.obj;
            if (intent != null) {
                String action = intent.getAction();

                int error = intent.getIntExtra("errorCode", 0);

                if (WAP_PUSH_RECEIVED_ACTION.equals(action)) {
                    handleOmacpReceived(intent, error);
                } else if (ACTION_BOOT_COMPLETED.equals(action)) {
                    OmacpMessageNotification.blockingUpdateNewMessageIndicator(
                            OmacpReceiverService.this, false);
                }
            }

            // NOTE: We MUST not call stopSelf() directly, since we need to
            // make sure the wake lock acquired by AlertReceiver is released.
            OmacpReceiver.finishStartingService(OmacpReceiverService.this, serviceId);
        }
    }

    private void handleOmacpReceived(Intent intent, int error) {
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService handleOmacpReceived: " + intent.getAction());
        }

        // get info from intent
        byte[] intentData = intent.getByteArrayExtra("data");
        // handle sec and pin code
        boolean isCorrect = false;
        NetWorkPinData pinData = new NetWorkPinData();
        isCorrect = handlePinVerify(intent, intentData, pinData);
        if (!isCorrect) {
            return;
        }
        ContentValues value = new ContentValues();
        // Parse the omacp Message
        try {
            parseOmacpMessage(intent, intentData, value, pinData);
            // Store the omacp message
            Uri uri = this.getContentResolver().insert(OmacpProviderDatabase.CONTENT_URI, value);
            if (uri != null) {
                OmacpMessageNotification.blockingUpdateNewMessageIndicator(this, true);
                // Modify the shared preference value to indicate the mms that
                // configuration message exist
                SharedPreferences sh = this.getSharedPreferences("omacp", this.MODE_WORLD_READABLE);
                Editor editor = sh.edit();
                editor.putBoolean("configuration_msg_exist", true);
                editor.commit();
            }
        }catch (IllegalArgumentException e){
            MTKlog.e(TAG, "intentData START_TAG type is null.");
            return;
        }
    }

    private class NetWorkPinData {
        private String mSec;

        private String mMac;
    }

    private boolean handlePinVerify(Intent intent, byte[] intentData, NetWorkPinData pinData) {
        HashMap<String, String> contentTypeParamaters = (HashMap<String, String>) intent
                .getSerializableExtra("contentTypeParameters");
        String sec = null;
        String mac = null;
        if (contentTypeParamaters == null && DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService contentTypeParamaters is null.");
        } else {
            pinData.mMac = contentTypeParamaters.get("MAC");
            pinData.mSec = contentTypeParamaters.get("SEC");
            sec = pinData.mSec;
            mac = pinData.mMac;
            if (DEBUG) {
                MTKlog.i(TAG, "OmacpReceiverService handleOmacpReceived: MAC is : " + mac + " "
                        + "SEC is : " + sec);
            }
        }

        // sec == "" indicates Network PIN, in this case, check SIM's IMSI, if
        // verify fails, then ignore this message
        if (sec != null && sec.equalsIgnoreCase("") && mac != null && !mac.equalsIgnoreCase("")) {
            int simId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
            String imsi = OmacpMessageSettingsDetail.getSimImsi(simId);
            byte[] key = OmacpMessageSettingsDetail.imsiToKey(imsi);
            if (key == null) {
                return false;
            }
            boolean correct = OmacpMessageSettingsDetail.verifyPin(key, 0, intentData, mac);

            if (!correct) {
                MTKlog.e(TAG,
                        "OmacpReceiverService Network PIN IMSI verify failed. Will ignore this message.");
                return false;
            }
        }
        return true;
    }

    private void parseOmacpMessage(Intent intent, byte[] intentData, ContentValues value, NetWorkPinData pinData) {
        ArrayList<ApplicationClass> apList = null;
        ArrayList<NapdefClass> napList = null;
        ArrayList<ProxyClass> pxList = null;
        String contextName = null;

        OmacpParser parser = new OmacpParser();
        getParser(intent, parser, intentData);
        apList = parser.getApSectionList();
        napList = parser.getNapList();
        pxList = parser.getPxList();
        contextName = parser.getContextName();

        // If the xml corrupt, then do not store it, just ignore it
        if ((apList == null || apList.isEmpty()) && (napList == null || napList.isEmpty())
                && (pxList == null || pxList.isEmpty())) {
            MTKlog.e(TAG, "OmacpReceiverService handleOmacpReceived parser error.");
            return;
        }

        int currentTime = (int) (System.currentTimeMillis() / 1000);
        String summary = getSavedSummary(apList, napList);
        // get sender and service center address from intent
        Bundle bundle = intent.getExtras();
        String sender = bundle.getString(Telephony.WapPush.ADDR);
        String serviceCenter = bundle.getString(Telephony.WapPush.SERVICE_ADDR);
        // get sim id
        int simId = intent.getIntExtra(PhoneConstants.GEMINI_SIM_ID_KEY, -1);
        if (DEBUG) {
            MTKlog.i(TAG, "OmacpReceiverService handleOmacpReceived from : " + sender + " "
                    + "service center is : " + serviceCenter + " " + "simId is : " + simId);
        }

        if (sender == null || serviceCenter == null) {
            MTKlog.e(TAG,
                    "OmacpReceiverService handleOmacpReceived: sender or service center is null!");
            return;
        }
        value.put("sim_id", simId);
        value.put("sender", sender);
        value.put("service_center", serviceCenter);
        value.put("seen", 0);
        value.put("read", 0);
        value.put("date", currentTime);
        value.put("installed", 0);
        String sec = pinData.mSec;
        String mac = pinData.mMac;
        if (sec == null || mac == null || sec.equalsIgnoreCase("") || mac.equalsIgnoreCase("")) {
            value.put("pin_unlock", 1);
        } else {
            value.put("pin_unlock", 0);
        }
        value.put("sec", sec);
        value.put("mac", mac);
        value.put("title", this.getString(R.string.configuration_message));
        value.put("summary", summary);
        value.put("body", intentData);
        value.put("context", contextName);
        value.put("mime_type", intent.getType());
    }

    private void getParser(Intent intent, OmacpParser parser, byte[] intentData) {
        String mimeType = intent.getType();
        if (mimeType.equalsIgnoreCase("text/vnd.wap.connectivity-xml")) {
            parser.setParser(OmacpParser.getTextParser());
        } else if (mimeType.equalsIgnoreCase("application/vnd.wap.connectivity-wbxml")) {
            parser.setParser(OmacpParser.getWbxmlParser());
        }
        if (intentData != null) {
            parser.parse(intentData);
        }
    }

    private String getSavedSummary(ArrayList<ApplicationClass> apList,
            ArrayList<NapdefClass> napList) {
        StringBuilder buf = new StringBuilder();

        ArrayList<String> list = new ArrayList<String>();

        if (apList != null) {
            int size = apList.size();
            for (int i = 0; i < size; i++) {
                ApplicationClass application = apList.get(i);
                getApplicationSummary(application, list);
            }
        }

        if (napList != null && !napList.isEmpty()) {
            list.add("apn");
        }

        int size = list.size();
        for (int i = 0; i < size; i++) {
            buf.append(list.get(i));
            buf.append(",");
        }

        return buf.toString();
    }

    private void getApplicationSummary(ApplicationClass application, ArrayList<String> list) {
        String appId = application.mAppid;
        String validAppId = null;
        if (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.SMTP_APPID)
                || application.mAppid.equalsIgnoreCase(OmacpMessageUtils.POP3_APPID)
                || application.mAppid.equalsIgnoreCase(OmacpMessageUtils.IMAP4_APPID)) {
            validAppId = OmacpMessageUtils.SMTP_APPID;
        } else if (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_APPID)
                && application.mCm != null) {
            // remove invalid mms setting if it only has mmsc, because
            // it moved to apn
            validAppId = OmacpMessageUtils.MMS_APPID;
        } else if (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_2_APPID)
                && (application.mCm != null || application.mRm != null || application.mMs != null
                        || application.mPcAddr != null || application.mMa != null)) {
            // if ap0005 mms setting only has mmsc, then ignore it,
            // because it has been moved to apn
            validAppId = OmacpMessageUtils.MMS_APPID;
        } else {
            validAppId = appId;
        }
        if (validAppId != null && !list.contains(validAppId)) {
            list.add(validAppId);
        }
    }

}
