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
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmLoopbackReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmLoopbackRsp;

import java.nio.ByteBuffer;

public class LoopBackTest extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private Button mBtnStart;
    private Button mBtnReturn;

    private NfcEmLoopbackRsp mResponse;
    private byte[] mRspArray;
    private boolean mEnableBackKey = true;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[LoopBackTest]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_EM_LOOPBACK_TEST_RSP)
                    .equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmLoopbackRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[LoopBackTest]Other response");
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
                        toastMsg = "LoopBackTest Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(
                                LoopBackTest.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "LoopBackTest Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "LoopBackTest Rsp Result: ERROR";
                        break;
                }
                Toast.makeText(LoopBackTest.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[LoopBackTest]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                showDialog(DIALOG_ID_WAIT);
                doTestAction(mBtnStart.getText().equals(
                        LoopBackTest.this.getString(R.string.hqa_nfc_start)));
            } else if (arg0.equals(mBtnReturn)) {
                LoopBackTest.this.onBackPressed();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_loopback_test);
        mBtnStart = (Button) findViewById(R.id.hqa_loopback_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_loopback_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_EM_LOOPBACK_TEST_RSP);
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

    private void setButtonsStatus(boolean b) {
        Elog.v(NfcMainPage.TAG, "[LoopBackTest]setButtonsStatus " + b);
        if (b) {
            mBtnStart.setText(R.string.hqa_nfc_start);
        } else {
            mBtnStart.setText(R.string.hqa_nfc_stop);
        }
        mEnableBackKey = b;
        mBtnReturn.setEnabled(b);
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmLoopbackReq requestCmd = new NfcEmLoopbackReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_EM_LOOPBACK_TEST_REQ,
                requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmLoopbackReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
        }
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
