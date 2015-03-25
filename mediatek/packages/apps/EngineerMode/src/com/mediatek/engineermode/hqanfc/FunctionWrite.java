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
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.hqanfc.NfcCommand.CommandType;
import com.mediatek.engineermode.hqanfc.NfcCommand.EmOptAction;
import com.mediatek.engineermode.hqanfc.NfcCommand.RspResult;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermOptRsp;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcNdefType;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.SmartPosterT;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.TextT;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.UrlT;

import java.nio.ByteBuffer;

public class FunctionWrite extends Activity {

    protected static final int HANDLER_MSG_GET_RSP = 200;

    private RadioGroup mRgTagType;
    private Spinner mSpLang;
    private EditText mEtCompany;
    private EditText mEtUrl;
    private EditText mEtText;
    private TextView mTvCompany;
    private TextView mTvUrl;
    private TextView mTvText;
    private Button mBtnWrite;
    private Button mBtnCancel;

    private NfcEmAlsReadermOptRsp mOptRsp;
    private byte[] mRspArray;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.v(NfcMainPage.TAG, "[FunctionWrite]mReceiver onReceive");
            String action = intent.getAction();
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP)
                    .equals(action)) {
                mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mOptRsp = new NfcEmAlsReadermOptRsp();
                    mOptRsp.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[FunctionWrite]Other response");
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
                        toastMsg = "Function Write Rsp Result: SUCCESS";
                        break;
                    case RspResult.FAIL:
                        toastMsg = "Function Write Rsp Result: FAIL";
                        break;
                    default:
                        toastMsg = "Function Write Rsp Result: ERROR";
                        break;
                }
                Toast.makeText(FunctionWrite.this, toastMsg, Toast.LENGTH_SHORT).show();
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[FunctionWrite]onClick button view is "
                    + ((Button) arg0).getText());
            if (arg0.equals(mBtnWrite)) {
                doWrite();
            } else if (arg0.equals(mBtnCancel)) {
                FunctionWrite.this.onBackPressed();
            } else {
                Elog.v(NfcMainPage.TAG, "[FunctionWrite]onClick noting.");
            }
        }

    };
    private final RadioGroup.OnCheckedChangeListener mCheckedChangeListener = new RadioGroup.OnCheckedChangeListener() {

        public void onCheckedChanged(RadioGroup group, int checkedId) {
            Elog.v(NfcMainPage.TAG, "[FunctionWrite]onCheckedChanged checkedId is " + checkedId);
            checkedChange(checkedId);
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.hqa_nfc_function_write);
        initComponents();
    }

    @Override
    protected void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_RSP);
        registerReceiver(mReceiver, filter);
    }

    @Override
    protected void onStop() {
        unregisterReceiver(mReceiver);
        super.onStop();
    }

    private void initComponents() {
        mRgTagType = (RadioGroup) findViewById(R.id.hqa_write_rg_tag_type);
        mRgTagType.setOnCheckedChangeListener(mCheckedChangeListener);
        mSpLang = (Spinner) findViewById(R.id.hqa_write_sp_lang);
        mEtCompany = (EditText) findViewById(R.id.hqa_write_et_company);
        mEtUrl = (EditText) findViewById(R.id.hqa_write_et_url);
        mEtText = (EditText) findViewById(R.id.hqa_write_et_text);
        mBtnWrite = (Button) findViewById(R.id.hqa_write_btn_write);
        mBtnWrite.setOnClickListener(mClickListener);
        mBtnCancel = (Button) findViewById(R.id.hqa_write_btn_cancel);
        mBtnCancel.setOnClickListener(mClickListener);
        mTvCompany = (TextView) findViewById(R.id.hqa_write_tv_company);
        mTvUrl = (TextView) findViewById(R.id.hqa_write_tv_url);
        mTvText = (TextView) findViewById(R.id.hqa_write_tv_text);
        mEtUrl.setSelection(0);
        mRgTagType.check(R.id.hqa_write_rb_type_uri);
    }

    protected void checkedChange(int checkedId) {
        switch (checkedId) {
            case R.id.hqa_write_rb_type_uri:
                mSpLang.setEnabled(false);
                mEtCompany.setVisibility(View.GONE);
                mTvCompany.setVisibility(View.GONE);
                mEtText.setVisibility(View.GONE);
                mTvText.setVisibility(View.GONE);
                mEtUrl.setVisibility(View.VISIBLE);
                mTvUrl.setVisibility(View.VISIBLE);
                mEtUrl.setText("");
                mEtUrl.setSelection(0);
                break;
            case R.id.hqa_write_rb_type_text:
                mSpLang.setEnabled(true);
                mEtCompany.setVisibility(View.GONE);
                mTvCompany.setVisibility(View.GONE);
                mEtText.setVisibility(View.VISIBLE);
                mTvText.setVisibility(View.VISIBLE);
                mEtUrl.setVisibility(View.GONE);
                mTvUrl.setVisibility(View.GONE);
                mEtText.setText("");
                mEtText.setSelection(mEtText.getText().length());
                break;
            default:
                break;
        }
    }

    private void doWrite() {
        if (checkInput()) {
            NfcEmAlsReadermOptReq request = new NfcEmAlsReadermOptReq();
            request.mAction = EmOptAction.WRITE;
            int tempInt = -1;
            switch (mRgTagType.getCheckedRadioButtonId()) {
                case R.id.hqa_write_rb_type_uri:
                    tempInt = NfcNdefType.URI;
                    break;
                case R.id.hqa_write_rb_type_text:
                    tempInt = NfcNdefType.TEXT;
                    break;
                default:
                    tempInt = NfcNdefType.OTHERS;
                    break;
            }
            request.mTagWriteNdef.mNdefType.mEnumValue = tempInt;
            request.mTagWriteNdef.mNdefLangType.mEnumValue = mSpLang.getSelectedItemPosition();
            switch (tempInt) {
                case NfcNdefType.URI:
                    UrlT url = new UrlT();
                    byte[] urlArray = mEtUrl.getText().toString().getBytes();
                    System.arraycopy(urlArray, 0, url.mUrlData, 0, urlArray.length);
                    url.mUrlDataLength = (short) url.mUrlData.length;
                    byte[] arrayU = url.getByteArray();
                    System.arraycopy(arrayU, 0, request.mTagWriteNdef.mNdefData.mData, 0,
                            arrayU.length);
                    request.mTagWriteNdef.mLength = arrayU.length;
                    break;
                case NfcNdefType.TEXT:
                    TextT text = new TextT();
                    byte[] textArray = mEtText.getText().toString().getBytes();
                    System.arraycopy(textArray, 0, text.mData, 0, textArray.length);
                    text.mDataLength = (short) textArray.length;
                    byte[] arrayT = text.getByteArray();
                    System.arraycopy(arrayT, 0, request.mTagWriteNdef.mNdefData.mData, 0,
                            arrayT.length);
                    request.mTagWriteNdef.mLength = arrayT.length;
                    break;
                case NfcNdefType.SP:
                    SmartPosterT smart = new SmartPosterT();
                    byte[] comArray = mEtCompany.getText().toString().getBytes();
                    System.arraycopy(comArray, 0, smart.mCompany, 0, comArray.length);
                    smart.mCompanyLength = (short) comArray.length;
                    byte[] comUrlArray = mEtUrl.getText().toString().getBytes();
                    System.arraycopy(comUrlArray, 0, smart.mCompanyUrl, 0, comUrlArray.length);
                    smart.mCompanyUrlLength = (short) comUrlArray.length;
                    byte[] arrayS = smart.getByteArray();
                    System.arraycopy(arrayS, 0, request.mTagWriteNdef.mNdefData.mData, 0,
                            arrayS.length);
                    request.mTagWriteNdef.mLength = arrayS.length;
                    break;
                case NfcNdefType.OTHERS:
                    break;
                default:
                    break;
            }
            NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_ALS_READER_MODE_OPT_REQ,
                    request);
        } else {
            Toast.makeText(this, "Input error", Toast.LENGTH_SHORT).show();
        }
    }

    private boolean checkInput() {
        if (-1 == mRgTagType.getCheckedRadioButtonId()) {
            return false;
        }
        return true;
    }
}
