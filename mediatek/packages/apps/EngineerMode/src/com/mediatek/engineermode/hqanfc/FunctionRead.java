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
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;
import android.text.InputType;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.DataConvert;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmOptAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptRsp;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcNdefType;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcTagReadNdef;

import java.nio.ByteBuffer;

public class FunctionRead extends Activity {

    protected static final String PARENT_EXTRA_STR = "parent_ui_id";
    protected static final String BYTE_EXTRA_STR = "byte_data";
    protected static final int HANDLER_MSG_GET_RSP = 300;

    private RadioGroup mRgTagType;
//    private RadioButton mRbTypeUri;
//    private RadioButton mRbTypeText;
//    private RadioButton mRbTypeSmart;
    private RadioButton mRbTypeOthers;
    private EditText mTvLang;
    private EditText mTvRecordFlag;
    private EditText mTvRecordId;
    private EditText mTvRecordInf;
    private EditText mTvPayloadLength;
    private EditText mTvPayloadHex;
    private EditText mTvPayloadAscii;
    private Button mBtnRead;
    private Button mBtnCancel;

    private NfcEmAlsReadermOptRsp mOptRsp;
    private byte[] mRspArray;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[FunctionRead]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP)
                    .equals(action)) {
                mRspArray = intent.getExtras().getByteArray(
                        NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mOptRsp = new NfcEmAlsReadermOptRsp();
                    mOptRsp.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[FunctionRead]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (HANDLER_MSG_GET_RSP == msg.what) {
                String toastMsg = null;
                switch (mOptRsp.mResult) {
                case RspResult.SUCCESS:
                    toastMsg = "Function Read Rsp Result: SUCCESS";
                    updateUi(mOptRsp.mTagReadNdef);
                    break;
                case RspResult.FAIL:
                    toastMsg = "Function Read Rsp Result: FAIL";
                    break;
                default:
                    toastMsg = "Function Read Rsp Result: ERROR";
                    break;
                }
                Toast.makeText(FunctionRead.this, toastMsg, Toast.LENGTH_SHORT)
                        .show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[FunctionRead]onClick button view is " + ((Button) arg0).getText());
            if (arg0.equals(mBtnRead)) {
                doRead();
            } else if (arg0.equals(mBtnCancel)) {
                FunctionRead.this.onBackPressed();
            } else {
                Elog.v(NfcMainPage.TAG, "[FunctionRead]onClick noting.");
            }
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_function_read);
        initComponents();
        int parentId = getIntent().getIntExtra(PARENT_EXTRA_STR, 0);
        if (1 == parentId) {
            mBtnRead.setEnabled(false); // just show the result 
            byte[] optData = getIntent().getByteArrayExtra(BYTE_EXTRA_STR);
            NfcEmAlsReadermOptRsp optRsp = new NfcEmAlsReadermOptRsp();
            optRsp.readRaw(ByteBuffer.wrap(optData));
            updateUi(optRsp.mTagReadNdef);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE
                + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onPause() {
        unregisterReceiver(mReceiver);
        super.onPause();
    }

    private void initComponents() {
        mRgTagType = (RadioGroup) findViewById(R.id.hqa_read_rg_tag_type);
//        mRbTypeUri = (RadioButton) findViewById(R.id.hqa_read_rb_type_uri);
//        mRbTypeText = (RadioButton) findViewById(R.id.hqa_read_rb_type_text);
//        mRbTypeSmart = (RadioButton) findViewById(R.id.hqa_read_rb_type_smart);
        mRbTypeOthers = (RadioButton) findViewById(R.id.hqa_read_rb_type_others);
        mRbTypeOthers.setVisibility(View.GONE);
        mTvLang = (EditText) findViewById(R.id.hqa_read_tv_lang);
        mTvLang.setInputType(InputType.TYPE_NULL);
        mTvRecordFlag = (EditText) findViewById(R.id.hqa_read_tv_flag);
        mTvRecordFlag.setInputType(InputType.TYPE_NULL);
        mTvRecordId = (EditText) findViewById(R.id.hqa_read_tv_id);
        mTvRecordId.setInputType(InputType.TYPE_NULL);
        mTvRecordInf = (EditText) findViewById(R.id.hqa_read_tv_inf);
        mTvRecordInf.setInputType(InputType.TYPE_NULL);
        mTvPayloadLength = (EditText) findViewById(R.id.hqa_read_tv_length);
        mTvPayloadLength.setInputType(InputType.TYPE_NULL);
        mTvPayloadHex = (EditText) findViewById(R.id.hqa_read_tv_hex);
        mTvPayloadHex.setInputType(InputType.TYPE_NULL);
        mTvPayloadAscii = (EditText) findViewById(R.id.hqa_read_tv_ascii);
        mTvPayloadAscii.setInputType(InputType.TYPE_NULL);
        mBtnRead = (Button) findViewById(R.id.hqa_read_btn_read);
        mBtnRead.setOnClickListener(mClickListener);
        mBtnCancel = (Button) findViewById(R.id.hqa_read_btn_cancel);
        mBtnCancel.setOnClickListener(mClickListener);
        mRgTagType.check(-1);
    }

    private void doRead() {
        NfcEmAlsReadermOptReq request = new NfcEmAlsReadermOptReq();
        request.mAction = EmOptAction.READ;
        NfcClient.getInstance().sendCommand(
                CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_REQ, request);
    }

    private void updateUi(NfcTagReadNdef info) {
        int tempInt = -1;
        switch (info.mNdefType.mEnumValue) {
        case NfcNdefType.URI:
            tempInt = R.id.hqa_read_rb_type_uri;
            break;
        case NfcNdefType.TEXT:
            tempInt = R.id.hqa_read_rb_type_text;
            break;
        case NfcNdefType.SP:
            tempInt = R.id.hqa_read_rb_type_smart;
            break;
        case NfcNdefType.OTHERS:
            tempInt = R.id.hqa_read_rb_type_others;
            break;
        default:
            Elog.d(NfcMainPage.TAG, "[FunctionRead]NfcNdefType is error");
            break;
        }
        mRgTagType.check(tempInt);
        mTvRecordFlag.setText(DataConvert.printHexString(info.mRecordFlags));
        mTvRecordId.setText(DataConvert.printHexString(info.mRecordId, 0));
        mTvRecordInf.setText(DataConvert.printHexString(info.mRecordTnf));
        mTvPayloadLength.setText(String.valueOf(info.mLength));
        mTvLang.setText(new String(info.mLang));
        mTvPayloadAscii.setText(new String(info.mData,0, info.mLength));
        mTvPayloadHex.setText(DataConvert.printHexString(info.mData, info.mLength));
    }
}
