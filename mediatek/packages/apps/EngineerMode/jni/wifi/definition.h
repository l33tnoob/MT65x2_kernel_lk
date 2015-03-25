/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
** $Id: definition.h,v 1.4 2008/11/11 03:11:18 MTK01725 Exp $
*/

/*******************************************************************************
** Copyright (c) 2005 - 2007 MediaTek Inc.
**
** All rights reserved. Copying, compilation, modification, distribution
** or any other use whatsoever of this material is strictly prohibited
** except in accordance with a Software License Agreement with
** MediaTek Inc.
********************************************************************************
*/

/*
** $Log: definition.h,v $
 *
 * 04 29 2011 xiao.liu
 * [ALPS00044734] [Need Patch] [Volunteer Patch][EM] resolve all build warning. alps.GB
 * warning. alps
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.4  2008/11/11 03:11:18  MTK01725
** Add a 802_11_SUPPORT_RATE OID definition.
**
** Revision 1.3  2008/06/12 02:32:45  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:37:49  MTK01385
** 1. add setPnpPower().
**
** Revision 1.1  2008/05/26 14:04:36  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.1  2007/10/16 06:58:50  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/


#ifndef __DEFINITION_H__
#define __DEFINITION_H__

namespace android{

#define MT5911							0x5911
#define MT5912							0x5912
#define MT5905							0x5905
#define MT5921							0x5921
#ifdef MT6620
#undef MT6620
#define MT6620							0x6620
#endif

#define DEVID_IDMSK                     BITS(0,15)
#define DEVID_REVMSK                    BITS(16,18)

/* ERROR CODE for test function return values */
#define ERROR_RFTEST_SUCCESS                 0
#define ERROR_RFTEST_GENERAL_ERROR          -1
#define ERROR_RFTEST_NIC_INDEX_OUT_RANGE    -2
#define ERROR_RFTEST_NIC_UNINITIALIZE       -3
#define ERROR_RFTEST_NIC_UNKNOWN_IC         -4
#define ERROR_RFTEST_NDIS_OID_FAILURE       -5
#define ERROR_RFTEST_UNSUPPORTED            -6
#define ERROR_RFTEST_EEPROM_NOT_PRESENT       -7

#define OID_802_11_SUPPORTED_RATES                      0x0D01020E
//
//  PnP and PM OIDs, NDIS default OIDS
//

#define OID_PNP_SET_POWER                               0xFD010101



/* MediaTek custom-defined OIDs */
#define OID_IPC_OID_INTERFACE_VERSION                   0xFFA0C000
#define OID_CUSTOM_802_11_SUPPORTED_RATES               0xFFA0C00E
#define OID_IPC_PHY_PREAMBLE_TYPE                       0xFFA0C502
#define OID_IPC_LINK_QUALITY                            0xFFA0C503
#define OID_IPC_XMIT_BYTES                              0xFFA0C508
#define OID_IPC_RCV_BYTES                               0xFFA0C509
//#define OID_IPC_HW_RADIO_ON_OFF_STATE                   0xFFA0C50C
#define OID_IPC_SW_RADIO_ON_OFF_STATE                   0xFFA0C516
#define OID_IPC_CURRENT_COUNTRY                         0xFFA0C58A
#define OID_IPC_SUPPORTED_DOMAINS                       0xFFA0C521
#define OID_IPN_MULTI_DOMAIN_CAPABILITY                 0xFFA0C522
#define OID_IPN_CCX_CONFIGURATION                       0xFFA0C530



/* Power Management OID */
#define OID_MTK_PM_TRIGGER_EVENT                        0xFFA0C540

/* QoS Related OID */
#define OID_IPN_QOS_ADD_TS                              0xFFA0C550
#define OID_IPN_QOS_DELETE_TS                           0xFFA0C551
#define OID_CUSTOM_QOS_UAPSD                            0xFFA0C552

/* MT5921 specific OIDs */
#define OID_CUSTOM_ROAMING_EN                           0xFFA0C588

/* Precedent OIDs */
#define OID_IPC_MCR_RW                                  0xFFA0C801
#define OID_IPC_BBCR_RW                                 0xFFA0C802
#define OID_IPC_EEPROM_RW                               0xFFA0C803
#define OID_IPC_WEP_STATISTICS                          0xFFA0C805	//OID_CUSTOM_EFUSE_RW in MT6620
#define OID_IPC_RESET                                   0xFFA0C806
#define OID_IPC_HW_STATISTICS                           0xFFA0C80D
#define OID_IPC_DEBUG_LEVEL_MODULE                      0xFFA0C80E
#define OID_IPC_ACCESSING_USB_PIPE                      0xFFA0C80F
#define OID_CUSTOM_NIC_INFO                             0xFFA0C810
//#define OID_IPC_RDS_MEASURE                             0xFFA0C811


#define OID_IPC_TEST_MODE                               0xFFA0C901
#define OID_IPC_TEST_PACKET_RX                          0xFFA0C902
#define OID_IPC_TEST_RX_STATUS                          0xFFA0C903
#define OID_IPC_TEST_PACKET_TX                          0xFFA0C904
#define OID_IPC_TEST_TX_STATUS                          0xFFA0C905
#define OID_IPC_ABORT_TEST_MODE                         0xFFA0C906
#define OID_IPN_TEST_CHANNEL_FREQUENCY_LIST             0xFFA0C90D
#define OID_CUSTOM_TEST_LOW_POWER_CAP                   0xFFA0C910
//1012 fifi
#define OID_CUSTOM_MTK_WIFI_TEST                        0xFFA0C911

//NVRAM/EEPROM configuration source query OID
#define OID_CUSTOM_CFG_SRC_TYPE                        0xFFA0C942
typedef enum _ENUM_CFG_SRC_TYPE_T{
		CFG_SRC_TYPE_EEPROM,
		CFG_SRC_TYPE_NVRAM,
		CFG_SRC_TYPE_UNKNOWN,
		CFG_SRC_TYPE_ENUM
}ENUM_CFG_SRC_TYPE_T, *P_ENUM_CFG_SRC_TYPE_T;

#define OID_CUSTOM_CFG_EEPROM_TYPE                        0xFFA0C943
typedef enum _ENUM_CFG_EEPROM_TYPE_T{
		CFG_EEPROM_TYPE_NO,
		CFG_EEPROM_TYPE_YES,
		CFG_EEPROM_TYPE_ENUM
}ENUM_CFG_EEPROM_TYPE_T, *P_ENUM_CFG_EEPROM_TYPE_T;
//NVRAM set/query OID
#define OID_IPC_NVRAM_RW                        0xFFA0C941

/*
New wireless extensions API - SET/GET convention(even ioctl numbers
are root only)
*/
#define	IOCTL_SET_INT			(SIOCIWFIRSTPRIV + 0)
#define	IOCTL_GET_INT			(SIOCIWFIRSTPRIV + 1)
#define	IOCTL_SET_ADDRESS		(SIOCIWFIRSTPRIV + 2)
#define	IOCTL_GET_ADDRESS		(SIOCIWFIRSTPRIV + 3)
#define	IOCTL_SET_STR			(SIOCIWFIRSTPRIV + 4)
#define	IOCTL_GET_STR			(SIOCIWFIRSTPRIV + 5)
#define	IOCTL_SET_KEY			(SIOCIWFIRSTPRIV + 6)
#define	IOCTL_GET_KEY			(SIOCIWFIRSTPRIV + 7)
#define	IOCTL_SET_STRUCT		(SIOCIWFIRSTPRIV + 8)
#define	IOCTL_GET_STRUCT		(SIOCIWFIRSTPRIV + 9)
#define	IOCTL_SET_STRUCT_FOR_EM		(SIOCIWFIRSTPRIV + 11)



#define PRIV_CMD_OID	15
#define PRIV_CMD_GET_CH_LIST	24
#define PRIV_CMD_SET_GET_VALUE	20


/**/
/* IEEE 802.11 OIDs*/
/**/
#define OID_802_11_BSSID                        0x0D010101
#define OID_802_11_SSID                         0x0D010102
#define OID_802_11_INFRASTRUCTURE_MODE          0x0D010108
#define OID_802_11_ADD_WEP                      0x0D010113
#define OID_802_11_REMOVE_WEP                   0x0D010114
#define OID_802_11_DISASSOCIATE                 0x0D010115
#define OID_802_11_AUTHENTICATION_MODE          0x0D010118
#define OID_802_11_PRIVACY_FILTER               0x0D010119
#define OID_802_11_BSSID_LIST_SCAN              0x0D01011A
#define OID_802_11_WEP_STATUS                   0x0D01011B
#define OID_802_11_RELOAD_DEFAULTS              0x0D01011C
#define OID_802_11_ADD_KEY                      0x0D01011D
#define OID_802_11_REMOVE_KEY                   0x0D01011E
#define OID_802_11_ASSOCIATION_INFORMATION      0x0D01011F
#define OID_802_11_NETWORK_TYPES_SUPPORTED      0x0D010203
#define OID_802_11_NETWORK_TYPE_IN_USE          0x0D010204
#define OID_802_11_TX_POWER_LEVEL               0x0D010205
#define OID_802_11_RSSI                         0x0D010206
#define OID_802_11_RSSI_TRIGGER                 0x0D010207
#define OID_802_11_FRAGMENTATION_THRESHOLD      0x0D010209
#define OID_802_11_RTS_THRESHOLD                0x0D01020A
#define OID_802_11_NUMBER_OF_ANTENNAS           0x0D01020B
#define OID_802_11_RX_ANTENNA_SELECTED          0x0D01020C
#define OID_802_11_TX_ANTENNA_SELECTED          0x0D01020D
#define OID_802_11_SUPPORTED_RATES              0x0D01020E
#define OID_802_11_DESIRED_RATES                0x0D010210
#define OID_802_11_CONFIGURATION                0x0D010211
#define OID_802_11_STATISTICS                   0x0D020212
#define OID_802_11_POWER_MODE                   0x0D010216
#define OID_802_11_BSSID_LIST                   0x0D010217

#define OID_802_3_CURRENT_ADDRESS   			0x01010102


}

//#define MAX_NIC_NUM 5
#endif //__DEFINITION_H__
