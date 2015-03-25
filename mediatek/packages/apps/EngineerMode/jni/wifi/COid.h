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
** $Id: COid.h,v 1.6 2008/12/17 13:06:24 MTK01425 Exp $
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
** $Log: COid.h,v $
 *
 * 11 16 2012 ben.niu
 * Add CTIA test setting/getting
 *
 * 01 10 2012 ben.niu
 * [ALPS00110759] Get chip support channels.
 * Add getSupportChannelList
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.6  2008/12/17 13:06:24  MTK01425
** Add setOID and queryOID
**
** Revision 1.5  2008/11/11 03:06:09  MTK01725
** no message
**
** Revision 1.4  2008/10/28 13:45:44  MTK01725
** Frog add IO Pin Test and other little modification.
**
** Revision 1.3  2008/06/12 02:32:14  MTK01385
** 1. add Anritsu 8860B test Mode support.
**
** Revision 1.2  2008/06/04 08:37:17  MTK01385
** 1. add setPnpPower(), setXtalTrimToCr(), queryThermoinfo() and setThermoEn().
**
** Revision 1.1  2008/05/26 14:04:36  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.8  2008/02/23 16:32:43  MTK01385
** 1 add support set Crystal Frequency Trim and RCPI offset to eeprom.
**
** Revision 1.6  2008/02/22 14:34:57  MTK01385
** 1. Adapter add EEPROM byte string read/write feature.
**
** Revision 1.5  2008/02/22 13:16:51  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.4  2008/01/03 07:50:48  MTK01385
** 1. Add Set Chip type function.
**
** Revision 1.3  2007/12/14 08:18:16  MTK01385
** 1. fix C4251 warning.
**
** Revision 1.2  2007/12/11 15:41:27  MTK01385
** 1. Add build wpdnic.dll support.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.6  2007/11/12 06:25:19  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.5  2007/11/09 04:02:49  MTK01267
** refine code
**
** Revision 1.4  2007/10/24 05:35:59  MTK01086
** Rename COid_local to CLocalOID, COid_socket to CSocketOID
**
** Revision 1.3  2007/10/17 08:51:14  MTK01267
** move DAC function to mt5911, mt5912
**
** Revision 1.2  2007/10/16 13:23:42  MTK01086
** Modify CAdapter to prevent user call destructor
**
** Revision 1.1  2007/10/16 06:57:36  MTK01267
** Inital version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
*/


#ifndef _COID_H_
#define _COID_H_

#include "type.h"
#include <errno.h>
//#include "WiFi_EM_API.h"


namespace android{

class CAdapter;
class CWrapper;

typedef struct {
	const char * Name;
	const char * Desc;
} AdapterName;
/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/
/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/

class COID
{
public:
	COID(){}
    virtual ~COID() {};

	//called by CSite
	virtual BOOL   GetChipID(UINT_32 & ChipID, const char *Name) = 0;
    // virtual INT_32 EnumAdapter(deque<AdapterName> & adaptersNameList) =0;

    //called by CAdapter
    virtual INT_32 setOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen) =0;
    virtual INT_32 queryOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite) =0;

private:

};


class CLocalOID : public COID
{
private:
	HANDLE m_handle;
	CAdapter* Adapter;
	char m_name[IFNAMSIZ + 1];
	char chipName[10];
public:
    CLocalOID(const char* ifname);
    CLocalOID(const char* ifname, const char *chipName);
    ~CLocalOID();

	BOOL   GetChipID(UINT_32 & ChipID, const char *Name);

    //INT_32 EnumAdapter(deque<AdapterName> & adaptersNameList);
    INT_32 setOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen);
    INT_32 queryOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite);
#if 0
private:
    //do mux of protocol driver, control the access of the protocol driver
    BOOL checkDevice(CAdapter *aAdapter);
    BOOL closeDevice(const wstring &AdapterName);
    BOOL openDevice(const wstring &AdapterName);
#endif
};

#ifndef HAVE_ANDROID_OS
class CSocketOID : public COID
{
private:
    Socket *m_socket;
public:
    CSocketOID(CSite *aSite, const CHAR * HostName, UINT_16 HostPort = 5001);
    ~CSocketOID(void);

    BOOL   GetChipID(UINT_32 & ChipID, const wstring &Name);
    //INT_32 EnumAdapter(deque<AdapterName> & adaptersNameList);
    INT_32 setOID(CAdapter *aAdapter, UINT_32 oid, UCHAR *buf, UINT_32 bufLen);
    INT_32 queryOID(CAdapter *aAdapter, UINT_32 oid, UCHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite);

private:
    UINT_32
    MakeEtherRequest(
        wstring wAdapterID,
        BOOL bSetOid,
        UINT_32 oid,
        CHAR * pBuf,
        UINT_32 bytes,
        UINT_32 * outputBytes);

    //for socket send and receive
	INT_32 SendData(CHAR *Buffer, INT_32 Count) { return m_socket->SendData(Buffer, Count); }
	INT_32 RecvData(CHAR *Buffer, INT_32 Count) { return m_socket->RecvData(Buffer, Count); }
    void ErrorHandle(INT_32 SendOrRecv);
};
#endif


/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/
#define ERR_DEV_REMOVE -1
#define ERR_DEV_REINSERT -2

/* BB control register set */
#define BBCR_CR0                0
#define BBCR_CR1                1
#define BBCR_CR2                2
#define BBCR_CR3                3
#define BBCR_CR4                4
#define BBCR_CR5                5
#define BBCR_CR6                6
#define BBCR_CR7                7
#define BBCR_CR8                8
#define BBCR_CR9                9
#define BBCR_CR10               10
#define BBCR_CR11               11
#define BBCR_CR12               12
#define BBCR_CR13               13
#define BBCR_CR14               14
#define BBCR_CR15               15
#define BBCR_CR16               16
#define BBCR_CR17               17
#define BBCR_CR18               18
#define BBCR_CR19               19
#define BBCR_CR20               20
#define BBCR_CR21               21
#define BBCR_CR22               22
#define BBCR_CR23               23
#define BBCR_CR24               24
#define BBCR_CR25               25
#define BBCR_CR26               26
#define BBCR_CR27               27
#define BBCR_CR28               28
#define BBCR_CR29               29
#define BBCR_CR30               30
#define BBCR_CR31               31
#define BBCR_CR32               32
#define BBCR_CR33               33
#define BBCR_CR34               34
#define BBCR_CR35               35
#define BBCR_CR36               36
#define BBCR_CR37               37
#define BBCR_CR38               38
#define BBCR_CR39               39
#define BBCR_CR80               80
#define BBCR_CR81               81
#define BBCR_CR82               82
#define BBCR_CR83               83
#define BBCR_CR84               84
#define BBCR_CR85               85
#define BBCR_CR86               86
#define BBCR_CR87               87
#define BBCR_CR88               88
#define BBCR_CR89               89
#define BBCR_CR90               90
#define BBCR_CR91               91
#define BBCR_CR92               92
#define BBCR_CR93               93
#define BBCR_CR94               94
#define BBCR_CR95               95
#define BBCR_CR96               96
#define BBCR_CR97               97
#define BBCR_CR98               98
#define BBCR_CR99               99
#define BBCR_CR100              100
#define BBCR_CR101              101
#define BBCR_CR102              102
#define BBCR_CR103              103
#define BBCR_CR104              104
#define BBCR_CR105              105
#define BBCR_CR106              106
#define BBCR_CR107              107
#define BBCR_CR108              108
#define BBCR_CR109              109
#define BBCR_CR110              110
#define BBCR_CR111              111
#define BBCR_CR112              112
#define BBCR_CR113              113
#define BBCR_CR114              114
#define BBCR_CR115              115
#define BBCR_CR116              116
#define BBCR_CR117              117
#define BBCR_CR118              118
#define BBCR_CR119              119
#define BBCR_CR120              120
#define BBCR_CR121              121
#define BBCR_CR122              122
#define BBCR_CR123              123
#define BBCR_CR124              124
#define BBCR_CR125              125
#define BBCR_CR126              126
#define BBCR_CR127              127
#define BBCR_CR128              128
#define BBCR_CR129              129

#ifndef BG_RATE_
#define BG_RATE_

#define RATE_1M                 0x02    /* 1M */
#define RATE_2M                 0x04    /* 2M */
#define RATE_5_5M               0x0B    /* 5.5M */
#define RATE_11M                0x16    /* 11M */
#define RATE_6M                 0x0c    /* 6M */
#define RATE_9M                 0x12    /* 9M */
#define RATE_12M                0x18    /* 12M */
#define RATE_18M                0x24    /* 18M */
#define RATE_24M                0x30    /* 24M */
#define RATE_36M                0x48    /* 36M */
#define RATE_48M                0x60    /* 48M */
#define RATE_54M                0x6c    /* 54M */

typedef enum {
    PHY_80211G,
    PHY_80211AG,
    PHY_80211A,
    PHY_80211B
} PHY_MODE;

#endif
/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
typedef struct _CHN_FREQ{
    INT_32 band; //0 for 2_4, 1 for 5
    INT_32 channelNum;
    INT_32 freq;
}TChnFreq;

/* NIC BBCR configuration entry structure */
typedef struct _NIC_BBCR_CONFIG_ENTRY {
    INT_8 *      name_p;
    UINT_8       offset;
    UINT_8       value;
} NIC_BBCR_CONFIG_ENTRY, *PNIC_BBCR_CONFIG_ENTRY;

#ifndef _COUNTRY_CODE_
#define _COUNTRY_CODE_

typedef struct {
    char    countryCode[2];
} COUNTRY_CODE;

typedef enum {
    PM_FULLY_AWAKE,
    PM_SLEEP
} POWER_MANAGEMENT_STATE;

#endif

/*******************************************************************************
*                          M A C R O S
********************************************************************************
*/
#define EEPROM_RD_DWORD2BYTE_STR(_eepWordOffset, _pacTmp) \
	{ \
        UINT_32 _u4Tmp;\
        readEEPRom16(_eepWordOffset, &_u4Tmp);\
        sprintf(_pacTmp, "%02X%02X", ((UINT_8) ( _u4Tmp & 0x000000FF)), ((UINT_8) ( (_u4Tmp>>8) & 0x000000FF) ));\
	}

#define EEPROM_WR_BYTE_STR_2_DWORD(_eepWordOffset, _paucValue) \
	{ \
        UINT_32 _u4Tmp1, _u4Tmp2;\
        sscanf(_paucValue, "%02X%02X", &_u4Tmp1, &_u4Tmp2 );\
        _u4Tmp1 = ((_u4Tmp1<<0) & 0x000000FF) | ((_u4Tmp2<<8) & 0x0000FF00);\
        writeEEPRom16(_eepWordOffset, _u4Tmp1);\
	}


/*******************************************************************************
*                          F U N C T I O N S
********************************************************************************
*/

class CAdapter
{
private:
	char Name[NAMESIZE];
	char ProtocolName[NAMESIZE];
   	INT_32 m_referCnt;

protected:
    COID *m_OID;
    virtual ~CAdapter(void);

public:
    CAdapter(const char *AdapterName, const char *Desc, COID * oid, const char* wrapperName);

    void OpenDevice(void) {
        m_referCnt++;
    }
    void CloseDevice(void) {
		m_referCnt--;
		if (m_referCnt == 0) {
            delete this;
        }
	}
    bool CanDelete(void) { return (m_referCnt == 0); }
   	const char* GetWstrAdapterID() { return ProtocolName; }
    const char*  GetFullAdapterID() { return Name; }
//    CSite * GetSite(void) { return Site; }

//========================[Adapter function]====================================

//[For MP in testfunc.h]
public:
//[MP]==General=================================================================
    
#if 0
    virtual INT_32 setPacketTxEx(
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
    INT_32 getPacketTxStatusEx(
        long *sentCount,
        long *ackCount,
        long *alc,
        long *cckGainControl,
        long *ofdmGainControl);
    INT_32 eepromReadByteStr (
	    UINT_32 u4Addr,
        UINT_32 u4Length,
        char *pszStr
	    );
    INT_32 eepromWriteByteStr (
	UINT_32 u4Addr,
    UINT_32 u4Length,
    char *pszStr
	);
    // ring OSC
    INT_32 setLowPowerTest(INT_32 triggerDuration);
    INT_32 getLowPowerTestResult(INT_32 * triggerd, INT_32 * triggeredDuration);

    //not in ATE======
    INT_32 setOperatingCountry(CHAR * chregDomain);
    INT_32 getChannelList(UINT_32 * frenqBuf, INT_32 maxNum);
    INT_32 getSupportedRates(UINT_16 * outputRates, INT_32 maxNum);
    INT_32 setPnpPower(INT_32 i4PowerMode);
// Added by mtk54046 @ 2012-01-05 for get support channel list
    INT_32 getSupportChannelList(UINT_32 *pau4Channel);
// Added by mtk54046 @ 2012-11-15 for CTIA test setting/getting
    INT_32 sw_cmd(int set, UINT_32 adr, UINT_32 *dat);
//[MP]==For each chip implement=================================================
	virtual INT_32 setChannel(INT_32 channelFreq) = 0;
    virtual INT_32 getPacketRxStatus(INT_32 * rxOk_p, INT_32 * rxCrcErr_p) = 0;
    virtual INT_32 SetATParam(UINT_32 offset, UINT_32 value)=0;
    virtual INT_32 GetATParam(UINT_32 offset, UINT_32 * value)=0;

    virtual INT_32 setTestMode(void) =0;
    virtual INT_32 setStandBy(void) =0;
    virtual INT_32 setOutputPower(INT_32 nTxRate, INT_32 txPower, INT_32 txAntenna) =0;
    virtual INT_32 setPacketRx(INT_32 condition, RX_ANT_SEL nAntenna) =0;

    virtual INT_32 setEEPRomFromFile(TCHAR* fileName) = 0;

    virtual INT_32 setEEPromCKSUpdated(void) =0;
    virtual INT_32 getEEPRomVersion(INT_32* version) =0;
    virtual INT_32 setEEPRomSize(INT_32 eepromSz) =0;
    
    virtual INT_32 setTXMaxPowerToEEProm(INT_32 channelFreg, INT_32 bCck, INT_32 nTxPwr) =0;
    virtual INT_32 setTXMaxPowerToEEPromEx(
        INT_32 channelFreq,
        INT_32 rate,
        INT_32 gainControl,
        INT_32 outputPower,
        INT_32 targetAlc) =0;
    virtual INT_32 readTxPowerFromEEProm(
        INT_32 channelFreq,
        INT_32 bCck,
        INT_32 *nTxPwr,
        INT_32 ndBindex) =0;

    // pin IO
    virtual INT_32 setOutputPin(INT_32 pinIndex, INT_32 outputLevel) =0;
    virtual INT_32 getInputPin(INT_32 pinIndex, INT_32* inputLevel) =0;
    virtual INT_32 testInputPin(INT_32 pinIndex, INT_32 inputLevel) =0;
    virtual INT_32 restoreIOPin(void) =0;

    virtual INT_32 setXtalTrimToCr(UINT_32 u4Value)=0;
    //no in ATE========
    virtual INT_32 setCarrierSuppression(
        INT_32 nModulationType,
        INT_32 txPower,
        INT_32 txAntenna) =0;

    virtual INT_32 setLocalFrequecy(INT_32 txPower, INT_32 txAntenna) =0;
    virtual INT_32 getAlcInfo(
        INT_32* b24gAvailable,
        INT_32* b5gAvailable,
        INT_32* bUseSlopeRate) =0;
    virtual INT_32 setAlcInfoToEeprom(
        INT_32 b24gAvailable,
        INT_32 b5gAvailable) =0;
    virtual INT_32 setAlcSlopeRatioToEeprom(
        INT_32  slope1Divider,
        INT_32  slope1Dividend,
        INT_32  slope2Divider,
        INT_32  slope2Dividend) =0;
    virtual INT_32 getAlcSlopeRatio(
        INT_32* slope1Divider,
        INT_32* slope1Dividend,
        INT_32* slope2Divider,
        INT_32* slope2Dividend) =0;

    virtual INT_32 setMACAddrToEEProm(UINT_8* macAddr) =0;
    virtual INT_32 setCountryToEEProm(UINT_8* country) =0;
    virtual INT_32 setPhyModeToEEProm(INT_32 mode) =0;
    //specific for Thermo mechanism used from MT5921 
    virtual INT_32 queryThermoInfo(INT_32 * pi4Enable, UINT_32 * pu4RawVal) =0;
    virtual INT_32 setThermoEn(INT_32 i4Enable) =0;

 //   virtual INT_32 testIOPin(UINT_32 u4ModuleIndex);

//[not for MP in labfunc.h]
public:
    //General===================================================================
    INT_32 getChannelBand(INT_32 freq, INT_32* channel, INT_32* band);
    INT_32 getOperatingCountry(CHAR* country);
    INT_32 getMACAddr(UINT_8 * szMACAddr);
    INT_32 getSupportedCountries(COUNTRY_CODE* countries, INT_32 maxNum);
    INT_32 setPowerManagementState(POWER_MANAGEMENT_STATE state);

    //For each chip implement===================================================
    virtual INT_32 getChannel(INT_32 * channelConfig_p) = 0;
    virtual INT_32 readMCR32(UINT_32 offset, UINT_32* value)=0;
    virtual INT_32 writeMCR32(UINT_32 offset, UINT_32 value)=0;
    virtual INT_32 readBBCR8(UINT_32 offset, UINT_32* value)=0;
    virtual INT_32 writeBBCR8(UINT_32 offset, UINT_32 value)=0;
    virtual INT_32 readEEPRom16(UINT_32 offset, UINT_32* value)=0;
    virtual INT_32 readSpecEEPRom16(UINT_32 offset, UINT_32* value)=0;
    virtual INT_32 writeEEPRom16(UINT_32 offset, UINT_32 value)=0;
    virtual INT_32 setXtalFreqTrimToEEProm (
	            INT_32 i4FreqTrim
	            ) = 0;
    virtual INT_32 setRcpiOffsetToEEProm (
	            INT_32 channelFreq,
	            INT_32 offset
	            ) = 0;


    virtual INT_32 setNormalMode(void) =0;
    virtual INT_32 getRadarStatus(void) =0;
    virtual CHAR * getRFICType(void) =0;
    virtual INT_32 readBBCRStatistic(
        UINT_16* ed,
        UINT_16* osd,
        UINT_16* sq1,
        UINT_16* sfd,
        UINT_16* fcs) =0;

    virtual INT_32 readTxPowerFromEEPromEx(
        INT_32 channelFreq,
        INT_32 rate,
        INT_32 *nTxPwr,
        INT_32 *outputPower,
        INT_32 *targetAlc) =0;

    virtual INT_32 getEEPRomSize(INT_32 * eepromSz) =0;
    virtual INT_32 getSpecEEPRomSize(INT_32 * eepromSz) =0;
    virtual INT_32 setDACOffsetToEEProm(UINT_16 dacOffset) =0;
    virtual INT_32 getDACOffset(UINT_16* dacOffset) =0;
    virtual INT_32 setChipType(UINT_32 u4ChipType)=0;
    virtual INT_32 setAnritsu8860bTestSupportEn ( INT_32    i4Enable )=0;
protected:
    //For OID operation
    INT_32 ReadEepDataFromFile(TCHAR* szFileName, UINT_16* value, INT_32 nMaxItem);
    INT_32 ReadDPDParaFromFile(TCHAR* szFileName, UINT_32* value, INT_32 nMaxItem);

    //fifi_add
    INT_32 setCCKPreambleType(INT_32 preamble);

    UINT_32 BBCRSave[8];
    UINT_32 ioRegSave[3];
    UINT_32 PPMCRSave;
    INT_32 m_bEnableContiPktTx;

    CHAR rfType[256];
public:
	INT_32 setOID(UINT_32 oid, CHAR *buf, UINT_32 bufLen);
    INT_32 queryOID(UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite);

};

}

#endif //_COID_H_
