package com.hissage.db;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class NmsDBOpenHelper extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "nmsDB.db";
    private static final int DATABASE_VERSION = 1;
    public static final String ID = "_id";
    public static final String DEFAULT_SORT_ORDER = "id DESC";

    public static final String CHAT_SETTINGS_TABLE = "chat_settings_tab";
    public static final String CONTACT_ID = "contactId";
    public static final String WALLPAPER = "wallpaper";
    public static final String NOTIFICATION = "notification";
    public static final String MUTE = "mute";
    public static final String MUTE_START = "mute_start";
    public static final String RINGTONE = "ringtone";
    public static final String VIBRATE = "vibrate";

    public static final String INVITE_TABLE = "invite_info_tab";
    public static final String RECORDID = "recdId";
    public static final String LAST_CONTACT = "last_contact";
    public static final String CONTACT_COUNT = "contact_count";
    public static final String COUNT_TODAY = "count_today";
    public static final String LATER_TIME = "later_time";

    public static final String SIM_ID_TABLE = "sim_id_tab";
    public static final String IMSI = "imsi";
    public static final String SIM_ID = "sim_id";

    public NmsDBOpenHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase sqlitedatabase) {
        sqlitedatabase
                .execSQL("CREATE TABLE chat_settings_tab (_id INTEGER PRIMARY KEY AUTOINCREMENT,contactId INTEGER,wallpaper NTEXT,notification INTEGER,mute INTEGER, mute_start INTERGER,ringtone NTEXT,vibrate INTEGER)");

        sqlitedatabase
                .execSQL("CREATE TABLE invite_info_tab (_id INTEGER PRIMARY KEY AUTOINCREMENT,recdId INTEGER,last_contact INTEGER,contact_count INTEGER,count_today INTEGER,later_time INTEGER)");

        sqlitedatabase
                .execSQL("CREATE TABLE sim_id_tab (_id INTEGER PRIMARY KEY AUTOINCREMENT,sim_id INTEGER, imsi NTEXT)");

    }

    @Override
    public void onUpgrade(SQLiteDatabase sqlitedatabase, int oldVersion, int newVersion) {
        String s = (new StringBuilder("Upgrading database from version ")).append(oldVersion)
                .append(" to ").append(newVersion).append(", which will destroy all old data")
                .toString();
        Log.w("new Nms DB", s);
        sqlitedatabase.execSQL("DROP TABLE IF EXISTS chat_settings");
        sqlitedatabase.execSQL("DROP TABLE IF EXISTS invite_info");
        sqlitedatabase.execSQL("DROP TABLE IF EXISTS sim_id");
        onCreate(sqlitedatabase);

    }

}
