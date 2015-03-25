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

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.i18n.phonenumbers.Phonemetadata.PhoneMetadata;

import android.telephony.PhoneNumberUtils;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.content.Context;
import android.text.TextUtils;
import android.telephony.Rlog;

import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.android.internal.telephony.ITelephony;

import com.mediatek.common.MediatekClassFactory;
import com.mediatek.common.telephony.IPhoneNumberExt;
import com.mediatek.common.featureoption.FeatureOption;

/** 
 * MediaTek Phone Number Utility extended API.
 *
 * @hide 
 */
public class PhoneNumberUtilsEx 
{
    static final String LOG_TAG = "PhoneNumberUtilsEx";
    private static final boolean DBG = false;
      
    public static final int ID_VALID_ECC = 1;
    public static final int ID_VALID_BUT_NEED_AREA_CODE = 2;
    public static final int ID_VALID = 3;
    public static final int ID_VALID_DOMESTIC_ONLY = 4;
    public static final int ID_INVALID = 5;
    public static final int ID_VALID_WHEN_CALL_EXIST = 6;

    private static IPhoneNumberExt mPhoneNumberExt;
    static {
        try{
            mPhoneNumberExt = MediatekClassFactory.createInstance(IPhoneNumberExt.class);
        } catch (Exception e){
            e.printStackTrace();
        }
    }

    private static final char PLUS_SIGN_CHAR = '+';
    private static final String PLUS_SIGN_STRING = "+";

    /**
     * @hide only for GsmMmiCode invoked by GSMPhone.dial
     */
    public static String extractGsmMmiNetworkPortion(String phoneNumber) {
        if (phoneNumber == null) {
            return null;
        }

        int len = phoneNumber.length();
        StringBuilder ret = new StringBuilder(len);
        boolean firstCharAdded = false;
        // mtk00732 allow "+" after "*" in GsmMmiCode
        boolean starfound = false;

        for (int i = 0; i < len; i++) {
            char c = phoneNumber.charAt(i);
            if (PhoneNumberUtils.isDialable(c) && (c != '+' || !firstCharAdded || 
                ((c == '+') && (i > 1) && (phoneNumber.charAt(i-1) == '*')))) {
                firstCharAdded = true;
                ret.append(c);
            } else if (PhoneNumberUtils.isStartsPostDial (c)) {
                break;
            }
        }

        return ret.toString();
    }

    /**
     * {@hide}
     */     
    public static String prependPlusToNumber(String number) {
        return ((mPhoneNumberExt != null) ? mPhoneNumberExt.prependPlusToNumber(number) : number);
    }

    /**
     * isVoiceMailNumber: checks a given number against the voicemail
     *   number provided by the RIL and SIM card. The caller must have
     *   the READ_PHONE_STATE credential.
     *
     * @param number the number to look up.
     * @param simId the SIM card ID
     * @return true if the number is in the list of voicemail. False
     * otherwise, including if the caller does not have the permission
     * to read the VM number.
     * @hide TODO: pending API Council approval
     * @internal
     */
    public static boolean isVoiceMailNumber(String number, int simId) {
        return ((mPhoneNumberExt != null) ? mPhoneNumberExt.isVoiceMailNumber(number, simId) : false);
    }

    /**
     * isIdleSsString: checks a given number if a idle ss string
     *
     * @param dialString dialing number
     * @return true if it idle ss string
     * @hide 
     * @internal
     */
    public static boolean isIdleSsString(String dialString) {
        Rlog.d(LOG_TAG, "isIdleSsString(): dialString = " + dialString);
        String newDialString = PhoneNumberUtils.stripSeparators(dialString);
        String networkPortion = PhoneNumberUtils.extractNetworkPortionAlt(newDialString);

        Pattern sPatternSuppService = Pattern.compile(
        "((\\*|#|\\*#|\\*\\*|##)(\\d{2,3})(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*))?)?)?)?#)(.*)");
        Matcher m;
        boolean ret = false;

        m = sPatternSuppService.matcher(networkPortion);

        if (m.matches()) {
            String action = m.group(2);
            String sc = m.group(3);
            String dialNumber = m.group(12);
            Rlog.d(LOG_TAG, "action = " + action + ", sc = " + sc + ", dialNumber = " + dialNumber);
            if ((sc != null && sc.equals("31")) && (action != null && (action.equals("*") || action.equals("#"))) && (dialNumber != null && dialNumber.length() != 0)) {
                Rlog.d(LOG_TAG, networkPortion + " is temporary CLIR");
            } else {
                ret = true;
            }
        } else if (networkPortion.endsWith("#")) {
            ret = true;
        } else if ((networkPortion != null && networkPortion.length() <= 2)
             && !((networkPortion.length() == 2 && networkPortion.charAt(0) == '1') || networkPortion.equals("0") || networkPortion.equals("00"))) {
            ret = true;
        }

        Rlog.d(LOG_TAG, networkPortion + " isIdleSsString: " + ret);
        return ret;
    }

    /**
     * isIncallSsString: checks a given number if an incall ss string
     *
     * @param dialString dialing number
     * @return true if it is an incall  ss string
     * @hide 
     * @internal
     */
    public static boolean isIncallSsString(String dialString) {

        return ((dialString != null && dialString.length() <= 2)
                && !PhoneNumberUtils.isEmergencyNumber(dialString)
                && !(dialString.equals("0") || dialString.equals("00")));
    }
    
    /**
     * Return the extracted phone number.
     *
     * @param phoneNumber Phone number string.
     * @return Return number whiched is extracted the CLIR part.
     * @hide 
     * @internal
     */
    public static String extractCLIRPortion(String phoneNumber) {
        if (phoneNumber == null) {
            return null;
        }

        if (phoneNumber.startsWith("*31#") || phoneNumber.startsWith("#31#")) {
            log(phoneNumber + " Start with *31# or #31#, return " + phoneNumber.substring(4));
            return phoneNumber.substring(4);
        } else if (phoneNumber.indexOf(PLUS_SIGN_STRING) != -1 && 
                   phoneNumber.indexOf(PLUS_SIGN_STRING) == phoneNumber.lastIndexOf(PLUS_SIGN_STRING)){
            Pattern p = Pattern.compile("(^[#*])(.*)([#*])(.*)(#)$");
            Matcher m = p.matcher(phoneNumber);
            if (m.matches()) {
                if ("".equals(m.group(2))) {
                    // Started with two [#*] ends with #
                    // So no dialing number and we'll just return "" a +, this handles **21#+
                    log(phoneNumber + " matcher pattern1, return empty string.");
                    return "";
                } else if (m.group(4) != null && m.group(4).length() > 1 && m.group(4).charAt(0) == PLUS_SIGN_CHAR) {
                    // Starts with [#*] and ends with #
                    // Assume group 4 is a dialing number such as *21*+1234554#
                    log(phoneNumber + " matcher pattern1, return " + m.group(4));
                    return m.group(4);
                }
            } else {
                p = Pattern.compile("(^[#*])(.*)([#*])(.*)");
                m = p.matcher(phoneNumber);
                if (m.matches() && m.group(4) != null && m.group(4).length() > 1 && m.group(4).charAt(0) == PLUS_SIGN_CHAR) {
                    // Starts with [#*] and only one other [#*]
                    // Assume the data after last [#*] is dialing number (i.e. group 4) such as *31#+11234567890.
                    // This also includes the odd ball *21#+
                    log(phoneNumber + " matcher pattern2, return " + m.group(4));
                    return m.group(4);
                }
            }
        }
        
        return phoneNumber;
    }
    
    /**
     * Return the validity of phone number according to country iso.
     * The numbering rule for each country is different.
     *
     * @param countryIso  Country ISO.
     * @param phoneNumber phone number string to be checked.
     * @return Return value may be one of the following:
     *         ID_VALID_ECC(1) - Emergency numbers;
     *         ID_VALID_BUT_NEED_AREA_CODE(2) - Valid but needs area code;
     *         ID_VALID(3) - Valid with area code or no need for area code;
     *         ID_VALID_DOMESTIC_ONLY(4) - Domestic valid only, which means that this numbers can be dialed in its home country but cannot be dialed out side of its home country;
     *         ID_INVALID(5) - Invalid which means the number is invalid in its home country.
     * @hide 
     * @internal
     */
    public static int isValidNumber(String countryIso, String phoneNumber) {
        Rlog.d(LOG_TAG, "[isValidNumber] countryIso: " + countryIso + ", phoneNumber: " + phoneNumber);
    
        if ((countryIso == null) || (phoneNumber == null)) {
    	    return ID_INVALID;
    	}
               
        String number = PhoneNumberUtils.extractNetworkPortion(PhoneNumberUtils.stripSeparators(phoneNumber));
        boolean matchResult = false;
        boolean areaCodeMatchResult = false;
        int result = ID_VALID;
        String patternString = "";
        String areaCodePattern = "";

        String[] CHINA_INTERNATIONAL_PREFIXS = {"00"};
        String[] TAIWAN_INTERNATIONAL_PREFIXS = {"002","005","006","007","009", "016", "017", "019"};

        if (countryIso.equalsIgnoreCase("cn")) {
            patternString = "1[3-8]{1}[0-9]{1}[0-9]{8}|" +           /* 11 digits with leading number between "130" and "189"  */
                            "01[3-8]{1}[0-9]{1}[0-9]{8}|" +          /* "0" + 11 digits with leading number between "130" and "189"  */
                            "[1-9]{1}[0-9]{5,7}|" +                  /* 6 or 7 or 8 digits with no leading "0" */
                            "11[0-9]{1}114|" +                       /* 6 digits, starts from "11" and ends with "114" */ 
                            "400[0-9]{7}|" +                         /* 10 digits with leading number "400" */
                            "179[0-9]{8,}|" +                        /* At least 11 digits with leading number "179" */
                            "125[0-9]{8,}|";                         /* At least 11 digits with leading number "125" */

            areaCodePattern = "010[1-9]{1}[0-9]{7}|" +                 /* 010(3 area code) + 8 digital number with no leading"0" */
                              "02[0-9]{1}[1-9]{1}[0-9]{7}|" +          /* 02X(3 area code) + 8 digital number with no leading"0" */
                              "0[3-9]{1}[0-9]{2}[1-9]{1}[0-9]{6,7}|" + /* 0XXX(4 area code) + 7 or 8 digital number with no leading"0" */
                              "010[1-9]{1}[0-9]{2,4}|" +               /* 010(3 area code) + 3~5 special number with no leading "0" */
                              "02[0-9]{1}[1-9]{1}[0-9]{2,4}|" +        /* 02X(3 area code) + 3~5 special number with no leading "0" */
                              "0[3-9]{1}[0-9]{2}[1-9]{1}[0-9]{2,4}|" + /* 0XXX(4 area code) + 3~5 special number with no leading "0" */
                              "01011[0-9]{1}114|" +                    /* 010(3 area code) + 6 digits, starts from "11" and ends with "114" */ 
                              "02[0-9]{1}11[0-9]{1}114|" +             /* 02X(3 area code) + 6 digits, starts from "11" and ends with "114" */ 
                              "0[3-9]{1}[0-9]{2}11[0-9]{1}114|";       /* 0XXX(4 area code) + 6 digits, starts from "11" and ends with "114" */ 

            /* International prefix match */
            for(String prefix : CHINA_INTERNATIONAL_PREFIXS){
                if(number.startsWith(prefix)){                      /* The number starts with CHINA_INTERNATIONAL_PREFIX */
                    Rlog.d(LOG_TAG, "isValidNumber = CN start with " + prefix);
                    return result;
                }
            }
        }
        else if (countryIso.equalsIgnoreCase("tw")) {
           patternString = "09[0-9]{8}|" +                        /* 10 digits with leading number "09" */
                           "0[2-8]{1}[0-9]{7,8}|";                /* 9 or 10 digits with leading number between "02" and "08" */

            /* International prefix match */
            for(String prefix : TAIWAN_INTERNATIONAL_PREFIXS){
                if(number.startsWith(prefix)){                      /* The number starts with TW_INTERNATIONAL_PREFIX */
                    Rlog.d(LOG_TAG, "isValidNumber = TW start with " + prefix);
                    return result;
                }
            }
        } else {
            // Currently only support "cn" and "tw", so return ID_VALID directly. 
            return ID_VALID;
        }
        
        patternString = patternString + "[1-9]{1}[0-9]{2,4}|" +   /* 3 to 5 digits with no leading "0" */
                                        "000|08";                 /* ECC number : "000" and "08" */

        Pattern p = Pattern.compile(patternString);
        Matcher m = p.matcher(number);
        matchResult = m.matches();
        Rlog.d(LOG_TAG, "number = " + number +", matchResult = " + matchResult);

        if (!matchResult && areaCodePattern.length() > 0)
        {
            p = Pattern.compile(areaCodePattern);
            m = p.matcher(number);
            areaCodeMatchResult = m.matches();
            Rlog.d(LOG_TAG, "number = " + number +", areaCodeMatchResult = " + areaCodeMatchResult);
        }
        
        if (matchResult || areaCodeMatchResult) {
            if (PhoneNumberUtils.isEmergencyNumber(phoneNumber)) {
                result = ID_VALID_ECC;
            } else if (isAreaCodeNeeded(countryIso, phoneNumber)) {
                result = ID_VALID_BUT_NEED_AREA_CODE;
            } else if (isDomesticOnly(countryIso, phoneNumber)) {
                result = ID_VALID_DOMESTIC_ONLY;
            } else if (areaCodeMatchResult && isValidNationalNumber(countryIso, phoneNumber) == false) {
                result = ID_INVALID;
            }
        } else if (isSpecialMmiNumber(phoneNumber) == true) {
            result = ID_VALID_WHEN_CALL_EXIST;
        } else {
            result = ID_INVALID;
        }
        
        return result;
    }
    
    /**
     * Return the international prefix string according to country iso.
     *
     * @param countryIso  Country ISO.
     * @return Return international prefix.
     * @hide 
     * @internal
     */
    public static String getInternationalPrefix(String countryIso) {
    	  if (countryIso == null) {
    	  	  return "";
    	  }
    	
        PhoneNumberUtil util = PhoneNumberUtil.getInstance();
        PhoneMetadata metadata = util.getMetadataForRegion(countryIso);
        if (metadata != null) {
        	  String prefix = metadata.getInternationalPrefix();
        	  if (countryIso.equalsIgnoreCase("tw")) {
        	  	  prefix = "0(?:0[25679] | 16 | 17 | 19)";
        	  }
        	  return prefix;
        }
        
        return null;
    }
    
    
    /**
     * Check if the phone number is only for domestic(home country).
     *
     * @param countryIso  Country ISO.
     * @param phoneNumber phone number string to be checked.
     * @return Return true if the phone number is only for domestic, else return false.
     * @hide 
     * @internal
     */
    private static boolean isDomesticOnly(String countryIso, String phoneNumber) {
    	  if ((countryIso == null) || (phoneNumber == null)) {
    	  	  return false;
    	  }

        boolean result = true;
        String number = PhoneNumberUtils.extractNetworkPortion(PhoneNumberUtils.stripSeparators(phoneNumber));
        if (countryIso.equalsIgnoreCase("cn") ||
                countryIso.equalsIgnoreCase("tw")) {
            String patternString = "[1-9]{1}[0-9]{2,5}";
            Pattern p = Pattern.compile(patternString);
            Matcher m = p.matcher(number);
            result = (m.matches() && !PhoneNumberUtils.isEmergencyNumber(phoneNumber));
        }
        return result;
    }

    /**
     * Check if the phone number should be added area code to dial out according to country iso.
     * The numbering rule for each country is different.
     *
     * @param countryIso  Country ISO.
     * @param phoneNumber phone number string to be checked.
     * @return Return true if the phone number should be added area code before dialing out, else return false.
     * @hide 
     * @internal     
     */
    public static boolean isAreaCodeNeeded(String countryIso, String phoneNumber) {
        if ((countryIso == null) || (phoneNumber == null)) {
            return false;
        }

        String number = PhoneNumberUtils.extractNetworkPortion(PhoneNumberUtils.stripSeparators(phoneNumber));
        boolean result = false;
        if (countryIso.equalsIgnoreCase("cn")) {
           String patternString = "[1-9]{1}[0-9]{2,7}";  /* 3 to 8 digits with no leading "0" */
           Pattern p = Pattern.compile(patternString);
           Matcher m = p.matcher(number);
           result = m.matches();
        }
        
        return result;
    }

    /**
     * Check if the phone number is valid national number. (Only check "CN" case now)
     *
     * @param countryIso  Country ISO.
     * @param phoneNumber phone number string to be checked.
     * @return Return true if the phone number is match national number rules, else return false.
     * @hide 
     * @internal     
     */
    private static boolean isValidNationalNumber(String countryIso, String phoneNumber) {
    	if ((countryIso == null) || (phoneNumber == null)) {
    	    return false;
    	}

        Rlog.d(LOG_TAG, "[isValidNationalNumber]countryIso: " + countryIso + ", phonenumber: " + phoneNumber);
        
        if (phoneNumber.startsWith("0"))
        {
            phoneNumber = phoneNumber.substring(1, phoneNumber.length());
            Rlog.d(LOG_TAG, "[isValidNationalNumber] cut '0' - phonenumber: " + phoneNumber);
        }

        boolean result = false;
        String number = PhoneNumberUtils.stripSeparators(phoneNumber);

        /*Reference: http://zh.wikipedia.org/wiki/%E4%B8%AD%E5%8D%8E%E4%BA%BA%E6%B0
            %91%E5%85%B1%E5%92%8C%E5%9B%BD%E7%94%B5%E8%AF%9D%E5%8C%BA%E5%8F%B7 */
        String[] CHINA_AREA_PREFIXS = {
            "10", 
            "21", "22", "23", "24", "25", "26", "27", "28", "29", "20",
            "311", "312", "313", "314", "315", "316", "317", "318", "319", "310", "335",
            "349", "351", "352", "353", "354", "355", "356", "357", "358", "350",
            "371", "372", "373", "374", "375", "376", "377", "378", "379", "370",
            "391", "392", "393", "394", "395", "396", "397", "398",
            "411", "412", "414", "415", "416", "417", "418", "419", "421", "427", "429",
            "431", "432", "433", "434", "435", "436", "437", "438", "439", 
            "451", "452", "453", "454", "455", "456", "457", "458", "459", "464", "467", "468", "469",
            "471", "472", "473", "474", "475", "476", "477", "478", "479", "470", "482", "483",
            "511", "512", "513", "514", "515", "516", "517", "518", "519", "510", "523", "527",
            "531", "532", "533", "534", "535", "536", "537", "538", "539", "530", "543", "546", 
            "631", "632", "633", "634", "635", 
            "551", "552", "553", "554", "555", "555", "556", "557", "558", "559", 
            "561", "562", "563", "564", "565", "566",
            "571", "572", "573", "574", "575", "576", "577", "578", "579", "570", "580",
            "591", "592", "593", "594", "595", "596", "597", "598", "599",
            "631", "632", "633", "634", "635", 
            "660", "662", "663", "668",
            "691", "692", 
            "711", "712", "713", "714", "715", "716", "717", "718", "719", "710", "722", "724", "728",
            "731", "734", "735", "736", "737", "738", "739", "730", "743", "744", "745", "746",
            "750", "751", "752", "753", "754", "755", "756", "757", "758", "759", 
            "760", "762", "763", "766", "768", "769", "660", "662", "663", "668",
            "771", "772", "773", "774", "775", "776", "777", "778", "779", "770",
            "791", "792", "793", "794", "795", "796", "797", "798", "799", "790", "701",
            "812", "813", "816", "817", "818", "825", "826", "827", 
            "831", "832", "833", "834", "835", "836", "837", "838", "839", "830", 
            "851", "852", "853", "854", "855", "856", "857", "858", "859",
            "871", "872", "873", "874", "875", "876", "877", "878", "879", "870", 
            "883", "886", "887", "888", "691", "692",
            "891", "892", "893", "894", "895", "896", "897", "898",
            "911", "912", "913", "914", "915", "916", "917", "919",
            "931", "932", "933", "934", "935", "936", "937", "938", "939", "930", "940", "941", "943",
            "951", "952", "953", "954", "955",
            "971", "972", "973", "974", "975", "976", "977", "979", "970",
            "991", "992", "993", "994", "995", "996", "997", "998", "999", "990",
            "901", "902", "903", "906", "908", "909"};

        if (countryIso.equalsIgnoreCase("cn")) {
            for(String prefix : CHINA_AREA_PREFIXS){
                if(number.startsWith(prefix)) {
                    if (number.charAt(prefix.length()) == '0'){       /* The number after the area code is "0" - invalid number  */
                        Rlog.d(LOG_TAG, "isValidNationalNumber = CN invalid number " + number.substring(0, prefix.length() + 1));
                        result = false;
                    } else {
                        Rlog.d(LOG_TAG, "isValidNationalNumber = CN number " + number.substring(0, prefix.length() + 1));
                        result = true;
                    }
                    break;
                }
            }
        }
        return result;
    }


    /**
     * Check if the phone number is a special MMI number. 
     *
     * @param phoneNumber phone number string to be checked.
     * @return Return true if the phone number is match special MMI number, else return false.
     * @hide
     * @internal     
     */
    private static boolean isSpecialMmiNumber(String phoneNumber) {
        String patternString = "[0-5]{1}|" +                 /* 0, 1, 2, 3, 4, 5 */
                               "[1-2]{1}[1-9]{1}";           /* 1x, 2x with x = 1~9  */
  
        Pattern p = Pattern.compile(patternString);
        Matcher m = p.matcher(phoneNumber);
        return m.matches();
    }

    /**
     * Get coutrycode to region code mapping table. 
     *
     * @return Return the mapping table without unknown region code(001).
     * @hide 
     * @internal     
     */
    public static Map<Integer, List<String>> getCountryCodeToRegionCodeMap() {
        PhoneNumberUtil util = PhoneNumberUtil.getInstance();
        Map<Integer, List<String>> regionMap = new HashMap<Integer, List<String>>(286);
        Set<Integer> countryCodeKeySet = util.getCountryCallingCodeToRegionCodeMap().keySet();
        
        for (Iterator iterator = countryCodeKeySet.iterator(); iterator.hasNext();) {
            Integer countryCode = (Integer) iterator.next();
            List<String> regionCodeList = util.getCountryCallingCodeToRegionCodeMap().get(countryCode);
            for (Iterator iterator2 = regionCodeList.iterator(); iterator2.hasNext();) {
                String countryISO = (String)iterator2.next();
    
                if (countryISO.compareTo("001") != 0)
                {
                  regionMap.put(countryCode, regionCodeList);
                  break;
                }
            }
        }
    
        return regionMap;
    }


    private static void log(String msg) {
        Rlog.d(LOG_TAG, msg);
    }

}

