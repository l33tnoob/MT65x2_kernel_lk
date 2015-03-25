package com.mediatek.regionalphone;

import java.io.File;

import android.content.ContentValues;
import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.net.Uri;
import android.os.Handler;
import android.util.Log;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.WallpaperManager;

import com.mediatek.regionalphone.Configuration;
import com.mediatek.xlog.Xlog;

public class RegionalPhoneRunnable implements Runnable {

    private static final String TAG = Common.LOG_TAG;
    private Context mContext;
    private String mPreMCCMNC = "";
    private SQLiteOpenHelper mDbOpenHelper;

    public RegionalPhoneRunnable(Context context) {
        this.mContext = context;
        this.mPreMCCMNC = Configuration.getInstance().readMCCMNC();
        this.mDbOpenHelper = new DatabaseOpenHelper(this.mContext);
    }

    @Override
    public void run() {
        modifyDatabaseForFirstValidSIM();
    }

    public void modifyDatabaseForFirstValidSIM() {
        if (isFirstValidSIMCard()) {
            SQLiteDatabase db = mDbOpenHelper.getWritableDatabase();

            String mcc_mnc_timestamp = this.mPreMCCMNC + Configuration.getInstance().getTimestamp();
            modifyMMSSMSTable(db, mcc_mnc_timestamp);
            modifyApnTable(db, mcc_mnc_timestamp);
            modifySettingTable(db, mcc_mnc_timestamp);
            // This must first modify SearchEngineTable then modify BrowserTable,
            // because Browser don't register observer of SearchEngineTable.
            modifySearchEngineTable(db, mcc_mnc_timestamp);
            modifyBrowserTable(db, mcc_mnc_timestamp);
            modifyWallpaperTable(db, mcc_mnc_timestamp);

            db.close();

            sendNotification();
        }
    }

    private void sendNotification() {
        NotificationManager notifiManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        Notification noti = new Notification.Builder(this.mContext)
                .setContentTitle("Finish Regional Phone Config")
                .setContentText("Finish Regional Phone Config")
                .setSmallIcon(R.drawable.ic_launcher).build();

        notifiManager.notify(1, noti);
    }

    private boolean isFirstValidSIMCard() {
        Xlog.d(TAG, "isFirstValidSIMCard in");
        boolean flag = false;

        // first valid SIM card
        if (this.mPreMCCMNC.equals("")) {
            String mcc_mnc = Configuration.getInstance().readMCCMNCFromProperty();
            if (mcc_mnc.length() >= 5) {
                String mcc = mcc_mnc.substring(0, 3);
                String mnc = mcc_mnc.substring(3);

                SQLiteDatabase db = mDbOpenHelper.getReadableDatabase();
                Cursor cursor = db.query(RegionalPhoneOp.TABLE_MCC_MNC, new String[] {
                        RegionalPhoneOp.MCC_MNC.MCC, RegionalPhoneOp.MCC_MNC.MNC },
                        RegionalPhoneOp.MCC_MNC.MCC + "=? and " + RegionalPhoneOp.MCC_MNC.MNC
                                + "=?", new String[] { mcc, mnc }, null, null, null);
                // can find out mcc/mnc in default table
                if (cursor != null && cursor.getCount() > 0) {
                    flag = true;
                    this.mPreMCCMNC = mcc_mnc;
                    Configuration.getInstance().saveMCCMNC(mcc_mnc);
                } else {
                    Configuration.getInstance().saveInvalidMCCMNC(mcc_mnc);
                }

                if (cursor != null) {
                    cursor.close();
                }
            }
        }

        return flag;
    }

    private void modifyMMSSMSTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifyMMSSMSTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_MMS_SMS, null,
                RegionalPhoneOp.MMS_SMS.MCC_MNC + "=?", new String[] { this.mPreMCCMNC }, null,
                null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.MMS_SMS._ID));
            String creationMode = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.MMS_SMS.MMS_CREATION_MODE));
            String CNumber = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.MMS_SMS.SMS_C_NUMBER));
            ContentValues values = new ContentValues();
            values.put(RegionalPhone.MMS_SMS._ID, _id);
            values.put(RegionalPhone.MMS_SMS.MMS_CREATION_MODE, creationMode);
            values.put(RegionalPhone.MMS_SMS.SMS_C_NUMBER, CNumber);
            values.put(RegionalPhone.MMS_SMS.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            this.mContext.getContentResolver().insert(RegionalPhone.MMS_SMS_URI, values);
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void modifyApnTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifyApnTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_APN, null, RegionalPhoneOp.APN.MCC_MNC
                + "=?", new String[] { this.mPreMCCMNC }, null, null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.APN._ID));
            String mms_name = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.APN.MMS_NAME));
            String mms_server = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.APN.MMS_SERVER));
            String mms_GPRS_APN = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.APN.MMS_GPRS_APN));
            String sms_preferredBearer = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.APN.SMS_PREFERRED_BEARER));
            String mms_proxy = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.APN.MMS_PROXY));
            int mms_port = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.APN.MMS_PORT));
            ContentValues values = new ContentValues();
            values.put(RegionalPhone.APN._ID, _id);
            values.put(RegionalPhone.APN.MMS_NAME, mms_name);
            values.put(RegionalPhone.APN.MMS_SERVER, mms_server);
            values.put(RegionalPhone.APN.MMS_GPRS_APN, mms_GPRS_APN);
            values.put(RegionalPhone.APN.SMS_PREFERRED_BEARER, sms_preferredBearer);
            values.put(RegionalPhone.APN.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            values.put(RegionalPhone.APN.MMS_PROXY, mms_proxy);
            values.put(RegionalPhone.APN.MMS_PORT, mms_port);
            this.mContext.getContentResolver().insert(RegionalPhone.APN_URI, values);
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void modifySettingTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifySettingTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_SETTINGS, null,
                RegionalPhoneOp.SETTINGS.MCC_MNC + "=?", new String[] { this.mPreMCCMNC }, null,
                null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.SETTINGS._ID));
            int NITZAutoUpdate = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.SETTINGS.NITZ_AUTOUPDATE));
            int wifi = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.SETTINGS.WIFI_DEFAULT));
            ContentValues values = new ContentValues();
            values.put(RegionalPhone.SETTINGS._ID, _id);
            values.put(RegionalPhone.SETTINGS.NITZ_AUTOUPDATE, NITZAutoUpdate);
            values.put(RegionalPhone.SETTINGS.WIFI_DEFAULT, wifi);
            values.put(RegionalPhone.SETTINGS.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            this.mContext.getContentResolver().insert(RegionalPhone.SETTINGS_URI, values);
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void modifyBrowserTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifyBrowserTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_BROWSER, null,
                RegionalPhoneOp.BROWSER.MCC_MNC + "=?", new String[] { this.mPreMCCMNC }, null,
                null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.BROWSER._ID));
            // book marks info
            String bookmarkTitle = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.BROWSER.BOOKMARK_TITLE));
            String bookmarkURL = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.BROWSER.BOOKMARK_URL));
            byte[] thumbnail = cursor.getBlob(cursor.getColumnIndex(RegionalPhoneOp.BROWSER.THUMBNAIL));
            int folder = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.BROWSER.IS_FOLDER));
            int parent = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.BROWSER.PARENT));

            ContentValues values = new ContentValues();
            values.put(RegionalPhone.BROWSER._ID, _id);

            values.put(RegionalPhone.BROWSER.BOOKMARK_TITLE, bookmarkTitle);
            values.put(RegionalPhone.BROWSER.BOOKMARK_URL, bookmarkURL);
            values.put(RegionalPhone.BROWSER.THUMBNAIL, thumbnail);
            values.put(RegionalPhone.BROWSER.IS_FOLDER, folder);
            values.put(RegionalPhone.BROWSER.PARENT, parent);

            values.put(RegionalPhone.BROWSER.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            this.mContext.getContentResolver().insert(RegionalPhone.BROWSER_URI, values);
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void modifySearchEngineTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifySearchEngineTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_SEARCHENGINE, null,
                RegionalPhoneOp.SEARCHENGINE.MCC_MNC + "=?", new String[] { this.mPreMCCMNC }, null,
                null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.BROWSER._ID));

            // search engine info
            String searchEngineName = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.SEARCH_ENGINE_NAME));
            String searchEngineLabel = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.SEARCH_ENGINE_LABEL));
            String keyword = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.KEYWORD));
            String favicon = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.FAVICON));
            String searchURL = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.SEARCH_URL));
            String encoding = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.ENCODING));
            String suggestionURL = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.SEARCHENGINE.SUGGESTION_URL));

            ContentValues values = new ContentValues();
            values.put(RegionalPhone.SEARCHENGINE._ID, _id);

            values.put(RegionalPhone.SEARCHENGINE.SEARCH_ENGINE_NAME, searchEngineName);
            values.put(RegionalPhone.SEARCHENGINE.SEARCH_ENGINE_LABEL, searchEngineLabel);
            values.put(RegionalPhone.SEARCHENGINE.KEYWORD, keyword);
            values.put(RegionalPhone.SEARCHENGINE.FAVICON, favicon);
            values.put(RegionalPhone.SEARCHENGINE.SEARCH_URL, searchURL);
            values.put(RegionalPhone.SEARCHENGINE.ENCODING, encoding);
            values.put(RegionalPhone.SEARCHENGINE.SUGGESTION_URL, suggestionURL);

            values.put(RegionalPhone.SEARCHENGINE.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            this.mContext.getContentResolver().insert(RegionalPhone.SEARCHENGINE_URI, values);
        }

        if (cursor != null) {
            cursor.close();
        }
    }

    private void modifyWallpaperTable(SQLiteDatabase db, String mcc_mnc_timestamp) {
        Xlog.d(TAG, "modifyWallpaperTable in");
        Cursor cursor = db.query(RegionalPhoneOp.TABLE_WALLPAPER, null,
                RegionalPhoneOp.WALLPAPER.MCC_MNC + "=?", new String[] { this.mPreMCCMNC }, null,
                null, null);
        while (cursor != null && cursor.moveToNext()) {
            int _id = cursor.getInt(cursor.getColumnIndex(RegionalPhoneOp.WALLPAPER._ID));
            String fileName = cursor.getString(cursor.getColumnIndex(RegionalPhoneOp.WALLPAPER.IMAGE_FILE_NAME));
            ContentValues values = new ContentValues();
            values.put(RegionalPhone.WALLPAPER._ID, _id);
            values.put(RegionalPhone.WALLPAPER.IMAGE_FILE_NAME, fileName);
            values.put(RegionalPhone.WALLPAPER.MCC_MNC_TIMESTAMP, mcc_mnc_timestamp);
            this.mContext.getContentResolver().insert(RegionalPhone.WALLPAPER_URI, values);
        }

        try {
             WallpaperManager.getInstance(this.mContext).clear();
        } catch (Exception e) {
             Xlog.d(TAG, "WallpaperManager clear exception!");
             e.printStackTrace();
        }

        if (cursor != null) {
            cursor.close();
        }
    }

}
