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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.engineermode.bluetooth;

/**
 * BT jni java part method.
 * 
 * @author mtk54040
 * 
 */
public class BtTest {
    private int mPatter;
    private int mChannels;
    private int mPocketType;
    private int mPocketTypeLen;
    private int mFreq;
    private int mPower;

    static {
        System.loadLibrary("em_bt_jni");
    }

    // this function is only called in BLE feature test
    public native int getChipId();

    public native int init();

    /*
     * -1: error 0: BLE is not supported 1: BLE is supported
     */
    public native int isBLESupport();

    public native int doBtTest(int kind);

    public native boolean noSigRxTestStart(int nPatternIdx, int nPocketTypeIdx,
            int nFreq, int nAddress);

    public native int[] noSigRxTestResult();

    public native int unInit(); // for uninitialize

    public native char[] hciCommandRun(char[] hciCmd, int cmdLength);

    public native char[] hciReadEvent();

    public native int relayerStart(int portNumber, int serialSpeed);

    public native int relayerExit();

    public native int getChipIdInt();

    public native int getChipEcoNum();

    public native char[] getPatchId();

    public native long getPatchLen();
    
    public native int setFWDump(long value);
    public native int isComboSupport();
    public native int pollingStart();
    public native int pollingStop();
    
    public native boolean setSSPDebugMode(boolean value);

    public BtTest() {
        mPatter = -1;
        mChannels = -1;
        mPocketType = -1;
        mPocketTypeLen = 0;
        mFreq = 0;
        mPower = 7;
    }

//    public int getPatter() {
//        return mPatter;
//    }

//    public int getChannels() {
//        return mChannels;
//    }

    public int getPocketType() {
        return mPocketType;
    }

//    public int getPocketTypeLen() {
//        return mPocketTypeLen;
//    }

    public int getFreq() {
        return mFreq;
    }

//    public int getPower() {
//        return mPower;
//    }

    public void setPatter(int val) {
        mPatter = val;
    }

    public void setChannels(int val) {
        mChannels = val;
    }

    public void setPocketType(int val) {
        mPocketType = val;
    }

    public void setPocketTypeLen(int val) {
        mPocketTypeLen = val;
    }

    public void setFreq(int val) {
        mFreq = val;
    }

    public void setPower(int val) {
        mPower = val;
    }
}
