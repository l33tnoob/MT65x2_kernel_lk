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
#define LOG_TAG "buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <string.h>
//#include <cutils/pmem.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <mtkcam/drv/isp_drv.h>
#include "buf_mgr.h"
#include <linux/cache.h>

using namespace NS3A;

BufInfoList_T BufMgr::m_rHwBufList[ECamDMA_NUM];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BufMgr&
BufMgr::
getInstance()
{
    static  BufMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BufMgr::
BufMgr()
    : m_pIMemDrv(IMemDrv::createInstance())
    , m_pIspDrv(MNULL)
    , m_pIspReg(MNULL)
    , m_pVirtIspDrvCQ0(MNULL)
    , m_pVirtIspRegCQ0(MNULL)
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BufMgr::
~BufMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
init()
{
    MRESULT ret = S_BUFMGR_OK;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    

	MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

	Mutex::Autolock lock(m_Lock);

	if (m_Users > 0)
	{
		MY_LOG("%d has created \n", m_Users);
		android_atomic_inc(&m_Users);
		return S_BUFMGR_OK;
	}

    // IspDrv init
    m_pIspDrv = IspDrv::createInstance();

    MY_LOG_IF(m_bDebugEnable,"[m_pIspDrv]:0x%08x \n",reinterpret_cast<MUINT32>(m_pIspDrv));

    if (!m_pIspDrv) {
        MY_ERR("IspDrv::createInstance() fail \n");
        return ret;
    }

    ret = m_pIspDrv->init();
    if (FAILED(ret)) {
        MY_ERR("pIspDrv->init() fail \n");
        return ret;
    }

    m_pIspReg = (isp_reg_t*)m_pIspDrv->getRegAddr();

    m_pVirtIspDrvCQ0 = m_pIspDrv->getCQInstance(ISP_DRV_CQ0);
    MY_LOG_IF(m_bDebugEnable,"[m_pVirtIspDrvCQ0]:0x%08x \n", reinterpret_cast<MUINT32>(m_pVirtIspDrvCQ0));

    m_pVirtIspRegCQ0 = (isp_reg_t*)m_pVirtIspDrvCQ0->getRegAddr();
    MY_LOG_IF(m_bDebugEnable,"[m_pVirtIspRegCQ0]:0x%08x \n", reinterpret_cast<MUINT32>(m_pVirtIspRegCQ0));


    // AAO DMA buffer init

    // clear all
    m_rHwBufList[ECamDMA_AAO].clear();

    // Enqueue HW buffer
    for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
        m_rAAOBufInfo[i].useNoncache = 1;
        allocateBuf(m_rAAOBufInfo[i], AAO_BUFFER_SIZE);
        enqueueHwBuf(ECamDMA_AAO, m_rAAOBufInfo[i]);
    }

    DMAInit(camdma2type<ECamDMA_AAO>());

    // Enable AA stat
    AAStatEnable(MTRUE);

    debugPrint(ECamDMA_AAO);

    // AFO DMA buffer init

    // clear all
    m_rHwBufList[ECamDMA_AFO].clear();

    // Enqueue HW buffer
    for (MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++) {
        m_rAFOBufInfo[i].useNoncache = 1;
        allocateBuf(m_rAFOBufInfo[i], AFO_BUFFER_SIZE);
        //enqueueHwBuf(ECamDMA_AFO, m_rAFOBufInfo[i]);
    }

    DMAInit(camdma2type<ECamDMA_AFO>());

    // Enable AF stat
    AFStatEnable(MTRUE);

    //debugPrint(ECamDMA_AFO);


    android_atomic_inc(&m_Users);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
uninit()
{
    MRESULT ret = S_BUFMGR_OK;
    IMEM_BUF_INFO buf_info;

	MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

	Mutex::Autolock lock(m_Lock);

	// If no more users, return directly and do nothing.
	if (m_Users <= 0)
	{
		return S_BUFMGR_OK;
	}

	// More than one user, so decrease one User.
	android_atomic_dec(&m_Users);

	if (m_Users == 0) // There is no more User after decrease one User
	{
	    // Disable AA stat
        AAStatEnable(MFALSE);

        // AAO DMA buffer uninit
        DMAUninit(camdma2type<ECamDMA_AAO>());

        for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
            freeBuf(m_rAAOBufInfo[i]);
        }

	    // Disable AF stat
        AFStatEnable(MFALSE);

        // AFO DMA buffer uninit
        DMAUninit(camdma2type<ECamDMA_AFO>());

        for (MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++) {
            freeBuf(m_rAFOBufInfo[i]);
        }

        // IspDrv uninit
        if (m_pIspDrv) {
            ret = m_pIspDrv->uninit();
            if (FAILED(ret)) {
                MY_ERR("m_pIspDrv->uninit() fail \n");
                return ret;
        }
    }

        m_pIspReg = MNULL;
        m_pIspDrv = MNULL;
        m_pVirtIspDrvCQ0 = MNULL;
        m_pVirtIspRegCQ0 = MNULL;
        
    }
	else	// There are still some users.
	{
		MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
	}

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
debugPrint(MINT32 i4DmaChannel)
{
    BufInfoList_T::iterator it;

    for (it = m_rHwBufList[i4DmaChannel].begin(); it != m_rHwBufList[i4DmaChannel].end(); it++ ) {
        MY_LOG_IF(m_bDebugEnable,"m_rHwBufList[%d].virtAddr:[0x%x]/phyAddr:[0x%x] \n",i4DmaChannel,it->virtAddr,it->phyAddr);
    }

    return S_BUFMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
enqueueHwBuf(MINT32 i4DmaChannel, BufInfo_T& rBufInfo)
{
    if (i4DmaChannel >= ECamDMA_NUM) {
        MY_ERR("Illegal DMA channel = %d\n", i4DmaChannel);
        return E_BUFMGR_ILLEGAL_DMA_CHANNEL;
    }

    if (i4DmaChannel == ECamDMA_AAO)   {
        m_rHwBufList[i4DmaChannel].push_back(rBufInfo);
    }

    return S_BUFMGR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
dequeueHwBuf(MINT32 i4DmaChannel, BufInfo_T& rBufInfo)
{
    if (i4DmaChannel >= ECamDMA_NUM) {
        MY_ERR("Illegal DMA channel = %d\n", i4DmaChannel);
        return E_BUFMGR_ILLEGAL_DMA_CHANNEL;
    }

    if (i4DmaChannel == ECamDMA_AAO)   {
        if (m_rHwBufList[i4DmaChannel].size()) {
            rBufInfo = m_rHwBufList[i4DmaChannel].front();

            m_rHwBufList[i4DmaChannel].pop_front();
        }
    }
    else if (i4DmaChannel == ECamDMA_AFO)  {    
        rBufInfo = m_rAFOBufInfo[0];
    }
    
    return S_BUFMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
BufMgr::
getCurrHwBuf(MINT32 i4DmaChannel)
{
    if (i4DmaChannel >= ECamDMA_NUM) {
        MY_ERR("Illegal DMA channel = %d\n", i4DmaChannel);
        return E_BUFMGR_ILLEGAL_DMA_CHANNEL;
    }

    if (m_rHwBufList[i4DmaChannel].size() > 0) {
        return m_rHwBufList[i4DmaChannel].front().phyAddr;
    }
    else { // No free buffer
        MY_ERR("No free buffer\n");
        return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
BufMgr::
getNextHwBuf(MINT32 i4DmaChannel)
{
    BufInfoList_T::iterator it;

    if (i4DmaChannel >= ECamDMA_NUM) {
        MY_ERR("Illegal DMA channel = %d\n", i4DmaChannel);
        return E_BUFMGR_ILLEGAL_DMA_CHANNEL;
    }

    if (m_rHwBufList[i4DmaChannel].size() > 1) {
    	it = m_rHwBufList[i4DmaChannel].begin();
        it++;
        return it->phyAddr;
    }
    else { // No free buffer
       MY_ERR("No free buffer\n");
       return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return E_BUFMGR_MEMORY_ERROR;
    }

    if (m_pIMemDrv->mapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return E_BUFMGR_MEMORY_ERROR;
    }

    return S_BUFMGR_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
freeBuf(BufInfo_T &rBufInfo)
{
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return E_BUFMGR_MEMORY_ERROR;
    }

    if (m_pIMemDrv->freeVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return E_BUFMGR_MEMORY_ERROR;
    }

    return S_BUFMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAConfig(camdma2type<ECamDMA_AAO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    ISP_WRITE_BITS(m_pIspReg , CAM_AAO_OFST_ADDR, OFFSET_ADDR, AAO_OFFSET_ADDR); // CAM_AAO_OFST_ADDR
    ISP_WRITE_BITS(m_pIspReg , CAM_AAO_XSIZE, XSIZE, AAO_XSIZE); // CAM_AAO_XSIZE
    //ISP_WRITE_BITS(m_pIspReg , CAM_AAO_YSIZE, YSIZE, AAO_YSIZE); // CAM_AAO_YSIZE
    ISP_WRITE_BITS(m_pIspReg , CAM_AAO_STRIDE, BUS_SIZE, AAO_STRIDE_BUS_SIZE); // CAM_AAO_STRIDE

    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AAO_OFST_ADDR, OFFSET_ADDR) = %d\n", ISP_READ_BITS(m_pIspReg , CAM_AAO_OFST_ADDR, OFFSET_ADDR));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AAO_XSIZE, XSIZE) = %d\n", ISP_READ_BITS(m_pIspReg , CAM_AAO_XSIZE, XSIZE));
    //MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AAO_YSIZE, YSIZE) = %d\n", ISP_READ_BITS(m_pIspReg , CAM_AAO_YSIZE, YSIZE));
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AAO_STRIDE, BUS_SIZE) = %d\n", ISP_READ_BITS(m_pIspReg , CAM_AAO_STRIDE, BUS_SIZE));

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
updateDMABaseAddr(camdma2type<ECamDMA_AAO>, MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    if (!u4BaseAddr) {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }

    ISP_WRITE_BITS(m_pIspReg , CAM_AAO_BASE_ADDR, BASE_ADDR, u4BaseAddr); // CAM_AAO_BASE_ADDR
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AAO_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS(m_pIspReg , CAM_AAO_BASE_ADDR, BASE_ADDR));

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAInit(camdma2type<ECamDMA_AAO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    DMAConfig(camdma2type<ECamDMA_AAO>());
    updateDMABaseAddr(camdma2type<ECamDMA_AAO>(), getCurrHwBuf(ECamDMA_AAO));

    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_EN_SET, AAO_EN_SET, 1); // AAO DMA (double buffer)

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAUninit(camdma2type<ECamDMA_AAO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    // disable AAO
    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_EN_CLR, AAO_EN_CLR, 1); // AAO DMA (double buffer)

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
AAStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"AAStatEnable(%d)\n",En);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    if (En) { // Enable
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_SET, AA_EN_SET, 1); // AA statistics (double buffer)
    }
    else { // Disable
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_CLR, AA_EN_CLR, 1); // AA statistics (double buffer)
    }

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAConfig(camdma2type<ECamDMA_AFO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()-AF\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    ISP_WRITE_BITS(m_pIspReg , CAM_AFO_XSIZE, XSIZE, AFO_XSIZE); // CAM_AFO_XSIZE

    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AFO_XSIZE, XSIZE) = %d\n", ISP_READ_BITS(m_pIspReg , CAM_AFO_XSIZE, XSIZE));

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
updateDMABaseAddr(camdma2type<ECamDMA_AFO>, MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"%s()-AF\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    if (!u4BaseAddr) {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }

    ISP_WRITE_BITS(m_pIspReg , CAM_AFO_BASE_ADDR, BASE_ADDR, u4BaseAddr); // CAM_AFO_BASE_ADDR
    MY_LOG_IF(m_bDebugEnable,"ISP_READ_BITS(m_pIspReg , CAM_AFO_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS(m_pIspReg , CAM_AFO_BASE_ADDR, BASE_ADDR));

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAInit(camdma2type<ECamDMA_AFO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()-AF\n", __FUNCTION__);

    DMAConfig(camdma2type<ECamDMA_AFO>());
    updateDMABaseAddr(camdma2type<ECamDMA_AFO>(), m_rAFOBufInfo[0].phyAddr);

    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_EN_SET, ESFKO_EN_SET, 1);

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
DMAUninit(camdma2type<ECamDMA_AFO>)
{
    MY_LOG_IF(m_bDebugEnable,"%s()-AF\n", __FUNCTION__);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    // enable ESFKO done
    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_INT, ESFKO_DONE_EN, 1);

    // wait ESFKO done
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Clear = ISP_DRV_IRQ_CLEAR_WAIT;
    WaitIrq.Type = ISP_DRV_IRQ_TYPE_DMA;
    WaitIrq.Status = ISP_DRV_IRQ_DMA_INT_ESFKO_DONE_ST;
    WaitIrq.Timeout = 1000; // 1 sec

    //m_pIspDrv->waitIrq(WaitIrq);

    // disable ESFKO
    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_EN_CLR, ESFKO_EN_CLR, 1);

    // disable ESFKO done
    ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_DMA_INT, ESFKO_DONE_EN, 0);

    return S_ISPMGR_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
BufMgr::
AFStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"AFStatEnable(%d)\n",En);

    if (!m_pIspReg) {
        MY_ERR("m_pIspReg is NULL\n");
        return E_ISPMGR_NULL_POINTER;
    }

    if (En) { // Enable
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_SET, SGG_EN_SET, 1);
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_SET, AF_EN_SET, 1); // AF statistics (double buffer)
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_INT_EN, AF_DON_EN, 1);   
    }
    else { // Disable
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_CLR, SGG_EN_CLR, 1);    
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_EN1_CLR, AF_EN_CLR, 1); // AF statistics (double buffer)
        ISP_WRITE_ENABLE_BITS(m_pIspReg , CAM_CTL_INT_EN, AF_DON_EN, 0);        
    }

    return S_ISPMGR_OK;
}

