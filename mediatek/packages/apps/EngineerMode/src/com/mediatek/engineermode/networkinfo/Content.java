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

package com.mediatek.engineermode.networkinfo;

public class Content {
    /** Item Index */
    public static final int CELL_INDEX = 0;
    public static final int CHANNEL_INDEX = 1;
    public static final int CTRL_INDEX = 2;
    public static final int RACH_INDEX = 3;
    public static final int LAI_INDEX = 4;
    public static final int RADIO_INDEX = 5;
    public static final int MEAS_INDEX = 6;
    public static final int CA_INDEX = 7;
    public static final int CONTROL_INDEX = 8;
    public static final int SI2Q_INDEX = 9;
    public static final int MI_INDEX = 10;
    public static final int BLK_INDEX = 11;
    public static final int TBF_INDEX = 12;
    public static final int GPRS_INDEX = 13;
    public static final int MM_INFO_INDEX = 53;
    public static final int TCM_MMI_INDEX = 59;
    public static final int CSCE_SERV_CELL_STATUS_INDEX = 75;
    public static final int CSCE_NEIGH_CELL_STATUS_INDEX = 76;
    public static final int CSCE_MULTIPLMN_INDEX = 81;
    public static final int UMTS_CELL_STATUS_INDEX = 90;
    public static final int PERIOD_IC_BLER_REPORT_INDEX = 99;
    public static final int URR_UMTS_SRNC_INDEX = 111;
    public static final int PSDATA_RATE_STATUS_INDEX = 140;
    public static final int HSERV_CELL_INDEX = 155;
    public static final int HANDOVER_SEQUENCE_INDEX = 130;
    public static final int UL_ADM_POOL_STATUS_INDEX = 185;
    public static final int UL_PSDATA_RATE_STATUS_INDEX = 186;
    public static final int UL_HSDSCH_RECONFIG_STATUS_INDEX = 187;
    public static final int UL_URLC_EVENT_STATUS_INDEX = 188;
    public static final int UL_PERIOD_IC_BLER_REPORT_INDEX = 189;

    /** Item data size */
    public static final int CELL_SEL_SIZE = 6;
    public static final int CH_DSCR_SIZE = 340;
    public static final int CTRL_CHAN_SIZE = 14;
    public static final int RACH_CTRL_SIZE = 14;
    public static final int LAI_INFO_SIZE = 28;
    public static final int RADIO_LINK_SIZE = 16;
    public static final int MEAS_REP_SIZE = 1384;
    public static final int CAL_LIST_SIZE = 260;
    public static final int CONTROL_MSG_SIZE = 4;
    public static final int SI2Q_INFO_SIZE = 10;
    public static final int MI_INFO_SIZE = 8;
    public static final int BLK_INFO_SIZE = 80;
    public static final int TBF_INFO_SIZE = 56;
    public static final int GPRS_GEN_SIZE = 32;

    // LXO, stupid code..
    public static final int M3G_MM_EMINFO_SIZE = 30 * 2;
    public static final int M_3G_TCMMMI_INFO_SIZE = 7 * 2;
    public static final int CSCE_SERV_CELL_STATUS_SIZE = 52 * 2;
    public static final int CSCE_MULTI_PLMN_SIZE = 37 * 2;
    public static final int UMTS_CELL_STATUS_SIZE = 772 * 2;
    public static final int PERIOD_IC_BLER_REPORT_SIZE = 100 * 2;
    public static final int URR_UMTS_SRNC_SIZE = 2 * 2;
    public static final int SLCE_PS_DATA_RATE_STATUS_SIZE = 100 * 2;
    public static final int MEME_HSERV_CELL_SIZE = 8 * 2;

    public static final int HANDOVER_SEQUENCE_SIZE = 16 * 2; // alignment enabled
    public static final int ADM_POOL_STATUS_SIZE = 32 * 2;
    public static final int UL2_PSDATA_RATE_STATUS_SIZE = 8 * 2;
    public static final int UL_HSDSCH_RECONFIG_STATUS_SIZE = 8 * 2;
    public static final int URLC_EVENT_STATUS_SIZE = 18 * 2;
    public static final int UL_PERIOD_IC_BLER_REPORT_SIZE = 100 * 2;

    public static final int XGCSCE_NEIGH_CELL_STATUS_SIZE = 520 * 2;
}
