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
import android.util.Log;
public class VCard{
	private final String TAG = "VCard";
	private static final String BEGIN = "BEGIN:VCARD";
	private static final String END = "END:VCARD";
	private static final String VERSION = "VERSION";
	private static final String NAME = "N";
	private static final String FORMAT_NAME = "FN";
	private static final String TELEPHONE = "TEL";
	private static final String EMAIL = "EMAIL";	
	private static final String CRLF = "\r\n";
	private static final String SEPRATOR = ":";
	public static final String VERSION_21 = "2.1";
	public static final String VERSION_30 = "3.0";	
	
	
	private String mVersion = "2.1"; //defaut 2.1
	private String mName;
	private String mFormatName;
	private String mTelephone;	
	private String mEmail;

	public VCard(String version){
		if (version.equals(VERSION_21) || version.equals(VERSION_30)) {
			mVersion = version;
		} else {
			mVersion = VERSION_21;
		}
	}
	public VCard(){
		mVersion = VERSION_21;		
	}
	public void setName(String name) {
		mName = name;
	}
	public void setFormatName(String name) {
		mFormatName = name;
	}
	public void setTelephone(String tel){
		mTelephone = tel;
	}
	public void setEmail(String email){
		mEmail = email;
	}

	public void reset(){
		mEmail = null;
		mTelephone = null;
		mFormatName = null;
		mName = null;
	}

	public String getName(){
		return mName;
	}
	public String getFormatName(){
		return mFormatName;
	}
	public String getTelephone(){
		return mTelephone;
	}
	public String getEmail(){
		return mEmail;
	}

	public String toString(){
		StringBuilder vCard = new StringBuilder();
		vCard.append(BEGIN);
		vCard.append(CRLF);

		//version
		vCard.append(VERSION);
		vCard.append(SEPRATOR);
		vCard.append(mVersion);		
		vCard.append(CRLF);
		
		//N (name) is neccessary
		vCard.append(NAME);
		vCard.append(SEPRATOR);
		if (mName != null) {
			vCard.append(mName);
		}
		vCard.append(CRLF);

		if (mVersion.equals(VERSION_30)) {
			//FN (name) is neccessary in vCard 3.0
			vCard.append(FORMAT_NAME);
			vCard.append(SEPRATOR);
			if (mName != null) {
				vCard.append(mFormatName);
			}
			vCard.append(CRLF);
		}

		if (mTelephone != null) {
			vCard.append(TELEPHONE);
			vCard.append(SEPRATOR);
			vCard.append(mTelephone);		
			vCard.append(CRLF);
		}
		if (mEmail != null) {
			vCard.append(EMAIL);
			vCard.append(SEPRATOR);
			vCard.append(mEmail);		
			vCard.append(CRLF);
		}
		
		vCard.append(END);
		return vCard.toString();
	}
	public void parse(String vcard){
		if (vcard == null) {
			return;
		}
		String[] elements = vcard.split(CRLF);
		for(String element: elements) {
			String[] item = element.split(SEPRATOR);
			if (item.length < 2) {
				continue;
			}
			String key = item[0].trim();
			String value = item[1].trim();
			if (key.equals(NAME)) {
				mName = value;
			} else if (key.equals(FORMAT_NAME)) {
				mFormatName = value;
			} else if (key.equals(TELEPHONE)) {
				mTelephone = value;
			} else if (key.equals(EMAIL)) {
				mEmail = value;
			} else {
				log("unrecognized key:"+key);
			}
		}
		
	}

	private void log(String info){
		if (null != info){
			Log.v(TAG, info);
		}
	}
	
}

