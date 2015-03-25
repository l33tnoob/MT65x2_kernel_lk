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

package com.mediatek.engineermode.memory;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import com.mediatek.engineermode.FeatureHelpPage;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.io.File;
import java.util.ArrayList;

public class Memory extends Activity implements OnItemClickListener {

    private static final String TAG = "EM/Memory";
    protected static final String FLASH_TYPE = "HAVE_EMMC";
    private static final String EMMC_PROC_FILE = "/proc/emmc";
    private boolean mHaveEmmc = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.memory);
        ListView itemList = (ListView) findViewById(R.id.list_memory_item);
        mHaveEmmc = new File(EMMC_PROC_FILE).exists();

        ArrayList<String> items = new ArrayList<String>();
        if (mHaveEmmc) {
            items.add(getString(R.string.memory_item_emmc));
        } else {
            items.add(getString(R.string.memory_item_nand));
        }
        items.add(getString(R.string.help));
        // items.add(getString(R.string.memory_item_emi));
        Xlog.v(TAG, "have emmc? " + mHaveEmmc);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        itemList.setAdapter(adapter);
        itemList.setOnItemClickListener(this);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = null;
        switch (arg2) {
        case 0:
            intent = new Intent(this, NandFlash.class);
            break;
        case 1:
            //intent = new Intent(this, EmiRegister.class);
            intent = new Intent(this, FeatureHelpPage.class);
            intent.putExtra(FeatureHelpPage.HELP_TITLE_KEY, R.string.help);
            intent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY, R.string.memory_help_msg);
            break;
        default:
            break;
        }
        if (null == intent) {
            Toast.makeText(this, R.string.memory_select_error,
                    Toast.LENGTH_LONG).show();
            Xlog.d(TAG, "Select error");
        } else {
            intent.putExtra(FLASH_TYPE, mHaveEmmc);
            this.startActivity(intent);
        }
    }
}
