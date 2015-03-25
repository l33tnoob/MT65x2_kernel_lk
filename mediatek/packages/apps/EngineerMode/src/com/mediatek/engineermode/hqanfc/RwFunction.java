package com.mediatek.engineermode.hqanfc;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.DataConvert;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmOptAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.ReaderModeRspResult;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermNtf;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptRsp;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPollingNty;

import java.nio.ByteBuffer;

public class RwFunction extends Activity {

    protected static final int HANDLER_MSG_GET_RSP = 300;
    private static final int HANDLER_MSG_GET_NTF = 100;
    private static final int HANDLER_MSG_GET_POLLING_NTF = 200;
    private TextView mTvUid;
    private Button mBtnRead;
    private Button mBtnWrite;
    private Button mBtnFormat;
    //private byte[] mReadermRspArray;
    //private NfcEmAlsReadermNtf mTransferReadermNtf;
    private NfcEmAlsReadermNtf mReceivedReadermNtf;
    private byte[] mRspArray;
    private NfcEmAlsReadermOptRsp mOptRsp;
    private boolean mUnregisterReceiver = false;
    private NfcEmPollingNty mPollingNty;
    
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[RwFunction]mReceiver onReceive");
            String action = intent.getAction();
            mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP)
                    .equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mOptRsp = new NfcEmAlsReadermOptRsp();
                    mOptRsp.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_NTF)
                    .equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                   // mReceivedReadermNtf = new NfcEmAlsReadermNtf();
                    mReceivedReadermNtf.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_NTF);
                }
            } else if((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_NTF)
             .equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mPollingNty = new NfcEmPollingNty();
                    mPollingNty.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_POLLING_NTF);
                }
            }else {
                Elog.v(NfcMainPage.TAG, "[RwFunction]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            String toastMsg = null;
            if (HANDLER_MSG_GET_RSP == msg.what) {
                switch (mOptRsp.mResult) {
                    case RspResult.SUCCESS:
                        toastMsg = "Rw Format Rsp Result: SUCCESS";
                        mBtnFormat.setEnabled(false);
                        mBtnRead.setEnabled(true);
                        mBtnWrite.setEnabled(true);
                        break;
                    case RspResult.FAIL:
                        toastMsg = "Rw Format Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "Rw Format Rsp Result: ERROR";
                        break;
                }

            } else if (HANDLER_MSG_GET_NTF == msg.what) {
                switch (mReceivedReadermNtf.mResult) {
                    case ReaderModeRspResult.CONNECT:
                        toastMsg = "ReaderMode Ntf Result: CONNECT";
                        if (mReceivedReadermNtf.mIsNdef == 0 || mReceivedReadermNtf.mIsNdef == 1
                                || mReceivedReadermNtf.mIsNdef == 2) {
                            if (null == mRspArray) {
                                toastMsg = "Not get the response";
                                RwFunction.this.finish();
                            } else {
                                updateButtonUI(mReceivedReadermNtf.mIsNdef);
                                updateUid();
                            }
                        }
                        break;
                    case ReaderModeRspResult.DISCONNECT:
                        toastMsg = "ReaderMode Ntf Result: DISCONNECT";
                        //mUnregisterReceiver = true;
                        //unregisterReceiver(mReceiver);
                        //RwFunction.this.finish();
                        updateButtonUI(4);
                        mTvUid
                            .setText("Tag disconnect...and re-polling...");
                        break;
                    case ReaderModeRspResult.FAIL:
                        toastMsg = "ReaderMode Ntf Result: FAIL";
                        if (null == mRspArray) {
                            toastMsg = "Not get the response";
                            RwFunction.this.finish();
                        } else {
                            updateButtonUI(3);
                            updateUid();
                        }
                        break;
                    default:
                        toastMsg = "Tag is not NDEF format";
                        if (null == mRspArray) {
                            toastMsg = "Not get the response";
                            RwFunction.this.finish();
                        } else {
                            updateButtonUI(3);
                            updateUid();
                        }
                        break;
                }
            } else if (HANDLER_MSG_GET_POLLING_NTF == msg.what) {
                 switch (mPollingNty.mDetectType) {
                    case NfcCommand.EM_ENABLE_FUNC_READER_MODE:
                        // show RW function ui
                        mReceivedReadermNtf.readRaw(ByteBuffer.wrap(mPollingNty.mData));
                        this.sendEmptyMessage(HANDLER_MSG_GET_NTF);
                        break;
                    default:
                         toastMsg = "Please back to polling loop mode screen";
                        break;
                }
            }
            Elog.v(NfcMainPage.TAG, "[RwFunction]"+toastMsg);
            if(toastMsg != null) {
                Toast.makeText(RwFunction.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[RwFunction]onClick button view is "
                    + ((Button) arg0).getText());
            Intent intent = new Intent();
            if (arg0.equals(mBtnRead)) {
                intent.setClass(RwFunction.this, FunctionRead.class);
                intent.putExtra(FunctionRead.PARENT_EXTRA_STR, 0);
                startActivity(intent);
            } else if (arg0.equals(mBtnWrite)) {
                intent.setClass(RwFunction.this, FunctionWrite.class);
                startActivity(intent);
            } else if (arg0.equals(mBtnFormat)) {
                doFormat();
            } else {
                Elog.v(NfcMainPage.TAG, "[RwFunction]onClick noting.");
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_rw_function);
        mTvUid = (TextView) findViewById(R.id.hqa_nfc_rw_tv_uid);
        mBtnRead = (Button) findViewById(R.id.hqa_nfc_rw_btn_read);
        mBtnWrite = (Button) findViewById(R.id.hqa_nfc_rw_btn_write);
        mBtnFormat = (Button) findViewById(R.id.hqa_nfc_rw_btn_format);
        mBtnRead.setOnClickListener(mClickListener);
        mBtnWrite.setOnClickListener(mClickListener);
        mBtnFormat.setOnClickListener(mClickListener);
        Intent intent = getIntent();
        int ndefType = intent.getIntExtra(ReaderMode.KEY_READER_MODE_RSP_NDEF, 1);
        updateButtonUI(ndefType);
        mRspArray = intent.getByteArrayExtra(ReaderMode.KEY_READER_MODE_RSP_ARRAY);
        if (null == mRspArray) {
            Toast.makeText(this, "Not get the response", Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        if (null == mReceivedReadermNtf) {
            mReceivedReadermNtf = new NfcEmAlsReadermNtf();
        }
        mReceivedReadermNtf.readRaw(ByteBuffer.wrap(mRspArray));
        mHandler.sendEmptyMessage(HANDLER_MSG_GET_NTF); // update button UI
    }
    
    private void updateButtonUI(int ndefType) {
        mBtnRead.setVisibility(View.VISIBLE);
        mBtnFormat.setVisibility(View.VISIBLE);
        mBtnWrite.setVisibility(View.VISIBLE);
        if (ndefType == 1) { // ndef
            mBtnRead.setEnabled(true);
            mBtnWrite.setEnabled(true);
            mBtnFormat.setEnabled(false);
        } else if (ndefType == 0) { // format
            mBtnFormat.setEnabled(true);
            mBtnRead.setEnabled(false);
            mBtnWrite.setEnabled(false);
        } else if (ndefType == 2) { // read function only
            mBtnRead.setEnabled(true);
            mBtnFormat.setEnabled(false);
            mBtnWrite.setEnabled(false);
        } else if (ndefType == 3) { // result fail case
            mBtnRead.setEnabled(false);
            mBtnFormat.setEnabled(false);
            mBtnWrite.setEnabled(false);
            Elog.v(NfcMainPage.TAG, "ReaderModeRspResult.FAIL, disabe all buttons");
        } else if (ndefType == 4) { // result fail case;
            mBtnRead.setVisibility(View.GONE);
            mBtnFormat.setVisibility(View.GONE);
            mBtnWrite.setVisibility(View.GONE);
            Elog.v(NfcMainPage.TAG, "ReaderModeRspResult.DISCONNECT, Hide all buttons");
        }
    }

    private void updateUid() {
        mTvUid
                .setText("UID: "
                        + DataConvert.printHexString(mReceivedReadermNtf.mUid,
                               mReceivedReadermNtf.mUidLen));
    }
    @Override
    protected void onStart() {
        Elog.v(NfcMainPage.TAG, "[RwFunction]onStart");
        super.onStart();
        updateUid();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP);
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_NTF); // for reader mode ntf
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_NTF);  // for polling mode ntf
        registerReceiver(mReceiver, filter);
        mUnregisterReceiver = false;
    }

    @Override
    protected void onStop() {
        Elog.v(NfcMainPage.TAG, "[RwFunction]onStop");
        if(mUnregisterReceiver == false) {
            unregisterReceiver(mReceiver);
        }
        super.onStop();
    }
    @Override
    public void onBackPressed() {
        Elog.v(NfcMainPage.TAG, "[RwFunction]onBackPressed" );
        mUnregisterReceiver = true;
        unregisterReceiver(mReceiver);
        super.onBackPressed();
    }
    @Override
    protected void onDestroy() {
        Elog.v(NfcMainPage.TAG, "[RwFunction]onDestroy" );
        super.onDestroy();
    }

    private void doFormat() {
        NfcEmAlsReadermOptReq request = new NfcEmAlsReadermOptReq();
        request.mAction = EmOptAction.FORMAT;
        NfcClient.getInstance()
                .sendCommand(CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_REQ, request);
    }

}
