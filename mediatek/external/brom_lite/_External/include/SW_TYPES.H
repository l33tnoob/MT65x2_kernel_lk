/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2003
 *
 *******************************************************************************/

/*******************************************************************************
 * Filename:
 * ---------
 *   sw_types.h
 *
 * Project:
 * --------
 *   Download Agent
 *
 * Description:
 * ------------
 *   Common type and structure definition
 *
 * Author:
 * -------
 *   Amos Hsu
 *
 *******************************************************************************/

#ifndef SW_TYPES_H
#define SW_TYPES_H


/*
 * general definitions
 */

typedef signed char    int8;
typedef signed short   int16;
typedef signed int    int32;
typedef signed int     intx;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
#ifdef WIN32
    typedef unsigned long uint64;
#else
    typedef unsigned long long uint64;
#endif
typedef unsigned int   uintx;
typedef unsigned char  U8;
typedef unsigned short U16;
typedef unsigned int  U32;
#ifdef _MSC_VER
    typedef unsigned __int64  U64;
#else
    typedef unsigned long long U64;
#endif
typedef int32          S32;
typedef unsigned int   Ux;
typedef unsigned char*  P_U8;
typedef unsigned short* P_U16;
typedef unsigned int*   P_U32;
typedef unsigned int*   P_Ux;
typedef unsigned long   ADDRESS_TYPE;


#ifndef __cplusplus
typedef unsigned char  bool;
#endif

/*
 * Definitions for BOOLEAN
 */

#define FALSE          0
#define TRUE           1

/*
 * Definitions for NULL
 */
#ifndef NULL
#define NULL           0
#endif

/*
 * For GFH library
 */
#if defined(_MSC_VER)
#define __WIN32_STDCALL   __stdcall
#else
#define __WIN32_STDCALL
#endif

#endif  /* SW_TYPES_H */

