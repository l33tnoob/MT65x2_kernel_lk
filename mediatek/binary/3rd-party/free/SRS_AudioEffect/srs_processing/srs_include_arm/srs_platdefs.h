/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
  SRS Labs CONFIDENTIAL
  @Copyright 2009 by SRS Labs.
  All rights reserved.

	(RCS keywords below, do not edit)
	$Id: //srstech/srs_common/std_fxp/include/srs_platdefs.h#11 $
  $Author: oscarh $
  $Date: 2011/02/17 $
  
  Defines various macroes describing platforms (Processors, compilers and DSP extensions)
  
  This file should not contain anything but #defines and no // comments.
  This is because it is used to preprocess many kinds of files.
*/

#ifndef __SRS_PLATDEFS_H__
#define __SRS_PLATDEFS_H__

/*Compiler defs*/
#ifdef _MSC_VER			/*The compiler is MSVC: either Win32 or WinCE*/
	#define SRS_MSVC	1
#else
	#define SRS_MSVC	0
#endif
              
#define SRS_RVCT    defined(__ARMCC_VERSION)          /*The compiler is RVCT, including ADS*/
#define SRS_XCC			defined(__XCC__)				  				/*Tensilica xt-xcc or xt-xc++*/
#define SRS_GCC     (defined(__GNUC__) && !SRS_RVCT && !SRS_XCC)  /*The compiler is general GCC, excluding RVCT GNU mode and xt-xcc*/
#define SRS_CCS   	defined(__TI_COMPILER_VERSION__)  /*The compiler is TI DSP compiler: CCS*/


/*Processor defs*/
#if SRS_MSVC && defined(_WIN32) && !WINCE
	#define SRS_PC	1		/*PC platform*/
#else
	#define SRS_PC	0
#endif
	 
/*MSVC defines _ARM_, RVCT defines __arm__, ADS defines __arm. gcc defines __arm__     
Using "cpp -b <machine> -dM" to see gcc's predefines...*/
#if (defined(_ARM_) || defined(__arm__) || defined(__arm))
  #define SRS_ARM   1
#else
  #define SRS_ARM   0
#endif

/*SDE defines __mips*/
#if (defined(__mips) || defined(__mips__))  
  #define SRS_MIPS  1
#else
  #define SRS_MIPS  0
#endif

/*Xtensa processors*/
#ifdef __XTENSA__
  #define SRS_XTENSA  1
#else
  #define SRS_XTENSA  0
#endif
 
#define SRS_TIC6X		defined(_TMS320C6X)	/*TI C6000 DSP*/
#define SRS_TIC64		defined(_TMS320C6400) /*TI C6400 or C6400+*/
#define SRS_TIC64P	defined(_TMS320C6400_PLUS) /*TI C6400+*/
#define SRS_TIC67		defined(_TMS320C6700) /*TI C6700 or TI C6700+*/
#define SRS_TIC67P	defined(_TMS320C6700_PLUS) /*TI C6700+*/


/*Qualcomm Hexagon processors*/
#ifdef __qdsp6__
  #define SRS_HEXAGON 1
#else
  #define SRS_HEXAGON 0
#endif

#ifdef __IAR_SYSTEMS_ICC__
  #define SRS_IARSYS  1
#else
  #define SRS_IARSYS  0
#endif


/*Compound platform defs*/
#define SRS_MSVC_ARM    (SRS_MSVC && SRS_ARM)     /*a.k.a WinCE platform*/
#define SRS_RVCT_ARM    (SRS_RVCT && SRS_ARM)
#define SRS_GCC_ARM     (SRS_GCC  && SRS_ARM)
#define SRS_GCC_MIPS    (SRS_GCC  && SRS_MIPS)
#define SRS_XCC_XTENSA  (SRS_XCC && SRS_XTENSA)
#define SRS_GCC_HEXAGON (SRS_GCC && SRS_HEXAGON)

/*DSP extension*/
/*ARM_V5TE_COMPATIBLE: used to test if the ARM DSP extension available on the target processor*/
#if SRS_RVCT_ARM
  #define ARM_V5TE_COMPATIBLE   !(defined(__TARGET_ARCH_4) || defined(__TARGET_ARCH_4T) || defined(__TARGET_ARCH_5T)) /*ARM DSP extension available*/
  #define ARM_V6_COMPATIBLE		(defined(__TARGET_ARCH_6) || defined(__TARGET_ARCH_7A) || defined(__TARGET_ARCH_7R))
  #define ARM_V7_COMPATIBLE		(defined(__TARGET_ARCH_7A) || defined(__TARGET_ARCH_7R))	/*Cortex family in this architecture*/
  #define SRS_BIG_ED        defined(__BIG_ENDIAN)
  #define SRS_LIT_ED     !BIG_ENDIAN
#endif

#if SRS_MSVC_ARM
  #if _M_ARMT>=5
    #define ARM_V5TE_COMPATIBLE   1
  #else
    #define ARM_V5TE_COMPATIBLE   0
  #endif
#endif

#if SRS_MSVC
	#define SRS_BIG_ED	0
	#define SRS_LIT_ED	1
#endif

#if SRS_GCC_MIPS
  /*MIPS_FEATURE_DSP: used to test if the MIPS DSP ASE is available on the target processor*/
  #define MIPS_FEATURE_DSP  defined(__mips_dsp)
  /*Endian*/
  #define SRS_BIG_ED        defined(__MIPSEB)
  #define SRS_LIT_ED		defined(__MIPSEL)
#endif

#if SRS_GCC_ARM
  /*
  #if !defined(__ARM_ARCH_4__) && !defined(__ARM_ARCH_4T__) && !defined(__ARM_ARCH_5__) && !defined(__ARM_ARCH_5T__) \
	  && !defined(__ARM_ARCH_5TE__) && !defined(__ARM_ARCH_6__) && !defined(__ARM_ARCH_7A__) && !defined(__ARM_ARCH_7R__)
	#error Must define correct ARM architecture
  #endif
  */
  #define ARM_V5TE_COMPATIBLE   (defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__))
  #define ARM_V6_COMPATIBLE		(defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__))
  #define ARM_V7_COMPATIBLE		(defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__))	/*Cortex family in this architecture*/
  #define SRS_BIG_ED			defined(__ARMEB__)
  #define SRS_LIT_ED			defined(__ARMEL__)
#endif

#if SRS_XCC
  #define SRS_BIG_ED	defined(__XTENSA_EB__)
  #define SRS_LIT_ED	defined(__XTENSA_EL__)
#endif

#if SRS_HEXAGON
	#define SRS_BIG_ED	0
	#define SRS_LIT_ED	1
#endif

#if SRS_IARSYS
	#define SRS_LIT_ED	__LITTLE_ENDIAN__
	#define SRS_BIG_ED	!SRS_LIT_ED
#endif

#if (SRS_TIC6X)
	#define SRS_BIG_ED 	defined(_BIG_ENDIAN)
	#define SRS_LIT_ED	!SRS_BIG_ED
#endif

#define SRS_APPLE_MAC         (__APPLE__ && __GNUC__ && __i386__ )  
//it is a subset of SRS_GCC, normally we built a program in MAC, especailly for IOS SIMULATOR.
#define SRS_APPLE_IOS_ARM     (__APPLE__ && __GNUC__ && __arm__ )   
//it is a subset of SRS_GCC_ARM, arch an endian are same as its. we use it for IOS device building.
//both SRS_APPLE_MAC and SRS_APPLE_IOS_ARM are subsets of SRS_GCC below.
//use SRS_APPLE_MAC in *ver.cpp to define PlatformRev, while use SRS_ARM for SRS_APPLE_IOS_ARM.
#if (SRS_APPLE_MAC)
	#define SRS_BIG_ED 	!defined(__LITTLE_ENDIAN__)
	#define SRS_LIT_ED	defined(__LITTLE_ENDIAN__)
#endif


/******************************************************************************
 *Important Note:
 *   Read on if you are porting SRS IPs to your platform
 * To port SRS IPs to your platform:
 * 1. Define the flag that can identify your compiler, and add it to
 *    the "Supported compilers list" below
 * 2. Define endianess for your platform. Specifically, assign values to
 *    SRS_BIG_ED and SRS_LIT_ED
 *****************************************************************************/

/*To do: Add defines for other supported compilers*/
/*Supported compilers list:*/
#if !(SRS_MSVC || SRS_RVCT || SRS_GCC || SRS_XCC|| SRS_TIC6X || SRS_IARSYS)
	#error "Not supported compiler"
#endif

#if !(defined(SRS_BIG_ED) && defined(SRS_LIT_ED))
	#error "Endian macros not defined"
#endif

/*Misc common defs*/
#define NOT_IN_OPT_LIST(platforms)	 (!(platforms) || defined(DISABLE_OPT_CODE))

#endif /*__SRS_PLATDEFS_H__*/
