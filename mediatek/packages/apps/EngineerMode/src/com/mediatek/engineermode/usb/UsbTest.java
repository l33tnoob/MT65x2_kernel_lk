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

package com.mediatek.engineermode.usb;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

/**
 * Test USB OTG IF/EX function test
 * 
 * @author mtk54040
 * 
 */
public class UsbTest extends Activity implements OnClickListener {
    private Button mBtnEnVbusStart;
    private Button mBtnEnVbusStop;
    private Button mBtnDeVbusStart;
    private Button mBtnDeVbusStop;
    private Button mBtnEnSrpStart;
    private Button mBtnEnSrpStop;
    private Button mBtnDeSrpStart;
    private Button mBtnDeSrpStop;
    private Button mBtnAUutStart;
    private Button mBtnAUutStop;
    private Button mBtnBUutStart;
    private Button mBtnBUutStop;
    private Button mBtnBUutTD59;
    private Button[] mBtnList = null;

    private static final String TAG = "USBTest";

    // dialog ID and MSG ID
    private static final int DLG_STOP = 1;
    private static final int DLG_MSG = 2;
    private static final int DLG_UNKNOW_MSG = 3;
    private static final int DLG_ERROR_MSG = 4;
    // private static final int DLG_IN_PROCESS = 5;

    // private static final int DLGID_OP_IN_PROCESS = 1;
    private static final int OP_IN_PROCESS = 10;
    private static final int OP_FINISH = 11;
    private static final int UPDATAT_MSG = 12;
    private static final int ERROR_MSG = 13;

    private static final int GET_MSG = 20;
    private static final int START_TEST = 21;
    // private static final int STOP_TEST = 22;
    // private static Handler mainHandler = null;
    // ProgressDialog mDialogSearchProgress = null;
    private HandlerThread mResultCollectThread = null;
    private ResultCollectHandler mResultCollectHandler = null;
    private HandlerThread mTestThread = null;
    private TestHandler mTestHandler = null;

    //private WorkHandler mWorkHandler = null; // used to handle the work
                                                // thread

    private static final int ENABLE_VBUS = 0x01;
    private static final int ENABLE_SRP = 0x02;
    private static final int DETECT_SRP = 0x03;
    private static final int DETECT_VBUS = 0x04;
    private static final int A_UUT = 0x05;
    private static final int B_UUT = 0x06;
    private static final int TEST_SE0_NAK = 0x07;
    private static final int TEST_J = 0x08;
    private static final int TEST_K = 0x09;
    private static final int TEST_PACKET = 0x0a;
    private static final int SUSPEND_RESUME = 0x0b;
    private static final int GET_DESCRIPTOR = 0x0c;
    private static final int SET_FEATURE = 0x0d;
    private static final int TD_5_9 = 0X0e;

    private static final int[] BUTTONS_IDS_IF = { R.id.USB_IF_Elec_EnVBUS_Start_ID,
        R.id.USB_IF_Elec_EnVBUS_Stop_ID, R.id.USB_IF_Elec_DeVBUS_Start_ID,
        R.id.USB_IF_Elec_DeVBUS_Stop_ID, R.id.USB_IF_Elec_EnSRP_Start_ID,
        R.id.USB_IF_Elec_EnSRP_Stop_ID, R.id.USB_IF_Elec_DeSRP_Start_ID,
        R.id.USB_IF_Elec_DeSRP_Stop_ID, R.id.USB_IF_Proto_AUUT_Start_ID,
        R.id.USB_IF_Proto_AUUT_Stop_ID, R.id.USB_IF_Proto_BUUT_Start_ID,
        R.id.USB_IF_Proto_BUUT_Stop_ID, R.id.USB_IF_Proto_BUUT_TD5_9_ID };
    private static final int[] BUTTONS_IDS_EX = { R.id.USB_EX_ITEM1_Start_ID,
        R.id.USB_EX_ITEM1_Stop_ID, R.id.USB_EX_ITEM2_Start_ID,
        R.id.USB_EX_ITEM2_Stop_ID, R.id.USB_EX_ITEM3_Start_ID,
        R.id.USB_EX_ITEM3_Stop_ID, R.id.USB_EX_ITEM4_Start_ID,
        R.id.USB_EX_ITEM4_Stop_ID, R.id.USB_EX_ITEM5_Start_ID,
        R.id.USB_EX_ITEM5_Stop_ID, R.id.USB_EX_ITEM6_Start_ID,
        R.id.USB_EX_ITEM6_Stop_ID, R.id.USB_EX_ITEM7_Start_ID,
        R.id.USB_EX_ITEM7_Stop_ID };
    private static final int[] IF_COMMOND = { ENABLE_VBUS, DETECT_VBUS,
            ENABLE_SRP, DETECT_SRP, A_UUT, B_UUT };
    private static final int[] EX_COMMAND = { TEST_SE0_NAK, TEST_J, TEST_K,
            TEST_PACKET, SUSPEND_RESUME, GET_DESCRIPTOR, SET_FEATURE };

    private int mCommand = 0;
    private int mMsg = 0;
    private boolean mRun = false;
    private boolean mTestIf = true;
    private int[] mBtnIds;
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle extra = getIntent().getExtras();
        if (null != extra) {
            mTestIf = extra.getBoolean(UsbList.IF_TEST);
        }
        Xlog.v(TAG, "is test IF ? " + mTestIf);
        if (mTestIf) {
            setContentView(R.layout.usb_test);
            mBtnList = new Button[BUTTONS_IDS_IF.length];
            mBtnIds = BUTTONS_IDS_IF;
        } else {
            setTitle(R.string.USB_EX_TEST);
            setContentView(R.layout.usb_test_ex);
            mBtnList = new Button[BUTTONS_IDS_EX.length];
            mBtnIds = BUTTONS_IDS_EX;
        }
        for (int i = 0; i < mBtnIds.length; i++) {
            mBtnList[i] = (Button) findViewById(mBtnIds[i]);
            mBtnList[i].setOnClickListener(this);
        }
        if (!UsbDriver.nativeInit()) {
            Toast.makeText(this, R.string.USB_not_support, Toast.LENGTH_SHORT)
                    .show();
            finish();
        }

        mResultCollectThread = new HandlerThread(TAG + "/ResultCollect");
        mResultCollectThread.start();
        mResultCollectHandler = new ResultCollectHandler(mResultCollectThread.getLooper());
        mTestThread = new HandlerThread(TAG + "/Test");
        mTestThread.start();
        mTestHandler = new TestHandler(mTestThread.getLooper());
    }

    private Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case OP_IN_PROCESS:
                showDialog(DLG_STOP);
                break;
            case OP_FINISH:
                dismissDialog(DLG_STOP);
                break;
            case UPDATAT_MSG:
                if (mMsg >= UsbDriver.MSG_LEN || mMsg < 0) {
                    showDialog(DLG_UNKNOW_MSG);
                } else {
                    showDialog(DLG_MSG);
                }
                break;
            case ERROR_MSG:
                showDialog(DLG_ERROR_MSG);
                break;
            default:
                break;
            }
        }
    };

    /**
     * Enable or disable the button group
     * 
     * @param enable
     */
    private void updateAllBtn(boolean enable) {
        for (Button btn : mBtnList) {
            btn.setEnabled(enable);
        }
    }

    /**
     * Enable only one button
     * 
     * @param selBtn
     */
    private void makeOneBtnEnable(Button selBtn) {
        for (Button btn : mBtnList) {
            if (btn == selBtn) {
                btn.setEnabled(true);
            } else {
                btn.setEnabled(false);
            }
        }
    }
    
    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        switch (id) {
        case DLG_MSG:
        case DLG_UNKNOW_MSG:
            removeDialog(id);
            break;
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Xlog.d(TAG, "-->onCreateDialog");
        if (id == DLG_STOP) {
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setTitle(R.string.USB_IF_TEST);
            dialog.setMessage(getString(R.string.USB_IF_stop));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);
            return dialog;
        } else if (id == DLG_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    UsbDriver.MSG[mMsg]).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        } else if (id == DLG_UNKNOW_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    String.valueOf(mMsg)).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        } else if (id == DLG_ERROR_MSG) {
            AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                    false).setTitle(R.string.USB_message).setMessage(
                    R.string.USB_msg_err).setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // finish();
                        }
                    }).create();
            return dialog;
        }
        return null;
    }

    @Override
    protected void onDestroy() {
        Xlog.v(TAG, "-->onDestroy");
        if (mRun) {
        	mRun = false;
        	 if (!UsbDriver.nativeStopTest(mCommand)) {
        		 Xlog.w(TAG, "onDestroy() nativeStopTest fail");
             }
        }
        UsbDriver.nativeCleanMsg();
        UsbDriver.nativeDeInit();
        mResultCollectThread.quit();
        mTestThread.quit();
        super.onDestroy();
    }

    public void onClick(View arg0) {
        boolean isSTART = false;
        Button stopBtn = null;
        if (arg0.getId() == BUTTONS_IDS_IF[BUTTONS_IDS_IF.length - 1]) {
            mCommand = TD_5_9;
            isSTART = true;
            stopBtn = mBtnList[mBtnList.length - 3];
        } else {
            int btnIndex = findBtnIndex(arg0.getId());
            mCommand = mTestIf ? IF_COMMOND[btnIndex / 2]
                    : EX_COMMAND[btnIndex / 2];
            isSTART = false;
            if (0 == btnIndex % 2) {
                isSTART = true;
                stopBtn = mBtnList[btnIndex + 1];
            }
        }
        Xlog.v(TAG, "isSTART--" + isSTART);
        Xlog.v(TAG, "command--" + mCommand);
        if (isSTART) {
            UsbDriver.nativeCleanMsg();
            makeOneBtnEnable(stopBtn);
            mTestHandler.sendEmptyMessage(START_TEST);
            mRun = true;
            mResultCollectHandler.sendEmptyMessage(GET_MSG);
        } else if (mRun) {
            mRun = false;
            if (!UsbDriver.nativeStopTest(mCommand)) {
                Toast
                        .makeText(this, R.string.USB_stop_fail,
                                Toast.LENGTH_SHORT).show();
                UsbDriver.nativeCleanMsg();
            }
            updateAllBtn(true);
        }
    }

    private int findBtnIndex(int id) {
        for (int i = 0; i < mBtnIds.length; i++) {
            if (id == mBtnIds[i]) {
                Xlog.d(TAG, "find btn index: " + i);
                return i;
            }
        }
        Xlog.d(TAG, "find btn index error");
        return -1;
    }

    private final class TestHandler extends Handler {
    	TestHandler(Looper looper) {
    		super(looper);
    	}
    	
    	@Override
    	public void handleMessage(Message msg) {
    		switch (msg.what) {
    		case START_TEST:
    			Xlog.v(TAG, "command--" + mCommand);
                if (!UsbDriver.nativeStartTest(mCommand)) {
                    mUiHandler.sendEmptyMessage(ERROR_MSG);
                }
                Xlog.v(TAG, "Task finish");
    			break;
    		default:
    			Xlog.w(TAG, "Unhandled msg: " + msg.what);
    			break;
    		}
    		super.handleMessage(msg);
    	}
    }
    
    private final class ResultCollectHandler extends Handler {
    	ResultCollectHandler(Looper looper) {
    		super(looper);
    	}
    	
    	@Override
    	public void handleMessage(Message msg) {
    		switch (msg.what) {
    		case GET_MSG:
                 if (mRun) {
                	 mMsg = UsbDriver.nativeGetMsg();
                	 Xlog.d(TAG, "getMsg() " + mMsg);
                     if (0 != mMsg) {
                         mUiHandler.sendEmptyMessage(UPDATAT_MSG);
                     } else {
                	     sendEmptyMessageDelayed(GET_MSG, 200);
                     }
                 }
    			break;
    		default:
    			Xlog.w(TAG, "Unhandled msg: " + msg.what);
    			break;
    		}
    		super.handleMessage(msg);
    	}
    }

}
