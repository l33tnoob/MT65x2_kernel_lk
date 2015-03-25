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

/*----------------------------------------------------------------------------*
 * No Warranty                                                                *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MTK with respect to any MTK *
 * Deliverables or any use thereof, and MTK Deliverables are provided on an   *
 * "AS IS" basis.  MTK hereby expressly disclaims all such warranties,        *
 * including any implied warranties of merchantability, non-infringement and  *
 * fitness for a particular purpose and any warranties arising out of course  *
 * of performance, course of dealing or usage of trade.  Parties further      *
 * acknowledge that Company may, either presently and/or in the future,       *
 * instruct MTK to assist it in the development and the implementation, in    *
 * accordance with Company's designs, of certain softwares relating to        *
 * Company's product(s) (the "Services").  Except as may be otherwise agreed  *
 * to in writing, no warranties of any kind, whether express or implied, are  *
 * given by MTK with respect to the Services provided, and the Services are   *
 * provided on an "AS IS" basis.  Company further acknowledges that the       *
 * Services may contain errors, that testing is important and Company is      *
 * solely responsible for fully testing the Services and/or derivatives       *
 * thereof before they are used, sublicensed or distributed.  Should there be *
 * any third party action brought against MTK, arising out of or relating to  *
 * the Services, Company agree to fully indemnify and hold MTK harmless.      *
 * If the parties mutually agree to enter into or continue a business         *
 * relationship or other arrangement, the terms and conditions set forth      *
 * hereunder shall remain effective and, unless explicitly stated otherwise,  *
 * shall prevail in the event of a conflict in the terms in any agreements    *
 * entered into between the parties.                                          *
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Copyright(c) 2009, MediaTek, Inc.
 * All rights reserved.
*
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclosure of this information in whole or in part is prohibited.
 *-----------------------------------------------------------------------------
 * $File: //Department/MATV_IP/matvctrl_src/hostlib.h $
 * $Revision: #1 $
 * $DateTime: 2010/09/16 22:07:59 $
 * $Change: 17190 $
 * $Id: //Department/MATV_IP/matvctrl_src/hostlib.h#1 $
 * $Author: charlie.lu $
 *---------------------------------------------------------------------------*/
/**
 *   @file hostlib.h
 *		
 *   @author lh.hsiao 
 */
#ifndef __HOSTLIB_H__
#define __HOSTLIB_H__

#if defined(__MTK_TARGET__)

#include "kal_release.h"



#define xdata
#define CODE
#define BIT  UINT8
#define PRIVATE static
#define TRUE 1
#define FALSE 0

typedef UINT8 BOOL;
typedef UINT8 UCHAR;

typedef UINT8 U8;
typedef UINT16 U16;
typedef UINT32 U32;
typedef INT8 S8;
typedef INT16 S16;
typedef INT32 S32;
typedef UINT8 BYTE;
typedef UINT16 WORD;
typedef UINT32 DWRD;

/////////////////////////////////////////////////////////////////////////////
//power on/off/suspend
/////////////////////////////////////////////////////////////////////////////
UINT32 ATVChipID(void);
UINT8 bChipInit(UINT8 tvmode);
UINT8 bChipSuspend(UINT8 on);
UINT8 bChipShutdown(void);
void vMainloop(UINT32 delay_in_10ms); //chip main loop service running by task

/////////////////////////////////////////////////////////////////////////////
//ch scan/control
/////////////////////////////////////////////////////////////////////////////
#define Freq2DR(freqkhz)	(UINT16)((freqkhz*2)/125) //(freqkhz*16)/1000
#define DR2Freq(freqDR)		(((UINT32)freqDR*125)/2)
extern UINT16 Freq2DR_Fn(UINT32 freqkhz);
extern UINT32 DR2Freq_Fn(UINT16 freqDR);

enum 
{
	CHINS_AUTO=0,
	CHINS_FULLSCAN=1,
	CHINS_1CH=2
};

extern void vChSearchInit(UINT16 freqDR, UINT8 scantype);
extern void vChSearchStop(void);
extern void vChQuickInstall(void);
extern void vChChg(UINT8 ch);
extern void vChNext(UINT8 inc);

extern UINT8 bIsChSearching(void);

struct ChSearchState
{
	UINT8	mode;
	UINT8	is_scanning;
	UINT8	ch_latest_updated;
};

extern void vChSearchQuery(struct ChSearchState *state);

struct ChRec 
{
	UINT16	freqDR;
	UINT8	sndsys;	/* reference sv_const.h, TV_AUD_SYS_T ...*/
	UINT8	colsys;	/* reference sv_const.h, SV_CS_PAL_N, SV_CS_PAL,SV_CS_NTSC358...*/
	UINT8	flag;
};

#if 0
enum
{
	CH_VALID=1
};
#endif

extern UINT8 bChTab_Get(UINT8 n,struct ChRec * rec);//return 1:success 0:fail
extern UINT8 bChTab_Set(UINT8 n,struct ChRec * rec);//return 1:success 0:fail
extern UINT8 bChTab_Clear(UINT8 n);//return 1:success 0:fail
extern UINT8 bChTab_ClearAll(void);//return 1:success 0:fail

typedef  void (*AutoScanProgCB_t)(UINT8 precent,UINT8 ch,UINT8 chnum);
typedef  void (*FullScanProgCB_t)(UINT8 precent,UINT32 freq,UINT32 freq_start,UINT32 freq_end);
typedef  void (*ScanFinishCB_t)(UINT8 chnum);
void RegisterChScanCB(AutoScanProgCB_t auto_cb,FullScanProgCB_t full_cb,ScanFinishCB_t finish_cb);

/////////////////////////////////////////////////////////////////////////////
//AUDIO
/////////////////////////////////////////////////////////////////////////////

	void vApiAudioPlay(BOOL fgWaitPlay);
	void vApiAudioStop(void);	
	void vApiAudioSetMasterVolume(BYTE bValue);
	UINT32 dwApiAudioGetFormatSrc(void);
	void vApiAudioSetAtvOutputMode(UINT32 eSoundMode);
//meta	
	BYTE bApiAudioGetFmRdoDetectStatus (UINT32 *info);	
	BYTE bApiAudioGetSoundSystem(UINT32 *info);

typedef  void (*AudNotifyCB_t)(UINT32 format);
void RegisterAudNotifyCB(AudNotifyCB_t aud_notify_cb);

/////////////////////////////////////////////////////////////////////////////
//FM radio
/////////////////////////////////////////////////////////////////////////////
#ifndef __TV_AUD_SYS_T__
#define __TV_AUD_SYS_T__
typedef enum 
{
	SV_NONE_DETECTED,	//0x0
	SV_MTS,			//0x1
	SV_FM_FM,		//0x2
	SV_NTSC_M,		//0x3
	SV_A2_BG,		//0x4
	SV_A2_DK1,		//0x5
	SV_A2_DK2,		//0x6
	SV_A2_DK3,		//0x7
	SV_PAL_I,		//0x8
	SV_PAL_BG,		//0x09
	SV_PAL_DK,		//0x0a
	SV_SECAM_L,     //0x0b
	SV_SECAM_L_PLUM,//0x0c
	SV_SECAM_BG,	//0x0d
	SV_SECAM_DK,    //0x0e
	SV_FMRDO,       //0x0f
	SV_SLT,         //0x10
	SV_PAL_I_FMMONO,      //0x11
	SV_PAL_BG_FMMONO,      //0x12
	SV_PAL_DK_FMMONO,      //0x13
	SV_KOREA,    // 0x14
	SV_NUM	
} TV_AUD_SYS_T;
#endif


extern void vATD_SetSystem(UINT8 u1SubSysIdx);
extern UINT8 bApiFmRdoScan (UINT32 u4Freq, UINT8 u1IsStepUp, UINT32 u4Step);
extern void vApiFmRdoScanInit (void);
extern void vApiMT5192_FmRdoSetFreq (UINT32 u4Freq, UINT8 u1Mode);
extern void vApiAudioSetChannelSrc(TV_AUD_SYS_T bChSrc);
extern void vApiAudioMainloop (void);
extern BOOL fgApiAudioIsFmRdoDetectDone (void);
extern UINT8 bApiAudioGetFmRdoDetectResult (UINT32 *u4Rssi, UINT32 *u4Noise);






/////////////////////////////////////////////////////////////////////////////
//TVD
/////////////////////////////////////////////////////////////////////////////
enum TVD_BLOCK{
    TVD_BLOCK_IS443=0,
    TVD_BLOCK_IS358,
    TVD_BLOCK_UNKNOW
};

extern UINT8 bTvd3dGetColorSystem(void);
extern void vDrvTvd3dSetColorSystem(UINT8 bColSys);
extern void vTvd3dReset(void);
extern UINT8 fgDrvTvdCheckTVDLock(void);
extern UINT8 bDrvTvdCheckBurstFreq(void);
extern UINT8 bTvd3dGetNRLevel(void);

extern void vApiVideoPosBrightness(BOOL polarity, UINT8 bValue);
extern void vApiVideoPreBrightness(BOOL polarity, UINT8 bValue);
extern void vApiVideoContrast(UINT8 bValue);
extern void vApiVideoSaturation(UINT8 bValue);


/////////////////////////////////////////////////////////////////////////////
//Chip/Vendor dependent
/////////////////////////////////////////////////////////////////////////////
enum
{
	//export to standard enum (mandatory)
	EXPORT_SIG_RSSI=0,
	EXPORT_SIG_SNR,
	EXPORT_SIG_STRENGTH,
	EXPORT_TVD_MaxFrameRate,
	EXPORT_AUD_OutputMode,
	EXPORT_AUD_SWMute,
	
	/* item >= 100, variant info, not fixed
	 * example
	 * MTK_GAIN0=100,
	 * MTK_GAIN1=101 
	 */
	MTK_TVD_MaxFrameRate=180,	/* ref. eTVDMaxFramerate */
	MTK_TVD_CamIFRefMCLK,		/* ref. eOnOff */
	MTK_TVD_CamIFMode,		/* ref. eTVDCamIFMode */
	MTK_TVD_CamIFCaptureMode,	/* ref. eOnOff */
	MTK_TVD_CamIFResolution,	/* (width<<16)|height */
	MTK_TVD_SMOOTHER,			/* ref. (eTVDSMOOTHER<<4)|Thr */
	MTK_TVD_PCLK_INV,			/* ref. eOnOff */
	MTK_TVD_FrameRate,
	MTK_TVD_FRDIV2_WA,			/* 1~5, 0xff*/
	MTK_TVD_ScanLockPara,			/*VOFST_TH Recheck_TH CHECKTVD_ROUND,  */
	MTK_AUD_OutputMode=190, 	/* ref. eAUDOutputMode */
	MTK_AUD_SWMute,	
	MTK_CLI_PORT=200,

	MTK_DRV_VERSION_ID=202,
	MTK_DRV_VERSION_SUB_ID,
	MTK_LCM_WRITECYCLE=210,
	MTK_PAD_DRIVING,	/* (pin<<4)|driving ; pin:0~7, H, V, PCLK*/
	MTK_ATD_EQ,
	MTK_TVD_AdapC,
	MTK_AUD_AVC=214,
	MTK_AUD_SmartToggling,
	MTK_TVD_DropFrameRate,
	MTK_TVD_EnableSharpness,
	MTK_TVD_Enable1DNR,
	MTK_TVD_MN_DropFrameRate,
	MTK_TVD_RealTimeMode,
	MTK_TVD_MN_RealTimeMode,
	MTK_TVD_RecoverDropFrameRate,
	MTK_TVD_DelayMode,
	MTK_TVD_AdjustNR,
	MTK_TVD_Bypass
};

INT32 DrvGetChipDep(UINT8 item);
BOOL DrvSetChipDep(UINT8 item,INT32 val);

#endif /* __MTK_TARGET__ */
#endif /* __HOSTLIB_H__ */
