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
** $Id: mt5921.cpp,v 1.6 2008/11/21 14:16:43 MTK01725 Exp $
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
** $Log: mt6620.cpp,v $
 *
 * 07 04 2011 xiao.liu
 * [ALPS00058377] [Need Patch] [Volunteer Patch][EM] modify wifi.
 * .
 *
 * 04 29 2011 xiao.liu
 * [ALPS00044734] [Need Patch] [Volunteer Patch][EM] resolve all build warning. alps.GB
 * warning. alps
 *
 * 12 15 2010 Chaozhong.Liang
 * test done for mt6620 wifi 
 *
 * 11 01 2010 Chaozhong.Liang
 * init from mt5921.cpp
 * .
 *
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.6  2008/11/21 14:16:43  MTK01725
** Change another method to test INT_N with IO pin GPIO_0
**
** Revision 1.5  2008/11/11 03:07:07  MTK01725
** Return a fixed channel list. This function is still not ready.
**
** Revision 1.4  2008/10/28 13:45:44  MTK01725
** Frog add IO Pin Test and other little modification.
**
** Revision 1.3  2008/06/12 02:35:05  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:50:37  MTK01385
** 1. add setXtalTrimToCr(), queryThermoinfo() and setThermoEn().
**
** Revision 1.1  2008/05/26 14:04:37  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.11  2008/05/19 12:09:34  MTK01385
** 1. before BB output power, MAC should tx a normal packet first
**
** Revision 1.10  2008/04/11 02:28:29  MTK01385
** 1. Based on MT5921MP_BB_RF_Config_v0.8.doc
**  to update Tx carrier Suppression function.
** 2. Fix MCR offset typo in Tx output power and Tx carrier Suppression functions.
** (MCR_ACDR4(x), MCR_ACDR1 (v))
**
** Revision 1.9  2008/03/25 09:34:10  MTK01385
** 1. Based on MT5921MP_BB_RF_Config_v0.4.doc
**  to update Tx output power and Tx carrier Suppression functions.
** 2. Fix LED pin output low error when do the loopback test.
**
** Revision 1.8  2008/02/23 17:35:51  MTK01385
** 1. MT5921 EEPROM related definitions move from header file to .c file.
**
** Revision 1.7  2008/02/23 16:31:43  MTK01385
** 1 add support set Crystal Frequency Trim and RCPI offset to eeprom.
**
** Revision 1.5  2008/02/22 13:16:12  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.4  2008/01/08 01:37:58  MTK01385
** 1. Move EEPROM definitions from MT5921 c file to header file for UI usage.
**
** Revision 1.3  2008/01/03 07:56:01  MTK01385
** 1. Add Set Chip type function.
** 2. Modify eeprom checksum update.
**
** Revision 1.2  2007/12/11 15:44:49  MTK01385
** 1. 1.	Modify Tx Output Power, Tx Carrier Suppression and Tx Local Frequency function based on SD request.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.3  2007/11/12 06:25:04  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.2  2007/11/09 03:56:48  MTK01267
** refine code
**
** Revision 1.1  2007/10/16 06:58:51  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/
#include "mt6620.h"
#include "param.h"

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
namespace android{
	typedef enum {
		cTxPwr2G4Cck = 0x0,		/*1M ~ 11M*/
		acReserved_0, 
		acReserved_1, 
		acReserved_2, 
		
		cTxPwr2G4OFDM_BPSK, 		/*2.4GHz  6M ~ 9M*/
		cTxPwr2G4OFDM_QPSK, 		/*2.4GHz  12M ~ 18M*/
		cTxPwr2G4OFDM_16QAM, 		/*2.4GHz  24M ~ 36M*/
		cTxPwr2G4OFDM_Reserved, 
		cTxPwr2G4OFDM_48Mbps, 		/*2.4GHz  48M*/
		cTxPwr2G4OFDM_54Mbps, 		/*2.4GHz  54M*/
		
		cTxPwr2G4HT20_BPSK, 		/*2.4GHz  MCS0-20Mbandwidth*/
		cTxPwr2G4HT20_QPSK, 		/*2.4GHz  MCS1_2-20Mbandwidth*/
		cTxPwr2G4HT20_16QAM, 		/*2.4GHz  MCS3_4-20Mbandwidth*/
		cTxPwr2G4HT20_MCS5, 		/*2.4GHz  MCS5-20Mbandwidth*/
		cTxPwr2G4HT20_MCS6, 		/*2.4GHz  MCS6-20Mbandwidth*/
		cTxPwr2G4HT20_MCS7, 		/*2.4GHz  MCS7-20Mbandwidth*/
		
		cTxPwr2G4HT40_BPSK, 		/*2.4GHz  MCS0-40Mbandwidth*/
		cTxPwr2G4HT40_QPSK, 		/*2.4GHz  MCS1_2-40Mbandwidth*/
		cTxPwr2G4HT40_16QAM, 		/*2.4GHz  MCS3_4-40Mbandwidth*/
		cTxPwr2G4HT40_MCS5, 		/*2.4GHz  MCS5-40Mbandwidth*/
		cTxPwr2G4HT40_MCS6, 		/*2.4GHz  MCS6-40Mbandwidth*/
		cTxPwr2G4HT40_MCS7, 		/*2.4GHz  MCS7-40Mbandwidth*/
		
		cTxPwr5GOFDM_BPSK, 			/*5GHz  6M ~ 9M*/
		cTxPwr5GOFDM_QPSK, 			/*5GHz  12M ~ 18M*/
		cTxPwr5GOFDM_16QAM, 		/*5GHz  24M ~ 36M*/
		cTxPwr5GOFDM_Reserved, 
		cTxPwr5GOFDM_48Mbps, 		/*5GHz	48M*/
		cTxPwr5GOFDM_54Mbps, 		/*5GHz	54M*/
		
		cTxPwr5GHT20_BPSK, 			/*5GHz	MCS0-20Mbandwidth*/
		cTxPwr5GHT20_QPSK, 			/*5GHz	MCS1_2-20Mbandwidth*/
		cTxPwr5GHT20_16QAM, 		/*5GHz	MCS3_4-20Mbandwidth*/
		cTxPwr5GHT20_MCS5, 			/*5GHz	MCS5-20Mbandwidth*/
		cTxPwr5GHT20_MCS6, 			/*5GHz	MCS6-20Mbandwidth*/
		cTxPwr5GHT20_MCS7, 			/*5GHz	MCS7-20Mbandwidth*/
		
		cTxPwr5GHT40_BPSK, 			/*5GHz	MCS0-40Mbandwidth*/
		cTxPwr5GHT40_QPSK, 			/*5GHz	MCS1_2-40Mbandwidth*/
		cTxPwr5GHT40_16QAM, 		/*5GHz	MCS3_4-40Mbandwidth*/
		cTxPwr5GHT40_MCS5, 			/*5GHz	MCS5-40Mbandwidth*/
		cTxPwr5GHT40_MCS6, 			/*5GHz	MCS6-40Mbandwidth*/
		cTxPwr5GHT40_MCS7, 			/*5GHz	MCS7-40Mbandwidth*/
		}NVRAM_RATE_BANDWIDTH_GROUP;
typedef struct _DATA_RATE_REG_MAPPING {
		INT_32 i4RateIndex;
	    INT_32 i4RegIndex;
	} DATA_RATE_CFG_MAPPING, *P_DATA_RATE_CFG_MAPPING;
const DATA_RATE_CFG_MAPPING rateRegMapping[] = {
		{0, 0},
		{1, 0},
		{2, 0},
		{3, 0},
		
		{4, cTxPwr2G4OFDM_BPSK},
		{5, cTxPwr2G4OFDM_BPSK},
		{6, cTxPwr2G4OFDM_QPSK},
		{7, cTxPwr2G4OFDM_QPSK},
		{8, cTxPwr2G4OFDM_16QAM},
		{9, cTxPwr2G4OFDM_16QAM},
		{10, cTxPwr2G4OFDM_48Mbps},
		{11, cTxPwr2G4OFDM_54Mbps},
		
		{12, cTxPwr2G4HT20_BPSK},
		{13, cTxPwr2G4HT20_QPSK},
		{14, cTxPwr2G4HT20_QPSK},
		{15, cTxPwr2G4HT20_16QAM},
		{16, cTxPwr2G4HT20_16QAM},
		{18, cTxPwr2G4HT20_MCS5},
		{18, cTxPwr2G4HT20_MCS6},
		{19, cTxPwr2G4HT20_MCS7},
		
	};
/************************************************************************
*                          F U N C T I O N S
*************************************************************************
*/
/*_________________________________________________________________________
**  setChannel
**
**  descriptions: Set RF channel
**  parameters:
**          nCardIndex: Which NIC
**         channelConfig, channel frequency, in KHz
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setChannel (
	INT_32 channelConfig
	)
{
 
	    MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;
	
		DEBUGFUNC("CAdapter::setChannel");
		em_error((char*)"CAdapter::setChannel(channel index = %x)\n", channelConfig);
	
	    m_AT_cmd.funcIndex = 18;	//center frquencey
	    m_AT_cmd.funcData = channelConfig;	//in unit of KHz
	
		if (!m_OID->setOID(this,
			OID_CUSTOM_MTK_WIFI_TEST,
	        (CHAR *)&m_AT_cmd,
	        sizeof(MTK_WIFI_TEST_STRUC_V2))
			) {
	        ERRORLOG("setChannel failed");
	        return ERROR_RFTEST_NDIS_OID_FAILURE;
	    }
		return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getChannel
**
**  descriptions: Query RF channel
**  parameters:
**         nCardIndex: NIC index number
**  output:
**         channelConfig_p,  pointer to channel configuration (in KHz)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getChannel (
	INT_32 *channelConfig_p
	)
{

		    MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;
			UINT_32 BytesRead;
		
			DEBUGFUNC("CMT6620::getChannel");
		
		    m_AT_cmd.funcIndex = 18;
		    m_AT_cmd.funcData = 0;  //clear before read
		
			if (!m_OID->queryOID(this,
					OID_CUSTOM_MTK_WIFI_TEST,
					(CHAR *)&m_AT_cmd,
					sizeof(MTK_WIFI_TEST_STRUC_V2),
					&BytesRead)
					) {
		        ERRORLOG("getChannel failed");
		        return ERROR_RFTEST_NDIS_OID_FAILURE;
		    }
		
			*channelConfig_p = m_AT_cmd.funcData;
		
			return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  qryRxStatus
**
**  descriptions: Query Rx status
**  parameters:
**         nCardIndex: Which NIC
**         rxOk_p,            pointer to recv ok count from interrupt
**         rxCrcErr_p,        pointer to recv CRC error count from interrupt
**  return:
**         check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getPacketRxStatus (
    INT_32 *rxOk_p,
    INT_32 *rxCrcErr_p
    )
{
		MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;
		UINT_32 BytesRead;
	
		DEBUGFUNC("CMT6620::getPacketRxStatus");
	
	    m_AT_cmd.funcIndex = 34;	//received packet ok count
	    m_AT_cmd.funcData = 0;  //clear before read
	
		if (!m_OID->queryOID(this,
				OID_CUSTOM_MTK_WIFI_TEST,
				(CHAR *)&m_AT_cmd,
				sizeof(MTK_WIFI_TEST_STRUC_V2),
				&BytesRead)
				) {
	        ERRORLOG("getPacketRxStatus failed");
	        return ERROR_RFTEST_NDIS_OID_FAILURE;
	    }
	
		*rxOk_p = m_AT_cmd.funcData;
		m_AT_cmd.funcIndex = 35;	//received packet crc error count
	    m_AT_cmd.funcData = 0;  //clear before read
	
		if (!m_OID->queryOID(this,
				OID_CUSTOM_MTK_WIFI_TEST,
				(CHAR *)&m_AT_cmd,
				sizeof(MTK_WIFI_TEST_STRUC_V2),
				&BytesRead)
				) {
	        ERRORLOG("getPacketRxStatus failed");
	        return ERROR_RFTEST_NDIS_OID_FAILURE;
	    }
		*rxCrcErr_p = m_AT_cmd.funcData;
	
		return ERROR_RFTEST_SUCCESS;		

}


/*_________________________________________________________________________
**  readMCR32
**
**  descriptions: read 32-bit data from MCR
**  parameters:
**          nCardIndex: NIC index number
**          offset: address offset of the MCR
**  output:
**          value:  value read from the MCR (size: 4 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readMCR32 (UINT_32 offset, UINT_32 * value)
{

	IPC_MCR_RW_STRUC_V2 m_sMACREG_RW;
	UINT_32 BytesRead;

	DEBUGFUNC("CMT6620::readMCR32");

    m_sMACREG_RW.mcrIndex = offset;
    m_sMACREG_RW.mcrData = 0;  //clear before read

	if (!m_OID->queryOID(this,
			OID_IPC_MCR_RW,
			(CHAR *)&m_sMACREG_RW,
			sizeof(IPC_MCR_RW_STRUC_V2),
			&BytesRead)
			) {
        ERRORLOG("Failed to read configuration");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sMACREG_RW.mcrData;
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  writeMCR32
**
**  descriptions: write 32-bit data to MCR
**  parameters:
**          nCardIndex: NIC index number
**          offset: address offset of the MCR
**          value:  value set to the MCR (size: 4 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::writeMCR32 (UINT_32 offset, UINT_32 value)
{
	IPC_MCR_RW_STRUC_V2 m_sMACREG_RW;

	DEBUGFUNC("CMT6620::writeMCR32");

    m_sMACREG_RW.mcrIndex = offset;
    m_sMACREG_RW.mcrData = value;

	if (!m_OID->setOID(this,
		OID_IPC_MCR_RW,
        (CHAR *)&m_sMACREG_RW,
        sizeof(IPC_MCR_RW_STRUC_V2))
		) {
        ERRORLOG("writeMCR32 failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  ReadBBReg8
**
**  descriptions: read 8-bit data in MMI interface
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the BB registers
**  output
**          value:  value read from the BB register (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readBBCR8 (UINT_32 offset, UINT_32* value)
{

	DEBUGFUNC("CMT6620::readBBCR8:omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  WriteBBReg8
**
**  descriptions: write 8-bit data in MMI interface
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the BB registers
**          value:  value set to the BB register (size: 1 Byte)
**  return:
**          check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::writeBBCR8 (UINT_32 offset, UINT_32 value)
{
	DEBUGFUNC("CMT6620::writeBBCR8:omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  ReadEEPRom16
**
**  descriptions: read 16-bit data to EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the EEPRom offset
**  output:
**          value:  value read from the EEPRom offset (size: 2 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readEEPRom16 (UINT_32 offset, UINT_32 * value)
{
	IPC_EEPROM_RW_STRUC_V2 m_sEEPRom_RW;
	UINT_32 BytesRead;
	DEBUGFUNC("CMT6620::readEEPRom16");
	INT_32 status;
	INT_32 i4EepromSize;	
	status = getEEPRomSize(&i4EepromSize);
    if (status < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }
    /*for temp 2010.11.26
    if(i4EepromSize == ESCR_EEPROM_NOT_PRESENT ) {
        em_error("%s", "EEProm not present.\n");
        return ERROR_RFTEST_EEPROM_NOT_PRESENT;
    }
    */
	m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_RW;
    m_sEEPRom_RW.eepromIndex = (UINT_8)offset;
    m_sEEPRom_RW.eepromData = 0;   //clear before read


    if (!m_OID->queryOID(this,
	        isEepromUsed ? OID_IPC_EEPROM_RW : OID_IPC_NVRAM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("readEEPRom16 failed ");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	*value = m_sEEPRom_RW.eepromData;
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  readSpecEEPRom16
**
**  descriptions: read 16-bit data from EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the EEPRom offset
**  output:
**          value:  value read from the EEPRom offset (size: 2 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note: this function is for EEPRom only --for EM button "EEPROM save as file"
**__________________________________________________________________________*/
INT_32
CMT6620::readSpecEEPRom16 (UINT_32 offset, UINT_32 * value)
{
	IPC_EEPROM_RW_STRUC_V2 m_sEEPRom_RW;
	UINT_32 BytesRead;
	DEBUGFUNC("CMT6620::readSpecEEPRom16");
	m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_RW;
    m_sEEPRom_RW.eepromIndex = (UINT_8)offset;
    m_sEEPRom_RW.eepromData = 0;   //clear before read


    if (!m_OID->queryOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("Failed to read configuration");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sEEPRom_RW.eepromData;
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  WriteEEPRom16
**
**  descriptions: write 16-bit data to EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          offset: index of the EEPRom offset (0, 1, 2, 3......)
**          value:  value set to the EEPRom offset (size: 2 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::writeEEPRom16 (UINT_32 offset, UINT_32 value)
{
	IPC_EEPROM_RW_STRUC_V2 m_sEEPRom_RW;
	INT_32 status;
	INT_32 i4EepromSize;
	DEBUGFUNC("CMT6620::readEEPRom16");
    m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_RW;
    m_sEEPRom_RW.eepromIndex = (UINT_8)offset;
    m_sEEPRom_RW.eepromData = (UINT_16)value;

	if (!m_OID->setOID(this,
            isEepromUsed ? OID_IPC_EEPROM_RW : OID_IPC_NVRAM_RW,
            (CHAR *)&m_sEEPRom_RW,
            sizeof(IPC_EEPROM_RW_STRUC_V2))
            ) {
        ERRORLOG("writeEEPRom16 failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
	}
	return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  setTestMode
**
**  descriptions: set wifi chip to test mode
**  parameters:
**          none
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**  reviewed ok for 6620
**__________________________________________________________________________*/
INT_32
CMT6620::setTestMode(void)
{
    PARAM_RFTEST_INFO info;

    DEBUGFUNC("CMT6620::setTestMode");
    info.Length = sizeof(info.Length);
	if ( !m_OID->setOID(this,
			OID_IPC_TEST_MODE,
			(CHAR *) NULL,
			0)) {
      	em_error((char*)("Failed to set driver to test mode, error %s\n", strerror(errno)));
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}
	em_error((char*)"setTestMode succeed\n");
    // Store original IO register setting
    readMCR32(MCR_IOUDR,   &u4IoRegSave[PIN_REGISTER_IOUDR]);
    readMCR32(MCR_IOPCR,   &u4IoRegSave[PIN_REGISTER_IOPCR]);
    readMCR32(MCR_SCR,     &u4IoRegSave[PIN_REGISTER_SCR]);
    readMCR32(MCR_LCR,     &u4IoRegSave[PIN_REGISTER_LCR]);
    readMCR32(MCR_BTCER0,  &u4IoRegSave[PIN_REGISTER_BTCER0]);
    readMCR32(MCR_BTCER1,  &u4IoRegSave[PIN_REGISTER_BTCER1]);
    readMCR32(MCR_BTCER2,  &u4IoRegSave[PIN_REGISTER_BTCER2]);
    readMCR32(MCR_BTCER3,  &u4IoRegSave[PIN_REGISTER_BTCER3]);
    readMCR32(MCR_RICR,    &u4IoRegSave[PIN_REGISTER_RICR]);

    //Store original BB register setting
    BBCR_RD_32(BBCR_CR97, &u4IoRegSave[BB_REGISTER_CR97]);
    BBCR_RD_32(BBCR_CR98, &u4IoRegSave[BB_REGISTER_CR98]);
    BBCR_RD_32(BBCR_CR100, &u4IoRegSave[BB_REGISTER_CR100]);

    //Store original MAC setting for BB test mode
    readMCR32(MCR_CCR,    &u4IoRegSave[MAC_REGISTER_CCR]);
    readMCR32(MCR_ACDR4,    &u4IoRegSave[MAC_REGISTER_ACDR4]);

    return ERROR_RFTEST_SUCCESS;
} /* setTestMode */

/*_________________________________________________________________________
**  setDriverNormalMode
**
**  descriptions: Set the driver to normal mode
**  parameters:
**  return:
**          0, if successful
**         -1, otherwise
**  note:
**  reviewed ok for 6620
**__________________________________________________________________________*/
INT_32
CMT6620::setNormalMode(void)
{
    UINT_32 tmpU4;

    DEBUGFUNC("CMT6620::setNormalMode");

    SetATParam(1, 0); /* Make sure any TX/RX test is stopped */
    MTK_WIFI_TEST_STRUC_V2 m_TMP_cmd;
	if ( !m_OID->setOID(this,
            OID_IPC_ABORT_TEST_MODE,
            (CHAR*) NULL,
            0)
            ) {
        ERRORLOG("setNormalMode failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    /* Restore original IO register setting */
    //restoreIOPin();

    /* Restore original BB register setting */
    //setStandBy();
    
    return ERROR_RFTEST_SUCCESS;
} /* setNormalMode */

/*_________________________________________________________________________
**  StandBy
**
**  descriptions:   stop BB continuous Tx mode
**  parameters:
**  return:
**           0, if successful
**          -1, Failed to set stop pattern
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setStandBy(void)
{
    UINT_32 u4Tmp;

    SetATParam(1, 0);        // Stop TX/RX


    /*===================frog keep the BB init and RF init code below=============*/

    /* Restore original BB register setting */
	//BBCR_WR_32(BBCR_CR97, u4IoRegSave[BB_REGISTER_CR97]);
    BBCR_WR_32(BBCR_CR97, BBCR_CR97_DEFAULT_VALUE);                            // For TX Default Value
	//BBCR_WR_32(BBCR_CR98, u4IoRegSave[BB_REGISTER_CR98]);
	//BBCR_WR_32(BBCR_CR100, u4IoRegSave[BB_REGISTER_CR100]);

    /* Resotre original MAC register for BB test mode */
    readMCR32(MCR_CCR,   &u4Tmp);
    u4Tmp = (u4Tmp &~ CCR_BB_CLOCK_MASK) | CCR_BB_CLOCK_CTRL_BY_TR_PE;           // Baseband Clock
    writeMCR32(MCR_CCR,   u4Tmp); 

    writeMCR32(MCR_RICR,  u4IoRegSave[PIN_REGISTER_RICR]);      // RF Interface Control Register
    writeMCR32(MCR_ACDR4, u4IoRegSave[MAC_REGISTER_ACDR4]);     // AFE Configuration Data Register


    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setOutputPower
**
**  descriptions: Enable continuous tx. PAU disabled. Manually controlled the
**                  TX_RX, TX_PE, PA_PE, TR_SW on, RX_PE off.
**  parameters:
**          nTxRate: 0/1/2/3 fo 1M/2M/5.5M/11M bps
**  return:
**          0, if successful
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setOutputPower(
    INT_32 nTxRate,
    INT_32 txPower,
    INT_32 txAntenna
    )
{
	DEBUGFUNC("CMT6620::setOutputPower:omitted");
/*	
    P_DATA_RATE_ENTRY_STRUCT_T  prDataRateEntry = NULL;
    BOOL                    	fgIsShortPreamble;
    INT_32                      i4Tmp;
    UINT_32                     u4BBCR;
    UINT_32                     u4MCR;
    INT_32                      result;
    INT_32                      i4Cnt;
    UINT_32                     u4Value;
    INT_32                      i4AtRate;
    em_printf(MSG_DEBUG, "Tx Rate = %d, Tx power = %d, Tx Antenna = %d.\n", nTxRate, txPower, txAntenna);
//this part we need to review
    if(nTxRate > DATE_RATE_WITH_SHORT_PREAMBLE) {
        fgIsShortPreamble = TRUE;
    }
    else {
        fgIsShortPreamble = FALSE;
    }

    nTxRate %= DATE_RATE_WITH_SHORT_PREAMBLE;

    for (i4Tmp =0; i4Tmp< NUM_SUPPORTED_RATES; i4Tmp++) {
        if(arDataRateTable[i4Tmp].i4BBUiCfg == nTxRate )
        {
            i4AtRate = i4Tmp;
            prDataRateEntry = &arDataRateTable[i4Tmp];
            em_printf(MSG_DEBUG, "Table Rate index = %d.\n", i4Tmp);

        }
    }

    //before BB output power, MAC should tx a normal packet first.
  
	// Rate
	SetATParam(3, i4AtRate);

    // Length
	SetATParam(6, 100);

	// Count
	SetATParam(7, 1);
	
	// TXOP
	//SetATParam(10, 0x00020000);

	// Retrylimit
	SetATParam(13, 1);

	// Enable TX
	SetATParam(1, 1);
	
    //Sleep(1000);
    // Check Tx Complete 
    for (i4Cnt = 0; i4Cnt < 100; i4Cnt++) {
	    GetATParam(1, &u4Value);
	    if (u4Value == 0) {
    			break;
	    } else {
		    sleep(10);
	    }
    }
    
    //end
    u4MCR = (BITS(19, 20) | BIT (18) | BIT (8) | BIT (4) | BIT (2)   );
    result = writeMCR32(MCR_RICR, u4MCR);
    if(result)
        return result;

    //MCR_CCR 0x0060
    readMCR32(MCR_CCR,   &u4MCR);
    u4MCR = (u4MCR &~ CCR_BB_CLOCK_MASK) | CCR_BB_CLOCK_ON;
    writeMCR32(MCR_CCR,   u4MCR); 

    if (prDataRateEntry == NULL) {
       em_error("The Data Rate index = %d is not supported.\n", nTxRate);
       return ERROR_RFTEST_UNSUPPORTED;

    }

    if (prDataRateEntry->fgIsCCK == FALSE) {
        fgIsShortPreamble = FALSE;  //OFDM does not support short preamble
    }
    



    u4BBCR = ( ((prDataRateEntry->i4BBHwVal <<10) & BITS(10,13)) |\
            ((txPower<<0) & BITS(0,7)) | ((txAntenna<<9) &  BIT(9)) );
    if(fgIsShortPreamble) {
        u4BBCR |= BIT(14);
    }
    if(prDataRateEntry->fgIsCCK == FALSE ) {
        u4BBCR |= BIT(15);
    }
    BBCR_WR_32(BBCR_CR98, u4BBCR);
    em_printf(MSG_DEBUG,"BBCR_CR98 = 0x%08X.\n", u4BBCR);

    u4BBCR = (BITS(14, 15) | BIT (3) );
    BBCR_WR_32(BBCR_CR97, u4BBCR);
    em_printf(MSG_DEBUG, "BBCR_CR97 = 0x%08X.\n", u4BBCR);
*/
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setCarrierSuppressionMeasure
**
**  descriptions:
**  parameters:
**              nModulationType: 0 CCK, 1 OFDM
**  return:
**          0, if successful
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setCarrierSuppression(
    INT_32 nModulationType,
    INT_32 txPower,
    INT_32 txAntenna
    )
{
	DEBUGFUNC("CMT6620::setCarrierSuppression:omitted");
/*    
	UINT_32                     u4BBCR;
    UINT_32                     u4MCR;
    INT_32                      result;
    u4MCR = (BITS(19, 20) | BIT (18) | BIT (8) | BIT (4) | BIT (2)  );
    result = writeMCR32(MCR_RICR, u4MCR);
    if(result)
        return result;

    //BB_CR60
    readMCR32(MCR_CCR,   &u4MCR);
    u4MCR = (u4MCR &~ CCR_BB_CLOCK_MASK) | CCR_BB_CLOCK_ON;
    writeMCR32(MCR_CCR,   u4MCR); 

    //MCR (0x0084): b31 = 1・b0
    readMCR32(MCR_ACDR4,   &u4MCR);
    u4MCR = (u4MCR &~ ACDR4_DAC_CLK_MODE);
    writeMCR32(MCR_ACDR4,   u4MCR); 

    //MCR (0x0078): b1-b0 = CCK : 2・b01 , OFDM :2・b10
    readMCR32(MCR_ACDR1,   &u4MCR);
    u4MCR = (u4MCR &~ ACDR1_RG_DACCLK_SEL_MASK);
    if (nModulationType ) { //OFDM Mode
        u4MCR |= ACDR1_RG_DACCLK_SEL_80M;
    }
    else { //CCK Mode
        u4MCR |= ACDR1_RG_DACCLK_SEL_40M;
    }
    writeMCR32(MCR_ACDR1,   u4MCR); 

    u4BBCR = ( ((txPower<<0) & BITS(0,7)) | ((txAntenna<<9) &  BIT(9)) );
    BBCR_WR_32(BBCR_CR98, u4BBCR);


    if (nModulationType) { //OFDM Mode
        //set Test Mode continuous wave enable, OFDM long train symbol
         u4BBCR = ( BIT(15) | BIT (12)  | BIT (3) );
         BBCR_WR_32(BBCR_CR97, u4BBCR);
    }
	else { //CCK Mode
        //set Test Mode continuous wave enable, QPSK 90 degree
         u4BBCR = ( BIT(15) | BIT (13) | BIT (12) | BIT (3) );
         BBCR_WR_32(BBCR_CR97, u4BBCR);

    }
*/
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setLocalFrequecy
**
**  descriptions:
**  parameters:
**              nCardIndex: Which NIC
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setLocalFrequecy(INT_32 txPower, INT_32 txAntenna)
{
	DEBUGFUNC("CMT6620::setLocalFrequecy:omitted");
/*
    UINT_32 u4BBCR;
    UINT_32                     u4MCR;
    INT_32                      result;


    u4MCR = (BITS(19, 20) | BIT (18) | BIT (8) | BIT (4) | BIT (2)   );
    result = writeMCR32(MCR_RICR, u4MCR);
    if(result)
        return result;

   
    u4BBCR = 0x7F7F;
    BBCR_WR_32(BBCR_CR100, u4BBCR);

    u4BBCR = ( ((txPower<<0) & BITS(0,7)) | ((txAntenna<<9) &  BIT(9)) );
    BBCR_WR_32(BBCR_CR98, u4BBCR);

    //u4BBCR = (BIT(9) | BIT(15) | BIT (3) );
    u4BBCR = 0x0008;
    BBCR_WR_32(BBCR_CR97, u4BBCR);
*/
	return ERROR_RFTEST_SUCCESS;
}

CHAR *
CMT6620::getRFICType(void)
{
	DEBUGFUNC("CMT6620::getRFICType:omitted");
	return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT6620::readBBCRStatistic(
    UINT_16* ed,
    UINT_16* osd,
    UINT_16* sq1,
    UINT_16* sfd,
    UINT_16* fcs)
{
	DEBUGFUNC("CMT6620::readBBCRStatistic:omitted");
	return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT6620::getRadarStatus(void)
{
	DEBUGFUNC("CMT6620::getRadarStatus:omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setAlcInfoToEeprom
**
**  descriptions:
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setAlcInfoToEeprom (
    INT_32 b24gAvailable,
    INT_32 b5gAvailable)
{
	DEBUGFUNC("CMT6620::setAlcInfoToEeprom:omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getAlcInfo
**
**  descriptions: Return ALC info field
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getAlcInfo(
    INT_32* b24gAvailable,
    INT_32* b5gAvailable,
    INT_32* bUseSlopeRate)
{
	DEBUGFUNC((char*)"CMT6620::getAlcInfo:omitted");
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setAlcSlopeRatioToEeprom
**
**  descriptions: Update ALC Info Field
**  parameters:
**    slope1Divider:
**    slope1Dividend:
**    slope2Divider:
**    slope2Dividend:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setAlcSlopeRatioToEeprom (
    INT_32	slope1Divider,
    INT_32	slope1Dividend,
    INT_32	slope2Divider,
    INT_32	slope2Dividend
    )
{
	DEBUGFUNC((char*)"CMT6620::setAlcSlopeRatioToEeprom:omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getAlcSlopeRatio
**
**  descriptions: Return ALC Slope Ration field
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getAlcSlopeRatio (
    INT_32* slope1Divider,
    INT_32* slope1Dividend,
    INT_32* slope2Divider,
    INT_32* slope2Dividend
    )
{
	DEBUGFUNC((char*)"CMT6620::getAlcSlopeRatio:omitted");
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setTXMaxPowerToEEProm
**
**  descriptions: Save TX Power from EEPROM
**  parameters:
**             nChannel:   Channel frequence in kHz
**             bCck:       1: CCK 0:OFDM
**             nTxPwr:     TX Power
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setTXMaxPowerToEEProm(INT_32 channelFreg, INT_32 bCck, INT_32 nTxPwr)
{
	DEBUGFUNC((char*)"CMT6620::setTXMaxPowerToEEProm:omitted");
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setTXMaxPowerToEEPromEx
**
**  descriptions: Save TX gain control value to EEPROM
**  parameters:
**             nCardIndex:  Which adapter.
**             nChannel:    Channel frequence in kHz
**             rate:        In unit of 500K
**             gainControl: Gain control value
**             outputPower: Measured output power
**             targetAlc:   Expected target ALC
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setTXMaxPowerToEEPromEx(
	INT_32 channelFreq,
	INT_32 rate,
	INT_32 gainControl,
	INT_32 outputPower,
	INT_32 targetAlc
	)
{
	DEBUGFUNC((char*)"CMT6620::setTXMaxPowerToEEPromEx:omitted");
	/*
    char    aucSzTmp[512]; 
    UINT_16 u2TxGroupLenWord;
    UINT_32 u4ByteOffset;
    INT_32  i4Ch;
    INT_32  i4Band;
    UINT_8  ucRateGroup;
    UINT_8  ucI;
    
    getChannelBand(channelFreq, &i4Ch, &i4Band);

    if (i4Band == 1 || i4Ch > 14) {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    ucRateGroup = 255;
    //get rate group
    for (ucI =0; ucI< NUM_SUPPORTED_RATES; ucI++) {
        if(arDataRateTable[ucI].i4BBUiCfg == rate )
        {
            ucRateGroup = arDataRateTable[ucI].ucRateGruopEep;
        }
    }
    if (ucRateGroup > EEPROM_RATE_GROUP_OFDM_48_54M) {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    u2TxGroupLenWord = EEPROM_2G_CCK_TXPWR_GAIN_END - EEPROM_2G_CCK_TXPWR_GAIN_START +1;
    u4ByteOffset = (EEPROM_2G_CCK_TXPWR_GAIN_START + \
     (ucRateGroup* u2TxGroupLenWord))*2 + (i4Ch-1)*3;
 
    sprintf(aucSzTmp, "%02X%02X%02X", gainControl, outputPower, targetAlc);
    eepromWriteByteStr(u4ByteOffset, 3, aucSzTmp);
*/
	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  readTxPowerFromEEProm
**
**  descriptions: Retrieve TX Power from EEPROM
**  parameters:
**             nCardIndex: Which adapter.
**             nChannel:    Channel number
**             bCck:          1: CCK 0:OFDM
**             ndBindex:    0: MAX 1:0db 2:5db 3:10db
**  result:
**             nTxPwr:       Points to buffer of TX Power
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readTxPowerFromEEProm (
    INT_32 channelFreq,
    INT_32 bCck,
    INT_32 *nTxPwr,
    INT_32 ndBindex
    )
{
	DEBUGFUNC((char*)"CMT6620::readTxPowerFromEEProm:omitted");
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  readTxPowerFromEEPromEx
**
**  descriptions: Retrieve TX Power from EEPROM
**  parameters:
**             nCardIndex:  Which adapter.
**             channelFreq: Frequency
**             rate:        Rate in unit of 500K
**  result:
**             nTxPwr:       Points to buffer of TX Power
**             outputPower:  Points to buffer of output power. In unit of dBm.
**             targetAlc:    Points to buffer of target ALC value.
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readTxPowerFromEEPromEx(
    INT_32 comboChanIndex,		/*channel index and bandwidth flag, bit 0~30: channel index ; bit 31:bandwidth flag(1-20MHz, 0-40MHz)*/
    INT_32 rateIndex,			/**/	
    INT_32 *pi4TxPwrGain,
    INT_32 *pi4Eirp,		/*not used*/
    INT_32 *pi4ThermoVal	/*not used*/
    )
{

	INT_32 oddFlag = 0x0;
	INT_32 registerOffset = 0x0;
	UINT_32 txPwrValue = 0x00;
	INT_32 channelIndex = comboChanIndex & 0x7fff;
	INT_32 result = 0x00;
	DEBUGFUNC("CMT6620::readTxPowerFromEEPromEx");
	em_printf(MSG_DEBUG, (char*)"comboChanIndex = %d, rateIndex = %d", comboChanIndex, rateIndex );
	
	INT_32 txPowerBase = 12;	//base offset of tx power vlaue in NVRAM/EEPROM
	INT_32 fre5GFlag = 0;		/*0- 2G4 frequency serials; 1- 5G frequency serials; */
	INT_32 band40MFlag  = (comboChanIndex & 0x8000) == 0 ?  0 : 1;	/*bandwidth flag: 0 - 20MHz serials(including 20MHz , L20MHz and U20MHz ); 1 - 40MHz serials*/
	if(channelIndex > 13)
	{
		fre5GFlag = 1;
	}
	
	if(rateIndex == 20) /*MCS32*/
	{
		//omitted MCS32 + 20MHz
		registerOffset = cTxPwr2G4HT20_BPSK + fre5GFlag * 18;
		if(band40MFlag != 1)
		{
			em_printf(MSG_DEBUG, (char*)"rate = MCS32, bandWidth = *20M: omitted");
			*pi4TxPwrGain = 0;
    			return ERROR_RFTEST_SUCCESS;
		}
	}
	else if(rateIndex < 4)
	{
		/*1M, 2M, 5.5M, 11M  - omit channel information for this rate*/
		registerOffset = cTxPwr2G4Cck;
	}
	else
	{	
		/*4 ~ 19  -- 6M, 9M, 12M ~ 54M, MCS0 ~ MCS7*/
		registerOffset = rateRegMapping[rateIndex].i4RegIndex + fre5GFlag * 18;
		registerOffset += (rateIndex > 0x0b ? band40MFlag : 0) * 6;
	}
	registerOffset += 12; //12 is the base offset in NVRAM
	oddFlag = registerOffset%2;	//judge whether registerOffset is an odd or even number
	em_printf(MSG_DEBUG, (char*)"register offset = %d", registerOffset );
	result = readEEPRom16(registerOffset/2, &txPwrValue);
	if(result == 0)
	{
		if(oddFlag)
		{
			txPwrValue = txPwrValue & 0xff00;
			*pi4TxPwrGain = txPwrValue >> 8;
		}
		else
		{
			*pi4TxPwrGain = txPwrValue & 0x00ff;	
		}
	}
	*pi4Eirp = 0x0;
	*pi4ThermoVal = 0x0;
	return result;
	/*
	char    aucSzTmp[512]; 
    UINT_16 u2TxGroupLenWord;
    UINT_32 u4ByteOffset;
    INT_32  i4Ch;
    INT_32  i4Band;
    UINT_8  ucRateGroup;
    UINT_8  ucI;
    
    getChannelBand(channelFreq, &i4Ch, &i4Band);

    if (i4Band == 1 || i4Ch > 14) {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    ucRateGroup = 255;
    //get rate group
    for (ucI =0; ucI< NUM_SUPPORTED_RATES; ucI++) {
        if(arDataRateTable[ucI].i4BBUiCfg == rate )
        {
            ucRateGroup = arDataRateTable[ucI].ucRateGruopEep;
        }
    }
    if (ucRateGroup > EEPROM_RATE_GROUP_OFDM_48_54M) {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    u2TxGroupLenWord = EEPROM_2G_CCK_TXPWR_GAIN_END - EEPROM_2G_CCK_TXPWR_GAIN_START +1;
    u4ByteOffset = (EEPROM_2G_CCK_TXPWR_GAIN_START + \
     (ucRateGroup* u2TxGroupLenWord))*2 + (i4Ch-1)*3;

    eepromReadByteStr(u4ByteOffset, 3, aucSzTmp);
    sscanf(aucSzTmp, "%02X%02X%02X", pi4TxPwrGain, pi4Eirp, pi4ThermoVal);
	*/
	

}

/*_________________________________________________________________________
**  setPacketRx
**
**  descriptions:
**  parameters:
**         nCardIndex: NIC index number
**         fgRxEnable: Enable RX or Disable RX
**                    TRUE: enable
**                    FALSE: disable
**         nAntenna:  RX Ant
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setPacketRx(
    INT_32 condition,             /* The old definition, currently it is used to indicate ALC enable */
    RX_ANT_SEL nAntenna           /* Not used any more */
    )
{
	DEBUGFUNC("CMT6620::setPacketRx:omitted");
/*	
    UINT_32 u4Tmp;
    INT_32 fgAlcEnable = condition;


    // RX Status is enabled. 
    // 1. Make sure RX is stoped 
    SetATParam(1, 0);

    // 2. Set RX Status Bit in MCR 
    // record the initial value for RFB G0/ 1/ 2 setting
    readMCR32(0xf4, &u4Tmp);

    // OR BIT(0) | BIT(1) | BIT(2) | BIT(5) 
	writeMCR32(0xf4, 
        u4Tmp | BIT(0) | BIT(1) | BIT(2) | BIT(5));


    // ALC: We may need another param to enable the setting for user.
    // nAntenna is temperoryly used for ALC enable.
    if (fgAlcEnable) {
        SetATParam(9, TRUE);
    }
    else {
        SetATParam(9, FALSE);
    }

	// Enable RX
	SetATParam(1, 2);
 */   

	return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  setEEPromCKSUpdated
**
**  descriptions:
**  parameters:
**         void
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setEEPromCKSUpdated (void)
{
	UINT_8 	checkSum;
	UINT_16 *pu2EEPRDat;
    UINT_32 u4EepromData;
    INT_32  i4EepromSize;
  	UINT_16	eepromValue;
	INT_32 	status;
    DEBUGFUNC("CMT6620::setEEPromCKSUpdated: omitted");
/*
	status = getEEPRomSize(&i4EepromSize);
    if (status < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }
    if(i4EepromSize == ESCR_EEPROM_NOT_PRESENT ) {
        em_error("%s", "EEProm not present.\n");
        return ERROR_RFTEST_EEPROM_NOT_PRESENT;
    }
    
    pu2EEPRDat = new UINT_16[i4EepromSize/2];
    if ( pu2EEPRDat == NULL ) {
        em_error("Memory allocate fail, size %d.\n", i4EepromSize);
        return ERROR_RFTEST_GENERAL_ERROR;
    }



    //1. Get all content
	for (int i = 0 ; i < (i4EepromSize/2) ; i++) {
		readEEPRom16(i, &u4EepromData);
		pu2EEPRDat[i] = (UINT_16)u4EepromData;
	}

    //2. Calculate HIF section checksum
    if( u4ChipType == CHIP_TYPE_MP ) {
        checkSum = nicCalculateChecksumByte(&pu2EEPRDat[EEPROM_HIF_CHKSUM_START_MP],
		    (EEPROM_HIF_CHKSUM_MP - EEPROM_HIF_CHKSUM_START_MP + 1) * 2 -1);
	    eepromValue = (UINT_8)(~checkSum);
	    eepromValue = (eepromValue << 8) | (UINT_8)pu2EEPRDat[EEPROM_HIF_CHKSUM_MP];
	    writeEEPRom16(EEPROM_HIF_CHKSUM_MP, eepromValue);
    }
    else if (u4ChipType == CHIP_TYPE_MPW) {
        checkSum = nicCalculateChecksumByte(&pu2EEPRDat[EEPROM_HIF_CHKSUM_START_MPW],
		    (EEPROM_HIF_CHKSUM_MPW - EEPROM_HIF_CHKSUM_START_MPW + 1) * 2 -1);
	    eepromValue = (UINT_8)(~checkSum);
	    eepromValue = (eepromValue << 8) | (UINT_8)pu2EEPRDat[EEPROM_HIF_CHKSUM_MPW];
	    writeEEPRom16(EEPROM_HIF_CHKSUM_MPW, eepromValue);
    }

    //3. Calculate NIC setting section checksum
    checkSum = nicCalculateChecksumByte(&pu2EEPRDat[EEPROM_NIC_CHKSUM_START],
	    (EEPROM_NIC_CHKSUM - EEPROM_NIC_CHKSUM_START + 1) * 2 -1);
    eepromValue = (UINT_8)(~checkSum);
    eepromValue = (eepromValue << 8) | (UINT_8)pu2EEPRDat[EEPROM_NIC_CHKSUM];
    writeEEPRom16(EEPROM_NIC_CHKSUM, eepromValue);
  
    delete [] pu2EEPRDat;
*/
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setEEPRomSize
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          value:  size of the EEPRom (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setEEPRomSize (
    INT_32 eepromSz
    )
{
	UINT_32  value;
	UINT_32  status ;
	INT_32  iSize ;
	DEBUGFUNC("CMT6620::setEEPRomSize");
	status = getEEPRomSize(&iSize);
    /*if (status < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }*/
    if(iSize == ESCR_EEPROM_NOT_PRESENT ) {
        em_error((char*)"%s", "EEProm not present.\n");
        return ERROR_RFTEST_EEPROM_NOT_PRESENT;
    }
	
	switch(eepromSz)
	{
		case 0:
			value = ESCR_EEPROM_NOT_PRESENT;
			break;
		case 128:
			value = ESCR_EEPROM_SIZE_128_BYTES;
			break;
		case 256:
		case 512:
			value = ESCR_EEPROM_SIZE_256_512_BYTES;
			break;
		case 1024:
			value = ESCR_EEPROM_SIZE_1024_BYTES;
			break;
		case 2048:
			value = ESCR_EEPROM_SIZE_2048_BYTES;
			break;
		default:
			return ERROR_RFTEST_GENERAL_ERROR;/* illegal value */
	}

	if (writeMCR32(MCR_ECSR, value) == 0) {
		return ERROR_RFTEST_SUCCESS;
	}
	else {
		return ERROR_RFTEST_GENERAL_ERROR;
	}
}

/*_________________________________________________________________________
**  GetEEPromSize
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          value:  size of the EEPRom (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getEEPRomSize (
	INT_32* eepromSz
	)
{
	UINT_32 BytesRead;
    IPC_EEPROM_RW_STRUC_V2 m_sEEPRom_RW;

	m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_SZ;
    m_sEEPRom_RW.eepromData     = 0;   //clear before read
	DEBUGFUNC("CMT6620::getEEPRomSize");
	//check current configuration source - which one EEPROM/NVRAM is used?
	if(!m_OID->queryOID(this,
	        OID_CUSTOM_CFG_SRC_TYPE,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("getConfigurationSource failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    else {
    	/*test code
	    	char *s = (char *)&m_sEEPRom_RW.ucEEPROMMethod;
	    	for(int i = 0;i < BytesRead; i++)
	    	{
	    		em_error("s[%d] = %d\n", i, *(s + i));
	    	}
	    	*/
        if(CFG_SRC_TYPE_EEPROM == m_sEEPRom_RW.ucEEPROMMethod)
      	{
      		//EEPROM is used
      		isEepromUsed = true;
		DEBUGFUNC("CMT6620::getEEPRomSize - EEPROM exist");
      	}
      	else if(CFG_SRC_TYPE_NVRAM == m_sEEPRom_RW.ucEEPROMMethod)
      	{
      		isEepromUsed = false;
		DEBUGFUNC("CMT6620::getEEPRomSize - NVRAM exist");
    	}
    	else
    	{
    		//unknown
    		return ERROR_RFTEST_NDIS_OID_FAILURE;
    	}
    }
	m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_SZ;
    m_sEEPRom_RW.eepromData     = 0;   //clear before read
    if(!m_OID->queryOID(this,
	        isEepromUsed ? OID_IPC_EEPROM_RW : OID_IPC_NVRAM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("getEEPRomSize failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    else {
        *eepromSz = (int)m_sEEPRom_RW.eepromData;
        *eepromSz = *eepromSz > 512 ? 512 : *eepromSz;
        em_error((char*)"size = %d\n", *eepromSz);
    }
   		return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  GetEEPromSize
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          nCardIndex: Which NIC
**          value:  size of the EEPRom (size: 1 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getSpecEEPRomSize (
	INT_32* eepromSz
	)
{
	UINT_32 BytesRead;
    IPC_EEPROM_RW_STRUC_V2 m_sEEPRom_RW;

    m_sEEPRom_RW.ucEEPROMMethod = 0;
    m_sEEPRom_RW.eepromData     = 0;   //clear before read
	if(!m_OID->queryOID(this,
		        OID_CUSTOM_CFG_EEPROM_TYPE,
		        (CHAR *)&m_sEEPRom_RW,
		        sizeof(IPC_EEPROM_RW_STRUC_V2),
		        &BytesRead)
		        ) {
	        ERRORLOG("CMT6620::getSpecEEPRomSize failed");
	        return ERROR_RFTEST_NDIS_OID_FAILURE;
	    }
	    else {
			/*
			char *s = (char *)&m_sEEPRom_RW.ucEEPROMMethod;
	    	for(int i = 0;i < BytesRead; i++)
	    	{
	    		em_error("s[%d] = %d\n", i, *(s + i));
	    	}
	    	*/
	        if(CFG_EEPROM_TYPE_NO == m_sEEPRom_RW.ucEEPROMMethod)
	      	{
	      		//EEPROM does not exist
				DEBUGFUNC("CMT6620::getSpecEEPRomSize - EEPROM type = no");
				*eepromSz = 0x00;
				return ERROR_RFTEST_SUCCESS;
	      	}
	      	else if(CFG_EEPROM_TYPE_YES == m_sEEPRom_RW.ucEEPROMMethod)
	      	{
	      		//EEPROM exist
	      		isEepromUsed = true;
				DEBUGFUNC("CMT6620::getSpecEEPRomSize - EEPROM type = yes");	
	    	}
	    	else
	    	{
	    		//unknown
				DEBUGFUNC("CMT6620::getSpecEEPRomSize - EEPROM type = unknown");	
	    		return ERROR_RFTEST_NDIS_OID_FAILURE;
	    	}
	    }
		m_sEEPRom_RW.ucEEPROMMethod = EEPROM_METHOD_SZ;
    	m_sEEPRom_RW.eepromData     = 0;   //clear before read
    if(!m_OID->queryOID(this,
	        OID_IPC_EEPROM_RW,
	        (CHAR *)&m_sEEPRom_RW,
	        sizeof(IPC_EEPROM_RW_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("getSpecEEPromSize failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    else {
        *eepromSz = (int)m_sEEPRom_RW.eepromData;
        *eepromSz = *eepromSz > 512 ? 512 : *eepromSz;
        em_error((char*)"size = %d\n", *eepromSz);
    }
   		return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  setMACAddrToEEProm
**
**  descriptions: Save MAC Address to EEPROM
**  parameters:
**             nCardIndex: Which adapter.
**             macAddr:    MAC address
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setMACAddrToEEProm (
    UINT_8* macAddr
    )
{
	UINT_32 eepromValue, i, offset;
	DEBUGFUNC("CMT6620::setMACAddrToEEProm: omitted");
/*
	if (!macAddr) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	offset = EEPROM_MAC_ADDR_BYTE_1_0;
	for (i = 0 ; i < 3; i++) {
		eepromValue = macAddr[i*2];
		eepromValue |= ((UINT_16)macAddr[i*2 + 1]) << 8;
		writeEEPRom16(offset, eepromValue);
		offset += 1;
	}
*/
	return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getEEPRomVersion
**
**  descriptions: Return EEPROM version
**  parameters:
**         nCardIndex: NIC index number
**         fileName: a null terminated string indicate the file to be read
**  output:
**         version: Output buffer for version
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getEEPRomVersion (
	INT_32* version
	)
{
	INT_32 ver;
	DEBUGFUNC("CMT6620::getEEPRomVersion: omitted");
	/*
	ver = this->getEEPRomVersion_();

	if(ver < 0)
        return ERROR_RFTEST_GENERAL_ERROR;

    *version = ver;
    */
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setCountryToEEProm
**
**  descriptions:
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setCountryToEEProm (
	UINT_8* country
	)
{
    /*Input "CA" A is country[1]*/
	UINT_16 u2Tmp;
	DEBUGFUNC("CMT6620::setCountryToEEProm: omitted");
	/*
    if (!country) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    u2Tmp = ((UINT_16)country[1])<<8;
    u2Tmp |= country[0];
    //Tarnsfer to 0x4143, 'A' become high byte
    if (writeEEPRom16(EEPROM_REG_DOMAIN, u2Tmp)) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }
	*/
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setPhyModeToEEProm
**
**  descriptions: Update EEPROM phy mode
**  parameters:
**             nCardIndex: Which adapter.
**             mode:       Phy. mode
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setPhyModeToEEProm (
    INT_32 mode
    )
{
	DEBUGFUNC("CMT6620::setPhyModeToEEProm: omitted");
	return ERROR_RFTEST_SUCCESS;

}

INT_32
CMT6620::setDACOffsetToEEProm (
	UINT_16 dacOffset
	)
{
	DEBUGFUNC("CMT6620::setDACOffsetToEEProm: omitted");
	return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT6620::getDACOffset (
	UINT_16* dacOffset
	)
{
	DEBUGFUNC("CMT6620::getDACOffset: omitted");
	return ERROR_RFTEST_SUCCESS;

}

/*******************************************************************************
**  setOutputPin
**
**  descriptions: This function sets the particular pin as an output one
**                and the output level
**  parameters:
**      index - card index
**      pinIndex - Pin index
**      outputLevel - output for band
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT6620::setOutputPin (
	INT_32 pinIndex,
	INT_32 outputLevel)
{
    INT_32 i4Status = ERROR_RFTEST_SUCCESS;
    UINT_32 u4Tmp;

	DEBUGFUNC("CMT6620::setOutputPin: omitted");
/*
    switch(pinIndex) {
    case IO_PIN_GPIO0:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp = (u4Tmp | SCR_GPIO0_ENABLE_OUTPUT_MODE);
        u4Tmp &= ~SCR_BT_ACT_SEL;
        if ( outputLevel ) {
            u4Tmp |= SCR_GPIO0_WDATA;
        }
        else {
            u4Tmp &= ~SCR_GPIO0_WDATA;
        }
        writeMCR32(MCR_SCR, u4Tmp);
        break;

    case IO_PIN_GPIO1:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp = (u4Tmp | SCR_GPIO1_ENABLE_OUTPUT_MODE);
        u4Tmp &= ~SCR_BTFREQ_SEL;

        if ( outputLevel ) {
            u4Tmp |= SCR_GPIO1_WDATA;
        }
        else {
            u4Tmp &= ~SCR_GPIO1_WDATA;
        }
        writeMCR32(MCR_SCR, u4Tmp);
        break;

    case IO_PIN_GPIO2:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp = (u4Tmp | SCR_GPIO2_ENABLE_OUTPUT_MODE);
        if ( outputLevel ) {
            u4Tmp |= SCR_GPIO2_WDATA;
        }
        else {
            u4Tmp &= ~SCR_GPIO2_WDATA;
        }
        writeMCR32(MCR_SCR, u4Tmp);
        break;

    case IO_PIN_OE_N:
        // Please refer to jacky for this register 
        readMCR32(MCR_IOUDR, &u4Tmp);
        u4Tmp &= ~ ( IOUDR_OE_N_PU | IOUDR_OE_N_PD );       // Set for output
        u4Tmp &= ~ ( IOUDR_BT_PRI_PU | IOUDR_BT_PRI_PD );   // Set for input
        writeMCR32(MCR_IOUDR, u4Tmp);
        //-----------------------------------------
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_OE_N_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_OE_N_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_WE_N:
        // Please refer to jacky for this register 
        readMCR32(MCR_IOUDR, &u4Tmp);
        u4Tmp &= ~ ( IOUDR_WE_N_PU | IOUDR_WE_N_PD );       // Set for output
        writeMCR32(MCR_IOUDR, u4Tmp);
        //-----------------------------------------
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_WE_N_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_WE_N_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_D4:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D4_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D4_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_D5:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D5_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D5_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;
    
    case IO_PIN_D8:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D8_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D8_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_D9:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D9_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D9_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_D10:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D10_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D10_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_D11:
        readMCR32(MCR_IOPCR, &u4Tmp);
        u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
        if ( outputLevel ) {
            u4Tmp |= IOPCR_LOOPBACK_D11_WDATA;
        }
        else {
            u4Tmp &= ~IOPCR_LOOPBACK_D11_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4Tmp);
        break;

    case IO_PIN_WLAN_ACT:
        //Disabe MAC Tx and RX
        readMCR32(MCR_RICR, &u4Tmp);
        u4Tmp &= ~(RICR_MAC_TX_EN | RICR_MAC_RX_EN |RICR_RF_SW_MODE);
        writeMCR32(MCR_RICR, u4Tmp);
        //Config MAC PTA function
        u4Tmp = 0;
        writeMCR32(MCR_BTCER0, u4Tmp);
        writeMCR32(MCR_BTCER1, u4Tmp);
        writeMCR32(MCR_BTCER2, u4Tmp);
        writeMCR32(MCR_BTCER3, u4Tmp);

        //Set 2 wire mode
        u4Tmp = PTA_BTCER1_WIRE_MODE_2WIRE;
        writeMCR32(MCR_BTCER1, u4Tmp);

        u4Tmp = PTA_BTCER0_COEXIST_EN;
        if ( outputLevel ) {
            u4Tmp &= ~PTA_BTCER0_WLAN_ACT_POL;
        }
        else {
            u4Tmp |= PTA_BTCER0_WLAN_ACT_POL;
        }
        writeMCR32(MCR_BTCER0, u4Tmp);
        break;

    case IO_PIN_LCR:
        if ( outputLevel ) {
            u4Tmp = LCR_LED_OUTPUT;
        }
        else {
            u4Tmp = 0;
            //u4Tmp = ~LCR_LED_OUTPUT;
        }
        writeMCR32(MCR_LCR, u4Tmp);
        break;

    case IO_PIN_BT_PRI_N:
    case IO_PIN_CS_N:
    case IO_PIN_D6:
    case IO_PIN_D7:
    case IO_PIN_D12:
    case IO_PIN_D13:
    case IO_PIN_D14:
    case IO_PIN_D15:
      	em_error("This IO Pin %d does not support outputmode\n", pinIndex);
        i4Status = ERROR_RFTEST_UNSUPPORTED;
        break;
    default:
      	em_error("This IO Pin %d does not support\n", pinIndex);
        i4Status = ERROR_RFTEST_UNSUPPORTED;

        break;
    }
    return i4Status;
   */ 
   return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  getInputPin
**
**  descriptions: This function set the particular pin as an input pin and
**                returns the input level.
**  parameters:
**      nCardIndex - card index
**      pinIndex - Pin index
**  result:
**      inputLevel - Input level detected by hardware
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT6620::getInputPin (
	INT_32 pinIndex,
	INT_32* pi4InputLevel)
{
    INT_32 i4Status = ERROR_RFTEST_SUCCESS;
    UINT_32 u4Tmp;

    DEBUGFUNC("CMT6620::getInputPin: omitted");
/*
    switch(pinIndex) {
    case IO_PIN_GPIO0:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp &= ~(SCR_GPIO0_ENABLE_OUTPUT_MODE);
        u4Tmp &= ~SCR_BT_ACT_SEL;
        writeMCR32(MCR_SCR, u4Tmp);
 
        readMCR32(MCR_SCR, &u4Tmp);
  	    em_printf(MSG_DEBUG, "MCR SCR return value 0x%08X\n", u4Tmp);

        if ( u4Tmp &  SCR_GPIO0_RDATA ) {
            *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
            *pi4InputLevel = PIN_LEVEL_LOW;
        }

        break;

    case IO_PIN_GPIO1:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp &= ~(SCR_GPIO1_ENABLE_OUTPUT_MODE);
        u4Tmp &= ~SCR_BTFREQ_SEL;
        writeMCR32(MCR_SCR, u4Tmp);
        readMCR32(MCR_SCR, &u4Tmp);
        if ( u4Tmp &  SCR_GPIO1_RDATA ) {
            *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
            *pi4InputLevel = PIN_LEVEL_LOW;
        }

        break;

    case IO_PIN_GPIO2:
        readMCR32(MCR_SCR, &u4Tmp);
        u4Tmp &= ~(SCR_GPIO2_ENABLE_OUTPUT_MODE);
        writeMCR32(MCR_SCR, u4Tmp);
        readMCR32(MCR_SCR, &u4Tmp);
        if ( u4Tmp &  SCR_GPIO2_RDATA ) {
            *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
            *pi4InputLevel = PIN_LEVEL_LOW;
        }

        break;

    case IO_PIN_BT_PRI_N:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_BT_PRI_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D6:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D6_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D7:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D7_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D12:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D12_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D13:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D13_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D14:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D14_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_D15:
        readMCR32(MCR_IOPCR, &u4Tmp);
        if (u4Tmp & IOPCR_LOOPBACK_TEST_EN) {

        }
        else {
            u4Tmp |= IOPCR_LOOPBACK_TEST_EN;
            writeMCR32(MCR_IOPCR, u4Tmp);
            readMCR32(MCR_IOPCR, &u4Tmp);
        }
        
        if ( u4Tmp &  IOPCR_LOOPBACK_D15_RDATA ) {
             *pi4InputLevel = PIN_LEVEL_HIGH;
        }
        else {
             *pi4InputLevel = PIN_LEVEL_LOW;
        }
        break;

    case IO_PIN_OE_N:
    case IO_PIN_WE_N:
    case IO_PIN_CS_N:
    case IO_PIN_D4:
    case IO_PIN_D5:
    case IO_PIN_D8:
    case IO_PIN_D9:
    case IO_PIN_D10:
    case IO_PIN_D11:
    case IO_PIN_WLAN_ACT:
    case IO_PIN_LCR:
      	em_error("This IO Pin %d does not support input mode\n", pinIndex);
        i4Status = ERROR_RFTEST_UNSUPPORTED;
        break;
    default:
      	em_error("This IO Pin %d does not support\n", pinIndex);
        i4Status = ERROR_RFTEST_UNSUPPORTED;

        break;
    }
  	em_printf(MSG_DEBUG, "This IO Pin %d return value %d\n", pinIndex, *pi4InputLevel);
	*/
    return i4Status;
}

/*******************************************************************************
**  testInputPin
**
**  descriptions: This function sets the particular pin as an input for further tests
**  parameters:
**      nCardIndex - card index
**      pinIndex - Pin index
**      inputLevel - intended test level
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT6620::testInputPin (
	INT_32 pinIndex,
	INT_32	inputLevel)
{
	DEBUGFUNC("CMT6620::testInputPin: omitted");
	return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  restoreIOPin
**
**  descriptions: This function retores I/O pin settings
**  parameters:
**      nCardIndex - card index
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CMT6620::restoreIOPin (
    void
    )
{
	INT_32 result = ERROR_RFTEST_SUCCESS;
	DEBUGFUNC("CMT6620::restoreIOPin: omitted");
	/*
    // Restore original IO register setting 
	result = writeMCR32(MCR_IOUDR,   u4IoRegSave[PIN_REGISTER_IOUDR]);
	if(result)
		return result;

	result = writeMCR32(MCR_IOPCR,   u4IoRegSave[PIN_REGISTER_IOPCR]);

	if(result)
		return result;

	result = writeMCR32(MCR_SCR,     u4IoRegSave[PIN_REGISTER_SCR]);

	if(result)
		return result;

	result = writeMCR32(MCR_LCR,     u4IoRegSave[PIN_REGISTER_LCR]);

    if(result)
        return result;

    //For BTCERx, need to disable first before apply new setting.
    result = writeMCR32(MCR_BTCER0, 0);
    if(result)
        return result;
    
    result = writeMCR32(MCR_BTCER1, u4IoRegSave[PIN_REGISTER_BTCER1]);
    if(result)
        return result;

    result = writeMCR32(MCR_BTCER2, u4IoRegSave[PIN_REGISTER_BTCER2]);
    if(result)
        return result;

    result = writeMCR32(MCR_BTCER3, u4IoRegSave[PIN_REGISTER_BTCER3]);
    if(result)
        return result;

    result = writeMCR32(MCR_BTCER0, u4IoRegSave[PIN_REGISTER_BTCER0]);
    if(result)
        return result;

    result = writeMCR32(MCR_RICR,  u4IoRegSave[PIN_REGISTER_RICR]);
    if(result)
        return result;
	*/
    return result;

}

//======================================================================

/*_________________________________________________________________________
**  updateALCOffset
**
**  descriptions: Update ALC Offset table according to EEPROM content
**  parameters:
**  return:
**         0, if successful negative fails
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::updateALCOffset(void)
{
	DEBUGFUNC("CMT6620::updateALCOffset: omitted");
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  findMaxPowerByteOffset
**
**  descriptions:  Given Channel, whether CCK, which turning point and band, to return EEPROM offset
**  parameters:
**              nChannel:    1 ~ 14 for 2.4G, 240 ~ 161
**              bCck:          for CCK for not, only valid when b24Gs is true
**              markIndex:
**  return:
**              -1: Failed to locate,
**              others: zero-based balue to represent turning point to describe TX Power curve
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::findMaxPowerByteOffset(INT_32 nChannel, INT_32 bCck, INT_32 b24Gs)
{
	DEBUGFUNC("CMT6620::findMaxPowerByteOffset: omitted");
    return ERROR_RFTEST_SUCCESS;
}


INT_32
CMT6620::setPPMCRAndSCR(void)
{
	DEBUGFUNC("CMT6620::setPPMCRAndSCR: omitted");
    return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT6620::setBBTxPower(INT_32 txPower)
{
	DEBUGFUNC("CMT6620::setBBTxPower: omitted");
    return ERROR_RFTEST_SUCCESS;
}

INT_32
CMT6620::setTxAnt(INT_32 antenna)
{
	DEBUGFUNC("CMT6620::setTxAnt: omitted");
    return ERROR_RFTEST_SUCCESS;
}

UINT_8
CMT6620::getTxRate(INT_32 nTxRate, INT_32 nPreamble)
{
	DEBUGFUNC("CMT6620::getTxRate: omitted");
	return ERROR_RFTEST_SUCCESS;
}

UINT_8
CMT6620::nicCalculateChecksumByte(void * startingAddr_p,UINT_32 length)
{
    UINT_32 i;
    UINT_8 * cp;
    UINT_8 result = 0;
    DEBUGFUNC("CMT6620::nicCalculateChecksumByte: omitted");
    /*
    cp = (UINT_8*) startingAddr_p;
    for (i = 0; i < length; i++, cp++) {
        result += *cp;
    }
    */
    return result;
}

/*_________________________________________________________________________
**  getEEPRomVersion_
**
**  descriptions: Return EEPROM version
**  parameters:
**  return:
**      negative, failed
**      0 or posive: version number
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getEEPRomVersion_(void)
{
    UINT_32 value;
    UINT_32 eepValue;
	DEBUGFUNC("CMT6620::getEEPRomVersion_: omitted");
	/*
    if(readMCR32(MCR_ECSR, &value) != ERROR_RFTEST_SUCCESS)
        return -1;

    if((value & ESCR_EEPROM_SIZE_MASK) == ESCR_EEPROM_NOT_PRESENT)
        return -1;

    if(readEEPRom16(EEPROM_CLK_CFG_VERSION, &eepValue) != ERROR_RFTEST_SUCCESS)
        return -1;
	
    return ((eepValue & EEPROM_LAYOUT_VERSION_MASK)>>EEPROM_LAYOUT_VERSION_OFFSET);
    */
    return -1;
}

/*_________________________________________________________________________
**  SetATParam
**
**  descriptions: set WIFI_TEST command to chip
**  parameters:
**  return:
**      check ERROR_RFTEST_*
**  note:
**  reviewed ok for 6620
**__________________________________________________________________________*/
INT_32
CMT6620::SetATParam (UINT_32 offset, UINT_32 value)
{
	MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;

	DEBUGFUNC("CMT6620::SetATParam");
	em_error((char*)"CMT6620::SetATParam(FunctionIndex = %d, FunctionData = %d)\n", offset, value);
	/*add for set set packet content (function index = 12), this function is not supported in current mt6620 firmware*/
	if(offset == 5  || 
	offset == 8 || 
	offset == 10 || 
	offset == 11 || 
	offset == 12 || 
	offset == 13 || 
	offset == 14 || 
	offset == 17 ||
	offset == 19)
	{
		em_printf(MSG_DEBUG, (char*)"CMT6620::SetATParam(FunctionIndex = %d, FunctionData = %d) - blocked.\n", offset, value);
		return ERROR_RFTEST_SUCCESS;
	}
	/*special for function index = 12 ends*/
    m_AT_cmd.funcIndex = offset;
    m_AT_cmd.funcData = value;

	if (!m_OID->setOID(this,
		OID_CUSTOM_MTK_WIFI_TEST,
        (CHAR *)&m_AT_cmd,
        sizeof(MTK_WIFI_TEST_STRUC_V2))
		) {
        ERRORLOG("SetATParam failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  GetATParam
**
**  descriptions: get WIFI_TEST command state
**  parameters:
**  return:
**      check ERROR_RFTEST_*
**  note:
**  reviewed ok for 6620
**__________________________________________________________________________*/
INT_32
CMT6620::GetATParam (UINT_32 offset, UINT_32 * value)
{
	MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;
	UINT_32 BytesRead;

	DEBUGFUNC("CMT6620::GetATParam");
	if(offset == 5  || 
	offset == 8 || 
	offset == 10 || 
	offset == 11 || 
	offset == 12 || 
	offset == 13 || 
	offset == 14 || 
	offset == 17 ||
	offset == 19)
	{
		em_printf(MSG_DEBUG, (char*)"CMT6620::GetATParam(FunctionIndex = %d)- blocked.\n", offset);
		return ERROR_RFTEST_SUCCESS;
	}
    m_AT_cmd.funcIndex = offset;
    m_AT_cmd.funcData = 0;  //clear before read

	if (!m_OID->queryOID(this,
			OID_CUSTOM_MTK_WIFI_TEST,
			(CHAR *)&m_AT_cmd,
			sizeof(MTK_WIFI_TEST_STRUC_V2),
			&BytesRead)
			) {
        ERRORLOG("GetATParam failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_AT_cmd.funcData;

	return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  setChipType
**
**  descriptions: Config Chip type from UI for EEPROM chksum calculation.
**  parameters:
**  return:
**      negative, failed
**      0 or posive: version number
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setChipType (UINT_32 u4InChipType) 
{
	DEBUGFUNC("CMT6620::setChipType:omitted");
/*
    if(u4InChipType >= CHIP_TYPE_MAX) {
        return ERROR_RFTEST_NIC_UNKNOWN_IC;
    }
    u4ChipType = u4InChipType;
  	em_printf(MSG_DEBUG, "Chip type change to %d.\n", u4ChipType);
*/
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setEEPRomFromFile
**
**  descriptions: Burn EEPRom by reading the data from file
**  parameters:
**         nCardIndex: NIC index number
**         fileName: a null terminated string indicate the file to be read
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setEEPRomFromFile(TCHAR* fileName)
{
	INT_32 iSize, status;
	INT_32 i;
	UINT_16* usEEPRDat = NULL;
	DEBUGFUNC("CMT6620::setEEPRomFromFile");
	status = getEEPRomSize(&iSize);
    if (status < 0) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }
    if(iSize == ESCR_EEPROM_NOT_PRESENT ) {
        em_error((char*)"%s", "EEProm not present.\n");
        return ERROR_RFTEST_EEPROM_NOT_PRESENT;
    }

    usEEPRDat = (UINT_16*) new char[iSize];

    INT_32 nNum = ReadEepDataFromFile(fileName,
        usEEPRDat, iSize/sizeof(UINT_16));

    if (nNum <= 0 ) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    for (i = 0; i < iSize / 2; i++) {
        if (0 !=  writeEEPRom16(i, usEEPRDat[i]) ) {
            ERRORLOG("Fail to read write");
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }

    for (i = 0; i < iSize / 2; i++) {
        UINT_32 value;
        if (0 != readEEPRom16(i, &value) ) {
            ERRORLOG("Fail to read EEPROM back");
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }

        /* Fail  */
        if (value != usEEPRDat[i]) {
            ERRORLOG("Read EEPROM value after write fails\n");
            delete [] usEEPRDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }

    if (setEEPromCKSUpdated()) {
        ERRORLOG("Fail to update NIC EEPROM checksum");
        delete [] usEEPRDat;
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setXtalFreqTrimToEEProm
**
**  descriptions: Save Xtal Frequency offet to EEPROM
**  parameters:
**             i4FreqTrim:   offset set value
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setXtalFreqTrimToEEProm (
	INT_32 i4FreqTrim
	)
{
    char    aucSzTmp[3]; 
 
    sprintf(aucSzTmp, "%02X", (unsigned int)i4FreqTrim);
    eepromWriteByteStr(EEPROM_XTAL_FREQ_TRIM_BYTE_OFFSET, 1, aucSzTmp);

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setRcpiOffsetToEEProm
**
**  descriptions: Save Per Channel RCPI offset to EEPROM
**  parameters:
**             channelFreq:    Channel frequence in kHz
**             offset:        rcpi offset -128~+127
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setRcpiOffsetToEEProm (
	INT_32 channelFreq,
	INT_32 offset
	)
{
    char    aucSzTmp[4]; 
    UINT_32 u4ByteOffset;
    INT_32  i4Ch;
    INT_32  i4Band;
    UINT_8  icOffset;
    
    getChannelBand(channelFreq, &i4Ch, &i4Band);

    if (i4Band == 1 || i4Ch > 14) {
        return ERROR_RFTEST_UNSUPPORTED;
    }

    icOffset = 0 + (UINT_8) offset;
    /*get rate group*/
    u4ByteOffset = (EEPROM_2G_RCPI_OFFSET_TABLE_START )*2 + (i4Ch-1)*1;
 
    sprintf(aucSzTmp, "%02X", (UINT_8)icOffset);
    eepromWriteByteStr(u4ByteOffset, 1, aucSzTmp);

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setXtalTrimToCr
**
**  descriptions: Set Xtal Trim value to RFCR
**  parameters:
**             u4Value:       frequency trim value
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setXtalTrimToCr (
    UINT_32 u4Value
	)
{
    UINT_32 u4RFCR;
    UINT_32 u4Tmp;
	
    readMCR32(RFCR_CR4, &u4RFCR);
    u4Tmp = (u4RFCR& ~RFCR4_XO_TRIM_MASK) | ((u4Value<<RFCR4_XO_TRIM_OFFSET) & RFCR4_XO_TRIM_MASK );
    writeMCR32(RFCR_CR4, u4Tmp);
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getXtalTrimToCr
**
**  descriptions: Get Xtal Trim value from RFCR
**  parameters:
**             pu4Value:       frequency trim value pointer
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/

INT_32 
CMT6620::getXtalTrimToCr (
	UINT_32 *pu4Value
	)
{
   INT_32 ret;
   UINT_32 u4RFCR;	 
	
   ret = readMCR32(RFCR_CR4, &u4RFCR);
   *pu4Value = (u4RFCR & RFCR4_XO_TRIM_MASK) >> RFCR4_XO_TRIM_OFFSET;	

   return ret;
}
/*_________________________________________________________________________
**  queryThermoInfo
**
**  descriptions: Query Adapter's thermo information
**  parameters:
**             pi4Enable:       0: thermo disable. 1: thermo enable
**             pu4RawVal:       the thermo raw data, 0xFFFFFFFF hardward busy.
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::queryThermoInfo (
    INT_32 *  pi4Enable,
    UINT_32 * pu4RawVal
	)
{
    UINT_32 u4ALCR;
    INT_32  i4Enable;
    UINT_32 u4RawVal;
    DEBUGFUNC("CMT6620::queryThermoInfo:omitted");
    /*
    readMCR32(MCR_ALCR0, &u4ALCR);
    if (u4ALCR & ALCR_ALC_CALCULATION_EN ) {
        i4Enable = 1;
   
        for( int i=0; i<100; i++) {
            readMCR32(MCR_ALCR0, &u4ALCR);
            if(u4ALCR&ALCR_ALC_BUSY) {
                sleep(1);
                if(i==99) {
                    u4RawVal=0xFFFFFFFF;
                }
            }
            else{
                u4RawVal=u4ALCR&ALCR_ALC_RAW_VALUE_MASK;
                break;
            }
        }
        

    }
    else {
        i4Enable = 0;
        u4RawVal = 0;
    }
	
    if (pi4Enable != NULL) {
        *pi4Enable = i4Enable;
    }
        
    if (pu4RawVal!=NULL) {
        *pu4RawVal=u4RawVal;
    }
    */
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  setThermoEn
**
**  descriptions: Set Adapter's thermo function enable/disable
**  parameters:
**             i4Enable:       0: thermo disable. 1: thermo enable
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::setThermoEn (
    INT_32    i4Enable
	)
{
    UINT_32 u4ALCR;
    DEBUGFUNC("CMT6620::setThermoEn:omitted");
/*
    readMCR32(MCR_ALCR0, &u4ALCR);
    if (i4Enable == 1 ) {
       u4ALCR |= ALCR_ALC_CALCULATION_EN;
    }
    else {
       u4ALCR &= ~ALCR_ALC_CALCULATION_EN;
    }
    writeMCR32(MCR_ALCR0, u4ALCR);
*/
    return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  setAnritsu8860bTestSupportEn
**
**  descriptions: Set Adapter to enable/disable Anritsu test support
**  parameters:
**             i4Enable:       0: disable. 1: enable
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**        When enable, it will disable roaming and "No Ack when overflow".
**        When disable, it will restore original raoming and NoAck setting.
**__________________________________________________________________________*/
INT_32
CMT6620::setAnritsu8860bTestSupportEn (
    INT_32    i4Enable
	)
{
    
    UINT_32 u4RFCR;
    UINT_32 u4Enable;
    UINT_32 u4BytesRead;
	DEBUGFUNC("CMT6620::setAnritsu8860bTestSupportEn:omitted");
	/*
    if (i4Enable == 0) {
        //disable support.
        if(fgAnritsuSupportEn == TRUE) {
            u4Enable = u4OriRoamingEn;
            if (!m_OID->setOID(this,
		        OID_CUSTOM_ROAMING_EN,
                (CHAR *)&u4Enable,
                sizeof(u4Enable))
		        ) {
                ERRORLOG("Failed to set OID_CUSTOM_ROAMING_EN");
                return ERROR_RFTEST_NDIS_OID_FAILURE;
            }
            u4RFCR = u4OriRFCR;
            writeMCR32(MCR_RFCR, u4RFCR);

        }

        fgAnritsuSupportEn = FALSE;
    }
    else if (i4Enable == 1) {
        if(fgAnritsuSupportEn==TRUE) {
            return  ERROR_RFTEST_SUCCESS;
        }
        //enable support.
        if (!m_OID->queryOID(this,
			OID_CUSTOM_ROAMING_EN,
			(CHAR *)&u4Enable,
			sizeof(u4Enable),
			&u4BytesRead)
			) {
            ERRORLOG("Failed to read OID_CUSTOM_ROAMING_EN ");
            return ERROR_RFTEST_NDIS_OID_FAILURE;
        }
	    
        u4OriRoamingEn = u4Enable;
        if (u4Enable == 1) {
            u4Enable = 0;
	        if (!m_OID->setOID(this,
		        OID_CUSTOM_ROAMING_EN,
                (CHAR *)&u4Enable,
                sizeof(u4Enable))
		        ) {
                ERRORLOG("Failed to set OID_CUSTOM_ROAMING_EN");
                return ERROR_RFTEST_NDIS_OID_FAILURE;
            }
        }
        readMCR32(MCR_RFCR, &u4RFCR);
        u4OriRFCR = u4RFCR;
        if(u4RFCR & RFCR_RX_NOACK_CTRL)  {
            u4RFCR &= (~RFCR_RX_NOACK_CTRL);
            writeMCR32(MCR_RFCR, u4RFCR);
        }
        fgAnritsuSupportEn = TRUE;

    }
    else {

        return ERROR_RFTEST_UNSUPPORTED;

    }
	*/
    return ERROR_RFTEST_SUCCESS;
}

/*_________________________________________________________________________
**  getDPDLength
**
**  descriptions: check EE_Type[1:0] in offset 0x14 to get the size of EEPRom
**  parameters:
**          dpdLength: length of DPD parameters
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::getDPDLength (
	INT_32* dpdLength
	)
{
	UINT_32 BytesRead;
	MTK_WIFI_TEST_STRUC_V2 m_sDPDLength_R;
	DEBUGFUNC("CMT6620::getDPDLength");
	
	m_sDPDLength_R.funcIndex = 36;	//result information
	m_sDPDLength_R.funcData = 39;	//DPD calibration result
	
    if(!m_OID->queryOID(this,
	        OID_CUSTOM_MTK_WIFI_TEST,
	        (CHAR *)&m_sDPDLength_R,
	        sizeof(MTK_WIFI_TEST_STRUC_V2),
	        &BytesRead)
	        ) {
        ERRORLOG("getDPDLength failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    else {
        *dpdLength = (int)m_sDPDLength_R.funcData;
		return ERROR_RFTEST_SUCCESS;
    }
    return ERROR_RFTEST_SUCCESS;
}

/************************************************************************
*                          F U N C T I O N S
*************************************************************************
*/
/*_________________________________________________________________________
**  readDPD32
**
**  descriptions: read 32-bit data from DPD parameters
**  parameters:
**          offset: address offset of the DPD
**  output:
**          value:  value read from the DPD parameters (size: 4 Byte)
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::readDPD32 (UINT_32 offset, UINT_32 * value)
{

	MTK_WIFI_TEST_STRUC_V2 m_sDPD_Parameter;
	UINT_32 BytesRead;

	DEBUGFUNC("CMT6620::readDPD32");

   	m_sDPD_Parameter.funcIndex = 39;	//DPD calibration result
	m_sDPD_Parameter.funcData = offset;

	if (!m_OID->queryOID(this,
			OID_CUSTOM_MTK_WIFI_TEST,
			(CHAR *)&m_sDPD_Parameter,
			sizeof(MTK_WIFI_TEST_STRUC_V2),
			&BytesRead)
			) {
        ERRORLOG("readDPD32 failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	*value = m_sDPD_Parameter.funcData;
	return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  writeDPD32
**
**  descriptions: write 32-bit data to DPD
**  parameters:
**          offset: address offset of the DPD
**          value:  value set to the DPD 
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::writeDPD32 (UINT_32 offset, UINT_32 value)
{
	MTK_WIFI_TEST_STRUC_V2 m_sDPD_Parameter;
	UINT_32 BytesRead;

	DEBUGFUNC("CMT6620::writeDPD32");

   	m_sDPD_Parameter.funcIndex = 64 | (offset << 16);	//DPD calibration result
	m_sDPD_Parameter.funcData = value;

	if (!m_OID->setOID(this,
		OID_CUSTOM_MTK_WIFI_TEST,
        (CHAR *)&m_sDPD_Parameter,
        sizeof(MTK_WIFI_TEST_STRUC_V2))
		) {
        ERRORLOG("writeDPD32 failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	return ERROR_RFTEST_SUCCESS;
}

INT_32 
CMT6620::setDPDFromFile(TCHAR* fileName)
{
	
	INT_32 i;
	UINT_32* u4DPDDat = NULL;
	INT_32 iSize, status;
	status = getDPDLength(&iSize);
    if (status < 0) {
        ERRORLOG("getDPDLength failed");
        return ERROR_RFTEST_GENERAL_ERROR;
    }
    
    u4DPDDat = (UINT_32*) new UINT_32[(iSize + 3) / 4];//round by 4, because each time we read/write DPD parameters, there are [4 bytes] in each operation, but the iSize unit is [byte]

    INT_32 nNum = ReadDPDParaFromFile(fileName,
        u4DPDDat, (iSize + 3) / 4);

    if (nNum <= 0 ) {
        ERRORLOG("ReadDPDParaFromFile failed");
        return ERROR_RFTEST_GENERAL_ERROR;
    }
	em_printf(MSG_DEBUG, (char*)"file line = %d, DPD length = %x", nNum, iSize);	
    for (i = 0; ((i << 2) < iSize) && (i < nNum); i ++) {
    	
    	em_printf(MSG_DEBUG, (char*)"offset = %d, value in file = %x", i << 2, u4DPDDat[i]);
        if ( writeDPD32(4 * i, u4DPDDat[i]) != 0) {
            ERRORLOG("Fail to read write DPD parameters");
            delete []u4DPDDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }

    for (i = 0; ((i << 2) < iSize) && (i < nNum); i++) {
        UINT_32 value = 0;
        if (readDPD32((i << 2), &value) != 0) {
            ERRORLOG("Fail to read DPD back");
            delete [] u4DPDDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
		em_printf(MSG_DEBUG, (char*)"offset = %d, value in file = %x, DPD in device = %x", i << 2, u4DPDDat[i] ,value );	

        
        if (value != u4DPDDat[i]) {
			/* check failed  */
            em_printf(MSG_DEBUG, (char*)"verify DPD parameters after write fails:offset = %d\n", i);
            delete [] u4DPDDat;
            return ERROR_RFTEST_GENERAL_ERROR;
        }
    }


    return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  setPacketTxEx with ALC information
**
**  descriptions: Construct packet for transmission with ALC information
**  parameters:
**              nCardIndex: Which NIC
**              szBuf,          Tx packet content
**              bufSize,        Tx packet length
**              bLongPreamble,  TRUE if long preamble is used, FALSE otherwise. Only valid for CCK rates
**              txRate,         transmitting rate for Tx packet
**              pktCount,       Amount of Tx pakcets to send out
**              pktInterval,    Interval delay between each Tx packet
**              bGainControl,   whether gain control value is provided
**              gainControl,    RF gain control value
**              bTrackAlc,      Whether to track ALC
**              bTargetAlc,     Whether target ALC is provided
**              targetAlcValue, target ALC value
**              txAntenna,      Transmission Antenna Selection
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
#if 0
INT_32
CMT6620::setPacketTxEx (
	UINT_8 * szBuf,
	INT_32 bufSize,                             
	INT_32 bLongPreamble,                       
	DOUBLE txRate,                              
	INT_32 pktCount,                             
	INT_32 pktInterval,                           
	INT_32 bGainControl,
	INT_32 gainControl,            
	INT_32 bTrackAlc,
    INT_32 bTargetAlc,
    INT_32 targetAlcValue,
    INT_32 txAntenna
	)
{
    /*
        だ
        1. continuous TX
        2. TX output power
        3. carrier suppression
        4. local leakage
        5. Enter Power OFF

    */

	
    INT_32 i;
    UINT_32 u4NewRateIdx = 0, u4TxGain = 0;
	DEBUGFUNC("CMT6620::setPacketTxEx:omitted");
/*
	DEBUGFUNC("CMT6620::setPacketTxEx");
	INITLOG((_T("\n")));

    if (bufSize == 0) { // help to read packet content from file
        return ERROR_RFTEST_GENERAL_ERROR;
    }


    for (i = 0 ; i < bufSize; i=i+2) {
        UINT_32 u4PacketContent = 0;

        u4PacketContent |= ((CHAR)szBuf[i] & 0xFF) << 24;

        if ((i+1) < bufSize) {
            u4PacketContent |= ((CHAR)szBuf[i+1] & 0xFF) << 16;
        }

        u4PacketContent |= (UINT_16)(i & 0xFFFF);

        if (SetATParam(12, u4PacketContent) != ERROR_RFTEST_SUCCESS ) {
            ERRORLOG((TEXT("Failed to set packets content\n")));
            break;
        }
    }

    // Length
    if (SetATParam(6, (UINT_32)bufSize) != ERROR_RFTEST_SUCCESS ) {
        ERRORLOG((TEXT("Failed to set packet length\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // Count
    if (SetATParam(7, (UINT_32)pktCount) != ERROR_RFTEST_SUCCESS ) {
        ERRORLOG((TEXT("Failed to set packet count\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // interval
    if (SetATParam(8, (UINT_32)pktInterval) != ERROR_RFTEST_SUCCESS ) {
        ERRORLOG((TEXT("Failed to set packet interval\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // Antenna
    if (SetATParam(5, (UINT_32)txAntenna) != ERROR_RFTEST_SUCCESS ) {  //For compability reason
        ERRORLOG((TEXT("Failed to set Tx antenna\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // INT_32 bTargetAlc, not used.    
#if 1
    // TX Gain
    if (!bGainControl) {
        switch ((UINT_32)txRate) {
            case 0: // 1M
            case 1: // 2M
            case 2: // 5.5M
            case 3: // 11M
                u4TxGain = 0x2E;
                break;
            case 4: // 6M
            case 5: // 9M
            case 8: // 24M
            case 9: // 36M
            case 10:// 48M
            case 11:// 54M
                u4TxGain = 0x33;
                break;
            case 6: // 12M
            case 7: // 18M
                u4TxGain = 0x32;
                break;
            default:
                u4TxGain = 0x33;
                break;
        }
    }
    else {
        u4TxGain = gainControl;
    }

    // Rate
    // TODO: Find the original rate index. We may need to do some mapping.    
    if (SetATParam(3, (UINT_32)txRate) != ERROR_RFTEST_SUCCESS ) {
        ERRORLOG((TEXT("Failed to set Tx rate\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
#else
    // TX Gain            
    switch ((UINT_32)txRate) {    
        case 0x02: // 1M
            u4NewRateIdx = 0;
            u4TxGain = 0x2E;
            break;
        case 0x04: // 2M
            u4NewRateIdx = 1;
            u4TxGain = 0x2E;
            break;
        case 0x0B: // 5.5M
            u4NewRateIdx = 2;
            u4TxGain = 0x2E;
            break;
        case 0x16: // 11M
            u4NewRateIdx = 3;
            u4TxGain = 0x2E;
            break;
        case 0x0C: // 6M
            u4NewRateIdx = 4;
            u4TxGain = 0x33;
            break;
        case 0x12: // 9M
            u4NewRateIdx = 5;
            u4TxGain = 0x33;
            break;
        case 0x30: // 24M
            u4NewRateIdx = 8;
            u4TxGain = 0x33;
            break;
        case 0x48: // 36M
            u4NewRateIdx = 9;
            u4TxGain = 0x33;
            break;
        case 0x60:// 48M
            u4NewRateIdx = 10;
            u4TxGain = 0x33;
            break;
        case 0x6C:// 54M
            u4NewRateIdx = 11;
            u4TxGain = 0x33;
            break;
        case 0x18: // 12M
            u4NewRateIdx = 6;
            u4TxGain = 0x32;
            break;
        case 0x24: // 18M
            u4NewRateIdx = 7;
            u4TxGain = 0x32;
            break;
        default:
            u4TxGain = 0x33;
            break;        
    }

    if (bGainControl) {
        u4TxGain = gainControl;
    }

    // Rate
    // TODO: Find the original rate index. We may need to do some mapping.    
    if (SetATParam(3, u4NewRateIdx) != ERROR_RFTEST_SUCCESS ) {
        ERRORLOG((TEXT("Failed to set Tx rate\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

#endif
        
    if (SetATParam(2, u4TxGain) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set Tx gain\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    
    // Preamble
    if (SetATParam(4, (UINT_32)bLongPreamble) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set Preamble\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // ALC
    if (SetATParam(9, (UINT_32)bTrackAlc) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set Preamble\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    // TXOP
    if (SetATParam(10, 0x00020000) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set TXOP\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
    
    // Retrylimit
    if (SetATParam(13, 1) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set Retry limit\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
		
    // Queue  Default Queue is from AC1
    if (SetATParam(14, 2) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set AC Queue 1\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

	// Enable TX
    if (SetATParam(1, 1) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG((TEXT("Failed to set TX\n")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
*/
    return 0;
}
#endif

INT_32
CMT6620::testLoopD4toD15 (
    UINT_32 u4TestInput,
    BOOL *pfgResult
    )
{
     
    UINT_32 u4Tmp, u4ExpectResult = 0;
	DEBUGFUNC("CMT6620::testLoopD4toD15:omitted");
/*
    // Enable Loopback test bit 
    readMCR32(MCR_IOPCR, &u4Tmp);
    if ((u4Tmp & IOPCR_LOOPBACK_TEST_EN) == 0) {
        // Loopback mode is not enabled.
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    u4TestInput &= 0x7F;    // We only test 7 pins. They are OE_N, D4, D5, D8, D9, D10, D11 
    u4ExpectResult = (u4TestInput >> 1);
    
    if (u4TestInput & 0x1) {
        // OE_N high, BT_PRI should be expected to be high.
        u4ExpectResult |= 0x40;
    }

    u4TestInput <<= 17;
    u4ExpectResult <<=24;

    // Clear old result 
    u4Tmp &= ~IOPCR_LOOPBACK_WDATA_MASK;
    // Set new output value 
    u4Tmp |= u4TestInput;

    // Write to MCR 
    writeMCR32(MCR_IOPCR, u4Tmp);

    // Read from MCR 
    readMCR32(MCR_IOPCR, &u4Tmp); 

    if ((u4Tmp & BITS(24, 30)) != u4ExpectResult) {
        *pfgResult = FALSE;
    }
    else {
        *pfgResult = TRUE;
    }
*/
    return ERROR_RFTEST_SUCCESS;
}


/*_________________________________________________________________________
**  testIOPin
**
**  descriptions: Test the loopback pin for different module
**  parameters:
**             u4ModuleIndex - 1: WiFi-Module, 0: Co-Module
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CMT6620::testIOPin (
    UINT_32 u4ModuleIndex
	)
{
    UINT_32 u4OriIOPCR, u4OriSCR;
    UINT_32 u4Tmp, u4Idx, u4LastResult;
    INT_32 i4Status = ERROR_RFTEST_SUCCESS;
    DEBUGFUNC("CMT6620::testLoopD4toD15:omitted");
/*
    // Store the original SCR value 
    readMCR32(MCR_SCR, &u4OriSCR);

    //---------INT_N Test Start----
#if 0
    // Disable Global Interrupt 
    readMCR32(MCR_HLPCR, &u4Tmp);
    u4Tmp |= HLPCR_CLR_GLOBAL_INT_EN;
    writeMCR32(MCR_HLPCR, u4Tmp);    

    // Set GPIO_0 to GPIO 0 function     
    u4Tmp = u4OriSCR;
    // Clear Enable bit for Output Mode 
    u4Tmp &= ~(SCR_GPIO0_ENABLE_OUTPUT_MODE);
    // Clear this bit for GPIO function 
    u4Tmp &= ~SCR_BT_ACT_SEL;
    writeMCR32(MCR_SCR, u4Tmp);
#endif
    if (SetATParam(18, 0) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG("Failed to set INT_N pin\n");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }


    if (GetATParam(18, &u4Tmp) != ERROR_RFTEST_SUCCESS) {
        ERRORLOG("Failed to get INT_N pin\n");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }    

    if (u4Tmp == 0) {
        i4Status = ERROR_RFTEST_GENERAL_ERROR;
    }

#if 0
    // Enable Global Interrupt 
    readMCR32(MCR_HLPCR, &u4Tmp);
    u4Tmp |= HLPCR_SET_GLOBAL_INT_EN;
    writeMCR32(MCR_HLPCR, u4Tmp);
#endif
    //---------OSC_EN Test Start----

     Set GPIO_2 to GPIO 2 function    
    readMCR32(MCR_SCR, &u4Tmp);
    // Clear Enable bit for Output Mode 
    u4Tmp &= ~(SCR_GPIO2_ENABLE_OUTPUT_MODE);
    writeMCR32(MCR_SCR, u4Tmp);

    // Read the GPIO_2 Data 
    readMCR32(MCR_SCR, &u4Tmp);

    if ((u4Tmp &  SCR_GPIO2_RDATA) == 0) {
        // OSC_EN is always high,
         //  Error when this bit is not set.
        
        i4Status = ERROR_RFTEST_GENERAL_ERROR;
    }    

    if (u4ModuleIndex == 1) {
        // This is Co-Module index, skip the test below. 

        //Restore original SCR value 
        writeMCR32(MCR_SCR, u4OriSCR);

        return i4Status;
    }

    //====================================

    // Store the original IOPCR value 
    readMCR32(MCR_IOPCR, &u4OriIOPCR);

    // Enable Loopback test bit 
    u4Tmp = u4OriIOPCR;
    u4Tmp |= IOPCR_LOOPBACK_TEST_EN;  
    writeMCR32(MCR_IOPCR, u4Tmp);

#if 1

    for (u4Idx = 0; u4Idx < 8; u4Idx++) {
        BOOL fgResult;
        UINT_32 u4TestPattern;

        u4TestPattern = BIT(u4Idx);

        i4Status = testLoopD4toD15(u4TestPattern, &fgResult);

        if (fgResult == FALSE) {
            i4Status = ERROR_RFTEST_GENERAL_ERROR;
        }

        if (i4Status == ERROR_RFTEST_GENERAL_ERROR) {
            // Stop the test to prevent i4Status been set to SUCCESS again 
            break;
        }
    }


#else
    //---------OE_N -> BT_PRI Test Start----    
    for (u4Idx = 0; u4Idx < 2; u4Idx++) {
        readMCR32(MCR_IOPCR, &u4LastResult);
        if (u4Idx == 0) {
            u4LastResult |= IOPCR_LOOPBACK_OE_N_WDATA;
        }
        else {
            u4LastResult &= ~IOPCR_LOOPBACK_OE_N_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4LastResult);

        readMCR32(MCR_IOPCR, &u4Tmp);    
        if ((u4LastResult ^ u4Tmp) != IOPCR_LOOPBACK_BT_PRI_RDATA) {
            i4Status = ERROR_RFTEST_GENERAL_ERROR;
        }
    }

#if 1
    for (u4Idx = 0; u4Idx < 2; u4Idx++) {
        UINT_32 u4ExpectResult = 0;

        readMCR32(MCR_IOPCR, &u4LastResult);
        u4LastResult &= ~IOPCR_LOOPBACK_WDATA_MASK;
        if (u4Idx == 0) {
            u4LastResult |= 0xA80000;
            u4ExpectResult = 0x2A000000;
        }
        else {
            u4LastResult |= 0x540000;
            u4ExpectResult = 0x15000000;
        }
        writeMCR32(MCR_IOPCR, u4LastResult);
    
        readMCR32(MCR_IOPCR, &u4Tmp);    
        if ((u4Tmp & BITS(24, 29)) != u4ExpectResult) {
            i4Status = ERROR_RFTEST_GENERAL_ERROR;
        }

    }
#else
    for (u4Idx = 0; u4Idx < 6; u4Idx++) {
        UINT_32 u4CurrWPin, u4CurrRPin, u4Idx_2;

        u4CurrWPin = IOPCR_LOOPBACK_D4_WDATA;        
        u4CurrRPin = IOPCR_LOOPBACK_D6_RDATA;

        u4CurrWPin <<= u4Idx;
        u4CurrRPin <<= u4Idx;

        for (u4Idx_2 = 0; u4Idx_2 < 2; u4Idx_2++) {
            readMCR32(MCR_IOPCR, &u4LastResult);
            if (u4Idx_2 == 0) {
                // Output level high when u4Idx_2 == 0 
                u4LastResult |= u4CurrWPin;
            }
            else {
                // Output level low when u4Idx_2 == 1 
                u4LastResult &= ~u4CurrWPin;
            }
            writeMCR32(MCR_IOPCR, u4LastResult);

            readMCR32(MCR_IOPCR, &u4Tmp);    
            if ((u4LastResult ^ u4Tmp) != u4CurrRPin) {
                i4Status = ERROR_RFTEST_GENERAL_ERROR;
            }
        }
    }
#endif
#endif

    // Set GPIO_1 to GPIO 1 function     
    readMCR32(MCR_SCR, &u4Tmp);
    // Clear Enable bit for Output Mode 
    u4Tmp &= ~(SCR_GPIO1_ENABLE_OUTPUT_MODE);
    u4Tmp &= ~SCR_BTFREQ_SEL;
    writeMCR32(MCR_SCR, u4Tmp);

    //---------WE_N -> GPIO1 Test Start----    
    for (u4Idx = 0; u4Idx < 2; u4Idx++) {
        readMCR32(MCR_IOPCR, &u4LastResult);
        if (u4Idx == 0) {
            u4LastResult |= IOPCR_LOOPBACK_WE_N_WDATA;
        }
        else {
            u4LastResult &= ~IOPCR_LOOPBACK_WE_N_WDATA;
        }
        writeMCR32(MCR_IOPCR, u4LastResult);

        readMCR32(MCR_SCR, &u4Tmp);
        if ( u4Tmp &  SCR_GPIO1_RDATA ) {
            u4Tmp = 1;
        }
        else {
            u4Tmp = 0;
        }

        if (u4Idx == u4Tmp) {
            i4Status = ERROR_RFTEST_GENERAL_ERROR;
        }        
    }


    // Restore original IOPCR value 
    writeMCR32(MCR_IOPCR, u4OriIOPCR);

    // Restore original SCR value 
    writeMCR32(MCR_SCR, u4OriSCR);
*/
	return i4Status;
}


INT_32 
CMT6620::getChannelList (
    UINT_32 * frenqBuf, 
    INT_32 maxNum)
{
    INT_32 i;
    const UINT_32 chnlList[] = {2412000,
                                2417000,
                                2422000,
                                2427000,                    
                                2432000,
                                2437000,
                                2442000,
                                2447000,                    
                                2452000,
                                2457000,                    
                                2462000,
                                2467000,
                                2472000,                    
                                2484000
                                };
    

    for (i=0; i < maxNum; i++) {
        if (i > 13) {
            frenqBuf[i] = 0;
        }
        else {
            frenqBuf[i] = chnlList[i];
        }
    }


    return 0;
}

/*added by mtk80758*/
/*
INT_32
CMT6620::setTxPower(INT_32 txPower)
{
	MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;

	DEBUGFUNC("CMT6620::SetATParam");
	
	if(txPower > 127)
	{
		txPower = 127;
	}
	
	if(txPower< -128)
	{
		txPower = -128;
	}
	
	em_error("CMT6620::setTxPower(txPower %d)\n", txPower);
    m_AT_cmd.funcIndex = 2;		//set Tx gain of RF
    m_AT_cmd.funcData = (txPower  + 128) * 2;

	if (!m_OID->setOID(this,
		OID_CUSTOM_MTK_WIFI_TEST,
        (CHAR *)&m_AT_cmd,
        sizeof(MTK_WIFI_TEST_STRUC_V2))
		) {
        ERRORLOG("setTxPower failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	return ERROR_RFTEST_SUCCESS;
}


INT_32
CMT6620::setPacketLength(INT_32 pckLength)
{
	MTK_WIFI_TEST_STRUC_V2 m_AT_cmd;

	DEBUGFUNC("CMT6620::setPacketLength");
	
	em_error("CMT6620::setPacketLength(pckLength %d)\n", pckLength);
    m_AT_cmd.funcIndex = 6;			//Packet Length (MPDU) in bytes
    m_AT_cmd.funcData = pckLength; 	//do we need to limit max length of MPDU?

	if (!m_OID->setOID(this,
		OID_CUSTOM_MTK_WIFI_TEST,
        (CHAR *)&m_AT_cmd,
        sizeof(MTK_WIFI_TEST_STRUC_V2))
		) {
        ERRORLOG("setPacketLength failed");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	return ERROR_RFTEST_SUCCESS;
}
*/

}
