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

package com.mediatek.smsreg;

import java.io.File;

import android.R.integer;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.mediatek.common.dm.DmAgent;

public class SmsRegConst {

    public static final int CMCCID = 1;
    public static final int CUCOMID = 2;
    public static final int GEMINI_SIM_1 = 0;
    public static final int GEMINI_SIM_2 = 1;
    public static final int[] GEMSIM = { GEMINI_SIM_1, GEMINI_SIM_2 };
    public static final String GEMINI_SIM_ID_KEY = "simId";
    private static final String TAG = "SmsReg/SmsRegConst";
    /**
     * Config files
     */
    private static final String CONFIG_FILE = "smsSelfRegConfig.xml";
    private static final String TEST_PATH_IN_SYSTEM = "/system/etc/dm/test/";
    private static final String PRODUCTIVE_PATH_IN_SYSTEM = "/system/etc/dm/productive/";

    private static final String TEST_PATH_IN_CUSTOM = "/custom/etc/dm/test/";
    private static final String PRODUCTIVE_PATH_IN_CUSTOM = "/custom/etc/dm/productive/";

    public static String getSwitchValue() {
        try {
            IBinder binder = ServiceManager.getService("DmAgent");
            if (binder == null) {
                Log.e("MTKPhone", "ServiceManager.getService(DmAgent) failed.");
                return null;
            }
            DmAgent agent = DmAgent.Stub.asInterface(binder);
            byte[] switchValue = agent.getSwitchValue();
            if (switchValue != null) {
                return new String(switchValue);
            } else {
                return "0";
            }
        } catch (RemoteException e) {
            throw new Error(e);
        }
    }

    public static String getConfigPath() {
        if (getSwitchValue().equals("1")) {
            File smsRegConfig = new File(PRODUCTIVE_PATH_IN_CUSTOM + CONFIG_FILE);
            if (smsRegConfig.exists()) {
                Log.e(TAG, "CIP PRODUCTIVE Smsreg Config Path");
                return PRODUCTIVE_PATH_IN_CUSTOM + CONFIG_FILE;
            } else {
                return PRODUCTIVE_PATH_IN_SYSTEM + CONFIG_FILE;
            }
            
        } else {
            File smsRegConfig = new File(TEST_PATH_IN_CUSTOM + CONFIG_FILE);
            if (smsRegConfig.exists()) {
                Log.e(TAG, "CIP TEST Smsreg Config Path");
                return TEST_PATH_IN_CUSTOM + CONFIG_FILE;
            } else {
                return TEST_PATH_IN_SYSTEM + CONFIG_FILE;
            }

        }
    }

    /**
     * SIM card state: Unknown. Signifies that the SIM is in transition between
     * states. For example, when the user inputs the SIM pin under PIN_REQUIRED
     * state, a query for sim status returns this state before turning to
     * SIM_STATE_READY.
     */
    public static final int SIM_STATE_UNKNOWN = 0;
    /** SIM card state: no SIM card is available in the device */
    public static final int SIM_STATE_ABSENT = 1;
    /** SIM card state: Locked: requires the user's SIM PIN to unlock */
    public static final int SIM_STATE_PIN_REQUIRED = 2;
    /** SIM card state: Locked: requires the user's SIM PUK to unlock */
    public static final int SIM_STATE_PUK_REQUIRED = 3;
    /** SIM card state: Locked: requries a network PIN to unlock */
    public static final int SIM_STATE_NETWORK_LOCKED = 4;
    /** SIM card state: Ready */
    public static final int SIM_STATE_READY = 5;

    /**
     * actions
     */
    public static final String ACTION_BOOTCOMPLETED =
            "android.intent.action.BOOT_COMPLETED";
    public static final String RETRY_SEND_SMSREG =
        "com.mediatek.smsreg.RETRY_SEND_SMSREG";
    public static final String DM_REGISTER_SMS_RECEIVED_ACTION =
            "android.intent.action.DM_REGISTER_SMS_RECEIVED";
    public static final String ACTION_SIM_STATE_CHANGED =
            "android.intent.action.SIM_STATE_CHANGED";
    
    public static final String ACTION_PREPARE_CONFIRM_DIALOG =
        "com.mediatek.smsreg.NOTIFY_CONFIRM_DIALOG";
    
    public static final String ACTION_CONFIRM_DIALOG_START =
        "com.mediatek.smsreg.SEND_MESSAGE_CONFIRM_DIALOG_START";
    
    public static final String ACTION_CONFIRM_DIALOG_END =
        "com.mediatek.smsreg.SEND_MESSAGE_CONFIRM_DIALOG_END";
    
    //for reset CollectSetPermission file,if there is new sim card re-register
    public static final String DM_SMSREG_MESSAGE_NEW = 
        "com.mediatek.mediatekdm.smsreg.new";
    /**
     * Extra key
     */
    //
    public static final String EXTRA_SLOT_ID = "slotid";
    public static final String EXTRA_IS_NEED_SEND_MSG = "isneedsend";
    //Notification id
    public static final int ID_NOTIFICATION_SEND_MSG_DIALOG = 1;
    //do not modify DM_BOOT_START_ENABLE_KEY ,
    public static final String DM_BOOT_START_ENABLE_KEY = "dm_boot_start_enable_key";
    public static final int DEF_DM_BOOT_START_ENABLE_VALUE = 1;
}
