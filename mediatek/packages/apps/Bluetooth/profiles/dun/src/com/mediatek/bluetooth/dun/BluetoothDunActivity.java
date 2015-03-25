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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*******************************************************************************
 *
 * Filename:
 * ---------
 * BluetoothDunActivity.java
 *
 * Project:
 * --------
 *   BT Project
 *
 * Description:
 * ------------
 *   This file is used to create DUN dialog activity
 *
 * Author:
 * -------
 * Ting Zheng
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision: 
 * $Modtime:
 * $Log: 
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/
package com.mediatek.bluetooth.dun;

import com.mediatek.bluetooth.R;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDun;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
//import android.preference.Preference;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
//import android.widget.CompoundButton;
//import android.widget.EditText;
import android.widget.TextView;
import android.widget.Button;
//import android.widget.CompoundButton.OnCheckedChangeListener;
//import android.text.format.Formatter;

import com.android.internal.app.AlertActivity;
import com.android.internal.app.AlertController;

public class BluetoothDunActivity extends AlertActivity 
	implements DialogInterface.OnClickListener
{
    private static final String TAG = "BluetoothDunActivity";
    private static final boolean debug = true;

    private boolean response = false;

    private View mView;
    private TextView messageView;
    
    private BroadcastReceiver mReceiver = new BroadcastReceiver() 
    {
        @Override
        public void onReceive(Context context, Intent intent) 
        {
            String action = intent.getAction();
            if (action.equals(BluetoothDun.STATE_CHANGED_ACTION)) 
            {
                int state = intent.getIntExtra(BluetoothDun.EXTRA_STATE, BluetoothDun.STATE_DISCONNECTED);
                if (state == BluetoothDun.STATE_DISCONNECTED)	
                {
                    response = true;
                    onDisconnect();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) 
    {
        if (debug) Log.d(TAG, "DUN activity on create");
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        BluetoothDevice device = (BluetoothDevice)intent.getExtra(BluetoothDunService.EXTRA_DEVICE);

        showDunDialog(device);

        registerReceiver(mReceiver, new IntentFilter(
            BluetoothDun.STATE_CHANGED_ACTION));
    }

    private void showDunDialog(BluetoothDevice device) 
    {
        final AlertController.AlertParams p = mAlertParams;

        p.mIconId = android.R.drawable.ic_dialog_info;
        p.mTitle = getString(R.string.bluetooth_dun_authorize_title);
        p.mView = createView(device);
        p.mPositiveButtonText = getString(R.string.bluetooth_dun_authorize_allow);
        p.mPositiveButtonListener = this;
        p.mNegativeButtonText = getString(R.string.bluetooth_dun_authorize_decline);
        p.mNegativeButtonListener = this;
        //mOkButton = mAlert.getButton(DialogInterface.BUTTON_POSITIVE);
        setupAlert();
    }

    private String createDisplayText(BluetoothDevice device) 
    {
        String mRemoteName = device.getName();
        String mMessage = getString(R.string.bluetooth_dun_authorize_message, mRemoteName);
        return mMessage;
    }

    private View createView(BluetoothDevice device) 
    {
        mView = getLayoutInflater().inflate(R.layout.bt_dun_access, null);
        messageView = (TextView)mView.findViewById(R.id.message);
        if (messageView != null)
        {
            messageView.setText(createDisplayText(device));
        }
        return mView;
    }

    public void onClick(DialogInterface dialog, int which) 
    {
        switch (which) 
        {
            case DialogInterface.BUTTON_POSITIVE:
                sendAuthResult(BluetoothDunService.RESULT_USER_ACCEPT);
                break;
            
            case DialogInterface.BUTTON_NEGATIVE:
                sendAuthResult(BluetoothDunService.RESULT_USER_REJECT);
                break;
            default:
                break;
        }
        finish();		
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) 
    {
        if ((keyCode == KeyEvent.KEYCODE_BACK) ||
	   (keyCode == KeyEvent.KEYCODE_HOME))
        {
            if (debug) Log.d(TAG, "onKeyDown(): back key, so reject the incoming request");
            sendAuthResult(BluetoothDunService.RESULT_USER_REJECT);
            finish();
        }
        return true;
    }

    private void onDisconnect() 
    {
        finish();
    }

    @Override
    protected void onPause() 
    {
        super.onPause();
        if (debug) Log.d(TAG, "onPause()");
    }

    @Override
    protected void onStop() 
    {
        super.onStop();
        if (debug) Log.d(TAG, "onStop()");

        //re-send incoming notification when incoming request is active
        if (!response)
        {
            Intent intent = new Intent(BluetoothDunService.RESEND_NOTIFICATION_ACTION);
            sendBroadcast(intent);
        }
    }

    @Override
    protected void onDestroy() 
    {
        if (debug) Log.d(TAG, "onDestroy()");
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    private void sendAuthResult(int result)
    {
        response = true;
        if (debug) Log.d(TAG, "sendAuthResult: accept=" + result);	
        Intent intent = new Intent(BluetoothDunService.ACCESS_RESPONSE_ACTION);
        intent.putExtra(BluetoothDunService.EXTRA_ACCESS_RESULT, result);
        sendBroadcast(intent);
    }

}
