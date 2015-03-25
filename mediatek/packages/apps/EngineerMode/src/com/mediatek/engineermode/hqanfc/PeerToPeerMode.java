package com.mediatek.engineermode.hqanfc;

import android.app.Activity;
import android.app.AlertDialog;
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
import com.mediatek.engineermode.hqanfc.NfcCommand.P2pDisableCardM;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsP2pNtf;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsP2pReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsP2pRsp;

import java.nio.ByteBuffer;

public class PeerToPeerMode extends Activity {

    protected static final String KEY_P2P_RSP_ARRAY = "p2p_rsp_array";
    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int HANDLER_MSG_GET_NTF = 201;
    private static final int DIALOG_ID_RESULT = 0;
    private static final int DIALOG_ID_WAIT = 1;

    private static final int CHECKBOX_TYPEA = 0;
    private static final int CHECKBOX_TYPEF = 1;
    private static final int CHECKBOX_PASSIVE_MODE = 2;
    private static final int CHECKBOX_ACTIVE_MODE = 3;
    private static final int CHECKBOX_INITIATOR = 4;
    private static final int CHECKBOX_TARGET = 5;
    private static final int CHECKBOX_DISABLE_CARD = 6;
    private static final int CHECKBOX_NUMBER = 7;

    private static final int RADIO_P2P_TYPEA_106 = 0;
    private static final int RADIO_P2P_TYPEA_212 = 1;
    private static final int RADIO_P2P_TYPEA_424 = 2;
    private static final int RADIO_P2P_TYPEA_848 = 3;
    private static final int RADIO_P2P_TYPEF_212 = 4;
    private static final int RADIO_P2P_TYPEF_424 = 5;
    private static final int RADIO_NUMBER = 6;

    private CheckBox[] mSettingsCkBoxs = new CheckBox[CHECKBOX_NUMBER];
    private RadioButton[] mSettingsRadioButtons = new RadioButton[RADIO_NUMBER];
    private RadioGroup mRgTypeA;
    private RadioGroup mRgTypeF;
    private Button mBtnSelectAll;
    private Button mBtnClearAll;
    private Button mBtnStart;
    private Button mBtnReturn;
    private Button mBtnRunInBack;
    
    private AlertDialog mAlertDialog = null;

    private NfcEmAlsP2pNtf mP2pNtf;
    private NfcEmAlsP2pRsp mP2pRsp;
    private byte[] mRspArray;
    private String mNtfContent;
    private boolean mEnableBackKey = true;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]mReceiver onReceive: " + action);
            mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_P2P_MODE_NTF).equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mP2pNtf = new NfcEmAlsP2pNtf();
                    mP2pNtf.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_NTF);
                }
            } else if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_P2P_MODE_RSP).equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mP2pRsp = new NfcEmAlsP2pRsp();
                    mP2pRsp.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            String toastMsg = null;
            if (HANDLER_MSG_GET_NTF == msg.what) {
                switch (mP2pNtf.mResult) {
                    case RspResult.NFC_STATUS_LINK_UP:
                        showDialog(DIALOG_ID_RESULT);
                        break;
                    case RspResult.NFC_STATUS_LINK_DOWN:
                        if (mAlertDialog != null) {
                            mAlertDialog.dismiss();
                        }
                        break;
                    default:
                        toastMsg = "P2P Data Exchange is ERROR";
                        break;
                }
            } else if (HANDLER_MSG_GET_RSP == msg.what) {
                dismissDialog(DIALOG_ID_WAIT);
                switch (mP2pRsp.mResult) {
                    case RspResult.SUCCESS:
                        toastMsg = "P2P Mode Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(PeerToPeerMode.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "P2P Mode Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "P2P Mode Rsp Result: ERROR";
                        break;
                }
            }
            Toast.makeText(PeerToPeerMode.this, toastMsg, Toast.LENGTH_SHORT).show();
        }
    };

    private final CheckBox.OnCheckedChangeListener mCheckedListener = new CheckBox.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
            Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]onCheckedChanged view is " + buttonView.getText() + " value is "
                    + checked);
            if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_TYPEA])) {
                for (int i = 0; i < mRgTypeA.getChildCount(); i++) {
                    mRgTypeA.getChildAt(i).setEnabled(checked);
                }
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_TYPEF])) {
                for (int i = 0; i < mRgTypeF.getChildCount(); i++) {
                    mRgTypeF.getChildAt(i).setEnabled(checked);
                }
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]onClick button view is " + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                if (!checkRoleSelect()) {
                    Toast.makeText(PeerToPeerMode.this, R.string.hqa_nfc_p2p_role_tip, Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_ID_WAIT);
                    doTestAction(mBtnStart.getText().equals(PeerToPeerMode.this.getString(R.string.hqa_nfc_start)));
                }
            } else if (arg0.equals(mBtnSelectAll)) {
                changeAllSelect(true);
            } else if (arg0.equals(mBtnClearAll)) {
                changeAllSelect(false);
            } else if (arg0.equals(mBtnReturn)) {
                PeerToPeerMode.this.onBackPressed();
            } else if (arg0.equals(mBtnRunInBack)) {                
                doTestAction(null);                
                Intent intent = new Intent(Intent.ACTION_MAIN);                
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);                
                intent.addCategory(Intent.CATEGORY_HOME);                
                startActivity(intent);            
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_p2p_mode);
        initComponents();
        changeAllSelect(true);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_P2P_MODE_RSP);
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_P2P_MODE_NTF);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onDestroy() {
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
        Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]initComponents");
        mSettingsCkBoxs[CHECKBOX_TYPEA] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_typea);
        mSettingsCkBoxs[CHECKBOX_TYPEA].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_TYPEF] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_typef);
        mSettingsCkBoxs[CHECKBOX_TYPEF].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_PASSIVE_MODE] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_passive_mode);
        mSettingsCkBoxs[CHECKBOX_ACTIVE_MODE] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_active_mode);
        mSettingsCkBoxs[CHECKBOX_INITIATOR] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_initiator);
        mSettingsCkBoxs[CHECKBOX_TARGET] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_target);
        mSettingsCkBoxs[CHECKBOX_DISABLE_CARD] = (CheckBox) findViewById(R.id.hqa_p2pmode_cb_disable_card_emul);

        mSettingsRadioButtons[RADIO_P2P_TYPEA_106] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typea_106);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_212] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typea_212);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_424] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typea_424);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_848] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typea_848);
        mSettingsRadioButtons[RADIO_P2P_TYPEF_212] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typef_212);
        mSettingsRadioButtons[RADIO_P2P_TYPEF_424] = (RadioButton) findViewById(R.id.hqa_p2pmode_rb_typef_424);

        mRgTypeA = (RadioGroup) findViewById(R.id.hqa_p2pmode_rg_typea);
        mRgTypeF = (RadioGroup) findViewById(R.id.hqa_p2pmode_rg_typef);

        mBtnSelectAll = (Button) findViewById(R.id.hqa_p2pmode_btn_select_all);
        mBtnSelectAll.setOnClickListener(mClickListener);
        mBtnClearAll = (Button) findViewById(R.id.hqa_p2pmode_btn_clear_all);
        mBtnClearAll.setOnClickListener(mClickListener);
        mBtnStart = (Button) findViewById(R.id.hqa_p2pmode_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_p2pmode_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnRunInBack = (Button) findViewById(R.id.hqa_p2pmode_btn_run_back);        
        mBtnRunInBack.setOnClickListener(mClickListener);        
        mBtnRunInBack.setEnabled(false);
    }

    private void setButtonsStatus(boolean b) {
        if (b) {
            mBtnStart.setText(R.string.hqa_nfc_start);
        } else {
            mBtnStart.setText(R.string.hqa_nfc_stop);
        }
        mEnableBackKey = b;
        mBtnRunInBack.setEnabled(!b);
        mBtnReturn.setEnabled(b);
        mBtnSelectAll.setEnabled(b);
        mBtnClearAll.setEnabled(b);
    }

    private void changeAllSelect(boolean checked) {
        Elog.v(NfcMainPage.TAG, "[PeerToPeerMode]changeAllSelect status is " + checked);
        for (int i = CHECKBOX_TYPEA; i < mSettingsCkBoxs.length; i++) {
            mSettingsCkBoxs[i].setChecked(checked);
        }
        mSettingsCkBoxs[CHECKBOX_DISABLE_CARD].setChecked(false);
        if (checked) {
            mRgTypeA.check(R.id.hqa_p2pmode_rb_typea_106);
            mRgTypeF.check(R.id.hqa_p2pmode_rb_typef_212);
        }
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmAlsP2pReq requestCmd = new NfcEmAlsP2pReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_ALS_P2P_MODE_REQ, requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmAlsP2pReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
        }
        int temp = 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TYPEA].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TYPEF].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;
        requestCmd.mSupportType = temp;

        RadioButton[] typeADateRateBoxs = { mSettingsRadioButtons[RADIO_P2P_TYPEA_106], mSettingsRadioButtons[RADIO_P2P_TYPEA_212],
                mSettingsRadioButtons[RADIO_P2P_TYPEA_424], mSettingsRadioButtons[RADIO_P2P_TYPEA_848] };
        requestCmd.mTypeADataRate = BitMapValue.getTypeAbDataRateValue(typeADateRateBoxs);

        RadioButton[] typeFDateRateBoxs = { mSettingsRadioButtons[RADIO_P2P_TYPEF_212], mSettingsRadioButtons[RADIO_P2P_TYPEF_424] };
        requestCmd.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(typeFDateRateBoxs);

//        requestCmd.mTypeADataRate = BitMapValue.getTypeADataRateValue(mRgTypeA);
//        requestCmd.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(mRgTypeF);
        requestCmd.mIsDisableCardM = mSettingsCkBoxs[CHECKBOX_DISABLE_CARD].isChecked() ? P2pDisableCardM.DISABLE
                : P2pDisableCardM.NOT_DISABLE;
        temp = 0;
        temp |= mSettingsCkBoxs[CHECKBOX_INITIATOR].isChecked() ? NfcCommand.EM_P2P_ROLE_INITIATOR_MODE : 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TARGET].isChecked() ? NfcCommand.EM_P2P_ROLE_TARGET_MODE : 0;
        requestCmd.mRole = temp;
        temp = 0;
        temp |= mSettingsCkBoxs[CHECKBOX_PASSIVE_MODE].isChecked() ? NfcCommand.EM_P2P_MODE_PASSIVE_MODE : 0;
        temp |= mSettingsCkBoxs[CHECKBOX_ACTIVE_MODE].isChecked() ? NfcCommand.EM_P2P_MODE_ACTIVE_MODE : 0;
        requestCmd.mMode = temp;
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (DIALOG_ID_WAIT == id) {
            ProgressDialog dialog = null;
            dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.hqa_nfc_dialog_wait_message));
            dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
            dialog.setCancelable(false);
            return dialog;
        } else if (DIALOG_ID_RESULT == id) {
            mAlertDialog = new AlertDialog.Builder(PeerToPeerMode.this).setTitle(R.string.hqa_nfc_p2p_mode_ntf_title)
                    .setMessage("P2P link is up").setPositiveButton(android.R.string.ok, null).create();
            return mAlertDialog;
        }
        return null;
    }

    private boolean checkRoleSelect() {
        boolean result = true;
        if (!mSettingsCkBoxs[CHECKBOX_INITIATOR].isChecked() && !mSettingsCkBoxs[CHECKBOX_TARGET].isChecked()) {
            result = false;
        }
        if (!mSettingsCkBoxs[CHECKBOX_PASSIVE_MODE].isChecked() && !mSettingsCkBoxs[CHECKBOX_ACTIVE_MODE].isChecked()) {
            result = false;
        }
        return result;
    }
}
