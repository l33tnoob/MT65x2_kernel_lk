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

package com.mediatek.bluetooth.map.mime;

import java.util.HashSet;

public class MimeContent{
	public static final String DATE = "Date"; 
	public static final String SUBJECT = "Subject"; 
	public static final String MESSAGE_ID = "Message-ID"; 
	public static final String FROM = "From"; 
	public static final String TO = "To"; 
	public static final String CC = "Cc"; 
	public static final String BCC = "Bcc"; 
	public static final String REPLY_TO = "Reply-To"; 
	public static final String MIME_VERSION = "MIME-Version"; 	
	public static final String CONTENT_TYPE = "Content-Type"; 
	public static final String BOUNDARY = "boundary"; 
	
	public static final String CONTENT_TRANSFER_ENCONDING = "Content-Transfer-Encoding";  
	public static final String CONTENT_TRANSFER_ENCONDING_8_BIT = "8bit";  
	public static final String CONTENT_TRANSFER_ENCONDING_BASE_64 = "base64";  
	public static final String CONTENT_DISPOSITION = "Content-Disposition"; 
	public static final String CONTENT_LOCATION = "Content-Location"; 
	public static final String CONTENT_ID = "Content-ID"; 

	public static final String FILE_NAME = "filename";
	public static final String NAME = "name";

	public static final String CHARSET = "charset"; 
	
	public static final String TEXT_PLAIN        = "text/plain";
  
	public static final String SIZE = "size"; 

	public static final String CONTNET_DISPOSITION_INLINE = "inline";
  
	private static HashSet<String> mHeaderField;
	static {
		mHeaderField = new HashSet<String>();
		mHeaderField.add(DATE);
		mHeaderField.add(SUBJECT);
		mHeaderField.add(MESSAGE_ID);
		mHeaderField.add(FROM);
		mHeaderField.add(TO);		
		mHeaderField.add(CC);		
		mHeaderField.add(BCC);		
		mHeaderField.add(REPLY_TO);		
		mHeaderField.add(MIME_VERSION);		
		mHeaderField.add(CONTENT_TYPE);		
		mHeaderField.add(BOUNDARY);	
	}

	public static boolean isHeaderField(String field) {
		return field != null && mHeaderField.contains(field);
	}
	
}
