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

import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.os.Bundle;
import android.os.IBinder;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsCbCmasInfo;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.util.Log;

import com.mediatek.cmas.ext.ICmasDuplicateMessage;

import java.util.ArrayList;
import java.util.HashSet;

/**
 * This service manages the display and animation of broadcast messages.
 * Emergency messages display with a flashing animated exclamation mark icon,
 * and an alert tone is played when the alert is first shown to the user
 * (but not when the user views a previously received broadcast).
 */
public class CellBroadcastAlertService extends Service {
    private static final String TAG = "[CMAS]CellBroadcastAlertService";

    /** Intent action to display alert dialog/notification, after verifying the alert is new. */
    static final String SHOW_NEW_ALERT_ACTION = "cellbroadcastreceiver.SHOW_NEW_ALERT";

    public static final String CB_ROWID = "msg_rowid";

    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";

    /** Use the same notification ID for non-emergency alerts. */
    static final int NOTIFICATION_ID = 1;

    /** Container for message ID and geographical scope, for duplicate message detection. */
    private static final class MessageServiceCategoryAndScope {
        private final int mServiceCategory;
        private final int mSerialNumber;
        private final SmsCbLocation mLocation;

        MessageServiceCategoryAndScope(int serviceCategory, int serialNumber,
                SmsCbLocation location) {
            mServiceCategory = serviceCategory;
            mSerialNumber = serialNumber;
            mLocation = location;
        }

        @Override
        public int hashCode() {
            return mLocation.hashCode() + 5 * mServiceCategory + 7 * mSerialNumber;
        }

        @Override
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            }
            if (o instanceof MessageServiceCategoryAndScope) {
                MessageServiceCategoryAndScope other = (MessageServiceCategoryAndScope) o;
                return (mServiceCategory == other.mServiceCategory &&
                        mLocation.equals(other.mLocation));
            }
            return false;
        }

        @Override
        public String toString() {
            return "{mServiceCategory: " + mServiceCategory + " serial number: " + mSerialNumber +
                    " location: " + mLocation.toString() + '}';
        }
    }

    /** Cache of received message IDs, for duplicate message detection. */
    private static final HashSet<MessageServiceCategoryAndScope> sCmasIdSet =
            new HashSet<MessageServiceCategoryAndScope>(8);

    /** Maximum number of message IDs to save before removing the oldest message ID. */
    private static final int MAX_MESSAGE_ID_SIZE = 65535;

    /** List of message IDs received, for removing oldest ID when max message IDs are received. */
    private static final ArrayList<MessageServiceCategoryAndScope> sCmasIdList =
            new ArrayList<MessageServiceCategoryAndScope>(8);

    /** Index of message ID to replace with new message ID when max message IDs are received. */
    private static int sCmasIdListIndex = 0;

    private static Comparer sCompareList = new Comparer();

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();
        if (Telephony.Sms.Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION.equals(action) ||
                Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION.equals(action)) {
            handleCellBroadcast(intent);
            //handleCellBroadcastIntent(intent);
        } else if (SHOW_NEW_ALERT_ACTION.equals(action)) {
            showNewAlert(intent);
        } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
            sCompareList.clear();
        } else {
            Log.e(TAG, "Unrecognized intent action: " + action);
        }
        return START_NOT_STICKY;
    }

   private void handleCellBroadcast(Intent intent) {
        Log.d(TAG, "handleCellBroadcast " + intent);
        Bundle extras = intent.getExtras();
        if (extras == null) {
            Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no extras!");
            return;
        }

        final SmsCbMessage message = (SmsCbMessage) extras.get("message");

        if (message == null) {
            Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no message extra");
            return;
        }

        final CellBroadcastMessage cbm = new CellBroadcastMessage(message);
        if (!isMessageEnabledByUser(cbm)) {
            Log.d(TAG, "ignoring alert of type " + cbm.getServiceCategory() + " by user preference");
            return;
        }
        if (!sCompareList.add(cbm)) {
            // reject the duplicate cmas in non power-off & non off-airplane
            Log.d(TAG, "ignoring duplicate alert with " + cbm);
            return;
        }
        final Intent alertIntent = new Intent(SHOW_NEW_ALERT_ACTION);
        alertIntent.setClass(this, CMASPresentationService.class);
        alertIntent.putExtra("message", cbm);

        // write to database on a background thread
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        int res = ICmasDuplicateMessage.NEW_CMAS_PROCESS;
                        // ICmasDuplicateMessage optDetection = getOperatorInterfaceFordupulication();
                        ICmasDuplicateMessage optDetection = (ICmasDuplicateMessage) 
                             CellBroadcastPluginManager.getCellBroadcastPluginObject(
                             CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE);
                        // Log.d(TAG, "execute, get ICmasDuplicateMessage " + optDetection);
                        if (optDetection != null) {
                            res = optDetection.handleDuplicateMessage(provider, cbm);
                            Log.d(TAG, "res = " + res);
                        }

                        switch (res) {
                        case ICmasDuplicateMessage.NEW_CMAS_PROCESS: // NewCMASProcess
                            // handle update msg
                            if (!handleUpdatedCB(provider, cbm)) {
                                return true;
                            }
                            Log.d(TAG, "before insertNewBroadcast, sn " + cbm.getSerialNumber());
                            //if (provider.insertNewBroadcast(cbm)) {
                            long rowId = provider.addNewBroadcast(cbm);
                            if (rowId > -1) {
                                alertIntent.putExtra(CB_ROWID, rowId);
                                startService(alertIntent);
                                /*
                                Cursor c = provider.getAllCellBroadcastCursor();
                                try {
                                    c.moveToFirst();
                                    int oldSN = c.getInt(c.getColumnIndexOrThrow(Telephony.CellBroadcasts.SERIAL_NUMBER));
                                    Log.d(TAG, "after insertNewBroadcast, sn " + oldSN);
                                } finally {
                                    c.close();
                                }
                                */
                                return true;
                            }
                            return false;
                        case ICmasDuplicateMessage.PRESENT_CMAS_PROCESS: // PresentCMASProcess
                            long id= provider.getRowId(message);
                            Log.d(TAG, "id = " + id);
                            if(id > -1) {
                                alertIntent.putExtra(CB_ROWID, id);
                            }
                            startService(alertIntent);
                            return true;
                        case ICmasDuplicateMessage.DISCARD_CMAS_PROCESS: // DiscardCMASProcess
                        default:
                            return true;
                        }
                    }
                });
    }

    private boolean handleUpdatedCB(CellBroadcastContentProvider provider, CellBroadcastMessage cbm) {
        Cursor c = provider.getAllCellBroadcastCursor();
        String oldIDstr = Comparer.searchUpdatedCB(c, cbm);

        if (oldIDstr != null) {
            if (oldIDstr.equals(Comparer.INVALID_UPDATE_CB)) {
                Log.d(TAG, "handleUpdatedCB " + oldIDstr);
                return false;
            }

            if (!provider.deleteBroadcast(Integer.valueOf(oldIDstr).intValue())) {
                Log.e(TAG, "error handleUpdateCB,failed to delete ID " + oldIDstr);
            }
        }

        return true;
    }

    private void handleCellBroadcastIntent(Intent intent) {
        Bundle extras = intent.getExtras();
        if (extras == null) {
            Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no extras!");
            return;
        }

        SmsCbMessage message = (SmsCbMessage) extras.get("message");

        if (message == null) {
            Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no message extra");
            return;
        }

        final CellBroadcastMessage cbm = new CellBroadcastMessage(message);
        if (!isMessageEnabledByUser(cbm)) {
            Log.d(TAG, "ignoring alert of type " + cbm.getServiceCategory() +
                    " by user preference");
            return;
        }

        // Check for duplicate message IDs according to CMAS carrier requirements. Message IDs
        // are stored in volatile memory. If the maximum of 65535 messages is reached, the
        // message ID of the oldest message is deleted from the list.
        MessageServiceCategoryAndScope newCmasId = new MessageServiceCategoryAndScope(
                message.getServiceCategory(), message.getSerialNumber(), message.getLocation());

        // Add the new message ID to the list. It's okay if this is a duplicate message ID,
        // because the list is only used for removing old message IDs from the hash set.
        if (sCmasIdList.size() < MAX_MESSAGE_ID_SIZE) {
            sCmasIdList.add(newCmasId);
        } else {
            // Get oldest message ID from the list and replace with the new message ID.
            MessageServiceCategoryAndScope oldestCmasId = sCmasIdList.get(sCmasIdListIndex);
            sCmasIdList.set(sCmasIdListIndex, newCmasId);
            Log.d(TAG, "message ID limit reached, removing oldest message ID " + oldestCmasId);
            // Remove oldest message ID from the set.
            sCmasIdSet.remove(oldestCmasId);
            if (++sCmasIdListIndex >= MAX_MESSAGE_ID_SIZE) {
                sCmasIdListIndex = 0;
            }
        }
        // Set.add() returns false if message ID has already been added
        if (!sCmasIdSet.add(newCmasId)) {
            Log.d(TAG, "ignoring duplicate alert with " + newCmasId);
            return;
        }

        final Intent alertIntent = new Intent(SHOW_NEW_ALERT_ACTION);
        alertIntent.setClass(this, CMASPresentationService.class);
        alertIntent.putExtra("message", cbm);

        // write to database on a background thread
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        if (provider.insertNewBroadcast(cbm)) {
                            // new message, show the alert or notification on UI thread
                            startService(alertIntent);
                            return true;
                        } else {
                            return false;
                        }
                    }
                });
    }

    private void showNewAlert(Intent intent) {
        Bundle extras = intent.getExtras();
        if (extras == null) {
            Log.e(TAG, "received SHOW_NEW_ALERT_ACTION with no extras!");
            return;
        }

        CellBroadcastMessage cbm = (CellBroadcastMessage) extras.get("message");

        if (cbm == null) {
            Log.e(TAG, "received SHOW_NEW_ALERT_ACTION with no message extra");
            return;
        }

        if (CellBroadcastConfigService.isEmergencyAlertMessage(cbm)) {
            // start alert sound / vibration / TTS and display full-screen alert
            openEmergencyAlertNotification(cbm);
        } else {
            // add notification to the bar
            addToNotificationBar(cbm);
        }
    }

    /**
     * Filter out broadcasts on the test channels that the user has not enabled,
     * and types of notifications that the user is not interested in receiving.
     * This allows us to enable an entire range of message identifiers in the
     * radio and not have to explicitly disable the message identifiers for
     * test broadcasts. In the unlikely event that the default shared preference
     * values were not initialized in CellBroadcastReceiverApp, the second parameter
     * to the getBoolean() calls match the default values in res/xml/preferences.xml.
     *
     * @param message the message to check
     * @return true if the user has enabled this message type; false otherwise
     */
    private boolean isMessageEnabledByUser(CellBroadcastMessage message) {
        Log.d(TAG, "isMessageEnabledByUser " + message.getServiceCategory());

        if (message.isEtwsMessage()) {
            if (message.isEtwsTestMessage()) {
                return PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                        CellBroadcastSettings.KEY_ENABLE_ETWS_TEST_ALERTS, false);
            }
            return true;
        }

        if (message.isCmasMessage()) {
            //Log.d(TAG, "in isMessageEnabledByUser , CMASMessageClass " + message.getCmasMessageClass());
            switch (message.getCmasMessageClass()) {
                case SmsCbCmasInfo.CMAS_CLASS_PRESIDENTIAL_LEVEL_ALERT:
                    return true;
                case SmsCbCmasInfo.CMAS_CLASS_EXTREME_THREAT:
                    return PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_EXTREME_ALERTS, true);
                case SmsCbCmasInfo.CMAS_CLASS_SEVERE_THREAT:
                    return PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                            CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_SEVERE_ALERTS, true);
                case SmsCbCmasInfo.CMAS_CLASS_CHILD_ABDUCTION_EMERGENCY:
                    return PreferenceManager.getDefaultSharedPreferences(this)
                            .getBoolean(CheckBoxAndSettingsPreference.KEY_ENABLE_CMAS_AMBER_ALERTS, true);
                case SmsCbCmasInfo.CMAS_CLASS_REQUIRED_MONTHLY_TEST:
                
                    SharedPreferences rmtPrefs = this.getSharedPreferences(
                            PREF_NAME, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE
                            | MODE_MULTI_PROCESS);
                    boolean resOfRmt = rmtPrefs.getBoolean(CellBroadcastConfigService.ENABLE_CMAS_RMT_SUPPORT, false);
                    Log.d(TAG, "in isMessageEnabledByUser , CMAS setting " + resOfRmt);
                    return resOfRmt;

                case SmsCbCmasInfo.CMAS_CLASS_CMAS_EXERCISE:

                    SharedPreferences exePrefs = this.getSharedPreferences(
                            PREF_NAME, MODE_WORLD_READABLE | MODE_WORLD_WRITEABLE
                            | MODE_MULTI_PROCESS);
                    boolean resOfexe = exePrefs.getBoolean(CellBroadcastConfigService.ENABLE_CMAS_EXERCISE_SUPPORT, false);
                    Log.d(TAG, "in isMessageEnabledByUser , EXER setting " + resOfexe);
                    return resOfexe;
                default:
                    return false;    // presidential-level CMAS alerts are always enabled
            }
        }

        return false;    // other broadcast messages are always enabled
    }

    /**
     * Display a full-screen alert message for emergency alerts.
     * @param message the alert to display
     */
    private void openEmergencyAlertNotification(CellBroadcastMessage message) {
        // Acquire a CPU wake lock until the alert dialog and audio start playing.
        CellBroadcastAlertWakeLock.acquireScreenCpuWakeLock(this);

        // Close dialogs and window shade
        Intent closeDialogs = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        sendBroadcast(closeDialogs);

        // start audio/vibration/speech service for emergency alerts
        Intent audioIntent = new Intent(this, CellBroadcastAlertAudio.class);
        audioIntent.setAction(CellBroadcastAlertAudio.ACTION_START_ALERT_AUDIO);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

        int duration;   // alert audio duration in ms
        if (message.isCmasMessage()) {
            // CMAS requirement: duration of the audio attention signal is 10.5 seconds.
            duration = 10500;
        } else {
            duration = Integer.parseInt(prefs.getString(
                    CellBroadcastSettings.KEY_ALERT_SOUND_DURATION,
                    CellBroadcastSettings.ALERT_SOUND_DEFAULT_DURATION)) * 1000;
        }
        audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_DURATION_EXTRA, duration);

        if (message.isEtwsMessage()) {
            // For ETWS, always vibrate, even in silent mode.
            audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_VIBRATE_EXTRA, true);
            audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_ETWS_VIBRATE_EXTRA, true);
        } else {
            // For other alerts, vibration can be disabled in app settings.
            audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_VIBRATE_EXTRA,
                    prefs.getBoolean(CellBroadcastSettings.KEY_ENABLE_ALERT_VIBRATE, true));
        }

        String messageBody = message.getMessageBody();

        if (prefs.getBoolean(CellBroadcastSettings.KEY_ENABLE_ALERT_SPEECH, true)) {
            audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_MESSAGE_BODY, messageBody);

            String language = message.getLanguageCode();
            if (message.isEtwsMessage() && !"ja".equals(language)) {
                Log.w(TAG, "bad language code for ETWS - using Japanese TTS");
                language = "ja";
            } else if (message.isCmasMessage() && !"en".equals(language)) {
                Log.w(TAG, "bad language code for CMAS - using English TTS");
                language = "en";
            }
            audioIntent.putExtra(CellBroadcastAlertAudio.ALERT_AUDIO_MESSAGE_LANGUAGE,
                    language);
        }
        startService(audioIntent);

        // Decide which activity to start based on the state of the keyguard.
        Class c = CellBroadcastAlertDialog.class;
        KeyguardManager km = (KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE);
        if (km.inKeyguardRestrictedInputMode()) {
            // Use the full screen activity for security.
            c = CellBroadcastAlertFullScreen.class;
        }

        ArrayList<CellBroadcastMessage> messageList = new ArrayList<CellBroadcastMessage>(1);
        messageList.add(message);

        Intent alertDialogIntent = createDisplayMessageIntent(this, c, messageList);
        alertDialogIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(alertDialogIntent);
    }

    /**
     * Add the new alert to the notification bar (non-emergency alerts), or launch a
     * high-priority immediate intent for emergency alerts.
     * @param message the alert to display
     */
    private void addToNotificationBar(CellBroadcastMessage message) {
        int channelTitleId = CellBroadcastResources.getDialogTitleResource(message);
        CharSequence channelName = getText(channelTitleId);
        String messageBody = message.getMessageBody();

        // Pass the list of unread non-emergency CellBroadcastMessages
        ArrayList<CellBroadcastMessage> messageList = CellBroadcastReceiverApp
                .addNewMessageToList(message);

        // Create intent to show the new messages when user selects the notification.
        Intent intent = createDisplayMessageIntent(this, CellBroadcastAlertDialog.class,
                messageList);
        intent.putExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, true);

        PendingIntent pi = PendingIntent.getActivity(this, 0, intent,
                PendingIntent.FLAG_ONE_SHOT | PendingIntent.FLAG_UPDATE_CURRENT);

        // use default sound/vibration/lights for non-emergency broadcasts
        Notification.Builder builder = new Notification.Builder(this)
                .setSmallIcon(R.drawable.ic_notify_alert)
                .setTicker(channelName)
                .setWhen(System.currentTimeMillis())
                .setContentIntent(pi)
                .setDefaults(Notification.DEFAULT_ALL);

        builder.setDefaults(Notification.DEFAULT_ALL);

        // increment unread alert count (decremented when user dismisses alert dialog)
        int unreadCount = messageList.size();
        if (unreadCount > 1) {
            // use generic count of unread broadcasts if more than one unread
            builder.setContentTitle(getString(R.string.notification_multiple_title));
            builder.setContentText(getString(R.string.notification_multiple, unreadCount));
        } else {
            builder.setContentTitle(channelName).setContentText(messageBody);
        }

        NotificationManager notificationManager =
            (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);

        notificationManager.notify(NOTIFICATION_ID, builder.build());
    }

    static Intent createDisplayMessageIntent(Context context, Class intentClass,
            ArrayList<CellBroadcastMessage> messageList) {
        // Trigger the list activity to fire up a dialog that shows the received messages
        Intent intent = new Intent(context, intentClass);
        intent.putParcelableArrayListExtra(CellBroadcastMessage.SMS_CB_MESSAGE_EXTRA, messageList);
        return intent;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;    // clients can't bind to this service
    }
}
