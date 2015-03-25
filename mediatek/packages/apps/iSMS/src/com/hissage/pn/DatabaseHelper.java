package com.hissage.pn;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

public class DatabaseHelper extends SQLiteOpenHelper {

    public static final String database_name = "push.db";
    public static final String table_name = "demo";
    private static final int database_version = 1;
    public static final String KEY_ID = "_id";
    public static final String KEY_MSGID = "msgid";
    public static final String KEY_MSG = "msg";

    DatabaseHelper(Context context) {
        super(context, database_name, null, database_version);
        // Log.i(hpnsActivity.LogTag, "database helper init");
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("create table " + table_name + "(" + KEY_ID
                + " integer primary key autoincrement, " + KEY_MSGID + " integer, " + KEY_MSG
                + " text)");
        // Log.i(hpnsActivity.LogTag, "table:" + table_name + "created");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("drop table if exists " + table_name);
        // Log.i(hpnsActivity.LogTag, "table:" + table_name + " dropped");
        onCreate(db);
    }
}
