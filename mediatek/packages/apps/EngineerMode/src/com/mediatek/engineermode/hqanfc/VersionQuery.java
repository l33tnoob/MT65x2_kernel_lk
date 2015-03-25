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
//import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmVersionReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmVersionRsp;

import java.nio.ByteBuffer;
import java.lang.Integer;

public class VersionQuery extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int DIALOG_ID_WAIT = 0;
    private static final String START_STR = "$PNFC";
    private static final String END_STR = "*";
    private Button mBtnReturn;
    private TextView mTvMwVersion;
    private TextView mTvFwVersion;
    private TextView mTvHwVersion;

    private NfcEmVersionRsp mResponse;
    private byte[] mRspArray;
    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[VersionQuery]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_SW_VERSION_RESPONSE).equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mResponse = new NfcEmVersionRsp();
                    mResponse.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[VersionQuery]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (HANDLER_MSG_GET_RSP == msg.what) {
                dismissDialog(DIALOG_ID_WAIT);
                String mwVersion = new String(mResponse.mMwVersion);
                String fwVersion = Integer.toHexString(mResponse.mFwVersion);
                String hwVersion = Integer.toHexString(mResponse.mHwVersion);
                Elog.v(NfcMainPage.TAG, mwVersion);
                Elog.v(NfcMainPage.TAG, String.valueOf(mResponse.mFwVersion));
                Elog.v(NfcMainPage.TAG, String.valueOf(mResponse.mHwVersion));
                mTvMwVersion.setText(VersionQuery.this.getString(R.string.hqa_nfc_mw_version) + mwVersion);
                mTvFwVersion.setText(VersionQuery.this.getString(R.string.hqa_nfc_fw_version) + fwVersion);
                mTvHwVersion.setText(VersionQuery.this.getString(R.string.hqa_nfc_hw_version) + hwVersion);
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[VersionQuery]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnReturn)) {
                VersionQuery.this.onBackPressed();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_version_query);
        mBtnReturn = (Button) findViewById(R.id.hqa_version_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mTvMwVersion = (TextView) findViewById(R.id.hqa_version_tv_mw_version);
        mTvFwVersion = (TextView) findViewById(R.id.hqa_version_tv_fw_version);
        mTvHwVersion = (TextView) findViewById(R.id.hqa_version_tv_hw_version);

        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_SW_VERSION_RESPONSE);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onResume() {
        super.onResume();
//        mTvMwVersion.setText("");
//        mTvFwVersion.setText("");
//        mTvHwVersion.setText("");
        showDialog(DIALOG_ID_WAIT);
        sendCommand();
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
//        NfcEmPnfcReq requestCmd = new NfcEmPnfcReq();
//        fillRequest(requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_SW_VERSION_QUERY, null);
    }

//    private void fillRequest(NfcEmPnfcReq requestCmd) {
//        requestCmd.mAction = EmAction.ACTION_START;
//        String temp = START_STR + mEtPnfc.getText() + END_STR;
//        byte[] cmdArray = temp.getBytes();
//        System.arraycopy(cmdArray, 0, requestCmd.mData, 0, cmdArray.length);
//        requestCmd.mDataLen = cmdArray.length;
//    }
}
