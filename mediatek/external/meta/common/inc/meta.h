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
 *   meta.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *    header file of main function
 *
 * Author:
 * -------
 *   Lu.Zhang (MTK80251) 09/11/2009
 *
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 05 11 2010 lu.zhang
 * [ALPS00005327]CCAP 
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 01 20 2010 lu.zhang
 * [ALPS00004332]Create META 
 * .
 *
 * 
 *
 *******************************************************************************/


//
// TST driver.
//


#ifndef __META_H__
#define __META_H__
#include <stdio.h>

//*****************************************************************************
//
//                          META Driver MACRO def
//
//*****************************************************************************
// the name of tst rx message
//#define TSTRXMSG TEXT("TSTRxMsg")
//#define TSTRXMSG 0001
// the name of tst tx message 
//#define TSTTXMSG TEXT("TSTTxMsg")
#define TSTTXMSG "/data/tsttxmsg"	
// the name of ft rx message
//#define FTRXMSG  TEXT("FTRxMsg")
#define FTRXMSG  "/data/ftrxmsg"

#define CCCTTXMSG "/data/cccitxmsg"
// the name of tst tx message
//#define FTTXMSG  TEXT("FTTxMsg")
//#define FTTXMSG  1000



//becuase of the muximum of a frame is 2048, so we have following limitation.
//the maximum size of frame
//Wayne revise
#define FrameMaxSize 4096*16//2048

//the size of peer buf header
#define PeerHeaderlen 8
// the maximum size of peer buf
#define PeerBufMaxlen 4000*8 //2000
// the maximum size of peer buf + local buf
#define FTMaxSize (FrameMaxSize -PeerHeaderlen - 9)

//the size of message queue
#define QUEUE_ENTRIES  16

//mesage len define :  2048 + header(4*3)
//mesage len define :  4096 + header(4*3) 
#define MAX_QUEUELEN    4096*8+4*3 //4108 //2060


// type define, it is for compatiable with feature phone code.
typedef signed char     kal_int8;
typedef signed short    kal_int16;
typedef signed int      kal_int32;
typedef long long       kal_int64;
typedef unsigned char	kal_uint8;
typedef unsigned short	kal_uint16;
typedef unsigned int	kal_uint32;
typedef char            kal_char;

#define kal_bool	BOOL
#define KAL_TRUE	TRUE
#define KAL_FALSE	FALSE


//*****************************************************************************
//
//                          META Driver data structure def
//
//*****************************************************************************



// defie the type of frame.
typedef enum
{
    AP_FRAME =0,	//ap side
    MD_FRAME		//modem side
} META_FRAME_TYPE;


// the data pass between FT and TST
typedef struct
{
    META_FRAME_TYPE eFrameType;	//frame type
    kal_uint8 uData[FrameMaxSize];		//data: local buff + peer buff //wayne update from 2048 to FrameMaxSize
    kal_int16 LocalLen;			//local len
    kal_int16 PeerLen;			//peer len
} META_RX_DATA;



//*****************************************************************************
//
//                          META Driver MACRO def
//
//*****************************************************************************
// log define
//#ifdef SHIP_BUILD
//#define MetaLogMsg(cond,printf_exp) ((cond)?(NKDbgPrintfW printf_exp),1:0)

//#else

//#define MetaLogMsg(cond,printf_exp) RETAILMSG(cond,printf_exp)

//#endif //SHIP_BUILD




//*****************************************************************************
//
//                          META Driver MACRO def
//
//*****************************************************************************




#endif

