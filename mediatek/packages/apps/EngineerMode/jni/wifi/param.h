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
** $Id: param.h,v 1.2 2008/06/04 08:53:12 MTK01385 Exp $
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
** $Log: param.h,v $
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.2  2008/06/04 08:53:12  MTK01385
** 1. add PNP power states.
**
** Revision 1.1  2008/05/26 14:04:37  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.1  2007/10/16 06:58:51  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/

#ifndef _PARAM_H
#define _PARAM_H

#include "type.h"

namespace android{
	
	typedef struct _IPC_MCR_RW_STRUC {
	    UINT_8              bRead;              /* read: 1, write: 0 */
	    UINT_8              reserved[3];
	    UINT_32             mcrIndex;
	    UINT_32             mcrData;
	} IPC_MCR_RW_STRUC, *PIPC_MCR_RW_STRUC;
	
	typedef struct _IPC_MCR_RW_STRUC_V2 {
	    UINT_32             mcrIndex;
	    UINT_32             mcrData;
	} IPC_MCR_RW_STRUC_V2, *PIPC_MCR_RW_STRUC_V2F;
	
	typedef struct _IPC_BBCR_RW_STRUC {
	    UINT_8              bRead;              /* read: 1, write: 0 */
	    UINT_8              bbcrIndex;
	    UINT_8              bbcrData;
	    UINT_8              reserved;
	} IPC_BBCR_RW_STRUC, *PIPC_BBCR_RW_STRUC;
	
	#define EEPROM_METHOD_RW    1
	#define EEPROM_METHOD_SZ    0
	
	typedef struct _IPC_EEPROM_RW_STRUC {
	    UINT_8              bRead;              /* read: 1, write: 0 */
	    UINT_8              ucEEPROMMethod;
	    UINT_8              eepromIndex;
	    UINT_8              reserved;
	    UINT_16             eepromData;
	} IPC_EEPROM_RW_STRUC, *PIPC_EEPROM_RW_STRUC;
	
	typedef struct _IPC_EEPROM_RW_STRUC_V2 {
	    UINT_8              ucEEPROMMethod;
	    UINT_8              eepromIndex;
	    UINT_8              reserved;
	    UINT_16             eepromData;
	} IPC_EEPROM_RW_STRUC_V2, *PIPC_EEPROM_RW_STRUC_V2;
	
	/* RF Test for rx status */
	typedef struct _TEST_RX_STATUS_STRUC {
	    UINT_32             intRxOk;            /* number of packets that Rx ok from interrupt */
	    UINT_32             intCrcErr;          /* number of packets that CRC error from interrupt */
	    UINT_32             intShort;           /* number of packets that is short preamble from interrupt */
	    UINT_32             intLong;            /* number of packets that is long preamble from interrupt */
	    UINT_32             pauRxPktCount;      /* number of packets that Rx ok from PAU */
	    UINT_32             pauCrcErrCount;     /* number of packets that CRC error from PAU */
	    UINT_32             pauRxFifoFullCount; /* number of packets that is short preamble from PAU */
	    UINT_32             pauCCACount;        /*CCA rising edge count */
	} TEST_RX_STATUS_STRUC, *PTEST_RX_STATUS_STRUC;
	
	#define MAX_BUF_SZ                              2048
	
	typedef struct _TX_PACKET_V0_STRUC {
	    UINT_32             pktLength;                  /* Length of the Tx Packet */
	    UINT_8              pktContent[MAX_BUF_SZ];  /* Content of the Tx Packet */
	    UINT_32             pktStatus;
	    UINT_32             pktCount;
	    UINT_32             pktInterval;                /* Interval between each Tx Packet */
	    UINT_8              txPower;
	    UINT_8              txAnt;
	} TX_PACKET_V0_STRUC, *PTX_PACKET_V0_STRUC;
	
	#define TX_PACKET_FLAG_ENABLE_ALC_TRACK         BIT(0)
	#define TX_PACKET_FLAG_TARGET_ALC_PROVIDE       BIT(1)
	#define TX_PACKET_FLAG_NO_TXGAIN                BIT(2)     /* TX Packet Struc has no gain informatioin. txGain must be set as 0xFF */
	
	typedef struct _TX_PACKET_STRUC {
	    UINT_32             pktLength;				/* Length of the Tx Packet */
	    UINT_8              pktContent[MAX_BUF_SZ]; /* Content of the Tx Packet */
	    UINT_32             pktStatus;
	    UINT_32             pktCount;
	    UINT_32             pktInterval;			/* Interval between each Tx Packet */
	    UINT_8              txGain;                 /* Gain control value */
	    UINT_8              txAnt;					/* No longer used. use OID_802_11_TX_ANTENNA_SELECTED */
	    UINT_8              reserved[2];
	    UINT_32             txFlags;
	    UINT_32             targetAlc;
	} TX_PACKET_STRUC, *PTX_PACKET_STRUC;
	
	typedef struct _TX_STATUS_STRUC {
	    UINT_32           pktSentStatus;
	    UINT_32           pktSentCount;
	    UINT_16           avgAlc;
	    UINT_8            cckGainControl;
	    UINT_8            ofdmGainControl;
	} TX_STATUS_STRUC, *PTX_STATUS_STRUC;
	
	typedef struct _COUNTRY_STRING_ENTRY {
	    UINT_8      countryCode[2];
	    UINT_8      environmentCode[2];
	} COUNTRY_STRING_ENTRY, *PCOUNTRY_STRING_ENTRY;
	
	typedef struct _IPN_SUPPORTED_CHANNEL_FREQUENCY_LIST {
	    UINT_32  numberOfItems;
	    UINT_32  channelFreqInkHz[1];
	} IPN_SUPPORTED_CHANNEL_FREQUENCY_LIST, *PIPN_SUPPORTED_CHANNEL_FREQUENCY_LIST;
	
	/* The structure definitions for OID_IPC_SUPPORTED_DOMAINS */
	typedef struct _IPC_DOMAIN_INFORMATION {
	    UINT_32     length;
	    UINT_8      countryCode[2];
	    UINT_8      flags;
	    UINT_8      reserved;
	} IPC_DOMAIN_INFORMATION, *PIPC_DOMAIN_INFORMATION;
	
	typedef struct _IPC_SUPPORTED_DOMAINS {
	    UINT_32                 numberOfItems;
	    IPC_DOMAIN_INFORMATION  domain[1];
	} IPC_SUPPORTED_DOMAINS, *PIPC_SUPPORTED_DOMAINS;
	
	typedef struct _TEST_LOW_POWER_EVENT{
	    UINT_32             length;
	    UINT_32             triggerTimeOut;     /* In unit of TU */
	    UINT_8              triggered;
	    UINT_8              reserved[3];
	    UINT_32             enabledSysTime;     /* In unit of system time */
	    UINT_32             triggeredSysTime;   /* In unit of system time */
	} TEST_LOW_POWER_EVENT, *PTEST_LOW_POWER_EVENT;
	
	
	typedef struct _PARAM_RFTEST_INFO {
	    UINT_32     Length;       /* Length of structure */
	    UINT_32     nicInfoContentLen;
	    UINT_8      nicInfoContent[1];
	} PARAM_RFTEST_INFO, *PPARAM_RFTEST_INFO;
	
	/*--------------------------------------------------------------*/
	/* Set/Query power saving mode                                  */
	/*--------------------------------------------------------------*/
	typedef enum _PARAM_POWER_MODE
	{
	    Param_PowerModeCAM,
	    Param_PowerModeMAX_PSP,
	    Param_PowerModeFast_PSP,
	    Param_PowerModeMax                      /* Upper bound, not real case */
	} PARAM_POWER_MODE, *PPARAM_POWER_MODE;
	
	typedef enum _IPC_PHY_PREAMBLE_TYPE {
	    phyPreambleTypeLong,
	    phyPreambleTypeShort,
	    phyPreambleTypeAuto
	} IPC_PHY_PREAMBLE_TYPE, *PIPC_PHY_PREAMBLE_TYPE;
	
	/*--------------------------------------------------------------*/
	/* Set/Query data rates.                                        */
	/*--------------------------------------------------------------*/
	#define PARAM_MAX_LEN_RATES                     8
	#define PARAM_MAX_LEN_RATES_EX                  16
	
	typedef UINT_8              PARAM_RATES[PARAM_MAX_LEN_RATES];
	typedef UINT_8              PARAM_RATES_EX[PARAM_MAX_LEN_RATES_EX];
	
	/* Debug level and Bebug Module setting */
	typedef struct _DEBUG_LEVELMODULES_STRUC {
	    UINT_32             dwLevel;
	    UINT_32             dwModules;
	} DEBUG_LEVELMODULES_STRUC, *PDEBUG_LEVELMODULES_STRUC;
	
	typedef struct _PARAM_NIC_INFO {
	    UINT_32     Length;       /* Length of structure */
	    UINT_32     nicInfoContentLen;
	    UINT_8      nicInfoContent[1];
	} PARAM_NIC_INFO, *PPARAM_NIC_INFO;
	
	/*--------------------------------------------------------------*/
	/* Set/Query basic 802.11 configuration.                        */
	/*--------------------------------------------------------------*/
	typedef struct _PARAM_802_11_CONFIG_FH
	{
	    UINT_32             Length;             /* Length of structure */
	    UINT_32             HopPattern;         /* Defined as 802.11 */
	    UINT_32             HopSet;             /* to one if non-802.11 */
	    UINT_32             DwellTime;          /* In unit of Kusec */
	} PARAM_802_11_CONFIG_FH, *PPARAM_802_11_CONFIG_FH;
	
	typedef struct _PARAM_802_11_CONFIG
	{
	    UINT_32             Length;             /* Length of structure */
	    UINT_32             BeaconPeriod;       /* In unit of Kusec */
	    UINT_32             ATIMWindow;         /* In unit of Kusec */
	    UINT_32             DSConfig;           /* Channel frequency in unit of kHz */
	    PARAM_802_11_CONFIG_FH    FHConfig;
	} PARAM_802_11_CONFIG, *PPARAM_802_11_CONFIG;
	
	/*--------------------------------------------------------------*/
	/* Set/Query MAC Receive Data Structure (RDS) measurement       */
	/*--------------------------------------------------------------*/
	typedef struct _IPC_RDS_MEASURE_STRUC {
	    UINT_8  ucPara0;
	    UINT_8  ucPara1;
	    UINT_8  ucPara2;
	    UINT_8  ucPara3;
	    UINT_32 u4Para4;
	    UINT_32 u4Para5;
	    UINT_32 u4Para6;
	    UINT_32 u4Para7;
	} IPC_RDS_MEASURE_STRUC, *PIPC_RDS_MEASURE_STRUC;
	
	//1012 fifi
	/* Set/Get TX info */
	typedef struct _MTK_WIFI_TEST_STRUC {
	    UINT_8   bRead;
	    UINT_8   reserved[3];
	    UINT_32  funcIndex;
	    UINT_32  funcData;
	} MTK_WIFI_TEST_STRUC, *PMTK_WIFI_TEST_STRUC;
	
	typedef struct _MTK_WIFI_TEST_STRUC_V2 {
	    UINT_32  funcIndex;
	    UINT_32  funcData;
	} MTK_WIFI_TEST_STRUC_V2, *PMTK_WIFI_TEST_STRUC_V2;
	/*For OID_PNP_SET_POWER*/
	typedef enum _PARAM_DEVICE_POWER_STATE
	{
	    ParamDeviceStateUnspecified = 0,
	    ParamDeviceStateD0,
	    ParamDeviceStateD1,
	    ParamDeviceStateD2,
	    ParamDeviceStateD3,
	    ParamDeviceStateMaximum
	} PARAM_DEVICE_POWER_STATE, *PPARAM_DEVICE_POWER_STATE;
	
	#define MCR_HISR                0x0004
	#define MCR_HIER                0x000C
	#define MCR_HFCR                0x0040
	#define MCR_HFDR                0x0044
	#define MCR_ASR                  0x004C
	#define MCR_ICCR0               0x01F0
	#define MCR_ICCR1               0x01F4
	#define MCR_ICCR2               0x01F8
	#define MCR_ICCR3               0x01FC
	
	typedef struct _NDIS_TRANSPORT_STRUCT{
		UINT_32	ndisOidCmd;
		UINT_32	inNdisOidlength;
		UINT_32	outNdisOidLength;
		UINT_8	ndisOidContent[64];
	}NDIS_TRANSPORT_STRUCT, *P_NDIS_TRANSPORT_STRUCT;
	
	//moved from mt5921.cpp and mt6620.cpp
	typedef struct _TX_RATE_INFO {
	    BOOL	cck;
	    BOOL	b24G;   /* TRUE if 2.4G FALSE if 5G */
	    int     rate;   /* in unit of 500K */
	    int     index;  /* index for each modulation */
	    int     offset;
	} TX_RATE_INFO;

	typedef struct _TX_MAX_POWER_OFFSET {
	    int offset;
	    int nChannel[2];
	} TX_MAX_POWER_OFFSET;
	
	typedef struct _DATA_RATE_ENTRY_STRUCT_T {
	    char * pcRateString;
	    INT_32 i4BBUiCfg; /*Script config value 2(1M), 4(2M), 11(5.5M), 22 (11M)...*/
	    INT_32 i4BBHwVal; /*HW config value 0(1M), 1(2M), 2(5.5M), 3 (11M)...*/
	    UINT_8 ucRateGruopEep;
	    BOOL fgIsCCK;  /*CCK: TRUE, OFDM: FALSE*/
	}DATA_RATE_ENTRY_STRUCT_T, *P_DATA_RATE_ENTRY_STRUCT_T;

	typedef struct _SW_CMD_TRANSPORT_STRUCT {
		UINT_32 u4CmdId;
		UINT_32 u4CmdValue;
	}SW_CMD_TRANSPORT_STRUCT, *P_SW_CMD_TRANSPORT_STRUCT;
	/*******************************************************************************
	*                          C O N S T A N T S
	********************************************************************************
	*/
	#define MCR_ECSR                        0x0054
	#define ESCR_EEPROM_NOT_PRESENT         0
	#define ESCR_EEPROM_SIZE_MASK           BITS(17, 19)
	#define ESCR_EEPROM_SIZE_128_BYTES      BIT(17)
	#define ESCR_EEPROM_SIZE_256_512_BYTES  BITS(17,18)
	#define ESCR_EEPROM_SIZE_1024_BYTES     BIT(19)
	#define ESCR_EEPROM_SIZE_2048_BYTES     (BIT(17)|BIT(19))
	#define ESCR_EEPROM_MAX_SIZE_DW         2048   
	
	#define MCR_CCR                         0x0060
	#define MCR_ACDR1                       0x0078
	#define MCR_ACDR2                       0x007C
	#define MCR_ACDR4                       0x0084    
	
	
	/* Clock Control Register*/
	#define CCR_BB_CLOCK_OFF                0
	#define CCR_BB_CLOCK_RESERVED           BIT(30)
	#define CCR_BB_CLOCK_CTRL_BY_TR_PE      BIT(31)
	#define CCR_BB_CLOCK_ON                 BITS(30, 31)
	#define CCR_BB_CLOCK_MASK               BITS(30, 31)
	
	/* AFE Configuration Data 1 Register*/
	#define ACDR1_RG_DACCLK_SEL_MASK        BITS(0,1)
	#define ACDR1_RG_DACCLK_SEL_20M         0
	#define ACDR1_RG_DACCLK_SEL_40M         1
	#define ACDR1_RG_DACCLK_SEL_80M         2
	#define ACDR1_RG_DACCLK_SEL_XTAL_CLK    3
	
	/* AFE Configuration Data 4 Register*/
	#define ACDR4_DAC_CLK_MODE              BIT(31)
	
	/* HIF Control Register */
	#define MCR_HCR                         0x0014
	
	/* HIF Low Power Control Register */
	#define MCR_HLPCR                       0x003C
	
	/* 32K Clock Calibration Register Register */
	#define MCR_32KCCR                      0x005C
	
	/* system control */
	#define MCR_SCR                         0x0090
	#define MCR_LCR                         0x0094
	#define MCR_IOUDR                       0x0068
	#define MCR_IOPCR                       0x006C
	#define MCR_RICR                        0x0098
	// Bluetooth Co-existence Regiester 0-3
	#define MCR_BTCER0                      0x011C
	#define MCR_BTCER1                      0x0120
	#define MCR_BTCER2                      0x0124
	#define MCR_BTCER3                      0x0128
	
	//Receive Filter Control Register
	#define MCR_RFCR                        0x0150
	
	// ALC Control Register 0
	#define MCR_ALCR0                       0x0178
	
	//RFCR 
	#define RFCR_CR4                        0x0410
	
	/* HIF Control Register (HCR 0x0014) bit definitions */
	#define HCR_INT_POLARITY                BIT(18)
	
	/* HIF Low Power Control Register (HLPCR 0x003C) bit definitions */
	#define HLPCR_CLR_GLOBAL_INT_EN         BIT(5)
	#define HLPCR_SET_GLOBAL_INT_EN         BIT(4)
	
	/* System Control Register (SCR 0x0090) bit definitions */
	#define SCR_GPIO1_POLAR_HIGH            BIT(29)
	#define SCR_GPIO1_POLAR_LOW             0
	#define SCR_GPIO1_CHAIN_SEL             BIT(28)
	#define SCR_BTFREQ_SEL                  BIT(27)
	#define SCR_GPIO1_WDATA                 BIT(26)
	#define SCR_GPIO1_ENABLE_OUTPUT_MODE    BIT(25)
	#define SCR_GPIO1_ENABLE_INPUT_MODE     0
	#define SCR_GPIO1_RDATA                 BIT(24)
	
	#define SCR_GPIO0_POLAR_HIGH            BIT(21)
	#define SCR_GPIO0_POLAR_LOW             0
	#define SCR_GPIO0_CHAIN_SEL             BIT(20)
	#define SCR_BT_ACT_SEL                  BIT(19)
	#define SCR_GPIO0_WDATA                 BIT(18)
	#define SCR_GPIO0_ENABLE_OUTPUT_MODE    BIT(17)
	#define SCR_GPIO0_ENABLE_INPUT_MODE     0
	#define SCR_GPIO0_RDATA                 BIT(16)
	
	#define SCR_GPIO2_POLAR_HIGH            BIT(13)
	#define SCR_GPIO2_POLAR_LOW             0
	#define SCR_GPIO2_CHAIN_SEL             BIT(12)
	#define SCR_GPIO2_WDATA                 BIT(10)
	#define SCR_GPIO2_ENABLE_OUTPUT_MODE    BIT(9)
	#define SCR_GPIO2_ENABLE_INPUT_MODE     0
	#define SCR_GPIO2_RDATA                 BIT(8)
	
	#define SCR_DB_EN_GIO                   BIT(7)
	#define SCR_DB_EN_EINT                  BIT(6)
	
	#define SCR_EINT_POLAR_HIGH             BIT(5)
	#define SCR_EINT_POLAR_LOW              0
	
	#define SCR_ACK                         BIT(4)
	
	#define SCR_SENS_EDGE                   0
	#define SCR_SENS_LEVEL                  BIT(3)
	
	#define SCR_MIB_CTR_RD_CLEAR            BIT(2)
	#define SCR_OP_MODE_STA                 0
	#define SCR_OP_MODE_ADHOC               BIT(0)
	#define SCR_OP_MODE_MASK                BITS(0,1)
	
	/* LED Configuration Register (LCR 0x0094) bit definitions */
	#define LCR_LED_OUTPUT                  BIT(25)
	#define LCR_LED_POLARITY                BIT(24)
	#define LCR_LED_MODE_RX_RFCR_BEACON     BIT(20)
	#define LCR_LED_MODE_RX_EX_RFCR_BEACON  BIT(19)
	#define LCR_LED_MODE_RX                 BIT(18)
	#define LCR_LED_MODE_TX_BEACON          BIT(17)
	#define LCR_LED_MODE_TX_WO_BEACON       BIT(16)
	#define LCR_LED_MODE_MASK               BITS(16, 21)
	
	/* 32K Clock Calibration Register Register (32KCCR 0x005C) bit definitions */
	#define MCR_32KCCR_SLOW_CNT                 BITS(0, 5)
	
	/* I/O Pull Up/Down Register (IOUDR 0x0068) bit definitions*/
	#define IOUDR_BT_PRI_PU                 BIT(19)
	#define IOUDR_WE_N_PU                   BIT(18)
	#define IOUDR_OE_N_PU                   BIT(17)
	#define IOUDR_BT_PRI_PD                 BIT(3)
	#define IOUDR_WE_N_PD                   BIT(2)
	#define IOUDR_OE_N_PD                   BIT(1)
	
	/* I/O Pin Control Register (IOPCR 0x006C) bit definitions*/
	#define IOPCR_LOOPBACK_TEST_EN          BIT(31)
	#define IOPCR_LOOPBACK_BT_PRI_RDATA     BIT(30)
	#define IOPCR_LOOPBACK_D15_RDATA        BIT(29)
	#define IOPCR_LOOPBACK_D14_RDATA        BIT(28)
	#define IOPCR_LOOPBACK_D13_RDATA        BIT(27)
	#define IOPCR_LOOPBACK_D12_RDATA        BIT(26)
	#define IOPCR_LOOPBACK_D7_RDATA         BIT(25)
	#define IOPCR_LOOPBACK_D6_RDATA         BIT(24)
	#define IOPCR_LOOPBACK_D11_WDATA        BIT(23)
	#define IOPCR_LOOPBACK_D10_WDATA        BIT(22)
	#define IOPCR_LOOPBACK_D9_WDATA         BIT(21)
	#define IOPCR_LOOPBACK_D8_WDATA         BIT(20)
	#define IOPCR_LOOPBACK_D5_WDATA         BIT(19)
	#define IOPCR_LOOPBACK_D4_WDATA         BIT(18)
	#define IOPCR_LOOPBACK_OE_N_WDATA       BIT(17)
	#define IOPCR_LOOPBACK_WE_N_WDATA       BIT(16)
	
	#define IOPCR_LOOPBACK_RDATA_MASK       BITS(24, 30)
	#define IOPCR_LOOPBACK_WDATA_MASK       BITS(16, 23)
	
	#define PTA_HW_DISABLE                  (0)
	#define PTA_HW_ENABLE                   (1)
	/*BTCER0*/
	#define PTA_BTCER0_COEXIST_EN           BIT(0)
	#define PTA_BTCER0_WLAN_ACT_POL         BIT(1)
	#define PTA_BTCER0_BURST_MODE           BIT(2)
	#define PTA_BTCER0_WLAN_ACK             BIT(3)
	#define PTA_BTCER0_TX_MODE              BITS(4,5)
	#define PTA_BTCER0_RX_MODE              BIT(6)
	#define PTA_BTCER0_WLAN_AC0             BIT(8)
	#define PTA_BTCER0_WLAN_AC1             BIT(9)
	#define PTA_BTCER0_WLAN_AC2             BIT(10)
	#define PTA_BTCER0_WLAN_AC3             BIT(11)
	#define PTA_BTCER0_WLAN_BCN             BIT(12)
	#define PTA_BTCER0_WLAN_AC4             BIT(13)
	#define PTA_BTCER0_WLAN_RX              BIT(14)
	#define PTA_BTCER0_WLAN_CTRL            BIT(15)
	#define PTA_BTCER0_BCN_TIMEOUT_EN       BIT(16)
	#define PTA_BTCER0_QCFP_TIMEOUT_EN      BIT(17)
	#define PTA_BTCER0_SP_TIMEOUT_EN        BIT(18)
	#define PTA_BTCER0_REMAIN_TIME          BITS(24,31)
	/*BTCER1*/
	#define PTA_BTCER1_BT_2ND_SAMPLE_TIME   BITS(0,4)
	#define PTA_BTCER1_BT_1ST_SAMPLE_TIME   BITS(5,7)
	#define PTA_BTCER1_1ST_SAMPLE_MODE      BITS(8,9)
	#define PTA_BTCER1_2ND_SAMPLE_MODE      BITS(10,11)
	#define PTA_BTCER1_BT_PRI_MODE          BIT(12)
	#define PTA_BTCER1_BT_TR_MODE           BIT(13)
	#define PTA_BTCER1_CONCATE_MODE         BIT(14)
	#define PTA_BTCER1_T6_PERIOD            BITS(16,20)
	#define PTA_BTCER1_SINGLE_ANT           BIT(28)
	#define PTA_BTCER1_WIRE_MODE            BITS(29,30)
	
	#define PTA_BTCER1_WIRE_MODE_1WIRE      (0)
	#define PTA_BTCER1_WIRE_MODE_2WIRE      BIT(30)
	#define PTA_BTCER1_WIRE_MODE_3_41WIRE   BIT(29,30)
	
	
	
	/* RF Interface Control Register (0x0098)*/
	// transmission control by MT6620
	#define RICR_MAC_TX_EN                   BIT(10)
	// receiving control by MT6620
	#define RICR_MAC_RX_EN                   BIT(9)
	#define RICR_RF_SW_MODE                  BIT(8)
	
	#define BBCR_CR97_DEFAULT_VALUE          0x00000000
	
	/* Receive Filter Control Register (0x0150)*/
	#define RFCR_RX_NOACK_CTRL              BIT(18)
	
	
	// ALCR
	#define ALCR_ALC_CALCULATION_EN     BIT(31)
	#define ALCR_ALC_TRIGGER            BIT(30)
	#define ALCR_ALC_BUSY               BIT(29)
	#define ALCR_ALC_AR_FACTOR_MASK     BITS(24, 25)
	#define ALCR_AR_PARM_1_OF_32        0
	#define ALCR_AR_PARM_1_OF_16        BIT(24)
	#define ALCR_AR_PARM_1_OF_4         BIT(25)
	#define ALCR_AR_PARM_1_OF_1         BITS(24,25)
	
	#define ALCR_ALC_MIN_THRESHOLD      BITS(18,23)
	#define ALCR_ALC_MAX_THRESHOLD      BITS(12,17)
	#define ALCR_ALC_CAL_VALUE_MASK     BITS(6,11)
	#define ALCR_ALC_RAW_VALUE_MASK     BITS(0,5)
	
	
	/* RF Control Register 4*/
	#define RFCR4_XO_TRIM_MASK               BITS(21, 27)
	#define RFCR4_XO_TRIM_OFFSET             21
	
	/*EEPROM section*/
	
	/* EEPROM data locations (word offset) */
	#define EEPROM_SIGNATURE                        0x00 /* EEPROM signature */
	#define EEPROM_SIGNATURE_MT6620                 0x6620
	
	
	/* Host interface dependent  */
	/* SDIO */
	#define EEPROM_SDIO_CCCR                        0x01 /* SDIO[3:0], CCCR[3:0], 4'h0, SD[3:0] */
	#define EEPROM_SDIO_SRW                         0x02 /* SRW, S4MI, SCSI, SDC, SMB, LSC, 4BLS, SMPC, SHS 7'h0 */
	#define EEPROM_SDIO_IO_CODE                     0x03 /* 4'h0, IO code[3:0], 8'h0 */
	#define EEPROM_SDIO_SPS                         0x04 /* 7'h0, SPS, 8'h0 */
	#define EEPROM_SDIO_OCR_LOWER                   0x05 /* OCR[7:0], OCR[15:8] */
	#define EEPROM_SDIO_OCR_HIGHER                  0x06 /* OCR[23:16], 8'h0 */
	
	
	#define EEPROM_DEBUG                            0x07 /*2'h0, debug_en, dbg_port2_sel, dbg_port1_sel, dbg_port0_sel, dbg_byte_sel[1:0], hif_debug_sel[7:0]*/
	#define EEPROM_SDRAM_CFG                        0x08 /*2'h0, RC4_SRAM_DLY, TR_SRAM_DLY, WT_SRAM_DLY, PTN_SRAM_DLY, MAC_SRAM_DLY, BBP_SRAM_DLY*/
	/*MP chip*/
	#define EEPROM_SDIO_HIF_CTL_MP                  0x09 /*7'h0,sdio_test_option,4'h0, hif_ctrl_odc, hif_data_odc*/
	#define EEPROM_CIS_START_MP                     0x0A /* CIS0 start(b15~b8), reserved(b7~b0) */
	#define EEPROM_CIS_LEN_MP                       0x0B /* [15:8]: CIS 1 length, [7:0]: CIS 0 length */
	
	/*MPW chip*/
	#define EEPROM_CIS_START_MPW                    0x09 /* CIS0 start(b15~b8), reserved(b7~b0) */
	#define EEPROM_CIS_LEN_MPW                      0x0A /* [15:8]: CIS 1 length, [7:0]: CIS 0 length */
	
	
	#define EEPROM_INTERFACE_CFG_CHKSUM             0x0C /* checksum(b15~b8), reserved(b7~b0) */
	#define EEPROM_CHECKSUM_MASK                    BITS(8,15)
	
	#define EEPROM_REG_DOMAIN                       0x0F /* regulatory domain */
	#define EEPROM_OSC_STABLE_TIME                  0x10 /* OSC Stable Time in us */
	#define EEPROM_LED_MODE                         0x11 /* LED configuration */
	#define EEPROM_XTAL_FREQ_TRIM_BYTE_OFFSET            0x23
	/* EEPROM LED configuration & PHY mode (word offset: 0x0B) */
	#define EEPROM_LED_MODE_MASK                    BITS(0,3)
	#define EEPROM_LED_MODE_DISABLE                 0x0000
	#define EEPROM_LED_MODE_1                       0x0001
	#define EEPROM_LED_MODE_2                       0x0002
	
	#define EEPROM_CLK_CFG_VERSION                  0x12 /* Slow clock Config and EEPROM Version*/
	#define EEPROM_LAYOUT_VERSION_MASK              BITS(8,15)
	#define EEPROM_LAYOUT_VERSION_OFFSET            8
	
	#define EEPROM_MAC_ADDR_BYTE_1_0                0x13 /* MAC address [15:0] */
	#define EEPROM_MAC_ADDR_BYTE_3_2                0x14 /* MAC address [31:16] */
	#define EEPROM_MAC_ADDR_BYTE_5_4                0x15 /* MAC address [47:32] */
	#define EEPROM_THERMO_USAGE_BAND_SEL            0x16 /* Thermo-Sensor Usage & Band Selection*/
	#define EEPROM_BAND_MODE_MASK                   BITS(0,1)
	#define EEPROM_PHY_MODE_G                       0x0000
	#define EEPROM_PHY_MODE_A_G                     0x0001
	#define EEPROM_PHY_MODE_A                       0x0002
	#define EEPROM_PHY_MODE_RESERVED                0x0003
	
	
	#define EEPROM_THERMO_VGA_SLOPE                 0x17 /* Thermo-Sensor Slope & VGA Gain Slope*/
	
	#define EEPROM_VGA_SLOPE_MASK                   BITS(0,7)
	#define EEPROM_VGA_SLOPE_OFFSET                 0
	#define EEPROM_THERMO_SLOPE_MASK                BITS(8,15)
	#define EEPROM_THERMO_SLOPE_OFFSET              8
	
	/* CCK TX Power Gain Table for 2.4G band */
	#define EEPROM_2G_CCK_TXPWR_GAIN_START          0x18 /*Tuple {TX power gain, EIRP, thermo-value} CCK in each 2.4G channel.*/
	#define EEPROM_2G_CCK_TXPWR_GAIN_END            0x2C
	
	/* OFDM TX Power Gain Table for 2.4G band */
	#define EEPROM_2G_OFDM0_TXPWR_GAIN_START        0x2D  /*Tuple {TX power gain, EIRP, thermo-value} OFDM 6_9M in each 2.4G channel.*/
	#define EEPROM_2G_OFDM0_TXPWR_GAIN_END          0x41
	#define EEPROM_2G_OFDM1_TXPWR_GAIN_START        0x42  /*Tuple {TX power gain, EIRP, thermo-value} OFDM 12_18M in each 2.4G channel.*/
	#define EEPROM_2G_OFDM1_TXPWR_GAIN_END          0x56
	#define EEPROM_2G_OFDM2_TXPWR_GAIN_START        0x57  /*Tuple {TX power gain, EIRP, thermo-value} OFDM 24_36M in each 2.4G channel.*/
	#define EEPROM_2G_OFDM2_TXPWR_GAIN_END          0x6B
	#define EEPROM_2G_OFDM3_TXPWR_GAIN_START        0x6C  /*Tuple {TX power gain, EIRP, thermo-value} OFDM 48_54M in each 2.4G channel.*/
	#define EEPROM_2G_OFDM3_TXPWR_GAIN_END          0x80
	
	/*RX LNA Control Threshold*/
	#define EEPROM_LNA_CONTROL_THRESHOLD_START      0x81  /*Tuple {LNA_Turbo temperature difference, LNA_Restore temperature difference, Thermo-value}.*/
	#define EEPROM_LNA_CONTROL_THRESHOLD_END        0x82
	
	/*2.4G RCPI Offset Table*/
	#define EEPROM_2G_RCPI_OFFSET_TABLE_START       0x83  /*Compensate for the RCPI value with -128 ~ +127 about RF variant in each 2.4G channel.*/
	#define EEPROM_2G_RCPI_OFFSET_TABLE_END         0x89
	
	/* EEPROM offset for NIC setting section checksum */
	#define EEPROM_NIC_CHKSUM                       0x9F
	#define EEPROM_NIC_CHKSUM_MASK                  BITS(8,15)
	#define EEPROM_NIC_CHKSUM_START                 0x0D
	
	/* EEPROM offset for HIF section checksum */
	#define EEPROM_HIF_CHKSUM_MP                    0x0C
	#define EEPROM_HIF_CHKSUM_MASK_MP               BITS(8,15)
	#define EEPROM_HIF_CHKSUM_START_MP              0x01
	#define EEPROM_HIF_CHKSUM_MPW                   0x0B
	#define EEPROM_HIF_CHKSUM_MASK_MPW              BITS(8,15)
	#define EEPROM_HIF_CHKSUM_START_MPW             0x01
	
	
	
	#define EEPROM_RATE_GROUP_CCK           0x0
	#define EEPROM_RATE_GROUP_OFDM_6_9M     0x1
	#define EEPROM_RATE_GROUP_OFDM_12_18M   0x2
	#define EEPROM_RATE_GROUP_OFDM_24_36M   0x3
	#define EEPROM_RATE_GROUP_OFDM_48_54M   0x4
	#define EEPROM_RATE_GROUP_OFDM_MCS   	0x4	//added by mtk80758, not compatiable, not compatiable, need to confirm with Cp.Wu

	#define NUM_SUPPORTED_RATES (sizeof(arDataRateTable)/sizeof(DATA_RATE_ENTRY_STRUCT_T))
	#define DATE_RATE_WITH_SHORT_PREAMBLE (200)
	
	/************************************************************************
	*                          M A C R O S
	*************************************************************************/
	#define BBCR_WR_32(_cr, _u4Value) \
	    {\
	        writeMCR32(((_cr)*4+0x200), _u4Value);\
	    }
	
	#define BBCR_RD_32(_cr, _pu4Value) \
	    {\
	        readMCR32(((_cr)*4+200), _pu4Value);\
	    }
	

}

#endif
