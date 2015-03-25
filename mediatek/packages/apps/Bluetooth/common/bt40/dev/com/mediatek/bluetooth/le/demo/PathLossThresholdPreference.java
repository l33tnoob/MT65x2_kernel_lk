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

package com.mediatek.bluetooth.le.demo;

import android.content.Context;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

public class PathLossThresholdPreference extends DialogPreference implements SeekBar.OnSeekBarChangeListener, CheckBox.OnCheckedChangeListener{

    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

        String t = String.valueOf(value);
        this.valueText.setText(this.suffix == null ? t : t.concat(this.suffix));
        if (shouldPersist())
            persistInt(value);
        callChangeListener(new Integer(value));
    }

    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {

    }

    private static final String androidns = "http://schemas.android.com/apk/res/android";

    private Context context;

    private SeekBar seekBar;
    private TextView splashText, valueText;


    private String dialogMessage, suffix;
    private int defaultValue, max, value = 0;

    public PathLossThresholdPreference( Context context, AttributeSet attrs ){

        super( context, attrs );

        this.context = context;
        this.dialogMessage = attrs.getAttributeValue(androidns, "dialogMessage");
        this.suffix = attrs.getAttributeValue(androidns, "text");
        this.defaultValue = attrs.getAttributeIntValue(androidns, "defaultValue", 0);
        this.max = attrs.getAttributeIntValue(androidns, "max", 100);
    }

    protected View onCreateDialogView() {

        LinearLayout.LayoutParams params;
        LinearLayout layout = new LinearLayout( this.context );
        layout.setOrientation( LinearLayout.VERTICAL );
        layout.setPadding(6, 6, 6, 6);

        this.splashText = new TextView( this.context );
        if( this.dialogMessage != null )
            this.splashText.setText( this.dialogMessage );
        layout.addView( this.splashText );

        this.valueText = new TextView(this.context);
        this.valueText.setGravity(Gravity.CENTER_HORIZONTAL);
        this.valueText.setTextSize(22);
        params = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        layout.addView( this.valueText, params );

        this.seekBar = new SeekBar( this.context );
        this.seekBar.setOnSeekBarChangeListener(this);
        layout.addView( this.seekBar, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.FILL_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT));

        if (shouldPersist())
            this.value = getPersistedInt(this.defaultValue);

        this.seekBar.setMax(this.max);
        this.seekBar.setProgress(this.value);
        this.seekBar.setOnSeekBarChangeListener(this);
        return layout;
    }

    protected void onBindDialogView(View v) {
        super.onBindDialogView(v);
        this.seekBar.setMax(this.max);
        this.seekBar.setProgress(this.value);
    }

    protected void onSetInitialValue(boolean restore, Object defaultValue) {
        super.onSetInitialValue(restore, defaultValue);
        if (restore)
            this.value = shouldPersist() ? getPersistedInt(this.defaultValue) : 0;
        else
            this.value = (Integer) defaultValue;
    }

    public void setMax(int max) {
        this.max = max;
    }

    public int getMax() {
        return this.max;
    }

    public void setProgress(int progress) {
        this.value = progress;
        if ( this.seekBar != null)
            this.seekBar.setProgress(progress);
    }

    public int getProgress() {
        return this.value;
    }

}
