package com.orangelabs.rcs.provider.ipcall;

import android.net.Uri;

import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;

/**
* IP call history provider data 
* 
* @author owom5460
*/
public class IPCallData {
	
	// Database URI
	public static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.ipcall/ipcall");
		
	// Column names
	public static final String KEY_ID = "_id";
	public static final String KEY_CONTACT = "contact";
	public static final String KEY_EVENT_TYPE = "event_type";
	public static final String KEY_AUDIO_MIME_TYPE = "audio_mime_type";
	public static final String KEY_VIDEO_MIME_TYPE = "video_mime_type";
	public static final String KEY_TIMESTAMP = "_date";
	public static final String KEY_NUMBER_MESSAGES ="number_of_messages";
	public static final String KEY_STATUS = "status";
	public static final String KEY_SESSION_ID = "sessionId";
		
	// Event direction
	public static final int EVENT_INCOMING = 16;
	public static final int EVENT_OUTGOING = 17;	
	
	// "status" values
	public static final int STATUS_STARTED = EventsLogApi.STATUS_STARTED; 
	public static final int STATUS_FAILED = EventsLogApi.STATUS_FAILED;
	public static final int STATUS_CANCELED = EventsLogApi.STATUS_CANCELED;
	public static final int STATUS_TERMINATED = EventsLogApi.STATUS_TERMINATED;
}
