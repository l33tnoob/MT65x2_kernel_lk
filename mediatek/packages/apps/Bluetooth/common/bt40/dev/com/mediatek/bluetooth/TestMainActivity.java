/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.bluetooth;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;

import com.mediatek.bluetooth.ilm.MsgTestService;
import com.mediatek.bluetooth.ilm.ilm_native;
import com.mediatek.bluetooth.prx.monitor.PrxmDeviceMgmtActivity;
import com.mediatek.bluetooth.service.IBluetoothPrxm;
import com.mediatek.bluetooth.service.IBluetoothPrxr;
import com.mediatek.bluetooth.settings.BluetoothAdvancedSettingsActivity;
import com.mediatek.bluetooth.sys.ts.TestStackService;
import com.mediatek.bluetooth.util.BtLog;

public class TestMainActivity extends Activity implements View.OnClickListener {

    private static final String[] TEST_CASE = {
            "Start Test Stack Service", "Print Message Id", "Start Proximity Monitor Service", "Start Proximity Monitor UI",
            "Start Proximity Reporter Service", "Start Proximity Reporter UI", "Test Message - GlueGen"
    };

    private Button[] mTestBtn;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        LinearLayout ll = new LinearLayout(this);
        ll.setOrientation(LinearLayout.VERTICAL);
        ll.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
        this.setContentView(ll);

        this.mTestBtn = new Button[TEST_CASE.length];
        for (int i = 0; i < this.mTestBtn.length; i++) {

            this.mTestBtn[i] = new Button(this);
            this.mTestBtn[i].setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT));
            this.mTestBtn[i].setText(TEST_CASE[i]);
            this.mTestBtn[i].setOnClickListener(this);
            ll.addView(this.mTestBtn[i]);
        }
    }

    public void onClick(View view) {

        int index = 0;

        // Start Test Stack Service
        if (view == this.mTestBtn[index++]) {

            this.startService(new Intent(TestStackService.ACTION_START_STACK_SERVICE));
        } else if (view == this.mTestBtn[index++]) { // Print Message Id
            ilm_native.print_message_id();
        } else if (view == this.mTestBtn[index++]) { // Start Proximity Monitor Service

            Intent intent = new Intent(IBluetoothPrxm.class.getName());
            this.startService(intent);
        } else if (view == this.mTestBtn[index++]) { // Start Proximity Monitor UI

            Intent intent = new Intent(PrxmDeviceMgmtActivity.ACTION_START);
            // intent.putExtra( ProximityMonitorSettings.EXTRA_BD_ADDR, "7E:3F:4E:66:11:72" );
            this.startActivity(intent);
        } else if (view == this.mTestBtn[index++]) { // Start Proximity Reporter Service

            Intent intent = new Intent(IBluetoothPrxr.class.getName());
            this.startService(intent);
        } else if (view == this.mTestBtn[index++]) { // Start Proximity Monitor UI (Advanced Settings)

            Intent intent = new Intent(this.getApplicationContext(), BluetoothAdvancedSettingsActivity.class);
            this.startActivity(intent);
        } else if (view == this.mTestBtn[index++]) { // Test Message - GlueGen

            BtLog.w("Start test Message.............");
            // MsgTestService mts1 = new MsgTestService( new com.mediatek.bluetooth.sys.msg.GluegenMsgProvider() );
            // mts1.startTest();
            MsgTestService mts2 = new MsgTestService(new com.mediatek.bluetooth.ilm.MtkgenMsgProvider());
            mts2.startTest();
        }
    }
}
