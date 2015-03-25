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

package com.mediatek.engineermode.bandselect;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * The class for select phone1 or phone2
 */

public class BandModeSimSelect extends Activity implements OnItemClickListener {

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.bandmodesimselect);

        ListView simTypeListView = (ListView) findViewById(R.id.listview_bandmode_sim_select);

        ArrayAdapter<String> adapter;
        if (FeatureOption.MTK_GEMINI_SUPPORT) {

            ArrayList<String> array = new ArrayList<String>();
            array.add(getString(R.string.bandmode_sim1));
            array.add(getString(R.string.bandmode_sim2));
            if (FeatureOption.MTK_GEMINI_3SIM_SUPPORT) {
                array.add(getString(R.string.bandmode_sim3));
            }
            if (FeatureOption.MTK_GEMINI_4SIM_SUPPORT) {
                array.add(getString(R.string.bandmode_sim4));
            }
            adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_1, array);
        } else {
            ArrayList<String> array = new ArrayList<String>();
            array.add(getString(R.string.bandmode_sim1));
            adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_list_item_1, array);
        }

        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);

    }

    /**
     * @see android.widget.AdapterView.OnItemClickListener#onItemClick(android.widget.AdapterView,
     *      android.view.View, int, long)
     * @param parent the Adapter for  parent
     * @param view the View to display
     * @param position the integer of item position
     * @param id the long of ignore
     */
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        Intent intent = new Intent();
        int simType;
        switch (position) {
        case 0:
            intent.setClass(this, BandSelect.class);
            simType = BandModeContent.GEMINI_SIM_1;
            intent.putExtra("mSimType", simType);
            break;
        case 1:
            intent.setClass(this, BandSelect.class);
            simType = BandModeContent.GEMINI_SIM_2;
            intent.putExtra("mSimType", simType);
            break;
        case 2:
            intent.setClass(this, BandSelect.class);
            simType = BandModeContent.GEMINI_SIM_3;
            intent.putExtra("mSimType", simType);
            break;
        case 3:
            intent.setClass(this, BandSelect.class);
            simType = BandModeContent.GEMINI_SIM_4;
            intent.putExtra("mSimType", simType);
        default:
            break;
        }
        this.startActivity(intent);

    }
}
