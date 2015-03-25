/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package android.bluetooth;

import android.util.Log;

import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import org.xmlpull.v1.XmlPullParser;
import android.util.Xml;
import java.util.ArrayList;

public class ConfigHelper{
	private static final String TAG = "ConfigHelper";
	private static final boolean DBG = false;

	private static ArrayList<ProfileConfig> profileList = new ArrayList<ProfileConfig>();
	private static boolean mInit = false;
	
	public static void readXML(String xmlFileName)
	{
		InputStream in;
	    File xmlFile = new File(xmlFileName);
		XmlPullParser parser = Xml.newPullParser();

		if(mInit == true)
		{
			Log.d(TAG, "btconfig.xml has be read!\n");
			return;
		}
		
		try{
			in = new FileInputStream(xmlFile);
		}catch(FileNotFoundException e){
			log("readXML Failed: " + e);
			return;
		}
		
		try{
			String strName = null;
			String strValue = null;
			ProfileConfig profileConfig = null;
			parser.setInput(in, "UTF-8");

			int eventType = parser.getEventType();
			while(eventType != XmlPullParser.END_DOCUMENT){
				switch(eventType){
					case XmlPullParser.START_DOCUMENT:
						log("Xml Start Document!\n");
						break;
					case XmlPullParser.START_TAG:
						strName = parser.getName();
						log("[START TAG]: " + strName);
						if(strName != null && strName.equalsIgnoreCase("profile"))
						{
							profileConfig = new ProfileConfig();
						}
						break;
					case XmlPullParser.TEXT:
						if(parser.isWhitespace())
						{
							log("[TAG]: " + strName + " [TEXT] is whitespace!\n");
							break;
						}
						strValue = parser.getText();
						if(strName != null && strName.equalsIgnoreCase("name"))
						{
							profileConfig.setProfileID(strValue);
						}
						else if(strName != null && strName.equalsIgnoreCase("value")) {
							if(strValue.compareTo("true") == 0)
								profileConfig.setProfileEnabled(true);
							else
								profileConfig.setProfileEnabled(false);
						}
						log("[TAG]: " + strName + " [TEXT] is " + strValue);
						break;
					case XmlPullParser.END_TAG:
						strName = parser.getName();
						log("[END TAG]: " + strName);
						if(strName != null && strName.equalsIgnoreCase("profile"))
						{
							profileList.add(profileConfig);
						}
						break;
				}
				eventType = parser.next();
				log("Get Next EventType!\n");
			}

			in.close();
			mInit = true;
		}catch(Exception e){
			log("readXML Failed: " + e);
		}
	}

	public static boolean checkSupportedProfiles(String id)
	{
		readXML("/system/etc/bluetooth/btconfig.xml");
		ProfileConfig[] profiles = new ProfileConfig[profileList.size()];
			
		profileList.toArray(profiles);
		for(int i = 0; i < profiles.length; i++)
		{
			if(profiles[i].getProfileID().equals(id))
			{
				log("Advanced " + profiles[i].getProfileID()+ " Enabled!\n");
				return profiles[i].getProfileEnabled();
			}
		}
		log("Advanced " + id + " Disabled!\n");
		return false;
	}

	public static boolean isAdvancedProfileEnabled()
	{
		readXML("/system/etc/bluetooth/btconfig.xml");
		ProfileConfig[] profiles = new ProfileConfig[profileList.size()];
			
		profileList.toArray(profiles);
		for(int i = 0; i < profiles.length; i++)
		{
			if(profiles[i].getProfileEnabled())
			{
				log("Advanced Profile Enabled!\n");
				return true;
			}
		}
		log("Advanced Profile Disabled!\n");
		return false;
	}

	private static final String[] ADVANCED_SETTING_IDS = {
		ProfileConfig.PROFILE_ID_FTP,
		ProfileConfig.PROFILE_ID_SIMAP,
		ProfileConfig.PROFILE_ID_PRXR,
		ProfileConfig.PROFILE_ID_MAPS
	};
	
	public static boolean isAdvanceSettingEnabled()
	{
		readXML("/system/etc/bluetooth/btconfig.xml");
		ProfileConfig[] profiles = new ProfileConfig[profileList.size()];
			
		profileList.toArray(profiles);
		for(int j = 0; j < ADVANCED_SETTING_IDS.length; j++)
		{
			for(int i = 0; i < profiles.length; i++)
			{
				if(profiles[i].getProfileID().equals(ADVANCED_SETTING_IDS[j]))
				{
					if(profiles[i].getProfileEnabled())
					{
						log("Advanced Settings Enabled!\n");
						return true;
					}
				}
			}
		}
		log("Advanced Settings Disabled!\n");
		return false;
	}

	private static void log(String msg)
	{
		if(DBG)
			Log.d(TAG, msg);
	}
}

