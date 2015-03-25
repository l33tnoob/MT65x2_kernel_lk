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

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;

import android.widget.NumberPicker;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;


public class CopiesPickerDialog extends AlertDialog implements OnClickListener {

    private static final String TAG = "CopiesPickerDialog";

    private static final String COPIES = "copies";

    //private int mInitialCopies;
    //private int mMaxCopies;
    private int mCopies;
    private final NumberPicker mCopiesPicker;
    private final OnCopiesSetListener mCallback;

    public interface OnCopiesSetListener {

        void onCopiesSet(int number);
    }



    public CopiesPickerDialog(Context context, OnCopiesSetListener callBack, int maxCopies) {

        super(context, com.android.internal.R.style.Theme_Dialog_Alert);

        mCallback = callBack;
        //mInitialCopies = initialCopies;
        //mMaxCopies = maxCopies;

        setTitle(R.string.bt_bpp_copies_title);

        setButton(context.getText(R.string.bt_bpp_copies_set), this);
        setButton2(context.getText(R.string.bt_bpp_copies_cancel), (OnClickListener) null);

        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = inflater.inflate(R.layout.bt_bpp_copies_picker_dialog, null);
        setView(view);

        mCopiesPicker = (NumberPicker) view.findViewById(R.id.number_picker);
        if ( null == mCopiesPicker ) {
             Xlog.e(TAG, "mCopiesPicker is null");

        }
        else {
            //mCopiesPicker.setRange(1, maxCopies);
            //mCopiesPicker.setCurrent(1);
            //mCopiesPicker.setSpeed(100);
            mCopiesPicker.setMinValue(1);
            mCopiesPicker.setMaxValue(maxCopies);
            mCopiesPicker.setValue(1);
            mCopiesPicker.setOnLongPressUpdateInterval(100);
        }    
    }

    public void updateCopies(int copies) {
        mCopiesPicker.setValue(copies);
    }

    public void onClick(DialogInterface dialog, int which) {
        if (mCallback != null) {
            mCopiesPicker.clearFocus();
            mCallback.onCopiesSet(mCopiesPicker.getValue());
        }
    }

    @Override
    public Bundle onSaveInstanceState() {
        Bundle state = super.onSaveInstanceState();
        state.putInt(COPIES, mCopiesPicker.getValue());
        return state;
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        int number = savedInstanceState.getInt(COPIES);
        mCopiesPicker.setValue(number);
    }
}
