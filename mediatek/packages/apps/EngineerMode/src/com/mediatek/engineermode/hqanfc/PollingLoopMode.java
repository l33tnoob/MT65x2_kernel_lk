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
import android.widget.EditText;
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
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsCardmRsp;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsP2pNtf;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmAlsReadermNtf;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPollingNty;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPollingReq;
import com.mediatek.engineermode.hqanfc.NfcEmReqRsp.NfcEmPollingRsp;

import java.nio.ByteBuffer;

public class PollingLoopMode extends Activity {

    private static final int HANDLER_MSG_GET_RSP = 200;
    private static final int HANDLER_MSG_GET_NTF = 100;
    private static final int DIALOG_ID_RESULT = 0;
    private static final int DIALOG_ID_WAIT = 1;

    private static final int CHECKBOX_READER_MODE = 0;
    private static final int CHECKBOX_READER_TYPEA = 1;
    private static final int CHECKBOX_READER_TYPEB = 2;
    private static final int CHECKBOX_READER_TYPEF = 3;
    private static final int CHECKBOX_READER_TYPEV = 4;
   // private static final int CHECKBOX_READER_TYPEB2 = 5;
    private static final int CHECKBOX_READER_KOVIO = 5;

    private static final int CHECKBOX_P2P_MODE = 6;
    private static final int CHECKBOX_P2P_TYPEA = 7;
    private static final int CHECKBOX_P2P_TYPEF = 8;
    private static final int CHECKBOX_P2P_PASSIVE_MODE = 9;
    private static final int CHECKBOX_P2P_ACTIVE_MODE = 10;
    private static final int CHECKBOX_P2P_INITIATOR = 11;
    private static final int CHECKBOX_P2P_TARGET = 12;
    private static final int CHECKBOX_P2P_DISABLE_CARD = 13;

    private static final int CHECKBOX_CARD_MODE = 14;
   // private static final int CHECKBOX_CARD_VITRUAL_CARD = 16;
    private static final int CHECKBOX_CARD_MODE_TYPEA = 15;
    private static final int CHECKBOX_CARD_MODE_TYPEB = 16;
    private static final int CHECKBOX_CARD_MODE_TYPEF = 17;
    private static final int CHECKBOXS_NUMBER = 18;

    private static final int RADIO_READER_TYPEA_106 = 0;
    private static final int RADIO_READER_TYPEA_212 = 1;
    private static final int RADIO_READER_TYPEA_424 = 2;
    private static final int RADIO_READER_TYPEA_848 = 3;
    private static final int RADIO_READER_TYPEB_106 = 4;
    private static final int RADIO_READER_TYPEB_212 = 5;
    private static final int RADIO_READER_TYPEB_424 = 6;
    private static final int RADIO_READER_TYPEB_848 = 7;
    private static final int RADIO_READER_TYPEF_212 = 8;
    private static final int RADIO_READER_TYPEF_424 = 9;
    private static final int RADIO_READER_TYPEV_166 = 10;
    private static final int RADIO_READER_TYPEV_2648 = 11;
    private static final int RADIO_P2P_TYPEA_106 = 12;
    private static final int RADIO_P2P_TYPEA_212 = 13;
    private static final int RADIO_P2P_TYPEA_424 = 14;
    private static final int RADIO_P2P_TYPEA_848 = 15;
    private static final int RADIO_P2P_TYPEF_212 = 16;
    private static final int RADIO_P2P_TYPEF_424 = 17;
    private static final int RADIO_CARD_SWIO1 = 18;
    private static final int RADIO_CARD_SWIO2 = 19;
    private static final int RADIO_CARD_SWIOSE = 20;
    private static final int RADIO_NUMBER = 21;

    private EditText mTvPeriod;
    private CheckBox[] mSettingsCkBoxs = new CheckBox[CHECKBOXS_NUMBER];
    private RadioButton[] mSettingsRadioButtons = new RadioButton[RADIO_NUMBER];
    private RadioGroup mRgPollingSelect;
    private RadioButton mRbPollingSelectListen;
    private RadioButton mRbPollingSelectPause;

    private RadioGroup mRgTypeA;
    private RadioGroup mRgTypeB;
    private RadioGroup mRgTypeF;
    private RadioGroup mRgTypeVSubCarrier;
    private RadioGroup mRgTypeVMode;
    private RadioGroup mRgTypeVRate;

    private RadioGroup mRgP2pTypeA;
    private RadioGroup mRgP2pTypeF;

    private RadioGroup mRgCardSwio;
    
    private CheckBox mCbTypeA;
    private CheckBox mCbTypeB;
    private CheckBox mCbTypeF;
    
    private Button mBtnSelectAll;
    private Button mBtnClearAll;
    private Button mBtnStart;
    private Button mBtnReturn;
    private Button mBtnRunInBack;

    private AlertDialog mAlertDialog = null;

    private NfcEmPollingRsp mPollingRsp;
    private NfcEmPollingNty mPollingNty;
    private byte[] mRspArray;
    private String mNtfContent;
    private boolean mEnableBackKey = true;
    private boolean mUnregisterReceiver = false;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.v(NfcMainPage.TAG, "[PollingLoopMode]mReceiver onReceive: " + action);
            mRspArray = intent.getExtras().getByteArray(NfcCommand.MESSAGE_CONTENT_KEY);
            if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_RSP).equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mPollingRsp = new NfcEmPollingRsp();
                    mPollingRsp.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_RSP);
                }
            } else if ((NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_NTF).equals(action)) {
                if (null != mRspArray) {
                    ByteBuffer buffer = ByteBuffer.wrap(mRspArray);
                    mPollingNty = new NfcEmPollingNty();
                    mPollingNty.readRaw(buffer);
                    mHandler.sendEmptyMessage(HANDLER_MSG_GET_NTF);
                }
            } else {
                Elog.v(NfcMainPage.TAG, "[PollingLoopMode]Other response");
            }
        }
    };

    private final Handler mHandler = new Handler() {

        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            String toastMsg = null;
            if (HANDLER_MSG_GET_RSP == msg.what) {
                dismissDialog(DIALOG_ID_WAIT);
                switch (mPollingRsp.mResult) {
                    case RspResult.SUCCESS:
                        toastMsg = "Poling Loop Mode Rsp Result: SUCCESS";
                        if (mBtnStart.getText().equals(PollingLoopMode.this.getString(R.string.hqa_nfc_start))) {
                            setButtonsStatus(false);
                        } else {
                            setButtonsStatus(true);
                        }
                        break;
                    case RspResult.FAIL:
                        toastMsg = "Poling Loop Mode Rsp Result: FAIL";
                        break;
                    case RspResult.NFC_STATUS_NO_SIM:
                        toastMsg = "Poling Loop Mode Rsp Result: No SIM";
                        break;
                    default:
                        toastMsg = "Poling Loop Mode Rsp Result: ERROR";
                        break;
                }
            } else if (HANDLER_MSG_GET_NTF == msg.what) {
                switch (mPollingNty.mDetectType) {
                    case NfcCommand.EM_ENABLE_FUNC_P2P_MODE:
                        NfcEmAlsP2pNtf alsP2pNtf = new NfcEmAlsP2pNtf();
                        alsP2pNtf.readRaw(ByteBuffer.wrap(mPollingNty.mData));
                        if (RspResult.NFC_STATUS_LINK_UP == alsP2pNtf.mResult) {
//                            toastMsg = "P2P Data Exchange is terminated";
                            // mNtfContent = new String(alsP2pNtf.mData);
                            showDialog(DIALOG_ID_RESULT);
                        } else if (RspResult.NFC_STATUS_LINK_DOWN == alsP2pNtf.mResult) {
//                            toastMsg = "P2P Data Exchange is On-going";
                            if (mAlertDialog != null) {
                                mAlertDialog.dismiss();
                            }
                        } else {
                            toastMsg = "P2P Data Exchange is ERROR";
                        }
                        break;
                    case NfcCommand.EM_ENABLE_FUNC_READER_MODE:
                        // show RW function ui
                        NfcEmAlsReadermNtf readermNtf = new NfcEmAlsReadermNtf();
                        readermNtf.readRaw(ByteBuffer.wrap(mPollingNty.mData));
                        Intent intent = new Intent();
                        intent.putExtra(ReaderMode.KEY_READER_MODE_RSP_ARRAY, mPollingNty.mData);
                        intent.putExtra(ReaderMode.KEY_READER_MODE_RSP_NDEF, readermNtf.mIsNdef);
                        intent.setClass(PollingLoopMode.this, RwFunction.class);
                        unregisterReceiver(mReceiver);
                        mUnregisterReceiver = true;
                        startActivity(intent);
                        break;
                    case NfcCommand.EM_ENABLE_FUNC_RCARDR_MODE:
                        NfcEmAlsCardmRsp alsCardRsp = new NfcEmAlsCardmRsp();
                        alsCardRsp.readRaw(ByteBuffer.wrap(mPollingNty.mData));
                        if (RspResult.SUCCESS == alsCardRsp.mResult) {
                            toastMsg = "CardEmulation Rsp Result: SUCCESS";
                        } else if (RspResult.FAIL == alsCardRsp.mResult) {
                            toastMsg = "CardEmulation Rsp Result: FAIL";
                        } else if (RspResult.NFC_STATUS_NO_SIM == alsCardRsp.mResult) {
                            toastMsg = "CardEmulation Rsp Result: No SIM";
                        } else {
                            toastMsg = "CardEmulation Rsp Result: ERROR";
                        }
                        break;
                    default:
                        break;
                }
            }
            Toast.makeText(PollingLoopMode.this, toastMsg, Toast.LENGTH_SHORT).show();
        }
    };

    private void setRadioGroup(RadioGroup rg, boolean checked) {
        for (int i = 0; i < rg.getChildCount(); i++) {
            rg.getChildAt(i).setEnabled(checked);
        }
    }

    private final CheckBox.OnCheckedChangeListener mCheckedListener = new CheckBox.OnCheckedChangeListener() {
        public void onCheckedChanged(CompoundButton buttonView, boolean checked) {
            Elog.v(NfcMainPage.TAG, "[PollingLoopMode]onCheckedChanged view is " + buttonView.getText() + " value is "
                    + checked);
            if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_MODE])) {
                for (int i = CHECKBOX_READER_TYPEA; i <= CHECKBOX_READER_KOVIO; i++) {
                    mSettingsCkBoxs[i].setEnabled(checked);
                }
                setRadioGroup(mRgTypeA, checked);
                setRadioGroup(mRgTypeB, checked);
                setRadioGroup(mRgTypeF, checked);
                setRadioGroup(mRgTypeVSubCarrier, checked);
                setRadioGroup(mRgTypeVMode, checked);
                setRadioGroup(mRgTypeVRate, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEA])) {
                setRadioGroup(mRgTypeA, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEB])) {
                setRadioGroup(mRgTypeB, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEF])) {
                setRadioGroup(mRgTypeF, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_READER_TYPEV])) {
                setRadioGroup(mRgTypeVSubCarrier, checked);
                setRadioGroup(mRgTypeVMode, checked);
                setRadioGroup(mRgTypeVRate, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_P2P_MODE])) {
                for (int i = CHECKBOX_P2P_TYPEA; i <= CHECKBOX_P2P_DISABLE_CARD; i++) {
                    mSettingsCkBoxs[i].setEnabled(checked);
                }
                setRadioGroup(mRgP2pTypeA, checked);
                setRadioGroup(mRgP2pTypeF, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_P2P_TYPEA])) {
                setRadioGroup(mRgP2pTypeA, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_P2P_TYPEF])) {
                setRadioGroup(mRgP2pTypeF, checked);
            } else if (buttonView.equals(mSettingsCkBoxs[CHECKBOX_CARD_MODE])) {
                setRadioGroup(mRgCardSwio, checked);
                for (int i = CHECKBOX_CARD_MODE_TYPEA; i <= CHECKBOX_CARD_MODE_TYPEF; i++) {
                    mSettingsCkBoxs[i].setEnabled(checked);
                }
            }
        }
    };

    private final Button.OnClickListener mClickListener = new Button.OnClickListener() {

        @Override
        public void onClick(View arg0) {
            Elog.v(NfcMainPage.TAG, "[PollingLoopMode]onClick button view is " + ((Button) arg0).getText());
            if (arg0.equals(mBtnStart)) {
                if (!checkRoleSelect()) {
                    Toast.makeText(PollingLoopMode.this, R.string.hqa_nfc_p2p_role_tip, Toast.LENGTH_LONG).show();
                } else {
                    showDialog(DIALOG_ID_WAIT);
                    doTestAction(mBtnStart.getText().equals(PollingLoopMode.this.getString(R.string.hqa_nfc_start)));
                }
            } else if (arg0.equals(mBtnSelectAll)) {
                changeAllSelect(true);
            } else if (arg0.equals(mBtnClearAll)) {
                changeAllSelect(false);
            } else if (arg0.equals(mBtnReturn)) {
                PollingLoopMode.this.onBackPressed();
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
        setContentView(R.layout.hqa_nfc_pollingloop_mode);
        initComponents();
        changeAllSelect(true);
        IntentFilter filter = new IntentFilter();
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_RSP);
        filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_NTF);
        registerReceiver(mReceiver, filter);
    }
    @Override
    protected void onStart() {
        Elog.v(NfcMainPage.TAG, "[PollingLoopMode]onStart");
        if(mUnregisterReceiver) {
            Elog.v(NfcMainPage.TAG, "register receiver");
            mUnregisterReceiver = false;
            IntentFilter filter = new IntentFilter();
            filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_RSP);
            filter.addAction(NfcCommand.ACTION_PRE + CommandType.MTK_NFC_EM_POLLING_MODE_NTF);
            registerReceiver(mReceiver, filter);
        }
        super.onStart();  
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
        Elog.v(NfcMainPage.TAG, "[PollingLoopMode]initComponents");
        // reader mode
        mRgPollingSelect = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_polling_select);
        mRbPollingSelectListen = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_polling_listen);
        mRbPollingSelectPause = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_polling_pause);
        mTvPeriod = (EditText) findViewById(R.id.hqa_pollingmode_et_polling_period);
        mSettingsCkBoxs[CHECKBOX_READER_MODE] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_reader_mode);
        mSettingsCkBoxs[CHECKBOX_READER_MODE].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEA] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_typea);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEA].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEB] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_typeb);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEB].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEF] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_typef);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEF].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEV] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_typev);
        mSettingsCkBoxs[CHECKBOX_READER_TYPEV].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_READER_KOVIO] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_kovio);

        mSettingsRadioButtons[RADIO_READER_TYPEA_106] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typea_106);
        mSettingsRadioButtons[RADIO_READER_TYPEA_212] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typea_212);
        mSettingsRadioButtons[RADIO_READER_TYPEA_424] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typea_424);
        mSettingsRadioButtons[RADIO_READER_TYPEA_848] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typea_848);
        mSettingsRadioButtons[RADIO_READER_TYPEB_106] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typeb_106);
        mSettingsRadioButtons[RADIO_READER_TYPEB_212] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typeb_212);
        mSettingsRadioButtons[RADIO_READER_TYPEB_424] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typeb_424);
        mSettingsRadioButtons[RADIO_READER_TYPEB_848] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typeb_848);
        mSettingsRadioButtons[RADIO_READER_TYPEF_212] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typef_212);
        mSettingsRadioButtons[RADIO_READER_TYPEF_424] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typef_424);
        mSettingsRadioButtons[RADIO_READER_TYPEV_166] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typev_166);
        mSettingsRadioButtons[RADIO_READER_TYPEV_2648] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_typev_2648);

        mRgTypeA = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typea);
        mRgTypeB = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typeb);
        mRgTypeF = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typef);
        mRgTypeVSubCarrier = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typev_subcarrier);
        mRgTypeVMode = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typev_mode);
        mRgTypeVRate = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_typev_rate);

        // p2p
        mSettingsCkBoxs[CHECKBOX_P2P_MODE] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_mode);
        mSettingsCkBoxs[CHECKBOX_P2P_MODE].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_P2P_TYPEA] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_typea);
        mSettingsCkBoxs[CHECKBOX_P2P_TYPEA].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_P2P_TYPEF] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_typef);
        mSettingsCkBoxs[CHECKBOX_P2P_TYPEF].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_P2P_PASSIVE_MODE] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_passive_mode);
        mSettingsCkBoxs[CHECKBOX_P2P_ACTIVE_MODE] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_active_mode);
        mSettingsCkBoxs[CHECKBOX_P2P_INITIATOR] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_initiator);
        mSettingsCkBoxs[CHECKBOX_P2P_TARGET] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_target);
        mSettingsCkBoxs[CHECKBOX_P2P_DISABLE_CARD] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_p2p_disable_card_emu);

        mSettingsRadioButtons[RADIO_P2P_TYPEA_106] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typea_106);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_212] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typea_212);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_424] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typea_424);
        mSettingsRadioButtons[RADIO_P2P_TYPEA_848] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typea_848);
        mSettingsRadioButtons[RADIO_P2P_TYPEF_212] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typef_212);
        mSettingsRadioButtons[RADIO_P2P_TYPEF_424] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_p2p_typef_424);

        mRgP2pTypeA = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_p2p_typea);
        mRgP2pTypeF = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_p2p_typef);

        // card mode
        mSettingsCkBoxs[CHECKBOX_CARD_MODE] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_card_emu_mode);
        mSettingsCkBoxs[CHECKBOX_CARD_MODE].setOnCheckedChangeListener(mCheckedListener);
        mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEA] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_card_emu_typea);
        mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEB] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_card_emu_typeb);
        mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEF] = (CheckBox) findViewById(R.id.hqa_pollingmode_cb_card_emu_typef);
        
        mSettingsRadioButtons[RADIO_CARD_SWIO1] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_card_emu_swio1);
        mSettingsRadioButtons[RADIO_CARD_SWIO2] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_card_emu_swio2);
        mSettingsRadioButtons[RADIO_CARD_SWIOSE] = (RadioButton) findViewById(R.id.hqa_pollingmode_rb_card_emu_swiose);
   
        mRgCardSwio = (RadioGroup) findViewById(R.id.hqa_pollingmode_rg_swio);

        mBtnSelectAll = (Button) findViewById(R.id.hqa_pollingmode_btn_select_all);
        mBtnSelectAll.setOnClickListener(mClickListener);
        mBtnClearAll = (Button) findViewById(R.id.hqa_pollingmode_btn_clear_all);
        mBtnClearAll.setOnClickListener(mClickListener);
        mBtnStart = (Button) findViewById(R.id.hqa_pollingmode_btn_start_stop);
        mBtnStart.setOnClickListener(mClickListener);
        mBtnReturn = (Button) findViewById(R.id.hqa_pollingmode_btn_return);
        mBtnReturn.setOnClickListener(mClickListener);
        mBtnRunInBack = (Button) findViewById(R.id.hqa_pollingmode_btn_run_back);
        mBtnRunInBack.setOnClickListener(mClickListener);
        mBtnRunInBack.setEnabled(false);
        mRgPollingSelect.check(R.id.hqa_pollingmode_rb_polling_listen);
        mTvPeriod.setText("500");
        mTvPeriod.setSelection(mTvPeriod.getText().toString().length());
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
        Elog.v(NfcMainPage.TAG, "[PollingLoopMode]changeDisplay status is " + checked);
        mRgPollingSelect.check(R.id.hqa_pollingmode_rb_polling_listen);
        mTvPeriod.setText("500");
        for (int i = 0; i < mSettingsCkBoxs.length; i++) {
            mSettingsCkBoxs[i].setChecked(checked);
        }
        mSettingsCkBoxs[CHECKBOX_P2P_DISABLE_CARD].setChecked(false); // default false
        if (checked) {
            mRgTypeA.check(R.id.hqa_pollingmode_rb_typea_106);
            mRgTypeB.check(R.id.hqa_pollingmode_rb_typeb_106);
            mRgTypeF.check(R.id.hqa_pollingmode_rb_typef_212);
            mRgTypeVSubCarrier.check(R.id.hqa_pollingmode_rb_typev_dual_subcarrier);
            mRgTypeVMode.check(R.id.hqa_pollingmode_rb_typev_mode_4);
            mRgTypeVRate.check(R.id.hqa_pollingmode_rb_typev_166);

            mRgP2pTypeA.check(R.id.hqa_pollingmode_rb_p2p_typea_106);
            mRgP2pTypeF.check(R.id.hqa_pollingmode_rb_p2p_typef_212);

            mRgCardSwio.check(R.id.hqa_pollingmode_rb_card_emu_swio1);
        }
    }

    private void doTestAction(Boolean bStart) {
        sendCommand(bStart);
    }

    private void sendCommand(Boolean bStart) {
        NfcEmPollingReq requestCmd = new NfcEmPollingReq();
        fillRequest(bStart, requestCmd);
        NfcClient.getInstance().sendCommand(CommandType.MTK_NFC_EM_POLLING_MODE_REQ, requestCmd);
    }

    private void fillRequest(Boolean bStart, NfcEmPollingReq requestCmd) {
        if (null == bStart) {
            requestCmd.mAction = EmAction.ACTION_RUNINBG;
            requestCmd.mP2pmReq.mAction = EmAction.ACTION_RUNINBG;
            requestCmd.mReadermReq.mAction = EmAction.ACTION_RUNINBG;
            requestCmd.mCardmReq.mAction = EmAction.ACTION_RUNINBG;
        } else if (bStart.booleanValue()) {
            requestCmd.mAction = EmAction.ACTION_START;
            requestCmd.mP2pmReq.mAction = EmAction.ACTION_START;
            requestCmd.mReadermReq.mAction = EmAction.ACTION_START;
            requestCmd.mCardmReq.mAction = EmAction.ACTION_START;
        } else {
            requestCmd.mAction = EmAction.ACTION_STOP;
            requestCmd.mP2pmReq.mAction = EmAction.ACTION_STOP;
            requestCmd.mReadermReq.mAction = EmAction.ACTION_STOP;
            requestCmd.mCardmReq.mAction = EmAction.ACTION_STOP;
        }
        requestCmd.mPhase = mRbPollingSelectListen.isChecked() ? 0 : 1;
        try {
            requestCmd.mPeriod = Integer.valueOf(mTvPeriod.getText().toString());
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Please input the right Period.", Toast.LENGTH_SHORT).show();
        }
        CheckBox[] functionBoxs = { mSettingsCkBoxs[CHECKBOX_READER_MODE], mSettingsCkBoxs[CHECKBOX_CARD_MODE],
                mSettingsCkBoxs[CHECKBOX_P2P_MODE] };
        requestCmd.mEnableFunc = BitMapValue.getFunctionValue(functionBoxs);
        // p2p
        int p2pTemp = 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_TYPEA].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_TYPEF].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;
        requestCmd.mP2pmReq.mSupportType = p2pTemp;

        RadioButton[] typeADateRateBoxs = { mSettingsRadioButtons[RADIO_P2P_TYPEA_106], mSettingsRadioButtons[RADIO_P2P_TYPEA_212],
                mSettingsRadioButtons[RADIO_P2P_TYPEA_424], mSettingsRadioButtons[RADIO_P2P_TYPEA_848] };
        requestCmd.mP2pmReq.mTypeADataRate = BitMapValue.getTypeAbDataRateValue(typeADateRateBoxs);

        RadioButton[] typeFDateRateBoxs = { mSettingsRadioButtons[RADIO_P2P_TYPEF_212], mSettingsRadioButtons[RADIO_P2P_TYPEF_424] };
        requestCmd.mP2pmReq.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(typeFDateRateBoxs);

        requestCmd.mP2pmReq.mIsDisableCardM = mSettingsCkBoxs[CHECKBOX_P2P_DISABLE_CARD].isChecked() ? 
                P2pDisableCardM.DISABLE : P2pDisableCardM.NOT_DISABLE;
        p2pTemp = 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_INITIATOR].isChecked() ? NfcCommand.EM_P2P_ROLE_INITIATOR_MODE : 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_TARGET].isChecked() ? NfcCommand.EM_P2P_ROLE_TARGET_MODE : 0;
        requestCmd.mP2pmReq.mRole = p2pTemp;
        p2pTemp = 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_PASSIVE_MODE].isChecked() ? NfcCommand.EM_P2P_MODE_PASSIVE_MODE : 0;
        p2pTemp |= mSettingsCkBoxs[CHECKBOX_P2P_ACTIVE_MODE].isChecked() ? NfcCommand.EM_P2P_MODE_ACTIVE_MODE : 0;
        requestCmd.mP2pmReq.mMode = p2pTemp;

        // reader mode
        CheckBox[] typeBoxs = { mSettingsCkBoxs[CHECKBOX_READER_TYPEA], mSettingsCkBoxs[CHECKBOX_READER_TYPEB],
                mSettingsCkBoxs[CHECKBOX_READER_TYPEF], mSettingsCkBoxs[CHECKBOX_READER_TYPEV],
                mSettingsCkBoxs[CHECKBOX_READER_KOVIO] };
        requestCmd.mReadermReq.mSupportType = BitMapValue.getTypeValue(typeBoxs);

        RadioButton[] readerADateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEA_106],
                mSettingsRadioButtons[RADIO_READER_TYPEA_212], mSettingsRadioButtons[RADIO_READER_TYPEA_424],
                mSettingsRadioButtons[RADIO_READER_TYPEA_848] };
        requestCmd.mReadermReq.mTypeADataRate = BitMapValue.getTypeAbDataRateValue(readerADateRateBoxs);

        RadioButton[] readerBDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEB_106],
                mSettingsRadioButtons[RADIO_READER_TYPEB_212], mSettingsRadioButtons[RADIO_READER_TYPEB_424],
                mSettingsRadioButtons[RADIO_READER_TYPEB_848] };
        requestCmd.mReadermReq.mTypeBDataRate = BitMapValue.getTypeAbDataRateValue(readerBDateRateBoxs);

        RadioButton[] readerFDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEF_212],
                mSettingsRadioButtons[RADIO_READER_TYPEF_424] };
        requestCmd.mReadermReq.mTypeFDataRate = BitMapValue.getTypeFDataRateValue(readerFDateRateBoxs);

        RadioButton[] readerVDateRateBoxs = { mSettingsRadioButtons[RADIO_READER_TYPEV_166],
                mSettingsRadioButtons[RADIO_READER_TYPEV_2648] };
        requestCmd.mReadermReq.mTypeVDataRate = BitMapValue.getTypeVDataRateValue(readerVDateRateBoxs);

        if (mRgTypeVSubCarrier.getCheckedRadioButtonId() == R.id.hqa_pollingmode_rb_typev_subcarrier) {
            requestCmd.mReadermReq.mTypeVSubcarrier = 0;
        } else {
            requestCmd.mReadermReq.mTypeVSubcarrier = 1;
        }

        if (mRgTypeVMode.getCheckedRadioButtonId() == R.id.hqa_pollingmode_rb_typev_mode_4) {
            requestCmd.mReadermReq.mTypeVCodingMode = 0;
        } else {
            requestCmd.mReadermReq.mTypeVCodingMode = 1;
        }

        // card mode
        int cardTemp = 0;
        
        cardTemp |= mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEA].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_A : 0;
        cardTemp |= mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEB].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_B : 0;
        cardTemp |= mSettingsCkBoxs[CHECKBOX_CARD_MODE_TYPEF].isChecked() ? NfcCommand.EM_ALS_READER_M_TYPE_F : 0;
        requestCmd.mCardmReq.mSupportType = cardTemp;
        
        cardTemp = 0;

        switch (mRgCardSwio.getCheckedRadioButtonId()) {
        case R.id.hqa_pollingmode_rb_card_emu_swio1:
            cardTemp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO1;
            break;
        case R.id.hqa_pollingmode_rb_card_emu_swio2:
            cardTemp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIO2;
            break;
        case R.id.hqa_pollingmode_rb_card_emu_swiose:
            cardTemp = NfcCommand.EM_ALS_CARD_M_SW_NUM_SWIOSE;
            break;
        default:
            break;
        }
        requestCmd.mCardmReq.mSwNum = cardTemp;
        requestCmd.mCardmReq.mFgVirtualCard = 0;//mSettingsCkBoxs[CHECKBOX_CARD_VITRUAL_CARD].isChecked() ? 1 : 0;
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
            AlertDialog alertDialog = null;
            alertDialog = new AlertDialog.Builder(PollingLoopMode.this).setTitle(R.string.hqa_nfc_p2p_mode_ntf_title)
                    .setMessage("P2P link is up").setPositiveButton(android.R.string.ok, null).create();
            return alertDialog;
        }
        return null;
    }

    private boolean checkRoleSelect() {
        boolean result = true;
        if (!mSettingsCkBoxs[CHECKBOX_P2P_INITIATOR].isChecked() && !mSettingsCkBoxs[CHECKBOX_P2P_TARGET].isChecked()) {
            result = false;
        }
        if (!mSettingsCkBoxs[CHECKBOX_P2P_PASSIVE_MODE].isChecked()
                && !mSettingsCkBoxs[CHECKBOX_P2P_ACTIVE_MODE].isChecked()) {
            result = false;
        }
        return result;
    }
}
