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

/*
 * Copyright (C) 2009 The Android Open Source Project
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
 */

package com.android.bluetooth.pbap;

import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.provider.ContactsContract.Data;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Utilities for VCard handling codes.
 */
public class VCardUtils {
    /*
     * TODO: some of methods in this class should be placed to the more
     * appropriate place...
     */

    // Note that not all types are included in this map/set, since, for example,
    // TYPE_HOME_FAX is
    // converted to two attribute Strings. These only contain some minor fields
    // valid in both
    // vCard and current (as of 2009-08-07) Contacts structure.
    private static final Map<Integer, String> KNOWN_PHONE_TYPE_MAP_ITOS;

    private static final Set<String> PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS;

    private static final Map<String, Integer> KNOWN_PHONE_TYPES_MAP_STOI;

    static {
        KNOWN_PHONE_TYPE_MAP_ITOS = new HashMap<Integer, String>();
        KNOWN_PHONE_TYPES_MAP_STOI = new HashMap<String, Integer>();

        KNOWN_PHONE_TYPE_MAP_ITOS.put(Phone.TYPE_CAR, Constants.ATTR_TYPE_CAR);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_CAR, Phone.TYPE_CAR);
        KNOWN_PHONE_TYPE_MAP_ITOS.put(Phone.TYPE_PAGER, Constants.ATTR_TYPE_PAGER);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PAGER, Phone.TYPE_PAGER);
        KNOWN_PHONE_TYPE_MAP_ITOS.put(Phone.TYPE_ISDN, Constants.ATTR_TYPE_ISDN);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_ISDN, Phone.TYPE_ISDN);

        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_HOME, Phone.TYPE_HOME);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_WORK, Phone.TYPE_WORK);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_CELL, Phone.TYPE_MOBILE);

        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_OTHER, Phone.TYPE_OTHER);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_CALLBACK, Phone.TYPE_CALLBACK);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_COMPANY_MAIN,
                Phone.TYPE_COMPANY_MAIN);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_RADIO, Phone.TYPE_RADIO);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_TELEX, Phone.TYPE_TELEX);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_TTY_TDD, Phone.TYPE_TTY_TDD);
        KNOWN_PHONE_TYPES_MAP_STOI.put(Constants.ATTR_TYPE_PHONE_EXTRA_ASSISTANT,
                Phone.TYPE_ASSISTANT);

        PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS = new HashSet<String>();
        PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS.add(Constants.ATTR_TYPE_MODEM);
        PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS.add(Constants.ATTR_TYPE_MSG);
        PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS.add(Constants.ATTR_TYPE_BBS);
        PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS.add(Constants.ATTR_TYPE_VIDEO);
    }

    public static String getPhoneAttributeString(Integer type) {
        return KNOWN_PHONE_TYPE_MAP_ITOS.get(type);
    }

    /**
     * Returns Interger when the given types can be parsed as known type.
     * Returns String object when not, which should be set to label.
     */
    public static Object getPhoneTypeFromStrings(Collection<String> types) {
        int type = -1;
        String label = null;
        boolean isFax = false;
        boolean hasPref = false;

        if (types != null) {
            for (String typeString : types) {
                typeString = typeString.toUpperCase();
                if (typeString.equals(Constants.ATTR_TYPE_PREF)) {
                    hasPref = true;
                } else if (typeString.equals(Constants.ATTR_TYPE_FAX)) {
                    isFax = true;
                } else {
                    if (typeString.startsWith("X-") && type < 0) {
                        typeString = typeString.substring(2);
                    }
                    Integer tmp = KNOWN_PHONE_TYPES_MAP_STOI.get(typeString);
                    if (tmp != null) {
                        type = tmp;
                    } else if (type < 0) {
                        type = Phone.TYPE_CUSTOM;
                        label = typeString;
                    }
                }
            }
        }
        if (type < 0) {
            if (hasPref) {
                type = Phone.TYPE_MAIN;
            } else {
                // default to TYPE_HOME
                type = Phone.TYPE_HOME;
            }
        }
        if (isFax) {
            if (type == Phone.TYPE_HOME) {
                type = Phone.TYPE_FAX_HOME;
            } else if (type == Phone.TYPE_WORK) {
                type = Phone.TYPE_FAX_WORK;
            } else if (type == Phone.TYPE_OTHER) {
                type = Phone.TYPE_OTHER_FAX;
            }
        }
        if (type == Phone.TYPE_CUSTOM) {
            return label;
        } else {
            return type;
        }
    }

    public static boolean isValidPhoneAttribute(String phoneAttribute, int vcardType) {
        // TODO: check the following.
        // - it may violate vCard spec
        // - it may contain non-ASCII characters
        //
        // TODO: use vcardType
        return (phoneAttribute.startsWith("X-") || phoneAttribute.startsWith("x-") || PHONE_TYPE_SET_UNKNOWN_TO_CONTACTS
                .contains(phoneAttribute));
    }

    public static String[] sortNameElements(int vcardType, String familyName, String middleName,
            String givenName) {
        String[] list = new String[3];
        switch (VCardConfig.getNameOrderType(vcardType)) {
            case VCardConfig.NAME_ORDER_JAPANESE:
                // TODO: Should handle Ascii case?
                list[0] = familyName;
                list[1] = middleName;
                list[2] = givenName;
                break;
            case VCardConfig.NAME_ORDER_EUROPE:
                list[0] = middleName;
                list[1] = givenName;
                list[2] = familyName;
                break;
            default:
                list[0] = givenName;
                list[1] = middleName;
                list[2] = familyName;
                break;
        }
        return list;
    }

    public static int getPhoneNumberFormat(final int vcardType) {
        if (VCardConfig.isJapaneseDevice(vcardType)) {
            return PhoneNumberUtils.FORMAT_JAPAN;
        } else {
            return PhoneNumberUtils.FORMAT_NANP;
        }
    }

    /**
     * Inserts postal data into the builder object. Note that the data structure
     * of ContactsContract is different from that defined in vCard. So some
     * conversion may be performed in this method. See also {
     * {@link #getVCardPostalElements(ContentValues)}
     */
    public static void insertStructuredPostalDataUsingContactsStruct(int vcardType,
            final ContentProviderOperation.Builder builder,
            final ContactStruct.PostalData postalData) {
        builder.withValueBackReference(StructuredPostal.RAW_CONTACT_ID, 0);
        builder.withValue(Data.MIMETYPE, StructuredPostal.CONTENT_ITEM_TYPE);

        builder.withValue(StructuredPostal.TYPE, postalData.type);
        if (postalData.type == StructuredPostal.TYPE_CUSTOM) {
            builder.withValue(StructuredPostal.LABEL, postalData.label);
        }

        builder.withValue(StructuredPostal.POBOX, postalData.pobox);
        // Extended address is dropped since there's no relevant entry in
        // ContactsContract.
        builder.withValue(StructuredPostal.STREET, postalData.street);
        builder.withValue(StructuredPostal.CITY, postalData.localty);
        builder.withValue(StructuredPostal.REGION, postalData.region);
        builder.withValue(StructuredPostal.POSTCODE, postalData.postalCode);
        builder.withValue(StructuredPostal.COUNTRY, postalData.country);

        builder.withValue(StructuredPostal.FORMATTED_ADDRESS, postalData
                .getFormattedAddress(vcardType));
        if (postalData.isPrimary) {
            builder.withValue(Data.IS_PRIMARY, 1);
        }
    }

    /**
     * Returns String[] containing address information based on vCard spec (PO
     * Box, Extended Address, Street, Locality, Region, Postal Code, Country
     * Name). All String objects are non-null ("" is used when the relevant data
     * is empty). Note that the data structure of ContactsContract is different
     * from that defined in vCard. So some conversion may be performed in this
     * method. See also {
     * {@link #insertStructuredPostalDataUsingContactsStruct(int, 
     * android.content.ContentProviderOperation.Builder, android.pim.vcard.ContactStruct.PostalData)}
     */
    public static String[] getVCardPostalElements(ContentValues contentValues) {
        String[] dataArray = new String[7];
        dataArray[0] = contentValues.getAsString(StructuredPostal.POBOX);
        if (dataArray[0] == null) {
            dataArray[0] = "";
        }
        // Extended addr. There's no relevant data in ContactsContract.
        dataArray[1] = "";
        dataArray[2] = contentValues.getAsString(StructuredPostal.STREET);
        if (dataArray[2] == null) {
            dataArray[2] = "";
        }
        // Assume that localty == city
        dataArray[3] = contentValues.getAsString(StructuredPostal.CITY);
        if (dataArray[3] == null) {
            dataArray[3] = "";
        }
        String region = contentValues.getAsString(StructuredPostal.REGION);
        if (!TextUtils.isEmpty(region)) {
            dataArray[4] = region;
        } else {
            dataArray[4] = "";
        }
        dataArray[5] = contentValues.getAsString(StructuredPostal.POSTCODE);
        if (dataArray[5] == null) {
            dataArray[5] = "";
        }
        dataArray[6] = contentValues.getAsString(StructuredPostal.COUNTRY);
        if (dataArray[6] == null) {
            dataArray[6] = "";
        }

        return dataArray;
    }

    public static String constructNameFromElements(int nameOrderType, String familyName,
            String middleName, String givenName) {
        return constructNameFromElements(nameOrderType, familyName, middleName, givenName, null,
                null);
    }

    public static String constructNameFromElements(int nameOrderType, String familyName,
            String middleName, String givenName, String prefix, String suffix) {
        StringBuilder builder = new StringBuilder();
        String[] nameList = sortNameElements(nameOrderType, familyName, middleName, givenName);
        boolean first = true;
        if (!TextUtils.isEmpty(prefix)) {
            first = false;
            builder.append(prefix);
        }
        for (String namePart : nameList) {
            if (!TextUtils.isEmpty(namePart)) {
                if (first) {
                    first = false;
                } else {
                    builder.append(' ');
                }
                builder.append(namePart);
            }
        }
        if (!TextUtils.isEmpty(suffix)) {
            if (!first) {
                builder.append(' ');
            }
            builder.append(suffix);
        }
        return builder.toString();
    }

    public static boolean containsOnlyPrintableAscii(String str) {
        if (str == null || TextUtils.isEmpty(str)) {
            return true;
        }

        final int length = str.length();
        final int asciiFirst = 0x20;
        final int asciiLast = 0x126;
        for (int i = 0; i < length; i = str.offsetByCodePoints(i, 1)) {
            int c = str.codePointAt(i);
            if (c < asciiFirst || asciiLast < c) {
                return false;
            }
        }
        return true;
    }

    /**
     * This is useful when checking the string should be encoded into
     * quoted-printable or not, which is required by vCard 2.1. See the
     * definition of "7bit" in vCard 2.1 spec for more information.
     */
    public static boolean containsOnlyNonCrLfPrintableAscii(String str) {
        if (str == null || TextUtils.isEmpty(str)) {
            return true;
        }

        final int length = str.length();
        final int asciiFirst = 0x20;
        final int asciiLast = 0x126;
        for (int i = 0; i < length; i = str.offsetByCodePoints(i, 1)) {
            int c = str.codePointAt(i);
            if (c < asciiFirst || asciiLast < c || c == '\n' || c == '\r') {
                return false;
            }
        }
        return true;
    }

    /**
     * This is useful since vCard 3.0 often requires the ("X-") properties and
     * groups should contain only alphabets, digits, and hyphen. Note: It is
     * already known some devices (wrongly) outputs properties with characters
     * which should not be in the field. One example is "X-GOOGLE TALK". We
     * accept such kind of input but must never output it unless the target is
     * very specific to the device which is able to parse the malformed input.
     */
    public static boolean containsOnlyAlphaDigitHyphen(String str) {
        if (TextUtils.isEmpty(str)) {
            return true;
        }

        final int lowerAlphabetFirst = 0x41; // included ('A')
        final int lowerAlphabetLast = 0x5b; // not included ('[')
        final int upperAlphabetFirst = 0x61; // included ('a')
        final int upperAlphabetLast = 0x7b; // included ('{')
        final int digitFirst = 0x30; // included ('0')
        final int digitLast = 0x39; // included ('9')
        final int hyphen = '-';
        final int length = str.length();
        for (int i = 0; i < length; i = str.offsetByCodePoints(i, 1)) {
            int codepoint = str.codePointAt(i);
            if (!((lowerAlphabetFirst <= codepoint && codepoint < lowerAlphabetLast)
                    || (upperAlphabetFirst <= codepoint && codepoint < upperAlphabetLast)
                    || (digitFirst <= codepoint && codepoint < digitLast) || (codepoint == hyphen))) {
                return false;
            }
        }
        return true;
    }

    // TODO: Replace wth the method in Base64 class.
    private static char sPad = '=';

    private static final char[] ENCODE64 = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
            'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
            'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
            'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    public static String encodeBase64(byte[] data) {
        if (data == null) {
            return "";
        }

        char[] charBuffer = new char[(data.length + 2) / 3 * 4];
        int position = 0;
        int threeBytes = 0;
        for (int i = 0; i < data.length - 2; i += 3) {
            threeBytes = ((data[i] & 0xFF) << 16) + ((data[i + 1] & 0xFF) << 8) + (data[i + 2] & 0xFF);
            charBuffer[position++] = ENCODE64[threeBytes >> 18];
            charBuffer[position++] = ENCODE64[(threeBytes >> 12) & 0x3F];
            charBuffer[position++] = ENCODE64[(threeBytes >> 6) & 0x3F];
            charBuffer[position++] = ENCODE64[threeBytes & 0x3F];
        }
        switch (data.length % 3) {
            case 1: // [111111][11 0000][0000 00][000000]
                threeBytes = ((data[data.length - 1] & 0xFF) << 16);
                charBuffer[position++] = ENCODE64[threeBytes >> 18];
                charBuffer[position++] = ENCODE64[(threeBytes >> 12) & 0x3F];
                charBuffer[position++] = sPad;
                charBuffer[position++] = sPad;
                break;
            case 2: // [111111][11 1111][1111 00][000000]
                threeBytes = ((data[data.length - 2] & 0xFF) << 16)
                        + ((data[data.length - 1] & 0xFF) << 8);
                charBuffer[position++] = ENCODE64[threeBytes >> 18];
                charBuffer[position++] = ENCODE64[(threeBytes >> 12) & 0x3F];
                charBuffer[position++] = ENCODE64[(threeBytes >> 6) & 0x3F];
                charBuffer[position++] = sPad;
                break;
            default:
                break;
        }

        return new String(charBuffer);
    }

    public static String toHalfWidthString(String orgString) {
        if (orgString == null || TextUtils.isEmpty(orgString)) {
            return null;
        }
        StringBuilder builder = new StringBuilder();
        int length = orgString.length();
        for (int i = 0; i < length; i++) {
            // All Japanese character is able to be expressed by char.
            // Do not need to use String#codepPointAt().
            char ch = orgString.charAt(i);
            CharSequence halfWidthText = JapaneseUtils.tryGetHalfWidthText(ch);
            if (halfWidthText != null) {
                builder.append(halfWidthText);
            } else {
                builder.append(ch);
            }
        }
        return builder.toString();
    }

    private VCardUtils() {
    }

    /**
     * Test whether the character is allowable for phone number. The allowable
     * character set for phone number is those which can be input when New/Edit
     * the contacts.
     *
     * @param ch
     * @return true if ch is an allowable phone number character
     */
    public static boolean isAllowablePhoneNumberChar(char ch) {
        boolean ret = false;
        // TODO: This set can be extended if necessary
        ret = Character.isDigit(ch) || ch == '(' || ch == '/' || ch == ')' || ch == '-'
                || ch == 'N' || ch == ',' || ch == '.' || ch == '*' || ch == '#' || ch == '+'
                || ch == ' ';
        return ret;
    }
}

/**
 * TextUtils especially for Japanese. TODO: make this in android.text in the
 * future
 */
class JapaneseUtils {
    private static final Map<Character, String> HALF_WIDTH_MAP = new HashMap<Character, String>();

    static {
        // There's no logical mapping rule in Unicode. Sigh.
        HALF_WIDTH_MAP.put('\u3001', "\uFF64");
        HALF_WIDTH_MAP.put('\u3002', "\uFF61");
        HALF_WIDTH_MAP.put('\u300C', "\uFF62");
        HALF_WIDTH_MAP.put('\u300D', "\uFF63");
        HALF_WIDTH_MAP.put('\u301C', "~");
        HALF_WIDTH_MAP.put('\u3041', "\uFF67");
        HALF_WIDTH_MAP.put('\u3042', "\uFF71");
        HALF_WIDTH_MAP.put('\u3043', "\uFF68");
        HALF_WIDTH_MAP.put('\u3044', "\uFF72");
        HALF_WIDTH_MAP.put('\u3045', "\uFF69");
        HALF_WIDTH_MAP.put('\u3046', "\uFF73");
        HALF_WIDTH_MAP.put('\u3047', "\uFF6A");
        HALF_WIDTH_MAP.put('\u3048', "\uFF74");
        HALF_WIDTH_MAP.put('\u3049', "\uFF6B");
        HALF_WIDTH_MAP.put('\u304A', "\uFF75");
        HALF_WIDTH_MAP.put('\u304B', "\uFF76");
        HALF_WIDTH_MAP.put('\u304C', "\uFF76\uFF9E");
        HALF_WIDTH_MAP.put('\u304D', "\uFF77");
        HALF_WIDTH_MAP.put('\u304E', "\uFF77\uFF9E");
        HALF_WIDTH_MAP.put('\u304F', "\uFF78");
        HALF_WIDTH_MAP.put('\u3050', "\uFF78\uFF9E");
        HALF_WIDTH_MAP.put('\u3051', "\uFF79");
        HALF_WIDTH_MAP.put('\u3052', "\uFF79\uFF9E");
        HALF_WIDTH_MAP.put('\u3053', "\uFF7A");
        HALF_WIDTH_MAP.put('\u3054', "\uFF7A\uFF9E");
        HALF_WIDTH_MAP.put('\u3055', "\uFF7B");
        HALF_WIDTH_MAP.put('\u3056', "\uFF7B\uFF9E");
        HALF_WIDTH_MAP.put('\u3057', "\uFF7C");
        HALF_WIDTH_MAP.put('\u3058', "\uFF7C\uFF9E");
        HALF_WIDTH_MAP.put('\u3059', "\uFF7D");
        HALF_WIDTH_MAP.put('\u305A', "\uFF7D\uFF9E");
        HALF_WIDTH_MAP.put('\u305B', "\uFF7E");
        HALF_WIDTH_MAP.put('\u305C', "\uFF7E\uFF9E");
        HALF_WIDTH_MAP.put('\u305D', "\uFF7F");
        HALF_WIDTH_MAP.put('\u305E', "\uFF7F\uFF9E");
        HALF_WIDTH_MAP.put('\u305F', "\uFF80");
        HALF_WIDTH_MAP.put('\u3060', "\uFF80\uFF9E");
        HALF_WIDTH_MAP.put('\u3061', "\uFF81");
        HALF_WIDTH_MAP.put('\u3062', "\uFF81\uFF9E");
        HALF_WIDTH_MAP.put('\u3063', "\uFF6F");
        HALF_WIDTH_MAP.put('\u3064', "\uFF82");
        HALF_WIDTH_MAP.put('\u3065', "\uFF82\uFF9E");
        HALF_WIDTH_MAP.put('\u3066', "\uFF83");
        HALF_WIDTH_MAP.put('\u3067', "\uFF83\uFF9E");
        HALF_WIDTH_MAP.put('\u3068', "\uFF84");
        HALF_WIDTH_MAP.put('\u3069', "\uFF84\uFF9E");
        HALF_WIDTH_MAP.put('\u306A', "\uFF85");
        HALF_WIDTH_MAP.put('\u306B', "\uFF86");
        HALF_WIDTH_MAP.put('\u306C', "\uFF87");
        HALF_WIDTH_MAP.put('\u306D', "\uFF88");
        HALF_WIDTH_MAP.put('\u306E', "\uFF89");
        HALF_WIDTH_MAP.put('\u306F', "\uFF8A");
        HALF_WIDTH_MAP.put('\u3070', "\uFF8A\uFF9E");
        HALF_WIDTH_MAP.put('\u3071', "\uFF8A\uFF9F");
        HALF_WIDTH_MAP.put('\u3072', "\uFF8B");
        HALF_WIDTH_MAP.put('\u3073', "\uFF8B\uFF9E");
        HALF_WIDTH_MAP.put('\u3074', "\uFF8B\uFF9F");
        HALF_WIDTH_MAP.put('\u3075', "\uFF8C");
        HALF_WIDTH_MAP.put('\u3076', "\uFF8C\uFF9E");
        HALF_WIDTH_MAP.put('\u3077', "\uFF8C\uFF9F");
        HALF_WIDTH_MAP.put('\u3078', "\uFF8D");
        HALF_WIDTH_MAP.put('\u3079', "\uFF8D\uFF9E");
        HALF_WIDTH_MAP.put('\u307A', "\uFF8D\uFF9F");
        HALF_WIDTH_MAP.put('\u307B', "\uFF8E");
        HALF_WIDTH_MAP.put('\u307C', "\uFF8E\uFF9E");
        HALF_WIDTH_MAP.put('\u307D', "\uFF8E\uFF9F");
        HALF_WIDTH_MAP.put('\u307E', "\uFF8F");
        HALF_WIDTH_MAP.put('\u307F', "\uFF90");
        HALF_WIDTH_MAP.put('\u3080', "\uFF91");
        HALF_WIDTH_MAP.put('\u3081', "\uFF92");
        HALF_WIDTH_MAP.put('\u3082', "\uFF93");
        HALF_WIDTH_MAP.put('\u3083', "\uFF6C");
        HALF_WIDTH_MAP.put('\u3084', "\uFF94");
        HALF_WIDTH_MAP.put('\u3085', "\uFF6D");
        HALF_WIDTH_MAP.put('\u3086', "\uFF95");
        HALF_WIDTH_MAP.put('\u3087', "\uFF6E");
        HALF_WIDTH_MAP.put('\u3088', "\uFF96");
        HALF_WIDTH_MAP.put('\u3089', "\uFF97");
        HALF_WIDTH_MAP.put('\u308A', "\uFF98");
        HALF_WIDTH_MAP.put('\u308B', "\uFF99");
        HALF_WIDTH_MAP.put('\u308C', "\uFF9A");
        HALF_WIDTH_MAP.put('\u308D', "\uFF9B");
        HALF_WIDTH_MAP.put('\u308E', "\uFF9C");
        HALF_WIDTH_MAP.put('\u308F', "\uFF9C");
        HALF_WIDTH_MAP.put('\u3090', "\uFF72");
        HALF_WIDTH_MAP.put('\u3091', "\uFF74");
        HALF_WIDTH_MAP.put('\u3092', "\uFF66");
        HALF_WIDTH_MAP.put('\u3093', "\uFF9D");
        HALF_WIDTH_MAP.put('\u309B', "\uFF9E");
        HALF_WIDTH_MAP.put('\u309C', "\uFF9F");
        HALF_WIDTH_MAP.put('\u30A1', "\uFF67");
        HALF_WIDTH_MAP.put('\u30A2', "\uFF71");
        HALF_WIDTH_MAP.put('\u30A3', "\uFF68");
        HALF_WIDTH_MAP.put('\u30A4', "\uFF72");
        HALF_WIDTH_MAP.put('\u30A5', "\uFF69");
        HALF_WIDTH_MAP.put('\u30A6', "\uFF73");
        HALF_WIDTH_MAP.put('\u30A7', "\uFF6A");
        HALF_WIDTH_MAP.put('\u30A8', "\uFF74");
        HALF_WIDTH_MAP.put('\u30A9', "\uFF6B");
        HALF_WIDTH_MAP.put('\u30AA', "\uFF75");
        HALF_WIDTH_MAP.put('\u30AB', "\uFF76");
        HALF_WIDTH_MAP.put('\u30AC', "\uFF76\uFF9E");
        HALF_WIDTH_MAP.put('\u30AD', "\uFF77");
        HALF_WIDTH_MAP.put('\u30AE', "\uFF77\uFF9E");
        HALF_WIDTH_MAP.put('\u30AF', "\uFF78");
        HALF_WIDTH_MAP.put('\u30B0', "\uFF78\uFF9E");
        HALF_WIDTH_MAP.put('\u30B1', "\uFF79");
        HALF_WIDTH_MAP.put('\u30B2', "\uFF79\uFF9E");
        HALF_WIDTH_MAP.put('\u30B3', "\uFF7A");
        HALF_WIDTH_MAP.put('\u30B4', "\uFF7A\uFF9E");
        HALF_WIDTH_MAP.put('\u30B5', "\uFF7B");
        HALF_WIDTH_MAP.put('\u30B6', "\uFF7B\uFF9E");
        HALF_WIDTH_MAP.put('\u30B7', "\uFF7C");
        HALF_WIDTH_MAP.put('\u30B8', "\uFF7C\uFF9E");
        HALF_WIDTH_MAP.put('\u30B9', "\uFF7D");
        HALF_WIDTH_MAP.put('\u30BA', "\uFF7D\uFF9E");
        HALF_WIDTH_MAP.put('\u30BB', "\uFF7E");
        HALF_WIDTH_MAP.put('\u30BC', "\uFF7E\uFF9E");
        HALF_WIDTH_MAP.put('\u30BD', "\uFF7F");
        HALF_WIDTH_MAP.put('\u30BE', "\uFF7F\uFF9E");
        HALF_WIDTH_MAP.put('\u30BF', "\uFF80");
        HALF_WIDTH_MAP.put('\u30C0', "\uFF80\uFF9E");
        HALF_WIDTH_MAP.put('\u30C1', "\uFF81");
        HALF_WIDTH_MAP.put('\u30C2', "\uFF81\uFF9E");
        HALF_WIDTH_MAP.put('\u30C3', "\uFF6F");
        HALF_WIDTH_MAP.put('\u30C4', "\uFF82");
        HALF_WIDTH_MAP.put('\u30C5', "\uFF82\uFF9E");
        HALF_WIDTH_MAP.put('\u30C6', "\uFF83");
        HALF_WIDTH_MAP.put('\u30C7', "\uFF83\uFF9E");
        HALF_WIDTH_MAP.put('\u30C8', "\uFF84");
        HALF_WIDTH_MAP.put('\u30C9', "\uFF84\uFF9E");
        HALF_WIDTH_MAP.put('\u30CA', "\uFF85");
        HALF_WIDTH_MAP.put('\u30CB', "\uFF86");
        HALF_WIDTH_MAP.put('\u30CC', "\uFF87");
        HALF_WIDTH_MAP.put('\u30CD', "\uFF88");
        HALF_WIDTH_MAP.put('\u30CE', "\uFF89");
        HALF_WIDTH_MAP.put('\u30CF', "\uFF8A");
        HALF_WIDTH_MAP.put('\u30D0', "\uFF8A\uFF9E");
        HALF_WIDTH_MAP.put('\u30D1', "\uFF8A\uFF9F");
        HALF_WIDTH_MAP.put('\u30D2', "\uFF8B");
        HALF_WIDTH_MAP.put('\u30D3', "\uFF8B\uFF9E");
        HALF_WIDTH_MAP.put('\u30D4', "\uFF8B\uFF9F");
        HALF_WIDTH_MAP.put('\u30D5', "\uFF8C");
        HALF_WIDTH_MAP.put('\u30D6', "\uFF8C\uFF9E");
        HALF_WIDTH_MAP.put('\u30D7', "\uFF8C\uFF9F");
        HALF_WIDTH_MAP.put('\u30D8', "\uFF8D");
        HALF_WIDTH_MAP.put('\u30D9', "\uFF8D\uFF9E");
        HALF_WIDTH_MAP.put('\u30DA', "\uFF8D\uFF9F");
        HALF_WIDTH_MAP.put('\u30DB', "\uFF8E");
        HALF_WIDTH_MAP.put('\u30DC', "\uFF8E\uFF9E");
        HALF_WIDTH_MAP.put('\u30DD', "\uFF8E\uFF9F");
        HALF_WIDTH_MAP.put('\u30DE', "\uFF8F");
        HALF_WIDTH_MAP.put('\u30DF', "\uFF90");
        HALF_WIDTH_MAP.put('\u30E0', "\uFF91");
        HALF_WIDTH_MAP.put('\u30E1', "\uFF92");
        HALF_WIDTH_MAP.put('\u30E2', "\uFF93");
        HALF_WIDTH_MAP.put('\u30E3', "\uFF6C");
        HALF_WIDTH_MAP.put('\u30E4', "\uFF94");
        HALF_WIDTH_MAP.put('\u30E5', "\uFF6D");
        HALF_WIDTH_MAP.put('\u30E6', "\uFF95");
        HALF_WIDTH_MAP.put('\u30E7', "\uFF6E");
        HALF_WIDTH_MAP.put('\u30E8', "\uFF96");
        HALF_WIDTH_MAP.put('\u30E9', "\uFF97");
        HALF_WIDTH_MAP.put('\u30EA', "\uFF98");
        HALF_WIDTH_MAP.put('\u30EB', "\uFF99");
        HALF_WIDTH_MAP.put('\u30EC', "\uFF9A");
        HALF_WIDTH_MAP.put('\u30ED', "\uFF9B");
        HALF_WIDTH_MAP.put('\u30EE', "\uFF9C");
        HALF_WIDTH_MAP.put('\u30EF', "\uFF9C");
        HALF_WIDTH_MAP.put('\u30F0', "\uFF72");
        HALF_WIDTH_MAP.put('\u30F1', "\uFF74");
        HALF_WIDTH_MAP.put('\u30F2', "\uFF66");
        HALF_WIDTH_MAP.put('\u30F3', "\uFF9D");
        HALF_WIDTH_MAP.put('\u30F4', "\uFF73\uFF9E");
        HALF_WIDTH_MAP.put('\u30F5', "\uFF76");
        HALF_WIDTH_MAP.put('\u30F6', "\uFF79");
        HALF_WIDTH_MAP.put('\u30FB', "\uFF65");
        HALF_WIDTH_MAP.put('\u30FC', "\uFF70");
        HALF_WIDTH_MAP.put('\uFF01', "!");
        HALF_WIDTH_MAP.put('\uFF02', "\"");
        HALF_WIDTH_MAP.put('\uFF03', "#");
        HALF_WIDTH_MAP.put('\uFF04', "$");
        HALF_WIDTH_MAP.put('\uFF05', "%");
        HALF_WIDTH_MAP.put('\uFF06', "&");
        HALF_WIDTH_MAP.put('\uFF07', "'");
        HALF_WIDTH_MAP.put('\uFF08', "(");
        HALF_WIDTH_MAP.put('\uFF09', ")");
        HALF_WIDTH_MAP.put('\uFF0A', "*");
        HALF_WIDTH_MAP.put('\uFF0B', "+");
        HALF_WIDTH_MAP.put('\uFF0C', ",");
        HALF_WIDTH_MAP.put('\uFF0D', "-");
        HALF_WIDTH_MAP.put('\uFF0E', ".");
        HALF_WIDTH_MAP.put('\uFF0F', "/");
        HALF_WIDTH_MAP.put('\uFF10', "0");
        HALF_WIDTH_MAP.put('\uFF11', "1");
        HALF_WIDTH_MAP.put('\uFF12', "2");
        HALF_WIDTH_MAP.put('\uFF13', "3");
        HALF_WIDTH_MAP.put('\uFF14', "4");
        HALF_WIDTH_MAP.put('\uFF15', "5");
        HALF_WIDTH_MAP.put('\uFF16', "6");
        HALF_WIDTH_MAP.put('\uFF17', "7");
        HALF_WIDTH_MAP.put('\uFF18', "8");
        HALF_WIDTH_MAP.put('\uFF19', "9");
        HALF_WIDTH_MAP.put('\uFF1A', ":");
        HALF_WIDTH_MAP.put('\uFF1B', ";");
        HALF_WIDTH_MAP.put('\uFF1C', "<");
        HALF_WIDTH_MAP.put('\uFF1D', "=");
        HALF_WIDTH_MAP.put('\uFF1E', ">");
        HALF_WIDTH_MAP.put('\uFF1F', "?");
        HALF_WIDTH_MAP.put('\uFF20', "@");
        HALF_WIDTH_MAP.put('\uFF21', "A");
        HALF_WIDTH_MAP.put('\uFF22', "B");
        HALF_WIDTH_MAP.put('\uFF23', "C");
        HALF_WIDTH_MAP.put('\uFF24', "D");
        HALF_WIDTH_MAP.put('\uFF25', "E");
        HALF_WIDTH_MAP.put('\uFF26', "F");
        HALF_WIDTH_MAP.put('\uFF27', "G");
        HALF_WIDTH_MAP.put('\uFF28', "H");
        HALF_WIDTH_MAP.put('\uFF29', "I");
        HALF_WIDTH_MAP.put('\uFF2A', "J");
        HALF_WIDTH_MAP.put('\uFF2B', "K");
        HALF_WIDTH_MAP.put('\uFF2C', "L");
        HALF_WIDTH_MAP.put('\uFF2D', "M");
        HALF_WIDTH_MAP.put('\uFF2E', "N");
        HALF_WIDTH_MAP.put('\uFF2F', "O");
        HALF_WIDTH_MAP.put('\uFF30', "P");
        HALF_WIDTH_MAP.put('\uFF31', "Q");
        HALF_WIDTH_MAP.put('\uFF32', "R");
        HALF_WIDTH_MAP.put('\uFF33', "S");
        HALF_WIDTH_MAP.put('\uFF34', "T");
        HALF_WIDTH_MAP.put('\uFF35', "U");
        HALF_WIDTH_MAP.put('\uFF36', "V");
        HALF_WIDTH_MAP.put('\uFF37', "W");
        HALF_WIDTH_MAP.put('\uFF38', "X");
        HALF_WIDTH_MAP.put('\uFF39', "Y");
        HALF_WIDTH_MAP.put('\uFF3A', "Z");
        HALF_WIDTH_MAP.put('\uFF3B', "[");
        HALF_WIDTH_MAP.put('\uFF3C', "\\");
        HALF_WIDTH_MAP.put('\uFF3D', "]");
        HALF_WIDTH_MAP.put('\uFF3E', "^");
        HALF_WIDTH_MAP.put('\uFF3F', "_");
        HALF_WIDTH_MAP.put('\uFF41', "a");
        HALF_WIDTH_MAP.put('\uFF42', "b");
        HALF_WIDTH_MAP.put('\uFF43', "c");
        HALF_WIDTH_MAP.put('\uFF44', "d");
        HALF_WIDTH_MAP.put('\uFF45', "e");
        HALF_WIDTH_MAP.put('\uFF46', "f");
        HALF_WIDTH_MAP.put('\uFF47', "g");
        HALF_WIDTH_MAP.put('\uFF48', "h");
        HALF_WIDTH_MAP.put('\uFF49', "i");
        HALF_WIDTH_MAP.put('\uFF4A', "j");
        HALF_WIDTH_MAP.put('\uFF4B', "k");
        HALF_WIDTH_MAP.put('\uFF4C', "l");
        HALF_WIDTH_MAP.put('\uFF4D', "m");
        HALF_WIDTH_MAP.put('\uFF4E', "n");
        HALF_WIDTH_MAP.put('\uFF4F', "o");
        HALF_WIDTH_MAP.put('\uFF50', "p");
        HALF_WIDTH_MAP.put('\uFF51', "q");
        HALF_WIDTH_MAP.put('\uFF52', "r");
        HALF_WIDTH_MAP.put('\uFF53', "s");
        HALF_WIDTH_MAP.put('\uFF54', "t");
        HALF_WIDTH_MAP.put('\uFF55', "u");
        HALF_WIDTH_MAP.put('\uFF56', "v");
        HALF_WIDTH_MAP.put('\uFF57', "w");
        HALF_WIDTH_MAP.put('\uFF58', "x");
        HALF_WIDTH_MAP.put('\uFF59', "y");
        HALF_WIDTH_MAP.put('\uFF5A', "z");
        HALF_WIDTH_MAP.put('\uFF5B', "{");
        HALF_WIDTH_MAP.put('\uFF5C', "|");
        HALF_WIDTH_MAP.put('\uFF5D', "}");
        HALF_WIDTH_MAP.put('\uFF5E', "~");
        HALF_WIDTH_MAP.put('\uFF61', "\uFF61");
        HALF_WIDTH_MAP.put('\uFF62', "\uFF62");
        HALF_WIDTH_MAP.put('\uFF63', "\uFF63");
        HALF_WIDTH_MAP.put('\uFF64', "\uFF64");
        HALF_WIDTH_MAP.put('\uFF65', "\uFF65");
        HALF_WIDTH_MAP.put('\uFF66', "\uFF66");
        HALF_WIDTH_MAP.put('\uFF67', "\uFF67");
        HALF_WIDTH_MAP.put('\uFF68', "\uFF68");
        HALF_WIDTH_MAP.put('\uFF69', "\uFF69");
        HALF_WIDTH_MAP.put('\uFF6A', "\uFF6A");
        HALF_WIDTH_MAP.put('\uFF6B', "\uFF6B");
        HALF_WIDTH_MAP.put('\uFF6C', "\uFF6C");
        HALF_WIDTH_MAP.put('\uFF6D', "\uFF6D");
        HALF_WIDTH_MAP.put('\uFF6E', "\uFF6E");
        HALF_WIDTH_MAP.put('\uFF6F', "\uFF6F");
        HALF_WIDTH_MAP.put('\uFF70', "\uFF70");
        HALF_WIDTH_MAP.put('\uFF71', "\uFF71");
        HALF_WIDTH_MAP.put('\uFF72', "\uFF72");
        HALF_WIDTH_MAP.put('\uFF73', "\uFF73");
        HALF_WIDTH_MAP.put('\uFF74', "\uFF74");
        HALF_WIDTH_MAP.put('\uFF75', "\uFF75");
        HALF_WIDTH_MAP.put('\uFF76', "\uFF76");
        HALF_WIDTH_MAP.put('\uFF77', "\uFF77");
        HALF_WIDTH_MAP.put('\uFF78', "\uFF78");
        HALF_WIDTH_MAP.put('\uFF79', "\uFF79");
        HALF_WIDTH_MAP.put('\uFF7A', "\uFF7A");
        HALF_WIDTH_MAP.put('\uFF7B', "\uFF7B");
        HALF_WIDTH_MAP.put('\uFF7C', "\uFF7C");
        HALF_WIDTH_MAP.put('\uFF7D', "\uFF7D");
        HALF_WIDTH_MAP.put('\uFF7E', "\uFF7E");
        HALF_WIDTH_MAP.put('\uFF7F', "\uFF7F");
        HALF_WIDTH_MAP.put('\uFF80', "\uFF80");
        HALF_WIDTH_MAP.put('\uFF81', "\uFF81");
        HALF_WIDTH_MAP.put('\uFF82', "\uFF82");
        HALF_WIDTH_MAP.put('\uFF83', "\uFF83");
        HALF_WIDTH_MAP.put('\uFF84', "\uFF84");
        HALF_WIDTH_MAP.put('\uFF85', "\uFF85");
        HALF_WIDTH_MAP.put('\uFF86', "\uFF86");
        HALF_WIDTH_MAP.put('\uFF87', "\uFF87");
        HALF_WIDTH_MAP.put('\uFF88', "\uFF88");
        HALF_WIDTH_MAP.put('\uFF89', "\uFF89");
        HALF_WIDTH_MAP.put('\uFF8A', "\uFF8A");
        HALF_WIDTH_MAP.put('\uFF8B', "\uFF8B");
        HALF_WIDTH_MAP.put('\uFF8C', "\uFF8C");
        HALF_WIDTH_MAP.put('\uFF8D', "\uFF8D");
        HALF_WIDTH_MAP.put('\uFF8E', "\uFF8E");
        HALF_WIDTH_MAP.put('\uFF8F', "\uFF8F");
        HALF_WIDTH_MAP.put('\uFF90', "\uFF90");
        HALF_WIDTH_MAP.put('\uFF91', "\uFF91");
        HALF_WIDTH_MAP.put('\uFF92', "\uFF92");
        HALF_WIDTH_MAP.put('\uFF93', "\uFF93");
        HALF_WIDTH_MAP.put('\uFF94', "\uFF94");
        HALF_WIDTH_MAP.put('\uFF95', "\uFF95");
        HALF_WIDTH_MAP.put('\uFF96', "\uFF96");
        HALF_WIDTH_MAP.put('\uFF97', "\uFF97");
        HALF_WIDTH_MAP.put('\uFF98', "\uFF98");
        HALF_WIDTH_MAP.put('\uFF99', "\uFF99");
        HALF_WIDTH_MAP.put('\uFF9A', "\uFF9A");
        HALF_WIDTH_MAP.put('\uFF9B', "\uFF9B");
        HALF_WIDTH_MAP.put('\uFF9C', "\uFF9C");
        HALF_WIDTH_MAP.put('\uFF9D', "\uFF9D");
        HALF_WIDTH_MAP.put('\uFF9E', "\uFF9E");
        HALF_WIDTH_MAP.put('\uFF9F', "\uFF9F");
        HALF_WIDTH_MAP.put('\uFFE5', "\u005C\u005C");
    }

    /**
     * Return half-width version of that character if possible. Return null if
     * not possible
     *
     * @param ch input character
     * @return CharSequence object if the mapping for ch exists. Return null
     *         otherwise.
     */
    public static CharSequence tryGetHalfWidthText(char ch) {
        if (HALF_WIDTH_MAP.containsKey(ch)) {
            return HALF_WIDTH_MAP.get(ch);
        } else {
            return null;
        }
    }
}
