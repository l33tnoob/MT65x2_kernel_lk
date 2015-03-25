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

package com.mediatek.bluetooth.share;

import android.content.ContentValues;
import android.database.Cursor;
import android.net.Uri;
import android.provider.BaseColumns;

import java.util.Map.Entry;
import java.util.Set;

/**
 * TODO [OPP] total / done will be received from update event, need to keep in db ?
 */
public class BluetoothShareTask {

    public static final int ID_NULL = -519;

    public static final int TYPE_OPPC_GROUP_START = 0;

    public static final int TYPE_OPPC_PUSH = TYPE_OPPC_GROUP_START + 1;

    public static final int TYPE_OPPC_PULL = TYPE_OPPC_GROUP_START + 2;

    public static final int TYPE_OPPC_GROUP_END = TYPE_OPPC_GROUP_START + 9;

    public static final int TYPE_OPPS_GROUP_START = 10;

    public static final int TYPE_OPPS_PUSH = TYPE_OPPS_GROUP_START + 1;

    public static final int TYPE_OPPS_PULL = TYPE_OPPS_GROUP_START + 2;

    public static final int TYPE_OPPS_GROUP_END = TYPE_OPPS_GROUP_START + 9;

    public static final int TYPE_BIPI_GROUP_START = 20;

    public static final int TYPE_BIPI_PUSH = TYPE_BIPI_GROUP_START + 1;

    public static final int TYPE_BIPI_GROUP_END = TYPE_BIPI_GROUP_START + 9;

    public static final int TYPE_BIPR_GROUP_START = 30;

    public static final int TYPE_BIPR_PUSH = TYPE_BIPR_GROUP_START + 1;

    public static final int TYPE_BIPR_GROUP_END = TYPE_BIPR_GROUP_START + 9;

    public static final int STATE_PENDING = 1; // initial: user confirmed

    public static final int STATE_REJECTING = 2; // temp: rejecting => rejected

    public static final int STATE_ABORTING = 3;// temp: aborting => aborted

    public static final int STATE_ONGOING = 4; // temp: ongoing => failure /

    // success

    public static final int STATE_REJECTED = 5; // finish - rejected

    public static final int STATE_ABORTED = 6; // finish - aborted

    public static final int STATE_FAILURE = 7; // finish - failure

    public static final int STATE_SUCCESS = 8; // finish - success

    public static final int STATE_CLEARED = 9; // cleared - cleared by user

    /**
     * Select Condition
     */
    // finished task: SUCCESS or FAILURE
    public static final String SC_FINISHED_TASK = BluetoothShareTaskMetaData.TASK_STATE + " in ("
            + BluetoothShareTask.STATE_SUCCESS + "," + BluetoothShareTask.STATE_FAILURE + ")";

    // incoming task: server-push + client-pull
    public static final String SC_INCOMING_TASK = BluetoothShareTaskMetaData.TASK_TYPE + " in ("
            + BluetoothShareTask.TYPE_OPPS_PUSH + "," + BluetoothShareTask.TYPE_OPPC_PULL + ","
            + BluetoothShareTask.TYPE_BIPR_PUSH + ")";

    // outgoing task: server-pull + client-push
    public static final String SC_OUTGOING_TASK = BluetoothShareTaskMetaData.TASK_TYPE + " in ("
            + BluetoothShareTask.TYPE_OPPC_PUSH + "," + BluetoothShareTask.TYPE_OPPS_PULL + ","
            + BluetoothShareTask.TYPE_BIPI_PUSH + ")";

    /**
     * Bluetooth Share Task Metadata
     */
    public interface BluetoothShareTaskMetaData extends BaseColumns {

        String TABLE_NAME = "share_tasks";

        Uri CONTENT_URI = Uri.parse("content://" + BluetoothShareProvider.AUTHORITY + "/" + TABLE_NAME);

        String CONTENT_TYPE = "vnd.android.cursor.dir/vnd.mtkbt.share.task";

        String CONTENT_ITEM_TYPE = "vnd.android.cursor.item/vnd.mtkbt.share.task";

        String DEFAULT_SORT_ORDER = "modified DESC";

        // metadata
        String TASK_TYPE = "type";

        String TASK_STATE = "state";

        String TASK_RESULT = "result";

        // request
        String TASK_OBJECT_NAME = "name";

        String TASK_OBJECT_URI = "uri";

        String TASK_OBJECT_FILE = "data";

        String TASK_MIMETYPE = "mime";

        String TASK_PEER_NAME = "peer_name";

        String TASK_PEER_ADDR = "peer_addr";

        // progress
        String TASK_TOTAL_BYTES = "total";

        String TASK_DONE_BYTES = "done";

        // timestamp
        String TASK_CREATION_DATE = "creation";

        String TASK_MODIFIED_DATE = "modified";

        String TASK_IS_HANDOVER = "ishandover";
    }

    public Uri getTaskUri() {

        if (this.mId == ID_NULL) {

            throw new IllegalStateException("null id task can't get uri");
        } else {
            return Uri.withAppendedPath(BluetoothShareTaskMetaData.CONTENT_URI, Integer.toString(this.mId));
        }
    }

    // metadata
    private int mId = ID_NULL;

    private int mType;

    private int mState;

    private String mResult;

    private String mData;

    private boolean mIsHandover;

    // request
    private String mObjectName;

    private String mObjectUri;

    private String mMimeType;

    private String mPeerName;

    private String mPeerAddr;

    // progress
    private long mTotalBytes;

    private long mDoneBytes;

    // timestamp
    private long mCreationDate = 0;

    private long mModifiedDate = 0;

    public BluetoothShareTask(int type) {

        this.mType = type;
    }

    public BluetoothShareTask(Cursor cursor) {

        this.mId = cursor.getInt(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData._ID));
        this.mType = cursor.getInt(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_TYPE));
        this.mState = cursor.getInt(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_STATE));
        this.mResult = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_RESULT));

        this.mObjectName = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_NAME));
        this.mObjectUri = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_URI));
        this.mData = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_OBJECT_FILE));
        this.mMimeType = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_MIMETYPE));
        this.mPeerName = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_NAME));
        this.mPeerAddr = cursor.getString(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_PEER_ADDR));

        this.mTotalBytes = cursor.getLong(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES));
        this.mDoneBytes = cursor.getLong(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_DONE_BYTES));

        this.mCreationDate = cursor.getLong(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_CREATION_DATE));
        this.mModifiedDate = cursor.getLong(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE));
        this.mIsHandover = (cursor.getLong(cursor.getColumnIndexOrThrow(BluetoothShareTaskMetaData.TASK_IS_HANDOVER)) == 1);  
    }

    /**
     * create ContentValues for ContentProvider operations
     *
     * @return
     */
    public ContentValues getContentValues() {

        ContentValues values = new ContentValues();

        // existing record
        if (this.mId != ID_NULL) {
            values.put(BluetoothShareTaskMetaData._ID, this.mId);
        }
        if (this.mCreationDate != 0) {
            values.put(BluetoothShareTaskMetaData.TASK_CREATION_DATE, this.mCreationDate);
        }
        if (this.mModifiedDate != 0) {
            values.put(BluetoothShareTaskMetaData.TASK_MODIFIED_DATE, this.mModifiedDate);
        }
        values.put(BluetoothShareTaskMetaData.TASK_TYPE, this.mType);
        values.put(BluetoothShareTaskMetaData.TASK_STATE, this.mState);
        values.put(BluetoothShareTaskMetaData.TASK_RESULT, this.mResult);
        values.put(BluetoothShareTaskMetaData.TASK_IS_HANDOVER, (this.mIsHandover == true)?1:0);

        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_NAME, this.mObjectName);
        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_URI, this.mObjectUri);
        values.put(BluetoothShareTaskMetaData.TASK_OBJECT_FILE, this.mData);
        values.put(BluetoothShareTaskMetaData.TASK_MIMETYPE, this.mMimeType);
        values.put(BluetoothShareTaskMetaData.TASK_PEER_NAME, this.mPeerName);
        values.put(BluetoothShareTaskMetaData.TASK_PEER_ADDR, this.mPeerAddr);

        values.put(BluetoothShareTaskMetaData.TASK_TOTAL_BYTES, this.mTotalBytes);
        values.put(BluetoothShareTaskMetaData.TASK_DONE_BYTES, this.mDoneBytes);

        return values;
    }

    public String getPrintableString() {

        StringBuilder res = new StringBuilder();
        ContentValues cv = this.getContentValues();
        Set<Entry<String, Object>> set = cv.valueSet();
        for (Entry<String, Object> e : set) {

            res.append("[").append(e.getKey()).append("=").append(e.getValue()).append("]");
        }
        return res.toString();
    }

    public boolean isOppcTask() {

        return (TYPE_OPPC_GROUP_START < this.mType && this.mType < TYPE_OPPC_GROUP_END);
    }

    public boolean isOppsTask() {

        return (TYPE_OPPS_GROUP_START < this.mType && this.mType < TYPE_OPPS_GROUP_END);
    }

    public static enum Direction {
        in, out
    };

    public Direction getDirection() {
        switch (this.mType) {
            case BluetoothShareTask.TYPE_OPPC_PULL:
            case BluetoothShareTask.TYPE_OPPS_PUSH:
            case BluetoothShareTask.TYPE_BIPR_PUSH:
                return Direction.in; // R.drawable.bluetooth_opp_pull_anim0;
            case BluetoothShareTask.TYPE_OPPC_PUSH:
            case BluetoothShareTask.TYPE_OPPS_PULL:
            case BluetoothShareTask.TYPE_BIPI_PUSH:
                return Direction.out; // R.drawable.bluetooth_opp_push_anim0;
            default:
                return Direction.out;
        }
    }

    /**********************************************************************************************************
     * Getter / Setter
     **********************************************************************************************************/

    public boolean isHandover() {
        return mIsHandover;
    }

    public void setHandover(boolean isHandover) {
        this.mIsHandover = isHandover;
    }

    public int getState() {
        return this.mState;
    }

    public void setState(int state) {
        this.mState = state;
    }

    public String getResult() {
        return this.mResult;
    }

    public void setResult(String result) {
        this.mResult = result;
    }

    public String getObjectName() {
        return this.mObjectName;
    }

    public void setObjectName(String objectName) {
        this.mObjectName = objectName;
    }

    public String getObjectUri() {
        return this.mObjectUri;
    }

    public void setObjectUri(String objectUri) {
        this.mObjectUri = objectUri;
    }

    public String getMimeType() {
        return this.mMimeType;
    }

    public void setMimeType(String mimeType) {

        if (mimeType != null) {

            // MIME type matching in the Android framework is case-sensitive
            // (unlike formal RFC MIME types).
            // As a result, you should always specify MIME types using lowercase
            // letters.
            this.mMimeType = mimeType.toLowerCase();
        } else {
            this.mMimeType = mimeType;
        }
    }

    public String getPeerName() {
        return this.mPeerName;
    }

    public void setPeerName(String peerName) {
        this.mPeerName = peerName;
    }

    public String getPeerAddr() {
        return this.mPeerAddr;
    }

    public void setPeerAddr(String peerAddr) {
        this.mPeerAddr = peerAddr;
    }

    public long getTotalBytes() {
        return this.mTotalBytes;
    }

    public void setTotalBytes(long totalBytes) {
        this.mTotalBytes = totalBytes;
    }

    public long getDoneBytes() {
        return this.mDoneBytes;
    }

    public void setDoneBytes(long doneBytes) {
        this.mDoneBytes = doneBytes;
    }

    public long getCreationDate() {
        return this.mCreationDate;
    }

    public void setCreationDate(long creationDate) {
        this.mCreationDate = creationDate;
    }

    public long getModifiedDate() {
        return this.mModifiedDate;
    }

    public void setModifiedDate(long modifiedDate) {
        this.mModifiedDate = modifiedDate;
    }

    public int getId() {
        return this.mId;
    }

    public void setId(int id) {
        this.mId = id;
    }

    public int getType() {
        return this.mType;
    }

    public String getData() {
        return this.mData;
    }

    public void setData(String data) {
        this.mData = data;
    }
}
