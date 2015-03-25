package com.mediatek.regionalphone;

import java.io.FileNotFoundException;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.widget.Toast;
import com.mediatek.xlog.Xlog;

public class RegionalPhoneProvider extends ContentProvider {

    private static final String TAG = Common.LOG_TAG;

    private static UriMatcher matcher = new UriMatcher(UriMatcher.NO_MATCH);

    private static final int MMS_SMS_TABLE = 1;
    private static final int APN_TABLE = 2;
    private static final int SETTINGS_TABLE = 3;
    private static final int BROWSER_TABLE = 4;
    private static final int WALLPAPER_TABLE = 5;
    private static final int SEARCHENGINE_TABLE = 6;
    private static final int BROWSER_TABLE_ROW = 7;

    private DatabaseOpenHelper mDbOpenHelper;

    static {
        matcher.addURI(RegionalPhone.AUTHORITY, "mms_sms", RegionalPhoneProvider.MMS_SMS_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "apn", RegionalPhoneProvider.APN_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "settings", RegionalPhoneProvider.SETTINGS_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "browser", RegionalPhoneProvider.BROWSER_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "wallpaper", RegionalPhoneProvider.WALLPAPER_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "searchengine", RegionalPhoneProvider.SEARCHENGINE_TABLE);
        matcher.addURI(RegionalPhone.AUTHORITY, "browser/#", RegionalPhoneProvider.BROWSER_TABLE_ROW);
    }

    @Override
    public int delete(Uri uri, String selection, String[] selectionArgs) {
        return 0;
    }

    @Override
    public String getType(Uri uri) {
        switch (matcher.match(uri)) {
        case MMS_SMS_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.mms";
        case APN_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.apn";
        case SETTINGS_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.settings";
        case BROWSER_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.browser";
        case WALLPAPER_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.wallpaper";
        case SEARCHENGINE_TABLE:
            return "vnd.android.cursor.dir/com.mtk.regionalphonemanager.searchengine";
        case BROWSER_TABLE_ROW:
            return "vnd.android.cursor.item/com.mtk.regionalphonemanager.browser";
        default:
            throw new IllegalArgumentException("getType() unknow uri:" + uri);
        }
    }

    @Override
    public Uri insert(Uri uri, ContentValues value) {
        SQLiteDatabase db = this.mDbOpenHelper.getWritableDatabase();
        String tableName = uri.getLastPathSegment();
        long rowid = db.insertOrThrow(tableName, null, value);
        if (rowid > 0) {
            long id = value.getAsLong(RegionalPhone.BROWSER._ID);
            Uri newUri = ContentUris.withAppendedId(uri, id);
            Xlog.d(TAG, "after insert uri: " + newUri);
            this.getContext().getContentResolver().notifyChange(newUri, null);
            return newUri;
        }
        return null;
    }

    @Override
    public boolean onCreate() {
        Xlog.d(TAG, "RegionalPhoneProvider::create");
        this.mDbOpenHelper = new DatabaseOpenHelper(this.getContext());
        return true;
    }

    @Override
    public Cursor query(Uri uri, String[] projecion, String selection, String[] selectionArgs,
            String sortOrder) {
        SQLiteDatabase db = this.mDbOpenHelper.getReadableDatabase();
        if (!mDbOpenHelper.isDatabaseExit) {
            return null;
        }
        switch (matcher.match(uri)) {
        case MMS_SMS_TABLE:
            return db.query(RegionalPhone.TABLE_MMS_SMS, projecion, selection, selectionArgs, null,
                    null, sortOrder);
        case APN_TABLE:
            return db.query(RegionalPhone.TABLE_APN, projecion, selection, selectionArgs, null,
                    null, sortOrder);
        case SETTINGS_TABLE:
            return db.query(RegionalPhone.TABLE_SETTINGS, projecion, selection, selectionArgs,
                    null, null, sortOrder);
        case BROWSER_TABLE:
            return db.query(RegionalPhone.TABLE_BROWSER, projecion, selection, selectionArgs, null,
                    null, sortOrder);
        case WALLPAPER_TABLE:
            return db.query(RegionalPhone.TABLE_WALLPAPER, projecion, selection, selectionArgs,
                    null, null, sortOrder);
        case SEARCHENGINE_TABLE:
            return db.query(RegionalPhone.TABLE_SEARCHENGINE, projecion, selection, selectionArgs,
                    null, null, sortOrder);
        case BROWSER_TABLE_ROW:
            long id = ContentUris.parseId(uri);
            String condition = RegionalPhone.BROWSER._ID + "=" + id;
            if (selection != null && !"".equals(selection)) {
                selection += " AND " + condition;
            } else {
                selection = condition;
            }
            return db.query(RegionalPhone.TABLE_BROWSER, projecion, selection, selectionArgs, null,
                        null, sortOrder);
        default:
            throw new IllegalArgumentException("query() unkown uri" + uri);
        }
    }

    @Override
    public int update(Uri uri, ContentValues value, String selection, String[] selectionArgs) {
        SQLiteDatabase db = this.mDbOpenHelper.getReadableDatabase();
        if (!mDbOpenHelper.isDatabaseExit) {
            return 0;
        }
        int num = 0;
        switch (matcher.match(uri)) {
        case MMS_SMS_TABLE:
            num = db.update(RegionalPhone.TABLE_MMS_SMS, value, selection, selectionArgs);
            break;
        case APN_TABLE:
            num = db.update(RegionalPhone.TABLE_APN, value, selection, selectionArgs);
            break;
        case SETTINGS_TABLE:
            num = db.update(RegionalPhone.TABLE_SETTINGS, value, selection, selectionArgs);
            break;
        case BROWSER_TABLE:
            num = db.update(RegionalPhone.TABLE_BROWSER, value, selection, selectionArgs);
            break;
        case WALLPAPER_TABLE:
            num = db.update(RegionalPhone.TABLE_WALLPAPER, value, selection, selectionArgs);
            break;
        case SEARCHENGINE_TABLE:
            num = db.update(RegionalPhone.TABLE_SEARCHENGINE, value, selection, selectionArgs);
            break;
        default:
            throw new IllegalArgumentException("update() unkown uri" + uri);
        }
        if (num != 0) {
            getContext().getContentResolver().notifyChange(uri, null);
        }
        return num;
    }
}
