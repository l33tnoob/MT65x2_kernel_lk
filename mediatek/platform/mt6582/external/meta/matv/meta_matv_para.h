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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_matv_para.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   define the struct for Meta MATV
 *
 * Author:
 * -------
 *  Siyang.Miao (MTK80734)
 *
 ****************************************************************************/
#ifndef __META_MATV_PARA_H_
#define __META_MATV_PARA_H_

#include "FT_Public.h"
#include "meta_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __FT_PRIVATE_H__
typedef struct 
{
	kal_uint32	freq; //khz
	kal_uint8	sndsys;	/* reference sv_const.h, TV_AUD_SYS_T ...*/
	kal_uint8	colsys;	/* reference sv_const.h, SV_CS_PAL_N, SV_CS_PAL,SV_CS_NTSC358...*/
	kal_uint8	flag;
} matv_ch_entry;
#endif

/* MATV Operation enumeration */
typedef enum 
{ 
	FT_MATV_OP_POWER_ON = 0 
	,FT_MATV_OP_POWER_OFF 
	,FT_MATV_OP_SET_REGION 
	,FT_MATV_OP_SCAN 
	,FT_MATV_OP_STOP_SCAN
    ,FT_MATV_OP_GET_CHANNEL_LIST
    ,FT_MATV_OP_CHANGE_CHANNEL
    ,FT_MATV_OP_SET_CHANNEL_PROPERTY
    ,FT_MATV_OP_GET_CHANNEL_QUALITY
    ,FT_MATV_OP_GET_CHANNEL_QUALITY_ALL
    ,FT_MATV_OP_GET_CHIPNAME
    ,FT_MATV_OP_END 
} FT_MATV_CMD_TYPE;

typedef struct 
{ 	
	kal_uint8        		m_ucChannel;
    matv_ch_entry	m_rmatv_ch_entry; 
} FT_MATV_SET_CHANNEL_PROPERTY_REQ_T;

typedef union 
{ 
	kal_uint8	m_ucRegion;
    kal_uint8	m_ucScanMode;
    kal_uint8	m_ucChannel;
    kal_uint8	m_ucItem;
    FT_MATV_SET_CHANNEL_PROPERTY_REQ_T m_rSetChannelProperty; 
} FT_MATV_CMD_U;

typedef struct 
{ 
	FT_H         		header;
    FT_MATV_CMD_TYPE	type;
    FT_MATV_CMD_U		cmd; 
} FT_MATV_REQ;

typedef struct 
{ 
	kal_uint8        		m_ucChannels;
    matv_ch_entry	m_rmatv_ch_entry[70]; 
} FT_MATV_GET_CHANNEL_LIST_CNF_T;

typedef struct 
{ 
	kal_int32	m_i4QualityIndex[128]; 
} FT_MATV_GET_CHANNEL_QUALITY_ALL_CNF_T;

typedef union 
{ 
	kal_int32 m_i4QualityIndex; 
	kal_uint8 m_ucProgress; 
	char chipname[20];
} FT_MATV_CNF_U;

typedef struct
{ 
	FT_H           		header;
    FT_MATV_CMD_TYPE	type;
    kal_uint32         		status;
    FT_MATV_CNF_U  		result; 
} FT_MATV_CNF;

void META_MATV_OP(FT_MATV_REQ *req) ;

#ifdef __cplusplus
}
#endif

#endif
