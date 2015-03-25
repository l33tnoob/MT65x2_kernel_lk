/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.telephony;

import java.util.Arrays;
import java.util.Locale;

import android.text.Editable;
import android.text.SpannableStringBuilder;
import android.text.Selection;
import android.telephony.Rlog;
import com.mediatek.common.featureoption.FeatureOption;
import android.os.SystemProperties;
import com.android.internal.telephony.Phone;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import com.mediatek.telephony.TelephonyManagerEx;
import com.android.internal.telephony.PhoneConstants;


/**
 * MediaTek Phone Number Format Tool.
 * There are 22 country&region supported.
 * 
 * Country&Region	Code
 * 
 * China Mainland	+86
 * China Taiwan		+886
 * China Honkong	+852
 * China Macao		+853
 * England			+44
 *	France			+33
 *	Italy			+39
 *	Germany			+39
 *	Russian			+7
 *	India			+91
 *	Spain			+34
 *	Malaysia		+60
 *	Singapore		+65
 *	Indonesia		+62
 *	Thailand		+66
 *	Vietnam			+84
 *	Portugal		+351
 *	Poland			+48
 *	Australia		+61
 *	New Zealand		+64
 *  Brazil			+55
 *  Turkey			+90
 *  @hide
 */
public class PhoneNumberFormatUtilEx {
	
    public static final String TAG = "PhoneNumberFormatUtilEx";
    public static final boolean DEBUG = false;
    /**
     *  List of country codes for countries that use the NANP
     *  Need to SYNC google default PhoneNumberUtils if changes
     */
    private static final String[] NANP_COUNTRIES = new String[] {
        "US", // United States
        "CA", // Canada
        "AS", // American Samoa
        "AI", // Anguilla
        "AG", // Antigua and Barbuda
        "BS", // Bahamas
        "BB", // Barbados
        "BM", // Bermuda
        "VG", // British Virgin Islands
        "KY", // Cayman Islands
        "DM", // Dominica
        "DO", // Dominican Republic
        "GD", // Grenada
        "GU", // Guam
        "JM", // Jamaica
        "PR", // Puerto Rico
        "MS", // Montserrat
        "MP", // Northern Mariana Islands
        "KN", // Saint Kitts and Nevis
        "LC", // Saint Lucia
        "VC", // Saint Vincent and the Grenadines
        "TT", // Trinidad and Tobago
        "TC", // Turks and Caicos Islands
        "VI", // U.S. Virgin Islands
    };
	
    /** The current locale is unknown, look for a country code or don't format */
    public static final int FORMAT_UNKNOWN = 0;
    /** NANP formatting */
    public static final int FORMAT_NANP = 1;
    public static final String[] NANP_INTERNATIONAL_PREFIXS = {"011"};
    /** Japanese formatting */
    public static final int FORMAT_JAPAN = 2;
    public static final String[] JAPAN_INTERNATIONAL_PREFIXS = {"010","001","0041","0061"};
    /**
     * China mainland +86 or 0086
     */
    public static final int FORMAT_CHINA_MAINLAND = 3;
    /**
     * China Hongkong +852 or 00852
     */
    public static final int FORMAT_CHINA_HONGKONG = 4;
    
    /**
     * it comes from "http://www.chahaoba.com/%E9%A6%99%E6%B8%AF"
     */
    public static final String[] HONGKONG_INTERNATIONAL_PREFIXS = {"001","0080","0082","009"};
    
    /**
     * China MACAU +853 or 00853
     */
    public static final int FORMAT_CHINA_MACAU = 5;
    
    /**
     * TAIWAN +886
     */
    public static final int FORMAT_TAIWAN = 6;
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Taiwan"
     */
    public static final String[] TAIWAN_INTERNATIONAL_PREFIXS = {"002","005","006","007","009","019"};
    
    public static final int FORMAT_ENGLAND = 7;
    
    public static final int FORMAT_FRANCE = 8;
    /**
     * it comes from "http://countrycode.org/france"
     */
    public static final String[] FRANCE_INTERNATIONAL_PREFIXS = {"00","40","50","70","90"};
    
    public static final int FORMAT_ITALY = 9;
    
    public static final int FORMAT_GERMANY = 10;
    
    public static final int FORMAT_RUSSIAN = 11;
    //TODO RUSSIAN INTERNATIONAL PREFIXS
    
    
    public static final int FORMAT_INDIA = 12;
    
    public static final int FORMAT_SPAIN = 13;
    
    public static final int FORMAT_MALAYSIA = 14;
    
    public static final int FORMAT_SINGAPORE = 15;
    
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Singapore"
     */
    public static final String[] SINGAPORE_INTERNATIONAL_PREFIXS = {"001","002","008","012","013","018","019"};
    
    public static final int FORMAT_INDONESIA = 16;
    
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Indonesia"
     */
    public static final String[] INDONESIA_INTERNATIONAL_PREFIXS = {"001","007","008","009"};
    
    public static final int FORMAT_THAILAND = 17;
    
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Thailand"
     */
    public static final String[] THAILAND_INTERNATIONAL_PREFIXS = {"001","004","005","006","007","008","009"};
    
    public static final int FORMAT_VIETNAM = 18;
    
    public static final int FORMAT_PORTUGAL = 19;
    
    public static final int FORMAT_POLAND = 20;
    
    public static final int FORMAT_AUSTRALIA = 21;
    
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Australia"
     */
    public static final String[] AUSTRALIA_INTERNATIONAL_PREFIXS = {"0011","0014","0015","0016","0018","0019"};
    
    public static final int FORMAT_NEW_ZEALAND = 22;
    
    
    public static final int FORMAT_BRAZIL = 23;
    
    /**
     * it comes from "http://en.wikipedia.org/wiki/Telephone_numbers_in_Brazil"
     */
    public static final String[] BRAZIL_INTERNATIONAL_PREFIXS = {"0012","0014","0015","0021","0023","0025","0031","0041"};
    
    public static final int FORMAT_TURKEY = 24;
    
    /**
     * ***Warning****
     * this country code array index matches FORMAT_XXXXX - 1.
     * so if you add value, you must add new FORMAT_XXXXX, and the index still matches it.
     */
    public static String[] FORMAT_COUNTRY_CODES = {
    	"1",	"81",	"86",	"852",	"853",	"886", "44",	
    	"33",	"39",	"49",	"7",	"91",	"34",  "60",
    	"65",	"62",	"66",	"84",	"351",	"48",  "61",
    	"64",	"55",	"90",
    };
    
    /**
     * the country Alpha-2.
     * reference: "http://en.wikipedia.org/wiki/ISO_3166-1_alpha-2"
     * ****Warning****
     * this names match FORMAT_COUNTRY_CODES, and the index is also FORMAT_XXXXX - 1.
     * so if you add value, you must add new FORMAT_XXXXX, and the index still matches it.
     */
    public static final String[] FORMAT_COUNTRY_NAMES = {
    	"US",	"JP",	"CN",	"HK",	"MO",	"TW",	"GB",
    	"FR",   "IT",	"DE",	"RU",	"IN",	"ES",	"MY",
    	"SG",	"ID",	"TH",	"VN",	"PT",	"PL",	"AU",
    	"NZ",	"BR",	"TR",
    };
	
    /**
     * Returns the phone number formatting type for the given locale.
     *
     * @param locale The locale of interest, usually {@link Locale#getDefault()}
     * @return The formatting type for the given locale, or FORMAT_UNKNOWN if the formatting
     * rules are not known for the given locale
     * @hide
     */
    public static int getFormatTypeForLocale(Locale locale) {
//        String country = locale.getCountry();
        String simIso = getDefaultSimCountryIso();
        log("getFormatTypeForLocale Get sim sio:"+simIso);
        return getFormatTypeFromCountryCode(simIso);
    }

	/**
     * Get default sim country ISO. 
     * If the system is GEMINI System, the sim card is default sim, else the sim card is sim1; if the default sim is not inserted, select other sim.
     * 
     * Returns the ISO country code equivalent for the SIM provider's country code.
     * @return
     */
    /*package*/static String getDefaultSimCountryIso(){
       int simId;
       String iso = null;
       if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
            simId = SystemProperties.getInt(PhoneConstants.GEMINI_DEFAULT_SIM_PROP, -1);
             if (simId == -1) {// No default sim setting
                 simId = PhoneConstants.GEMINI_SIM_1;
             }
             if(!TelephonyManagerEx.getDefault().hasIccCard(simId)){
                 //simId = Phone.GEMINI_SIM_2 ^ simId;
                 if(TelephonyManagerEx.getDefault().hasIccCard(PhoneConstants.GEMINI_SIM_1)){
                     simId = PhoneConstants.GEMINI_SIM_1;
                 }else if (TelephonyManagerEx.getDefault().hasIccCard(PhoneConstants.GEMINI_SIM_2)){
                     simId = PhoneConstants.GEMINI_SIM_2;
                 }else if (PhoneConstants.GEMINI_SIM_NUM >=3 && TelephonyManagerEx.getDefault().hasIccCard(PhoneConstants.GEMINI_SIM_3)){
                     simId = PhoneConstants.GEMINI_SIM_3;
                 }else if (PhoneConstants.GEMINI_SIM_NUM >=4 && TelephonyManagerEx.getDefault().hasIccCard(PhoneConstants.GEMINI_SIM_4)){
                     simId = PhoneConstants.GEMINI_SIM_4;
                 }
             }
             iso = TelephonyManagerEx.getDefault().getSimCountryIso(simId);
       }else{
           iso = TelephonyManager.getDefault().getSimCountryIso();
       }      
       return iso;
	}

    
   private static int getFormatTypeFromCountryCodeInternal (String country) {
        // Check for the NANP countries
        int length = NANP_COUNTRIES.length;
        for (int i = 0; i < length; i++) {
            if (NANP_COUNTRIES[i].compareToIgnoreCase(country) == 0) {
                return FORMAT_NANP;
            }
        }
        if ("jp".compareToIgnoreCase(country) == 0) {
            return FORMAT_JAPAN;
        }
        return FORMAT_UNKNOWN;
    }
    /**
     * MediaTek extension for getting format type from country code.
     * @param  country Country code.
     * @return Return format type according to country parameter.
     * @hide
     */
    public static int getFormatTypeFromCountryCode(String country){
    	int type = FORMAT_UNKNOWN;
    	if (country != null && country.length() != 0) {
    		type = getFormatTypeFromCountryCodeInternal(country);
//    		type = PhoneNumberUtils.getFormatTypeFromCountryCode(country);
			if(type == FORMAT_UNKNOWN){
		    	int index = 0;
	            for(String name : FORMAT_COUNTRY_NAMES){
	    	        index++;
	    	        if(name.compareToIgnoreCase(country) == 0){
	    		        type = index;
	    		        break;
	    	        }
	            }
	            //for UK. which has two iso code
	            if(type == FORMAT_UNKNOWN && "UK".compareToIgnoreCase(country) == 0){
	    	        type = FORMAT_ENGLAND;
	            }
			}
	    }
    	log("Get Format Type:"+type);
    	return type;
    }
    
    /**
     * MediaTek format phone number.
     * @param text
     * @hide
     */
    public static String formatNumber(String source) {
    	Locale sCachedLocale;
        sCachedLocale = Locale.getDefault();
    	return formatNumber(source, getFormatTypeForLocale(sCachedLocale));
    }

    /**
     * MediaTek format phone number.
     * @param text
     * @param defaultFormattingType
     * @hide
     */
//   public static void formatNumber(Editable text, int defaultFormattingType){
//   	String result = formatNumber(text.toString(),defaultFormattingType);
//   	//text.append(result);
//		text.replace(0,text.length(),result);
//    }
    public static void formatNumber(Editable text, int defaultFormattingType){
        String result = formatNumber(text.toString(),defaultFormattingType);
    	if(result != null && !result.equals(text.toString())){
	    	//record the old cursor.
	    	int oldIndex = Selection.getSelectionStart(text);
	    	int digitCount = oldIndex;
	    	int i = 0;
	    	char c;   
	    	for(i=0;i<oldIndex;i++){
	        	c = text.charAt(i);
	        	if(c == ' ' || c == '-'){
	            		digitCount -- ;
	        	}
	        }
	    
	    	text.replace(0, text.length(), result);
	    
	    	//update the cursor to old cursor
	    	int count = 0;    
	        for(i=0;i < text.length() && count < digitCount ;i++){
	        	c = text.charAt(i);
	        	if(!(c == ' ' || c == '-')){
	            		count ++ ;
	        	}
	        }
	        Selection.setSelection(text, i);
	        //Rlog.d(TAG,"OldIndex: "+oldIndex+", digitCount: "+digitCount + ", newCursor: "+i);
    	}
    
    }

	/**
	 * check the input number is '0-9','-','*','#',' ','+'
	 * warning: now we don't support WILD,PAUSE,WAIT.
	 * @param text
	 * @return
	 */
	/*package*/static boolean checkInputNormalNumber(CharSequence text){
		boolean result = true;
		char c;
		for(int index=0;index< text.length();index++){
			c = text.charAt(index);
			if(!((c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+' || c == ' ' || c == '-')){
				result  = false;
				break;
			}
		}
		return result;
	} 
    
    /**
     * MediaTek format phone number.
     * @param text
     * @param defaultFormattingType
     * @return
     */
	public static String formatNumber(String text, int defaultFormattingType){
		log("MTK Format Number:"+text+" "+defaultFormattingType);
		if(!checkInputNormalNumber(text)){
			log("Abnormal Number:"+text+", do nothing.");
			return text;
		}
		text = removeAllDash(new StringBuilder(text));
		int formatType = (defaultFormattingType == FORMAT_UNKNOWN) ? FORMAT_NANP : defaultFormattingType;
		String result = text;
		if (text.length() > 2 && text.charAt(0) == '+') {
	        if (text.charAt(1) == '1') {
	            formatType = FORMAT_NANP;
	        } else if (text.length() >= 3 && text.charAt(1) == '8'&& text.charAt(2) == '1') {
	            formatType = FORMAT_JAPAN;
	        } else if (formatType == FORMAT_NANP || formatType == FORMAT_JAPAN){
		    	result = mtkFormatNumber(text,formatType);
		    	return result;
	        } 
	     }
		log("formatNumber:"+formatType);
		 switch (formatType) {
         case FORMAT_NANP:
         case FORMAT_JAPAN:
        	 result = PhoneNumberUtils.formatNumber(text, formatType);
        	 break;
         default:
        	 result = mtkFormatNumber(text,formatType);
		 }
		
		return result;
	}
	
	/**
	 * MediaTek Extension to format phone number.
	 * 
	 * @param text
	 * @param defaultFormatType
	 * @return
	 */
	/*package*/static String mtkFormatNumber(String text,int defaultFormatType){
		log("MTK Format Number:"+text+" "+defaultFormatType);
		int length = text.length();
		if(length<6){
			// The string is either a shortcode or too short to be formatted
			return text;
		}
		
      //check there are '*' or '#' in the number,remove all ' ' and '-', then return.
      //alps00036166 also check sip number '@'
		if(text.contains("*") || text.contains("#") || text.contains("@")){
			return removeAllDash(new StringBuilder(text));
		}
		
		int formatType = defaultFormatType;
		//Update format type from number.
		int[] match = getFormatTypeFromNumber(text, defaultFormatType);
		int startIndex = 0;
		if(match!=null && match[1] != FORMAT_UNKNOWN){
			formatType = match[1];
			startIndex = match[0];
		}
		//only input more 4 code and less 16 code follow country code.
		//the longest length of mobile phone brazil is 12, and maybe three '-'.
		if(length < startIndex+4 ){
			return text;
		}else if(length > startIndex + 15){
			return text;
		}
		String result = text;
		StringBuilder sb = new StringBuilder(text);
	    int blankPosition = -1;
	    // Strip the dashes first and add format blank, as we're going to add them back
	    blankPosition = removeAllDashAndFormatBlank(sb,startIndex);
		//there are may +886 9-11 ,delete 1, +886 9-1. so check again.
	    if(sb.length() < startIndex + 4 || sb.length() == startIndex + 4 && sb.charAt(blankPosition+1) == '0'){
	    	return sb.toString();
	    }
	    
		switch(formatType){
		case FORMAT_CHINA_MAINLAND:
			result = formatChinaNumber(sb,blankPosition);
			break;
		case FORMAT_CHINA_HONGKONG:
		case FORMAT_SINGAPORE:
			result = formatHeightLengthWithoutRegionCodeNumber(sb,blankPosition);
			break;
		case FORMAT_CHINA_MACAU:
			result = formatMacauNumber(sb,blankPosition);
			break;
		case FORMAT_NANP:
			SpannableStringBuilder ssb = null;
			if(blankPosition >= 0){
				ssb = new SpannableStringBuilder(sb.substring(startIndex+1));
				PhoneNumberUtils.formatNanpNumber(ssb);
				result = sb.substring(0,startIndex+1).concat(ssb.toString());
			}else{
				ssb = new SpannableStringBuilder(sb);
				PhoneNumberUtils.formatNanpNumber(ssb);
				result = ssb.toString();
			}
			break;
		case FORMAT_JAPAN:
			SpannableStringBuilder ssb2 = null;
			if(blankPosition >= 0){
				ssb2 = new SpannableStringBuilder(sb.substring(startIndex+1));
				PhoneNumberUtils.formatJapaneseNumber(ssb2);
				result = sb.substring(0,startIndex+1).concat(ssb2.toString());
			}else{
				ssb2 = new SpannableStringBuilder(sb);
				PhoneNumberUtils.formatJapaneseNumber(ssb2);
				result = ssb2.toString();
			}
			break;
		case FORMAT_TAIWAN:
			result = formatTaiwanNumber(sb,blankPosition);
			break;
		case FORMAT_VIETNAM:
			result = formatVietnamNubmer(sb,blankPosition);
			break;
		case FORMAT_PORTUGAL:
			result = formatPortugalNumber(sb,blankPosition);
			break;
		case FORMAT_POLAND:
			result = formatPolandNumber(sb,blankPosition);
			break;
		case FORMAT_AUSTRALIA:
			result = formatAustraliaNumber(sb,blankPosition);
			break;
		case FORMAT_NEW_ZEALAND:
			result = formatNewZealandNumber(sb,blankPosition);
			break;
		case FORMAT_THAILAND:
			result = formatThailandNumber(sb,blankPosition);
			break;
		case FORMAT_INDONESIA:
			result = formatIndonesiaNumber(sb,blankPosition);
			break;
		case FORMAT_MALAYSIA:
			result = formatMalaysiaNumber(sb,blankPosition);
			break;
		case FORMAT_SPAIN:
			result = formatSpainNumber(sb,blankPosition);
			break;
		case FORMAT_RUSSIAN:
			result = formatRussianNumber(sb,blankPosition);
			break;
		case FORMAT_GERMANY:
			result = formatGermanyNumber(sb,blankPosition);
			break;
		case FORMAT_INDIA:
			result = formatIndiaNumber(sb,blankPosition);
			break;
		case FORMAT_ITALY:
			result = formatItalyNumber(sb,blankPosition);
			break;
		case FORMAT_FRANCE:
			result = formatFranceNumber(sb,blankPosition);
			break;
		case FORMAT_ENGLAND:
			result = formatEnglandNumber(sb,blankPosition);
			break;
		case FORMAT_BRAZIL:
			result = formatBrazilNumber(sb,blankPosition);
			break;		    
		case FORMAT_TURKEY:
			result = formatTurkeyNumber(sb,blankPosition);	
			break;
		default:
			//move all ' ' and '-'
			result = removeAllDash(sb);
		}
		return result;
	}
	
	/**
	 * get the format from common number, the now country international prefix is STD '00' or '+'
	 * return a integer array, the length is 2, the first element is the internal number start index
	 * the second element is format value. if the format is unknown, the start index set 0.
	 * @param text the length of text must more or equals 5
	 * @return
	 */
	private static int[] getFormatTypeByCommonPrefix(String text){
		int result = FORMAT_UNKNOWN;
		int index = 0;
		int startIndex = 0;
		int[] match = new int[2];
		//is start with '00' or '+'
		if(text.length()>0 && text.charAt(0)=='+'){
			startIndex = 1;
		}else if(text.length()>1 && text.charAt(0)=='0' && text.charAt(1) == '0'){
			startIndex = 2;
		}
		if(startIndex != 0){
			for(String pattern : FORMAT_COUNTRY_CODES){
				index++;
				if(text.startsWith(pattern, startIndex)){
					result = index;
					startIndex = startIndex + pattern.length();
					break;
				}
			}
		}
		if(result == FORMAT_UNKNOWN){
			startIndex = 0;
		}
		match[0] = startIndex;
		match[1] = result;
		return match;
	}
	
	/**
	 * get the format from number, the now country international prefix is special in prefixs.
	 * return a integer array, the length is 2, the first element is the internal number start index
	 * the second element is format value. if the format is unknown, the start index set 0.
	 * @param text
	 * @param prefixs
	 * @return
	 */
	private static int[] getFormatNumberBySpecialPrefix(String text, String[] prefixs){
		int result = FORMAT_UNKNOWN;
		int index = 0;
		int startIndex = 0;
		int[] match = new int[2];
		//is start with '+'
		if(text.charAt(0) == '+'){
			startIndex = 1;
		}else{
			// is start with special prefix
			for(String prefix : prefixs){
				if(text.startsWith(prefix)){
					startIndex = prefix.length();
					break;
				}
			}
		}
		//is start with '+' or special prefix
		if(startIndex > 0){
			for(String pattern : FORMAT_COUNTRY_CODES){
				index++;
				if(text.startsWith(pattern, startIndex)){
					result = index;
					startIndex = startIndex + pattern.length();
					break;
				}
			}
		}
		if(result == FORMAT_UNKNOWN){
			startIndex = 0;
		}
		match[0] = startIndex;
		match[1] = result;
		return match;
	}
	
	/**
	 * get the format from number
	 * China mainland, China macao, England, Italy, Germany, India, Spain, Malaysia, Vietnam, Portugal, Poland, New Zealand is common prefix: '00' or '+'
	 * return a integer array, the length is 2, the first element is the internal number start index
	 * the second element is format value. if the format is unknown, the start index set 0.
	 * @param text
	 * @param defaultFormatType
	 * @param FORMAT_CHIAN_HONGKONG 
	 * @return
	 */
	private static int[] getFormatTypeFromNumber(String text, int defaultFormatType){
		int[] match = null;
		switch(defaultFormatType){
		case FORMAT_CHINA_MAINLAND:
		case FORMAT_CHINA_MACAU:
		case FORMAT_ENGLAND:
		case FORMAT_ITALY:
		case FORMAT_GERMANY:
		case FORMAT_INDIA:
		case FORMAT_SPAIN:
		case FORMAT_MALAYSIA:
		case FORMAT_VIETNAM:
		case FORMAT_PORTUGAL:
		case FORMAT_POLAND:
		case FORMAT_NEW_ZEALAND:
		case FORMAT_TURKEY:
		//just '+' for Russian. 
		case FORMAT_RUSSIAN:
			match = getFormatTypeByCommonPrefix(text);
			break;
		case FORMAT_TAIWAN:
			match = getFormatNumberBySpecialPrefix(text,TAIWAN_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_CHINA_HONGKONG:
			match = getFormatNumberBySpecialPrefix(text,HONGKONG_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_NANP:
			match = getFormatNumberBySpecialPrefix(text,NANP_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_JAPAN:
			match = getFormatNumberBySpecialPrefix(text,JAPAN_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_FRANCE:
			match = getFormatNumberBySpecialPrefix(text,FRANCE_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_SINGAPORE:
			match = getFormatNumberBySpecialPrefix(text,SINGAPORE_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_INDONESIA:
			match = getFormatNumberBySpecialPrefix(text,INDONESIA_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_THAILAND:
			match = getFormatNumberBySpecialPrefix(text,THAILAND_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_AUSTRALIA:
			match = getFormatNumberBySpecialPrefix(text,AUSTRALIA_INTERNATIONAL_PREFIXS);
			break;
		case FORMAT_BRAZIL:
			match = getFormatNumberBySpecialPrefix(text,BRAZIL_INTERNATIONAL_PREFIXS);
			break;
		}
		return match;
	}
	
	/**
	 * remove all dash.
	 * @param sb
	 * @return
	 */
	private static String removeAllDash(StringBuilder sb){
		int p = 0;
		while (p < sb.length()) {
            if (sb.charAt(p) == '-' || sb.charAt(p)== ' ') {
            	sb.deleteCharAt(p);
            } else {
                p++;
            }
        }
		return sb.toString();
	}
	
	/**
	 * remove all dash and add format blank, if stated with international code.
	 * @param sb
	 * @param startIndex
	 * @return
	 */
	private static int removeAllDashAndFormatBlank(StringBuilder sb,int startIndex){
		int p = 0;
		int index = -1;
		while (p < sb.length()) {
            if (sb.charAt(p) == '-' || sb.charAt(p)== ' ') {
            	sb.deleteCharAt(p);
            } else {
                p++;
            }
        }
        if(startIndex > 0){
        	//add blank follow country code
        	index = startIndex;
        	sb.replace(index, index, " ");
        }
        return index;
	}
	
	/**
	 * remove trailing dashes.
	 * @param sb
	 * @return
	 */
	private static String removeTrailingDashes(StringBuilder sb){
		// Remove trailing dashes
        int len = sb.length();
        while (len > 0) {
            if (sb.charAt(len - 1) == '-') {
                sb.delete(len - 1, len);
                len--;
            } else {
                break;
            }
        }
        return sb.toString();
	}
	
	/**
	 * format China Mianland Call number
	 * 
	 * reference "http://www.ct10000.com/main/services/05/"
	 * 			 "http://en.wikipedia.org/wiki/Telephone_numbers_in_China"
	 * 
	 * +86 10-NNNNNNNN    International Beijing Telephone
	 * +86 2N-NNNNNNNN    International Super city Telephone
	 * +86 NNN-NNNNNNNN   International Common region Telephone
	 * +86 NNN-NNNN-NNNN  International mobile phone
	 * 
	 * 010-NNNNNNNN      Beijing Telephone
	 * 02N-NNNNNNNN      Super city Telephone
	 * 0NNN-NNNNNNN
	 * 10-NNNNNNNN
	 * 2N-NNNNNNNN
	 * NNN-NNNNNNNN     Common region Telephone
	 * NNN-NNNN-NNNN     mobile phone
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatChinaNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	//for beijing or other super city.
        	//+86 10NNN -> +86 10-NNN
        	if(c1 == '1' && c2 == '0' || c1 == '2'){
        		dashPositions[numDashes++] = index + 2;
        	}else if(c1 == '1'){
        		//for mobile phone
        		//+86 1NNNN - > +86 1NN-NN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//+86 1NN-NNNNNN -> +86 1NN-NNNN-NN
        		if(length > index + 8){
        			dashPositions[numDashes++] = index + 7;
        		}
        	}else {//for other common region code.
        		dashPositions[numDashes++] = index + 3;
        	}
        }else{
        	char c1 = sb.charAt(phoneNumPosition);
        	char c2 = sb.charAt(phoneNumPosition+1);
        	if(c1 == '1' && c2 != '0'){
        		//for mobile phone
        		//1NNNN - > 1NN-NN
        		if(length > phoneNumPosition + 4){
        			dashPositions[numDashes++] = phoneNumPosition + 3;
        		}
        		//1NN-NNNNNN -> 1NN-NNNN-NN
        		if(length > phoneNumPosition + 8){
        			dashPositions[numDashes++] = phoneNumPosition + 7;
        		}
        	}else if(c1 == '1' && c2 == '0'){
        		//1NNNN - > 1NN-NN
        		if(length > phoneNumPosition + 3){
        			dashPositions[numDashes++] = phoneNumPosition + 2;
        		}
        	}else{
        		//No we don't know the code has region code(ignored 0), only when then length of number is more than 8
        		if(length > phoneNumPosition + 8){
        			if(c1 == '2'){
        				dashPositions[numDashes++] = phoneNumPosition + 2;
        			}else{
        				dashPositions[numDashes++] = phoneNumPosition + 3;
        			}
        		}
        	}
        }
        for (int i = 0; i < numDashes; i++) {
            int pos = dashPositions[i];
            sb.replace(pos + i, pos + i, "-");
        }
        return sb.toString();
	}
	
	/**
	 * format Taiwan Call Number
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Taiwan"
	 * 
	 * +886 9-NNNN-NNNN
	 * +886 N-NNNN-NNNN
	 * +886 N-NNN-NNNN
	 * +886 NN-NNNN-NNNN
	 * +886 NN-NNN-NNNN
	 * +886 NNN-NNN-NNN
	 * 
	 *	0N-NNNN-NNNN    
	 *	0N-NNN-NNNN
	 *	0NN-NNNN-NNNN
	 *	0NN-NNN-NNNN
	 *  0NNN-NNN-NNN
	 *	09-NNNN-NNNN     mobile phone
	 *
	 *	NNN-NNNN
	 *	NNNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatTaiwanNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	char c3 = sb.charAt(index+2);
        	//for mobile phone
        	if(c1 == '9'){
//        		dashPositions[numDashes++] = index + 1;
//        		//+886 9-NNNNNN -> +886 9-NNNN-NN
//        		if(length > index + 6){
//        			dashPositions[numDashes++] = index + 5;
//        		}
        		if(length > index + 4)
        		dashPositions[numDashes++] = index + 3;
        		//+886 9NN-NNNN -> +886 9NN-NNN-NNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else if((c1 == '8' && c2 == '2' && c3 == '6') || (c1 == '8' && c2 == '3' && c3 == '6')){
        		//for 0826, 0836
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//+886 NNN-NNN-NNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else if((c1 == '3' && c2 == '7') || (c1 == '4' && c2 == '9') || (c1 == '8' && c2 == '9') || (c1 == '8' && c2 == '2')){
        		//for 037, 049, 089, 082
        		dashPositions[numDashes++] = index + 2;
        		//+886 NN-NNN-NNNN
        		if(length > index + 6 && length < index + 10){
        			dashPositions[numDashes++] = index + 5;
        		}else if(length >= index + 10){//+886 NN-NNNN-NNNN
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else{
        		//for 02 04 05 06 07 08 ......
        		dashPositions[numDashes++] = index + 1;
        		//+886 N-NNN-NNNN
        		if(length > index + 6 && length < index + 9){
        			dashPositions[numDashes++] = index + 4;
        		}else if(length >= index + 9){//+886 N-NNNN-NNNN
        			dashPositions[numDashes++] = index + 5;
        		}
        	}
        }else{
        	if(length > phoneNumPosition + 4 && length < phoneNumPosition + 8){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}else if(length >= phoneNumPosition + 8){//NNNN-NNNN....
        		dashPositions[numDashes++] = phoneNumPosition + 4;
        	}
        }
        for (int i = 0; i < numDashes; i++) {
            int pos = dashPositions[i];
            sb.replace(pos + i, pos + i, "-");
        }
		return sb.toString();
	}
	
	/**
	 * format China Macau Call number.
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Macau"
	 * 
	 * +853 NNNN-NNNN
	 * NNNN-NNNN
	 * 01 NNNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatMacauNumber(StringBuilder sb, int blankPosition){
		int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
		//01 for Hongkong
		if(sb.charAt(phoneNumPosition) == '0' && sb.charAt(phoneNumPosition+1) == '1'){
			sb.replace(phoneNumPosition+2, phoneNumPosition+2, " ");
			return formatHeightLengthWithoutRegionCodeNumber(sb,blankPosition+3);
		}else{
			return formatHeightLengthWithoutRegionCodeNumber(sb,blankPosition);
		}
	}
	
	/**
	 * format number which length is 8, insert a hyphen to index 4. 
	 * it is used for Hongkong, Macau, Singapore
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Hong_Kong"
	 * 			  "http://en.wikipedia.org/wiki/Telephone_numbers_in_Macau"
	 * 			  "http://en.wikipedia.org/wiki/Telephone_numbers_in_Singapore"
	 * 
	 * +NNN NNNN-NNNN
	 * NNNN-NNNN
	 * @param text
	 * @param startIndex
	 * @return
	 */
	private static String formatHeightLengthWithoutRegionCodeNumber(StringBuilder sb, int blankPosition){
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;

        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
       
        if(sb.length()>=phoneNumPosition+6){
        	dashPositions[numDashes++] = phoneNumPosition+4; 
        }
        for (int i = 0; i < numDashes; i++) {
            int pos = dashPositions[i];
            sb.replace(pos + i, pos + i, "-");
        }
        return removeTrailingDashes(sb);
	}
	
	/**
	 * format Vietnam call number.
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Vietnam"
	 * 
	 * +84 4|8-XXXXXXXX
	 * +84 XX-XXXXXXX
	 * +84 YYY-XXXXXX
	 * 09Y-NNN-NNNN
	 * 01YY-NNN-NNNN
	 * 
	 * 04-NNNNNNNN
	 * 08-NNNNNNNN
	 * 0NN-NNNNNNN
	 * 0NNN-NNNNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatVietnamNubmer(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	//for 02,04
        	if(c1 == '4' || c1 == '8'){
        		dashPositions[numDashes++] = index+1; 
        	}else if(c1 == '2' && ( c2 == '1' || c2 == '3' || c2 == '4' || c2 == '8') 
					||(c1 == '3' && (c2 == '2' || c2 == '5'))
					||(c1 == '6' && c2 == '5')
					||(c1 == '7' && (c2== '1' || c2 == '8'))){
        		//for 021X,023X,024X,028X, 032X,035X, 065X, 071X,078X
        		if(length > index + 4){
        			dashPositions[numDashes++] = index+3; 
        		}
        	}else if(c1 == '9'){//for mobile phone 09N
        		//+84 9NNN -> +84 9N-NN
        		dashPositions[numDashes++] = index+2; 
        		//+84 9N-NNNNN -> +84 9N-NNN-NN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index+5; 
        		}
        	}else if(c1 == '1'){//for mobile phone 01NN
        		//+84 1NNNN -> +84 1NN-NN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index+3; 
        		}
        		//+84 1NN-NNNNN - > +84 1NN-NNN-NN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index+6; 
        		}
        	}else{
        		//for XX
        		dashPositions[numDashes++] = index+2; 
        	}
        }
        for (int i = 0; i < numDashes; i++) {
            int pos = dashPositions[i];
            sb.replace(pos + i, pos + i, "-");
        }
        return sb.toString();
	}
	
	
	/**
	 * format Portugal Number
	 * 
	 * +351 9T-NNN-NNNN
	 * +351 NN-NNN-NNNN
	 * 9T-NNN-NNNN
	 * NN-NNN-NNNN
	 * 
	 * @param text
	 * @param startIndex
	 * @return
	 */
	private static String formatPortugalNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();        
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        if(length > phoneNumPosition + 4){
        	dashPositions[numDashes++] = phoneNumPosition + 2;
        }
        if(length > phoneNumPosition + 8){
        	dashPositions[numDashes++] = phoneNumPosition + 5;
        }
        for (int i = 0; i < numDashes; i++) {
            int pos = dashPositions[i];
            sb.replace(pos + i, pos + i, "-");
        }
        return sb.toString();
	}
	
	/**
	 * format Brazil Phone Number
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Brazil"
	 * 
	 * +55-aa-nnnn-nnnn
	 * 
	 * 0-xx-aa-nnnn-nnnn
	 * 0-aa-nnnn-nnnn
	 * 
	 * nnnn-nnnn
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatBrazilNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[5];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		dashPositions[numDashes++] = phoneNumPosition + 1;
        		index ++;
        	}
        	if(length > index + 3){
        		dashPositions[numDashes++] = index + 2;
        	}
        	if(length > index + 7 && length <= index + 10){
        		dashPositions[numDashes++] = index + 6;
        	}else if(length > index + 10){
        		dashPositions[numDashes++] = index + 4;
        		dashPositions[numDashes++] = index + 8;
        	}
        }else if(length > phoneNumPosition + 5){
        	dashPositions[numDashes++] = phoneNumPosition + 4;
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Poland call number
	 * 
	 * +48 NN-NNN-NN-NN mobile phone(started with 5,6,7,8)
	 * +48 NNN-NNN-NNN  fixed phone
	 * NN-NNN-NN-NN 
	 * NNN-NNN-NNN
	 * reference site: http://en.wikipedia.org/wiki/Telephone_numbers_in_Poland
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatPolandNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
		// When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[3];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //for mobile phone(started with 5,6,7,8)
        if(sb.charAt(phoneNumPosition)>='5' && sb.charAt(phoneNumPosition)<='8'){
        	if(length > phoneNumPosition+4){
        		dashPositions[numDashes++] = phoneNumPosition + 2;
        	}
        	if(length > phoneNumPosition+6){
        		dashPositions[numDashes++] = phoneNumPosition + 5;
        	}
        	if(length > phoneNumPosition+8){
        		dashPositions[numDashes++] = phoneNumPosition + 7;
        	}
        }else{
        	if(length > phoneNumPosition+5){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}
        	if(length > phoneNumPosition+8){
        		dashPositions[numDashes++] = phoneNumPosition + 6;
        	}
        }
		int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Australia Number
	 * 
	 * +61 4NN-NNN-NNN
	 * +61 X-XXXX-XXXX
	 * 04XX-NNN-NNN mobile phone
	 * 0X-XXXX-XXXX landline phone
	 * XXXX-XXXX
	 * 
	 * reference site: http://en.wikipedia.org/wiki/Telephone_numbers_in_Australia
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatAustraliaNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	//mobile phone
        	if(sb.charAt(index) == '4'){
        		//+61 4NNNNN -> +61 4NN-NNN or 04NNNNN -> 04NN-NNN
        		if(length > index + 5){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//+61 4NN-NNNNNN->+61 4NN-NNN-NNN or 04NN NNNNNN - > 04NN-NNN-NNN
        		if(length > index + 8){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else{
        		//+61 XNNNN -> +61 X-NNNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 1;
        		}
        		//+61 X-NNNNNNNN -> +61 X-NNNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}
        }else{
        	//only do when the length of local number is 8
        	//XXXXXXXX->XXXX-XXXX
        	System.out.println(length);
        	if(length == phoneNumPosition + 8 ){
        		dashPositions[numDashes++] = phoneNumPosition + 4;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format New Zealand call number
	 * 
	 * +64 N-NNN-NNNN
	 * +64 2X-NNN-NNNN
	 * 02N-NNN-NNNN mobile phone
     * 0N-NNN-NNNN landline phone
     * NNN-NNNN landline phone
	 * 
	 * reference site: http://en.wikipedia.org/wiki/Telephone_numbers_in_New_Zealand
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatNewZealandNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	//mobile phone. start 2X but not 24.
        	if(sb.charAt(index) == '2' && sb.charAt(index+1)!= '4'){
        		//+64 2XNNN -> +64 2X-NNN or 02XNNN -> 02X-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+64 2X-NNNNNNN->+61 2X-NNN-NNNN or 02X-NNNNNNN - > 02X-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}else{
        		//+64 XNNN -> +64 X-NNN
        		if(length > index + 3){
        			dashPositions[numDashes++] = index + 1;
        		}
        		//+64 X-NNNNNNN -> +61 X-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 4;
        		}
        	}
        }else{
        	//only do when the length of local number is 7
        	//NNNNNNN->NNN-NNNN
        	System.out.println(length);
        	if(length == phoneNumPosition + 7 ){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Tailand call number
	 * 
	 * +66 2-NNN-NNNN
	 * +66 NN-NNN-NNN
	 * +66 8N-NNN-NNNN
	 * 02-NNN-NNNN        
	 * 0XX-NNN-NNN        
	 * 08X-NNN-NNNN       
	 * 
	 * reference "http://www.wtng.info/wtng-66-th.html"
	 * reference "http://en.wikipedia.org/wiki/Telephone_numbers_in_Thailand"
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatThailandNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	//mobile phone. started with 8 or 08
        	if(sb.charAt(index) == '8' ){
        		//+66 8XNNN -> +66 8X-NNN or 08XNNN -> 08X-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+66 8X-NNNNNN->+61 8X-NNN-NNN or 08X-NNNNNNN - > 08X-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}else if(sb.charAt(index) == '2'){ //for Bangkok started with 2 or 02.         		
        		//+66 2NNN -> +64 2-NNN
        		if(length > index + 3){
        			dashPositions[numDashes++] = index + 1;
        		}
        		//+66 2-NNNNNNN -> +66 2-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 4;
        		}
        	}else{//for other region code with NN or 0NN
        		//+66 XXNNN -> +66 XX-NNN or 0XXNNN -> 0XX-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+66 XX-NNNNNN->+61 XX-NNN-NNN or 0XX-NNNNNNN - > 0XX-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}
        }else{
        	//To dial a land line in Thailand all over the country, the format is Area Code + Phone Number.
        	//Even through dial local phone, must add area code.
        	//only do when the length of local number is 7
        	//NNNNNNN->NNN-NNNN
        	//System.out.println(length);
        	//if(length == phoneNumPosition + 7 ){
        	//	dashPositions[numDashes++] = phoneNumPosition + 3;
        	//}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Indonesia call number.
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Indonesia"
	 * 
	 * +62 8NN-NNN-NNN
	 * +62 8NN-NNN-NNNN
	 * +62 8NN-NNNN-NNNN
	 * +62 NN-NNNN-NNNN
	 * +62 NNN-NNN-NNNN
	 * 08NN-NNN-NNN
	 * 08NN-NNN-NNNN
	 * 08NN-NNNN-NNNN
	 * 0NN-NNNN-NNNN
	 * 0NNN-NNN-NNNN
	 * 8NN-NNNN-NNNN
	 * 8NN-NNN-NNNN
	 * 8NN-NNN-NNN
	 * NNNN-NNNN
	 * NNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatIndonesiaNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
        int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	char c3 = sb.charAt(index+2);
        	//mobile phone started with +62 8 or 08
        	if(c1 == '8'){
        		//+62 8NNNNN -> +62 8NN-NNN or 08NNNNN -> 08NN-NNN
        		if(length > index + 5){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//+62 8NN-NNNNN.. -> +62 8NN-NNN-NN.., when the length of mobile phone number is less then 11
        		if(length >= index + 8 && length <= index+10){
        			dashPositions[numDashes++] = index + 6;
        		}
        		//+62 8NN-NNNNNNNN -> +62 8NN-NNNN-NNNN, when the length of mobile phone number is more then 10.
        		if(length > index + 10 ){
        			dashPositions[numDashes++] = index + 7;
        		}
        	}else if((c1 == '2' && (c2=='1' || c2 == '2' || c2 == '4'))
        			||(c1 == '3' && c2 == '1')
        			||(c1 == '6' && c2 == '1' && c3 != '9')){ 
        		//region code 21,22,24,31,61(not 619)
        		//+62 XXNNN -> +62 XX-NNN or 0XXNNN -> 0XX-NNN
        		if(length > index + 3){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+62 XX-NNNNNNNN -> +62 XX-NNNN-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else{
        		//region code YYY
        		//+62 YYYNNN -> +62 YYY-NNN or 0YYYNNN -> 0YYY-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//+62 YYY-NNNNNNN -> +62 YYY-NNN-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}
        }else{
        	//NNNNNNN->NNN-NNNN
        	if(length == phoneNumPosition + 7){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}else if(length == phoneNumPosition + 8){
        		//NNNNNNNN->NNNN-NNNN
        		dashPositions[numDashes++] = phoneNumPosition + 4;
        	}else if(sb.charAt(phoneNumPosition) == '8'){//FOR 8NNNNNNNNNN...
        		if(length > phoneNumPosition + 8 && length <= phoneNumPosition + 10){
        			dashPositions[numDashes++] = phoneNumPosition + 3;
        			dashPositions[numDashes++] = phoneNumPosition + 6;
        		}else if(length > phoneNumPosition + 10){
        			dashPositions[numDashes++] = phoneNumPosition + 3;
        			dashPositions[numDashes++] = phoneNumPosition + 7;
        		}
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Malaysia call number
	 * 
	 * reference "http://en.wikipedia.org/wiki/Telephone_numbers_in_Malaysia"
	 * 
	 * +60 3-NNNNNNNN
	 * +60 X-NNNNNNN
	 * +60 8X-NNNNNN
	 * +60 1X-NNN-NNNN
	 * 
	 * 2-NNNN-NNNN (to Singapore)
	 * 03-NNNNNNNN
	 * 0X-NNNNNNN
	 * 08X-NNNNNN
	 * 01X-NNN-NNNN
	 * 
	 * 1X-NNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatMalaysiaNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	//+60 X-NNNNNNN or +60 3-NNNNNNNN or 03-NNNNNNNN or 0X-NNNNNNN
        	if(c1 >= '3' && c1 <= '7' || c1 == '9'){
        		//+60 XNNNN -> +60 X-NNNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 1;
        		}
        	}else if(c1 == '8'){
        		//+60 8XNNN -> +60 8X-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        	}else if(c1 == '1'){
        		//mobile phone
        		//+60 1XNNN -> +60 1X-NNN or 01XNNN -> 01X-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+60 1X-NNNNNN->+60 1X-NNN-NNN or 01X-NNNNNNN - > 01X-NNN-NNNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}else if(c1 == '2'){ //to Singapore
        		//+60 2NNNN -> +60 2-NNNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 1;
        		}
        		//+60 2NNNNNNN -> +60 2-NNNN-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}
        }else if(sb.charAt(phoneNumPosition) == '2' && length > phoneNumPosition + 8){//to Singapore
        	// 2NNNN -> 2-NNNN
        	dashPositions[numDashes++] = phoneNumPosition + 1;
        	// 2NNNNNNN -> 2-NNNN-NNNN
        	dashPositions[numDashes++] = phoneNumPosition + 5;
        }else if(sb.charAt(phoneNumPosition) == '1' && length > phoneNumPosition + 8){
        	//for mobile phone. 
        	dashPositions[numDashes++] = phoneNumPosition + 2;
        	dashPositions[numDashes++] = phoneNumPosition + 5;
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Spain Number
	 * 
	 * reference "http://en.wikipedia.org/wiki/Telephone_numbers_in_Spain"
	 * 
	 * +34 NNN-NNN-NNN
	 * 
	 * NNN-NNN-NNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatSpainNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //+34 NNNNNN - > +34 NNN-NNNN
        if(length > phoneNumPosition + 5){
        	dashPositions[numDashes++] = phoneNumPosition + 3;
        }
        //+34 NNNNNNNN-> +34 NNN-NNN-NN
        if(length > phoneNumPosition + 7){
        	dashPositions[numDashes++] = phoneNumPosition + 6;
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	private static int[] INDIA_7_MOBILE_AREA_CODE = {
		
	};
	//TODO maybe use table, and binary search is better.
	*/
	
	/**
	 * India Three digits area codes
	 * 
	 * reference: "http://www.bsnl.co.in/stdsearch.php"
	 * 
	 */
	private static final int[] INDIA_THREE_DIGIG_AREA_CODES = {
		120,121,122,124,129,130,131,132,135,141,144,145,151,154,160,161,164,171,172,175,177,180,181,183,184,186,191,194,
		212,215,217,230,231,233,240,241,250,251,253,257,260,261,265,268,278,281,285,286,288,291,294,326,341,342,343,353,
		354,360,361,364,368,369,370,372,373,374,376,381,385,389,413,416,421,422,423,424,427,431,435,451,452,461,462,468,
		469,470,471,474,475,476,477,478,479,480,481,483,484,485,487,490,491,494,495,496,497,512,515,522,532,535,542,548,
		551,562,565,571,581,591,595,612,621,631,641,651,657,661,663,671,674,680,712,721,724,731,733,734,744,747,751,755,
		761,771,788,816,820,821,824,831,832,836,861,863,866,870,877,878,883,884,891
	};
	
	/**
	 * check 8NNN,7NNN is moble phone number, or is fixed number.
	 * if is mobile phone number,return 0. 
	 * if is two digits region code, return 2;
	 * if is three digits region code, return 3;
	 * if is four digits region code, reutrn 4;
	 * 
	 * Waring: we are now using the strategy that mobile phones to expend, 
	 *         when the number is not used, it is the mobile phone number by default, but the statistics are not complete.
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Mobile_telephone_numbering_in_India"
	 * reference: "http://www.bsnl.co.in/stdsearch.php"
	 * 
	 * @param c1 c1 != '0'
	 * @param c2
	 * @param c3
	 * @param c4
	 * @return
	 */
	private static int checkIndiaNumber(char c1, char c2, char c3, char c4){
		int result = -1;
		int temp = (c3-'0')*10 + (c4 -'0');
		if(c1 == '9'){
			result = 0;
		}else if(c1 == '8'){
			if((c2 == '0' && (temp< 20 || temp>=50 && temp <= 60 || temp>=80))
					||(c2 == '1' && (temp < 10 || temp>=20 && temp <= 29 || temp>=40 && temp<=49))
					||(c2 == '7' && (temp >= 90 || temp == 69))
					||(c2 == '8' && (temp < 10 || temp == 17 || temp >=25 && temp <=28 || temp == 44 || temp == 53 || temp >=90))
					||(c3 == '9' && (temp < 10 || temp == 23 || temp == 39 || temp >=50 && temp <=62 || temp == 67 || temp == 68 || temp >=70))){
				result = 0;
			}
		}else if(c1 == '7'){
			//TODO maybe use table, and binary search is better.
			if(c2 == '0' //expend to mobile phone number 
				|| (c2 == '2' && (temp == 0 || temp >=4 && temp <=9 || temp == 50 || temp == 59 || temp>=75 && temp <=78 || temp == 93 || temp == 9))
				|| (c2 == '3' && (temp == 73 || temp == 76 || temp == 77 || temp == 96 || temp == 98 || temp == 99))
				|| (c2 == '4' && (temp < 10 || temp == 11 || temp >= 15 && temp <= 19 || temp == 28 || temp == 29 || temp == 39 || temp == 83 || temp == 88 || temp == 89 || temp == 98 || temp == 99))
				|| (c2 == '5' && (temp <= 4 || temp == 49 || temp == 50 || temp >=66 && temp <=69 || temp == 79 || temp>= 87 && temp<=89 || temp >= 97))
				|| (c2 == '6' && (temp == 0 || temp == 2 || temp == 7 || temp == 20 || temp == 31 || temp == 39 || temp == 54 || temp == 55 || temp>=65 && temp <=69 || temp>=76 && temp <=79 || temp >=96))
				|| (c2 == '7' && (temp == 2 || temp == 8 ||  temp == 9 || temp >= 35 && temp <= 39 || temp == 42 || temp == 60 || temp == 77 || temp>=95))
				|| (c2 == '8' && temp <=39 && (temp == 0 || temp >= 7 && temp <=9 || temp == 14 || temp >=27 && temp <=30 || temp>=37 && temp<=39))
				|| (c2 == '8' && temp > 39 && (temp == 42 || temp == 45 || temp == 60 || temp >=69 && temp <=79 || temp >=90 ))){
				result = 0;
			}
		}
		if(result == 0){
			return result;
		}
		if((c1 == '1' && c2 == '1')
				|| (c1 == '2' && (c2 == '0'|| c2=='2' ))
				|| (c1 == '3' && c2 == '3')
				|| (c1 == '4' && (c2 == '0' || c2 == '4'))
				|| (c1 == '7' && c2 == '9')
				|| (c1 == '8' && c2 == '0')){
			result = 2;
		}else{
			int key = (c1-'0') * 100 + (c2-'0') * 10 + (c3-'0');
			if(Arrays.binarySearch(INDIA_THREE_DIGIG_AREA_CODES, key)>=0){
				result = 3;
			}else{
				result = 4;
			}
		}
		return result;
	}
	
	/**
	 * format India call number
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_India"
	 * 
	 * +91 XX-YY-NNNNNN
	 * +91 NN-NNNNNNNN
	 * +91 NNN-NNNNNNN
	 * +91 NNNN-NNNNNN
	 * 
	 * 0XX-YY-NNNNNN
	 * 0NN-NNNNNNNN
	 * 0NNN-NNNNNNN
	 * 0NNNN-NNNNNN
	 * 
	 * XX-YY-NNNNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatIndiaNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;

        //has country code. or started with '0'
    	char c = sb.charAt(phoneNumPosition);
        if((phoneNumPosition > 0 && c != '0') || 
        		(c == '0' && length > phoneNumPosition+4)){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	char c3 = sb.charAt(index+2);
        	char c4 = sb.charAt(index+3);
        	//check the india number type.
        	int type = checkIndiaNumber(c1,c2,c3,c4);
        	//for mobile phone
        	if(type == 0){
        		dashPositions[numDashes++] = index + 2;
        		//+91 9X-YYNNNN -> +91 9X-YY-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 4;
        		}
        	}else if(type == 2){
        		//+91 NNNN -> + 91 NN-NN
        		dashPositions[numDashes++] = index + 2;
        	}else if(type == 3){
        		//+91 NNNNN -> + 91 NNN-NN
        		dashPositions[numDashes++] = index + 3;
        	}else{
        		//+91 NNNNNN -> +91 NNNN-NN
        		if(length > index + 5){
        			dashPositions[numDashes++] = index + 4;
        		}
        	}        	
        }else if(length > phoneNumPosition + 8){ 
        	//XXYYNNNNNN -> XX-YY-NNNNNN
        	dashPositions[numDashes++] = phoneNumPosition + 2;
        	dashPositions[numDashes++] = phoneNumPosition + 4;
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format Russian Number
	 * 
	 * reference "http://en.wikipedia.org/wiki/Telephone_numbers_in_Russia"
	 * 
	 * Because in Russian, when dialing Long Distance or International Call, the prefix is 8~ or 8~10, it come to DTMF.
	 * There are plans to change those prefixes to '0' for national and '00' for international dialing, but they are not yet implemented.
	 * So, we don't check any Long Distance or International Call prefix, all number format as follow:
	 * 
	 * +7 NNN-NNN-NN-NN
	 * +7 9NN-NNN-NN-NN
	 * YYY-NNN-NN-NN only in same country, so YYY can be ignored.
	 * NNN-NN-NN
	 * NN-NN-NN
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatRussianNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[3];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. 
        if(phoneNumPosition > 0){
        	//+7 NNNNNN -> +7 NNN-NNN
        	if(length > phoneNumPosition + 5){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}
        	//+7 NNN-NNNNNN-> +7 NNN-NNN-NN
        	if(length > phoneNumPosition + 7){
        		dashPositions[numDashes++] = phoneNumPosition + 6;
        	}
        	//+7 NNN-NNN-NNNN-> +7 NNN-NNN-NN-NN
        	if(length > phoneNumPosition + 9){
        		dashPositions[numDashes++] = phoneNumPosition + 8;
        	}
        }else if(length == phoneNumPosition + 6){
        	//NNNNNN -> NN-NN-NN
        	dashPositions[numDashes++] = phoneNumPosition + 2;
        	dashPositions[numDashes++] = phoneNumPosition + 4;
        }else if(length == phoneNumPosition + 7){
        	//NNNNNNN -> NNN-NN-NN
        	dashPositions[numDashes++] = phoneNumPosition + 3;
        	dashPositions[numDashes++] = phoneNumPosition + 5;
        }else if(length >= phoneNumPosition + 8){
        	//NNNNNNNN - > NNN-NNN-NN
        	dashPositions[numDashes++] = phoneNumPosition + 3;
        	dashPositions[numDashes++] = phoneNumPosition + 6;
        	//NNN-NNN-NNNN -> NNN-NNN-NN-NN
        	if(length > phoneNumPosition + 9){
        		dashPositions[numDashes++] = phoneNumPosition + 8;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * Germany region codes: three digits, exclude XX1.
	 * 
	 * Warning: 212 must don't follow 9.
	 * reference: "http://en.wikipedia.org/wiki/Area_codes_in_Germany"
	 * 
	 */
	private static final int[] Germany_THREE_PART_REGION_CODES = {
			202,203,208,209,212,214,221,228,234,249,310,335,340,345,365,375,385,395,
			457,458,459,700,709,710,728,729,749,759,769,778,779,786,787,788,789,792,
			798,799,800,872,875,879,900,902,903,906
	};
	
	/**
	 * Germany region codes: four digits, and started with 3.
	 * reference: "http://en.wikipedia.org/wiki/Area_codes_in_Germany"
	 */
	private static final int[] Germany_FOUR_PART_REGION_CODES = {
			3301,3302,3303,3304,3306,3307,3321,3322,3327,3328,3329,3331,3332,3334,
			3335,3337,3338,3341,3342,3344,3346,3361,3362,3364,3366,3371,3372,3375,
			3377,3378,3379,3381,3382,3385,3386,3391,3394,3395,3421,3423,3425,3431,
			3433,3435,3437,3441,3443,3445,3447,3448,3461,3462,3464,3466,3471,3473,
			3475,3476,3491,3493,3494,3496,3501,3504,3521,3522,3523,3525,3528,3529,
			3531,3533,3537,3541,3542,3544,3546,3561,3562,3563,3564,3571,3573,3574,
			3576,3578,3581,3583,3585,3586,3588,3591,3592,3594,3596,3601,3603,3605,
			3606,3621,3622,3623,3624,3626,3627,3628,3629,3631,3632,3634,3635,3636,
			3641,3643,3644,3647,3661,3663,3671,3672,3675,3677,3679,3680,3681,3682,
			3683,3685,3686,3691,3693,3695,3721,3722,3723,3724,3725,3726,3727,3731,
			3733,3735,3737,3741,3744,3745,3761,3762,3763,3764,3765,3771,3772,3773,
			3774,3821,3831,3834,3838,3841,3843,3844,3847,3871,3874,3876,3877,3881,
			3883,3886,3901,3921,3923,3925,3928,3931,3933,3935,3937,3941,3942,3943,
			3944,3946,3947,3949,3961,3962,3963,3964,3965,3966,3967,3968,3969,3971,
			3973,3976,3981,3984,3991,3994,3996,3997
	};
	
	/**
	 * format Germany call Number
	 * 
	 * Prefix 	Service type
	 *  01 	Non-geographic area codes
	 *	02 	Geographic area codes around Dusseldorf
	 *	03 	Geographic area codes around Berlin, except 031 and 032
	 *	04 	Geographic area codes around Hamburg
	 *	05 	Geographic area codes around Hannover
	 *	06 	Geographic area codes around Frankfurt am Main
	 *	07 	Geographic area codes around Stuttgart, except 0700
	 *	08 	Geographic area codes around Munich, except 0800
	 *	09 	Geographic area codes around Nuremberg, except 0900
	 *	11 	Network services
	 * 
	 * Reference: "http://en.wikipedia.org/wiki/%2B49"
	 * 
	 * WARNING: Geographic area codes have a length of two to five digits (not including the 0 trunk code). 
	 * Two Digits: 		Berlin (030), Hamburg (040), Frankfurt (069) and Munich (089). 
	 * Three Digits:    as 0XX1 or in Germany_THREE_PART_REGION_CODES, and is not previous case.
	 * Four Digits:		it is not previous case, if don't start with 03, and , or in Germany_FOUR_PART_REGION_CODES, is four digits.
	 * Five Digits:		start with 03, and is not previous case, is five digits.
	 * 
	 * +49 NN-NNN-NNNNN
	 * +49 NNN-NNN-NNNNN
	 * +49 NNNN-NNN-NNNN
	 * +49 3NNNN-NNN-NNN
	 * +49 1NN-NNNNNN-NN
	 * 
	 * 0NN-NNN-NNNNN
	 * 0NNN-NNN-NNNNN
	 * 0NNNN-NNN-NNNN
	 * 03NNNN-NNN-NNN
	 * 01NN-NNNNNN-NN
	 * 
	 * NNN-NNNNN
	 * NNN-NNNN
	 * NNN-NNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatGermanyNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        //has country code. or started with '0'
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	//for +49 1X
        	if(c1 == '1'){
        		//+49 1XXNNN -> +49 1XX-NNN or 01XXNNN -> 01XX-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 3;
        		}
        		//mobile phone 015X, 016X, 017X
        		if(c2 == '5' || c2 == '6' || c2 == '7'){
	        		//+49 1XX-NNNNNNNN->+49 1XX-NNNNNN-NN or 01XX-NNNNNNNN - > 01XX-NNNNNN-NN
	        		if(length > index + 10){
	        			dashPositions[numDashes++] = index + 9;
	        		}
        		}
        	}else //Berlin (030), Hamburg (040), Frankfurt (069) and Munich (089).
        		if((c1 == '3' && c2 == '0') || (c1 == '4' && c2 == '0')
        			|| (c1 == '6' && c2 == '9') || (c1 == '8' && c2 == '9')){
        		//+49 NNNNN -> +49 NN-NNN or 0NNNNN -> 0NN-NNN
        		if(length > index + 4){
        			dashPositions[numDashes++] = index + 2;
        		}
        		//+49 NN-NNNNNN -> +49 NN-NNN-NNN
        		if(length > index + 6){
        			dashPositions[numDashes++] = index + 5;
        		}
        	}else if(length > index + 3){       		
        		char c3 = sb.charAt(index+2);
        		char c4 = sb.charAt(index+3);
        		int key3 = (c1-'0') * 100 + (c2-'0') * 10 + (c3-'0');
        		int key4 = key3 * 10 + (c4-'0');
        		//0XX1 or in Germany_THREE_PART_REGION_CODES is 0NNN region code.
        		if(c3 == '1' || (Arrays.binarySearch(Germany_THREE_PART_REGION_CODES, key3) >=0 && (key3!=212 || key3== 212 && c4 != '9'))){
        			//+49 NNNNNN -> +49 NNN-NNN
        			if(length > index + 4){
        				dashPositions[numDashes++] = index + 3;
        			}
        			//+49 NNN-NNNNNN -> +49 NNN-NNN-NNN
        			if(length > index + 7){
        				dashPositions[numDashes++] = index + 6;
        			}
        		}else //if don't start with 03, and is not previous case or in Germany_FOUR_PART_REGION_CODES, is four digits. 
        			if(c1!='3' || c1=='3' && Arrays.binarySearch(Germany_FOUR_PART_REGION_CODES, key4) >=0 ){
        				//+49 NNNNNNN -> +49 NNNN-NNN
            			if(length > index + 5){
            				dashPositions[numDashes++] = index + 4;
            			}
            			//+49 NNNN-NNNNNN -> +49 NNNN-NNN-NNN
            			if(length > index + 8){
            				dashPositions[numDashes++] = index + 7;
            			}
        		}else{
        			//start with 03, and is not previous case, is five digits.
        			//+49 NNNNNNNN -> +49 NNNNN-NNN
        			if(length > index + 6){
        				dashPositions[numDashes++] = index + 5;
        			}
        			//+49 NNNNN-NNNNNN -> +49 NNNNN-NNN-NNN
        			if(length > index + 9){
        				dashPositions[numDashes++] = index + 8;
        			}
        		}
        	}
        }else if(length >= phoneNumPosition + 6 && length <= phoneNumPosition + 8){
        	dashPositions[numDashes++] = phoneNumPosition + 3;
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * Italy Mobile Phone Number Prefixs
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Italy"
	 * 
	 */
	private static final int[] ITALY_MOBILE_PREFIXS = {
		328, 329, 330, 333, 334, 335, 336, 337, 338,
		339, 347, 348, 349, 360, 368, 380, 388, 389 
	};
	
	/**
	 * format Italy Number
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_Italy"
	 * 
	 * Land line numbers (area code+exchange+number) are generally 9 or 10 digits long, 
	 * although they can be as little as 6 or as many as 11 digits. 
	 * Mobile numbers are always 10 digits long, with the only exception of very old TIM numbers, 
	 * which are 9 digits long (though those are now extremely rare).
	 * Mobile phone number in Italy: without a zero, started with a 3.
	 * 
	 * +39 3NN-NNN-NNNN
	 * +39 N-NNNNNNNN
	 * +39 NN-NNNNNNNN
	 * +39 NNN-NNNNNNN
	 * 
	 * 3NN-NNN-NNNN
	 * 0N-NNNNNNNN
	 * 0NN-NNNNNNNN
	 * 0NNN-NNNNNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatItalyNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        if(phoneNumPosition > 0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	char c3 = sb.charAt(index+2);
        	int key = (c1-'0') * 100 + (c2-'0') * 10 + (c3 - '0');
        	//mobile phone number
        	if(Arrays.binarySearch(ITALY_MOBILE_PREFIXS, key) >=0){
        		//+39 3NNNNN -> +39 3NN-NNN
    			if(length > index + 5){
    				dashPositions[numDashes++] = index + 3;
    			}
    			//+39 NNN-NNNNNN -> +49 NNN-NNN-NNN
    			if(length > index + 8){
    				dashPositions[numDashes++] = index + 6;
    			}
        	}else if(c1 == '2' || c1 == '6'){
        		//for Milan(02) 
        		//    Rome (including State of Vatican City) and Aprilia 06
        		dashPositions[numDashes++] = index + 1;
        	}else if(c2 == '0' || c2 == '1' || c2 == '5' || c2 == '9'){
        		//10,11,15,19
        		//30,31,35,39
        		//40,41,45,49
        		//50,51,55,59
        		//70,71,75,79
        		//80,81,85,89
        		//90,91,95,99
        		//+39 NNNNN - > +39 NN-NNN
        		if(length > index + 4){
    				dashPositions[numDashes++] = index + 2;
    			}
        	}else{
        		//+39 NNNNNN-> + 39 NNN-NNN
        		if(length > index + 5){
    				dashPositions[numDashes++] = index + 3;
    			}
        	}
        }else{
        	char c1 = sb.charAt(phoneNumPosition);
        	char c2 = sb.charAt(phoneNumPosition+1);
        	char c3 = sb.charAt(phoneNumPosition+2);
        	int key = (c1-'0') * 100 + (c2-'0') * 10 + (c3 - '0');
        	if(Arrays.binarySearch(ITALY_MOBILE_PREFIXS, key) >=0){
        		//+39 3NNNNN -> +39 3NN-NNN
    			if(length > phoneNumPosition + 5){
    				dashPositions[numDashes++] = phoneNumPosition + 3;
    			}
    			//+39 NNN-NNNNNN -> +49 NNN-NNN-NNN
    			if(length > phoneNumPosition + 7){
    				dashPositions[numDashes++] = phoneNumPosition + 6;
    			}
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
	    return sb.toString();
	}
	
	/**
	 * format France Call Number
	 * 
	 * refernce "http://en.wikipedia.org/wiki/Telephone_numbers_in_France"
	 * 
	 * All geographic numbers had to be dialed in the ten-digit format, even for local calls. 
	 * The international access code also changed from 19 to 00. 
	 * Following liberalisation in 1998, subscribers could access different carriers by 
	 * replacing the '0' (omitted from numbers when called from outside France) with another digit. 
	 * For example Cegetel required subscribers to dial '7', e.g: Paris 71 xx xx xx xx, instead of 01 xx xx xx xx. 
	 * Similarly, the international access code using Cegetel would be '70', instead of '00'.
	 * 00 (France Telecom)
     * 40 (TELE 2)
     * 50 (OMNICOM)
     * 70 (LE 7 CEGETEL)
     * 90 (9 TELECOM) 
     * 
	 * +33 N-NN-NN-NN-NN
	 * 
	 * TN-NN-NN-NN-NN, T = 0,4,5,7,9
	 * 
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatFranceNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[4];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        int c = sb.charAt(phoneNumPosition);
        if(phoneNumPosition > 0 || c == '0' || c == '4' || c == '5' || c == '7' || c== '9'){
        	int index = phoneNumPosition;
        	if(phoneNumPosition == 0 &&(c == '0' || c == '4' || c == '5' || c == '7' || c== '9') || phoneNumPosition>0 && c == '0'){
        		index ++;
        	}
        	//+33 NNNN-> +33 N-NNN 
        	dashPositions[numDashes++] = index + 1;
        	//+33 N-NNNN-> +33 N-NN-NN
        	if(length > index + 4){
        		dashPositions[numDashes++] = index + 3;
        	}
        	//+33 N-NN-NNNN -> +33 N-NN-NN-NN
        	if(length > index + 6){
        		dashPositions[numDashes++] = index + 5;
        	}
        	//+33 N-NN-NN-NNNN -> +33 N-NN-NN-NN-NN
        	if(length > index + 8){
        		dashPositions[numDashes++] = index + 7;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
        return sb.toString();
	}
	
	/**
	 * format United Kingdom call number
	 * 
	 * All mobile telephone numbers have 10 national (significant) numbers after the "0" trunk code. 
	 * In the United Kingdom, area codes are two, three, four, or, rarely, five digits long (after the initial zero). 
	 * Regions with shorter area codes, typically large cities, permit the allocation of more telephone numbers 
	 * as the local number portion has more digits. Local customer numbers are four to eight figures long. 
	 * The total number of digits is ten, but in a very few areas the total may be nine digits (after the initial zero). 
	 * 
	 * reference: "http://en.wikipedia.org/wiki/Telephone_numbers_in_the_United_Kingdom"
	 * 			  "http://en.wikipedia.org/wiki/List_of_United_Kingdom_dialling_codes"
	 *            "http://www.ofcom.org.uk/static/archive/oftel/publications/numbering/2003/num_guide.htm"
	 * 
	 * +44 2N-NNNN-NNNN
	 * +44 1NN-NNN-NNNN
	 * +44 1NNN-NNNNN(N)
	 * +44 1NNNN-NNNNN(N)
	 * +44 3NN-NNN-NNNN
	 * +44 5N-NNNN-NNNN
	 * +44 7NNN-NNNNNN
	 * +44 8NN-NNN-NNNN
	 * +44 9NN-NNN-NNNN
	 * 
	 * 02N-NNNN-NNNN
	 * 01NN-NNN-NNNN
	 * 01NNN-NNNNN(N)
	 * 01NNNN-NNNNN(N)
	 * 03NN-NNN-NNNN
	 * 05N-NNNN-NNNN
	 * 07NNN-NNNNNN
	 * 08NN-NNN-NNNN
	 * 09NN-NNN-NNNN
	 * 
	 * NNNN-NNNN
	 * NNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatEnglandNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        if(phoneNumPosition>0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	char c1 = sb.charAt(index);
        	char c2 = sb.charAt(index+1);
        	char c3 = sb.charAt(index+2);
        	//for mobile phone
        	if(c1 == '7'){
        		//+44 7NNNNNNNNN -> +44 7NNN-NNNNNN
        		if(length > index + 5){
        			dashPositions[numDashes++] = index + 4;
        		}
        	}else if(c1 == '2'){
        		//+44 2NNN - > +44 2N-NN
        		dashPositions[numDashes++] = index + 2;
        		//+44 2N-NNNNNNNN -> +44 2N-NNNN-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
        	}else if(c1 == '1'){ //for region code.
        		char c4 = sb.charAt(index+2);
        		int key = (c1-'0')*1000 + (c2 - '0')*100 + (c3 - '0') * 10 + c4;
	        	if(c2 == '1' || c3 == '1'){
		        	//+44 1NN-NN -> +44 1NN-NN
		        	if(length > index + 4){
		        		dashPositions[numDashes++] = index + 3;
		        	}
		        	//+44 1NN-NNNNN -> 1NN-NNN-NN
		        	if(length > index + 7){
		       			dashPositions[numDashes++] = index + 6;
		       		}
	        	}else if(key != 1387 && key != 1539 && key != 1697 && key != 1768 && key != 1946){
	        		// don't started with 01387, 01539, 01697, 01524, 01768, 01946
	        		//+44 1NNNNN -> +44 1NNN-NN
		        	if(length > index + 5){
		        		dashPositions[numDashes++] = index + 4;
		        	}
	        	}else{
	        		//+44 1NNNNNN -> +44 1NNNN-NN
		        	if(length > index + 6){
		        		dashPositions[numDashes++] = index + 5;
		        	}
	        	}
        	}else if(c1 == '3' || c1 == '8' || c1 == '9'){
        		//+44 NNNNN-> +44 NNN-NN
        		if(length > index + 4){
	        		dashPositions[numDashes++] = index + 3;
        		}
        		//+44 NNN-NNNNN -> NNN-NNN-NN
	        	if(length > index + 7){
	       			dashPositions[numDashes++] = index + 6;
	       		}
        	}else{
    			//other as 0NN-NNNN-NNNN
        		//+44 NNNN - > +44 NN-NN
        		dashPositions[numDashes++] = index + 2;
        		//+44 NN-NNNNNNNN -> +44 NN-NNNN-NNNN
        		if(length > index + 7){
        			dashPositions[numDashes++] = index + 6;
        		}
    		}
        }else{
        	if(length > phoneNumPosition + 4 && length < phoneNumPosition + 8){//NNN-NNNN
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}else if(length >= phoneNumPosition + 8){//NNNN-NNNN
        		dashPositions[numDashes++] = phoneNumPosition + 4;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
        return sb.toString();
	}
	
	/**
	 * format Turkey Phone Number.
	 * 
	 * reference "http://en.wikipedia.org/wiki/Telephone_numbers_in_Turkey"
	 * 
	 * +90 NNN-NNN-NNNN
	 * 0NNN-NNN-NNNN
	 * NNN-NNNN
	 * 
	 * @param sb
	 * @param blankPosition
	 * @return
	 */
	private static String formatTurkeyNumber(StringBuilder sb, int blankPosition){
		int length = sb.length();
        // When scanning the number we record where dashes need to be added,
        // if they're non-0 at the end of the scan the dashes will be added in
        // the proper places.
		int dashPositions[] = new int[2];
        int numDashes = 0;
        int phoneNumPosition = (blankPosition == -1) ? 0 : blankPosition + 1;
        if(phoneNumPosition>0 || sb.charAt(phoneNumPosition) == '0'){
        	int index = phoneNumPosition;
        	if(sb.charAt(phoneNumPosition) == '0'){
        		index ++;
        	}
        	//+90 NNNNN -> +90 NNN-NN
        	if(length > index + 4){
        		dashPositions[numDashes++] = index + 3;
        	}
        	//+90 NNN-NNNNN - > +90 NNN-NNN-NN
        	if(length > index + 7){
        		dashPositions[numDashes++] = index + 6;
        	}
        }else{
        	//NNNNNNN -> NNN-NNNN
        	if(length > phoneNumPosition + 4){
        		dashPositions[numDashes++] = phoneNumPosition + 3;
        	}
        }
        int pos;
		for (int i = 0; i < numDashes; i++) {
	        pos = dashPositions[i];
	        sb.replace(pos + i, pos + i, "-");
	    }
        return sb.toString();
	}

	/**
	 * Dump log information.
	 * @param info Log information.
	 * @hide
	 */	 
	public static void log(String info){
		if(DEBUG) Rlog.d(TAG, info);
        }
}
