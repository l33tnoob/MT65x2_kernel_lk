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

package com.android.bluetooth.pbap;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.bluetooth.pbap.BluetoothPbapPath;

import java.util.ArrayList;
import java.util.HashSet;

import javax.obex.ResponseCodes;

public class BluetoothPbapVCardListing {
    private static final String TAG = "BluetoothPbapVCardListing";

    public static final boolean DEBUG = true;

    /* VCard list ordering */
    public static final int VCARD_ORDER_ALPHA = 0;

    public static final int VCARD_ORDER_INDEX = 1;

    public static final int VCARD_ORDER_PHONETICAL = 2;

    public static final int VCARD_ORDER_DEFAULT = VCARD_ORDER_ALPHA;

    /* VCard search attribute */
    public static final int VCARD_SEARCH_NAME = 0;

    public static final int VCARD_SEARCH_NUMBER = 1;

    public static final int VCARD_SEARCH_SOUND = 2;

    public static final int VCARD_SEARCH_DEFAULT = VCARD_SEARCH_NAME;

    /* VCard listing header */
    private static final String VCARD_LISTING_BEGIN = "<?xml version=\"1.0\"?>"
            + "<!DOCTYPE vcard-listing SYSTEM \"vcard-listing.dtd\">"
            + "<vCard-listing version=\"1.0\">";

    private static final String VCARD_LISTING_END = "</vCard-listing>";

    private static final String[] PB_PROJECTION = new String[] {
            Contacts._ID, // 0
            Contacts.DISPLAY_NAME, // 1
    };

    static final String[] PB_NUMBER_PROJECTION = new String[] {
            RawContacts.CONTACT_ID, // 0
            Contacts.DISPLAY_NAME, // 1
    };

    private static final String[] PB_NAME_PROJECTION = new String[] {
        Contacts.DISPLAY_NAME, // 1
    };

    /* member variable */
    private String mLocalName = null;

    private String mLocalNum = null;

    private Cursor mCursor = null;

    private Context mContext = null;

    private BluetoothPbapWriter mWriter = null;

    private String mResultPath = null;

    private boolean mDirty = true; // If not listed before, set to true to
                                   // prevent get entry request

    private Long[] mIDList = null; // This is used to keep all IDs of list
                                   // result sorted by name(only for PB)

    private boolean mSimDirty = true; // If SIM folder is not listed before, set
                                      // to true to prevent get entry request

    private Long[] mSimIDList = null; // This is used to keep all IDs of list
                                      // result sorted by name(only for
                                      // sim1\telecom\PB)

    private int mSearchAttrib = 0;

    private String mSearchValue = null;

    // private int mListOffset = 0;
    // private int mMaxListCount = 0;
    //private BluetoothPbapSimAdn mSimAdn = null;

    private boolean mIOTSolutionOn = false;

    public BluetoothPbapVCardListing(Context context, String localName, String localNum, boolean iotSolutionOn) {
        printLog("BluetoothPbapVCardListing : localName=" + localName + ", localNum=" + localNum);
        mLocalName = localName;
        mLocalNum = localNum;
        mContext = context;
        //mSimAdn = new BluetoothPbapSimAdn(context);
        mIOTSolutionOn = iotSolutionOn;
    }

    public void resetPbapSearchAttrib() {
        mSearchAttrib = -1;//reset it to -1, always relist
    }
    /*****************
     * Public interface *
     ******************/
    /* Return OBEX result code */
    public int list(int type, int order, int searchAttr, String searchVal, int listOffset,
            int maxListCount) {
        int ownerNotInc = 1;
        int ret = ResponseCodes.OBEX_HTTP_PRECON_FAILED;
        ArrayList<String> result = null;

        printLog("init(" + type + "," + order + "," + searchAttr + "," + searchVal + ","
                + listOffset + "," + maxListCount + ")");
        // TODO: Init query cursor
        // Create writer
        if (!openWriter()) {
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }

        result = new ArrayList<String>();
        if (result == null) {
            errorLog("Alloc result failed");
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }

        // Get cursor
        switch (type) {
            case BluetoothPbapPath.FOLDER_TYPE_PB:
                /*
                 * Check if owner is included. owner will not be included in
                 * search result
                 */
                if (TextUtils.isEmpty(searchVal)) {
                    ownerNotInc = 0;
                }
                if (mDirty || !isSameAttribute(searchAttr, searchVal)) {
                    mSearchAttrib = searchAttr;
                    mSearchValue = searchVal;
                    ret = listPb(true, listOffset, maxListCount, result);
                    if (ret == ResponseCodes.OBEX_HTTP_OK) {
                        mDirty = false;
                    } else {
                        mDirty = true;
                    }
                } else {
                    ret = listPb(false, listOffset, maxListCount, result);
                }
                break;
            case BluetoothPbapPath.FOLDER_TYPE_ICH:
            case BluetoothPbapPath.FOLDER_TYPE_OCH:
            case BluetoothPbapPath.FOLDER_TYPE_MCH:
            case BluetoothPbapPath.FOLDER_TYPE_CCH:
                ret = listCallLog(type, listOffset, maxListCount, searchAttr, searchVal, result);
                break;
            case BluetoothPbapPath.FOLDER_TYPE_SIM1_PB:
                /*
                 * Check if owner is included. owner will not be included in
                 * search result
                 */
                if (TextUtils.isEmpty(searchVal)) {
                    ownerNotInc = 0;
                }
                if (mSimDirty || !isSameAttribute(searchAttr, searchVal)) {
                    mSearchAttrib = searchAttr;
                    mSearchValue = searchVal;
                    ret = listSimPb(true, listOffset, maxListCount, result);
                    if (ret == ResponseCodes.OBEX_HTTP_OK) {
                        mSimDirty = false;
                    } else {
                        mSimDirty = true;
                    }
                } else {
                    ret = listSimPb(false, listOffset, maxListCount, result);
                }
                break;
            default:
                errorLog("unsupported type=" + type);
                ret = ResponseCodes.OBEX_HTTP_BAD_REQUEST;
                break;
        }

        // Output begin headser
        if (mWriter == null) {
            ret = ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        } else if (ret == ResponseCodes.OBEX_HTTP_OK) {
            boolean r;
            r = mWriter.write(VCARD_LISTING_BEGIN);
            for (int i = 0; r && i < result.size(); i++) {
                r = mWriter.write("<card handle=\"" + (listOffset + i + ownerNotInc)
                        + ".vcf\" name=\"" + result.get(i) + "\"" + "/>");
            }
            if (r) {
                r = mWriter.write(VCARD_LISTING_END);
            }
            mWriter.terminate();
            if (r) {
                mResultPath = mWriter.getPath();
            } else {
                ret = ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
        } else {
            mWriter.terminate();
        }
        mWriter = null;
        return ret;
    }

    public String getPath() {
        return mResultPath;
    }

    public long queryPbID(int index) {
        printLog("[API] queryPbID(" + index + ")");
        if (mDirty) {
            /*
             * Relisting : set list offset to 1 to prevent owner card is
             * included
             */
            listPb(true, 1, 0, null);
        }
        if (mIDList != null && mIDList.length > index) {
            return mIDList[index];
        }
        errorLog("ERR] can not found id for index " + index + ". mIDList.length = "
                + ((mIDList == null) ? 0 : mIDList.length));
        return -1;
    }

    public long querySimPbID(int index) {
        printLog("[API] querySimPbID(" + index + ")");
        if (mSimDirty) {
            /*
             * Relisting : set list offset to 1 to prevent owner card is
             * included
             */
            listSimPb(true, 1, 0, null);
        }
        if (mSimIDList != null && mSimIDList.length > index) {
            return mSimIDList[index];
        }
        errorLog("ERR] can not found id for index " + index + ". mSimIDList.length = "
                + ((mSimIDList == null) ? 0 : mSimIDList.length));
        return -1;
    }

    // public boolean isDirty() {
    // return mDirty;
    // }
    /*****************
     * Private functions *
     ******************/
    private boolean openWriter() {
        boolean ret = true;
        printLog("[API] openWriter");
        // Close existed one first
        if (mWriter != null) {
            mWriter.terminate();
        } else {
            mWriter = new BluetoothPbapWriter();
        }

        if (mWriter != null) {
            if (!mWriter.init(mContext)) {
                mWriter = null;
                ret = false;
            }
        } else {
            ret = false;
        }

        if (!ret) {
            errorLog("Failed to open PbapWriter");
        }
        return ret;
    }

    private String getOwnerName() {
        if (!TextUtils.isEmpty(mLocalName)) {
            return mLocalName;
        } else if (!TextUtils.isEmpty(mLocalNum)) {
            return mLocalNum;
        } else {
            printLog("getOwnerName : name=" + mLocalName + ", num=" + mLocalNum);
            return new String("");
        }
    }

    private String genIDList(int offset, int count) {
        String idList = null;
        if (mIDList != null && mIDList.length >= (offset + count)) {
            idList = new String("(");
            // for(long i : mIDList) {
            for (int i = 0; i < count; i++) {
                idList += mIDList[i + offset];
                idList += ",";
            }
            if (idList.charAt(idList.length() - 1) == ',') {
                idList = idList.substring(0, idList.length() - 1);
            }
            idList = idList + ")";
        }
        printLog("[API] genIDList(" + offset + "," + count + ") = " + idList);
        return idList;
    }

    private String genSimIDList(int offset, int count) {
        String idList = null;
        if (mSimIDList != null && mSimIDList.length >= (offset + count)) {
            idList = new String("(");
            // for(long i : mIDList) {
            for (int i = 0; i < count; i++) {
                idList += mSimIDList[i + offset];
                idList += ",";
            }
            if (idList.charAt(idList.length() - 1) == ',') {
                idList = idList.substring(0, idList.length() - 1);
            }
            idList = idList + ")";
        }
        printLog("[API] genSimIDList(" + offset + "," + count + ") = " + idList);
        return idList;
    }

    /* Return OBEX result code */
    private int listPb(boolean relist, int listOffset, int maxListCount, ArrayList<String> result) {
        Uri uri = null;
        ContentResolver resolver = mContext.getContentResolver();
        Cursor cursor = null;
        boolean ownerInc = (TextUtils.isEmpty(mSearchValue));
        printLog("[API] listPb(" + String.valueOf(relist) + "," + listOffset + "," + maxListCount
                + ")");
        printLog("SearchVal=" + mSearchValue + ", mSearchAttrib=" + mSearchAttrib);
        try {
            if (resolver == null) {
                errorLog("[ERR] resolver is null");
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
            if (ownerInc) {
                // If owner is included
                if (listOffset == 0) {
                    // if listOffset is 0
                    printLog("Add owner name : " + getOwnerName());
                    result.add(getOwnerName());
                    maxListCount--;
                }
                if (listOffset > 0) {
                    listOffset--;
                }
            }
            if (relist) {
                String selection = null;
                if (!mIOTSolutionOn){
                    selection = RawContacts.INDICATE_PHONE_SIM + "=" + RawContacts.INDICATE_PHONE;
                } else {
                    selection = "";
                }
                String[] projection = null;
                HashSet<Long> idSet = null;
                ArrayList<Long> idList = null;
    
                mIDList = null;
                if (!TextUtils.isEmpty(mSearchValue)) {
                    if (mSearchAttrib != VCARD_SEARCH_NUMBER) {
                        uri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri
                                .encode(mSearchValue));
                        projection = PB_PROJECTION;
                    } else {
                        uri = Phone.CONTENT_URI;
                        projection = PB_NUMBER_PROJECTION;
                        if (!mIOTSolutionOn) {
                            selection += " AND " + Phone.NUMBER + " LIKE '%" + mSearchValue + "%'";
                        } else {
                            selection += Phone.NUMBER+" LIKE '%"+mSearchValue+"%'";
                        }
                    }
                } else {
                    uri = Contacts.CONTENT_URI;
                    projection = PB_PROJECTION;
                }
                printLog("selection=" + selection);
                cursor = resolver.query(uri, projection, selection, null, "upper("
                        + Contacts.DISPLAY_NAME + ")");
                if (cursor != null) {
                    printLog("get cursor successfully. count=" + cursor.getCount());
                    // Retrieve data
                    cursor.moveToFirst();
                    idSet = new HashSet<Long>();
                    idList = new ArrayList<Long>();
                    printLog("Retrieve data : listOffset=" + listOffset + ", maxListCount="
                            + maxListCount);
                    while (!cursor.isAfterLast()) {
                        if (idSet.add(cursor.getLong(0))) {
                            // ID is not contained by idSet
                            idList.add(cursor.getLong(0));
                            if (maxListCount > 0 && idList.size() > listOffset) {
                                printLog("Add one record : " + cursor.getString(1) + ",ID:"
                                        + cursor.getLong(0));
                                result.add(cursor.getString(1));
                                maxListCount--;
                            }
                        }
                        cursor.moveToNext();
                    }
                    mIDList = idList.toArray(new Long[0]);
                    idSet = null;
                    idList = null;
                } else {
                    errorLog("Query id list failed");
                }
            } else {
                if ((mIDList.length - listOffset) < maxListCount) {
                    maxListCount = (mIDList.length - listOffset > 0) ? (mIDList.length - listOffset) : 0;
                } 
                if (maxListCount != 0) {
                    String idList = genIDList(listOffset, maxListCount);
                    if (idList != null) {
                        uri = Contacts.CONTENT_URI;
                        cursor = resolver.query(uri, PB_NAME_PROJECTION,
                                Contacts._ID + " IN " + idList, null, Contacts.DISPLAY_NAME);
                        if (cursor == null || cursor.getCount() != maxListCount) {
                            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
                        } else {
                            // Get name list
                            cursor.moveToFirst();
                            while (!cursor.isAfterLast()) {
                                printLog("Get record!!");
                                result.add(cursor.getString(0));
                                cursor.moveToNext();
                            }
                        }
                    } else {
                        return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
                    }
                }
            }
            return ResponseCodes.OBEX_HTTP_OK;
        } finally {
            // Close cursor
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /* Return OBEX result code */

    private int listSimPb(boolean relist, int listOffset, int maxListCount, ArrayList<String> result) {
        Uri uri = null;
        ContentResolver resolver = mContext.getContentResolver();
        Cursor cursor = null;
        boolean ownerInc = (TextUtils.isEmpty(mSearchValue));
        printLog("[API] listSimPb(" + String.valueOf(relist) + "," + listOffset + ","
                + maxListCount + ")");
        printLog("SearchVal=" + mSearchValue + ", mSearchAttrib=" + mSearchAttrib);
        try {
            if (resolver == null) {
                errorLog("[ERR] resolver is null");
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
            if (ownerInc) {
                // If owner is included
                if (listOffset == 0) {
                    // if listOffset is 0
                    printLog("Add owner name : " + getOwnerName());
                    result.add(getOwnerName());
                    maxListCount--;
                }
                if (listOffset > 0) {
                    listOffset--;
                }
            }
            if (relist) {
                String selection = null;
                if (!mIOTSolutionOn) {
                    selection = RawContacts.INDICATE_PHONE_SIM + ">" + RawContacts.INDICATE_PHONE;
                } else {
                    selection = Contacts.IN_VISIBLE_GROUP + "=" + 1000;
                }
                String[] projection = null;
                HashSet<Long> idSet = null;
                ArrayList<Long> idList = null;
    
                mSimIDList = null;
                if (!TextUtils.isEmpty(mSearchValue)) {
                    if (mSearchAttrib != VCARD_SEARCH_NUMBER) {
                        uri = Uri.withAppendedPath(Contacts.CONTENT_FILTER_URI, Uri
                                .encode(mSearchValue));
                        projection = PB_PROJECTION;
                    } else {
                        uri = Phone.CONTENT_URI;
                        projection = PB_NUMBER_PROJECTION;
                        selection += " AND " + Phone.NUMBER + " LIKE '%" + mSearchValue + "%'";
                    }
                } else {
                    uri = Contacts.CONTENT_URI;
                    projection = PB_PROJECTION;
                }
                printLog("selection=" + selection);
                cursor = resolver.query(uri, projection, selection, null, "upper("
                        + Contacts.DISPLAY_NAME + ")");
                if (cursor != null) {
                    printLog("get cursor successfully. count=" + cursor.getCount());
                    // Retrieve data
                    cursor.moveToFirst();
                    idSet = new HashSet<Long>();
                    idList = new ArrayList<Long>();
                    printLog("Retrieve data : listOffset=" + listOffset + ", maxListCount="
                            + maxListCount);
                    while (!cursor.isAfterLast()) {
                        if (idSet.add(cursor.getLong(0))) {
                            // ID is not contained by idSet
                            idList.add(cursor.getLong(0));
                            if (maxListCount > 0 && idList.size() > listOffset) {
                                printLog("Add one record : " + cursor.getString(1) + ",ID:"
                                        + cursor.getLong(0));
                                result.add(cursor.getString(1));
                                maxListCount--;
                            }
                        }
                        cursor.moveToNext();
                    }
                    mSimIDList = idList.toArray(new Long[0]);
                    idSet = null;
                    idList = null;
                } else {
                    errorLog("Query id list failed");
                }
            } else {
                if ((mSimIDList.length - listOffset) < maxListCount) {
                    maxListCount = (mSimIDList.length - listOffset > 0) ? (mSimIDList.length - listOffset) : 0;
                } 
                if (maxListCount != 0) {
                    String idList = genSimIDList(listOffset, maxListCount);
                    if (idList != null) {
                        uri = Contacts.CONTENT_URI;
                        cursor = resolver.query(uri, PB_NAME_PROJECTION,
                                Contacts._ID + " IN " + idList, null, Contacts.DISPLAY_NAME);
                        if (cursor == null || cursor.getCount() != maxListCount) {
                            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
                        } else {
                            // Get name list
                            cursor.moveToFirst();
                            while (!cursor.isAfterLast()) {
                                printLog("Get record!!");
                                result.add(cursor.getString(0));
                                cursor.moveToNext();
                            }
                        }
                    } else {
                        return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
                    }
                }
            }
            return ResponseCodes.OBEX_HTTP_OK;
        } finally {
            // Close cursor
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /* Return OBEX result code */
    private int listCallLog(int type, int listOffset, int maxListCount, int searchAttrib,
            String searchVal, ArrayList<String> result) {
        int ret = ResponseCodes.OBEX_HTTP_OK;
        Uri myUri = CallLog.Calls.CONTENT_URI;
        Cursor cursor = null;
        String selection = null;
        String[] projection = new String[] {
                Calls.CACHED_NAME, // 0
                Calls.NUMBER, // 1
        };

        printLog("[API] listCallLog");
        switch (type) {
            case BluetoothPbapPath.FOLDER_TYPE_ICH:
                selection = Calls.TYPE + "=" + CallLog.Calls.INCOMING_TYPE;
                break;
            case BluetoothPbapPath.FOLDER_TYPE_OCH:
                selection = Calls.TYPE + "=" + CallLog.Calls.OUTGOING_TYPE;
                break;
            case BluetoothPbapPath.FOLDER_TYPE_MCH:
                selection = Calls.TYPE + "=" + CallLog.Calls.MISSED_TYPE;
                break;
            case BluetoothPbapPath.FOLDER_TYPE_CCH:
                break;
            default:
                return ResponseCodes.OBEX_HTTP_BAD_REQUEST;
        }
        /* Add search into selection */
        if (!TextUtils.isEmpty(searchVal)) {
            if (selection != null) {
                selection += " AND ";
            } else {
                selection = new String();
            }
            if (searchAttrib != VCARD_SEARCH_NUMBER) {
                selection += Calls.CACHED_NAME + " LIKE '%" + searchVal + "%'";
            } else {
                selection += Calls.NUMBER + " LIKE '%" + searchVal + "%'";
            }
        }
        printLog("selection is " + selection);
        try {
            String num = null;
            int callSize = 0;
            cursor = mContext.getContentResolver().query(myUri, projection, selection, null,
                    CallLog.Calls.DATE + " DESC");
            if (cursor != null) {
                callSize = cursor.getCount();
                printLog("callSize==" + callSize);
                if (callSize <= listOffset) {
                    ret = ResponseCodes.OBEX_HTTP_NOT_FOUND;
                } else {
                    /*
                     * if((callSize-listOffset) < maxListCount) { maxListCount =
                     * callSize-listOffset; }
                     */
                    cursor.moveToPosition(listOffset);
                    while (maxListCount > 0 && !cursor.isAfterLast()) {
                        num = cursor.getString(0);
                        if (TextUtils.isEmpty(num)) {
                            num = cursor.getString(1);
                        }
                        if (num != null) {
                            result.add(num);
                        } else {
                            result.add(new String());
                        }
                        cursor.moveToNext();
                        maxListCount--;
                    }
                }
            } else {
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return ret;
    }

    /*
     * private int listSimPb(int listOffset, int maxListCount, int searchAttrib,
     * String searchVal, ArrayList<String> result) { ListIterator<AdnRecord>
     * iterator= null; AdnRecord adn = null; String name = null; boolean
     * ownerInc = (TextUtils.isEmpty(searchVal) == true); if(ownerInc) {
     * if(listOffset == 0) { result.add(mSimAdn.getOwnerName()); maxListCount--;
     * } if(listOffset > 0) listOffset--; } // update ADN if(
     * !mSimAdn.updateAdn() ) return mSimAdn.getLastError(); // search ADN
     * if(!TextUtils.isEmpty(searchVal)) { if(searchAttrib !=
     * VCARD_SEARCH_NUMBER) { mSimAdn.searchAdn(searchVal,false); }else{
     * mSimAdn.searchAdn(searchVal,true); } } //sort ADN mSimAdn.sortAdn(); //
     * fill result if(mSimAdn.getCount() <= listOffset) return
     * ResponseCodes.OBEX_HTTP_NOT_FOUND; if( maxListCount >
     * (mSimAdn.getCount()-listOffset)){ maxListCount =
     * (mSimAdn.getCount()-listOffset); } iterator =
     * mSimAdn.getAdnList().listIterator(listOffset); while(maxListCount > 0 &&
     * iterator.hasNext()){ maxListCount--; adn = iterator.next(); name =
     * adn.getAlphaTag(); if(TextUtils.isEmpty(name)) name = adn.getNumber();
     * printLog("listSimPb : name="+name); if(TextUtils.isEmpty(name)){
     * errorLog("Empty Adn record at "+(iterator.nextIndex()-1)+". Remove it");
     * iterator.remove(); }else{ result.add(name); } } return
     * ResponseCodes.OBEX_HTTP_OK; }
     */
    private boolean isSameAttribute(int searchAttr, String searchVal) {

        boolean b = true;

        if (!TextUtils.isEmpty(searchVal) || !TextUtils.isEmpty(mSearchValue)) {
            if ((searchAttr != mSearchAttrib) || !TextUtils.equals(searchVal, mSearchValue)) {
                b = false;
            }
        }
        return b;

//        if (((TextUtils.isEmpty(searchVal) != true) || (TextUtils.isEmpty(mSearchValue) != true))
//                && (searchAttr != mSearchAttrib || TextUtils.equals(searchVal, mSearchValue) != true)) {
//            return false;
//        } else {
//            return true;
//        }
    }

    /* Utility function */
    private void printLog(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }

    private void errorLog(String msg) {
        Log.e(TAG, msg);
    }
}
