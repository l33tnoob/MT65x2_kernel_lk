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

//*****************************************************************************
// [File] MTK_Type.h
// [Version] v1.0
// [Revision Date] 2008-03-31
// [Author] YC Chien, yc.chien@mediatek.com, 21558
// [Description]
//*****************************************************************************

#ifndef MTK_TYPE_H
#define MTK_TYPE_H


typedef unsigned char           MTK_UINT8;
typedef signed char             MTK_INT8;

typedef unsigned short int      MTK_UINT16;
typedef signed short int        MTK_INT16;

typedef unsigned int            MTK_UINT32;
typedef signed int              MTK_INT32;

typedef unsigned long           MTK_UINT64;
typedef signed long             MTK_INT64;


typedef enum 
{
    MTK_FALSE = 0,
    MTK_TRUE = 1
}   MTK_BOOL;

typedef unsigned int MTK_FILE;

typedef enum
{
    MTK_FS_READ = 0,     // open file for reading (r)
    MTK_FS_WRITE,        // create file for writing, discard previous contents if any (w)
    MTK_FS_APPEND,       // open or create file for writing at end of file (a)
    MTK_FS_RW,           // open file for reading and writing (r+)
    MTK_FS_RW_DISCARD,   // create file for reading and writing, discard previous contents if any (w+)
    MTK_FS_RW_APPEND     // open or create file for reading and writing at end of file (a+)
}   MTK_FMODE;

typedef enum
{
    MTK_FS_SEEK_SET = 0, // seek from beginning of file
    MTK_FS_SEEK_CUR,     // seek from current position
    MTK_FS_SEEK_END      // seek from end of file
}   MTK_FSEEK;

typedef struct _MTK_TIME
{
    MTK_UINT16  Year;
    MTK_UINT16  Month;
    MTK_UINT16  Day;
    MTK_UINT16  Hour;
    MTK_UINT16  Min;
    MTK_UINT16  Sec;
    MTK_UINT16  Msec;
}   MTK_TIME;

typedef enum
{
  HSDBGT_INIT = 0,
  HSDBGT_LIB,
  HSDBGT_KER,
  HSDBGT_CAL,
  HSDBGT_SYS,
  HSDBGT_COMM,
  HSDBGT_MSG,
  HSDBGT_STR,
  HSDBGT_MEM,
  HSDBGT_ALL,
  HSDBGT_END
} MTK_DEBUG_TYPE;

typedef enum
{
  HSDBGL_NONE = 0,
  HSDBGL_ERR,
  HSDBGL_WRN,
  HSDBGL_INFO1,
  HSDBGL_END
} MTK_DEBUG_LEVEL;

#define MTK_BEE_VOID void
#define MTK_BEE_INT int

#endif /* MTK_TYPE_H */
