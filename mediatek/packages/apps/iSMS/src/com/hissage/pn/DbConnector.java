package com.hissage.pn;

import java.util.Vector;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

public class DbConnector {
    private SQLiteDatabase db;

    private DatabaseHelper mDatabaseHelper;

    private static Vector<DbConnector> connects = new Vector<DbConnector>();

    private DbConnector(Context context) {
        mDatabaseHelper = new DatabaseHelper(context);
        db = mDatabaseHelper.getWritableDatabase();
        // Log.i(hpnsActivity.LogTag, "db init");
    }

    public synchronized static DbConnector getConnection(Context context) {
        // Log.i(hpnsActivity.LogTag, "db connections:" + connects.size());
        for (DbConnector connect : connects)
            if (!connect.db.isOpen()) {
                connect.db = connect.mDatabaseHelper.getWritableDatabase();
                return connect;
            } else {
                return connect;
            }

        DbConnector connect = new DbConnector(context);
        connects.add(connect);
        return connect;
    }

    // private Context mContext = null;

    public synchronized void close() {
        if (mDatabaseHelper != null)
            mDatabaseHelper.close();
    }

    public synchronized long insertLog(int testId, String msg) {
        ContentValues cv = new ContentValues();
        cv.put(DatabaseHelper.KEY_MSGID, testId);
        cv.put(DatabaseHelper.KEY_MSG, msg);
        return db.insert(DatabaseHelper.table_name, null, cv);
    }

    public synchronized Cursor queueAll() {
        String[] columns = new String[] { DatabaseHelper.KEY_ID, DatabaseHelper.KEY_MSG };
        Cursor cursor = db.query(DatabaseHelper.table_name, columns, null, null, null, null, null);

        return cursor;
    }

    public synchronized void deleteAll() {
        if (mDatabaseHelper != null) {
            mDatabaseHelper.onUpgrade(db, 0, 0);
        }
    }

}
