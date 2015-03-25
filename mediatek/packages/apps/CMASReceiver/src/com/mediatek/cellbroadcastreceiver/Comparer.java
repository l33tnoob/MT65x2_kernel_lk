/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.cellbroadcastreceiver;

import android.database.Cursor;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.util.Log;

import com.android.internal.telephony.gsm.SmsCbConstants;

import java.lang.reflect.Field;

import java.util.ArrayList;
import java.util.HashSet;

/**
 * This service manages the display and animation of broadcast messages. Emergency messages display with a flashing animated
 * exclamation mark icon, and an alert tone is played when the alert is first shown to the user (but not when the user views
 * a previously received broadcast).
 */
public class Comparer {
    public static final String INVALID_UPDATE_CB = "invalidID";

    private static final String TAG = "[CMAS]Comparer";

    private static final String CB_SMSCBMESSAGE_FIELD = "mSmsCbMessage";

    /** Maximum number of message IDs to save before removing the oldest message ID. */
    private static final int MAX_MESSAGE_ID_SIZE = 65535;

    /** Cache of received message IDs, for duplicate message detection. */
    private HashSet<CompareInfo> mItemSet = new HashSet<CompareInfo>(8);

    /** List of message IDs received, for removing oldest ID when max message IDs are received. */
    private ArrayList<CompareInfo> mItemList = new ArrayList<CompareInfo>(8);

    /** Index of message ID to replace with new message ID when max message IDs are received. */
    private int mItemListIndex = 0;

    /** Container for message ID and geographical scope, for duplicate message detection. */
    private static final class CompareInfo {
        private final int mMessageId;
        private final SmsCbLocation mLocation;
        private final int mGeographicalScope;
        private final int mSerialNumber;
        private boolean mRead;

        CompareInfo(int messageId, SmsCbLocation location, int gs, int serialNum, boolean readflag) {
            mMessageId = messageId;
            mLocation = location;
            mGeographicalScope = gs;
            mSerialNumber = serialNum;
            mRead = readflag;
        }

        @Override
        public int hashCode() {
            int hcode = mMessageId * 31 + mSerialNumber;

            Log.d(TAG, this + " hashcode  " + hcode);
            return hcode;
        }

        @Override
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            }

            if (o instanceof CompareInfo) {
                CompareInfo other = (CompareInfo) o;
                boolean res = isIdentify(other);
                Log.d(TAG, this + " equals other  " + other);
                Log.d(TAG, " equals ? " + res);
                return res;
            }
            return false;
        }

        @Override
        public String toString() {
            return "{messageId: " + mMessageId + " location: " + mLocation.toString() + " GeographicalScope "
                    + mGeographicalScope + " SerialNumber " + mSerialNumber + '}';
        }

        private boolean isIdentify(CompareInfo other) {
            boolean res = false;

            if (mMessageId != other.mMessageId || mSerialNumber != other.mSerialNumber) {
                return res;
            }
            switch (mGeographicalScope) {
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE_IMMEDIATE:
                if (mLocation.getCid() == other.mLocation.getCid()) {
                    res = true;
                }
                break;
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_PLMN_WIDE: {
                res = true;
            }
                break;
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_LA_WIDE:
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE:
                if (mLocation.getLac() == other.mLocation.getLac()) {
                    res = true;
                }
                break;
            default:
                Log.e(TAG, " error mGeographicalScope , in isIdendity()");
                break;
            }

            return res;
        }
    }

    private static SmsCbMessage getSmsCbFromCellBroadcast(CellBroadcastMessage cbm) {
        Class ownerClass = cbm.getClass();
        Object property = null;

        if (cbm == null) {
            return null;
        }

        try {
            Field field = ownerClass.getDeclaredField(CB_SMSCBMESSAGE_FIELD);
            field.setAccessible(true);
            property = field.get(cbm);
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        return (SmsCbMessage) property;
    }

    public Comparer() {
    }

    public boolean add(CellBroadcastMessage cbm) {
        if (cbm == null) {
            Log.e(TAG, "add null CellBroadcastMessage");
            return false;
        }

        SmsCbMessage smsCb = getSmsCbFromCellBroadcast(cbm);
        if (smsCb == null) {
            return false;
        }
        CompareInfo newInfo = new CompareInfo(cbm.getServiceCategory(), smsCb.getLocation(),
                smsCb.getGeographicalScope(), smsCb.getSerialNumber(), cbm.isRead());

        // Add the new message ID to the list. It's okay if this is a duplicate message ID,
        // because the list is only used for removing old message IDs from the hash set.
        if (mItemList.size() < MAX_MESSAGE_ID_SIZE) {
            mItemList.add(newInfo);
        } else {
            // Get oldest message ID from the list and replace with the new message ID.
            CompareInfo oldestId = mItemList.get(mItemListIndex);
            mItemList.set(mItemListIndex, newInfo);
            Log.d(TAG, "message ID limit reached, removing oldest message ID " + oldestId);
            // Remove oldest message ID from the set.
            mItemSet.remove(oldestId);
            if (++mItemListIndex >= MAX_MESSAGE_ID_SIZE) {
                mItemListIndex = 0;
            }
        }

        // Set.add() returns false if message ID has already been added
        if (!mItemSet.add(newInfo)) {
            return false;
        }

        return true;
    }

    public boolean isReadForDuplicateMessage(CellBroadcastMessage cbm) {
        if (cbm == null || mItemSet.size() == 0) {
            return false;
        }

        SmsCbMessage smsCb = getSmsCbFromCellBroadcast(cbm);
        if (smsCb == null) {
            return false;
        }
        CompareInfo newInfo = new CompareInfo(cbm.getServiceCategory(), smsCb.getLocation(), smsCb.getGeographicalScope(),
                smsCb.getSerialNumber(), cbm.isRead());

        // Log.d(TAG, " isReadForDuplicateMessage size " + mItemSet.size());
        for (CompareInfo itemInfo : mItemSet) {
            if (newInfo.isIdentify(itemInfo)) {

                Log.d(TAG, " markReadForDuplicateMessage item read " + itemInfo.mRead);
                return itemInfo.mRead;
            }
        }

        return false;
    }

    public static int getUpdateNumOfCb(CellBroadcastMessage cbm) {
        int updateNum = 0;
        if (cbm == null) {
            return updateNum;
        }

        SmsCbMessage smsCB = getSmsCbFromCellBroadcast(cbm);
        Log.d(TAG, "isUpdateCb smsCB " + smsCB);
        if (smsCB != null) {
            int serNum = smsCB.getSerialNumber();
            updateNum = serNum & 0x0000000f;
        }
        return updateNum;
    }

    public static String searchUpdatedCB(Cursor cursor, CellBroadcastMessage cbm) {
        if (cursor == null) {
            return null;
        }

        SmsCbMessage smsCB = getSmsCbFromCellBroadcast(cbm);
        if (smsCB == null) {
            return null;
        }

        int serialNum = smsCB.getSerialNumber();
        int gs = smsCB.getGeographicalScope();
        int messageCode = (serialNum & 0x3ff0) >> 4;
        int messageIdentifer = smsCB.getServiceCategory();
        int updateNum = serialNum & 0x0000000f;

        String cbId = null;
        try {

            cursor.moveToFirst();
            if (cursor.getCount() == 0) {
                return null;
            }
            Log.d(TAG, "searchUpdatedCB cursor size " + cursor.getCount());
            do {
                int oldGS = cursor.getInt(cursor.getColumnIndexOrThrow(
                        Telephony.CellBroadcasts.GEOGRAPHICAL_SCOPE));
                int oldSN = cursor.getInt(cursor.getColumnIndexOrThrow(
                        Telephony.CellBroadcasts.SERIAL_NUMBER));
                int oldMsgCode = (oldSN & 0x3ff0) >> 4;
                int oldMsgIdentifier = cursor.getInt(cursor.getColumnIndexOrThrow(
                        Telephony.CellBroadcasts.SERVICE_CATEGORY));
                int oldUpdateNum = oldSN & 0x0000000f;

                if (oldGS == gs && oldMsgCode == messageCode && 
                        oldMsgIdentifier == messageIdentifer) {
                    //Log.d(TAG, "searchUpdatedCB sn " + serialNum + " vs old sn " + oldSN);
                    //Log.d(TAG, "searchUpdatedCB " + updateNum + " vs " + oldUpdateNum);

                    if (updateNum > oldUpdateNum) {

                        cbId = Integer.toString(cursor.getInt(cursor.getColumnIndexOrThrow(Telephony.CellBroadcasts._ID)));
                    } else if (updateNum < oldUpdateNum) {
                        Log.d(TAG, "searchUpdatedCB INVALID_UPDATE_CB");
                        cbId = INVALID_UPDATE_CB;
                    }
                    break;

                }
            } while (cursor.moveToNext());
        } finally {
            cursor.close();
        }

        Log.d(TAG, "searchUpdatedCB cbId " + cbId);
        return cbId;
    }

    public static Comparer createFromCursor(Cursor cursor) {
        if (cursor == null) {
            return null;
        }

        Log.d(TAG, " createFromCursor cursor size " + cursor.getCount());

        Comparer newCbList = new Comparer();

        try {
            if (cursor.getCount() == 0) {
                return null;
            }
            cursor.moveToFirst();
            do {
                newCbList.add(CellBroadcastMessage.createFromCursor(cursor));
            } while (cursor.moveToNext());
            // newCbList.add(CellBroadcastMessage.createFromCursor(cursor));
        } finally {
            cursor.close();
        }

        Log.d(TAG, " newCbList size " + newCbList.mItemSet.size());

        return newCbList;
    }

    public void clear() {
        Log.d(TAG, "clear : set currSize " + mItemSet.size());
        mItemSet.clear();
        mItemList.clear();
        mItemListIndex = 0;
    }

}
