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
** $Id: WiFi_EM_API.cpp,v 1.0 2010/05/07 14:49:33 MTK80743 $
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
** $Log: WiFi_EM_API.cpp,v $
 *
 * 11 16 2012 ben.niu
 * Add CTIA test setting/getting

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
#include "WiFi_EM_API.h"
#include "type.h"
#include "dbg.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

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

namespace android {

#ifndef __ARDATARATETABLE__
#define __ARDATARATETABLE__
	DATA_RATE_ENTRY_STRUCT_T arDataRateTable[] = {
	    {(char*)"1M",      2,      0x00,  EEPROM_RATE_GROUP_CCK,         TRUE},
	    {(char*)"2M",      4,      0x01,  EEPROM_RATE_GROUP_CCK,         TRUE},
	    {(char*)"5.5M",    11,     0x02,  EEPROM_RATE_GROUP_CCK,         TRUE},
	    {(char*)"11M",     22,     0x03,  EEPROM_RATE_GROUP_CCK,         TRUE},
	    {(char*)"6M",      12,     0x0B,  EEPROM_RATE_GROUP_OFDM_6_9M,   FALSE},
	    {(char*)"9M",      18,     0x0F,  EEPROM_RATE_GROUP_OFDM_6_9M,   FALSE},
	    {(char*)"12M",     24,     0x0A,  EEPROM_RATE_GROUP_OFDM_12_18M, FALSE},
	    {(char*)"18M",     36,     0x0E,  EEPROM_RATE_GROUP_OFDM_12_18M, FALSE},
	    {(char*)"24M",     48,     0x09,  EEPROM_RATE_GROUP_OFDM_24_36M, FALSE},
	    {(char*)"36M",     72,     0x0D,  EEPROM_RATE_GROUP_OFDM_24_36M, FALSE},
	    {(char*)"48M",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_48_54M, FALSE},
	    {(char*)"54M",     108,    0x0C,  EEPROM_RATE_GROUP_OFDM_48_54M, FALSE},
	    //added by mtk80758, not compatiable, need to confirm with Cp.Wu
	    {(char*)"MCS0",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS1",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS2",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS3",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS4",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS5",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS6",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS7",     96,     0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE},
	    {(char*)"MCS32",    96,    0x08,  EEPROM_RATE_GROUP_OFDM_MCS, FALSE}
	};
#endif	

CAdapter *CWrapper::m_adapter = NULL;
COID  *CWrapper::m_oid = NULL;
char    CWrapper::m_wrapperID[NAMESIZE];
UINT_32 CWrapper::ChipID = 0;
CWrapper::CWrapper()
{
#if 0
	char *ifname = "wlan0";
	wstring name, desc;
	string str_desc;

	m_wrapperID = wrapperID;	
	if(m_wrapperID == "Local" | m_wrapperID == "local" | m_wrapperID == "LOCAL")
		m_oid = new CLocalOID(ifname);
	else{
		perror("CWrapper wrong wrapperID");
		return;
	}
	
	m_oid.GetChipID(ChipID, ifname);
	name = string(ifname).s2ws();
	str_desc = string.format("0:X0000", ChipID & DEVID_IDMSK);
	desc = str_desc.s2ws();
	em_printf(MSG_DEBUG, (char*)"CWrapper name is %s  desc is %s", name.ws2s().c_str(), desc.ws2s().c_str());
#endif


}

CWrapper::~CWrapper(void)
{

}


INT_32 
CWrapper::Initialize(const char * wrapperID)
{
		const char *ifname = "wlan0";
		char desc[NAMESIZE];
		INT_32 	ret = ERROR_RFTEST_GENERAL_ERROR;	

		assert(wrapperID);
		strcpy(m_wrapperID, wrapperID);
		
		if(!strcmp(m_wrapperID, "Local")||!strcmp(m_wrapperID, "local")||!strcmp(m_wrapperID, "LOCAL"))
			m_oid = new CLocalOID(ifname);
		else{
			em_error((char*)"CWrapper wrong wrapperID");
			goto out;
		}
		
		/*//added by MTK80758, CWrapperID now identifies CHIP ID MT5921 or MT6620
		if(strcmp(m_wrapperID, "MT5921"))
		{
			m_oid = new CLocalOID(ifname, "MT5921");
		}else if(strcmp(m_wrapperID, "MT6620"))
		{
			m_oid = new CLocalOID(ifname, "MT6620");
		}
		else
		{
			em_error("CWrapper wrong wrapperID");
			goto out;
		}
		*/	
		m_oid->GetChipID(ChipID, ifname);
		sprintf(desc, "%x", ChipID & DEVID_IDMSK);
		em_printf(MSG_DEBUG, (char*)"CWrapper name is %s desc is %s\n", ifname, desc);
		
		switch(ChipID & DEVID_IDMSK)
		{
#if 0
			case 0x5901:
			case 0x5911: 
				m_adapter= new CMT5911(name, desc, m_oid, this);
				break;
			
			case 5:
			case 0x5912:	
				m_adapter = new CMT5912(name, desc, m_oid, this);
				break;
	
			case 0x5905: 
				m_adapter = new CMT5905(name, desc, m_oid, this);
				break;
#endif			
			case 0x5921: 
				ChipID = 0x5921;
				em_printf(MSG_DEBUG, (char*)"Chip ID = 0x%x", ChipID);
				m_adapter = new CMT5921(ifname, desc, m_oid, CWrapper::getName());
				//ret = ERROR_RFTEST_SUCCESS;	
				ret = 0x5921;
				break;
			case 0x6620: 
				ChipID = 0x6620;
				em_printf(MSG_DEBUG, (char*)"Chip ID = 0x%x", ChipID);
				m_adapter = new CMT6620(ifname, desc, m_oid, CWrapper::getName());
				//ret = ERROR_RFTEST_SUCCESS;	
				ret = 0x6620;
				break;	
			default: 
				m_adapter = NULL;
				em_printf(MSG_ERROR, (char*)"Not supported Chip ID = 0x%x", ChipID);
				break;	 
	
		}

out:
		return ret;
}


INT_32
CWrapper::Uninitialize()
{
	m_adapter->CloseDevice();
	delete m_oid;
	
	return ERROR_RFTEST_SUCCESS;
}




INT_32 
CWrapper::setTestMode (
        void
        ) 
{ 
	
    	return ChipID == 0x6620 ? 
    	((CMT6620*)m_adapter)->setTestMode(): 
    	((CMT5921*)m_adapter)->setTestMode(); 
}

INT_32 
CWrapper::setNormalMode (
        void
        )
{ 
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setNormalMode() :
    ((CMT5921*)m_adapter)->setNormalMode() ; 
}


INT_32 
CWrapper::setStandBy (
        void
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setStandBy() :
    ((CMT5921*)m_adapter)->setStandBy()
    ;
}

INT_32 
CWrapper::setEEPRomSize ( 
        INT_32  i4EepromSz
        )
{   
	em_printf(MSG_DEBUG, (char*)"u4Length is %d\n", i4EepromSz);
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setEEPRomSize(i4EepromSz) :
    ((CMT5921*)m_adapter)->setEEPRomSize(i4EepromSz);
}

INT_32 
CWrapper::setEEPRomFromFile ( 
        TCHAR* atcFileName
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setEEPRomFromFile(atcFileName) :
    ((CMT5921*)m_adapter)->setEEPRomFromFile(atcFileName)
    ;
}

#if 0
INT_32 
CWrapper::setTxMaxPowerToEEPromEx ( 
        INT_32  i4ChnFreg, 
        INT_32  i4Rate, 
        INT_32  i4TxPwrGain, 
        INT_32  i4Eirp, 
        INT_32  i4ThermoVal
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setTxMaxPowerToEEPromEx(
                i4ChnFreg, 
                i4Rate, 
                i4TxPwrGain, 
                i4Eirp, 
                i4ThermoVal
                ) : 
	((CMT6620*)m_adapter)->setTxMaxPowerToEEPromEx(
                i4ChnFreg, 
                i4Rate, 
                i4TxPwrGain, 
                i4Eirp, 
                i4ThermoVal
                );
}
#endif

INT_32
CWrapper::readTxPowerFromEEPromEx (
        INT_32  i4ChnFreg, 
        INT_32  i4Rate, 
        INT_32  *pi4TxPwrGain,
        INT_32  *pi4Eirp,
        INT_32  *pi4ThermoVal
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->readTxPowerFromEEPromEx(
                i4ChnFreg, 
                i4Rate, 
                pi4TxPwrGain,
                pi4Eirp,
                pi4ThermoVal
                ) :
    ((CMT5921*)m_adapter)->readTxPowerFromEEPromEx(
                i4ChnFreg, 
                i4Rate, 
                pi4TxPwrGain,
                pi4Eirp,
                pi4ThermoVal
                );	
}

INT_32 
CWrapper::setEEPromCKSUpdated (
        void
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setEEPromCKSUpdated() :
    ((CMT5921*)m_adapter)->setEEPromCKSUpdated()  ;
}

INT_32 
CWrapper::getPacketTxStatusEx ( 
        INT_32 * pi4SentCount, 
        INT_32 * pi4AckCount, 
        INT_32 * pi4Alc, 
        INT_32 * pi4CckGainControl, 
        INT_32 * pi4OfdmGainControl
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->getPacketTxStatusEx(
                pi4SentCount, 
                pi4AckCount, 
                pi4Alc, 
                pi4CckGainControl, 
                pi4OfdmGainControl
                ) :
     ((CMT5921*)m_adapter)->getPacketTxStatusEx(
                pi4SentCount, 
                pi4AckCount, 
                pi4Alc, 
                pi4CckGainControl, 
                pi4OfdmGainControl
                ) ;
}    


INT_32 
CWrapper::getPacketRxStatus ( 
        INT_32 *    pi4RxOk, 
        INT_32 *    pi4RxCrcErr
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->getPacketRxStatus(
                pi4RxOk, 
                pi4RxCrcErr
        ) :
    ((CMT5921*)m_adapter)->getPacketRxStatus(
                pi4RxOk, 
                pi4RxCrcErr
        );
}

INT_32 
CWrapper::setOutputPower ( 
        INT_32  i4Rate, 
        INT_32  i4TxPwrGain, 
        INT_32  i4TxAnt
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setOutputPower(
                i4Rate, 
                i4TxPwrGain, 
                i4TxAnt
                ) :
     ((CMT5921*)m_adapter)->setOutputPower(
                i4Rate, 
                i4TxPwrGain, 
                i4TxAnt
                );
}

INT_32 
CWrapper::setLocalFrequecy (
        INT_32  i4TxPwrGain, 
        INT_32  i4TxAnt
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setLocalFrequecy(
                i4TxPwrGain, 
                i4TxAnt
                ) : 
     ((CMT5921*)m_adapter)->setLocalFrequecy(
                i4TxPwrGain, 
                i4TxAnt
                );
}


INT_32 
CWrapper::setCarrierSuppression ( 
        INT_32  i4Modulation, 
        INT_32  i4TxPwrGain, 
        INT_32  i4TxAnt
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setCarrierSuppression(
                i4Modulation, 
                i4TxPwrGain, 
                i4TxAnt
                ) :
     ((CMT5921*)m_adapter)->setCarrierSuppression(
                i4Modulation, 
                i4TxPwrGain, 
                i4TxAnt
                );
}

INT_32 
CWrapper::setOperatingCountry (
        CHAR *acChregDomain   
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setOperatingCountry(acChregDomain) :
    ((CMT5921*)m_adapter)->setOperatingCountry(acChregDomain);
}

INT_32 
CWrapper::setChannel (
        INT_32  i4ChFreqkHz
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setChannel(i4ChFreqkHz) :
    ((CMT5921*)m_adapter)->setChannel(i4ChFreqkHz);
}

INT_32 
CWrapper::getSupportedRates ( 
        UINT_16 *pu2RateBuf, 
        INT_32  i4MaxNum
        )
{
    return m_adapter->getSupportedRates(
				pu2RateBuf, 
                i4MaxNum
                );
}


INT_32 
CWrapper::setOutputPin (
        INT_32  i4PinIndex, 
        INT_32  i4OutputLevel
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setOutputPin(
                i4PinIndex, 
                i4OutputLevel
                ) :
    ((CMT5921*)m_adapter)->setOutputPin(
                i4PinIndex, 
                i4OutputLevel
                );
}

INT_32 
CWrapper::readEEPRom16 (
        UINT_32 u4Offset,
        UINT_32 *pu4Value 
        )
{
	return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->readEEPRom16( 
		  		u4Offset,
                pu4Value 
                ):
    ((CMT5921*)m_adapter)->readEEPRom16( 
		  		u4Offset,
                pu4Value 
                );
}

INT_32 
CWrapper::readSpecEEPRom16 (
        UINT_32 u4Offset,
        UINT_32 *pu4Value 
        )
{
	return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->readSpecEEPRom16( 
		  		u4Offset,
                pu4Value 
                ):
    ((CMT5921*)m_adapter)->readSpecEEPRom16( 
		  		u4Offset,
                pu4Value 
                );
}

INT_32 
CWrapper::writeEEPRom16 ( 
        UINT_32 u4Offset, 
        UINT_32 u4Value
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->writeEEPRom16(	 
		  		u4Offset, 
                u4Value
                ):
	((CMT5921*)m_adapter)->writeEEPRom16(	 
		  		u4Offset, 
                u4Value
                ) ;               	
                
}

INT_32 
CWrapper::eepromReadByteStr ( 
        UINT_32 u4Addr, 
        UINT_32 u4Length, 
        INT_8   *paucStr 
        )
{
	em_printf(MSG_DEBUG, (char*)"%s u4Length is %d\n", __FUNCTION__, u4Length);

    return ChipID == 0x6620 ? 
    ((CMT6620*)m_adapter)->eepromReadByteStr(
		  		u4Addr, 
                u4Length, 
                (CHAR *)paucStr 
                ) :
   ((CMT5921*)m_adapter)->eepromReadByteStr(
		  		u4Addr, 
                u4Length, 
                (CHAR *)paucStr 
                ) ;
}


INT_32 
CWrapper::eepromWriteByteStr (
        UINT_32 u4Addr, 
        UINT_32 u4Length, 
        INT_8   *paucStr 
        )
{
   em_printf(MSG_DEBUG, (char*)"%s u4Length is %d\n", __FUNCTION__, u4Length);
   
   	return ChipID == 0x6620 ? 
   	((CMT6620*)m_adapter)->eepromWriteByteStr(
                u4Addr, 
                u4Length, 
                (CHAR *)paucStr 
                ):
   ((CMT5921*)m_adapter)->eepromWriteByteStr(
                u4Addr, 
                u4Length, 
                (CHAR *)paucStr 
                );
             
}

   
INT_32 
CWrapper::setATParam ( 
        UINT_32 u4FuncIndex, 
        UINT_32 u4FuncData
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->SetATParam(u4FuncIndex, u4FuncData) : 
    ((CMT5921*)m_adapter)->SetATParam(u4FuncIndex, u4FuncData) 	;
}


INT_32 
CWrapper::getATParam ( 
        UINT_32 u4FuncIndex, 
        UINT_32 *pu4FuncData
        )
{
    return ChipID == 0x6620 ?   
    ((CMT6620*)m_adapter)->GetATParam(u4FuncIndex, pu4FuncData) : 
    ((CMT5921*)m_adapter)->GetATParam(u4FuncIndex, pu4FuncData);
}


INT_32 
CWrapper::setXtalTrimToCr (
        UINT_32 u4Value
        )
{
    return ChipID == 0x6620 ?   
    ((CMT6620*)m_adapter)->setXtalTrimToCr(u4Value) :
    ((CMT5921*)m_adapter)->setXtalTrimToCr(u4Value)	;
}

INT_32
CWrapper::getXtalTrimToCr(
		UINT_32	 * pu4Value)
{
	return ChipID == 0x6620 ?  
	((CMT6620*)m_adapter)->getXtalTrimToCr(pu4Value) : 
	((CMT5921*)m_adapter)->getXtalTrimToCr(pu4Value) 	;

}

INT_32
CWrapper::queryThermoInfo (
        INT_32 *    pi4Enable, 
        UINT_32 *   pu4RawVal
        )
{
    return ChipID == 0x6620 ?   
    ((CMT6620*)m_adapter)->queryThermoInfo(pi4Enable,pu4RawVal) :
    ((CMT5921*)m_adapter)->queryThermoInfo(pi4Enable,pu4RawVal);
}

INT_32
CWrapper::setThermoEn (
        INT_32  i4Enable
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->setThermoEn(i4Enable) : 
    ((CMT5921*)m_adapter)->setThermoEn(i4Enable);
}


INT_32
CWrapper::getEEPRomSize (
        INT_32* pi4EepromSz
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->getEEPRomSize(pi4EepromSz) : 
    ((CMT5921*)m_adapter)->getEEPRomSize(pi4EepromSz) ;
}

INT_32
CWrapper::getSpecEEPRomSize (
        INT_32* pi4EepromSz
        )
{
    return ChipID == 0x6620 ?  
    ((CMT6620*)m_adapter)->getSpecEEPRomSize(pi4EepromSz) : 
    ((CMT5921*)m_adapter)->getSpecEEPRomSize(pi4EepromSz) ;
}

INT_32
CWrapper::setPnpPower ( 
        INT_32  i4PowerMode
        )
{
    return m_adapter->setPnpPower(i4PowerMode);
}

INT_32
CWrapper::setAnritsu8860bTestSupportEn ( 
        INT_32 i4Enable
        )
{
    return ChipID == 0x6620 ?   
    ((CMT6620*)m_adapter)->setAnritsu8860bTestSupportEn(i4Enable):
    ((CMT5921*)m_adapter)->setAnritsu8860bTestSupportEn(i4Enable)
    ;
}

INT_32
CWrapper::readMCR32(
	UINT_32 offset, UINT_32 *value
	)
{
	return ChipID == 0x6620 ?  
	((CMT6620*)m_adapter)->readMCR32(offset,value) :
	((CMT5921*)m_adapter)->readMCR32(offset,value);
}

INT_32	
CWrapper::writeMCR32(
	UINT_32 offset, UINT_32 value
	)
{
	return ChipID == 0x6620 ?  
	((CMT6620*)m_adapter)->writeMCR32(offset,value) :
	((CMT5921*)m_adapter)->writeMCR32(offset,value);
}

INT_32	
CWrapper::getDPDLength(
	INT_32* pi4DPDLength
	)
{
	if( ChipID == 0x6620 )
	{
		return ((CMT6620*)m_adapter)->getDPDLength(pi4DPDLength);
	}
	else
	{
		DEBUGFUNC((char*)"CWrapper::getDPDLength - error for mt5921");
		*pi4DPDLength = 0;
		return 0x0;
	}
	
}

INT_32
CWrapper::readDPD32(
	UINT_32 offset, UINT_32 *value
	)
{
	if( ChipID == 0x6620 )
	{
		return ((CMT6620*)m_adapter)->readDPD32(offset, value);
	}
	else
	{
		DEBUGFUNC((char*)"CWrapper::readDPD32 - error for mt5921");
		return 0x0;
	}
}

INT_32	
CWrapper::writeDPD32(
	UINT_32 offset, UINT_32 value
	)
{
	if( ChipID == 0x6620 )
	{
		return ((CMT6620*)m_adapter)->writeDPD32(offset,value) ;
	}
	else
	{
		DEBUGFUNC((char*)"CWrapper::writeDPD32 - error for mt5921");
		return 0x0;
	}
}
INT_32 CWrapper::setDPDFromFile ( TCHAR * atcFileName)
{
	if( ChipID == 0x6620 )
	{
		return ((CMT6620*)m_adapter)->setDPDFromFile(atcFileName);
	}
	else
	{
		DEBUGFUNC((char*)"CWrapper::setDPDFromFile - error for mt5921");
		return 0x0;
	}
}

// Added by mtk54046 @ 2012-01-05 for get support channel list
INT_32 CWrapper::getSupportChannelList(UINT_32 *pau4Channel)
{
	return m_adapter->getSupportChannelList(pau4Channel);
}

INT_32 CWrapper::doCTIATestSet(UINT_32 u4Id, UINT_32 u4Value)
{
	return m_adapter->sw_cmd(1, u4Id, &u4Value);
}

INT_32 CWrapper::doCTIATestGet(UINT_32 u4Id, UINT_32 *pu4Value)
{
	return m_adapter->sw_cmd(0, u4Id, pu4Value);
}

}
