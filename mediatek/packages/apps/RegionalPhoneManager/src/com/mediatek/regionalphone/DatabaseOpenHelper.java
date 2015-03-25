package com.mediatek.regionalphone;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import com.mediatek.xlog.Xlog;

public class DatabaseOpenHelper extends SQLiteOpenHelper {

    private static final String TAG = Common.LOG_TAG;

    // not consider multi user now
    private static final String DB_PATH = "/data/data/com.mediatek.regionalphone/databases/";
    private static final String DB_SYSTEM_PATH = "/system/etc/regionalphone/";
    private static final String DB_CUSTOMER_PATH = "/custom/etc/regionalphone/";
    private static final String DB_NAME = "regionalphone.db";
    private final Context mContext;
    private static final int VERSION = 1;
    private static final int LIMITTABLENUM = 1;
    public boolean isDatabaseExit = true;

    public DatabaseOpenHelper(Context context) {
        super(context, DB_NAME, null, VERSION);
        this.mContext = context;
        initDatabase();
    }

    private void initDatabase() {
        Xlog.d(TAG, "initDatabase");
        try {
            copyDataBase();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * Creates a empty database on the system and rewrites it with your own database.
     * */
    private void copyDataBase() throws IOException {
        Xlog.d(TAG, "copyDataBase");
        boolean dbExist = checkDataBase();
        if (dbExist) {
            // do nothing - database already exist
        } else {
            // By calling this method and empty database will be created into the default system
            // path of your application so we are gonna be able to overwrite that database with our
            // database.
            this.getReadableDatabase();

            try {
                copyDBFile();
            } catch (IOException e) {
                Xlog.d(TAG, e.getMessage());
                throw new Error("Error copying database");
            }
        }
    }

    /**
     * Check if the database already exist to avoid re-copying the file each time you open the
     * application.
     * 
     * @return true if it exists, false if it doesn't
     */
    private boolean checkDataBase() {
        Xlog.d(TAG, "checkDataBase in");
        boolean flag = false;
        String myPath = DB_PATH + DB_NAME;

        if (new File(myPath).exists()) {
            // String s = "select count(*) as count from sqlite_master where type=\"table\"";
            SQLiteDatabase sq = this.getReadableDatabase();
            int tableNum = 0;
            Cursor cr = sq.query("sqlite_master", new String[] { "count(*) as count" }, "type=?",
                    new String[] { "table" }, null, null, null);
            if (cr != null && cr.getCount() > 0) {
                cr.moveToFirst();
                tableNum = cr.getInt(cr.getColumnIndex("count"));
                Xlog.d(TAG, "checkDataBase tableNum = " + tableNum);
            }

            if (tableNum > LIMITTABLENUM) {
                flag = true;
                Xlog.d(TAG, "checkDataBase exists, flag=" + flag);
            }

            if (cr != null) {
                cr.close();
            }
        }

        Xlog.d(TAG, "checkDataBase, flag=" + flag);
        return flag;
    }

    /**
     * Copies your database from system/etc or customer/etc to the just created empty database in the
     * system folder, from where it can be accessed and handled. This is done by transfering
     * bytestream.
     * */
    private void copyDBFile() throws IOException {
        Xlog.d(TAG, "copyDBFile in");
        // Open your local db as the input stream
        // InputStream myInput = mContext.getResources().openRawResource(R.raw.regionalphone);
        InputStream myInput = null;
        OutputStream myOutput = null;
        try {

            if (new File(DB_CUSTOMER_PATH + DB_NAME).exists()) {
                Xlog.d(TAG, DB_CUSTOMER_PATH + DB_NAME + " exist");
                myInput = new FileInputStream(DB_CUSTOMER_PATH + DB_NAME);
            } else if (new File(DB_SYSTEM_PATH + DB_NAME).exists()) {
                Xlog.d(TAG, DB_SYSTEM_PATH + DB_NAME + " exist");
                myInput = new FileInputStream(DB_SYSTEM_PATH + DB_NAME);
            } else {
                Xlog.d(TAG, "There is no database exist");
                isDatabaseExit = false;
                return;
            }

            myOutput = new FileOutputStream(DB_PATH + DB_NAME);
            // transfer bytes from the inputfile to the outputfile
            byte[] buffer = new byte[1024];
            int length;
            while ((length = myInput.read(buffer)) > 0) {
                myOutput.write(buffer, 0, length);
            }
            myOutput.flush();
        } catch (FileNotFoundException e) {
            Xlog.i(TAG, "File not found");
        } finally {
            // Close the streams
            if (myInput != null) {
                myInput.close();
            }
            if (myOutput != null) {
                myOutput.close();
            }
        }
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        Xlog.d(TAG, "onCreate in");
    }

    @Override
    public void onUpgrade(SQLiteDatabase arg0, int arg1, int arg2) {
        Xlog.d(TAG, "onUpgrade in");
    }

}
