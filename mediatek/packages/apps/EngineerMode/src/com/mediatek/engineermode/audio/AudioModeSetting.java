/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.audio;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.media.AudioSystem;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** Audio mode parameter settings. */
public class AudioModeSetting extends Activity implements OnClickListener {

    private static final String TAG = "EM/Audio_modesetting";

    /** normal, headset, handfree. */
    private static int sMaxVolMode = 3;
    /** 7 level. */
    private static int sMaxVolLevel = 7;
    /** 8 level. */
    private static int sMaxVolType = 8;
    /** 8 types. */
    private static final int MAX_VOL_SIZE = 6;
    private static int[] sOffSet = { sMaxVolMode * sMaxVolLevel * 0,
            sMaxVolMode * sMaxVolLevel * 1, sMaxVolMode * sMaxVolLevel * 2,
            sMaxVolMode * sMaxVolLevel * 3, sMaxVolMode * sMaxVolLevel * 4,
            sMaxVolMode * sMaxVolLevel * 5, sMaxVolMode * sMaxVolLevel * 6,
            sMaxVolMode * sMaxVolLevel * 7 };
    /** Media type number. */
    private static int sTypeMedia = 6;

    private static final int TYPE_MAX_NORMAL = 6;
    private static final int TYPE_MAX_HEADSET = 6;
    private static final int TYPE_MAX_SPEAKER = 6;
    private static final int TYPE_MAX_HEADSPEAKER = 8;
    private static final int TYPE_MAX_EXTAMP = 6;
    /** Data struct size. */
    private static int sStructSize = sMaxVolMode * sMaxVolLevel * sMaxVolType;
    /** Get custom data size. */
    private static final int GET_CUSTOMD_DATASIZE = 5;
    /** Get custom data. */
    private static int sSetCustomerData = 6;
    /** Set custom data size. */
    private static int sGetCustomerData = 7;
    /** Get data error dialog id. */
    private static final int DIALOG_ID_GET_DATA_ERROR = 1;
    /** set audio mode parameter value success dialog id. */
    private static final int DIALOG_ID_SET_SUCCESS = 2;
    /** set audio mode parameter value failed dialog id. */
    private static final int DIALOG_ID_SET_ERROR = 3;
    private static final int VALUE_RANGE_255 = 255;
    private static final int VALUE_RANGE_160 = 160;
    private static int sModeMicIndex = 2;
    private static int sModeSphIndex = 4;
    private static int sModeSph2Index = 4;
    private static int sModeSidIndex = 5;
    private static final int AUDIO_COMMAND_PARAM0 = 0x20;
    private static final int AUDIO_COMMAND_PARAM1 = 0x21;
    private static final int AUDIO_COMMAND_PARAM2 = 0x22;
    private static final int CONSTANT_256 = 256;
    private static final int CONSTANT_0XFF = 0xFF;

    /** Selected category: normal, headset, loudspeaker or headset_loudspeaker. */
    private int mCurrentMode;
    /** Selected mode index. */
    private int mTypeIndex;
    /** Selected level index. */
    private int mLevelIndex;
    /** value range 255. */
    private int mValueRange = VALUE_RANGE_255;
    /** Tip for the value text view. */
    private TextView mValText;
    /** Set value button. */
    private Button mBtnSet;
    /** Fill the value edit text. */
    private EditText mValueEdit;
    /** Set the max voice button. */
    private Button mBtnSetMaxVol;
    /** Fill the max value edit text. */
    private EditText mEditMaxVol;
    /** Fir summary text view. */
    private TextView mFirsummary;
    /** Set the max voice button. */
    private Button mBtnSetMaxVolSpeaker;
    /** Fill the max value edit text. */
    private EditText mEditMaxVolSpeaker;
    private Spinner mFirSpinner;
    /** Audio data byte array. */
    private byte[] mData = null;
    /** Is first fir set?. */
    private boolean mIsFirstFirSet = true;
    private boolean mSupportEnhance = false;
    private int[] mRealUsageVols;
    private Spinner mVolTypeSpinner;
    private Spinner mVolLevelSpinner;
    
    private class ValLevelItemSelectListener implements OnItemSelectedListener{
		@Override
		public void onItemSelected(AdapterView<?> arg0, View arg1,
                int arg2, long arg3) {

            mLevelIndex = arg2;
            mValueEdit.setText(String.valueOf(getValue(mData, mCurrentMode,
                    mTypeIndex, mLevelIndex)));
            setMaxVolEdit();
            Xlog.v(TAG, "SLevel: " + mCurrentMode + " " + mTypeIndex + " "
                    + mLevelIndex);

        }
		@Override
        public void onNothingSelected(AdapterView<?> arg0) {
            Xlog.v(TAG, "noting selected.");
        }
    	
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Resources resources = getResources();
        final Intent intent = getIntent();
        mCurrentMode = intent.getIntExtra(Audio.CURRENT_MODE, 0);
        mSupportEnhance = intent.getBooleanExtra(Audio.ENHANCE_MODE, false);
        Xlog.v(TAG, "mCurrentMode: " + mCurrentMode + "mSupportEnhance: "
                + mSupportEnhance);
        if (mSupportEnhance) {
            setContentView(R.layout.audio_modesetting_enhance);
            sMaxVolMode = 4;
            sMaxVolLevel = 15;
            sMaxVolType = 9;
            sTypeMedia = 7;
            sStructSize = sMaxVolMode * sMaxVolLevel * sMaxVolType
                    + TYPE_MAX_NORMAL + TYPE_MAX_HEADSET + TYPE_MAX_SPEAKER
                    + TYPE_MAX_HEADSPEAKER + TYPE_MAX_EXTAMP + sMaxVolType;
            sSetCustomerData = 0x101;
            sGetCustomerData = 0x100;
            sModeSph2Index = 5;
            sModeSidIndex = 6;
            sOffSet = new int[] { sMaxVolMode * sMaxVolLevel * 0,
                    sMaxVolMode * sMaxVolLevel * 1,
                    sMaxVolMode * sMaxVolLevel * 2,
                    sMaxVolMode * sMaxVolLevel * 3,
                    sMaxVolMode * sMaxVolLevel * 4,
                    sMaxVolMode * sMaxVolLevel * 5,
                    sMaxVolMode * sMaxVolLevel * 6,
                    sMaxVolMode * sMaxVolLevel * 7,
                    sMaxVolMode * sMaxVolLevel * 8,
                    sMaxVolMode * sMaxVolLevel * 9 };
        } else {
            setContentView(R.layout.audio_modesetting);
            int dataSize = AudioSystem.getAudioCommand(GET_CUSTOMD_DATASIZE);
            if (dataSize != sStructSize) {
                Xlog.d(TAG, "assert! Check the structure size!");
                Toast.makeText(AudioModeSetting.this,
                        "Error: the structure size is error",
                        Toast.LENGTH_SHORT).show();
                finish();
                return;
            }
        }
        mData = new byte[sStructSize];
        Arrays.fill(mData, 0, sStructSize, (byte) 0);
        // get the current data
        int ret = AudioSystem
                .getAudioData(sGetCustomerData, sStructSize, mData);
        if (ret != 0) {
            showDialog(DIALOG_ID_GET_DATA_ERROR);
            Xlog.i(TAG, "AudioModeSetting GetAudioData return value is : "
                    + ret);
        }
        
        if (mSupportEnhance) {
        	mRealUsageVols = new int[sMaxVolType];
        	String debugStr = "[";
	        for (int i = 0; i < sMaxVolType; i++) {
	        	mRealUsageVols[sMaxVolType - i - 1] = mData[sStructSize - i - 1];
	        	debugStr += mData[sStructSize - i - 1] + ",";
	        }
	        Xlog.d(TAG, "mRealUsageVols: " + debugStr + "]");
        }
        mBtnSet = (Button) findViewById(R.id.Audio_ModeSetting_Button);
        mValueEdit = (EditText) findViewById(R.id.Audio_ModeSetting_EditText);
        if (mSupportEnhance) {
            mBtnSetMaxVol = (Button) findViewById(R.id.Audio_MaxVol_Set_headset);
            mEditMaxVol = (EditText) findViewById(R.id.Audio_MaxVol_Edit_headset);
            mBtnSetMaxVolSpeaker = (Button) findViewById(R.id.Audio_MaxVol_Set_speaker);
            mEditMaxVolSpeaker = (EditText) findViewById(R.id.Audio_MaxVol_Edit_speaker);
        } else {
            mBtnSetMaxVol = (Button) findViewById(R.id.Audio_MaxVol_Set);
            mEditMaxVol = (EditText) findViewById(R.id.Audio_MaxVol_Edit);
        }
        mVolTypeSpinner = (Spinner) findViewById(R.id.Audio_ModeSetting);
        mVolLevelSpinner = (Spinner) findViewById(R.id.Audio_Level);
        mValText = (TextView) findViewById(R.id.Audio_ModeSetting_TextView);
        if (mSupportEnhance) {
            if (mCurrentMode != 3) {
                TextView tvView = (TextView) findViewById(R.id.Audio_MaxVol_Tv_Max);
                tvView.setText(R.string.Audio_MaxVol_Text);
                View v = findViewById(R.id.Audio_MaxVol_Dual_Set);
                v.setVisibility(View.GONE);
            }
        } else {
            mFirSpinner = (Spinner) findViewById(R.id.Audio_Fir_Spinner);
            mFirsummary = (TextView) findViewById(R.id.Audio_Fir_Title);
        }
        final ArrayList<String> adapterList = new ArrayList<String>();
        if (mCurrentMode == 0) {
            adapterList.addAll(Arrays.asList(resources
                    .getStringArray(mSupportEnhance ? R.array.mode_type0_enh
                            : R.array.mode_type0)));
        } else if (mCurrentMode == 1) {
            adapterList.addAll(Arrays.asList(resources
                    .getStringArray(mSupportEnhance ? R.array.mode_type1_enh
                            : R.array.mode_type1)));
        } else if (mCurrentMode == 2) {
            adapterList.addAll(Arrays.asList(resources
                    .getStringArray(mSupportEnhance ? R.array.mode_type2_enh
                            : R.array.mode_type2)));
        } else {
            adapterList.addAll(Arrays.asList(resources
                    .getStringArray(R.array.mode_type3_enh)));
        }
        ArrayAdapter<String> mTypeAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, adapterList);
        mTypeAdatper
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVolTypeSpinner.setAdapter(mTypeAdatper);
        mVolTypeSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> arg0, View arg1,
                    int arg2, long arg3) {

                mValText.setText(R.string.Audio_ModeSetting_TextView);
                mValueRange = VALUE_RANGE_255;
                if (mCurrentMode == 0) {
                    if (arg2 == 0 || arg2 == 1) {
                        mTypeIndex = arg2 + 1;
                    } else {
                        mTypeIndex = arg2 + 2;
                    }
                } else if (mCurrentMode == 1) {
                    mTypeIndex = arg2 + 1;
                } else {
                    mTypeIndex = arg2;
                }
                Xlog.d(TAG, "mTypeIndex is:" + mTypeIndex);
                // Mode:Sph/Sid/Mic
                if (mTypeIndex == sModeSphIndex || mTypeIndex == sModeSph2Index
                        || mTypeIndex == sModeSidIndex
                        || mTypeIndex == sModeMicIndex) {
                    mEditMaxVol.setEnabled(false);
                    mBtnSetMaxVol.setEnabled(false);
                    if (mSupportEnhance) {
                        mEditMaxVolSpeaker.setEnabled(false);
                        mBtnSetMaxVolSpeaker.setEnabled(false);
                    }
                    if (mTypeIndex == sModeSphIndex
                            || mTypeIndex == sModeSph2Index) { // Mode Sph
                        mValText.setText(R.string.text_tip);
                        mValueRange = VALUE_RANGE_160;
                    }
                } else {
                    mEditMaxVol.setEnabled(true);
                    mBtnSetMaxVol.setEnabled(true);
                    if (mSupportEnhance) {
                        mEditMaxVolSpeaker.setEnabled(true);
                        mBtnSetMaxVolSpeaker.setEnabled(true);
                    }
                }
                mValueEdit.setText(String.valueOf(getValue(mData, mCurrentMode,
                        mTypeIndex, mLevelIndex)));
                setMaxVolEdit();
                Xlog.v(TAG, "SMode: " + mCurrentMode + " " + mTypeIndex + " "
                        + mLevelIndex);
                // update level spinner for 6589
                if (mSupportEnhance) {
	                String itemName = arg0.getSelectedItem().toString();
	                Xlog.d(TAG, "itemName: " + itemName);
	                ArrayAdapter<String> adapter = new ArrayAdapter<String>(AudioModeSetting.this,
	                        android.R.layout.simple_spinner_item, getVolLevelList(itemName));
	                adapter
	                        .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	                mVolLevelSpinner.setAdapter(adapter);
                }
            }

            public void onNothingSelected(AdapterView<?> arg0) {
                Xlog.v(TAG, "noting selected.");
            }
        });

        List<String> volLevelList;
        if (mSupportEnhance) {
        	volLevelList = getVolLevelList(adapterList.get(0));
        } else {
        	volLevelList = Arrays.asList(resources
                    .getStringArray(R.array.mode_level));
        }
        ArrayAdapter<String> mLevelAdatper = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, volLevelList);
        mLevelAdatper
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVolLevelSpinner.setAdapter(mLevelAdatper);
        mVolLevelSpinner.setOnItemSelectedListener(new ValLevelItemSelectListener());

        mBtnSet.setOnClickListener(this);
        mBtnSetMaxVol.setOnClickListener(this);
        if (mSupportEnhance) {
            mBtnSetMaxVolSpeaker.setOnClickListener(this);
        } else {
            ArrayAdapter<String> mFirAdatper = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, resources
                            .getStringArray(R.array.mode_fir));
            mFirAdatper
                    .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mFirSpinner.setAdapter(mFirAdatper);
            mFirSpinner.setOnItemSelectedListener(new OnItemSelectedListener() {

                public void onItemSelected(AdapterView<?> arg0, View arg1,
                        int arg2, long arg3) {
                    if (mIsFirstFirSet) {
                        mFirsummary.setText(R.string.fir_text);
                        mIsFirstFirSet = false;
                        return;
                    }
                    int ret = -1;
                    if (mCurrentMode == 0) { // normal mode
                        ret = AudioSystem.setAudioCommand(AUDIO_COMMAND_PARAM0,
                                arg2);
                        Xlog.v(TAG, "set normal fir Z" + arg2);
                        // headset mode
                    } else if (mCurrentMode == 1) {
                        ret = AudioSystem.setAudioCommand(AUDIO_COMMAND_PARAM1,
                                arg2);
                        Xlog.v(TAG, "set headset fir Z" + arg2);
                        // loudspeaker mode
                    } else if (mCurrentMode == 2) {
                        ret = AudioSystem.setAudioCommand(AUDIO_COMMAND_PARAM2,
                                arg2);
                        Xlog.v(TAG, "set loudspeaker fir Z" + arg2);
                    }
                    if (-1 == ret) {
                        mFirsummary.setText("FIR set error!");
                        Toast.makeText(AudioModeSetting.this,
                                "Set error, check permission.",
                                Toast.LENGTH_LONG).show();
                    } else {
                        mFirsummary.setText("Current selected: " + arg2);
                    }
                }

                public void onNothingSelected(AdapterView<?> arg0) {
                    Xlog.v(TAG, "noting selected.");
                }
            });
        }
        mValueEdit.setText(String.valueOf(getValue(mData, mCurrentMode,
                mTypeIndex, mLevelIndex)));
        setMaxVolEdit();

    }
    
    private int getUsageVol(String volTypeStr) {
    	if (mRealUsageVols == null || mRealUsageVols.length != sMaxVolType) {
    		throw new RuntimeException("Invalid mRealUsageVols");
    	}
    	
    	if (getString(R.string.audio_val_type_ring).equals(volTypeStr)) {
    		return mRealUsageVols[0];
    	} else if (getString(R.string.audio_val_type_sip).equals(volTypeStr)) {
    		return mRealUsageVols[1];
    	}else if (getString(R.string.audio_val_type_mic).equals(volTypeStr)) {
    		return mRealUsageVols[2];
    	}else if (getString(R.string.audio_val_type_fmr).equals(volTypeStr)) {
    		return mRealUsageVols[3];
    	}else if (getString(R.string.audio_val_type_sph).equals(volTypeStr)) {
    		return mRealUsageVols[4];
    	}else if (getString(R.string.audio_val_type_sph2).equals(volTypeStr)) {
    		return mRealUsageVols[5];
    	}else if (getString(R.string.audio_val_type_sid).equals(volTypeStr)) {
    		return mRealUsageVols[6];
    	}else if (getString(R.string.audio_val_type_media).equals(volTypeStr)) {
    		return mRealUsageVols[7];
    	}else if (getString(R.string.audio_val_type_matv).equals(volTypeStr)) {
    		return mRealUsageVols[8];
    	}
    	return -1;
    }
    
    private List<String> getVolLevelList(String volTypeStr) {
    	List<String> list = new ArrayList<String>();
    	String prefix = "Level ";
    	int usageVol = getUsageVol(volTypeStr);
    	
    	for (int i = 0; i < usageVol; i++) {
    		list.add(prefix + i);
    	}
    	return list;
    }

    private void setValue(byte[] dataPara, int mode, int type, int level,
            byte val) {
        if (dataPara == null || mode >= sMaxVolMode || type >= sMaxVolType
                || level >= sMaxVolLevel) {
            Xlog.d(TAG, "assert! Check the setting value.");
        }
        Xlog.d(TAG, "setValue() mode:" + mode + ", type:" + type + "level:" + level);
        dataPara[mode * sMaxVolLevel + level + sOffSet[type]] = val;
    }

    private int getValue(byte[] dataPara, int mode, int type, int level) {
        if (dataPara == null || mode >= sMaxVolMode || type >= sMaxVolType
                || level >= sMaxVolLevel) {
            Xlog.d(TAG, "assert! Check the setting value.");
        }
        return CONSTANT_0XFF
                & (dataPara[mode * sMaxVolLevel + level + sOffSet[type]]);
    }

    private void setMaxVolEdit() {
        Xlog.i(TAG, "Set max vol Edit.");
        if (mSupportEnhance) {
            mEditMaxVol.setText(String.valueOf(getMaxValue(mData, mCurrentMode,
                    false)));
            if (mCurrentMode == 3) {
                mEditMaxVolSpeaker.setText(String.valueOf(getMaxValue(mData,
                        mCurrentMode, true)));
            }
        } else {
            if (mCurrentMode == 0) { // normal mode
                mEditMaxVol.setText(String.valueOf(getValue(mData, 0,
                        sTypeMedia, sModeSphIndex)));
                Xlog.i(TAG, "0 is "
                        + getValue(mData, 0, sTypeMedia, sModeSphIndex));
            } else if (mCurrentMode == 1) { // headset mode
                mEditMaxVol.setText(String.valueOf(getValue(mData, 0,
                        sTypeMedia, sModeSidIndex)));
                Xlog.i(TAG, "1 is "
                        + getValue(mData, 0, sTypeMedia, sModeSidIndex));
            } else if (mCurrentMode == 2) { // loudspeaker mode
                mEditMaxVol.setText(String.valueOf(getValue(mData, 0,
                        sTypeMedia, sTypeMedia)));
                Xlog.i(TAG, "2 is "
                        + getValue(mData, 0, sTypeMedia, sTypeMedia));
            } else {
                mEditMaxVol.setText("0");
                Xlog.i(TAG, "error is " + 0);
            }
        }
    }

    private void setMaxValue(byte[] dataPara, int mode, byte val, boolean dual) {
        if (dataPara == null || mode >= sMaxVolMode) {
            Xlog.d(TAG, "assert! Check the setting value.");
        }
        if (dual && (mode == 3)) {
            dataPara[sOffSet[sMaxVolType] + mode * MAX_VOL_SIZE + 1] = val;
        } else {
            dataPara[sOffSet[sMaxVolType] + mode * MAX_VOL_SIZE] = val;
        }
    }

    private int getMaxValue(byte[] dataPara, int mode, boolean dual) {
        if (dataPara == null || mode >= sMaxVolMode) {
            Xlog.d(TAG, "assert! Check the setting value.");
        }
        if (dual && (mode == 3)) {
            return CONSTANT_0XFF
                    & (dataPara[sOffSet[sMaxVolType] + mode * MAX_VOL_SIZE + 1]);
        } else {
            return CONSTANT_0XFF
                    & (dataPara[sOffSet[sMaxVolType] + mode * MAX_VOL_SIZE]);
        }
    }

    private void setMaxVolData(byte val) {
        if (mCurrentMode == 0) { // normal mode
            setValue(mData, 0, sTypeMedia, sModeSphIndex, val);
        } else if (mCurrentMode == 1) { // headset mode
            setValue(mData, 0, sTypeMedia, sModeSidIndex, val);
        } else if (mCurrentMode == 2) { // loudspeaker mode
            setValue(mData, 0, sTypeMedia, sTypeMedia, val);
        }
    }

    private void setMaxVolData(byte val, boolean dual) {
        setMaxValue(mData, mCurrentMode, val, dual);
    }

    private void setAudioData() {
        int result = AudioSystem.setAudioData(sSetCustomerData, sStructSize,
                mData);
        if (0 == result) {
            showDialog(DIALOG_ID_SET_SUCCESS);
        } else {
            showDialog(DIALOG_ID_SET_ERROR);
            Xlog.i(TAG, "AudioModeSetting SetAudioData return value is : "
                    + result);
        }
    }

    private boolean checkEditNumber(EditText edit, int maxValue) {
        String editStr = edit.getText().toString();
        if (null == editStr || editStr.length() == 0) {
            Toast.makeText(this, getString(R.string.input_null_tip), Toast.LENGTH_LONG)
                    .show();
            return false;
        }
        try {
            if (Integer.valueOf(editStr) > maxValue) {
                Toast.makeText(this, getString(R.string.number_arrage_tip) + maxValue,
                        Toast.LENGTH_LONG).show();
                return false;
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, getString(R.string.number_arrage_tip) + maxValue,
                    Toast.LENGTH_LONG).show();
            return false;
        }
        return true;
    }

    /**
     * click the set button.
     * 
     * @param arg0
     *            : click which view
     */
    public void onClick(View arg0) {

        if (arg0.getId() == mBtnSet.getId()) {

            if (!checkEditNumber(mValueEdit, mValueRange)) {
                return;
            }
            String editString = mValueEdit.getText().toString();
            int editInteger = Integer.valueOf(editString);
            byte editByte = (byte) editInteger;
            setValue(mData, mCurrentMode, mTypeIndex, mLevelIndex, editByte);
            setAudioData();
        } else if (arg0.getId() == mBtnSetMaxVol.getId()) {
            if (!checkEditNumber(mEditMaxVol, VALUE_RANGE_160)) {
                return;
            }
            String editString = mEditMaxVol.getText().toString();
            int editInteger = Integer.valueOf(editString);
            byte editByte = (byte) editInteger;
            if (mSupportEnhance) {
                setMaxVolData(editByte, false);
            } else {
                setMaxVolData(editByte);
            }
            setAudioData();
        } else if (arg0.getId() == mBtnSetMaxVolSpeaker.getId()) {
            if (!checkEditNumber(mEditMaxVolSpeaker, VALUE_RANGE_160)) {
                return;
            }
            String editString = mEditMaxVolSpeaker.getText().toString();
            int editInteger = Integer.valueOf(editString);
            byte editByte = (byte) editInteger;
            setMaxVolData(editByte, true);
            setAudioData();
        }
    }

    @Override
    public Dialog onCreateDialog(int dialogId) {
        switch (dialogId) {
        case DIALOG_ID_GET_DATA_ERROR:
            return new AlertDialog.Builder(this).setTitle(
                    R.string.get_data_error_title).setMessage(
                    R.string.get_data_error_msg).setPositiveButton(
                    android.R.string.ok, new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            removeDialog(DIALOG_ID_GET_DATA_ERROR);
                            finish();
                        }

                    }).setNegativeButton(android.R.string.cancel, null)
                    .create();
        case DIALOG_ID_SET_SUCCESS:
            return new AlertDialog.Builder(this).setTitle(
                    R.string.set_success_title).setMessage(
                    R.string.set_success_msg).setPositiveButton(
                    android.R.string.ok, null).create();
        case DIALOG_ID_SET_ERROR:
            return new AlertDialog.Builder(this).setTitle(
                    R.string.set_error_title)
                    .setMessage(R.string.set_error_msg).setPositiveButton(
                            android.R.string.ok, null).create();
        default:
            return null;
        }
    }
}
