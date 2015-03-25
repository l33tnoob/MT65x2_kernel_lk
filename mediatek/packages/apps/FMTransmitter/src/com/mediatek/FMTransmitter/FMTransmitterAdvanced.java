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

package com.mediatek.FMTransmitter;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;

import com.mediatek.common.featureoption.FeatureOption;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Locale;

public class FMTransmitterAdvanced extends Activity {
    public static final String TAG = "FMTx/FMTransmitterAdvanced";
    public static final String ACTIVITY_RESULT = "ACTIVITY_RESULT";

    private FMTransmitterActivity mRefMainActivity = null;
    
    private static final String LV_COLUMN_STATION_TYPE = "STATION_TYPE";
    private static final String LV_COLUMN_STATION_NAME = "STATION_NAME";
    private static final String LV_COLUMN_STATION_FREQ = "STATION_FREQ";
    private static final String LV_COLUMN_VALUE_FREQ = "STATION_FREQ_VALUE"; // Save the frequency value into hash map.
    private static final int LV_CAPACITY = 1024;
    
    private static final String CURRENTSTATION = "currentstation";
    
    // The max count of characters limited in edit text.50khz is 6
    private static final int MAX_FREQUENCY_LENGTH = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 6 : 5;

    // Search parameters.
    private static final int SEARCH_CHANNEL_COUNT = 10;
    private static final int SEARCH_CHANNEL_DIRECTION = 0;
    private static final int SEARCH_CHANNEL_GAP = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 5 : 1;
    
    
    public int mCurrentStation = FMTransmitterStation.FIXED_STATION_FREQ; // 100.0 MHz
    
    private boolean mIsDestroying = false;
    private static boolean sInSearching = false;
    //private static final String TYPE_MSGID = "MSGID";
    //private static final int MSGID_SEARCH_FINISH = 1;

    private EditText mEditTextFrequency = null;
    private Button mButtonOK = null;
    private Button mButtonSearch = null;
    
    private ListView mListviewChannels = null;
    private SimpleAdapter mSimpleAdapter = null;
    private ArrayList<HashMap<String, Object>> mListStations = null;
    private int mStationCount = 0; // Record how many stations displayed in list view.
    private static final int BASE_NUMBER = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 100 : 10;
    private Context mContext;
    private Toast mToast = null;
    
    SearchAsyncTask mSearchAsyncTask = null;
    
    private static FragmentManager sFragmentManager;
    
    private static final String FMTARNSMITTERADVANECD_FRAGMENT_TAG = "456dialog";
    
    public void onCreate(Bundle savedInstanceState) {
        FMTxLogUtils.d(TAG, ">>> FMTransmitterAdvanced.onCreat");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.advanced);
        
        sFragmentManager = getFragmentManager();
        mContext = getApplicationContext();
        mRefMainActivity = FMTransmitterActivity.sRefThis;
        initSimpleAdapter();
        initListView();
        mListviewChannels.setOnItemClickListener(
            new AdapterView.OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    // Set the selected frequency to main UI and finish the advanced UI.
                    HashMap<String, Object> hashmap = mListStations.get(position);
                    if (null != hashmap.get(LV_COLUMN_VALUE_FREQ)) {
                        int iStation = (Integer)hashmap.get(LV_COLUMN_VALUE_FREQ);
                        Intent intentResult = new Intent();
                        intentResult.putExtra(ACTIVITY_RESULT, iStation);
                        setResult(RESULT_OK, intentResult);
                        finish();
                    } 
                }
            }
        );

        mEditTextFrequency = (EditText)findViewById(R.id.edittext_frequency);
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            // ToDo add can only input two decimal filter
            mEditTextFrequency.setFilters(new InputFilter[]{mFilter50Khz,new InputFilter.LengthFilter(
                    MAX_FREQUENCY_LENGTH)});
        } else {
            mEditTextFrequency.setFilters(new InputFilter[]{new InputFilter.LengthFilter(MAX_FREQUENCY_LENGTH)});
        }
        Intent intent = getIntent();
        mCurrentStation = intent.getIntExtra(CURRENTSTATION, FMTransmitterStation.FIXED_STATION_FREQ);
        mEditTextFrequency.setText(formatStation(mCurrentStation));
        mEditTextFrequency.addTextChangedListener(
            new TextWatcher() {
                public void afterTextChanged(Editable s) {
                    FMTxLogUtils.d(TAG, ">>> afterTextChanged");
                    boolean bEnableOKButton = false;
                    CharSequence cs = mEditTextFrequency.getText();
                    int iFreq = getIntFromEditText();
                    // Cannot be null text; text length bigger than 1; the first char is not 0.
                    if (null != cs && cs.length() > 1 && '0' != cs.charAt(0)) {
                        if (iFreq >= FMTransmitterStation.LOWEST_STATION && iFreq <= FMTransmitterStation.HIGHEST_STATION) {
                            bEnableOKButton = true;
                        }
                    }
                    mButtonOK.setEnabled(bEnableOKButton);
                    FMTxLogUtils.d(TAG, "<<< afterTextChanged");
                }
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                    //do noting
                }
                
                public void onTextChanged(CharSequence s, int start, int before, int count) {
                    //do nothing
                }
            }
        );
        mButtonOK = (Button)findViewById(R.id.button_ok);
        mButtonOK.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    FMTxLogUtils.d(TAG, ">>> onClick OK");
                    int iFreq = getIntFromEditText();
                    if (iFreq >= FMTransmitterStation.LOWEST_STATION && iFreq <= FMTransmitterStation.HIGHEST_STATION) {
                        Intent intentResult = new Intent();
                        intentResult.putExtra(ACTIVITY_RESULT, iFreq);
                        setResult(RESULT_OK, intentResult);
                        finish();
                    }
                    FMTxLogUtils.d(TAG, "<<< onClick OK");
                }
            }
        );
        mButtonSearch = (Button)findViewById(R.id.button_search);
        mButtonSearch.setOnClickListener(
            new View.OnClickListener() {
                public void onClick(View v) {
                    handleSearchClick();
                }
            }
        );
       // registerHandler();
        FMTxLogUtils.d(TAG, "<<< FMTransmitterAdvanced.onCreat");
    }
    
    private void handleSearchClick() {
        if (sInSearching) {
            FMTxLogUtils.e(TAG, "Error: already searching.");
        } else {
            if (null != mSearchAsyncTask && !mSearchAsyncTask.isCancelled()) {
                mSearchAsyncTask.cancel(true);
                mSearchAsyncTask = null;
            }
            mSearchAsyncTask = new SearchAsyncTask();
            mSearchAsyncTask.execute();
        }
    }

    private void initSimpleAdapter() {
        mListStations = new ArrayList<HashMap<String, Object>>(LV_CAPACITY);
        mSimpleAdapter = new SimpleAdapter(
            this,
            mListStations,
            R.layout.simpleadapter,
            new String[]{LV_COLUMN_STATION_TYPE, LV_COLUMN_STATION_FREQ, LV_COLUMN_STATION_NAME},
            new int[]{R.id.lv_station_type, R.id.lv_station_freq, R.id.lv_station_name}
        );
        mSimpleAdapter.setViewBinder(
            new SimpleAdapter.ViewBinder() {
                
                public boolean setViewValue(View view, Object data,
                        String textRepresentation) {
                    if (view instanceof ImageView) {
                        ((ImageView)view).setBackgroundResource(0);
                        // If the station is a favorite station, set its icon;
                        // otherwise, it does not have an icon.
                        // mark the code for not used favorite station type
                        if (FMTransmitterStation.STATION_TYPE_FAVORITE == (Integer)data) {
                            ((ImageView)view).setImageResource(R.drawable.btn_disable);
                        } else {
                            ((ImageView)view).setImageResource(0);
                        }
                        
                        return true;
                    }
                    return false;
                }
            }
        );
        mListviewChannels = (ListView)findViewById(R.id.station_list);
        mListviewChannels.setAdapter(mSimpleAdapter);
    }
        
    InputFilter mFilter50Khz = new InputFilter() {

        public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                int dstart, int dend) {
            if ("".equals(source.toString())) {
                return null;
            }
            // according the point divide string 
            String[] splitArray = dest.toString().split("\\.");
            // input have point, should delete the redundant
            if (splitArray.length > 1) {
                String fraction = splitArray[1];
                int deleteIndex = fraction.length() - 1;
                if (deleteIndex > 0) {
                    int dotIndex = dest.toString().indexOf(".") + 1;
                    if (dstart > dotIndex) {
                    return source.subSequence(start, end - deleteIndex); 
                    } else {
                        return dest.subSequence(dstart,dend) + source.toString();
                    }
                }
            }
            return null;

        }
    };
    
    private void initListView() {
        HashMap<String, Object> hashmap = null;
        // Display all the stations in the data base.
        Uri uri = FMTransmitterStation.Station.CONTENT_URI;
        Cursor cur = managedQuery(uri, FMTransmitterStation.COLUMNS, null, null, null);
        if (null != cur) {
            // Add searched stations into list.
            cur.moveToFirst();
            while (!cur.isAfterLast()) {
                int iType = cur.getInt(cur.getColumnIndex(FMTransmitterStation.Station.COLUMN_STATION_TYPE));
                if (FMTransmitterStation.STATION_TYPE_SEARCHED == iType) {
                    String sStationName = "";
                    int iStation = cur.getInt(cur.getColumnIndex(FMTransmitterStation.Station.COLUMN_STATION_FREQ));
                    String sStationFreq = null;
                    sStationFreq = formatStation(iStation);
//                    if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
//                        sStationFreq = String.format(Locale.ENGLISH, "%.2f",  (float)iStation/BASE_NUMBER);
//                        
//                    } else {
//                        sStationFreq = String.format(Locale.ENGLISH, "%.1f",  (float)iStation/BASE_NUMBER);
//                    }
                    hashmap = new HashMap<String, Object>();
                    hashmap.put(LV_COLUMN_STATION_TYPE, iType);
                    hashmap.put(LV_COLUMN_STATION_FREQ, sStationFreq);
                    hashmap.put(LV_COLUMN_STATION_NAME, sStationName);
                    hashmap.put(LV_COLUMN_VALUE_FREQ, iStation);
                    mListStations.add(hashmap);
                    mStationCount++;
                }
                
                cur.moveToNext();
            }
            
        }
        
        mSimpleAdapter.notifyDataSetChanged();
    }
    

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {        
        super.onRestoreInstanceState(savedInstanceState);
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onRestoreInstanceState()"); 
        if (!sInSearching) {
            if (sFragmentManager != null) {
                FMTxLogUtils.d(TAG, "sFragmentManager != null in onSaveInstanceState()");
                DialogFragment prefragdialog = (DialogFragment)sFragmentManager.findFragmentByTag(
                        FMTARNSMITTERADVANECD_FRAGMENT_TAG);
                if (prefragdialog != null) {                    
                    prefragdialog.dismissAllowingStateLoss();                    
                }
            } else {
                FMTxLogUtils.w(TAG, "sFragmentManager == null in onRestoreInstanceState()");
            }   
        }
    }

    public void onStart() {
        super.onStart();
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onStart");
    }

    public void onResume() {
        super.onResume();
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onResume");
    }

    public void onPause() {
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onPause");
        super.onPause();
    }

    public void onStop() {
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onStop");      
        
        super.onStop();       
       
    }

    public void onDestroy() {
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onDestroy");
        mIsDestroying = true;     
        sFragmentManager = null;        
        super.onDestroy();
    }

    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        FMTxLogUtils.d(TAG, "FMTransmitterAdvanced.onConfigurationChanged");
    }

    private int getIntFromEditText() {
        FMTxLogUtils.v(TAG, ">>> FMTransmitterAdvanced.getIntFromEditText");
        int iRet = 0;
        int iTemp = 0;
        CharSequence cs = mEditTextFrequency.getText();
        FMTxLogUtils.v(TAG, "The string in edit text is: " + cs);
        if (null != cs && cs.length() > 0 && (cs.charAt(0) != '.')) {
            float fFreq = Float.parseFloat(cs.toString());
            FMTxLogUtils.v(TAG, "Convert string to float is: " + fFreq);
            if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
                if ((fFreq * BASE_NUMBER) % 5 == 0) {
                    iRet = (int)(fFreq * BASE_NUMBER);
                } else {
                    iTemp = (int)(fFreq * BASE_NUMBER);
                    // Toast user that the input frequency is wrong,if the user input frequency 
                    // is not between the valid frequency
                    if (iTemp >= FMTransmitterStation.LOWEST_STATION && 
                            iTemp <= FMTransmitterStation.HIGHEST_STATION) {
                            showToast();
                    }
                }
            } else {
                iRet = (int)(fFreq * BASE_NUMBER);
            }
        }
        FMTxLogUtils.v(TAG, "<<< FMTransmitterAdvanced.getIntFromEditText: " + iRet);
        return iRet;
    }
    
    private class SearchAsyncTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            FMTxLogUtils.d(TAG, "onPreExecute() called!");
            if (sFragmentManager != null) {
                FMTxLogUtils.d(TAG, "sFragmentManager != null in onPreExecute()");
                DialogFragment newFragment = ProgressDialogFragment.newInstance();
                newFragment.setCancelable(false);
                newFragment.show(sFragmentManager,FMTARNSMITTERADVANECD_FRAGMENT_TAG);
                sFragmentManager.executePendingTransactions(); 
            }
        }

        @Override
        protected Void doInBackground(Void... arg0) {
            sInSearching = true;
            if (null != mRefMainActivity) {
                if (!mRefMainActivity.isTxPowerUp()) {
                    mRefMainActivity.powerUpTx((float)mRefMainActivity.mCurrentStation / BASE_NUMBER);
                }

                if (mRefMainActivity.isTxPowerUp()) {
                    // Clean the searched stations in database.
                    FMTransmitterStation.cleanSearchedStations(mContext);
                    
                    // Search BASE_NUMBER unoccupied channels.
                    float[] channels1 = null;
                    float[] channels2 = null;
                    float[] channels = null;
                    int count1 = 0;
                    int count2 = 0;

                    int iStation = mRefMainActivity.mCurrentStation + SEARCH_CHANNEL_GAP;
                    if (iStation > FMTransmitterStation.HIGHEST_STATION) {
                        iStation = FMTransmitterStation.HIGHEST_STATION;
                    }
                    channels1 = mRefMainActivity.searchChannelsForTx(
                            (float)iStation / BASE_NUMBER, SEARCH_CHANNEL_DIRECTION, SEARCH_CHANNEL_COUNT);
                    if (null != channels1) {
                        count1 = channels1.length;
                    }
                    FMTxLogUtils.v(TAG, "Search out channel count in first time: " + count1);

                    if (count1 < SEARCH_CHANNEL_COUNT) {
                        FMTxLogUtils.v(TAG, "Search again from the lowest frequency");
                        channels2 = mRefMainActivity.searchChannelsForTx(
                                (float)FMTransmitterStation.LOWEST_STATION / BASE_NUMBER, 
                                SEARCH_CHANNEL_DIRECTION, SEARCH_CHANNEL_COUNT - count1);

                        if (null != channels2) {
                            count2 = channels2.length;
                        }
                        FMTxLogUtils.v(TAG, "Search out channel count in second time: " + count2);
                    }

                    if (null == channels2) {
                        channels = channels1;
                    } else if (null == channels1) {
                        channels = channels2;
                    } else {
                        // Add the channels2 to channels1
                        channels = new float[count1 + count2];
                        for (int i = 0; i < channels.length; i++) {
                            if (i < count1) {
                                channels[i] = channels1[i];
                            } else {
                                channels[i] = channels2[i - count1];
                            }
                        }
                    }

                    if (null != channels) {
                        FMTxLogUtils.v(TAG, "Search out channel count: " + channels.length);

                        // Save the searched stations into data base.
                        for (int i = 0; i < channels.length; i++) {
                            FMTxLogUtils.v(TAG, "Channels found " + i + ": " + channels[i]);
                            int iChannel = (int)(channels[i] * BASE_NUMBER);
                            if (iChannel >= FMTransmitterStation.HIGHEST_STATION
                                ||  iChannel <= FMTransmitterStation.LOWEST_STATION) {
                                FMTxLogUtils.v(TAG, "Ignore the invalid channel.");
                            } else {
                                FMTransmitterStation.insertStationToDB(
                                    mContext,
                                    "Unoccupied channel",
                                    iChannel,
                                    FMTransmitterStation.STATION_TYPE_SEARCHED
                                );
                            }
                        }
                        
                    }
                }
            }
            sInSearching = false;
            return null;
        }
        @Override
        protected void onPostExecute(Void result) {           
            FMTxLogUtils.d(TAG, "onPostExecute() called!");
            if (sFragmentManager != null) {
                FMTxLogUtils.d(TAG, "sFragmentManager != null in onPostExecute()");
                DialogFragment prefragdialog = (DialogFragment)sFragmentManager.findFragmentByTag(
                        FMTARNSMITTERADVANECD_FRAGMENT_TAG);
                if (prefragdialog != null) {
                    prefragdialog.dismissAllowingStateLoss();
                }
            } else {
                FMTxLogUtils.w(TAG, "sFragmentManager == null in onPostExecute()");
            }            
            
            if (mIsDestroying) {
                FMTxLogUtils.w(TAG, "Warning: app is being destroyed.");
                return;
            }

            mListStations.clear();
            mStationCount = 0;
            initListView();
        }        

    }
    private void showToast() {
        
        if (mToast == null) {
          mToast = Toast.makeText(mContext, getString(R.string.toast_input_error), Toast.LENGTH_SHORT); 
        }
        mToast.show();
    }
    private String formatStation(int station) {
        String result = null;
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            result = String.format(Locale.ENGLISH, "%.2f",  (float)station / BASE_NUMBER);
        } else {
            result = String.format(Locale.ENGLISH, "%.1f",  (float)station / BASE_NUMBER);
        }
        return result;
    }
}
