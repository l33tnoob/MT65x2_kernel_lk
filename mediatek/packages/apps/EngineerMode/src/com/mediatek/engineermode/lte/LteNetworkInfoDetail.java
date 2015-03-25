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

package com.mediatek.engineermode.lte;

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

public class LteNetworkInfoDetail extends Activity implements OnClickListener {
    private static final String TAG = "EM/LteNetworkInfo";
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
    private int mSimType;
    private Phone mPhone = null;
    private GeminiPhone mGeminiPhone = null;
    private Timer mTimer = new Timer();
    private int mFlag = 0;

    static class NetworkInfoPage {
        public String label;
        public int[] types;

        NetworkInfoPage(String label, int[] types) {
            this.label = label;
            this.types = types;
        }
    }

    private HashMap<Integer, String> mNetworkInfo = new HashMap<Integer, String>();
    private NetworkInfoPage[] mItem = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.lte_networkinfo_detail);

        String pageLabels[] = getResources().getStringArray(R.array.lte_network_info_labels);

        mItem = new NetworkInfoPage[pageLabels.length];
        mSimType = getIntent().getIntExtra("mSimType", PhoneConstants.GEMINI_SIM_1);
        int[] checked = getIntent().getIntArrayExtra("mChecked");
        if (null != checked) {
            for (int i = 0; i < checked.length; i++) {
                if (1 == checked[i]) {
                    mItem[mItemCount] = new NetworkInfoPage(pageLabels[i], UrcParser.getTypes(i));
                    mItemCount++;
                }
            }
        } else { // Should not happen
            finish();
            return;
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
        String name = mItem[mCurrentItem].label;
        int[] types = mItem[mCurrentItem].types;

        String text = "<" + (mCurrentItem + 1) + "/" + mItemCount + "> " + "[" + name + "]\n";
        for (int i = 0; i < types.length; i++) {
            String raw = mNetworkInfo.get(types[i]);
            String info = new String(UrcParser.parse(types[i], raw == null ? null : raw.toCharArray()));

            // Special handling for 4G info on MMDC page
            if (name.equals("MMDC") && i == 4) {
                text += "[4G]\n";
                String[] s = info.split("\n");
                for (int j = 1; j < 5; j++) {
                    text += s[j] + "\n";
                }
            } else {
                text += info;
            }
        }
        mInfo.setText(text);
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
                    for (int i = 0; i < mItemCount; i++) {
                        for (int j = 0; j < mItem[i].types.length; j++) {
                            String[] atCommand = new String[2];
                            atCommand[0] = "AT+EINFO=" + mFlag + "," + mItem[i].types[j] + ",0";
                            atCommand[1] = "+EINFO";
                            sendATCommand(atCommand, MSG_NW_INFO_OPEN);
                        }
                    }
                }
                // fall through
            case MSG_NW_INFO_OPEN:
            case MSG_NW_INFO_CLOSE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    showToast(getString(R.string.send_at_fail));
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
                    type = Integer.parseInt(data[0]);
                } catch (NumberFormatException e) {
                    showToast("Return type error");
                    return;
                }

                mNetworkInfo.put(type, data[1]);

                int size = 2 * UrcParser.size(type);
                if (size != data[1].length()) {
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

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }
}

