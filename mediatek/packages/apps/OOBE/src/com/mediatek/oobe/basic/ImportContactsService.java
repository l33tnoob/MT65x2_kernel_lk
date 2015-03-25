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
package com.mediatek.oobe.basic;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.ContentProviderOperation;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.widget.RemoteViews;

import com.mediatek.oobe.R;
import com.mediatek.oobe.ext.IOobeMiscExt;
import com.mediatek.oobe.utils.Utils;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.HashMap;

public class ImportContactsService extends IntentService {
    private static final String TAG = "OOBE.contactService";
    public static final String SIM_ID = "sim_id";
    public static final String PHONE = "Phone";
    public static final String TABLET = "Tablet";
    public static final String ACCOUNT_TYPE = "Local Phone Account";

    private static final int NOTIFICATION_ID = 0x00011;
    private static final int ID_CONTACT_ID_INDEX = 0;
    private static final int ID_DISPLAY_NAME_PRIMARY_COLUMN_INDEX = 1;
    private static final int MAX_OP_COUNT_IN_ONE_BATCH = 400;

    private NotificationManager mNotificationManager;
    private Notification mNotification;

    private Handler mHandler;
    private MyBinder mBinder;
    private boolean mCopyFlag;
    private long mSimId;
    private int mTotalCount;
    private int mCurrentIndex;
    public IOobeMiscExt mExt;

    private HashMap<Long, SimInfoPreference> mSimInfoPreMap;

    private static final String[] DATA_ALLCOLUMNS = new String[] {
        Data._ID,
        Data.MIMETYPE,
        Data.IS_PRIMARY,
        Data.IS_SUPER_PRIMARY,
        Data.DATA1,
        Data.DATA2,
        Data.DATA3,
        Data.DATA4,
        Data.DATA5,
        Data.DATA6,
        Data.DATA7,
        Data.DATA8,
        Data.DATA9,
        Data.DATA10,
        Data.DATA11,
        Data.DATA12,
        Data.DATA13,
        Data.DATA14,
        Data.DATA15,
        Data.SYNC1,
        Data.SYNC2,
        Data.SYNC3,
        Data.SYNC4,
        Data.IS_ADDITIONAL_NUMBER
    };

    public class MyBinder extends Binder {
        public ImportContactsService getService() {
            return ImportContactsService.this;
        }
    }

    public ImportContactsService() {
        super("ImportContactsService");
    }

    @Override
     public void onCreate() {
        super.onCreate();
        Xlog.d(TAG,"onCreate()");
        mBinder = new MyBinder();
        mCopyFlag = false;
        mSimId = -1;

        mExt = Utils.getOobeMiscPlugin(getBaseContext());
        mNotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        mNotification = new Notification(R.drawable.contacts_imp_prog_notification, 
            mExt.replaceSimToSimUim(getString(R.string.oobe_title_import_contacts)),
            System.currentTimeMillis());
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }

    public long getSimId() {
        return mSimId;
    }

    /*
    * set sim info list
    */
    public void setSimInfoList(HashMap<Long, SimInfoPreference> simInfoMap) {
        mSimInfoPreMap = simInfoMap;
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Xlog.d(TAG,"onHandleIntent()");
        if (mSimInfoPreMap != null && mSimInfoPreMap.size() > 0) {
            mTotalCount = getAllContactsNumber();
            mCurrentIndex = 0;
            mCopyFlag = true;
            SimInfoPreference p;
            for (Long id : mSimInfoPreMap.keySet()) {
                p = mSimInfoPreMap.get(id);
                if (p != null && p.isChecked() && !p.isFinishImporting()) {
                    mSimId = p.getSimId();
                    handleCopyRequest(p.getSimId());
                }
            }

            //send message to update UI
            if (mCopyFlag) {
                sendMessage(ImportContactsActivity.ID_COPY_ALL_END, 0, (int) mSimId);
            }
            mCopyFlag = false;
            mNotificationManager.cancel(NOTIFICATION_ID);
        }
    }

    private void handleCopyRequest(long simId) {
        Xlog.d(TAG,"onHandleStartCopy, sim id: " + simId);
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(
                Contacts.CONTENT_URI,
                new String[] {RawContacts._ID, RawContacts.DISPLAY_NAME_PRIMARY}, 
                RawContacts.INDICATE_PHONE_SIM + " = " + simId, 
                null, null);
            if (cursor != null) {
                int totalNumber = cursor.getCount();
                if (totalNumber == 0) {
                    return;
                }
                createNotificationBar(totalNumber);
                long contactId;
                final ArrayList<ContentProviderOperation> operationList = new ArrayList<ContentProviderOperation>();
                int indexInSim = 0;
                while (cursor.moveToNext() && mCopyFlag) {
                    contactId = cursor.getLong(ID_CONTACT_ID_INDEX);

                    queryAndInsert(contactId, operationList);

                    updateNotificationBar(++mCurrentIndex, mTotalCount);
                    //update progress bar
                    sendMessage(ImportContactsActivity.ID_COPYING, ++indexInSim, totalNumber);
                }
                if (operationList.size() > 0) {
                    try {
                        getContentResolver().applyBatch(ContactsContract.AUTHORITY, operationList);
                    } catch (RemoteException e) {
                        Xlog.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    } catch (OperationApplicationException e) {
                        Xlog.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                    }
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
            //send message to update UI
            int msgId = mCopyFlag ? ImportContactsActivity.ID_COPY_ONE_END : 
                        ImportContactsActivity.ID_COPY_CANCEL;
            sendMessage(msgId, 0, (int) mSimId);
        }
    }

    /*
    * cancel import contacts
    */
    public void handleCancelRequest() {
        mCopyFlag = false;
    }

    /*
    * get all contacts in the sim card whick is checked
    * @param simId sim id of the sim card
    */
    private int getAllContactsNumber() {
        int count = 0;
        if (mSimInfoPreMap != null && mSimInfoPreMap.size() > 0) {
            Cursor cursor = null;
            SimInfoPreference p;
            for (Long id : mSimInfoPreMap.keySet()) {
                p = mSimInfoPreMap.get(id);
                if (p != null && p.isChecked() && !p.isFinishImporting()) {
                    try {
                        cursor = getContentResolver().query(
                            Contacts.CONTENT_URI,
                            new String[] {RawContacts._ID, RawContacts.DISPLAY_NAME_PRIMARY}, 
                            RawContacts.INDICATE_PHONE_SIM + " = " + p.getSimId(), 
                            null, null);
                        if (cursor != null) {
                            count += cursor.getCount();
                        }
                    } finally {
                        if (cursor != null) {
                            cursor.close();
                        }
                    }
                }
            }
        }
        return count;
    }

    /*
    * create notification bar
    * @param totalNumber total number of the sim cards
    */
    private void createNotificationBar(int totalNumber) {
        RemoteViews contentView = new RemoteViews(getPackageName(), R.layout.import_progress_notification);
        contentView.setImageViewResource(R.id.download_icon, R.drawable.contacts_imp_prog_statbar);
        contentView.setTextViewText(R.id.title, 
            mExt.replaceSimToSimUim(getString(R.string.oobe_title_import_contacts)));
        mNotification.contentView = contentView;

        mNotification.contentView.setProgressBar(R.id.progress_importing, totalNumber, 0, false);
        mNotificationManager.notify(NOTIFICATION_ID, mNotification);
    }

    /*
    * update notification bar
    * @param progress number of imported contacts
    * @param totalNumber total number of the sim cards
    */
    private void updateNotificationBar(int progress, int totalNumber) {
        if (mNotification != null) {
            mNotification.contentView.setProgressBar(R.id.progress_importing, totalNumber, 
                        progress, false);
            String text = String.format(getString(R.string.oobe_note_progress_copy_contacts),
                        progress, totalNumber);
            mNotification.contentView.setTextViewText(R.id.summary, text);
            mNotificationManager.notify(NOTIFICATION_ID, mNotification);
        }
    }
    /*
    * send message to target
    * @param msgId message id
    * @param number number of imported contacts
    * @param totalNumber number of contacts in current sim card
    */
    private void sendMessage(int msgId, int number, int totalNumber) {
        if (mHandler != null) {
            Message msg = Message.obtain(mHandler, msgId, number, totalNumber);
            msg.sendToTarget();
        }
    }

    /*
    * get all information in data table by contact id
    * @param contactId contact id of the cursor
    */
    private void queryAndInsert(long contactId, ArrayList<ContentProviderOperation> operationList) {
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(Data.CONTENT_URI, 
                DATA_ALLCOLUMNS, Data.RAW_CONTACT_ID + "=? ", 
                new String[] { String.valueOf(contactId) }, null);

            int backRef = operationList.size();

            // insert basic information to raw_contacts table
            insertRawContacts(operationList);

            cursor.moveToPosition(-1);

            while (cursor.moveToNext()) {
                //do not copy group data between different account.
                String mimeType = cursor.getString(cursor.getColumnIndex(Data.MIMETYPE));
                if (GroupMembership.CONTENT_ITEM_TYPE.equals(mimeType)) {
                    Xlog.d(TAG, "GroupMembership.CONTENT_ITEM_TYPE == mimeType");
                    continue;
                }
                //insert all data of the cursor
                insertAllColumeInfo(cursor, operationList, backRef);
            }
            if (operationList.size() > MAX_OP_COUNT_IN_ONE_BATCH) {
                try {
                    getContentResolver().applyBatch(ContactsContract.AUTHORITY, operationList);
                } catch (RemoteException e) {
                    Xlog.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                } catch (OperationApplicationException e) {
                    Xlog.e(TAG, String.format("%s: %s", e.toString(), e.getMessage()));
                }
                operationList.clear();
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /*
    * insert basic information to operation list
    * @param operationList content provider operation list
    */
    private void insertRawContacts(ArrayList<ContentProviderOperation> operationList) {
        ContentProviderOperation.Builder builder = ContentProviderOperation.newInsert(RawContacts.CONTENT_URI);
        ContentValues contactvalues = new ContentValues();
        contactvalues.put(RawContacts.INDICATE_PHONE_SIM, RawContacts.INDICATE_PHONE);
        contactvalues.put(RawContacts.AGGREGATION_MODE, RawContacts.AGGREGATION_MODE_DISABLED);

        String name = getResources().getBoolean(
            com.android.internal.R.bool.preferences_prefer_dual_pane) ? TABLET : PHONE;
        contactvalues.put(RawContacts.ACCOUNT_NAME, name);
        contactvalues.put(RawContacts.ACCOUNT_TYPE, ACCOUNT_TYPE);
        builder.withValues(contactvalues);
        operationList.add(builder.build());
    }

    /*
    * insert all information to operation list
    * @param cursor queryed by contact id
    * @param operationList content provider operation list
    */
    private void insertAllColumeInfo(Cursor cursor, 
            ArrayList<ContentProviderOperation> operationList, int backRef) {
        ContentProviderOperation.Builder builder = 
                ContentProviderOperation.newInsert(RawContacts.CONTENT_URI);
        builder = ContentProviderOperation.newInsert(Data.CONTENT_URI);
        String[] columnNames = cursor.getColumnNames();
        for (int index = 1; index < columnNames.length; index++) {
            switch (cursor.getType(index)) {
                case Cursor.FIELD_TYPE_NULL:
                    // don't put anything in the content values
                    break;
                case Cursor.FIELD_TYPE_INTEGER:
                    builder.withValue(columnNames[index], cursor.getLong(index));
                    break;
                case Cursor.FIELD_TYPE_STRING:
                    builder.withValue(columnNames[index], cursor.getString(index));
                    break;
                case Cursor.FIELD_TYPE_BLOB:    
                    builder.withValue(columnNames[index], cursor.getBlob(index));
                    break;
                default:
                    throw new IllegalStateException("Invalid or unhandled data type");
            }
        }
        builder.withValueBackReference(Data.RAW_CONTACT_ID, backRef);
        operationList.add(builder.build());
    }
}
