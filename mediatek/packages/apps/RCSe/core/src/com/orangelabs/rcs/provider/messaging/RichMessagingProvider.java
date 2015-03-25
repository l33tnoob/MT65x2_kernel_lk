/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

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
import android.text.TextUtils;

/**
 * Rich messaging history provider
 * 
 * @author mhsm6403
 */
public class RichMessagingProvider extends ContentProvider {
	// Database table
	public static final String TABLE = "messaging";
	
	// Create the constants used to differentiate between the different
	// URI requests
	private static final int MESSAGING = 1;
	private static final int MESSAGING_ID = 2;
	private static final int MESSAGING_SESSION = 3;
	private static final int MESSAGING_TYPE_DISCRIMINATOR = 4;
	
	/**
     * M: Added to display chat list history
     * @{
     */	
	private static final int MESSAGING_RECENT = 5; // FOR RECENT O2O CHATS
	private static final int MESSAGING_GROUP_RECENT = 6; // FOR RECENT GROUP CHATS
	private static final int MESSAGING_GROUP_RECENT_CHATS = 7; // FOR RECENT MESSAGES FOR A GROUP
	private static final int MESSAGING_ALIAS_NAME = 8; // FOR ALIAS NAME
	/**
	 * @}
	 */
	
	// Allocate the UriMatcher object, where a URI ending in 'contacts'
	// will correspond to a request for all contacts, and 'contacts'
	// with a trailing '/[rowID]' will represent a single contact row.
	private static final UriMatcher uriMatcher;


	static {
		uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging", MESSAGING);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/#", MESSAGING_ID);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/session", MESSAGING_SESSION);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/type_discriminator/#", MESSAGING_TYPE_DISCRIMINATOR);
		
		/**
	     * M: Added to display chat list history
	     * @{
	     */	
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/recent_messages", MESSAGING_RECENT);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/recent_group_messages", MESSAGING_GROUP_RECENT);
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/recent_chats_for_group/*", MESSAGING_GROUP_RECENT_CHATS);
		/**
		 * @}
		 */
		
		/**
		 * M: Added to get alias name 
		 * */ 
		uriMatcher.addURI("com.orangelabs.rcs.messaging", "messaging/alias_name/*", MESSAGING_ALIAS_NAME);
		
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
			case MESSAGING:
				return "vnd.android.cursor.dir/com.orangelabs.rcs.messaging";
			case MESSAGING_ID:
				return "vnd.android.cursor.item/com.orangelabs.rcs.messaging";
			default:
				throw new IllegalArgumentException("Unsupported URI " + uri);
		}
	}
	
    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        SQLiteDatabase db = openHelper.getReadableDatabase();
    	Cursor raw_cursor = null;
    	Cursor cursor = null;
    	boolean rawCursorFlag = false; //enable if a query is raw query
        // Generate the body of the query
        String groupBy=null;
        int match = uriMatcher.match(uri);
        switch(match) {
            case MESSAGING:
                break;
            case MESSAGING_ID:
                qb.appendWhere(RichMessagingData.KEY_ID + "=" + uri.getPathSegments().get(1));
                break;
            case MESSAGING_SESSION:
            	groupBy=RichMessagingData.KEY_CHAT_SESSION_ID;
            	sort=RichMessagingData.KEY_TIMESTAMP+ " ASC";
            	break;
            case MESSAGING_TYPE_DISCRIMINATOR:
            	qb.appendWhere(RichMessagingData.KEY_TYPE+"=");
            	qb.appendWhere(uri.getPathSegments().get(2));
            	break;
            	
            	/**
        	     * M: Added to display chat list history
        	     * @{
        	     */	
            case MESSAGING_RECENT:
            	String recentMessagingQuery = getRecentMessageQuery();
            	try{
            	    rawCursorFlag = true;
            		raw_cursor = db.rawQuery(recentMessagingQuery,null);
            	}catch(Exception err){
            	    if(raw_cursor!=null){
                        raw_cursor.close();
                    }
            	}
            	break;
            case MESSAGING_GROUP_RECENT:
            	String  recentGroupMessagingQuery  =  getRecentGroupMessageQuery();
            	try{
            	    rawCursorFlag = true;
            		raw_cursor = db.rawQuery(recentGroupMessagingQuery,null);
            	}catch(Exception err){
            	    if(raw_cursor!=null){
                        raw_cursor.close();
                    }
            	}
            	break;
            case MESSAGING_GROUP_RECENT_CHATS: 	//get recent chats for a group
            	
                rawCursorFlag = true;
                String groupChatID = uri.getPathSegments().get(2);
                groupChatID = Uri.decode(groupChatID);
            	String  recentChatsForGroupQuery  =  recentChatsForGroupQuery(groupChatID,sort);
            	try{
            		raw_cursor = db.rawQuery(recentChatsForGroupQuery,null);
            	}catch(Exception err){
            	     if(raw_cursor!=null){
            	         raw_cursor.close();
            	     }
            	}
            	break;
            	
            case MESSAGING_ALIAS_NAME:	
            	 rawCursorFlag = true;
                 String contactNumber = uri.getPathSegments().get(2);
                 contactNumber = Uri.decode(contactNumber);
             	String  aliasNameQuery  =  getAliasNameQuery(contactNumber,sort);
             	try{
             		raw_cursor = db.rawQuery(aliasNameQuery,null);
             	}catch(Exception err){
             	     if(raw_cursor!=null){
             	         raw_cursor.close();
             	     }
             	}
             	break;
            	/**
        		 * @}
        		 */
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        /**
	     * M: modified to display chat list history
	     * @{
	     */	
        //if the query was not raw cursor
        if(!rawCursorFlag){
            //SQLiteDatabase db = openHelper.getReadableDatabase();
            cursor = qb.query(db, projectionIn, selection, selectionArgs, groupBy, null, sort);

		// Register the contexts ContentResolver to be notified if
		// the cursor result set changes.
            cursor.setNotificationUri(getContext().getContentResolver(), uri);
        // Also notify changes to the Event log provider
        getContext().getContentResolver().notifyChange(EventLogData.CONTENT_URI, null);
            return cursor;
        }
        else{
            return raw_cursor;
        }
        
        /**
		 * @}
		 */
    }
    
    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
	        case MESSAGING:
	            count = db.update(TABLE, values, where, whereArgs);
	            break;
            case MESSAGING_ID:
                String segment = uri.getPathSegments().get(1);
                int id = Integer.parseInt(segment);
                count = db.update(TABLE, values, RichMessagingData.KEY_ID + "=" + id, null);
                break;
            default:
                throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        // Also notify changes to the Event log provider
        getContext().getContentResolver().notifyChange(EventLogData.CONTENT_URI, null);
        return count;
    }
    
    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        switch(uriMatcher.match(uri)){
	        case MESSAGING:
	        case MESSAGING_ID:
	            // Insert the new row, will return the row number if successful
	        	// Use system clock to generate id : it should not be a common int otherwise it could be the 
	        	// same as an id present in MmsSms table (and that will create uniqueness problem when doing the tables merge) 
	        	/**
	             * M: Commented to make "Id" column auto increment and not dependedent on current system time
	             * 
	             * @{
	             */	
	        	/*
	        	int id = (int)System.currentTimeMillis();
	        	if (Integer.signum(id) == -1){
	        		// If generated id is <0, it is problematic for uris
	        		id = -id;
	        	}
	        	initialValues.put(RichMessagingData.KEY_ID, id);
	        	*/
	        	/**
	        	 * @}
	        	 */
	        	initialValues.put(RichMessagingData.KEY_SIZE,0);
	    		long rowId = db.insert(TABLE, null, initialValues);
	    		uri = ContentUris.withAppendedId(RichMessagingData.CONTENT_URI, rowId);
	        	break;
	        default:
	    		throw new SQLException("Failed to insert row into " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        // Also notify changes to the Event log provider
		getContext().getContentResolver().notifyChange(EventLogData.CONTENT_URI, null);
		return uri;
    }
    
    /**
     * This method should not be used if deletion isn't made on the whole messages of a contact.
     * Prefer methods from RichMessaging class, otherwise Recycler wont work.
     *  
     * If all messages of a contact, or all rich messages are to be deleted, this method could be used.
     */
    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        int count = 0;
        switch(uriMatcher.match(uri)){
	        case MESSAGING:
	        	count = db.delete(TABLE, where, whereArgs);
	        	break;
	        case MESSAGING_ID:
	        	String segment = uri.getPathSegments().get(1);
				count = db.delete(TABLE, RichMessagingData.KEY_ID + "="
						+ segment
						+ (!TextUtils.isEmpty(where) ? " AND ("	+ where + ')' : ""),
						whereArgs);
				
				break;
	        	
	        default:
	    		throw new SQLException("Failed to delete row " + uri);
        }
		getContext().getContentResolver().notifyChange(uri, null);
        // Also notify changes to the Event log provider
        getContext().getContentResolver().notifyChange(EventLogData.CONTENT_URI, null);
        return count;    
   }	
    
    
    /**
     * M: Added to display chat list history
     * @{
     */	
    //GET THE QUERY FOR FETCHING RECENT MESSAGES
    private String getRecentMessageQuery(){
/*    
    	String query = "SELECT t1.contact,t1._id, t1._date " +
    				" FROM messaging AS t1 "+
    					" WHERE "+ 
    					"( SELECT COUNT(*)  as count FROM messaging AS t2 WHERE  t2.contact = t1.contact AND t2._id > t1._id) = 0 "+
    					" and (type = 0 or type =1) and is_spam = 0 order by _id desc limit 10";
*/
String query = "select contact, _id, _date from messaging " +
    	"where (type = 0 or type = 1 or type = 6 or type = 7) " 
    	+" group by contact "
    	+" order by _id  desc limit 10 ";

    	return query;
    }
    
    // Get the query for recent group messages
    private String getRecentGroupMessageQuery() {
      
/*
        String query = "select t1.contact, max(t2._id) as _id, max(t2._date) as _date from "
                + "( "
                + "select contact, _date , max(chat_session_id) as chat_session_id  from  "
                + " messaging  "
                + " where  "
                + "(type = 3 or type = 4 or type = 5) "
                + "and  "
                + " (status = 14 or status = 15) "
                + "group by contact "
                + "order by max(chat_session_id) desc "
                + ") t1 "
                + "inner join messaging as t2 "
                + "where t2.chat_session_id = t1.chat_session_id   "
                + "and (t2.type = 3 or t2.type = 4) "
                + "group by t1.chat_session_id "
                + "order by t2._id desc limit 10";
*/

String query = " select t1.contact, t1.chat_id, t2._id from "
        	+ " ( select chat_id, _id , contact from messaging where " 
        	+ " ( type = 3 or type = 4 or type = 5)"
        	+ " and (status = 14 or status = 15) " 
        	+ " group by chat_id order by _id desc ) t1 " 
        	+ " join " 
        	+ " ( select chat_id , max(_id) as _id from messaging where ( type = 3 or type = 4 ) "
        	+ " group by chat_id " 
        	+ " order by _id desc ) t2 " 
        	+ " on t1.chat_id = t2.chat_id " 
        	+ " group by t1.chat_id " 
        	+ " order by t2._id desc limit 10 ";
        return query;


    }
        
    /* M:
     * query to fetch grp chat mesages for a contact grp 
    */
    private String recentChatsForGroupQuery(String groupChatID, String sortOrder){
	
    	/*
    	String query = "select _id from messaging" 
    		+" where chat_session_id in "
    		+"( "
    		+"select chat_session_id as recent_sessions  from "
    		+"messaging  "
    		+"where  "
    		+" (type = 3 or type = 4 or type = 5) "
    		+" and contact = '"+groupAppendedContacts+"'"
    		+") "
    		+"and _data <> '' "
    		+"and (type = 3 or type = 4 ) "
    		+"order by _id desc ";
    	*/
    	String query = "select _id from messaging " 
                       +" where chat_id = '"+groupChatID+"'"
                       +" and (type = 3 or type = 4 ) "
                       +" order by _id desc ";
    	query += sortOrder;
    	return query;
    }
    
    private String getAliasNameQuery(String contactNO, String sortOrder){  
    	String query = "select name from messaging " 
                +" WHERE ((contact='"+contactNO+"')" 
                +" and (type=2) AND (status=14)) "
                +" group by contact "
                +" order by _id desc ";
	return query;
    }
    
    /**
	 * @}
	 */
}
