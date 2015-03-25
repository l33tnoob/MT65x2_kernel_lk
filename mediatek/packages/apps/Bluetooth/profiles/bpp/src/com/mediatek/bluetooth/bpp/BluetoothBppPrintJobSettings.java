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

package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;

import android.os.Bundle;

import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;


import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.preference.ListPreference;

import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import android.app.Activity;
import android.app.Dialog;


import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;


public class BluetoothBppPrintJobSettings
            extends PreferenceActivity 
            implements OnSharedPreferenceChangeListener,
                       View.OnClickListener,
                       CopiesPickerDialog.OnCopiesSetListener {


    private static final String TAG = "BluetoothBppPrintJobSettings";

    public static final String EXTRA_PAPER_SIZE = " com.mediatek.bluetooth.bppprintjobsettings.extra.papersize";
    public static final String EXTRA_NUMBER_COPIES = " com.mediatek.bluetooth.bppprintjobsettings.extra.copies";
    public static final String EXTRA_SIDES_SETTING = " com.mediatek.bluetooth.bppprintjobsettings.extra.sides";
    public static final String EXTRA_SHEET_SETTING = " com.mediatek.bluetooth.bppprintjobsettings.extra.pagespersheet";
    public static final String EXTRA_ORIENTATION_SETTING = " com.mediatek.bluetooth.bppprintjobsettings.extra.orientation";
    public static final String EXTRA_QUALITY_SETTING = " com.mediatek.bluetooth.bppprintjobsettings.extra.quality";

    public static final String EXTRA_FILE_NAME = " com.mediatek.bluetooth.bppprintjobsettings.extra.filename";


    public static final String EXTRA_EXCEPTION = " com.mediatek.bluetooth.bppprintjobsettings.extra.exception";

    public static final String ACTION_ATTR_UPDATE = "com.mediatek.bluetooth.bppprinting.action.ATTR_UPDATE";



    private static final String KEY_PAPER_SIZE = "paper_size_setting";
    private static final String KEY_NUMBER_COPIES = "number_of_copies_setting";
    private static final String KEY_SIDES_SETTING = "sides_setting";
    private static final String KEY_SHEET_SETTING = "pages_per_sheet_setting";
    private static final String KEY_ORIENTATION_SETTING = "orientation_setting";
    private static final String KEY_QUALITY_SETTING = "quality_setting";


    private static final int REQUEST_PRINT_PROCESSING_RESULT = 1981;
    private boolean retrunFromPrintingDialog = false;

    private ListPreference mPaperSize;
    private Preference mNumberOfCopies;
    private ListPreference mSidesSetting;
    private ListPreference mPagesPerSheet;
    private ListPreference mOrientation;
    private ListPreference mQualitySetting;

    private static final int DIALOG_COPIESPICKER = 0;


    private String mFileName;
    private int mMaxCopies;
    private int mCopies = 1;

    private Button mGetDefaultValue;
    private Button mPrint;


    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Xlog.v(TAG, "onReceive");

            String action = intent.getAction();

            if (action.equals(ACTION_ATTR_UPDATE)) {
               if ( -1 == intent.getIntExtra(EXTRA_EXCEPTION, 1) )
               {
                   Xlog.v(TAG, "Exception");
                   BluetoothBppPrintJobSettings.this.finish();
               }
               else
               {
                   Xlog.v(TAG, "updateAttr");
                   updateAttr();
               }
            }
        }
    };



    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Xlog.v(TAG, "onCreate......");


        setContentView(R.layout.bt_bpp_printer_attributes);
        addPreferencesFromResource(R.xml.bt_bpp_printer_attributes_pref);


        Intent intent = getIntent();
        String [] paperSize = intent.getStringArrayExtra( EXTRA_PAPER_SIZE );
        String [] sidesSetting = intent.getStringArrayExtra( EXTRA_SIDES_SETTING );
        String [] paperPerSheet = intent.getStringArrayExtra( EXTRA_SHEET_SETTING );
        String [] orientation = intent.getStringArrayExtra( EXTRA_ORIENTATION_SETTING );
        String [] qualitySetting = intent.getStringArrayExtra( EXTRA_QUALITY_SETTING );

        mMaxCopies =  intent.getIntExtra(EXTRA_NUMBER_COPIES, 1);
        mFileName = intent.getStringExtra(EXTRA_FILE_NAME);

 //       String [] paperSize = {"4 x 6", "A3", "A4"};
 //       String [] sidesSetting = {"One-sided", "Two-sided-long-edge", "Two-sided_short-dege"};
 //       String [] paperPerSheet = {"1", "2", "4", "8"};
 //       String [] orientation = {"Portrait", "Landscape", "reverse-portrait", "reverse-landscape"};
 //       String [] qualitySetting = {"Draft", "Normal", "High"};

        mPaperSize = (ListPreference)findPreference(KEY_PAPER_SIZE);
        if ( null == mPaperSize ) {
            Xlog.e(TAG, "PaperSize preference is null");
        }
        else {
            mPaperSize.setEntries(paperSize);
            mPaperSize.setEntryValues(paperSize);
        }


        mNumberOfCopies = findPreference(KEY_NUMBER_COPIES);
        if ( null == mNumberOfCopies ) {
            Xlog.e(TAG, "NumberOfCopies preference is null");
        }
        else { 
            if ( 1 == mMaxCopies ) {
                mNumberOfCopies.setEnabled(false);
            }
            else {
                mNumberOfCopies.setEnabled(true);
            }
            mNumberOfCopies.setSummary("1");
        }


        mSidesSetting = (ListPreference)findPreference(KEY_SIDES_SETTING);
        if ( null == mSidesSetting ) {
            Xlog.e(TAG, "SideSetting preference is null");
        }
        else {
            mSidesSetting.setEntries(sidesSetting);
            mSidesSetting.setEntryValues(sidesSetting);
        }


        mPagesPerSheet = (ListPreference)findPreference(KEY_SHEET_SETTING);
        if ( null == mPagesPerSheet ) {
            Xlog.e(TAG, "PagesPerSheet preference is null");
        }
        else {
            mPagesPerSheet.setEntries(paperPerSheet);
            mPagesPerSheet.setEntryValues(paperPerSheet);
        }

     
        mOrientation = (ListPreference)findPreference(KEY_ORIENTATION_SETTING);
        if ( null == mOrientation ) {
            Xlog.e(TAG, "Orientation preference is null");
        }
        else {
            mOrientation.setEntries(orientation);
            mOrientation.setEntryValues(orientation);
        }


        mQualitySetting = (ListPreference)findPreference(KEY_QUALITY_SETTING);
        if ( null == mQualitySetting ) {
            Xlog.e(TAG, "QualitySetting preference is null");
        }
        else {
            mQualitySetting.setEntries(qualitySetting);
            mQualitySetting.setEntryValues(qualitySetting);
        }


        mGetDefaultValue = (Button) findViewById(R.id.get_default_value_button);
        if ( null == mGetDefaultValue ) {
            Xlog.e(TAG, "Get Default Value button is null");
        }
        else {
            mGetDefaultValue.setOnClickListener(this);
        }  


        mPrint = (Button) findViewById(R.id.print_button);
        if ( null == mPrint ) {
            Xlog.e(TAG, "Print button is null");
        }
        else {
            mPrint.setOnClickListener(this);
        }


        registerReceiver(mReceiver, new IntentFilter(ACTION_ATTR_UPDATE));
    }



    @Override
    protected void onStart() {
        Xlog.v(TAG, "onStart()");
        super.onStart();
        //getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);

        retrunFromPrintingDialog = false; 
        updateAttr();
    }

    @Override
    protected void onResume() {
        Xlog.v(TAG, "onResume()");
        super.onResume();
        //getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
        SharedPreferences printSettings = getPreferenceScreen().getSharedPreferences();
        if ( null == printSettings ) {
            Xlog.v(TAG, "share preferences is null");
        }
        else {
            printSettings.registerOnSharedPreferenceChangeListener(this);
        }
    }

    @Override
    protected void onPause() {
        Xlog.v(TAG, "onPause()");
        super.onPause();
        //getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
        SharedPreferences printSettings = getPreferenceScreen().getSharedPreferences();
        if ( null == printSettings ) {
            Xlog.v(TAG, "share preferences is null");
        }
        else {
            printSettings.unregisterOnSharedPreferenceChangeListener(this);
        }


        if (retrunFromPrintingDialog == false) {
            Intent intent = new Intent(this, BluetoothBppManager.class);
            intent.putExtra("action", BluetoothBppManager.ACTION_CANCEL);
            startService(intent);
        }
        finish();  
    }

    @Override
    protected void onStop() {

        Xlog.v(TAG, "onStop()");
/*
        if (retrunFromPrintingDialog == false) {
            Intent intent = new Intent(this, BluetoothBppManager.class);
            intent.putExtra("action", BluetoothBppManager.ACTION_CANCEL);
            startService(intent);
        }
*/
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "onDestroy()");
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    private void updateAttr() {
        mPaperSize.setValueIndex(0);
        mPaperSize.setSummary(mPaperSize.getValue());
        mSidesSetting.setValueIndex(0);
        mSidesSetting.setSummary(mSidesSetting.getValue());
        mPagesPerSheet.setValueIndex(0);
        mPagesPerSheet.setSummary(mPagesPerSheet.getValue());
        mOrientation.setValueIndex(0);
        mOrientation.setSummary(mOrientation.getValue());
        mQualitySetting.setValueIndex(0);
        mQualitySetting.setSummary(mQualitySetting.getValue());

        mCopies = 1;
        mNumberOfCopies.setSummary("1");
    }



    
    public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {

        Xlog.v(TAG, "onSharePreferenceChanged......");

        mPaperSize.setSummary(mPaperSize.getValue());
        //mNumberOfCopies;
        mSidesSetting.setSummary(mSidesSetting.getValue());
        mPagesPerSheet.setSummary(mPagesPerSheet.getValue());
        mOrientation.setSummary(mOrientation.getValue());
        mQualitySetting.setSummary(mQualitySetting.getValue());

    }


    public void onCopiesSet(int copies){
        mCopies = copies;
        mNumberOfCopies.setSummary(Integer.toString(mCopies));
    }


    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (preference == mNumberOfCopies) {
            showDialog(DIALOG_COPIESPICKER);
        }
        return false;
    }


    @Override
    public Dialog onCreateDialog(int id) {
        Dialog d;

        switch (id) {
        case DIALOG_COPIESPICKER: {
            d = new CopiesPickerDialog( this, this, mMaxCopies);
            break;
        }
        default:
            d = null;
            break;
        }
        return d;
    }



    @Override
    public void onPrepareDialog(int id, Dialog d) {
        switch (id) {
        case DIALOG_COPIESPICKER: {

            CopiesPickerDialog cp = (CopiesPickerDialog)d;
            cp.updateCopies(mCopies);


            break;
        }
        default:
            break;
        }
    }


    public void onClick(View v) {
        if (v == mGetDefaultValue) {
             Xlog.v(TAG, "Start Bpp Manager to change printer !");

             Intent intent = new Intent(this, BluetoothBppManager.class);
             intent.putExtra("action", BluetoothBppManager.ACTION_GET_DEFAULT_VALUE);

             this.startService(intent);
        } else if ( v == mPrint ) {
             Xlog.v(TAG, "Start Bpp Manager to print !");

             Intent intent = new Intent(this, BluetoothBppManager.class);
             intent.putExtra("action", BluetoothBppManager.ACTION_PRINT);

             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_PAPER_SIZE ,mPaperSize.getValue());
             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_NUMBER_COPIES, mCopies);
             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_SIDES_SETTING, mSidesSetting.getValue());
             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_SHEET_SETTING, mPagesPerSheet.getValue());
             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_ORIENTATION_SETTING, mOrientation.getValue());
             intent.putExtra( BluetoothBppPrintJobSettings.EXTRA_QUALITY_SETTING, mQualitySetting.getValue() );

             startService(intent);

             intent = new Intent(this, BluetoothBppPrinting.class);
             intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
             intent.setFlags(Intent.FLAG_ACTIVITY_NO_HISTORY);
             intent.putExtra("action", BluetoothBppPrinting.ACTION_PRINTING_UPDATE);
             intent.putExtra(BluetoothBppPrinting.EXTRA_PERCENTAGE, 0 );
             intent.putExtra(BluetoothBppPrinting.EXTRA_DIALOG_TYPE, BluetoothBppPrinting.DIALOG_PRINT_PROCESSING_INIT);
             intent.putExtra(BluetoothBppPrinting.EXTRA_FILE_NAME, mFileName );
             intent.putExtra(BluetoothBppPrinting.EXTRA_REASON, getString(R.string.reason_nondefine) );
             //temp release
             retrunFromPrintingDialog = true;
             //temp release end

             startActivityForResult(intent, REQUEST_PRINT_PROCESSING_RESULT);

        }
    }


    @Override
    protected void onActivityResult( int requestCode, int resultCode, Intent data ){
        super.onActivityResult( requestCode, resultCode, data );
        Xlog.v(TAG, "onActivityResult !");


        if( requestCode == REQUEST_PRINT_PROCESSING_RESULT){
            retrunFromPrintingDialog = true;
            if( resultCode == RESULT_OK ) {

                Xlog.v(TAG, "Result_OK");

            }
            else if( resultCode == BluetoothBppPrinting.RESULT_HIDE ) {
             
                Xlog.v(TAG, "Result_HIDE");
              //home key press
               

            }
            else if( resultCode == BluetoothBppPrinting.RESULT_CANCEL ) {
              //temp release
              //retrunFromPrintingDialog = false;
                Xlog.v(TAG, "Result_CANCEL");
              //  Intent intent = new Intent(this, BluetoothBppManager.class);
              //  intent.putExtra("action", BluetoothBppManager.ACTION_CANCEL);
              //  startService(intent);
            }
            finish();
        }
    }
}

