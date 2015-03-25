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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.text.SpannableStringBuilder;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.omacp.R;
import com.mediatek.omacp.parser.ApplicationClass;
import com.mediatek.omacp.parser.ApplicationClass.AppAddr;
import com.mediatek.omacp.parser.ApplicationClass.AppAuth;
import com.mediatek.omacp.parser.ApplicationClass.Port;
import com.mediatek.omacp.parser.ApplicationClass.Resource;
import com.mediatek.omacp.parser.NapdefClass;
import com.mediatek.omacp.parser.NapdefClass.NapAuthInfo;
import com.mediatek.omacp.parser.NapdefClass.Validity;
import com.mediatek.omacp.parser.OmacpParser;
import com.mediatek.omacp.parser.ProxyClass;
import com.mediatek.omacp.parser.ProxyClass.PxAuthInfo;
import com.mediatek.omacp.parser.ProxyClass.PxPhysical;
import com.mediatek.omacp.provider.OmacpProviderDatabase;
import com.mediatek.omacp.utils.MTKlog;
import com.mediatek.telephony.TelephonyManagerEx;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.SignatureException;
import java.util.ArrayList;
import java.util.HashMap;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import android.content.res.Configuration;

public class OmacpMessageSettingsDetail extends Activity implements OnClickListener {

    private static final int USERPIN = 0;

    private static final int NETWPIN = 1;

    private static final int USERNETWPIN = 2;

    private static final int USERPINMAC = 3;

    private static final String TAG = "Omacp/OmacpMessageSettingsDetail";

    private static final boolean DEBUG = true;

    private static final String[] INSTALLATION_PROJECTION = {
            OmacpProviderDatabase._ID, OmacpProviderDatabase.SIM_ID,
            OmacpProviderDatabase.INSTALLED, OmacpProviderDatabase.PIN_UNLOCK,
            OmacpProviderDatabase.SEC, OmacpProviderDatabase.MAC, OmacpProviderDatabase.BODY,
            OmacpProviderDatabase.CONTEXT, OmacpProviderDatabase.MIME_TYPE
    };

    private static final int ID = 0;

    private static final int SIM_ID = 1;

    private static final int INSTALLED = 2;

    private static final int PIN_LOCK = 3;

    private static final int SEC = 4;

    private static final int MAC = 5;

    private static final int BODY = 6;

    private static final int CONTEXT = 7;

    private static final int MIME_TYPE = 8;

    private Object mMarkAsBlockedSyncer = new Object();

    private static final String HMAC_SHA1_ALGORITHM = "HmacSHA1";

    private boolean mMarkAsReadBlocked;

    private TextView mDetailText;

    private Button mFullInstallBtn;

    private Button mCustomInstallBtn;

    // IDs for alert dialog
    private static final int DIALOG_INSTALLING = 8000;

    private static final int DIALOG_RE_INSTALL_NOTIFY = 8001;

    private static final int DIALOG_UNLOCK_PIN = 8002;

    private static final int DIALOG_UNLOCK_PIN_2 = 8003;

    // IDs for event
    private static final int EVENT_APPLICATION_INSTALL_TIME_OUT = 2000;

    private static final int EVENT_APN_INSTALL_TIME_OUT = 2001;

    private static final int INSTALL_TIME_OUT_LENGTH = 140000;

    private static final int EVENT_APN_SWITCH_TIME_OUT = 2002;

    private static final int APN_SWITCH_TIME_OUT_LENGTH = 30000;

    // Result constant
    private static final int RESULT_CONSTANT_SUCCEED = 1;

    private static final int RESULT_CONSTANT_NOT_RETURNED = 0;

    private static final int RESULT_CONSTANT_FAILED = -1;

    private static long sMessageId = -1;

    private static int sSimId = -1;

    private static boolean sInstalled = false;

    private boolean mReInstall = false;

    private static boolean sPinUnlock = false;

    private static int sSec = -1;

    private static String sMac;

    private static byte[] sBody;

    private static String sContextIdentifier; // currently not used

    private static String sMimeType;

    private static boolean sIsFullInstallation = true;

    private ArrayList<String> mApSettingsListName;

    private ArrayList<Boolean> mApSettingsListNameChecked = new ArrayList<Boolean>();

    private ArrayList<ResultType> mApplicationResultList = new ArrayList<ResultType>();

    private ResultType mApnResultObj;

    private ArrayList<ApplicationClass> mApList = null;

    private ArrayList<NapdefClass> mNapList = null;

    private ArrayList<ProxyClass> mPxList = null;

    private AlertDialog mCustomDialog;

    private static final String APP_ID_KEY = "appId";

    private static final String APP_SETTING_ACTION = "com.mediatek.omacp.settings";

    private static final String APP_SETTING_RESULT_ACTION = "com.mediatek.omacp.settings.result";

    private static TelephonyManager sTeleManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);

        setContentView(R.layout.message_settings_detail);

        mDetailText = (TextView) findViewById(R.id.message_settings_detail_info);
        mFullInstallBtn = (Button) findViewById(R.id.full_install_btn);
        mCustomInstallBtn = (Button) findViewById(R.id.custom_install_btn);

        mFullInstallBtn.setOnClickListener(this);
        mCustomInstallBtn.setOnClickListener(this);

        Intent intent = getIntent();

        if (DEBUG) {
            MTKlog.i(TAG, "OmacpMessageSettingsDetail onCreate savedInstanceState = "
                    + savedInstanceState + " intent = " + intent);
        }

        initActivityState(intent);

        registerReceiver(mResultReceiver, new IntentFilter(APP_SETTING_RESULT_ACTION));

        setProgressBarIndeterminateVisibility(false);

        sTeleManager = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
    }

    @Override
    protected void onDestroy() {
        removeDialog(DIALOG_INSTALLING);
        if (mIsNetworkStateReceiverRegistered) {
            unregisterReceiver(mNetworkStateIntentReceiver);
        }
        unregisterReceiver(mResultReceiver);
        mTimerHandler.removeMessages(EVENT_APPLICATION_INSTALL_TIME_OUT);
        mTimerHandler.removeMessages(EVENT_APN_INSTALL_TIME_OUT);
        mTimerHandler.removeMessages(EVENT_APN_SWITCH_TIME_OUT);
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
    MTKlog.d(TAG, "onConfigurationChanged-Start");
    super.onConfigurationChanged(newConfig);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_INSTALLING:
                ProgressDialog progressDialog = new ProgressDialog(this);
                progressDialog.setMessage(getResources().getString(
                        R.string.installing_progress_message));
                progressDialog.setIndeterminate(true);
                progressDialog.setCancelable(false);
                return progressDialog;

            case DIALOG_RE_INSTALL_NOTIFY:
                return new AlertDialog.Builder(this).setCancelable(true)
                        .setMessage(R.string.re_install_notify_message)
                        .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                // TODO Auto-generated method stub
                                mReInstall = true;
                                handleInstall();
                            }
                        }).setNegativeButton(R.string.no, null)
                        .setTitle(R.string.re_install_notify_title).create();

            case DIALOG_UNLOCK_PIN:
                return showUnLockPinDialog(DIALOG_UNLOCK_PIN_2);

            case DIALOG_UNLOCK_PIN_2:
               return showUnLockPinDialog(DIALOG_UNLOCK_PIN);

            default:
                return null;
        }
    }

    private Dialog showUnLockPinDialog(final int dialogUnlockPin) {
        LayoutInflater factory = LayoutInflater.from(this);
        final View textEntryView = factory.inflate(R.layout.unlock_pin_dialog_text_entry, null);
        final EditText editText = (EditText) ((LinearLayout) textEntryView)
                .findViewById(R.id.pin_edit);
        AlertDialog.Builder builder = new AlertDialog.Builder(this)
                .setTitle(R.string.unlock_pin_dialog_title).setCancelable(true)
                .setView(textEntryView)
                .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        /* User clicked OK so do some stuff */
                        String inputPin = editText.getText().toString();
                        if (!isPinCorrect(inputPin.getBytes())) {
                            if (DEBUG) {
                                MTKlog.d(TAG, "OmacpMessageSettingsDetail pin unlock failed, inputPin is : " + inputPin);
                            }
                            showDialog(dialogUnlockPin);
                        } else {
                            sPinUnlock = true;
                            // write the database
                            markMessageAsPinUnlock();
                            handleInstall();
                        }
                        editText.setText("");
                    }
                }).setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        /* User clicked OK so do some stuff */
                        editText.setText("");
                    }
                });
        builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
            public void onCancel(DialogInterface arg0) {
                // TODO Auto-generated method stub
                editText.setText("");
            }
        });
        AlertDialog pinDialog = builder.create();
        pinDialog.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
        return pinDialog;
    }

    private void markMessageAsPinUnlock() {
        new Runnable() {
            public void run() {
                synchronized (mMarkAsBlockedSyncer) {
                    if (mMarkAsReadBlocked) {
                        try {
                            mMarkAsBlockedSyncer.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }

                    ContentResolver resolver = OmacpMessageSettingsDetail.this.getContentResolver();

                    ContentValues values = new ContentValues(1);
                    values.put("pin_unlock", 1);

                    Uri messageUri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI,
                            sMessageId);

                    resolver.update(messageUri, values, null, null);
                }
            }
        }.run();
    }

    private boolean isPinCorrect(byte[] pin) {
        if (pin == null || pin.length == 0) {
            return false;
        }
        byte[] key;
        // if it is USERNETWPIN, then use imsi + user pin
        if (sSec == USERNETWPIN) {
            String imsi = getSimImsi(sSimId);
            byte[] imsiKey = imsiToKey(imsi);
            if (imsiKey == null) {
                return false;
            }

            int lenPin = pin.length;
            int lenImsiKey = imsiKey.length;
            int lenKey = lenPin + lenImsiKey;
            key = new byte[lenKey];
            for (int i = 0; i < lenKey; i++) {
                if (i < lenImsiKey) {
                    key[i] = imsiKey[i];
                } else {
                    key[i] = pin[i - lenImsiKey];
                }
            }
        } else {
            key = pin;
        }
        return verifyPin(key, sSec, sBody, sMac);
    }

    public static String getSimImsi(int simId) {
        String imsi;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            TelephonyManagerEx mTelephonyManager = TelephonyManagerEx.getDefault();
            imsi = mTelephonyManager.getSubscriberId(simId);
        } else {
            imsi = sTeleManager.getSubscriberId();
        }

        MTKlog.i(TAG, "imsi is : " + imsi);
        return imsi;
    }

    public static boolean verifyPin(byte[] pin, int sec, byte[] body, String mac) {

        switch (sec) {

            // NETWPIN, USERPIN, USERNETWPIN using the same authentication
            // method: M = HMAC - SHA(K, A)
            case USERPIN:
            case NETWPIN:
            case USERNETWPIN:
                String inputMac = null;
                try {
                    inputMac = calculateRFC2104HMAC(body, pin);
                } catch (SignatureException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
                return (inputMac != null && inputMac.equalsIgnoreCase(mac));

                // USERPINMAC
            case USERPINMAC:
                return verifyUSERPINMAC(pin);

                // error, then return false
            default:
                return false;
        }
    }

    private static boolean verifyUSERPINMAC(byte[] key) {

        // calculate mMac's m(i)
        String mMacResult = "";
        char[] mMacCharArray = sMac.toCharArray();
        int length = mMacCharArray.length;
        for (int i = 0; i < length; i++) {
            mMacResult += Integer.parseInt(mMacCharArray[i] + "") % 10 + 48;
        }

        // calculate user's m'(i)

        // first calculate the user's MAC
        String userMac = null;
        try {
            userMac = calculateRFC2104HMAC(sBody, key);
        } catch (SignatureException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        // then calculate the user's m'(i) based on user's MAC
        String userMacResult = null;
        if (userMac == null) {
            MTKlog.e(TAG, "OmacpMessageSettingsDetail verifyUSERPINMAC userMac is null.");
            return false;
        }

        char[] userMacCharArray = userMac.toCharArray();
        int lengthUser = userMacCharArray.length;
        for (int i = 0; i < lengthUser; i++) {
            userMacResult += Integer.parseInt(userMacCharArray[i] + "") % 10 + 48;
        }

        // compare m(i) and m'(i)
        return mMacResult.equalsIgnoreCase(userMacResult);
    }

    /**
     * @param data The data to be signed. (The wbxml as a string of Hex digits.)
     * @param key The signing key. (E.g. USERPIN of '1234')
     */
    public static String calculateRFC2104HMAC(byte[] data, byte[] pin)
            throws SignatureException {

        String result;

        try {

            // Get an hmac_sha1 key from the raw key bytes
            byte[] keyBytes = pin;
            SecretKeySpec signingKey = new SecretKeySpec(keyBytes, HMAC_SHA1_ALGORITHM);

            // Get an hmac_sha1 Mac instance and initialize with the signing key
            Mac mac = Mac.getInstance(HMAC_SHA1_ALGORITHM);
            mac.init(signingKey);

            // Compute the hmac on input data bytes
            byte[] rawHmac = mac.doFinal(data);

            // Convert raw bytes to Hex string
            result = bytesToHexString(rawHmac);

            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail MAC is : " + result);
            }

        } catch (InvalidKeyException e) {
            e.printStackTrace();
            return null;
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
        return result;
    }

    /*
     * Convert byte[] to hex string
     * @param src byte[] data
     * @return hex string
     */
    public static String bytesToHexString(byte[] src) {
        StringBuilder stringBuilder = new StringBuilder();
        if (src == null || src.length <= 0) {
            return null;
        }
        for (int i = 0; i < src.length; i++) {
            int v = src[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }
        return stringBuilder.toString();
    }

    /*
     Network Pin is the IMSI.  The IMSI is required to be in the semi-octet format defined in GSM 11.11
     (ESTI: ETS 300 977) (See 10.3.2 page 54-55 of Version 5.10.1).
     The IMSI description:

      BYTE      Description          B8  B7 B6 B5    B4  B3 B2 B1
       1        IMSI length
       2        Digit 1 | Parity     M1  .. .. L1    P   0  0  1
       3        Digit 3 | Digit 2    M3  .. .. L3    M2  .. .. L2
       4        Digit 5 | Digit 4    M5  .. .. L5    M4  .. .. L4
       5        Digit 7 | Digit 6    M7  .. .. L7    M6  .. .. L6
       6        Digit 9 | Digit 8    M9  .. .. L9    M8  .. .. L8
       7        Digit 11 | Digit 10  M11 .. .. L11   M10 .. .. L10
       8        Digit 13 | Digit 12  M11 .. .. L11   M12 .. .. L12
       9        Digit 15 | Digit 14  M11 .. .. L11   M14 .. .. L14

       Key:  P   - parity bit
       Mx  - MSB of digit x
       Lx  - LSB of digit x

       The IMSI length byte MUST NOT be used in the key.

       Any unused bytes (i.e. IMSI length less than 15) MUST NOT be
       used in the key.

       If the IMSI length is an even number, the key MUST USE the
       filler 0xF in the spare nibble.

       Example 1: IMSI Length is 15

       IMSI = 262022033864727

       GSM 11.11 Format:

       BYTE    Description               Value  Note
       1      IMSI length               0x08   NOT USED IN THE KEY
       2      Digit 1 | Parity          0x29
       3      Digit 3 | Digit 2         0x26
       4      Digit 5 | Digit 4         0x20
       5      Digit 7 | Digit 6         0x02
       6      Digit 9 | Digit 8         0x33
       7      Digit 11 | Digit 10       0x68
       8      Digit 13 | Digit 12       0x74
       9      Digit 15 | Digit 14       0x72

       key should point to an array  {0x29, 0x26, 0x20, 0x02, 0x33, 0x68, 0x74, 0x72}
       and keyLength should be 8

       Example 2: IMSI Length is 2

       IMSI = 26

       GSM 11.11 Format:

       BYTE    Description               Value  Note
       1      IMSI length               0x02   NOT USED IN THE KEY
       2      Digit 1 | Parity          0x29
       3      Digit 3 | Digit 2         0xF6
       4      Digit 5 | Digit 4         0xFF   UNUSED DATA
       5      Digit 7 | Digit 6         0xFF   UNUSED DATA
       6      Digit 9 | Digit 8         0xFF   UNUSED DATA
       7      Digit 11 | Digit 10       0xFF   UNUSED DATA
       8      Digit 13 | Digit 12       0xFF   UNUSED DATA
       9      Digit 15 | Digit 14       0xFF   UNUSED DATA

       key should point to an array {0x21, 0xF6} and keyLength should be 2.

     */

    public static byte[] imsiToKey(String imsi) {
        if (imsi == null) {
            return null;
        }
        int len = imsi.length();
        int lenKey = len / 2 + 1;
        boolean even = len % 2 != 0;

        byte[] key = new byte[lenKey];
        for (int i = 0; i < lenKey; i++) {
            if (i == 0) {
                key[0] = (byte) (0x00 + (imsi.charAt(0) - '0') * 16 + 9);
            } else if (i == (lenKey - 1) && !even) {
                key[i] = (byte) (0x00 + 0xF0 + (imsi.charAt(len - 1) - '0'));
            } else {
                key[i] = (byte) (0x00 + (imsi.charAt(i * 2) - '0') * 16 + (imsi.charAt(i * 2 - 1) - '0'));
            }
        }

        MTKlog.i(TAG, "imsiToKey is : " + key.toString());
        return key;
    }

    public void onClick(View view) {
        // TODO Auto-generated method stub
        if (view.getId() == mFullInstallBtn.getId()) {
            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail fullInstallBtn click.");
            }

            sIsFullInstallation = true;
            handleInstall();

        } else if (view.getId() == mCustomInstallBtn.getId()) {
            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail customInstallBtn click.");
            }

            showCustomDialog();
        }
    }

    private void showCustomDialog() {
        int size = mApSettingsListName.size();
        CharSequence[] items = new CharSequence[size];
        boolean[] defaultValues = new boolean[size];
        for (int i = 0; i < size; i++) {
            items[i] = mApSettingsListName.get(i);
            defaultValues[i] = false;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMultiChoiceItems(items, defaultValues,
                new DialogInterface.OnMultiChoiceClickListener() {
                    public void onClick(DialogInterface dialog, int which, boolean isChecked) {
                        // TODO Auto-generated method stub
                        if (isChecked) {
                            mApSettingsListNameChecked.set(which, true);
                        } else {
                            mApSettingsListNameChecked.set(which, false);
                        }

                        if (DEBUG) {
                            MTKlog.i(TAG, "OmacpMessageSettingsDetail mApSettingsListNameChecked is : "
                                            + mApSettingsListNameChecked);
                        }

                        mCustomDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
                        for (int i = 0; i < mApSettingsListNameChecked.size(); i++) {
                            if (mApSettingsListNameChecked.get(i)) {
                                mCustomDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(
                                        true);
                                break;
                            }
                        }

                    }
                });

        builder.setTitle(R.string.custom_install_text);
        builder.setCancelable(true);
        builder.setPositiveButton(R.string.install_text, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                // TODO Auto-generated method stub
                sIsFullInstallation = false;
                handleInstall();
            }
        });
        builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
            public void onCancel(DialogInterface arg0) {
                // TODO Auto-generated method stub
                for (int i = 0; i < mApSettingsListNameChecked.size(); i++) {
                    mApSettingsListNameChecked.set(i, false);
                }
            }
        });

        mCustomDialog = builder.create();
        mCustomDialog.getListView().clearChoices();
        for (int i = 0; i < size; i++) {
            mApSettingsListNameChecked.set(i, false);
        }
        mCustomDialog.show();
        mCustomDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(false);
    }

    private void initActivityState(Intent intent) {
        // If we have been passed a message_id, use that to find our message.
        Uri intentData = null;
        long messageId = intent.getLongExtra("message_id", 0);
        if (messageId > 0) {
            intentData = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, messageId);
        } else {
            intentData = intent.getData();
        }

        if (DEBUG) {
            MTKlog.i(TAG, "OmacpMessageSettingsDetail initActivityState intentData is : "
                    + intentData);
        }

        if (intentData != null) {
            Cursor c = getContentResolver().query(intentData, INSTALLATION_PROJECTION, null, null,
                    null);
            if (c != null && c.moveToFirst()) {
                try {
                    sMessageId = c.getLong(ID);
                    sSimId = c.getInt(SIM_ID);
                    sInstalled = (c.getInt(INSTALLED) != 0);
                    sPinUnlock = (c.getInt(PIN_LOCK) != 0);
                    sSec = c.getInt(SEC);
                    sMac = c.getString(MAC);
                    sBody = c.getBlob(BODY);
                    sContextIdentifier = c.getString(CONTEXT);
                    sMimeType = c.getString(MIME_TYPE);
                } finally {
                    c.close();
                }
            }

            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail initActivityState class variable is : \n"
                        + "mMessageId is : " + sMessageId + "\n"
                        + "mSimId is : " + sSimId + "\n"
                        + "mInstalled is : " + sInstalled + "\n"
                        + "mPinUnlock is : " + sPinUnlock + "\n"
                        + "mSec is : " + sSec + "\n"
                        + "mMac is : " + sMac + "\n"
                        + "mContextIdentifier is : " + sContextIdentifier + "\n"
                        + "mMimeType is : " + sMimeType + "\n");
            }

            /*
             * Parse the omacp Message
             */
            OmacpParser parser = new OmacpParser();
            if (sMimeType.equalsIgnoreCase("text/vnd.wap.connectivity-xml")) {
                parser.setParser(OmacpParser.getTextParser());
            } else if (sMimeType.equalsIgnoreCase("application/vnd.wap.connectivity-wbxml")) {
                parser.setParser(OmacpParser.getWbxmlParser());
            }
            parser.parse(sBody);
            mApList = parser.getApSectionList();
            mNapList = parser.getNapList();
            mPxList = parser.getPxList();

            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail mApList is : " + mApList);
                MTKlog.i(TAG, "OmacpMessageSettingsDetail mNapList is : " + mNapList);
                MTKlog.i(TAG, "OmacpMessageSettingsDetail mPxList is : " + mPxList);
            }

            mApSettingsListName = OmacpMessageUtils.getValidApplicationNameSet(this, mApList,
                    mNapList);
            for (int i = 0; i < mApSettingsListName.size(); i++) {
                mApSettingsListNameChecked.add(false);
            }

            markAsRead();

            SpannableStringBuilder settingsDetailInfo = OmacpMessageUtils.getSettingsDetailInfo(
                    this, mApList, mNapList, mPxList);
            if (settingsDetailInfo == null || settingsDetailInfo.length() == 0) {
                showInvalidSettingDialog();
            } else {
                mDetailText.setText(settingsDetailInfo);
                mDetailText.setTextSize(17.0f);
            }
        }
    }

    private void showInvalidSettingDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.error)
                .setMessage(this.getString(R.string.detail_invalid_setting_error_msg))
                .setCancelable(true)
                .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        // TODO Auto-generated method stub
                        OmacpMessageSettingsDetail.this.finish();
                    }
                }).setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface arg0) {
                        // TODO Auto-generated method stub
                        OmacpMessageSettingsDetail.this.finish();
                    }
                }).show();
    }

    private void markAsRead() {
        new Thread(new Runnable() {
            public void run() {
                synchronized (mMarkAsBlockedSyncer) {
                    if (mMarkAsReadBlocked) {
                        try {
                            mMarkAsBlockedSyncer.wait();
                        } catch (InterruptedException e) {
                            MTKlog.e(TAG, "OmacpMessageSettingDetail InterruptedException.");
                        }
                    }

                    Uri messageUri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI,
                            sMessageId);
                    ContentValues mReadContentValues = new ContentValues(2);
                    mReadContentValues.put("read", 1);
                    mReadContentValues.put("seen", 1);
                    OmacpMessageSettingsDetail.this.getContentResolver().update(messageUri,
                            mReadContentValues, null, null);
                }
                OmacpMessageNotification.updateAllNotifications(OmacpMessageSettingsDetail.this);
            }
        }).start();
    }

    private void handleInstall() {

        // If already installed, then notify the overwrite, else install
        // directly
        if (sInstalled && !mReInstall) {
            showDialog(DIALOG_RE_INSTALL_NOTIFY);
            return;
        }

        // If pin is not unlocked, then notify the user to unlock the pin, else
        // install directly
        if (!sPinUnlock) {
            showDialog(DIALOG_UNLOCK_PIN);
            return;
        }

        // show progress dlg
        showDialog(DIALOG_INSTALLING);

        // Send the intent to apn first, when the apn's result is back, then
        // sendIntentsToApplications
        sendIntentsToApn();

        // If there is no apn setting, then send the intent to the corresponding
        // applications directly
        if (mApnResultObj == null) {
            sendIntentsToApplications();
        }
    }

    private void sendIntentsToApn() {
        mApnResultObj = null;
        ArrayList<Intent> apnIntentList = new ArrayList<Intent>();
        if (mNapList != null && !mNapList.isEmpty()) {
            int index = mApSettingsListName.indexOf(OmacpMessageUtils.getAppName(this,
                    OmacpMessageUtils.APN_APPID));
            if (!(!sIsFullInstallation && !mApSettingsListNameChecked.get(index))) {
                int napSize = mNapList.size();
                //Append the mMessageId which can distinguish apn
                int messageIdAppend = 1;
                for (int i = 0; i < napSize; i++) {
                    NapdefClass napDef = mNapList.get(i);
                    // BEARER parameter is optional
                    // if(nap.BEARER.size() > 0 &&
                    // nap.BEARER.get(0).equalsIgnoreCase("GSM-GPRS")){
                    Intent intent = new Intent();
                    intent.setAction(APP_SETTING_ACTION);
                    String mimeType = "application/com.mediatek.omacp-apn";
                    intent.setType(mimeType);
                    intent.putExtra(APP_ID_KEY, OmacpMessageUtils.APN_APPID);
                    intent.putExtra("context", sContextIdentifier);
                    intent.putExtra("simId", sSimId);
                    //check px's napId and put proxy params to intent
                    checkPxNapId(napDef, intent);
                    // apn id
                    intent.putExtra("APN-ID", String.valueOf(sMessageId + "_" + messageIdAppend));
                    messageIdAppend++;
                    apnIntentList.add(intent);
                }
            }
            // }
        }
        modifyAppParam(apnIntentList);

        if (!apnIntentList.isEmpty()) {
            Intent intent = new Intent();
            intent.setAction(APP_SETTING_ACTION);
            String mimeType = "application/com.mediatek.omacp-apn";
            intent.setType(mimeType);
            intent.putExtra("apn_setting_intent", apnIntentList);
            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail sendBroadcast intent is : " + intent);
            }
            this.sendBroadcast(intent);
            mApnResultObj = new ResultType(OmacpMessageUtils.APN_APPID,
                    RESULT_CONSTANT_NOT_RETURNED);
            mTimerHandler.sendEmptyMessageDelayed(EVENT_APN_INSTALL_TIME_OUT,
                    INSTALL_TIME_OUT_LENGTH);
        }
    }

    // Modification for application's TO-NAPID and TO-PROXY and match application
    private void modifyAppParam(ArrayList<Intent> apnIntentList) {
        for (int index = 0; index < apnIntentList.size(); index++) {
            Intent intent = apnIntentList.get(index);
            String napid = intent.getStringExtra("NAPID-ID");
            String proxy = intent.getStringExtra("PROXY-ID");
            boolean needModify = false;
            for (int apIndex = 0; apIndex < mApList.size(); apIndex++) {
                ApplicationClass application = mApList.get(apIndex);
                if (application.mToProxy != null) {
                    for (int proxyIndex = 0; proxyIndex < application.mToProxy.size(); proxyIndex++) {
                        if (application.mToProxy.get(proxyIndex).equalsIgnoreCase(proxy)) {
                            MTKlog.i(TAG, "application's TO-PROXY match. application index is : "
                                    + apIndex);
                            needModify = true;
                            break;
                        }
                    }
                }
                // This application is consistent with, and check if need add
                // more parameters
                if (needModify) {
                    if (application.mAppid != null
                            && (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_APPID) || (application.mAppid
                                    .equalsIgnoreCase(OmacpMessageUtils.MMS_2_APPID)))) {
                        intent.putExtra("APN-TYPE", "mms");
                        intent.removeExtra("PXADDR");
                        intent.removeExtra("PORT");

                    } else if (application.mAppid != null) {
                        intent.removeExtra("MMS-PROXY");
                        intent.removeExtra("MMS-PORT");
                        intent.removeExtra("MMSC");
                        if (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.BROWSER_APPID)) {
                            intent.putExtra("APN-TYPE", "default");
                        } else if (application.mAppid
                                .equalsIgnoreCase(OmacpMessageUtils.SUPL_APPID)) {
                            intent.putExtra("APN-TYPE", "supl");
                        }
                    }
                    break;
                }
            }
        }
    }

    private void checkPxNapId(NapdefClass napDef, Intent intent) {
        ProxyClass proxyClass = null;
        if (mPxList == null) {
            MTKlog.i(TAG, "OmacpMessageSettingsDetail mPxList is null ");
            return;
        }
        boolean flag = false;
        int pxListSize = mPxList.size();
        for (int n = 0; n < pxListSize; n++) {
            proxyClass = mPxList.get(n);
            if (proxyClass.mPxphysical != null && !proxyClass.mPxphysical.isEmpty()) {
                ArrayList<String> toNapIdList = proxyClass.mPxphysical.get(0).mToNapid;
                if (toNapIdList == null) {
                    continue;
                }
                flag = checkFlagByNapId(toNapIdList, napDef);
                if (flag) {
                    break;
                }
            }
        }
        if (DEBUG) {
            MTKlog.i(TAG,
                    "OmacpMessageSettingsDetail NAPID whether is in proxy, the flag is : "
                            + flag);
        }
        // add px parameters
        if (flag && proxyClass != null) {
            setPxParams(intent, proxyClass);
        }
        // set napdef parameters
        setNapParams(intent, napDef);
        // mtk apn type parameter
        String apnType = OmacpMessageUtils.getAPNType(mApList);
        intent.putExtra("APN-TYPE", apnType);
        // mtk mms parameters
        if (mApList != null) {
            addMmsParams(intent, proxyClass);
        }
    }

    private boolean checkFlagByNapId(ArrayList<String> toNapIdList, NapdefClass napDef) {
        boolean flag = false;
        for (int m = 0; m < toNapIdList.size(); m++) {
            String toNapId = toNapIdList.get(m);
            if (napDef.mNapid.equalsIgnoreCase(toNapId)) {
                flag = true;
                break;
            }
        }
        return flag;
    }

    private void setPxParams(Intent intent, ProxyClass proxyClass) {
        // add application parameters
        intent.putExtra("PROXY-ID", proxyClass.mProxyId);
        intent.putExtra("PROXY-PW", proxyClass.mProxyPw);
        intent.putExtra("PPGAUTH-TYPE", proxyClass.mPpgauthType);
        intent.putExtra("PROXY-PROVIDER-ID", proxyClass.mProxyProviderId);
        intent.putExtra("NAME", proxyClass.mName);
        // it.putExtra("DOMAIN", px.DOMAIN);
        intent.putExtra("TRUST", proxyClass.mTrust);
        intent.putExtra("MASTER", proxyClass.mMaster);
        intent.putExtra("STARTPAGE", proxyClass.mStartpage);
        intent.putExtra("BASAUTH-ID", proxyClass.mBasauthId);
        intent.putExtra("BASAUTH-PW", proxyClass.mBasauthPw);
        // it.putExtra("WSP-VERSION", px.WSP_VERSION);
        // it.putExtra("PUSHENABLED", px.PUSHENABLED);
        // it.putExtra("PULLENBALED", px.PULLENBALED);
        // pxauthinfo
        int pxAuthInfoSize = proxyClass.mPxauthinfo.size();
        ArrayList<HashMap<String, String>> pxAuthInfoMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<PxAuthInfo> pxAuthInfoList = proxyClass.mPxauthinfo;
        for (int j = 0; j < pxAuthInfoSize; j++) {
            HashMap<String, String> map = new HashMap<String, String>();
            map.put("PXAUTH-TYPE", pxAuthInfoList.get(j).mPxauthType);
            map.put("PXAUTH-ID", pxAuthInfoList.get(j).mPxauthId);
            map.put("PXAUTH-PW", pxAuthInfoList.get(j).mPxauthPw);

            pxAuthInfoMapList.add(map);
        }
        intent.putExtra("PXAUTHINFO", pxAuthInfoMapList);
        // use only the first physical proxy, ignore others
        PxPhysical pxPhysical = proxyClass.mPxphysical.get(0);
        intent.putExtra("PHYSICAL-PROXY-ID", pxPhysical.mPhysicalProxyId);
        intent.putExtra("DOMAIN", pxPhysical.mDomain);
        intent.putExtra("PXADDR", pxPhysical.mPxaddr);
        intent.putExtra("PXADDRTYPE", pxPhysical.mPxaddrtype);
        intent.putExtra("PXADDR-FQDN", pxPhysical.mPxaddrFqdn);
        intent.putExtra("WSP-VERSION", pxPhysical.mWspVersion);
        intent.putExtra("PUSHENABLED", pxPhysical.mPushenabled);
        intent.putExtra("PULLENABLED", pxPhysical.mPullenabled);
        // port
        if (proxyClass.mPxphysical != null && !proxyClass.mPxphysical.isEmpty()) {
            int portSize = proxyClass.mPxphysical.get(0).mPort.size();
            ArrayList<HashMap<String, String>> portMapList = new ArrayList<HashMap<String, String>>();
            ArrayList<Port> portList = proxyClass.mPxphysical.get(0).mPort;
            for (int j = 0; j < portSize; j++) {
                HashMap<String, String> map = new HashMap<String, String>();
                map.put("PORTNBR", portList.get(j).mPortnbr);
                if (!portList.get(j).mService.isEmpty()) {
                    // using the first one, ignore others
                    map.put("SERVICE", portList.get(j).mService.get(0));
                }
                portMapList.add(map);
            }
            intent.putExtra("PORT", portMapList);
        }
    }

    private void setNapParams(Intent intent, NapdefClass napDef) {
        intent.putExtra("NAPID", napDef.mNapid);
        // Judge whether the BEARER size is 0.
        if (!napDef.mBearer.isEmpty() && napDef.mBearer.get(0) != null) {
            intent.putExtra("BEARER", napDef.mBearer.get(0));
        }
        intent.putExtra("NAP-NAME", napDef.mName);
        intent.putExtra("INTERNET", napDef.mInternet);
        intent.putExtra("NAP-ADDRESS", napDef.mNapaddress);
        intent.putExtra("NAP-ADDRTYPE", napDef.mNapaddrtype);
        intent.putExtra("DNS-ADDR", napDef.mDnsaddr);
        intent.putExtra("CALLTYPE", napDef.mCalltype);
        intent.putExtra("LOCAL-ADDR", napDef.mLocaladdr);
        intent.putExtra("LOCAL-ADDRTYPE", napDef.mLocaladdrtype);
        intent.putExtra("LINKSPEED", napDef.mLinkspeed);
        intent.putExtra("DNLINKSPEED", napDef.mDnlinkspeed);
        intent.putExtra("LINGER", napDef.mLinger);
        intent.putExtra("DELIVERY-ERR-SDU", napDef.mDeliveryerrsdu);
        intent.putExtra("DELIVERY-ORDER", napDef.mDeliveryorder);
        intent.putExtra("TRAFFIC-CLASS", napDef.mTrafficclass);
        intent.putExtra("MAX-SDU-SIZE", napDef.mMaxsdusize);
        intent.putExtra("MAX-BITRATE-UPLINK", napDef.mMaxbitrateuplink);
        intent.putExtra("MAX-BITRATE-DNLINK", napDef.mMaxbitratednlink);
        intent.putExtra("RESIDUAL-BER", napDef.mResidualber);
        intent.putExtra("SDU-ERROR-RATIO", napDef.mSduerrorratio);
        intent.putExtra("TRAFFIC-HANDL-PROI", napDef.mTraffichandlproi);
        intent.putExtra("TRANSFER-DELAY", napDef.mTransferdelay);
        intent.putExtra("GUARANTEED-BITRATE-UPLINK", napDef.mGuaranteedbitrateuplink);
        intent.putExtra("GUARANTEED-BITRATE-DNLINK", napDef.mGuaranteedbitratednlink);
        intent.putExtra("MAX-NUM-RETRY", napDef.mMaxnumretry);
        intent.putExtra("FIRST-RETRY-TIMEOUT", napDef.mFirstretrytimeout);
        intent.putExtra("REREG-THRESHOLD", napDef.mReregthreshold);
        intent.putExtra("T-BIT", napDef.mTbit);
        // napauthinfo
        int napAuthInfoSize = napDef.mNapauthinfo.size();
        ArrayList<HashMap<String, String>> napAuthInfoMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<NapAuthInfo> napAuthInfoList = napDef.mNapauthinfo;
        for (int j = 0; j < napAuthInfoSize; j++) {
            HashMap<String, String> map = new HashMap<String, String>();
            map.put("AUTHTYPE", napAuthInfoList.get(j).mAuthtype);
            map.put("AUTHNAME", napAuthInfoList.get(j).mAuthname);
            map.put("AUTHSECRET", napAuthInfoList.get(j).mAuthsecret);
            if (!napAuthInfoList.get(j).mAuthentity.isEmpty()) {
                map.put("AUTH_ENTITY", napAuthInfoList.get(j).mAuthentity.get(0));
            }
            map.put("SPI", napAuthInfoList.get(j).mSpi);

            napAuthInfoMapList.add(map);
        }
        intent.putExtra("NAPAUTHINFO", napAuthInfoMapList);
        // validity
        int validitySize = napDef.mValidity.size();
        ArrayList<HashMap<String, String>> validityMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<Validity> validityList = napDef.mValidity;
        for (int j = 0; j < validitySize; j++) {
            HashMap<String, String> map = new HashMap<String, String>();
            map.put("COUNTRY", validityList.get(j).mCountry);
            map.put("NETWORK", validityList.get(j).mNetwork);
            map.put("SID", validityList.get(j).mSid);
            map.put("SOC", validityList.get(j).mSoc);
            map.put("VALIDUNTIL", validityList.get(j).mValiduntil);
            validityMapList.add(map);
        }
        intent.putExtra("VALIDITY", validityMapList);
    }

    private void addMmsParams(Intent intent, ProxyClass proxyClass) {
        for (int d = 0; d < mApList.size(); d++) {
            if (mApList.get(d).mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_APPID)
                    || mApList.get(d).mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_2_APPID)) {
                ArrayList<String> addr = mApList.get(d).mAddr;
                if (addr != null && !addr.isEmpty()) {
                    intent.putExtra("MMSC", addr.get(0));
                    MTKlog.i(TAG, "apn MMSC is : " + addr.get(0));
                } else if (!mApList.get(d).mAppaddr.isEmpty()
                        && mApList.get(d).mAppaddr.get(0).mAddr != null) {
                    intent.putExtra("MMSC", mApList.get(d).mAppaddr.get(0).mAddr);
                    MTKlog.i(TAG, "apn MMSC is : " + mApList.get(d).mAppaddr.get(0).mAddr);
                }
                if (proxyClass != null && proxyClass.mPxphysical != null
                        && !proxyClass.mPxphysical.isEmpty()) {
                    intent.putExtra("MMS-PROXY", proxyClass.mPxphysical.get(0).mPxaddr);
                    MTKlog.i(TAG, "apn MMS PROXY is : " + proxyClass.mPxphysical.get(0).mPxaddr);
                    if (!proxyClass.mPxphysical.get(0).mPort.isEmpty()) {
                        intent.putExtra("MMS-PORT",
                                proxyClass.mPxphysical.get(0).mPort.get(0).mPortnbr);
                        MTKlog.d(TAG, "apn MMS PORT is : "
                                        + proxyClass.mPxphysical.get(0).mPort.get(0).mPortnbr);
                    }
                }
            }
        }
    }

    // Send intents to applications except apn
    private void sendIntentsToApplications() {
        // reset, mApplicationResultList can not be null
        mApplicationResultList.clear();
        // In case mApList be null
        if (mApList != null) {
            ArrayList<Intent> emailIntentList = new ArrayList<Intent>();
            int size = mApList.size();
            boolean isContinue = false;
            for (int i = 0; i < size; i++) {
                ApplicationClass application = mApList.get(i);
                addValidApplication(application, emailIntentList);
            }
            if (!emailIntentList.isEmpty()) {
                ResultType result = new ResultType(OmacpMessageUtils.SMTP_APPID,
                        RESULT_CONSTANT_NOT_RETURNED);
                sendEmailIntent(emailIntentList, result);
                mApplicationResultList.add(result);
            }
        }
        // handle not returned result
        handleApplicationResultList();
    }

    private boolean addValidApplication(ApplicationClass application,
            ArrayList<Intent> emailIntentList) {
        if (null == OmacpMessageUtils.getAppName(this, application.mAppid)) {
            MTKlog.e(TAG,
                    "OmacpMessageSettingsDetail sendIntentsToApplications invalid application settings.");
            return true;
        }
        boolean isInclueded = false;
        for (int b = 0; b < mApplicationResultList.size(); b++) {
            if (OmacpMessageUtils.getAppName(this, mApplicationResultList.get(b).mAppId)
                    .equalsIgnoreCase(OmacpMessageUtils.getAppName(this, application.mAppid))) {
                isInclueded = true;
                break;
            }
        }
        // if w4 mms setting only has mmsc, then ignore it, because it
        // has been moved to apn
        // if ap0005 mms setting only has mmsc, then ignore it,
        // because it has been moved to apn
        if (isInclueded
                || (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_APPID) && application.mCm == null)
                || (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.MMS_2_APPID)
                        && application.mCm == null && application.mRm == null
                        && application.mMs == null && application.mPcAddr == null && application.mMa == null)) {
            return true;
        }
        int index = mApSettingsListName.indexOf(OmacpMessageUtils.getAppName(this,
                application.mAppid));
        if (!sIsFullInstallation && !mApSettingsListNameChecked.get(index)) {
            return true;
        }
        Intent intent = new Intent();
        intent.setAction(APP_SETTING_ACTION);
        String mimeType = "application/com.mediatek.omacp-" + application.mAppid;
        intent.setType(mimeType);
        intent.putExtra(APP_ID_KEY, application.mAppid);
        intent.putExtra("context", sContextIdentifier); // currently not
                                                        // used
        intent.putExtra("simId", sSimId);
        // add application parameters
        addApplicationParams(intent, application);
        // add Email result
        addEmailToResultList(intent, application, emailIntentList);
        return false;
    }

    private void addApplicationParams(Intent intent, ApplicationClass application) {
        intent.putExtra("APPID", application.mAppid);
        intent.putExtra("PROVIDER-ID", application.mProviderId);
        intent.putExtra("NAME", application.mName);
        intent.putExtra("AACCEPT", application.mAaccept);
        intent.putExtra("APROTOCOL", application.mAprotocol);
        intent.putExtra("TO-PROXY", application.mToProxy);
        intent.putExtra("TO-NAPID", application.mToNapid);
        intent.putExtra("ADDR", application.mAddr);
        // add application specific parameters
        intent.putExtra("CM", application.mCm);
        intent.putExtra("RM", application.mRm);
        intent.putExtra("MS", application.mMs);
        intent.putExtra("PC-ADDR", application.mPcAddr);
        intent.putExtra("Ma", application.mMa);
        intent.putExtra("INIT", application.mInit);
        intent.putExtra("FROM", application.mFrom);
        intent.putExtra("RT-ADDR", application.mRtAddr);
        intent.putExtra("MAX-BANDWIDTH", application.mMaxBandwidth);
        intent.putExtra("NETINFO", application.mNetinfo);
        intent.putExtra("MIN-UDP-PORT", application.mMinUdpPort);
        intent.putExtra("MAX-UDP-PORT", application.mMaxUdpPort);
        intent.putExtra("SERVICES", application.mServices);
        intent.putExtra("CIDPREFIX", application.mCidprefix);

        // app addr
        int appAddrSize = application.mAppaddr.size();
        ArrayList<HashMap<String, String>> appAddrMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<AppAddr> appAddrList = application.mAppaddr;
        for (int j = 0; j < appAddrSize; j++) {
            if (!appAddrList.get(j).mPort.isEmpty()) {
                for (int n = 0; n < appAddrList.get(j).mPort.size(); n++) {
                    HashMap<String, String> map = new HashMap<String, String>();
                    map.put("ADDR", appAddrList.get(j).mAddr);
                    map.put("ADDRTYPE", appAddrList.get(j).mAddrtype);
                    map.put("PORTNBR", appAddrList.get(j).mPort.get(n).mPortnbr);
                    if (!appAddrList.get(j).mPort.get(n).mService.isEmpty()) {
                        map.put("SERVICE", appAddrList.get(j).mPort.get(n).mService.get(0));
                        // using the first one, ignore others
                    }
                    appAddrMapList.add(map);
                }
            } else {
                HashMap<String, String> map = new HashMap<String, String>();
                map.put("ADDR", appAddrList.get(j).mAddr);
                map.put("ADDRTYPE", appAddrList.get(j).mAddrtype);
                appAddrMapList.add(map);
            }

        }
        intent.putExtra("APPADDR", appAddrMapList);

        // app auth
        int appAuthSize = application.mAppauth.size();
        ArrayList<HashMap<String, String>> appAuthMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<AppAuth> appAuthList = application.mAppauth;
        for (int j = 0; j < appAuthSize; j++) {
            HashMap<String, String> map = new HashMap<String, String>();
            map.put("AAUTHLEVEL", appAuthList.get(j).mAauthlevel);
            map.put("AAUTHTYPE", appAuthList.get(j).mAauthtype);
            map.put("AAUTHNAME", appAuthList.get(j).mAauthname);
            map.put("AAUTHSECRET", appAuthList.get(j).mAauthsecret);
            map.put("AAUTHDATA", appAuthList.get(j).mAauthdata);

            appAuthMapList.add(map);
        }
        intent.putExtra("APPAUTH", appAuthMapList);

        // resource
        int resourceSize = application.mResource.size();
        ArrayList<HashMap<String, String>> resourceMapList = new ArrayList<HashMap<String, String>>();
        ArrayList<Resource> resourceList = application.mResource;
        for (int j = 0; j < resourceSize; j++) {
            HashMap<String, String> map = new HashMap<String, String>();
            map.put("URI", resourceList.get(j).mUri);
            map.put("NAME", resourceList.get(j).mName);
            map.put("AACCEPT", resourceList.get(j).mAaccept);
            map.put("AAUTHTYPE", resourceList.get(j).mAauthtype);
            map.put("AAUTHNAME", resourceList.get(j).mAauthname);
            map.put("AAUTHSECRET", resourceList.get(j).mAauthsecret);
            map.put("AAUTHDATA", resourceList.get(j).mAauthdata);
            map.put("STARTPAGE", resourceList.get(j).mStartpage);

            resourceMapList.add(map);
        }
        intent.putExtra("RESOURCE", resourceMapList);
    }

    private void addEmailToResultList(Intent intent, ApplicationClass application,
            ArrayList<Intent> emailIntentList) {
        if (application.mAppid.equalsIgnoreCase(OmacpMessageUtils.SMTP_APPID)
                || application.mAppid.equalsIgnoreCase(OmacpMessageUtils.POP3_APPID)
                || application.mAppid.equalsIgnoreCase(OmacpMessageUtils.IMAP4_APPID)) {
            emailIntentList.add(intent);
        } else {
            ResultType result = new ResultType(application.mAppid, 0);
            if (checkIfApplicationSupport(application.mAppid)) {
                if (DEBUG) {
                    MTKlog.i(TAG, "OmacpMessageSettingsDetail sendBroadcast intent is : " + intent);
                }
                this.sendBroadcast(intent);
            } else {
                // If not support, then label it failed directly
                result.mResult = RESULT_CONSTANT_FAILED;
            }
            mApplicationResultList.add(result);
        }
    }

    private static boolean checkIfApplicationSupport(String appId) {

        if (appId.equalsIgnoreCase(OmacpMessageUtils.MMS_APPID)) {
            return OmacpApplicationCapability.sMms;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.BROWSER_APPID)) {
            return OmacpApplicationCapability.sBrowser;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.APN_APPID)) {
            return true;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.IMAP4_APPID)) {
            return OmacpApplicationCapability.sEmail;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.POP3_APPID)) {
            return OmacpApplicationCapability.sEmail;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.SMTP_APPID)) {
            return OmacpApplicationCapability.sEmail;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.DM_APPID)) {
            return OmacpApplicationCapability.sDm;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.SUPL_APPID)) {
            return OmacpApplicationCapability.sSupl;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.RTSP_APPID)) {
            return OmacpApplicationCapability.sRtsp;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.DS_APID)) {
            return OmacpApplicationCapability.sDs;
        } else if (appId.equalsIgnoreCase(OmacpMessageUtils.IMPS_APPID)) {
            return OmacpApplicationCapability.sImps;
        } else {
            MTKlog.e(TAG, "OmacpMessageSettingsDetail getAppName unknown app.");
            return false;
        }
    }

    private void sendEmailIntent(ArrayList<Intent> emailIntentList, ResultType result) {
        Intent intent = new Intent();
        intent.setAction(APP_SETTING_ACTION);
        String mimeType = "application/com.mediatek.omacp-25";
        intent.setType(mimeType);
        intent.putExtra("email_setting_intent", emailIntentList);
        if (checkIfApplicationSupport(OmacpMessageUtils.SMTP_APPID)) {
            if (DEBUG) {
                MTKlog.i(TAG, "OmacpMessageSettingsDetail sendBroadcast intent is : " + intent);
            }

            this.sendBroadcast(intent);
        } else {
            // If not support, then label it failed directly
            result.mResult = RESULT_CONSTANT_FAILED;
        }
    }

    private void handleApplicationResultList() {
        if (!mApplicationResultList.isEmpty()) {
            boolean isNeedSendTimeoutMsg = false;
            for (int d = 0; d < mApplicationResultList.size(); d++) {
                if (mApplicationResultList.get(d).mResult == RESULT_CONSTANT_NOT_RETURNED) {
                    isNeedSendTimeoutMsg = true;
                    break;
                }
            }
            // If all the application are not supported, then there setting
            // result already set and can give the report directley
            if (isNeedSendTimeoutMsg) {
                mTimerHandler.sendEmptyMessageDelayed(EVENT_APPLICATION_INSTALL_TIME_OUT,
                        INSTALL_TIME_OUT_LENGTH);
            } else {
                handleFinishInstall();
            }
        } else if (mApnResultObj != null) {
            // If no application need to set, then give the report directly
            // about apn setting result
            handleFinishInstall();
        }
    }

    private static class ResultType {
        public String mAppId;

        public int mResult;

        public ResultType(String appId, int result) {
            mAppId = appId;
            mResult = result;
        }
    }

    private BroadcastReceiver mResultReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(APP_SETTING_RESULT_ACTION)) {
                String appId = intent.getStringExtra(APP_ID_KEY);
                boolean installSuccess = intent.getBooleanExtra("result", false);

                if (DEBUG) {
                    MTKlog.d(TAG, "OmacpMessageSettingsDetail result received, appId is : "
                            + appId + " " + "result is : " + installSuccess);
                }

                if (appId.equalsIgnoreCase(OmacpMessageUtils.APN_APPID)) {
                    if (mApnResultObj == null) {
                        MTKlog.e(TAG, "OmacpMessageSettingsDetail mResultReceiver mApnResultObj is null.");
                    } else {
                        sendApplicationsByApnResult(installSuccess);
                    }
                    mTimerHandler.removeMessages(EVENT_APN_INSTALL_TIME_OUT);

                } else {
                    int size = mApplicationResultList.size();
                    sendApplicationsByAppResult(size, appId, installSuccess);
                }
            }
        }
    };

    private void sendApplicationsByApnResult(boolean installSuccess) {
        if (installSuccess) {
            mApnResultObj.mResult = RESULT_CONSTANT_SUCCEED;
            // Due to GPRS switch, application install should
            // wait until the GPRS network switch complete
            // The following cases, no need
            // 1: no application to install
            // 2: WiFi
            // 3: APN install fail
            if (mApList != null && !mApList.isEmpty()) {
                ConnectivityManager cm = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
                NetworkInfo info = cm.getActiveNetworkInfo();
                if (info != null && info.getType() == ConnectivityManager.TYPE_MOBILE) {
                    IntentFilter networkStateChangedFilter = new IntentFilter();
                    networkStateChangedFilter
                            .addAction(ConnectivityManager.CONNECTIVITY_ACTION);
                    mStickyIntent = OmacpMessageSettingsDetail.this
                            .registerReceiver(mNetworkStateIntentReceiver,
                                    networkStateChangedFilter);
                    mIsNetworkStateReceiverRegistered = true;
                    mTimerHandler.sendEmptyMessageDelayed(
                            EVENT_APN_SWITCH_TIME_OUT, APN_SWITCH_TIME_OUT_LENGTH);
                    MTKlog.d(TAG,
                            "OmacpMessageSettingsDetail mResultReceiver register apn switch receiver.");
                } else {
                    sendIntentsToApplications();
                }
            } else {
                sendIntentsToApplications();
            }
        } else {
            mApnResultObj.mResult = RESULT_CONSTANT_FAILED;
            sendIntentsToApplications();
        }
    }

    private void sendApplicationsByAppResult(int size, String appId, boolean installSuccess) {
        for (int i = 0; i < size; i++) {
            ResultType obj = mApplicationResultList.get(i);
            if (appId.equals(obj.mAppId)) {
                if (installSuccess) {
                    obj.mResult = RESULT_CONSTANT_SUCCEED;
                } else {
                    obj.mResult = RESULT_CONSTANT_FAILED;
                }
            }
        }

        // check if all the results are returned
        boolean flag = true;
        for (int i = 0; i < mApplicationResultList.size(); i++) {
            if (mApplicationResultList.get(i).mResult == RESULT_CONSTANT_NOT_RETURNED) {
                flag = false;
                break;
            }
        }

        if (flag) {
            mTimerHandler.removeMessages(EVENT_APPLICATION_INSTALL_TIME_OUT);
            handleFinishInstall();
        }
    }

    private Intent mStickyIntent = null;
    private boolean mIsNetworkStateReceiverRegistered = false;

    // network changed receiver to check if installed APN switch finished
    private BroadcastReceiver mNetworkStateIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(ConnectivityManager.CONNECTIVITY_ACTION)) {

                NetworkInfo info = intent
                        .getParcelableExtra(ConnectivityManager.EXTRA_NETWORK_INFO);
                MTKlog.d(TAG, "Network Receiver info.getType():" + info.getType()
                        + "--info.isConnected():" + info.isConnected() + "--info.isAvailable():"
                        + info.isAvailable());
                if (mStickyIntent != null) {
                    mStickyIntent = null;
                    MTKlog.d(TAG, "OmacpMessageSettingsDetail mNetworkStateIntentReceiver it is sticky intent, ignore it.");
                    return;
                }

                if (info.isConnected() && info.getType() == ConnectivityManager.TYPE_MOBILE) {
                    sendIntentsToApplications();
                    OmacpMessageSettingsDetail.this.unregisterReceiver(mNetworkStateIntentReceiver);
                    mIsNetworkStateReceiverRegistered = false;
                    mTimerHandler.removeMessages(EVENT_APN_SWITCH_TIME_OUT);
                }
            }
        }
    };

    private void markMessageAsInstalled() {
        new Runnable() {
            public void run() {
                synchronized (mMarkAsBlockedSyncer) {
                    if (mMarkAsReadBlocked) {
                        try {
                            mMarkAsBlockedSyncer.wait();
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                    }

                    ContentResolver resolver = OmacpMessageSettingsDetail.this.getContentResolver();

                    ContentValues values = new ContentValues(1);
                    values.put("installed", 1);

                    Uri messageUri = ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI,
                            sMessageId);

                    resolver.update(messageUri, values, null, null);
                }
            }
        }.run();
    }

    private void giveInstallationReport() {
        StringBuilder report = new StringBuilder();

        int size = mApplicationResultList.size();
        for (int i = 0; i < size; i++) {
            ResultType obj = mApplicationResultList.get(i);

            if (i > 0) {
                report.append("\n");
            }

            // append app name
            report.append(OmacpMessageUtils.getAppName(this, obj.mAppId));

            // append ": "
            report.append(": ");

            // append installation result
            if (obj.mResult == RESULT_CONSTANT_SUCCEED) {
                report.append(this.getString(R.string.result_success));
            } else if (obj.mResult == RESULT_CONSTANT_FAILED) {
                report.append(this.getString(R.string.result_failed));
            } else {
                report.append(this.getString(R.string.unknown));
            }

        }

        if (mApnResultObj != null) {
            if (report.length() > 0) {
                report.append("\n");
            }

            // append apn name
            report.append(OmacpMessageUtils.getAppName(this, mApnResultObj.mAppId));

            // append ": "
            report.append(": ");

            // append installation result
            if (mApnResultObj.mResult == RESULT_CONSTANT_SUCCEED) {
                report.append(this.getString(R.string.result_success));
            } else if (mApnResultObj.mResult == RESULT_CONSTANT_FAILED) {
                report.append(this.getString(R.string.result_failed));
            } else {
                report.append(this.getString(R.string.unknown));
            }
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.installation_report).setMessage(report).setCancelable(true)
                .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface arg0, int arg1) {
                        // TODO Auto-generated method stub
                        OmacpMessageSettingsDetail.this.finish();
                    }
                }).setOnCancelListener(new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface arg0) {
                        // TODO Auto-generated method stub
                        OmacpMessageSettingsDetail.this.finish();
                    }
                }).show();

    }

    // Case: installing... and need more time, another omacp message comes,
    // click the notification enter OmacpMessageList
    // When the installing time out to show the result, JE will happen because
    // OmacpMessageList is the current activity
    // Solution: remove the messages of this activity when enter onStop function
    @Override
    protected void onStop() {
        super.onStop();
    }

    private Handler mTimerHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_APPLICATION_INSTALL_TIME_OUT:
                    MTKlog.e(TAG, "OmacpMessageSettingsDetail application install time out......");
                    int size = mApplicationResultList.size();
                    for (int i = 0; i < size; i++) {
                        if (mApplicationResultList.get(i).mResult == RESULT_CONSTANT_NOT_RETURNED) {
                            mApplicationResultList.get(i).mResult = RESULT_CONSTANT_FAILED;
                        }
                    }
                    handleFinishInstall();
                    break;
                case EVENT_APN_INSTALL_TIME_OUT:
                    MTKlog.e(TAG, "OmacpMessageSettingsDetail apn install time out......");
                    if (mApnResultObj.mResult == RESULT_CONSTANT_NOT_RETURNED) {
                        mApnResultObj.mResult = RESULT_CONSTANT_FAILED;
                    }
                    sendIntentsToApplications();
                    break;
                case EVENT_APN_SWITCH_TIME_OUT:
                    MTKlog.e(TAG, "OmacpMessageSettingsDetail apn switch time out......");
                    sendIntentsToApplications();
                    break;
                default:
                    MTKlog.e(TAG, "OmacpMessageSettingsDetail no proper event type.");
                    break;
            }
        }
    };

    // when installation finished, handle the remaining things
    private void handleFinishInstall() {
        removeDialog(DIALOG_INSTALLING);
        markMessageAsInstalled();
        mReInstall = false;
        sInstalled = true;
        giveInstallationReport();
    }

    public static Intent createIntent(Context context, long messageId) {
        Intent intent = new Intent(context, OmacpMessageSettingsDetail.class);
        if (messageId > 0) {
            intent.setData(ContentUris.withAppendedId(OmacpProviderDatabase.CONTENT_URI, messageId));
        }
        return intent;
    }

}
