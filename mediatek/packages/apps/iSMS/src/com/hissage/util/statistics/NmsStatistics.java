package com.hissage.util.statistics;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Vector;

import org.json.JSONException;
import org.json.JSONObject;

import android.R.bool;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.inputmethodservice.Keyboard.Key;
import android.os.IInterface;
import android.provider.SyncStateContract.Constants;
import android.text.TextUtils;
import android.text.style.BulletSpan;

import com.hissage.R.string;
import com.hissage.timer.NmsTimer;
import com.hissage.util.log.NmsLog;
import com.hissage.util.preference.NmsPreferences;

public final class NmsStatistics extends SQLiteOpenHelper {
    
    /* public key index */
    public static int KEY_EMO_ACTIVATE_PROMPT = 0 ;
    public static int KEY_EMO_ACTIVATE_TRY = 1;
    public static int KEY_EMO_ACTIVATE_OK = 2;
    
    public static int KEY_MEDIA_ACTIVATE_PROMPT = 3 ;
    public static int KEY_MEDIA_ACTIVATE_TRY = 4;
    public static int KEY_MEDIA_ACTIVATE_OK = 5;
    
    public static int KEY_SETTING_ACTIVATE_PROMPT = 6 ;
    public static int KEY_SETTING_ACTIVATE_TRY = 7;
    public static int KEY_SETTING_ACTIVATE_OK = 8;
    
    public static int KEY_DLG_ACTIVATE_PROMPT = 9 ;
    public static int KEY_DLG_ACTIVATE_TRY = 10;
    public static int KEY_DLG_ACTIVATE_OK = 11;
    
    public static int KEY_OTHER_ACTIVATE_PROMPT = 12;
    public static int KEY_OTHER_ACTIVATE_TRY = 13;
    public static int KEY_OTHER_ACTIVATE_OK = 14;
    
    public static int KEY_DISABLE_ISMS = 15;
    public static int KEY_ENABLE_ISMS = 16;
    
    public static int KEY_OPEN_SEND_BY_SMS = 17;
    public static int KEY_CLOSE_SEND_BY_SMS = 18;
    
//    public static int KEY_PROMPT_ACTIVATE_PROMPT = 19 ;
//    public static int KEY_PROMPT_ACTIVATE_TRY = 20;
//    public static int KEY_PROMPT_ACTIVATE_OK = 21;
//    
//    public static int KEY_MESSAGE_ACTIVATE_PROMPT = 22 ;
//    public static int KEY_MESSAGE_ACTIVATE_TRY = 23;
//    public static int KEY_MESSAGE_ACTIVATE_OK = 24;
    
    public static int tips_activate_prompt = 19;
    public static int tips_activate_try = 20;
    public static int tips_activate_ok = 21;

    public static int sms_activate_prompt = 22 ;
    public static int sms_activate_try = 23;
    public static int sms_activate_ok = 24;

    private static final String DB_NAME = "nms_statistics.db" ;
    private static final String DB_TABLE_NAME = "statistics" ;
    private static final int    DB_VERSION = 1 ;
    private static final int    DB_LAZY_UPDATE_COUNT = 20 ;
    private static final int    DB_LAZY_UPDATE_TIME  = 5 * 60 ;
    
    private static final String LOG_TAG = "statistics" ;
    
    private static final String COLUMN_ARRAY[] = {
        "emo_activate_prompt", "emo_activate_try", "emo_activate_ok",
        "media_activate_prompt", "media_activate_try", "media_activate_ok",
        "setting_activate_prompt", "setting_activate_try", "setting_activate_ok",
        "dialog_activate_prompt", "dialog_activate_try", "dialog_activate_ok",
        "other_activate_prompt", "other_activate_try", "other_activate_ok",
        "disable_isms_count", "enable_isms_count", "open_send_by_sms", "close_send_by_sms",
        "tips_activate_prompt", "tips_activate_try", "tips_activate_ok",
        "sms_activate_prompt", "sms_activate_try", "sms_activate_ok" ,
        //   "key_prompt_activate_prompt","key_prompt_activate_try","key_prompt_activate_ok",
        // "key_message_activate_prompt","key_message_activate_try","key_message_activate_ok"
    } ;
    

    private static NmsStatistics singleton = null ;
    private boolean isInitOK = true ;
    private Vector<String> lazyUpdates = new Vector<String>() ;
    
    
    private NmsStatistics(Context context) {
        super(context, DB_NAME, null, DB_VERSION) ;
    }
    
    @Override
    public void onCreate(SQLiteDatabase db) {
        StringBuilder builder = new StringBuilder() ;
        
        builder.append("id INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT NOT NULL COLLATE NOCASE") ;
        
        for (int i = 0; i < COLUMN_ARRAY.length; i++) {
            builder.append(String.format(",%s INTEGER NOT NULL DEFAULT 0", COLUMN_ARRAY[i])) ;
        }
        
        builder.append(",UNIQUE (date)") ;
        
        String sql = String.format("CREATE TABLE %s(%s)", DB_TABLE_NAME, builder.toString()) ;
        try {
            db.execSQL(sql);
        } catch (Exception e) {
            isInitOK = false ;
            NmsLog.error(LOG_TAG, "fatal error that create table got execption: " + e.toString()) ;
        }
        
        // db.close();   
        
        NmsLog.trace(LOG_TAG, "onCreate is call") ;
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        NmsLog.error(LOG_TAG, String.format("fatal error that no upgrade support yet, oldVersion: %d, newVersion: %d", oldVersion, newVersion)) ;
    }
    
    @Override
    protected void finalize() throws Throwable {
        updateToDB() ;
        super.finalize();
    }
    
    private synchronized void updateToDB() {
        
        try {
            if (lazyUpdates.isEmpty()) 
                return ;

            String curDate = getDbDateString() ;
            if (TextUtils.isEmpty(curDate)) {
                NmsLog.error(LOG_TAG, "error to get the current date string") ;
                return ;
            }
            
            SQLiteDatabase db = getWritableDatabase() ;
            
            db.execSQL(String.format("INSERT OR IGNORE INTO %s(date) VALUES('%s');", DB_TABLE_NAME, curDate)) ;
            
            for (String updateColumn : lazyUpdates) {
                db.execSQL(String.format("UPDATE OR IGNORE %s SET %s=(SELECT %s FROM %s WHERE date='%s')+1 WHERE date='%s';", 
                        DB_TABLE_NAME, updateColumn, updateColumn, DB_TABLE_NAME, curDate, curDate)) ;
            }
            
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
        
        lazyUpdates.clear() ;
    }
    
    private void doIncKeyVal(int keyIndex) {
        
        lazyUpdates.add(COLUMN_ARRAY[keyIndex]) ;
        
        if (lazyUpdates.size() >= DB_LAZY_UPDATE_COUNT) {
            NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_STATISTICS);
            new Thread(new Runnable() {
                public void run() {
                    updateToDB() ;
                }
            }).start() ;
            return ;
        }
        
        NmsTimer.NmsSetTimer(NmsTimer.NMS_TIMERID_STATISTICS, DB_LAZY_UPDATE_TIME);
    }
    
    private JSONObject doToJasonData() {
        
        NmsTimer.NmsKillTimer(NmsTimer.NMS_TIMERID_STATISTICS);
        updateToDB() ;
        
        JSONObject root = new JSONObject() ;
        
        StringBuilder builder = new StringBuilder() ;
        
        for (int i = 0; i < COLUMN_ARRAY.length; i++) {
            builder.append(String.format(",%s", COLUMN_ARRAY[i])) ;
        }
        
        String sql = String.format("SELECT date %s FROM %s ORDER BY date", builder.toString(), DB_TABLE_NAME) ;
        
        SQLiteDatabase db = getReadableDatabase();
        
        db.beginTransaction() ;
        Cursor cursor = db.rawQuery(sql, null);
        if (cursor == null) {
            NmsLog.error(LOG_TAG, "doToJasonData rawQuery got cursor is null") ;
            return null;
        }
        
        try {
            while (cursor.moveToNext()) {
                JSONObject dateObject = new JSONObject() ;
                
                for (int i = 0; i < COLUMN_ARRAY.length; i++) {
                    dateObject.put(COLUMN_ARRAY[i], cursor.getInt(i + 1)) ; /* + 1 because the date was the column 0*/
                }
                
                root.put(cursor.getString(0), dateObject) ;
            }
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
            root = null ;
        }
        
        cursor.close();
        db.endTransaction() ;
        return root;
    }
    
    private synchronized void doClear() {
        getWritableDatabase().execSQL(String.format("DELETE FROM %s", DB_TABLE_NAME)) ;
    }
    
    private static String getDbDateString() {
        return new SimpleDateFormat("yyyy-MM-dd").format(new Date());
    }
    
    private static boolean isInited(String logStr) {
        if (singleton == null) {
            NmsLog.error(LOG_TAG, "error that the singleton is NOT init yet for " + logStr) ;
            return false ;
        }
        
        if (!singleton.isInitOK) {
            NmsLog.error(LOG_TAG, "error that the singleton is init ERROR for " + logStr) ;
            return false ;
        }
        
        return true ;
    }
    
    /* I insert all the public functions code into a try-catch block
     * Yeah, it may violate some JAVA criterion, and it's ugly in someone's option.
     * but it's safe, for an statistics module should never make the host app crash, even it's NOT work.
     * */
    public static synchronized void init(Context context) {
        try {
            if (singleton != null) {
                NmsLog.error(LOG_TAG, "error that the singleton is init yet") ;
                return ;
            }
            
            singleton = new NmsStatistics(context) ;
            NmsLog.trace(LOG_TAG, "init is call") ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
    
    public static void incKeyVal(int keyIndex) {
        
        try {
            if (!isInited("incKeyVal"))
                return ;
            
            if (keyIndex < 0 || keyIndex >= COLUMN_ARRAY.length) {
                NmsLog.error(LOG_TAG, String.format("fatal error to inc keyIndex: %d is invalid", keyIndex) ) ;
                return ;
            }
            
            singleton.doIncKeyVal(keyIndex) ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
    
    
    public static JSONObject toJasonData() {
        try {
            if (!isInited("toJasonData"))
                return null;
            
            return singleton.doToJasonData() ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
        
        return null ;
    }
    
    public static void clear() {
        
        try {
            if (!isInited("clear data"))
                return ;
            singleton.doClear() ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
    
    static public void handleTimerEvent() {
        try {
            if (!isInited("time out"))
                return ;
            singleton.updateToDB() ;
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e) ;
        }
    }
}
