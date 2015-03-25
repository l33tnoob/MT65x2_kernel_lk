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

#include "meta_adc.h"
#include "WM2Linux.h"

#define TEST_PMIC_PRINT 	_IO('k', 0)
#define PMIC_READ 				_IOW('k', 1, int)
#define PMIC_WRITE 				_IOW('k', 2, int)
#define SET_PMIC_LCDBK 		_IOW('k', 3, int)
#define ADC_CHANNEL_READ 	_IOW('k', 4, int)
#define GET_CHANNEL_NUM    	_IOW('k', 5, int)
#define ADC_EFUSE_ENABLE    _IOW('k', 6, int)
//add for meta tool-----------------------------------------
#define Get_META_BAT_VOL _IOW('k', 10, int) 
#define Get_META_BAT_SOC _IOW('k', 11, int) 
//add for meta tool-----------------------------------------



int meta_adc_fd=0;

/*Input : ChannelNUm, Counts*/ 
/*Output : Sum, Result (0:success, 1:failed)*/
int adc_in_data[2] = {1,1}; 

int adc_out_data[2] = {1,1}; 

void Meta_AUXADC_OP(AUXADC_REQ *req, char *peer_buff, unsigned short peer_len)
{
	AUXADC_CNF ADCMetaReturn;
	int ret;

	memset(&ADCMetaReturn, 0, sizeof(ADCMetaReturn));

	ADCMetaReturn.header.id=req->header.id+1;
	ADCMetaReturn.header.token=req->header.token;
	ADCMetaReturn.status=META_SUCCESS;
	ADCMetaReturn.ADCStatus=TRUE;
	
	/* open file */
	meta_adc_fd = open("/dev/mtk-adc-cali",O_RDWR, 0);
	if (meta_adc_fd == -1) {
		printf("Open /dev/mtk-adc-cali : ERROR \n");
		META_LOG("Open /dev/mtk-adc-cali : ERROR \n");
		ADCMetaReturn.status=META_FAILED;
		goto ADC_Finish;
	}
	
	adc_in_data[0] = req->dwChannel;
	adc_in_data[1] = req->dwCount;
	ret = ioctl(meta_adc_fd, ADC_CHANNEL_READ, adc_in_data);
	if (ret == -1)
	{
		ADCMetaReturn.dwData = 0;
		ADCMetaReturn.status = META_FAILED;
	}
	else
	{
		if (adc_in_data[1]==0) { 
			ADCMetaReturn.dwData = adc_in_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}
		else {
			ADCMetaReturn.dwData = 0;
			ADCMetaReturn.status = META_FAILED;
		}
	}

	printf("Meta_AUXADC_OP : CH[%d] in %d times : %d\n", req->dwChannel, req->dwCount, adc_in_data[0]);	
	META_LOG("Meta_AUXADC_OP : CH[%d] in %d times : %d\n", req->dwChannel, req->dwCount, adc_in_data[0]);	
	META_LOG("Meta_AUXADC_OP : CH[%d] in %d times : %d : dwdata\n", req->dwChannel, req->dwCount, ADCMetaReturn.dwData);
	
	close(meta_adc_fd);

ADC_Finish:
	if (false == WriteDataToPC(&ADCMetaReturn,sizeof(ADCMetaReturn),NULL,0)) {
        printf("Meta_AUXADC_OP : WriteDataToPC() fail 2\n");
		META_LOG("Meta_AUXADC_OP : WriteDataToPC() fail 2\n");
    }
	printf("Meta_AUXADC_OP : Finish !\n");
	META_LOG("Meta_AUXADC_OP : Finish !\n");
	
}

void Meta_ADC_OP(ADC_REQ *req, char *peer_buff, unsigned short peer_len)
{
	ADC_CNF ADCMetaReturn;
	int ret;

	memset(&ADCMetaReturn, 0, sizeof(ADCMetaReturn));

	ADCMetaReturn.header.id=req->header.id+1;
	ADCMetaReturn.header.token=req->header.token;
	ADCMetaReturn.status=META_SUCCESS;
	ADCMetaReturn.type=req->type;

	//add for meta tool-----------------------------------------
	meta_adc_fd = open("/dev/MT_pmic_adc_cali",O_RDWR, 0);
	if (meta_adc_fd == -1) {
		printf("Open /dev/MT_pmic_adc_cali : ERROR \n");
		META_LOG("Open /dev/MT_pmic_adc_cali : ERROR \n");
		ADCMetaReturn.status=META_FAILED;
		goto ADC_Finish;
	}
	
	adc_in_data[0] = req->type;
	if(adc_in_data[0] == ADC_OP_BAT_VOL) {
		ret = ioctl(meta_adc_fd, Get_META_BAT_VOL, adc_out_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_bat_vol.vol = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}
	}
	else if(adc_in_data[0] == ADC_OP_BAT_CAPACITY){
		ret = ioctl(meta_adc_fd, Get_META_BAT_SOC, adc_out_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_bat_capacity.capacity = (int)adc_out_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}

	}

	printf("Meta_AUXADC_OP : ADC_OP = %d, %d\n", req->type, adc_out_data[0]);	
	close(meta_adc_fd);
	//add for meta tool-----------------------------------------
	
	/* open file */
/*	meta_adc_fd = open("/dev/mtk-adc-cali",O_RDWR, 0);
	if (meta_adc_fd == -1) {
		printf("Open /dev/mtk-adc-cali : ERROR \n");
		META_LOG("Open /dev/mtk-adc-cali : ERROR \n");
		ADCMetaReturn.status=META_FAILED;
		goto ADC_Finish;
	}
	
	adc_in_data[0] = req->type;
	if(adc_in_data[0] == ADC_OP_GET_CHANNE_NUM) {
		ret = ioctl(meta_adc_fd, GET_CHANNEL_NUM, adc_in_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_channel_num.num = (unsigned int)adc_in_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}
	}
	else if(adc_in_data[0] == ADC_OP_QUERY_EFUSE_CAL_EXIST){
		ret = ioctl(meta_adc_fd, ADC_EFUSE_ENABLE, adc_in_data);
		if (ret == -1)
		{
			ADCMetaReturn.status = META_FAILED;
		}
		else
		{
			ADCMetaReturn.cnf.m_channel_num.num = (unsigned int)adc_in_data[0];
			ADCMetaReturn.status = META_SUCCESS;
		}

	}

	printf("Meta_AUXADC_OP : ADC_OP = %d, %d\n", req->type, adc_in_data[0]);	
	close(meta_adc_fd);    */

ADC_Finish:
	if (false == WriteDataToPC(&ADCMetaReturn,sizeof(ADCMetaReturn),NULL,0)) {
        printf("Meta_AUXADC_OP : WriteDataToPC() fail 2\n");
		META_LOG("Meta_AUXADC_OP : WriteDataToPC() fail 2\n");
    }
	printf("Meta_AUXADC_OP : Finish !\n");
	META_LOG("Meta_AUXADC_OP : Finish !\n");	
}

BOOL Meta_AUXADC_Init(void)
{
	return true;
}

BOOL Meta_AUXADC_Deinit(void)
{
	return true;
}

