package com.orangelabs.rcs.provider.messaging;

import com.orangelabs.rcs.provider.RichProviderHelper;
import com.orangelabs.rcs.provider.eventlogs.EventLogData;

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

public class IntegratedMessageMappingProvider extends ContentProvider {

	
	private static final UriMatcher uriMatcher;

	public static final String TABLE = RichMessagingData.TABLE_MESSAGE_INTEGRATED;
	private static final int INTEGRATED_MESSAGING = 1;
	
	
    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;
    
	static {
		uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		uriMatcher.addURI("com.orangelabs.rcs.messaging.integrated", "messaging", INTEGRATED_MESSAGING);
	}
	
	@Override
	public int delete(Uri uri, String where, String[] whereArgs) {
		SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
	        case INTEGRATED_MESSAGING:
	        	count = db.delete(TABLE, where, whereArgs);
	        	break;
        }
        return count; 
	}

	
	
	@Override
	public String getType(Uri arg0) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	  public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
		// TODO Auto-generated method stub
		   Cursor cursor = null;
		   SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
	       qb.setTables(TABLE);
	        SQLiteDatabase db = openHelper.getReadableDatabase();
	    	Cursor raw_cursor = null;
	    	boolean rawCursorFlag = false; //enable if a query is raw query
	        // Generate the body of the query
	        String groupBy=null;
	        int match = uriMatcher.match(uri);
	        switch(match) {
	            case INTEGRATED_MESSAGING:
	               //SQLiteDatabase db = openHelper.getReadableDatabase();
	                cursor = qb.query(db, projectionIn, selection, selectionArgs, groupBy, null, sort);
	                break;
	        }
            return cursor;
	}
	
	
	 @Override
	    public Uri insert(Uri uri, ContentValues initialValues) {
	        SQLiteDatabase db = openHelper.getWritableDatabase();
	        switch(uriMatcher.match(uri)){
		        case INTEGRATED_MESSAGING:
		            // Insert the new row, will return the row number if successful
		        	// Use system clock to generate id : it should not be a common int otherwise it could be the 
		        	// same as an id present in MmsSms table (and that will create uniqueness problem when doing the tables merge) 
		    		long rowId = db.insert(TABLE, null, initialValues);
		    		uri = ContentUris.withAppendedId(RichMessagingData.CONTENT_URI_INTEGRATED, rowId);
		        	break;
		        default:
		    		throw new SQLException("Failed to insert row into " + uri);
	        }
			return uri;
	    }

	@Override
	public boolean onCreate() {
		// TODO Auto-generated method stub
		
		if(RichProviderHelper.getInstance()==null){
        	RichProviderHelper.createInstance(this.getContext());
        }
        this.openHelper = RichProviderHelper.getInstance();
        return true;

	}



	@Override
	public int update(Uri uri, ContentValues values, String selection,
			String[] selectionArgs) {
		// TODO Auto-generated method stub
		return 0;
	}

}
