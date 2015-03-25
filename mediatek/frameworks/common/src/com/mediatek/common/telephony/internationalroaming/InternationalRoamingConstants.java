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

public class InternationalRoamingConstants {
    // Type of SIM card.
    public static final int CARD_TYPE_UNKNOWN = 0;
    public static final int CARD_TYPE_DUAL_MODE = 1;
    public static final int CARD_TYPE_SINGLE_CDMA = 2;
    public static final int CARD_TYPE_SINGLE_GSM = 3;

    // Sim switch succeed.
    public static final int SIM_SWITCH_RESULT_SUCCESS = 0;
    // Sim switch failed with unknown error.
    public static final int SIM_SWITCH_RESULT_ERROR_GENERIC = -1;
    // Sim switch failed because it is already in phone switching process.
    public static final int SIM_SWITCH_RESULT_ERROR_BUSY = -2;
    // Sim switch failed because the given type is the same as current phone
    // type.
    public static final int SIM_SWITCH_RESULT_ERROR_SAMETYPE = -3;
    // Sim switch failed because there is no SIM inserted.
    public static final int SIM_SWITCH_RESULT_ERROR_NO_SIM = -4;
    // Sim switch failed because there is no SIM inserted.
    public static final int SIM_SWITCH_RESULT_ERROR_IMSI_NOT_READY = -5;
    // Sim switch failed because the card is not ready or it is single CDMA/GSM
    // card.
    public static final int SIM_SWITCH_RESULT_ERROR_CARD_TYPE = -6;
    // Sim switch failed because GSM2 phone has no service, this only happens
    // when CDMA phone switch to GSM.
    public static final int SIM_SWITCH_RESULT_ERROR_GSM2_NO_SERVCE = -7;
    // Sim switch failed because GSM2 phone has register in home network, only
    // happens when CDMA phone switch to GSM.
    public static final int SIM_SWITCH_RESULT_ERROR_GSM2_REG_IN_HOME = -8;
    // Sim switch failed because GSM2 phone is in a voice call, this only
    // happens when CDMA phone switch to GSM.
    public static final int SIM_SWITCH_RESULT_ERROR_GSM2_IN_VOICE_CALL = -9;

    // Resume network mode.
    public static final int RESUME_NW_GSM = 0;
    public static final int RESUME_NW_CDMA = 1;

    // Swtich the current phone to GSM phone.
    public static final int SIM_SWITCH_MODE_GSM = 0;
    // Swtich the current phone to CDMA phone.
    public static final int SIM_SWITCH_MODE_CDMA = 1;
    // Inverse the phone type, for example, switch to GSM phone if the current
    // is CDMA phone.
    public static final int SIM_SWITCH_MODE_INVERSE = 2;

    public static final int SIM_SWITCHING_STAT_BUSY = -1;
    public static final int SIM_SWITCHING_STAT_AVAILABLE = 0;

    // IR SIM switch start and done message.
    public static final int EVENT_RADIO_IR_SIM_SWITCH_START = 1000;
    public static final int EVENT_RADIO_IR_SIM_SWITCH_DONE = 1001;

    // Radio power on reason.
    public static final int RADIO_ON_REASON_UNKNOWN = 0;
    public static final int RADIO_ON_REASON_TURNOFF_AIRPLANE_MODE = 1;
    public static final int RADIO_ON_REASON_SWITCH_PHONE = 2;

    /**
     * System property used to identify the external modem is GSM or CDMA,
     * MTKPhoneFactory read this property when makeDefaultPhone, if the property
     * value is 0(GSM), then GeminiPhone will create G+G instances, if it is
     * 1(CDMA), GeminiPhone will create C+G phone instances.
     */
    public static final String PROPERTY_EVDO_MODE = "mediatek.evdo.mode.dualtalk";
}
