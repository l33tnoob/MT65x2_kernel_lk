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

package com.mediatek.common.telephony.internationalroaming;

/**
 * Define a set of interface used for GeminiPhone to communicate with
 * InternationalRoamingController for international romaing feature.
 */
public interface IInternationalRoamingController {
    /**
     * Request to switch phone to the given mode.
     * 
     * @param mode The target mode, could be SIM_SWITCH_MODE_GSM,
     *            SIM_SWITCH_MODE_CDMA or
     *            SIM_SWITCH_MODE_INVERSE(InternationalRoamingConstants).
     * @param fromUser Whether the switch phone process is triggered by user.
     * @return SIM_SWITCH_RESULT_SUCCESS if we switch successfully, or else with
     *         error code(InternationalRoamingConstants).
     */
    int switchPhone(int mode, boolean fromUser);

    /**
     * Whether the telephony framework is in SIM swithing.
     * 
     * @return True if the telephony framework is in SIM swithing.
     */
    boolean isUnderSimSwitching();

    /**
     * Get the card type of dual phone, the type may be single GSM, single CDMA,
     * dual card or unknow type (InternationalRoamingConstants).
     * 
     * @return The card type of dual phone
     */
    int getDualModePhoneCardType();

    /**
     * Whether we has searched network on GSM mode.
     * 
     * @return True if we has searched network on GSM mode, or else false.
     */
    boolean hasSearchedOnGsm();

    /**
     * Whether we has searched network on CDMA mode.
     * 
     * @return True if we has searched network on CDMA mode, or else false.
     */
    boolean hasSearchedOnCdma();

    /**
     * Whether we need to ignore the searched state, if user force to switch
     * phone, we will try to register on network without checking whether we has
     * searched on CDMA or GSM.
     * 
     * @return True if we need to ignore the searched state.
     */
    boolean ignoreSearchedState();

    /**
     * Whether the given network(MCC) is home network.
     * 
     * @param mcc
     * @return True if the given MCC is the same as MCC of home network.
     */
    boolean isHomeNetwork(String mcc);

    /**
     * Whether we need to boot with GSM modem, checked when telephony framework
     * initialized with C+G, this is used for client to switch to the right mode
     * early.
     * 
     * @param iccid
     * @return True if need to switch phone to GSM, or else false.
     */
    boolean needToBootOnGsm(String iccid);

    /**
     * Whether we need to boot with CDMA modem, checked when telephony framework
     * initialized with G+G, this is used for client to switch to the right mode
     * early.
     * 
     * @param iccid
     * @return True if need to switch phone to CDMA, or else false.
     */
    boolean needToBootOnCDMA(String iccid);

    /**
     * Clear CDMA avoid network list.
     */
    void clearCdmaAvoidNetworkList();

    /**
     * Whether it is the first time to register on network.
     * 
     * @return True if the device is just booted or back from airplane mode.
     */
    boolean isFirstRegistration();

    /**
     * Resume register to the suspend network.
     * 
     * @param networkMode GSM or CDMA.
     * @param suspendedSession suspended session, only valid for GSM mode.
     */
    void resumeRegistration(int networkMode, int suspendedSession);
    
    /**
     * Get phone type of last successful registration, used for memorize network
     * selection.
     * 
     * @return Phone type of last successful registration.
     */
    int getLastPhoneType();

    /**
     * Get MCC information of the last registered network, used for memorize
     * network selection.
     * 
     * @return MCC string of the last registered network.
     */
    String getLastNetworkMcc();

    /**
     * Used to recycle/release resources.
     */
    void dispose();
}
