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
** $Id: CAdapter.cpp,v 1.5 2008/12/17 13:06:16 MTK01425 Exp $
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
** $Log: CAdapter.cpp,v $
 *
 * 11 16 2012 ben.niu
 * Add CTIA test setting/getting
 *
 * 03 01 2012 ben.niu
 * [ALPS00242156] Get chip support channels bug: EM Can't support for more 5GHz channels.
 * Change array copy size from "64" to "wrq.u.data.length*sizeof(uint32_t)"
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
** Revision 1.5  2008/12/17 13:06:16  MTK01425
** Add setOID and queryOID
**
** Revision 1.4  2008/11/11 03:15:05  MTK01725
** Change an OID name.
**
** Revision 1.3  2008/10/28 13:45:42  MTK01725
** Frog add IO Pin Test and other little modification.
**
** Revision 1.2  2008/06/04 08:35:22  MTK01385
** 1. add setPnpPower().
**
** Revision 1.1  2008/05/26 14:04:36  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.5  2008/04/29 01:08:07  MTK01385
** 1. Modify function description.
**
** Revision 1.4  2008/02/23 14:58:52  MTK01385
** 1. fix input byte length in eeprom write byte string.
**
** Revision 1.3  2008/02/22 14:34:09  MTK01385
** 1. Adapter add EEPROM byte string read/write feature.
**
** Revision 1.2  2008/02/22 13:16:12  MTK01385
** 1. Add support to script to burn EEPROM with specific file
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.6  2007/11/12 06:25:03  MTK01267
** move readMCR32, writeMCR32, readBBCR8, writeBBCR8,
** readEEPRom16, writeEEPRom16 to each chip folder.
**
** Revision 1.5  2007/11/12 02:58:09  MTK01267
** change wince_dll\ to wince\
**
** Revision 1.4  2007/11/12 02:44:56  MTK01267
** Support wince
**
** Revision 1.3  2007/11/09 04:05:39  MTK01267
** refine code
**
** Revision 1.2  2007/10/17 09:29:46  MTK01267
** remove DAC function to mt5911, mt5912
**
** Revision 1.1  2007/10/16 06:58:50  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/


#include <sys/socket.h>
#include "COid.h"
#include "param.h"
#include "type.h"


namespace android{

CAdapter::CAdapter(const char* AdapterName, const char* Desc, COID * oid, const char* wrapperName) :
m_referCnt(0)
{
#if HAVE_ANDROID_OS
	m_OID = oid;
	strcpy(ProtocolName, AdapterName);


	//Name = AdapterName.ws2s() + string("@") +  Wrapper.getName();
	strcpy(Name, AdapterName);
	strcat(Name, "@");
	strcpy(Name, wrapperName);

#else
	Site = HostSite;
	m_OID = oid;
	ProtocolName = AdapterName;

#if defined (UNDER_CE)
	//Name = Desc;
   	char Temp[256];

    WideCharToMultiByte( CP_ACP, 0, Desc.c_str(), -1, Temp, 256, NULL, NULL );
    //sprintf(Name, "%s@%s", Temp, Site->GetName());
	Name = string(Temp) + string("@") + Site->GetName();

#else
	TCHAR Temp[256];
	WideCharToMultiByte( CP_ACP, 0, Desc.c_str(), -1, Temp, 256, NULL, NULL );
	Name = string(Temp) + _T("@") + Site->GetName();
#endif
#endif
}

CAdapter::~CAdapter(void)
{
    m_OID = NULL;
}

//=[testFunc]===================================================================

/*******************************************************************************
**  getChannelBand
**
**  descriptions: Return RF band and channel number by given frequency
**  parameters:
**      freq - Frequency
**      channel - output for channel number
**      band - output for band
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::getChannelBand (INT_32 freq, INT_32* channel, INT_32* band)
{
    static const TChnFreq dsConfigValues[] =
        {{0, 1, 2412000}, /* 2.4G from CH1 ~ 14*/
        {0, 2, 2417000},
        {0, 3, 2422000},
        {0, 4, 2427000},
        {0, 5, 2432000},
        {0, 6, 2437000},
        {0, 7, 2442000},
        {0, 8, 2447000},
        {0, 9, 2452000},
        {0, 10, 2457000},
        {0, 11, 2462000},
        {0, 12, 2467000},
        {0, 13, 2472000},
        {0, 14, 2484000},
        {1, 8,    5040000 }, /* 5G */
        {1, 12,   5060000 },
        {1, 16,   5080000 },
        /* US U-NII lower band (5.15-5.25 GHz) */
        {1, 34,   5170000 },
        {1, 36,   5180000 },
        {1, 38,   5190000 },
        {1, 40,   5200000 },
        {1, 42,   5210000 },
        {1, 44,   5220000 },
        {1, 46,   5230000 },
        {1, 48,   5240000 },
        /* US U-NII middle band (5.25-5.35 GHz) */
        {1, 52,   5260000 },
        {1, 56,   5280000 },
        {1, 60,   5300000 },
        {1, 64,   5320000 },
        /* CEPT 5.47-5.725 GHz band */
        {1, 100,  5500000 },
        {1, 104,  5520000 },
        {1, 108,  5540000 },
        {1, 112,  5560000 },
        {1, 116,  5580000 },
        {1, 120,  5600000 },
        {1, 124,  5620000 },
        {1, 128,  5640000 },
        {1, 132,  5660000 },
        {1, 136,  5680000 },
        {1, 140,  5700000 },
        /* US U-NII upper band (5.725-5.825 GHz) */
        {1, 149,  5745000 },
        {1, 153,  5765000 },
        {1, 157,  5785000 },
        {1, 161,  5805000 },
        /* Japan 4.9 GHz band */
        {1, 240,  4920000 },
        {1, 244,  4940000 },
        {1, 248,  4960000 },
        {1, 252,  4980000 }
        };

    /* Successful */
    for (unsigned int j = 0 ; j < sizeof(dsConfigValues)/sizeof(TChnFreq); j++) {
        if(dsConfigValues[j].freq == freq) {
            *band = dsConfigValues[j].band;
            *channel = dsConfigValues[j].channelNum;
            return 0;
        }
    }

    return -1;
}



/*_________________________________________________________________________
**  ReadEepDataFromFile
**
**  descriptions: read the file contained eeprom data and save it to ushort array
**  parameters:
**         szFileName: a string indicate the file to be read
**         nMaxItem:   Max available buffer item
**  output:
**         value:      output buffer
**  return:
**         negative, if fail
**         zero or more, number of return items in value
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::ReadEepDataFromFile(TCHAR* szFileName, UINT_16* value, INT_32 nMaxItem)
{
    INT_32 nByteNum = 0;
    FILE* file;
    INT_32 i = 0 ;

    

#if defined(UNICODE)
	DEBUGFUNC("ReadEepDataFromFile with UNICODE\n");
    TCHAR output[256];
    _stprintf(output, TEXT("file %s max:%d\n"), szFileName, nMaxItem);
    ERRORLOG((output));

    file = _tfopen(szFileName, TEXT("r"));
    if(file == NULL){
        ERRORLOG((TEXT("Open failed\n")));
        return -1;
    }

    UINT_16 eepromValue;
    INT_32 status;

    while(1){
        status = _ftscanf(file, _T("%04x"), (unsigned int *)&eepromValue);

        if(status == EOF)
            break;

        if(status == 0){
            continue;
        }
        value[i] = eepromValue;
        i ++;
    }

    fclose(file);
    /*
    int j;
    for( j = 0 ; j < i ; j ++){
        _stprintf(output, TEXT("%d:%04x-%04x-%04x-%04x\n"),
            j,
            value[j], value[j + 1], value[j + 2], value[j + 3]);
        j += 4;
    }
    */
#else
	DEBUGFUNC("ReadEepDataFromFile without UNICODE\n");
    char strBuf[512];
    UINT_32 eepValue;

    file = fopen(szFileName, "r");
    if(file == NULL)
        return -1;

    while(fscanf(file, "%04c", strBuf) != EOF && i < nMaxItem) {
    	//em_error("ReadEepDataFromFile:offset = %d, string = %s", i, strBuf);
        eepValue = strtol(strBuf,NULL,16);
	//em_error("ReadEepDataFromFile:offset = %d, value = 0x%04x", i, eepValue);
        value[i] = (UINT_16)eepValue;
        i++;
    }

    fclose(file);
#endif
    return i;
}

/*_________________________________________________________________________
**  ReadDPDParaFromFile
**
**  descriptions: read the file contained eeprom data and save it to ushort array
**  parameters:
**         szFileName: a string indicate the file to be read
**         nMaxItem:   Max available buffer item
**  output:
**         value:      output buffer
**  return:
**         negative, if fail
**         zero or more, number of return items in value
**  note: for MT6620 only
**__________________________________________________________________________*/
INT_32
CAdapter::ReadDPDParaFromFile(TCHAR* szFileName, UINT_32* value, INT_32 nMaxItem)
{
    INT_32 nByteNum = 0;
    FILE* file;
    INT_32 i = 0 ;

    
	em_printf(MSG_DEBUG, (char*)"file %s max:%d\n", szFileName, nMaxItem);
#if defined(UNICODE)
	DEBUGFUNC("ReadDPDParaFromFile with UNICODE\n");
    TCHAR output[256];
    _stprintf(output, TEXT("file %s max:%d\n"), szFileName, nMaxItem);
    ERRORLOG((output));

    file = _tfopen(szFileName, TEXT("r"));
    if(file == NULL){
        ERRORLOG((TEXT("Open failed\n")));
        return -1;
    }

    UINT_32 dpdValue;
    INT_32 status;

    while(1){
        status = _ftscanf(file, _T("%04x"), (unsigned int *)&dpdValue);

        if(status == EOF)
            break;

        if(status == 0){
            continue;
        }
        value[i] = dpdValue;
        i ++;
		if(i >= nMaxItem)
		{
		//data in file exceeds nMaxItem, if we donot break under this circustomtance, system will crash
			break;
		}
    }

    fclose(file);
    /*
    int j;
    for( j = 0 ; j < i ; j ++){
        _stprintf(output, TEXT("%d:%04x-%04x-%04x-%04x\n"),
            j,
            value[j], value[j + 1], value[j + 2], value[j + 3]);
        j += 4;
    }
    */
#else
	DEBUGFUNC("ReadDPDParaFromFile without UNICODE\n");
    char strBuf[256];
    UINT_32 eepValue;

    file = fopen(szFileName, "r");
    if(file == NULL)
        return -1;

    while(fscanf(file, "%s", strBuf) != EOF && i < nMaxItem) {
    	em_printf(MSG_DEBUG, (char*)"strBuf = %s", strBuf);
        eepValue = strtol(strBuf,NULL,16);

        value[i] = eepValue;
        i++;
    }

    fclose(file);
#endif
    return i;
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
CAdapter::setPacketTxEx (
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
	DEBUGFUNC("CAdapter::setPacketTxEx");
	INITLOG((_T("\n")));

	INT_32 i, result;
	TX_PACKET_STRUC m_TxPacket;

    PARAM_RATES m_TxRate;

    if (bufSize == 0) { // help to read packet content from file
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    for (i = 0 ; i < bufSize; i++) {
        m_TxPacket.pktContent[i] = (CHAR)szBuf[i];
    }

    m_TxPacket.pktLength = (UINT)bufSize;
    m_TxPacket.pktCount = pktCount;
    m_TxPacket.pktInterval = pktInterval;
    m_TxPacket.txAnt = (char)txAntenna; /* For compability reason */
	m_TxPacket.targetAlc = 0;
    m_TxPacket.reserved[0] = 0;
    m_TxPacket.reserved[1] = 0;
	m_TxPacket.targetAlc = 0;
    memset(m_TxRate, 0, sizeof(PARAM_RATES));

    m_TxRate[0] = (UINT_8)txRate;

    if ( !m_OID->setOID(this,
			OID_802_11_DESIRED_RATES,
			(CHAR *)m_TxRate,
			sizeof(PARAM_RATES))
			) {
        ERRORLOG((TEXT("Failed to set Tx rate")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    UINT_32	txAnt = txAntenna;

    if ( !m_OID->setOID(this,
			OID_802_11_TX_ANTENNA_SELECTED,
			(CHAR *)&txAnt,
			sizeof(UINT_32))
			) {
        ERRORLOG((TEXT("Failed to set Tx antenna")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    Sleep(200);

    if (bLongPreamble) {
        result = setCCKPreambleType(phyPreambleTypeLong);
        if(result != ERROR_RFTEST_SUCCESS)
            return result;
    }
    else {
        result = setCCKPreambleType(phyPreambleTypeShort);
        if(result != ERROR_RFTEST_SUCCESS)
            return result;
    }

    if (bGainControl) {
    	m_TxPacket.txGain = (char)gainControl;
    	m_TxPacket.txFlags &= ~TX_PACKET_FLAG_NO_TXGAIN;
    }
	else {
    	m_TxPacket.txGain = 0xFF;
    	m_TxPacket.txFlags |= TX_PACKET_FLAG_NO_TXGAIN;
    }

    if (bTrackAlc) {
    	m_TxPacket.txFlags |= TX_PACKET_FLAG_ENABLE_ALC_TRACK;
    }
	else {
        m_TxPacket.txFlags &= ~TX_PACKET_FLAG_ENABLE_ALC_TRACK;
    }

    if (bTargetAlc) {
    	m_TxPacket.txFlags |= TX_PACKET_FLAG_TARGET_ALC_PROVIDE;
    	m_TxPacket.targetAlc = targetAlcValue;
    }
	else {
        m_TxPacket.txFlags &= ~TX_PACKET_FLAG_TARGET_ALC_PROVIDE;
    }

    if ( !m_OID->setOID(this,
			OID_IPC_TEST_PACKET_TX,
			(CHAR *)&m_TxPacket,
			sizeof(TX_PACKET_STRUC))
			) {
        ERRORLOG((TEXT("Failed to set tx packet")));

        /* Driver is old version */
        if (bGainControl && (bTargetAlc == FALSE) && (bTrackAlc == FALSE)) {
            TX_PACKET_V0_STRUC oldPacket;

            memcpy(&oldPacket, &m_TxPacket, sizeof(TX_PACKET_V0_STRUC));
            if ( !m_OID->setOID(this,
					OID_IPC_TEST_PACKET_TX,
					(CHAR *)&oldPacket,
					sizeof(TX_PACKET_V0_STRUC))
					) {
                return ERROR_RFTEST_NDIS_OID_FAILURE;
            }
        }
		else {
            return ERROR_RFTEST_NDIS_OID_FAILURE;
        }
    }

    m_bEnableContiPktTx = TRUE;

    return 0;
}
#endif
/*******************************************************************************
**  setCCKPreambleType
**
**  descriptions: This setup preamble type
**  parameters:
**      nCardIndex - card index
**      preamble - IPC_PHY_PREAMBLE_TYPE
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
#if 0
INT_32
CAdapter::setCCKPreambleType(INT_32 preamble)
{

    if ( !m_OID->setOID(this,
			OID_IPC_PHY_PREAMBLE_TYPE,
			(CHAR *)&preamble,
			sizeof(preamble))
			) {
        ERRORLOG((TEXT("Failed to set OID_802_11_TX_ANTENNA_SELECTED")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    return ERROR_RFTEST_SUCCESS;
}
#endif
/*_________________________________________________________________________
**  getPacketTxStatusEx
**
**  descriptions: Query Tx status and ALC information
**  parameters:
**         nCardIndex: Which NIC
**         sentCount,   pointer to varable sentCount to record how much packets have been sent
**         ackCount,    pointer to varable sentCount to record how much packets have been sent successfully
**         alc,    pointer to varable average ALC value
**  return:
**         check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::getPacketTxStatusEx (
    long *sentCount,
    long *ackCount,
    long *alc,
    long *cckGainControl,
    long *ofdmGainControl)
{
	UINT_32 BytesRead;
    TX_STATUS_STRUC m_TxStatus;
    //char  acDbgMsg[200];

	if ( !m_OID->queryOID(this,
			OID_IPC_TEST_TX_STATUS,
			(CHAR *)&m_TxStatus,
			sizeof(TX_STATUS_STRUC),
			&BytesRead )
			) {
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	else {
#if 0
        *sentCount = 0 + m_TxStatus.pktSentCount;
        *ackCount  = 0 + m_TxStatus.pktSentStatus;
        *alc       = 0 + m_TxStatus.avgAlc;
        *cckGainControl = 0 + m_TxStatus.cckGainControl;
        *ofdmGainControl = 0 + m_TxStatus.ofdmGainControl;
#endif
        *sentCount = m_TxStatus.pktSentCount;
        *ackCount  = m_TxStatus.pktSentStatus;
        *alc       = m_TxStatus.avgAlc;
        *cckGainControl = m_TxStatus.cckGainControl;
        *ofdmGainControl = m_TxStatus.ofdmGainControl;

        /*/sprintf(acDbgMsg, "send = %d, ok=%d, alc= %d, cck=0x%x, ofdm=0x%x, byteNeed=%d\n",\
//             m_TxStatus.pktSentCount, m_TxStatus.pktSentStatus, m_TxStatus.avgAlc,\
//           m_TxStatus.cckGainControl, m_TxStatus.ofdmGainControl, BytesRead);
        //ERRORLOG((TEXT(acDbgMsg)));
        //ERRORLOG((TEXT("send = %d, ok=%d, alc= %d, cck=0x%x, ofdm=0x%x\n"),\
        //   m_TxStatus.pktSentCount, m_TxStatus.pktSentStatus, m_TxStatus.avgAlc,\
        //   m_TxStatus.cckGainControl, m_TxStatus.ofdmGainControl));*/

    }

    return ERROR_RFTEST_SUCCESS;
}



/*_________________________________________________________________________
**  setOperatingCountry
**
**  descriptions: Set current Country information
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::setOperatingCountry (
	CHAR *chregDomain
	)
{
	COUNTRY_STRING_ENTRY m_regDomain;

    // The code here is strange....frog.
	memcpy(m_regDomain.countryCode, chregDomain, sizeof(chregDomain));
	memcpy(m_regDomain.environmentCode, " ", sizeof(m_regDomain.environmentCode));

	if (!m_OID->setOID(this,
			OID_IPC_CURRENT_COUNTRY,
			(CHAR *)&m_regDomain,
			sizeof(COUNTRY_STRING_ENTRY))
			) {
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}
	else {
		return ERROR_RFTEST_SUCCESS;
	}

}

/*_________________________________________________________________________
**  getChannelList
**
**  descriptions: Issue Query Channel List OID to retrive supported channel
**                frequency in unit of kHz
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::getChannelList (
	UINT_32* frenqBuf,
	INT_32 maxNum
	)
{
	UINT_32 BytesRead;
	INT_32 m_bQueryNewChannelListOid = FALSE;
	INT_32 numChannel;

	if (frenqBuf == NULL) {
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	PIPN_SUPPORTED_CHANNEL_FREQUENCY_LIST m_ChannelList2;

	BytesRead = 1024;

	m_ChannelList2 = (PIPN_SUPPORTED_CHANNEL_FREQUENCY_LIST) new CHAR[BytesRead];
	if ( !m_OID->queryOID(this,
			OID_IPN_TEST_CHANNEL_FREQUENCY_LIST,
			(CHAR*)m_ChannelList2,
			BytesRead,
			&BytesRead)
			) {
		m_bQueryNewChannelListOid = FALSE;
	}
	else {
		memset(frenqBuf, 0, sizeof(UINT_32) * maxNum);

		int i;
		m_bQueryNewChannelListOid = TRUE;
		numChannel = m_ChannelList2->numberOfItems;
		for (i = 0; i < (int)m_ChannelList2->numberOfItems && i < maxNum; i++) {
			frenqBuf[i] = m_ChannelList2->channelFreqInkHz[i];
		}
	}

	if (m_bQueryNewChannelListOid) {
		return ERROR_RFTEST_SUCCESS;
	}
	else {
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

}

INT_32
CAdapter::getSupportedRates (
	UINT_16* outputRates,
	INT_32 maxNum
	)
{
    UINT_8 result[16];
    UINT_32 BytesReturned;

	if ( !m_OID->queryOID(this,
	        OID_802_11_SUPPORTED_RATES,
	        (CHAR*)result,
	        sizeof(result), &BytesReturned)
	        ) {
        ERRORLOG("Failed to radio testing");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }


	for (int i = 0 ; (i < maxNum) && (i < 16) ; i++) {
		outputRates[i] = (UINT_16)result[i];
	}
	return ERROR_RFTEST_SUCCESS;

}



/*******************************************************************************
**  setLowPowerTest
**
**  descriptions: setup trigger condition
**  parameters:
**      nCardIndex - card index
**      triggerDuration - System time between event setup and trigger
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::setLowPowerTest (
	INT_32 triggerDuration
	)
{
    TEST_LOW_POWER_EVENT event;

	event.length = sizeof(TEST_LOW_POWER_EVENT);
    event.triggerTimeOut = (triggerDuration * 1000)/1024; /* In unit of TU */

    if ( !m_OID->setOID(this,
			OID_CUSTOM_TEST_LOW_POWER_CAP,
			(CHAR *)&event,
			sizeof(TEST_LOW_POWER_EVENT))
			) {
        ERRORLOG("Failed to set radio off");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  getLowPowerTestResult
**
**  descriptions: Return trigger result
**  parameters:
**      nCardIndex - card index
**  result:
**      triggerd - Whether the low power event is generated.
**      triggeredDuration - System time between event setup and trigger
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::getLowPowerTestResult (
    INT_32* triggerd,
    INT_32* triggeredDuration
    )
{
    UINT_32 BytesRead;
    TEST_LOW_POWER_EVENT event;

	event.length = sizeof(TEST_LOW_POWER_EVENT);

    if(!m_OID->queryOID(this,
			OID_CUSTOM_TEST_LOW_POWER_CAP,
			(CHAR *)&event,
			sizeof(TEST_LOW_POWER_EVENT),
			&BytesRead)
			) {
        ERRORLOG("Failed to query OID_CUSTOM_TEST_LOW_POWER_CAP");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    *triggerd = event.triggered;

    if (event.triggered) {
        if (event.triggeredSysTime >= event.enabledSysTime) {
            *triggeredDuration = event.triggeredSysTime - event.enabledSysTime;
        }
        else {
            *triggeredDuration = 0xFFFFFFFF -
                (event.enabledSysTime - event.triggeredSysTime);
        }
    }

    return ERROR_RFTEST_SUCCESS;

}



/*_________________________________________________________________________
**  getMACAddr
**
**  descriptions: Return MAC address information maintained in MIB
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::getMACAddr (
	UINT_8 * szMACAddr
	)
{
    CHAR MACaddress[6];
    UINT_32 BytesRead;

	if (szMACAddr == NULL) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }

    if (!m_OID->queryOID(this,
			OID_802_3_CURRENT_ADDRESS,
        	MACaddress,
        	6,
        	&BytesRead) && BytesRead == 6
        	) {
        return ERROR_RFTEST_GENERAL_ERROR;
    }
	else {
        memcpy(szMACAddr, MACaddress, 6);
        return ERROR_RFTEST_SUCCESS;
    }
}

/*_________________________________________________________________________
**  getSupportedCountries
**
**  descriptions: Return supported countries information maintained in MIB
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::getSupportedCountries (
	COUNTRY_CODE* countries,
	INT_32 maxNum
	)
{
    PIPC_SUPPORTED_DOMAINS supportedDomain_p;
    PIPC_DOMAIN_INFORMATION country_p;
    //UINT_32 oidBufLen = 4096;
    UINT_32 oidBufLen = 1024; //modify for remote testing

	supportedDomain_p = (PIPC_SUPPORTED_DOMAINS)(new UINT_8[oidBufLen]);
	if (supportedDomain_p == NULL) {
		return ERROR_RFTEST_GENERAL_ERROR;
	}

	country_p = supportedDomain_p->domain;

	if(!m_OID->queryOID(this,
			OID_IPC_SUPPORTED_DOMAINS,
			(CHAR*)supportedDomain_p,
			oidBufLen,
			&oidBufLen)
			) {
		delete supportedDomain_p;
		return ERROR_RFTEST_GENERAL_ERROR;
	}
	else {
		int i;

		for (i =0; (i < (int)supportedDomain_p->numberOfItems) && (i < maxNum); i++) {
			memcpy(countries[i].countryCode, country_p[i].countryCode, 2);
		}

		delete supportedDomain_p;
		return ERROR_RFTEST_SUCCESS;
	}

}

/*******************************************************************************
**  setPowerManagementState
**
**  descriptions: setup WLAN function enable/disable
**  parameters:
**      nCardIndex - card index
**      state - On/off state
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::setPowerManagementState (
	POWER_MANAGEMENT_STATE state
	)
{
    UINT_32 newState;

	if (state == PM_FULLY_AWAKE) {
		newState = 1;
	}
	else {
		newState = 0;
	}

	if ( !m_OID->setOID(this,
			OID_IPC_SW_RADIO_ON_OFF_STATE,
			(CHAR *)&newState,
			sizeof(newState))
			) {
		ERRORLOG((char*)"Failed to set radio off");
		return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	if (state == PM_FULLY_AWAKE) {
		/* Because Radio On/Off causes driver to enter MAC_IDLE state. */
		return setTestMode();
	}

	return ERROR_RFTEST_SUCCESS;

}

/*_________________________________________________________________________
**  getOperatingCountry
**
**  descriptions: Get current Country information
**  parameters:
**  result:
**  return:
**              check ERROR_RFTEST_XXXX
**  note:
**__________________________________________________________________________*/
INT_32
CAdapter::getOperatingCountry (
	CHAR* country
	)
{
    COUNTRY_STRING_ENTRY m_regDomain;
    UINT_32 bytes;

	if (!m_OID->queryOID(this,
		    OID_IPC_CURRENT_COUNTRY,
		    (CHAR *)&m_regDomain,
		    sizeof(COUNTRY_STRING_ENTRY), &bytes)
		    ) {
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	else {
        country[0] = m_regDomain.countryCode[0];
        country[1] = m_regDomain.countryCode[1];
        return ERROR_RFTEST_SUCCESS;
    }
}

/*******************************************************************************
**  setRDSCounterSet
**
**  descriptions: T
**  parameters:
**      ucPara0 ~ u4Para7 - see doc??
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
#if 0 //fifi_add
INT_32
CAdapter::setRDSCounterSet (
    IN  UINT_8  ucPara0,
    IN  UINT_8  ucPara1,
    IN  UINT_8  ucPara2,
    IN  UINT_8  ucPara3,
    IN  UINT_32 u4Para4,
    IN  UINT_32 u4Para5,
    IN  UINT_32 u4Para6,
    IN  UINT_32 u4Para7
    )
{
	IPC_RDS_MEASURE_STRUC rRDSMeasure;

	rRDSMeasure.ucPara0 = ucPara0;
    rRDSMeasure.ucPara1 = ucPara1;
    rRDSMeasure.ucPara2 = ucPara2;
    rRDSMeasure.ucPara3 = ucPara3;
    rRDSMeasure.u4Para4 = u4Para4;
    rRDSMeasure.u4Para5 = u4Para5;
    rRDSMeasure.u4Para6 = u4Para6;
    rRDSMeasure.u4Para7 = u4Para7;

	if ( !m_OID->setOID(this,
			OID_IPC_RDS_MEASURE,
			(CHAR *)&rRDSMeasure,
			sizeof(rRDSMeasure))
			) {
		ERRORLOG((TEXT("Failed to set OID_IPC_RDS_MEASURE")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
	}

	return ERROR_RFTEST_SUCCESS;
}
#endif

/*******************************************************************************
**  getRDSCounterSet
**
**  descriptions: T
**  parameters:
**      ucPara0 ~ u4Para7 - see doc??
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
#if 0 //fifi_add
INT_32
CAdapter::getRDSCounterSet (
    IN  UINT_8      ucPara0,
    OUT UINT_8*     pucPara1,
    OUT UINT_8*     pucPara2,
    OUT UINT_8*     pucPara3,
    OUT UINT_32*    pu4Para4,
    OUT UINT_32*    pu4Para5,
    OUT UINT_32*    pu4Para6,
    OUT UINT_32*    pu4Para7
    )
{
	DWORD BytesRead;
	IPC_RDS_MEASURE_STRUC rRDSMeasure;

	memset(&rRDSMeasure, 0, sizeof(IPC_RDS_MEASURE_STRUC));

    rRDSMeasure.ucPara0 = ucPara0;
    rRDSMeasure.ucPara1 = *pucPara1;

	if (!m_OID->queryOID(this,
		    OID_IPC_RDS_MEASURE,
	        (CHAR *)&rRDSMeasure,
	        sizeof(IPC_RDS_MEASURE_STRUC),
	        &BytesRead)
		    ) {
        ERRORLOG((TEXT("Failed to read OID_IPC_RDS_MEASURE")));
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	else {
		*pucPara1 = rRDSMeasure.ucPara1;
        *pucPara2 = rRDSMeasure.ucPara2;
        *pucPara3 = rRDSMeasure.ucPara3;
        *pu4Para4 = rRDSMeasure.u4Para4;
        *pu4Para5 = rRDSMeasure.u4Para5;
        *pu4Para6 = rRDSMeasure.u4Para6;
        *pu4Para7 = rRDSMeasure.u4Para7;
        return ERROR_RFTEST_SUCCESS;
	}

	return ERROR_RFTEST_SUCCESS;
}
#endif

/*******************************************************************************
**  eepromReadByteStr
**
**  descriptions: read eeprom byte string 
**  parameters:
**      u4Addr   - EEPROM byte offset
**      u4Lenght - byte length
**      pszStr -   pointer to string buffer
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::eepromReadByteStr (
	UINT_32 u4Addr,
    UINT_32 u4Length,
    char *pszStr
	)
{
    UINT_32 u4StartAddrWD, u4EdAddrWD;
    UINT_32 u4I;
    char szTmp[512]; //Use local cstring to do string append, delete jobs, or it always show NULL.
    char    acTmp[8];//sprintf will disturb 4 bytes data for each parameters.
    UINT_32 u4Head;
    UINT_32 u4Tail;
	INT_32  ret = 0;

	if(u4Length == 0 || pszStr == NULL){
		em_printf(MSG_ERROR,(char*)"The verification of %s failed", __FUNCTION__);
		return -1;
	}
	
    
    u4StartAddrWD = u4Addr / 2;
    u4EdAddrWD = (u4Addr + u4Length -1) /2;
    
    memset(szTmp, 0, sizeof(szTmp));
    u4Head = 0;
    u4Tail = 0;
    /*1. read word boundary string*/
    for(u4I = u4StartAddrWD; u4I <= u4EdAddrWD;  u4I++) {
        EEPROM_RD_DWORD2BYTE_STR( u4I, (&acTmp[0]));
		em_printf(MSG_DEBUG, (char*)"%s", __FUNCTION__);
        em_dump(acTmp, 8)
        memcpy(&szTmp[u4Tail], acTmp, 4 );
        u4Tail +=4;
    }

    //DBGPRINT(("2EEPROM byte read offset 0x%04X and length = %d, %s\n", u4Addr, u4Length, szTmp));


    /*2. check start address is odd, start from 0 */
    if( u4Addr & 0x00000001) {
        u4Head += 2;

    }
    /*/DBGPRINT(("3EEPROM byte read offset 0x%04X and length = %d, %s\n", u4Addr, u4Length, \
    //    szTmp));*/

    /*3. check end address is even */
    if( (( u4Addr+u4Length -1) & 0x00000001) == 0x00000000) {
        u4Tail -=2;

    }

    /*/DBGPRINT(("4EEPROM byte read offset 0x%04X and length = %d, %s\n", u4Addr, u4Length, \
    //    szTmp));*/


    /*4. add '/0 to end of string */
    szTmp[u4Tail] = '\0';
    memcpy(pszStr, &szTmp[u4Head], (u4Tail-u4Head+1) );

	return ret;

}

/*******************************************************************************
**  eepromWriteByteStr
**
**  descriptions: write eeprom byte string 
**  parameters:
**      u4Addr   - EEPROM byte offset
**      u4Lenght - byte length
**      pszStr -   pointer to string buffer
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32 
CAdapter::eepromWriteByteStr (
	UINT_32 u4Addr,
    UINT_32 u4Length,
    char *pszStr
	)
{
    UINT_32 u4StartAddrWD, u4EdAddrWD;
    UINT_32 u4I;
    char szTmp[512]; //Use local string buffer.
    UINT_32 u4Tmp, u4Tmp1;
//    UINT_16 u2Tmp;
    UINT_16 u2StrByteCnt;
    char    acTmp[4];
	INT_32	ret = 0;
 

    /*/DBGPRINT(("1EEPROM byte write offset 0x%04X and length = %d, %s\n", u4Addr, u4Length, \
    //    m_szEepromByteStrValue));*/
	if(u4Length == 0 || pszStr == NULL){
		em_printf(MSG_ERROR,(char*)"The verification of %s failed", __FUNCTION__);
		return -1;
	}

	em_printf(MSG_DEBUG, (char*)"%s", __FUNCTION__);
	em_dump(pszStr, u4Length);
    u4StartAddrWD = u4Addr / 2;
    u4EdAddrWD = (u4Addr + u4Length -1) /2;


    u4I = u4StartAddrWD;
    u2StrByteCnt = 0;

    memset(szTmp, 0, sizeof(szTmp));
    strcpy(szTmp, pszStr);

    //DBGPRINT(("lpsz = %s\n", lpsz));
	if(u4Length == 0 || pszStr == NULL){
		em_printf(MSG_ERROR, (char*)"The verification of %s failed.", __FUNCTION__);
		return -1;
	}

    do {
        /*1. check start address is odd*/
        if( u4Addr & 0x00000001) {
            readEEPRom16(u4I, &u4Tmp);
            sscanf(szTmp, "%02X", (unsigned int *)&u4Tmp1);
            //DBGPRINT(("1 u4Tmp1 = 0x%02X\n", u4Tmp1));

            u4Tmp = (u4Tmp & 0x000000FF) | ((u4Tmp1)<<8) &0x0000FF00;
            ret += writeEEPRom16(u4I, u4Tmp);
            u4I++;
            u2StrByteCnt +=2;
            if(u4Length == 1) {
                break;
            }

        }

        /*2. middle dw*/
        for(; u4I < u4EdAddrWD;  u4I++) {

            acTmp[0]= szTmp[u2StrByteCnt];
            acTmp[1]= szTmp[u2StrByteCnt+1];
            acTmp[2]= szTmp[u2StrByteCnt+2];
            acTmp[3]= szTmp[u2StrByteCnt+3];
            //acTmp[4] = '\0'; //warning: array subscript is above array bounds
            //DBGPRINT(("1 actmp  = %s\n", acTmp));

            EEPROM_WR_BYTE_STR_2_DWORD(u4I, acTmp);
            u2StrByteCnt+=4;
        }
        /*3. check end address is even */
        if( (( u4Addr+u4Length -1) & 0x00000001) == 0x00000000) {

            ret += readEEPRom16(u4I, &u4Tmp);
            sscanf(&szTmp[u2StrByteCnt], "%02X", (unsigned int *)&u4Tmp1);
            //DBGPRINT(("2 u4Tmp1 = 0x%02X\n", u4Tmp1));
            u4Tmp = (u4Tmp & 0x0000FF00) | ((u4Tmp1)<<0) &0x000000FF;
            writeEEPRom16(u4I, u4Tmp);
            
            u4I++;
            u2StrByteCnt+=2;


        }
        else{
            acTmp[0]= szTmp[u2StrByteCnt];
            acTmp[1]= szTmp[u2StrByteCnt+1];
            acTmp[2]= szTmp[u2StrByteCnt+2];
            acTmp[3]= szTmp[u2StrByteCnt+3];
            //DBGPRINT(("2 actmp  = %s\n", acTmp)); 
            EEPROM_WR_BYTE_STR_2_DWORD(u4I, acTmp);
            u2StrByteCnt+=4;
        }
    }while(false);

	return ret;
}

/*******************************************************************************
**  setPnpPower
**
**  descriptions: This setup PNP power state
**  parameters:
**      i4PowerMode - D0, full on. D3, power off.
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::setPnpPower(INT_32 i4PowerMode)
{
    PARAM_DEVICE_POWER_STATE rPowerState = (PARAM_DEVICE_POWER_STATE)i4PowerMode;

    if ( !m_OID->setOID(this,
			OID_PNP_SET_POWER,
			(CHAR*)&i4PowerMode,
			sizeof(PARAM_DEVICE_POWER_STATE))
			) {
        ERRORLOG((char*)"Failed to set OID_PNP_SET_POWER");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }

    return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  getSupportChannelList
**
**  descriptions: Get wifi chip supported channels currently
**  parameters:
**      pau4Channel - pointer to list that save the channels.
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::getSupportChannelList(UINT_32 *pau4Channel)
{
    struct iwreq wrq;
    int i, skfd, ret = 0;
    uint32_t au4ChannelList[64];
    /* initialize socket */
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0) {
        em_error((char*)"socket(AF_INET, SOCK_DGRAM)");
        return -1;
    }
    /* initialize WEXT request */
    wrq.u.data.pointer = &(au4ChannelList[0]);
    wrq.u.data.length = sizeof(uint32_t)*64;
    wrq.u.data.flags = PRIV_CMD_GET_CH_LIST;
    strncpy(wrq.ifr_name, "wlan0", IFNAMSIZ);
    /* do IOCTL */
    if (ioctl(skfd, IOCTL_GET_INT, &wrq) < 0) {
        em_error((char*)"%s ioctl failed, %s", __FUNCTION__, strerror(errno));
        ret = ERROR_RFTEST_UNSUPPORTED;
        goto exit;
    }
    em_printf(MSG_DEBUG, (char*)"%s:operation succeed", __FUNCTION__);
    /* Then supported channel list will be store in au4ChannelList */
    for (i = 0; i < wrq.u.data.length; i++) {
        printf("(%d) Ch#%d\n", i, au4ChannelList[i]);
        em_printf(MSG_DEBUG, "(%d) Ch#%d\n",  i, au4ChannelList[i]);
    }
    *pau4Channel = wrq.u.data.length;
    memcpy(pau4Channel+1, au4ChannelList, wrq.u.data.length*sizeof(uint32_t));
    ret = ERROR_RFTEST_SUCCESS;
exit:
    close(skfd);
    return ret;
}

/*******************************************************************************
**  sw_cmd
**
**  descriptions: CTIA test setting/getting
**  parameters:
**      set - the setting flag
**      adr - setting/getting address
**      dat - buffer store value
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::sw_cmd(int set, UINT_32 adr, UINT_32 *dat)
{
	em_error((char*)"native ID: %u, value: %u", adr, *dat);
	int skfd, ret = 0;
	int ioctl_dir = IOCTL_SET_STRUCT_FOR_EM;
	struct iwreq iwr;
	SW_CMD_TRANSPORT_STRUCT cmd_in_tmp;
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
	    em_error((char*)"socket(AF_INET, SOCK_DGRAM)");
	    return -1;
	}
	memset(&iwr, 0, sizeof(iwreq));
	strncpy(iwr.ifr_name, "wlan0", IFNAMSIZ);
	iwr.u.data.flags = PRIV_CMD_SET_GET_VALUE;
	cmd_in_tmp.u4CmdId = adr;
	cmd_in_tmp.u4CmdValue = *dat;
	iwr.u.data.pointer = &cmd_in_tmp;
	iwr.u.data.length = sizeof(SW_CMD_TRANSPORT_STRUCT);
	if (!set) {
		ioctl_dir = IOCTL_GET_STRUCT;
	}
	ret = ioctl(skfd, ioctl_dir, &iwr);
	if (set) {
		if (ret < 0) {
			em_error((char*)"%s ioctl set failed, %s", __FUNCTION__, strerror(errno));
			goto exit;
		} else {
			em_error((char*)"%s ioctl set success, %u", __FUNCTION__, *dat);
		}
	} else {
		if (ret < 0) {
			em_error((char*)"%s ioctl get failed, %s", __FUNCTION__, strerror(errno));
			goto exit;
		} else {
			*dat = cmd_in_tmp.u4CmdValue;
			em_error((char*)"%s ioctl get success, %u", __FUNCTION__, *dat);
		}
	}
	em_error((char*)"cmd_in_tmp: %u, %u", cmd_in_tmp.u4CmdId, cmd_in_tmp.u4CmdValue);
exit:
    close(skfd);
    return ret;
}

#if 0

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
CAdapter::testIOPin (
    UINT_32 u4ModuleIndex
	)
{
	return ERROR_RFTEST_SUCCESS;
}
#endif
/*******************************************************************************
**  setOID
**
**  descriptions: Set OID command into the adapter
**  parameters:
**      oid - The OID IO command
**      buf - The buffer carries the parameters
**      bufLen - The length of buf
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::setOID(UINT_32 oid, CHAR *buf, UINT_32 bufLen) {

	if ( !m_OID->setOID(this,
			oid,
			buf,
			bufLen)
			) {
        ERRORLOG((char*)"Failed to set OID_PNP_SET_POWER");
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	return ERROR_RFTEST_SUCCESS;
}

/*******************************************************************************
**  queryOID
**
**  descriptions: Query OID to get information from the adapter
**  parameters:
**      oid - The OID IO command
**      buf - The buffer carries the response data
**      bufLen - The length of buf
**      bytesRead - The physical data length returned in buf
**  return:
**      0 - success negative fails
**  note:
*******************************************************************************/
INT_32
CAdapter::queryOID(UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesRead) {

	if ( !m_OID->queryOID(this,
			oid,
			buf,
			bufLen,
			bytesRead )
			) {
        return ERROR_RFTEST_NDIS_OID_FAILURE;
    }
	return ERROR_RFTEST_SUCCESS;
}

}
