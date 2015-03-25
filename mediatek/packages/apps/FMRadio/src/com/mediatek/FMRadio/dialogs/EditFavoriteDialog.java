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

package com.mediatek.FMRadio.dialogs;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Selection;
import android.text.Spanned;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.mediatek.FMRadio.FMRadioStation;
import com.mediatek.FMRadio.FMRadioUtils;
import com.mediatek.FMRadio.R;
import com.mediatek.common.featureoption.FeatureOption;

/**
 * provider function to edit favorite station name and frequency, 
 * caller should implement AddFavoriteListener
 *
 */
public class EditFavoriteDialog extends DialogFragment {
    private static final String STATION_NAME = "station_name";
    private static final String STATION_FREQ = "station_freq";
    private EditFavoriteListener mListener = null;
    private EditText mEditTextFrequency = null;
    
    /**
     * create edit favorite dialog instance, caller should implement edit favorite listener
     * @param stationName
     * @param stationFreq
     * @return edit favorite dialog
     */
    public static EditFavoriteDialog newInstance(String stationName, int stationFreq) {
        EditFavoriteDialog fragment = new EditFavoriteDialog();
        Bundle args = new Bundle(2);
        args.putString(STATION_NAME, stationName);
        args.putInt(STATION_FREQ, stationFreq);
        fragment.setArguments(args);
        return fragment;
    }
    
    /**
     * edit favorite listener
     *
     */
    public interface EditFavoriteListener {
        /**
         * edit favorite station name and station frequency
         */
        void editFavorite();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (EditFavoriteListener) activity;
        } catch (ClassCastException e) {
            e.printStackTrace();
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        String stationName = getArguments().getString(STATION_NAME);
        int stationFreq = getArguments().getInt(STATION_FREQ);
        View v = View.inflate(getActivity(), R.layout.editstation, null);
        EditText editTextStationName = (EditText) v.findViewById(R.id.dlg_edit_station_name_text);
        mEditTextFrequency = (EditText) v.findViewById(R.id.dlg_edit_station_freq_text);
        
        // 50KHZ will be six chars
        final int lengthOf50Khz = 6;
        final int lengthOf100Khz = 5;
        final int maxFrequencyLength = 
                FeatureOption.MTK_FM_50KHZ_SUPPORT ? lengthOf50Khz : lengthOf100Khz;
        mEditTextFrequency.setFilters(new InputFilter[] { mFilter,
                new InputFilter.LengthFilter(maxFrequencyLength) });
        if (FeatureOption.MTK_FM_50KHZ_SUPPORT) {
            mEditTextFrequency.addTextChangedListener(mWatcher50KHZ);
        } else {
            mEditTextFrequency.addTextChangedListener(mWatcher100KHZ);
        }
        
        mEditTextFrequency.setText(FMRadioUtils.formatStation(stationFreq));
        if (null == stationName || "".equals(stationName)) {
            editTextStationName.setHint(R.string.default_station_name);
        } else {
            editTextStationName.setHint(stationName);
        }
        editTextStationName.requestFocus();
        editTextStationName.requestFocusFromTouch();
        // Edit
        editTextStationName.setText(stationName);
        Editable text = editTextStationName.getText();
        Selection.setSelection(text, text.length());
        return new AlertDialog.Builder(getActivity())
                // Must call setTitle here or the title will not be displayed.
                .setTitle(getString(R.string.dlg_addedit_title_edit)).setView(v)
                .setPositiveButton(R.string.edit_frequency_overwrite_text, new DialogInterface.OnClickListener() {

                    public void onClick(DialogInterface dialog, int which) {
                        mListener.editFavorite();
                    }
                     })
                .setNegativeButton(R.string.btn_cancel, null)
                .create();
    }

    InputFilter mFilter = new InputFilter() {

        public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                int dstart, int dend) {
            final int accuracy = FeatureOption.MTK_FM_50KHZ_SUPPORT ? 2 : 1;
            
            if ("".equals(source.toString())) {
                return null;
            }
            
            // according the point divide string 
            String[] splitArray = dest.toString().split("\\.");
            // input have point, should delete the redundant
            if (splitArray.length > 1) {
                String fraction = splitArray[1];
                int deleteIndex = fraction.length() + 1 - accuracy;
                if (deleteIndex > 0) {
                    int dotIndex = dest.toString().indexOf(".") + 1;
                    if (dstart >= dotIndex) {
                        return source.subSequence(start, end - deleteIndex);
                    } else {
                        return dest.subSequence(dstart, dend) + source.toString();
                    }
                }
            }
            return null;
        }
    };
    
    // add for 100khz
    // add for overwrite frequency feature
    private TextWatcher mWatcher100KHZ = new TextWatcher() {
        
        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (null == getDialog()) {
                return;
            }
            CharSequence cs = mEditTextFrequency.getText();
            float frequency = 0;
            try {
                frequency = Float.parseFloat(cs.toString());
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
            final int convertData = 10;
            int station = Math.round(frequency * convertData); 
            Button positiveButton = ((AlertDialog)getDialog())
            .getButton(DialogInterface.BUTTON_POSITIVE);
            
            if (null != positiveButton) {
                if (FMRadioStation.isStationExistInChList(getActivity().getApplicationContext(), station)) {
                    positiveButton.setText(R.string.edit_frequency_overwrite_text);
                } else {
                    positiveButton.setText(R.string.btn_ok);
                }
            }
        }
        
        /**
         * not need to implement
         */
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }
        
        /**
         * not need to implement
         */
        public void afterTextChanged(Editable s) {
            
        }
    };
    
    
    // add for 50khz
    // add for overwrite frequency feature
    private TextWatcher mWatcher50KHZ = new TextWatcher() {
        
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            if (null == getDialog()) {
                return;
            }
            CharSequence cs = mEditTextFrequency.getText();
            float frequency = 0;
            try {
                frequency = Float.parseFloat(cs.toString());
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
            
            final int convertData = 100;
            int station = Math.round(frequency * convertData); 
            Button positiveButton = ((AlertDialog)getDialog())
                    .getButton(DialogInterface.BUTTON_POSITIVE);
            final int checkNumber = 5;
            // just the last of digital is 5 or 0
            if (0 == (station % checkNumber)) {
                // add for overwrite frequency feature
                if (null != positiveButton) {
                    if (FMRadioStation.isStationExistInChList(getActivity().getApplicationContext(), station)) {
                        positiveButton.setText(R.string.edit_frequency_overwrite_text);
                    } else {
                        positiveButton.setText(R.string.btn_ok);
                    }
                    positiveButton.setEnabled(true);
                }
                
            } else {
                if (null != positiveButton) {
                    positiveButton.setEnabled(false);
                }
                Context context = getActivity().getApplicationContext();
                Toast.makeText(context, context.getString(R.string.toast_invalid_input),
                        Toast.LENGTH_SHORT).show();
            }
        }
        /**
         * not need to implement
         */
        public void beforeTextChanged(CharSequence s, int start, int count,
                int after) {
            
        }
        
        /**
         * not need to implement
         */
        public void afterTextChanged(Editable s) {
            
            
        }
    };

}
