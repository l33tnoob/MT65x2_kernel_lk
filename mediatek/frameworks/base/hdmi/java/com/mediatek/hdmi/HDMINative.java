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

package com.mediatek.hdmi;

import android.util.Log;

import com.mediatek.common.hdmi.IHDMINative;

public class HDMINative implements IHDMINative {

    public final static String TAG = "HdmiNative";

    private static boolean sLoaded = false;

    static {
        try {
            System.loadLibrary("mtkhdmi_jni");
            sLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "HdmiNative load library fail");
        }
        Log.d(TAG, "load libhdminative_jni.so " + sLoaded);
    }

    public boolean enableHDMI(boolean enabled) {
        if (sLoaded) {
            return enableHdmiNative(enabled);
        }
        return false;
    }

    public native boolean enableHdmiNative(boolean enabled);

    public boolean enableHDMIIPO(boolean enabled) {
        return sLoaded ? enableHdmiIpoNative(enabled) : false;
    }

    public native boolean enableHdmiIpoNative(boolean enabled);

    public boolean enableVideo(boolean enabled) {
        return sLoaded ? enableVideoNative(enabled) : false;
    }

    public native boolean enableVideoNative(boolean enabled);

    public boolean enableAudio(boolean enabled) {
        return sLoaded ? enableAudioNative(enabled) : false;
    }

    public native boolean enableAudioNative(boolean enabled);

    public boolean enableCEC(boolean enbaled) {
        return sLoaded ? enableCecNative(enbaled) : false;
    }

    public native boolean enableCecNative(boolean enbaled);

    public boolean enableHDCP(boolean enabled) {
        return sLoaded ? enableHdcpNative(enabled) : false;
    }

    public native boolean enableHdcpNative(boolean enabled);

    public boolean setVideoConfig(int newValue) {
        return sLoaded ? setVideoConfigNative(newValue) : false;
    }

    public native boolean setVideoConfigNative(int newValue);

    public boolean setAudioConfig(int newValue) {
        return sLoaded ? setAudioConfigNative(newValue) : false;
    }

    public native boolean setAudioConfigNative(int newValue);

    public boolean setDeepColor(int colorSpace, int deepColor) {
        return sLoaded ? setDeepColorNative(colorSpace, deepColor) : false;
    }

    public native boolean setDeepColorNative(int colorSpace, int deepColor);

    public boolean setHDCPKey(byte[] key) {
        return sLoaded ? setHdcpKeyNative(key) : false;
    }

    public native boolean setHdcpKeyNative(byte[] key);

    public boolean setHDMIDRMKey() {
        return sLoaded ? setHdmiDrmKeyNative() : false;
    }

    public native boolean setHdmiDrmKeyNative();

    public boolean setCECAddr(byte laNum, byte[] la, char pa, char svc) {
        return sLoaded ? setCecAddrNative(laNum, la, pa, svc) : false;
    }

    public native boolean setCecAddrNative(byte laNum, byte[] la, char pa,
            char svc);

    public boolean setCECCmd(byte initAddr, byte destAddr, char opCode,
            byte[] operand, int size, byte enqueueOk) {
        return sLoaded ? setCecCmdNative(initAddr, destAddr, opCode, operand,
                size, enqueueOk) : false;
    }

    public native boolean setCecCmdNative(byte initAddr, byte destAddr,
            char opCode, byte[] operand, int size, byte enqueueOk);

    public boolean hdmiPowerEnable(boolean enabled) {
        return sLoaded ? hdmiPowerEnableNative(enabled) : false;
    }

    public native boolean hdmiPowerEnableNative(boolean enabled);

    public boolean hdmiPortraitEnable(boolean enabled) {
        return sLoaded ? hdmiPortraitEnableNative(enabled) : false;
    }

    public native boolean hdmiPortraitEnableNative(boolean enabled);

    public boolean isHdmiForceAwake() {
        return sLoaded ? isHdmiForceAwakeNative() : false;
    }

    public native boolean isHdmiForceAwakeNative();

    public int[] getEDID() {
        return sLoaded ? getEdidNative() : null;
    }

    public native int[] getEdidNative();

    public char[] getCECAddr() {
        return sLoaded ? getCecAddrNative() : null;
    }

    public native char[] getCecAddrNative();

    public int[] getCECCmd() {
        return sLoaded ? getCecCmdNative() : null;
    }

    public native int[] getCecCmdNative();

    public boolean notifyOtgState(int otgState) {
        return sLoaded ? notifyOtgStateNative(otgState) : false;
    }

    public native boolean notifyOtgStateNative(int otgState);

    public int getDisplayType() {
        return sLoaded ? getDisplayTypeNative() : 0;
    }

    public native int getDisplayTypeNative();

    public boolean needSwDrmProtect() {
        return sLoaded ? needSwDrmProtectNative() : false;
    }

    public native boolean needSwDrmProtectNative();
}
