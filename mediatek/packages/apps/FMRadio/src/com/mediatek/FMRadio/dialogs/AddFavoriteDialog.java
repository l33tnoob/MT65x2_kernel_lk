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
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.mediatek.FMRadio.ExtensionUtils;
import com.mediatek.FMRadio.FMRadioUtils;
import com.mediatek.FMRadio.R;
import com.mediatek.FMRadio.ext.IProjectStringExt;
/**
 * provider function to add a station as favorite, 
 * caller should implement AddFavoriteListener
 *
 */
public class AddFavoriteDialog extends DialogFragment {
    private static final String STATION_NAME = "station_name";
    private static final String STATION_FREQ = "station_freq";
    private AddFavoriteListener mListener = null;
    
    /**
     * create add favorite dialog instance according station name and station frequency
     * @param stationName station name
     * @param stationFreq station frequency
     * @return add favorite dialog instance
     */
    public static AddFavoriteDialog newInstance(String stationName, int stationFreq) {
        AddFavoriteDialog fragment = new AddFavoriteDialog();
        Bundle args = new Bundle(2);
        args.putString(STATION_NAME, stationName);
        args.putInt(STATION_FREQ, stationFreq);
        fragment.setArguments(args);
        return fragment;
    }
    
    /**
     * add favorite listener, caller should implement 
     *
     */
    public interface AddFavoriteListener {
        /**
         * add a station as favorite 
         */
        void addFavorite();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (AddFavoriteListener) activity;
        } catch (ClassCastException e) {
            e.printStackTrace();
        }
    }
    
    
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        String stationName = getArguments().getString(STATION_NAME);
        int stationFreq = getArguments().getInt(STATION_FREQ);
        Context context = getActivity().getApplicationContext();
        IProjectStringExt projectStringExt = ExtensionUtils.getExtension(context);
        View v = View.inflate(getActivity(), R.layout.addstation, null);
        EditText editTextStationName = (EditText) v.findViewById(R.id.dlg_add_station_name_text);
        ((TextView) v.findViewById(R.id.dlg_add_station_freq_text))
                     .setText(FMRadioUtils.formatStation(stationFreq));
        // if not have current station name, hint with default station name
        // else hint current station name
        if (null == stationName || "".equals(stationName)) {
            editTextStationName.setHint(R.string.default_station_name);
        } else {
            editTextStationName.setHint(stationName);
        }
        editTextStationName.requestFocus();
        editTextStationName.requestFocusFromTouch();
        editTextStationName.setText("");
        return new AlertDialog.Builder(getActivity())
                // Must call setTitle here or the title will not be displayed.
                .setTitle(projectStringExt.getProjectString(context,
                        R.string.add_to_favorite, R.string.add_to_favorite1))
                .setView(v)
                .setPositiveButton(R.string.btn_ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        mListener.addFavorite();
                    }
                }).setNegativeButton(R.string.btn_cancel, null) // cancel button not need to handle
                                                                // any thing
                .create();
    }

}
