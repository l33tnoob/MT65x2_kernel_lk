package com.orangelabs.rcs.provider.ipcall;

import com.orangelabs.rcs.provider.RichProviderHelper;
import com.orangelabs.rcs.provider.sharing.RichCallData;

import android.content.ContentProvider;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.text.TextUtils;

/**
 * IP call history provider
 * 
 * @author owom5460
 */
public class IPCallProvider extends ContentProvider {
	// Database table
	public static final String TABLE = "ipcall";
		
	// Create the constants used to differentiate between the different
	// URI requests
	private static final int IPCALL = 1;
	private static final int IPCALL_ID = 2;
		
	// Allocate the UriMatcher object, where a URI ending in 'ipcall'
	// will correspond to a request for all ipcall, and 'ipcall'
	// with a trailing '/[rowID]' will represent a single ipcall row.
	private static final UriMatcher uriMatcher;
	static {
		uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		uriMatcher.addURI("com.orangelabs.rcs.ipcall", "ipcall", IPCALL);
		uriMatcher.addURI("com.orangelabs.rcs.ipcall", "ipcall/#", IPCALL_ID);
	}
			
	/**
	 * Database helper class
	 */
	private SQLiteOpenHelper openHelper;	
	 

	@Override
	public boolean onCreate() {
		if(RichProviderHelper.getInstance()==null){
        	RichProviderHelper.createInstance(this.getContext());
        }
        this.openHelper = RichProviderHelper.getInstance();
        return true;
	}
	

	@Override
	public String getType(Uri uri) {
		switch(uriMatcher.match(uri)){
			case IPCALL:
				return "vnd.android.cursor.dir/com.orangelabs.rcs.ipcall";
			case IPCALL_ID:
				return "vnd.android.cursor.item/com.orangelabs.rcs.ipcall";
			default:
				throw new IllegalArgumentException("Unsupported URI " + uri);
		}
	}
	
	
	@Override
	public Cursor query(Uri uri, String[] projectionIn, String where,
			String[] whereArgs, String sort) {
		SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch(match) {
            case IPCALL:
                break;
            case IPCALL_ID:
                // TODO with IPCallData 
            	//qb.appendWhere(RichCallData.KEY_ID + "=" + uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = openHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, where, whereArgs, null, null, sort);

		// Register the contexts ContentResolver to be notified if the cursor result set changes.
        if (c != null) {
        	c.setNotificationUri(getContext().getContentResolver(), IPCallData.CONTENT_URI);
        }

        return c;
	}

	
	@Override
	public int update(Uri uri, ContentValues values, String where,
			String[] whereArgs) {
		int count;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
	        case IPCALL:
	            count = db.update(TABLE, values, where, null);
	            break;
            case IPCALL_ID:
                String segment = uri.getPathSegments().get(1);
                int id = Integer.parseInt(segment);
                count = db.update(TABLE, values, RichCallData.KEY_ID + "=" + id, null);
                break;
            default:
                throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
	}
	

	@Override
	public Uri insert(Uri uri, ContentValues initialValues) {
		SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
	        case IPCALL:
	        case IPCALL_ID:
	            // Insert the new row, will return the row number if successful
	        	// Use system clock to generate id : it should not be a common int otherwise it could be the 
	        	// same as an id present in MmsSms table (and that will create uniqueness problem when doing the tables merge) 
	        	int id = (int)System.currentTimeMillis();
	        	if (Integer.signum(id) == -1){
	        		// If generated id is <0, it is problematic for uris
	        		id = -id;
	        	}
	        	initialValues.put(RichCallData.KEY_ID, id);
	    		long rowId = db.insert(TABLE, null, initialValues);
	    		uri = ContentUris.withAppendedId(RichCallData.CONTENT_URI, rowId);
	        	break;
	        default:
	    		throw new SQLException("Failed to insert row into " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        return uri;
	}

	
	@Override
	public int delete(Uri uri, String where, String[] whereArgs) {
		SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
	        case IPCALL:
	        	count = db.delete(TABLE, where, whereArgs);
	        	break;
	        case IPCALL_ID:
	        	String segment = uri.getPathSegments().get(1);
				count = db.delete(TABLE, RichCallData.KEY_ID + "="
						+ segment
						+ (!TextUtils.isEmpty(where) ? " AND ("	+ where + ')' : ""),
						whereArgs);
				
				break;
	        	
	        default:
	    		throw new SQLException("Failed to delete row " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        return count;    
	}




}
