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

package com.mediatek.perfservice;

import java.lang.Object;
import java.lang.Exception;

import android.content.Context;

import com.mediatek.common.featureoption.FeatureOption;
import com.mediatek.common.perfservice.*;
import com.mediatek.xlog.Xlog;
import android.util.Log;

public class PerfService extends IPerfService.Stub {

    private static final String TAG = "PerfService";

    private IPerfServiceManager perfServiceMgr;

    public PerfService(Context context, IPerfServiceManager pm ) {
        perfServiceMgr = pm;
    }

    public void boostEnable(int scenario) {
        //log("boostEnable");
        perfServiceMgr.boostEnable(scenario);
    }

    public void boostDisable(int scenario) {
        //log("boostDisable");
        perfServiceMgr.boostDisable(scenario);
    }

    public void boostEnableTimeout(int scenario, int timeout) {
        //log("boostEnable");
        perfServiceMgr.boostEnableTimeout(scenario, timeout);
    }

    public void notifyAppState(java.lang.String packName, java.lang.String className, int state) {
        //log("notifyAppState");
        perfServiceMgr.notifyAppState(packName, className, state);
    }

    public int userReg(int scn_core, int scn_freq) {
        //log("userReg");
        return perfServiceMgr.userReg(scn_core, scn_freq);
    }

    public void userUnreg(int handle) {
        //log("userUnreg");
        perfServiceMgr.userUnreg(handle);
    }

    public void userEnable(int handle) {
        //log("userEnable");
        perfServiceMgr.userEnable(handle);
    }

    public void userEnableTimeout(int handle, int timeout) {
        //log("userEnable");
        perfServiceMgr.userEnableTimeout(handle, timeout);
    }

    public void userDisable(int handle) {
        //log("userDisable");
        perfServiceMgr.userDisable(handle);
    }

    public void userResetAll() {
        //log("userDisable");
        perfServiceMgr.userResetAll();
    }

    public void userDisableAll() {
        //log("userDisable");
        perfServiceMgr.userDisableAll();
    }

    private void log(String info) {
        Xlog.d(TAG, "[PerfService] " + info + " ");
    }

    private void loge(String info) {
        Xlog.e(TAG, "[PerfService] ERR: " + info + " ");
    }
}

