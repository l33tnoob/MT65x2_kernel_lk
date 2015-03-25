package com.hissage.db;

import com.hissage.util.log.NmsLog;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;

public class NmsContentResolver {

    public static Cursor query(ContentResolver cr, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        Cursor cursor = null;
        try {
            if (null == cr) {
                return null;
            }

            cursor = cr.query(uri, projection, selection, selectionArgs, sortOrder);
            return cursor;
        } catch (Exception e) {
            if(cursor != null ){
                cursor.close();
            }
            //NmsLog.nmsAssertException(e);
            return null;
        }
    }
    
    public static int delete(ContentResolver cr, Uri uri, String where, String[] selectionArgs) {
        try{
            if (null == cr) {
                return 0;
            }
            return cr.delete(uri, where, selectionArgs);
        }catch(Exception e){
            NmsLog.nmsAssertException(e);
            return 0;
        }
    }
    
    public static Uri insert(ContentResolver cr, Uri uri, ContentValues values){
        try {
            if (null == cr) {
                return null;
            }
            return cr.insert(uri, values);
        } catch (Exception e) {
            NmsLog.nmsAssertException(e);
            return null;
        }
    }
    
    public static int update(ContentResolver cr, Uri uri, ContentValues values, String where, String[] selectionArgs){
        try{
            if (null == cr) {
                return 0;
            }
            return cr.update(uri, values, where, selectionArgs);
        }catch(Exception e){
            NmsLog.nmsAssertException(e);
            return 0;
        }
    }

}
