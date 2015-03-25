package com.mediatek.bluetooth.pbap;
 
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Event;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Note;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.provider.ContactsContract.CommonDataKinds.Website;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

import com.mediatek.bluetooth.pbap.ContactInfo.EmailWithType;
import com.mediatek.bluetooth.pbap.ContactInfo.EventWithType;
import com.mediatek.bluetooth.pbap.ContactInfo.ImWithType;
import com.mediatek.bluetooth.pbap.ContactInfo.OrganizationWithType;
import com.mediatek.bluetooth.pbap.ContactInfo.PhoneNumberWithType;
import com.mediatek.bluetooth.pbap.ContactInfo.PostalWithType;
import com.mediatek.bluetooth.pbap.ContactSimInfo.PhoneNumberSimWithType;

import java.util.ArrayList;
import java.util.Map;
 
 
public class PbapOperateContacts { 
 
    private static final String TAG = "[BT][PBAPUT][PbapOperateContacts]";
    
    public static final int INCOMING_TYPE = 1; 
    public static final int OUTGOING_TYPE = 2; 
    public static final int MISSED_TYPE = 3; 
    public ContentValues values = new ContentValues(); 
    private static ArrayList<Uri> sInsertedContactsID = new ArrayList<Uri>();
    private static ArrayList<Uri> sInsertedSimContactsID = new ArrayList<Uri>();
    private static ArrayList<Uri> sInsertedCallLogsId = new ArrayList<Uri>();
     
    public PbapOperateContacts() {

    }
    
    // CIList is the storage of the phonebook, cx is the context who call this method
    public void insertContactInfo(ArrayList<ContactInfo> contactInfoList, Context cx) { 
        Log.d(TAG, "insertContactInfo enter");
        // for storing the address information 
        String[] address; 

        for (ContactInfo ci:contactInfoList) { 
            
             /**
              * At first, insert a null value into the RawContacts.CONTENT_URI, 
              * the purpose is getting the return is rawContactId
              * Then use the rawContactId to insert value. 
              * You must insert a null value first, or you cannot  see this contact in phonebook. That is a rule
              */ 
            ContentValues contentvalues = new ContentValues();
            Log.d("insertContactInfo", "result  CX" + (cx == null));
            //At first, insert a null value into the RawContacts.CONTENT_URI, purpose is getting the return is rawContactId 
            Uri rawContactUri = cx.getContentResolver().insert(RawContacts.CONTENT_URI, contentvalues); 
            long rawContactId = ContentUris.parseId(rawContactUri); 
            
            //insert nickname into Data table, the table will be used to be insert into phone book
            if (ci.getCiNickName() != null) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId);  
                contentvalues.put(Data.MIMETYPE, Nickname.CONTENT_ITEM_TYPE);//type is content
                contentvalues.put(Nickname.NAME, ci.getCiNickName()); 
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            
            for (String note : ci.getCiNoteList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId);  
                contentvalues.put(Data.MIMETYPE, Note.CONTENT_ITEM_TYPE);//type is content
                contentvalues.put(Note.NOTE, note); 
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            
            for (String website : ci.getCiWebsiteList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId);  
                contentvalues.put(Data.MIMETYPE, Website.CONTENT_ITEM_TYPE);//type is content
                contentvalues.put(Website.URL, website); 
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
                
            // insert name information into Data table, the table will be used to be insert into phone book
            if (ci.getCiNameSize() > 0) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId);  
                contentvalues.put(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);//type is content
                for (Map.Entry<String, Integer> item : ci.getCiFlagInfo().entrySet()) {
                    contentvalues.put(item.getKey(), item.getValue());
                }
                
              //only from DISPLAY_NAME to PHONETIC_FAMILY_NAME
                for (Map.Entry<String, String> item: ci.getCiNameMapObj().entrySet()) {
                    contentvalues.put(item.getKey(), item.getValue()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            
            //insert phone number information into Data table 
            for (PhoneNumberWithType phoneNum : ci.getCiPhoneNumList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE); 
                contentvalues.put(Phone.NUMBER, phoneNum.getNumber()); 
                contentvalues.put(Phone.TYPE, phoneNum.getType()); 
                if (phoneNum.getType() == Phone.TYPE_CUSTOM) {
                    contentvalues.put(Phone.LABEL, phoneNum.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            //insert Email information into Data table 
            for (EmailWithType email : ci.getCiEmailList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, Email.CONTENT_ITEM_TYPE); 
                contentvalues.put(Email.DATA, email.getAddress()); 
                contentvalues.put(Email.TYPE, email.getType()); 
                if (email.getType() == Email.TYPE_CUSTOM) {
                    contentvalues.put(Email.LABEL, email.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            //insert Address information into Data table  
            // address should be match this format: 
            // street +"-"+poBox+"-"+NEIGHBORHOOD+"-"+city+"-"+state+"-"+postalCode+"-"+country
            int structuredPostalType;
            for (PostalWithType postal : ci.getCiPostalList()) {
                address = postal.getAddress().split("-"); 
                structuredPostalType = postal.getType();
                String postalDetail = StructuredPostal.POBOX;
                
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, StructuredPostal.CONTENT_ITEM_TYPE); 
                for (int j = 0; (j <= 6) && j < (address.length); j++) {
                    switch (j) {
                    case 0:
                        postalDetail = StructuredPostal.STREET;
                        break;
                    case 1:
                        postalDetail = StructuredPostal.POBOX;
                        break;
                    case 2:
                        postalDetail = StructuredPostal.NEIGHBORHOOD;
                        break;
                    case 3:
                        postalDetail = StructuredPostal.CITY;
                        break;
                    case 4:
                        postalDetail = StructuredPostal.REGION;
                        break;
                    case 5:
                        postalDetail = StructuredPostal.POSTCODE;
                        break;
                    case 6:
                        postalDetail = StructuredPostal.COUNTRY;
                        break;                        
                    default:
                        continue;

                    }
                    if (!address[j].equalsIgnoreCase("null")) {
                        contentvalues.put(postalDetail, address[j]); 
                    }
                }
                contentvalues.put(StructuredPostal.TYPE, structuredPostalType); 
                if (structuredPostalType == StructuredPostal.TYPE_CUSTOM) {
                    contentvalues.put(StructuredPostal.LABEL, postal.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues);
            }

            //insert IM information into Data table 
            for (ImWithType im : ci.getCiImList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, Im.CONTENT_ITEM_TYPE); 
                contentvalues.put(Im.DATA, im.getAddress()); 
                //this item is not important, but must be set. Or there is a error: 
                //Data2 must be specified when Data3 is defined.
                contentvalues.put(Im.TYPE, Im.TYPE_HOME);
                contentvalues.put(Im.PROTOCOL, im.getType()); 
                if (im.getType() == Im.TYPE_CUSTOM) {
                    contentvalues.put(Im.LABEL, im.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            
            //insert Event(Birthday) information into Data table 
            for (EventWithType event : ci.getCiEventList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, Event.CONTENT_ITEM_TYPE); 
                contentvalues.put(Event.DATA, event.getStartDate()); 
                contentvalues.put(Event.TYPE, event.getType()); 
                if (event.getType() == Event.TYPE_CUSTOM) {
                    contentvalues.put(Event.LABEL, event.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            }
            
            //insert Event(Birthday) information into Data table 
            for (OrganizationWithType organization : ci.getCiOrganizationList()) {
                contentvalues.clear(); 
                contentvalues.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentvalues.put(Data.MIMETYPE, Organization.CONTENT_ITEM_TYPE); 
                contentvalues.put(Organization.COMPANY, organization.getCompany()); 
                contentvalues.put(Organization.TYPE, organization.getType()); 
                if (organization.getType() == Organization.TYPE_CUSTOM) {
                    contentvalues.put(Organization.LABEL, organization.getLable()); 
                }
                contentvalues.put(Organization.TITLE, organization.getTitle());
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentvalues); 
            } 
            
            sInsertedContactsID.add(rawContactUri);
        }
        
    } 
    
    public void deleteAllContacts(Context context) {
        for (Uri idUri : sInsertedContactsID) {
            Log.d(TAG, "deleteAllContacts");
            context.getContentResolver().delete(idUri, null, null);
        }
        sInsertedContactsID.clear();
        
    }

    public void insertContactSimInfo(ArrayList<ContactSimInfo> contentInfoList, Context cx) { 
        
        // for storing the address information 
        String[] address; 

        for (ContactSimInfo ci : contentInfoList) { 
            
             /**
              * At first, insert a null value into the RawContacts.CONTENT_URI, perpose is getting the return is rawContactId
              * Then use the rawContactId to insert value. 
              * You must insert a null value first, or you cannot  see this contact in phonebook. That is a rule
              */ 
            ContentValues contentValue = new ContentValues(); 
            contentValue.put(RawContacts.INDICATE_PHONE_SIM, 1);
            //At first, insert a null value into the RawContacts.CONTENT_URI, purpose is getting the return is rawContactId 
            Uri rawContactUri = cx.getContentResolver().insert(RawContacts.CONTENT_URI, contentValue); 
            long rawContactId = ContentUris.parseId(rawContactUri); 
         
            // insert name information into Data table, the table will be used to be insert into phone book
            if (ci.getCiNameSize() > 0) {
                contentValue.clear(); 
                contentValue.put(Data.RAW_CONTACT_ID, rawContactId);  
                contentValue.put(Data.MIMETYPE, StructuredName.CONTENT_ITEM_TYPE);//type is content
                
                //only from DISPLAY_NAME to PHONETIC_FAMILY_NAME
                for (Map.Entry<String, String> item : ci.getCiNameMapObj().entrySet()) {
                    contentValue.put(item.getKey(), item.getValue()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentValue); 
            }
            
            //insert phone number information into Data table 
            for (PhoneNumberSimWithType phoneNum : ci.getCiPhoneNumList()) {
                contentValue.clear(); 
                contentValue.put(Data.RAW_CONTACT_ID, rawContactId); 
                contentValue.put(Data.MIMETYPE, Phone.CONTENT_ITEM_TYPE); 
                contentValue.put(Phone.NUMBER, phoneNum.getNumber()); 
                contentValue.put(Phone.TYPE, phoneNum.getType()); 
                if (phoneNum.getType() == Phone.TYPE_CUSTOM) {
                    contentValue.put(Phone.LABEL, phoneNum.getLable()); 
                }
                cx.getContentResolver().insert(android.provider.ContactsContract.Data.CONTENT_URI, contentValue); 
            }
            
            sInsertedSimContactsID.add(rawContactUri);
        }
        
    }
    
    public void deleteAllSimContacts(Context context) {
        for (Uri idUri : sInsertedSimContactsID) {
            Log.d(TAG, "deleteAllSimContacts");
            context.getContentResolver().delete(idUri, null, null);
        }
        sInsertedContactsID.clear();
    }
      
     // CHList is the storage of the call history, cx is the context who call this method

     public void addCallHistory(ArrayList<CallHistory> callHistoryList, Context cx) {
          
         /**
          *  ch_id  Number(10)  No                              call history ID
          *  ch_dialing_tel     Varchar2(255)   No              call host number
          *  ch_called_tel  Varchar2(255)   No                  call reciver number
          *  ch_call_start  date    No  sysdate                 time of call start
          *  ch_call_end    date    No  sysdate                 time of call end
          *  ch_calt_time   Number(10)  No                      how long time of this call
          *  ch_talk_type   Number(1)   No                      call type,0 means voice call,1 means video call
          *  ch_call_type   Number(1)   No                      call type,0 means outgoing call,
          *                                                                 1 means incoming call,2 means missed call
          *  ch_du_id   Number(10)  No                          user id
          *  ch_insert_time Date    No  sysdate                 time of this record created 
          *  ch_remark_info Varchar2(2048)                      others
          */ 
         for (CallHistory ch:callHistoryList) {
              
             Uri rawCallLogUri;
             values.clear(); 
              
             values.put(CallLog.Calls.TYPE, ch.getChCallType());   
             if (ch.getChCalledNumber() != null) {
                 values.put(CallLog.Calls.NUMBER, ch.getChCalledNumber());   
             }
             values.put(CallLog.Calls.DATE, ch.getChCalledDate()); 
             values.put(CallLog.Calls.NEW, ch.getChCalledNew());//0 means new missed, 1 means user already read this record
             if (ch.getChNumberName() != null) {
                 values.put(CallLog.Calls.CACHED_NAME, ch.getChNumberName());
             }
             values.put(CallLog.Calls.CACHED_NUMBER_TYPE, ch.getChNumberType());
             if (ch.getChNumberLable() != null) {
                 values.put(CallLog.Calls.CACHED_NUMBER_LABEL, ch.getChNumberLable());
             }
             
             rawCallLogUri = cx.getContentResolver().insert(CallLog.Calls.CONTENT_URI, values);   
             Log.d(TAG, "rawCallLogUri" + rawCallLogUri.toString());
             
             sInsertedCallLogsId.add(rawCallLogUri);
         } 
          
          
     } 
     
     public void deleteAllCallLogs(Context context) {
         for (Uri idUri : sInsertedCallLogsId) {
             Log.d(TAG, "deleteAllCallLogs");
             long insertedCallId = ContentUris.parseId(idUri);
             context.getContentResolver().delete(CallLog.Calls.CONTENT_URI, Calls._ID + "=?", 
                     new String[]{new Long(insertedCallId).toString()});
         }
         sInsertedCallLogsId.clear();
         
     }
     
} 
