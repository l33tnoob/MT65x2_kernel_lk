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
import android.widget.EditText;
import android.widget.Toast;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPnfcReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPnfcRsp;

import java.nio.ByteBuffer;

public class PnfcCommand extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private static final String START_STR = "$PNFC";
    private static final String END_STR = "*";
    private Button mBtnReturn;
    private Button mBtnSend;
    private EditText mEtPnfc;
    private NfcEmPnfcRsp mResponse;
    private byte[] mRspArray;
    private TextView mResultTv;
    
//    private boolean mEnableBackKey = true;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[PnfcCommand]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_PNFC_CMD_RSP).equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmPnfcRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[PnfcCommand]Other response");
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
                        toastMsg = "PnfcCommand Rsp Result: SUCCESS";
                        break;
                    case RspResult.FAIL:
                        toastMsg = "PnfcCommand Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "PnfcCommand Rsp Result: ERROR";
                        break;
                }
                String infoMsg = new String(mResponse.mData);
                Elog.v(NfcMainPage.TAG, toastMsg);
                Elog.v(NfcMainPage.TAG, infoMsg);
                mResultTv.setText(toastMsg+"\n"+infoMsg);
                
                //Toast.makeText(PnfcCommand.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[PnfcCommand]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnSend)) {
                mResultTv.setText("");
                showDialog(DIALOG_ID_WAIT);
                sendCommand();
            } else if (arg0.equals(mBtnReturn)) {
                PnfcCommand.this.onBackPressed();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_pnfc_command);
        mBtnReturn = (Button) findViewById(R.id.hqa_pnfc_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnSend = (Button) findViewById(R.id.hqa_pnfc_btn_send);
        mBtnSend.setOnClickListener(mClickListener);
        mEtPnfc = (EditText) findViewById(R.id.hqa_pnfc_et_pnfc);
        mEtPnfc.setSelection(0);
        mResultTv = (TextView) findViewById(R.id.hqa_pnfc_tv_reslut);
        
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_PNFC_CMD_RSP);
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
        NfcEmPnfcReq requestCmd = new NfcEmPnfcReq();
        fillRequest(requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_PNFC_CMD_REQ, requestCmd);
    }

    private void fillRequest(NfcEmPnfcReq requestCmd) {
        requestCmd.mAction = EmAction.ACTION_START;
        String temp = START_STR + mEtPnfc.getText() + END_STR;
        byte[] cmdArray = temp.getBytes();
        System.arraycopy(cmdArray, 0, requestCmd.mData, 0, cmdArray.length);
        requestCmd.mDataLen = cmdArray.length;
    }
}
