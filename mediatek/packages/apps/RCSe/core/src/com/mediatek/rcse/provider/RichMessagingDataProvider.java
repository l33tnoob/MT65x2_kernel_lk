/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.provider;

import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;
import java.util.Collections;
import java.util.Map;
import android.net.Uri;

import android.content.ContentProvider;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.ModelImpl.ChatListProvider;
import com.orangelabs.rcs.provider.messaging.RichMessagingData;
import com.orangelabs.rcs.service.api.client.eventslog.EventsLogApi;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.mediatek.rcse.api.Logger;

/**
 * This class provided as interface for content provider
 */
public class RichMessagingDataProvider {
	
	   private static final String TAG = "RichMessagingDataProvider";
	   
		/**
		 * Current instance
		 */
		private static RichMessagingDataProvider instance = null;

		/**
		 * Content resolver
		 */
		private ContentResolver cr;
		
		/**
		 * Database URI
		 */
		private Uri databaseUri = RichMessagingData.CONTENT_URI;

		/**
		 * CHAT HISTORY LIMIT FOR CONTACT 
		 * */
		
		private String CONTACT_CHAT_HISTORY_LIMIT = "50";
		
		/**
		 * The logger
		 */
		//private Logger logger = Logger.getLogger(this.getClass().getName());
		
		/**
		 * Create instance
		 * 
		 * @param ctx Context
		 */
		public static synchronized void createInstance(Context ctx) {
			if (instance == null) {
				instance = new RichMessagingDataProvider(ctx);
			}
		}
		
		/**
		 * Returns instance
		 * 
		 * @return Instance
		 */
		public static RichMessagingDataProvider getInstance() {
			return instance;
		}
		
		/**
	     * Constructor
	     * 
	     * @param ctx Application context
	     */

		private RichMessagingDataProvider(Context ctx) {			
	        this.cr = ctx.getContentResolver();
		}

		 
		//get recent o2o chats
		private Cursor getRecentO2OChats() {
		   
	    	Uri databaseRecentMsgUri = Uri.parse("content://com.orangelabs.rcs.messaging/messaging/recent_messages");
	    	Cursor cursor = null;
	    	
	    	
	    	 Logger.d(TAG,"getRecentO2OChats() : fetching recentO2O Chats");
	    	 
	    
	    	
	    	try {
	    		//get the cursor for recent o2o chat history
	    		cursor = cr.query(databaseRecentMsgUri, null, null, null, null);
       
            } catch (Exception e) {
            	
            	Logger.d(TAG,"getRecentO2OChats() : EXCEPTION :"+ e.getMessage());
            	
            	
            	  if(cursor!=null){
                      cursor.close();
                    	cursor = null;	
                    }
                e.printStackTrace();
            }
	    	
	    	return cursor;
	    }
		
		//get recent group chats 
		 private Cursor getRecentGroupChats(){		  
		    	Uri databaseRecentGroupMsgUri = Uri.parse("content://com.orangelabs.rcs.messaging/messaging/recent_group_messages");
		    	Cursor cursor = null;	
		    	
		    	Logger.d(TAG,"getRecentGroupChats() : fetching recent group Chats");
		    	
		    	try {
		    		//get the cursor for recent o2o chat history
		    		cursor = cr.query(databaseRecentGroupMsgUri, null, null, null, null);
	       
	            } catch (Exception e) {
	            	
	            	
	            	Logger.d(TAG,"getRecentGroupChats() : EXCEPTION :"+ e.getMessage());
	            	
	            	
	            	  if(cursor!=null){
                      cursor.close();
                    	cursor = null;	
                    }
	                e.printStackTrace();
	            }	
		    	return cursor;
		 }
		
	/*
	 * get the recent contacts for a contact 
	 * */	
		public ArrayList<Integer> getRecentChatForContact(String Contact, Integer offset){
		    ArrayList<Integer> recentContactChats = new ArrayList<Integer>();
		    Cursor cursor = null;
		    int chatHistoryLimit = 1;
		    try {
		    	
		        final String[] projection = { RichMessagingData.KEY_ID};
		    			
		        String orderBy =  RichMessagingData.KEY_ID + " DESC "
                                 + " LIMIT " + offset+ " , "+CONTACT_CHAT_HISTORY_LIMIT  ;
		        
		        
		        cursor = cr.query(databaseUri, projection, 
                        "(" + RichMessagingData.KEY_CONTACT + " LIKE '%" + Contact +"%' " 
                        + ") AND ( " + 
                        RichMessagingData.KEY_TYPE + " = 0 OR " + RichMessagingData.KEY_TYPE + " = 1 OR "
                        + RichMessagingData.KEY_TYPE + " = 6 OR " + RichMessagingData.KEY_TYPE + " = 7 "
                        + ") AND (" +
                        RichMessagingData.KEY_IS_SPAM + " = 0 " 
                        + " ) " ,
                        null ,
                        orderBy
                       );
                
                
                if(cursor!=null && cursor.moveToFirst()){    
                    do
                    {
                        String msgID = cursor.getString(cursor.getColumnIndex(RichMessagingData.KEY_ID));      
                        recentContactChats.add(Integer.parseInt(msgID));
                        chatHistoryLimit++;
                    }while(cursor.moveToNext()&& chatHistoryLimit<20);
                }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
		    finally{
		        if(cursor != null)
		            cursor.close();
		    }
		    return recentContactChats;
		}
		
		
		public ArrayList<Integer> getRecentChatsForGroup(String groupChatID, Integer offset){
		
		
			ArrayList<Integer> recentChatsForGroup = new ArrayList<Integer>();
			
			/*
			 * //get the appended contacts
			 
	    	for(Participant participant: participantList){
				appendedContacts += participant.getContact() +";";
			}
	    	*/
	    	
	    	Logger.d(TAG,"getRecentChatsForGroup() : for groupChatID "+ groupChatID);
	    	
			Uri databaseChatForGroupUri = Uri.parse("content://com.orangelabs.rcs.messaging/messaging/recent_chats_for_group");
		    Cursor cursor = null;
			int chatHistoryLimit = 1;  			
			try {
				    		
			    
			    String orderBy =  " LIMIT " + offset+ " , "+CONTACT_CHAT_HISTORY_LIMIT  ;
			    
			    
			            //get the cursor chats for a group
				    		cursor = cr.query(Uri.withAppendedPath(databaseChatForGroupUri,Uri.encode(groupChatID)), null, null, null, orderBy);
			               
				    		
			                if(cursor!=null && cursor.moveToFirst()){    
			                    do
			                    {
			                        String msgID = cursor.getString(cursor.getColumnIndex(RichMessagingData.KEY_ID));      
			                        recentChatsForGroup.add(Integer.parseInt(msgID));
			                        chatHistoryLimit++;
			                    }while(cursor.moveToNext()&& chatHistoryLimit<20);
			                }
				    		
			       
			       } catch (Exception e) {
			            	
			            	Logger.d(TAG,"getRecentChatsForGroup() : EXCEPTION :"+ e.getMessage());
			            			            	
			            	  if(cursor!=null){
			                      cursor.close();
			                    	cursor = null;	
			                    }
			                e.printStackTrace();
			       }finally{
			    	   if(cursor!=null){
		                      cursor.close();
		                    	cursor = null;	
		                    }
			       }
				    	
			    return recentChatsForGroup;

		}
		
		
		public ArrayList<ChatListProvider> getRecentChats(){
			
			Logger.d(TAG,"getRecentChats()");
			
			ArrayList<ChatListProvider> result = new ArrayList<ChatListProvider>();
			 
			  //get the cursor for the o2o and group chats 
			   Cursor curRecentO2OChat = getRecentO2OChats();
			   Cursor curRecentGroupChat = getRecentGroupChats();
			   
			   //create a TREEMAP and add cursor values using timestamp as key for both the cursors
			   TreeMap<String, ChatListProvider> recentChatMap = new TreeMap<String, ChatListProvider>(Collections.reverseOrder());
			   
			   try{
				   ArrayList<String> contactList = new ArrayList<String>();
				   //add o2o in tree 
				   if(curRecentO2OChat!= null && curRecentO2OChat.moveToFirst()){
					   
					   Logger.d(TAG,"getRecentChats() : getting data from curRecentO2OChat");
					 
					   do{
						   String msgID = curRecentO2OChat.getString(curRecentO2OChat.getColumnIndex(RichMessagingData.KEY_ID));
	                       String contact = curRecentO2OChat.getString(curRecentO2OChat.getColumnIndex(RichMessagingData.KEY_CONTACT)); 
	                       //String dateTimeStamp = curRecentO2OChat.getString(curRecentO2OChat.getColumnIndex(RichMessagingData.KEY_TIMESTAMP)); 
	                       
	                       //format the contact
	                       contact = PhoneUtils.extractNumberFromUri(contact);
	                       
	                       Logger.d(TAG,"getRecentChats() : contact - "+ contact + " ; msgID - "+msgID);
	                       
	                       //if contact is not present already in the list
	                       if(!contactList.contains(contact)){
	                    	   contactList.add(contact);
	                    	   
	                    	   //create chat list
	                    	   ChatListProvider chatList =  createChatList(msgID,contact,"",0);
	                    	   
	                    	   Logger.d(TAG,"getRecentChats() : contact - "+ contact +" : added to map");
	                    	   //add the chat list in the map with timestamp as the key
	                    	   recentChatMap.put(msgID,chatList);
	                    	   
	                    	   
	                       }
	                       else{
	                    	   Logger.d(TAG,"getRecentChats() : contact - "+ contact +" : already present");
	                    	   //reject the contact because it was already added
	                    	   //its is used becasue a contact may be present in different formats in database
	                       }
	                       
						   
					   }while(curRecentO2OChat.moveToNext());
		
					   
					   if(curRecentO2OChat!=null){
						   curRecentO2OChat.close();
						   curRecentO2OChat = null;
					   }
				   } 
				   
				   
				   //add grp chat in tree 
				   if(curRecentGroupChat!= null && curRecentGroupChat.moveToFirst()){
						
					   
					   Logger.d(TAG,"getRecentChats() : getting data from curRecentGroupChat");
					   do{
						   String msgID = curRecentGroupChat.getString(curRecentGroupChat.getColumnIndex(RichMessagingData.KEY_ID));
	                       String contact = curRecentGroupChat.getString(curRecentGroupChat.getColumnIndex(RichMessagingData.KEY_CONTACT)); 
	                       
	                       String chat_id = curRecentGroupChat.getString(curRecentGroupChat.getColumnIndex(RichMessagingData.KEY_CHAT_ID));
	                       //String dateTimeStamp = curRecentGroupChat.getString(curRecentGroupChat.getColumnIndex(RichMessagingData.KEY_TIMESTAMP)); 
	                       
	                       Logger.d(TAG,"getRecentChats() : contact - "+ contact + " ; msgID - "+msgID+"; chat_id :"+ chat_id);
	                       
	                       
	                       
	                       //formatting not required for grp contact case
	                  
	                       Logger.d(TAG,"getRecentChats() : contact - "+ contact +" : added to map");
	                       
	                       ChatListProvider chatList =  createChatList(msgID,contact,chat_id, 1);
	                       
	                       recentChatMap.put(msgID,chatList);
	                       
						   
					   }while(curRecentGroupChat.moveToNext());
		
					   if(curRecentGroupChat!=null){
						   curRecentGroupChat.close();
						   curRecentGroupChat = null;
					   }
				   } 
				   
				   
				   
				   Logger.d(TAG,"getRecentChats() : "+ "iterating through map");
				   //as the tree map sorts the componenet based on key value so 
				   //if we iterate through it the most recent values are fetched
				 		//and add them in the result
					//give top 20 recent records
				   Integer count = 0;
				   for (Map.Entry entry : recentChatMap.entrySet()) {
					   		if(count == 20){
					   			break;
					   		}
							ChatListProvider chatlistProvider = (ChatListProvider)entry.getValue();
							
							
							result.add(chatlistProvider);
							count++;
							Logger.d(TAG,"getRecentChats() msg number "+count +" : "+ chatlistProvider.getMessageId());
							for(Participant participant : chatlistProvider.getParticipantlist()){
								Logger.d(TAG,"getRecentChats() msg number "+count +" : "+ participant.getContact());
							}
				   }	

				   
				   
				   
			   }catch(Exception e){
				   e.printStackTrace();
				   
			   }finally{
				   
				   Logger.d(TAG,"getRecentChats() closing curRecentO2OChat and curRecentGroupChat cursors");
				   //close the cursors
				   if(curRecentO2OChat!=null)
					   curRecentO2OChat.close();
				   
				   if(curRecentGroupChat!=null)
					   curRecentGroupChat.close();
			   }
			 
			 
			 return result;	
		}
		
		
		private ChatListProvider createChatList(String msgID,String contact,String chat_id, Integer mode){
			 
			 ChatListProvider chatList = null;
             chatList = new ChatListProvider();
             
             if(mode == 0){ // one to one chat case
            	 
            	 String contactName = contact;
    			 Participant participant = null;
    			 participant =  new Participant(contact, contactName); 
                 
                 ArrayList<Participant> listParticipant = new ArrayList<Participant>();
                 listParticipant.add(participant);
                 
                 chatList.setMessageId(Integer.parseInt(msgID)); //set message id
                 chatList.setParticipantlist(listParticipant); // add the participant list
                 chatList.setChatId(chat_id);
             }
             else if(mode == 1){//group chat case
            	 
            	 //for group chat case the contacts are semicolmn seperated so we need to fetch individual values 
            	 ArrayList<Participant> listParticipant = new ArrayList<Participant>();
            	 
            	 List<String> participantNames =  Arrays.asList(contact.split(";"));
                 
					int size = participantNames.size();
					for(int i=0; i<size; i++){
						
						String contactName = participantNames.get(i);
						Participant participant = null;
		    			participant =  new Participant(participantNames.get(i), contactName);	
		    			listParticipant.add(participant);
					}
					 chatList.setMessageId(Integer.parseInt(msgID)); //set message id
	                 chatList.setParticipantlist(listParticipant); // add the participant list	
	                 chatList.setChatId(chat_id);
             }          
           return chatList;
		}
		
        public boolean isChatPresent(String chatId) {
        boolean result = false;
        String[] project = new String[] {
        RichMessagingData.KEY_STATUS
        };
        String selection = "(" + RichMessagingData.KEY_CHAT_ID + "='" + chatId
                + "')";
        Cursor cursor = cr.query(databaseUri, project, selection, null, null);
        try {
            if (cursor != null) {
                int count = cursor.getCount();
                result = (count != 0 ? true : false);
                
                Logger.d(TAG,"isChatExisted() count: " + count);
            }

        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return result;
    }

        /* delete the group chat + messages */
        public void deleteGroupChat(String chatID){
        	
        	Logger.d(TAG,"deleteGroupChat() : for chat_id = "+ chatID);
        	
        	String includeGroupChat = RichMessagingData.KEY_TYPE + " = " + EventsLogApi.TYPE_GROUP_CHAT_SYSTEM_MESSAGE + " OR " +
        			RichMessagingData.KEY_TYPE + " = " + EventsLogApi.TYPE_INCOMING_GROUP_CHAT_MESSAGE + " OR  " +
        			RichMessagingData.KEY_TYPE + " = " + EventsLogApi.TYPE_OUTGOING_GROUP_CHAT_MESSAGE;
        		
        		
        		// Delete entries
        		int deletedRows = cr.delete(databaseUri, 
        				RichMessagingData.KEY_CHAT_ID + " = '" + chatID + "' AND ( " + includeGroupChat + " )", 
        				null);
        		Logger.d(TAG,"deleteGroupChat() : deleted rows "+ deletedRows);	
        }


        //delete a message from the datbase by msgID
        public void deleteMessage(String msgID){
		    String messageID = "";
        	Cursor cursor = null;			
			try {
				messageID = msgID + "";
			      //get the cursor chats for a group
				int deletedRows = cr.delete(databaseUri,
						RichMessagingData.KEY_MESSAGE_ID + " = '" + messageID+"'", 
						null);
			} 
			catch (Exception e) {
				Logger.d(TAG,"deleteMessage() : EXCEPTION :"+ e.getMessage());
				// TODO: handle exception
			}
        }
}


