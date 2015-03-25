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
** $Id: mt6620.h,v 1.5 2008/11/11 03:12:13 MTK01725 Exp $
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
** $Log: mt6620.h,v $
 *
 * 11 01 2010 Chaozhong.Liang
 * init from mt5921.h
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
** Revision 1.5  2008/11/11 03:12:13  MTK01725
** Override a getChannelList method from CAdapter. This funciton is not ready yet.
**
** Revision 1.4  2008/10/28 13:45:44  MTK01725
** Frog add IO Pin Test and other little modification.
**
** Revision 1.3  2008/06/12 02:35:42  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:51:53  MTK01385
** 1. add setXtalTrimToCr(), queryThermoinfo() and setThermoEn().
**
** Revision 1.1  2008/05/26 14:04:37  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.9  2008/03/25 09:34:29  MTK01385
** 1. Based on MT5921MP_BB_RF_Config_v0.4.doc
**  to update Tx output power and Tx carrier Suppression functions.
**
** Revision 1.8  2008/02/23 17:36:15  MTK01385
** 1. MT5921 EEPROM related definitions move from header file to .c file.
**
** Revision 1.7  2008/02/23 16:32:12  MTK01385
** 1 add support set Crystal Frequency Trim and RCPI offset to eeprom.
**
** Revision 1.5  2008/02/22 13:16:12  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.4  2008/01/08 01:38:27  MTK01385
** 1. Move EEPROM definitions from MT5921 c file to header file for UI usage.
**
** Revision 1.3  2008/01/03 07:56:42  MTK01385
** 1. Add Set Chip type function.
**
** Revision 1.2  2007/12/11 15:44:17  MTK01385
** 1. 1.	Modify Tx Output Power, Tx Carrier Suppression and Tx Local Frequency function based on SD request.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.3  2007/11/12 06:25:04  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.2  2007/10/16 13:23:42  MTK01086
** Modify CAdapter to prevent user call destructor
**
** Revision 1.1  2007/10/16 06:58:51  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/

#ifndef _MT6620_H
#define _MT6620_H


#include "COid.h"
#include "WiFi_EM_API.h"
/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/
namespace android{

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

class CMT6620 : public CAdapter{
protected:
    ~CMT6620(void){}

public:
    CMT6620(const char *Name, const char *Desc, COID* oid, const char* wrapperName):CAdapter(Name, Desc, oid, wrapperName), fgAnritsuSupportEn(false)
       , u4OriRoamingEn(0) {isEepromUsed = false;}

	UINT_32 ChipID() { return 0x6620;}

    //For MP====================================================================
    //In ATE
    INT_32 setChannel(INT_32 channelFreq);
    INT_32 getPacketRxStatus(INT_32 * rxOk_p, INT_32 * rxCrcErr_p);
    INT_32 getChannel(INT_32 * channelConfig_p);
    INT_32 setTestMode(void);
    INT_32 setStandBy(void);
    INT_32 setOutputPower(INT_32 nTxRate, INT_32 txPower, INT_32 txAntenna);
    INT_32 setEEPRomFromFile(TCHAR* fileName);
    INT_32 setTXMaxPowerToEEProm(INT_32 channelFreg, INT_32 bCck, INT_32 nTxPwr);
    INT_32 setTXMaxPowerToEEPromEx(
        INT_32 channelFreq,
        INT_32 rate,
        INT_32 gainControl,
        INT_32 outputPower,
        INT_32 targetAlc);

    INT_32 readTxPowerFromEEProm(
        INT_32 channelFreq,
        INT_32 bCck,
        INT_32 *nTxPwr,
        INT_32 ndBindex);

    INT_32 setPacketRx(INT_32 condition, RX_ANT_SEL nAntenna);
    INT_32 setEEPromCKSUpdated(void);
    INT_32 setEEPRomSize(INT_32 eepromSz);
    INT_32 getEEPRomVersion(INT_32* version);
    INT_32 setOutputPin(INT_32 pinIndex, INT_32 outputLevel);
    INT_32 getInputPin(INT_32 pinIndex, INT_32* inputLevel);
    INT_32 testInputPin(INT_32 pinIndex, INT_32 inputLevel);
    INT_32 restoreIOPin(void);
    INT_32 setXtalFreqTrimToEEProm (
        INT_32 i4FreqTrim
        );
    INT_32 setRcpiOffsetToEEProm (
	    INT_32 channelFreq,
	    INT_32 offset
	    );
    INT_32 setXtalTrimToCr (
        UINT_32 u4Value
        );

    //not in ATE
    INT_32 getXtalTrimToCr (
        UINT_32 *pu4Value
        );
	
    INT_32 setCarrierSuppression(
            INT_32 nModulationType,
            INT_32 txPower,
            INT_32 txAntenna);

    INT_32 setLocalFrequecy(INT_32 txPower, INT_32 txAntenna);
    INT_32 setAlcInfoToEeprom(
            INT_32 b24gAvailable,
            INT_32 b5gAvailable);

    INT_32 getAlcInfo(
        INT_32* b24gAvailable,
        INT_32* b5gAvailable,
        INT_32* bUseSlopeRate);

    INT_32 setAlcSlopeRatioToEeprom(
        INT_32  slope1Divider,
        INT_32  slope1Dividend,
        INT_32  slope2Divider,
        INT_32  slope2Dividend);

    INT_32 getAlcSlopeRatio(
        INT_32* slope1Divider,
        INT_32* slope1Dividend,
        INT_32* slope2Divider,
        INT_32* slope2Dividend);
    INT_32 setMACAddrToEEProm(UINT_8* macAddr);

    INT_32 setCountryToEEProm(UINT_8* country);
    INT_32 setPhyModeToEEProm(INT_32 mode);
    INT_32 queryThermoInfo(INT_32 * pi4Enable, UINT_32 * pu4RawVal);
    INT_32 setThermoEn(INT_32 i4Enable);

#if 0
		INT_32 setPacketTxEx(
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
        INT_32 txAntenna);
#endif
    INT_32 getChannelList(
        UINT_32 * frenqBuf, 
        INT_32 maxNum);

    INT_32 testIOPin(UINT_32 u4ModuleIndex);

    //not for MP================================================================
    INT_32 readMCR32(UINT_32 offset, UINT_32* value);
    INT_32 writeMCR32(UINT_32 offset, UINT_32 value);
    INT_32 readBBCR8(UINT_32 offset, UINT_32* value);
    INT_32 writeBBCR8(UINT_32 offset, UINT_32 value);
    INT_32 readEEPRom16(UINT_32 offset, UINT_32 * value);
    INT_32 readSpecEEPRom16(UINT_32 offset, UINT_32 * value);
    INT_32 writeEEPRom16(UINT_32 offset, UINT_32 value);
    
    INT_32 setNormalMode(void);
    CHAR * getRFICType(void);
    INT_32 readBBCRStatistic(
        UINT_16* ed,
        UINT_16* osd,
        UINT_16* sq1,
        UINT_16* sfd,
        UINT_16* fcs);

    INT_32 getRadarStatus(void);
    INT_32 readTxPowerFromEEPromEx(
        INT_32 channelFreq,
        INT_32 rate,
        INT_32 *nTxPwr,
        INT_32 *outputPower,
        INT_32 *targetAlc);

    INT_32 getEEPRomSize(INT_32 * eepromSz);
    INT_32 getSpecEEPRomSize(INT_32 *pi4EepromSz);
    INT_32 setDACOffsetToEEProm(UINT_16 dacOffset);
    INT_32 getDACOffset(UINT_16* dacOffset);

    INT_32 SetATParam(UINT_32 offset, UINT_32 value);
    INT_32 GetATParam(UINT_32 offset, UINT_32 * value);
    INT_32 setChipType(UINT_32 u4ChipType);
    INT_32 setAnritsu8860bTestSupportEn ( INT_32    i4Enable );
	INT_32 getDPDLength (INT_32* dpdLength);
	INT_32 readDPD32 (UINT_32 offset, UINT_32 * value);
	INT_32 writeDPD32 (UINT_32 offset, UINT_32 value);
	INT_32 setDPDFromFile(TCHAR* fileName);
private:
    INT_32 testLoopD4toD15 (UINT_32 u4TestInput, BOOL *pfgResult);
    INT_32 updateALCOffset(void);
    INT_32 findMaxPowerByteOffset(INT_32 nChannel, INT_32 bCck, INT_32 b24Gs);
    UINT_8 getTxRate(INT_32 nTxRate, INT_32 nPreamble);
    INT_32 setPPMCRAndSCR(void);
    INT_32 setTxAnt(INT_32 antenna);
    UINT_8 nicCalculateChecksumByte(void * startingAddr_p, UINT_32 length);
    INT_32 getEEPRomVersion_(void);
    INT_32 setBBTxPower(INT_32 txPower);
	

    UINT_32 u4IoRegSave[BACKUP_REGISTER_NUM];
    UINT_32 u4ChipType;
    UINT_32 u4OriRFCR;


    bool fgAnritsuSupportEn;
    UINT_32 u4OriRoamingEn;
    bool isEepromUsed;
};

}

#endif

