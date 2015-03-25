/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth.map;

//MAP should be upcase, or else conflict with Map
public class MAP {
	
	public final static String MAP_SETTING_TAG = "BLUETOOTH_MAP_SETTING";
	public final static String ACCOUNT_ID_SETTING = "ACCOUNT_ID_SETTING";
	public final static String SIM_ID_SETTING = "SIM_ID_SETTING";

	
	
	
	//message list request 
	//mask
	public static final int Message_List_Mask_Subject          = 0x0000;
	public static final int Message_List_Mask_Datatime         = 0x0001;
	public static final int Message_List_Mask_SenderName       = 0x0002;
	public static final int Message_List_Mask_SenderAddress    = 0x0004;
	public static final int Message_List_Mask_ReplyAddress     = 0x0008;
	public static final int Message_List_Mask_RecipientName    = 0x0010;
	public static final int Message_List_Mask_Recipientaddress = 0x0020;
	public static final int Message_List_Mask_Type             = 0x0040;
	public static final int Message_List_Mask_Size             = 0x0080;
	public static final int Message_List_Mask_Text             = 0x0100;
	public static final int Message_List_Mask_RecipientStatus  = 0x0200;
	public static final int Message_List_Mask_AttachmentSize   = 0x0400;
	public static final int Message_List_Mask_Priority         = 0x0800;
	public static final int Message_List_Mask_Read             = 0x1000;
	public static final int Message_List_Mask_Sent             = 0x2000;
	public static final int Message_List_Mask_Protected        = 0x4000;
	//message type
	public static final String MESSAGE_TYPE_EMAIL				= "Email";
	public static final String MESSAGE_TYPE_SMS_GSM				= "SMS_GSM";
	public static final String MESSAGE_TYPE_CDMA				= "SMS_CDMA";
	public static final String MESSAGE_TYPE_MMS 					= "MMS";

	public static final int MSG_TYPE_SMS_GSM			= 0x01;	// Emails on RFC2822 or MIME type basis
	public static final int MSG_TYPE_SMS_CDMA			= 0x02;	// GSM short messages
	public static final int MSG_TYPE_EMAIL	 			= 0x04;	// CDMA short messages
	public static final int MSG_TYPE_MMS				= 0x08;	// 3GPP MMS messages
	public static final int MSG_TYPE_ALL				= 0x00;	// 3GPP MMS message

	//handle base 

	//priority
	public static final int PRIORITY_STATUS_NO_FILTERING 	= 0;
	public static final int PRIORITY_STATUS_HIGH 			= 1;
	public static final int PRIORITY_STATUS_NON_HIGH 		= 2;
	
	

	//charset
	public static final int CHARSET_NATIVE 					= 0;
	public static final int CHARSET_UTF8						= 1;	

	//fraction message request
	public static final int FRACTION_REQUEST_FIRST 			= 0;
	public static final int FRACTION_REQUEST_NEXT			= 1;	

	//fraction deliver response
	public static final int FRACTION_DELIVER_MORE 			= 0;
	public static final int FRACTION_DELIVER_LAST			= 1;	
	public static final int FRACTION_DELIVER_NO				= 2;

	//read status
	public static final int UNREAD_STATUS                  	= 1;
	public static final int READ_STATUS			= 2;
	public static final int ALL_READ_STATUS  	= 0;
	//max length for subject is 256 in MAP spec. 
	//But the actual string length exceed 256 bytes and cut it to 256 bytes, no terminal char('\0') exist 
	public static final int MAX_SUBJECT_LEN					= 254;
	public static final int MAX_FOLDER_LEN						= 256;


	//recepient status
	public static final int RECEPIENT_STATUS_COMPLETE 			= 0;
	public static final int RECEPIENT_STATUS_FRACTIONED 		= 1;
	public static final int RECEPIENT_STATUS_NOTIFICATION 		= 2;

	//send report event :
	public static final int EVENT_NEW_MESSAGE 					= 0;
	public static final int EVENT_DELIVERY_SUCCESS 				= 1;
	public static final int EVENT_SEND_SUCCESS  				= 2;
	public static final int EVENT_DELIVERY_FAILURE 				= 3;
	public static final int EVENT_SEND_FAILURE  				= 4;
	public static final int EVENT_MEMORY_FULL  					= 5;	
	public static final int EVENT_MEMORY_AVAILABLE  			= 6;
	public static final int EVENT_MESSAGE_DELETED  				= 7;	
	public static final int EVENT_MESSAGE_SHIFT  				= 8;
            

	//foler definition
	class Mailbox {
	public static final String TELECOM							= "telecom";
	public static final String MSG 								= "msg";
	public static final String INBOX 							= "inbox";
	public static final String OUTBOX 							= "outbox";
	public static final String SENT 							= "sent";
	public static final String DELETED		 					= "deleted";	
	public static final String DRAFT	 						= "draft";
	}
	
	public static final String ROOT_PATH	 					= "/data/@btmtk/profile/map";
	

	public static final int MAX_MSG_ENVELOPE_NUM				= 3;

	//language
	public static final int LANG_ENGLISH 						= 0;		// For CDMA-SMS
	public static final int LANG_FRENCH						= 1;			// For CDMA-SMS
	public static final int LANG_SPANISH						= 2;// For CDMA-SMS and GSM-SMS
	public static final int LANG_JAPANESE						= 3;			// For CDMA-SMS
	public static final int LANG_KOREAN						= 4;			// For CDMA-SMS
	public static final int LANG_CHINESE						= 5;			// For CDMA-SMS
	public static final int LANG_HEBREW						= 6;			// For CDMA-SMS
	public static final int LANG_TURKISH						= 7;			// For GSM-SMS
	public static final int LANG_PORTUGUESE					= 8;		// For GSM-SMS
	public static final int LANG_UNKNOWN						= 9;			// For GSM-SMS and CDMA-SMS


	//encoding
	public static final int ENCODING_8BIT 						= 0;			// For Email/MMS : 8-Bit-Clean encoding
	public static final int ENCODING_G7BIT						= 1;			// For GSM-SMS : GSM 7 bit Default Alphabet
	public static final int ENCODING_G7BITEXT					= 2;			// For GSM-SMS : GSM 7 bit Alphabet with national language extension 
	public static final int ENCODING_GUCS2						= 3;			// For GSM-SMS
	public static final int ENCODING_G8BIT						= 4;			// For GSM-SMS 
	public static final int ENCODING_C8BIT						= 5;			// For CDMA-SMS : Octet, unspecified
	public static final int ENCODING_CEPM						= 6;				// For CDMA-SMS : Extended Protocol Message
	public static final int ENCODING_C7ASCII					= 7;			// For CDMA-SMS : 7-bit ASCII
	public static final int ENCODING_CIA5						= 8;				// For CDMA-SMS : IA5
	public static final int ENCODING_CUNICODE					= 9;		// For CDMA-SMS : UNICODE
	public static final int ENCODING_CSJIS						= 10;				// For CDMA-SMS : Shift-JIS
	public static final int ENCODING_CKOREAN					= 11;			// For CDMA-SMS : Korean
	public static final int ENCODING_CLATINHEB					= 12;		// For CDMA-SMS : Latin/Hebrew
	public static final int ENCODING_CLATIN					= 13;			// For CDMA-SMS : Latin


	//event report result	
	public static final int RESULT_OK							= 1;			
	public static final int RESULT_ERROR						= 0;			

	//priority attribute
	public static final boolean HIGH_PRIORITY						= true;
	public static final boolean LOW_PRIORITY					= false;

	//message status operation
	public static final int STATUS_SWITCH_READ					= 0;
	public static final int STATUS_SWITCH_DELETE				= 1;

	//message status value	
	public static final int STATUS_SWITCH_NO				= 0;
	public static final int STATUS_SWITCH_YES				= 1;
	
	//delete message value	
	public static final int DELETE_MESSAGE				= 0;
	public static final int RESTORE_MESSAGE				= 1;
	
	//TODO: we should check the network type(GSM/CDMA?)
	public static boolean isMessageTypeValid(int type){
		switch(type){
			case MSG_TYPE_EMAIL:
			case MSG_TYPE_MMS:
			case MSG_TYPE_SMS_CDMA:
			case MSG_TYPE_SMS_GSM:
				return true;
			default:
				return false;
		}
	}
}

