/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "meta_battery.h"
#include "WM2Linux.h"

BOOL Meta_Battery_Init()
{
	printf("Meta_Battery_Init ! \n");
	META_LOG("Meta_Battery_Init ! \n");
	return true;
}

BOOL Meta_Battery_Deinit(){
	printf("Meta_Battery_Deinit ! \n");
	META_LOG("Meta_Battery_Deinit ! \n");
	return true;
}

void Meta_Battery_OP(FT_BATT_REQ* req, BYTE* peer_buf,unsigned short peer_len)
{
	/* dummy code */
	
	FT_BATT_CNF batcnf;
	
	memset(&batcnf, 0, sizeof(batcnf));

	META_LOG("Meta_Battery_OP");
	
	batcnf.header.id=req->header.id+1;
	batcnf.header.token=req->header.token;
	batcnf.status=META_SUCCESS;
	batcnf.type=req->type;
	batcnf.result.m_rWmCmdWriteResult.DL_Status=BAT_FILE_Success;

	if (peer_buf==NULL) {
		printf("peer_buf is NULL \n");
		META_LOG("peer_buf is NULL \n");
		return;
	}

	switch (req->cmd.m_rWmCmdWriteReq.nReqWriteFileStatus)
	{
		case BAT_FILE_ONCE:
			batcnf.result.m_rWmCmdWriteResult.nCnfWriteFileStatus=BAT_FILE_ONCE;
			break;
		case BAT_FILE_START:
			batcnf.result.m_rWmCmdWriteResult.nCnfWriteFileStatus=BAT_FILE_START;
			break;	
		case BAT_FILE_ONGOING:
			batcnf.result.m_rWmCmdWriteResult.nCnfWriteFileStatus=BAT_FILE_ONGOING;
			break;	
		case BAT_FILE_CLOSE:
			batcnf.result.m_rWmCmdWriteResult.nCnfWriteFileStatus=BAT_FILE_CLOSE;
			break;	
	}

	WriteDataToPC(&batcnf, sizeof(batcnf), NULL, 0);	
}

void Meta_Battery_Read_FW(FT_BATT_READ_INFO_REQ *req)
{
	/* dummy code */

	FT_BATT_READ_INFO_CNF batreadcnf;

	memset(&batreadcnf,0, sizeof(batreadcnf));

	META_LOG("Meta_Battery_Read_FW");

	batreadcnf.header.id=req->header.id+1;
	batreadcnf.header.token=req->header.token;
	batreadcnf.status=META_SUCCESS;
	batreadcnf.type=req->type;
	
	batreadcnf.result.m_rWmFwReadResult.Drv_Status=BAT_READ_INFO_SUCCESS;
	batreadcnf.result.m_rWmSocReadResult.Drv_Status=BAT_READ_INFO_SUCCESS;
	batreadcnf.result.m_rWmSocWriteResult.Drv_Status=BAT_READ_INFO_SUCCESS;

	WriteDataToPC(&batreadcnf, sizeof(batreadcnf), NULL, 0);
}

void Meta_Battery_UPdate_FW(FT_BATT_REQ* req,  BYTE* bDataAddress,unsigned short data_number)
{
	/* dummy code */

	FT_BATT_CNF batcnf;

	memset(&batcnf, 0, sizeof(batcnf));

	META_LOG("Meta_Battery_UPdate_FW");

	batcnf.header.id=req->header.id+1;
	batcnf.header.token=req->header.token;
	batcnf.status=META_SUCCESS;
	batcnf.type=req->type;
	
	batcnf.result.m_rWmCmdUpdateResult.nCnfUpdateStatus=BAT_FILE_Success;

	switch (req->cmd.m_rWmCmdUpdateReq.nReqStartStatus)
	{
		case BAT_FILE_ONCE:
			batcnf.result.m_rWmCmdUpdateResult.nCnfStartStatus=BAT_FILE_ONCE;
			break;	
		case BAT_FILE_START:
			batcnf.result.m_rWmCmdUpdateResult.nCnfStartStatus=BAT_FILE_START;
			break;	
		case BAT_FILE_ONGOING:
			batcnf.result.m_rWmCmdUpdateResult.nCnfStartStatus=BAT_FILE_ONGOING;
			break;	
		case BAT_FILE_CLOSE:
			batcnf.result.m_rWmCmdUpdateResult.nCnfStartStatus=BAT_FILE_CLOSE;
			break;	
	}

	WriteDataToPC(&batcnf, sizeof(batcnf), NULL, 0);	
}

