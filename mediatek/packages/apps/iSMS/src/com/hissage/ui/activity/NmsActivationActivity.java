package com.hissage.ui.activity;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.hissage.R;
import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.jni.engineadapter;
//M: Activation Statistics
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.receiver.system.NmsSMSReceiver;
import com.hissage.struct.SNmsMsgType;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.NmsIntentStrId;
import com.hissage.util.log.NmsLog;
//M: Activation Statistics
import com.hissage.util.statistics.NmsStatistics;

public class NmsActivationActivity extends NmsBaseActivity implements OnClickListener {

    private static final String TAG = "NmsActivationActivity";

    private TextView mMainTitle = null;
    private TextView mTitle = null;

    private TextView mCountryTitle = null;
    private Spinner mCountryContent = null;

    private TextView mPhoneTitle = null;
    private EditText mPhoneEdit = null;

    private ProgressBar mProgressWait = null;
    private TextView mWaitVerify = null;

    private TextView mVerifyFailTitle = null;
    private TextView mVerifyFailContent = null;

    private TextView mWorkingSmsTip = null;
    private View mLine = null;

    private Button mSendOrEditNumber = null;
    private Button mCancel = null;

    private RegBroadCast mReceiver = null;
    private String mNumber = null;
    private String mUnifyNumber = null;
    private long mSim_id = NmsConsts.INVALID_SIM_ID;
    private int countryIndex = 41;
    private boolean mRegSmsSent = false;
    private boolean mVerifyFlag = false;
    private static boolean mIsDlgShown = false;
    private int mActivateType;

    private void initSpinner() {
        long currentSimId = 0;
        if (mSim_id < 0) {
            currentSimId = NmsPlatformAdapter.getInstance(this).getCurrentSimId();
        } else {
            currentSimId = mSim_id;
        }
        int slot = NmsPlatformAdapter.getInstance(this).getSlotIdBySimId(currentSimId);
        String currentImsi = NmsPlatformAdapter.getInstance(this).getImsi(slot);
        
        if(TextUtils.isEmpty(currentImsi))
            return;

        String[] imsiPreArray = this.getResources().getStringArray(R.array.imsi_pre_list);
        // String[] phoneArray = NmsService.getInstance().getResources()
        // .getStringArray(R.array.phone_number_pre_list);
        for (int i = 0; i < imsiPreArray.length; ++i) {
            if (currentImsi.startsWith(imsiPreArray[i])) {
                countryIndex = i;
                return;
            }
        }
    }

    private void init() {

        initSpinner();

        mMainTitle = (TextView) findViewById(R.id.main_title);
        mTitle = (TextView) findViewById(R.id.title);

        mCountryTitle = (TextView) findViewById(R.id.country_title);
        mCountryContent = (Spinner) findViewById(R.id.country_content);
        mCountryContent.setSelection(countryIndex);
        mCountryContent.setEnabled(false);

        mPhoneTitle = (TextView) findViewById(R.id.phone_title);
        mPhoneEdit = (EditText) findViewById(R.id.phone);
        if (!TextUtils.isEmpty(mNumber)) {
            mPhoneEdit.setText(mNumber);
        }

        mProgressWait = (ProgressBar) findViewById(R.id.progress_wait);
        mWaitVerify = (TextView) findViewById(R.id.wait_for_verify);

        mVerifyFailTitle = (TextView) findViewById(R.id.verify_fail_title);
        mVerifyFailContent = (TextView) findViewById(R.id.verify_fail_content);

        mWorkingSmsTip = (TextView) findViewById(R.id.working_sms_tip);
        mLine = (View) findViewById(R.id.divider);

        mSendOrEditNumber = (Button) findViewById(R.id.send_or_edit);
        mSendOrEditNumber.setOnClickListener(this);
        mCancel = (Button) findViewById(R.id.cancel);
        mCancel.setOnClickListener(this);

        initMode();
    }

    private void initMode() {
        mVerifyFlag = false;
        mMainTitle.setText(R.string.STR_NMS_ACTIVE);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.INVISIBLE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.GONE);

        mProgressWait.setVisibility(View.VISIBLE);
        mWaitVerify.setVisibility(View.VISIBLE);
        mWaitVerify.setText(R.string.STR_NMS_CONNECTING);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.INVISIBLE);
        mSendOrEditNumber.setVisibility(View.GONE);

        mLine.setVisibility(View.GONE);
        
        mCancel.setVisibility(View.GONE);
        mCancel.setText(R.string.STR_NMS_CANCEL);
    }

    private void inputMode() {
        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE);
        mTitle.setVisibility(View.VISIBLE);

        mCountryTitle.setVisibility(View.VISIBLE);
        mCountryContent.setVisibility(View.VISIBLE);

        mPhoneTitle.setVisibility(View.VISIBLE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(true);

        mProgressWait.setVisibility(View.GONE);
        mWaitVerify.setVisibility(View.GONE);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.GONE);
        mSendOrEditNumber.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setText(R.string.STR_NMS_SEND);

        mCancel.setText(R.string.STR_NMS_CANCEL);

        mLine.setVisibility(View.VISIBLE);
    }

    private void verifingMode() {
        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE2);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.GONE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(false);
        mPhoneEdit.clearFocus();

        mProgressWait.setVisibility(View.VISIBLE);
        mWaitVerify.setVisibility(View.VISIBLE);
        mWaitVerify.setText(R.string.STR_NMS_VERIFYING);

        mVerifyFailTitle.setVisibility(View.GONE);
        mVerifyFailContent.setVisibility(View.GONE);

        mWorkingSmsTip.setVisibility(View.VISIBLE);
        
        
        mSendOrEditNumber.setVisibility(View.GONE);
        mSendOrEditNumber.setText(R.string.STR_NMS_EDIT_NUMBER);

        mCancel.setText(R.string.STR_NMS_CANCEL);
        mLine.setVisibility(View.GONE);

    }

    private void verifyFailMode() {
        mMainTitle.setText(R.string.STR_NMS_ACTIVAT_MAIN_TITLE2);
        mTitle.setVisibility(View.GONE);

        mCountryTitle.setVisibility(View.GONE);
        mCountryContent.setVisibility(View.GONE);

        mPhoneTitle.setVisibility(View.GONE);
        mPhoneEdit.setVisibility(View.VISIBLE);
        mPhoneEdit.setEnabled(false);
        mPhoneEdit.clearFocus();

        mProgressWait.setVisibility(View.GONE);
        mWaitVerify.setVisibility(View.GONE);

        mVerifyFailTitle.setVisibility(View.VISIBLE);
        mVerifyFailContent.setVisibility(View.VISIBLE);

        mWorkingSmsTip.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setVisibility(View.VISIBLE);
        mSendOrEditNumber.setText(R.string.STR_NMS_EDIT_NUMBER);

        mCancel.setText(R.string.STR_NMS_RETRY);
        mLine.setVisibility(View.VISIBLE);
    }

    void initPhoneNumber() {
        if (mSim_id > 0) {
            int slotId = NmsPlatformAdapter.getInstance(this).getSlotIdBySimId(mSim_id);
            if (slotId >= 0) {
                mNumber = NmsPlatformAdapter.getInstance(this).getLine1Number(slotId);
            }
        }
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activition_layout);
        setFinishOnTouchOutside(false) ;

        Intent i = getIntent();
        if (i == null
                || NmsConsts.INVALID_SIM_ID == (mSim_id = i.getLongExtra(NmsConsts.SIM_ID,
                        NmsConsts.INVALID_SIM_ID))) {
            NmsLog.error(TAG, "not find sim_id at intent: " + i);
            finish();
            return;
        }

        if (!isSimCardReady()) {
            Toast.makeText(getApplicationContext(), R.string.STR_NMS_NO_SIM, Toast.LENGTH_SHORT)
                    .show();
            finish();
            return;
        }

        initPhoneNumber();

        init();

        NmsSMSReceiver.getInstance().setRegPhone(mNumber);
        if (null == mReceiver) {
            regResRecver();
        }
        //M: Add new feature: ISMS-214    
        mActivateType = i.getIntExtra(NmsConsts.ACTIVATE_TYPE, NmsIpMessageConsts.NmsUIActivateType.OTHER) ;
        
        int keyIndex = NmsStatistics.KEY_OTHER_ACTIVATE_TRY ;
        if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.EMOTION) 
            keyIndex = NmsStatistics.KEY_EMO_ACTIVATE_TRY ;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.MULTI_MEDIA) 
            keyIndex = NmsStatistics.KEY_MEDIA_ACTIVATE_TRY ;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.SETTING)
            keyIndex = NmsStatistics.KEY_SETTING_ACTIVATE_TRY ;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.DIALOG) 
            keyIndex = NmsStatistics.KEY_DLG_ACTIVATE_TRY ;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.PROMPT) 
            keyIndex = NmsStatistics.tips_activate_try ;
        else if (mActivateType == NmsIpMessageConsts.NmsUIActivateType.MESSAGE) 
            keyIndex = NmsStatistics.sms_activate_try ;
        NmsStatistics.incKeyVal(keyIndex) ;
        engineadapter.get().nmsUISetUserActivteType(mActivateType) ;
        engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
    }


    private void regResRecver() {
        mReceiver = new RegBroadCast();
        IntentFilter filter = new IntentFilter();
        filter.addAction(NmsIntentStrId.NMS_REG_STATUS);
        filter.addAction(NmsIntentStrId.NMS_REG_SMS_ERROR);
        filter.addAction(NmsIntentStrId.NMS_REG_INPUT_PHONENUM);
        registerReceiver(mReceiver, filter);
    }

    @Override
    public void finish() {
        if (null != mReceiver) {
            unregisterReceiver(mReceiver);
        }
        mReceiver = null;
        super.finish();
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {  

        if(keyCode == KeyEvent.KEYCODE_BACK){
            try {
                if (mCancel != null && ((String)mCancel.getText()).equals(getString(R.string.STR_NMS_CANCEL))) {
                    NmsLog.trace(TAG, "user press the back key, just cancel the activation") ;
                    //engineadapter.get().nmsUICancelActivation() ;
                }
                
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e) ;
            }
        }  
        
        return super.onKeyDown(keyCode, event) ;
 }
    
    private void sendSMS() {
        mNumber = mPhoneEdit.getText().toString();
        if (TextUtils.isEmpty(mNumber) || mNumber.length() < 5) {
            Toast.makeText(this, R.string.STR_NMS_ILLEGAL_ADDRESS, Toast.LENGTH_SHORT).show();
            return;
        }
        mRegSmsSent = true;
        mUnifyNumber = engineadapter.get().nmsUIGetUnifyPhoneNumber(mNumber);
        NmsIpMessageApiNative.nmsInputNumberForActivation(mUnifyNumber, (int) mSim_id);
        verifingMode();
    }

    private void showChooseNetworkDlg() {

        if (mIsDlgShown) {
            NmsLog.error(TAG, "this dlg was shown, so ignore this.");
            return;
        }
        mIsDlgShown = true;

        new AlertDialog.Builder(NmsActivationActivity.this).setTitle(R.string.STR_NMS_MAIN)
                .setMessage(R.string.STR_NMS_NETWORK_UNAVL)
                .setPositiveButton(R.string.STR_NMS_RETRY, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        engineadapter.get().nmsUISetUserActivteType(mActivateType);
                        engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
                        mIsDlgShown = false;
                        dialog.cancel();
                    }
                })
                .setNegativeButton(R.string.STR_NMS_CANCEL, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mIsDlgShown = false;
                        finish();
                    }
                }).setCancelable(false).create().show();

    }

    private void regFail(Context context) {
        if (mRegSmsSent) {
            verifyFailMode();
        } else {
            showChooseNetworkDlg();
        }
    }

    private void recvRegOver(boolean isNewUser) {
        // if (isNewUser) {
        new AlertDialog.Builder(NmsActivationActivity.this)
                .setTitle(R.string.STR_NMS_CONGRATULATIONS_TITLE)
                .setMessage(R.string.STR_NMS_CONGRATULATIONS_CONTENT)
                .setPositiveButton(R.string.STR_NMS_OK, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        // Intent i = new Intent(NmsActivationActivity.this,
                        // NmsProfileSettingsActivity.class);
                        // startActivity(i);
                        //dialog.cancel();
                        finish();
                    }
                })
                // .setNegativeButton(R.string.STR_NMS_START_ISMS,
                // new DialogInterface.OnClickListener() {
                // @Override
                // public void onClick(DialogInterface dialog, int which) {
                // finish();
                // }
                // })
                .setCancelable(false)
                .create().show();
        // } else {
        // Toast.makeText(this, R.string.STR_NMS_WELCOME_BACK,
        // Toast.LENGTH_SHORT).show();
        //
        // finish();
        // }

    }

    class RegBroadCast extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            String action = intent.getAction();
            NmsLog.trace(TAG, "The Activation hissage action:" + action);

            if (action.equals(NmsIntentStrId.NMS_REG_SMS_ERROR)) {
                new AlertDialog.Builder(context)
                        .setTitle(getString(R.string.STR_NMS_HISSAGE))
                        .setMessage(getString(R.string.STR_NMS_SEND_SMS_FAIL))
                        .setPositiveButton(R.string.STR_NMS_OK,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        dialog.cancel();
                                    }
                                }).create().show();
                verifyFailMode();
            } else if (NmsIntentStrId.NMS_REG_INPUT_PHONENUM.equals(action)) {
                if (mVerifyFlag) {
                    mVerifyFlag = false;
                    sendSMS();
                } else {
                    inputMode();
                }
            } else if (action.equals(NmsIntentStrId.NMS_REG_STATUS)) {
                int regStatus = intent.getIntExtra("regStatus", -1);

                switch (regStatus) {
                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_OVER:
                    Toast.makeText(getApplicationContext(),R.string.STR_NMS_ENABLE_SUCCESS, Toast.LENGTH_SHORT).show();
                    finish();
                    break;

                case SNmsMsgType.NMS_UI_MSG_REGISTRATION_FAIL:
                    regFail(context);
                    break;

                case SNmsMsgType.NMS_UI_MSG_CHOOSE_IAP:
                    showChooseNetworkDlg();
                    break;

                case SNmsMsgType.NMS_UI_MSG_REMIND_USER_TO_SET_NAME:
                    Toast.makeText(context, R.string.STR_NMS_REMIND_USER_SET_NAME,
                            Toast.LENGTH_LONG).show();
                    break;

                case SNmsMsgType.NMS_UI_MSG_VERIF_ING:
                    break;

                default:
                    NmsLog.trace(TAG, "some case not necessary, case: " + regStatus);
                }

            }
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mCancel) {

            String tip = (String) mCancel.getText();
            if (tip.equals(getString(R.string.STR_NMS_RETRY))) {
                if (TextUtils.isEmpty(mNumber)) {
                    initMode();
                } else {
                    mVerifyFlag = true;
                    verifingMode();
                }
                engineadapter.get().nmsUISetUserActivteType(mActivateType);
                engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
            } else {
                NmsLog.trace(TAG, "user press the cancel button, just cancel the activation") ;
                engineadapter.get().nmsUICancelActivation() ;
                this.finish();
            }

        } else if (v == mSendOrEditNumber) {
            String tip = (String) mSendOrEditNumber.getText();
            if (tip.equals(getString(R.string.STR_NMS_SEND))) {
                sendSMS();
            } else {
                initMode();
                engineadapter.get().nmsUISetUserActivteType(mActivateType);
                engineadapter.get().nmsUIActivateSimCard((int) mSim_id);
            }
        }

    }

}
