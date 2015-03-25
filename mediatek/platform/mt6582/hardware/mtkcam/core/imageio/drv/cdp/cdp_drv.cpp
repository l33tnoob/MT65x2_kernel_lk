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
#define LOG_TAG "CdpDrv"

#include <utils/Errors.h>
//#include <cutils/log.h> 
#include "camera_sysram.h"  // For SYSRAM.
//#include <cutils/pmem.h>
//#include <cutils/memutil.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>

#include "imageio_types.h"    // For type definitions.
#include <mtkcam/drv/isp_reg.h>    // For ISP register structures.
#include <mtkcam/drv/isp_drv.h>    // For ISP_[READ|WRITE]_[BITS|REG] macros.
#include "cdp_drv_imp.h"

#include <cutils/properties.h>  // For property_get().

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{CdpDrv} "
//#undef   DBG_LOG_LEVEL                      // Decide Debug Log level for current file. Can only choose from 2~8.
//#define  DBG_LOG_LEVEL      2               // 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(cdp_drv);
//EXTERN_DBG_LOG_VARIABLE(cdp_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (cdp_drv_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (cdp_drv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define VRZ_EXIST   0
#define RSP_EXIST   0
#define VRZO_EXIST  0
#define DISPO_HAS_ROTATE    0

class CdpDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    CdpDbgTimer(char const*const pszTitle)
        : mpszName(pszTitle)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
    {
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL ProfilingPrint(char const*const pszInfo = "") const
    {
        MINT32 const i4EndUs = getUs();
        if  (0==mIdx)
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
        }
        else
        {
            LOG_INF("[%s] %s:(%d-th) ===> [start-->now: %.06f ms] [last-->now: %.06f ms]", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000, (float)(i4EndUs-mi4LastUs)/1000);
        }
        mi4LastUs = i4EndUs;

	    //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};

#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 0   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 1   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CdpDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#else   // Using LDVT.
    #if 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   CdpDbgTimer DbgTmr(#EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      DbgTmr.ProfilingPrint(LOG_STRING);
    #else   // No profiling.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);
    #endif  // Diff Profile tool.
#endif  // USING_MTK_LDVT

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static IspDrv* m_pPhyIspDrv;
static MUINT32 CdpDrvRzUint[CDP_DRV_ALGO_AMOUNT] =
{
    32768,
    1048576,
    1048576
};

static MUINT32 CdpDrvRzTable[CDP_DRV_RZ_TABLE_AMOUNT] =
{
    32768,
    36408,
    40960,
    46811,
    54613,
    65536
};

/**************************************************************************
*       P R I V A T E    F U N C T I O N    D E C L A R A T I O N
**************************************************************************/


/**************************************************************************
* 
**************************************************************************/
CdpDrvImp::CdpDrvImp()
{
    MUINT32 i, j;
    GLOBAL_PROFILING_LOG_START(Event_CdpDrv);	// Profiling Start.

    LOG_INF("");
   
    mInitCount  = 0;
    mSysramUsageCount = 0;
    mpIspReg        = NULL;
    m_pPhyIspDrv    = NULL;
    m_pPhyIspReg    = NULL;
    mFdSysram       = -1;

    SysramAlloc.Alignment = 0;
    SysramAlloc.Size = 0;
    SysramAlloc.User = SYSRAM_USER_VIDO;
    SysramAlloc.Addr = 0;
    SysramAlloc.TimeoutMS = 100;

    for (i = 0; i < CDP_DRV_ROTDMA_AMOUNT; i++)
    {
        for (j = 0; j < CDP_DRV_LC_AMOUNT; j++)
        {
            mRotationBuf[i][j].Fd = -1;
        }
    }
}


/**************************************************************************
* 
**************************************************************************/
CdpDrvImp::~CdpDrvImp()
{
    LOG_INF("");
    GLOBAL_PROFILING_LOG_END(); 	// Profiling End.
}


/**************************************************************************
* 
**************************************************************************/
CdpDrv *CdpDrv::CreateInstance(MBOOL fgIsGdmaMode)
{
    DBG_LOG_CONFIG(imageio, cdp_drv);

    LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);

    return CdpDrvImp::GetInstance(fgIsGdmaMode);
}


/**************************************************************************
* 
**************************************************************************/
CdpDrv *CdpDrvImp::GetInstance(MBOOL fgIsGdmaMode)
{
    static CdpDrvImp Singleton;

    //LOG_INF("");
    LOG_INF("fgIsGdmaMode: %d.", fgIsGdmaMode);

    Singleton.m_fgIsGdmaMode = fgIsGdmaMode;

    return &Singleton;
}


/**************************************************************************
* 
**************************************************************************/
MVOID CdpDrvImp::DestroyInstance()
{
    LOG_INF("+");
}

/**************************************************************************
* 
**************************************************************************/
MBOOL CdpDrvImp::Init()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CdpDrv_Init);

    Mutex::Autolock lock(mLock);    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    LOG_INF("+,mInitCount(%d),mSysramUsageCount(%d),m_fgIsGdmaMode(%d)", mInitCount, mSysramUsageCount, (int)m_fgIsGdmaMode);

    if (mInitCount >= CDP_DRV_INIT_MAX)	// Show warning when CDR Drv count too large.
    {
        LOG_WRN("Warning: mInitCount >= CDP_DRV_INIT_MAX.");
    }
    else if (mInitCount == 0)	// First CDP Drv instance, must do some init procedure.
    {
        if (!m_fgIsGdmaMode)    // GDMA mode won't use rotate, so don't need SYSRAM.
        {
            // Open SYSRAM kernel drv.
            LOG_INF("open camera-sysram kernel driver");
            mFdSysram = open("/dev/"SYSRAM_DEV_NAME, O_RDONLY, 0);
            if (mFdSysram < 0)
            {
                LOG_ERR("Sysram kernel open fail, errno(%d): %s.", errno, strerror(errno));
                Result = MFALSE;
                goto lbEXIT;
            }

            LOCAL_PROFILING_LOG_PRINT("Open SYSRAM kernel drv");

            // Allocate RotationBuf in SYSRAM.
            if ( !AllocateRotationBuf() )
            {
                Result = MFALSE;
                goto lbEXIT;
            };

            LOCAL_PROFILING_LOG_PRINT("AllocateRotationBuf");
        }
    }

	// Increase CDR Drv count.
    android_atomic_inc(&mInitCount);
    LOCAL_PROFILING_LOG_PRINT("atomic_inc");

lbEXIT:
    
    LOG_INF("-,Result(%d),mInitCount(%d),mFdSysram(%d),mSysramUsageCount(%d)", Result, mInitCount, mFdSysram, mSysramUsageCount);

    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}

/**************************************************************************
* 
**************************************************************************/
MBOOL CdpDrvImp::Uninit()
{
    MBOOL Result = MTRUE;

    GLOBAL_PROFILING_LOG_PRINT(__func__);
    LOCAL_PROFILING_LOG_AUTO_START(Event_CdpDrv_Uninit);

    Mutex::Autolock lock(mLock);

    LOG_INF("+,mInitCount(%d),mSysramUsageCount(%d)", mInitCount, mSysramUsageCount);

    if (mInitCount > 1)  // More than one user.
    {
        android_atomic_dec(&mInitCount);
        LOCAL_PROFILING_LOG_PRINT("atomic_dec");
    }
    else if (mInitCount == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mInitCount);
        LOCAL_PROFILING_LOG_PRINT("atomic_dec");

        if (!m_fgIsGdmaMode)    // GDMA mode won't use rotate, so don't need SYSRAM.
        {
            // Free RotationBuf in SYSRAM.
            FreeRotationBuf();

            LOCAL_PROFILING_LOG_PRINT("FreeRotationBuf");

            // Close SYSRAM kernel drv.
            if (mFdSysram >= 0)
            {
                close(mFdSysram);
    	        //     Reset all Fd to -1. (This can only be done after close(mFdSysram).)
                for (MUINT32 i = 0; i < CDP_DRV_ROTDMA_AMOUNT; i++)
                {
                    for (MUINT32 j = 0; j < CDP_DRV_LC_AMOUNT; j++)
                    {
                        mRotationBuf[i][j].Fd = -1;
                    }
                }
            }

            LOCAL_PROFILING_LOG_PRINT("Close SYSRAM kernel drv");
        }

    }
    else // mInitCount <= 0. No CDP Drv user.
    {
        // do nothing.
        LOG_WRN("No CDR Drv to un-init.");
    }

    if (mInitCount >= CDP_DRV_INIT_MAX)
    {
        LOG_WRN("mInitCount >= CDP_DRV_INIT_MAX.");
    }

EXIT:
    
    LOG_INF("-.Result(%d),mInitCount(%d),mFdSysram(%d),mSysramUsageCount(%d)", Result, mInitCount, mFdSysram, mSysramUsageCount);
    LOCAL_PROFILING_LOG_PRINT("Exit");
    return Result;
}

/**************************************************************************
* @brief Set ISP Reg pointer for CDP Drv to use
* @param [IN]     pIspReg         Pointer of ISP Reg.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::SetIspReg(isp_reg_t *pIspReg)
{
    LOG_DBG("+,pIspReg(0x%08x)", (MUINT32)pIspReg);

    if (mInitCount <= 0)
    {
        LOG_ERR("No CDP Drv. Please init one first!");
        return MFALSE;
    }

    if (pIspReg == NULL)
    {
        LOG_ERR("pIspReg is NULL.");
        return MFALSE;
    }

    mpIspReg = pIspReg;

    return MTRUE;
}

/**************************************************************************
* @brief Set ISP Reg pointer for CDP Drv to use.
* @param [IN]     pIspReg         Pointer of ISP Reg.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::SetPhyIspDrv(IspDrv *pPhyIspDrv)
{
    LOG_DBG("+,pPhyIspDrv(0x%08x)", (MUINT32)pPhyIspDrv);

    if (mInitCount <= 0)
    {
        LOG_ERR("No CDP Drv. Please init one first!");
        return MFALSE;
    }

    if (pPhyIspDrv == NULL)
    {
        LOG_ERR("pPhyIspDrv is NULL.");
        return MFALSE;
    }

    m_pPhyIspDrv = pPhyIspDrv;
    m_pPhyIspReg = m_pPhyIspDrv->getRegAddrMap();
    mpIspReg = m_pPhyIspReg;    // To avoid CheckReady() fail.

    return MTRUE;
}

/**************************************************************************
* @brief Check if CDR Drv is ready.
* 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv
* 2. Check if mpIspReg is ready
* 
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::CheckReady()
{
    // 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv.
    if (mInitCount <= 0)
    {
        LOG_ERR("No more CDP Drv user. Please init one first!");
        return MFALSE;
    }

    // 2. Check if mpIspReg is ready.
    if (mpIspReg == NULL)
    {
        LOG_ERR("mpIspReg is NULL.");
        return MFALSE;
    }

    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CalAlgoAndCStep(
    CDP_DRV_MODE_ENUM eFrameOrTpipeOrVrmrg,
    CDP_DRV_RZ_ENUM eRzName,
    MUINT32 SizeIn_H,
    MUINT32 SizeIn_V,
    MUINT32 u4CroppedSize_H,
    MUINT32 u4CroppedSize_V,
    MUINT32 SizeOut_H,
    MUINT32 SizeOut_V,
    CDP_DRV_ALGO_ENUM *pAlgo_H,
    CDP_DRV_ALGO_ENUM *pAlgo_V,
    MUINT32 *pTable_H,
    MUINT32 *pTable_V,
    MUINT32 *pCoeffStep_H,
    MUINT32 *pCoeffStep_V)
{
    MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
    MUINT32 u4SupportingWidth_4tap  = 0;
    MUINT32 u4SupportingWidth_4Ntap = 0;
    MUINT32 u4SupportingWidth_Ntap  = 0;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    MUINT32 u4SizeToBeChecked_H = 0;
    MUINT32 u4SizeToBeChecked_V = 0;

    LOG_INF("+,Mode(%d),eRzName(%d),In/Crop/Out:(%d,%d)/(%d,%d)/(%d,%d).",eFrameOrTpipeOrVrmrg, eRzName, SizeIn_H, SizeIn_V, u4CroppedSize_H, u4CroppedSize_V, SizeOut_H, SizeOut_V);

    // Check if CDP Drv is ready.
    if (!CheckReady())
    {
        Result = MFALSE;
        goto lbExit;
    }

    // Calculate Mm1 = u4CroppedSize - 1.
    Mm1_H = u4CroppedSize_H - 1;
    Mm1_V = u4CroppedSize_V - 1;
    // Calculate Nm1 = SizeOut - 1.
    Nm1_H = SizeOut_H - 1;
    Nm1_V = SizeOut_V - 1;

    // Decide Supporting Width.
    switch (eFrameOrTpipeOrVrmrg)
    {
        case CDP_DRV_MODE_FRAME:
        {
            switch (eRzName)
            {
                case CDP_DRV_RZ_CDRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_FRAME_CDRZ_N_TAP;
                break;

                case CDP_DRV_RZ_CURZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_FRAME_CURZ_4_TAP;
                break;

                case CDP_DRV_RZ_PRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_FRAME_PRZ_N_TAP;
                break;

                default:
                    LOG_ERR("Not support eRzName. eRzName: %d.", eRzName);
                    Result = MFALSE;
                    goto lbExit;
            }
        }
        break;

        case CDP_DRV_MODE_TPIPE:
        {
            switch (eRzName)
            {
                case CDP_DRV_RZ_CDRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_TPIPE_CDRZ_N_TAP;
                break;

                case CDP_DRV_RZ_CURZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_TPIPE_CURZ_4_TAP;
                break;

                case CDP_DRV_RZ_PRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_TPIPE_PRZ_N_TAP;
                break;

                default:
                    LOG_ERR("Not support eRzName. eRzName: %d.", eRzName);
                    Result = MFALSE;
                    goto lbExit;
            }
        }
        break;

        case CDP_DRV_MODE_VRMRG:
        {
            switch (eRzName)
            {
                case CDP_DRV_RZ_CDRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_VRMRG_CDRZ_N_TAP;
                break;

                case CDP_DRV_RZ_CURZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_VRMRG_CURZ_4_TAP;
                break;

                case CDP_DRV_RZ_PRZ:
                u4SupportingWidth_4tap  = CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4_TAP;
                u4SupportingWidth_4Ntap = CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_4N_TAP;
                u4SupportingWidth_Ntap  = CDP_DRV_SUPPORT_WIDTH_VRMRG_PRZ_N_TAP;
                break;

                default:
                    LOG_ERR("Not support eRzName. eRzName: %d.", eRzName);
                    Result = MFALSE;
                    goto lbExit;
            }
        }
        break;

        default:
            LOG_ERR("Not support eFrameOrTpipeOrVrmrg. eFrameOrTileOrVrmrg: %d.", eFrameOrTpipeOrVrmrg);
            Result = MFALSE;
            goto lbExit;
    }

    // Calculate horizontal part.
    // Pick the smaller one as u4WidthToBeChecked.
    if (u4CroppedSize_H <= SizeOut_H)
    {
        u4SizeToBeChecked_H = u4CroppedSize_H;
    }
    else
    {
        u4SizeToBeChecked_H = SizeOut_H;
    }
    
    //====== Calculate Algo/CoeffStep/Table ======

    // 4-tap
    if( (eRzName == CDP_DRV_RZ_CURZ) ||        
        ((u4CroppedSize_H  * CDP_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_H &&    // 32x >= (Ratio = SizeOut/u4CroppedSize)
         (SizeOut_H * CDP_DRV_RZ_4_TAP_RATIO_MIN) >  u4CroppedSize_H)      // (Ratio = u4CroppedSize/SizeIn) > 1/2
      )
    {
        // Check supporting width.
        if( (eFrameOrTpipeOrVrmrg != CDP_DRV_MODE_TPIPE) && (u4SizeToBeChecked_H > u4SupportingWidth_4tap) )
        {
            LOG_ERR("Exceed supporting width. Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4tap);
            Result = MFALSE;
            goto lbExit;
        }
        //LOG_INF("Using 4_TAP: Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4tap);

        //     Decide Algorithm.
        *pAlgo_H = CDP_DRV_ALGO_4_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Mm1_H * CdpDrvRzUint[*pAlgo_H] + (Nm1_H >> 1)) / Nm1_H);
        //     Find Table.
        for ((*pTable_H) = 0; (*pTable_H) < CDP_DRV_RZ_TABLE_AMOUNT; (*pTable_H)++)
        {
            if ((*pCoeffStep_H) <= CdpDrvRzTable[*pTable_H])
            {
                break;
            }
        }

        // When Table exceed CDP_DRV_RZ_TABLE_AMOUNT, use last table.
        if ((*pTable_H) >= CDP_DRV_RZ_TABLE_AMOUNT)
        {
            (*pTable_H) = CDP_DRV_RZ_TABLE_AMOUNT - 1;
        }
    }
    else if     // 4N-tap
    (   (SizeOut_H * CDP_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_H &&    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.    //Vent@20120627: Joseph Lai suggests that when ratio is 1/2, accumulation should be used (i.e. 4n-tap).
        (SizeOut_H * CDP_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_H       // (Ratio = SizeOut/u4CroppedSize) >= 1/64.
    )
    {
        // Check supporting width.
        if ( (eFrameOrTpipeOrVrmrg != CDP_DRV_MODE_TPIPE) && (u4SizeToBeChecked_H > u4SupportingWidth_4Ntap) )   // Exceed supporting width, switch to N-tap.
        {
            LOG_DBG("Switch to N-tap because exceed support width. Mode: %d. eRzName: %d. u4CropSize_H: %d. SizeOut_H: %d. 4N-SupportWidth: %d. N-SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4Ntap, u4SupportingWidth_Ntap);
            //     Decide Algorithm.
            *pAlgo_H = CDP_DRV_ALGO_N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_H = (MUINT32)((Nm1_H * CdpDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
            //     Find Table.
            (*pTable_H) = CDP_DRV_RZ_N_TAP_TABLE;
        }
        else    // Supporting width check passed.
        {
            //LOG_INF("Using 4N-tap. Mode: %d. eRzName: %d. u4CropSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_4Ntap);
            //     Decide Algorithm.
            *pAlgo_H = CDP_DRV_ALGO_4N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_H = (MUINT32)((Nm1_H * CdpDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
            //     Find Table.
            (*pTable_H) = CDP_DRV_RZ_4N_TAP_TABLE;
        }

    }
#if 0   // no N-Tap in for horizontal MT6582 (confirmed by Joseph)

    else if     // N-tap
    (   (SizeOut_H * CDP_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_H &&     // (Ratio = SizeOut/SizeIn) < 1/64.
        (SizeOut_H * CDP_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_H       // (Ratio = SizeOut/SizeIn) >= 1/256
    ) 
    {

    
        // Check supporting width.
        if ( (eFrameOrTpipeOrVrmrg != CDP_DRV_MODE_TPIPE) && (u4SizeToBeChecked_H > u4SupportingWidth_Ntap) )
        {
            LOG_ERR("Exceed supporting width. Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_Ntap);
            Result = MFALSE;
            goto lbExit;
        }
        //LOG_INF("Using N-tap. Mode: %d. eRzName: %d. u4CroppedSize_H: %d. SizeOut_H: %d. SupportWidth: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_H, SizeOut_H, u4SupportingWidth_Ntap);

        //     Decide Algorithm.
        *pAlgo_H = CDP_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_H = (MUINT32)((Nm1_H * CdpDrvRzUint[*pAlgo_H] + Mm1_H - 1) / Mm1_H);
        //     Find Table.
        (*pTable_H) = CDP_DRV_RZ_N_TAP_TABLE;
    }
#endif
    else    // Ratio out of range.
    {
        LOG_ERR("Not support ratio. u4CroppedSize_H: %d, SizeOut_H: %d.", u4CroppedSize_H, SizeOut_H);
        Result = MFALSE;
        goto lbExit;
    }


    // Calculate vertical part.
    // Calculate Algo/CoeffStep/Table

    // 4-tap
    if( (eRzName == CDP_DRV_RZ_CURZ) ||
        ((u4CroppedSize_V  * CDP_DRV_RZ_4_TAP_RATIO_MAX) >= SizeOut_V &&        // 32x >= (Ratio = SizeOut/u4CroppedSize).
         (SizeOut_V * CDP_DRV_RZ_4_TAP_RATIO_MIN) >=  u4CroppedSize_V            // (Ratio = u4CroppedSize/SizeIn) > 1/2.
        )
      )
    {
        //     Decide Algorithm.
        *pAlgo_V = CDP_DRV_ALGO_4_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Mm1_V * CdpDrvRzUint[*pAlgo_V] + (Nm1_V >> 1)) / Nm1_V);
        //     Find Table.
        for ((*pTable_V) = 0; (*pTable_V) < CDP_DRV_RZ_TABLE_AMOUNT; (*pTable_V)++)
        {
            if ((*pCoeffStep_V) <= CdpDrvRzTable[*pTable_V])
            {
                break;
            }
        }

        // When Table exceed CDP_DRV_RZ_TABLE_AMOUNT, use last table.
        if ((*pTable_V) >= CDP_DRV_RZ_TABLE_AMOUNT)
        {
            (*pTable_V) = CDP_DRV_RZ_TABLE_AMOUNT - 1;
        }
    }
#if 0   // no 4N-Tap, N-Tap for vertical in MT6582 (confirmed by Joseph)

    else if     // 4N-tap
    (
        (SizeOut_V * CDP_DRV_RZ_4N_TAP_RATIO_MAX) <= u4CroppedSize_V &&    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.    //Vent@20120627: Joseph Lai suggests that when ratio is 1/2, accumulation should be used (i.e. 4n-tap).
        (SizeOut_V * CDP_DRV_RZ_4N_TAP_RATIO_MIN) >= u4CroppedSize_V)      // (Ratio = SizeOut/u4CroppedSize) >= 1/64.
    
    {
        // Check horizontal ratio.
        if ((SizeOut_H * CDP_DRV_RZ_4N_TAP_RATIO_MAX) >= u4CroppedSize_H )    // (Ratio = SizeOut/u4CroppedSize) <= 1/2.    //Vent@20120627: Joseph Lai said that only when "horizontal" ratio is at least 1/2 can the "vertical" 4n-tap be used.
        {
            LOG_DBG("Switch to N-tap because horizontal ratio not smaller than 1/2. Mode: %d. eRzName: %d. u4CropSize_V: %d. SizeOut_V: %d. u4CropSize_H: %d. SizeOut_H: %d.", eFrameOrTpipeOrVrmrg, eRzName, u4CroppedSize_V, SizeOut_V, u4CroppedSize_H, SizeOut_H);
            //     Decide Algorithm.
            *pAlgo_V = CDP_DRV_ALGO_N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_V = (MUINT32)((Nm1_V * CdpDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
            //     Find Table.
            (*pTable_V) = CDP_DRV_RZ_N_TAP_TABLE;
        }
        else    // Supporting width check passed.
        {
            //     Decide Algorithm.
            *pAlgo_V = CDP_DRV_ALGO_4N_TAP;
            //     Calculate CoefStep.
            *pCoeffStep_V = (MUINT32)((Nm1_V * CdpDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
            //     Find Table.
            (*pTable_V) = CDP_DRV_RZ_4N_TAP_TABLE;
        }
    }
    else if     // N-tap
    (
        (SizeOut_V * CDP_DRV_RZ_N_TAP_RATIO_MAX) <  u4CroppedSize_V &&     // (Ratio = SizeOut/SizeIn) < 1/64.
        (SizeOut_V * CDP_DRV_RZ_N_TAP_RATIO_MIX) >= u4CroppedSize_V)       // (Ratio = SizeOut/SizeIn) >= 1/256.
    {
        //     Decide Algorithm.
        *pAlgo_V = CDP_DRV_ALGO_N_TAP;
        //     Calculate CoefStep.
        *pCoeffStep_V = (MUINT32)((Nm1_V * CdpDrvRzUint[*pAlgo_V] + Mm1_V - 1) / Mm1_V);
        //     Find Table.
        (*pTable_V) = CDP_DRV_RZ_N_TAP_TABLE;
    }
#endif
    else    // Ratio out of range.
    {
        LOG_ERR("Not support ratio. u4CroppedSize_V(%d),SizeOut_V(%d)", u4CroppedSize_V, SizeOut_V);
        Result = MFALSE;
        goto lbExit;
    }

lbExit:
    
    LOG_INF("- X. Result(%d),Algo(%d,%d),Table(%d,%d),CoeffStep(%d,%d)", Result, *pAlgo_H, *pAlgo_V, *pTable_H, *pTable_V, *pCoeffStep_H, *pCoeffStep_V);
    
    return Result;

}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CalOffset(
    CDP_DRV_ALGO_ENUM Algo,
    MBOOL   IsWidth,
    MUINT32 CoeffStep,
    MFLOAT  Offset,
    MUINT32 *pLumaInt,
    MUINT32 *pLumaSub,
    MUINT32 *pChromaInt,
    MUINT32 *pChromaSub)
{
    MUINT32 OffsetInt, OffsetSub;

    LOG_INF("+,Algo(%d),IsWidth(%d),CoeffStep(%d),Offset(%f)", Algo, IsWidth, CoeffStep, Offset);

    // Check if CDP Drv is ready.
    if (!CheckReady())
    {
        return MFALSE;
    }

    OffsetInt = floor(Offset);
    OffsetSub = CdpDrvRzUint[Algo] * (Offset - floor(Offset));

    // Calculate pChromaInt/pChromaSub according current algorithm.
    if (Algo == CDP_DRV_ALGO_4_TAP)
    {
        *pLumaInt = OffsetInt;
        *pLumaSub = OffsetSub;

        //
        if (IsWidth)
        {
            *pChromaInt = floor(Offset / (2.0)); // Because format is YUV422, so the width of chroma is half of Y.
            *pChromaSub = CdpDrvRzUint[Algo] * 2 * (Offset / (2.0) - floor(Offset / (2.0)));
        }
        else
        {
            *pChromaInt = (*pLumaInt);
            *pChromaSub = (*pLumaSub);
        }
    }
    else
    {
        *pLumaInt = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CdpDrvRzUint[Algo]) / CdpDrvRzUint[Algo];
        *pLumaSub = (OffsetInt * CoeffStep + OffsetSub * CoeffStep / CdpDrvRzUint[Algo]) % CdpDrvRzUint[Algo];
        *pChromaInt = (*pLumaInt);
        *pChromaSub = (*pLumaSub);
    }

    LOG_INF("-, LumaInt/Sub(%d, %d), ChromaInt/Sub(%d, %d).", *pLumaInt, *pLumaSub, *pChromaInt, *pChromaSub);
    return MTRUE;
}

/**************************************************************************
* @brief Allocate memory for rotation.
* 1. Check CDP Drv instance count. If count <= 0, it means no CDP Drv
* 2. Check if mpIspReg is ready.
*
* @param [IN]     Format          Image format of the rotate image.
* @param [IN]     Size            Image size of the rotate image.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::AllocateRotationBuf()
{
    MBOOL Result = MTRUE;


    LOG_INF("+,mSysramUsageCount(%d)", mSysramUsageCount);
    
#if CDP_DRV_BUF_SYSRAM
//    Mutex::Autolock lock(mLock);      // Already locked in Init().   // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

//    SYSRAM_ALLOC_STRUCT SysramAlloc;

    //TODO
    if (mFdSysram < 0)
    {
        LOG_ERR("No SYSRAM kernel drv.");
        Result = MFALSE;
        goto lbEXIT;
    }

    // Start to allocate SYSRAM.
	if (mSysramUsageCount == 0)	// If first user who allocated SYSRAM buffer.
    {
        SysramAlloc.Alignment = 0;
        //SysramAlloc.Size = LumaBufSize + ChromaBufSize;
        SysramAlloc.Size = CDP_DRV_BUF_SYSRAM_SIZE;    //Vent@20121016: Always allocate max CDP SYSRAM usage, because we only allocate once, but there maybe many thread who uses CDP has diff SYSRAM requirement at the same time.
        SysramAlloc.Addr = 0;
        SysramAlloc.TimeoutMS = 100;

        if (ioctl(mFdSysram, SYSRAM_ALLOC, &SysramAlloc) < 0)   // Allocate fail.
        {
            LOG_ERR("SYSRAM_ALLOC error.");
            Result = MFALSE;
            goto lbEXIT;
        }
        else    // Allocate success.
        {
            LOG_INF("Allocated success. SYSRAM base addr: 0x%08X.", SysramAlloc.Addr);
        }

	}   // End of if (mSysramUsageCount == 0).

	// Allocate/recalculate LumaBufAddr/ChromaBufAddr done. Increase SYSRAM usage count.
    android_atomic_inc(&mSysramUsageCount);

#elif CDP_DRV_BUF_PMEM  // Not used.

    if (
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd   >= 0 ||
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd >= 0
    )
    {
        LOG_DBG("Free memory, RotDma %d, Format %d, Size %d.", RotDma, Format, Size);
        FreeRotationBuf(RotDma);
    }

    //
    switch (Format)
    {
        case CDP_DRV_FORMAT_YUV422:
        case CDP_DRV_FORMAT_YUV420:
        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_XRGB8888:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = Size;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd);
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd);
            break;
        }

        case CDP_DRV_FORMAT_Y:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = Size;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd);
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  0;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = 0;
            break;
        }

        case CDP_DRV_FORMAT_RGB565:
        {
            //Luma
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = 0;
            mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr = 0;
            //Chroma
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size =  Size;
            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = (MUINT32)pmem_alloc_sync(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size, &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd));

            if (mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr == 0)
            {
                mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = -1;
                Result = MFALSE;
                goto lbEXIT;
            }

            mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr = (MUINT32)pmem_get_phys(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd);
            break;
        }

        default:
        {
            LOG_ERR("Unknown Format: %d.", Format);
            return MFALSE;
        }
    }

#endif  // Use diff memory type.

lbEXIT:
    LOG_INF("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);

    return Result;

}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::FreeRotationBuf()
{
    MBOOL Result = MTRUE;

    LOG_INF("+,mSysramUsageCount(%d)", mSysramUsageCount);

#if CDP_DRV_BUF_SYSRAM

//    Mutex::Autolock lock(mLock);  // Already locked in Uninit().

    SYSRAM_USER_ENUM SysramUser = SYSRAM_USER_VIDO;

    // Start to free SYSRAM.
    if (mSysramUsageCount > 1)  // More than one user.
    {
        android_atomic_dec(&mSysramUsageCount);
    }
    else if (mSysramUsageCount == 1)   // Last user, must do some un-init procedure.
    {
        android_atomic_dec(&mSysramUsageCount);

        // Free SYSRAM buffer.
        if (mFdSysram >= 0)
        {
            if (ioctl(mFdSysram, SYSRAM_FREE, &SysramUser) < 0)
            {
                LOG_ERR("SYSRAM_FREE error.");
                Result = MFALSE;
            }
        }

        // Reset mRotationBuf[i][j].
        for (MUINT32 i = 0; i < CDP_DRV_ROTDMA_AMOUNT; i++)
        {
            for (MUINT32 j = 0; j < CDP_DRV_LC_AMOUNT; j++)
            {
                // mRotationBuf[i][j].Fd = -1;  // Reset Fd to -1 can only be done after close(mFdSysram).
                mRotationBuf[i][j].Size    = 0;
                mRotationBuf[i][j].VirAddr = 0;
                mRotationBuf[i][j].PhyAddr = 0;
            }
        }

    }
    else // mSysramUsageCount <= 0. No SYSRAM user.
    {
        // do nothing.
        LOG_INF("No SYSRAM to free.");
    }

#elif CDP_DRV_BUF_PMEM

    for (i = 0; i < CDP_DRV_LC_AMOUNT; i++)
    {
        if (mRotationBuf[RotDma][i].Fd >= 0)
        {
            LOG_INF("LC(%d),Size(%d),VirAddr(0x%08X)", i, mRotationBuf[RotDma][i].Size, mRotationBuf[RotDma][i].VirAddr);
            pmem_free((MUINT8*)(mRotationBuf[RotDma][i].VirAddr), mRotationBuf[RotDma][i].Size, mRotationBuf[RotDma][i].Fd);
            mRotationBuf[RotDma][i].Fd = -1;
        }
    }

    //
#endif  // Diff memory type.


lbEXIT:
    LOG_INF("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);
    return Result;

}

/**************************************************************************
* @brief Change the address of LumaBuf and ChromaBuf in SYSRAM
*
* @param [IN]     RotDma          Use which RotDma
* @param [IN]     Format          Image format of the rotate image
* @param [IN]     Size            Image size of the rotate image
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail
**************************************************************************/
MBOOL CdpDrvImp::RecalculateRotationBufAddr(
    CDP_DRV_ROTDMA_ENUM     RotDma,
    MUINT32                 LumaBufSize,
    MUINT32                 ChromaBufSize
)
{
    MBOOL Result = MTRUE;
    MUINT32 *pLargerBufSize = NULL;
    MUINT32 *pSmallerBufSize = NULL;
    MUINT32 *pLargerBufAddr = NULL;
    MUINT32 *pSmallerBufAddr = NULL;

    LOG_DBG("+,Luma/Chroma BufSize:(%d, %d),mSysramUsageCount(%d)", LumaBufSize, ChromaBufSize, mSysramUsageCount);

    Mutex::Autolock lock(mLock);    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    // If exceed total SYSRAM size, exit.
    if ( (LumaBufSize + ChromaBufSize) > CDP_DRV_BUF_SYSRAM_SIZE)
    {
        LOG_ERR("Buffer size (%d) exceeds total SYSRAM size (%d).", (LumaBufSize + ChromaBufSize), CDP_DRV_BUF_SYSRAM_SIZE);
        LOG_ERR("SL_TEST_SMT SKIP  Result = MFALSE");
//SL TEST SMT hang problem        Result = MFALSE;
            goto lbEXIT;
    }

    // Decide mRotationBuf[RotDma][].PhyAddr.
    // Decide which is LargerBuf, which is SmallerBuf.
    if(LumaBufSize >= ChromaBufSize)
    {
        LOG_DBG("LumaBufSize >= ChromaBufSize.");

        pLargerBufSize  = &LumaBufSize;
        pSmallerBufSize = &ChromaBufSize;
        LOG_DBG("*pSmaller/LargerBufSize: %d, %d.", *pSmallerBufSize, *pLargerBufSize);

        pLargerBufAddr  = &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA  ].PhyAddr);
        pSmallerBufAddr = &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr);
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }
    else    // LumaBufSize < ChromaBufSize
    {
        LOG_DBG("LumaBufSize < ChromaBufSize.");

        pLargerBufSize  = &ChromaBufSize;
        pSmallerBufSize = &LumaBufSize;
        LOG_DBG("*pSmaller/LargerBufSize: %d, %d.", *pSmallerBufSize, *pLargerBufSize);

        pLargerBufAddr  = &(mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr);
        pSmallerBufAddr = &(mRotationBuf[RotDma][CDP_DRV_LC_LUMA  ].PhyAddr);
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }

    //     Always put SmallerBuf in Bank0, LargerBuf in Bank1.
    if(*pSmallerBufSize <= CDP_DRV_BUF_SYSRAM_BANK0_SIZE)    // SmallerBuf is smaller than Bank0 (32K), i.e. SmallerBuf can fit in Bank0.
    {
        LOG_DBG("Separate Allocation.");

        *pSmallerBufAddr = SysramAlloc.Addr;   // SmallerBuf at Bank0, SmallerBuf_start align to Bank0_START.

        if (*pLargerBufSize <= CDP_DRV_BUF_SYSRAM_BANK1_SIZE)     // LargerBuf <= Bank1 (48K), i.e. LargerBuf can fit in Bank1.
        {
            *pLargerBufAddr = (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_START);   // LargerBuf at Bank1, ChromaBuf_start align to Bank1_START.
        }
        else    // LargerBuf can't fit in Bank1.
        {
            *pLargerBufAddr = ((SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END) - *pLargerBufSize) >> 3 << 3;  // VIDO_BUF_BASE_ADDR* uust align to 8-byte.   // LargerBuf at Bank1 (overlap to Bank0), LargerBuf_end align to Bank1_END.

            // Check if overlap SmallerBuf.
            if ((*pSmallerBufAddr + *pSmallerBufSize) > *pLargerBufAddr)
            {
                LOG_ERR("Overlap SmallerBuf after align to 8-byte. S-/L-Addr: (0x%08x, 0x%08x). S-/L-Size: %d, %d.", *pSmallerBufAddr, *pLargerBufAddr, *pSmallerBufSize, *pLargerBufSize);
            }
        }
        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }
    else    // SmallerBuf is larger than Bank0 (32K), i.e. SmallerBuf will overlap to Bank1.
    {
        LOG_DBG("Adjacent Allocation.");

        *pSmallerBufAddr = SysramAlloc.Addr;
        *pLargerBufAddr  = (SysramAlloc.Addr + *pSmallerBufSize + 7 ) >> 3 << 3;    // VIDO_BUF_BASE_ADDR* uust align to 8-byte.

        // Check if exceed SYSRAM size.
        if ((*pLargerBufAddr + *pLargerBufSize) > (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END))
        {
            LOG_ERR("Exceed SYSRAM size after align to 8-byte. S-/L-Addr: (0x%08x, 0x%08x). S-/L-Size: %d, %d. Limit: 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr, *pSmallerBufSize, *pLargerBufSize, (SysramAlloc.Addr + CDP_DRV_BUF_SYSRAM_OFFSET_TO_BANK1_ADDR_END));
        }

        LOG_DBG("*pSmaller/LargerBufAddr: 0x%08x, 0x%08x.", *pSmallerBufAddr, *pLargerBufAddr);
    }

    // Decide mRotationBuf[RotDma][].Fd/Size/VirAddr
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd = mFdSysram;
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size = LumaBufSize;
    mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr = 0;

    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd = mFdSysram;
    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size = ChromaBufSize;
    mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr = 0;

    LOG_DBG("SysramAlloc::Alignment: %d, Size: %d, User: %d. Addr: 0x%08X. TimeoutMS: %d.", SysramAlloc.Alignment, SysramAlloc.Size, (MUINT32)SysramAlloc.User, SysramAlloc.Addr, SysramAlloc.TimeoutMS);
    LOG_DBG("mRotationBuf[%d][LUMA]::   Fd: %d. Size: %d. VirAddr: 0x%08X. PhyAddr: 0x%08X.",
        RotDma,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Fd,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].Size,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].VirAddr,
        mRotationBuf[RotDma][CDP_DRV_LC_LUMA].PhyAddr
    );
    LOG_DBG("mRotationBuf[%d][CHROMA]:: Fd: %d. Size: %d. VirAddr: 0x%08X. PhyAddr: 0x%08X.",
        RotDma,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Fd,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].Size,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].VirAddr,
        mRotationBuf[RotDma][CDP_DRV_LC_CHROMA].PhyAddr
    );


lbEXIT:
    LOG_DBG("- ,ret(%d),mSysramUsageCount(%d)", Result, mSysramUsageCount);

    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::Reset()
{
    LOG_DBG("+");

    if (!CheckReady())
    {
        return MFALSE;
    }
   
    CDRZ_Enable(MFALSE);
    CURZ_Enable(MFALSE);
#if 0 //js_test remove below later
    VRZ_Enable(MFALSE);
    VRZO_Enable(MFALSE);
    RSP_Enable(MFALSE);
#endif //js_test remove below later
    VIDO_Enable(MFALSE);
    PRZ_Enable(MFALSE);
    DISPO_Enable(MFALSE);

    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::ResetDefault()
{
    LOG_DBG("+");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    CDRZ_ResetDefault();
    CURZ_ResetDefault();
#if 0 //js_test remove below later
    VRZ_ResetDefault();
    VRZO_ResetDefault();
    RSP_ResetDefault();
#endif //js_test remove below later
    VIDO_ResetDefault();
    PRZ_ResetDefault();
    DISPO_ResetDefault();
    //
    return MTRUE;
}

/**************************************************************************
* @brief Mapping ENUM value used by pipe to ENUM value used by HW register.
*
* @param [IN]     eInFormat       Format ENUM value used by pipe.
* @param [IN]     eInPlane        Plane ENUM value used by pipe.
* @param [OUT]    pu4OutPlane     Plane ENUM value used by HW register.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::RotDmaEnumRemapping(
    CDP_DRV_FORMAT_ENUM eInFormat,
    CDP_DRV_PLANE_ENUM eInPlane,
    MUINT32 *pu4OutPlane)
{
    LOG_DBG("+,eInFormat(%d),eInPlane(%d)", eInFormat, eInPlane);
    MBOOL Result = MTRUE;
    MUINT32 u4OutPlane = 0;

    // Mapping VIDO_FORMAT_1(format)/VIDO_FORMAT_2(block type)/VIDO_FORMAT_3(plane)/VIDO_FORMAT_SEQ(data sequence).
    switch (eInFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            u4OutPlane = eInPlane;
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            if (eInPlane == CDP_DRV_PLANE_2)
            {
                u4OutPlane = CDP_DRV_YUV420_PLANE_2;
            }
            else if (eInPlane == CDP_DRV_PLANE_3)
            {
                u4OutPlane = CDP_DRV_YUV420_PLANE_3;
            }
            else    // CDP_DRV_PLANE_1 or other invalid value.
            {
                LOG_ERR("YUV420 format: eInPlane(%d) incorrect, set to default 0 (CDP_DRV_YUV420_PLANE_2).", eInPlane);
                u4OutPlane = CDP_DRV_YUV420_PLANE_2;
            }
        }
        break;

        case CDP_DRV_FORMAT_Y:
        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_RGB565:
        case CDP_DRV_FORMAT_XRGB8888:
        {
            u4OutPlane = CDP_DRV_PLANE_1;
        }
        break;

        default:
        {
            LOG_ERR("Unknown eInFormat(%d).", eInFormat);
            Result = MFALSE;
        }
    }

    // Output transformed result.
    *pu4OutPlane = u4OutPlane;
    //
    LOG_DBG("-,Result(%d),u4OutPlane(%d)", *pu4OutPlane);

    return Result;
}

/**************************************************************************
* @brief Mapping ENUM value used by pipe to ENUM value used by HW register.
*
* @param [IN]     eInFormat       Format ENUM value used by pipe.
* @param [IN]     eInPlane        Plane ENUM value used by pipe.
* @param [IN]     eInSequence     Sequence ENUM value used by pipe.
* @param [OUT]    pu4OutPlane     Plane ENUM value used by HW register.
* @param [OUT]    pu4OutSequence  Sequence ENUM value used by HW register.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::InputImgFormatCheck(
    CDP_DRV_FORMAT_ENUM    eInFormat,
    CDP_DRV_PLANE_ENUM     eInPlane,
    CDP_DRV_SEQUENCE_ENUM  eInSequence)
{
    LOG_DBG("+,eInFormat(%d),eInPlane(%d),eInSequence(%d)", eInFormat, eInPlane, eInSequence);
    MBOOL Result = MTRUE;

    // Mapping VIDO_FORMAT_1(format)/VIDO_FORMAT_2(block type)/VIDO_FORMAT_3(plane)/VIDO_FORMAT_SEQ(data sequence).
    switch (eInFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            if (
                ( eInPlane == CDP_DRV_PLANE_1 || eInPlane == CDP_DRV_PLANE_3 )   &&
                (
                    eInSequence == CDP_DRV_SEQUENCE_YVYU ||
                    eInSequence == CDP_DRV_SEQUENCE_YUYV ||
                    eInSequence == CDP_DRV_SEQUENCE_VYUY ||
                    eInSequence == CDP_DRV_SEQUENCE_UYVY
                )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else if (
                eInPlane == CDP_DRV_PLANE_2   &&
                ( eInSequence == CDP_DRV_SEQUENCE_VUVU || eInSequence == CDP_DRV_SEQUENCE_UVUV )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YUV422 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            if (
                ( eInPlane == CDP_DRV_PLANE_2 || eInPlane == CDP_DRV_PLANE_3 )   &&
                ( eInSequence == CDP_DRV_SEQUENCE_VUVU || eInSequence == CDP_DRV_SEQUENCE_UVUV )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YUV420 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_Y:
        {
            if ( (eInPlane == CDP_DRV_PLANE_1) && (eInSequence == CDP_DRV_SEQUENCE_Y) )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("YOnly format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_RGB888:
        case CDP_DRV_FORMAT_RGB565:
        {
            if (
                (eInPlane == CDP_DRV_PLANE_1) &&
                (eInSequence == CDP_DRV_SEQUENCE_RGB || eInSequence == CDP_DRV_SEQUENCE_BGR)
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("RGB888 and RGB565 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        case CDP_DRV_FORMAT_XRGB8888:
        {
            if (
                (eInPlane == CDP_DRV_PLANE_1) &&
                (
                    eInSequence == CDP_DRV_SEQUENCE_XRGB ||
                    eInSequence == CDP_DRV_SEQUENCE_XBGR ||
                    eInSequence == CDP_DRV_SEQUENCE_RGBX ||
                    eInSequence == CDP_DRV_SEQUENCE_BGRX
                )
            )
            {
                Result = MTRUE; // Check OK.
            }
            else
            {
                LOG_ERR("XRGB8888 format: eInPlane(%d) or eInSequence(%d) incorrect.", eInPlane, eInSequence);
                Result = MFALSE;    // Check NG.
            }
        }
        break;

        default:
        {
            LOG_ERR("Unknown eInFormat(%d).", eInFormat);
            Result = MFALSE;
        }
    }

    //
    LOG_DBG("-,Result(%d)");

    return Result;

}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DumpReg()
{
    LOG_DBG("+");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    CDRZ_DumpReg();
    CURZ_DumpReg();
#if 0 //js_test remove below later
    VRZ_DumpReg();
    VRZO_DumpReg();
    RSP_DumpReg();
#endif //js_test remove below later
    VIDO_DumpReg();
    PRZ_DumpReg();
    DISPO_DumpReg();
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    
    if(!CheckReady())
    {
        LOG_ERR("Please init first!");
        return MFALSE;
    }
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_EN2_SET, CDRZ_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2_SET, CDRZ_EN_SET, En);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_ResetDefault()
{
    LOG_DBG("+");
    
    if(!CheckReady())
    {
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_CONTROL                            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_INPUT_IMAGE                        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_OUTPUT_IMAGE                       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_HORIZONTAL_COEFF_STEP              , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_VERTICAL_COEFF_STEP                , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET   , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_DERING_1                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_DERING_2                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_DERING_1                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CDRZ_DERING_2                           , 0x00000000);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL                            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_INPUT_IMAGE                        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_OUTPUT_IMAGE                       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_HORIZONTAL_COEFF_STEP              , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_VERTICAL_COEFF_STEP                , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET   , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2                           , 0x00000000);
    }
    
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_DumpReg()
{
    LOG_DBG("+");
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("DERING_1                           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_DERING_1));
        LOG_DBG("DERING_2                           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_DERING_2));
        LOG_DBG("DERING_1                           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_DERING_1));
        LOG_DBG("DERING_2                           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CDRZ_DERING_2));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("DERING_1                           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1));
        LOG_DBG("DERING_2                           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2));
        LOG_DBG("DERING_1                           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1));
        LOG_DBG("DERING_2                           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2));
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    
    if(!CheckReady())
    {
        return MFALSE;
    }    
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_HORIZONTAL_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_HORIZONTAL_EN, En);
    }
   
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    
    if(!CheckReady())
    {
        return MFALSE;
    }    
   
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Vertical_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Vertical_EN, En);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_EnableFirst(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Vertical_First, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Vertical_First, En);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);
    
    if(!CheckReady())
    {
        return MFALSE;
    }    
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm, Algo);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Horizontal_Algorithm, Algo);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);
    
    if(!CheckReady())
    {
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Vertical_Algorithm, Algo);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Vertical_Algorithm, Algo);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
#if 0 //js_test remove below later
MBOOL CdpDrvImp::CDRZ_EnableDering(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Dering_en, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Dering_en, En);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::CDRZ_H_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_DERING_1, CDRZ_Dering_Threshold_1H, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_DERING_2, CDRZ_Dering_Threshold_2H, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1, CDRZ_Dering_Threshold_1H, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2, CDRZ_Dering_Threshold_2H, Threshold2);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::CDRZ_V_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_DERING_1, CDRZ_Dering_Threshold_1V, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_DERING_2, CDRZ_Dering_Threshold_2V, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_1, CDRZ_Dering_Threshold_1V, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_DERING_2, CDRZ_Dering_Threshold_2V, Threshold2);
    }
    //
    return MTRUE;
}
#endif  //js_test remove below later


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }
    
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Truncation_Bit_H, Bit);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Truncation_Bit_H, Bit);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }    

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Truncation_Bit_V, Bit);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Truncation_Bit_V, Bit);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Horizontal_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Horizontal_Table_Select, Table);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }
    

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CONTROL, CDRZ_Vertical_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CONTROL, CDRZ_Vertical_Table_Select, Table);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);

    if(!CheckReady())
    {
        return MFALSE;
    }

    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_W, Size);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_INPUT_IMAGE, CDRZ_Input_Image_H, Size);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_W, Size);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_OUTPUT_IMAGE, CDRZ_Output_Image_H, Size);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_HORIZONTAL_COEFF_STEP, CDRZ_Horizontal_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_HORIZONTAL_COEFF_STEP, CDRZ_Horizontal_Coeff_Step, CoeffStep);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_VERTICAL_COEFF_STEP, CDRZ_Vertical_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_VERTICAL_COEFF_STEP, CDRZ_Vertical_Coeff_Step, CoeffStep);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_H_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }
    
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     CDRZ_Luma_Horizontal_Integer_Offset       , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    CDRZ_Luma_Horizontal_Subpixel_Offset      , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   CDRZ_Chroma_Horizontal_Integer_Offset     , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  CDRZ_Chroma_Horizontal_Subpixel_Offset    , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     CDRZ_Luma_Horizontal_Integer_Offset       , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    CDRZ_Luma_Horizontal_Subpixel_Offset      , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   CDRZ_Chroma_Horizontal_Integer_Offset     , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  CDRZ_Chroma_Horizontal_Subpixel_Offset    , ChromaSub);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_V_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }

    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET,       CDRZ_Luma_Vertical_Integer_Offset     , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      CDRZ_Luma_Vertical_Subpixel_Offset    , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     CDRZ_Chroma_Vertical_Integer_Offset   , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    CDRZ_Chroma_Vertical_Subpixel_Offset  , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_INTEGER_OFFSET,       CDRZ_Luma_Vertical_Integer_Offset     , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      CDRZ_Luma_Vertical_Subpixel_Offset    , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     CDRZ_Chroma_Vertical_Integer_Offset   , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CDRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    CDRZ_Chroma_Vertical_Subpixel_Offset  , ChromaSub);
    }
    
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Config(
    CDP_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
    CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
    CDP_DRV_IMG_CROP_STRUCT     Crop)
{
    CDP_DRV_ALGO_ENUM Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt;
    MUINT32 LumaSub;
    MUINT32 ChromaInt;
    MUINT32 ChromaSub;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.
    
    LOG_DBG("In(%d,%d),Crop(%d,%d),Out(%d,%d),CropStart(%f,%f)",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else if( Crop.Width.Start < 0 ||
        Crop.Width.Size > SizeIn.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("[CDRZ_Config] Error crop. InWidth(%d),Crop.Width::Start: %f, Size: %d.", SizeIn.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }

    // ====== CDRZ Algo ======
    
    Result = CalAlgoAndCStep(
                eFrameOrTileOrVrmrg,
                CDP_DRV_RZ_CDRZ,
                SizeIn.Width,
                SizeIn.Height,
                Crop.Width.Size,
                Crop.Height.Size,
                SizeOut.Width,
                SizeOut.Height,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V);
    if(!Result)
    {
        LOG_ERR("CalAlgoAndCStep fail.");
        return MFALSE;
    }
    
    //====== Height ======
    
    if(!CalOffset(Algo_H, MTRUE, CoeffStep_H, Crop.Width.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail.");
        return MFALSE;
    }
    
    CDRZ_H_EnableScale(MTRUE);
    CDRZ_H_SetAlgo(Algo_H);
#if 0   //js_test remove below later
    CDRZ_H_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1H,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2H);
#endif  //js_test remove below later
    CDRZ_H_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    CDRZ_H_SetTable(Table_H);
    CDRZ_H_SetInputSize(SizeIn.Width);
    CDRZ_H_SetOutputSize(SizeOut.Width);
    CDRZ_H_SetCoeffStep(CoeffStep_H);
    CDRZ_H_SetOffset( LumaInt,
                      LumaSub,
                      ChromaInt,
                      ChromaSub);    

    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else if( Crop.Height.Start < 0 ||
        Crop.Height.Size > SizeIn.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop. InHeight: %d. Crop.Height::Start: %f, Size: %d.",SizeIn.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }
    
    //====== Vertical ======
    
    if(!CalOffset(Algo_V, MFALSE, CoeffStep_V, Crop.Height.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail.");
        return MFALSE;
    }
    
    CDRZ_V_EnableScale(MTRUE);
    CDRZ_V_EnableFirst(MFALSE);
    CDRZ_V_SetAlgo(Algo_V);
#if 0   //js_test remove below later
    CDRZ_V_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1V,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2V);
#endif  //js_test remove below later
    CDRZ_V_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    CDRZ_V_SetTable(Table_V);
    CDRZ_V_SetInputSize(SizeIn.Height);
    CDRZ_V_SetOutputSize(SizeOut.Height);
    CDRZ_V_SetCoeffStep(CoeffStep_V);
    CDRZ_V_SetOffset( LumaInt,
                      LumaSub,
                      ChromaInt,
                      ChromaSub);
    
    // CDRZ_EnableDering(MTRUE);    //js_test remove below later
    
    CDRZ_Enable(MTRUE);
    
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CDRZ_Unconfig()
{
    LOG_DBG("+");
    
    if(!CheckReady())
    {
        return MFALSE;
    }
    
    CDRZ_Enable(MFALSE);
    
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_EN2_SET, CURZ_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2_SET, CURZ_EN_SET, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_CONTROL                            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_INPUT_IMAGE                        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_OUTPUT_IMAGE                       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_HORIZONTAL_COEFF_STEP              , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_VERTICAL_COEFF_STEP                , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET   , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_DERING_1                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_DERING_2                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_DERING_1                           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CURZ_DERING_2                           , 0x00000000);

        #if 0 //js_test remove below later
        ISP_WRITE_REG(mpIspReg, CAM_EIS_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_EIS_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
        #endif //js_test remove below later
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL                            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_INPUT_IMAGE                        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_OUTPUT_IMAGE                       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_HORIZONTAL_COEFF_STEP              , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_VERTICAL_COEFF_STEP                , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET   , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1                           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2                           , 0x00000000);

        #if 0 //js_test remove below later
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
        #endif //js_test remove below later
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CONTROL                                = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                            = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP                  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET          = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("DERING_1                               = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_DERING_1));
        LOG_DBG("DERING_2                               = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_DERING_2));
        LOG_DBG("DERING_1                               = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_DERING_1));
        LOG_DBG("DERING_2                               = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CURZ_DERING_2));

        #if 0 //js_test remove below later
        LOG_DBG("EIS LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("EIS LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("EIS CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("EIS LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("EIS CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_EIS_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        #endif //js_test remove below later
    }
    else    // GDMA mode.
    {
        LOG_DBG("CONTROL                                = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                            = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP                  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET          = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("DERING_1                               = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1));
        LOG_DBG("DERING_2                               = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2));
        LOG_DBG("DERING_1                               = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1));
        LOG_DBG("DERING_2                               = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2));

        #if 0 //js_test remove below later
        LOG_DBG("EIS LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("EIS LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("EIS CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("EIS LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("EIS CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("EIS CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_EIS_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
        #endif //js_test remove below later
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CONTROL, CURZ_HORIZONTAL_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL, CURZ_HORIZONTAL_EN, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CONTROL, CURZ_Vertical_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL, CURZ_Vertical_EN, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
#if 0   //js_test remove below later
MBOOL CdpDrvImp::CURZ_EnableDering(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CONTROL, CURZ_Dering_en, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL, CURZ_Dering_en, En);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::CURZ_H_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_DERING_1, CURZ_Dering_Threshold_1H, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_DERING_2, CURZ_Dering_Threshold_2H, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1, CURZ_Dering_Threshold_1H, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2, CURZ_Dering_Threshold_2H, Threshold2);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::CURZ_V_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_DERING_1, CURZ_Dering_Threshold_1V, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_DERING_2, CURZ_Dering_Threshold_2V, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_1, CURZ_Dering_Threshold_1V, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_DERING_2, CURZ_Dering_Threshold_2V, Threshold2);
    }
    //
    return MTRUE;
}
#endif  //js_test remove below later

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CONTROL, CURZ_Horizontal_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL, CURZ_Horizontal_Table_Select, Table);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CONTROL, CURZ_Vertical_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CONTROL, CURZ_Vertical_Table_Select, Table);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_INPUT_IMAGE, CURZ_Input_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_INPUT_IMAGE, CURZ_Input_Image_W, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_INPUT_IMAGE, CURZ_Input_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_INPUT_IMAGE, CURZ_Input_Image_H, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size+1 > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    } else if (Size & 0x01) {
        LOG_DBG("Odd size handling. Original Size: %d.", Size);
        Size++;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_OUTPUT_IMAGE, CURZ_Output_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_OUTPUT_IMAGE, CURZ_Output_Image_W, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_OUTPUT_IMAGE, CURZ_Output_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_OUTPUT_IMAGE, CURZ_Output_Image_H, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_HORIZONTAL_COEFF_STEP, CURZ_Horizontal_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_HORIZONTAL_COEFF_STEP, CURZ_Horizontal_Coeff_Step, CoeffStep);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_VERTICAL_COEFF_STEP, CURZ_Vertical_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_VERTICAL_COEFF_STEP, CURZ_Vertical_Coeff_Step, CoeffStep);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_H_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }
   //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     CURZ_Luma_Horizontal_Integer_Offset   , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    CURZ_Luma_Horizontal_Subpixel_Offset  , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   CURZ_Chroma_Horizontal_Integer_Offset , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  CURZ_Chroma_Horizontal_Subpixel_Offset, ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     CURZ_Luma_Horizontal_Integer_Offset   , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    CURZ_Luma_Horizontal_Subpixel_Offset  , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   CURZ_Chroma_Horizontal_Integer_Offset , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  CURZ_Chroma_Horizontal_Subpixel_Offset, ChromaSub);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_V_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt(%d),LumaSub(%d),ChromaInt(%d),ChromaSub(%d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }
    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET,       CURZ_Luma_Vertical_Integer_Offset     , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      CURZ_Luma_Vertical_Subpixel_Offset    , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET,     CURZ_Chroma_Vertical_Integer_Offset   , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    CURZ_Chroma_Vertical_Subpixel_Offset  , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_INTEGER_OFFSET,       CURZ_Luma_Vertical_Integer_Offset     , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      CURZ_Luma_Vertical_Subpixel_Offset    , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_INTEGER_OFFSET,     CURZ_Chroma_Vertical_Integer_Offset   , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CURZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    CURZ_Chroma_Vertical_Subpixel_Offset  , ChromaSub);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_Config(
    CDP_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
    CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
    CDP_DRV_IMG_CROP_STRUCT     Crop)
{
    CDP_DRV_ALGO_ENUM Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt;
    MUINT32 LumaSub;
    MUINT32 ChromaInt;
    MUINT32 ChromaSub;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.

    //
    LOG_DBG("In(%d, %d). Crop(%d, %d). Out(%d, %d). CropStart(%f, %f).",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);
    //
    if (!CheckReady())
    {
        return MFALSE;
    }
    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else
    if( Crop.Width.Start < 0 ||
        Crop.Width.Size > SizeIn.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("Error crop. InWidth: %d. Crop.Width::Start: %f, Size: %d.",SizeIn.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }

    //
    Result = CalAlgoAndCStep(
                eFrameOrTileOrVrmrg,
                CDP_DRV_RZ_CURZ,
                SizeIn.Width,
                SizeIn.Height,
                Crop.Width.Size,
                Crop.Height.Size,
                SizeOut.Width,
                SizeOut.Height,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V
             );
    if (!Result)
    {
        LOG_ERR("CalAlgoAndCStep fail.");
        return MFALSE;
    }
    //
    if (!CalOffset(Algo_H, MTRUE, CoeffStep_H, Crop.Width.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail.");
        return MFALSE;
    }
    //
    CURZ_H_EnableScale(MTRUE);
#if 0   //js_test remove below later
    CURZ_H_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1H,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2H);
#endif  //js_test remove below later
    CURZ_H_SetTable(Table_H);
    CURZ_H_SetInputSize(SizeIn.Width);
    CURZ_H_SetOutputSize(SizeOut.Width);
    CURZ_H_SetCoeffStep(CoeffStep_H);
    CURZ_H_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //Height
    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else
    if( Crop.Height.Start < 0 ||
        Crop.Height.Size > SizeIn.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop. InHeight: %d. Crop.Height::Start: %f, Size: %d.",SizeIn.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }
    //
    if(!CalOffset(Algo_V, MFALSE, CoeffStep_V, Crop.Height.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail");
        return MFALSE;
    }
    //
    CURZ_V_EnableScale(MTRUE);
#if 0   //js_test remove below later
    CURZ_V_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1V,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2V);
#endif  //js_test remove below later
    CURZ_V_SetTable(Table_V);
    CURZ_V_SetInputSize(SizeIn.Height);
    CURZ_V_SetOutputSize(SizeOut.Height);
    CURZ_V_SetCoeffStep(CoeffStep_V);
    CURZ_V_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //
//    CURZ_EnableDering(MTRUE); //js_test remove below later
    //
    CURZ_Enable(MTRUE);
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::CURZ_Unconfig()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    CURZ_Enable(MFALSE);
    //
    return MTRUE;
}


#if VRZ_EXIST

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_EN2_SET, VRZ_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2_SET, VRZ_EN_SET, En);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_CONTROL                             , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_INPUT_IMAGE                         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_OUTPUT_IMAGE                        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_HORIZONTAL_COEFF_STEP               , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_VERTICAL_COEFF_STEP                 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CONTROL                             , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_INPUT_IMAGE                         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_OUTPUT_IMAGE                        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_HORIZONTAL_COEFF_STEP               , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_VERTICAL_COEFF_STEP                 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_H_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CONTROL, VRZ_HORIZONTAL_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CONTROL, VRZ_HORIZONTAL_EN, En);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_V_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CONTROL, VRZ_Vertical_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CONTROL, VRZ_Vertical_EN, En);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_H_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_INPUT_IMAGE, VRZ_Input_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_INPUT_IMAGE, VRZ_Input_Image_W, Size);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_V_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_INPUT_IMAGE, VRZ_Input_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_INPUT_IMAGE, VRZ_Input_Image_H, Size);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_OUTPUT_IMAGE, VRZ_Output_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_OUTPUT_IMAGE, VRZ_Output_Image_W, Size);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_OUTPUT_IMAGE, VRZ_Output_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_OUTPUT_IMAGE, VRZ_Output_Image_H, Size);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_HORIZONTAL_COEFF_STEP, VRZ_Horizontal_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_HORIZONTAL_COEFF_STEP, VRZ_Horizontal_Coeff_Step, CoeffStep);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_VERTICAL_COEFF_STEP, VRZ_Vertical_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_VERTICAL_COEFF_STEP, VRZ_Vertical_Coeff_Step, CoeffStep);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_H_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaIntSub(%d, %d),ChromaInt/Sub(%d, %d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     VRZ_Luma_Horizontal_Integer_Offset     , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    VRZ_Luma_Horizontal_Subpixel_Offset    , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   VRZ_Chroma_Horizontal_Integer_Offset   , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  VRZ_Chroma_Horizontal_Subpixel_Offset  , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     VRZ_Luma_Horizontal_Integer_Offset     , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    VRZ_Luma_Horizontal_Subpixel_Offset    , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   VRZ_Chroma_Horizontal_Integer_Offset   , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  VRZ_Chroma_Horizontal_Subpixel_Offset  , ChromaSub);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_V_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt/Sub(%d, %d), ChromaInt/Sub(%d, %d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET,       VRZ_Luma_Vertical_Integer_Offset   , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      VRZ_Luma_Vertical_Subpixel_Offset  , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     VRZ_Chroma_Vertical_Integer_Offset , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    VRZ_Chroma_Vertical_Subpixel_Offset, ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_INTEGER_OFFSET,       VRZ_Luma_Vertical_Integer_Offset   , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      VRZ_Luma_Vertical_Subpixel_Offset  , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     VRZ_Chroma_Vertical_Integer_Offset , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    VRZ_Chroma_Vertical_Subpixel_Offset, ChromaSub);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_Config(
    CDP_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
    CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
    CDP_DRV_IMG_CROP_STRUCT     Crop)
{
    MUINT32 CoeffStep;
    MUINT32 LumaInt;
    MUINT32 LumaSub;
    MUINT32 ChromaInt;
    MUINT32 ChromaSub;
    MUINT32 Mm1,Nm1;
    //
    LOG_DBG("In(%d, %d). Crop(%d, %d). Out(%d, %d). CropStart(%f, %f).",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else
    if( Crop.Width.Start < 0 ||
        Crop.Width.Size > SizeIn.Width ||
        Crop.Width.Size < SizeOut.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("Error crop,Width(%d),Crop.Width.Start(%f),Crop.Width.Size(%d)",SizeIn.Width,Crop.Width.Start,Crop.Width.Size);
        return MFALSE;
    }
    //
    Mm1 = Crop.Width.Size-1;
    Nm1 = SizeOut.Width-1;
    CoeffStep = (MUINT32)((Mm1*CdpDrvRzUint[CDP_DRV_ALGO_N_TAP]+Nm1-1)/Nm1);
    //
    if(!CalOffset(CDP_DRV_ALGO_N_TAP,MTRUE,CoeffStep,Crop.Width.Start,&LumaInt,&LumaSub,&ChromaInt,&ChromaSub))
    {
        LOG_ERR("CalOffset fail");
        return MFALSE;
    }
    //
    VRZ_H_EnableScale(MTRUE);
    VRZ_H_SetInputSize(SizeIn.Width);
    VRZ_H_SetOutputSize(SizeOut.Width);
    VRZ_H_SetCoeffStep(CoeffStep);
    VRZ_H_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //Height
    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else
    if( Crop.Height.Start < 0 ||
        Crop.Height.Size > SizeIn.Height ||
        Crop.Height.Size < SizeOut.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop,Height(%d),Crop.Height.Start(%f),Crop.Height.Size(%d)",SizeIn.Height,Crop.Height.Start,Crop.Height.Size);
        return MFALSE;
    }
    //
    Mm1 = Crop.Height.Size-1;
    Nm1 = SizeOut.Height-1;
    CoeffStep = (MUINT32)((Mm1*CdpDrvRzUint[CDP_DRV_ALGO_N_TAP]+Nm1-1)/Nm1);
    //
    if(!CalOffset(CDP_DRV_ALGO_N_TAP,MFALSE,CoeffStep,Crop.Height.Start,&LumaInt,&LumaSub,&ChromaInt,&ChromaSub))
    {
        LOG_ERR("CalOffset fail");
        return MFALSE;
    }
    //
    VRZ_V_EnableScale(MTRUE);
    VRZ_V_SetInputSize(SizeIn.Height);
    VRZ_V_SetOutputSize(SizeOut.Height);
    VRZ_V_SetCoeffStep(CoeffStep);
    VRZ_V_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //
    VRZ_Enable(MTRUE);
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZ_Unconfig()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    VRZ_Enable(MFALSE);
    //
    return MTRUE;
}


#endif  // VRZ_EXIST


#if RSP_EXIST

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_EN2, UV_CRSA_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2, UV_CRSA_EN, En);
    }
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_CRSP_CTRL           , CDP_DRV_DEFAULT_CRSP_CTRL);
        ISP_WRITE_REG(mpIspReg, CAM_CRSP_INT            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CRSP_OUTPUT_SIZE    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_CRSP_CSTEP_OFST     , 0x00000000);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CTRL           , CDP_DRV_DEFAULT_CRSP_CTRL);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_INT            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_OUTPUT_SIZE    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST     , 0x00000000);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CRSP_CTRL));
        LOG_DBG("INT            = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CRSP_INT));
        LOG_DBG("OUTPUT_SIZE    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CRSP_OUTPUT_SIZE));
        LOG_DBG("CSTEP_OFST     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_CRSP_CSTEP_OFST));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CTRL));
        LOG_DBG("INT            = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_INT));
        LOG_DBG("OUTPUT_SIZE    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_OUTPUT_SIZE));
        LOG_DBG("CSTEP_OFST     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST));
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_SetFormat(CDP_DRV_UV_FORMAT_ENUM Format)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CTRL, CRSP_OUTPUT_FORMAT, Format);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CTRL, CRSP_OUTPUT_FORMAT, Format);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_SetInterrupt(
    MBOOL   En,
    MBOOL   WriteClear)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CTRL, CRSP_INT_1_EN, En);
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CTRL, CRSP_INT_WC_EN, WriteClear);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CTRL, CRSP_INT_1_EN, En);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CTRL, CRSP_INT_WC_EN, WriteClear);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_GetInterruptStatus()
{
    LOG_DBG("");
    MBOOL Result = MFALSE;
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        Result = (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_CRSP_INT, CRSP_INT_1));
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_INT, CRSP_INT_1, Result);
    }
#endif

    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_OUTPUT_SIZE, CRSP_WIDTH, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_OUTPUT_SIZE, CRSP_WIDTH, Size);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_OUTPUT_SIZE, CRSP_HEIGHT, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_OUTPUT_SIZE, CRSP_HEIGHT, Size);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_RSP_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_RSP_COEFF_STEP);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CSTEP_OFST, CRSP_CSTEP_X, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST, CRSP_CSTEP_X, CoeffStep);
    }
#endif
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_RSP_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_RSP_COEFF_STEP);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CSTEP_OFST, CRSP_CSTEP_Y, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST, CRSP_CSTEP_Y, CoeffStep);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_H_SetOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_RSP_OFFSET)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",Offset,CDP_DRV_MASK_RSP_OFFSET);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CSTEP_OFST, CRSP_OFST_X, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST, CRSP_OFST_X, Offset);
    }
#endif
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::RSP_V_SetOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_RSP_OFFSET)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",Offset,CDP_DRV_MASK_RSP_OFFSET);
        return MFALSE;
    }
    //
#if 0 //js_test remove below later
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_CRSP_CSTEP_OFST, CRSP_OFST_Y, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CRSP_CSTEP_OFST, CRSP_OFST_Y, Offset);
    }
#endif
    //
    return MTRUE;
}

#endif  // RSP_EXIST

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_EN2_SET, PRZ_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_EN2_SET, PRZ_EN_SET, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_SetSource(CDP_DRV_PRZ_SRC_ENUM Source)
{
    LOG_DBG("Source(%d)",Source);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_SEL, PRZ_OPT_SEL, Source);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_SEL, PRZ_OPT_SEL, Source);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_CONTROL                             , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_INPUT_IMAGE                         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_OUTPUT_IMAGE                        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_HORIZONTAL_COEFF_STEP               , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_VERTICAL_COEFF_STEP                 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL                             , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_INPUT_IMAGE                         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_OUTPUT_IMAGE                        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_HORIZONTAL_COEFF_STEP               , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_VERTICAL_COEFF_STEP                 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET     , 0x00000000);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CONTROL                            = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL));
        LOG_DBG("INPUT_IMAGE                        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_INPUT_IMAGE));
        LOG_DBG("OUTPUT_IMAGE                       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_OUTPUT_IMAGE));
        LOG_DBG("HORIZONTAL_COEFF_STEP              = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_HORIZONTAL_COEFF_STEP));
        LOG_DBG("VERTICAL_COEFF_STEP                = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_VERTICAL_COEFF_STEP));
        LOG_DBG("LUMA_HORIZONTAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_HORIZONTAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_INTEGER_OFFSET   = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_HORIZONTAL_SUBPIXEL_OFFSET  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET));
        LOG_DBG("LUMA_VERTICAL_INTEGER_OFFSET       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("LUMA_VERTICAL_SUBPIXEL_OFFSET      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_INTEGER_OFFSET     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET));
        LOG_DBG("CHROMA_VERTICAL_SUBPIXEL_OFFSET    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET));
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_HORIZONTAL_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_HORIZONTAL_EN, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_EnableScale(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Vertical_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Vertical_EN, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_EnableFirst(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Vertical_First, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Vertical_First, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Horizontal_Algorithm, Algo);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Horizontal_Algorithm, Algo);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetAlgo(CDP_DRV_ALGO_ENUM Algo)
{
    LOG_DBG("Algo(%d)",Algo);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Vertical_Algorithm, Algo);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Vertical_Algorithm, Algo);
    }
    */
    return MTRUE;
}


//-----------------------------------------------------------------------------
#if 0   //js_test remove below later
MBOOL CdpDrvImp::PRZ_EnableDering(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Dering_en, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Dering_en, En);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::PRZ_H_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_DERING_1, PRZ_Dering_Threshold_1H, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_DERING_2, PRZ_Dering_Threshold_2H, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_DERING_1, PRZ_Dering_Threshold_1H, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_DERING_2, PRZ_Dering_Threshold_2H, Threshold2);
    }
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::PRZ_V_SetDeringThreshold(
    MUINT32     Threshold1,
    MUINT32     Threshold2)
{
    LOG_DBG("Threshold1(%d),Threshold2(%d)",Threshold1,Threshold2);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Threshold1 > CDP_DRV_MASK_DERING_THRESHOLD_1)
    {
        LOG_ERR("Threshold1(%d) is out of %d",Threshold1,CDP_DRV_MASK_DERING_THRESHOLD_1);
        return MFALSE;
    }
    //
    if(Threshold2 > CDP_DRV_MASK_DERING_THRESHOLD_2)
    {
        LOG_ERR("Threshold2(%d) is out of %d",Threshold2,CDP_DRV_MASK_DERING_THRESHOLD_2);
        return MFALSE;
    }
    //
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_DERING_1, PRZ_Dering_Threshold_1V, Threshold1);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_DERING_2, PRZ_Dering_Threshold_2V, Threshold2);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_DERING_1, PRZ_Dering_Threshold_1V, Threshold1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_DERING_2, PRZ_Dering_Threshold_2V, Threshold2);
    }
    //
    return MTRUE;
}
#endif  //js_test remove below later


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Truncation_Bit_H, Bit);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Truncation_Bit_H, Bit);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetTruncBit(MUINT32 Bit)
{
    LOG_DBG("Bit(%d)",Bit);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Bit > CDP_DRV_MASK_TRUNC_BIT)
    {
        LOG_ERR("Bit(%d) is out of %d",Bit,CDP_DRV_MASK_TRUNC_BIT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Truncation_Bit_V, Bit);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Truncation_Bit_V, Bit);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Horizontal_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Horizontal_Table_Select, Table);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetTable(MUINT32 Table)
{
    LOG_DBG("Table(%d)",Table);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Table > CDP_DRV_MASK_TABLE_SELECT)
    {
        LOG_ERR("Table(%d) is out of %d",Table,CDP_DRV_MASK_TABLE_SELECT);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CONTROL, PRZ_Vertical_Table_Select, Table);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CONTROL, PRZ_Vertical_Table_Select, Table);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size+1 > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    } else if (Size & 0x01) {
        LOG_DBG("Odd size handling. Original Size: %d.", Size);
        Size++;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_INPUT_IMAGE, PRZ_Input_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_INPUT_IMAGE, PRZ_Input_Image_W, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetInputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_INPUT_IMAGE, PRZ_Input_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_INPUT_IMAGE, PRZ_Input_Image_H, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size+1 > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    } else if (Size & 0x01) {
        LOG_DBG("Odd size handling. Original Size: %d.", Size);
        Size++;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_OUTPUT_IMAGE, PRZ_Output_Image_W, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_OUTPUT_IMAGE, PRZ_Output_Image_W, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_OUTPUT_IMAGE, PRZ_Output_Image_H, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_OUTPUT_IMAGE, PRZ_Output_Image_H, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_HORIZONTAL_COEFF_STEP, PRZ_Horizontal_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_HORIZONTAL_COEFF_STEP, PRZ_Horizontal_Coeff_Step, CoeffStep);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetCoeffStep(MUINT32 CoeffStep)
{
    LOG_DBG("CoeffStep(%d)",CoeffStep);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(CoeffStep > CDP_DRV_MASK_COEFF_STEP)
    {
        LOG_ERR("CoeffStep(%d) is out of %d",CoeffStep,CDP_DRV_MASK_COEFF_STEP);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_VERTICAL_COEFF_STEP, PRZ_Vertical_Coeff_Step, CoeffStep);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_VERTICAL_COEFF_STEP, PRZ_Vertical_Coeff_Step, CoeffStep);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_H_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt/Sub(%d, %d),ChromaInt/Sub(%d, %d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     PRZ_Luma_Horizontal_Integer_Offset     , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    PRZ_Luma_Horizontal_Subpixel_Offset    , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   PRZ_Chroma_Horizontal_Integer_Offset   , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  PRZ_Chroma_Horizontal_Subpixel_Offset  , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET,     PRZ_Luma_Horizontal_Integer_Offset     , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET,    PRZ_Luma_Horizontal_Subpixel_Offset    , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET,   PRZ_Chroma_Horizontal_Integer_Offset   , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET,  PRZ_Chroma_Horizontal_Subpixel_Offset  , ChromaSub);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_V_SetOffset(
    MUINT32     LumaInt,
    MUINT32     LumaSub,
    MUINT32     ChromaInt,
    MUINT32     ChromaSub)
{
    LOG_DBG("LumaInt/Sub(%d, %d),ChromaInt/Sub(%d, %d)",LumaInt,LumaSub,ChromaInt,ChromaSub);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaInt > CDP_DRV_MASK_INT_OFFSET ||
        ChromaInt > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("LumaInt(%d) or ChromaInt(%d) is out of %d",LumaInt,ChromaInt,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    if( LumaSub > CDP_DRV_MASK_SUB_OFFSET ||
        ChromaSub > CDP_DRV_MASK_SUB_OFFSET)
    {
        LOG_ERR("LumaSub(%d) or ChromaSub(%d) is out of %d",LumaSub,ChromaSub,CDP_DRV_MASK_SUB_OFFSET);
        return MFALSE;
    }

    ///To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET,       PRZ_Luma_Vertical_Integer_Offset       , LumaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      PRZ_Luma_Vertical_Subpixel_Offset      , LumaSub);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     PRZ_Chroma_Vertical_Integer_Offset     , ChromaInt);
        ISP_WRITE_BITS(mpIspReg, CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    PRZ_Chroma_Vertical_Subpixel_Offset    , ChromaSub);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_INTEGER_OFFSET,       PRZ_Luma_Vertical_Integer_Offset       , LumaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET,      PRZ_Luma_Vertical_Subpixel_Offset      , LumaSub);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_INTEGER_OFFSET,     PRZ_Chroma_Vertical_Integer_Offset     , ChromaInt);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET,    PRZ_Chroma_Vertical_Subpixel_Offset    , ChromaSub);
    }
   */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_Config(
    CDP_DRV_MODE_ENUM           eFrameOrTileOrVrmrg,
    CDP_DRV_IMG_SIZE_STRUCT     SizeIn,
    CDP_DRV_IMG_SIZE_STRUCT     SizeOut,
    CDP_DRV_IMG_CROP_STRUCT     Crop)
{
    CDP_DRV_ALGO_ENUM Algo_H, Algo_V;
    MUINT32 Table_H, Table_V;
    MUINT32 CoeffStep_H, CoeffStep_V;
    MUINT32 LumaInt;
    MUINT32 LumaSub;
    MUINT32 ChromaInt;
    MUINT32 ChromaSub;
    MBOOL Result = MTRUE;   // MTRUE: success. MFALSE: fail.

    //
    LOG_DBG("In(%d, %d). Crop(%d, %d). Out(%d, %d). CropStart(%f, %f).",SizeIn.Width, SizeIn.Height, Crop.Width.Size, Crop.Height.Size, SizeOut.Width, SizeOut.Height, Crop.Width.Start, Crop.Height.Start);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //Width
    if(Crop.Width.Size == 0)
    {
        Crop.Width.Start = 0;
        Crop.Width.Size = SizeIn.Width;
    }
    else
    if( Crop.Width.Start < 0 ||
        Crop.Width.Size > SizeIn.Width ||
        (Crop.Width.Start+Crop.Width.Size) > SizeIn.Width)
    {
        LOG_ERR("Error crop. InWidth: %d. Crop.Width::Start: %f, Size: %d.",SizeIn.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }
    //
    Result = CalAlgoAndCStep(
                eFrameOrTileOrVrmrg,
                CDP_DRV_RZ_PRZ,
                SizeIn.Width,
                SizeIn.Height,
                Crop.Width.Size,
                Crop.Height.Size,
                SizeOut.Width,
                SizeOut.Height,
                &Algo_H,
                &Algo_V,
                &Table_H,
                &Table_V,
                &CoeffStep_H,
                &CoeffStep_V
             );
    if (!Result)
    {
        LOG_ERR("CalAlgoAndCStep fail");
        return MFALSE;
    }
    //
    if(!CalOffset(Algo_H, MTRUE, CoeffStep_H, Crop.Width.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail");
        return MFALSE;
    }
    //
    PRZ_H_EnableScale(MTRUE);
    PRZ_H_SetAlgo(Algo_H);
#if 0   //js_test remove below later
    PRZ_H_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1H,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2H);
#endif  //js_test remove below later
    PRZ_H_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    PRZ_H_SetTable(Table_H);
    PRZ_H_SetInputSize(SizeIn.Width);
    PRZ_H_SetOutputSize(SizeOut.Width);
    PRZ_H_SetCoeffStep(CoeffStep_H);
    PRZ_H_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //Height
    if(Crop.Height.Size == 0)
    {
        Crop.Height.Start = 0;
        Crop.Height.Size = SizeIn.Height;
    }
    else
    if( Crop.Height.Start < 0 ||
        Crop.Height.Size > SizeIn.Height ||
        (Crop.Height.Start+Crop.Height.Size) > SizeIn.Height)
    {
        LOG_ERR("Error crop. InHeight: %d. Crop.Height::Start: %f, Size: %d.",SizeIn.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }
    //
    if(!CalOffset(Algo_V, MFALSE, CoeffStep_V, Crop.Height.Start, &LumaInt, &LumaSub, &ChromaInt, &ChromaSub))
    {
        LOG_ERR("CalOffset fail");
        return MFALSE;
    }
    //
    PRZ_V_EnableScale(MTRUE);
    PRZ_V_EnableFirst(MFALSE);
    PRZ_V_SetAlgo(Algo_V);
#if 0   //js_test remove below later
    PRZ_V_SetDeringThreshold(
        CDP_DRV_DEFAULT_DERING_THRESHOLD_1V,
        CDP_DRV_DEFAULT_DERING_THRESHOLD_2V);
#endif  //js_test remove below later
    PRZ_V_SetTruncBit(CDP_DRV_DEFAULT_TRUNC_BIT);
    PRZ_V_SetTable(Table_V);
    PRZ_V_SetInputSize(SizeIn.Height);
    PRZ_V_SetOutputSize(SizeOut.Height);
    PRZ_V_SetCoeffStep(CoeffStep_V);
    PRZ_V_SetOffset(
        LumaInt,
        LumaSub,
        ChromaInt,
        ChromaSub);
    //
//    PRZ_EnableDering(MTRUE);  //js_test remove below later
    //
    PRZ_Enable(MTRUE);
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::PRZ_Unconfig()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    PRZ_Enable(MFALSE);
    //
    return MTRUE;
}


#if VRZO_EXIST


/**************************************************************************
*
**************************************************************************/
#if 0 //js_test remove below later
MBOOL CdpDrvImp::VRZO_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_CTL_DMA_EN, VRZO_EN, En);
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZO_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_CTRL           , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_DMA_PERF       , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE  , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BUF_SIZE       , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR0 , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR1 , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR2 , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR3 , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_SOFT_RST       , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_INT_EN         , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_CROP_OFST      , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_TAR_SIZE       , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BASE_ADDR      , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_OFST_ADDR      , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_STRIDE         , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_BASE_ADDR_C    , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_OFST_ADDR_C    , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_STRIDE_C       , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_PADDING        , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_DITHER         , 0x00000000);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_DIT_SEED_R     , CDP_DRV_DEFAULT_DIT_SEED_R);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_DIT_SEED_G     , CDP_DRV_DEFAULT_DIT_SEED_G);
    ISP_WRITE_REG(mpIspReg, CAM_VRZO_DIT_SEED_B     , CDP_DRV_DEFAULT_DIT_SEED_B);
    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZO_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    LOG_DBG("CTRL           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_CTRL));
    LOG_DBG("DMA_PERF       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_DMA_PERF));
    LOG_DBG("MAIN_BUF_SIZE  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE));
    LOG_DBG("BUF_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BUF_SIZE));
    LOG_DBG("BUF_BASE_ADDR0 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR0));
    LOG_DBG("BUF_BASE_ADDR1 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR1));
    LOG_DBG("BUF_BASE_ADDR2 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR2));
    LOG_DBG("BUF_BASE_ADDR3 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BUF_BASE_ADDR3));
    LOG_DBG("SOFT_RST       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_SOFT_RST));
    LOG_DBG("INT_EN         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_INT_EN));
    LOG_DBG("CROP_OFST      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_CROP_OFST));
    LOG_DBG("TAR_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_TAR_SIZE));
    LOG_DBG("BASE_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BASE_ADDR));
    LOG_DBG("OFST_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_OFST_ADDR));
    LOG_DBG("STRIDE         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_STRIDE));
    LOG_DBG("BASE_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_BASE_ADDR_C));
    LOG_DBG("OFST_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_OFST_ADDR_C));
    LOG_DBG("STRIDE_C       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_STRIDE_C));
    LOG_DBG("PADDING        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_PADDING));
    LOG_DBG("DITHER         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_DITHER));
    LOG_DBG("DIT_SEED_R     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_DIT_SEED_R));
    LOG_DBG("DIT_SEED_G     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_DIT_SEED_G));
    LOG_DBG("DIT_SEED_B     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VRZO_DIT_SEED_B));
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VRZO_SetFormat(
    CDP_DRV_FORMAT_ENUM     Format,
    CDP_DRV_BLOCK_ENUM      Block,
    CDP_DRV_PLANE_ENUM      Plane,
    CDP_DRV_SEQUENCE_ENUM   Sequence)
{
    MBOOL Result = MTRUE;
    //
    LOG_DBG("- E. InFormat: %d. InBlock: %d. InPlane: %d. InSequence: %d.", Format, Block, Plane, Sequence);
    //
    if (!CheckReady())
    {
        return MFALSE;
    }

    // Check if input image format is valid. (Currently Only print out a warning message, won't block execution.)
    Result = InputImgFormatCheck(Format, Plane, Sequence);

    // Get correct Plane value for HW register according to input image format and plane.
    MUINT32 u4OutFormat = 0, u4OutPlane = 0;
    Result = RotDmaEnumRemapping(Format, Plane, &u4OutPlane);

    if (Result)
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_FORMAT_1   , Format);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_FORMAT_2   , Block);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_FORMAT_3   , u4OutPlane);
        ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_FORMAT_SEQ , Sequence);
    }
    else
    {
        LOG_ERR("Error Format(%d), Block(%d), Plane(%d), or Sequence(%d).", Format, Block, Plane, Sequence);
    }

    //
    LOG_DBG("- X. Result : %d. OutFormat: %d. OutBlock: %d. OutPlane: %d. OutSequence: %d.", Result, Format, Block, u4OutPlane, Sequence);

    return Result;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_EnableCrop(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_CROP_EN, En);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_H_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CROP_OFST, VRZO_CROP_OFST_X, Offset);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_V_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CROP_OFST, VRZO_CROP_OFST_Y, Offset);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode)
{
    LOG_DBG("PaddingMode(%d)",PaddingMode);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_PADDING_MODE, PaddingMode);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation)
{
    LOG_DBG("Rotation(%d)",Rotation);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_ROTATION, Rotation);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetRotationBuf(
    MBOOL       FIFO,
    MBOOL       DoubleBuf,
    MUINT32     LineNum,
    MUINT32     Width,
    MUINT32     Size)
{
    LOG_DBG("FIFO(%d),DoubleBuf(%d),LineNum(%d),Width(%d),Size(%d)",FIFO,DoubleBuf,LineNum,Width,Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(LineNum > CDP_DRV_MASK_BUF_LINE_NUM)
    {
        LOG_ERR("LineNum(%d) is out of %d",LineNum,CDP_DRV_MASK_BUF_LINE_NUM);
        return MFALSE;
    }
    //
    if(Width > CDP_DRV_MASK_BUF_WIDTH)
    {
        LOG_ERR("Width(%d) is out of %d",Width,CDP_DRV_MASK_BUF_WIDTH);
        return MFALSE;
    }
    //
    if( Size == 0 ||
        Size > CDP_DRV_MASK_BUF_SIZE)
    {
        LOG_DBG("Size(%d) is 0 or out of %d",Size,CDP_DRV_MASK_BUF_SIZE);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE,  VRZO_FIFO_MODE            , FIFO);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE,  VRZO_DOUBLE_BUF_MODE      , DoubleBuf);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE,  VRZO_MAIN_BUF_LINE_NUM    , LineNum);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_MAIN_BUF_SIZE,  VRZO_MAIN_BLK_WIDTH       , Width);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BUF_SIZE,       VRZO_MAIN_BUF_LINE_SIZE   , Size);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetRotationBufAddr(
    MUINT32     LumaAddr0,
    MUINT32     LumaAddr1,
    MUINT32     ChromaAddr0,
    MUINT32     ChromaAddr1)
{
    LOG_DBG("LumaAddr0(0x%08X),LumaAddr1(0x%08X),ChromaAddr0(0x%08X),ChromaAddr1(0x%08X)",LumaAddr0,LumaAddr1,ChromaAddr0,ChromaAddr1);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaAddr0 == 0 ||
        LumaAddr1 == 0 ||
        ChromaAddr0 == 0 ||
        ChromaAddr1 == 0)
    {
        LOG_DBG("LumaAddr0(0x%08X),LumaAddr1(0x%08X),ChromaAddr0(0x%08X) or ChromaAddr1(0x%08X) is 0",LumaAddr0,LumaAddr1,ChromaAddr0,ChromaAddr1);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BUF_BASE_ADDR0, VRZO_BUF_BASE_ADDR0   , LumaAddr0);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BUF_BASE_ADDR1, VRZO_BUF_BASE_ADDR1   , LumaAddr1);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BUF_BASE_ADDR2, VRZO_BUF_BASE_ADDR2   , ChromaAddr0);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BUF_BASE_ADDR3, VRZO_BUF_BASE_ADDR3   , ChromaAddr1);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_EnableFlip(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_FLIP, En);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_EnableSOFReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_SOF_RESET_EN, En);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_UV_XSEL, UV);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_CTRL, VRZO_UV_YSEL, UV);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetDMAPerf(
    MUINT32                 PriLowThr,
    MUINT32                 PriThr,
    CDP_DRV_BURST_LEN_ENUM  BurstLen)
{
    LOG_DBG("PriLowThr(%d),PriThr(%d),BurstLen(%d)",PriLowThr,PriThr,BurstLen);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(PriLowThr > CDP_DRV_MASK_FIFO_PRI_LOW_THR)
    {
        LOG_ERR("PriLowThr(%d) is out of %d",PriLowThr,CDP_DRV_MASK_FIFO_PRI_LOW_THR);
        return MFALSE;
    }
    //
    if(PriThr > CDP_DRV_MASK_FIFO_PRI_THR)
    {
        LOG_ERR("PriThr(%d) is out of %d",PriThr,CDP_DRV_MASK_FIFO_PRI_THR);
        return MFALSE;
    }
    //
    if(BurstLen > CDP_DRV_MASK_MAX_BURST_LEN)
    {
        LOG_ERR("BurstLen(%d) is out of %d",BurstLen,CDP_DRV_MASK_MAX_BURST_LEN);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DMA_PERF, VRZO_FIFO_PRI_LOW_THR   , PriLowThr);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DMA_PERF, VRZO_FIFO_PRI_THR       , PriThr);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DMA_PERF, VRZO_MAX_BURST_LEN      , BurstLen);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_EnableReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_SOFT_RST, VRZO_SOFT_RST, En);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_GetResetStatus(void)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    return (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_VRZO_SOFT_RST_STAT, VRZO_SOFT_RST_STAT));
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetInterrupt(
    MBOOL   En,
    MBOOL   WriteClear)
{
    LOG_DBG("En(%d),WriteClear(%d)",En,WriteClear);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_INT_EN, VRZO_INT_1_EN, En);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_INT_EN, VRZO_INT_WC_EN, WriteClear);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_GetInterruptStatus(void)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    return (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_VRZO_INT, VRZO_INT_1));
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_TAR_SIZE, VRZO_XSIZE, Size);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_TAR_SIZE, VRZO_YSIZE, Size);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetOutputAddr(
    MUINT32     PhyAddr,
    MUINT32     Offset,
    MUINT32     Stride,
    MUINT32     PhyAddrC,
    MUINT32     OffsetC,
    MUINT32     StrideC)
{
    LOG_DBG("PhyAddr(0x%08X),Offset(0x%08X),Stride(0x%08X)",PhyAddr,Offset,Stride);
    LOG_DBG("PhyAddrC(0x%08X),OffsetC(0x%08X),StrideC(0x%08X)",PhyAddrC,OffsetC,StrideC);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( Offset > CDP_DRV_MASK_ADDR_OFFSET ||
        OffsetC > CDP_DRV_MASK_ADDR_OFFSET)
    {
        LOG_ERR("Offset(0x%08X) or OffsetC(0x%08X) is out of 0x%08X",Offset,OffsetC,CDP_DRV_MASK_ADDR_OFFSET);
        return MFALSE;
    }
    //
    if( Stride > CDP_DRV_MASK_ADDR_STRIDE ||
        StrideC > CDP_DRV_MASK_ADDR_STRIDE)
    {
        LOG_ERR("Stride(0x%08X) or StrideC(0x%08X) is out of 0x%08X",Stride,StrideC,CDP_DRV_MASK_ADDR_STRIDE);
        return MFALSE;
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BASE_ADDR,  VRZO_BASE_ADDR    , PhyAddr);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_OFST_ADDR,  VRZO_OFST_ADDR    , Offset);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_STRIDE,     VRZO_STRIDE       , Stride);
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_BASE_ADDR_C,VRZO_BASE_ADDR_C  , PhyAddrC);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_OFST_ADDR_C,VRZO_OFST_ADDR_C  , OffsetC);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_STRIDE_C,   VRZO_STRIDE_C     , StrideC);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetCamera2DispPadding(
    MUINT32     Y,
    MUINT32     U,
    MUINT32     V)
{
    LOG_DBG("Y(%d),U(%d),V(%d)",Y,U,V);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( Y > CDP_DRV_MASK_PADDING ||
        U > CDP_DRV_MASK_PADDING ||
        V > CDP_DRV_MASK_PADDING)
    {
        LOG_ERR("Y(%d),U(%d) or V(%d ) is out of %d",Y,U,V,CDP_DRV_MASK_PADDING);
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_PADDING, VRZO_PADDING_Y, Y);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_PADDING, VRZO_PADDING_U, U);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_PADDING, VRZO_PADDING_V, V);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetDithering(
    MBOOL                   En,
    CDP_DRV_DIT_MODE_ENUM   Mode,
    MUINT32                 InitX,
    MUINT32                 InitY,
    MUINT32                 XRGBDummy)
{
    LOG_DBG("En(%d),Mode(%d),InitX(%d),InitY(%d),XRGBDummy(0x%02X)",En,Mode,InitX,InitY,XRGBDummy);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( InitX > CDP_DRV_MASK_DIT_INIT ||
        InitY > CDP_DRV_MASK_DIT_INIT)
    {
        LOG_ERR("InitX(%d) or InitY(%d) is out of %d",InitX,InitY,CDP_DRV_MASK_DIT_INIT);
    }
    //
    if(XRGBDummy > CDP_DRV_MASK_DIT_XRGB_DUMMY)
    {
        LOG_ERR("XRGBDummy(0x%02X) is out of 0x%02X",XRGBDummy,CDP_DRV_MASK_DIT_INIT);
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DITHER, VRZO_DIT_EN       , En);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DITHER, VRZO_DIT_MODE     , Mode);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DITHER, VRZO_DIT_INIT_X   , InitX);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DITHER, VRZO_DIT_INIT_Y   , InitY);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DITHER, VRZO_XRGB_DUMMY   , XRGBDummy);
    //
    return MTRUE;
}



//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_SetDitheringSeed(
    MUINT32     R,
    MUINT32     G,
    MUINT32     B)
{
    LOG_DBG("R(0x%08X),G(0x%08X),B(0x%08X)",R,G,B);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( R > CDP_DRV_MASK_DIT_SEED ||
        G > CDP_DRV_MASK_DIT_SEED ||
        B > CDP_DRV_MASK_DIT_SEED)
    {
        LOG_ERR("R(0x%08X),G(0x%08X) or B(0x%08X) is out of %d",R,G,B,CDP_DRV_MASK_DIT_SEED);
    }
    //
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DIT_SEED_R, VRZO_DIT_SEED_R, R);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DIT_SEED_G, VRZO_DIT_SEED_G, G);
    ISP_WRITE_BITS(mpIspReg, CAM_VRZO_DIT_SEED_B, VRZO_DIT_SEED_B, B);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_Config(
    CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
    CDP_DRV_IMG_CROP_STRUCT     Crop,
    CDP_DRV_FORMAT_ENUM         Format,
    CDP_DRV_PLANE_ENUM          Plane,
    CDP_DRV_SEQUENCE_ENUM       Sequence,
    CDP_DRV_ROTATION_ENUM       Rotation,
    MBOOL                       Flip)
{
    MUINT32 BufWidth,BufSize;
    //
    LOG_DBG("Width(%d),Height(%d),Format(%d),Plane(%d),Sequence(%d),Rotation(%d),Flip(%d)",ImgSize.Width,ImgSize.Height,Format,Plane,Sequence,Rotation,Flip);
    LOG_DBG("Crop.Width:Start(%f),Size(%d)",Crop.Width.Start,Crop.Width.Size);
    LOG_DBG("Crop.Height:Start(%f),Size(%d)",Crop.Height.Start,Crop.Height.Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //Width
    if(Crop.Width.Size == 0)
    {
        VRZO_H_SetCropOffset(0);
        VRZO_H_SetOutputSize(ImgSize.Width);
        Crop.Width.Size = ImgSize.Width;
    }
    else
    if( Crop.Width.Start < 0 ||
        (Crop.Width.Start+Crop.Width.Size) > ImgSize.Width ||
        (Crop.Width.Start-floor(Crop.Width.Start)) > 0)
    {
        LOG_ERR("Error crop,Width(%d),Crop.Width:Start(%f),Size(%d)",ImgSize.Width,Crop.Width.Start,Crop.Width.Size);
        return MFALSE;
    }
    else
    {
        VRZO_H_SetCropOffset(floor(Crop.Width.Start));
        VRZO_H_SetOutputSize(Crop.Width.Size);
    }
    //Height
    if(Crop.Height.Size == 0)
    {
        VRZO_V_SetCropOffset(0);
        VRZO_V_SetOutputSize(ImgSize.Height);
        Crop.Height.Size = ImgSize.Height;
    }
    else
    if( Crop.Height.Start < 0 ||
        (Crop.Height.Start+Crop.Height.Size) > ImgSize.Height ||
        (Crop.Height.Start-floor(Crop.Height.Start)) > 0)
    {
        LOG_ERR("Error crop,Height(%d),Crop.Height:Start(%f),Size(%d)",ImgSize.Height,Crop.Height.Start,Crop.Height.Size);
        return MFALSE;
    }
    else
    {
        VRZO_V_SetCropOffset(floor(Crop.Height.Start));
        VRZO_V_SetOutputSize(Crop.Height.Size);
    }
    //
    if(Format == CDP_DRV_FORMAT_YUV420)
    {
        VRZO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
        VRZO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
    }
    else
    {
        if( Rotation == CDP_DRV_ROTATION_0 ||
            Rotation == CDP_DRV_ROTATION_180)
        {
            VRZO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
            VRZO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
        }
        else
        {
            VRZO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
            VRZO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
        }
    }
    //
    VRZO_SetFormat(
        Format,
        CDP_DRV_BLOCK_SCAN_LINE,
        Plane,
        Sequence);
    VRZO_EnableCrop(MFALSE);
    VRZO_SetPaddingMode(CDP_DRV_PADDING_MODE_8);
    VRZO_SetRotation(Rotation);
    VRZO_EnableFlip(Flip);
    VRZO_EnableSOFReset(CDP_DRV_DEFAULT_SOF_RESET);
    VRZO_SetDMAPerf(
        CDP_DRV_DEFAULT_FIFO_PRI_LOW_THR,
        CDP_DRV_DEFAULT_FIFO_PRI_THR,
        CDP_DRV_BURST_LEN_4);
    VRZO_EnableReset(MFALSE);
    VRZO_SetInterrupt(MFALSE,MFALSE);
    VRZO_SetCamera2DispPadding(
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_Y,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_U,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_V);
    VRZO_SetDithering(
        MTRUE,
        CDP_DRV_DIT_MODE_16X16,
        CDP_DRV_DEFAULT_DIT_INIT_X,
        CDP_DRV_DEFAULT_DIT_INIT_Y,
        CDP_DRV_DEFAULT_DIT_XRGB_DUMMY);
    VRZO_SetDitheringSeed(
        CDP_DRV_DEFAULT_DIT_SEED_R,
        CDP_DRV_DEFAULT_DIT_SEED_G,
        CDP_DRV_DEFAULT_DIT_SEED_B);
    //
    if( Rotation == CDP_DRV_ROTATION_90 ||
        Rotation == CDP_DRV_ROTATION_270)
    {
        BufWidth = floor((Crop.Width.Size+CDP_DRV_DEFAULT_ROTATION_BUF_LINE_NUM-1)/CDP_DRV_DEFAULT_ROTATION_BUF_LINE_NUM);
        BufSize = BufWidth*CDP_DRV_DEFAULT_ROTATION_BUF_LINE_NUM;
        //
        if(!RecalculateRotationBufAddr(CDP_DRV_ROTDMA_VRZO,Format,BufSize))
        {
            LOG_ERR("Allocate buffer fail");
            return MFALSE;
        }
        VRZO_SetRotationBuf(
            MFALSE,
            MFALSE,
            CDP_DRV_DEFAULT_ROTATION_BUF_LINE_NUM,
            BufWidth,
            BufSize);
        VRZO_SetRotationBufAddr(
            mRotationBuf[CDP_DRV_ROTDMA_VRZO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VRZO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VRZO][CDP_DRV_LC_CHROMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VRZO][CDP_DRV_LC_CHROMA].PhyAddr);
    }
    //
    VRZO_Enable(MTRUE);
    //
    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL CdpDrvImp::VRZO_Unconfig(void)
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
//    FreeRotationBuf(CDP_DRV_ROTDMA_VRZO);    //Vent@20121201: SYSRAM Buf Free has been moved to CDP un-init.
    //
    VRZO_Enable(MFALSE);
    //
    return MTRUE;
}
#endif //js_test remove below later

#endif  // VRZO_EXIST

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_DMA_EN_SET, VIDO_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN_SET, VIDO_EN_SET, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_CTRL           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_DMA_PERF       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE  , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BUF_SIZE       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR0 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR1 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR2 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR3 , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_SOFT_RST       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_INT_EN         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_CROP_OFST      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_TAR_SIZE       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BASE_ADDR      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_OFST_ADDR      , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_STRIDE         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BASE_ADDR_C    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_OFST_ADDR_C    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_STRIDE_C       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_BASE_ADDR_V    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_OFST_ADDR_V    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_STRIDE_V       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_PADDING        , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_DITHER         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_DIT_SEED_R     , CDP_DRV_DEFAULT_DIT_SEED_R);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_DIT_SEED_G     , CDP_DRV_DEFAULT_DIT_SEED_G);
        ISP_WRITE_REG(mpIspReg, CAM_VIDO_DIT_SEED_B     , CDP_DRV_DEFAULT_DIT_SEED_B);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PERF       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE  , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_SIZE       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR0 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR1 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR2 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR3 , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_SOFT_RST       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_INT_EN         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CROP_OFST      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_TAR_SIZE       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR      , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_C    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_C    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_C       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_V    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_V    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_V       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_PADDING        , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_R     , CDP_DRV_DEFAULT_DIT_SEED_R);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_G     , CDP_DRV_DEFAULT_DIT_SEED_G);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_B     , CDP_DRV_DEFAULT_DIT_SEED_B);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_CTRL));
        LOG_DBG("DMA_PERF       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_DMA_PERF));
        LOG_DBG("MAIN_BUF_SIZE  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE));
        LOG_DBG("BUF_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BUF_SIZE));
        LOG_DBG("BUF_BASE_ADDR0 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR0));
        LOG_DBG("BUF_BASE_ADDR1 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR1));
        LOG_DBG("BUF_BASE_ADDR2 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR2));
        LOG_DBG("BUF_BASE_ADDR3 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BUF_BASE_ADDR3));
        LOG_DBG("SOFT_RST       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_SOFT_RST));
        LOG_DBG("INT_EN         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_INT_EN));
        LOG_DBG("CROP_OFST      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_CROP_OFST));
        LOG_DBG("TAR_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_TAR_SIZE));
        LOG_DBG("BASE_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BASE_ADDR));
        LOG_DBG("OFST_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_OFST_ADDR));
        LOG_DBG("STRIDE         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_STRIDE));
        LOG_DBG("BASE_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BASE_ADDR_C));
        LOG_DBG("OFST_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_OFST_ADDR_C));
        LOG_DBG("STRIDE_C       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_STRIDE_C));
        LOG_DBG("BASE_ADDR_V    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_BASE_ADDR_V));
        LOG_DBG("OFST_ADDR_V    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_OFST_ADDR_V));
        LOG_DBG("STRIDE_V       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_STRIDE_V));
        LOG_DBG("PADDING        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_PADDING));
        LOG_DBG("DITHER         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_DITHER));
        LOG_DBG("DIT_SEED_R     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_DIT_SEED_R));
        LOG_DBG("DIT_SEED_G     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_DIT_SEED_G));
        LOG_DBG("DIT_SEED_B     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_VIDO_DIT_SEED_B));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL));
        LOG_DBG("DMA_PERF       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PERF));
        LOG_DBG("MAIN_BUF_SIZE  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE));
        LOG_DBG("BUF_SIZE       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_SIZE));
        LOG_DBG("BUF_BASE_ADDR0 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR0));
        LOG_DBG("BUF_BASE_ADDR1 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR1));
        LOG_DBG("BUF_BASE_ADDR2 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR2));
        LOG_DBG("BUF_BASE_ADDR3 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR3));
        LOG_DBG("SOFT_RST       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_SOFT_RST));
        LOG_DBG("INT_EN         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_INT_EN));
        LOG_DBG("CROP_OFST      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CROP_OFST));
        LOG_DBG("TAR_SIZE       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_TAR_SIZE));
        LOG_DBG("BASE_ADDR      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR));
        LOG_DBG("OFST_ADDR      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR));
        LOG_DBG("STRIDE         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE));
        LOG_DBG("BASE_ADDR_C    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_C));
        LOG_DBG("OFST_ADDR_C    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_C));
        LOG_DBG("STRIDE_C       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_C));
        LOG_DBG("BASE_ADDR_V    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_V));
        LOG_DBG("OFST_ADDR_V    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_V));
        LOG_DBG("STRIDE_V       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_V));
        LOG_DBG("PADDING        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_PADDING));
        LOG_DBG("DITHER         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER));
        LOG_DBG("DIT_SEED_R     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_R));
        LOG_DBG("DIT_SEED_G     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_G));
        LOG_DBG("DIT_SEED_B     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_B));
    }
   */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetFormat(
    CDP_DRV_FORMAT_ENUM     Format,
    CDP_DRV_BLOCK_ENUM      Block,
    CDP_DRV_PLANE_ENUM      Plane,
    CDP_DRV_SEQUENCE_ENUM   Sequence)
{
    MBOOL Result = MTRUE;
    //
    LOG_DBG(" - E. InFormat: %d. InBlock: %d. InPlane: %d. InSequence: %d.", Format, Block, Plane, Sequence);
    //
    if (!CheckReady())
    {
        return MFALSE;
    }

    // Check if input image format is valid. (Currently Only print out a warning message, won't block execution.)
    Result = InputImgFormatCheck(Format, Plane, Sequence);

    // Get correct Plane value for HW register according to input image format and plane.
    MUINT32 u4OutFormat = 0, u4OutPlane = 0;
    Result = RotDmaEnumRemapping(Format, Plane, &u4OutPlane);

    if (Result)
    {//To Do : Need modify on MT6582
        /*
        if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
        {
            ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_FORMAT_1   , Format);
            ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_FORMAT_2   , Block);
            ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_FORMAT_3   , u4OutPlane);
            ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_FORMAT_SEQ , Sequence);
        }
        else    // GDMA mode.
        {
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_FORMAT_1   , Format);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_FORMAT_2   , Block);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_FORMAT_3   , u4OutPlane);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_FORMAT_SEQ , Sequence);
        }
        */
    }
    else
    {
        LOG_ERR("Error Format(%d), Block(%d), Plane(%d), or Sequence(%d).", Format, Block, Plane, Sequence);
    }

    //
    LOG_DBG(" - X. Result : %d. OutFormat: %d. OutBlock: %d. OutPlane: %d. OutSequence: %d.", Result, Format, Block, u4OutPlane, Sequence);

    return Result;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_EnableCrop(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if (!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_CROP_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_CROP_EN, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_H_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CROP_OFST, VIDO_CROP_OFST_X, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CROP_OFST, VIDO_CROP_OFST_X, Offset);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_V_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CROP_OFST, VIDO_CROP_OFST_Y, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CROP_OFST, VIDO_CROP_OFST_Y, Offset);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode)
{
    LOG_DBG("PaddingMode(%d)",PaddingMode);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_PADDING_MODE, PaddingMode);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_PADDING_MODE, PaddingMode);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation)
{
    LOG_DBG("Rotation(%d)",Rotation);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_ROTATION, Rotation);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_ROTATION, Rotation);
    }
    */
    return MTRUE;
}


/**************************************************************************
* @brief Calculate and set parameters (i.e. registers) about VIDO Rotation Buffer except Luma/Chroma Buf addresses.
*
* 1. Calculate u4LumaWidth/u4ChromaWidth.
* 2. Decide u4LumaLineNum/u4ChromaLineNum according to eFormat and ePlane.
* 3. Calculate u4LumaBlockWidth/u4ChromaBlockWidth.
* 4. Calculate u4LumaLineSize/u4ChromaLineSize.
* 5. Calculate u4LumaBufSize/u4ChromaBufSize.
*
* @param [IN]     eFormat             Image format of the rotate image.
* @param [IN]     ePlane              Plane of the rotate image.
* @param [IN]     u4TpipeWidth         Used tpipe width.
* @param [IN]     fgFifoMode          Use FIFO Mode or not.
* @param [IN]     fgDoubleBufMode     Use Double Buffer Mode or not.
* @param [OUT]    pu4LumaBufSize      Size of Luma Buffer.
* @param [OUT]    pu4ChromaBufSize    Size of Chroma Buffer.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::VIDO_RotationBufConfig(
    CDP_DRV_FORMAT_ENUM eFormat,
    CDP_DRV_PLANE_ENUM ePlane,
    MUINT32 u4TpipeWidth,
    MBOOL   fgFifoMode,
    MBOOL   fgDoubleBufMode,
    MUINT32 *pu4LumaBufSize,
    MUINT32 *pu4ChromaBufSize
)
{
    MBOOL ret = MTRUE;  // MTRUE: OK.
    LOG_DBG("- E. eFormat: %d. ePlane: %d. u4TpipeWidth: %d. fgFifoMode: %d. fgDoubleBufMode: %d.", eFormat, ePlane, u4TpipeWidth, fgFifoMode, fgDoubleBufMode);

    // Intermediate variable (that used to calculate final result).
    MUINT32 u4LumaWidth             = 0;
    MUINT32 u4ChromaWidth           = 0;
    MUINT32 u4ChromaLineNum         = 0;
    MUINT32 u4ChromaBlockWidth      = 0;
    // Final Result (that will be set to regiters.)
    MUINT32 u4LumaLineNum           = 0;
    MUINT32 u4LumaBlockWidth        = 0;
    MUINT32 u4LumaLineSize          = 0;    // MainBufLineSize.
    MUINT32 u4ChromaLineSize        = 0;    // SubBufLineSize.
    MUINT32 u4LumaBufSize           = 0;
    MUINT32 u4ChromaBufSize         = 0;

    // Calculate u4LumaWidth/u4ChromaWidth.
    if (eFormat == CDP_DRV_FORMAT_YUV420)    // For YUV420, Chroma width only half of Luma width.
    {
        u4LumaWidth     = u4TpipeWidth;
        u4ChromaWidth   = u4LumaWidth / 2;
    }
    else    // For other format, Chroma width == Luma width.
    {
        u4LumaWidth     = u4TpipeWidth;
        u4ChromaWidth   = u4LumaWidth;
    }

    // Decide u4LumaLineNum/u4ChromaLineNum according to eFormat and ePlane.
    switch (eFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            switch (ePlane)
            {
                case CDP_DRV_PLANE_1:
                u4LumaLineNum   = 32;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_2:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_3:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum;
                break;

                default:
                LOG_DBG("Unknown YUV422 plane, u4LumaLineNum/u4ChromaLineNum will use 1-plane setting.");
                u4LumaLineNum   = 32;
                u4ChromaLineNum = u4LumaLineNum / 2;
            }
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            switch (ePlane)
            {
                case CDP_DRV_PLANE_2:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_3:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum;
                break;

                default:
                LOG_DBG("Unknown YUV420 plane, u4LumaLineNum/u4ChromaLineNum will use 2-plane setting.");
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
            }
       }
        break;

        case CDP_DRV_FORMAT_Y:
        u4LumaLineNum   = 64;
        u4ChromaLineNum = 0;
        break;

        case CDP_DRV_FORMAT_RGB888:
        u4LumaLineNum   = 22;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        case CDP_DRV_FORMAT_RGB565:
        u4LumaLineNum   = 32;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        case CDP_DRV_FORMAT_XRGB8888:
        u4LumaLineNum   = 16;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        default:
        LOG_DBG("Unknown file format, u4LumaLineNum/u4ChromaLineNum will use YUV422 1-plane setting.");
        u4LumaLineNum   = 32;
        u4ChromaLineNum = u4LumaLineNum / 2;

    }

    // Calculate u4LumaBlockWidth/u4ChromaBlockWidth. BlockWidth = roundup(Width/LineNum).
    u4LumaBlockWidth   = (u4LumaWidth   + u4LumaLineNum   - 1) / u4LumaLineNum;
    u4ChromaBlockWidth = (u4ChromaWidth + u4ChromaLineNum - 1) / u4ChromaLineNum;

    // Calculate u4LumaLineSize/u4ChromaLineSize. LineSize = "(LineNum+1) * BlockWidth (FIFO mode)" or "LineNum * BlockWidth (non-FIFO mode)".
    u4LumaLineSize   = (u4LumaLineNum   + fgFifoMode) * u4LumaBlockWidth;
    u4ChromaLineSize = (u4ChromaLineNum + fgFifoMode) * u4ChromaBlockWidth;
    //     Special case for YUV420 3-plane. Chroma LineSize = (LineNum + FifoMode) * BlockWidth + u4ChromaBlockWidth
    if ((eFormat == CDP_DRV_FORMAT_YUV420) && (ePlane == CDP_DRV_PLANE_3) )
    {
        u4ChromaLineSize += u4ChromaBlockWidth;
    }

    // Calculate u4LumaBufSize/u4ChromaBufSize. BufSize = (LineNum + FifoMode) * LineSize.
    u4LumaBufSize   = (u4LumaLineNum   + fgFifoMode) * u4LumaLineSize;
    u4ChromaBufSize = (u4ChromaLineNum + fgFifoMode) * u4ChromaLineSize * 2;
    //     Check if total bufsize exceeds SYSRAM size for safety.
    if ( (u4LumaBufSize + u4ChromaBufSize) > 81920)
    {
        LOG_ERR("Total VIDO Rotation buf size (%d) exceeds SYSRAM size (81920 bytes).", u4LumaBufSize + u4ChromaBufSize);
    }

    #if 1   // For debug.
    LOG_DBG("u4LumaWidth/u4ChromaWidth:           %d, %d.", u4LumaWidth, u4ChromaWidth);
    LOG_DBG("u4LumaLineNum/u4ChromaLineNum:       %d, %d.", u4LumaLineNum, u4ChromaLineNum);
    LOG_DBG("u4LumaBlockWidth/u4ChromaBlockWidth: %d, %d.", u4LumaBlockWidth, u4ChromaBlockWidth);
    LOG_DBG("u4LumaLineSize/u4ChromaLineSize:     %d, %d.", u4LumaLineSize, u4ChromaLineSize);
    LOG_DBG("u4LumaBufSize/u4ChromaBufSize:       %d, %d.", u4LumaBufSize, u4ChromaBufSize);
    #endif  // For debug.

    // Update registers.

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE,  VIDO_FIFO_MODE            , fgFifoMode);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE,  VIDO_DOUBLE_BUF_MODE      , fgDoubleBufMode);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE,  VIDO_MAIN_BUF_LINE_NUM    , u4LumaLineNum);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_MAIN_BUF_SIZE,  VIDO_MAIN_BLK_WIDTH       , u4LumaBlockWidth);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_SIZE,       VIDO_MAIN_BUF_LINE_SIZE   , u4LumaLineSize);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_SIZE,       VIDO_SUB_BUF_LINE_SIZE    , u4ChromaLineSize);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE,  VIDO_FIFO_MODE            , fgFifoMode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE,  VIDO_DOUBLE_BUF_MODE      , fgDoubleBufMode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE,  VIDO_MAIN_BUF_LINE_NUM    , u4LumaLineNum);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_MAIN_BUF_SIZE,  VIDO_MAIN_BLK_WIDTH       , u4LumaBlockWidth);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_SIZE,       VIDO_MAIN_BUF_LINE_SIZE   , u4LumaLineSize);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_SIZE,       VIDO_SUB_BUF_LINE_SIZE    , u4ChromaLineSize);
    }
    */

    // Output value.
    *pu4LumaBufSize     = u4LumaBufSize;
    *pu4ChromaBufSize   = u4ChromaBufSize;

    LOG_DBG(" - X. ret: %d. u4LumaBufSize: %d. u4ChromaBufSize: %d.", ret, *pu4LumaBufSize, *pu4ChromaBufSize);

    return ret;

}

/**************************************************************************
* Note: In Single buffer mode or FIFO mode, DISPO_BUF_BASE_ADDR0/1 should be equal;
* DISPO_BUF_BASE_ADDR2/3 should be equal. In Double buffer mode,
* DISPO_BUF_BASE_ADDR0/1 should be set to diff buffers; DISPO_BUF_BASE_ADDR2/3
* should be set to diff buffers. FIFO mode is recommended instead of double
* buffer mode, because their efficiency is the same but double buffer mode
* needs twice as large as FIFO mode.
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetRotationBufAddr(
    MUINT32     LumaAddr0,
    MUINT32     LumaAddr1,
    MUINT32     ChromaAddr0,
    MUINT32     ChromaAddr1)
{
    LOG_DBG(" - E. LumaAddr0/1: (0x%08X, 0x%08X). ChromaAddr0/1: (0x%08X, 0x%08X).", LumaAddr0, LumaAddr1, ChromaAddr0, ChromaAddr1);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaAddr0 == 0 ||
        LumaAddr1 == 0 ||
        ChromaAddr0 == 0 ||
        ChromaAddr1 == 0
    )
    {
        LOG_ERR("LumaAddr0(0x%08X), LumaAddr1(0x%08X), ChromaAddr0(0x%08X) or ChromaAddr1(0x%08X) is 0.", LumaAddr0, LumaAddr1, ChromaAddr0, ChromaAddr1);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_BASE_ADDR0, VIDO_BUF_BASE_ADDR0, LumaAddr0);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_BASE_ADDR1, VIDO_BUF_BASE_ADDR1, LumaAddr1);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_BASE_ADDR2, VIDO_BUF_BASE_ADDR2, ChromaAddr0);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BUF_BASE_ADDR3, VIDO_BUF_BASE_ADDR3, ChromaAddr1);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR0, VIDO_BUF_BASE_ADDR0, LumaAddr0);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR1, VIDO_BUF_BASE_ADDR1, LumaAddr1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR2, VIDO_BUF_BASE_ADDR2, ChromaAddr0);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BUF_BASE_ADDR3, VIDO_BUF_BASE_ADDR3, ChromaAddr1);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_EnableFlip(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_FLIP, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_FLIP, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_EnableSOFReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_SOF_RESET_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_SOF_RESET_EN, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_UV_XSEL, UV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_UV_XSEL, UV);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_UV_YSEL, UV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_UV_YSEL, UV);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetDMAPerf(
    MUINT32                 PriLowThr,
    MUINT32                 PriThr,
    CDP_DRV_BURST_LEN_ENUM  BurstLen)
{
    LOG_DBG("PriLowThr(%d),PriThr(%d),BurstLen(%d)",PriLowThr,PriThr,BurstLen);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(PriLowThr > CDP_DRV_MASK_FIFO_PRI_LOW_THR)
    {
        LOG_ERR("PriLowThr(%d) is out of %d",PriLowThr,CDP_DRV_MASK_FIFO_PRI_LOW_THR);
        return MFALSE;
    }
    //
    if(PriThr > CDP_DRV_MASK_FIFO_PRI_THR)
    {
        LOG_ERR("PriThr(%d) is out of %d",PriThr,CDP_DRV_MASK_FIFO_PRI_THR);
        return MFALSE;
    }
    //
    if(BurstLen > CDP_DRV_MASK_MAX_BURST_LEN)
    {
        LOG_ERR("BurstLen(%d) is out of %d",BurstLen,CDP_DRV_MASK_MAX_BURST_LEN);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DMA_PERF, VIDO_FIFO_PRI_LOW_THR   , PriLowThr);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DMA_PERF, VIDO_FIFO_PRI_THR       , PriThr);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DMA_PERF, VIDO_MAX_BURST_LEN      , BurstLen);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PERF, VIDO_FIFO_PRI_LOW_THR   , PriLowThr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PERF, VIDO_FIFO_PRI_THR       , PriThr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PERF, VIDO_MAX_BURST_LEN      , BurstLen);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_EnableReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_SOFT_RST, VIDO_SOFT_RST, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_SOFT_RST, VIDO_SOFT_RST, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_GetResetStatus()
{
    LOG_DBG("");
    MBOOL Result = MFALSE;
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        Result = ISP_READ_BITS(mpIspReg, CAM_VIDO_SOFT_RST_STAT, VIDO_SOFT_RST_STAT);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_SOFT_RST_STAT, VIDO_SOFT_RST_STAT, Result);
    }

    */
    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetInterrupt(
    MBOOL   En,
    MBOOL   WriteClear)
{
    LOG_DBG("En(%d),WriteClear(%d)",En,WriteClear);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_INT_EN, VIDO_INT_1_EN, En);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_INT_EN, VIDO_INT_WC_EN, WriteClear);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_INT_EN, VIDO_INT_1_EN, En);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_INT_EN, VIDO_INT_WC_EN, WriteClear);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_GetInterruptStatus()
{
    LOG_DBG("");
    MBOOL Result = MFALSE;
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        Result = (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_VIDO_INT, VIDO_INT_1));
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_INT, VIDO_INT_1, Result);
    }
    */
    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_TAR_SIZE, VIDO_XSIZE, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_TAR_SIZE, VIDO_XSIZE, Size);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_TAR_SIZE, VIDO_YSIZE, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_TAR_SIZE, VIDO_YSIZE, Size);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetOutputAddr(
    MUINT32     PhyAddr,
    MUINT32     Offset,
    MUINT32     Stride,
    MUINT32     PhyAddrC,
    MUINT32     OffsetC,
    MUINT32     StrideC,
    MUINT32     PhyAddrV,
    MUINT32     OffsetV,
    MUINT32     StrideV
)
{
    LOG_DBG("- E. PhyAddr/C/V: 0x%08X/0x%08X/0x%08X", PhyAddr, PhyAddrC, PhyAddrV);
    LOG_DBG("Ofst/C/V: 0x%08X/0x%08X/0x%08X. Stride/C/V: 0x%08X/0x%08X/0x%08X.", Offset, OffsetC, OffsetV, Stride, StrideC, StrideV);
    //
    if (!CheckReady())
    {
        return MFALSE;
    }
    //
    if(
        Offset  > CDP_DRV_MASK_ADDR_OFFSET  ||
        OffsetC > CDP_DRV_MASK_ADDR_OFFSET  ||
        OffsetV > CDP_DRV_MASK_ADDR_OFFSET
    )
    {
        LOG_ERR("Offset(0x%08X), OffsetC(0x%08X) or OffsetV(0x%08X) is out of 0x%08X", Offset, OffsetC, OffsetV, CDP_DRV_MASK_ADDR_OFFSET);
        return MFALSE;
    }
    //
    if(
        Stride  > CDP_DRV_MASK_ADDR_STRIDE  ||
        StrideC > CDP_DRV_MASK_ADDR_STRIDE  ||
        StrideV > CDP_DRV_MASK_ADDR_STRIDE
    )
    {
        LOG_ERR("Stride(0x%08X), StrideC(0x%08X) or StrideV(0x%08X) is out of 0x%08X", Stride, StrideC, StrideV, CDP_DRV_MASK_ADDR_STRIDE);
        return MFALSE;
    }

    LOG_INF("cqPhy(0x%x),Vido(0x%x),VidCo(0x%x),VidVo(0x%x)",
            mpIspReg, PhyAddr, PhyAddrC, PhyAddrV);     // added for debug

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BASE_ADDR,  VIDO_BASE_ADDR    , PhyAddr);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_OFST_ADDR,  VIDO_OFST_ADDR    , Offset);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_STRIDE,     VIDO_STRIDE       , Stride);
        //
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BASE_ADDR_C,VIDO_BASE_ADDR_C  , PhyAddrC);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_OFST_ADDR_C,VIDO_OFST_ADDR_C  , OffsetC);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_STRIDE_C,   VIDO_STRIDE_C     , StrideC);
        //
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_BASE_ADDR_V,VIDO_BASE_ADDR_V  , PhyAddrV);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_OFST_ADDR_V,VIDO_OFST_ADDR_V  , OffsetV);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_STRIDE_V,   VIDO_STRIDE_V     , StrideV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR,  VIDO_BASE_ADDR    , PhyAddr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR,  VIDO_OFST_ADDR    , Offset);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE,     VIDO_STRIDE       , Stride);
        //
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_C,VIDO_BASE_ADDR_C  , PhyAddrC);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_C,VIDO_OFST_ADDR_C  , OffsetC);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_C,   VIDO_STRIDE_C     , StrideC);
        //
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_BASE_ADDR_V,VIDO_BASE_ADDR_V  , PhyAddrV);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_OFST_ADDR_V,VIDO_OFST_ADDR_V  , OffsetV);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_STRIDE_V,   VIDO_STRIDE_V     , StrideV);
    }
    */
    LOG_DBG(" - X.");
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetCamera2DispPadding(
    MUINT32     Y,
    MUINT32     U,
    MUINT32     V)
{
    LOG_DBG("Y(%d),U(%d),V(%d)",Y,U,V);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( Y > CDP_DRV_MASK_PADDING ||
        U > CDP_DRV_MASK_PADDING ||
        V > CDP_DRV_MASK_PADDING)
    {
        LOG_ERR("Y(%d),U(%d) or V(%d ) is out of %d",Y,U,V,CDP_DRV_MASK_PADDING);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_PADDING, VIDO_PADDING_Y, Y);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_PADDING, VIDO_PADDING_U, U);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_PADDING, VIDO_PADDING_V, V);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_PADDING, VIDO_PADDING_Y, Y);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_PADDING, VIDO_PADDING_U, U);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_PADDING, VIDO_PADDING_V, V);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetDithering(
    MBOOL                   En,
    CDP_DRV_DIT_MODE_ENUM   Mode,
    MUINT32                 InitX,
    MUINT32                 InitY,
    MUINT32                 XRGBDummy)
{
    LOG_DBG("En(%d),Mode(%d),InitX(%d),InitY(%d),XRGBDummy(0x%02X)",En,Mode,InitX,InitY,XRGBDummy);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( InitX > CDP_DRV_MASK_DIT_INIT ||
        InitY > CDP_DRV_MASK_DIT_INIT)
    {
        LOG_ERR("InitX(%d) or InitY(%d) is out of %d",InitX,InitY,CDP_DRV_MASK_DIT_INIT);
    }
    //
    if(XRGBDummy > CDP_DRV_MASK_DIT_XRGB_DUMMY)
    {
        LOG_ERR("XRGBDummy(0x%02X) is out of 0x%02X",XRGBDummy,CDP_DRV_MASK_DIT_INIT);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DITHER, VIDO_DIT_EN       , En);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DITHER, VIDO_DIT_MODE     , Mode);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DITHER, VIDO_DIT_INIT_X   , InitX);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DITHER, VIDO_DIT_INIT_Y   , InitY);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DITHER, VIDO_XRGB_DUMMY   , XRGBDummy);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER, VIDO_DIT_EN       , En);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER, VIDO_DIT_MODE     , Mode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER, VIDO_DIT_INIT_X   , InitX);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER, VIDO_DIT_INIT_Y   , InitY);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DITHER, VIDO_XRGB_DUMMY   , XRGBDummy);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_SetDitheringSeed(
    MUINT32     R,
    MUINT32     G,
    MUINT32     B)
{
    LOG_DBG("R(0x%08X),G(0x%08X),B(0x%08X)",R,G,B);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( R > CDP_DRV_MASK_DIT_SEED ||
        G > CDP_DRV_MASK_DIT_SEED ||
        B > CDP_DRV_MASK_DIT_SEED)
    {
        LOG_ERR("R(0x%08X),G(0x%08X) or B(0x%08X) is out of %d",R,G,B,CDP_DRV_MASK_DIT_SEED);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DIT_SEED_R, VIDO_DIT_SEED_R, R);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DIT_SEED_G, VIDO_DIT_SEED_G, G);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DIT_SEED_B, VIDO_DIT_SEED_B, B);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_R, VIDO_DIT_SEED_R, R);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_G, VIDO_DIT_SEED_G, G);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DIT_SEED_B, VIDO_DIT_SEED_B, B);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_Config(
    CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
    CDP_DRV_IMG_CROP_STRUCT     Crop,
    CDP_DRV_FORMAT_ENUM         Format,
    CDP_DRV_PLANE_ENUM          Plane,
    CDP_DRV_SEQUENCE_ENUM       Sequence,
    CDP_DRV_ROTATION_ENUM       Rotation,
    MBOOL                       Flip,
    MUINT32                     u4TpipeWidth,
    MBOOL                       fgDitherEnable
)
{
    MUINT32 BufWidth, BufSize;
    //
    LOG_DBG("ImgSize: (%d, %d). Crop: (%d, %d). CropStart: (%f, %f).", ImgSize.Width, ImgSize.Height, Crop.Width.Size, Crop.Height.Size, Crop.Width.Start, Crop.Height.Start);
    LOG_DBG("Format: %d. Plane: %d. Sequence: %d. Rotation: %d. Flip: %d. u4TpipeWidth: %d.", Format, Plane, Sequence, Rotation, Flip, u4TpipeWidth);

    //
    if (!CheckReady())
    {
        return MFALSE;
    }

    //Width
    if (Crop.Width.Size == 0)
    {
        VIDO_H_SetCropOffset(0);
        VIDO_H_SetOutputSize(ImgSize.Width);
        Crop.Width.Size = ImgSize.Width;
    }
    else if(
        Crop.Width.Start < 0 ||
        (Crop.Width.Start + Crop.Width.Size) > ImgSize.Width ||
        (Crop.Width.Start - floor(Crop.Width.Start)) > 0
    )
    {
        LOG_ERR("Error crop, ImgWidth: %d. Crop.Width::Start: %f. Size: %d.", ImgSize.Width, Crop.Width.Start, Crop.Width.Size);
        return MFALSE;
    }
    else
    {
        VIDO_H_SetCropOffset(floor(Crop.Width.Start));
        VIDO_H_SetOutputSize(Crop.Width.Size);
    }
    //Height
    if (Crop.Height.Size == 0)
    {
        VIDO_V_SetCropOffset(0);
        VIDO_V_SetOutputSize(ImgSize.Height);
        Crop.Height.Size = ImgSize.Height;
    }
    else if (
        Crop.Height.Start < 0 ||
        (Crop.Height.Start + Crop.Height.Size) > ImgSize.Height ||
        (Crop.Height.Start - floor(Crop.Height.Start)) > 0)
    {
        LOG_ERR("Error crop, ImgHeight: %d. Crop.Height::Start: %f. Size: %d.", ImgSize.Height, Crop.Height.Start, Crop.Height.Size);
        return MFALSE;
    }
    else
    {
        VIDO_V_SetCropOffset(floor(Crop.Height.Start));
        VIDO_V_SetOutputSize(Crop.Height.Size);
    }

    // Decide VIDO_UV_XSEL/VIDO_UV_YSEL according to Ratation/Flip and image format.
    if (Format == CDP_DRV_FORMAT_YUV420)
    {
        switch (Rotation)
        {
            case CDP_DRV_ROTATION_0:
            {
                if (Flip)
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                else
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                break;
            }
            case CDP_DRV_ROTATION_90:
            {
                if (Flip)
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                }
                else
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                }
                break;
            }
            case CDP_DRV_ROTATION_180:
            {
                if (Flip)
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                else
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                break;
            }
            case CDP_DRV_ROTATION_270:
            {
                if (Flip)
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                else
                {
                    VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
                    VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_ODD);
                }
                break;
            }
            default:
            {
                LOG_ERR("Unknown Rotation(%d)",Rotation);
                return MFALSE;
            }
        }
    }
    else    // Other image format.
    {
        if( Rotation == CDP_DRV_ROTATION_0 ||
            Rotation == CDP_DRV_ROTATION_180)
        {
            VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
            VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
        }
        else
        {
            VIDO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
            VIDO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
        }
    }

    //
    VIDO_SetFormat(
        Format,
        CDP_DRV_BLOCK_SCAN_LINE,
        Plane,
        Sequence
    );
    VIDO_EnableCrop(MTRUE); // VIDO_CROP_EN. It must be enabled when in tpipe mode. For other mode, it could be enabled or disabled. So always set to enabled.
    VIDO_SetPaddingMode(CDP_DRV_PADDING_MODE_8);
    VIDO_SetRotation(Rotation);
    VIDO_EnableFlip(Flip);
    VIDO_EnableSOFReset(CDP_DRV_DEFAULT_SOF_RESET);
    VIDO_SetDMAPerf(
        CDP_DRV_DEFAULT_FIFO_PRI_LOW_THR,
        CDP_DRV_DEFAULT_FIFO_PRI_THR,
        CDP_DRV_BURST_LEN_4);
    VIDO_EnableReset(MFALSE);
    VIDO_SetInterrupt(MFALSE, MFALSE);
    VIDO_SetCamera2DispPadding(
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_Y,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_U,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_V);
    VIDO_SetDithering(
        fgDitherEnable,
        CDP_DRV_DIT_MODE_16X16,
        CDP_DRV_DEFAULT_DIT_INIT_X,
        CDP_DRV_DEFAULT_DIT_INIT_Y,
        CDP_DRV_DEFAULT_DIT_XRGB_DUMMY);
    VIDO_SetDitheringSeed(
        CDP_DRV_DEFAULT_DIT_SEED_R,
        CDP_DRV_DEFAULT_DIT_SEED_G,
        CDP_DRV_DEFAULT_DIT_SEED_B);
    //

    // Allocate SYSRAM for rotation.
    if( Rotation == CDP_DRV_ROTATION_90 ||
        Rotation == CDP_DRV_ROTATION_270)
    {
        MUINT32 u4LumaBufSize   = 0;
        MUINT32 u4ChromaBufSize = 0;

        VIDO_RotationBufConfig(
            Format,
            Plane,
            u4TpipeWidth,
            MTRUE,  // fgFifoMode
            MFALSE, // fgDoubleBufMode
            &u4LumaBufSize,
            &u4ChromaBufSize
        );

        //
        if (!RecalculateRotationBufAddr(CDP_DRV_ROTDMA_VIDO, u4LumaBufSize, u4ChromaBufSize))
        {
            LOG_ERR("Recalculate SYSRAM Buf Addr fail.");
            LOG_ERR("SKIP Recalculate SYSRAM Buf Addr fail.");
//            return MFALSE;
        }

        //
        VIDO_SetRotationBufAddr(
            mRotationBuf[CDP_DRV_ROTDMA_VIDO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VIDO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VIDO][CDP_DRV_LC_CHROMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_VIDO][CDP_DRV_LC_CHROMA].PhyAddr);
    }
    else    // When no rotate or 180 degree rotate, still recalculate SYSRAM Buf Addr. Not really necessary.
    {
        if (!RecalculateRotationBufAddr(CDP_DRV_ROTDMA_VIDO, CDP_DRV_BUF_SYSRAM_BANK0_SIZE-1, CDP_DRV_BUF_SYSRAM_BANK1_SIZE-1))    // CDP_DRV_BUF_SYSRAM_BANK[0,1]_SIZE-1: Use a special size to indicate that it's special case for non-rotate/180 degree rotate case.
        {
            LOG_ERR("Recalculate SYSRAM Buf Addr fail.");
            LOG_ERR("SKIP Recalculate SYSRAM Buf Addr fail.");
//            return MFALSE;
        }
    }

#if RSP_EXIST

    //
    RSP_H_SetOutputSize(ImgSize.Width);
    RSP_V_SetOutputSize(ImgSize.Height);
    //
    if(Format == CDP_DRV_FORMAT_YUV420)
    {
        if(Rotation == CDP_DRV_ROTATION_0)
        {
            RSP_H_SetCoeffStep(2);
            RSP_H_SetOffset(0);
            RSP_V_SetCoeffStep(4);
            RSP_V_SetOffset(1);
        }
        else
        {
            RSP_H_SetCoeffStep(2);
            RSP_H_SetOffset(1);
            RSP_V_SetCoeffStep(4);
            RSP_V_SetOffset(0);
        }
    }
    else
    {
        RSP_H_SetCoeffStep(2);
        RSP_H_SetOffset(0);
        RSP_V_SetCoeffStep(2);
        RSP_V_SetOffset(0);
    }

    RSP_SetFormat(CDP_DRV_UV_FORMAT_YUV444);
#endif  // RSP_EXIST

    // Enable DISPO Pre-ultra.
    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_CTRL, VIDO_PREULTRA_EN, 0x1);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DMA_PREULTRA, VIDO_FIFO_PREULTRA_THR, 0x10);
        ISP_WRITE_BITS(mpIspReg, CAM_VIDO_DMA_PREULTRA, VIDO_FIFO_PREULTRA_LOW_THR, 0x10);    //
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_CTRL, VIDO_PREULTRA_EN, 0x1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PREULTRA, VIDO_FIFO_PREULTRA_THR, 0x10);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_VIDO_DMA_PREULTRA, VIDO_FIFO_PREULTRA_LOW_THR, 0x10);    //
    }
    */
    VIDO_Enable(MTRUE);

#if RSP_EXIST
    RSP_Enable(MTRUE);
#endif  // RSP_EXIST

    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::VIDO_Unconfig()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
//    FreeRotationBuf(CDP_DRV_ROTDMA_VIDO);    //Vent@20121201: SYSRAM Buf Free has been moved to CDP un-init.
    //
    VIDO_Enable(MFALSE);

#if RSP_EXIST
    RSP_Enable(MFALSE);
#endif  // RSP_EXIST

    //
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_Enable(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

   //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_DMA_EN_SET, DISPO_EN_SET, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_DMA_EN_SET, DISPO_EN_SET, En);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetSource(CDP_DRV_DISPO_SRC_ENUM Source)
{
    LOG_DBG("Source(%d)",Source);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_ENABLE_BITS(mpIspReg, CAM_CTL_SEL, DISP_VID_SEL, Source);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_ENABLE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_CTL_SEL, DISP_VID_SEL, Source);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_ResetDefault()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_CTRL              , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_DMA_PERF          , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE     , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BUF_SIZE          , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR0    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR1    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR2    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR3    , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_SOFT_RST          , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_INT_EN            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_CROP_OFST         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_TAR_SIZE          , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BASE_ADDR         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_OFST_ADDR         , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_STRIDE            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_BASE_ADDR_C       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_OFST_ADDR_C       , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_STRIDE_C          , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_PADDING           , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_DITHER            , 0x00000000);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_DIT_SEED_R        , CDP_DRV_DEFAULT_DIT_SEED_R);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_DIT_SEED_G        , CDP_DRV_DEFAULT_DIT_SEED_G);
        ISP_WRITE_REG(mpIspReg, CAM_DISPO_DIT_SEED_B        , CDP_DRV_DEFAULT_DIT_SEED_B);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL              , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PERF          , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE     , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_SIZE          , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR0    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR1    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR2    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR3    , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_SOFT_RST          , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_INT_EN            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CROP_OFST         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_TAR_SIZE          , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR         , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR_C       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR_C       , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE_C          , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_PADDING           , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER            , 0x00000000);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_R        , CDP_DRV_DEFAULT_DIT_SEED_R);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_G        , CDP_DRV_DEFAULT_DIT_SEED_G);
        ISP_IOCTL_WRITE_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_B        , CDP_DRV_DEFAULT_DIT_SEED_B);
    }
   */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_DumpReg()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_CTRL));
        LOG_DBG("DMA_PERF       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_DMA_PERF));
        LOG_DBG("MAIN_BUF_SIZE  = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE));
        LOG_DBG("BUF_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BUF_SIZE));
        LOG_DBG("BUF_BASE_ADDR0 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR0));
        LOG_DBG("BUF_BASE_ADDR1 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR1));
        LOG_DBG("BUF_BASE_ADDR2 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR2));
        LOG_DBG("BUF_BASE_ADDR3 = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BUF_BASE_ADDR3));
        LOG_DBG("SOFT_RST       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_SOFT_RST));
        LOG_DBG("INT_EN         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_INT_EN));
        LOG_DBG("CROP_OFST      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_CROP_OFST));
        LOG_DBG("TAR_SIZE       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_TAR_SIZE));
        LOG_DBG("BASE_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BASE_ADDR));
        LOG_DBG("OFST_ADDR      = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_OFST_ADDR));
        LOG_DBG("STRIDE         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_STRIDE));
        LOG_DBG("BASE_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_BASE_ADDR_C));
        LOG_DBG("OFST_ADDR_C    = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_OFST_ADDR_C));
        LOG_DBG("STRIDE_C       = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_STRIDE_C));
        LOG_DBG("PADDING        = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_PADDING));
        LOG_DBG("DITHER         = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_DITHER));
        LOG_DBG("DIT_SEED_R     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_DIT_SEED_R));
        LOG_DBG("DIT_SEED_G     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_DIT_SEED_G));
        LOG_DBG("DIT_SEED_B     = 0x%08X",ISP_READ_REG(mpIspReg, CAM_DISPO_DIT_SEED_B));
    }
    else    // GDMA mode.
    {
        LOG_DBG("CTRL           = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL));
        LOG_DBG("DMA_PERF       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PERF));
        LOG_DBG("MAIN_BUF_SIZE  = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE));
        LOG_DBG("BUF_SIZE       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_SIZE));
        LOG_DBG("BUF_BASE_ADDR0 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR0));
        LOG_DBG("BUF_BASE_ADDR1 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR1));
        LOG_DBG("BUF_BASE_ADDR2 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR2));
        LOG_DBG("BUF_BASE_ADDR3 = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR3));
        LOG_DBG("SOFT_RST       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_SOFT_RST));
        LOG_DBG("INT_EN         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_INT_EN));
        LOG_DBG("CROP_OFST      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CROP_OFST));
        LOG_DBG("TAR_SIZE       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_TAR_SIZE));
        LOG_DBG("BASE_ADDR      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR));
        LOG_DBG("OFST_ADDR      = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR));
        LOG_DBG("STRIDE         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE));
        LOG_DBG("BASE_ADDR_C    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR_C));
        LOG_DBG("OFST_ADDR_C    = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR_C));
        LOG_DBG("STRIDE_C       = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE_C));
        LOG_DBG("PADDING        = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_PADDING));
        LOG_DBG("DITHER         = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER));
        LOG_DBG("DIT_SEED_R     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_R));
        LOG_DBG("DIT_SEED_G     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_G));
        LOG_DBG("DIT_SEED_B     = 0x%08X",ISP_IOCTL_READ_REG(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_B));
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetFormat(
    CDP_DRV_FORMAT_ENUM     Format,
    CDP_DRV_BLOCK_ENUM      Block,
    CDP_DRV_PLANE_ENUM      Plane,
    CDP_DRV_SEQUENCE_ENUM   Sequence)
{
    MBOOL Result = MTRUE;
    //
    LOG_DBG("- E. InFormat: %d. InBlock: %d. InPlane: %d. InSequence: %d.", Format, Block, Plane, Sequence);

    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    // Check if input image format is valid. (Currently Only print out a warning message, won't block execution.)
    Result = InputImgFormatCheck(Format, Plane, Sequence);

    // Get correct Plane value for HW register according to input image format and plane.
    MUINT32 u4OutFormat = 0, u4OutPlane = 0;
    Result = RotDmaEnumRemapping(Format, Plane, &u4OutPlane);

    if (Result)
    {
        //To Do : Need modify on MT6582
        /*
        if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
        {
            ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_FORMAT_1, Format);
            ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_FORMAT_2, Block);
            ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_FORMAT_3, u4OutPlane);
            ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_FORMAT_SEQ, Sequence);
        }
        else    // GDMA mode.
        {
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_FORMAT_1, Format);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_FORMAT_2, Block);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_FORMAT_3, u4OutPlane);
            ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_FORMAT_SEQ, Sequence);
        }
        */
    }
    else
    {
        LOG_ERR("Error Format(%d), Block(%d), Plane(%d), or Sequence(%d).", Format, Block, Plane, Sequence);
    }

    LOG_DBG("- X. Result : %d. OutFormat: %d. OutBlock: %d. OutPlane: %d. OutSequence: %d.", Result, Format, Block, u4OutPlane, Sequence);
    return Result;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_EnableCrop(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_CROP_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_CROP_EN, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_H_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CROP_OFST, DISPO_CROP_OFST_X, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CROP_OFST, DISPO_CROP_OFST_X, Offset);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_V_SetCropOffset(MUINT32 Offset)
{
    LOG_DBG("Offset(%d)",Offset);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Offset > CDP_DRV_MASK_INT_OFFSET)
    {
        LOG_ERR("Offset(%d) is out of %d",Offset,CDP_DRV_MASK_INT_OFFSET);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CROP_OFST, DISPO_CROP_OFST_Y, Offset);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CROP_OFST, DISPO_CROP_OFST_Y, Offset);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetPaddingMode(CDP_DRV_PADDING_MODE_ENUM PaddingMode)
{
    LOG_DBG("PaddingMode(%d)",PaddingMode);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_PADDING_MODE, PaddingMode);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_PADDING_MODE, PaddingMode);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetRotation(CDP_DRV_ROTATION_ENUM Rotation)
{
    LOG_DBG("Rotation(%d)",Rotation);

    if (Rotation != CDP_DRV_ROTATION_0)
    {
        LOG_WRN("DISPO Rotation not 0.");
    }
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_ROTATION, Rotation);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_ROTATION, Rotation);
    }
    */
    return MTRUE;
}


/**************************************************************************
* @brief Calculate and set parameters (i.e. registers) about DISPO Rotation Buffer except Luma/Chroma Buf addresses.
*
* 1. Calculate u4LumaWidth/u4ChromaWidth.
* 2. Decide u4LumaLineNum/u4ChromaLineNum according to eFormat and ePlane.
* 3. Calculate u4LumaBlockWidth/u4ChromaBlockWidth.
* 4. Calculate u4LumaLineSize/u4ChromaLineSize.
* 5. Calculate u4LumaBufSize/u4ChromaBufSize.
*
* @param [IN]     eFormat             Image format of the rotate image.
* @param [IN]     ePlane              Plane of the rotate image.
* @param [IN]     u4TpipeWidth         Used tpipe width.
* @param [IN]     fgFifoMode          Use FIFO Mode or not.
* @param [IN]     fgDoubleBufMode     Use Double Buffer Mode or not.
* @param [OUT]    pu4LumaBufSize      Size of Luma Buffer.
* @param [OUT]    pu4ChromaBufSize    Size of Chroma Buffer.
* @return         OK or Fail. MTRUE: OK. MFALSE: Fail.
**************************************************************************/
MBOOL CdpDrvImp::DISPO_RotationBufConfig(
    CDP_DRV_FORMAT_ENUM eFormat,
    CDP_DRV_PLANE_ENUM ePlane,
    MUINT32 u4TpipeWidth,
    MBOOL   fgFifoMode,
    MBOOL   fgDoubleBufMode,
    MUINT32 *pu4LumaBufSize,
    MUINT32 *pu4ChromaBufSize)
{
    MBOOL ret = MTRUE;  // MTRUE: OK.
    LOG_DBG(" - E. eFormat: %d. ePlane: %d. u4TpipeWidth: %d. fgFifoMode: %d. fgDoubleBufMode: %d.", eFormat, ePlane, u4TpipeWidth, fgFifoMode, fgDoubleBufMode);

    // Intermediate variable (that used to calculate final result).
    MUINT32 u4LumaWidth             = 0;
    MUINT32 u4ChromaWidth           = 0;
    MUINT32 u4ChromaLineNum         = 0;
    MUINT32 u4ChromaBlockWidth      = 0;
    // Final Result (that will be set to regiters.)
    MUINT32 u4LumaLineNum           = 0;
    MUINT32 u4LumaBlockWidth        = 0;
    MUINT32 u4LumaLineSize          = 0;    // MainBufLineSize.
    MUINT32 u4ChromaLineSize        = 0;    // SubBufLineSize.
    MUINT32 u4LumaBufSize           = 0;
    MUINT32 u4ChromaBufSize         = 0;

    // Calculate u4LumaWidth/u4ChromaWidth.
    if (eFormat == CDP_DRV_FORMAT_YUV420)    // For YUV420, Chroma width only half of Luma width.
    {
        u4LumaWidth     = u4TpipeWidth;
        u4ChromaWidth   = u4LumaWidth / 2;
    }
    else    // For other format, Chroma width == Luma width.
    {
        u4LumaWidth     = u4TpipeWidth;
        u4ChromaWidth   = u4LumaWidth;
    }

    // Decide u4LumaLineNum/u4ChromaLineNum according to eFormat and ePlane.
    switch (eFormat)
    {
        case CDP_DRV_FORMAT_YUV422:
        {
            switch (ePlane)
            {
                case CDP_DRV_PLANE_1:
                u4LumaLineNum   = 32;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_2:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_3:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum;
                break;

                default:
                LOG_DBG("Unknown YUV422 plane, u4LumaLineNum/u4ChromaLineNum will use 1-plane setting.");
                u4LumaLineNum   = 32;
                u4ChromaLineNum = u4LumaLineNum / 2;
            }
        }
        break;

        case CDP_DRV_FORMAT_YUV420:
        {
            switch (ePlane)
            {
                case CDP_DRV_PLANE_2:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
                break;

                case CDP_DRV_PLANE_3:
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum;
                break;

                default:
                LOG_DBG("Unknown YUV420 plane, u4LumaLineNum/u4ChromaLineNum will use 2-plane setting.");
                u4LumaLineNum   = 64;
                u4ChromaLineNum = u4LumaLineNum / 2;
            }
       }
        break;

        case CDP_DRV_FORMAT_Y:
        u4LumaLineNum   = 64;
        u4ChromaLineNum = 0;
        break;

        case CDP_DRV_FORMAT_RGB888:
        u4LumaLineNum   = 22;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        case CDP_DRV_FORMAT_RGB565:
        u4LumaLineNum   = 32;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        case CDP_DRV_FORMAT_XRGB8888:
        u4LumaLineNum   = 16;
        u4ChromaLineNum = u4LumaLineNum;
        break;

        default:
        LOG_DBG("Unknown file format, u4LumaLineNum/u4ChromaLineNum will use YUV422 1-plane setting.");
        u4LumaLineNum   = 32;
        u4ChromaLineNum = u4LumaLineNum / 2;

    }

    // Calculate u4LumaBlockWidth/u4ChromaBlockWidth. BlockWidth = roundup(Width/LineNum).
    u4LumaBlockWidth   = (u4LumaWidth   + u4LumaLineNum   - 1) / u4LumaLineNum;
    u4ChromaBlockWidth = (u4ChromaWidth + u4ChromaLineNum - 1) / u4ChromaLineNum;

    // Calculate u4LumaLineSize/u4ChromaLineSize. LineSize = "(LineNum+1) * BlockWidth (FIFO mode)" or "LineNum * BlockWidth (non-FIFO mode)".
    u4LumaLineSize   = (u4LumaLineNum   + fgFifoMode) * u4LumaBlockWidth;
    u4ChromaLineSize = (u4ChromaLineNum + fgFifoMode) * u4ChromaBlockWidth;
    //     Special case for YUV420 3-plane. Chroma LineSize = (LineNum + FifoMode) * BlockWidth + u4ChromaBlockWidth
    if ((eFormat == CDP_DRV_FORMAT_YUV420) && (ePlane == CDP_DRV_PLANE_3) )
    {
        u4ChromaLineSize += u4ChromaBlockWidth;
    }

    // Calculate u4LumaBufSize/u4ChromaBufSize. BufSize = (LineNum + FifoMode) * LineSize.
    u4LumaBufSize   = (u4LumaLineNum   + fgFifoMode) * u4LumaLineSize;
    u4ChromaBufSize = (u4ChromaLineNum + fgFifoMode) * u4ChromaLineSize * 2;
    //     Check if total bufsize exceeds SYSRAM size for safety.
    if ( (u4LumaBufSize + u4ChromaBufSize) > 81920)
    {
        LOG_ERR("Total VIDO Rotation buf size (%d) exceeds SYSRAM size (81920 bytes).", u4LumaBufSize + u4ChromaBufSize);
    }

    #if 1   // For debug.
    LOG_DBG("u4LumaWidth/u4ChromaWidth:           %d, %d.", u4LumaWidth, u4ChromaWidth);
    LOG_DBG("u4LumaLineNum/u4ChromaLineNum:       %d, %d.", u4LumaLineNum, u4ChromaLineNum);
    LOG_DBG("u4LumaBlockWidth/u4ChromaBlockWidth: %d, %d.", u4LumaBlockWidth, u4ChromaBlockWidth);
    LOG_DBG("u4LumaLineSize/u4ChromaLineSize:     %d, %d.", u4LumaLineSize, u4ChromaLineSize);
    LOG_DBG("u4LumaBufSize/u4ChromaBufSize:       %d, %d.", u4LumaBufSize, u4ChromaBufSize);
    #endif  // For debug.

    // Update registers.
    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE,  DISPO_FIFO_MODE          , fgFifoMode);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE,  DISPO_DOUBLE_BUF_MODE    , fgDoubleBufMode);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE,  DISPO_MAIN_BUF_LINE_NUM  , u4LumaLineNum);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_MAIN_BUF_SIZE,  DISPO_MAIN_BLK_WIDTH     , u4LumaBlockWidth);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_SIZE,       DISPO_MAIN_BUF_LINE_SIZE , u4LumaLineSize);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_SIZE,       DISPO_SUB_BUF_LINE_SIZE  , u4ChromaLineSize);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE,  DISPO_FIFO_MODE          , fgFifoMode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE,  DISPO_DOUBLE_BUF_MODE    , fgDoubleBufMode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE,  DISPO_MAIN_BUF_LINE_NUM  , u4LumaLineNum);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_MAIN_BUF_SIZE,  DISPO_MAIN_BLK_WIDTH     , u4LumaBlockWidth);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_SIZE,       DISPO_MAIN_BUF_LINE_SIZE , u4LumaLineSize);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_SIZE,       DISPO_SUB_BUF_LINE_SIZE  , u4ChromaLineSize);
    }
    */

    // Output value.
    *pu4LumaBufSize     = u4LumaBufSize;
    *pu4ChromaBufSize   = u4ChromaBufSize;

    LOG_DBG(" - X. ret: %d. u4LumaBufSize: %d. u4ChromaBufSize: %d.", ret, *pu4LumaBufSize, *pu4ChromaBufSize);

    return ret;

}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetRotationBufAddr(
    MUINT32     LumaAddr0,
    MUINT32     LumaAddr1,
    MUINT32     ChromaAddr0,
    MUINT32     ChromaAddr1)
{
    LOG_DBG(" - E. LumaAddr0: 0x%08X. LumaAddr1: 0x%08X. ChromaAddr0: 0x%08X. ChromaAddr1: 0x%08X.", LumaAddr0, LumaAddr1, ChromaAddr0, ChromaAddr1);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( LumaAddr0 == 0 ||
        LumaAddr1 == 0 ||
        ChromaAddr0 == 0 ||
        ChromaAddr1 == 0
    )
    {
        LOG_ERR("LumaAddr0(0x%08X), LumaAddr1(0x%08X), ChromaAddr0(0x%08X) or ChromaAddr1(0x%08X) is 0.", LumaAddr0, LumaAddr1, ChromaAddr0, ChromaAddr1);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_BASE_ADDR0, DISPO_BUF_BASE_ADDR0, LumaAddr0);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_BASE_ADDR1, DISPO_BUF_BASE_ADDR1, LumaAddr1);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_BASE_ADDR2, DISPO_BUF_BASE_ADDR2, ChromaAddr0);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BUF_BASE_ADDR3, DISPO_BUF_BASE_ADDR3, ChromaAddr1);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR0, DISPO_BUF_BASE_ADDR0, LumaAddr0);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR1, DISPO_BUF_BASE_ADDR1, LumaAddr1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR2, DISPO_BUF_BASE_ADDR2, ChromaAddr0);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BUF_BASE_ADDR3, DISPO_BUF_BASE_ADDR3, ChromaAddr1);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_EnableFlip(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_FLIP, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_FLIP, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_EnableSOFReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_SOF_RESET_EN, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_SOF_RESET_EN, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_H_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_UV_XSEL, UV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_UV_XSEL, UV);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_V_SetUVSelect(CDP_DRV_UV_SELECT_ENUM UV)
{
    LOG_DBG("UV(%d)",UV);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_UV_YSEL, UV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_UV_YSEL, UV);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetDMAPerf(
    MUINT32                 PriLowThr,
    MUINT32                 PriThr,
    CDP_DRV_BURST_LEN_ENUM  BurstLen)
{
    LOG_DBG("PriLowThr(%d),PriThr(%d),BurstLen(%d)",PriLowThr,PriThr,BurstLen);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(PriLowThr > CDP_DRV_MASK_FIFO_PRI_LOW_THR)
    {
        LOG_ERR("PriLowThr(%d) is out of %d",PriLowThr,CDP_DRV_MASK_FIFO_PRI_LOW_THR);
        return MFALSE;
    }
    //
    if(PriThr > CDP_DRV_MASK_FIFO_PRI_THR)
    {
        LOG_ERR("PriThr(%d) is out of %d",PriThr,CDP_DRV_MASK_FIFO_PRI_THR);
        return MFALSE;
    }
    //
    if(BurstLen > CDP_DRV_MASK_MAX_BURST_LEN)
    {
        LOG_ERR("BurstLen(%d) is out of %d",BurstLen,CDP_DRV_MASK_MAX_BURST_LEN);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DMA_PERF, DISPO_FIFO_PRI_LOW_THR , PriLowThr);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DMA_PERF, DISPO_FIFO_PRI_THR     , PriThr);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DMA_PERF, DISPO_MAX_BURST_LEN    , BurstLen);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PERF, DISPO_FIFO_PRI_LOW_THR , PriLowThr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PERF, DISPO_FIFO_PRI_THR     , PriThr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PERF, DISPO_MAX_BURST_LEN    , BurstLen);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_EnableReset(MBOOL En)
{
    LOG_DBG("En(%d)",En);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_SOFT_RST, DISPO_SOFT_RST, En);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_SOFT_RST, DISPO_SOFT_RST, En);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_GetResetStatus()
{
    LOG_DBG("");
    MBOOL Result = MFALSE;
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        Result = (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_DISPO_SOFT_RST_STAT, DISPO_SOFT_RST_STAT));
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_SOFT_RST_STAT, DISPO_SOFT_RST_STAT, Result);
    }
    */
    return Result;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetInterrupt(
    MBOOL   En,
    MBOOL   WriteClear)
{
    LOG_DBG("En(%d),WriteClear(%d)",En,WriteClear);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_INT_EN, DISPO_INT_1_EN, En);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_INT_EN, DISPO_INT_WC_EN, WriteClear);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_INT_EN, DISPO_INT_1_EN, En);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_INT_EN, DISPO_INT_WC_EN, WriteClear);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_GetInterruptStatus()
{
    LOG_DBG("");
    MBOOL Result = MFALSE;
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        Result = (MBOOL)(ISP_READ_BITS(mpIspReg, CAM_DISPO_INT, DISPO_INT_1));
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_READ_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_INT, DISPO_INT_1, Result);
    }
    */
    return Result;
}

/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_H_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_TAR_SIZE, DISPO_XSIZE, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_TAR_SIZE, DISPO_XSIZE, Size);
    }
    */
    return MTRUE;
}

/**************************************************************************
*
**************************************************************************/

MBOOL CdpDrvImp::DISPO_V_SetOutputSize(MUINT32 Size)
{
    LOG_DBG("Size(%d)",Size);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if(Size > CDP_DRV_MASK_IMAGE_SIZE)
    {
        LOG_ERR("Size(%d) is out of %d",Size,CDP_DRV_MASK_IMAGE_SIZE);
        return MFALSE;
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_TAR_SIZE, DISPO_YSIZE, Size);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_TAR_SIZE, DISPO_YSIZE, Size);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetOutputAddr(
    MUINT32     PhyAddr,
    MUINT32     Offset,
    MUINT32     Stride,
    MUINT32     PhyAddrC,
    MUINT32     OffsetC,
    MUINT32     StrideC,
    MUINT32     PhyAddrV,
    MUINT32     OffsetV,
    MUINT32     StrideV)
{
    LOG_DBG("- E. PhyAddr/C/V: 0x%08X/0x%08X/0x%08X", PhyAddr, PhyAddrC, PhyAddrV);
    LOG_DBG("Ofst/C/V: 0x%08X/0x%08X/0x%08X. Stride/C/V: 0x%08X/0x%08X/0x%08X.", Offset, OffsetC, OffsetV, Stride, StrideC, StrideV);

    //
    if (!CheckReady())
    {
        return MFALSE;
    }
    //
    if (
        Offset  > CDP_DRV_MASK_ADDR_OFFSET  ||
        OffsetC > CDP_DRV_MASK_ADDR_OFFSET  ||
        OffsetV > CDP_DRV_MASK_ADDR_OFFSET
    )
    {
        LOG_ERR("Offset(0x%08X), OffsetC(0x%08X) or OffsetV(0x%08X) is out of 0x%08X", Offset, OffsetC, OffsetV, CDP_DRV_MASK_ADDR_OFFSET);
        return MFALSE;
    }
    //
    if (
        Stride  > CDP_DRV_MASK_ADDR_STRIDE  ||
        StrideC > CDP_DRV_MASK_ADDR_STRIDE  ||
        StrideV > CDP_DRV_MASK_ADDR_STRIDE
    )
    {
        LOG_ERR("Stride(0x%08X), StrideC(0x%08X) or StrideV(0x%08X) is out of 0x%08X", Stride, StrideC, StrideV, CDP_DRV_MASK_ADDR_STRIDE);
        return MFALSE;
    }

    LOG_INF("cqPhy(0x%x),Dispo(0x%x),DispCo(0x%x),DispVo(0x%x)",
            mpIspReg, PhyAddr, PhyAddrC, PhyAddrV);     // added for debug


    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BASE_ADDR,  DISPO_BASE_ADDR, PhyAddr);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_OFST_ADDR,  DISPO_OFST_ADDR, Offset);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_STRIDE,     DISPO_STRIDE, Stride);
        //
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BASE_ADDR_C,DISPO_BASE_ADDR_C, PhyAddrC);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_OFST_ADDR_C,DISPO_OFST_ADDR_C, OffsetC);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_STRIDE_C,   DISPO_STRIDE_C, StrideC);
        //
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_BASE_ADDR_V,DISPO_BASE_ADDR_V, PhyAddrV);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_OFST_ADDR_V,DISPO_OFST_ADDR_V, OffsetV);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_STRIDE_V,   DISPO_STRIDE_V, StrideV);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR,  DISPO_BASE_ADDR, PhyAddr);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR,  DISPO_OFST_ADDR, Offset);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE,     DISPO_STRIDE, Stride);
        //
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR_C,DISPO_BASE_ADDR_C, PhyAddrC);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR_C,DISPO_OFST_ADDR_C, OffsetC);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE_C,   DISPO_STRIDE_C, StrideC);
        //
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_BASE_ADDR_V,DISPO_BASE_ADDR_V, PhyAddrV);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_OFST_ADDR_V,DISPO_OFST_ADDR_V, OffsetV);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_STRIDE_V,   DISPO_STRIDE_V, StrideV);
        //
        // for debug
        if(ISP_READ_BITS(mpIspReg,CAM_DISPO_BASE_ADDR,DISPO_BASE_ADDR)!=PhyAddr){
            LOG_ERR("WAddr(0x%x)-RAddr(0x%x)",PhyAddr,ISP_READ_BITS(mpIspReg,CAM_DISPO_BASE_ADDR,DISPO_BASE_ADDR));
        }
    }
    */
    LOG_DBG(" - X.");
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetCamera2DispPadding(
    MUINT32     Y,
    MUINT32     U,
    MUINT32     V)
{
    LOG_DBG("Y(%d),U(%d),V(%d)",Y,U,V);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( Y > CDP_DRV_MASK_PADDING ||
        U > CDP_DRV_MASK_PADDING ||
        V > CDP_DRV_MASK_PADDING)
    {
        LOG_ERR("Y(%d),U(%d) or V(%d ) is out of %d",Y,U,V,CDP_DRV_MASK_PADDING);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_PADDING, DISPO_PADDING_Y, Y);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_PADDING, DISPO_PADDING_U, U);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_PADDING, DISPO_PADDING_V, V);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_PADDING, DISPO_PADDING_Y, Y);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_PADDING, DISPO_PADDING_U, U);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_PADDING, DISPO_PADDING_V, V);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetDithering(
    MBOOL                   En,
    CDP_DRV_DIT_MODE_ENUM   Mode,
    MUINT32                 InitX,
    MUINT32                 InitY,
    MUINT32                 XRGBDummy)
{
    LOG_DBG("En(%d),Mode(%d),InitX(%d),InitY(%d),XRGBDummy(0x%02X)",En,Mode,InitX,InitY,XRGBDummy);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( InitX > CDP_DRV_MASK_DIT_INIT ||
        InitY > CDP_DRV_MASK_DIT_INIT)
    {
        LOG_ERR("InitX(%d) or InitY(%d) is out of %d",InitX,InitY,CDP_DRV_MASK_DIT_INIT);
    }
    //
    if(XRGBDummy > CDP_DRV_MASK_DIT_XRGB_DUMMY)
    {
        LOG_ERR("XRGBDummy(0x%02X) is out of 0x%02X",XRGBDummy,CDP_DRV_MASK_DIT_INIT);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DITHER, DISPO_DIT_EN     , En);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DITHER, DISPO_DIT_MODE   , Mode);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DITHER, DISPO_DIT_INIT_X , InitX);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DITHER, DISPO_DIT_INIT_Y , InitY);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DITHER, DISPO_XRGB_DUMMY , XRGBDummy);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER, DISPO_DIT_EN     , En);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER, DISPO_DIT_MODE   , Mode);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER, DISPO_DIT_INIT_X , InitX);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER, DISPO_DIT_INIT_Y , InitY);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DITHER, DISPO_XRGB_DUMMY , XRGBDummy);
    }
    */
    return MTRUE;
}



/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_SetDitheringSeed(
    MUINT32     R,
    MUINT32     G,
    MUINT32     B)
{
    LOG_DBG("R(0x%08X),G(0x%08X),B(0x%08X)",R,G,B);
    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //
    if( R > CDP_DRV_MASK_DIT_SEED ||
        G > CDP_DRV_MASK_DIT_SEED ||
        B > CDP_DRV_MASK_DIT_SEED)
    {
        LOG_ERR("R(0x%08X),G(0x%08X) or B(0x%08X) is out of %d",R,G,B,CDP_DRV_MASK_DIT_SEED);
    }

    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DIT_SEED_R, DISPO_DIT_SEED_R, R);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DIT_SEED_G, DISPO_DIT_SEED_G, G);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DIT_SEED_B, DISPO_DIT_SEED_B, B);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_R, DISPO_DIT_SEED_R, R);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_G, DISPO_DIT_SEED_G, G);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DIT_SEED_B, DISPO_DIT_SEED_B, B);
    }
    */
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_Config(
    CDP_DRV_IMG_SIZE_STRUCT     ImgSize,
    CDP_DRV_IMG_CROP_STRUCT     Crop,
    CDP_DRV_FORMAT_ENUM         Format,
    CDP_DRV_PLANE_ENUM          Plane,
    CDP_DRV_SEQUENCE_ENUM       Sequence,
    CDP_DRV_ROTATION_ENUM       Rotation,
    MBOOL                       Flip,
    MBOOL                       fgDitherEnable)
{
    MUINT32 BufWidth,BufSize;
    //
    LOG_DBG("ImgSize: (%d, %d). Crop: (%d, %d). CropStart: (%f, %f).", ImgSize.Width, ImgSize.Height, Crop.Width.Size, Crop.Height.Size, Crop.Width.Start, Crop.Height.Start);
    LOG_DBG("Format: %d. Plane: %d. Sequence: %d. Rotation: %d. Flip: %d.", Format, Plane, Sequence, Rotation, Flip);

    //
    if(!CheckReady())
    {
        return MFALSE;
    }
    //Width
    if(Crop.Width.Size == 0)
    {
        DISPO_H_SetCropOffset(0);
        DISPO_H_SetOutputSize(ImgSize.Width);
        Crop.Width.Size = ImgSize.Width;
    }
    else
    if( Crop.Width.Start < 0 ||
        (Crop.Width.Start+Crop.Width.Size) > ImgSize.Width ||
        (Crop.Width.Start-floor(Crop.Width.Start)) > 0)
    {
        LOG_ERR("Error crop,Width(%d),Crop.Width:Start(%f),Size(%d)",ImgSize.Width,Crop.Width.Start,Crop.Width.Size);
        return MFALSE;
    }
    else
    {
        DISPO_H_SetCropOffset(floor(Crop.Width.Start));
        DISPO_H_SetOutputSize(Crop.Width.Size);
    }
    //Height
    if(Crop.Height.Size == 0)
    {
        DISPO_V_SetCropOffset(0);
        DISPO_V_SetOutputSize(ImgSize.Height);
        Crop.Height.Size = ImgSize.Height;
    }
    else
    if( Crop.Height.Start < 0 ||
        (Crop.Height.Start+Crop.Height.Size) > ImgSize.Height ||
        (Crop.Height.Start-floor(Crop.Height.Start)) > 0)
    {
        LOG_ERR("Error crop,Height(%d),Crop.Height:Start(%f),Size(%d)",ImgSize.Height,Crop.Height.Start,Crop.Height.Size);
        return MFALSE;
    }
    else
    {
        DISPO_V_SetCropOffset(floor(Crop.Height.Start));
        DISPO_V_SetOutputSize(Crop.Height.Size);
    }
    //
    if(Format == CDP_DRV_FORMAT_YUV420)
    {
        DISPO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
        DISPO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
    }
    else
    {
        if( Rotation == CDP_DRV_ROTATION_0 ||
            Rotation == CDP_DRV_ROTATION_180)
        {
            DISPO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
            DISPO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
        }
        else
        {
            DISPO_H_SetUVSelect(CDP_DRV_UV_SELECT_EVERY);
            DISPO_V_SetUVSelect(CDP_DRV_UV_SELECT_EVEN);
        }
    }
    //
    DISPO_SetFormat(
        Format,
        CDP_DRV_BLOCK_SCAN_LINE,
        Plane,
        Sequence);
    DISPO_EnableCrop(MTRUE);
    DISPO_SetPaddingMode(CDP_DRV_PADDING_MODE_8);
    DISPO_SetRotation(Rotation);
    DISPO_EnableFlip(Flip);
    DISPO_EnableSOFReset(CDP_DRV_DEFAULT_SOF_RESET);
    DISPO_SetDMAPerf(
        CDP_DRV_DEFAULT_FIFO_PRI_LOW_THR,
        CDP_DRV_DEFAULT_FIFO_PRI_THR,
        CDP_DRV_BURST_LEN_4);
    DISPO_EnableReset(MFALSE);
    DISPO_SetInterrupt(MFALSE,MFALSE);
    DISPO_SetCamera2DispPadding(
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_Y,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_U,
        CDP_DRV_DEFAULT_CAMERA_2_DISP_PADDING_V);
    DISPO_SetDithering(
        fgDitherEnable,
        CDP_DRV_DIT_MODE_16X16,
        CDP_DRV_DEFAULT_DIT_INIT_X,
        CDP_DRV_DEFAULT_DIT_INIT_Y,
        CDP_DRV_DEFAULT_DIT_XRGB_DUMMY);
    DISPO_SetDitheringSeed(
        CDP_DRV_DEFAULT_DIT_SEED_R,
        CDP_DRV_DEFAULT_DIT_SEED_G,
        CDP_DRV_DEFAULT_DIT_SEED_B);

    #if (DISPO_HAS_ROTATE)
    // Recalculate SYSRAM for rotation.
    if( Rotation == CDP_DRV_ROTATION_90 ||
        Rotation == CDP_DRV_ROTATION_270)
    {
        MUINT32 u4LumaBufSize   = 0;
        MUINT32 u4ChromaBufSize = 0;

        DISPO_RotationBufConfig(
            Format,
            Plane,
            u4TpipeWidth,
            MTRUE,  // fgFifoMode
            MFALSE, // fgDoubleBufMode
            &u4LumaBufSize,
            &u4ChromaBufSize
        );

        //
        if (!RecalculateRotationBufAddr(CDP_DRV_ROTDMA_DISPO, u4LumaBufSize, u4ChromaBufSize))
        {
            LOG_ERR("Recalculate SYSRAM Buf Addr fail");
            return MFALSE;
        }
        //
        DISPO_SetRotationBufAddr(
            mRotationBuf[CDP_DRV_ROTDMA_DISPO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_DISPO][CDP_DRV_LC_LUMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_DISPO][CDP_DRV_LC_CHROMA].PhyAddr,
            mRotationBuf[CDP_DRV_ROTDMA_DISPO][CDP_DRV_LC_CHROMA].PhyAddr);
    }
    else    // When no rotate or 180 degree rotate, still recalculate SYSRAM Buf Addr. Not really necessary.
    {
        if (!RecalculateRotationBufAddr(CDP_DRV_ROTDMA_DISPO, CDP_DRV_BUF_SYSRAM_BANK0_SIZE-1, CDP_DRV_BUF_SYSRAM_BANK1_SIZE-1))    // CDP_DRV_BUF_SYSRAM_BANK[0,1]_SIZE-1: Use a special size to indicate that it's special case for non-rotate/180 degree rotate case.
        {
            LOG_ERR("Recalculate SYSRAM Buf Addr fail.");
            return MFALSE;
        }
    }

    #endif  // DISPO_HAS_ROTATE

    // Enable DISPO Pre-ultra.
    //To Do : Need modify on MT6582
    /*
    if (!m_fgIsGdmaMode)    // Not GDMA mode (i.e. Normal mode).
    {
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_CTRL, DISPO_PREULTRA_EN, 0x1);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DMA_PREULTRA, DISPO_FIFO_PREULTRA_THR, 0x10);
        ISP_WRITE_BITS(mpIspReg, CAM_DISPO_DMA_PREULTRA, DISPO_FIFO_PREULTRA_LOW_THR, 0x10);
    }
    else    // GDMA mode.
    {
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_CTRL, DISPO_PREULTRA_EN, 0x1);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PREULTRA, DISPO_FIFO_PREULTRA_THR, 0x10);
        ISP_IOCTL_WRITE_BITS(m_pPhyIspDrv, m_pPhyIspDrv->getRegAddrMap(), CAM_DISPO_DMA_PREULTRA, DISPO_FIFO_PREULTRA_LOW_THR, 0x10);
    }
    */
    DISPO_Enable(MTRUE);
    //
    return MTRUE;
}


/**************************************************************************
*
**************************************************************************/
MBOOL CdpDrvImp::DISPO_Unconfig()
{
    LOG_DBG("");
    //
    if(!CheckReady())
    {
        return MFALSE;
    }

    #if (DISPO_HAS_ROTATE)
//    FreeRotationBuf(CDP_DRV_ROTDMA_DISPO);    //Vent@20121201: SYSRAM Buf Free has been moved to CDP un-init.
    #endif  // DISPO_HAS_ROTATE

    DISPO_Enable(MFALSE);
    //
    return MTRUE;
}


