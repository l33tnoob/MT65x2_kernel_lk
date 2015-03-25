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
** $Id: mt5912.h,v 1.3 2008/06/12 02:34:20 MTK01385 Exp $
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
** $Log: mt5912.h,v $
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.3  2008/06/12 02:34:20  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:49:52  MTK01385
** 1. add setXtalTrimToCr(), queryThermoinfo() and setThermoEn().
**
** Revision 1.1  2008/05/26 14:04:37  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.5  2008/02/23 16:31:11  MTK01385
** 1 add support set Crystal Frequency Trim and RCPI offset to eeprom.
**
** Revision 1.3  2008/02/22 13:16:12  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.2  2008/01/03 07:55:07  MTK01385
** 1. Add Set Chip type function.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.4  2007/11/12 06:25:04  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.3  2007/10/17 08:49:26  MTK01267
** move DAC function to mt5911, mt5912
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

#ifndef _MT5912_H
#define _MT5912_H


#include "COID.h"
/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/

class DllExport CMT5912 : public CAdapter
{
protected:
    ~CMT5912(void){}

public:
    CMT5912(const wstring &Name, const wstring &Desc, COID * oid, CSite *SiteName):CAdapter(Name, Desc, oid, SiteName){}
	UINT_32 ChipID() { return 0x5912;}

    //For MP====================================================================
    //In ATE
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
    INT_32 readEEPRom16(UINT_32 offset, UINT_32 * value);
    INT_32 writeEEPRom16(UINT_32 offset, UINT_32 value);
    
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

    //not for MP================================================================
    INT_32 readMCR32(UINT_32 offset, UINT_32* value);
    INT_32 writeMCR32(UINT_32 offset, UINT_32 value);
    INT_32 readBBCR8(UINT_32 offset, UINT_32* value);
    INT_32 writeBBCR8(UINT_32 offset, UINT_32 value);
    
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
    INT_32 setDACOffsetToEEProm(UINT_16 dacOffset);
    INT_32 getDACOffset(UINT_16* dacOffset);

    INT_32 SetATParam(UINT_32 offset, UINT_32 value);
    INT_32 GetATParam(UINT_32 offset, UINT_32 * value);
    INT_32 setChipType(UINT_32 u4ChipType);
    INT_32 setAnritsu8860bTestSupportEn ( INT_32    i4Enable );

private:
    INT_32 updateALCOffset(void);
    INT_32 findMaxPowerByteOffset(INT_32 nChannel, INT_32 bCck, INT_32 b24Gs);
    UINT_8 getTxRate(INT_32 nTxRate, INT_32 nPreamble);
    INT_32 setPPMCRAndSCR(void);
    INT_32 setTxAnt(INT_32 antenna);
    UINT_8 nicCalculateChecksumByte(void * startingAddr_p, UINT_32 length);
    INT_32 getEEPRomVersion_(void);
    INT_32 setBBTxPower(INT_32 txPower);

    //About DAC
    void DACInit(void);
    void DACSetep1(UINT_8* dposi, UINT_8* dposq);
    void DACSetep2(UINT_8* dnegi, UINT_8* dnegq);
    void DACSetep4(UINT_8 txosID, UINT_8 txosQD, UINT_8* dadci, UINT_8* dadcq);

    void calDADCoffset(
        UINT_8 dposi,
        UINT_8 dposq,
        UINT_8 dnegi,
        UINT_8 dnegq,
        INT_32* ddaci_int,
        INT_32* ddacq_int,
        UINT_32* double_dadci1,
        UINT_32* double_dadcq1);

    void interateDACAdjust(
        UINT_8 valueID,
        UINT_8 valueQD,
        UINT_32 targetIx2,
        UINT_32 targetQx2,
        UINT_8* bestI,
        UINT_8* bestQ);

};

#endif

