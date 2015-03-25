package com.hissage.util.statistics;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Vector;

import org.json.JSONObject;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteOpenHelper;
import android.text.TextUtils;

import com.hissage.timer.NmsTimer;
import com.hissage.util.log.NmsLog;

public final class NmsPrivateStatistics extends SQLiteOpenHelper {
    private static final String DB_NAME = "nms_private_statistics.db";
    private static final String DB_TABLE_NAME = "privateStatistics";
    private static final int DB_VERSION = 1;
    private int privateClickTime = 0;
    private int privateOpenFlag = 0;
    private int privateContacts = 0;
    private static final String LOG_TAG = "NmsPrivatestatistics";
    private static NmsPrivateStatistics singleton = null;
    private static final String COLUMN_ARRAY[] = { "private_click_time", "private_open_flag",
            "private_contacts", };
    private boolean isInitOK = true;

    public NmsPrivateStatistics(Context context) {
        super(context, DB_NAME, null, DB_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        // TODO Auto-generated method stub
        StringBuilder builder = new StringBuilder();

        builder.append("id INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT NOT NULL COLLATE NOCASE");

        for (int i = 0; i < COLUMN_ARRAY.length; i++) {
            builder.append(String.format(",%s INTEGER NOT NULL DEFAULT 0", COLUMN_ARRAY[i]));
        }
        builder.append(",UNIQUE (date)");

        String sql = String.format("CREATE TABLE %s(%s)", DB_TABLE_NAME, builder.toString());
        try {
            db.execSQL(sql);
        } catch (Exception e) {
            isInitOK = false;
            NmsLog.error(LOG_TAG, "fatal error that create table got execption: " + e.toString());
        }

        // db.close();

        NmsLog.trace(LOG_TAG, "onCreate is call");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // TODO Auto-generated method stub
        NmsLog.error(LOG_TAG, String.format(
                "fatal error that no upgrade support yet, oldVersion: %d, newVersion: %d",
                oldVersion, newVersion));
    }

    @Override
    protected void finalize() throws Throwable {
        // updateToDB() ;
        super.finalize();
    }

    private synchronized void updateToDB(boolean updateClick, boolean updateOpenFlag,
            boolean updateContacts) {
        try {
            String curDate = getDbDateString();
            if (TextUtils.isEmpty(curDate)) {
                NmsLog.error(LOG_TAG, "error to get the current date string");
                return;
            }

            SQLiteDatabase db = getWritableDatabase();
            db.execSQL(String.format("INSERT OR IGNORE INTO %s(date) VALUES('%s');", DB_TABLE_NAME,
                    curDate));

            if (updateClick) {
                db.execSQL(String
                        .format("UPDATE OR IGNORE %s SET %s=(SELECT %s FROM %s WHERE date='%s')+ %s WHERE date='%s';",
                                DB_TABLE_NAME, "private_click_time", "private_click_time",
                                DB_TABLE_NAME, curDate, privateClickTime, curDate));
                privateClickTime = 0;
            }
            if (updateOpenFlag) {
                db.execSQL(String.format("UPDATE OR IGNORE %s SET %s = %s WHERE date='%s';",
                        DB_TABLE_NAME, "private_open_flag", privateOpenFlag, curDate));
                privateOpenFlag = 0;
            }
            if (updateContacts) {
                db.execSQL(String.format("UPDATE OR IGNORE %s SET %s = %s WHERE date='%s';",
                        DB_TABLE_NAME, "private_contacts", privateContacts, curDate));
                privateContacts = 0;
            }
            getPrivateData();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private String doGetPrivateData() {

        StringBuilder builder = new StringBuilder();
        String mDataToUpgrage = null;
        String currentToUpgrade = null;
        for (int i = 0; i < COLUMN_ARRAY.length; i++) {
            builder.append(String.format(",%s", COLUMN_ARRAY[i]));
        }

        String sql = String.format("SELECT date %s FROM %s ORDER BY date", builder.toString(),
                DB_TABLE_NAME);

        SQLiteDatabase db = getReadableDatabase();
        db.beginTransaction();
        Cursor cursor = db.rawQuery(sql, null);
        if (cursor == null) {
            NmsLog.error(LOG_TAG, "doToJasonData rawQuery got cursor is null");
            return null;
        }

        try {
            String curDate = getDbDateString();
            while (cursor.moveToNext()) {
                if (!curDate.equalsIgnoreCase(cursor.getString(0))) {
                    currentToUpgrade = cursor.getString(0) + "_" + cursor.getString(1) + "_"
                            + cursor.getString(2) + "_" + cursor.getString(3);
                    if (mDataToUpgrage != null) {
                        mDataToUpgrage = mDataToUpgrage + ";" + currentToUpgrade;
                    } else {
                        mDataToUpgrage = currentToUpgrade;
                    }
                }
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        cursor.close();
        db.endTransaction();
        return mDataToUpgrage;
    }

    private void doUpdatePrivateClickTimeStatistics(int clickTime) {
        privateClickTime += clickTime;
        new Thread(new Runnable() {
            public void run() {
                updateToDB(true, false, false);
            }
        }).start();
        return;
    }

    private void doUpdatePrivateOpenFlagStatistics(int openFlag) {
        privateOpenFlag += openFlag;
        new Thread(new Runnable() {
            public void run() {
                updateToDB(false, true, false);
            }
        }).start();
        return;
    }

    private void doUpdatePrivateContactsStatistics(int contacts) {
        privateContacts = contacts;
        new Thread(new Runnable() {
            public void run() {
                updateToDB(false, false, true);
            }
        }).start();
        return;
    }

    public static void updatePrivateClickTimeStatistics(int clickTime) {
        try {
            if (!isInited("PrivateMsg"))
                return;
            singleton.doUpdatePrivateClickTimeStatistics(clickTime);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public static void updatePrivateOpenFlagStatistics(int openFlag) {
        try {
            if (!isInited("PrivateMsg"))
                return;
            singleton.doUpdatePrivateOpenFlagStatistics(openFlag);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public static void updatePrivateContactsStatistics(int contacts) {
        try {
            if (!isInited("PrivateMsg"))
                return;
            singleton.doUpdatePrivateContactsStatistics(contacts);
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    private synchronized void doClear() {
        String curDate = getDbDateString();
        getWritableDatabase().execSQL(
                String.format("DELETE FROM %s WHERE date!='%s'", DB_TABLE_NAME, curDate));
    }

    private static String getDbDateString() {
        return new SimpleDateFormat("yyyy-MM-dd").format(new Date());
    }

    private static boolean isInited(String logStr) {
        if (singleton == null) {
            NmsLog.error(LOG_TAG, "error that the singleton is NOT init yet for " + logStr);
            return false;
        }

        if (!singleton.isInitOK) {
            NmsLog.error(LOG_TAG, "error that the singleton is init ERROR for " + logStr);
            return false;
        }

        return true;
    }

    public static String getPrivateData() {
        try {
            if (!isInited("getPrivateData"))
                return null;

            return singleton.doGetPrivateData();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return null;
    }

    public static void clear() {

        try {
            if (!isInited("clear data"))
                return;
            singleton.doClear();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }

    public static synchronized void init(Context context) {
        try {
            if (singleton != null) {
                NmsLog.error(LOG_TAG, "error that the singleton is init yet");
                return;
            }

            singleton = new NmsPrivateStatistics(context);
            NmsLog.trace(LOG_TAG, "init is call");
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
        }
    }
}
