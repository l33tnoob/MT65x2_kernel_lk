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

package com.mediatek.engineermode.nfc;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.nfc.NfcAdapter;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcEntry extends Activity implements OnItemClickListener {

    /** Elog tag. */
    public static final String TAG = "EM/nfc";
    private static final int DIA_INIT_NFC_DRIVER = 0;
    private static final int DIA_START_ACTIVITY = 1;
    public static final String ENTRY_SETTING = "Settings";
    public static final String ENTRY_RAWDATA = "Raw Data";
    public static final String ENTRY_SOFTWARESTACK = "Software Stack";

    private List<String> mListData;
    private ListView mMenuListView = null;
    private boolean mInitNfcDrvOK = false;
    private int mInitNfcDriverRet;
    private final String mItemString[] =
        { ENTRY_SETTING, ENTRY_RAWDATA, ENTRY_SOFTWARESTACK };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.nfc_entry);
        mMenuListView = (ListView) findViewById(R.id.ListView_mainmenu);
        mMenuListView.setOnItemClickListener(this);
        Elog.i(TAG, "NfcEntry onCreate");
        /*
        closeNFCServiceAtStart();
        int ret = NfcNativeCallClass.initNfcDriver();
        if (0 == ret) {
            mInitNfcDrvOK = true;
            Elog.i(TAG, "NfcEntry initNfcDriver OK");
        } else {
            mInitNfcDrvOK = false;
            showDialog(DIA_INIT_NFC_DRIVER);
        }
        resetUIData();
        */
        resetUIData();  
    }

    @Override
    protected void onDestroy() {
        Elog.i(TAG, "NfcEntry onDestroy");
        super.onDestroy();
        if (mInitNfcDrvOK) {
            
            new Thread(new Runnable() {
                public void run() {
                    NfcNativeCallClass.deinitNfcDriver(); // maybe takes long time lead ANR , run in thread.
                }
            }).start();
            
            //NfcNativeCallClass.deinitNfcDriver(); // maybe takes long time lead
            // ANR
        }
        Elog.i(TAG, "NfcEntry onDestroy done");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.i(TAG, "NfcEntry onResume");
        //--- modify 2012/10/31 {
        closeNFCServiceAtStart();
        int ret = NfcNativeCallClass.initNfcDriver();
        if (0 == ret) {
            mInitNfcDrvOK = true;
            Elog.i(TAG, "NfcEntry initNfcDriver OK");
        } else {
            mInitNfcDrvOK = false;
            showDialog(DIA_INIT_NFC_DRIVER);
        }       
        // --- }
        mListData = getData();
        ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                mListData);
        mMenuListView.setAdapter(adapter);
    }

    private void closeNFCServiceAtStart() {
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp.isEnabled()) {
            if (adp.disable()) {
                Elog.i(TAG, "Nfc service set off.");
            } else {
                Elog.i(TAG, "Nfc service set off Fail.");
            }
        } else {
            Elog.i(TAG, "Nfc service is off");
        }
    }

    // private void openNFCServiceAtEnd() {
    // NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
    // if (!adp.isEnabled()) {
    // if (adp.enable()) {
    // Elog.i(TAG, "Nfc service set on.");
    // } else {
    // Elog.i(TAG, "Nfc service set on Fail.");
    // }
    // } else {
    // Elog.i(TAG, "Nfc service is on");
    // }
    // }

    private void resetUIData() {
        final SharedPreferences preferences =
            this.getSharedPreferences(NfcCommonDef.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        // setting always display
        final Editor editor = preferences.edit();
        editor.putBoolean(mItemString[0], true);
        editor.putBoolean(mItemString[1], false);
        editor.putBoolean(mItemString[2], false);
        editor.commit();
    }

    private List<String> getData() {
        ArrayList<String> items = new ArrayList<String>();
        final SharedPreferences preferences =
            this.getSharedPreferences(NfcCommonDef.PREFERENCE_KEY,
                android.content.Context.MODE_PRIVATE);
        // setting always display
        preferences.edit().putBoolean(mItemString[0], true).commit();
        for (int i = 0; i < mItemString.length; i++) {
            if (preferences.getBoolean(mItemString[i], false)) {
                items.add(mItemString[i]);
            }
        }
        return items;
    }

    /**
     * Click each camera setting item
     * 
     * @param arg0
     *            : selected item's adapter view
     * @param arg1
     *            : selected view
     * @param arg2
     *            : position of selected view
     * @param arg3
     *            : id of selected view
     */
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        try {
            Intent intent = new Intent();
            if (mListData.get(arg2).equalsIgnoreCase(ENTRY_SETTING)) {
                intent.setClassName(this,
                    "com.mediatek.engineermode.nfc.NfcSettings");
            } else if (mListData.get(arg2).equalsIgnoreCase(ENTRY_RAWDATA)) {
                intent.setClassName(this,
                    "com.mediatek.engineermode.nfc.NfcRawData");
            } else if (mListData.get(arg2)
                .equalsIgnoreCase(ENTRY_SOFTWARESTACK)) {
                intent.setClassName(this,
                    "com.mediatek.engineermode.nfc.NfcSoftwareStack");
            }
            this.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            showDialog(DIA_START_ACTIVITY);
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder;
        switch (id) {
        case DIA_INIT_NFC_DRIVER:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.dialog_error_title).setMessage(
                R.string.dialog_init_nfc_msg + mInitNfcDriverRet)
                .setPositiveButton(android.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        NfcEntry.this.finish();
                    }
                });
            dialog = builder.create();
            break;
        case DIA_START_ACTIVITY:
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.dialog_error_title).setMessage(
                R.string.start_activity_msg).setPositiveButton(
                android.R.string.ok, null);
            dialog = builder.create();
            break;
        default:
            break;
        }
        return dialog;
    }
}
