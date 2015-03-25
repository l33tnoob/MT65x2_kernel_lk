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

package com.orangelabs.rcs.utils;

import com.orangelabs.rcs.utils.logger.Logger;

/**
 * A URN for emergency and other well-known services.
 * It will translate the call number to URN.
 */
public abstract class UrnTranslator {
	/**
	 * class name tag.
	 */
	private static final String TAG = "UrnTranslator";

	/**
	 * The logger
	 */
	private static final Logger sLogger = Logger.getLogger(TAG);
	/**
     * Emergency services
     */
    protected static final String URN_SERVICE_SOS = "urn:service:sos";
    
    /**
     * Ambulance service
     */
    protected static final String URN_SERVICE_SOS_AMBULANCE = "urn:service:sos.ambulance";
    
    /**
     * Animal control
     */
    protected static final String URN_SERVICE_SOS_ANIMAL_CONTROL = "urn:service:sos.animal-control";
    
    /**
     * Fire service
     */
    protected static final String URN_SERVICE_SOS_FIRE = "urn:service:sos.fire";
    
    /**
     * Gas leaks and gas emergencies
     */
    protected static final String URN_SERVICE_SOS_GAS = "urn:service:sos.gas";
    
    /**
     * Maritime search and rescue
     */
    protected static final String URN_SERVICE_SOS_MARINE = "urn:service:sos.marine";
    
    /**
     * Mountain rescue
     */
    protected static final String URN_SERVICE_SOS_MOUNTAIN = "urn:service:sos.mountain";
    
    /**
     * Physician referral service
     */
    protected static final String URN_SERVICE_SOS_PHYSICIAN = "urn:service:sos.physician";
        
    /**
     * Poison control center
     */
    protected static final String URN_SERVICE_SOS_POISON = "urn:service:sos.poison";
    
    /**
     * Police, law enforcement
     */
    protected static final String URN_SERVICE_SOS_POLICE = "urn:service:sos.police";
    
    /**
     * Counseling services
     */
    protected static final String URN_SERVICE_COUNSELING = "urn:service:counseling";
    /**
     * Counseling for children
     */
    protected static final String URN_SERVICE_COUNSELING_CHILDREN = "urn:service:counseling.children";
    /**
     * Mental health counseling
     */
    protected static final String URN_SERVICE_COUNSELING_MENTAL_HEALTH = "urn:service:counseling.mental-health";
    
    /**
     * Suicide prevention hotline
     */
    protected static final String URN_SERVICE_COUNSELING_SUICIDE = "urn:service:counseling.suicide";
    
    /**
     * 
     * @param number
     * @return
     */
    protected abstract String translateNumberToURN(String number);
    
    /**
     * 
     * Translate American emergency number to URN.
     *
     */
    public static class AmericaUrnTranslator extends UrnTranslator{
    	/**
    	 * Emergency number of America.
    	 */
    	private static final String EMERGENCY_NUMBER = "911";
    	
		@Override
		protected String translateNumberToURN(String number) {
			String resultUrn = "";
			if (null == number) {
				if (sLogger.isActivated()) {
					sLogger.debug("translateNumberToURN: The number is null.");
				}
				return resultUrn;
			}
			
			if (EMERGENCY_NUMBER.equals(number)) {
				resultUrn = URN_SERVICE_SOS;
			}
			
			return resultUrn;
		}
    	
    }
    
    /**
     * 
     * Translate China emergency number to URN.
     *
     */
    public static class ChinaUrnTranslator extends UrnTranslator{
    	/**
    	 * Emergency number of China.
    	 */
    	private static final String POLICE = "110";
    	private static final String FIRE = "119";
    	private static final String EMERGENCY_CENTER = "120";
    	private static final String TRAFFIC_ACCIDENT = "122";
    	private static final String PHONE_NUMBER_QUERY = "114";
    	private static final String PHONE_STOPPAGE = "112";
    	private static final String WEATHER_FORECAST = "12121";
    	private static final String TIME_SERVICE = "12117";

		@Override
		protected String translateNumberToURN(String number) {
			String resultUrn = "";
			
			if (null == number) {
				if (sLogger.isActivated()) {
					sLogger.debug("translateNumberToURN: The number is null.");
				}
				return resultUrn;
			}
			
			if (POLICE.equals(number)) {
				resultUrn = URN_SERVICE_SOS_POLICE;
			}else if (FIRE.equals(number)) {
				resultUrn = URN_SERVICE_SOS_FIRE;
			}else if (EMERGENCY_CENTER.equals(number)) {
				resultUrn = URN_SERVICE_SOS_AMBULANCE;
			}else if (TRAFFIC_ACCIDENT.equals(number)) {
				resultUrn = URN_SERVICE_SOS;
			}else if (PHONE_NUMBER_QUERY.equals(number)) {
				resultUrn = URN_SERVICE_COUNSELING;
			}else if (PHONE_STOPPAGE.equals(number)) {
				resultUrn = URN_SERVICE_COUNSELING;
			}else if (WEATHER_FORECAST.equals(number)) {
				resultUrn = URN_SERVICE_COUNSELING;
			}else if (TIME_SERVICE.equals(number)) {
				resultUrn = URN_SERVICE_COUNSELING;
			}
			
			return resultUrn;
		}
    	
    }
}