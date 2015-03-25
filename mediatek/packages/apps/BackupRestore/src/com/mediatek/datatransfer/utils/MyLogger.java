package com.mediatek.datatransfer.utils;

import android.util.Log;

public class MyLogger {
    private MyLogger() {
    }

    public static final String LOG_TAG = "DataTransfer";
    public static final String BACKUP_ACTIVITY_TAG = "BackupActivity: ";
    public static final String BACKUP_SERVICE_TAG = "BackupService: ";
    public static final String BACKUP_ENGINE_TAG = "BackupEngine: ";

    public static final String APP_TAG = "App: ";
    public static final String CONTACT_TAG = "Contact: ";
    public static final String MESSAGE_TAG = "Message: ";
    public static final String MMS_TAG = "Mms: ";
    public static final String SMS_TAG = "SMS: ";
    public static final String MUSIC_TAG = "Music: ";
    public static final String PICTURE_TAG = "Picture: ";
    public static final String NOTEBOOK_TAG = "NoteBook: ";
    public static final String SETTINGS_TAG = "Settings: ";
    public static final String BOOKMARK_TAG = "Bookmark: ";
    
    public static void logV(String tag, String msg) {
        Log.v(tag, msg);
    }

    public static void logI(String tag, String msg) {
        Log.i(tag, msg);
    }

    public static void logD(String tag, String msg) {
        Log.d(tag, msg);
    }

    public static void logW(String tag, String msg) {
        Log.w(tag, msg);
    }

    public static void logE(String tag, String msg) {
        Log.e(tag, msg);
    }
}
