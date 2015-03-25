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
** $Id: dbg.h,v 1.1 2008/05/26 14:04:36 MTK01385 Exp $
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
** $Log: dbg.h,v $
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
** Revision 1.1  2008/05/26 14:04:36  MTK01385
** 1. move from WPDNIC root folder to WPDNIC\common
**
** Revision 1.2  2008/02/27 10:27:17  MTK01385
** 1. Disable debug message output.
**
** Revision 1.1.1.1  2007/12/10 07:23:01  MTK01385
** WPDWiFiTool for MT5921
**
** Revision 1.1  2007/10/16 06:58:50  MTK01267
** Initial version
**
** Revision 1.0  2007/08/23 03:46:12  MTK01267
** Initial version
**
*/

#ifndef _DBG_H
#define _DBG_H

#include <android/log.h>
#include <stdarg.h>

namespace android{

/*******************************************************************************
*                     C O M P I L E R   F L A G S
********************************************************************************
*/
#define DBG_TRACE_CALL 1

/*******************************************************************************
*                          C O N S T A N T S
********************************************************************************
*/


/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/
/*******************************************************************************
*                             M A C R O S
********************************************************************************
*/
//#if defined (_DEBUG)

//#include <android/log.h>
//#include <stdarg.h>


//enum MSG{MSG_DUMP, MSG_DEBUG, MSG_INFO, MSG_WARNING, MSG_ERROR};
#define MSG_DUMP   0
#define MSG_DEBUG  1
#define MSG_INFO 	2
#define MSG_WARNING  3
#define MSG_ERROR	4



#define em_debug_level MSG_DUMP


#if DBG_TRACE_CALL

	void android_printf(int level, char *format, ...);
	void em_error(char *format, ...);
	
	
	#define em_printf(level, ...)	\
		do {					\
			if((level) >= MSG_DUMP){	\
				android_printf((level), __VA_ARGS__);		\
			}											\
		}while(0)
	
	
	#define em_dump(setBuffer, bufLen)							\
		{										\
			int i = 0, j = 0, count = 0;						\
			char tmp[500] = {0};							\
			count = bufLen < 80? bufLen : 80;					\
				do{ 								\
					if(i != 0 && i % 8 == 0){				\
						sprintf(tmp + i * 5 + j, "%c", '\n');		\
						j++;						\
					}							\
					sprintf(tmp + i * 5 + j, "0x%02x ", setBuffer[i]);	\
					i++;							\
				}while(i < count );						\
				sprintf(tmp + i * 5 + j, "%c", '\0');				\
				android_printf(MSG_DUMP, "%s", tmp);				\
		}										\
	
	#define DEBUGFUNC(func) 		em_printf(MSG_DEBUG, (char*)(func))
	#define ERRORLOG(str)			em_printf(MSG_ERROR, (str))

#else
	#define em_printf(level, ...)	do{ } while(0)
	#define em_dump(buf, len)		do{ } while(0)
#endif

/*******************************************************************************
*                    D A T A   D E C L A R A T I O N S
********************************************************************************
*/
/*******************************************************************************
*                 F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

}

#endif
