/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.stuffthathappens.games;

import android.app.ListActivity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ListView;
import android.widget.ArrayAdapter;
//import android.util.Log;

public class OriMenuActivity extends ListActivity {	
	private String[] mString;
	private ArrayAdapter<String> mAdapter;
	private static int ITEM_RAW = 0;
	private static int ITEM_COMPASS = 1;
	private static int ITEM_STABLE = 2;
	private int[] mItemRes;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cube);
        mItemRes = new int[7];
        mItemRes[ITEM_RAW] = R.string.raw_output;
        mItemRes[ITEM_COMPASS] = R.string.compass;
        mItemRes[ITEM_STABLE] = R.string.stable;
        mString = new String[]
                 {
                 	getResources().getString(mItemRes[ITEM_RAW]),
                 	getResources().getString(mItemRes[ITEM_COMPASS]),
                 	getResources().getString(mItemRes[ITEM_STABLE]),
                 };
        mAdapter = new ArrayAdapter<String>(OriMenuActivity.this, R.layout.game_item, mString);
        OriMenuActivity.this.setListAdapter(mAdapter);
    } 
    
    protected void onListItemClick(ListView list, View view, int position, long id)
    {
    	if (id == ITEM_RAW) {
            Intent intent = new Intent();
            intent.setClass(OriMenuActivity.this, OriRawActivity.class);
            startActivity(intent);    		
    	} else if (id == ITEM_COMPASS) {
            Intent intent = new Intent();
            intent.setClass(OriMenuActivity.this, OriCompass.class);
            startActivity(intent);    		
    	} else if (id == ITEM_STABLE) {
            Intent intent = new Intent();
            intent.setClass(OriMenuActivity.this, OriStableActivity.class);
            startActivity(intent);    		    		
    	}    	
    	super.onListItemClick(list,view, position, id);
    }

}