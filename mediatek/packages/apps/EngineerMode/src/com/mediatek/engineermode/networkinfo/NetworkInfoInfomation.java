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

package com.mediatek.engineermode.networkinfo;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gemini.GeminiPhone;
import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

public class NetworkInfoInfomation extends Activity implements OnClickListener {
    private static final String TAG = "NetworkInfo";
    private static final int MSG_NW_INFO = 1;
    private static final int MSG_NW_INFO_URC = 2;
    private static final int MSG_NW_INFO_OPEN = 3;
    private static final int MSG_NW_INFO_CLOSE = 4;
    private static final int MSG_UPDATE_UI = 5;
    private static final int TOTAL_TIMER = 1000;
    private static final int FLAG_OR_DATA = 0xF7;
    private static final int FLAG_OFFSET_BIT = 0x08;
    private static final int FLAG_DATA_BIT = 8;

    private Button mPageUp;
    private Button mPageDown;
    private TextView mInfo;
    private Toast mToast = null;

    private int mItemCount = 0;
    private int mCurrentItem = 0;
    private int mItem[];
    private int mSimType;
    private NetworkInfoUrcParser mUrcParser = new NetworkInfoUrcParser();
    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    private Timer mTimer = new Timer();
    private int mFlag = 0;

    static class NetworkInfoItem {
        public String name;
        public String info;
        public int size;    // Only for error checking

        NetworkInfoItem(String name, int size) {
            this.name = name;
            this.info = new String();
            this.size = size;
        }
    }

    private HashMap<Integer, NetworkInfoItem> mNetworkInfo = new HashMap<Integer, NetworkInfoItem>() {{
        put(Content.CELL_INDEX,    new NetworkInfoItem("RR Cell Sel", Content.CELL_SEL_SIZE));
        put(Content.CHANNEL_INDEX, new NetworkInfoItem("RR Ch Dscr", Content.CH_DSCR_SIZE));
        put(Content.CTRL_INDEX,    new NetworkInfoItem("RR Ctrl chan", Content.CTRL_CHAN_SIZE));
        put(Content.RACH_INDEX,    new NetworkInfoItem("RR RACH Ctrl", Content.RACH_CTRL_SIZE));
        put(Content.LAI_INDEX,     new NetworkInfoItem("RR LAI Info", Content.LAI_INFO_SIZE));
        put(Content.RADIO_INDEX,   new NetworkInfoItem("RR Radio Link", Content.RADIO_LINK_SIZE));
        put(Content.MEAS_INDEX,    new NetworkInfoItem("RR Meas Rep", Content.MEAS_REP_SIZE));
        put(Content.CA_INDEX,      new NetworkInfoItem("RR Ca List", Content.CAL_LIST_SIZE));
        put(Content.CONTROL_INDEX, new NetworkInfoItem("RR Control Msg", Content.CONTROL_MSG_SIZE));
        put(Content.SI2Q_INDEX,    new NetworkInfoItem("RR SI2Q Info", Content.SI2Q_INFO_SIZE));
        put(Content.MI_INDEX,      new NetworkInfoItem("RR MI Info", Content.MI_INFO_SIZE));
        put(Content.BLK_INDEX,     new NetworkInfoItem("RR BLK Info", Content.BLK_INFO_SIZE));
        put(Content.TBF_INDEX,     new NetworkInfoItem("RR TBF Info", Content.TBF_INFO_SIZE));
        put(Content.GPRS_INDEX,    new NetworkInfoItem("RR GPRS Gen", Content.GPRS_GEN_SIZE));
        put(Content.MM_INFO_INDEX, new NetworkInfoItem("RR 3G MM EM Info", Content.M3G_MM_EMINFO_SIZE));
        put(Content.TCM_MMI_INDEX, new NetworkInfoItem("RR 3G TCM MMI EM Info", Content.M_3G_TCMMMI_INFO_SIZE));
        put(Content.CSCE_SERV_CELL_STATUS_INDEX,     new NetworkInfoItem("RR 3G CsceEMServCellSStatusInd", Content.CSCE_SERV_CELL_STATUS_SIZE));
        put(Content.CSCE_NEIGH_CELL_STATUS_INDEX,    new NetworkInfoItem("RR xG CsceEMNeighCellSStatusIndStructSize", Content.XGCSCE_NEIGH_CELL_STATUS_SIZE));
        put(Content.CSCE_MULTIPLMN_INDEX,            new NetworkInfoItem("RR 3G CsceEmInfoMultiPlmn", Content.CSCE_MULTI_PLMN_SIZE));
        put(Content.UMTS_CELL_STATUS_INDEX,          new NetworkInfoItem("RR 3G MemeEmInfoUmtsCellStatus", Content.UMTS_CELL_STATUS_SIZE));
        put(Content.PERIOD_IC_BLER_REPORT_INDEX,     new NetworkInfoItem("RR 3G MemeEmPeriodicBlerReportInd", Content.PERIOD_IC_BLER_REPORT_SIZE));
        put(Content.URR_UMTS_SRNC_INDEX,             new NetworkInfoItem("RR 3G UrrUmtsSrncId", Content.URR_UMTS_SRNC_SIZE));
        put(Content.PSDATA_RATE_STATUS_INDEX,        new NetworkInfoItem("RR 3G SlceEmPsDataRateStatusInd", Content.SLCE_PS_DATA_RATE_STATUS_SIZE));
        put(Content.HSERV_CELL_INDEX,                new NetworkInfoItem("RR 3G MemeEmInfoHServCellInd", Content.MEME_HSERV_CELL_SIZE));
        put(Content.HANDOVER_SEQUENCE_INDEX,         new NetworkInfoItem("RR 3G HandoverSequenceIndStuct", Content.HANDOVER_SEQUENCE_SIZE));
        put(Content.UL_ADM_POOL_STATUS_INDEX,        new NetworkInfoItem("RR 3G Ul2EmAdmPoolStatusIndStruct", Content.ADM_POOL_STATUS_SIZE));
        put(Content.UL_PSDATA_RATE_STATUS_INDEX,     new NetworkInfoItem("RR 3G Ul2EmPsDataRateStatusIndStruct", Content.UL2_PSDATA_RATE_STATUS_SIZE));
        put(Content.UL_HSDSCH_RECONFIG_STATUS_INDEX, new NetworkInfoItem("RR 3G Ul2EmHsdschReconfigStatusIndStruct", Content.UL_HSDSCH_RECONFIG_STATUS_SIZE));
        put(Content.UL_URLC_EVENT_STATUS_INDEX,      new NetworkInfoItem("RR 3G Ul2EmUrlcEventStatusIndStruct", Content.URLC_EVENT_STATUS_SIZE));
        put(Content.UL_PERIOD_IC_BLER_REPORT_INDEX,  new NetworkInfoItem("RR 3G Ul2EmPeriodicBlerReportInd", Content.UL_PERIOD_IC_BLER_REPORT_SIZE));
    }};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkinfo_info);

        // get the selected item and store its ID into the mItem array
        mItem = new int[NetworkInfo.TOTAL_ITEM_NUM];
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.GEMINI_SIM_1);
        int[] checked = intent.getIntArrayExtra("mChecked");
        if (null != checked) {
            for (int i = 0; i < checked.length; i++) {
                if (1 == checked[i]) {
                    mItem[mItemCount] = i;
                    mItemCount++;
                }
            }
        }

        int modemType = NetworkInfo.getModemType(mSimType);
        if (modemType == NetworkInfo.MODEM_TD) {
            NetworkInfoItem item = mNetworkInfo.get(Content.CSCE_SERV_CELL_STATUS_INDEX);
            item.size = Content.CSCE_SERV_CELL_STATUS_SIZE - Content.MI_INFO_SIZE;
        }

        mInfo = (TextView) findViewById(R.id.NetworkInfo_Info);
        mPageUp = (Button) findViewById(R.id.NetworkInfo_PageUp);
        mPageDown = (Button) findViewById(R.id.NetworkInfo_PageDown);
        mPageUp.setOnClickListener(this);
        mPageDown.setOnClickListener(this);
        registerNetwork();
    }

    @Override
    protected void onResume() {
        super.onResume();
        updateUI();
    }

    @Override
    public void onStop() {
        mTimer.cancel();
        super.onStop();
    }

    @Override
    public void onDestroy() {
        unregisterNetwork();
        super.onDestroy();
    }

    /*
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */
    public void onClick(View arg0) {
        if (arg0.getId() == mPageUp.getId()) {
            mCurrentItem = (mCurrentItem - 1 + mItemCount) % mItemCount;
            updateUI();
        } else if (arg0.getId() == mPageDown.getId()) {
            mCurrentItem = (mCurrentItem + 1) % mItemCount;
            updateUI();
        }
    }

    public void updateUI() {
        showNetworkInfo();
        mTimer.cancel();
        mTimer = new Timer();
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mUiHandler.sendEmptyMessage(MSG_UPDATE_UI);
            }
        }, TOTAL_TIMER, TOTAL_TIMER);
    }

    private void showNetworkInfo() {
        int type = mItem[mCurrentItem];
        String name = mNetworkInfo.get(type).name;
        String info = mNetworkInfo.get(type).info;
        mInfo.setText("<" + (mCurrentItem + 1) + "/" + mItemCount + ">\n"
                + "[" + name + "]\n" + mUrcParser.parseInfo(type, info, mSimType));
    }

    private void registerNetwork() {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mPhone = PhoneFactory.getDefaultPhone();
            mPhone.registerForNetworkInfo(mUrcHandler, MSG_NW_INFO_URC, null);
        } else {
            mGeminiPhone = (GeminiPhone) PhoneFactory.getDefaultPhone();
            mGeminiPhone.getPhonebyId(mSimType)
                    .registerForNetworkInfo(mUrcHandler, MSG_NW_INFO_URC, null);
        }

        String[] atCommand = { "AT+EINFO?", "+EINFO" };
        sendATCommand(atCommand, MSG_NW_INFO);
    }

    private void unregisterNetwork() {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mPhone.unregisterForNetworkInfo(mUrcHandler);
        } else {
            mGeminiPhone.getPhonebyId(mSimType).unregisterForNetworkInfo(mUrcHandler);
        }

        mFlag = mFlag & FLAG_OR_DATA;
        Xlog.v(TAG, "The close flag is :" + mFlag);
        String[] atCloseCmd = new String[2];
        atCloseCmd[0] = "AT+EINFO=" + mFlag;
        atCloseCmd[1] = "";
        sendATCommand(atCloseCmd, MSG_NW_INFO_CLOSE);
    }

    private void sendATCommand(String[] atCommand, int msg) {
        if (!FeatureOption.MTK_GEMINI_SUPPORT) {
            mPhone.invokeOemRilRequestStrings(atCommand, mATCmdHander.obtainMessage(msg));
        } else {
            mGeminiPhone.invokeOemRilRequestStringsGemini(atCommand,
                    mATCmdHander.obtainMessage(msg), mSimType);
        }
    }

    private Handler mATCmdHander = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case MSG_NW_INFO:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    String data[] = (String[]) ar.result;
                    Xlog.v(TAG, "data[0] is : " + data[0]);
                    Xlog.v(TAG, "flag is : " + data[0].substring(FLAG_DATA_BIT));
                    mFlag = Integer.valueOf(data[0].substring(FLAG_DATA_BIT));
                    mFlag = mFlag | FLAG_OFFSET_BIT;
                    Xlog.v(TAG, "flag change is : " + mFlag);
                    for (int j = 0; j < mItemCount; j++) {
                        String[] atCommand = new String[2];
                        atCommand[0] = "AT+EINFO=" + mFlag + "," + mItem[j] + ",0";
                        atCommand[1] = "+EINFO";
                        sendATCommand(atCommand, MSG_NW_INFO_OPEN);
                    }
                }
                // fall through
            case MSG_NW_INFO_OPEN:
            case MSG_NW_INFO_CLOSE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    Toast.makeText(NetworkInfoInfomation.this, getString(R.string.send_at_fail), Toast.LENGTH_SHORT);
                }
            }
        }
    };

    private final Handler mUrcHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == MSG_NW_INFO_URC) {
                AsyncResult ar = (AsyncResult) msg.obj;
                String data[] = (String[]) ar.result;
                Xlog.v(TAG, "Receive URC: " + data[0] + ", " + data[1]);

                int type = -1;
                try {
                    type  = Integer.parseInt(data[0]);
                } catch (NumberFormatException e) {
                    Toast.makeText(NetworkInfoInfomation.this,
                            "Return type error", Toast.LENGTH_SHORT).show();
                    return;
                }

                NetworkInfoItem item = mNetworkInfo.get(type);
                if (item == null) {
                    Xlog.e(TAG, "Invalid return type: " + type);
                    return;
                }
                item.info = data[1];

                if (item.size != data[1].length()) {
                    Xlog.w(TAG, "Wrong return length: " + data[1].length());
                }

                if (mCurrentItem == type) {
                    showNetworkInfo();
                }
            }
        }
    };

    private final Handler mUiHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.what == MSG_UPDATE_UI) {
                showNetworkInfo();
            }
        }
    };
}

