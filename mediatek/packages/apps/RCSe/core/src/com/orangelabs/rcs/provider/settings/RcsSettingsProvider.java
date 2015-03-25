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

package com.orangelabs.rcs.provider.settings;

import java.util.ArrayList;

import javax.sip.ListeningPoint;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Environment;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * RCS settings provider
 *
 * @author jexa7410
 */
public class RcsSettingsProvider extends ContentProvider {


   /**
     * Modified to achieve the auto configuration feature. @{
     */
    /**
     * Database table
     */
    public static final String FIRST_USER_ACCOUNT_TABLE = "settings";
    public static final String SECOND_USER_ACCOUNT_TABLE = "settings_2nd";
    public static final String THIRD_USER_ACCOUNT_TABLE = "settings_3rd";
    
	/**
	 * Database table
	 */
    private static final String TABLE = "settings";

	// Create the constants used to differentiate between the different URI requests
	private static final int SETTINGS = 1;
    private static final int SETTINGS_ID = 2;

	// Allocate the UriMatcher object, where a URI ending in 'settings'
	// will correspond to a request for all settings, and 'settings'
	// with a trailing '/[rowID]' will represent a single settings row.
    private static final UriMatcher uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);
    static {
        uriMatcher.addURI("com.orangelabs.rcs.settings", FIRST_USER_ACCOUNT_TABLE, SETTINGS);
        uriMatcher.addURI("com.orangelabs.rcs.settings", "FIRST_USER_ACCOUNT_TABLE/#", SETTINGS_ID);
        uriMatcher.addURI("com.orangelabs.rcs.settings", SECOND_USER_ACCOUNT_TABLE, SETTINGS);
        uriMatcher.addURI("com.orangelabs.rcs.settings", "SECOND_USER_ACCOUNT_TABLE/#", SETTINGS_ID);
        uriMatcher.addURI("com.orangelabs.rcs.settings", THIRD_USER_ACCOUNT_TABLE, SETTINGS);
        uriMatcher.addURI("com.orangelabs.rcs.settings", "THIRD_USER_ACCOUNT_TABLE/#", SETTINGS_ID);
    }

    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;
    
    /**
     * Database name
     */
    public static final String DATABASE_NAME = "rcs_settings.db";

    /**
     * Helper class for opening, creating and managing database version control
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 84;

        private Context ctx;

        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);

            this.ctx = ctx;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            /**
             * M: Modified to achieve the auto configuration feature. @{
             */
        	db.execSQL("CREATE TABLE " + FIRST_USER_ACCOUNT_TABLE + " ("
        			+ RcsSettingsData.KEY_ID + " integer primary key autoincrement,"
                    + RcsSettingsData.KEY_KEY + " TEXT,"
                    + RcsSettingsData.KEY_VALUE + " TEXT);");
        	initialTable(db,FIRST_USER_ACCOUNT_TABLE);
        	db.execSQL("CREATE TABLE " + SECOND_USER_ACCOUNT_TABLE + " ("
                    + RcsSettingsData.KEY_ID + " integer primary key autoincrement,"
                    + RcsSettingsData.KEY_KEY + " TEXT,"
                    + RcsSettingsData.KEY_VALUE + " TEXT);");
        	initialTable(db,SECOND_USER_ACCOUNT_TABLE);
        	db.execSQL("CREATE TABLE " + THIRD_USER_ACCOUNT_TABLE + " ("
        			+ RcsSettingsData.KEY_ID + " integer primary key autoincrement,"
                    + RcsSettingsData.KEY_KEY + " TEXT,"
                    + RcsSettingsData.KEY_VALUE + " TEXT);");
        	initialTable(db,THIRD_USER_ACCOUNT_TABLE);
        	/**
             * @}
             */
        }

            // Insert default values for parameters
/**
         * M: Modified to achieve the auto configuration feature. @{
         */
        private void initialTable(SQLiteDatabase db,String table){
            addParameter(db, table, RcsSettingsData.SERVICE_ACTIVATED, 				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.PRESENCE_INVITATION_RINGTONE, 		"");
            addParameter(db, table, RcsSettingsData.PRESENCE_INVITATION_VIBRATE, 		RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CSH_INVITATION_RINGTONE, 			"");
            addParameter(db, table, RcsSettingsData.CSH_INVITATION_VIBRATE, 			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CSH_AVAILABLE_BEEP, 				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.FILETRANSFER_INVITATION_RINGTONE, 	"");
            addParameter(db, table, RcsSettingsData.FILETRANSFER_INVITATION_VIBRATE, 	RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CHAT_INVITATION_RINGTONE, 			"");
            addParameter(db, table, RcsSettingsData.CHAT_INVITATION_VIBRATE, 			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CHAT_DISPLAYED_NOTIFICATION,       RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.FREETEXT1, 						ctx.getString(R.string.rcs_settings_label_default_freetext_1));
            addParameter(db, table, RcsSettingsData.FREETEXT2, 						ctx.getString(R.string.rcs_settings_label_default_freetext_2));
            addParameter(db, table, RcsSettingsData.FREETEXT3,							ctx.getString(R.string.rcs_settings_label_default_freetext_3));
            addParameter(db, table, RcsSettingsData.FREETEXT4,							ctx.getString(R.string.rcs_settings_label_default_freetext_4));
            addParameter(db, table, RcsSettingsData.MIN_BATTERY_LEVEL,                 "0");
            addParameter(db, table, RcsSettingsData.MAX_PHOTO_ICON_SIZE, 				"256");
            addParameter(db, table, RcsSettingsData.MAX_FREETXT_LENGTH, 				"100");
            addParameter(db, table, RcsSettingsData.MAX_GEOLOC_LABEL_LENGTH,			"100");
            addParameter(db, table, RcsSettingsData.GEOLOC_EXPIRATION_TIME,			"3600");
            addParameter(db, table, RcsSettingsData.MIN_STORAGE_CAPACITY,				"10240");
            addParameter(db, table, RcsSettingsData.MAX_CHAT_PARTICIPANTS, 			"10");
            addParameter(db, table, RcsSettingsData.MAX_CHAT_MSG_LENGTH, 				"100");
            addParameter(db, table, RcsSettingsData.MAX_GROUPCHAT_MSG_LENGTH,			"100");
            addParameter(db, table, RcsSettingsData.CHAT_IDLE_DURATION, 				"300");
            addParameter(db, table, RcsSettingsData.JOYN_DISABLE_STATUS, 				"0");
            addParameter(db, table, RcsSettingsData.MAX_FILE_TRANSFER_SIZE, 			"3072");
            addParameter(db, table, RcsSettingsData.WARN_FILE_TRANSFER_SIZE, 			"2048");
            addParameter(db, table, RcsSettingsData.MAX_IMAGE_SHARE_SIZE, 				"3072");
            addParameter(db, table, RcsSettingsData.MAX_VIDEO_SHARE_DURATION, 			"54000");
            addParameter(db, table, RcsSettingsData.MAX_CHAT_SESSIONS, 				"20");
            addParameter(db, table, RcsSettingsData.MAX_FILE_TRANSFER_SESSIONS, 		"1");
            addParameter(db, table, RcsSettingsData.MAX_IP_CALL_SESSIONS,				"1");
            addParameter(db, table, RcsSettingsData.SMS_FALLBACK_SERVICE, 				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.WARN_SF_SERVICE,	 				RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.AUTO_ACCEPT_CHAT,			 		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,             RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER,			RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IM_SESSION_START,	 				"1");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_USERNAME, 			"");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, 		"");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_HOME_DOMAIN, 		"");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID, 		"");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_PASSWORD, 			"");
            addParameter(db, table, RcsSettingsData.USERPROFILE_IMS_REALM,		 		"");
		    addParameter(db, table, RcsSettingsData.IMS_PROXY_ADDR_MOBILE,				"");
		    addParameter(db, table, RcsSettingsData.IMS_PROXY_PORT_MOBILE,				"5060");
		    addParameter(db, table, RcsSettingsData.IMS_PROXY_ADDR_WIFI,				"");
		    addParameter(db, table, RcsSettingsData.IMS_PROXY_PORT_WIFI,				"5060");
		    addParameter(db, table, RcsSettingsData.XDM_SERVER, 						"");
		    addParameter(db, table, RcsSettingsData.XDM_LOGIN,							"");
		    addParameter(db, table, RcsSettingsData.XDM_PASSWORD, 						"");
		    addParameter(db, table, RcsSettingsData.FT_HTTP_SERVER, 					"https://80.58.30.68:8447");
		    addParameter(db, table, RcsSettingsData.FT_HTTP_LOGIN,						"670999999@TMELAB");
		    addParameter(db, table, RcsSettingsData.FT_HTTP_PASSWORD, 					"670999999");
            addParameter(db, table, RcsSettingsData.FT_PROTOCOL,                   	RcsSettingsData.FT_PROTOCOL_MSRP);
            addParameter(db, table, RcsSettingsData.IM_CONF_URI, 						RcsSettingsData.DEFAULT_GROUP_CHAT_URI);
            addParameter(db, table, RcsSettingsData.ENDUSER_CONFIRMATION_URI,			"");
            addParameter(db, table, RcsSettingsData.COUNTRY_CODE,						"+33");
            addParameter(db, table, RcsSettingsData.COUNTRY_AREA_CODE,					"0");
            addParameter(db, table, RcsSettingsData.MSISDN,							"");
            addParameter(db, table, RcsSettingsData.CAPABILITY_CS_VIDEO, 				RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_IMAGE_SHARING,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_VIDEO_SHARING,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_IP_VOICE_CALL,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_IP_VIDEO_CALL,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_IM_SESSION,				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_IM_GROUP_SESSION,		RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_FILE_TRANSFER,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP,		RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE,		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,		RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL,RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_GROUP_CHAT_SF,			RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF,		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_RCS_EXTENSIONS,			"");
            addParameter(db, table, RcsSettingsData.IMS_SERVICE_POLLING_PERIOD, 		"300");
            addParameter(db, table, RcsSettingsData.SIP_DEFAULT_PORT, 					"5060");
            addParameter(db, table, RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE,   ListeningPoint.UDP);
            /**M : added new columns not prestn in stack */
            addParameter(db, table, RcsSettingsData.SECURE_MSRP_OVER_MOBILE,   RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.SECURE_RTP_OVER_MOBILE,   RcsSettingsData.FALSE);
            /**@*/

	    /**M : TEMP CHANGE changed from TCP to UDP to allow conection via WIFI */
            addParameter(db, table, RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI,     ListeningPoint.UDP);
            /**@*/
            addParameter(db, table, RcsSettingsData.TLS_CERTIFICATE_ROOT,              "");
            addParameter(db, table, RcsSettingsData.TLS_CERTIFICATE_INTERMEDIATE,      "");
            addParameter(db, table, RcsSettingsData.SIP_TRANSACTION_TIMEOUT, 			"120");
            addParameter(db, table, RcsSettingsData.MSRP_DEFAULT_PORT, 				"20000");
            addParameter(db, table, RcsSettingsData.RTP_DEFAULT_PORT, 					"10000");
            addParameter(db, table, RcsSettingsData.MSRP_TRANSACTION_TIMEOUT, 			"5");
            addParameter(db, table, RcsSettingsData.REGISTER_EXPIRE_PERIOD, 			"600000");
            addParameter(db, table, RcsSettingsData.REGISTER_RETRY_BASE_TIME, 			"30");
            addParameter(db, table, RcsSettingsData.REGISTER_RETRY_MAX_TIME, 			"1800");
            addParameter(db, table, RcsSettingsData.PUBLISH_EXPIRE_PERIOD, 			"3600");
            addParameter(db, table, RcsSettingsData.REVOKE_TIMEOUT, 					"300");
            addParameter(db, table, RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE, 		RcsSettingsData.DIGEST_AUTHENT);
            addParameter(db, table, RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI, 		RcsSettingsData.DIGEST_AUTHENT);
            addParameter(db, table, RcsSettingsData.TEL_URI_FORMAT, 					RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.RINGING_SESSION_PERIOD, 			"60");
            addParameter(db, table, RcsSettingsData.SUBSCRIBE_EXPIRE_PERIOD, 			"3600");
            addParameter(db, table, RcsSettingsData.IS_COMPOSING_TIMEOUT, 				"5");
            addParameter(db, table, RcsSettingsData.SESSION_REFRESH_EXPIRE_PERIOD, 	"0");
            addParameter(db, table, RcsSettingsData.PERMANENT_STATE_MODE,	 			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.TRACE_ACTIVATED,			 		RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.TRACE_LEVEL,	 					"" + Logger.DEBUG_LEVEL);
            addParameter(db, table, RcsSettingsData.SIP_TRACE_ACTIVATED, 				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.SIP_TRACE_FILE,                    Environment.getExternalStorageDirectory() + "/sip.txt");
            addParameter(db, table, RcsSettingsData.MEDIA_TRACE_ACTIVATED,				RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_REFRESH_TIMEOUT, 		"1");
            addParameter(db, table, RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT, 		"86400");
            addParameter(db, table, RcsSettingsData.CAPABILITY_POLLING_PERIOD,			"3600");
            addParameter(db, table, RcsSettingsData.IM_CAPABILITY_ALWAYS_ON,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.FT_CAPABILITY_ALWAYS_ON,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.IM_USE_REPORTS,					RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.NETWORK_ACCESS,					""+RcsSettingsData.ANY_ACCESS);
            addParameter(db, table, RcsSettingsData.SIP_TIMER_T1,						"2000");
            addParameter(db, table, RcsSettingsData.SIP_TIMER_T2,						"16000");
            addParameter(db, table, RcsSettingsData.SIP_TIMER_T4,						"17000");
            addParameter(db, table, RcsSettingsData.SIP_KEEP_ALIVE,					RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.SIP_KEEP_ALIVE_PERIOD,				"60");
            addParameter(db, table, RcsSettingsData.RCS_APN,							"");
            addParameter(db, table, RcsSettingsData.RCS_OPERATOR,						"");
            addParameter(db, table, RcsSettingsData.MAX_CHAT_LOG_ENTRIES,				"500");
            addParameter(db, table, RcsSettingsData.MAX_RICHCALL_LOG_ENTRIES,			"200");
            addParameter(db, table, RcsSettingsData.MAX_IPCALL_LOG_ENTRIES,			"200"); 
            addParameter(db, table, RcsSettingsData.GRUU,								RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.USE_IMEI_AS_DEVICE_ID,             RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CPU_ALWAYS_ON,                     RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.AUTO_CONFIG_MODE,					""+RcsSettingsData.HTTPS_AUTO_CONFIG);
            addParameter(db, table, RcsSettingsData.PROVISIONING_TERMS_ACCEPTED,       RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.PROVISIONING_VERSION,				"0");
            addParameter(db, table, RcsSettingsData.PROVISIONING_TOKEN,				"");
            addParameter(db, table, RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS,    "");
            addParameter(db, table, RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS_ONLY,RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.DIRECTORY_PATH_PHOTOS,				Environment.getExternalStorageDirectory() + "/joyn/photos/");
            addParameter(db, table, RcsSettingsData.DIRECTORY_PATH_VIDEOS,				Environment.getExternalStorageDirectory() + "/joyn/videos/");
            addParameter(db, table, RcsSettingsData.DIRECTORY_PATH_FILES,				Environment.getExternalStorageDirectory() + "/joyn/files/");
            addParameter(db, table, RcsSettingsData.SECURE_MSRP_OVER_WIFI,	     		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.SECURE_RTP_OVER_WIFI,				RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.CONVERGENT_MESSAGING_UX,			RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.CAPABILITY_SIP_AUTOMATA, 			RcsSettingsData.FALSE);           
            addParameter(db, table, RcsSettingsData.KEY_GSMA_RELEASE, 					RcsSettingsData.VALUE_GSMA_REL_BLACKBIRD);
            addParameter(db, table, RcsSettingsData.BE_IPVOICECALL_AUTH,					"15");
            addParameter(db, table, RcsSettingsData.BE_IPVIDEOCALL_AUTH,					"15");
            addParameter(db, table, RcsSettingsData.IPVOICECALL_BREAKOUT,				RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IPVOICECALL_BREAKOUT_CS,			RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IPVIDEOCALL_UPGRADE_FROM_CS,		RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IPVIDEOCALL_UPGRADE_ON_CAPERROR,	RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY,	RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.IPCALL_E2E_LABEL,					"");
            addParameter(db, table, RcsSettingsData.IPCALL_BREAKOUT_LABEL,				"");
            addParameter(db, table, RcsSettingsData.IPCALL_E2E_VOICECAPABILITYHANDLING, RcsSettingsData.FALSE); 
 /** BB Drop 1 */
            addParameter(db, table, RcsSettingsData.MESSAGING_UX,                      "0");
            addParameter(db, table, RcsSettingsData.DELIVERY_TIMEOUT,                  "300");           
            addParameter(db, table, RcsSettingsData.FT_HTTP_CAP_ALWAYS_ON,             "0");
            addParameter(db, table, RcsSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED,             RcsSettingsData.FALSE);
            addParameter(db, table, RcsSettingsData.ROAMING_AUTHORIZED,                RcsSettingsData.TRUE);
            addParameter(db, table, RcsSettingsData.RCS_ONLY_ROAMING_AUTHORIZED,       RcsSettingsData.TRUE);
            /** M: ftAutAccept @{ */
            addParameter(db, table, RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING,
                    RcsSettingsData.FALSE);
            
            addParameter(db, table, RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING,
                                        RcsSettingsData.TRUE);
            /** @}*/
            addParameter(db, table, RcsSettingsData.RCSE_COMPRESSING_IMAGE,            RcsSettingsData.FALSE);      
   
        }

        /**
         * Add a parameter in the database
         *
         * @param db Database
         * @param key Key
         * @param table name
         * @param value Value
         */
        private void addParameter(SQLiteDatabase db, String table,String key, String value) {
            String sql = "INSERT INTO " + TABLE + " (" +
            	RcsSettingsData.KEY_KEY + "," +
            	RcsSettingsData.KEY_VALUE + ") VALUES ('" +
            	key + "','" + value + "');";
            db.execSQL(sql);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            /**
             * M: Modified to achieve the auto configuration feature. @{
             */
            upgrade(db,FIRST_USER_ACCOUNT_TABLE,oldVersion,currentVersion);
            upgrade(db,SECOND_USER_ACCOUNT_TABLE,oldVersion,currentVersion);
            upgrade(db,THIRD_USER_ACCOUNT_TABLE,oldVersion,currentVersion);
            /**
             * @}
             */
        }
        
        /**
         * M: Modified to achieve the auto configuration feature. @{
         */
        private void upgrade(SQLiteDatabase db, String table, int oldVersion, int currentVersion) {
        	// Get old data before deleting the table
        	Cursor oldDataCursor = db.query(TABLE, null, null, null, null, null, null);

            // Get all the pairs key/value of the old table to insert them back
            // after update
        	ArrayList<ContentValues> valuesList = new ArrayList<ContentValues>();
        	while(oldDataCursor.moveToNext()){
        		String key = null;
        		String value = null;
        		int index = oldDataCursor.getColumnIndex(RcsSettingsData.KEY_KEY);
        		if (index!=-1) {
        			key = oldDataCursor.getString(index);
        		}
        		index = oldDataCursor.getColumnIndex(RcsSettingsData.KEY_VALUE);
        		if (index!=-1) {
        			value = oldDataCursor.getString(index);
        		}
        		if (key!=null && value!=null) {
	        		ContentValues values = new ContentValues();
	        		values.put(RcsSettingsData.KEY_KEY, key);
	        		values.put(RcsSettingsData.KEY_VALUE, value);
	        		valuesList.add(values);
        		}
        	}
            oldDataCursor.close();

        	// Delete old table
        	db.execSQL("DROP TABLE IF EXISTS " + TABLE);

            // Recreate table
        	onCreate(db);

        	// Put the old values back when possible
        	for (int i=0; i<valuesList.size();i++) {
        		ContentValues values = valuesList.get(i);
        		String whereClause = RcsSettingsData.KEY_KEY + "=" + "\""+ values.getAsString(RcsSettingsData.KEY_KEY) + "\"";
        		// Update the value with this key in the newly created database
	    		// If key is not present in the new version, this won't do anything
	   			db.update(TABLE, values, whereClause, null);
        	}
        }
    }

    @Override
    public boolean onCreate() {
        openHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public String getType(Uri uri) {
        int match = uriMatcher.match(uri);
        switch(match) {
            case SETTINGS:
                return "vnd.android.cursor.dir/com.orangelabs.rcs.settings";
            case SETTINGS_ID:
                return "vnd.android.cursor.item/com.orangelabs.rcs.settings";
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    /**
     * M: Added to achieve the auto configuration feature. @{
     */
    private String getTable(Uri uri){
        String table = null;
        if(LauncherUtils.getDebugMode(getContext())){
            table = FIRST_USER_ACCOUNT_TABLE;
        }else{
        if(uri != null){
            String strUri = uri.toSafeString();
            if(strUri.endsWith(FIRST_USER_ACCOUNT_TABLE)){
                table = FIRST_USER_ACCOUNT_TABLE;
            }else if(strUri.endsWith(SECOND_USER_ACCOUNT_TABLE)){
                table = SECOND_USER_ACCOUNT_TABLE;
            }else if(strUri.endsWith(THIRD_USER_ACCOUNT_TABLE)){
                table = THIRD_USER_ACCOUNT_TABLE;
            }else{
                
                   
               
            }
        }
        }
        return table;
    }
    /**
     * @}
     */

    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        /**
         * M: Modified to achieve the auto configuration feature. @{
         */
        String strUri = getTable(uri);

        if(strUri != null){
            qb.setTables(strUri);
        }else{
            return null;
        }

        SQLiteDatabase db = openHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);

		// Register the contexts ContentResolver to be notified if
		// the cursor result set changes.
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }

        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count;
        SQLiteDatabase db = openHelper.getWritableDatabase();
        /**
         * M: Modified to achieve the auto configuration feature. @{
         */
        String table = getTable(uri);
        if(table == null){
            return -1;
        }
        count = db.update(table, values, where, null);
        /**
         * @}
         */
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        throw new UnsupportedOperationException("Cannot insert URI " + uri);
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        throw new UnsupportedOperationException();
    }
}
