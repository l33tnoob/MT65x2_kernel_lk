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
 *   Meta_GPIO_Para.h
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
 *   MTK80465(Changlei Gao)

 *******************************************************************************/


#ifndef __META_GPIO_PARA_H__
#define __META_GPIO_PARA_H__

#include "FT_Public.h"
#include "gpio_exp.h"

typedef enum {
	GET_MODE_STA = 0,
	SET_MODE_0,  // 00
	SET_MODE_1,  // 01
	SET_MODE_2,  // 10
	SET_MODE_3,  // 11

	GET_DIR_STA, 
	SET_DIR_IN,  // 0
	SET_DIR_OUT, // 1

	GET_PULLEN_STA,  
	SET_PULLEN_DISABLE,  // 0
	SET_PULLEN_ENABLE,   // 1

	GET_PULL_STA,
	SET_PULL_DOWN,  // 0
	SET_PULL_UP,    // 1

//	GET_INV_STA,
//	SET_INV_ENABLE,  // 1
//	SET_INV_DISABLE, // 0

	GET_DATA_IN,
	GET_DATA_OUT,
	SET_DATA_LOW,  // 0
	SET_DATA_HIGH, // 1
}GPIO_OP;
	
 
typedef struct{
	FT_H	header;  //module do not need care it
	HW_GPIO	pin;	// pin number
	GPIO_OP	op;	// operation to GPIO	
}GPIO_REQ;

typedef struct{
	FT_H	header;  //module do not need care it
	unsigned int	status;
	unsigned int	data;
}GPIO_CNF;


bool Meta_GPIO_Init(void);
GPIO_CNF Meta_GPIO_OP(GPIO_REQ req, unsigned char* peer_buf, unsigned short peer_len); 
bool Meta_GPIO_Deinit(void);

#endif


