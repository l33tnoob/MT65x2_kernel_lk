package com.mediatek.security.datamanager;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.mediatek.common.mom.PermissionRecord;
import com.mediatek.security.service.PermControlUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class DatabaseHelper extends SQLiteOpenHelper {

    private static final String DB_NAME = "permission_control.db";
    private static final  int DB_VERSION = 1;
    //
    private static final String TABLE_PERMISSION_SETTINGS = "permission_settings";
    private static final String FIELD_ID = "_id";
    private static final String FIELD_NAME_PACKAGE = "packages_name";
    private static final String FIELD_NAME_PERMISSION = "permission_name";
    private static final String FIELD_NAME_PERMISSION_STATUS = "permissions_status";
    
    private HashMap<String, List<PermissionRecord>> mPkgKeyCache = new HashMap<String,List<PermissionRecord>>();
    private HashMap<String, List<PermissionRecord>> mPermKeyCache = new HashMap<String,List<PermissionRecord>>();
    private Context mContext;
    private SQLiteDatabase mDb;
    /**
     * The data base cache with package name as key
     * @return the cache map
     */
    public HashMap<String, List<PermissionRecord>> getmPkgKeyCache() {
        return mPkgKeyCache;
    }
    
    /**
     * The data base cache with permission name as key
     * @return the cache map
     */
    public HashMap<String, List<PermissionRecord>> getmPermKeyCache() {
        return mPermKeyCache;
    }
    
    /**
     * Construct of DatabaseHelper, this construct to init database and access
     * data base
     * @param context
     */
    public DatabaseHelper(Context context) {
        super(context, DB_NAME, null, DB_VERSION);
        mContext = context;
        mDb = getWritableDatabase();
    }

    /**
     * Get pkg data from database
     * @return HashMap<String, List<PermissionRecord>> with key as package name 
     */
    public HashMap<String, List<PermissionRecord>> getPkgFromDB() {
        Cursor cursor = null;
        HashMap<String, List<PermissionRecord>> dataMap = null;
        try {
            cursor = getCursor(TABLE_PERMISSION_SETTINGS);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    if (dataMap == null) {
                        dataMap = new HashMap<String, List<PermissionRecord>>();
                    }
                    loadDataIntoMap(cursor,dataMap);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return dataMap;
    }


    private void loadDataIntoMap(Cursor cursor,HashMap<String, List<PermissionRecord>> map) {
        List<PermissionRecord> permRecordListByPkg = null;
        
        //Get data from db by using cursor
        String packageName = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PACKAGE));
        String permName = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PERMISSION));
        // decrypt to get permission real name 
        permName = PermControlUtils.getPermissionName(Integer.valueOf(permName));
        String status = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PERMISSION_STATUS));
        
        //Put data into map
        if (map.containsKey(packageName)) {
            permRecordListByPkg = map.get(packageName);
            permRecordListByPkg.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
        } else {
            permRecordListByPkg = new ArrayList<PermissionRecord>();
            permRecordListByPkg.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
            map.put(packageName, permRecordListByPkg);
        }
    }
    
    public void initCacheData() {
        Cursor cursor = null;
        List<PermissionRecord> permRecordListByPkg = null;
        List<PermissionRecord> permRecordListByPerm = null;
        mPkgKeyCache.clear();
        mPermKeyCache.clear();
        try {
            cursor = getCursor(TABLE_PERMISSION_SETTINGS);
            if (cursor != null) {
                while (cursor.moveToNext()) {
                    String packageName = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PACKAGE));
                    String permName = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PERMISSION));
                    // decrypt to get permission real name 
                    permName = PermControlUtils.getPermissionName(Integer.valueOf(permName));
                    String status = cursor.getString(cursor.getColumnIndex(FIELD_NAME_PERMISSION_STATUS));
                    if (mPkgKeyCache.containsKey(packageName)) {
                        permRecordListByPkg = mPkgKeyCache.get(packageName);
                        permRecordListByPkg.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
                    } else {
                        permRecordListByPkg = new ArrayList<PermissionRecord>();
                        permRecordListByPkg.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
                        mPkgKeyCache.put(packageName, permRecordListByPkg);
                    }
                    if (mPermKeyCache.containsKey(permName)) {
                        permRecordListByPerm = mPermKeyCache.get(permName);
                        permRecordListByPerm.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
                    } else {
                        permRecordListByPerm = new ArrayList<PermissionRecord>();
                        permRecordListByPerm.add(new PermissionRecord(packageName,permName,Integer.valueOf(status)));
                        mPermKeyCache.put(permName, permRecordListByPerm);
                    }
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String sql = "CREATE TABLE IF NOT EXISTS " + TABLE_PERMISSION_SETTINGS + 
                " (" + FIELD_ID + " INTEGER primary key autoincrement," + 
                " " + FIELD_NAME_PACKAGE + " text," + 
                " " + FIELD_NAME_PERMISSION + " text," + 
                " " + FIELD_NAME_PERMISSION_STATUS + " text)";
        db.execSQL(sql);
    }
    
    /**
     * Because current there is no new add into db so do nothing
     */
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        
    }
    
    /**
     * Get the cursor of table
     * @param tableName name of table
     * @return cursor of the table
     */
    public Cursor getCursor(String tableName) {
        Cursor cursor = mDb.query(tableName, null, null, null, null, null, null);
        return cursor;
    }
    
    /**
     * add new data into database 
     * @param permRecord New PermissionRecord data
     * @return the inserted row new data inserted
     */
    public long add(PermissionRecord permRecord) {
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_PACKAGE, permRecord.mPackageName);
        //encrypt the permission name in code format
        String permCode = String.valueOf(PermControlUtils.getPermissionIndex(permRecord.mPermissionName));
        cv.put(FIELD_NAME_PERMISSION, permCode);
        cv.put(FIELD_NAME_PERMISSION_STATUS, String.valueOf(permRecord.getStatus()));
        long row = mDb.insert(TABLE_PERMISSION_SETTINGS, null, cv);
        return row;
    }
    
    
    /**
     * Delete the specific package from db
     * @param pkgName package name
     */
    public void delete(String pkgName) {
        String where = FIELD_NAME_PACKAGE + " = ?";
        String[] whereValue = new String[] {pkgName};
        mDb.delete(TABLE_PERMISSION_SETTINGS, where, whereValue);
    }
    
    /**
     * update the status of specific package with specific permission into db 
     * @param pkgName target package name
     * @param permName target permission names
     * @param status the permission status of package
     */
    public void updatePermStatus(String pkgName, String permName, int status) {
        String where = FIELD_NAME_PACKAGE + "= ?" + " AND " + FIELD_NAME_PERMISSION + "= ?";
        //encrypt the permission name in code format
        String permCode = String.valueOf(PermControlUtils.getPermissionIndex(permName));
        String[] whereValue = new String[]{pkgName, permCode};
        ContentValues cv = new ContentValues();
        cv.put(FIELD_NAME_PERMISSION_STATUS, String.valueOf(status));
        mDb.update(TABLE_PERMISSION_SETTINGS, cv, where, whereValue);
    }

    public void delete(List<String> removedList) {
        if (removedList != null) {
            for (String pkgName : removedList) {
                delete(pkgName);
            }
        }
    }

    public void add(HashMap<String, List<PermissionRecord>> newInstall) {
        if (newInstall != null) {
            for (String pkgName : newInstall.keySet()) {
                List<PermissionRecord> permRecordList = newInstall.get(pkgName);
                if (permRecordList != null) {
                    for (PermissionRecord record : permRecordList) {
                        add(record);
                    }
                }
            }
        }
    }
}
