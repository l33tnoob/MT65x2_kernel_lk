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

package com.mediatek.bluetooth.map.util;
import com.mediatek.bluetooth.map.MAP;
import android.util.Log;


public class HandleUtil{
	private final static String TAG = "HandleUtil";

	public static final long EMAIL_HANDLE_BASE			= 0x0000000000000000L; 
	public static final long SMS_GSM_HANDLE_BASE		= 0x1000000000000000L; 
	public static final long SMS_CDMA_HANDLE_BASE		= 0x2000000000000000L; 
	public static final long MMS_HANDLE_BASE			= 0x4000000000000000L;
	public static final long MESSAGE_HANDLE_MASK		= 0x0FFFFFFFFFFFFFFFL;
	
	public static int getMessageType(long handle) {
		if ((handle & MMS_HANDLE_BASE) > 0) {
			return MAP.MSG_TYPE_MMS;
		}
		if ((handle & SMS_GSM_HANDLE_BASE) > 0){
			return MAP.MSG_TYPE_SMS_GSM;
		}
		if ((handle & SMS_CDMA_HANDLE_BASE) > 0){
			return MAP.MSG_TYPE_SMS_CDMA;
		}
		if ((handle & EMAIL_HANDLE_BASE) == 0){
			return MAP.MSG_TYPE_EMAIL;
		}
		Log.v(TAG,"the handle seems abnormal : "+ handle);
		return 0;
	}
	
	public static long getHandle(int type, long msgId) {
		long id = msgId & MESSAGE_HANDLE_MASK;
		switch(type) {
			case MAP.MSG_TYPE_EMAIL:
				return id | EMAIL_HANDLE_BASE;
			case MAP.MSG_TYPE_MMS:
				return id | MMS_HANDLE_BASE;
			case MAP.MSG_TYPE_SMS_CDMA:
				return id | SMS_CDMA_HANDLE_BASE;
			case MAP.MSG_TYPE_SMS_GSM:
				return id | SMS_GSM_HANDLE_BASE;
			default:
				Log.v(TAG, "unexpected type : "+ type);
				return -1;
		}
	}
	public static long getId(long handle) {
		return handle & MESSAGE_HANDLE_MASK;		
	}

	public static long getInvalidId(int type) {
		switch(type) {
			case MAP.MSG_TYPE_EMAIL:
				return EMAIL_HANDLE_BASE - 1;
			case MAP.MSG_TYPE_MMS:
				return MMS_HANDLE_BASE -1;
			case MAP.MSG_TYPE_SMS_CDMA:
				return SMS_CDMA_HANDLE_BASE -1;
			case MAP.MSG_TYPE_SMS_GSM:
				return SMS_GSM_HANDLE_BASE -1;
			default:
				Log.v(TAG, "unexpected type : "+ type);
				return -1;
		}
	}

}
