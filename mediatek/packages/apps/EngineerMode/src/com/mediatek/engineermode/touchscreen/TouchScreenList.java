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

package com.mediatek.engineermode.touchscreen;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;


import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.IOException;
import java.util.ArrayList;

/**
 * TouchScreen test modules
 * 
 * @author mtk54040
 * 
 */
public class TouchScreenList extends Activity implements OnItemClickListener {

    private static final String TAG = "EM/TouchScreen";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.touchscreen);

        ListView listView = (ListView) findViewById(R.id.ListView_TouchScreen);
//        if (listView == null) {
//            Xlog.w(TAG, "clocwork worked...");
//            // not return and let exception happened.
//        }

        ArrayList<String> items = new ArrayList<String>();
        // items.add("HandWriting");
        // items.add("Verification");
        // items.add("Settings");
        // items.add("Rate Report");

        items.add(getString(R.string.TouchScreen_HandWriting));
        items.add(getString(R.string.TouchScreen_Verification));
        items.add(getString(R.string.TouchScreen_Settings));
        items.add(getString(R.string.TouchScreen_RateReport));
        try {
            String[] cmd = { "/system/bin/sh", "-c",
                    "cat /sys/module/tpd_setting/parameters/tpd_type_cap" };
            int ret = TouchScreenShellExe.execCommand(cmd);
            if (0 == ret) {
                Xlog.i(TAG, TouchScreenShellExe.getOutput());
                if (TouchScreenShellExe.getOutput().equalsIgnoreCase("1")) {
                    // items.add("MultiTouch");
                    items.add(getString(R.string.TouchScreen_MultiTouch));
                }
            }

        } catch (IOException e) {
            Xlog.i(TAG, e.toString());
        }


        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(this);
    }

    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = new Intent();
        switch (arg2) {
        case 0:
            intent.setClass(this, TsHandWriting.class);
            break;
        case 1:
            intent.setClass(this, TsVerifyList.class);
            break;
        case 2:
            intent.setClass(this, TouchScreenSettings.class);
            break;
        case 3:
            intent.setClass(this, TsRateReport.class);
            break;
        case 4:
            intent.setClass(this, TsMultiTouch.class);
            break;
        default:
            break;
        }

        this.startActivity(intent);
    }

}
