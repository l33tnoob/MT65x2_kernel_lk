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

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;
import com.mediatek.xlog.Xlog;

import com.mediatek.common.perfservice.*;

public class PerfServiceWrapper implements IPerfServiceWrapper {

    private static final String TAG = "PerfServiceWrapper";

    private IPerfService sService = null;
    private Context mContext;

    private int inited = 0;

    private void init() {
        if(inited == 0) {
            IBinder b = ServiceManager.checkService(Context.MTK_PERF_SERVICE);
            if(b != null) {
                sService = IPerfService.Stub.asInterface(b);
                if (sService != null)
                    inited = 1;
                else
                    log("ERR: getService() sService is still null..");
            }
        }
    }

    public PerfServiceWrapper(Context context) {
        mContext = context;
        init();
    }

    public void boostEnable(int scenario) {
        //log("boostEnable");
        try {
            init();
            if(sService != null)
                sService.boostEnable(scenario);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void boostDisable(int scenario) {
        //log("boostEnable");
        try {
            init();
            if(sService != null)
                sService.boostDisable(scenario);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void boostEnableTimeout(int scenario, int timeout) {
        //log("boostEnable");
        try {
            init();
            if(sService != null)
                sService.boostEnableTimeout(scenario, timeout);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void notifyAppState(String packName, String className, int state) {
        //log("boostEnable");
        try {
            init();
            if(sService != null)
                sService.notifyAppState(packName, className, state);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public int userReg(int scn_core, int scn_freq) {
        int handle = -1;
        //log("[userReg] - "+scn_core+", "+scn_freq);
        try {
            init();
            if(sService != null)
                handle = sService.userReg(scn_core, scn_freq);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
        //log("[userReg] - handle:"+handle);
        return handle;
    }

    public void userUnreg(int handle) {
        //log("[userUnreg] - "+handle);
        try {
            init();
            if(sService != null)
                sService.userUnreg(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void userEnable(int handle) {
        //log("[userEnable] - "+handle);
        try {
            init();
            if(sService != null)
                sService.userEnable(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void userDisable(int handle) {
        //log("[userDisable] - "+handle);
        try {
            init();
            if(sService != null)
                sService.userDisable(handle);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void userEnableTimeout(int handle, int timeout) {
        //log("[userEnableTimeout] - "+handle+", "+timeout);
        try {
            init();
            if(sService != null)
                sService.userEnableTimeout(handle, timeout);
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void userResetAll() {
        try {
            init();
            if(sService != null)
                sService.userResetAll();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    public void userDisableAll() {
        try {
            init();
            if(sService != null)
                sService.userDisableAll();
        } catch (RemoteException e) {
            loge("ERR: RemoteException in enable:" + e);
        }
    }

    private void log(String info) {
        Xlog.d(TAG, "[PerfServiceWrapper] " + info + " ");
    }

    private void loge(String info) {
        Xlog.e(TAG, "[PerfServiceWrapper] ERR: " + info + " ");
    }
}

