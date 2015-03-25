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
package com.mediatek.voicecommand.data;

import android.os.Bundle;

import com.mediatek.common.voicecommand.VoiceCommandListener;

public class DataPackage {

    /*
     * Just to package the sending data for apps
     */
    private DataPackage(){

    }

    public static Bundle packageResultInfo(int resultid, int extrainfo,
            String extrainfo1) {
        Bundle bundle = new Bundle();

        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO, extrainfo);
        bundle.putString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageResultInfo(int resultid, boolean extrainfo,
            String extrainfo1) {
        Bundle bundle = new Bundle();

        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putBoolean(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                extrainfo);
        bundle.putString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageResultInfo(int resultid, int extrainfo,
            int extrainfo1) {
        Bundle bundle = new Bundle();

        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO, extrainfo);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageResultInfo(int resultid, String extrainfo,
            String extrainfo1) {
        Bundle bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                extrainfo);
        bundle.putString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageResultInfo(int resultid, String[] extrainfo,
            int[] extrainfo1) {
        Bundle bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putStringArray(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                extrainfo);
        bundle.putIntArray(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageResultInfo(int resultid, String[] extrainfo,
            int extrainfo1) {
        Bundle bundle = new Bundle();
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT, resultid);
        bundle.putStringArray(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                extrainfo);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1,
                extrainfo1);

        return bundle;
    }

    public static Bundle packageErrorResult(int errorid) {

        return packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_ERROR, errorid, null);
    }

    public static Bundle packageSuccessResult() {
        return packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, 0, null);
    }

    public static Bundle packageSendInfo(String[] sendinfo, String[] sendinfo1) {
        Bundle bundle = new Bundle();
        bundle.putStringArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO,
                sendinfo);
        bundle.putStringArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1,
                sendinfo1);

        return bundle;
    }
}
