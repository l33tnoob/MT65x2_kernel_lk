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
** $Id: CWrapper.h,v 1.0 2010/05/07 14:49:33 MTK80743 $
*/

/*******************************************************************************
* Copyright (c) 2007 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

/*
** $Log: WiFi_EM_API.h,v $
 *
 * 01 10 2012 ben.niu
 * [ALPS00110759] Get chip support channels.
 * Add getSupportChannelList
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
** 1. init version.
**
*/
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#ifndef WIFI_EM_API_H
#define WIFI_EM_API_H

#include "type.h"
#include "param.h"
#include "COid.h"
#include "mt5921.h"
#include "mt6620.h"


namespace android{

	/*******************************************************************************
	*                              C O N S T A N T S
	********************************************************************************
	*/
#define CH_1     0x0
#define CH_2     0x01
#define CH_3     0x02
#define CH_4     0x03
#define CH_5     0x04
#define CH_6     0x05
#define CH_7     0x06
#define CH_8     0x07
#define CH_9     0x08
#define CH_10    0x09
#define CH_11    0x0a
#define CH_12    0x0b
#define CH_13    0x0c
#define CH_14    0x0d
#define CH_36    0x0e
#define CH_40    0x0f
#define CH_44    0x10
#define CH_48    0x11
#define CH_52    0x12
#define CH_56    0x13
#define CH_60    0x14
#define CH_64    0x15
#define CH_100    0x16
#define CH_104    0x17
#define CH_108    0x18
#define CH_112    0x19
#define CH_116    0x1a
#define CH_120    0x1b
#define CH_124    0x1c
#define CH_128    0x1d
#define CH_132    0x1e
#define CH_136    0x1f
#define CH_140    0x20
#define CH_149    0x21
#define CH_153    0x22
#define CH_157    0x23
#define CH_161    0x24
#define CH_165    0x25



#define EEPROM_RATE_GROUP_CCK           0x0
#define EEPROM_RATE_GROUP_OFDM_6_9M     0x1
#define EEPROM_RATE_GROUP_OFDM_12_18M   0x2
#define EEPROM_RATE_GROUP_OFDM_24_36M   0x3
#define EEPROM_RATE_GROUP_OFDM_48_54M   0x4
#define EEPROM_RATE_GROUP_20M_MCS0   	0x5
#define EEPROM_RATE_GROUP_20M_MCS1_2   	0x6
#define EEPROM_RATE_GROUP_20M_MCS3_4   	0x7
#define EEPROM_RATE_GROUP_20M_MCS5   	0x8
#define EEPROM_RATE_GROUP_20M_MCS6   	0x9
#define EEPROM_RATE_GROUP_20M_MCS7   	0xa
#define EEPROM_RATE_GROUP_40M_MCS0   	0xb
#define EEPROM_RATE_GROUP_40M_MCS1_2   	0xc
#define EEPROM_RATE_GROUP_40M_MCS3_4   	0xd
#define EEPROM_RATE_GROUP_40M_MCS5   	0xe
#define EEPROM_RATE_GROUP_40M_MCS6   	0xf
#define EEPROM_RATE_GROUP_40M_MCS7   	0x10
#define EEPROM_RATE_GROUP_40M_MCS32   	0x11
	/*******************************************************************************
	*                             D A T A   T Y P E S
	********************************************************************************
	*/
	typedef struct _RF_CHANNEL_PROG_ENTRY {
	    UINT_32      chnlNum;
	    UINT_32      chnlFreq;
	} RF_CHANNEL_PROG_ENTRY, *PRF_CHANNEL_PROG_ENTRY;

	typedef struct _DATA_RATE_SETTING {
		const char * pszRate;
		INT_32 i4RateCfg;
	    UINT_8 ucRateGruopEep;
	} DATA_RATE_SETTING, *P_DATA_RATE_SETTING;

	/*******************************************************************************
	*                            P U B L I C   D A T A
	********************************************************************************
	*/
	const RF_CHANNEL_PROG_ENTRY chnlList[] = {
	    { CH_1,    2412000},
	    { CH_2,    2417000},
	    { CH_3,    2422000},
	    { CH_4,    2427000},
	    { CH_5,    2432000},
	    { CH_6,    2437000},
	    { CH_7,    2442000},
	    { CH_8,    2447000},
	    { CH_9,    2452000},
	    { CH_10,   2457000},
	    { CH_11,   2462000},
	    { CH_12,   2467000},
	    { CH_13,   2472000},
	    { CH_14,   2484000},
	    { CH_36,   5180000},
	    { CH_40,   5200000},
	    { CH_44,   5220000},
	    { CH_48,   5240000},
	    { CH_52,   5260000},
	    { CH_56,   5280000},
	    { CH_60,   5300000},
	    { CH_64,   5320000},
	    { CH_100,   5500000},
	    { CH_104,   5520000},
	    { CH_108,   5540000},
	    { CH_112,   5560000},
	    { CH_116,   5580000},
	    { CH_120,   5600000},
	    { CH_124,   5620000},
	    { CH_128,   5640000},
	    { CH_132,   5660000},
	    { CH_136,   5680000},
	    { CH_140,   5700000},
	    { CH_149,   5745000},
	    { CH_153,   5765000},
	    { CH_157,   5785000},
	    { CH_161,   5805000},
	    { CH_165,   5825000},
	};



	const DATA_RATE_SETTING rateSetting[] = {
		{"1M",		2,    EEPROM_RATE_GROUP_CCK},
		{"2M",		4,    EEPROM_RATE_GROUP_CCK},
		{"5.5M",	11,   EEPROM_RATE_GROUP_CCK},
		{"11M",		22,   EEPROM_RATE_GROUP_CCK},
		{"6M",		12,   EEPROM_RATE_GROUP_OFDM_6_9M},
		{"9M",		18,   EEPROM_RATE_GROUP_OFDM_6_9M},
		{"12M",		24,   EEPROM_RATE_GROUP_OFDM_12_18M},
		{"18M",		36,   EEPROM_RATE_GROUP_OFDM_12_18M},
		{"24M",		48,   EEPROM_RATE_GROUP_OFDM_24_36M},
		{"36M",		72,   EEPROM_RATE_GROUP_OFDM_24_36M},
		{"48M",		96,   EEPROM_RATE_GROUP_OFDM_48_54M},
		{"54M",		108,  EEPROM_RATE_GROUP_OFDM_48_54M}
	};

// moved from mt5921.h
#ifndef __BACKUP_REGISTER_NUM__
#define __BACKUP_REGISTER_NUM__
typedef enum _ENUM_BACKUP_REGISTER_T {
    PIN_REGISTER_IOUDR = 0,                /* I/O Pull Up/Down Register */
    PIN_REGISTER_IOPCR,                    /* I/O Pin Control Register */
    PIN_REGISTER_SCR,                      /* System Control Register */
    PIN_REGISTER_LCR,                      /* LED Control Register */
    PIN_REGISTER_BTCER0,                   /* BT Coexistence Control Register 0*/
    PIN_REGISTER_BTCER1,                   /* BT Coexistence Control Register 1*/
    PIN_REGISTER_BTCER2,                   /* BT Coexistence Control Register 2*/
    PIN_REGISTER_BTCER3,                   /* BT Coexistence Control Register 3*/
    PIN_REGISTER_RICR,                     /* RF Interface Control Register */
    MAC_REGISTER_CCR,                      /* Clock Control Register */
    MAC_REGISTER_ACDR4,                    /* AFE Configuration Data 4 Register*/  
    BB_REGISTER_CR97,
    BB_REGISTER_CR98,
    BB_REGISTER_CR100,
    BACKUP_REGISTER_NUM
} ENUM_BACKUP_REGISTER_T;
#endif 
#define PIN_LEVEL_HIGH                    (1)
#define PIN_LEVEL_LOW                     (0)

typedef enum _ENUM_IO_PIN_T {
    IO_PIN_GPIO0 = 0,                /* I/O Pull Up/Down Register */
    IO_PIN_GPIO1,                    /* I/O Pin Control Register */
    IO_PIN_GPIO2,                      /* System Control Register */
    IO_PIN_CS_N,                      /* LED Control Register */
    IO_PIN_OE_N,
    IO_PIN_WE_N,
    IO_PIN_BT_PRI_N,
    IO_PIN_D4,
    IO_PIN_D5,
    IO_PIN_D6,
    IO_PIN_D7,
    IO_PIN_D8,
    IO_PIN_D9,
    IO_PIN_D10,
    IO_PIN_D11,
    IO_PIN_D12,
    IO_PIN_D13,
    IO_PIN_D14,
    IO_PIN_D15,
    IO_PIN_WLAN_ACT,
    IO_PIN_LCR,
    IO_PIN_NUM
} ENUM_IO_PIN_T;

typedef enum _ENUM_RX_MULTICAST_TYPE_T {
    MC_TYPE_NONE,
    MC_TYPE_LOOKUP,
    MC_TYPE_ALL,
    MC_TYPE_UPDATE_ONLY,
    MC_TYPE_NUM
} ENUM_RX_MULTICAST_TYPE_T, *P_ENUM_RX_MULTICAST_TYPE_T;


#define CHIP_TYPE_MP    0
#define CHIP_TYPE_MPW   1
#define CHIP_TYPE_MAX   2

extern DATA_RATE_ENTRY_STRUCT_T arDataRateTable[21];

	/*******************************************************************************
	*                           P R I V A T E   D A T A
	********************************************************************************
	*/

	/*******************************************************************************
	*                                 M A C R O S
	********************************************************************************
	*/

	/*******************************************************************************
	*                   F U N C T I O N   D E C L A R A T I O N S
	********************************************************************************
	*/

	/*******************************************************************************
	*                              F U N C T I O N S
	********************************************************************************
	*/
	class CAdapter;
	class COID;
	
	class CWrapper{
	private:
			  static CAdapter *m_adapter;
			  static COID		*m_oid;
			  static char 	m_wrapperID[NAMESIZE];
			  static UINT_32 ChipID;
	public:
			CWrapper(void);
		    	virtual ~CWrapper(void);

			static 	INT_32 Initialize(const char *wrapperID);
			static  INT_32 Uninitialize();

			static  INT_32 setTestMode (void);
			static 	INT_32 setNormalMode (void);
			static	INT_32 setStandBy (void);
			static 	INT_32 setEEPRomSize ( INT_32 i4EepromSz);
			static 	INT_32 setEEPRomFromFile ( TCHAR * atcFileName);
			static 	INT_32 readTxPowerFromEEPromEx ( INT_32 i4ChnFreg, INT_32 i4Rate, INT_32 *pi4TxPwrGain, INT_32 *pi4Eirp, INT_32 *pi4ThermoVal);
			static 	INT_32 setEEPromCKSUpdated (void);
			static 	INT_32 getPacketTxStatusEx ( INT_32 *pi4SentCount, INT_32 *pi4AckCount, INT_32 *pi4Alc, INT_32 *pi4CckGainControl, INT_32 *pi4OfdmGainControl);
			static 	INT_32 getPacketRxStatus ( INT_32 *pi4RxOk, INT_32 *pi4RxCrcErr);
			static 	INT_32 setOutputPower ( INT_32 i4Rate, INT_32 i4TxPwrGain, INT_32 i4TxAnt);
			static 	INT_32 setLocalFrequecy ( INT_32 i4TxPwrGain, INT_32 i4TxAnt);
			static 	INT_32 setCarrierSuppression ( INT_32 i4Modulation, INT_32 i4TxPwrGain, INT_32 i4TxAnt);
			static 	INT_32 setOperatingCountry ( CHAR *acChregDomain);
			static 	INT_32 setChannel ( INT_32 i4ChFreqkHz);
			static 	INT_32 getSupportedRates ( UINT_16 *pu2RateBuf, INT_32 i4MaxNum);
			static 	INT_32 setOutputPin ( INT_32 i4PinIndex, INT_32 i4OutputLevel);
			static  INT_32 readEEPRom16 ( UINT_32 u4Offset, UINT_32 *pu4Value );
			static  INT_32 readSpecEEPRom16 ( UINT_32 u4Offset, UINT_32 *pu4Value );
			static 	INT_32 writeEEPRom16 ( UINT_32 u4Offset, UINT_32 u4Value);
			static 	INT_32 eepromReadByteStr ( UINT_32 u4Addr, UINT_32 u4Length, INT_8 *paucStr );
			static 	INT_32 eepromWriteByteStr ( UINT_32 u4Addr, UINT_32 u4Length, INT_8 *paucStr);
			static 	INT_32 setATParam ( UINT_32 u4FuncIndex, UINT_32 u4FuncData);
			static 	INT_32 getATParam ( UINT_32 u4FuncIndex, UINT_32 *pu4FuncData);
			static 	INT_32 setXtalTrimToCr (UINT_32 u4Value);
			static  INT_32 getXtalTrimToCr(UINT_32 *pu4Value);
			static 	INT_32 queryThermoInfo (INT_32 *pi4Enable, UINT_32 *pu4RawVal);
			static 	INT_32 setThermoEn ( INT_32 i4Enable);
			static 	INT_32 getEEPRomSize ( INT_32 *pi4EepromSz);
			static 	INT_32 getSpecEEPRomSize ( INT_32 *pi4EepromSz);
			static 	INT_32 setPnpPower ( INT_32 i4PowerMode);
			static 	INT_32 setAnritsu8860bTestSupportEn ( INT_32 i4Enable);
			static 	INT_32 writeMCR32(UINT_32 offset, UINT_32 value);
			static	INT_32 readMCR32(UINT_32 offset, UINT_32 *value);

			static	INT_32 getDPDLength(INT_32* pi4DPDLength);
			static	INT_32 readDPD32(UINT_32 offset, UINT_32 *value);
			static 	INT_32 writeDPD32(UINT_32 offset, UINT_32 value);
			static 	INT_32 setDPDFromFile (TCHAR * atcFileName);
			static	const char *getName(void) { return m_wrapperID; }	
            // Added by mtk54046 @ 2012-01-05 for get support channel list
            static INT_32 getSupportChannelList(UINT_32 *pau4Channel);
            // Added by mtk54046 @ 2012-11-15 for CTIA test setting/getting
            static INT_32 doCTIATestSet(UINT_32 u4Id, UINT_32 u4Value);
            static INT_32 doCTIATestGet(UINT_32 u4Id, UINT_32 *pu4Value);
		/*Not used, but reserved.*/

		/*
			INT_32 destroy (void);
			INT_32 disableDev (INT_8 *);
			INT_32 enableDev (INT_8 *);
			INT_32 setTxMaxPowerToEEPromEx ( INT_32, INT_32, INT_32, INT_32, INT_32);
			INT_32 setMACAddrToEEProm (INT_8*); 
			INT_32 setCountryToEEProm (INT_8*);
			INT_32 getChannelList ( UINT_32 *, INT_32);
			INT_32 getInputPin ( INT_32, INT_32 *);
			INT_32 restoreIOPin (void );
			INT_32 getEEPRomVersion ( INT_32 *);
			INT_32 setXtalFreqTrimToEEProm ( INT_32);
			INT_32 setRcpiOffsetToEEProm ( INT_32,  INT_32);

		*/
		};

}
#endif

