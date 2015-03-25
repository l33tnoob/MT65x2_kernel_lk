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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/**
 * @file af_mgr.h
 * @brief AF manager, do focusing for raw sensor.
 */
#ifndef _AF_MGR_H_
#define _AF_MGR_H_

#include <af_feature.h>
class NvramDrvBase;
using namespace android;
namespace NS3A
{
class IAfAlgo;

/*******************************************************************************
*
*******************************************************************************/
class AfMgr
{
public:
    AfMgr();
    ~AfMgr();

    static AfMgr& getInstance();
    MRESULT init();
    MRESULT uninit();
    MRESULT doAF(MVOID *pAFStatBuf);

    MINT32 CCTOPAFOpeartion();
    MINT32 CCTOPMFOpeartion(MINT32 a_i4MFpos);
    MINT32 CCTOPAFGetAFInfo(MVOID *a_pAFInfo, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetBestPos(MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFCaliOperation(MVOID *a_pAFCaliData, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFSetFocusRange(MVOID *a_pFocusRange);
    MINT32 CCTOPAFGetFocusRange(MVOID *a_pFocusRange, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFGetNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFApplyNVRAMParam(MVOID *a_pAFNVRAM);
    MINT32 CCTOPAFSaveNVRAMParam();
    MINT32 CCTOPAFGetFV(MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen);
    MINT32 CCTOPAFEnable();
    MINT32 CCTOPAFDisable();
    MINT32 CCTOPAFGetEnableInfo(MVOID *a_pEnableAF, MUINT32 *a_pOutLen);

    MRESULT triggerAF();
    MRESULT setAFMode(MINT32 a_eAFMode);
    MRESULT setAFArea(CameraFocusArea_T a_sAFArea);
    MRESULT setCamMode(MINT32 a_eCamMode);
    MRESULT setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    MRESULT setFDInfo(MVOID* a_sFaces);
    MINT32  getAFMaxAreaNum();
    MINT32  getMaxLensPos();
    MINT32  getMinLensPos();    
    MINT32  getAFBestPos();  
    
    MINT32  getAFPos();
    MINT32  getAFStable();
    MINT32  getAFTableOffset();
    MINT32  getAFTableMacroIdx();
    MINT32  getAFTableIdxNum();
    MVOID*  getAFTable();
  
    AF_FULL_STAT_T getAFFullStat();    
    MRESULT enableAF(MINT32 a_i4En);

    MRESULT setMFPos(MINT32 a_i4Pos);
    MRESULT setFullScanstep(MINT32 a_i4Step);
    MINT32  isFocusFinish();    
    MINT32  isFocused();

    MVOID setAF_IN_HSIZE();
    MVOID setFlkWinConfig();    

    MRESULT getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo);
    MBOOL   setCallbacks(I3ACallBack* cb);
    MRESULT SingleAF_CallbackNotify();
    MRESULT setBestShotConfig();
    MRESULT calBestShotValue(MVOID *pAFStatBuf);
    MINT64  getBestShotValue();
    
    MVOID   printAFConfigLog0();
    MVOID   printAFConfigLog1();
    MVOID   autoFocus();
    MVOID   cancelAutoFocus();    
    MVOID   TimeOutHandle();
    
#if 0    
    MRESULT enableAF();
    MRESULT disableAF();
    MRESULT setFocusMode(MINT32 a_i4AFMode);
    MINT32  getFocusMode() const;

    MINT32  getFocusAreasNum();   // max supported areas number
	MRESULT setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea);
	MRESULT getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea, AF_RESULT_T **a_psFocusResult);

    MRESULT pauseFocus();
	MRESULT resetFocus();
    MBOOL   isFocusFinish();
    MBOOL   isFocused();

    MVOID   setFocusFullScanStep(MINT32 a_i4Step);
    MVOID   setFocusPos(MINT32 a_i4Pos);   // valid when FocusMode = MF
    MINT32  getFocusPos();
    MUINT32 getFocusValue();
    MINT32  getAFResultPos();

    

    MRESULT setFocusDistanceRange(MINT32 a_i4Distance_N, MINT32 a_i4Distance_M);
    MRESULT getFocusDistance(MINT32 &a_i4Near, MINT32 &a_i4Curr, MINT32 &a_i4Far);
#endif

    //MVOID    setAFCoef(AF_COEF_T a_sAFCoef);

private:
    MVOID setAFConfig(AF_CONFIG_T a_sAFConfig);    
    MVOID setAFWinTH(AF_CONFIG_T a_sAFConfig);
    MVOID setGMR(AF_CONFIG_T a_sAFConfig);    
    MVOID setAFWinConfig(AF_AREA_T a_sAFArea);        
    
    AF_STAT_T Trans4WintoOneStat(MVOID *pAFStatBuf);
    AF_STAT_T TransAFtoOneStat(MVOID *pAFStatBuf);    
    AF_FULL_STAT_T TransToFullStat(MVOID *pAFStatBuf);
    MINT32 Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max);        
    MRESULT getLensInfo(LENS_INFO_T &a_rLensInfo);
    MRESULT readOTP();

    volatile int  m_Users;
    mutable Mutex m_Lock;

    
    SensorHal*  m_pSensorHal;           
    MCUDrv*     m_pMcuDrv;
    IspDrv*     m_pIspDrv;    
    IspDrv*     m_pVirtIspDrvCQ0;
    isp_reg_t*  m_pIspReg;
    IAfAlgo*    m_pIAfAlgo;

    MINT32      m_i4CurrSensorDev;
    MINT32      m_i4CurrSensorId;
    MINT32      m_i4CurrLensId;

    AF_INPUT_T    m_sAFInput;
    AF_OUTPUT_T   m_sAFOutput;
    AF_PARAM_T    m_sAFParam;
    AF_CONFIG_T   m_sAFConfig;
    NVRAM_LENS_PARA_STRUCT  m_NVRAM_LENS;

    LIB3A_AF_MODE_T     m_eLIB3A_AFMode;

    CameraFocusArea_T   m_CameraFocusArea;
    AF_AREA_T           m_FDArea;

    MINT32 m_i4AF_in_Hsize;
    MINT32 m_i4AF_in_Vsize;

    MINT32 m_i4EnableAF;

    MINT32 m_i4MFPos;

    I3ACallBack*  m_pAFCallBack;

    MINT32 m_i4AFPreStatus;
    MINT32 m_bDebugEnable;

    MINT32 m_i4AF_TH[3];
    MINT32 m_i4AF_THEX[2];
    MINT32 m_i4GMR[3];
    
    MINT32 m_i4AutoFocus;

    AF_FULL_STAT_T m_sAFFullStat;

    MINT64 m_i8PreVStat[36];
    MINT64 m_i8BSSVlu;
};

};  //  namespace NS3A
#endif // _AF_MGR_H_

