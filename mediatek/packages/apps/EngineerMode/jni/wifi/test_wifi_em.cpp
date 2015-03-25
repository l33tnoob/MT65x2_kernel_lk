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

#include "WiFi_EM_API.h"
#include "type.h"
#include "dbg.h"

/*for Unit Test of WiFi EM*/

namespace android{

#if 1
void txGo()
{
	UINT_32	u4TxGain, u4Antenna, 
		u4TxPktLen, u4TxPktCnt;
	INT_32 i4RateSel = 212000;
	BOOL fgAlcEn = true;


		// TX gain
		CWrapper::setATParam(2, u4TxGain);

		// Rate
		CWrapper::setATParam(3, i4RateSel);

		// Preamble (fixed to 0)
		CWrapper::setATParam(4, 0);

		// Antenna
		CWrapper::setATParam(5, u4Antenna);

		// Length
		CWrapper::setATParam(6, u4TxPktLen);

        // Count
		CWrapper::setATParam(7, u4TxPktCnt);

		// interval
		CWrapper::setATParam(8, 0);

        // ALC
		CWrapper::setATParam(9, fgAlcEn);

		// TXOP
		CWrapper::setATParam(10, 0x00020000);

		//Packet Content
		CWrapper::setATParam(12, 0xff220004);
		CWrapper::setATParam(12, 0x33440006);
		CWrapper::setATParam(12, 0x55660008);
		CWrapper::setATParam(12, 0x55550019);
		CWrapper::setATParam(12, 0xaaaa001b);
		CWrapper::setATParam(12, 0xbbbb001d);

		// Retrylimit
		CWrapper::setATParam(13, 1);

		// Queue
		CWrapper::setATParam(14, 2);

		// Enable TX
		CWrapper::setATParam(1, 1);

}

void rxGo()
{
	
}
#endif

//const char* CWrapper::m_wrapperID = "uninitialized";
CAdapter *CWrapper::m_adapter = NULL;
COID  *CWrapper::m_oid = NULL;
char	CWrapper::m_wrapperID[NAMESIZE];

extern "C" int main(int argc, char *argv[])
{
	const char* str = "local";
//	CWrapper *cw = new CWrapper(str);
	UINT_32	val;
	INT_32	ret;
	INT_32 	select = 0;
	
	CWrapper::Initialize(str);

	while(1){
		printf("1:Enter test mode\n");
		printf("2:Enter normal mode\n");
		printf("3:Enter standby mode\n");
		printf("4:set EEPROM size to 512\n");
		printf("5:setEEPRomFromFile /data/eeprom.mobile\n");
		printf("6:setEEPromCKSUpdated\n");
		printf("7:getPacketTxStatusEx\n");
		printf("8:getPacketRxStatus\n");
		printf("9:setOutputPower\n");
		printf("10:setLocalFrequecy\n");
		printf("11:setCarrierSuppression\n");
		printf("12:setChannel\n");
		printf("14:writeEEPRom16\n");
		printf("15:eepromWriteByteStr\n");
		printf("16:setATParam(13, 1) -retry limit\n");
		printf("19:queryThermoInfo\n");
		printf("20:getEEPRomSize\n");
		printf("22:MCR32 read and write\n");
		printf("24:tx test\n");
		printf("25:rx test\n");
		printf("26:unInitialize\n");
		
		
		printf("Please Input Your the Selection of Test:\n");
		
		scanf("%d", &select);

		if(select > 24){
			printf("Incorrect  input.\n");
			break;
		}	

		ret = -1;
		switch(select){		
		case 1:		
		ret = CWrapper::setTestMode();
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 2:
		ret = CWrapper::setNormalMode();
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 3:
		ret = CWrapper::setStandBy();
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 4:
		ret = CWrapper::setEEPRomSize(512);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 5:
		ret = CWrapper::setEEPRomFromFile ("/data/eeprom.mobile");
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
#if 0
		INT_32 i4TxPwrGain, i4Eirp, i4ThermoVal;
		ret = CWrapper::readTxPowerFromEEPromEx (chnlList[6].chnlFreq,
						rateSetting[0].i4RateCfg, 
						&i4TxPwrGain, &i4Eirp, &i4ThermoVal);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %s failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "i4TxPwrGain %d, i4Eirp %d, i4ThermoVal %d \n",
						i4TxPwrGain, i4Eirp, i4ThermoVal);
#endif
		case 6:
		ret = CWrapper::setEEPromCKSUpdated ();
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
		
		case 7:
		{
		INT_32 i4SentCount, i4AckCount, i4Alc, i4CckGainControl, i4OfdmGainControl;
		ret = CWrapper::getPacketTxStatusEx ( &i4SentCount, &i4AckCount, &i4Alc, &i4CckGainControl, &i4OfdmGainControl);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "i4SentCount %d, i4AckCount %d, i4AckCount %d, i4Alc %d, i4CckGainControl %d, i4OfdmGainControl %d\n",
					i4SentCount, i4AckCount, i4AckCount, i4Alc, 
					i4CckGainControl, i4OfdmGainControl);
		}
		break;

		case 8:
		INT_32 i4RxOk, i4RxCrcErr;
		ret =  CWrapper::getPacketRxStatus (&i4RxOk, &i4RxCrcErr);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "getPacketRxStatus: i4RxOk is %d, i4RxCrcErr is %d\n", i4RxOk, i4RxCrcErr);
		break;

		case 9:
		ret = CWrapper::setOutputPower ( rateSetting[0].i4RateCfg,
									20, 0);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
		
		case 10:
		ret =  CWrapper::setLocalFrequecy (20, 0);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
	
		case 11:
		ret = CWrapper::setCarrierSuppression ( 0, 20, 0);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
	/*	ret = CWrapper::setOperatingCountry ( INT_8* acChregDomain);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %s failed\n", __FILE__, __LINE__);

		cancel, not used
	*/	case 12:	
		
ret = CWrapper::setChannel ( chnlList[6].chnlFreq);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
	/*	ret = CWrapper::getSupportedRates ( UINT_16 *pu2RateBuf, INT_32 i4MaxNum);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %s failed\n", __FILE__, __LINE__);
		cancel, not used
	*/
		case 13:
		ret = CWrapper::setOutputPin ( 20, 0);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 14:
		{

		ret =  CWrapper::writeEEPRom16 ( 100, 0x55);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		/*u4Offset  < 256*/

		UINT_32	u4Value = 0; 
		ret = CWrapper::readEEPRom16 ( 100, &u4Value );
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "readEEPRom16  0x%x", u4Value);
		break;
		}
		case 15:
#if 0

		{
		INT_8 str_eeprom[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		ret = CWrapper::eepromWriteByteStr( 0, sizeof(str_eeprom), str_eeprom);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);

	
	memset(str_eeprom, 0, sizeof(str_eeprom));
		ret = CWrapper::eepromReadByteStr(0, sizeof(str_eeprom), str_eeprom);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
		}
#else

		{
		char str_eeprom[9] = "12345678";
		ret = CWrapper::eepromWriteByteStr( 0, strlen(str_eeprom) / 2, (INT_8 *)str_eeprom);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);

	
	memset(str_eeprom, 0, sizeof(str_eeprom));
		ret = CWrapper::eepromReadByteStr(0, 4, (INT_8 *)str_eeprom);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		em_printf(MSG_DEBUG, "case 15 : ");
		em_dump(str_eeprom, strlen(str_eeprom));
		break;
		}

#endif
		case 16:
		ret = CWrapper::setATParam(13, 1);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "setATParam successed.\n");
		
		val = 0;
		ret = CWrapper::getATParam(13, &val);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "getATParam return %d\n", val);
		break;

		case 17:
		ret =  CWrapper::setXtalTrimToCr (20);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		UINT_32 u4Value;
		ret = CWrapper::getXtalTrimToCr(&u4Value);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "getXtalTrimToCr %d\n", u4Value);		
		break;
		
		case 18:
		ret = CWrapper::setThermoEn (1);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;

		case 19:
		INT_32 i4AlcEn;
		ret = CWrapper::queryThermoInfo (&i4AlcEn, NULL);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "queryThermoInfo %d\n", i4AlcEn);
		break;
		
		case 20:
		{
		INT_32 i4EepromSz = 0;
		ret = CWrapper::getEEPRomSize(&i4EepromSz);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "getEEPRomSize  i4EepromSz is %d", i4EepromSz);
		break;
		}
		case 21:
		{
		ret = CWrapper::setPnpPower (ParamDeviceStateD0);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
		}
		case 22:
		{
		UINT_32 mcr_value = 0;
		ret = CWrapper::readMCR32(MCR_ICCR2, &mcr_value);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "readMCR32 MCR_ICCR2 0x%x\n", mcr_value);

		ret = CWrapper::writeMCR32(MCR_ICCR2, 1);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);

		ret = CWrapper::readMCR32(MCR_ICCR2, &mcr_value);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		else
			em_printf(MSG_DEBUG, "readMCR32 MCR_ICCR2 0x%x\n", mcr_value);
		break;
		}
		case 23:
		ret = CWrapper::setAnritsu8860bTestSupportEn (1);
		if(ERROR_RFTEST_SUCCESS != ret)
			em_printf(MSG_ERROR, "%s %d failed\n", __FILE__, __LINE__);
		break;
		case 24:
			txGo();
		break;
		case 25:
			rxGo();
		break;
		case 26:
//			delete cw;
//			return 0;
			CWrapper::Uninitialize();
			return 0;

		default:
		break;
		}
		printf("curren loop ret = %d\n", ret);
	}

	return 0;
}

}
