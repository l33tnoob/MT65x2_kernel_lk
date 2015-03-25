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


package com.mediatek.bluetooth.simap;

import android.telephony.BtSimapOperResponse;
import android.util.Log;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.content.Context;
import com.android.internal.telephony.ITelephony;


public class AfAdapterResult extends BtSimapOperResponse {
    private static final String TAG = "BtSimapAfAdpter";

    private static final int SIMAP_OP_CONNECT_SIM    = 0;
    private static final int SIMAP_OP_DISCONNECT_SIM = 1;
    private static final int SIMAP_OP_POWER_ON_SIM   = 2;
    private static final int SIMAP_OP_POWER_OFF_SIM  = 3;
    private static final int SIMAP_OP_RESET_SIM      = 4;
    private static final int SIMAP_OP_APDU_REQ       = 5;

    protected int mResultCode;
    private ITelephony iTel;
        
    public AfAdapterResult() {
    	  super();
    	  mResultCode = ERR_NOT_SUPPORTED;
        
        iTel = ITelephony.Stub.asInterface(ServiceManager.getService(Context.TELEPHONY_SERVICE));
    }

    public int getResultCode() {
    	  return mResultCode;
    }

    public void onCommand(int fnCode, int iarg, String sarg) {
        Log.d(TAG, "onCommand(" + fnCode + ", " + iarg + ", " + sarg + ")");

        if(iTel == null){
            Log.e(TAG, "ITelephony is NULL");
            mResultCode = ERR_NOT_SUPPORTED;
            return;
        }

        try {

            switch(fnCode) {
            case SIMAP_OP_CONNECT_SIM:
                Log.d(TAG, "SIMAP_OP_CONNECT_SIM");
                mResultCode = iTel.btSimapConnectSIM(iarg, this);
                Log.d(TAG, "curType: " + getCurType());
                Log.d(TAG, "supportType: " + getSupportType());
                Log.d(TAG, "ATR: " + getAtrString());
                break;
            case SIMAP_OP_DISCONNECT_SIM:
                Log.d(TAG, "SIMAP_OP_DISCONNECT_SIM");
                mResultCode = iTel.btSimapDisconnectSIM();
                break;
            case SIMAP_OP_POWER_ON_SIM:
                Log.d(TAG, "SIMAP_OP_POWER_ON_SIM");
                mResultCode = iTel.btSimapPowerOnSIM(iarg, this);
                Log.d(TAG, "curType: " + getCurType());
                Log.d(TAG, "ATR: " + getAtrString());
                break;
            case SIMAP_OP_POWER_OFF_SIM:
                Log.d(TAG, "SIMAP_OP_POWER_OFF_SIM");
                mResultCode = iTel.btSimapPowerOffSIM();
                break;
            case SIMAP_OP_RESET_SIM:
                Log.d(TAG, "SIMAP_OP_RESET_SIM");
                mResultCode = iTel.btSimapResetSIM(iarg, this);
                Log.d(TAG, "curType: " + getCurType());
                Log.d(TAG, "ATR: " + getAtrString());
                break;
            case SIMAP_OP_APDU_REQ:
                Log.d(TAG, "SIMAP_OP_APDU_REQ");
                mResultCode = iTel.btSimapApduRequest(iarg, sarg, this);
                Log.d(TAG, "RspAPDU: " + getApduString());
                break;
            default:
                Log.d(TAG, "SIMAP_OP_UNKNOWN");
                mResultCode = ERR_NOT_SUPPORTED;
                return;
            } // End of switch(fnCode)

        } catch (RemoteException ex) {
            mResultCode = ERR_DATA_NOT_AVAILABLE;
            Log.e(TAG, "ITelephony api exception:" + ex);
        }
        
        Log.d(TAG, "resultCode: " + mResultCode);
    } // End of onCommand(fnCode, iarg, sarg)

} // End of class AfAdapterResult


