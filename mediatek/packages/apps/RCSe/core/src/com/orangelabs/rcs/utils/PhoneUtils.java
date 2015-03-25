/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.SystemProperties;
import android.preference.PreferenceManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.mediatek.rcse.api.Logger;
import com.mediatek.common.featureoption.FeatureOption;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.LauncherUtils;

import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.i18n.phonenumbers.Phonemetadata.PhoneMetadata;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.sip.ListeningPoint;

import org.xbill.DNS.Lookup;
import org.xbill.DNS.NAPTRRecord;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.TextParseException;
import org.xbill.DNS.Type;

import com.mediatek.telephony.TelephonyManagerEx;

/**
 * Phone utility functions
 * 
 * @author jexa7410
 */
public class PhoneUtils {
	/**
	 * Tel-URI format
	 */
	private static boolean TEL_URI_SUPPORTED = true;

	/**
     * M: Added to resolve the issue of can't sync contacts to RCSe. @{
     */
    /**
	 * Country code
	 */
	private static String COUNTRY_CODE = "+34";	
	/**
	 * M: Added to avoid the magic number problem @{T-Mobile
	 */
	private static final String INTERNATIONAL_PREFIX = "00";
	/** T-Mobile@} */
	/** M: add for format UUSD and star codes @{T-Mobile */
	private static final String Tel_URI_PREFIX = "tel:";
	private static final String SIP_URI_PREFIX = "sip:";
	private static final String AT_SIGN = "@";
	private static final String POUND_SIGN = "#";
	private static final String POUND_SIGN_HEX_VALUE = "23%";
	/** T-Mobile@} */
	/**
	 * Country area code
	 */
	private static String COUNTRY_AREA_CODE = "0";

    /** M: add for log @{ */
    /**
     * Class tag
     */
    private static final String TAG = "PhoneUtils";
    /** @} */

    /**
     * M: resolve issue for error handling international call prefix, eg.
     * 0086xxx. @{
     */
    /**
     * Plus
     */
    private static String COUNTRY_CODE_PLUS = "+";
    /**
     * Method in framework.
     */
    private static final String METHOD_GET_METADATA = "getMetadataForRegion";
    /**
     * Copy from framework to enrich international prefix.
     */
    private static final String REGION_TW = "TW";
    private static final String INTERNATIONAL_PREFIX_TW = "0(?:0[25679] | 16 | 17 | 19)";
    /**
     * International prefix.
     */
    private static String sInternationalPrefix = null;
    /** @} */

	/**
     * For debug, because at present we do not have vf sim card.
     */
    public static final HashMap<String, String> NORMAL_NUMBER_TO_VODAFONE_ACCOUNT = new HashMap<String, String>();
    public static final HashMap<String, String> VODAFONE_ACCOUNT_TO_NORMAL_NUMBER = new HashMap<String, String>();
    private static final String KEY_NUMBER ="numbers";
    private static final String KEY_ACCOUNT ="accounts";
    private static final String ITEM_DELIMITER ="%2D";
    /**
     * M: Add to indicates whether the RCS-e only APN is debug mode.@{
     */
    public static  boolean sIsApnDebug = false;
    /**
     * @}
     */
    
    /**
     * Add for the RCS-e only APN implementation. @{
     */
    private static final int IP_ADDRESS_SEGMENTS = 4;
    private static final String IP_PATTERN = "\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}";
    private static final String IP_SPLITTER = "\\.";
    /**
     * @}
     */

    /**
    * Telephone manager ex
    */
    private static TelephonyManagerEx mTelephonyManagerEx = null ;
    
   
    
    private static Context sContext = null;
    static {
        initNumbersAndAccounts();
        
        sIsApnDebug = LauncherUtils.getDebugMode(sContext);
    }



    private static void initNumbersAndAccounts() {
        if (sContext == null) {
            // AndroidFactory.getApplicationContext() may return null.
            sContext = AndroidFactory.getApplicationContext();
        }
        if (sContext != null) {
            SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(sContext);
            String numbersFragment = prefs.getString(KEY_NUMBER, "");
            String accountsFragment = prefs.getString(KEY_ACCOUNT, "");
            String[] numbers = numbersFragment.split(ITEM_DELIMITER);
            String[] accounts = accountsFragment.split(ITEM_DELIMITER);
            int size = 0;
            if ((size = numbers.length) == accounts.length) {
                for (int i = 0; i < size; i++) {
                    NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.put(numbers[i], accounts[i]);
                    VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.put(accounts[i], numbers[i]);
                }
            } else {
                Logger.d(TAG, "size of numbers and accounts are not equal");
            }
        } else {
            Logger.d(TAG, "context is null, loading shared preference fail");
        }
    }

    /**
     * Add entry to map.
     * 
     * @param number The number.
     * @param account The vodafone account.
     */
    public static void addMapEntry(String number, String account) {
        Logger.d(TAG, "addMapEntry entry, number: " + number + " account: " + account);
        NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.remove(number);
        if (NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.containsValue(account)) {
            NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.values().remove(account);
        }
        VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.remove(account);
        if (VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.containsValue(number)) {
            VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.values().remove(number);
        }
        NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.put(number, account);
        VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.put(account, number);
        Set<String> keys = NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.keySet();
        String numberFragment = "";
        String accountFragment = "";
        int size = keys.size();
        int index = 0;
        for (String key : keys) {
            index++;
            numberFragment = numberFragment + key;
            accountFragment = accountFragment + NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.get(key);
            if (index < size) {
                numberFragment = numberFragment + ITEM_DELIMITER;
                accountFragment = accountFragment + ITEM_DELIMITER;
            }
        }
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(sContext);
        prefs.edit().putString(KEY_NUMBER, numberFragment).apply();
        prefs.edit().putString(KEY_ACCOUNT, accountFragment).apply();
        Logger.d(TAG, "addMapEntry exit");
    }

    /**
     * Get vodafone account mapped with the specific number.
     * 
     * @param number The number to be mapped to vodafone account.
     * @return The vodafone account.
     */
    public static String getVfAccountViaNumber(String number) {
        return NORMAL_NUMBER_TO_VODAFONE_ACCOUNT.get(number);
    }

    /**
     * Get the number mapped to the specific vodafone account.
     * 
     * @param account The vodafone account.
     * @return The number mapped to the specific vodafone account.
     */
    public static String getNumberViaVfAccount(String account) {
        return VODAFONE_ACCOUNT_TO_NORMAL_NUMBER.get(account);
    }
    
	/**
	 * Set the country code
	 * 
	 * @param context Context
	 */
	public static synchronized void initialize(Context context) {
		RcsSettings.createInstance(context);
		TEL_URI_SUPPORTED = RcsSettings.getInstance().isTelUriFormatUsed();
		COUNTRY_CODE = RcsSettings.getInstance().getCountryCode();
		COUNTRY_AREA_CODE = RcsSettings.getInstance().getCountryAreaCode();
        /**
         * M: resolve issue for error handling international prefix code, eg.
         * 0086xxx. @{
         */
        String countryIso = null;
        try {
            countryIso = getDefaultSimCountryIso();
        } catch (ClassCastException e) {
            e.printStackTrace();
            Logger.e(TAG, "initialize() plz check whether your load matches your code base");
        }
        if (countryIso != null) {
            sInternationalPrefix = getInternationalPrefix(countryIso.toUpperCase());
        }
        Logger.d(TAG, "initialize() countryIso: " + countryIso + " sInternationalPrefix: "
                + sInternationalPrefix + " COUNTRY_CODE: " + COUNTRY_CODE);
        /** @} */
	}

    /**
     * Init context for loading preference.
     * 
     * @param context The context.
     */
    public static void initContext(Context context) {
        sContext = context;
        initNumbersAndAccounts();
	}

	/**
	 * Returns the country code
	 * 
	 * @return Country code
	 */
	public static String getCountryCode() {
		return COUNTRY_CODE;
	}
	
	/**
     * M: resolve issue for error handling international prefix code, eg.
     * 0086xxx. @{
     */

    /**
	 * Format a phone number to international format
	 * 
	 * @param number Phone number
	 * @return International number
	 */
	public static String formatNumberToInternational(String number) {
		if (number == null) {
			return null;
		}
		
		// Remove spaces
		number = number.trim();

		// Strip all non digits
		String phoneNumber = PhoneNumberUtils.stripSeparators(number);
        if(phoneNumber.equals(""))
        {
        	return "";
        }
        if (sInternationalPrefix == null) {
            String countryIso = null;
            try {
                countryIso = getDefaultSimCountryIso();
            } catch (ClassCastException e) {
                e.printStackTrace();
                Logger.e(TAG,
                        "formatNumberToInternational() plz check whether your load matches your code base");
            }
            if (countryIso != null) {
                sInternationalPrefix = getInternationalPrefix(countryIso.toUpperCase());
            }
            Logger.d(TAG, "formatNumberToInternational() countryIso: " + countryIso
                    + " sInternationalPrefix: " + sInternationalPrefix);
        }
        if (sInternationalPrefix != null) {
            Pattern pattern = Pattern.compile(sInternationalPrefix);
            Matcher matcher = pattern.matcher(number);
            StringBuilder formattedNumberBuilder = new StringBuilder();
            if (matcher.lookingAt()) {
                int startOfCountryCode = matcher.end();
                formattedNumberBuilder.append(COUNTRY_CODE_PLUS);
                formattedNumberBuilder.append(number.substring(startOfCountryCode));
                phoneNumber = formattedNumberBuilder.toString();
            }
        }
        Logger.d(TAG, "formatNumberToInternational() number: " + number + " phoneNumber: "
                + phoneNumber + " sInternationalPrefix: " + sInternationalPrefix);
		// Format into international
		if (phoneNumber.startsWith("00" + COUNTRY_CODE.substring(1))) {
			// International format
			phoneNumber = COUNTRY_CODE + phoneNumber.substring(4);
		} else
		if ((COUNTRY_AREA_CODE != null) && (COUNTRY_AREA_CODE.length() > 0) &&
				phoneNumber.startsWith(COUNTRY_AREA_CODE)) {
			// National number with area code
			phoneNumber = COUNTRY_CODE + phoneNumber.substring(COUNTRY_AREA_CODE.length());
		} else
		if (!phoneNumber.startsWith("+")) {
			// National number
			phoneNumber = COUNTRY_CODE + phoneNumber;
		}
		return phoneNumber;
	}
    /** @} */

	/**
	 * Format a phone number to a SIP URI
	 * 
	 * @param number Phone number
	 * @return SIP URI
	 */
	public static String formatNumberToSipUri(String number) {
		if (number == null) {
			return null;
		}

		// Remove spaces
		number = number.trim();
		
		// Extract username part
		if (number.startsWith("tel:")) {
			number = number.substring(4);
		} else		
		if (number.startsWith("sip:")) {
			number = number.substring(4, number.indexOf("@"));
		}
		
		if (TEL_URI_SUPPORTED) {
			// Tel-URI format
			return "tel:" + formatNumberToInternational(number);
		} else {
			// SIP-URI format
			return "sip:" + formatNumberToInternational(number) + "@" +
				ImsModule.IMS_USER_PROFILE.getHomeDomain() + ";user=phone";	 
		}
	}

	/**
	 * Format a phone number to a SIP address
	 * 
	 * @param number Phone number
	 * @return SIP address
	 */
	public static String formatNumberToSipAddress(String number) {
		String addr = formatNumberToSipUri(number);	 
		String displayName = RcsSettings.getInstance().getUserProfileImsDisplayName();
		if ((displayName != null) && (displayName.length() > 0)) {
			addr = "\"" + displayName + "\" <" + addr + ">"; 
		}
		return addr;
	}
	
	/**
	 * Extract user part phone number from a SIP-URI or Tel-URI or SIP address
	 * 
	 * @param uri SIP or Tel URI
	 * @return Number or null in case of error
	 */
	public static String extractNumberFromUri(String uri) {
		if (uri == null) {
			return null;
		}

		try {
			// Extract URI from address
			int index0 = uri.indexOf("<");
			if (index0 != -1) {
				uri = uri.substring(index0+1, uri.indexOf(">", index0));
			}			
			
			// Extract a Tel-URI
			int index1 = uri.indexOf("tel:");
			if (index1 != -1) {
				uri = uri.substring(index1+4);
			}
			
			// Extract a SIP-URI
			index1 = uri.indexOf("sip:");
			if (index1 != -1) {
				int index2 = uri.indexOf("@", index1);
				uri = uri.substring(index1+4, index2);
			}
			
			// Remove URI parameters
			int index2 = uri.indexOf(";"); 
			if (index2 != -1) {
				uri = uri.substring(0, index2);
			}
			
			// Format the extracted number (username part of the URI)
			return formatNumberToInternational(uri);
		} catch(Exception e) {
			return null;
		}
	}

	/**
	 * Extract display name from URI
	 * 
	 * @param uri URI
	 * @return Display name or null
	 */
	public static String extractDisplayNameFromUri(String uri) {
		if (uri == null) {
			return null;
		}

		try {
			int index0 = uri.indexOf("\"");
			if (index0 != -1) {
				int index1 = uri.indexOf("\"", index0+1);
				if (index1 > 0) {
					return uri.substring(index0+1, index1);
				}
			}			
			
			return null;
		} catch(Exception e) {
			return null;
		}
	}

	/**
	 * Compare phone number between two contacts
	 * 
	 * @param contact1 First contact
	 * @param contact2 Second contact
	 * @return Returns true if numbers are equals
	 */
	public static boolean compareNumbers(String contact1, String contact2) {
		String number1 = PhoneUtils.extractNumberFromUri(contact1);
		String number2 = PhoneUtils.extractNumberFromUri(contact2);
		if ((number1 == null) || (number2 == null)) {
			return false;
		}
		return number1.equals(number2);
	}
		
	/**
	 * Check if phone number is global
	 * @param phone
	 * @return
	 */
	public static boolean isGlobalPhoneNumber(final String phone) {
		if (phone == null)
			return false;
		if (PhoneNumberUtils.isGlobalPhoneNumber(phone)) {
			if (phone.length() > PhoneUtils.getCountryCode().length()) {
				return true;
			}
		}
		return false;
	}

	public static boolean isANumber(String number){
	    int size = number == null ? 0 : number.length();
	    if(size == 0){
	        return false;
	    }
	    int position = 0;
	    if(number.startsWith("+")){
	        position = 1;
	    }
	    for(; position < size; ++position){
            /**
             * M: Modified to resolve the sender name is not correct issue @{
             */
	        if(number.charAt(position) >= '0' && number.charAt(position) <= '9'){
	        /**
	         * @}
	         */
	            continue;
	        }else{
	            return false;
	        }
	    }
	    return true;
	}

    /** M: add for SIP OPTION optimization @{ */
    /**
     * Verify whether contact is a possible vodafone number.
     * 
     * @param contact The contact.
     * @return True if contact is possible vodafone number, otherwise false.
     */
    public static boolean isVodafoneValidNumber(String contact) {
        return true;
    }
    /** @} */
    /** M: add for SIP OPTION optimization @{ T-Mobile*/
    /**
	 * Encoding "#" sign to his hex decimal value.
	 * 
	 * @param number Phone number which may contains "#" sign
	 * @return number that "#" sign has been replace by "23%"
	 */
	public static String encodingPoundSignToHexDecimalValue(String number) {
		if (number == null) {
			return null;
		}
		
		return number.replace(POUND_SIGN, POUND_SIGN_HEX_VALUE);
	}
	
	/**
	 * Format a USSD code phone number to a SIP URI
	 * 
	 * @param ussdPhoneNumber USSD code phone number
	 * @return String SIP URI
	 */
	public static String formatUSSDCodeToSipUri(String ussdPhoneNumber) {
		if (ussdPhoneNumber == null) {
			return null;
		}

		// Remove spaces
		ussdPhoneNumber = ussdPhoneNumber.trim();
		
		// Extract username part
		if (ussdPhoneNumber.startsWith(Tel_URI_PREFIX)) {
			ussdPhoneNumber = ussdPhoneNumber.substring(4);
		} else if (ussdPhoneNumber.startsWith(SIP_URI_PREFIX)) {
			ussdPhoneNumber = ussdPhoneNumber.substring(4, ussdPhoneNumber.indexOf(AT_SIGN));
		}
		
		ussdPhoneNumber = encodingPoundSignToHexDecimalValue(ussdPhoneNumber);
		
		// SIP-URI format
		return SIP_URI_PREFIX + ussdPhoneNumber + AT_SIGN + ImsModule.IMS_USER_PROFILE.getHomeDomain();	 
	}
	
	/** T-Mobile@} */
	
	/** M: add for emergency number @{T-Mobile */
	/**
	 * Verify whether number is a emergency number.
	 * 
	 * @param number
	 *            the call number.
	 * @return True if number is emergency number, otherwise false.
	 */
	public static boolean isEmergencyNumber(String number) {
		Logger.d(TAG, "isEmergencyNumber entry, number: " + number);
		boolean result = false;
		result = PhoneNumberUtils.isEmergencyNumber(number);
		Logger.d(TAG, "isEmergencyNumber exit, result: " + result);
		return result;
	}
	/** T-Mobile@} */
	
	/**
	 * M: Add for RCS-e only APN. @{
	 */
	private static boolean isIpAddress(String value) {
        if (value.matches(IP_PATTERN)) {
            String ipSegment[] = value.split(IP_SPLITTER);
            if (ipSegment.length < IP_ADDRESS_SEGMENTS) {
                return false;
            } else {
                boolean isValid = (Integer.parseInt(ipSegment[0]) < 255)
                        && (Integer.parseInt(ipSegment[1]) < 255)
                        && (Integer.parseInt(ipSegment[2]) < 255)
                        && (Integer.parseInt(ipSegment[3]) < 255);
                return isValid;
            }
        } else {
            return false;
        }
    }

    /**
     * Return the InetAddress if the parameter is a IP address or host name
     * 
     * @param value IP address or host name
     * @return return the InetAddress or -1
     */
    public static int getInetAddress(String value) {
        if (value == null || TextUtils.isEmpty(value)) {
            return -1;
        } else {
            value = value.trim();
            if (isIpAddress(value)) {
                return getInetAddressFromIp(value);
            } else {
                return getInetAddressFromHost(value);
            }
        }
    }

    private static int getInetAddressFromHost(String hostName) {
        InetAddress inetAddress;
        try {
            inetAddress = InetAddress.getByName(hostName);
        } catch (UnknownHostException e) {
            Logger.d(TAG, "getInetAddressFromHost()-UnknownHostException");
            String ipAddress = getImsServerIpAddress();
            if (ipAddress == null) {
                return -1;
            } else {
                return getInetAddressFromIp(ipAddress);
            }
        }
        byte[] addressBytes;
        int address;
        addressBytes = inetAddress.getAddress();
        address = ((addressBytes[3] & 0xff) << 24) | ((addressBytes[2] & 0xff) << 16)
                | ((addressBytes[1] & 0xff) << 8) | (addressBytes[0] & 0xff);
        return address;
    }

    private static int getInetAddressFromIp(String ipAddress) {
        String ipSegment[] = ipAddress.split(IP_SPLITTER);
        if (ipSegment.length < IP_ADDRESS_SEGMENTS) {
            return -1;
        } else {
            int firstSeg = Integer.parseInt(ipSegment[0]);
            int secondSrg = Integer.parseInt(ipSegment[1]);
            int thirdSrg = Integer.parseInt(ipSegment[2]);
            int fourthSrg = Integer.parseInt(ipSegment[3]);
            if ((firstSeg < 255) && (secondSrg < 255) && (thirdSrg < 255) && (fourthSrg < 255)) {
                int address = ((fourthSrg & 0xff) << 24) | ((thirdSrg & 0xff) << 16)
                        | ((secondSrg & 0xff) << 8) | (firstSeg & 0xff);
                return address;
            } else {
                return -1;
            }
        }
    }
    
    /**
     * Get DNS NAPTR records
     * 
     * @param domain Domain
     * @return NAPTR records or null if no record
     */
    private static Record[] getDnsNAPTR(String domain) {
        try {
            Logger.d(TAG, "DNS NAPTR lookup for " + domain);
            Lookup lookup = new Lookup(domain, Type.NAPTR);
            Record[] result = lookup.run();
            int code = lookup.getResult();
            if (code != Lookup.SUCCESSFUL) {
                Logger.d(TAG, "Lookup error: " + code + "/" + lookup.getErrorString());
            }
            return result;
        } catch (TextParseException e) {
            Logger.d(TAG, "Not a valid DNS name");
            return null;
        } catch (IllegalArgumentException e) {
            Logger.d(TAG, "Not a valid DNS type");
            return null;
        }
    }
    
    private static String getImsServerIpAddress() {
        // First try to resolve via a NAPTR query, then a SRV
        // query and finally via A query
        String imsProxyProtocol = RcsSettings.getInstance().getSipDefaultProtocolForMobile();
        String imsProxyAddr = RcsSettings.getInstance().getImsProxyAddrForMobile();
        int imsProxyPort = RcsSettings.getInstance().getImsProxyPortForMobile();
        
        Logger.d(TAG, "Resolve IMS proxy address...");
        String ipAddress = null;
        // DNS NAPTR lookup
        String service;
        if (imsProxyProtocol.equalsIgnoreCase(ListeningPoint.UDP)) {
            service = "SIP+D2U";
        } else if (imsProxyProtocol.equalsIgnoreCase(ListeningPoint.TCP)) {
            service = "SIP+D2T";
        } else if (imsProxyProtocol.equalsIgnoreCase(ListeningPoint.TLS)) {
            service = "SIPS+D2T";
        } else {
            Logger.d(TAG, "Unkown SIP protocol");
            return ipAddress;
        }
        Record[] naptrRecords = getDnsNAPTR(imsProxyAddr);
        if ((naptrRecords != null) && (naptrRecords.length > 0)) {
            Logger.d(TAG, "NAPTR records found: " + naptrRecords.length);
            for (int i = 0; i < naptrRecords.length; i++) {
                NAPTRRecord naptr = (NAPTRRecord) naptrRecords[i];
                Logger.d(TAG, "NAPTR record: " + naptr.toString());
                Logger.d(TAG, "naptr.getService(): " + naptr.getService() + ", service = "
                        + service);
                if ((naptr != null) && naptr.getService().equalsIgnoreCase(service)) {
                    // DNS SRV lookup
                    Record[] srvRecords = getDnsSRV(naptr.getReplacement().toString());
                    Logger.d(TAG, "srvRecords: " + srvRecords);
                    if ((srvRecords != null) && (srvRecords.length > 0)) {
                        Logger.d(TAG, "NAPTR:DNS SRV lookup success");
                        SRVRecord srvRecord = getBestDnsSRV(srvRecords);
                        ipAddress = getDnsA(srvRecord.getTarget().toString());
                        imsProxyPort = srvRecord.getPort();
                        Logger.d(TAG, "ipAddress = " + ipAddress + ", imsProxyPort = "
                                + imsProxyPort);
                    } else {
                        // Direct DNS A lookup
                        Logger.d(TAG, "NAPTR:DNS SRV lookup failed then do direct DNS A lookup");
                        ipAddress = getDnsA(imsProxyAddr);
                    }
                }
            }
        } else {
            // Direct DNS SRV lookup
            Logger.d(TAG, "No NAPTR record found: use DNS SRV instead");
            String query;
            if (imsProxyAddr.startsWith("_sip.")) {
                query = imsProxyAddr;
            } else {
                query = "_sip._" + imsProxyProtocol.toLowerCase() + "." + imsProxyAddr;
            }
            Record[] srvRecords = getDnsSRV(query);
            if ((srvRecords != null) && (srvRecords.length > 0)) {
                SRVRecord srvRecord = getBestDnsSRV(srvRecords);
                ipAddress = getDnsA(srvRecord.getTarget().toString());
                imsProxyPort = srvRecord.getPort();
            } else {
                // Direct DNS A lookup
                Logger.d(TAG, "No SRV record found: use DNS A instead");
                ipAddress = getDnsA(imsProxyAddr);
            }
        }
        return ipAddress;
    }
    
    /**
     * Get DNS SRV records
     * 
     * @param domain Domain
     * @return SRV records or null if no record
     */
    private static Record[] getDnsSRV(String domain) {
        try {
            Logger.d(TAG, "DNS SRV lookup for " + domain);
            Lookup lookup = new Lookup(domain, Type.SRV);
            Record[] result = lookup.run();
            int code = lookup.getResult();
            if (code != Lookup.SUCCESSFUL) {
                Logger.d(TAG, "Lookup error: " + code + "/" + lookup.getErrorString());
            }
            return result;
        } catch (TextParseException e) {
            Logger.d(TAG, "Not a valid DNS name");
            return null;
        } catch (IllegalArgumentException e) {
            Logger.d(TAG, "Not a valid DNS type");
            return null;
        }
    }
    
    /**
     * Get DNS A record
     * 
     * @param domain Domain
     * @return IP address or null if no record
     */
    private static String getDnsA(String domain) {
        try {
            Logger.d(TAG, "DNS A lookup for " + domain);
            return InetAddress.getByName(domain).getHostAddress();
        } catch (UnknownHostException e) {
            Logger.d(TAG, "Unknown HostException");
            return null;
        }
    }

    /**
     * Get best DNS SRV record
     * 
     * @param records SRV records
     * @return IP address
     */
    private static SRVRecord getBestDnsSRV(Record[] records) {
        SRVRecord result = null;
        for (int i = 0; i < records.length; i++) {
            SRVRecord srv = (SRVRecord) records[i];
            Logger.d(TAG, "SRV record: " + srv.toString());
            if (result == null) {
                // First record
                result = srv;
            } else {
                // Next record
                if (srv.getPriority() < result.getPriority()) {
                    // Lowest priority
                    result = srv;
                } else if (srv.getPriority() == result.getPriority()) {
                    // Highest weight
                    if (srv.getWeight() > result.getWeight()) {
                        result = srv;
                    }
                }
            }
        }
        if (result != null) {
            Logger.d(TAG, "Best SRV record: " + result.toString());
        }
        return result;
    }
    
    /**
     * Route the RCSe only APN
     * @param connManager ContivitiManager instance
     */
    @SuppressWarnings("unchecked")
    public static void routeToHost(final ConnectivityManager connManager) {
        new AsyncTask<Void, Void, Boolean>() {
            @Override
            protected Boolean doInBackground(Void... arg0) {
                String mobileHost = RcsSettings.getInstance().getImsProxyAddrForMobile();
                Logger.d(TAG, "Mobile host: " + mobileHost);
                int mobileInetAddr = PhoneUtils.getInetAddress(mobileHost);
                if (connManager.requestRouteToHost(ConnectivityManager.TYPE_MOBILE_RCSE,
                        mobileInetAddr)) {
                    Logger.d(TAG, "Route to mobile host successed");
                    if (RcsSettings.getInstance().isServiceActivated()) {
                        LauncherUtils.launchRcsCoreService(sContext);
                        Logger.d(TAG, "Route to mobile host successed, "
                                + "and then launchRcsCoreService()");
                    }
                    return true;
                    // Launcher service
                } else {
                    Logger.d(TAG, "Route to mobile host failed");
                    return false;
                }
            }
        }.execute();
    }
    /**
     * @}
     */
    
    /**
     * M: Add to indicate whether the RCS-e is using the dummy account
     * 
     * @return true for using dummy account, otherwise return false;
     */
    public static boolean isUsingDummyAccount() {
        boolean usingDummyAccount = RcsSettings.getInstance().getAutoConfigMode() == RcsSettingsData.NO_AUTO_CONFIG ? true
                : false;
        Logger.d(TAG, "isUsingDummyAccount()-is using dummy account: " + usingDummyAccount);
        return usingDummyAccount;
    }
    /**
     * @}
     */

	   private static String getInternationalPrefix(String countryIso) {
        try {
            PhoneNumberUtil util = PhoneNumberUtil.getInstance();
            Method method = PhoneNumberUtil.class.getDeclaredMethod(METHOD_GET_METADATA,
                    String.class);
            method.setAccessible(true);
            PhoneMetadata metadata = (PhoneMetadata) method.invoke(util, countryIso);
            if (metadata != null) {
                String prefix = metadata.getInternationalPrefix();
                if (countryIso.equalsIgnoreCase(REGION_TW)) {
                    prefix = INTERNATIONAL_PREFIX_TW;
                }
                return prefix;
            }
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        return null;
    }


	
	private static String getDefaultSimCountryIso() {
		   int simId;
		   String iso = null;
		   if (FeatureOption.MTK_GEMINI_SUPPORT == true) {
			   simId = SystemProperties.getInt(PhoneConstants.GEMINI_DEFAULT_SIM_PROP, -1);
			   if (simId == -1) {// No default sim setting
				   simId = PhoneConstants.GEMINI_SIM_1;
			   }

                           if(mTelephonyManagerEx ==null){
                              mTelephonyManagerEx  = TelephonyManagerEx.getDefault();
                           }

			   if (!mTelephonyManagerEx.getDefault().hasIccCard(simId)) {
				   simId = PhoneConstants.GEMINI_SIM_2 ^ simId;
			   }

                             iso = mTelephonyManagerEx.getSimCountryIso(simId);

		   } else {
			   iso = TelephonyManager.getDefault().getSimCountryIso();
		   }
		   return iso;
	   }
}
