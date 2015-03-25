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
import android.util.Log;

public class GameCube extends ListActivity {	
	private static final String TAG = "Game_Cube";
	private String[] mString;
	private ArrayAdapter<String> mAdapter;
	private static int ITEM_SENSOR_LIST = 0;
	private static int ITEM_ACC_MENU = 1;
	private static int ITEM_MAG_MENU = 2;	
	private static int ITEM_ORI_MENU = 3;
	private static int ITEM_ALSPS_MENU = 4;
	private static int ITEM_MIX_MENU = 5;
	//private static int ITEM_BUBBLES = 6;
	
	private static int ITEM_PRESS_MENU = 6;
	private static int ITEM_GRAVITY_MENU = 7;
	private static int ITEM_LINEAR_ACC_MENU = 8;
	private static int ITEM_ROTATION_VEC_MENU = 9;
	private static int ITEM_GYRO_MENU = 10;
	private static int ITEM_STRESS_MENU = 11;
	private int[] mItemRes;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.cube);
        mItemRes = new int[16];
        mItemRes[ITEM_SENSOR_LIST] = R.string.sensor_list;
        mItemRes[ITEM_ACC_MENU] = R.string.acc_menu;
        mItemRes[ITEM_MAG_MENU] = R.string.mag_menu;
        mItemRes[ITEM_ORI_MENU] = R.string.ori_menu;  
        mItemRes[ITEM_ALSPS_MENU] = R.string.alsps_menu;
        mItemRes[ITEM_MIX_MENU] = R.string.mix_menu;
        //mItemRes[ITEM_BUBBLES] = R.string.bubbles;
		mItemRes[ITEM_PRESS_MENU] = R.string.press_menu;
		mItemRes[ITEM_GRAVITY_MENU] = R.string.gravity_menu;
		mItemRes[ITEM_LINEAR_ACC_MENU] = R.string.linear_acc_menu;
		mItemRes[ITEM_ROTATION_VEC_MENU] = R.string.rotation_vec_menu;
		mItemRes[ITEM_GYRO_MENU] = R.string.gyro_menu;
		mItemRes[ITEM_STRESS_MENU] = R.string.stress_menu;
		
				
        mString = new String[]
                 {
                 	getResources().getString(mItemRes[ITEM_SENSOR_LIST]),
                 	getResources().getString(mItemRes[ITEM_ACC_MENU]),
                 	getResources().getString(mItemRes[ITEM_MAG_MENU]),
                 	getResources().getString(mItemRes[ITEM_ORI_MENU]),
                 	getResources().getString(mItemRes[ITEM_ALSPS_MENU]),
                    getResources().getString(mItemRes[ITEM_MIX_MENU]),
                 	//getResources().getString(mItemRes[ITEM_BUBBLES]),
                 	//ADD NEW SENSORS
                 	getResources().getString(mItemRes[ITEM_PRESS_MENU]),
                 	getResources().getString(mItemRes[ITEM_GRAVITY_MENU]),
                 	getResources().getString(mItemRes[ITEM_LINEAR_ACC_MENU]),
                 	getResources().getString(mItemRes[ITEM_ROTATION_VEC_MENU]),
                 	getResources().getString(mItemRes[ITEM_GYRO_MENU]),
                    //END ADD NEW SENSORS
                    getResources().getString(mItemRes[ITEM_STRESS_MENU]),
                 	
                 };
        mAdapter = new ArrayAdapter<String>(GameCube.this, R.layout.game_item, mString);
        GameCube.this.setListAdapter(mAdapter);
    } 
    
    protected void onListItemClick(ListView list, View view, int position, long id)
    {
        Log.d(TAG, "id=: "+id);
    	if (id == ITEM_SENSOR_LIST) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, Sensors.class);
            startActivity(intent);  		
    	} else if (id == ITEM_ACC_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, AccMenuActivity.class);
            startActivity(intent);    		
    	} else if (id == ITEM_MAG_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, MagMenuActivity.class);
            startActivity(intent);    		
    	} else if (id == ITEM_ORI_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, OriMenuActivity.class);
            startActivity(intent);    		
    	} else if (id == ITEM_MIX_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, MixMenuActivity.class);
            startActivity(intent);    		  	    		
    	} else if (id == ITEM_ALSPS_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, AlsPsMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	} else if (id == ITEM_GRAVITY_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, GravityMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	} else if (id == ITEM_LINEAR_ACC_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, LinearAccMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	} else if (id == ITEM_ROTATION_VEC_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, RotationVecMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	} else if (id == ITEM_STRESS_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, StressTestMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	}
		else if (id == ITEM_GYRO_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, GyroMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	}
		else if (id == ITEM_PRESS_MENU) {
            Intent intent = new Intent();
            intent.setClass(GameCube.this, PressMenuActivity.class);
            startActivity(intent);    		  	    		    		
    	}
    	
    	super.onListItemClick(list,view, position, id);
    }

}