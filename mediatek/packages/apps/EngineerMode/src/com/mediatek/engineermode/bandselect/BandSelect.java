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

package com.mediatek.engineermode.bandselect;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

/**
 * @author mtk54044 //add declartion
 * 
 */
public class BandSelect extends Activity implements OnClickListener {

    private static final String LOG_TAG = "BandModeSim1";
    private static final int WCDMA = 0x04;
    private static final int TDSCDMA = 0x08;
    private static final String GSM_BASEBAND = "gsm.baseband.capability";

    private GeminiPhone mGeminiPhone = null;
    private Phone mPhoneProxey = null;

    private final ArrayList<BandModeMap> mGsmModeArray = new ArrayList<BandModeMap>();
    private final ArrayList<BandModeMap> mUmtsModeArray = new ArrayList<BandModeMap>();

    private Button mBtnSet;

    private boolean mIsThisAlive = false;
    private int mSimType;

    private final Handler mResponseHander = new Handler() {

        public void handleMessage(final Message msg) {

            if (!mIsThisAlive) {
                return;
            }
            AsyncResult asyncResult;
            switch (msg.what) {
            case BandModeContent.EVENT_QUERY_SUPPORTED:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {

                    setMode(asyncResult, BandModeContent.EVENT_QUERY_SUPPORTED);
                } else {
                    showDialog(BandModeContent.EVENT_QUERY_SUPPORTED);
                    setSupportedMode(0, 0);

                }
                break;
            case BandModeContent.EVENT_QUERY_CURRENT:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {
                    setMode(asyncResult, BandModeContent.EVENT_QUERY_CURRENT);

                } else {

                    removeDialog(BandModeContent.EVENT_QUERY_SUPPORTED);
                    showDialog(BandModeContent.EVENT_QUERY_CURRENT);
                    setCurrentMode(0, 0);
                }
                break;
            case BandModeContent.EVENT_SET:
                asyncResult = (AsyncResult) msg.obj;
                if (asyncResult.exception == null) {

                    showDialog(BandModeContent.EVENT_SET_OK);
                } else {
                    showDialog(BandModeContent.EVENT_SET_FAIL);
                }
                break;

            default:
                break;
            }
        }
    };
    
    /**
     * @return the value of the boolean to get TDD or FDD type
     */
    public static boolean getModemType(int simType) {
        String property = GSM_BASEBAND;
        if (simType > BandModeContent.GEMINI_SIM_1) {
            property = property + simType;
        }
        String networkType = SystemProperties.get(property);
        boolean tddMode;
        if (networkType == null) {
            tddMode = false;
        } else {
            try {
                final int mask = Integer.valueOf(networkType);
                if ((mask & WCDMA) != 0) {
                    tddMode = false;
                } else if ((mask & TDSCDMA) != 0) {
                    tddMode = true;
                } else {
                    tddMode = false;
                }
            } catch (NumberFormatException e) {
                tddMode = false;
            }
        }
        return tddMode;
    }

    private static class BandModeMap {

        public CheckBox mChkBox;
        public int mBit;
        
        /**
         * @param chkbox
         *            the CheckBox from the layout
         * @param bit
         *            the integer of the modem value
         */
        BandModeMap(final CheckBox chkbox, final int bit) {
            mChkBox = chkbox;
            mBit = bit;
        }
    }

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", BandModeContent.GEMINI_SIM_1);
        boolean modemType = getModemType(mSimType);
        if (mSimType == BandModeContent.GEMINI_SIM_1) {
            if (modemType) {
                setContentView(R.layout.tddbandselect);
                mBtnSet = (Button) findViewById(R.id.TDD_Btn_Set);
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_GSM_EGSM900),
                        BandModeContent.GSM_EGSM900_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_GSM_DCS1800),
                        BandModeContent.GSM_DCS1800_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_GSM_PCS1900),
                        BandModeContent.GSM_PCS1900_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_GSM_GSM850),
                        BandModeContent.GSM_GSM850_BIT));

                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_I),
                        BandModeContent.UMTS_BAND_I_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_II),
                        BandModeContent.UMTS_BAND_II_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_III),
                        BandModeContent.UMTS_BAND_III_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_IV),
                        BandModeContent.UMTS_BAND_IV_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_V),
                        BandModeContent.UMTS_BAND_V_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.TDD_UMTS_BAND_VI),
                        BandModeContent.UMTS_BAND_VI_BIT));
            } else {
                setContentView(R.layout.bandselect);
                mBtnSet = (Button) findViewById(R.id.BandSel_Btn_Set);
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_GSM_EGSM900),
                        BandModeContent.GSM_EGSM900_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_GSM_DCS1800),
                        BandModeContent.GSM_DCS1800_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_GSM_PCS1900),
                        BandModeContent.GSM_PCS1900_BIT));
                mGsmModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_GSM_GSM850),
                        BandModeContent.GSM_GSM850_BIT));

                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_I),
                        BandModeContent.UMTS_BAND_I_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_II),
                        BandModeContent.UMTS_BAND_II_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_III),
                        BandModeContent.UMTS_BAND_III_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_IV),
                        BandModeContent.UMTS_BAND_IV_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_V),
                        BandModeContent.UMTS_BAND_V_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_VI),
                        BandModeContent.UMTS_BAND_VI_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_VII),
                        BandModeContent.UMTS_BAND_VII_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_VIII),
                        BandModeContent.UMTS_BAND_VIII_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_IX),
                        BandModeContent.UMTS_BAND_IX_BIT));
                mUmtsModeArray.add(new BandModeMap(
                        (CheckBox) findViewById(R.id.BandSel_UMTS_BAND_X),
                        BandModeContent.UMTS_BAND_X_BIT));
            }
        } else {
            setContentView(R.layout.bandmodesim2);
            mBtnSet = (Button) findViewById(R.id.bandmodesim2_Btn_Set);
            mGsmModeArray.add(new BandModeMap(
                    (CheckBox) findViewById(R.id.bandmodesim2_GSM_EGSM900),
                    BandModeContent.GSM_EGSM900_BIT));
            mGsmModeArray.add(new BandModeMap(
                    (CheckBox) findViewById(R.id.bandmodesim2_GSM_DCS1800),
                    BandModeContent.GSM_DCS1800_BIT));
            mGsmModeArray.add(new BandModeMap(
                    (CheckBox) findViewById(R.id.bandmodesim2_GSM_PCS1900),
                    BandModeContent.GSM_PCS1900_BIT));
            mGsmModeArray.add(new BandModeMap(
                    (CheckBox) findViewById(R.id.bandmodesim2_GSM_GSM850),
                    BandModeContent.GSM_GSM850_BIT));
        }

        mIsThisAlive = true;

        mBtnSet.setOnClickListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (FeatureOption.MTK_GEMINI_SUPPORT) {
            Xlog.v(LOG_TAG, "mGeminiPhone");
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();

        } else {
            Xlog.v(LOG_TAG, "mPhoneProxey");
            mPhoneProxey = (Phone) PhoneFactory.getDefaultPhone();
        }
        querySupportMode();
        queryCurrentMode();
    }

    private void setMode(AsyncResult aSyncResult, int msg) {
        final String[] result = (String[]) aSyncResult.result;

        for (final String value : result) {
            Xlog.v(LOG_TAG, "--.>" + value);
            final String splitString = value.substring(BandModeContent.SAME_COMMAND
                    .length());
            final String[] getDigitalVal = splitString.split(",");

            if (getDigitalVal != null && getDigitalVal.length > 1) {
                if (Integer.valueOf(getDigitalVal[0].trim()) != null
                        && Integer.valueOf(getDigitalVal[1].trim()) != null) {
                    if (msg == BandModeContent.EVENT_QUERY_SUPPORTED) {
                        setSupportedMode(Integer.valueOf(getDigitalVal[0].trim()),
                                Integer.valueOf(getDigitalVal[1].trim()));
                    } else {
                        setCurrentMode(Integer.valueOf(getDigitalVal[0].trim()),
                                Integer.valueOf(getDigitalVal[1].trim()));
                    }
                }
            }
        }
    }
    /**
     * Query Modem supported band modes
     */

    private void querySupportMode() {
        final String modeString[] = { BandModeContent.QUERY_SUPPORT_COMMAND,
                BandModeContent.SAME_COMMAND };
        Xlog.v(LOG_TAG, "AT String:" + modeString[0]);
    sendATCommand(modeString, BandModeContent.EVENT_QUERY_SUPPORTED);

    }

    /**
     * Query Modem is being used band modes
     */
    private void queryCurrentMode() {
        final String modeString[] = { BandModeContent.QUERY_CURRENT_COMMAND,
                BandModeContent.SAME_COMMAND };
        Xlog.v(LOG_TAG, "AT String:" + modeString[0]);
    sendATCommand(modeString, BandModeContent.EVENT_QUERY_CURRENT);

    }

    /**
     * Query Modem is being used band modes
     */
    private void sendATCommand(String[] atCommand, int msg) {
    if (FeatureOption.MTK_GEMINI_SUPPORT) {
        if (mSimType == BandModeContent.GEMINI_SIM_1) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(atCommand,
                    mResponseHander.obtainMessage(msg), PhoneConstants.GEMINI_SIM_1);
        } else if (mSimType == BandModeContent.GEMINI_SIM_2) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(atCommand,
                    mResponseHander.obtainMessage(msg), PhoneConstants.GEMINI_SIM_2);
        } else if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT && mSimType == BandModeContent.GEMINI_SIM_3) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(atCommand,
                    mResponseHander.obtainMessage(msg), PhoneConstants.GEMINI_SIM_3);
        } else if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT && mSimType == BandModeContent.GEMINI_SIM_4) {
            mGeminiPhone.invokeOemRilRequestStringsGemini(atCommand,
                    mResponseHander.obtainMessage(msg), PhoneConstants.GEMINI_SIM_4);
        }
    } else {
        mPhoneProxey.invokeOemRilRequestStrings(atCommand, mResponseHander
                .obtainMessage(msg));
    }

    }

    /**
     *Set the selected modes
     * 
     * @param gsmValue
     *            the integer of GSM mode value
     * @param umtsValue
     *            the integer of UMTS mode value
     * @return false means set failed or success
     */
    private void setBandMode(final int gsmValue, final int umtsValue) {
        int gsmVal = gsmValue;
        int umtsVal = umtsValue;
        if (gsmVal > BandModeContent.GSM_MAX_VALUE
                || umtsVal > BandModeContent.UMTS_MAX_VALUE) {
            return ;
        }
        // null select is not allowed.
        if (gsmVal == 0) {
            gsmVal = BandModeContent.GSM_MAX_VALUE;
        }
        if (umtsVal == 0) {
            umtsVal = BandModeContent.UMTS_MAX_VALUE;
        }

        final String modeString[] = {
                BandModeContent.SET_COMMAND + gsmVal + "," + umtsVal, "" };
        Xlog.v(LOG_TAG, "AT String:" + modeString[0]);
    sendATCommand(modeString, BandModeContent.EVENT_SET);
        setCurrentMode(gsmVal, umtsVal);
        
    }

    /**
     * Get the selected GSM mode values
     * 
     * @return GSM value from the selected GSM box
     */
    private int getValFromGsmBox() {
        int gsmVal = 0;
        for (final BandModeMap m : mGsmModeArray) {
            if (m.mChkBox.isChecked()) {
                gsmVal |= (1 << m.mBit);
            }
        }
        return gsmVal;
    }

    /**
     * Get the selected GSM mode values
     * 
     * @return UMTS value from the selected UMTS box
     */

    private int getValFromUmtsBox() {
        int val = 0;

        for (final BandModeMap m : mUmtsModeArray) {
            if (m.mChkBox.isChecked()) {
                val |= (1 << m.mBit);
            }
        }
        return val;
    }

    /**
     * @param gsmVal
     *            the integer value from the modem
     * @param umtsVal
     *            the integer value from the modem
     */

    private void setCurrentMode(final int gsmVal, final int umtsVal) {
        for (final BandModeMap gsmMode : mGsmModeArray) {
            if ((gsmVal & (1 << gsmMode.mBit)) == 0) {
                gsmMode.mChkBox.setChecked(false);
            } else {

                if (gsmMode.mChkBox.isEnabled()) {
                    gsmMode.mChkBox.setChecked(true);
                }
            }
        }
        for (final BandModeMap umtsMode : mUmtsModeArray) {
            if ((umtsVal & (1 << umtsMode.mBit)) == 0) {
                umtsMode.mChkBox.setChecked(false);
            } else {

                if (umtsMode.mChkBox.isEnabled()) {
                    umtsMode.mChkBox.setChecked(true);
                }
            }
        }
    }

    /**
     * @param gsmVal
     *            the integer value from the modem
     * @param umtsVal
     *            the integer value from the modem
     */

    private void setSupportedMode(final int gsmVal, final int umtsVal) {
        for (final BandModeMap m : mGsmModeArray) {
            if ((gsmVal & (1 << m.mBit)) == 0) {

                m.mChkBox.setEnabled(false);
            } else {
                m.mChkBox.setEnabled(true);
            }
        }
        for (final BandModeMap m : mUmtsModeArray) {
            if ((umtsVal & (1 << m.mBit)) == 0) {

                m.mChkBox.setEnabled(false);
            } else {
                m.mChkBox.setEnabled(true);
            }
        }
    }

    /**
     * Button on click listener
     * 
     * @param arg0
     *            the view of the button event
     */
    public void onClick(final View arg0) {

        if (arg0.getId() == mBtnSet.getId()) {
            setBandMode(getValFromGsmBox(), getValFromUmtsBox());
        }

    }

    @Override
    protected Dialog onCreateDialog(final int dialogId) {
        if (BandModeContent.EVENT_QUERY_SUPPORTED == dialogId) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.query_result);
            builder.setMessage(R.string.query_support_message);
            builder.setPositiveButton(R.string.query_btn_text, null);
            builder.create().show();
        } else if (BandModeContent.EVENT_QUERY_CURRENT == dialogId) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.query_result);
            builder.setMessage(R.string.query_current_message);
            builder.setPositiveButton(R.string.query_btn_text, null);
            builder.create().show();
        } else if (BandModeContent.EVENT_SET_OK == dialogId) {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.set_success_title);
            builder.setMessage(R.string.set_success_message);
            builder.setPositiveButton(R.string.set_ok_text, null);
            builder.create().show();
        } else {
            final AlertDialog.Builder builder = new AlertDialog.Builder(
                    BandSelect.this);
            builder.setTitle(R.string.set_fail_title);
            builder.setMessage(R.string.set_fail_message);
            builder.setPositiveButton(R.string.set_fail_text, null);
            builder.create().show();
        }
          return super.onCreateDialog(dialogId);
    }

    @Override
    public void onDestroy() {
        mIsThisAlive = false;
        super.onDestroy();
    }

}
