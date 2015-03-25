/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cellbroadcastreceiver;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.gsm.SmsCbConstants;
import com.mediatek.cmas.ext.ICmasSimSwap;
import com.mediatek.telephony.SmsManagerEx;




//import static com.android.cellbroadcastreceiver.CellBroadcastReceiver.DBG;

/**
 * This service manages enabling and disabling ranges of message identifiers
 * that the radio should listen for. It operates independently of the other
 * services and runs at boot time and after exiting airplane mode.
 *
 * Note that the entire range of emergency channels is enabled. Test messages
 * and lower priority broadcasts are filtered out in CellBroadcastAlertService
 * if the user has not enabled them in settings.
 *
 * TODO: add notification to re-enable channels after a radio reset.
 */
public class CellBroadcastConfigService extends IntentService {
    private static final String TAG = "[CMAS]CellBroadcastConfigService";
    private static final boolean DBG = true;

    static final String ACTION_ENABLE_CHANNELS = "ACTION_ENABLE_CHANNELS";
    public static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";
    public static final String STORED_SUBSCRIBER_ID = "stored_subscriber_id";
    public static final String ENABLE_CMAS_RMT_SUPPORT = "enable_cmas_rmt_support";
    public static final String ENABLE_CMAS_EXERCISE_SUPPORT = "enable_cmas_exercise_support";

    public CellBroadcastConfigService() {
        super(TAG);          // use class name for worker thread name
    }

    private static void setChannelRange(SmsManager manager, String ranges, boolean enable) {
        try {
            for (String channelRange : ranges.split(",")) {
                int dashIndex = channelRange.indexOf('-');
                if (dashIndex != -1) {
                    int startId = Integer.decode(channelRange.substring(0, dashIndex));
                    int endId = Integer.decode(channelRange.substring(dashIndex + 1));
                    if (enable) {
                        if (DBG) {
                            log("enabling emergency IDs " + startId + '-' + endId);
                        }
                        //manager.enableCellBroadcastRange(startId, endId);
                        SmsManagerEx.getDefault().enableCellBroadcastRange(startId, endId, CellBroadcastReceiver.sSlotId);
                    } else {
                        if (DBG) {
                            log("disabling emergency IDs " + startId + '-' + endId);
                        }
                        //manager.disableCellBroadcastRange(startId, endId);
                        SmsManagerEx.getDefault().disableCellBroadcastRange(startId, endId, CellBroadcastReceiver.sSlotId);
                    }
                } else {
                    int messageId = Integer.decode(channelRange);
                    if (enable) {
                        if (DBG) {
                            log("enabling emergency message ID " + messageId);
                        }
                        //manager.enableCellBroadcast(messageId);
                        SmsManagerEx.getDefault().enableCellBroadcast(messageId, CellBroadcastReceiver.sSlotId);
                    } else {
                        if (DBG) {
                            log("disabling emergency message ID " + messageId);
                        }
                        //manager.disableCellBroadcast(messageId);
                        SmsManagerEx.getDefault().disableCellBroadcast(messageId, CellBroadcastReceiver.sSlotId);
                    }
                }
            }
        } catch (NumberFormatException e) {
            Log.e(TAG, "Number Format Exception parsing emergency channel range", e);
        }
    }

    /**
     * Returns true if this is a standard or operator-defined emergency alert message.
     * This includes all ETWS and CMAS alerts, except for AMBER alerts.
     * @param message the message to test
     * @return true if the message is an emergency alert; false otherwise
     */
    static boolean isEmergencyAlertMessage(CellBroadcastMessage message) {
        Log.d(TAG, "enter isEmergencyAlertMessage");
        if (message.isEmergencyAlertMessage()) {
            return true;
        }

        if (message.getServiceCategory() == 
            SmsCbConstants.MESSAGE_ID_CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY) {
            return true;
        }

        // Check for system property defining the emergency channel ranges to enable
        String emergencyIdRange = SystemProperties.get("ro.cellbroadcast.emergencyids");
        if (TextUtils.isEmpty(emergencyIdRange)) {
            return false;
        }
        try {
            int messageId = message.getServiceCategory();
            for (String channelRange : emergencyIdRange.split(",")) {
                int dashIndex = channelRange.indexOf('-');
                if (dashIndex != -1) {
                    int startId = Integer.decode(channelRange.substring(0, dashIndex));
                    int endId = Integer.decode(channelRange.substring(dashIndex + 1));
                    if (messageId >= startId && messageId <= endId) {
                        return true;
                    }
                } else {
                    int emergencyMessageId = Integer.decode(channelRange);
                    if (emergencyMessageId == messageId) {
                        return true;
                    }
                }
            }
        } catch (NumberFormatException e) {
            Log.e(TAG, "Number Format Exception parsing emergency channel range", e);
        }
        return false;
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (DBG) {
            log(" onHandleIntent " + intent);
        }
        if (ACTION_ENABLE_CHANNELS.equals(intent.getAction())) {
            try {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
                Resources res = getResources();

                // Check for system property defining the emergency channel ranges to enable
                String emergencyIdRange = SystemProperties.get("ro.cellbroadcast.emergencyids");

                boolean enableEmergencyAlerts = prefs.getBoolean(
                        CellBroadcastSettings.KEY_ENABLE_EMERGENCY_ALERTS, true);

                boolean enableChannel50Alerts = res.getBoolean(R.bool.show_brazil_settings) &&
                        prefs.getBoolean(CellBroadcastSettings.KEY_ENABLE_CHANNEL_50_ALERTS, true);
                if (DBG) {
                    log("enabling emergency cell broadcast channels " + emergencyIdRange);
                }
                SmsManager manager = SmsManager.getDefault();
                if (enableEmergencyAlerts) {
                    if (!TextUtils.isEmpty(emergencyIdRange)) {
                        setChannelRange(manager, emergencyIdRange, true);
                    } else {
                        if (DBG) {
                            log("enableCellBroadcastRange " + 
                                    SmsCbConstants.MESSAGE_ID_CMAS_FIRST_IDENTIFIER + 
                                    " - "
                                    + SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE);
                        }
                        // No emergency channel system property, enable all emergency channels
                        //manager.enableCellBroadcastRange(
                        //        SmsCbConstants.MESSAGE_ID_PWS_FIRST_IDENTIFIER,
                        //        SmsCbConstants.MESSAGE_ID_PWS_LAST_IDENTIFIER);
                        SmsManagerEx.getDefault().enableCellBroadcastRange(
                                SmsCbConstants.MESSAGE_ID_CMAS_FIRST_IDENTIFIER,
                                SmsCbConstants.MESSAGE_ID_CMAS_ALERT_OPERATOR_DEFINED_USE, 
                                CellBroadcastReceiver.sSlotId);

                        if (CellBroadcastReceiver.sEnableCbMsg != null)
                        {
                            CellBroadcastReceiver.sEnableCbMsg.sendToTarget();
                            CellBroadcastReceiver.sEnableCbMsg = null;
                        }
                    }
                    if (DBG) {
                        log("enabled emergency cell broadcast channels");
                    }
                } else {
                    // we may have enabled these channels previously, so try to disable them
                    if (DBG) {
                        log("disabling emergency cell broadcast channels");
                    }
                    if (!TextUtils.isEmpty(emergencyIdRange)) {
                        setChannelRange(manager, emergencyIdRange, false);
                    } else {
                        if (DBG) {
                            log("disableCellBroadcastRange " + 
                                    SmsCbConstants.MESSAGE_ID_CMAS_FIRST_IDENTIFIER + " - "
                                    + SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE);
                        }
                        // No emergency channel system property, disable all emergency channels
                        //manager.disableCellBroadcastRange(
                        //        SmsCbConstants.MESSAGE_ID_PWS_FIRST_IDENTIFIER,
                        //        SmsCbConstants.MESSAGE_ID_PWS_LAST_IDENTIFIER);
                        SmsManagerEx.getDefault().disableCellBroadcastRange(
                                SmsCbConstants.MESSAGE_ID_CMAS_FIRST_IDENTIFIER,
                                SmsCbConstants.MESSAGE_ID_CMAS_ALERT_EXERCISE,
                                CellBroadcastReceiver.sSlotId);
                    }
                    if (DBG) {
                        log("disabled emergency cell broadcast channels");
                    }
                }

                if (enableChannel50Alerts) {
                    if (DBG) {
                        log("enabling cell broadcast channel 50");
                    }
                    //manager.enableCellBroadcast(50);
                } else {
                    if (DBG) {
                        log("disabling cell broadcast channel 50");
                    }
                    //manager.disableCellBroadcast(50);
                }
            } catch (Exception ex) {
                Log.e(TAG, "exception enabling cell broadcast channels", ex);
            }
            if (intent.getBooleanExtra("isBootCompleted", false)) {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
                SharedPreferences.Editor editor = prefs.edit();
                if (!prefs.contains(ENABLE_CMAS_RMT_SUPPORT)) {
                    editor.putBoolean(ENABLE_CMAS_RMT_SUPPORT, false);
                }
                if (!prefs.contains(ENABLE_CMAS_EXERCISE_SUPPORT)) {
                    editor.putBoolean(ENABLE_CMAS_EXERCISE_SUPPORT, false);
                }
                editor.commit();
                editor.clear();
            }
        } else if (CellBroadcastReceiver.SMS_STATE_CHANGED_ACTION.equals(intent.getAction())) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
            boolean enableCB = false;
            if (!prefs.contains(CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST)) {
                enableCB = true;
                Log.d(TAG, "do not contain the enable_cell_broadcast ");
            } else {
                enableCB = prefs.getBoolean(CheckBoxAndSettingsPreference.KEY_ENABLE_CELLBROADCAST,
                        false);
                Log.d(TAG, "contain the enable_cell_broadcast,enableCB = " + enableCB);
            }
            Log.d(TAG, "SMS_STATE_CHANGED_ACTION enableCB " + enableCB);
            if ( enableCB ) {
                if (SmsManagerEx.getDefault().activateCellBroadcastSms(true,
                        CellBroadcastReceiver.sSlotId)) {
                    log("enable CB after SMS_STATE_CHANGED_ACTION arrived");
                    CellBroadcastReceiver.startConfigService(this);
                } else {
                    log("failed to enable CB after SMS_STATE_CHANGED_ACTION arrived");
                }
            }
        } else if (ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
            TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(
                    Context.TELEPHONY_SERVICE);
            Log.d(TAG, "ACTION_SIM_STATE_CHANGED " + tm.getSimState());
            if (tm != null) {
                int state = tm.getSimState();
                if (TelephonyManager.SIM_STATE_READY == state) {
                    String subscriberId = "";
                    subscriberId = tm.getSubscriberId();
                    Log.d(TAG, "subscriberId = " + subscriberId);
                    SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
                    String storedScriberId = prefs.getString(STORED_SUBSCRIBER_ID, "");
                    Log.d(TAG, "storedScriberId = " + storedScriberId);
                    if (!TextUtils.isEmpty(subscriberId)
                            && (TextUtils.isEmpty(storedScriberId) || !subscriberId.equals(storedScriberId))) {
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putString(STORED_SUBSCRIBER_ID, subscriberId);
                        editor.commit();
                        editor.clear();
                        ICmasSimSwap iCmasSimSwap = (ICmasSimSwap)CellBroadcastPluginManager
                                .getCellBroadcastPluginObject(CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_SIM_SWAP);
                        iCmasSimSwap.simSwap(getApplicationContext(), R.xml.default_preference);
                    }
                    // Log.d(TAG, "ACTION_SIM_STATE_CHANGED, startConfigService " + tm.getSimState());
                    // CellBroadcastReceiver.startConfigService(this);
                }
            }
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
}
