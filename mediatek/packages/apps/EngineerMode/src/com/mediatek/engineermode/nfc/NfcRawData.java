/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.nfc;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.nfc.NfcAdapter;

import java.util.ArrayList;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

public class NfcRawData extends Activity implements OnClickListener {
    private static final String TAG = "EM/nfc";
    private Button mBtnStart = null;
    private Button mBtnStop = null;
    private RadioGroup mRadioGpMode;
    private TextView mTextResult;
    private ArrayList<RadioButton> mRadioItems = new ArrayList<RadioButton>();
    private View mViewAlwayson;
    private View mViewCardEmul;
    private Spinner mAlwayseOnModuleAppendix;
    private Spinner mAlwayseOnBitrateAppendix;
    private Spinner mCardEmulModeTypeAppendix;
    private Spinner mCardEmulModeProtAppendix;

    private int mSelectionId = 0;
    private boolean mInStartTest = false;

    private static final int EVENT_OP_START = 101;
    private static final int EVENT_OP_STOP = 102;
    private static final int EVENT_OP_FIN = 103;
    private static final int EVENT_OP_MSG = 104;
    private static final int EVENT_UPDATE_RADIO_UI = 105;

    private static final int TEST_ID_ALWAYSE_ON_WITH = 1001;
    private static final int TEST_ID_ALWAYSE_ON_WO = 1002;
    private static final int TEST_ID_CARD_EMUL_MODE = 1003;
    private static final int TEST_ID_READER_MODE = 1004;
    private static final int TEST_ID_P2P_MODE = 1005;
    private static final int TEST_ID_SWP_SELF = 1006;
    private static final int TEST_ID_ANTENNA_SELF = 1007;
    private static final int TEST_ID_UID_RW = 1008;

    private static final int TEST_START = 1; // according to protocol
    private static final int TEST_STOP = 0;
    private static final int NUM_3 = 3;
    private static final int NUM_3F = 0x03;
    private static final int NUM_4F = 0x04;

    private boolean mInProgress = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Elog.i(TAG, "NfcRawData onCreate");
        setContentView(R.layout.nfc_rawdata);
        //Elog.i(TAG, "NfcRawData onResume");
        //initUI();         
    }

    @Override
    protected void onResume() { 
        super.onResume();
        closeNFCServiceAtStart(); 
        Elog.i(TAG, "NfcRawData onResume");
        initUI();           
    }

    /**
     * on click the button start and stop
     * 
     * @param arg0
     *            : clicked which view
     */
    public void onClick(View arg0) {
        if (arg0 == mBtnStart) {
            if (mRadioGpMode.getCheckedRadioButtonId() == -1) {
                Toast.makeText(getApplicationContext(),
                    "Test Mode is not selected.", Toast.LENGTH_LONG).show();
                return;
            }
            mHander.sendEmptyMessage(EVENT_OP_START);
        } else if (arg0 == mBtnStop) {
            mHander.sendEmptyMessage(EVENT_OP_STOP);
        } else {
            Elog.e(TAG, "ASSERT. Ghost view " + arg0.getClass().toString());
        }
    }


    private void closeNFCServiceAtStart() {
        NfcAdapter adp = NfcAdapter.getDefaultAdapter(getApplicationContext());
        if (adp.isEnabled()) {
            if (adp.disable()) {
                Elog.i(TAG, "Nfc service set off.");
            } else {
                Elog.i(TAG, "Nfc service set off Fail.");
            }
        } else {
            Elog.i(TAG, "Nfc service is off");
        }
    }

    private void initUI() {
        mBtnStart = (Button) findViewById(R.id.NFC_RawData_Start);
        mBtnStop = (Button) findViewById(R.id.NFC_RawData_Stop);
        mRadioGpMode = (RadioGroup) findViewById(R.id.NFC_RawData_Group);
        mTextResult = (TextView) findViewById(R.id.NFC_RawData_Result);

        mViewAlwayson = (View) findViewById(R.id.View_Alwayse_on_appendix);
        mViewCardEmul = (View) findViewById(R.id.View_Card_emul_mode_appendix);

        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item1));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item2));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item3));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item4));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item5));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item6));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item7));
        mRadioItems.add((RadioButton) findViewById(R.id.NFC_RawData_Item8));

        mBtnStart.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
        mRadioGpMode
            .setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(RadioGroup group, int checkedId) {
                    Elog.d(TAG, "mRadioGpMode checked Id " + checkedId);
                    mSelectionId = getSelectionId(checkedId);
                    addAppendix(mSelectionId);
                }

                private int getSelectionId(int radioId) {
                    final int[] idxs =
                        { R.id.NFC_RawData_Item1, R.id.NFC_RawData_Item2,
                            R.id.NFC_RawData_Item3, R.id.NFC_RawData_Item4,
                            R.id.NFC_RawData_Item5, R.id.NFC_RawData_Item6,
                            R.id.NFC_RawData_Item7, R.id.NFC_RawData_Item8 };
                    final int[] ids =
                        { TEST_ID_ALWAYSE_ON_WITH, TEST_ID_ALWAYSE_ON_WO,
                            TEST_ID_CARD_EMUL_MODE, TEST_ID_READER_MODE,
                            TEST_ID_P2P_MODE, TEST_ID_SWP_SELF,
                            TEST_ID_ANTENNA_SELF, TEST_ID_UID_RW };

                    for (int i = 0; i < idxs.length; i++) {
                        if (idxs[i] == radioId) {
                            return ids[i];
                        }
                    }
                    Elog.e(TAG, "Ghost RadioGroup checkId " + radioId);
                    return TEST_ID_ALWAYSE_ON_WITH;
                }
            });
        initSpinner();
        enableAllRadioBox(true);
        disableForTemp(); // why
        mRadioGpMode.check(R.id.NFC_RawData_Item3);
        mBtnStart.setEnabled(true);
        mBtnStop.setEnabled(false);
    }

    private void disableForTemp() {
        Elog.i(TAG, "disableForTemp");
        mRadioItems.get(0).setEnabled(false); // TX wait ACK
        mRadioItems.get(4).setEnabled(false); // P2P test 
		mRadioItems.get(6).setEnabled(false); // antenna test

        //mRadioItems.get(1).setEnabled(true);
        //mRadioItems.get(2).setEnabled(true);
        //mRadioItems.get(3).setEnabled(true);
        //mRadioItems.get(5).setEnabled(true);
        //mRadioItems.get(7).setEnabled(true);

    }

    // set view visible or gone depends on select raido button
    private void addAppendix(int selId) {
        if (selId == TEST_ID_ALWAYSE_ON_WITH || selId == TEST_ID_ALWAYSE_ON_WO) { // item1,2
            mViewAlwayson.setVisibility(View.VISIBLE);
            mViewCardEmul.setVisibility(View.GONE);
        } else if (selId == TEST_ID_CARD_EMUL_MODE) { // item3
            mViewAlwayson.setVisibility(View.GONE);
            mViewCardEmul.setVisibility(View.VISIBLE);
        } else { // item4-8
            mViewAlwayson.setVisibility(View.GONE);
            mViewCardEmul.setVisibility(View.GONE);
        }
    }

    private void initSpinner() {
        mAlwayseOnModuleAppendix =
            (Spinner) findViewById(R.id.NFC_RawData_Alwayson_appendix_module);
        mAlwayseOnBitrateAppendix =
            (Spinner) findViewById(R.id.NFC_RawData_Alwayson_appendix_bitrate);
        mCardEmulModeTypeAppendix =
            (Spinner) findViewById(R.id.NFC_RawData_cardEmulMode_appendix_type);
        mCardEmulModeProtAppendix =
            (Spinner) findViewById(R.id.NFC_RawData_cardEmulMode_appendix_protocol);

        final ArrayAdapter<String> alwaysOnModuleAdatper =
            new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, R.array.always_on_module);
        final ArrayAdapter<String> alwaysOnBitrateAdatper =
            new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, R.array.always_on_bitrate);
        final ArrayAdapter<String> cardEmulModeTypeAdatper =
            new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,
                R.array.card_emul_mode_type);
        final ArrayAdapter<String> cardEmulModeProtAdatper =
            new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item,
                R.array.card_emul_mode_prot);
        alwaysOnModuleAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        alwaysOnBitrateAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        cardEmulModeTypeAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        cardEmulModeProtAdatper
            .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mAlwayseOnModuleAppendix.setAdapter(alwaysOnModuleAdatper);
        mAlwayseOnBitrateAppendix.setAdapter(alwaysOnBitrateAdatper);
        mCardEmulModeTypeAppendix.setAdapter(cardEmulModeTypeAdatper);
        mCardEmulModeProtAppendix.setAdapter(cardEmulModeProtAdatper);
        mAlwayseOnModuleAppendix.setSelection(0);
        mAlwayseOnBitrateAppendix.setSelection(0);
        mCardEmulModeTypeAppendix.setSelection(0);
        mCardEmulModeProtAppendix.setSelection(0);
    }

    private Handler mHander = new Handler() {
        public void handleMessage(Message msg) {

            switch (msg.what) {
            case EVENT_OP_FIN:
                mBtnStart.setEnabled(true);
                mBtnStop.setEnabled(false);
				mInProgress = false ;
                break;
            case EVENT_OP_START:
                mBtnStart.setEnabled(false);
                mBtnStop.setEnabled(true);
				mInProgress = true;
                testing(true);
                break;
            case EVENT_OP_STOP:
                testing(false);
                break;
            case EVENT_OP_MSG:
                String result = msg.getData().getString("MSG");
                mTextResult.setText(result);
                break;
            case EVENT_UPDATE_RADIO_UI:
                boolean enabled = msg.getData().getBoolean("UP");
                for (RadioButton r : mRadioItems) {
                    r.setEnabled(enabled);
                }
                disableForTemp();
                break;
            default:
                break;
            }

        }
    };

    private void enableAllRadioBox(boolean enabled) {
        Message msg = new Message();
        msg.what = EVENT_UPDATE_RADIO_UI;
        Bundle bun = new Bundle();
        bun.putBoolean("UP", enabled);
        msg.setData(bun);
        mHander.sendMessage(msg);
    }

    private void testing(final boolean isStart) {
        mTextResult.setText("Waiting...");
        if (mInStartTest) {
			if (mSelectionId == TEST_ID_UID_RW) { // Read UID suppot 
				Elog.d(TAG, "ID = " + TEST_ID_UID_RW + ", Select ID = " + mSelectionId);
			}  else {
            	threadSendMsg("Operation is still in progress. Try later.");
            	return;
			}
        }
        Elog.d(TAG, " ++++ mInStartTest = " + mInStartTest + ", mInProgress = " + mInProgress);
		if ( mInProgress && !isStart && (mSelectionId == TEST_ID_UID_RW)) {  // Start command in progress and user click Stop button			
	        new Thread() {
	            @Override
	            public void run() {
	                Elog.d(TAG, "++++ Thread 1 , Start +++++");

                    try {
	                    NfcNativeCallClass.nfc_test_request req = makeRequest(isStart);
	                    NfcNativeCallClass.nfc_test_response resp = NfcNativeCallClass.testEntry(req);
                        handleResponse(resp);
                    } catch (Exception e) {
                        Elog.e(TAG, "" + e);
                    }
	            
        			enableAllRadioBox(true);
        	    	mHander.sendEmptyMessage(EVENT_OP_FIN);
                    threadSendMsg("Result: Stop");                    
	                mInStartTest = false;
                    Elog.d(TAG, "++++ Thread 1 , End +++++");
	            }
	        }.start();
						
		} else {
	        mInStartTest = true;
	        new Thread() {
	            @Override
	            public void run() {
	                Elog.d(TAG, "++++ Thread 2 , Start +++++");
	                if (isStart) {
	                    enableAllRadioBox(false);
	                }

                    try {
	                    NfcNativeCallClass.nfc_test_request req = makeRequest(isStart);
	                    NfcNativeCallClass.nfc_test_response resp = NfcNativeCallClass.testEntry(req);
                        handleResponse(resp);
                    } catch (Exception e) {
                        Elog.d(TAG, "" + e);
                    }
	                
	                if (!isStart) {
	                    enableAllRadioBox(true);
	                    mHander.sendEmptyMessage(EVENT_OP_FIN);
	                }
	                mInStartTest = false;
                    Elog.d(TAG, "++++ Thread 2 , End +++++");
	            }

	        }.start();		
		}
    }

    private NfcNativeCallClass.nfc_test_request makeRequest(boolean isStart) {
        NfcNativeCallClass.nfc_test_request req =
            new NfcNativeCallClass.nfc_test_request();
        req.which = mSelectionId;

        switch (mSelectionId) {
        case TEST_ID_ALWAYSE_ON_WITH:
        case TEST_ID_ALWAYSE_ON_WO:
            NfcNativeCallClass.nfc_tx_alwayson_request nfcTxOnReq =
                new NfcNativeCallClass.nfc_tx_alwayson_request();
            nfcTxOnReq.action = isStart ? TEST_START : TEST_STOP;
            nfcTxOnReq.type = 1; // always by protocol.

            byte modPos =
                (byte) mAlwayseOnModuleAppendix.getSelectedItemPosition();
            nfcTxOnReq.modulation_type =
                (modPos == AdapterView.INVALID_POSITION ? 0 : modPos);

            byte bitratePos =
                (byte) mAlwayseOnBitrateAppendix.getSelectedItemPosition();
            nfcTxOnReq.bitrate =
                (bitratePos == AdapterView.INVALID_POSITION ? 0 : modPos);

            req.target = nfcTxOnReq;
            Elog.i(TAG, String.format(
                "action %d, type %d, modulation %d, bitrate %d",
                nfcTxOnReq.action, nfcTxOnReq.type, nfcTxOnReq.modulation_type,
                nfcTxOnReq.bitrate));
            break;
        case TEST_ID_CARD_EMUL_MODE:
            NfcNativeCallClass.nfc_card_emulation_request nfcCardEmulReq =
                new NfcNativeCallClass.nfc_card_emulation_request();
            nfcCardEmulReq.action = isStart ? TEST_START : TEST_STOP;
            nfcCardEmulReq.type = 1; // always by protocol.

            int typePos = mCardEmulModeTypeAppendix.getSelectedItemPosition();
            nfcCardEmulReq.technology = (short) (1 << typePos);

            int protPos = mCardEmulModeProtAppendix.getSelectedItemPosition();
            nfcCardEmulReq.protocols = (short) (1 << protPos);

            req.target = nfcCardEmulReq;
            Elog.i(TAG, String.format(
                "action %d, type %d, technology %d, protocols %d",
                nfcCardEmulReq.action, nfcCardEmulReq.type,
                nfcCardEmulReq.technology, nfcCardEmulReq.protocols));
            break;
        case TEST_ID_UID_RW:
            NfcNativeCallClass.nfc_script_uid_request nfcScriptUidReq =
                new NfcNativeCallClass.nfc_script_uid_request();
            nfcScriptUidReq.action = isStart ? TEST_START : TEST_STOP;
            nfcScriptUidReq.type = 1; // always by protocol.
            nfcScriptUidReq.uid_type = 1; // 1: 4 short. 2: 7 short.
            nfcScriptUidReq.data[0] = 0x01;
            nfcScriptUidReq.data[1] = 0x02;
            nfcScriptUidReq.data[2] = NUM_3F;
            nfcScriptUidReq.data[NUM_3] = NUM_4F;

            req.target = nfcScriptUidReq;
            break;
        case TEST_ID_READER_MODE:
        case TEST_ID_P2P_MODE:
        case TEST_ID_SWP_SELF:
        case TEST_ID_ANTENNA_SELF:
            NfcNativeCallClass.nfc_script_request nfcScriptReq =
                new NfcNativeCallClass.nfc_script_request();
            nfcScriptReq.action = isStart ? TEST_START : TEST_STOP;
            nfcScriptReq.type = 1; // always by protocol.

            req.target = nfcScriptReq;
            break;
        default:
            break;
        }
        return req;
    }

    private void handleResponse(NfcNativeCallClass.nfc_test_response resp) {
        if (resp.target instanceof NfcNativeCallClass.nfc_script_uid_response) {
            NfcNativeCallClass.nfc_script_uid_response uidr =
                (NfcNativeCallClass.nfc_script_uid_response) resp.target;
            if (uidr.result != 0) {
                // if (uidr.result == NfcCommonDef.RESULT_STATUS_TIMEOUT) {
                // //threadSendMsg("ERROR: Send Timeout.");
                // } else {
                threadSendMsg("ERROR: result = " + uidr.result);
                // }
                // mHander.sendEmptyMessage(EVENT_OP_FIN);
            } else {
                String result =
                    "UID Test Result: \nData[" + NfcUtils.printArray(uidr.data)
                        + "]";
                threadSendMsg(result);
            }
        } else if (resp.target instanceof NfcNativeCallClass.nfc_script_response) {
            NfcNativeCallClass.nfc_script_response res =
                (NfcNativeCallClass.nfc_script_response) resp.target;
            if (res.result != 0) {
                // if (res.result == NfcCommonDef.RESULT_STATUS_TIMEOUT) {
                // //threadSendMsg("ERROR: Send Timeout.");
                // } else {
                threadSendMsg("ERROR: result = " + res.result);
                // }
                // mHander.sendEmptyMessage(EVENT_OP_FIN);
            } else {
                threadSendMsg("Result: OK");
            }
        }
    }

    private void threadSendMsg(String s) {
        Message msg = new Message();
        msg.what = EVENT_OP_MSG;
        Bundle bun = new Bundle();
        bun.putString("MSG", s);
        msg.setData(bun);
        mHander.sendMessage(msg);
    }

}
