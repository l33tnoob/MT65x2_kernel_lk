package com.mediatek.engineermode.hqanfc;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.BitMapValue;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.ReaderModeRspResult;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermNtf;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermRsp;

import java.nio.ByteBuffer;

public class ReaderMode extends Activity {

    protected static final String KEY_READER_MODE_RSP_ARRAY = "reader_mode_rsp_array";
    protected static final String KEY_READER_MODE_RSP_NDEF = "reader_mode_rsp_ndef";

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int HANDLER_MSG_GET_NTF = 100;
    private static final int DIALOG_ID_WAIT = 0;

    private static final int CHECKBOXS_NUMBER = 5;
    private static final int CHECKBOX_READER_TYPEA = 0;
    private static final int CHECKBOX_READER_TYPEB = 1;
    private static final int CHECKBOX_READER_TYPEF = 2;
    private static final int CHECKBOX_READER_TYPEV = 3;
   // private static final int CHECKBOX_READER_TYPEB2 = 4;
    private static final int CHECKBOX_READER_KOVIO = 4;

    private static final int RADIO_READER_TYPEA_106 = 0;
    private static final int RADIO_READER_TYPEA_212 = 1;
    private static final int RADIO_READER_TYPEA_424 = 2;
    private static final int RADIO_READER_TYPEA_848 = 3;
    private static final int RADIO_READER_TYPEB_106 = 4;
    private static final int RADIO_READER_TYPEB_212 = 5;
    private static final int RADIO_READER_TYPEB_424 = 6;
    private static final int RADIO_READER_TYPEB_848 = 7;
    private static final int RADIO_READER_TYPEF_212 = 8;
    private static final int RADIO_READER_TYPEF_424 = 9;
    private static final int RADIO_READER_TYPEV_166 = 10;
    private static final int RADIO_READER_TYPEV_2648 = 11;
    private static final int RADIO_NUMBER = 12;

    private CheckBox[] mSettingsCkBoxs = new CheckBox[CHECKBOXS_NUMBER];
    private RadioButton[] mSettingsRadioButtons = new RadioButton[RADIO_NUMBER];
    private RadioGroup mRgTypeA;
    private RadioGroup mRgTypeB;
    private RadioGroup mRgTypeF;
    private RadioGroup mRgTypeVSubCarrier;
    private RadioGroup mRgTypeVMode;
    private RadioGroup mRgTypeVRate;
    private Button mBtnSelectAll;
    private Button mBtnClearAll;
    private Button mBtnStart;
    private Button mBtnReturn;
    private Button mBtnRunInBack;
    private NfcEmAlsReadermRsp mResponse;
    private NfcEmAlsReadermNtf mReadermNtf;
    private byte[] mRspArray;
    private boolean mEnableBackKey = true; // can or can not press back key
    private boolean mUnregisterReceiver = false;
    private boolean mRunInBack = false;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.v(NfcMainPage.TAG, "[ReaderMode]mReceiver onReceive: " + action);
            mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_RSP).equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmAlsReadermRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_NTF)
                    .equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mReadermNtf = new NfcEmAlsReadermNtf();
                    mReadermNtf.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_NTF);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[ReaderMode]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            String toastMsg = null;
            if (HANDLER_MSG_GET_RSP == msg.what) {
                dismissDialog(DIALOG_ID_WAIT);
                switch (mResponse.mResult) {
                    case RspResult.SUCCESS:
                        toastMsg = "ReaderMode Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(
                                ReaderMode.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "ReaderMode Rsp Result: FAIL";
                        break;
                    case RspResult.NFC_STATUS_INVALID_FORMAT:
                        toastMsg = "ReaderMode Rsp Result: INVALID_FORMAT";
                        break;
                    case RspResult.NFC_STATUS_INVALID_NDEF_FORMAT:
                        toastMsg = "ReaderMode Rsp Result: INVALID_NDEF_FORMAT";
                        break;
                    case RspResult.NFC_STATUS_NDEF_EOF_REACHED:
                        toastMsg = "ReaderMode Rsp Result: NDEF_EOF_REACHED";
                        break;
                    case RspResult.NFC_STATUS_NOT_SUPPORT:
                        toastMsg = "ReaderMode Rsp Result: NOT_SUPPORT";
                        break;
                    default:
                        toastMsg = "ReaderMode Rsp Result: ERROR";
                        break;
                }
            } else if (HANDLER_MSG_GET_NTF == msg.what) {
                switch (mReadermNtf.mResult) {
                    case ReaderModeRspResult.CONNECT:
                        toastMsg = "ReaderMode Ntf Result: CONNECT";
                        if ((mRunInBack == false) && 
                                (mReadermNtf.mIsNdef == 0 || mReadermNtf.mIsNdef == 1 || mReadermNtf.mIsNdef == 2)) {
                            Intent intent = new Intent();
                            intent.putExtra(KEY_READER_MODE_RSP_ARRAY, mRspArray);
                            intent.putExtra(KEY_READER_MODE_RSP_NDEF, mReadermNtf.mIsNdef);
                            intent.setClass(ReaderMode.this, RwFunction.class);
                            unregisterReceiver(mReceiver);
                            mUnregisterReceiver = true;
                            startActivity(intent);
                        }
                        break;
                    case ReaderModeRspResult.DISCONNECT:
                        toastMsg = "ReaderMode Ntf Result: DISCONNECT";
                        break;
                    case ReaderModeRspResult.FAIL:
                        // toastMsg = "ReaderMode Ntf Result: FAIL"; ALPS00676063
                        Elog.v(NfcMainPage.TAG, "[ReaderMode]Ntf Result: FAIL");
                        if(mRunInBack) {
                            toastMsg = "ReaderMode Ntf Result: FAIL";
                        } else {
                            Intent intent = new Intent();
                            intent.putExtra(KEY_READER_MODE_RSP_ARRAY, mRspArray);
                            intent.putExtra(KEY_READER_MODE_RSP_NDEF, 3);
                            intent.setClass(ReaderMode.this, RwFunction.class);
                            unregisterReceiver(mReceiver);
                            mUnregisterReceiver = true;
                            startActivity(intent);
                        }
                        break;
                    default:
                        Elog.v(NfcMainPage.TAG, "ReaderMode Ntf Result: ERROR");
                        if(mRunInBack) {
                            toastMsg = "Tag is not NDEF format";
                        } else {
                            Intent defaultIntent = new Intent();
                            defaultIntent.putExtra(KEY_READER_MODE_RSP_ARRAY, mRspArray);
                            defaultIntent.putExtra(KEY_READER_MODE_RSP_NDEF, 3);
                            defaultIntent.setClass(ReaderMode.this, RwFunction.class);
                            unregisterReceiver(mReceiver);
                            mUnregisterReceiver = true;
                            startActivity(defaultIntent);
                        }
                        break;
                }
            }
            if(toastMsg != null) {
                Elog.v(NfcMainPage.TAG, "Toast: " + toastMsg);
                Toast.makeText(ReaderMode.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final CheckBox.OnCheckedChangeListener mCheckedListener = new CheckBox.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
            Elog.v(NfcMainPage.TAG, "[ReaderMode]onCheckedChanged view is " + buttonView.getText()
                    + " value is " + checked);
            if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEA])) {
                for (int i = 0; i < mRgTypeA.getChildCount(); i++) {
                    mRgTypeA.getChildAt(i).setEnabled(checked);
                }
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEB])) {
                for (int i = 0; i < mRgTypeB.getChildCount(); i++) {
                    mRgTypeB.getChildAt(i).setEnabled(checked);
                }
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEF])) {
                for (int i = 0; i < mRgTypeF.getChildCount(); i++) {
                    mRgTypeF.getChildAt(i).setEnabled(checked);
                }
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEV])) {
                for (int i = 0; i < mRgTypeVSubCarrier.getChildCount(); i++) {
                    mRgTypeVSubCarrier.getChildAt(i).setEnabled(checked);
                }
                for (int i = 0; i < mRgTypeVMode.getChildCount(); i++) {
                    mRgTypeVMode.getChildAt(i).setEnabled(checked);
                }
                for (int i = 0; i < mRgTypeVRate.getChildCount(); i++) {
                    mRgTypeVRate.getChildAt(i).setEnabled(checked);
                }
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[ReaderMode]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                showDialog(DIALOG_ID_WAIT);
                doTestAction(mBtnStart.getText().equals(
                        ReaderMode.this.getString(R.string.hqa_nfc_start)));
            } else if (arg0.equals(mBtnSelectAll)) {
                changeAllSelect(true);
            } else if (arg0.equals(mBtnClearAll)) {
                changeAllSelect(false);
            } else if (arg0.equals(mBtnReturn)) {
                ReaderMode.this.onBackPressed();
            } else if (arg0.equals(mBtnRunInBack)) {                
                doTestAction(null);                
                Intent intent = new Intent(Intent.ACTION_MAIN);                
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);                
                intent.addCategory(Intent.CATEGORY_HOME);               
                startActivity(intent);
                mRunInBack = true; // run in back, just show toast, do not call rwfunction.class.          
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Elog.v(NfcMainPage.TAG, "[ReaderMode]onCreate");
        setContentView(R.layout.hqa_nfc_reader_mode);
        initComponents();
        changeAllSelect(true);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_RSP);
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_NTF);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onStop() {
        Elog.v(NfcMainPage.TAG, "[ReaderMode]onStop");
        super.onStop();
    }
    @Override
    protected void onStart() {
        Elog.v(NfcMainPage.TAG, "[ReaderMode]onStart");
        mRunInBack = false;
        if(mUnregisterReceiver) {
            Elog.v(NfcMainPage.TAG, "register receiver");
            mUnregisterReceiver = false;
            IntentFilter filter = new IntentFilter();
            filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_RSP);
            filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_NTF);
            registerReceiver(mReceiver, filter);
        }
        super.onStart();  
    }
    
    @Override
    protected void onDestroy() {
        Elog.v(NfcMainPage.TAG, "[ReaderMode]onDestroy");
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (!mEnableBackKey) {
            return;
        }
        super.onBackPressed();
    }

    private void initComponents() {
        Elog.v(NfcMainPage.TAG, "[ReaderMode]initComponents");
        mSettingsCkBoxs[CHECKBOX_READER_TYPEA] = (CheckBox) findViewById(R.id.hqa_readermode_cb_typea);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEA].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEB] = (CheckBox) findViewById(R.id.hqa_readermode_cb_typeb);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEB].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEF] = (CheckBox) findViewById(R.id.hqa_readermode_cb_typef);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEF].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEV] = (CheckBox) findViewById(R.id.hqa_readermode_cb_typev);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEV].setOnCheckedChangeListener(mCheckedListener);

        mSettingsRadioButtons[RADIO_READER_TYPEA_106] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typea_106);
        mSettingsRadioButtons[RADIO_READER_TYPEA_212] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typea_212);
        mSettingsRadioButtons[RADIO_READER_TYPEA_424] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typea_424);
        mSettingsRadioButtons[RADIO_READER_TYPEA_848] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typea_848);
        mSettingsRadioButtons[RADIO_READER_TYPEB_106] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typeb_106);
        mSettingsRadioButtons[RADIO_READER_TYPEB_212] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typeb_212);
        mSettingsRadioButtons[RADIO_READER_TYPEB_424] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typeb_424);
        mSettingsRadioButtons[RADIO_READER_TYPEB_848] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typeb_848);
        mSettingsRadioButtons[RADIO_READER_TYPEF_212] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typef_212);
        mSettingsRadioButtons[RADIO_READER_TYPEF_424] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typef_424);
        mSettingsRadioButtons[RADIO_READER_TYPEV_166] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typev_166);
        mSettingsRadioButtons[RADIO_READER_TYPEV_2648] = (RadioButton) findViewById(R.id.hqa_readermode_rb_typev_2648);

        mRgTypeA = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typea);
        mRgTypeB = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typeb);
        mRgTypeF = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typef);
        mRgTypeVSubCarrier = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typev_subcarrier);
        mRgTypeVMode = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typev_mode);
        mRgTypeVRate = (RadioGroup) findViewById(R.id.hqa_readermode_rg_typev_rate);

        //mSettingsCkBoxs[CHECKBOX_READER_TYPEB2] = (CheckBox) findViewById(R.id.hqa_readermode_cb_typeb2);
        mSettingsCkBoxs[CHECKBOX_READER_KOVIO] = (CheckBox) findViewById(R.id.hqa_readermode_cb_kovio);
        mBtnSelectAll = (Button) findViewById(R.id.hqa_readermode_btn_select_all);
        mBtnSelectAll.setOnClickListener(mClickListener);
        mBtnClearAll = (Button) findViewById(R.id.hqa_readermode_btn_clear_all);
        mBtnClearAll.setOnClickListener(mClickListener);
        mBtnStart = (Button) findViewById(R.id.hqa_readermode_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_readermode_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnRunInBack = (Button) findViewById(R.id.hqa_readermode_btn_run_back);        
        mBtnRunInBack.setOnClickListener(mClickListener);
        mBtnRunInBack.setEnabled(false);
    }

    private void setButtonsStatus(boolean b) {
        if (b) {
            mBtnStart.setText(R.string.hqa_nfc_start);
        } else {
            mBtnStart.setText(R.string.hqa_nfc_stop);
        }
        mBtnRunInBack.setEnabled(!b);
        mEnableBackKey = b;
        mBtnReturn.setEnabled(b);
        mBtnSelectAll.setEnabled(b);
        mBtnClearAll.setEnabled(b);
    }

    private void changeAllSelect(boolean checked) {
        Elog.v(NfcMainPage.TAG, "[ReaderMode]changeDisplay status is " + checked);
        for (int i = CHECKBOX_READER_TYPEA; i < mSettingsCkBoxs.length; i++) {
            mSettingsCkBoxs[i].setChecked(checked);
        }
        if (checked) {
            mRgTypeA.check(R.id.hqa_readermode_rb_typea_106);
            mRgTypeB.check(R.id.hqa_readermode_rb_typeb_106);
            mRgTypeF.check(R.id.hqa_readermode_rb_typef_212);
            mRgTypeVSubCarrier.check(R.id.hqa_readermode_rb_typev_dual_subcarrier);
            mRgTypeVMode.check(R.id.hqa_readermode_rb_typev_mode_4);
            mRgTypeVRate.check(R.id.hqa_readermode_rb_typev_166);
        }
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmAlsReadermReq requestCmd = new NfcEmAlsReadermReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_ALS_READER_MODE_REQ, requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmAlsReadermReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
        }
        CheckBox[] typeBoxs = { mSettingsCkBoxs[CHECKBOX_READER_TYPEA],
                mSettingsCkBoxs[CHECKBOX_READER_TYPEB], mSettingsCkBoxs[CHECKBOX_READER_TYPEF],
                mSettingsCkBoxs[CHECKBOX_READER_TYPEV],
                mSettingsCkBoxs[CHECKBOX_READER_KOVIO] };
        requestCmd.mSupportType = BitMapValue.getTypeValue(typeBoxs);

        RadioButton[] readerADateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEA_106],
                mSettingsRadioButtons[RADIO_READER_TYPEA_212], mSettingsRadioButtons[RADIO_READER_TYPEA_424],
                mSettingsRadioButtons[RADIO_READER_TYPEA_848] };
        requestCmd.mTypeADataRate = BitMapValue.getTypeAbDataRateValue(readerADateRateBoxs);

        RadioButton[] readerBDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEB_106],
                mSettingsRadioButtons[RADIO_READER_TYPEB_212], mSettingsRadioButtons[RADIO_READER_TYPEB_424],
                mSettingsRadioButtons[RADIO_READER_TYPEB_848] };
        requestCmd.mTypeBDataRate = BitMapValue.getTypeAbDataRateValue(readerBDateRateBoxs);

        RadioButton[] readerFDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEF_212],
                mSettingsRadioButtons[RADIO_READER_TYPEF_424] };
        requestCmd.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(readerFDateRateBoxs);

        RadioButton[] readerVDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEV_166],
                mSettingsRadioButtons[RADIO_READER_TYPEV_2648] };
        requestCmd.mTypeVDataRate = BitMapValue.getTypeVDataRateValue(readerVDateRateBoxs);

        if (mRgTypeVSubCarrier.getCheckedRadioButtonId() == R.id.hqa_readermode_rb_typev_subcarrier) {
            requestCmd.mTypeVSubcarrier = 0;
        } else {
            requestCmd.mTypeVSubcarrier = 1;
        }

        if (mRgTypeVMode.getCheckedRadioButtonId() == R.id.hqa_readermode_rb_typev_mode_4) {
            requestCmd.mTypeVCodingMode = 0;
        } else {
            requestCmd.mTypeVCodingMode = 1;
        }
/*
        requestCmd.mTypeADataRate = BitMapValue.getTypeADataRateValue(mRgTypeA);
        requestCmd.mTypeBDataRate = BitMapValue.getTypeBDataRateValue(mRgTypeB);
        requestCmd.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(mRgTypeF);
        requestCmd.mTypeVDataRate = BitMapValue.getTypeVDataRateValue(mRgTypeVRate);
        requestCmd.mTypeVSubcarrier = BitMapValue.getTypeVSubcarrier(mRgTypeVSubCarrier);
        requestCmd.mTypeVCodingMode = BitMapValue.getTypeVCodingMode(mRgTypeVMode);
*/
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        ProgressDialog dialog = null;
        if (id == DIALOG_ID_WAIT) {
            dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.hqa_nfc_dialog_wait_message));
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setCancelable(false);
            return dialog;
        }
        return dialog;
    }

}
