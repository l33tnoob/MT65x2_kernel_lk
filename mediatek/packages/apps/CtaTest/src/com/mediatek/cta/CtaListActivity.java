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

package com.mediatek.cta;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListActivity;
import android.app.LoaderManager.LoaderCallbacks;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.CursorLoader;
import android.content.Intent;
import android.content.Loader;
import android.database.Cursor;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.MediaRecorder;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.CallLog;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.Telephony;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Mms;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CursorAdapter;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.SimpleCursorAdapter;
import android.widget.SimpleCursorAdapter.ViewBinder;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import com.android.internal.telephony.ISms;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.cta.R;
import com.mediatek.cta.camera.Camera;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Date;
import java.text.SimpleDateFormat;

public class CtaListActivity extends ListActivity implements LoaderCallbacks<Cursor> {
    private static final String TAG = "CTA";
    public static final String ID = "id";
    public static final int ID_CONTACTS = 0;
    public static final int ID_CALL_LOG = 1;
    public static final int ID_SMS = 2;
    public static final int ID_MMS = 3;

    private int mId;
    private SimpleCursorAdapter mAdapter;

    private ViewBinder mViewBinder = new ViewBinder() {
        @Override
        public boolean setViewValue(View view, Cursor cursor, int columnIndex) {
            switch (mId) {
            case ID_CALL_LOG:
                if (cursor.getColumnIndex(CallLog.Calls.TYPE) == columnIndex) {
                    if (cursor.getInt(columnIndex) == 1) {
                        ((TextView) view).setText("Incoming Call");
                    } else if (cursor.getInt(columnIndex) == 2) {
                        ((TextView) view).setText("Outgoing Call");
                    } else if (cursor.getInt(columnIndex) == 3) {
                        ((TextView) view).setText("Missed Call");
                    }
                    return true;
                }
            case ID_MMS:
                if (cursor.getColumnIndex(Mms.SUBJECT) == columnIndex) {
                    try {
                        int subject_charset = cursor.getInt(cursor.getColumnIndex(Mms.SUBJECT_CHARSET));
                        String sub = cursor.getString(columnIndex);
                        if (sub != null) {
                            ((TextView) view).setText(new String(sub.getBytes("iso-8859-1"), "UTF-8"));
                        }
                    } catch (UnsupportedEncodingException e) {
                        return false;
                    }
                    return true;
                } else if (cursor.getColumnIndex(Mms.DATE) == columnIndex) {
                    long date = cursor.getInt(columnIndex) * 1000L;
                    SimpleDateFormat sf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                    ((TextView) view).setText(sf.format(new Date(date)));
                    return true;
                }
            default:
                return false;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mId = getIntent().getIntExtra(ID, ID_CONTACTS);
        switch (mId) {
        case ID_CONTACTS:
            mAdapter = new SimpleCursorAdapter(this, android.R.layout.simple_list_item_1, null,
                    new String[] {Contacts.DISPLAY_NAME},
                    new int[] {android.R.id.text1}, 0);
            break;
        case ID_CALL_LOG:
            mAdapter = new SimpleCursorAdapter(this, android.R.layout.simple_list_item_2, null,
                    new String[] {CallLog.Calls.NUMBER, CallLog.Calls.TYPE},
                    new int[] {android.R.id.text1, android.R.id.text2}, 0);
            break;
        case ID_SMS:
            mAdapter = new SimpleCursorAdapter(this, android.R.layout.simple_list_item_1, null,
                    new String[] {Sms.BODY},
                    new int[] {android.R.id.text1}, 0);
            break;
        case ID_MMS:
            mAdapter = new SimpleCursorAdapter(this, android.R.layout.simple_list_item_2, null,
                    new String[] {Mms.SUBJECT, Mms.DATE},
                    new int[] {android.R.id.text1, android.R.id.text2}, 0);
            break;
        default:
            return;
        }
        mAdapter.setViewBinder(mViewBinder);
        setListAdapter(mAdapter);
        getLoaderManager().initLoader(ID_CONTACTS, null, this);
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        Log.i(TAG, "onCreateLoader");
        String[] projection;
        switch (mId) {
        case ID_CONTACTS:
            projection = new String[] {
                    Contacts._ID,
                    Contacts.DISPLAY_NAME
            };
            return new CursorLoader(CtaListActivity.this, Contacts.CONTENT_URI, projection,
                    null, null, null);
        case ID_CALL_LOG:
            projection = new String[] {
                    CallLog.Calls._ID,
                    CallLog.Calls.NUMBER,
                    CallLog.Calls.CACHED_NAME,
                    CallLog.Calls.TYPE
            };
            return new CursorLoader(CtaListActivity.this, CallLog.Calls.CONTENT_URI, projection,
                    null, null, null);
        case ID_SMS:
            projection = new String[] {
                    Sms._ID,
                    Sms.BODY
            };
            return new CursorLoader(CtaListActivity.this, Sms.CONTENT_URI, projection,
                    null, null, null);
        case ID_MMS:
            projection = new String[] {
                    Mms._ID,
                    Mms.SUBJECT,
                    Mms.SUBJECT_CHARSET,
                    Mms.DATE
            };
            return new CursorLoader(CtaListActivity.this, Mms.CONTENT_URI, projection,
                    null, null, null);
        default:
            return null;
        }
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor data) {
        Log.i(TAG, "onLoadFinished");
        mAdapter.swapCursor(data);
    }

    @Override
    public void onLoaderReset(Loader<Cursor> loader) {
        Log.i(TAG, "onLoaderReset");
        mAdapter.swapCursor(null);
    }
}

