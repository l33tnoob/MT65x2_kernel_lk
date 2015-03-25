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

package com.mediatek.apst.target.receiver;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Bundle;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;

import com.mediatek.apst.target.event.Event;
import com.mediatek.apst.target.event.EventDispatcher;
import com.mediatek.apst.target.event.IBackupAndRestoreListener;
import com.mediatek.apst.target.event.IBatteryListener;
import com.mediatek.apst.target.event.IPackageListener;
import com.mediatek.apst.target.event.ISdStateListener;
import com.mediatek.apst.target.event.ISmsListener;
import com.mediatek.apst.target.service.NotifyService;
import com.mediatek.apst.target.service.SmsSender;
import com.mediatek.apst.target.util.Debugger;

public class InternalReceiver extends BroadcastReceiver {

    public static final String ACTION_SMS_RECEIVED = "android.provider.Telephony.SMS_RECEIVED";

    public static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";

    public static final String ACTION_USB_CONNECT = "android.intent.action.UMS_CONNECTED";
    
//    com.mediatek.email.END_EMAIL_BACKUP -e path pathInTarget -e fileName nameOfBackupFile -ez isSuccessful isBackupSuccessful
    public static final String ACTION_END_EMAIL_BACKUP = "com.mediatek.email.END_EMAIL_BACKUP";
    public static final String ACTION_END_EMAIL_RESTORE = "com.mediatek.email.END_EMAIL_RESTORE";

    public static final String SIM_ID = "simid";
    // private IEventListener mActionHandler;

    private Context mContext;

    private boolean mRegistered;

    private int mBatteryLevel;
    private int mBatteryScale;

    /**
     * @param context The context to save.
     */
    public InternalReceiver(Context context) {
        super();
        this.mContext = context;
        this.mRegistered = false;
        this.mBatteryLevel = 0;
    }

    /**
     * @return Whether is registered.
     */
    public boolean isRegistered() {
        return this.mRegistered;
    }

    /**
     * @return The battery level.
     */
    public int getBatteryLevel() {
        return this.mBatteryLevel;
    }

    /**
     * @return The battery scale.
     */
    public int getBatteryScale() {
        return this.mBatteryScale;
    }

    /**
     * @param registered Set status of register.
     */
    public void setRegistered(boolean registered) {
        this.mRegistered = registered;
    }

    /**
     * Register all the receiver.
     */
    public void registerAll() {
        IntentFilter intentFilter = new IntentFilter();
        // intentFilter.addAction(Intent.ACTION_POWER_DISCONNECTED);
        intentFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        intentFilter.addAction(ACTION_SIM_STATE_CHANGED);
        // Added by Shaoying Han 2011-04-08
        intentFilter.addAction(Intent.SIM_SETTINGS_INFO_CHANGED);
        intentFilter.addAction(ACTION_SMS_RECEIVED);
        intentFilter.addAction(SmsSender.ACTION_SMS_SENT);
        intentFilter.addAction(SmsSender.ACTION_SMS_DELIVERED);
        // Backup email
        intentFilter.addAction(ACTION_END_EMAIL_BACKUP);
        intentFilter.addAction(ACTION_END_EMAIL_RESTORE);
        mContext.registerReceiver(this, intentFilter);

        // Media mount/unmount/remove
        intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
        intentFilter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        intentFilter.addDataScheme("file");
        mContext.registerReceiver(this, intentFilter);

        // Package add/remove/data clear
        intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        intentFilter.addAction(Intent.ACTION_PACKAGE_DATA_CLEARED);
        intentFilter.addDataScheme("package");
        mContext.registerReceiver(this, intentFilter);

        // intentFilter = new IntentFilter();
        // intentFilter.addAction(ACTION_USB_CONNECT);
        // intentFilter.addAction(ACTION_USB_DISCONNECT);
        // mContext.registerReceiver(this, intentFilter);

        setRegistered(true);
    }

    /**
     * Unregister all the receiver.
     */
    public void unregisterAll() {
        if (isRegistered()) {
            setRegistered(false);
            mContext.unregisterReceiver(this);
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Debugger.logI(new Object[] { context, intent }, "Intent received.");
        String strAction = intent.getAction();

        if (null == strAction) {
            Debugger.logW(new Object[] { context, intent },
                    "intent.getAction() returns null.");
        } else if (Intent.ACTION_BATTERY_CHANGED.equals(strAction)) {
            // Record battery level
            int batteryLevel = intent
                    .getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
            int batteryScale = intent
                    .getIntExtra(BatteryManager.EXTRA_SCALE, 0);
            if (batteryLevel != mBatteryLevel || batteryScale != mBatteryScale) {
                EventDispatcher.dispatchBatteryStateChangedEvent(new Event()
                        .put(IBatteryListener.LEVEL, batteryLevel).put(
                                IBatteryListener.SCALE, batteryScale));
            }
            mBatteryLevel = batteryLevel;
            mBatteryScale = batteryScale;
        } else if (Intent.ACTION_MEDIA_MOUNTED.equals(strAction)) {
            boolean readOnly = intent.getBooleanExtra("read-only", false);
            EventDispatcher.dispatchSdStateChangedEvent(new Event().put(
                    ISdStateListener.PRESENT, true).put(
                    ISdStateListener.MOUNTED, true).put(
                    ISdStateListener.WRITEABLE, !readOnly));
        } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(strAction)) {
            EventDispatcher.dispatchSdStateChangedEvent(new Event().put(
                    ISdStateListener.PRESENT, true).put(
                    ISdStateListener.MOUNTED, false).put(
                    ISdStateListener.WRITEABLE, false));
        } else if (Intent.ACTION_MEDIA_REMOVED.equals(strAction)) {
            EventDispatcher.dispatchSdStateChangedEvent(new Event().put(
                    ISdStateListener.PRESENT, false).put(
                    ISdStateListener.MOUNTED, false).put(
                    ISdStateListener.WRITEABLE, false));
        } else if (Intent.ACTION_MEDIA_BAD_REMOVAL.equals(strAction)) {
            EventDispatcher.dispatchSdStateChangedEvent(new Event().put(
                    ISdStateListener.PRESENT, false).put(
                    ISdStateListener.MOUNTED, false).put(
                    ISdStateListener.WRITEABLE, false));
        } else if (ACTION_SMS_RECEIVED.equals(strAction)) {
            SmsMessage msg = null;
            Bundle extras = intent.getExtras();
            if (null != extras) {
                Object[] pdusObj = (Object[]) extras.get("pdus");
                if (null != pdusObj && pdusObj.length > 0) {
                    long timestamp = System.currentTimeMillis();
                    String address = null;
                    String body = null;
                    for (int i = 0; i < pdusObj.length; i++) {
                        msg = SmsMessage.createFromPdu((byte[]) pdusObj[i]);
                        // timestamp = msg.getTimestampMillis();
                        if (null == address) {
                            address = msg.getDisplayOriginatingAddress();
                        } else if (!address.equals(msg
                                .getDisplayOriginatingAddress())) {
                            Debugger.logE(new Object[] { context, intent },
                                    "Pdus array contains different addresses!");
                        }
                        if (null == body) {
                            body = msg.getMessageBody();
                        } else {
                            body += msg.getMessageBody();
                        }
                    }
                    EventDispatcher.dispatchSmsReceivedEvent(new Event().put(
                            ISmsListener.AFTER_TIME_OF, timestamp - 300).put(
                            ISmsListener.ADDRESS, address).put(
                            ISmsListener.BODY, body));
                }
            }
        } else if (SmsSender.ACTION_SMS_SENT.equals(strAction)) {
            long id = intent.getLongExtra(SmsSender.EXTRA_ID, -1);
            long date = intent.getLongExtra(SmsSender.EXTRA_DATE, -1);
            Debugger.logD(SmsSender.EXTRA_ID + "=" + id + ", "
                    + SmsSender.EXTRA_DATE + "=" + date);
            boolean result;

            switch (getResultCode()) {
            case Activity.RESULT_OK:
                result = true;
                break;

            case SmsManager.RESULT_ERROR_GENERIC_FAILURE:
                result = false;
                break;

            case SmsManager.RESULT_ERROR_RADIO_OFF:
                result = false;
                break;

            case SmsManager.RESULT_ERROR_NULL_PDU:
                result = false;
                break;

            default:
                result = false;
                break;
            }

            EventDispatcher.dispatchSmsSentEvent(new Event().put(
                    ISmsListener.SMS_ID, id).put(ISmsListener.DATE, date).put(
                    ISmsListener.SENT, result));
        } else if (SmsSender.ACTION_SMS_DELIVERED.equals(strAction)) {
            // Do nothing
            Debugger.logI("equals");
        } else if (Intent.ACTION_PACKAGE_ADDED.equals(strAction)) {
            int uid = intent.getIntExtra(Intent.EXTRA_UID, -1);
            if (uid != -1) {
                EventDispatcher.dispatchPackageAddedEvent(new Event().put(
                        IPackageListener.UID, uid));
            }
        } else if (Intent.ACTION_PACKAGE_REMOVED.equals(strAction)) {
            // Do nothing
            Debugger.logI("equals");
        } else if (Intent.ACTION_PACKAGE_DATA_CLEARED.equals(strAction)) {
            int uid = intent.getIntExtra(Intent.EXTRA_UID, -1);
            if (uid != -1) {
                EventDispatcher.dispatchPackageDataClearedEvent(new Event()
                        .put(IPackageListener.UID, uid));
            }
        } else if (ACTION_SIM_STATE_CHANGED.equals(strAction)
                || Intent.SIM_SETTINGS_INFO_CHANGED.equals(strAction)) { 
            // Modified
            // by
            // Shaoying
            // Han

            Intent intentService = new Intent(context, NotifyService.class);
            intentService.putExtras(intent.getExtras());
            intentService.putExtra("Action", strAction);
            context.startService(intentService);
        } else if (ACTION_END_EMAIL_BACKUP.equals(strAction)) {
            EventDispatcher.dispatchEmailBackupEndEvent(new Event().put(
                    IBackupAndRestoreListener.EMAIL_PATH, intent.getStringExtra("path")).put(
                    IBackupAndRestoreListener.EMAIL_FILE_NAME, intent.getStringExtra("fileName")).put(
                    IBackupAndRestoreListener.EMAIL_IS_SUCCESSFUL, intent.getBooleanExtra("isSuccessful", false)));
        } else if (ACTION_END_EMAIL_RESTORE.equals(strAction)) {
            EventDispatcher.dispatchEmailRestoreEndEvent(new Event().put(
                    IBackupAndRestoreListener.EMAIL_PATH, intent.getStringExtra(IBackupAndRestoreListener.EMAIL_PATH)).put(
                    IBackupAndRestoreListener.EMAIL_FILE_NAME, intent.getStringExtra(IBackupAndRestoreListener.EMAIL_FILE_NAME)).put(
                    IBackupAndRestoreListener.EMAIL_IS_SUCCESSFUL, intent.getBooleanExtra(IBackupAndRestoreListener.EMAIL_IS_SUCCESSFUL, false)));
        }
    }
}
