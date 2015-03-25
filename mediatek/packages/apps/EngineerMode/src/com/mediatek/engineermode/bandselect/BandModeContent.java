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

package com.mediatek.engineermode.bandselect;

public class BandModeContent {
    /** GSM mode bit */
    public static final int GSM_EGSM900_BIT = 1;
    public static final int GSM_DCS1800_BIT = 3;
    public static final int GSM_PCS1900_BIT = 4;
    public static final int GSM_GSM850_BIT = 7;
    /** UMTS mode bit */
    public static final int UMTS_BAND_I_BIT = 0;
    public static final int UMTS_BAND_II_BIT = 1;
    public static final int UMTS_BAND_III_BIT = 2;
    public static final int UMTS_BAND_IV_BIT = 3;
    public static final int UMTS_BAND_V_BIT = 4;
    public static final int UMTS_BAND_VI_BIT = 5;
    public static final int UMTS_BAND_VII_BIT = 6;
    public static final int UMTS_BAND_VIII_BIT = 7;
    public static final int UMTS_BAND_IX_BIT = 8;
    public static final int UMTS_BAND_X_BIT = 9;

    /** Event or message id */
    public static final int EVENT_QUERY_SUPPORTED = 100;
    public static final int EVENT_QUERY_CURRENT = 101;
    public static final int EVENT_SET = 110;
    public static final int EVENT_SET_OK = 0;
    public static final int EVENT_SET_FAIL = 1;

    public static final int GSM_MAX_VALUE = 0xFF;
    public static final int UMTS_MAX_VALUE = 0xFFFF;

    /** AT Command */

    public static final String QUERY_SUPPORT_COMMAND = "AT+EPBSE=?";
    public static final String QUERY_CURRENT_COMMAND = "AT+EPBSE?";
    public static final String SET_COMMAND = "AT+EPBSE=";
    public static final String SAME_COMMAND = "+EPBSE:";

    /** SIM Type */

    public static final int GEMINI_SIM_1 = 1;
    public static final int GEMINI_SIM_2 = 2;
    public static final int GEMINI_SIM_3 = 3;
    public static final int GEMINI_SIM_4 = 4;

}
