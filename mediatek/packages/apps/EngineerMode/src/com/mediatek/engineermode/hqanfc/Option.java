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
import android.widget.EditText;
import android.widget.Toast;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmOptionReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmOptionRsp;

import java.nio.ByteBuffer;

public class Option extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private CheckBox mCbForceDownload;
    private CheckBox mCbAutoCheck;
    private Button mBtnReturn;
    private Button mBtnSet;
    private NfcEmOptionRsp mResponse;
    private byte[] mRspArray;
    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[Option]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_TESTMODE_SETTING_RSP).equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmOptionRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[Option]Other response");
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
                        toastMsg = "Option Rsp Result: SUCCESS";
                        break;
                    case RspResult.FAIL:
                        toastMsg = "Option Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "Option Rsp Result: ERROR";
                        break;
                }
                Toast.makeText(Option.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[Option]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnSet)) {
                showDialog(DIALOG_ID_WAIT);
                sendCommand();
            } else if (arg0.equals(mBtnReturn)) {
                Option.this.onBackPressed();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_option);
        mBtnReturn = (Button) findViewById(R.id.hqa_option_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnSet = (Button) findViewById(R.id.hqa_option_btn_set);
        mBtnSet.setOnClickListener(mClickListener);
        mCbForceDownload = (CheckBox) findViewById(R.id.hqa_option_force_download);
        mCbAutoCheck = (CheckBox) findViewById(R.id.hqa_option_auto_check);
        mCbAutoCheck.setChecked(true);
        
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_TESTMODE_SETTING_RSP);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
//        if (!mEnableBackKey) {
//            return;
//        }
        super.onBackPressed();
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

    private void sendCommand() {
        NfcEmOptionReq requestCmd = new NfcEmOptionReq();
        fillRequest(requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_TESTMODE_SETTING_REQ, requestCmd);
    }

    private void fillRequest(NfcEmOptionReq requestCmd) {
        requestCmd.mForceDownload = mCbForceDownload.isChecked() ? (short) 1 : (short) 0;
        requestCmd.mAutoCheck = mCbAutoCheck.isChecked() ? (short) 1 : (short) 0;
    }
}
