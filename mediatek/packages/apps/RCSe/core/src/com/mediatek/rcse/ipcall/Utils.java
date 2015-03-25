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

package com.mediatek.rcse.ipcall;

import java.io.File;
import java.util.Date;
import java.util.List;
import java.util.Vector;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.widget.Toast;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.service.api.client.contacts.ContactsApi;
import com.orangelabs.rcs.utils.PhoneUtils;

/**
 * Utility functions
 * 
 * @author jexa7410
 */
public class Utils {
	/**
	 * Notification ID for terms
	 */
	public static int NOTIF_ID_TERMS = 999; 

	/**
	 * Notification ID for chat
	 */
	public static int NOTIF_ID_CHAT = 1000; 
	
	/**
	 * Notification ID for file transfer
	 */
	public static int NOTIF_ID_FT = 1001; 

	/**
	 * Notification ID for image share
	 */
	public static int NOTIF_ID_IMAGE_SHARE = 1002; 

	/**
	 * Notification ID for video share
	 */
	public static int NOTIF_ID_VIDEO_SHARE = 1003; 

	/**
	 * Notification ID for IPcall
	 */
	public static int NOTIF_ID_IPCALL = 1004; 
	
	/**
	 * Notification ID for location share
	 */
	public static int NOTIF_ID_LOCATION_SHARE = 1004; 
	
	/**
	 * RCS-e extension feature tag prefix
	 */
	public final static String FEATURE_RCSE_EXTENSION = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse";

	/**
	 * Format caller id
	 * 
	 * @param intent Intent invitation
	 * @return Id
	 */
	public static String formatCallerId(Intent invitation) {
		String number = invitation.getStringExtra("contact");
		String displayName = invitation.getStringExtra("contactDisplayname"); 
		if ((displayName != null) && (displayName.length() > 0)) { 
			return displayName + " (" + number + ")";
		} else {
			return number;
		}
	}
	
	/**
	 * Create a contact selector based on the native address book
	 * 
	 * @param activity Activity
	 * @return List adapter
	 */
	public static ContactListAdapter createContactListAdapter(Activity activity) {
	    String[] PROJECTION = new String[] {
	    		Phone._ID,
	    		Phone.NUMBER,
	    		Phone.LABEL,
	    		Phone.TYPE,
	    		Phone.CONTACT_ID
		    };
        ContentResolver content = activity.getContentResolver();
		Cursor cursor = content.query(Phone.CONTENT_URI, PROJECTION, Phone.NUMBER + "!='null'", null, null);
		// There is now some filtering to do
		// A number may be present in national or international format, it is considered to be the same

		// List of unique number
		Vector<String> treatedNumbers = new Vector<String>();
		
		MatrixCursor matrix = new MatrixCursor(PROJECTION);
		while (cursor.moveToNext()){
			// Keep a trace of already treated row. Key is (phone number in international, phone contact id)
			String phoneNumber = PhoneUtils.formatNumberToInternational(cursor.getString(1));
			
			if (!treatedNumbers.contains(phoneNumber)){
				matrix.addRow(new Object[]{cursor.getLong(0), 
						phoneNumber,
						cursor.getString(2),
						cursor.getInt(3),
						cursor.getLong(4)});
				treatedNumbers.add(phoneNumber);
			}
		}
		cursor.close();
		
		return new ContactListAdapter(activity, matrix);
	}
	
	/**
	 * Create a contact selector with RCS capable contacts
	 * 
	 * @param activity Activity
	 * @return List adapter
	 */
	public static ContactListAdapter createRcsContactListAdapter(Activity activity) {
	    String[] PROJECTION = new String[] {
	    		Phone._ID,
	    		Phone.NUMBER,
	    		Phone.LABEL,
	    		Phone.TYPE,
	    		Phone.CONTACT_ID
		    };
		MatrixCursor matrix = new MatrixCursor(PROJECTION);
	    ContactsApi contactsApi = new ContactsApi(activity);
	    
	    // Get the list of RCS contacts 
	    List<String> rcsContacts = contactsApi.getRcsContacts();
	    ContentResolver content = activity.getContentResolver();
	    
		// Query all phone numbers
        Cursor cursor = content.query(Phone.CONTENT_URI, 
        		PROJECTION, 
        		null, 
        		null, 
        		null);

		// List of unique number
		Vector<String> treatedNumbers = new Vector<String>();
		while (cursor.moveToNext()){
			// Keep a trace of already treated row. Key is (phone number in international, phone contact id)
			String phoneNumber = PhoneUtils.formatNumberToInternational(cursor.getString(1));
			// If this number is RCS and not already in the list, take it 
			if (rcsContacts.contains(phoneNumber) && !treatedNumbers.contains(phoneNumber)){
				matrix.addRow(new Object[]{cursor.getLong(0), 
						phoneNumber,
						cursor.getString(2),
						cursor.getInt(3),
						cursor.getLong(4)});
				treatedNumbers.add(phoneNumber);
			}
		}
		cursor.close();
		
		return new ContactListAdapter(activity, matrix);
	}
	
	/**
	 * Create a multi contacts selector with RCS capable contacts
	 * 
	 * @param activity Activity
	 * @return List adapter
	 */
	public static MultiContactListAdapter createMultiContactImCapableListAdapter(Activity activity) {
	    String[] PROJECTION = new String[] {
	    		Phone._ID,
	    		Phone.NUMBER,
	    		Phone.LABEL,
	    		Phone.TYPE,
	    		Phone.CONTACT_ID
		    };

		MatrixCursor matrix = new MatrixCursor(PROJECTION);
	    ContactsApi contactsApi = new ContactsApi(activity);

	    // Get the list of RCS contacts 
	    List<String> rcsContacts = contactsApi.getRcsContacts();

	    ContentResolver content = activity.getContentResolver();
		// Query all phone numbers
        Cursor cursor = content.query(Phone.CONTENT_URI, PROJECTION, null, null, null);

		// List of unique number
		Vector<String> treatedNumbers = new Vector<String>();
		while (cursor.moveToNext()){
			// Keep a trace of already treated row. Key is (phone number in international, phone contact id)
			String phoneNumber = PhoneUtils.formatNumberToInternational(cursor.getString(1));
			// If this number is RCS and not already in the list, take it 
			if (rcsContacts.contains(phoneNumber) && !treatedNumbers.contains(phoneNumber)){
				matrix.addRow(new Object[]{cursor.getLong(0), 
						phoneNumber,
						cursor.getString(2),
						cursor.getInt(3),
						cursor.getLong(4)});
				treatedNumbers.add(phoneNumber);
			}
		}
		cursor.close();
		return new MultiContactListAdapter(activity, matrix);
	}
	
	/**
	 * Display a toast
	 * 
	 * @param activity Activity
	 * @param message Message to be displayed
	 */
    public static void displayToast(Activity activity, String message) {
        Toast.makeText(activity, message, Toast.LENGTH_SHORT).show();
    }

	/**
	 * Display a long toast
	 * 
	 * @param activity Activity
	 * @param message Message to be displayed
	 */
    public static void displayLongToast(Activity activity, String message) {
        Toast.makeText(activity, message, Toast.LENGTH_LONG).show();
    }
    
    /**
	 * Show a message and exit activity
	 * 
	 * @param activity Activity
	 * @param msg Message to be displayed
	 */
    public static void showMessageAndExit(final Activity activity, String msg) {
        if (activity.isFinishing()) {
        	return;
        }

		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(msg);
		builder.setTitle(R.string.title_msg);
		builder.setCancelable(false);
		builder.setPositiveButton(activity.getString(R.string.label_ok),
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						activity.finish();
					}
				});
		AlertDialog alert = builder.create();
		alert.show();
    }

	/**
	 * Show an message
	 * 
	 * @param activity Activity
	 * @param msg Message to be displayed
	 * @return Dialog
	 */
    public static AlertDialog showMessage(Activity activity, String msg) {
    	AlertDialog.Builder builder = new AlertDialog.Builder(activity);
    	builder.setMessage(msg);
    	builder.setTitle(R.string.title_msg);
    	builder.setCancelable(false);
    	builder.setPositiveButton(activity.getString(R.string.label_ok), null);
    	AlertDialog alert = builder.create();
    	alert.show();
		return alert;
    }

	/**
	 * Show a message with a specific title
	 * 
	 * @param activity Activity
	 * @param title Title of the dialog
	 * @param msg Message to be displayed
	 * @return Dialog
	 */
    public static AlertDialog showMessage(Activity activity, String title, String msg) {
    	AlertDialog.Builder builder = new AlertDialog.Builder(activity);
    	builder.setMessage(msg);
    	builder.setTitle(title);
    	builder.setCancelable(false);
    	builder.setPositiveButton(activity.getString(R.string.label_ok), null);
    	AlertDialog alert = builder.create();
    	alert.show();
		return alert;
    }

    /**
	 * Show a picture and exit activity
	 * 
	 * @param activity Activity
	 * @param url Picture to be displayed
	 */
    public static void showPictureAndExit(final Activity activity, String url) {
        if (activity.isFinishing()) {
        	return;
        }
        
        Toast.makeText(activity, activity.getString(R.string.label_receive_image, url), Toast.LENGTH_LONG).show();

        Intent intent = new Intent();  
        intent.setAction(android.content.Intent.ACTION_VIEW);  
        File file = new File(url);  
        intent.setDataAndType(Uri.fromFile(file), "image/*");  
        activity.startActivity(intent);        
    }
  
    /**
	 * Show an info with a specific title
	 * 
	 * @param activity Activity
	 * @param title Title of the dialog
	 * @param items Items
	 */
    public static void showList(Activity activity, String title, CharSequence[] items) {
        if (activity.isFinishing()) {
        	return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
    	builder.setTitle(title);
    	builder.setCancelable(false);
    	builder.setPositiveButton(activity.getString(R.string.label_ok), null);
        builder.setItems(items, null);
        AlertDialog alert = builder.create();
    	alert.show();
    }
    
	/**
	 * Show a progress dialog with the given parameters 
	 * 
	 * @param activity Activity
	 * @param msg Message to be displayed
	 * @return Dialog
	 */
	public static ProgressDialog showProgressDialog(Activity activity, String msg) {
        ProgressDialog dlg = new ProgressDialog(activity);
		dlg.setMessage(msg);
		dlg.setIndeterminate(true);
		dlg.setCancelable(true);
		dlg.setCanceledOnTouchOutside(false);
		dlg.show();
		return dlg;
	}    

	/**
     * Format a date to string
     * 
     * @param d Date
     * @return String
     */
    public static String formatDateToString(long d) {
    	if (d > 0L) {
	    	Date df = new Date(d);
	    	return df.toLocaleString();
    	} else {
    		return "";
    	}
    }
}
