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
 * @file awb_mgr.h
 * @brief AWB manager
 */

#ifndef _AWB_MGR_H_
#define _AWB_MGR_H_

#include <awb_feature.h>

namespace NS3A
{

/**  
 * @brief sensor resolution information
 */
typedef struct
{
  MUINT16 u2SensorPreviewWidth;  /*!< sensor preview width */
  MUINT16 u2SensorPreviewHeight; /*!< sensor preview height */
  MUINT16 u2SensorFullWidth;     /*!< sensor full width */
  MUINT16 u2SensorFullHeight;    /*!< sensor full height */
  MUINT16 u2SensorVideoWidth;    /*!< sensor video width */
  MUINT16 u2SensorVideoHeight;   /*!< sensor video height */
} SENSOR_RESOLUTION_INFO_T;

class IAwbAlgo;

/**  
 * @brief AWB manager 
 */
class AwbMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    AwbMgr(AwbMgr const&);
    //  Copy-assignment operator is disallowed.
    AwbMgr& operator=(AwbMgr const&);

public:  ////
    AwbMgr();
    ~AwbMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    /**  
     * @brief get instance
     */
    static AwbMgr& getInstance();

    /**  
     * @brief camera preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */       
    MRESULT cameraPreviewInit(MINT32 i4SensorDev, Param_T &rParam);

    /**  
     * @brief camcorder preview init
     * @param [in] i4SensorDev sensor device; please refer to ESensorDev_T in isp_tuning.h
     * @param [in] rParam parameter; please refer to aaa_hal_base.h
     */
    MRESULT camcorderPreviewInit(MINT32 i4SensorDev, Param_T &rParam);

    /**  
     * @brief camera capture init
     */    
    MRESULT cameraCaptureInit();

    /**  
     * @brief camera preview re-init
     * @param [in] rParam camera parameter; please refer to aaa_hal_base.h
     */
    MRESULT cameraPreviewReinit(Param_T &rParam);

    /**  
     * @brief uninit
     */    
    MRESULT uninit();

    /**  
     * @brief check if AWB lock is supported or not
     * @return always return TRUE
     */
    inline MBOOL isAWBLockSupported()
    {
        return MTRUE;
    }

    /**  
     * @brief check if AWB is enabled or not
     * @return TRUE if AWB is enabled
     */
    inline MBOOL isAWBEnable()
    {
        return m_bEnableAWB;
    }

    /**  
     * @brief set AWB mode
     * @param [in] i4NewAWBMode AWB mode; please refer to awb_feature.h 
     */
    MRESULT setAWBMode(MINT32 i4NewAWBMode);

    /**  
     * @brief get AWB mode
     * @return current AWB mode 
     */     
    MINT32 getAWBMode() const;

    /**  
     * @brief set strobe mode
     * @param [in] i4NewStrobeMode strobe mode; please refer to AWB_STROBE_MODE_T in awb_param.h
     */
    MRESULT setStrobeMode(MINT32 i4NewStrobeMode);

    /**  
     * @brief get strobe mode
     * @return current strobe mode
     */    
    MINT32 getStrobeMode() const;

    /**  
     * @brief set AWB lock
     * @param [in] bAWBLock TRUE: lock, FALSE: unlock 
     */     
    MRESULT setAWBLock(MBOOL bAWBLock);

    /**  
     * @brief enable AWB
     */     
    MRESULT enableAWB();

    /**  
     * @brief disable AWB
     */    
    MRESULT disableAWB();

    /**  
     * @brief preview AWB main function
     * @param [in] i4FrameCount current frame count
     * @param [in] bAEStable TRUE: AE is stable, FALSE: AE is unstable
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     */    
    MRESULT doPvAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf);

    /**  
     * @brief video AWB main function
     * @param [in] i4FrameCount current frame count
     * @param [in] bAEStable TRUE: AE is stable, FALSE: AE is unstable
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     */    
    MRESULT doVideoAWB(MINT32 i4FrameCount, MBOOL bAEStable, MINT32 i4SceneLV, MVOID *pAWBStatBuf);

     /**  
     * @brief touch focus AWB main function
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     */    
    MRESULT doAFAWB(MVOID *pAWBStatBuf);

    /**  
     * @brief pre-capture AWB main function
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     */     
    MRESULT doPreCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf);

    /**  
     * @brief capture AWB main function
     * @param [in] i4SceneLV scene LV
     * @param [in] pAWBStatBuf pointer to AWB statistics buffer
     */       
    MRESULT doCapAWB(MINT32 i4SceneLV, MVOID *pAWBStatBuf);

    /**  
     * @brief get EXIF debug info
     * @param [out] rAWBDebugInfo AWB debug info; please refer to dbg_awb_param.h
     * @param [out] rAWBDebugData AWB debug data; please refer to dbg_awb_param.h
     */    
    MRESULT getDebugInfo(AWB_DEBUG_INFO_T &rAWBDebugInfo, AWB_DEBUG_DATA_T &rAWBDebugData);

    /**  
     * @brief get correlated color temperature
     * @return correlated color temperature
     */    
    MINT32 getAWBCCT();

    /**  
     * @brief get ASD info
     * @param [out] a_rAWBASDInfo ASD info; please refer to awb_param.h 
     */
    MRESULT getASDInfo(AWB_ASD_INFO_T &a_rAWBASDInfo);

    /**  
     * @brief get AWB output
     * @param [out] a_rAWBOutput AWB algorithm output; please refer to awb_param.h
     */
    MRESULT getAWBOutput(AWB_OUTPUT_T &a_rAWBOutput);

    /**  
     * @brief set AF LV
     * @param [out] i4AFLV scene LV for touch AF
     */
    inline MVOID setAFLV(MINT32 i4AFLV)
    {
         m_i4AFLV = i4AFLV;
    }

    /**  
     * @brief get AF LV
     * @return scene LV for touch AF
     */
    inline MINT32 getAFLV()
    {
         return m_i4AFLV;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**  
     * @brief get sensor resolution
     */
    MRESULT getSensorResolution();

    /**  
     * @brief get NVRAM data
     */    
    MRESULT getNvramData();

    /**  
     * @brief AWB init
     * @param [in] rParam camera parameter
     */  
    MRESULT AWBInit(Param_T &rParam);

    /**  
     * @brief get EEPROM data
     */    
    MRESULT getEEPROMData();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CCT feature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MRESULT CCTOPAWBEnable();
    MRESULT CCTOPAWBDisable();
    MRESULT CCTOPAWBGetEnableInfo(MINT32 *a_pEnableAWB,MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetAWBGain(MVOID *a_pAWBGain, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBSetAWBGain(MVOID *a_pAWBGain);
    MRESULT CCTOPAWBApplyNVRAMParam(MVOID *a_pAWBNVRAM);
    MRESULT CCTOPAWBGetNVRAMParam(MVOID *a_pAWBNVRAM, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBSaveNVRAMParam();
    MRESULT CCTOPAWBSetAWBMode(MINT32 a_AWBMode);
    MRESULT CCTOPAWBGetAWBMode(MINT32 *a_pAWBMode, MUINT32 *a_pOutLen);
    MRESULT CCTOPAWBGetLightProb(MVOID *a_pAWBLightProb, MUINT32 *a_pOutLen);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IAwbAlgo* m_pIAwbAlgo;
    LIB3A_AWB_MODE_T m_eAWBMode;
    MINT32 m_i4SensorMode;
    MINT32 m_i4StrobeMode;
    MBOOL m_bEnableAWB;
    MBOOL m_bAWBLock;
    MBOOL m_bAdbAWBLock;
    MBOOL m_bOneShotAWB;
    MBOOL m_bAWBModeChanged;
    MBOOL m_bStrobeModeChanged;
    MINT32 const* m_pIsAWBActive;
    MINT32 const m_i4PvAWBCycleNum;
    MINT32 const m_i4VideoAWBCycleNum;
    MINT32 m_i4SensorDev;
    MBOOL m_bDebugEnable;
    MBOOL m_bInitState;
    MINT32 m_i4AFLV;
    MBOOL m_bSkipOneFrame;
};

};  //  namespace NS3A
#endif // _AWB_MGR_H_

