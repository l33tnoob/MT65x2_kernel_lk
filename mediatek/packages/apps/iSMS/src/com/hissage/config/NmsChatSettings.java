package com.hissage.config;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.contact.NmsContact;
import com.hissage.db.NmsContentResolver;
import com.hissage.db.NmsDBUtils;
import com.hissage.message.smsmms.NmsCreateSmsThread;
import com.hissage.message.smsmms.NmsSMSMMS;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.util.log.NmsLog;

/**
 * The Class NmsChatSettings, the threads(conversation) settings object, include
 * wall paper\notification etc.
 */
public class NmsChatSettings {

    /** The log TAG. */
    private static final String TAG = "SNmsChatSettings";

    /**
     * For the intent extra key, when user lunch the chat settings activity,
     * must set this param.
     */
    public static final String CHAT_SETTINGS_KEY = "contactId";

    /** this id is isms db id. */
    public short mContactId = 0;

    /** The chat room(bubble list) background. */
    public String mWallPaper = null;

    /** The notification switch, default is on. */
    public int mNotification = 1;

    /** The mute switch, default is off. */
    public int mMute = 0;

    /**
     * for the time stamp, when user choose mute1\2\4\8hours, the count start at
     * this time.
     */
    public int mMute_start = 0;

    /** recv incoming ipMsg, this ringtone will be play. */
    public String mRingtone = null;

    /** The incoming ipMsg vibrate switch, default is on. */
    public int mVibrate = 1;

    public long mThreadID = 0;

    public Context mContext = null;

    /**
     * Instantiates a new nms chat settings.
     * 
     * @param the
     *            Context for android app
     * @param contactId
     *            the contact id in isms DB.
     */
    public NmsChatSettings(Context context, short contactId) {
        if (1 == NmsSMSMMSManager.getInstance(context).isExtentionFieldExsit()) {
            querySettingsFromSysDB(context, queryThreadIdByContactId(context, contactId));
        } else {
            querySettingFromiSMSDB(context, contactId);
        }
    }

    public NmsChatSettings(Context context, String address, short contactId) {
        if (1 == NmsSMSMMSManager.getInstance(context).isExtentionFieldExsit()) {
            if (address != null) {
                querySettingsFromSysDB(context, queryThreadIdByAddress(context, address, contactId));
            } else {
                querySettingsFromSysDB(context, queryThreadIdByContactId(context, contactId));
            }
        } else {
            querySettingFromiSMSDB(context, contactId);
        }
    }

    private void querySettingFromiSMSDB(Context context, short contactId) {
        NmsChatSettings settings = NmsDBUtils.getDataBaseInstance(context).nmsGetChatSettings(
                contactId);
        if (null == settings) {
            NmsLog.error(TAG, "can not get chat settings: " + contactId);
            return;
        }
        mContactId = contactId;
        mContext = context;

        mThreadID = 10000;
        mWallPaper = settings.mWallPaper;
        mNotification = settings.mNotification;
        mMute = settings.mMute;
        mMute_start = settings.mMute_start;
        mRingtone = settings.mRingtone;
        mVibrate = settings.mVibrate;
        NmsLog.trace(
                TAG,
                String.format(
                        "new NmsChatSettings ok, wallpaper:%s, mNotify: %d, mMute: %d, Ringtone: %s, vibrate: %d",
                        mWallPaper, mNotification, mMute, mRingtone, mVibrate));
    }

    private long queryThreadIdByContactId(Context context, short contactId) {
        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
        if (null == contact) {
            NmsLog.error(TAG,
                    "can not get contact via contact id at query settings from sys db function.");
            return 0;
        }

        mContactId = contactId;
        mContext = context;

        long threadId = NmsCreateSmsThread.getOrCreateThreadId(context, contact.getNumber());

        return threadId;
    }

    private long queryThreadIdByAddress(Context context, String address, short contactId) {
        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaEngineId(contactId);
        if (null == contact || address == null) {
            NmsLog.error(TAG,
                    "can not get contact via contact id at query settings from sys db function.");
            return 0;
        }

        // long threadId = NmsCreateSmsThread.getOrCreateThreadId(context,
        // contact.getNumber());

        mContactId = contactId;
        mContext = context;
        long threadId = NmsCreateSmsThread.getThreadIdByAddress(context, address);

        return threadId;
    }

    private void querySettingsFromSysDB(Context context, long threadId) {
        if (threadId <= 0) {
            return;
        }

        Uri uri = ContentUris.withAppendedId(NmsSMSMMS.THREAD_SETTINGS, threadId);
        Cursor cursor = NmsContentResolver.query(context.getContentResolver(), uri, null, null,
                null, null);
        if (null == cursor || !cursor.moveToFirst()) {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
            uri = ContentUris.withAppendedId(NmsSMSMMS.THREAD_SETTINGS, 0);
            cursor = NmsContentResolver.query(context.getContentResolver(), uri, null, null, null,
                    null);
            if (null == cursor || !cursor.moveToFirst()) {
                if (cursor != null) {
                    cursor.close();
                }
                return;
            }
        }
        getWallpaperFromMtk(context, uri);

        mThreadID = threadId;
        mNotification = cursor.getInt(cursor.getColumnIndex("notification_enable"));
        mMute = cursor.getInt(cursor.getColumnIndex("mute"));
        mMute_start = cursor.getInt(cursor.getColumnIndex("mute_start"));
        mRingtone = cursor.getString(cursor.getColumnIndex("ringtone"));
        mVibrate = cursor.getInt(cursor.getColumnIndex("vibrate"));
        cursor.close();
        NmsLog.trace(
                TAG,
                String.format(
                        "new NmsChatSettings from sysdb ok, wallpaper:%s, mNotify: %d, mMute: %d, Ringtone: %s, vibrate: %d",
                        mWallPaper, mNotification, mMute, mRingtone, mVibrate));

    }

    private void getWallpaperFromMtk(Context context, Uri uri) {

        InputStream is = null;
        OutputStream os = null;
        String wallpaperPath = NmsCommonUtils.getSDCardPath(context) + File.separator
                + NmsCustomUIConfig.ROOTDIRECTORY + File.separator + "wallpaper" + File.separator
                + "mtkWallpaper";

        File file = new File(wallpaperPath);
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdirs();
        }
        file.delete();
        try {
            if (!file.createNewFile()) {
                return;
            }
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            return;
        }
        try {
            is = mContext.getContentResolver().openInputStream(uri);
            os = new BufferedOutputStream(new FileOutputStream(file));
        } catch (FileNotFoundException e) {
            // NmsLog.nmsPrintStackTrace(e);
            try {
                uri = ContentUris.withAppendedId(NmsSMSMMS.THREAD_SETTINGS, 0);
                is = mContext.getContentResolver().openInputStream(uri);
                os = new BufferedOutputStream(new FileOutputStream(file));
            } catch (FileNotFoundException e1) {
                // NmsLog.nmsPrintStackTrace(e1);
                return;
            }
        }
        byte[] buffer = new byte[256];
        try {
            for (int len = 0; (len = is.read(buffer)) != -1;) {
                os.write(buffer, 0, len);
            }
            is.close();
            os.close();
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            return;
        }
        mWallPaper = wallpaperPath;
    }

    /**
     * Checks for this chat settings is customized.
     * 
     * @return true, if customized, else false.
     */
    public boolean hasSettings() {
        return mContactId > 0;
    }

    /**
     * Instantiates a new chat settings, and do nothing, only for settings DB
     * create instance, not for every one.
     */
    public NmsChatSettings() {
    }

    /**
     * Checks if is mute.
     * 
     * @return true, if is mute
     */
    public boolean isMute() {
        return mMute > 0;
    }

    /**
     * Checks if is notification on.
     * 
     * @return true, if is notification on
     */
    public boolean isNotificationOn() {
        return 1 == mNotification;
    }

    /**
     * get chat wall paper.
     * 
     * @return the wall paper uri, return null if user never change this chat
     *         settings.
     */
    public Uri nmsGetChatWallpaper() {

        Uri uri = null;

        if (!TextUtils.isEmpty(mWallPaper)) {
            uri = Uri.parse(mWallPaper);
        }

        return uri;
    }

    /**
     * Checks if is vibrate.
     * 
     * @return true, if is vibrate
     */
    public boolean isVibrate() {
        return 1 == mVibrate;
    }
}
