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
#include <sys/ioctl.h>
#include <WM2Linux.h>

#include "meta_pmic.h"
#include "WM2Linux.h"

#define TEST_PMIC_PRINT 	_IO('k', 0)
#define PMIC_READ 			_IOW('k', 1, int)
#define PMIC_WRITE 			_IOW('k', 2, int)

int pmic_in_data[2] = {0x0,0x0}; /* cmd, data*/
int pmic_out_data[2] = {0x0,0x0}; /* data */

static int g_pmic_power_down[0x7D]={
	0,0x8,0x2,0,0,0,0,0,0x40,0,0,0x40,0,0,0,0,
	0,0,0,0,0,0,0,0,0x2,0x4,0x4,0,0,0,0,0,
	0,0,0,8,0,0,0,0,0,0xc,0,0,0,0,0,0x20,
	0,0,0,0,0,0,0,0,0,0x08,0,0,0,0,0,0x17,
	0,0,0,0,0,0,0,0,0,0,0,0xf,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0xa0,0,0x2,0,0,
	0xa0,0,0x2,0,0,0,0,0,0,0,0,0,0,0,0,0x0,
	0x80,0x3f,0,0,0,0,0,0,0,0,0,0,2
};

static int g_pmic_power_up[0x7D]={
	0,0x8,0x2,0,0,0,0,0,0x40,0,0,0x40,0,0,0,0,
	0,0,0,0,0,0,0,0,0x2,0x4,0x4,0,0,0,0,0,
	0,0,0,8,0,0,0,0,0,0xc,0,0,0,0,0,0x20,
	0,0,0,0,0,0,0,0,0,0x08,0,0,0,0,0,0x17,
	0,0,0,0,0,0x40,0x10,0x10,0,0,0,0xf,0,0x3f,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0xa0,0,0x2,0,0,
	0xa0,0,0x2,0,0,0,0,0,0,0,0,0,0,0,0,0x0,
	0x80,0x3f,0,0,0,0,0,0,0,0,0,0,2
};

int meta_pmic_fd =0;

FT_PMIC_REG_READ_CNF META_PMICR_OP ( FT_PMIC_REG_READ  *req )
{
	FT_PMIC_REG_READ_CNF pmic_cnf;
	int ret=0;

	memset(&pmic_cnf, 0, sizeof(FT_PMIC_REG_READ_CNF));	

	/* open file */
	meta_pmic_fd = open("/dev/MT6326-pmic",O_RDWR, 0);
	if (meta_pmic_fd == -1) {
		printf("Open /dev/MT6326-pmic : ERROR \n");
		META_LOG("Open /dev/MT6326-pmic : ERROR \n");
		pmic_cnf.m_rWmCmdReadResult.value = 0xff;
		pmic_cnf.m_rWmCmdReadResult.status = false;
		return pmic_cnf;
	}	

	pmic_out_data[0] = req->m_rWmCmdReadReq.registernumber;
	ret = ioctl(meta_pmic_fd, PMIC_READ, pmic_out_data);
	if (ret == -1)
	{
		pmic_cnf.m_rWmCmdReadResult.value = 0xff;
		pmic_cnf.m_rWmCmdReadResult.status = false;
	}
	else
	{
		pmic_cnf.m_rWmCmdReadResult.value = pmic_out_data[0];
		pmic_cnf.m_rWmCmdReadResult.status = true;
	}

	printf("\nMETA_PMICR_OP : Read Reg[%d] = %d \n", req->m_rWmCmdReadReq.registernumber, pmic_cnf.m_rWmCmdReadResult.value);
	META_LOG("\nMETA_PMICR_OP : Read Reg[%d] = %d \n", req->m_rWmCmdReadReq.registernumber, pmic_cnf.m_rWmCmdReadResult.value);

	close(meta_pmic_fd);

	return pmic_cnf;
}

FT_PMIC_REG_WRITE_CNF META_PMICW_OP ( FT_PMIC_REG_WRITE  *req )
{
	FT_PMIC_REG_WRITE_CNF pmic_cnf;
	int ret=0;

	memset(&pmic_cnf, 0, sizeof(FT_PMIC_REG_WRITE_CNF)); 

	/* open file */
	meta_pmic_fd = open("/dev/MT6326-pmic",O_RDWR, 0);
	if (meta_pmic_fd == -1) {
		printf("Open /dev/MT6326-pmic : ERROR \n");
		META_LOG("Open /dev/MT6326-pmic : ERROR \n");
		pmic_cnf.m_rWmCmdWriteResult.status = false;
		return pmic_cnf;
	}

	pmic_in_data[0] = req->m_rWmCmdWriteReq.registernumber;
	pmic_in_data[1] = req->m_rWmCmdWriteReq.registervalue;
	ret = ioctl(meta_pmic_fd, PMIC_WRITE, pmic_in_data);
	if (ret == -1)
	{
		pmic_cnf.m_rWmCmdWriteResult.status = false;
	}
	else
	{
		pmic_cnf.m_rWmCmdWriteResult.status = true;
	}
	
	printf("\nMETA_PMICW_OP : Write %x to Reg[%x]\n", pmic_in_data[1], pmic_in_data[0]);
	META_LOG("\nMETA_PMICW_OP : Write %x to Reg[%x]\n", pmic_in_data[1], pmic_in_data[0]);

	close(meta_pmic_fd);

	return pmic_cnf;
}

BOOL Meta_Pmic_Init()
{	
	return true;
}

BOOL Meta_Pmic_Deinit()
{	
	return true;
}

FT_LOW_POWER_CNF META_LOW_POWER_OP ( FT_LOW_POWER_REQ  *req )
{
	FT_LOW_POWER_CNF low_power_cnf;
	int ret=0;
	int tmp=0;

	printf("META_LOW_POWER_OP : START!\n");
	META_LOG("META_LOW_POWER_OP : START!\n");
		
	memset(&low_power_cnf, 0, sizeof(FT_LOW_POWER_CNF)); 
	low_power_cnf.header.id=req->header.id+1;
	low_power_cnf.header.token=req->header.token;
	low_power_cnf.status=META_SUCCESS;
	low_power_cnf.type=req->type;

	/* open file */
	meta_pmic_fd = open("/dev/MT6326-pmic",O_RDWR, 0);
	if (meta_pmic_fd == -1) {
		printf("Open /dev/MT6326-pmic : ERROR \n");
		META_LOG("Open /dev/MT6326-pmic : ERROR \n");
		low_power_cnf.power_status = LOW_POWER_FAILED;
		return low_power_cnf;				
	}

	switch(req->type)
	{
		case WM_CMD_POWER_DOWN:
		{
			//printf("META_LOW_POWER_OP : WM_CMD_POWER_DOWN!\n");	
			for(tmp=0;tmp<0x7D;tmp++){
				pmic_in_data[0] = (0x1A + tmp);
				pmic_in_data[1] = g_pmic_power_down[tmp];
				ret = ioctl(meta_pmic_fd, PMIC_WRITE, pmic_in_data);
				if (ret == -1)
				{
					low_power_cnf.power_status = LOW_POWER_FAILED;
					return low_power_cnf;				
				}
			}
			low_power_cnf.power_status = LOW_POWER_SUCCESS;
		}
		break;
		case WM_CMD_POWER_UP:
		{
			//printf("META_LOW_POWER_OP : WM_CMD_POWER_UP!\n");		
			for(tmp=0;tmp<0x7D;tmp++){
				pmic_in_data[0] = (0x1A + tmp);
				pmic_in_data[1] = g_pmic_power_up[tmp];
				ret = ioctl(meta_pmic_fd, PMIC_WRITE, pmic_in_data);
				if (ret == -1)
				{
					low_power_cnf.power_status = LOW_POWER_FAILED;
					return low_power_cnf;				
				}
			}
			low_power_cnf.power_status = LOW_POWER_SUCCESS;
		}
		break;
	}

	printf("META_LOW_POWER_OP : FINISH!\n");
	META_LOG("META_LOW_POWER_OP : FINISH!\n");

	close(meta_pmic_fd);

	return low_power_cnf;
}

BOOL FM_Low_Power(BOOL bOn)
{
	int tmp=0;
	int ret=0;
	BOOL dPmicErrCode=false;

	printf("Enter FM_Low_Power function in Factory Mode!\n");
	META_LOG("Enter FM_Low_Power function in Factory Mode!\n");

	/* open file */
	meta_pmic_fd = open("/dev/MT6326-pmic",O_RDWR, 0);
	if (meta_pmic_fd == -1) {
		printf("Open /dev/MT6326-pmic : ERROR \n");
		META_LOG("Open /dev/MT6326-pmic : ERROR \n");
		return dPmicErrCode;				
	}

	for(tmp=0;tmp<0x7D;tmp++){
		if(bOn)
		{
			pmic_in_data[0] = (0x1A + tmp);
			pmic_in_data[1] = g_pmic_power_up[tmp];
			ret = ioctl(meta_pmic_fd, PMIC_WRITE, pmic_in_data);
			if (ret == -1)
				dPmicErrCode = false;
			else
				dPmicErrCode = true;
			
			//printf("FM_Low_Power : Power Up Device in Factory Mode!\n");
		}
		else
		{
			pmic_in_data[0] = (0x1A + tmp);
			pmic_in_data[1] = g_pmic_power_down[tmp];
			ret = ioctl(meta_pmic_fd, PMIC_WRITE, pmic_in_data);
			if (ret == -1)
				dPmicErrCode = false;
			else
				dPmicErrCode = true;
			
			//printf("FM_Low_Power : Power Done Device in Factory Mode!\n");
		}
		if(dPmicErrCode == false)
		{
			printf("FM_Low_Power : Write Register Failed!\n");
			META_LOG("FM_Low_Power : Write Register Failed!\n");
			break;
		}
	}

	printf("Leave FM_Low_Power function in Factory Mode!\n");
	META_LOG("Leave FM_Low_Power function in Factory Mode!\n");

	close(meta_pmic_fd);
	
	return dPmicErrCode;
}

