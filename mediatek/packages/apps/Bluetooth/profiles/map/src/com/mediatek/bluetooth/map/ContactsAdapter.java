package com.mediatek.bluetooth.map;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.database.Cursor;
import android.net.Uri.Builder;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.provider.ContactsContract.SearchSnippetColumns;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.net.Uri;
import android.util.Log;

public class ContactsAdapter {
	private static final String TAG = "ContactsAdapter-MAP";

	public static final String LINE_NUMBER_SPERATOR = ";";
	
	private static final String[] DEFAULT_CONTACTS_PROJECTION = new String[] {
		Contacts._ID,
		Contacts.DISPLAY_NAME_PRIMARY, 
	};

	private static final int CONTACTS_COLUMN_ID = 0;
	private static final int CONTACTS_COLUMN_DISPLAY_NAME = 1;

	private static final String CONTACTS_SELECTION_BY_NAME = Contacts.DISPLAY_NAME_PRIMARY + " LIKE ?";
	private static final String CONTACTS_SELECTION_BY_CONTACT_ID = Contacts._ID + " =?";
	
	
	private static final String[] DEFAULT_DATA_PROJECTION = new String[] {		
		Data.RAW_CONTACT_ID,
		Phone.NUMBER,
	};
	private static final int DATA_COLUMN_RAW_CONTACT_ID = 0;
	private static final int DATA_COLUMN_NUMBER = 1;
	
	private static final String DATA_SELECTION_BY_CONTACT_ID = Data.RAW_CONTACT_ID + "=?" + " AND " 
													+ Data.MIMETYPE + "='" + Phone.CONTENT_ITEM_TYPE + "'";
	private static final String DATA_SELECTION_BY_NUMBER = Phone.NUMBER + " LIKE ?" + " AND " 
													+ Data.MIMETYPE + "='" + Phone.CONTENT_ITEM_TYPE + "'";	

	private static ContactsAdapter mAdapter;
	private ContentResolver mContentResolver; 
	private ContactsAdapter() {
	}
	
	public static ContactsAdapter getDefault(Context context) {
		synchronized(ContactsAdapter.class) {
			if (mAdapter == null) {
				mAdapter = new ContactsAdapter();
			}	
			if (!mAdapter.init(context)) {
				return null;
			}
			return mAdapter;
		}
	}
	private boolean init(Context context) {
		if (context != null) {
			mContentResolver = context.getContentResolver();
			return true;
		} else {
			return false;
		}
	}

	public String queryNumber (String words) {
		String[] arg = new String[]{"%"+words+"%"};
		Cursor contactsCursor = mContentResolver.query(Contacts.CONTENT_URI, DEFAULT_CONTACTS_PROJECTION, CONTACTS_SELECTION_BY_NAME, arg, Contacts.SORT_KEY_PRIMARY);
		StringBuilder sb = new StringBuilder();
		if (contactsCursor == null) {
			return null;
		}
		while (contactsCursor.moveToNext()) {
			long id = contactsCursor.getLong(CONTACTS_COLUMN_ID);
			Cursor dataCursor = mContentResolver.query(Data.CONTENT_URI, DEFAULT_DATA_PROJECTION, DATA_SELECTION_BY_CONTACT_ID,
					new String []{String.valueOf(id)}, null);
			if (dataCursor != null) {
				while (dataCursor.moveToNext()) {
					log(dataCursor.getString(DATA_COLUMN_NUMBER));
					sb.append(dataCursor.getString(DATA_COLUMN_NUMBER));
					sb.append(LINE_NUMBER_SPERATOR);
				}
				dataCursor.close();
			}
		}
		contactsCursor.close();
		return sb.toString();
	}

	//TODO: query with normalized number
	public String queryName(String number){
		if (number == null || number.length() == 0) {
			return null;
		}	
		String name = null;
		Cursor dataCs = mContentResolver.query(Data.CONTENT_URI, DEFAULT_DATA_PROJECTION, DATA_SELECTION_BY_NUMBER, 
								new String []{number}, null);
		if (dataCs == null) {
			return null;
		}
		if (dataCs.moveToNext()) {
			Uri uri =ContentUris.withAppendedId(Contacts.CONTENT_URI, dataCs.getLong(DATA_COLUMN_RAW_CONTACT_ID));
			Cursor contactsCs = mContentResolver.query(uri, DEFAULT_CONTACTS_PROJECTION,
											null, null, null);
			if (contactsCs == null) {
				return null;
			}
			if (contactsCs.moveToNext()) {
				name = contactsCs.getString(CONTACTS_COLUMN_DISPLAY_NAME);
			}
			contactsCs.close();
		}
		dataCs.close();
		return name;
		
	}

	public boolean isPhoneNumber(String number) {
        int numDigits = 0;
        int len = number.length();
        for (int i = 0; i < len; i++) {
            char c = number.charAt(i);
            if (Character.isDigit(c)) {
                numDigits ++;
            } else if (c == '*' || c == '#' || c == 'N' || c == '.' || c == ';'
                    || c == '-' || c == '(' || c == ')' || c == ' ') {
                // carry on
            } else if (c == '+' && numDigits == 0) {
                // plus before any digits is ok
            } else {
                return false; // not a phone number
            }
        }
        return (numDigits > 0);
    }

	private boolean doesPhoneNumberMatch(String[] targetArray, String[] templateArray) {
		if (targetArray == null || templateArray == null ||
			targetArray.length == 0 || templateArray.length == 0) {
			return false;
		}
		 
		for(String template:templateArray) {
			for (String target:targetArray) {
				if ((target.indexOf(template) != 0) || (template.indexOf(target)!= -1)) {
					return true;
				}
			}
		}
		return false;
	}

	/* return */
	public boolean doesPhoneNumberMatch (String target, String template1, String template2) {
		boolean match = false;
		boolean isTemplateEmpty = (template1 == null && template2 == null);
		log("doesPhoneNumberMatch(): target is "+ target+", template1 is "+ template1 +", template2 is "+ template2);
		if (target == null) {
			return false;
		}
		if (isTemplateEmpty) {
			return true;
		}
		
		if (template1 != null) {
			String[] targetArray = target.split(";");
			String[] templateArray = template1.split(";");
			if (doesPhoneNumberMatch(targetArray, templateArray)) {
				return true;
			}
		} 
		if (template2 != null && isPhoneNumber(template2)) {
			return target.indexOf(template2) != -1;
		} 
		return false;
		
	}
	
	private static void log(String info) {
		if (info == null) {
			return;
		}
		Log.v(TAG, info);
	}
}

