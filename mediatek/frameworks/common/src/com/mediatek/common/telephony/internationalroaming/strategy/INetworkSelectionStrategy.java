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

package com.mediatek.common.telephony.internationalroaming.strategy;

/**
 * Defines a set of network selection related interfaces used for
 * international roaming.
 */
public interface INetworkSelectionStrategy extends IBaseStrategy {
    /**
     * Whether we need to boot with GSM modem, checked when telephony framework
     * initialized with C+G, this is used for client to switch to the right mode
     * early.
     * 
     * @return True if need to switch phone to GSM, or else false.
     */
    boolean needToBootOnGsm();

    /**
     * Whether we need to boot with CDMA modem, checked when telephony framework
     * initialized with G+G, this is used for client to switch to the right mode
     * early. 
     * 
     * For example, we fast power off in GSM mode and the power on(IPO),
     * telephony phone framework will create G+G phone, but what we really need
     * is C+G to make the process as the same as normal power off/on, so we need
     * to switch phone in this case.
     * 
     * @return True if need to switch phone to CDMA, or else false.
     */
    boolean needToBootOnCdma();

    /**
     * Callback called before switch phone proceeded, give a chance for client
     * to check whether this switchPhone should be continued, if the return
     * value is not InternationalRoamingConstants.SIM_SWITCH_RESULT_SUCCESS,
     * this time switchPhone will be discard.
     * 
     * @return The pre-check result.
     */
    int onPreSwitchPhone();

    /**
     * Callback called after switch phone proceeded.
     */
    void onPostSwitchPhone();

    /**
     * Callback when GSM suspend to register on network, give a chance for
     * client to determine whether the modem should resume register GSM network
     * or switch to CDMA mode. This can be also called GSM 2nd network
     * selection.
     * 
     * @param plmnString List of PLMN strings get from GSM modem.
     * @param suspendedSession Suspended session id.
     */
    void onGsmSuspend(String[] plmnString, int suspendedSession);

    /**
     * Callback when CDMA suspend to register on network, give a chance for
     * client to determine whether the modem should resume register CDMA network
     * or switch to GSM mode. This can be also called CDMA 2nd network
     * selection.
     * 
     * @param plmnString PLMN string get from CDMA modem.
     */
    void onCdmaPlmnChanged(String plmnString);

    /**
     * Callback when there is no service(service state is
     * REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING). Client may
     * determine whether to keep searching network of current mode or switch to
     * another mode.
     * 
     * @param phoneType The type of the dual phone.
     */
    void onNoService(int phoneType);
}
