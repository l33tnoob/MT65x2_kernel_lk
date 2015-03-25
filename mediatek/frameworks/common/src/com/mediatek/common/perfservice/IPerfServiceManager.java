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

package com.mediatek.common.perfservice;

//import java.io.File;
//import java.io.OutputStreamWriter;
//import java.io.InputStream;
//import java.io.InputStreamReader;
//import java.io.FileInputStream;
//import java.io.BufferedReader;
//import java.io.IOException;
//import java.util.*;

//import android.content.Context;
//import android.content.Intent;
//import android.content.IntentFilter;
//import android.content.BroadcastReceiver;
//import android.os.Handler;
//import android.os.HandlerThread;
//import android.os.Looper;
//import android.os.Message;
//import android.os.Process;
//import android.util.Log;
//import static junit.framework.Assert.*;

public interface IPerfServiceManager {
    public static final int SCN_NONE           = 0;
    public static final int SCN_APP_SWITCH     = 1; /* apply for both launch/exit */
    public static final int SCN_APP_ROTATE     = 2;
    public static final int SCN_SW_CODEC       = 3;
    public static final int SCN_SW_CODEC_BOOST = 4;
    public static final int SCN_APP_TOUCH      = 5;

    public static final int STATE_PAUSED    = 0;
    public static final int STATE_RESUMED   = 1;
    public static final int STATE_DESTROYED = 2;
    public static final int STATE_DEAD      = 3;

    public void systemReady();

    public void boostEnable(int scenario);
    public void boostDisable(int scenario);
    public void boostEnableTimeout(int scenario, int timeout);
    public void notifyAppState(String packName, String className, int state);

    public int  userReg(int scn_core, int scn_freq);
    public void userUnreg(int handle);
    public void userEnable(int handle);
    public void userEnableTimeout(int handle, int timeout);
    public void userDisable(int handle);
    public void userResetAll();
    public void userDisableAll();
}

