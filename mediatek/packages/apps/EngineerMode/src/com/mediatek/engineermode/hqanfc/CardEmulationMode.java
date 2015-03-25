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
import android.widget.RadioGroup;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsCardmReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsCardmRsp;

import java.nio.ByteBuffer;

public class CardEmulationMode extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private RadioGroup mRgSwio;

    private CheckBox mCbTypeA;
    private CheckBox mCbTypeB;
    private CheckBox mCbTypeF;

    private Button mBtnSelectAll;
    private Button mBtnClearAll;
    private Button mBtnStart;
    private Button mBtnReturn;
    private Button mBtnRunInBack;
    
    private NfcEmAlsCardmRsp mResponse;
    private byte[] mRspArray;
    private boolean mEnableBackKey = true;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_CARD_MODE_RSP).equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmAlsCardmRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "Other response");
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
                        toastMsg = "CardEmulation Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(
                                CardEmulationMode.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "CardEmulation Rsp Result: FAIL";
                        break;
                    case RspResult.NFC_STATUS_NO_SIM:
                        toastMsg = "CardEmulation Rsp Result: No SIM";
                        break;
                    default:
                        toastMsg = "CardEmulation Rsp Result: ERROR";
                        break;
                }
                Toast.makeText(CardEmulationMode.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "onClick button view is " + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                showDialog(DIALOG_ID_WAIT);
                doTestAction(mBtnStart.getText().equals(
                        CardEmulationMode.this.getString(R.string.hqa_nfc_start)));
            } else if (arg0.equals(mBtnSelectAll)) {
                changeAllSelect(true);
            } else if (arg0.equals(mBtnClearAll)) {
                changeAllSelect(false);
            } else if (arg0.equals(mBtnReturn)) {
                CardEmulationMode.this.onBackPressed();
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
        setContentView(R.layout.hqa_nfc_card_emulation_mode);
        initComponents();
        changeAllSelect(true);
      //  mCbVirtualCardFunct.setChecked(false);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_CARD_MODE_RSP);
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
        Elog.v(NfcMainPage.TAG, "initComponents");
        mRgSwio = (RadioGroup) findViewById(R.id.hqa_cardmode_rg_swio);

        mCbTypeA = (CheckBox) findViewById(R.id.hqa_cardmode_cb_typea);
        mCbTypeB = (CheckBox) findViewById(R.id.hqa_cardmode_cb_typeb);
        mCbTypeF = (CheckBox) findViewById(R.id.hqa_cardmode_cb_typef);

       // mCbVirtualCardFunct = (CheckBox) findViewById(R.id.hqa_virtual_card);
        mBtnSelectAll = (Button) findViewById(R.id.hqa_cardmode_btn_select_all);
        mBtnSelectAll.setOnClickListener(mClickListener);
        mBtnClearAll = (Button) findViewById(R.id.hqa_cardmode_btn_clear_all);
        mBtnClearAll.setOnClickListener(mClickListener);
        mBtnStart = (Button) findViewById(R.id.hqa_cardmode_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_cardmode_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnRunInBack = (Button) findViewById(R.id.hqa_cardmode_btn_run_back);       
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
        Elog.v(NfcMainPage.TAG, "changeDisplay status is " + checked);
        if (checked) {
            mRgSwio.check(R.id.hqa_cardmode_rb_swio1);
        }

        mCbTypeA.setChecked(checked);
        mCbTypeB.setChecked(checked);
        mCbTypeF.setChecked(checked);


       // mCbVirtualCardFunct.setChecked(false);
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmAlsCardmReq requestCmd = new NfcEmAlsCardmReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_ALS_CARD_MODE_REQ, requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmAlsCardmReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
        }
        int temp = 0;

        temp |= mCbTypeA.isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
        temp |= mCbTypeB.isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_B : 0;
        temp |= mCbTypeF.isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;

        requestCmd.mSupportType = temp;
        temp = 0;
        switch (mRgSwio.getCheckedRadioButtonId()) {
        case R.id.hqa_cardmode_rb_swio1:
            temp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO1;
            break;
        case R.id.hqa_cardmode_rb_swio2:
            temp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO2;
            break;
        case R.id.hqa_cardmode_rb_swio_se:
            temp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIOSE;
            break;
        default:
            break;
        }
/*
        temp |= mCbSwio1.isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO1 : 0;
        temp |= mCbSwio2.isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO2 : 0;
        temp |= mCbSwioSe.isChecked() ? NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIOSE : 0;
*/
        requestCmd.mSwNum = temp;
        requestCmd.mFgVirtualCard = 0;//mCbVirtualCardFunct.isChecked() ? 1 : 0;
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
