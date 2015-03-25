/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   bwcs_custom.h
 *
 * Project:
 * --------
 *   YuSu
 *
 * Description:
 * ------------
 *    This file is the header of bwcs customization related function or definition.
 *
 * Author:
 * -------
 *  Saker Hsia(MTK02327) 
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 06 30 2010 saker.hsia
 * [ALPS00002764][Need Patch] [Volunteer Patch] ALPS.10X.W10.28 Volunteer patch for BWCS NVRAM customization 
 * .
 *
 *******************************************************************************/
#ifndef BWCS_CUSTOM_H
#define BWCS_CUSTOM_H


#define RT_RSSI_TH_BT            73
#define RT_RSSI_TH_WIFI1         73
#define RT_RSSI_TH_WIFI2         73

#define NRT_RSSI_TH_BT           73
#define NRT_RSSI_TH_WIFI1        73
#define NRT_RSSI_TH_WIFI2        73
#define ANT_PATH_COMP            10
#define ANT_SWITCH_PROT_TIME     10
#define BT_RX_RANGE              {0xC4, 0xE2}
#if defined (MTK_MT6611)
#define WIFI_TX_FLOW_CTRL        {0x0E00, 0x0001}
#define BT_TX_PWR_WIFI_OFF       0x7
#define BT_TX_PWR_SCO            0x4
#define BT_TX_PWR_ACL            0x4
#elif defined (MTK_MT6612)
#define WIFI_TX_FLOW_CTRL        {0x0E00, 0x0001}
#define BT_TX_PWR_WIFI_OFF       0x7
#define BT_TX_PWR_SCO            0x4
#define BT_TX_PWR_ACL            0x3
#elif defined (MTK_MT6616)
#define WIFI_TX_FLOW_CTRL        {0x1100, 0x0001}
#define BT_TX_PWR_WIFI_OFF       0x7
#define BT_TX_PWR_SCO            0x4
#define BT_TX_PWR_ACL            0x3
#else
#define WIFI_TX_FLOW_CTRL        {0x1100, 0x0001}
#define BT_TX_PWR_WIFI_OFF       0x7
#define BT_TX_PWR_SCO            0x4
#define BT_TX_PWR_ACL            0x4
#endif
#define RESERVED                 {0x00, 0x00, 0x00, 0x00, 0x00}
#endif 
