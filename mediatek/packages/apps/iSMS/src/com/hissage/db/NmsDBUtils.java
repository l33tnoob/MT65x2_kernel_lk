package com.hissage.db;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.text.TextUtils;
import android.util.Log;

import com.hissage.config.NmsChatSettings;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.service.NmsService;
import com.hissage.struct.SNmsInviteInfo;
import com.hissage.util.log.NmsLog;

public class NmsDBUtils {
    private static final String TAG = "NmsDBUtils";
    private static SQLiteDatabase mDataBase = null;
    private static NmsDBUtils mInstance = null;

    private NmsDBUtils(Context c) {
        if (mDataBase == null || !mDataBase.isOpen()) {
            NmsDBOpenHelper sqliteopenhelper = new NmsDBOpenHelper(c);
            mDataBase = sqliteopenhelper.getWritableDatabase();
        }
    }

    public static NmsDBUtils getDataBaseInstance(Context c) {
        if (null == mInstance) {
            mInstance = new NmsDBUtils(c);
        }
        return mInstance;
    }

    public NmsChatSettings nmsGetChatSettings(short contactId) {
        if (null == mDataBase) {
            NmsLog.error(TAG, "nmsGetChatSettings database is null.");
            return null;
        }

        Cursor cursor = mDataBase.rawQuery(String.format("SELECT * FROM %s WHERE %s = ?",
                NmsDBOpenHelper.CHAT_SETTINGS_TABLE, NmsDBOpenHelper.CONTACT_ID), new String[] { ""
                + contactId });
        if (null == cursor) {
            NmsLog.error(TAG,
                    "get chat settings from db error, may be contact first time, contact id:"
                            + contactId);
            return null;
        }
        if (!cursor.moveToFirst()) {
            NmsLog.warn(TAG, "get chat settings cursor move to first error:" + contactId);
            cursor.close();
            return null;
        }

        NmsChatSettings settings = null;

        do {
            short id = (short) cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.CONTACT_ID));
            if (id == contactId) {
                settings = new NmsChatSettings();
                settings.mContactId = (short) cursor.getInt(cursor
                        .getColumnIndex(NmsDBOpenHelper.CONTACT_ID));
                settings.mWallPaper = cursor.getString(cursor
                        .getColumnIndex(NmsDBOpenHelper.WALLPAPER));
                settings.mNotification = cursor.getInt(cursor
                        .getColumnIndex(NmsDBOpenHelper.NOTIFICATION));
                settings.mMute = cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.MUTE));
                settings.mMute_start = cursor.getInt(cursor
                        .getColumnIndex(NmsDBOpenHelper.MUTE_START));
                settings.mRingtone = cursor.getString(cursor
                        .getColumnIndex(NmsDBOpenHelper.RINGTONE));
                settings.mVibrate = cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.VIBRATE));
            }
        } while (cursor.moveToNext());

        cursor.close();

        return settings;
    }

    public int nmsSetChatSettings(NmsChatSettings settings) {
        if (null == mDataBase) {
            NmsLog.error(TAG, "nmsSetChatSettings database is null.");
            return 0;
        }
        if (null == settings || settings.mContactId <= 0) {
            NmsLog.error(TAG, "nmsSetChatSettings param error: " + settings);
            return 0;
        }

        ContentValues values = new ContentValues();

        if (null == nmsGetChatSettings(settings.mContactId)) {
            values.put(NmsDBOpenHelper.CONTACT_ID, settings.mContactId);
            values.put(NmsDBOpenHelper.WALLPAPER, settings.mWallPaper);
            values.put(NmsDBOpenHelper.NOTIFICATION, settings.mNotification);
            values.put(NmsDBOpenHelper.MUTE, settings.mMute);
            values.put(NmsDBOpenHelper.MUTE_START, settings.mMute_start);
            values.put(NmsDBOpenHelper.RINGTONE, settings.mRingtone);
            values.put(NmsDBOpenHelper.VIBRATE, settings.mVibrate);

            return (int) mDataBase.insert(NmsDBOpenHelper.CHAT_SETTINGS_TABLE, null, values);
        } else {
            values.put(NmsDBOpenHelper.WALLPAPER, settings.mWallPaper);
            values.put(NmsDBOpenHelper.NOTIFICATION, settings.mNotification);
            values.put(NmsDBOpenHelper.MUTE, settings.mMute);
            values.put(NmsDBOpenHelper.MUTE_START, settings.mMute_start);
            values.put(NmsDBOpenHelper.RINGTONE, settings.mRingtone);
            values.put(NmsDBOpenHelper.VIBRATE, settings.mVibrate);
            return (int) mDataBase.update(NmsDBOpenHelper.CHAT_SETTINGS_TABLE, values,
                    NmsDBOpenHelper.CONTACT_ID + " = ?", new String[] { "" + settings.mContactId });
        }
    }

    public SNmsInviteInfo nmsGetInviteInfo(short contactId) {
        if (null == mDataBase) {
            NmsLog.error(TAG, "nmsGetInviteInfo database is null.");
            return null;
        }
        if (contactId < 0) {
            NmsLog.error(TAG, "nmsGetIniviteInfo param error: " + contactId);
            return null;
        }
        SNmsInviteInfo info = null;
            Cursor cursor = mDataBase.rawQuery(String.format("SELECT * FROM %s WHERE %s = ?",
                    NmsDBOpenHelper.INVITE_TABLE, NmsDBOpenHelper.RECORDID), new String[] { ""
                    + contactId });
            if (null == cursor) {
                NmsLog.error(TAG, "no this person info in invite db, may be contact first time");
                return null;
            }
            if (!cursor.moveToFirst()) {
                NmsLog.warn(TAG, "nmsGetInviteInfo cursor move to first error.");
                cursor.close();
                return null;
            }
            info = new SNmsInviteInfo();
            short id = (short) cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.RECORDID));
            if (id == contactId) {
                info.recdId = id;
                info.contact_count = cursor
                        .getInt(cursor.getColumnIndex(NmsDBOpenHelper.CONTACT_COUNT));
                info.count_today = (short) cursor.getInt(cursor
                        .getColumnIndex(NmsDBOpenHelper.COUNT_TODAY));
                info.last_contact = cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.LAST_CONTACT));
                info.later_time = cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.LATER_TIME));
            }
            cursor.close();
        return info;
    }

    public int nmsAddInviteRecd(SNmsInviteInfo info) {
        
        if (null == mDataBase) {
            NmsLog.error(TAG, "nmsAddInviteRecd database is null.");
            return 0;
        }

        if (null == info || info.recdId < 0) {
            NmsLog.error(TAG, "nmsAddInviteRecd param error: " + info);
            return 0;
        }
        ContentValues values = new ContentValues();
      
        SNmsInviteInfo infoDB = nmsGetInviteInfo(info.recdId) ;
        
        if (null == infoDB ) {
            values.put(NmsDBOpenHelper.RECORDID, info.recdId);
            values.put(NmsDBOpenHelper.CONTACT_COUNT, info.contact_count);
            values.put(NmsDBOpenHelper.COUNT_TODAY, info.count_today);
            values.put(NmsDBOpenHelper.LAST_CONTACT, info.last_contact);
            values.put(NmsDBOpenHelper.LATER_TIME, info.later_time);
            return (int) mDataBase.insert(NmsDBOpenHelper.INVITE_TABLE, null, values);
        } else {

            values.put(NmsDBOpenHelper.CONTACT_COUNT, info.contact_count);
            values.put(NmsDBOpenHelper.COUNT_TODAY, info.count_today);
            values.put(NmsDBOpenHelper.LAST_CONTACT, info.last_contact);
            values.put(NmsDBOpenHelper.LATER_TIME, info.later_time);
            return mDataBase.update(NmsDBOpenHelper.INVITE_TABLE, values, NmsDBOpenHelper.RECORDID
                        + " = ?", new String[] { "" + info.recdId });
        }
    }

    public void nmsResetInviteRecd(short contactId) {

        SNmsInviteInfo info = new SNmsInviteInfo();
        info.recdId = contactId;
        ContentValues values = new ContentValues();
        values.put(NmsDBOpenHelper.CONTACT_COUNT, info.contact_count);
        values.put(NmsDBOpenHelper.COUNT_TODAY, info.count_today);
        values.put(NmsDBOpenHelper.LAST_CONTACT, info.last_contact);
        values.put(NmsDBOpenHelper.LATER_TIME, info.later_time);
        mDataBase.update(NmsDBOpenHelper.INVITE_TABLE, values, NmsDBOpenHelper.RECORDID + " = ?",
                new String[] { "" + info.recdId });
    }

    public void nmsDeleteInviteRecd(short contactId) {
        mDataBase.delete(NmsDBOpenHelper.INVITE_TABLE, NmsDBOpenHelper.RECORDID + " = ?",
                new String[] { "" + contactId });
    }

    private int nmsAddImsi(String imsi) {
        if (TextUtils.isEmpty(imsi)) {
            NmsLog.error(TAG, "nmsAddImsi param error: " + imsi);
            return -1;
        }
        int newSimId = 0;
        ContentValues values = new ContentValues();
        values.put(NmsDBOpenHelper.IMSI, imsi);

        int index = (int) mDataBase.insert(NmsDBOpenHelper.SIM_ID_TABLE, null, values);
        NmsLog.trace(TAG, "add new imsi to db, _id is: " + index);
        newSimId = (int) NmsPlatformAdapter.getInstance(NmsService.getInstance()).getSimIdByImsi(
                imsi, index);
        NmsLog.trace(TAG, "get sim id with FW: " + newSimId);
        values.put(NmsDBOpenHelper.SIM_ID, newSimId);
        mDataBase.update(NmsDBOpenHelper.SIM_ID_TABLE, values, NmsDBOpenHelper.ID + " = ?",
                new String[] { "" + index });
        return newSimId;

    }

    public String nmsGetImsi(int id) {
        if (id <= 0) {
            NmsLog.error(TAG, "nmsGetImsi param error: " + id);
            return null;
        }
        Cursor cursor = mDataBase.rawQuery(String.format("SELECT * FROM %s WHERE %s = ?",
                NmsDBOpenHelper.SIM_ID_TABLE, NmsDBOpenHelper.SIM_ID), new String[] { "" + id });
        if (null == cursor) {
            NmsLog.error(TAG, "no data in db about this sim id: " + id);
            return null;
        }
        if (!cursor.moveToFirst()) {
            cursor.close();
            NmsLog.error(TAG, "nmsGetImsi cursor move to first error.");
            return null;
        }

        String ret = cursor.getString(cursor.getColumnIndex(NmsDBOpenHelper.IMSI));
        cursor.close();
        return ret;
    }

    public int nmsGetSimId(String imsi) {
        if (TextUtils.isEmpty(imsi)) {
            NmsLog.error(TAG, "nmsGetSimId param error: " + imsi);
            return -1;
        }

        Cursor cursor = mDataBase.rawQuery(String.format("SELECT * FROM %s WHERE %s LIKE ?",
                NmsDBOpenHelper.SIM_ID_TABLE, NmsDBOpenHelper.IMSI), new String[] { imsi });
        if (null == cursor || !cursor.moveToFirst()) {
            NmsLog.error(TAG, "can not get this Imsi's id, so add it as new sim card");
            if (cursor != null) {
                cursor.close();
            }
            return nmsAddImsi(imsi);
        }

        int ret = -1;

        do {
            String str = cursor.getString(cursor.getColumnIndex(NmsDBOpenHelper.IMSI));
            if (imsi.equals(str)) {
                ret = cursor.getInt(cursor.getColumnIndex(NmsDBOpenHelper.SIM_ID));
            }
        } while (cursor.moveToNext());
        cursor.close();
        return ret;
    }

}
