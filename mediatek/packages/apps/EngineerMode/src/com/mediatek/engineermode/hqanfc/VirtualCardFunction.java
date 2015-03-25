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
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmVirtualCardReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmVirtualCardRsp;

import java.nio.ByteBuffer;

public class VirtualCardFunction extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private static final int CHECKBOXS_NUMBER = 5;
    private static final int CHECKBOX_TYPEA = 0;
    // private static final int CHECKBOX_TYPEA_106 = 1;
    // private static final int CHECKBOX_TYPEA_212 = 2;
    // private static final int CHECKBOX_TYPEA_424 = 3;
    // private static final int CHECKBOX_TYPEA_848 = 4;
    private static final int CHECKBOX_TYPEB = 1;
    // private static final int CHECKBOX_TYPEB_106 = 6;
    // private static final int CHECKBOX_TYPEB_212 = 7;
    // private static final int CHECKBOX_TYPEB_424 = 8;
    // private static final int CHECKBOX_TYPEB_848 = 9;
    private static final int CHECKBOX_TYPEF = 2;
    private static final int CHECKBOX_TYPEF_212 = 3;
    private static final int CHECKBOX_TYPEF_424 = 4;
   // private static final int CHECKBOX_TYPEB2 = 3;

    private static final int RADIO_TYPEF_212 = 0;
    private static final int RADIO_TYPEF_424 = 1;
    private static final int RADIO_NUMBER = 2;

    private CheckBox[] mSettingsCkBoxs = new CheckBox[CHECKBOXS_NUMBER];
    //private RadioButton[] mSettingsRadioButtons = new RadioButton[RADIO_NUMBER];
    //private RadioGroup mRgTypeF;
    private Button mBtnSelectAll;
    private Button mBtnClearAll;
    private Button mBtnStart;
    private Button mBtnReturn;
    private Button mBtnRunInBack;
    
    private NfcEmVirtualCardRsp mResponse;
    private byte[] mRspArray;
    private boolean mEnableBackKey = true;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_VIRTUAL_CARD_RSP).equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmVirtualCardRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {

        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (HANDLER_MSG_GET_RSP == msg.what) {
                dismissDialog(DIALOG_ID_WAIT);
                String toastMsg = null;
                switch (mResponse.mResult) {
                    case RspResult.SUCCESS:
                        toastMsg = "VirtualCardFunction Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(
                                VirtualCardFunction.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "VirtualCardFunction Rsp Result: FAIL";
                        break;
                    case RspResult.NFC_STATUS_REMOVE_SE:
                        toastMsg = "Please Remove SIM or uSD";
                        break;
                    default:
                        toastMsg = "VirtualCardFunction Rsp Result: ERROR";
                        break;
                }
                Toast.makeText(VirtualCardFunction.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final CheckBox.OnCheckedChangeListener mCheckedListener = new CheckBox.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
            Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]onCheckedChanged view is "
                    + buttonView.getText() + " value is " + checked);
            // if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_TYPEA])) {
            // for (int i = CHECKBOX_TYPEA_106; i < CHECKBOX_TYPEB; i++) {
            // mSettingsCkBoxs[i].setChecked(checked);
            // }
            // } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_TYPEB])) {
            // for (int i = CHECKBOX_TYPEB_106; i < CHECKBOX_TYPEF; i++) {
            // mSettingsCkBoxs[i].setChecked(checked);
            // }
            // } else
            if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_TYPEF])) {
               // for (int i = 0; i < mRgTypeF.getChildCount(); i++) {
               //     mRgTypeF.getChildAt(i).setEnabled(checked);
               // }
               mSettingsCkBoxs[CHECKBOX_TYPEF_212].setEnabled(checked);
               mSettingsCkBoxs[CHECKBOX_TYPEF_424].setEnabled(checked);
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                showDialog(DIALOG_ID_WAIT);
                doTestAction(mBtnStart.getText().equals(
                        VirtualCardFunction.this.getString(R.string.hqa_nfc_start)));
            } else if (arg0.equals(mBtnSelectAll)) {
                changeAllSelect(true);
            } else if (arg0.equals(mBtnClearAll)) {
                changeAllSelect(false);
            } else if (arg0.equals(mBtnReturn)) {
                VirtualCardFunction.this.onBackPressed();
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
        setContentView(R.layout.hqa_nfc_virtualcard_function);
        initComponents();
        changeAllSelect(true);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_VIRTUAL_CARD_RSP);
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
        Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]initComponents");
        mSettingsCkBoxs[CHECKBOX_TYPEA] = (CheckBox) findViewById(R.id.hqa_virtual_cb_typea);
        mSettingsCkBoxs[CHECKBOX_TYPEB] = (CheckBox) findViewById(R.id.hqa_virtual_cb_typeb);

        mSettingsCkBoxs[CHECKBOX_TYPEF] = (CheckBox) findViewById(R.id.hqa_virtual_cb_typef);
        mSettingsCkBoxs[CHECKBOX_TYPEF].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_TYPEF_212] = (CheckBox) findViewById(R.id.hqa_virtual_cb_typef_212);
        mSettingsCkBoxs[CHECKBOX_TYPEF_424] = (CheckBox) findViewById(R.id.hqa_virtual_cb_typef_424);

        //mSettingsRadioButtons[RADIO_TYPEF_212] = (RadioButton) findViewById(R.id.hqa_virtual_rb_typef_212);
        //mSettingsRadioButtons[RADIO_TYPEF_424] = (RadioButton) findViewById(R.id.hqa_virtual_rb_typef_424);

        //mRgTypeF = (RadioGroup) findViewById(R.id.hqa_virtual_rg_typef);

        mBtnSelectAll = (Button) findViewById(R.id.hqa_virtual_btn_select_all);
        mBtnSelectAll.setOnClickListener(mClickListener);
        mBtnClearAll = (Button) findViewById(R.id.hqa_virtual_btn_clear_all);
        mBtnClearAll.setOnClickListener(mClickListener);
        mBtnStart = (Button) findViewById(R.id.hqa_virtual_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_virtual_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnRunInBack = (Button) findViewById(R.id.hqa_virtual_btn_run_back);       
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
        Elog.v(NfcMainPage.TAG, "[VirtualCardFunction]changeAllSelect status is " + checked);
        for (int i = 0; i < mSettingsCkBoxs.length; i++) {
            mSettingsCkBoxs[i].setChecked(checked);
        }
        
        if (checked) {
            mSettingsCkBoxs[CHECKBOX_TYPEF_424].setChecked(false);
            //mRgTypeF.check(R.id.hqa_virtual_rb_typef_212);
        }
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmVirtualCardReq requestCmd = new NfcEmVirtualCardReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_VIRTUAL_CARD_REQ, requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmVirtualCardReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
        }
        int temp = 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TYPEA].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TYPEB].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_B : 0;
        temp |= mSettingsCkBoxs[CHECKBOX_TYPEF].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;

        requestCmd.mSupportType = temp;
        
        int rateVaule = 0;
        rateVaule |= mSettingsCkBoxs[CHECKBOX_TYPEF_212].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_212 : 0;
        rateVaule |= mSettingsCkBoxs[CHECKBOX_TYPEF_424].isChecked() ? NfcCommand.EM_ALS_READER_M_SPDRATE_424 : 0;
        
        requestCmd.mTypeFDataRate = rateVaule;
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
