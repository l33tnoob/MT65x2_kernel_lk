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
#define LOG_TAG "TdriMgr"

#include <cutils/xlog.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
//#include <cutils/pmem.h>

#include "mtkcam/common.h"
#include "tdri_mgr_imp.h"
#include <semaphore.h>

#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*() CameraProfile APIS.
using namespace CPTool;
/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define TDRI_MGR_DUMMY_CQ                   ((TPIPE_DRV_CQ_ENUM)0xFFFF)
#define TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY     (0x00)
//

class IspDbgTimer
{
protected:
    char const*const    mpszName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;

public:
    IspDbgTimer(char const*const pszTitle)
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
//        if  (0==mIdx)
//        {
//            ISP_FUNC_INF("[%s] %s:(%d-th) ===> %.06f ms", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4StartUs)/1000);
//        }
//        else
//        {
            LOG_INF("[%s] %s:(%d-th) ===> %.06f ms (Total time till now: %.06f ms)", mpszName, pszInfo, mIdx++, (float)(i4EndUs-mi4LastUs)/1000, (float)(i4EndUs-mi4StartUs)/1000);
//        }
        mi4LastUs = i4EndUs;

	    //sleep(4); //wait 1 sec for AE stable

        return  MTRUE;
    }
};
#ifndef USING_MTK_LDVT   // Not using LDVT.
    #if 1   // Use CameraProfile API
        static unsigned int G_emGlobalEventId = 0; // Used between different functions.
        static unsigned int G_emLocalEventId = 0;  // Used within each function.
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);       CPTLog(EVENT_ID, CPTFlagStart); G_emGlobalEventId = EVENT_ID;
        #define GLOBAL_PROFILING_LOG_END();                 CPTLog(G_emGlobalEventId, CPTFlagEnd);
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);     CPTLogStr(G_emGlobalEventId, CPTFlagSeparator, LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   AutoCPTLog CPTlogLocalVariable(EVENT_ID); G_emLocalEventId = EVENT_ID;
        #define LOCAL_PROFILING_LOG_PRINT(LOG_STRING);      CPTLogStr(G_emLocalEventId, CPTFlagSeparator, LOG_STRING);
    #elif 0   // Use debug print
        #define GLOBAL_PROFILING_LOG_START(EVENT_ID);
        #define GLOBAL_PROFILING_LOG_END();
        #define GLOBAL_PROFILING_LOG_PRINT(LOG_STRING);
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
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
        #define LOCAL_PROFILING_LOG_AUTO_START(EVENT_ID);   IspDbgTimer DbgTmr(#EVENT_ID);
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
 TDRI_MGR_CQ_MAPPING tdriMgrCqMap[ISP_DRV_CQ_NUM] = {
 {ISP_DRV_CQ03,        TDRI_MGR_DUMMY_CQ},
 {ISP_DRV_CQ0,         TDRI_MGR_DUMMY_CQ},
 {ISP_DRV_CQ0B,        TDRI_MGR_DUMMY_CQ},
 {ISP_DRV_CQ0C,        TDRI_MGR_DUMMY_CQ},
 {ISP_DRV_CQ01,        TPIPE_DRV_CQ01},
 {ISP_DRV_CQ01_SYNC,   TPIPE_DRV_CQ01},
 {ISP_DRV_CQ02,        TPIPE_DRV_CQ02},
 {ISP_DRV_CQ02_SYNC,   TPIPE_DRV_CQ02},
// {ISP_DRV_CQ03,        TDRI_MGR_DUMMY_CQ},
};

 TDRI_MGR_FUNC_MAPPING tdriMgrFuncMap[TDRI_MGR_FUNC_NUM] = {
 {TDRI_MGR_FUNC_BNR,        CAM_ISP_BNR,          CAM_DUMMY_,       CAM_DUMMY_,   TPIPE_DRV_UPDATE_BNR,            0x00000080, 0x00000000, 0x00000100},
 {TDRI_MGR_FUNC_LSC,         CAM_ISP_LSC,         CAM_DMA_LSCI,        CAM_DUMMY_,   TPIPE_DRV_UPDATE_LSC,            0x00000020, 0x00000000, 0x00000002},
 //{TDRI_MGR_FUNC_MFB,         CAM_ISP_MFB,         CAM_DUMMY_,          CAM_DUMMY_,   TPIPE_DRV_UPDATE_MFB,            0x00000000, 0x00000000, 0x00000000},  // mfb top control by ImageIO
 {TDRI_MGR_FUNC_CFA,         CAM_ISP_CFA,         CAM_DUMMY_,          CAM_DUMMY_,   TPIPE_DRV_UPDATE_CFA,            0x00200000, 0x00000000, 0x00000000},  // cfa top control by ImageIO
 {TDRI_MGR_FUNC_NBC,         CAM_ISP_NBC,         CAM_DUMMY_,          CAM_DUMMY_,   TPIPE_DRV_UPDATE_NBC,            0x00000000, 0x00000004, 0x00000000},
 {TDRI_MGR_FUNC_SEEE,        CAM_ISP_SEEE,        CAM_DUMMY_,          CAM_DUMMY_,   TPIPE_DRV_UPDATE_SEEE,           0x00000000, 0x00000010, 0x00000000},
 //{TDRI_MGR_FUNC_LCE_BASIC,   CAM_ISP_LCE_BASIC_1, CAM_ISP_LCE_BASIC_2, CAM_DMA_LCEI, TPIPE_DRV_UPDATE_LCE,            0x02000000, 0x00000000, 0x00001000},
 {TDRI_MGR_FUNC_SL2,         CAM_CDP_SL2_FEATUREIO_1, CAM_CDP_SL2_IMAGEIO, CAM_CDP_SL2_FEATUREIO_2,   TPIPE_DRV_UPDATE_SL2,  0x00000100, 0x00000000, 0x00000000},
 //{TDRI_MGR_FUNC_NR3D_TOP,    CAM_TOP_CTL_02,      CAM_DUMMY_,          CAM_DUMMY_,   TPIPE_DRV_UPDATE_NR3D,           0x00000000, 0x00000020, 0x00000000},
 //{TDRI_MGR_FUNC_NR3D,        CAM_CDP_3DNR,        CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000000, 0x00000000, 0x00000000},  // no update top in this index
 //{TDRI_MGR_FUNC_LCE_CUSTOM,  CAM_ISP_LCE_CUSTOM,  CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000000, 0x00000000, 0x00000000},  // no update top in this index
 {TDRI_MGR_FUNC_OBC,         CAM_ISP_OBC,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000008, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_PGN,         CAM_ISP_PGN,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000800, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_CCL,         CAM_ISP_CCL,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00400000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_G2G,         CAM_ISP_G2G,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00800000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_G2C,         CAM_ISP_G2C,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000000, 0x00000001, 0x00000000},
 //{TDRI_MGR_FUNC_DGM,         CAM_ISP_DGM,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x01000000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_GGMRB,       CAM_ISP_GGMRB,       CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x04000000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_GGMG,        CAM_ISP_GGMG,        CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x04000000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_GGM_CTL,     CAM_ISP_GGM_CTL,     CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x04000000, 0x00000000, 0x00000000},
 {TDRI_MGR_FUNC_PCA,         CAM_ISP_PCA,         CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000000, 0x00000008, 0x00000000},
 {TDRI_MGR_FUNC_PCA_CON,     CAM_ISP_PCA_CON,     CAM_DUMMY_,          CAM_DUMMY_,   TDRI_MGR_TPIPE_DRV_UPDATE_DUMMY, 0x00000000, 0x00000008, 0x00000000}};


/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
//-----------------------------------------------------------------------------
TdriMgrImp::TdriMgrImp()
                : pTdriDri(NULL)
                , pIspDrv(NULL)
                , mLock()
                , mInitCount(0)
{
    int i;
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_START(Event_TdriMgr);	// Profiling Start.
    mInitCount = 0;
    //
    for(i=TPIPE_DRV_CQ01;i<=TPIPE_DRV_CQ02;i++) {
        tdriMgrInfo[i].pDescriptorArray = NULL;
        tdriMgrInfo[i].pDesriptorNum = 0;
        tdriMgrInfo[i].pTopCtlEn1 = 0;
        tdriMgrInfo[i].pTopCtlEn2 = 0;
        tdriMgrInfo[i].pTopCtlDma = 0;
    }

}

//-----------------------------------------------------------------------------
TdriMgrImp::~TdriMgrImp()
{
    LOG_DBG("");
    GLOBAL_PROFILING_LOG_END(); 	// Profiling End.
}

//-----------------------------------------------------------------------------
TdriMgr& TdriMgr::getInstance(void)
{
    static  TdriMgrImp Singleton;
    return  Singleton;
}

//-----------------------------------------------------------------------------
MBOOL TdriMgrImp::init(void)
{
    MBOOL Result = MTRUE;
    //
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("init TdriMgr");
    Mutex::Autolock lock(mLock); // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.

    LOG_INF("mInitCount(%d)",mInitCount);
    //
    if(mInitCount == 0) {
        int i;
        LOCAL_PROFILING_LOG_PRINT("run tdri mgr init");
        // create tpipe driver instance
        pTdriDri = TpipeDrv::createInstance();
        if (!pTdriDri) {
            LOG_ERR("TpipeDrv::createInstance() fail \n");
            Result = MFALSE;
            goto EXIT;
        }
        if( pTdriDri->init() == 0) {
            LOG_ERR("TpipeDrv::init() fail \n");
            Result = MFALSE;
            goto EXIT;
        }
        // create isp driver instance
        pIspDrv = IspDrv::createInstance();
        if (!pIspDrv) {
            LOG_ERR("IspDrv::createInstance() fail \n");
            Result = MFALSE;
            goto EXIT;
        }

        for(i=TPIPE_DRV_CQ01;i<=TPIPE_DRV_CQ02;i++) {
            MUINT32 descArray, descNum, ctlEn1, ctlEn2, ctlDma;

            if(i == TPIPE_DRV_CQ01) {
                pIspDrv->lockSemaphoreCq1();
                //
                tdriMgrInfo[i].pVirIspDrv = pIspDrv->getCQInstance(ISP_DRV_CQ01_SYNC);
                tdriMgrInfo[i].pVirtIspReg = (isp_reg_t*)tdriMgrInfo[i].pVirIspDrv->getRegAddr();
                tdriMgrInfo[i].tdriMgrCfg.updateTdri.updateType = TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE;
                pIspDrv->getTpipeMgrVaCq1(&descArray, &descNum, &ctlEn1, &ctlEn2, &ctlDma);
                tdriMgrInfo[i].pDescriptorArray = (MUINT32*)descArray;
                tdriMgrInfo[i].pDesriptorNum = (MUINT32*)descNum;
                tdriMgrInfo[i].pTopCtlEn1 = (MUINT32*)ctlEn1;
                tdriMgrInfo[i].pTopCtlEn2 = (MUINT32*)ctlEn2;
                tdriMgrInfo[i].pTopCtlDma = (MUINT32*)ctlDma;
                //
                pIspDrv->unlockSemaphoreCq1();
            } else if(i == TPIPE_DRV_CQ02) {
                pIspDrv->lockSemaphoreCq2();
                //
                tdriMgrInfo[i].pVirIspDrv = pIspDrv->getCQInstance(ISP_DRV_CQ02_SYNC);
                tdriMgrInfo[i].pVirtIspReg = (isp_reg_t*)tdriMgrInfo[i].pVirIspDrv->getRegAddr();
                tdriMgrInfo[i].tdriMgrCfg.updateTdri.updateType = TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE;
                pIspDrv->getTpipeMgrVaCq2(&descArray, &descNum, &ctlEn1, &ctlEn2, &ctlDma);
                tdriMgrInfo[i].pDescriptorArray = (MUINT32*)descArray;
                tdriMgrInfo[i].pDesriptorNum = (MUINT32*)descNum;
                tdriMgrInfo[i].pTopCtlEn1 = (MUINT32*)ctlEn1;
                tdriMgrInfo[i].pTopCtlEn2 = (MUINT32*)ctlEn2;
                tdriMgrInfo[i].pTopCtlDma = (MUINT32*)ctlDma;
                //
                pIspDrv->unlockSemaphoreCq2();
            } else{
                LOG_ERR("[ERROR]not support this tdri cq(%d) number \n");
                goto EXIT;
            }

            LOG_DBG("i(%d) pVirIspDrv(0x%x) pVirtIspReg(0x%x) pDescriptorArray(0x%x) pDesriptorNum(0x%x) pTopCtlEn1(0x%x) pTopCtlEn2(0x%x) pTopCtlDma(0x%x)\n",
                    i,tdriMgrInfo[i].pVirIspDrv,tdriMgrInfo[i].pVirtIspReg,tdriMgrInfo[i].pDescriptorArray,tdriMgrInfo[i].pDesriptorNum,
                    tdriMgrInfo[i].pDesriptorNum,tdriMgrInfo[i].pTopCtlEn2,tdriMgrInfo[i].pTopCtlDma);

            // for run tpipe table
            tdriMgrInfo[i].tdriMgrCfg.updateTdri.partUpdateFlag = 0;
            //

            LOG_DBG("cq(%d) pVirIspDrv(0x%x) pVirtIspReg(0x%x) ",i,tdriMgrInfo[i].pVirIspDrv,tdriMgrInfo[i].pVirtIspReg);
        }
        //
        if( pIspDrv->init() == 0) {
            LOG_ERR("IspDrv::init() fail \n");
            Result = MFALSE;
            goto EXIT;
        }
        //
        pIspDrv->setCallbacks(mTdriMgrCB, this);
    }
    //
    android_atomic_inc(&mInitCount);
    //
    EXIT:
    LOG_INF("X:\n");
    //
    return Result;
}



//-----------------------------------------------------------------------------
MBOOL TdriMgrImp::uninit(void)
{
    MBOOL Result = MTRUE;
    int i;
    //
    GLOBAL_PROFILING_LOG_PRINT(__func__);
    GLOBAL_PROFILING_LOG_PRINT("Uninit TdriMgr");
    Mutex::Autolock lock(mLock);
    //
    LOG_INF("mInitCount(%d)",mInitCount);
    //
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0) {
        goto EXIT;
    }


    for(i=TPIPE_DRV_CQ01;i<=TPIPE_DRV_CQ02;i++) {
        if(i == TPIPE_DRV_CQ01) {
            pIspDrv->lockSemaphoreCq1();
            tdriMgrInfo[i].pDescriptorArray = NULL;
            tdriMgrInfo[i].pDesriptorNum = 0;
            tdriMgrInfo[i].pTopCtlEn1 = 0;
            tdriMgrInfo[i].pTopCtlEn2 = 0;
            tdriMgrInfo[i].pTopCtlDma = 0;
            pIspDrv->unlockSemaphoreCq1();
        } else if (i == TPIPE_DRV_CQ02) {
            pIspDrv->lockSemaphoreCq2();
            tdriMgrInfo[i].pDescriptorArray = NULL;
            tdriMgrInfo[i].pDesriptorNum = 0;
            tdriMgrInfo[i].pTopCtlEn1 = 0;
            tdriMgrInfo[i].pTopCtlEn2 = 0;
            tdriMgrInfo[i].pTopCtlDma = 0;
            pIspDrv->unlockSemaphoreCq2();
        } else{
            LOG_ERR("[ERROR]not support this tdri cq(%d) number \n");
        }
    }


    //tpipe driver
    pTdriDri->uninit();
    pTdriDri->destroyInstance();
    pTdriDri = NULL;

    //isp drv
    pIspDrv->setCallbacks(NULL, NULL);
    pIspDrv->uninit();
    pIspDrv->destroyInstance();
    pIspDrv = NULL;

    LOG_INF("Release\n");

EXIT:
    return Result;
}

//-----------------------------------------------------------------------------
MBOOL TdriMgrImp::mTdriMgrCB(
    MINT32 ispCqNum,
    MVOID *user)
{
    TdriMgrImp *pTdriMgrObj = (TdriMgrImp*)user;
    LOG_DBG("ispCqNum(%d) user(0x%x)\n",ispCqNum,user);

    pTdriMgrObj->handleTpipeTable(tdriMgrCqMap[ispCqNum].tdriMgrCQ);

    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::handleTpipeTable(TPIPE_DRV_CQ_ENUM tmgrCq)
{
    LOG_DBG("Cq(%d),UpdateFlag(0x%x)\n",tmgrCq,tdriMgrInfo[tmgrCq].tdriMgrCfg.updateTdri.partUpdateFlag);

    pTdriDri->configTdriPara(&tdriMgrInfo[tmgrCq].tdriMgrCfg);

    tdriMgrInfo[tmgrCq].tdriMgrCfg.updateTdri.partUpdateFlag = 0; // clean Tpipe update flag

    return MTRUE;
}


//-----------------------------------------------------------------------------
MBOOL TdriMgrImp::refreshTableSetting(TPIPE_DRV_CQ_ENUM tmgrCq, MINT32 tableUpdateFlag)
{
    LOG_DBG("tableUpdateFlag=0x%x\n",tableUpdateFlag);

    if(tableUpdateFlag & TPIPE_DRV_UPDATE_BNR) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.bnr_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.bnr_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.bnr, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.bnr, sizeof(TdriBnrCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.bnr_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.bnr, 0, sizeof(TdriBnrCfg));
       // tdriMgrInfo[tmgrCq].tdriMgrCfg.top.imgci_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.imgci_en;
       // tdriMgrInfo[tmgrCq].tdriMgrCfg.imgci_stride = tdriMgrInfo[tmgrCq].tdriTurningSetting.imgci_stride;
    }
    if(tableUpdateFlag & TPIPE_DRV_UPDATE_LSC) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.lsc_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.lsc_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.lsc, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.lsc, sizeof(TdriLscCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.lsc_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.lsc, 0, sizeof(TdriLscCfg));
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.lsci_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.lsci_en;
        tdriMgrInfo[tmgrCq].tdriMgrCfg.lsci_stride = tdriMgrInfo[tmgrCq].tdriTurningSetting.lsci_stride;
    }
 /*   if(tableUpdateFlag & TPIPE_DRV_UPDATE_MFB) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.mfb_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.mfb_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.mfb, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.mfb, sizeof(TdriMfbCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.mfb_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.mfb, 0, sizeof(TdriMfbCfg));
    }*/
    if(tableUpdateFlag & TPIPE_DRV_UPDATE_CFA){
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.cfa_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.cfa_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.cfa, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.cfa, sizeof(TdriCfaCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.cfa_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.cfa, 0, sizeof(TdriCfaCfg));
    }
    if(tableUpdateFlag & TPIPE_DRV_UPDATE_NBC) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.nbc_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.nbc_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.nbc, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.nbc, sizeof(TdriNbcCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.nbc_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.nbc, 0, sizeof(TdriNbcCfg));
    }
    if(tableUpdateFlag & TPIPE_DRV_UPDATE_SEEE) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.seee_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.seee_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.seee, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.seee, sizeof(TdriSeeeCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.seee_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.seee, 0, sizeof(TdriSeeeCfg));
    }
	if(tableUpdateFlag & TPIPE_DRV_UPDATE_SL2) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.sl2_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.sl2_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.sl2, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.sl2, sizeof(Tdrisl2Cfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.sl2_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.sl2, 0, sizeof(Tdrisl2Cfg));
    }
  /*  if(tableUpdateFlag & TPIPE_DRV_UPDATE_LCE) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.lce_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.lce_en;
        ::memcpy((char*)&tdriMgrInfo[tmgrCq].tdriMgrCfg.tuningFunc.lce, (char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.lce, sizeof(TdriLceCfg));
        tdriMgrInfo[tmgrCq].tdriTurningSetting.lce_en = 0;
        ::memset((char*)&tdriMgrInfo[tmgrCq].tdriTurningSetting.lce, 0, sizeof(TdriLceCfg));
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.lcei_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.lcei_en;
        tdriMgrInfo[tmgrCq].tdriMgrCfg.lcei_stride = tdriMgrInfo[tmgrCq].tdriTurningSetting.lcei_stride;
    }

    if(tableUpdateFlag & TPIPE_DRV_UPDATE_NR3D) {
        tdriMgrInfo[tmgrCq].tdriMgrCfg.top.nr3d_en = tdriMgrInfo[tmgrCq].tdriTurningSetting.nr3d_en;
        tdriMgrInfo[tmgrCq].tdriTurningSetting.nr3d_en = 0;
    }*/

    return MTRUE;
}

MBOOL
TdriMgrImp::flushSetting(ISP_DRV_CQ_ENUM ispCq)
{
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("flushSetting");
    LOG_INF("E:Cq(%d)",ispCq);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        // flush setting
        ::memset((char*)&tdriMgrInfo[TPIPE_DRV_CQ01].tdriTurningSetting, 0x00, sizeof(TDRI_MGR_TPIPE_TABLE_TURNING));
        ::memset((char*)&tdriMgrInfo[TPIPE_DRV_CQ01].tdriMgrCfg, 0x00, sizeof(TdriDrvCfg));
        tdriMgrInfo[TPIPE_DRV_CQ01].tdriMgrCfg.updateTdri.updateType = TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE;
        refreshTableSetting(TPIPE_DRV_CQ01, 0xffffffff);  // refresh all setting
        tdriMgrInfo[TPIPE_DRV_CQ01].tdriMgrCfg.updateTdri.partUpdateFlag = 0xffffffff; // clean Tpipe update flag
        handleTpipeTable(TPIPE_DRV_CQ01);

        pIspDrv->setTurnTopEn1(ISP_DRV_CQ01, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->setTurnTopEn2(ISP_DRV_CQ01, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->setTurnTopDma(ISP_DRV_CQ01, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->flushTurnCqTable(ISP_DRV_CQ01);
        //
        pIspDrv->unlockSemaphoreCq1();
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();

        // flush setting
        ::memset((char*)&tdriMgrInfo[TPIPE_DRV_CQ02].tdriTurningSetting, 0x00, sizeof(TDRI_MGR_TPIPE_TABLE_TURNING));
        ::memset((char*)&tdriMgrInfo[TPIPE_DRV_CQ02].tdriMgrCfg, 0x00, sizeof(TdriDrvCfg));
        tdriMgrInfo[TPIPE_DRV_CQ02].tdriMgrCfg.updateTdri.updateType = TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE;
        refreshTableSetting(TPIPE_DRV_CQ02, 0xffffffff);  // refresh all setting
        tdriMgrInfo[TPIPE_DRV_CQ02].tdriMgrCfg.updateTdri.partUpdateFlag = 0xffffffff; // clean Tpipe update flag  //check 89 setting
        handleTpipeTable(TPIPE_DRV_CQ02);
        pIspDrv->setTurnTopEn1(ISP_DRV_CQ02, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->setTurnTopEn2(ISP_DRV_CQ02, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->setTurnTopDma(ISP_DRV_CQ02, ISP_DRV_TURNING_TOP_RESET);
        pIspDrv->flushTurnCqTable(ISP_DRV_CQ02);
        //
        pIspDrv->unlockSemaphoreCq2();
    }
    else {
        LOG_WRN("not support this tMgrCq(%d) for flush",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }

    LOG_INF("X\n",ispCq);
    LOCAL_PROFILING_LOG_PRINT("End flushSetting");

    return MTRUE;

}

//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::applySetting(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM tmgFunc)
{
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        /* for updating tMgrCq virtual memory and descriptor */
        if((tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp1] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp1!=CAM_DUMMY_)){
            tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp1] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ01].pDesriptorNum)++;
        }

        if((tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp2] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp2!=CAM_DUMMY_)){

            tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp2] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ01].pDesriptorNum)++;
        }

        if((tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp3] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp3!=CAM_DUMMY_)){

            tdriMgrInfo[TPIPE_DRV_CQ01].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp3] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ01].pDesriptorNum)++;
        }


        (*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlEn1) |= tdriMgrFuncMap[tmgFunc].topCtlEn1;
        (*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlEn2) |= tdriMgrFuncMap[tmgFunc].topCtlEn2;
        (*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlDma) |= tdriMgrFuncMap[tmgFunc].topCtlDma;

        /* update tpipe turning flag for tpipe table */
        tdriMgrInfo[TPIPE_DRV_CQ01].tdriMgrCfg.updateTdri.partUpdateFlag |= tdriMgrFuncMap[tmgFunc].tpipeDrvFunc;

        /* check and get tdri table */
        if(tdriMgrFuncMap[tmgFunc].tpipeDrvFunc) {
            refreshTableSetting(TPIPE_DRV_CQ01, tdriMgrFuncMap[tmgFunc].tpipeDrvFunc);
        }

        LOG_DBG("cq1 pDesriptorNum(%d) pTopCtlEn1(0x%x) pTopCtlEn2(0x%x) pTopCtlDma(0x%x) partUpdateFlag(0x%x)\n",(*tdriMgrInfo[TPIPE_DRV_CQ01].pDesriptorNum),
                (*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlEn1),(*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlEn2),(*tdriMgrInfo[TPIPE_DRV_CQ01].pTopCtlDma),
                tdriMgrInfo[TPIPE_DRV_CQ01].tdriMgrCfg.updateTdri.partUpdateFlag);

        pIspDrv->unlockSemaphoreCq1();
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        /* for updating information for CQ setting */
        if((tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp1] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp1!=CAM_DUMMY_)){
            tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp1] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ02].pDesriptorNum)++;
        }

        if((tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp2] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp2!=CAM_DUMMY_)){

            tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp2] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ02].pDesriptorNum)++;
        }

        if((tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp3] == MFALSE)&&
            (tdriMgrFuncMap[tmgFunc].cqFuncGrp3!=CAM_DUMMY_)){

            tdriMgrInfo[TPIPE_DRV_CQ02].pDescriptorArray[tdriMgrFuncMap[tmgFunc].cqFuncGrp3] = MTRUE;
            (*tdriMgrInfo[TPIPE_DRV_CQ02].pDesriptorNum)++;
        }


        (*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlEn1) |= tdriMgrFuncMap[tmgFunc].topCtlEn1;
        (*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlEn2) |= tdriMgrFuncMap[tmgFunc].topCtlEn2;
        (*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlDma) |= tdriMgrFuncMap[tmgFunc].topCtlDma;

        /* update tpipe turning flag for tpipe table */
        tdriMgrInfo[TPIPE_DRV_CQ02].tdriMgrCfg.updateTdri.partUpdateFlag |= tdriMgrFuncMap[tmgFunc].tpipeDrvFunc;

        /* check and get tdri table */
        if(tdriMgrFuncMap[tmgFunc].tpipeDrvFunc) {
            refreshTableSetting(TPIPE_DRV_CQ02, tdriMgrFuncMap[tmgFunc].tpipeDrvFunc);
        }

        LOG_DBG("cq2 pDesriptorNum(%d) pTopCtlEn1(0x%x) pTopCtlEn2(0x%x) pTopCtlDma(0x%x) partUpdateFlag(0x%x)\n",(*tdriMgrInfo[TPIPE_DRV_CQ02].pDesriptorNum),
                (*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlEn1),(*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlEn2),(*tdriMgrInfo[TPIPE_DRV_CQ02].pTopCtlDma),
                tdriMgrInfo[TPIPE_DRV_CQ02].tdriMgrCfg.updateTdri.partUpdateFlag);

        pIspDrv->unlockSemaphoreCq2();
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for hold",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End applySetting");

    return MTRUE;
}


//-----------------------------------------------------------------------------
// bpc_tbl_size : imgci x size
MBOOL  TdriMgrImp::setBnr(ISP_DRV_CQ_ENUM ispCq, MBOOL bnrEn, int bpcEn)//, int bpc_tbl_en, int bpc_tbl_size, int imgciEn)//, int imgciStride)
{
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setBnr");
    LOG_DBG("ispCq(%d) tMgrCq(%d)",ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ);
    //LOG_DBG("bnrEn(%d) bpcEn(%d) bpc_tbl_en(%d) bpc_tbl_size(%d)",bnrEn, bpcEn, bpc_tbl_en, bpc_tbl_size);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr_en = (MINT32)bnrEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_en = bpcEn;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_tbl_en = bpc_tbl_en;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_tbl_size = bpc_tbl_size;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.imgci_en = imgciEn;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.imgci_stride = imgciStride;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr_en = (MINT32)bnrEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_en = bpcEn;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_tbl_en = bpc_tbl_en;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.bnr.bpc_tbl_size = bpc_tbl_size;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.imgci_en = imgciEn;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.imgci_stride = imgciStride;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setBnr",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setBnr");

    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::setLsc(ISP_DRV_CQ_ENUM ispCq, MBOOL lscEn, int sdblk_width, int sdblk_xnum, int sdblk_last_width,
                            int sdblk_height, int sdblk_ynum, int sdblk_last_height, int lsciEn, int lsciStride)
{
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setLsc");
    LOG_DBG("ispCq(%d),tMgrCq(%d),lscEn(%d),lsciEn(%d)",ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ,lscEn,lsciEn);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc_en = (MINT32)lscEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_width = sdblk_width;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_xnum = sdblk_xnum;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_last_width = sdblk_last_width;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_height = sdblk_height;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_ynum = sdblk_ynum;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_last_height = sdblk_last_height;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsci_en = lsciEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsci_stride = lsciStride;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc_en = (MINT32)lscEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_width = sdblk_width;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_xnum = sdblk_xnum;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_last_width = sdblk_last_width;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_height = sdblk_height;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_ynum = sdblk_ynum;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsc.sdblk_last_height = sdblk_last_height;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsci_en = lsciEn;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.lsci_stride = lsciStride;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setLsc",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setLsc");

    return MTRUE;
}
//-----------------------------------------------------------------------------
/*MBOOL  TdriMgrImp::setLce(ISP_DRV_CQ_ENUM ispCq, MBOOL lceEn, int lce_bc_mag_kubnx, int lce_offset_x,
                            int lce_bias_x, int lce_slm_width, int lce_bc_mag_kubny,
                            int lce_offset_y, int lce_bias_y, int lce_slm_height, int lceiEn, int lceiStride)
{
    Mutex::Autolock lock(mLock);
    //
    TPIPE_DRV_CQ_ENUM tMgrCq = tdriMgrCqMap[ispCq].tdriMgrCQ;
    //
    LOG_DBG("ispCq(%d) tMgrCq(%d)",ispCq,tMgrCq);
    //
    if (tMgrCq == TDRI_MGR_DUMMY_CQ) {
        LOG_ERR("isp cq(%d) number error\n",ispCq);
        return MFALSE;
    }
    //
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce_en = (MINT32)lceEn;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_bc_mag_kubnx = lce_bc_mag_kubnx;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_offset_x = lce_offset_x;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_bias_x = lce_bias_x;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_slm_width = lce_slm_width;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_bc_mag_kubny = lce_bc_mag_kubny;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_offset_y = lce_offset_y;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_bias_y = lce_bias_y;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lce.lce_slm_height = lce_slm_height;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lcei_en = lceiEn;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.lcei_stride = lceiStride;
    //
    return MTRUE;
}*/
//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::setNbc(ISP_DRV_CQ_ENUM ispCq, MBOOL en, int anr_eny,
    int anr_enc, int anr_iir_mode, int anr_scale_mode)
{
    //
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setNbc");
    LOG_DBG("ispCq(%d) tMgrCq(%d)",ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ);
    LOG_DBG("en(%d) anr_eny(%d) anr_enc(%d) anr_iir_mode(%d) anr_scale_mode(%d)\n",
            en,anr_eny,anr_enc,anr_iir_mode,anr_scale_mode);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc_en = (MINT32)en;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_eny = anr_eny;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_enc = anr_enc;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_iir_mode = anr_iir_mode;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_scale_mode = anr_scale_mode;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();

        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc_en = (MINT32)en;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_eny = anr_eny;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_enc = anr_enc;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_iir_mode = anr_iir_mode;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.nbc.anr_scale_mode = anr_scale_mode;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setNbc",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setNbc");
    //
    return MTRUE;
}

MBOOL  TdriMgrImp::
    setSl2(
        ISP_DRV_CQ_ENUM ispCq,
        MBOOL en
)
{
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setSl2");
    LOG_DBG("en(%d) ispCq(%d) tMgrCq(%d)",en,ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.sl2_en = (MINT32)en;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.sl2.hrz_comp = hrz_comp;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.sl2_en = (MINT32)en;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.sl2.hrz_comp = hrz_comp;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setSl2",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    //
    LOCAL_PROFILING_LOG_PRINT("End setSl2");
    return MTRUE;
}

//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::
    setSeee(
        ISP_DRV_CQ_ENUM ispCq,
        MBOOL en,
        int se_edge
        //int usm_over_shrink_en
)
{

    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setSeee");
    LOG_DBG("en(%d) ispCq(%d) tMgrCq(%d)",en,ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee_en = (MINT32)en;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee.se_edge = se_edge;
       // tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee.usm_over_shrink_en = usm_over_shrink_en;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee_en = (MINT32)en;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee.se_edge = se_edge;
        //tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.seee.usm_over_shrink_en = usm_over_shrink_en;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setSeee",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setSeee");
    return MTRUE;
}
/*
//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::setMfb(ISP_DRV_CQ_ENUM ispCq, int bld_mode, int bld_deblock_en)
{
    Mutex::Autolock lock(mLock);
    //
    TPIPE_DRV_CQ_ENUM tMgrCq = tdriMgrCqMap[ispCq].tdriMgrCQ;
    //
    LOG_DBG("ispCq(%d) tMgrCq(%d)",ispCq,tMgrCq);
    //
    if (tMgrCq == TDRI_MGR_DUMMY_CQ) {
        LOG_ERR("isp cq(%d) number error\n",ispCq);
        return MFALSE;
    }
    //
    tdriMgrInfo[tMgrCq].tdriTurningSetting.mfb.bld_mode = bld_mode;
    tdriMgrInfo[tMgrCq].tdriTurningSetting.mfb.bld_deblock_en = bld_deblock_en;
    //
    return MTRUE;
}
*/
//-----------------------------------------------------------------------------
MBOOL  TdriMgrImp::setCfa(ISP_DRV_CQ_ENUM ispCq, int bayer_bypass)
{
    //
    TPIPE_DRV_CQ_ENUM tMgrCq = tdriMgrCqMap[ispCq].tdriMgrCQ;
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setCfa");
    LOG_DBG("bayer_bypass(%d) ispCq(%d) tMgrCq(%d)",bayer_bypass,ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ);
    //
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.cfa.bayer_bypass = bayer_bypass;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.cfa_en = 1;
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.cfa.bayer_bypass = bayer_bypass;
        tdriMgrInfo[tdriMgrCqMap[ispCq].tdriMgrCQ].tdriTurningSetting.cfa_en = 1;
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setCfa",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setCfa");
    //
    return MTRUE;
}

//-----------------------------------------------------------------------------
/*MBOOL  TdriMgrImp::setNr3dTop(ISP_DRV_CQ_ENUM ispCq, MBOOL en)
{
    Mutex::Autolock lock(mLock);
    //
    TPIPE_DRV_CQ_ENUM tMgrCq = tdriMgrCqMap[ispCq].tdriMgrCQ;
    //
    LOG_DBG("en(%d)",en);
    //
    if (tMgrCq == TDRI_MGR_DUMMY_CQ) {
        LOG_ERR("isp cq(%d) number error\n",ispCq);
        return MFALSE;
    }
    //
    tdriMgrInfo[tMgrCq].tdriTurningSetting.nr3d_en = (MINT32)en;
    //
    return MTRUE;
}*/


//-----------------------------------------------------------------------------
MBOOL TdriMgrImp::setOtherEngin(ISP_DRV_CQ_ENUM ispCq, TDRI_MGR_FUNC_ENUM engin)
{
    //
    TPIPE_DRV_CQ_ENUM tMgrCq = tdriMgrCqMap[ispCq].tdriMgrCQ;
    //
    LOCAL_PROFILING_LOG_AUTO_START(Event_TdriMgr);
    LOCAL_PROFILING_LOG_PRINT("setOtherEngin");
    LOG_DBG("ispCq(%d),tMgrCq(%d),engine(%d)",ispCq,tdriMgrCqMap[ispCq].tdriMgrCQ,engin);
    //
    if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ01) {
        pIspDrv->lockSemaphoreCq1();
    }
    else if(tdriMgrCqMap[ispCq].tdriMgrCQ == TPIPE_DRV_CQ02) {
        pIspDrv->lockSemaphoreCq2();
    }
    else {
        LOG_ERR("[error]not support this tMgrCq(%d) for setOtherEngin",tdriMgrCqMap[ispCq].tdriMgrCQ);
        return MFALSE;
    }
    LOCAL_PROFILING_LOG_PRINT("End setOtherEngin");
    return MTRUE;
}





