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

import android.text.TextUtils;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.regex.Pattern;
import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;

public class Address {
    /**
     *  Address part, in the form local_part@domain_part. No surrounding angle brackets.
     */
    private String mAddress;

    /**
     * Name part. No surrounding double quote, and no MIME/base64 encoding.
     * This must be null if Address has no name part.
     */
    private String mPersonal;

    // Regex that matches address surrounded by '<>' optionally. '^<?([^>]+)>?$'
    private static final Pattern REMOVE_OPTIONAL_BRACKET = Pattern.compile("^<?([^>]+)>?$");
    // Regex that matches personal name surrounded by '""' optionally. '^"?([^"]+)"?$'
    private static final Pattern REMOVE_OPTIONAL_DQUOTE = Pattern.compile("^\"?([^\"]*)\"?$");
    // Regex that matches escaped character '\\([\\"])'
    private static final Pattern UNQUOTE = Pattern.compile("\\\\([\\\\\"])");

    private static final Address[] EMPTY_ADDRESS_ARRAY = new Address[0];

    // delimiters are chars that do not appear in an email address, used by pack/unpack
    private static final char LIST_DELIMITER_EMAIL = '\1';
    private static final char LIST_DELIMITER_PERSONAL = '\2';

    public Address(String address, String personal) {
        setAddress(address);
        setPersonal(personal);
    }

    public Address(String address) {
        setAddress(address);
    }

    public String getAddress() {
        return mAddress;
    }

    public void setAddress(String address) {
        this.mAddress = REMOVE_OPTIONAL_BRACKET.matcher(address).replaceAll("$1");
    }

	public static String getFormatAddress(String address) {
		ArrayList<Address> addressList = new ArrayList<Address>();
       	Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(address);
		for (int index = 0; (tokens != null && index < tokens.length); index ++) {
			Rfc822Token token = tokens[index];
			String subaddress = token.getAddress();
			if (!TextUtils.isEmpty(subaddress)) {
				addressList.add(new Address(subaddress));
			}
		}
    	return toString(addressList.toArray(new Address[0]));
    }

	public static boolean isValidAddress(String address){
		int len = address.length();
        int firstAt = address.indexOf('@');
        int lastAt = address.lastIndexOf('@');
        int firstDot = address.indexOf('.', lastAt + 1);
        int lastDot = address.lastIndexOf('.');
        return firstAt > 0 && firstAt == lastAt  //&& lastAt + 1 < firstDot
            && firstDot <= lastDot && lastDot < len - 1;
    }

 
    

    @Override
    public boolean equals(Object o) {
        if (o instanceof Address) {
            // It seems that the spec says that the "user" part is case-sensitive,
            // while the domain part in case-insesitive.
            // So foo@yahoo.com and Foo@yahoo.com are different.
            // This may seem non-intuitive from the user POV, so we
            // may re-consider it if it creates UI trouble.
            // A problem case is "replyAll" sending to both
            // a@b.c and to A@b.c, which turn out to be the same on the server.
            // Leave unchanged for now (i.e. case-sensitive).
            return getAddress().equals(((Address) o).getAddress());
        }
        return super.equals(o);
    }

    /**
     * Get human readable address string.
     * Do not use this for email header.
     * 
     * @return Human readable address string.  Not quoted and not encoded.
     */
 
    /**
     * Get human readable comma-delimited address string.
     * 
     * @param addresses Address array
     * @return Human readable comma-delimited address string.
     */
    public static String toString(Address[] addresses) {
        if (addresses == null || addresses.length == 0) {
            return null;
        }
        if (addresses.length == 1) {
            return addresses[0].toString();
        }
        StringBuffer sb = new StringBuffer(addresses[0].toString());
        for (int i = 1; i < addresses.length; i++) {
            sb.append(';');
            sb.append(addresses[i].toString());
        }
        return sb.toString();
    }
    
	public String toString() {
		return mAddress;
			
	}


   

    /**
     * Returns exactly the same result as Address.toString(Address.unpack(packedList)).
     */
    public static String unpackToString(String packedList) {
        return toString(unpack(packedList));
    }

  
    /**
     * Returns null if the packedList has 0 addresses, otherwise returns the first address.
     * The same as Address.unpack(packedList)[0] for non-empty list.
     * This is an utility method that offers some performance optimization opportunities.
     */
    public static Address unpackFirst(String packedList) {
        Address[] array = unpack(packedList);
        return array.length > 0 ? array[0] : null;
    }

    /**
     * Convert a packed list of addresses to a form suitable for use in an RFC822 header.
     * This implementation is brute-force, and could be replaced with a more efficient version
     * if desired.
     */
    

    /**
     * Unpacks an address list previously packed with pack()
     * @param addressList String with packed addresses as returned by pack()
     * @return array of addresses resulting from unpack
     */
    public static Address[] unpack(String addressList) {
        if (addressList == null || addressList.length() == 0) {
            return EMPTY_ADDRESS_ARRAY;
        }
        ArrayList<Address> addresses = new ArrayList<Address>();
        int length = addressList.length();
        int pairStartIndex = 0;
        int pairEndIndex = 0;

        /* addressEndIndex is only re-scanned (indexOf()) when a LIST_DELIMITER_PERSONAL
           is used, not for every email address; i.e. not for every iteration of the while().
           This reduces the theoretical complexity from quadratic to linear,
           and provides some speed-up in practice by removing redundant scans of the string.
        */
        int addressEndIndex = addressList.indexOf(LIST_DELIMITER_PERSONAL);

        while (pairStartIndex < length) {
            pairEndIndex = addressList.indexOf(LIST_DELIMITER_EMAIL, pairStartIndex);
            if (pairEndIndex == -1) {
                pairEndIndex = length;
            }
            Address address;
            if (addressEndIndex == -1 || pairEndIndex <= addressEndIndex) {
                // in this case the DELIMITER_PERSONAL is in a future pair,
                // so don't use personal, and don't update addressEndIndex
                address = new Address(addressList.substring(pairStartIndex, pairEndIndex), null);
            } else {
                address = new Address(addressList.substring(pairStartIndex, addressEndIndex),
                                      addressList.substring(addressEndIndex + 1, pairEndIndex));
                // only update addressEndIndex when we use the LIST_DELIMITER_PERSONAL
                addressEndIndex = addressList.indexOf(LIST_DELIMITER_PERSONAL, pairEndIndex + 1);
            }
            addresses.add(address);
            pairStartIndex = pairEndIndex + 1;
        }
        return addresses.toArray(EMPTY_ADDRESS_ARRAY);
    }

    /**
     * Packs an address list into a String that is very quick to read
     * and parse. Packed lists can be unpacked with unpack().
     * The format is a series of packed addresses separated by LIST_DELIMITER_EMAIL.
     * Each address is packed as
     * a pair of address and personal separated by LIST_DELIMITER_PERSONAL,
     * where the personal and delimiter are optional.
     * E.g. "foo@x.com\1joe@x.com\2Joe Doe"
     * @param addresses Array of addresses
     * @return a string containing the packed addresses.
     */
    public static String pack(Address[] addresses) {
        // TODO: return same value for both null & empty list
        if (addresses == null) {
            return null;
        }
        final int nAddr = addresses.length;
        if (nAddr == 0) {
            return "";
        }

        // shortcut: one email with no displayName
        if (nAddr == 1 && addresses[0].getPersonal() == null) {
            return addresses[0].getAddress();
        }

        StringBuffer sb = new StringBuffer();
        for (int i = 0; i < nAddr; i++) {
            if (i != 0) {
                sb.append(LIST_DELIMITER_EMAIL);
            }
            final Address address = addresses[i];
            sb.append(address.getAddress());
            final String displayName = address.getPersonal();
            if (displayName != null) {
                sb.append(LIST_DELIMITER_PERSONAL);
                sb.append(displayName);
            }
        }
        return sb.toString();
    }

    /**
     * Produces the same result as pack(array), but only packs one (this) address.
     */
    public String pack() {
        final String address = getAddress();
        final String personal = getPersonal();
        if (personal == null) {
            return address;
        } else {
            return address + LIST_DELIMITER_PERSONAL + personal;
        }
    }
	  public String getPersonal() {
        return mPersonal;
    }
	   public void setPersonal(String personal) {
        if (personal != null) {
            personal = REMOVE_OPTIONAL_DQUOTE.matcher(personal).replaceAll("$1");
            personal = UNQUOTE.matcher(personal).replaceAll("$1");
        //    personal = DecoderUtil.decodeEncodedWords(personal);
            if (personal.length() == 0) {
                personal = null;
            }
        }
        this.mPersonal = personal;
    }
	public static String packedToHeader(String packedList) {
		return toHeader(unpack(packedList));
	}

	public static String toHeader(Address[] addresses) {
        if (addresses == null || addresses.length == 0) {
            return null;
        }
        if (addresses.length == 1) {
            return addresses[0].toHeader();
        }
        StringBuffer sb = new StringBuffer(addresses[0].toHeader());
        for (int i = 1; i < addresses.length; i++) {
            // We need space character to be able to fold line.
            sb.append(", ");
            sb.append(addresses[i].toHeader());
        }
        return sb.toString();
    }
	public String toHeader() {
        return mAddress;
    }

}
