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

package com.mediatek.FMRadio;

import android.os.Bundle;

/**
 * Activity connect FMRadio service should implements this interface to update ui or status
 *
 */
public interface FMRadioListener {
    
    /**
     * directly call back from service to activity
     */
    // FM RDS station changed
    int LISTEN_RDSSTATION_CHANGED  = 0x00100010;
    
    // FM PS information changed
    int LISTEN_PS_CHANGED          = 0x00100011;
    
    // FM RT information changed
    int LISTEN_RT_CHANGED          = 0x00100100;
    
    // FM Record state changed
    int LISTEN_RECORDSTATE_CHANGED = 0x00100101; //1048833
    
    // FM record error occur
    int LISTEN_RECORDERROR         = 0x00100110; //1048848
    
    // FM record mode change
    int LISTEN_RECORDMODE_CHANGED  = 0x00100111; //4018849
    
    /**
     * bundle key
     */
    String SWITCH_ANNTENNA_VALUE     = "switch_anntenna_value";
    String CALLBACK_FLAG             = "callback_flag";
//    final String KEY_IS_DEVICE_OPEN        = "key_is_device_open";
    String KEY_IS_POWER_UP           = "key_is_power_up";
    String KEY_IS_POWER_DOWN         = "key_is_power_down";
    String KEY_IS_SWITCH_ANNTENNA    = "key_is_switch_anntenna";
    String KEY_IS_TUNE               = "key_is_tune";
    String KEY_TUNE_TO_STATION       = "key_tune_to_station";
    String KEY_IS_SEEK               = "key_is_seek";
    String KEY_SEEK_TO_STATION       = "key_seek_to_station";
    String KEY_IS_SCAN               = "key_is_scan";
    String KEY_RDS_STATION           = "key_rds_station";
    String KEY_PS_INFO               = "key_ps_info";
    String KEY_RT_INFO               = "key_rt_info";
    String KEY_STATION_NUM           = "key_station_num";
    
    // audio focus related
    String KEY_AUDIOFOCUS_CHANGED    = "key_audiofocus_changed";
    
    /************recroding*********/
    String KEY_RECORDING_STATE       = "key_is_recording_state";
    String KEY_RECORDING_ERROR_TYPE   = "key_recording_error_type";
    String KEY_IS_RECORDING_MODE = "key_is_recording_mode";
    
    /**
     * handle message: call back from service to activity
     */
    // Message to handle
    int MSGID_UPDATE_RDS              = 1;
    int MSGID_UPDATE_CURRENT_STATION  = 2;
    int MSGID_ANTENNA_UNAVAILABE      = 3;
    int MSGID_SWITCH_ANNTENNA         = 4;
    int MSGID_SET_RDS_FINISHED        = 5;
    int MSGID_SET_CHANNEL_FINISHED    = 6;
    int MSGID_SET_MUTE_FINISHED       = 7;
    // Fm main
    int MSGID_POWERUP_FINISHED        = 9;
    int MSGID_POWERDOWN_FINISHED      = 10;
    int MSGID_FM_EXIT                 = 11;
    int MSGID_SCAN_CANCELED           = 12;
    int MSGID_SCAN_FINISHED           = 13;
    int MSGID_AUDIOFOCUS_FAILED       = 14;
    int MSGID_TUNE_FINISHED           = 15;
    int MSGID_SEEK_FINISHED           = 16;
    int MSGID_ACTIVE_AF_FINISHED      = 18;
    // Recording
    int MSGID_RECORD_STATE_CHANGED    = 19;
    int MSGID_RECORD_ERROR            = 20;
    int MSGID_RECORD_MODE_CHANED      = 21;
    int MSGID_STARTRECORDING_FINISHED = 22;
    int MSGID_STOPRECORDING_FINISHED  = 23;
    int MSGID_STARTPLAYBACK_FINISHED  = 24;
    int MSGID_STOPPLAYBACK_FINISHED   = 25;
    int MSGID_SAVERECORDING_FINISHED  = 26;
    // audio focus related
    int MSGID_AUDIOFOCUS_CHANGED     = 30;

    int NOT_AUDIO_FOCUS = 33;

    /**
     * Activity final variables
     */
    int MSGID_SHOW_TOAST              = 100;
    int MSGID_REFRESH                 = 101;
    
    /**
     * call back method to activity from service
     */
    void onCallBack(Bundle bundle);
    
//    void onFmExit();
}