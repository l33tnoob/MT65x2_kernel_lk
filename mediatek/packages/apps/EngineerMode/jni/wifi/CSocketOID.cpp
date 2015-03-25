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
** $Id: CSocketOID.cpp,v 1.1.1.1 2007/12/10 07:23:01 MTK01385 Exp $
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
** $Log: CSocketOID.cpp,v $
 *
 * 09 02 2010 yong.luo
 * [ALPS00123924] [Need Patch] [Volunteer Patch]Engineer mode migrate to 2.2
 * .
 *
 * 06 22 2010 yong.luo
 * [ALPS00006740][Engineering Mode]WiFi feature is not ready on 1024.P3 
 * .
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.1  2007/11/12 03:19:00  MTK01267
** Initial version
**
** Revision 1.3  2007/10/22 06:45:19  MTK01267
** Initial version
**
**
*/

#include "COid.h"
#include "param.h"
//#pragma comment(lib, "wsock32.lib") //for XP winsock
#pragma comment(lib, "ws2.lib") //for CE winsock

typedef enum _NDIS_REQUEST_TYPE_Ex {
    NdisRequestQueryInformation,
    NdisRequestSetInformation,
    NdisRequestQueryStatistics,
    NdisRequestOpen,
    NdisRequestClose,
    NdisRequestSend,
    NdisRequestTransferData,
    NdisRequestReset,
    NdisRequestGeneric1,
    NdisRequestGeneric2,
    NdisRequestGeneric3,
    NdisRequestGeneric4
} NDIS_REQUEST_TYPE_Ex, *PNDIS_REQUEST_TYPE_Ex;

struct OID_DATA_REQUEST
{
	WCHAR_T wAdapterID[128];
	NDIS_REQUEST_TYPE_Ex RequestType;
	UINT_32 Oid;
	UINT_32 InformationBufferLength;
};

struct ADAPTER_INFO
{
	//INT_32	FriendlyNameLen;
	//WCHAR_T FriendlyName[256];
	INT_32	NetCfgInstanceIdLen;
    WCHAR_T NetCfgInstanceId[256];
};

CSocketOID::CSocketOID(CSite *aSite, const CHAR * HostName, UINT_16 HostPort) : COID(aSite)
{
	// Create socket
	m_socket = new Socket(HostName, HostPort);
	if (m_socket == NULL) {
		throw _T("SocketClient failed");
	}
}

CSocketOID::~CSocketOID(void)
{
	if (m_socket != NULL) delete m_socket;
}

INT_32
CSocketOID::EnumAdapter(deque<AdapterName> & adaptersNameList)
{
    //query remote agent to get adapter list

	//1. Send start request
	OID_DATA_REQUEST oidRequest;

	//include: cardIndex, type, oid, len
	memset(oidRequest.wAdapterID, 0, sizeof(oidRequest.wAdapterID));
	oidRequest.RequestType = NdisRequestSetInformation;
	oidRequest.Oid = 0;
	oidRequest.InformationBufferLength = 0;

	if (SendData((CHAR*)&oidRequest, sizeof(OID_DATA_REQUEST)) != sizeof(OID_DATA_REQUEST)) return 0;

	//2. Recv card number
	INT_32 card_num;
	if (RecvData((CHAR*)&card_num, sizeof(INT_32)) != sizeof(INT_32) ) return 0;

	//3. Recv list on this target platform
	AdapterName aName;
	for (int i=0; i<card_num; i++) {

		ADAPTER_INFO a_Adapter;
		if (RecvData((CHAR*)&a_Adapter, sizeof(ADAPTER_INFO)) != sizeof(ADAPTER_INFO) ) {
			return (INT_32)adaptersNameList.size();
		}

		// From CE, NetCfgInstanceId = FriendlyName = MT5912VL1, for example
		aName.Name = a_Adapter.NetCfgInstanceId;
		//aName.Desc = a_Adapter.FriendlyName;
		aName.Desc = aName.Name;
		adaptersNameList.push_back(aName);

	}

	return (INT_32)adaptersNameList.size();
}

BOOL
CSocketOID::GetChipID(UINT_32 & ChipID, const wstring &Name)
{
	UINT_32 Data;
    if (MakeEtherRequest(Name, FALSE, OID_IPC_OID_INTERFACE_VERSION, (CHAR *)&Data, sizeof(Data), NULL)) {
    	ChipID = Data;
    	return TRUE;
    }
    return FALSE;
}

INT_32
CSocketOID::setOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen) {
	// return TRUE or FALSE (not succeed)
    return MakeEtherRequest(aAdapter->GetWstrAdapterID(), TRUE, oid, buf, bufLen, NULL);
}

INT_32
CSocketOID::queryOID(CAdapter *aAdapter, UINT_32 oid, CHAR *buf, UINT_32 bufLen, UINT_32 *bytesWrite) {
	// return TRUE or FALSE (not succeed)
	return MakeEtherRequest(aAdapter->GetWstrAdapterID(), FALSE, oid, buf, bufLen, bytesWrite);
}

UINT_32
CSocketOID::MakeEtherRequest(
	wstring wAdapterID,
	BOOL bSetOid,
	UINT_32 oid,
    CHAR * pBuf,
    UINT_32 bytes,
    UINT_32 * outputBytes)
{
    INT_32 count;
    UINT_32 result = 0;
	UINT_32 BytesReturned = 0;

	// Send request
	char SendBuff[1500];
	
	OID_DATA_REQUEST *pReq = (OID_DATA_REQUEST *) SendBuff;
	pReq->RequestType = bSetOid ? NdisRequestSetInformation : NdisRequestQueryInformation;
	pReq->Oid = oid;
	pReq->InformationBufferLength = bytes;
	memset(pReq->wAdapterID, 0, sizeof(pReq->wAdapterID));
	//wAdapterID._Copy_s(pReq->wAdapterID, sizeof(pReq->wAdapterID)>>1, sizeof(pReq->wAdapterID)>>1);
	wAdapterID = wstring(pReq->wAdapterID);
	
	if (oid != OID_IPC_TEST_PACKET_TX) {
		// make sure SendBuff will not overflow
		if ( bytes > (sizeof(SendBuff)-sizeof(OID_DATA_REQUEST)) ) return FALSE;
		memcpy(SendBuff+sizeof(OID_DATA_REQUEST), pBuf, bytes);
		count = sizeof(OID_DATA_REQUEST)+ bytes;
	}
	else {
		// skip the packet contents filed
		memcpy(SendBuff+sizeof(OID_DATA_REQUEST), (CHAR*)pBuf, sizeof(UINT));
		INT_32 offset = FIELD_OFFSET(TX_PACKET_STRUC, pktStatus);
		memcpy(SendBuff+sizeof(OID_DATA_REQUEST)+sizeof(UINT), (CHAR*)pBuf+offset, bytes - offset);
		count = sizeof(OID_DATA_REQUEST)+ sizeof(UINT)+bytes-offset;
	}
	
	if (SendData(SendBuff, count) != count) return FALSE;

	//Recv response, result = 0 --> fail, else --> success
	UINT_32 oid_num = 0;
	if (RecvData((CHAR*)&oid_num,       sizeof(UINT_32)) != sizeof(UINT_32)) return FALSE;
	if (RecvData((CHAR*)&result,        sizeof(UINT_32)) != sizeof(UINT_32)) return FALSE;
	if (RecvData((CHAR*)&BytesReturned, sizeof(UINT_32)) != sizeof(UINT_32)) return FALSE;
	if (BytesReturned != 0) {
		if (RecvData((CHAR*)pBuf, BytesReturned) != BytesReturned) return FALSE;
	}

	if (outputBytes != NULL) {
		*outputBytes = 0;
		
		if( result == 0 ) *outputBytes = BytesReturned;
	}

    if(result == 0) return FALSE;
    else return TRUE;
}
