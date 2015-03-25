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

package com.mediatek.FMTransmitter;

public class FMTransmitterNative {
    
    static {
        System.loadLibrary("fmjni");
    }
    static native boolean opendev();
    static native boolean closedev();
    static native boolean powerup(float frequency);
    static native boolean powerdown(int type);
    static native boolean tune(float frequency);
    static native float seek(float frequency, boolean isUp);
    static native short[] autoscan();
    static native boolean stopscan();
    //static native int getrssi();
    static native int rdsset(boolean rdson);
    static native short readrds();
    //static native short getPI();
    //static native byte getPTY();
    static native byte[] getPS();
    static native byte[] getLRText();
    static native short activeAF();
    //static native short[] getAFList();
    static native short activeTA();
    static native short deactiveTA();
    static native int setmute(boolean mute);
    //static native int getchipid();
    static native int isRDSsupport();
    static native short[] getTXFreqList(float curFrequency, int direction, int number);
    static native int isTXSupport();
    static native boolean powerupTX(float frequency);    
    static native int isRDSTXSupport();
    static native boolean setRDSTXEnabled(boolean flag);
    static native boolean setRDSTX(short pi, char[] ps, short[] rds, int rdsCnt);
    static native boolean tuneTX(float frequency);
    /*
    static boolean isTXSupport()
    {
        return true;
    }
    
    static boolean opendev()
    {
        return true;
    }
    
    static boolean closedev()
    {
        return true;
    }
    
    static boolean powerupTX(float frequency)
    {
        return true;
    }
    
    static boolean powerdown()
    {
        return true;
    }
    
    static boolean tune(float frequency)
    {
        return true;
    }
    
    static float[] getTXFreqList(float curFrequency, int number)
    {
        float channelList[] = new float[number];
        for(int i = 0 ;i < number; i++)
        {
            channelList[i] = curFrequency + i * 0.1f;
        }
        return channelList;
    }
    
    

    
    static boolean isRDSTXSupport()
    {
        return true;
    }
    
    static boolean setRDSTXEnabled(boolean state)
    {
        return true;
    }
    
    static boolean setRDSTX(char[] text)
    {
        return true;
    }

    */

}
