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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file  eis_drv.cpp

#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <linux/cache.h>

#include <mtkcam/featureio/EIS_Type.h>

#include <mtkcam/drv/imem_drv.h>

#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>

#include "m4u_lib.h"

#include "eis_drv.h"

/****************************************************************************************
* Define Value
****************************************************************************************/
#define LOG_TAG "EISDrv"

#undef __func__
#define __func__ __FUNCTION__

#define EIS_LOG(fmt, arg...)    XLOGD("[%s]"          fmt, __func__,           ##arg)
#define EIS_WRN(fmt, arg...)    XLOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define EIS_ERR(fmt, arg...)    XLOGE("[%s] %s ERR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

//#define EIS_MEMORY_SIZE      408    // 51 * 64 (bits) = 408 bytes
#define EIS_MEMORY_SIZE      256    // 32 * 64 (bits) = 256 bytes

/*******************************************************************************
* Global variable
********************************************************************************/
static MINT32 g_debugDump = 0;

/*******************************************************************************
*
********************************************************************************/
EisDrvBase *EisDrvBase::createInstance()
{
    return EisDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
EisDrvBase *EisDrv::getInstance()
{
    EIS_LOG("+");
    static EisDrv singleton;

    if(singleton.init() != EIS_RETURN_NO_ERROR)
    {
        EIS_LOG("singleton.init() fail");
        return NULL;
    }

    EIS_LOG("-");
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::destroyInstance()
{
	uninit();
}

/*******************************************************************************
*
********************************************************************************/
EisDrv::EisDrv() : EisDrvBase()
{
    // reference count
    mUsers = 0;

    //search window offset max value
    mFLOffsetMax_H = 15;
    mFLOffsetMax_V = 65;

    //ISP object
    m_pISPDrvObj  = NULL;
    m_pISPVirtDrv = NULL;

    // ISP register address
    mISPRegAddr  = 0;

    //IMEM
    m_pIMemDrv = NULL;

    mEisIMemInfo.memID = -5;
    mEisIMemInfo.virtAddr = mEisIMemInfo.phyAddr = mEisIMemInfo.size = 0;

    // config protection usage
    mConfigFail = MFALSE;
    mImgW = 0;
    mImgH = 0;
    mDSRatio = 0;
    mMBNum_H = 0;
    mMBNum_V = 0;
    mRPNum_H = 0;
    mRPNum_V = 0;
    mFLOfset_H = 0;
    mFLOfset_V = 0;
    mMBOfset_H = 0;
    mMBOfset_V = 0;
}

/*******************************************************************************
*
********************************************************************************/
EisDrv::~EisDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::init()
{
    EIS_LOG("mUsers(%d)",mUsers);

    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        EIS_LOG("%d has inited",mUsers);
        android_atomic_inc(&mUsers);
        err = EIS_RETURN_NO_ERROR;
        return err;
    }

    android_atomic_inc(&mUsers);    // increase reference count

    //====== Prepare Memory for DMA ======

    //IMEM
    m_pIMemDrv = IMemDrv::createInstance();
    if(m_pIMemDrv == NULL)
    {
        EIS_LOG("Null IMemDrv Obj");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    MUINT32 eisMemSize = EIS_MEMORY_SIZE;

    createMemBuf(eisMemSize,1,&mEisIMemInfo);
    if(mEisIMemInfo.virtAddr == 0 && mEisIMemInfo.phyAddr == 0)
    {
        EIS_LOG("create IMem fail");
        err = EIS_RETURN_MEMORY_ERROR;
        return err;
    }

    EIS_LOG("EisIMem : memID(%d),size(%u),virAdd(0x%x),phyAddr(0x%x)",mEisIMemInfo.memID,
                                                                      mEisIMemInfo.size,
                                                                      mEisIMemInfo.virtAddr,
                                                                      mEisIMemInfo.phyAddr);

    //====== Create ISP Driver Get ISP HW Register Address ======

    m_pISPDrvObj = IspDrv::createInstance();
    if(m_pISPDrvObj == NULL)
    {
        EIS_ERR("m_pISPDrvObj create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    if(MTRUE != m_pISPDrvObj->init())
    {
        EIS_LOG("m_pISPDrvObj->init() fail");
        err = EIS_RETURN_API_FAIL;
        return err;
    }
#if 0
    // Command Queue
    m_pISPVirtDrv = m_pISPDrvObj->getCQInstance(ISP_DRV_CQ0);
    if(m_pISPVirtDrv == NULL)
    {
        EIS_ERR("m_pISPVirtDrv create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    mISPRegAddr = (MUINT32)m_pISPVirtDrv->getRegAddr();
    if(mISPRegAddr == 0)
    {
        EIS_ERR("get mISPRegAddr fail");
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    m_pISPVirtDrv->cqAddModule(ISP_DRV_CQ0, CAM_DMA_EISO);
    m_pISPVirtDrv->cqAddModule(ISP_DRV_CQ0, CAM_ISP_EIS);
#else

    mISPRegAddr = m_pISPDrvObj->getRegAddr();
    if(mISPRegAddr == 0)
    {
        EIS_ERR("get mISPRegAddr fail");
        err = EIS_RETURN_API_FAIL;
        return err;
    }

#endif

    EIS_LOG("mISPRegAddr = 0x%8x",mISPRegAddr);

    //====== Initializa EIS ======

    setEISOAddr();          // set DMA(EISO) output memory address

    EIS_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::uninit()
{
    EIS_LOG("mUsers(%d) ",mUsers);
    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers <= 0) // No more users
    {
        EIS_LOG("No user");
        return EIS_RETURN_NO_ERROR;
    }

    // >= one user
    android_atomic_dec(&mUsers);

    if(mUsers == 0)
    {
        //====== Reset Register ======

        resetRegister();

        //====== Free Memory ======

        destroyMemBuf(1,&mEisIMemInfo);

        mEisIMemInfo.memID = -5;
        mEisIMemInfo.virtAddr = mEisIMemInfo.phyAddr = mEisIMemInfo.size = 0;

        if(m_pIMemDrv != NULL)
        {
            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //====== Destory ISP Driver Object ======

        if(m_pISPVirtDrv != NULL)
        {
            m_pISPVirtDrv = NULL;
        }

        if(m_pISPDrvObj != NULL)
        {
            err = m_pISPDrvObj->uninit();
            if(err < 0)
            {
                EIS_ERR("m_pISPDrvObj->uninit fail");
                return err;
            }

            m_pISPDrvObj->destroyInstance();

            m_pISPDrvObj = NULL;
        }

        //====== Reset Member Variable ======

        mFLOffsetMax_H = 15;
        mFLOffsetMax_V = 65;
        mISPRegAddr  = 0;
        mConfigFail = MFALSE;
        mImgW = 0;
        mImgH = 0;
        mDSRatio = 0;
        mMBNum_H = 0;
        mMBNum_V = 0;
        mRPNum_H = 0;
        mRPNum_V = 0;
        mFLOfset_H = 0;
        mFLOfset_V = 0;
        mMBOfset_H = 0;
        mMBOfset_V = 0;
    }
    else
    {
        EIS_LOG("Still %d users ", mUsers);
    }

    EIS_LOG("X");
    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    EIS_LOG("Cnt(%u),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->allocVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->mapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisDrv::destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    EIS_LOG("Cnt(%u)", bufCnt);

    MINT32 err = EIS_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                EIS_LOG("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            EIS_LOG("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->unmapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->freeVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MVOID EisDrv::resetRegister()
{
    EIS_LOG("+");

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_SEL_SET, EIS_SEL_SET,0);
    ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_SEL_SET, EIS_RAW_SEL_SET,0);
    ISP_WRITE_BITS(pEis, CAM_CTL_SPARE3, EIS_DB_LD_SEL,0);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H, 1);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V, 1);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HRP, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VRP, 0);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_KNEE, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_CLIP, 0);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HWIN, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN, 0);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_H, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_H, 3);
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_FIR_H, 16);

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_V, 3);

    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_CENTER, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_SOURROUND, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_CENTER, 0);
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_SURROUND, 0);

    ISP_WRITE_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_H,0);
    ISP_WRITE_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_V,0);

    ISP_WRITE_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_H,0);
    ISP_WRITE_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_V,0);

    EIS_LOG("-");
}


/*******************************************************************************
* 15004008, CAM_CTL_EN2, EIS_EN[16]
********************************************************************************/
MVOID EisDrv::enableEIS(MBOOL a_Enable)
{
    EIS_LOG("Enable(0x%x)", a_Enable);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    //15004008, CAM_CTL_EN2_SET, EIS_EN_SET[16]

    if(a_Enable == MTRUE)   //enable
    {
        //ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_EN2_SET, EIS_EN_SET, 0x1);
        ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_EN2, EIS_EN, 1);
        ISP_WRITE_BITS(pEis, CAM_CTL_CDP_DCM_DIS, EIS_DCM_DIS, 1);   // only need in MT6589

        #if 1	//disable this block if Af is ready
        ISP_WRITE_BITS(pEis , CAM_SGG_PGN, SGG_GAIN, 16);
        ISP_WRITE_BITS(pEis , CAM_SGG_GMR, SGG_GMR1, 31);
        ISP_WRITE_BITS(pEis , CAM_SGG_GMR, SGG_GMR2, 63);
        ISP_WRITE_BITS(pEis , CAM_SGG_GMR, SGG_GMR3, 127);
        ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 1);
        ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_EN1_SET, SGG_EN_SET, 1);
        #endif
    }
    else if(a_Enable == MFALSE)  //disable
    {
        //ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_EN2_CLR, EIS_EN_CLR, 0x1);
        ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_EN2, EIS_EN, 0);
        ISP_WRITE_BITS(pEis, CAM_CTL_CDP_DCM_DIS, EIS_DCM_DIS, 0);  // only need in MT6589
    }
    else
    {
        EIS_ERR("wrong value");
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL EisDrv::isEISEnable()
{
    MBOOL ret;

    //15004008, CAM_CTL_EN2, EIS_EN[16]
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ret = (0x1 & ISP_READ_BITS(pEis, CAM_CTL_EN2, EIS_EN));

    EIS_LOG("%d",ret);

    return ret;
}

/*******************************************************************************
* 15004018, CAM_CTL_SEL, EIS_SEL[15]
********************************************************************************/
MVOID EisDrv::setEISSel(MBOOL a_EisSel)
{
    EIS_LOG("[0-before CDRZ, 1-after CDRZ] EisSel(0x%x)", (a_EisSel & 0x1));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_SEL_SET, EIS_SEL_SET,(a_EisSel & 0x1));
}

/*******************************************************************************
* 15004018, CAM_CTL_SEL, EIS_RAW_SEL[16]
********************************************************************************/
MVOID EisDrv::setEISRawSel(MBOOL a_EisRawSel)
{
    EIS_LOG("[0-CDP, 1-RAW] EisRawSel(0x%x)", (a_EisRawSel & 0x1));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_SEL_SET, EIS_RAW_SEL_SET,(a_EisRawSel & 0x1));
}

/*******************************************************************************
* 1500406C, CAM_CTL_SPARE3, EIS_DB_LD_SEL[6]
********************************************************************************/
MVOID EisDrv::setEIS_DB_SEL(MBOOL a_EisDB)
{
    EIS_LOG("[0-no change, 1-raw_db_load1] EisDB(0x%x)", (a_EisDB & 0x1));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_ENABLE_BITS(pEis, CAM_CTL_SPARE3, EIS_DB_LD_SEL,(a_EisDB & 0x1));
}

/*******************************************************************************
* 15004DC0, CAM_EIS_PREP_ME_CTRL1
********************************************************************************/
MVOID EisDrv::setEISFilterDS(MINT32 a_DS)
{
    EIS_LOG("a_DS(0x%x)", (a_DS & 0x7));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    switch(a_DS)
    {
    case 1 :
    case 2 :
    case 4 :
        break;
    default :
        EIS_ERR("Error down sample ratio");
        return;
        break;
    }

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H, (a_DS & 0x7));
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V, (a_DS & 0x7));

    mDSRatio = (a_DS & 0x7);
}

/*******************************************************************************
* 15004DC0, CAM_EIS_PREP_ME_CTRL1
********************************************************************************/
MVOID EisDrv::setRPNum(MINT32 a_RPNum_H, MINT32 a_RPNum_V)
{
    EIS_LOG("[H:1-16,V:1-8] H(%d),V(%d)", a_RPNum_H, a_RPNum_V);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    //====== Horizontal ======

    boundaryCheck(a_RPNum_H, 16, 1);
    EIS_LOG("final RPNum_H(0x%x)", (a_RPNum_H & 0x1F));

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HRP, (a_RPNum_H & 0x1F));
    mRPNum_H = (a_RPNum_H & 0x1F);

    //====== Vertical ======

    MINT32 tempMBNum_V = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN);

    if(tempMBNum_V != mMBNum_V)
    {
        EIS_ERR("ISP read ME_NUM_VWIN fail : tempMBNum_V(%d),mMBNum_V(%d)", tempMBNum_V, mMBNum_V);
        tempMBNum_V = mMBNum_V;
        mConfigFail |= MTRUE;
    }
    else
    {
        mConfigFail |= MFALSE;
    }

    if(tempMBNum_V <= 4)
    {
        boundaryCheck(a_RPNum_V,8,1);
    }
    else
    {
        boundaryCheck(a_RPNum_V,4,1);
    }

    EIS_LOG("MBNum_V(0x%x),final RPNum_V(0x%x)",tempMBNum_V,(a_RPNum_V & 0xF));

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VRP, (a_RPNum_V & 0xF));
    mRPNum_V = (a_RPNum_V & 0xF);
}

/*******************************************************************************
* 15004DC0, CAM_EIS_PREP_ME_CTRL1
********************************************************************************/
void EisDrv::setADKneeClip(MINT32 a_Knee, MINT32 a_Clip)
{
    EIS_LOG("Knee(%d),Clip(%d)", a_Knee, a_Clip);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    boundaryCheck(a_Knee,15,0);
    boundaryCheck(a_Clip,15,0);

    EIS_LOG("Final Knee(0x%x),Clip(0x%x)", (a_Knee & 0xF), (a_Clip & 0xF));

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_KNEE, (a_Knee & 0xF));
    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_CLIP, (a_Clip & 0xF));
}

/*******************************************************************************
* 15004DC0, CAM_EIS_PREP_ME_CTRL1
********************************************************************************/
MVOID EisDrv::setMBNum(MINT32 a_MBNum_H, MINT32 a_MBNum_V)
{
    EIS_LOG("[H:1-4,V:1-8] H(%d),V(%d)", a_MBNum_H, a_MBNum_V);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    //====== Horizontal =====

    boundaryCheck(a_MBNum_H, 4, 1);
    EIS_LOG("final MBNum_H(0x%x)", (a_MBNum_H & 0x7));

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HWIN, (a_MBNum_H & 0x7));
    mMBNum_H = (a_MBNum_H & 0x7);

    //====== Vertical ======

    boundaryCheck(a_MBNum_V, 8, 1);
    EIS_LOG("final MBNum_V(0x%x)", (a_MBNum_V & 0xF));

    ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN, (a_MBNum_V & 0xF));
    mMBNum_V = (a_MBNum_V & 0xF);
}

/*******************************************************************************
* 15004DC4, CAM_EIS_PREP_ME_CTRL2
********************************************************************************/
MVOID EisDrv::setFilter_H(MINT32 a_Gain, MINT32 a_IIRGain, MINT32 a_FIRGain)
{
    EIS_LOG("Gain(%d),IIRGain(%d),FIRGain(%d)", (a_Gain & 0x3), (a_IIRGain & 0x7), (a_FIRGain & 0x3F));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    if(a_Gain != 0 && a_Gain != 1 && a_Gain != 3)
    {
        EIS_ERR("wrong a_Gain, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_H, (a_Gain & 0x3));
    }

    if (a_IIRGain != 3 && a_IIRGain != 4)
    {
        EIS_ERR("wrong a_IIRGain, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_H, (a_IIRGain & 0x7));
    }

    if (a_FIRGain != 16 && a_FIRGain != 32)
    {
        EIS_ERR("wrong a_FIRGain, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_FIR_H, (a_FIRGain & 0x3F));
    }
}

/*******************************************************************************
* 15004DC4, CAM_EIS_PREP_ME_CTRL2
********************************************************************************/
MVOID EisDrv::setFilter_V(MINT32 a_IIRGain)
{
    EIS_LOG("IIRGain(0x%x)", (a_IIRGain & 0x7));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    if (a_IIRGain != 3 && a_IIRGain != 4)
    {
        EIS_ERR("wrong a_IIRGain, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_V, (a_IIRGain & 0x7));
    }
}

/*******************************************************************************
* 15004DC4, CAM_EIS_PREP_ME_CTRL2
********************************************************************************/
MVOID EisDrv::setWRPEnable(MBOOL a_Enable)
{
    EIS_LOG("Enable(0x%x)", (a_Enable & 0x1));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    if (a_Enable != MTRUE && a_Enable != MFALSE)
    {
        EIS_ERR("wrong a_Enable, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, EIS_WRP_EN, (a_Enable & 0x1));
    }
}

/*******************************************************************************
* 15004DC4, CAM_EIS_PREP_ME_CTRL2
********************************************************************************/
MVOID EisDrv::setFirstFrame(MBOOL a_First)
{
    EIS_LOG("First(0x%x)", (a_First & 0x1));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    if (a_First != MTRUE && a_First != MFALSE)
    {
        EIS_ERR("wrong a_First, setting fail");
    }
    else
    {
        ISP_WRITE_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, EIS_FIRST_FRM, (a_First & 0x1));
    }
}

/*******************************************************************************
* 15004DC8, CAM_EIS_LMV_TH
********************************************************************************/
MVOID EisDrv::setLMV_TH(MINT32 a_Center_X, MINT32 a_Surrond_X, MINT32 a_Center_Y, MINT32 a_Surrond_Y)
{
    EIS_LOG("HC(%d),HS(%d),VC(%d),VS(%d)", a_Center_X, a_Surrond_X, a_Center_Y, a_Surrond_Y);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    boundaryCheck(a_Center_X,  255, 0);
    boundaryCheck(a_Surrond_X, 255, 0);
    boundaryCheck(a_Center_Y,  255, 0);
    boundaryCheck(a_Surrond_Y, 255, 0);

    EIS_LOG("Final HC(0x%x),HS(0x%x),VC(0x%x),VS(0x%x)",(a_Center_X  & 0xFF),(a_Surrond_X & 0xFF),(a_Center_Y  & 0xFF),(a_Surrond_Y & 0xFF));

    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_CENTER, (a_Center_X  & 0xFF));
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_SOURROUND, (a_Surrond_X & 0xFF));
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_CENTER, (a_Center_Y  & 0xFF));
    ISP_WRITE_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_SURROUND, (a_Surrond_Y & 0xFF));
}

/*******************************************************************************
* 15004DCC, CAM_EIS_FL_OFFSET
********************************************************************************/
MVOID EisDrv::setFLOffsetMax(MINT32 a_FLOffsetMax_H, MINT32 a_FLOffsetMax_V)
{
    EIS_LOG("Max_H(%d),Max_V(%d)", a_FLOffsetMax_H,a_FLOffsetMax_V);

    mFLOffsetMax_H = a_FLOffsetMax_H;
    mFLOffsetMax_V = a_FLOffsetMax_V;

    boundaryCheck(mFLOffsetMax_H, 15, 0);
    boundaryCheck(mFLOffsetMax_V, 33, 0);

    EIS_LOG("final Max_H(%d),Max_V(%d)", mFLOffsetMax_H,mFLOffsetMax_V);
}

/*******************************************************************************
* 15004DCC, CAM_EIS_FL_OFFSET
********************************************************************************/
MVOID EisDrv::setFLOffset(MINT32 a_FLOffset_H, MINT32 a_FLOffset_V)
{
    EIS_LOG("H(%d),V(%d)", a_FLOffset_H, a_FLOffset_V);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    boundaryCheck(a_FLOffset_H, mFLOffsetMax_H, (0 - mFLOffsetMax_H));
    boundaryCheck(a_FLOffset_V, mFLOffsetMax_V, (0 - (mFLOffsetMax_V - 1)));

    EIS_LOG("final FLOfs_H(0x%x), FLOfs_V(0x%x)", (a_FLOffset_H & 0xFFF), (a_FLOffset_V & 0xFFF));

    ISP_WRITE_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_H, (a_FLOffset_H & 0xFFF));
    ISP_WRITE_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_V, (a_FLOffset_V & 0xFFF));

    mFLOfset_H = (a_FLOffset_H & 0xFFF);
    mFLOfset_V = (a_FLOffset_V & 0xFFF);
}

/*******************************************************************************
* 15004DD0, CAM_EIS_MB_OFFSET
********************************************************************************/
MVOID EisDrv::setMBOffset_H(MINT32 a_MBOffset_H)
{
    EIS_LOG("H(%d)", a_MBOffset_H);

    MBOOL errFlag = MFALSE;
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    MINT32 upBound,lowBound;
    MINT32 tempWidth      = ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, WIDTH);
    MINT32 tempFLOffset_H = Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_H), 12);
    MINT32 tempDSRatio_H  = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H);
    MINT32 tempMBNum_H    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HWIN);
    MINT32 tempMBInterval_H = Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_H), 12);
    MINT32 tempRPNum_H    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HRP);

    //====== Value Checking ======

    if(mImgW != tempWidth)
    {
        EIS_ERR("ISP read WIDTH fail : tempWidth(%d),mImgW(%d)", tempWidth, mImgW);
        tempWidth = mImgW;
        errFlag = MTRUE;
    }

    if(mFLOfset_H != tempFLOffset_H)
    {
        EIS_ERR("ISP read FL_OFFSET_H fail : tempFLOffset_H(%d),mFLOfset_H(%d)", tempFLOffset_H, mFLOfset_H);
        tempFLOffset_H = mFLOfset_H;
        errFlag = MTRUE;
    }

    if(mDSRatio != tempDSRatio_H)
    {
        EIS_ERR("ISP read PREP_DS_IIR_H fail : tempDSRatio_H(%d),mDSRatio(%d)", tempDSRatio_H, mDSRatio);
        tempDSRatio_H = mDSRatio;
        errFlag = MTRUE;
    }

    if(mMBNum_H != tempMBNum_H)
    {
        EIS_ERR("ISP read ME_NUM_HWIN fail : tempMBNum_H(%d),mMBNum_H(%d)", tempMBNum_H, mMBNum_H);
        tempMBNum_H = mMBNum_H;
        errFlag = MTRUE;
    }

    if(errFlag == MFALSE)
    {
        mConfigFail |= MFALSE;
    }
    else
    {
        mConfigFail |= MTRUE;
    }

    //====== Check Limitation ======

    // low bound
    if(tempFLOffset_H < 0)
    {
        lowBound = 11 - tempFLOffset_H;
    }
    else
    {
        lowBound = 11 + tempFLOffset_H;
    }

    // up bound
    if(tempFLOffset_H > 0)
    {
        upBound = tempWidth/tempDSRatio_H - tempRPNum_H*16 - tempFLOffset_H - tempMBInterval_H*(tempMBNum_H-1);
    }
    else
    {
        upBound = tempWidth/tempDSRatio_H - tempRPNum_H*16 - 1 - tempMBInterval_H*(tempMBNum_H-1);
    }

    EIS_LOG("W(%d),FLOfs_H(%d),DSRat_H(%d),MBNum_H(%d),MBInt_H(%d),RPNum_H(%d)"
            ,tempWidth,tempFLOffset_H,tempDSRatio_H,tempMBNum_H,tempMBInterval_H,tempRPNum_H);
    EIS_LOG("upB(%d),lowB(%d)", upBound, lowBound);

    if(upBound < lowBound)
    {
        EIS_ERR("wrong boundary");
    }
    else
    {
        boundaryCheck(a_MBOffset_H, upBound, lowBound);

        EIS_LOG("final MBOfs_H(0x%x)", (a_MBOffset_H & 0xFFF));

        ISP_WRITE_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_H,(a_MBOffset_H & 0xFFF));
        mMBOfset_H = (a_MBOffset_H & 0xFFF);
    }
}

/*******************************************************************************
* 15004DD0, CAM_EIS_MB_OFFSET
********************************************************************************/
MVOID EisDrv::setMBOffset_V(MINT32 a_MBOffset_V)
{
    EIS_LOG("V(%d)", a_MBOffset_V);

    MBOOL errFlag = MFALSE;
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    MINT32 upBound,lowBound;
    MINT32 tempHeight     = ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, HEIGHT);
    MINT32 tempFLOffset_V = Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_V), 12);
    MINT32 tempDSRatio_V  = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V);
    MINT32 tempMBNum_V    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN);
    MINT32 tempMBInterval_V = Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_V), 12);
    MINT32 tempRPNum_V    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VRP);

    //====== Value Checking ======

    if(mImgH != tempHeight)
    {
        EIS_ERR("ISP read HEIGHT fail : tempHeight(%d),mImgH(%d)", tempHeight, mImgH);
        tempHeight = mImgH;
        errFlag = MTRUE;
    }

    if(mFLOfset_V != tempFLOffset_V)
    {
        EIS_ERR("ISP read FL_OFFSET_V fail : tempFLOffset_V(%d),mFLOfset_V(%d)", tempFLOffset_V, mFLOfset_V);
        tempFLOffset_V = mFLOfset_V;
        errFlag = MTRUE;
    }

    if(mDSRatio != tempDSRatio_V)
    {
        EIS_ERR("ISP read PREP_DS_IIR_V fail : tempDSRatio_V(%d),mDSRatio(%d)", tempDSRatio_V, mDSRatio);
        tempDSRatio_V = mDSRatio;
        errFlag = MTRUE;
    }

    if(mMBNum_V != tempMBNum_V)
    {
        EIS_ERR("ISP read ME_NUM_VWIN fail : tempMBNum_V(%d),mMBNum_V(%d)", tempMBNum_V, mMBNum_V);
        tempMBNum_V = mMBNum_V;
        errFlag = MTRUE;
    }

    if(errFlag == MFALSE)
    {
        mConfigFail |= MFALSE;
    }
    else
    {
        mConfigFail |= MTRUE;
    }

    //====== Check Limitation ======

    // low bound
    if(tempFLOffset_V < 0)
    {
        lowBound = 9 - tempFLOffset_V;
    }
    else
    {
        lowBound = 9 + tempFLOffset_V;
    }

    // up bound
    if(tempFLOffset_V > 0)
    {
        upBound = tempHeight/tempDSRatio_V - tempRPNum_V*16 - tempFLOffset_V - tempMBInterval_V*(tempMBNum_V-1);
    }
    else
    {
        upBound = tempHeight/tempDSRatio_V - tempRPNum_V*16 - 1 - tempMBInterval_V*(tempMBNum_V-1);
    }

    EIS_LOG("H(%d),FLOfs_V(%d),DSRat_V(%d),MBNum_V(%d),MBInt_V(%d),RPNum_V(%d)"
            ,tempHeight,tempFLOffset_V,tempDSRatio_V,tempMBNum_V,tempMBInterval_V,tempRPNum_V);
    EIS_LOG("upB(%d), lowB(%d)", upBound, lowBound);

    if(upBound < lowBound)
    {
        EIS_ERR("wrong boundary");
    }
    else
    {
        boundaryCheck(a_MBOffset_V, upBound, lowBound);

        EIS_LOG("final MBOfs_V(0x%x)", (a_MBOffset_V & 0xFFF));

        ISP_WRITE_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_V,(a_MBOffset_V & 0xFFF));
        mMBOfset_V = (a_MBOffset_V & 0xFFF);
    }
}

/*******************************************************************************
* 15004DD4, CAM_EIS_MB_INTERVAL
********************************************************************************/
MVOID EisDrv::setMBInterval_H(MINT32 a_MBInterval_H)
{
    EIS_LOG("H(%d)", a_MBInterval_H);

    MBOOL errFlag = MFALSE;
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    MINT32 upBound,lowBound;
    MINT32 tempWidth      = ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, WIDTH);
    MINT32 tempFLOffset_H = Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_H), 12);
    MINT32 tempDSRatio_H  = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H);
    MINT32 tempRPNum_H    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HRP);
    MINT32 tempMBNum_H    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HWIN);
    MINT32 tempMBOffset_H = Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_H), 12);

    //====== Value Checking ======

    if(mImgW != tempWidth)
    {
        EIS_ERR("ISP read WIDTH fail : tempWidth(%d),mImgW(%d)", tempWidth, mImgW);
        tempWidth = mImgW;
        errFlag = MTRUE;
    }

    if(mFLOfset_H != tempFLOffset_H)
    {
        EIS_ERR("ISP read FL_OFFSET_H fail : tempFLOffset_H(%d),mFLOfset_H(%d)", tempFLOffset_H, mFLOfset_H);
        tempFLOffset_H = mFLOfset_H;
        errFlag = MTRUE;
    }

    if(mDSRatio != tempDSRatio_H)
    {
        EIS_ERR("ISP read PREP_DS_IIR_H fail : tempDSRatio_H(%d),mDSRatio(%d)", tempDSRatio_H, mDSRatio);
        tempDSRatio_H = mDSRatio;
        errFlag = MTRUE;
    }

    if(mRPNum_H != tempRPNum_H)
    {
        EIS_ERR("ISP read ME_NUM_HRP fail : tempRPNum_H(%d),mRPNum_H(%d)", tempRPNum_H, mRPNum_H);
        tempRPNum_H = mRPNum_H;
        errFlag = MTRUE;
    }

    if(mMBNum_H != tempMBNum_H)
    {
        EIS_ERR("ISP read ME_NUM_HWIN fail : tempMBNum_H(%d),mMBNum_H(%d)", tempMBNum_H, mMBNum_H);
        tempMBNum_H = mMBNum_H;
        errFlag = MTRUE;
    }

    if(mMBOfset_H != tempMBOffset_H)
    {
        EIS_ERR("ISP read MB_OFFSET_H fail : tempMBOffset_H(%d),mMBOfset_H(%d)", tempMBOffset_H, mMBOfset_H);
        tempMBOffset_H = mMBOfset_H;
        errFlag = MTRUE;
    }

    if(errFlag == MFALSE)
    {
        mConfigFail |= MFALSE;
    }
    else
    {
        mConfigFail |= MTRUE;
    }

    //====== Check Limitation ======

    // low bound
    lowBound = (tempRPNum_H + 1) * 16;

    // up bound
    if(tempFLOffset_H > 0)
    {
        upBound = ((tempWidth / tempDSRatio_H) - tempMBOffset_H - tempRPNum_H*16 - tempFLOffset_H) / (tempMBNum_H-1);
    }
    else
    {
        upBound = ((tempWidth / tempDSRatio_H) - tempMBOffset_H - tempRPNum_H*16 - 1) / (tempMBNum_H-1);
    }

    EIS_LOG("W(%d),FLOfs_H(%d),DSRat_H(%d),RPNum_H(%d),MBNum_H(%d),MBOfs_H(%d)"
            ,tempWidth,tempFLOffset_H,tempDSRatio_H,tempRPNum_H,tempMBNum_H,tempMBOffset_H);
    EIS_LOG("upB(%d), lowB(%d)", upBound, lowBound);

    if(upBound < lowBound)
    {
        EIS_ERR("wrong boundary");
    }
    else
    {
        boundaryCheck(a_MBInterval_H, upBound, lowBound);

        EIS_LOG("final MBInt_H(0x%x)", (a_MBInterval_H & 0xFFF));

        ISP_WRITE_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_H,(a_MBInterval_H & 0xFFF));
    }
}

/*******************************************************************************
* 15004DD4, CAM_EIS_MB_INTERVAL
********************************************************************************/
MVOID EisDrv::setMBInterval_V(MINT32 a_MBInterval_V)
{
    EIS_LOG("V(%d)", a_MBInterval_V);

    MBOOL errFlag = MFALSE;
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    MINT32 upBound,lowBound;
    MINT32 tempHeight     = ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, HEIGHT);
    MINT32 tempFLOffset_V = Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_V), 12);
    MINT32 tempDSRatio_V  = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V);
    MINT32 tempRPNum_V    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VRP);
    MINT32 tempMBNum_V    = ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN);
    MINT32 tempMBOffset_V = Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_V), 12);

    //====== Value Checking ======

    if(mImgH != tempHeight)
    {
        EIS_ERR("ISP read HEIGHT fail : tempHeight(%d),mImgH(%d)", tempHeight, mImgH);
        tempHeight = mImgH;
        errFlag = MTRUE;
    }

    if(mFLOfset_V != tempFLOffset_V)
    {
        EIS_ERR("ISP read FL_OFFSET_V fail : tempFLOffset_V(%d),mFLOfset_V(%d)", tempFLOffset_V, mFLOfset_V);
        tempFLOffset_V = mFLOfset_V;
        errFlag = MTRUE;
    }

    if(mDSRatio != tempDSRatio_V)
    {
        EIS_ERR("ISP read PREP_DS_IIR_V fail : tempDSRatio_V(%d),mDSRatio(%d)", tempDSRatio_V, mDSRatio);
        tempDSRatio_V = mDSRatio;
        errFlag = MTRUE;
    }

    if(mRPNum_V != tempRPNum_V)
    {
        EIS_ERR("ISP read ME_NUM_VRP fail : tempRPNum_V(%d),mRPNum_V(%d)", tempRPNum_V, mRPNum_V);
        tempRPNum_V = mRPNum_V;
        errFlag = MTRUE;
    }

    if(mMBNum_V != tempMBNum_V)
    {
        EIS_ERR("ISP read ME_NUM_VWIN fail : tempMBNum_H(%d),mMBNum_V(%d)", tempMBNum_V, mMBNum_V);
        tempMBNum_V = mMBNum_V;
        errFlag = MTRUE;
    }

    if(mMBOfset_V != tempMBOffset_V)
    {
        EIS_ERR("ISP read MB_OFFSET_V fail : tempMBOffset_V(%d),mMBOfset_V(%d)", tempMBOffset_V, mMBOfset_V);
        tempMBOffset_V = mMBOfset_V;
        errFlag = MTRUE;
    }

    if(errFlag == MFALSE)
    {
        mConfigFail |= MFALSE;
    }
    else
    {
        mConfigFail |= MTRUE;
    }

    //====== Check Limitation ======

    // low bound
    lowBound = (tempRPNum_V + 1) * 16 + 1;

    // up bound
    if(tempFLOffset_V > 0)
    {
        upBound = ((tempHeight / tempDSRatio_V) - tempMBOffset_V - tempRPNum_V*16 - tempFLOffset_V) / (tempMBNum_V-1);
    }
    else
    {
        upBound = ((tempHeight / tempDSRatio_V) - tempMBOffset_V - tempRPNum_V*16 - 1) / (tempMBNum_V-1);
    }

    EIS_LOG("H(%d),FLOfs_V(%d),DSRat_V(%d),RPNum_V(%d),MBNum_V(%d),MBOfs_V(%d)"
        ,tempHeight,tempFLOffset_V,tempDSRatio_V,tempRPNum_V,tempMBNum_V,tempMBOffset_V);
    EIS_LOG("upB(%d), lowB(%d)", upBound, lowBound);

    if(upBound < lowBound)
    {
        EIS_ERR("wrong boundary");
    }
    else
    {
        boundaryCheck(a_MBInterval_V, upBound, lowBound);

        EIS_LOG("final MBInt_V(0x%x)", (a_MBInterval_V & 0xFFF));

        ISP_WRITE_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_V, (a_MBInterval_V & 0xFFF));
    }
}

/*******************************************************************************
* 15004DE0, CAM_EIS_IMAGE_CTRL
********************************************************************************/
MVOID EisDrv::setEISImage(MINT32 a_ImgWidth, MINT32 a_ImgHeight)
{
    EIS_LOG("W(%d),H(%d)",(a_ImgWidth & 0x1FFF),(a_ImgHeight & 0x1FFF));

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_BITS(pEis, CAM_EIS_IMAGE_CTRL, WIDTH,(a_ImgWidth & 0x1FFF));
    ISP_WRITE_BITS(pEis, CAM_EIS_IMAGE_CTRL, HEIGHT,(a_ImgHeight & 0x1FFF));

    mImgW = (a_ImgWidth & 0x1FFF);
    mImgH = (a_ImgHeight & 0x1FFF);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::setEISOAddr()
{
    EIS_LOG("VA(0x%x),PA(0x%x)",mEisIMemInfo.virtAddr, mEisIMemInfo.phyAddr);

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    ISP_WRITE_REG(pEis, CAM_EISO_BASE_ADDR, mEisIMemInfo.phyAddr);
    ISP_WRITE_BITS(pEis, CAM_EISO_XSIZE, XSIZE, EIS_MEMORY_SIZE-1); // fix number : 0x197(407)
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::getFLOffsetMax(MINT32 &a_FLOffsetMax_H, MINT32 &a_FLOffsetMax_V)
{
    a_FLOffsetMax_H = mFLOffsetMax_H;
    a_FLOffsetMax_V = mFLOffsetMax_V;

    EIS_LOG("Max_H(%d),Max_V(%d)", a_FLOffsetMax_H, a_FLOffsetMax_V);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::getDSRatio(MINT32 &a_DS_H, MINT32 &a_DS_V)
{
    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    a_DS_H = (MINT32)ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H);
    a_DS_V = (MINT32)ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V);

    EIS_LOG("DS_H(%d),DS_V(%d)",a_DS_H,a_DS_V);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::getStatistic(EIS_STATISTIC_T *a_pEIS_Stat)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

    MUINT32 *pEISOAddr = (MUINT32 *)mEisIMemInfo.virtAddr;

    if(g_debugDump == 3)
    {
        EIS_LOG("+");
        EIS_LOG("pEISOAddr(0x%x)",(MUINT32)pEISOAddr);
    }

    //====== BASE + 0~31 ======

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        if(i != 0)
        {
            pEISOAddr += 2;  // 64bits(8bytes)

            if(g_debugDump == 3)
            {
                EIS_LOG("1. i(%d),pEISOAddr(0x%x)",i,(MUINT32)pEISOAddr);
            }
        }

        a_pEIS_Stat->i4LMV_X2[i]	= Complement2(*pEISOAddr & 0x1F, 5);              //[0:4]
        a_pEIS_Stat->i4LMV_Y2[i]	= Complement2(((*pEISOAddr & 0x3E0) >> 5), 5);    //[5:9]
        a_pEIS_Stat->i4SAD[i]		= (*pEISOAddr & 0x7FC00) >> 10;                    //[10:18]
        a_pEIS_Stat->i4NewTrust_X[i]= (*pEISOAddr & 0x03F80000) >> 19;            //[19:25]
        a_pEIS_Stat->i4NewTrust_Y[i]= ((*pEISOAddr & 0xFC000000) >> 26)             //[26:32]
									 + ((*(pEISOAddr+1) & 0x00000001) << 6);
        a_pEIS_Stat->i4LMV_X[i]		= Complement2(((*(pEISOAddr + 1) & 0x00003FFE) >> 1), 13);     //[33:45] -> [1:13]
        a_pEIS_Stat->i4LMV_Y[i]		= Complement2(((*(pEISOAddr + 1) & 0x07FFC000) >> 14), 13); //[46:58] -> [14:26]

        a_pEIS_Stat->i4SAD2[i]		= 0;
        a_pEIS_Stat->i4AVG[i]		= 0;
    }

#if 0
    for(MINT32 i = 0; i < 10; ++i)
    {
		EIS_LOG("EIS[%d]=mv(%d,%d), trust(%d,%d), sad(%d), avg(%d), mv2(%d,%d), sad2(%d)"
				, i
				, a_pEIS_Stat->i4LMV_X[i], a_pEIS_Stat->i4LMV_Y[i]
				, a_pEIS_Stat->i4NewTrust_X[i], a_pEIS_Stat->i4NewTrust_Y[i]
				, a_pEIS_Stat->i4SAD[i]
				, a_pEIS_Stat->i4AVG[i]
				, a_pEIS_Stat->i4LMV_X2[i], a_pEIS_Stat->i4LMV_Y2[i]
				, a_pEIS_Stat->i4SAD2[i]
				);
    }
#endif

#if 0
    pEISOAddr = (MUINT32 *)mEisIMemInfo.virtAddr;
    //for(MINT32 i = 0; i < 2*EIS_MAX_WIN_NUM; ++i)
    for(MINT32 i = 0; i < 10; ++i)
    {
		EIS_LOG("EIS[%d]=(0x%08x)",i,pEISOAddr[i]);
    }
#endif


#if 0
    EIS_LOG("LMV");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, LMV_X = %d, LMV_Y = %d",(i/4),(i%4),a_pEIS_Stat->i4LMV_X[i],a_pEIS_Stat->i4LMV_Y[i]);
    }

    EIS_LOG("LMV_2nd");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, LMV_X2 = %d, LMV_Y2 = %d",(i/4),(i%4),a_pEIS_Stat->i4LMV_X2[i],a_pEIS_Stat->i4LMV_Y2[i]);
    }

    EIS_LOG("MinSAD");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, MinSAD = %d",(i/4),(i%4),a_pEIS_Stat->i4SAD[i]);
    }

    EIS_LOG("MinSAD_2nd");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, MinSAD2 = %d",(i/4),(i%4),a_pEIS_Stat->i4SAD2[i]);
    }

    EIS_LOG("AvgSAD");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, AvgSAD = %d",(i/4),(i%4),a_pEIS_Stat->i4AVG[i]);
    }

    EIS_LOG("NewTrust");
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, NewTrust_X = %d, NewTrust_Y = %d",(i/4),(i%4),a_pEIS_Stat->i4NewTrust_X[i],a_pEIS_Stat->i4NewTrust_Y[i]);
    }

    dumpReg();
#endif

    if(g_debugDump == 3)
    {
        EIS_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL EisDrv::configStatus()
{
    return mConfigFail;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::resetConfigStatus()
{
    mConfigFail = MFALSE;
    EIS_LOG("mConfigFail(%d)",mConfigFail);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrv::dumpReg()
{
    EIS_LOG("+");

    isp_reg_t *pEis = (isp_reg_t *)mISPRegAddr;

    EIS_LOG("ESFKO_EN(0x%x)",ISP_READ_BITS(pEis, CAM_CTL_DMA_EN, ESFKO_EN));
    EIS_LOG("SGG_EN(0x%x)",ISP_READ_BITS(pEis, CAM_CTL_EN1, SGG_EN));
    EIS_LOG("SGG_PGN(0x%x)",ISP_READ_REG(pEis, CAM_SGG_PGN));
    EIS_LOG("SGG_GMR1(0x%x)",ISP_READ_BITS(pEis, CAM_SGG_GMR, SGG_GMR1));
    EIS_LOG("SGG_GMR2(0x%x)",ISP_READ_BITS(pEis, CAM_SGG_GMR, SGG_GMR2));
    EIS_LOG("SGG_GMR3(0x%x)",ISP_READ_BITS(pEis, CAM_SGG_GMR, SGG_GMR3));


    EIS_LOG("EISO(0x%x)",ISP_READ_REG(pEis, CAM_EISO_BASE_ADDR));
    EIS_LOG("EIS_EN(0x%x)",ISP_READ_BITS(pEis, CAM_CTL_EN2, EIS_EN));
    EIS_LOG("EIS_RAW_SEL(0x%x)",ISP_READ_BITS(pEis, CAM_CTL_SEL, EIS_RAW_SEL));
    EIS_LOG("EIS_DB_LD_SEL(0x%x)",ISP_READ_BITS(pEis, CAM_CTL_SPARE3, EIS_DB_LD_SEL));

    EIS_LOG("DS_IIR_H(0x%x),DS_IIR_V(0x%x),NumRP_H(0x%x),NumRP_V(0x%x),CLIP(0x%x),KNEE(0x%x),NumMB_H(0x%x),NumMB_V(0x%x)",
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_H),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, PREP_DS_IIR_V),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HRP),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VRP),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_CLIP),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_AD_KNEE),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_HWIN),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL1, ME_NUM_VWIN));

    EIS_LOG("GAIN_H(0x%x),GAIN_IIR_H(0x%x),GAIN_IIR_V(0x%x),GAIN_FIR_H(0x%x),WRP_EN(0x%x),FIRST_FRM(0x%x)",
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_H),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_H),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_GAIN_IIR_V),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, PREP_FIR_H),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, EIS_WRP_EN),
                           ISP_READ_BITS(pEis, CAM_EIS_PREP_ME_CTRL2, EIS_FIRST_FRM));

    EIS_LOG("X_CENTER(0x%x),X_SURROUND(0x%x),Y_CENTER(0x%x),Y_SURROUND(0x%x)",
                           ISP_READ_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_CENTER),
                           ISP_READ_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_X_SOURROUND),
                           ISP_READ_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_CENTER),
                           ISP_READ_BITS(pEis, CAM_EIS_LMV_TH, LMV_TH_Y_SURROUND));

    EIS_LOG("FL_OFFSET_H(0x%x),FL_OFFSET_V(0x%x)",
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_H), 12),
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_FL_OFFSET, FL_OFFSET_V), 12));

    EIS_LOG("MB_OFFSET_H(0x%x),MB_OFFSET_V(0x%x)",
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_H), 12),
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_OFFSET, MB_OFFSET_V), 12));

    EIS_LOG("MB_INTERVAL_H(0x%x),MB_INTERVAL_V(0x%x)",
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_H), 12),
                           Complement2(ISP_READ_BITS(pEis, CAM_EIS_MB_INTERVAL, MB_INTERVAL_V), 12));

    EIS_LOG("WIDTH(0x%x),HEIGHT(0x%x),PIPE_MODE(0x%x)",
                           ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, WIDTH),
                           ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, HEIGHT),
                           ISP_READ_BITS(pEis, CAM_EIS_IMAGE_CTRL, PIPE_MODE));

    EIS_LOG("-");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::max(MINT32 a, MINT32 b)
{
    if(a >= b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrv::Complement2(MUINT32 Vlu, MUINT32 Digit)
{
    MINT32 Result;

    if (((Vlu >> (Digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~Vlu + 1) & ((1 << Digit) - 1));
    }
    else
    {
        Result = (MINT32)(Vlu & ((1 << Digit) - 1));
    }

    return Result;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID EisDrv::boundaryCheck(MINT32 &a_input, MINT32 upBound, MINT32 lowBound)
{
    if(a_input > upBound)
    {
        a_input = upBound;
    }

    if(a_input < lowBound)
    {
        a_input = lowBound;
    }
}


