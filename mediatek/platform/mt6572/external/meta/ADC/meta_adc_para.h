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


/*******************************************************************************
 *
 * Filename:
 * ---------
 *   Meta_ADC_Para.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   MTK80198(Chunlei Wang)
 *
 * 
 *
 *******************************************************************************/


#ifndef __META_AUXADC_PARA_H__
#define __META_AUXADC_PARA_H__

#include "FT_Public.h"
 
typedef struct{
	FT_H	    	header;  //module do not need care it
	unsigned char	dwChannel;	// channel 0-8
	unsigned short	dwCount;	// Detect number	
}AUXADC_REQ;

typedef struct{
	FT_H	    header;  //module do not need care it
	//DWORD			dwData;
	int			dwData;
	BOOL			ADCStatus;
	unsigned char	status;
}AUXADC_CNF;

/*the testcase enum define of adc module*/
typedef enum {
	ADC_OP_GET_CHANNE_NUM = 0,		//V 0
	ADC_OP_QUERY_EFUSE_CAL_EXIST,	//V 1
	ADC_OP_BAT_VOL,
	ADC_OP_BAT_CAPACITY,
	ADC_OP_END						//
} ADC_OP;

//=============Request==========================
typedef union {
	unsigned int dummy;
} META_ADC_REQ_U;

typedef struct {
	FT_H header;	//module do not need care it
	ADC_OP type;
	META_ADC_REQ_U 	req;
} ADC_REQ;

//============confirm=================================
typedef struct {
	unsigned int num;		//the number of channel available
} ADC_CHANNEL_NUM_T;

typedef struct {
	unsigned int isExist;	//Exist (1); Not exist(0)
} ADC_EFUSE_CAL_T;

typedef struct {
	int vol;	//battery voltage
} ADC_BAT_VOL;

typedef struct {
	int capacity;	//battery capacity
} ADC_BAT_CAPACITY;

typedef union {
	ADC_CHANNEL_NUM_T m_channel_num;
	ADC_EFUSE_CAL_T	m_efuse_cal;
	ADC_BAT_VOL m_bat_vol;
	ADC_BAT_CAPACITY m_bat_capacity;
	unsigned int dummy;
}META_ADC_CNF_U;

typedef struct {
	FT_H header;	//module do not need care it
	ADC_OP type;
	unsigned int status;
	unsigned int dummy;
	META_ADC_CNF_U cnf;	//fm->FT
} ADC_CNF;

/* please implement this function   */
BOOL 			Meta_AUXADC_Init(void);
void Meta_AUXADC_OP(AUXADC_REQ *req, char *peer_buff, unsigned short peer_len); 
BOOL 			Meta_AUXADC_Deinit(void);
void Meta_ADC_OP(ADC_REQ *req, char *peer_buff, unsigned short peer_len);

#endif
