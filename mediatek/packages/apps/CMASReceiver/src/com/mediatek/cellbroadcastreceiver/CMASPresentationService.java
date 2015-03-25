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
import android.os.PowerManager;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.util.ArrayList;

public class CMASPresentationService extends Service {
    private static final String TAG = "[CMAS][CMASPresentationService]";

    static final int NOTIFICATION_ID = 100;

    private PowerManager.WakeLock mWakeLock;

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "enter onStartCommand");

        showNewAlert(intent);

        return START_NOT_STICKY;
    }

    private void showNewAlert(Intent intent) {
        Bundle extras = intent.getExtras();
        if (extras == null) {
            Log.e(TAG, "received SHOW_NEW_ALERT_ACTION with no extras!");
            return;
        }

        CellBroadcastMessage cbm = (CellBroadcastMessage) extras.get("message");
        long msgRowId = extras.getLong(CellBroadcastAlertService.CB_ROWID);
        if (msgRowId < 0) {
            Log.e(TAG, "error message, do not show it");
            return;
        }

        if (cbm == null) {
            Log.e(TAG, "received SHOW_NEW_ALERT_ACTION with no message extras!");
        }

        if (CellBroadcastConfigService.isEmergencyAlertMessage(cbm)) {
            // start alert sound/ vibration/ TTS adn display full-screen alert
            openEmergencyAlertNotification(cbm, msgRowId);
        }
    }

    private void acquireTimedWakelock(int time) {
        if (mWakeLock == null) {
            PowerManager pm = (PowerManager)getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP, TAG);
        }

        mWakeLock.acquire(time);
    }

    /**
     * Display a full-screen alert message for emergency alerts.
     * @param message the alert to display
     */
    private void openEmergencyAlertNotification(CellBroadcastMessage message, long msgRowId) {
        Log.i(TAG, "enter openEmergencyAlertNotification");
        // Acquire a CPU wake lock until the alert dialog and audio start playing.
        //CellBroadcastAlertWakeLock.acquireScreenCpuWakeLock(this);
        acquireTimedWakelock(5000);

        // Close dialogs and window shade
        Intent closeDialogs = new Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        sendBroadcast(closeDialogs);

        // start audio/vibration/speech service for emergency alerts
        Intent audioIntent = new Intent(this, CellBroadcastAlertAudio.class);
        audioIntent.setAction(CellBroadcastAlertAudio.ACTION_START_ALERT_AUDIO);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);

        int duration; //aler audio duration in ms
        if (message.isCmasMessage()) {
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

        int channelTitleId = CellBroadcastResources.getDialogTitleResource(message);
        CharSequence channelName = getText(channelTitleId);
        String messageBody = message.getMessageBody();

        TelephonyManager tm = (TelephonyManager)getSystemService(Service.TELEPHONY_SERVICE);
        if (getEnbaleAlertSpeech() && tm.getCallState() == TelephonyManager.CALL_STATE_IDLE) {
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

        Intent notifyIntent = createDisplayMessageIntent(this, CellBroadcastListActivity.class, messageList);
        notifyIntent.putExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, true);

        PendingIntent pi = PendingIntent.getActivity(this, 0, notifyIntent,
                PendingIntent.FLAG_ONE_SHOT | PendingIntent.FLAG_UPDATE_CURRENT);

        Notification.Builder builder = new Notification.Builder(this)
            .setSmallIcon(R.drawable.ic_notification_alarm)
            .setTicker(getText(CellBroadcastResources.getDialogTitleResource(message)))
            .setWhen(System.currentTimeMillis())
            .setContentIntent(pi)
            //.setFullScreenIntent(pi, true)
            /*.setContentTitle(channelName)
            .setContentText(messageBody)*/
            .setDefaults(Notification.DEFAULT_LIGHTS);

        int unreadCount = getUnReadCount();
        if (unreadCount > 1) {
            builder.setContentTitle(getString(R.string.alerts));
            builder.setContentText(getString(R.string.notification_multiple, unreadCount));
        } else {
            builder.setContentTitle(channelName);
            builder.setContentText(messageBody);
        }


        NotificationManager notificationManager =
            (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);

        notificationManager.notify(NOTIFICATION_ID, builder.getNotification());


        /*Intent alertDialogIntent = createDisplayMessageIntent(this, c, messageList);
        alertDialogIntent.putExtra(CellBroadcastAlertFullScreen.FROM_NOTIFICATION_EXTRA, true);
        alertDialogIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(alertDialogIntent);*/
        Log.i(TAG, "before call to showView");
        CMASAlertFullWindow showWindow = CMASAlertFullWindow.getInstance(getApplicationContext());
        showWindow.showView(getLatestMessage(messageList), msgRowId, true);
        /*if (km.inKeyguardRestrictedInputMode()) {
            Log.i(TAG, "inKeyguardRestrictedInputMode ");
            showWindow = new CMASAlertFullWindow(getApplicationContext(), messageList, true);
        } else {
            showWindow = new CMASAlertDialogWindow(getApplicationContext(), messageList, true);
        }
        showWindow.showView();*/

    }

    CellBroadcastMessage getLatestMessage(ArrayList<CellBroadcastMessage> messageList) {
        int index = messageList.size() - 1;
        if (index >= 0) {
            return messageList.get(index);
        } else {
            return null;
        }
    }

    static Intent createDisplayMessageIntent(Context context, Class intentClass,
            ArrayList<CellBroadcastMessage> messageList) {
        // Trigger the list activity to fire up a dialog that shows the received messages
        Intent intent = new Intent(context, intentClass);
        intent.putParcelableArrayListExtra(CellBroadcastMessage.SMS_CB_MESSAGE_EXTRA, messageList);
        return intent;
    }

    private int getUnReadCount() {
        /*int ret = 0;
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(
                getContentResolver()).execute(
                        new CellBroadcastContentProvider.CellBroadcastOperation() {
                            public boolean execute(CellBroadcastContentProvider provider) {
                                // TODO Auto-generated method stub
                                ret = provider.getUnreadCellBroadcastCount();
                                return false;
                            }

                        });

        return ret;*/

        Log.i(TAG, "getUnReadCount");
        int unreadCount = 0;
        Cursor cursor = getContentResolver().query(CellBroadcastContentProvider.CONTENT_URI, null, 
                Telephony.CellBroadcasts.MESSAGE_READ + " =0", null, null);
        if (cursor != null) {
            unreadCount = cursor.getCount();
            cursor.close();
        }

        return unreadCount;
    }

    private boolean getEnbaleAlertSpeech() {
        return PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_SPEECH, true);
    }

}
