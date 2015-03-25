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

package com.mediatek.FMRadio;

/**
 * This class define FM native interface, will description FM native interface
 * 
 */
public class FMRadioNative {
    static {
        System.loadLibrary("fmjni");
    }

    /**
     * open fm device, call before power up
     * 
     * @return (true,success; false, failed)
     */
    static native boolean opendev();

    /**
     * close fm device, call after power down
     * 
     * @return (true, success; false, failed)
     */
    static native boolean closedev();

    /**
     * power up FM with frequency use long antenna
     * 
     * @param frequency
     *            frequency(50KHZ, 87.55; 100KHZ, 87.5)
     * @return (true, success; false, failed)
     */
    static native boolean powerup(float frequency);

    /**
     * power down FM
     * 
     * @param type
     *            (0, FMRadio; 1, FMTransimitter)
     * @return (true, success; false, failed)
     */
    static native boolean powerdown(int type);

    /**
     * tune to frequency
     * 
     * @param frequency
     *            frequency(50KHZ, 87.55; 100KHZ, 87.5)
     * @return (true, success; false, failed)
     */
    static native boolean tune(float frequency);

    /**
     * seek with frequency in direction
     * 
     * @param frequency
     *            frequency(50KHZ, 87.55; 100KHZ, 87.5)
     * @param isUp
     *            (true, next channel; false previous channel)
     * @return frequency(float)
     */
    static native float seek(float frequency, boolean isUp);

    /**
     * auto scan(from 87.50-108.00)
     * 
     * @return scan channel array(short)
     */
    static native short[] autoscan();

    /**
     * stop scan, also can stop seek, other native when scan should call stop scan first, else will
     * execute wait auto scan finish
     * 
     * @return (true, can stop scan process; false, can't stop scan process)
     */
    static native boolean stopscan();

    /**
     * get rssi from hardware(use for engineer mode)
     * 
     * @return rssi value
     */
    static native int readRssi();

    /**
     * open or close rds fuction
     * 
     * @param rdson
     *            rdson (true, open; false, close)
     * @return
     */
    static native int rdsset(boolean rdson);

    /**
     * read rds events
     * 
     * @return rds event type
     */
    static native short readrds();

    /**
     * get program identification
     * 
     * @return program identification
     */
    static native short getPI();

    /**
     * get program type
     * 
     * @return program type
     */
    static native byte getPTY();

    /**
     * get program service(program name)
     * 
     * @return program name
     */
    static native byte[] getPS();

    /**
     * get radio text, RDS standard not support Chinese character
     * 
     * @return radio text(byte)
     */
    static native byte[] getLRText();

    /**
     * active alternative frequencies
     * 
     * @return frequency(float)
     */
    static native short activeAF();

    /**
     * get alternative frequency list
     * 
     * @return alternative frequency list(array)
     */
    static native short[] getAFList();

    /**
     * active traffic announcement
     * 
     * @return traffic announcement channel(short)
     */
    static native short activeTA();

    /**
     * deactive traffic announcement
     * 
     * @return the previous channel(short)
     */
    static native short deactiveTA();

    /**
     * mute or unmute FM voice
     * 
     * @param mute
     *            (true, mute; false, unmute)
     * @return (true, success; false, failed)
     */
    static native int setmute(boolean mute);

    /**
     * get chip id
     * 
     * @return chipId(0x6620, 0x6626 ,0x6628)
     */
    static native int getchipid();

    /**
     * Inquiry if RDS is support in driver
     * 
     * @return (1, support; 0, NOT support; -1, error)
     */
    //WCN Should change it
    static native int isRDSsupport();

    /**
     * Inquiry if FM is powered up if FMRX or FMTX power up, return true
     * 
     * @return (1, Powered up; 0, Did NOT powered up)
     */
     //WCN Should change it
    static native int isFMPoweredUp();

    /**
     * switch antenna
     * 
     * @param antenna
     *            antenna (0, long antenna, 1 short antenna)
     * @return (0, success; 1 failed; 2 not support)
     */
    static native int switchAntenna(int antenna);

    /**
     * Inquiry if fm stereo mono(true, stereo; false mono)
     * 
     * @return (true, stereo; false, mono)
     */
    static native boolean stereoMono();

    /**
     * Force set to stero/mono mode
     * 
     * @param isMono
     *            (true, mono; false, stereo)
     * @return (true, success; false, failed)
     */
    static native boolean setStereoMono(boolean isMono);

    /**
     * Read cap array of short antenna
     * 
     * @return cap array value
     */
    static native short readCapArray();

    /**
     * read rds bler
     * 
     * @return rds bler value
     */
    static native short readRdsBler();

    /**
     * FM over BT
     * 
     * @param bEnable
     *            (true,To enable FM over BT; false,To disable FM over BT)
     * @return (true,success; false, failed)
     */
    static native boolean setFMViaBTController(boolean bEnable);

    /**
     * get hardware version
     * 
     * @return hardware version information array(0, ChipId; 1, EcoVersion; 2, PatchVersion; 3,
     *         DSPVersion)
     */
    static native int[] getHardwareVersion();
    static native short[] scannew(int upper, int lower, int space);
    static native int seeknew(int upper, int lower, int space, int freq, int dir, int lev);
    static native boolean tunenew(int upper, int lower, int space, int freq);
    /**
     * send variables to native, and get some variables return.
     * @param val send to native
     * @return get value from native
     */
    static native short[] emcmd(short[] val);
    /**
     * set RSSI, desense RSSI, mute gain soft
     * @param index flag which will execute
     * (0:rssi threshold,1:desense rssi threshold,2: SGM threshold)
     * @param value send to native
     * @return execute ok or not
     */
    static native boolean emsetth(int index, int value);
}
