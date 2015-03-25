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
#define LOG_TAG "CCTIF"

//
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <mtkcam/acdk/cct_feature.h>
#include "sensor_drv.h"
#include "nvram_drv.h"
#include <mtkcam/drv/isp_drv.h>
#include "cct_main.h"
#include "cct_imp.h"
//


/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    XLOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
CCTIF* CCTIF::createInstance()
{
    return  new CctImp;
}

void CctImp::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
CctImp::CctImp()
    : CCTIF()
    , m_pCctCtrl(NULL)
{
    MY_LOG("[CCTIF] E\n");

    IspDrv** pisp_drv = &m_cctctrl_prop.isp_prop.m_pispdrv;
    NvramDrvBase** pnvram_drv = &m_cctctrl_prop.nvram_prop.m_pnvramdrv;

    m_pSensorHalObj = SensorHal::createInstance();
    *pisp_drv = IspDrv::createInstance();
    *pnvram_drv = NvramDrvBase::createInstance();

}

/*******************************************************************************
*
********************************************************************************/
CctImp::~CctImp()
{
    MY_LOG("[~CCTIF] E\n");

    if (m_pSensorHalObj) {
        m_pSensorHalObj->destroyInstance();
        m_pSensorHalObj = NULL;
    }

    if  (m_pCctCtrl )
    {
        m_pCctCtrl->destroyInstance();
        m_pCctCtrl = NULL;
    }

    if(m_cctctrl_prop.isp_prop.m_pispdrv)
    {
        m_cctctrl_prop.isp_prop.m_pispdrv->uninit();
    }

}
/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::init(MINT32 sensorType)
{
    MUINT32 sen_id;
    MINT32 err = CCTIF_NO_ERROR;
    IspDrv* pisp_drv = m_cctctrl_prop.isp_prop.m_pispdrv;
    NvramDrvBase* pnvram_drv = m_cctctrl_prop.nvram_prop.m_pnvramdrv;
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>** pbuf_isp = &m_cctctrl_prop.nvram_prop.pbufIf_isp;
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>** pbuf_shd = &m_cctctrl_prop.nvram_prop.pbufIf_shd;
	NSNvram::BufIF<NVRAM_CAMERA_3A_STRUCT>** pbuf_3a = &m_cctctrl_prop.nvram_prop.pbufIf_3a; 
	NSNvram::BufIF<NVRAM_LENS_PARA_STRUCT>** pbuf_ln = &m_cctctrl_prop.nvram_prop.pbufIf_ln; 
	


    /*
    *   SENSOR INIT
    */
    mSensorDev = sensorType;
    if(!m_pSensorHalObj) {
        MY_ERR("[CctImp::init] m_pSensorHalObj != NULL before init()\n");
        return -1;
    }

    m_pSensorHalObj->init();
    err = m_pSensorHalObj->sendCommand((halSensorDev_e)sensorType,
                                       SENSOR_CMD_SET_SENSOR_DEV,
                                       0,
                                       0,
                                       0);
    if (err != SENSOR_NO_ERROR) {
        MY_ERR("[CctImp::init] set sensor dev error\n");
        return -1;
    }

    err = m_pSensorHalObj->sendCommand((halSensorDev_e)sensorType, SENSOR_CMD_GET_SENSOR_ID, (MINT32)&sen_id);
	if (err != SENSOR_NO_ERROR) {
        MY_ERR("[CctImp::init] get sensor id error\n");
        return -1;
    }

    m_cctctrl_prop.sen_prop.m_sen_id = sen_id;
    m_cctctrl_prop.sen_prop.m_sen_type = (CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType;
	MY_LOG("[CctImp::init] sen_id = %d\n", sen_id); 
	MY_LOG("[CctImp::init] sensorType = %d\n", sensorType); 

    /*
    *   ISP INIT
    */
    if(!pisp_drv) {
        MY_ERR("[CctImp::init] m_pispdrv == NULL before init()\n");
        return -1;
    }
    pisp_drv->init();


    /*
    *   NVRAM INIT
    */
    if(!pnvram_drv) {
        MY_ERR("[CctImp::init] pnvram_drv == NULL before init()\n");
        return -1;
    }
    *pbuf_isp = pnvram_drv->getBufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>();
    *pbuf_shd = pnvram_drv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();
	*pbuf_3a = pnvram_drv->getBufIF<NVRAM_CAMERA_3A_STRUCT>(); 
    aaaCCTFeatureControl(ACDK_CCT_AF_INIT, NULL, 0, NULL, 0, NULL); //init AF before NVRAM get buf of lens
	*pbuf_ln = pnvram_drv->getBufIF<NVRAM_LENS_PARA_STRUCT>(); 
	


    /*
    *   CCT CTRL INIT
    */
    m_pCctCtrl = CctCtrl::createInstance(&m_cctctrl_prop);
    if  (!m_pCctCtrl )
    {
        MY_ERR("[CctImp::init] m_pCctCtrl == NULL\n");
        return  -1;
    }

    return  CCTIF_NO_ERROR;

}


MINT32 CctImp::uninit()
{
	aaaCCTFeatureControl(ACDK_CCT_AF_UNINIT, NULL, 0, NULL, 0, NULL);
	m_pSensorHalObj->uninit();
    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
static
MUINT32
getSensorID(SensorDrv& rSensorDrv, CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum)
{
    switch  ( eSensorEnum )
    {
    case DUAL_CAMERA_MAIN_SENSOR:
        return  rSensorDrv.getMainSensorID();
    case DUAL_CAMERA_SUB_SENSOR:
        return  rSensorDrv.getSubSensorID();
    default:
        break;
    }
    return  -1;
}

CctCtrl*
CctCtrl::
createInstance(const cctctrl_prop_t *prop)
{

    CctCtrl* pCctCtrl = NULL;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM sen_type = prop->sen_prop.m_sen_type;
    MUINT32 sen_id = prop->sen_prop.m_sen_id;
    IspDrv* pisp_drv = prop->isp_prop.m_pispdrv;
    NvramDrvBase* pnvram_drv = prop->nvram_prop.m_pnvramdrv;
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>* pbufif_isp = prop->nvram_prop.pbufIf_isp;
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>* pbufif_shd = prop->nvram_prop.pbufIf_shd;
	NSNvram::BufIF<NVRAM_CAMERA_3A_STRUCT>* pbufif_3a = prop->nvram_prop.pbufIf_3a; 
	NSNvram::BufIF<NVRAM_LENS_PARA_STRUCT>* pbufif_ln = prop->nvram_prop.pbufIf_ln; 
    NVRAM_CAMERA_ISP_PARAM_STRUCT*  pbuf_isp = pbufif_isp->getRefBuf(sen_type, sen_id);
    NVRAM_CAMERA_SHADING_STRUCT*    pbuf_shd = pbufif_shd ->getRefBuf(sen_type, sen_id);
	NVRAM_CAMERA_3A_STRUCT*    pbuf_3a = pbufif_3a ->getRefBuf(sen_type, sen_id); 
	NVRAM_LENS_PARA_STRUCT*    pbuf_ln = pbufif_ln ->getRefBuf(sen_type, sen_id); 

    pCctCtrl = new CctCtrl(prop, pbuf_isp, pbuf_shd, pbuf_3a, pbuf_ln); 

    return  pCctCtrl;

}

void
CctCtrl::
destroyInstance()
{
    delete  this;
}

CctCtrl::
CctCtrl(
    const cctctrl_prop_t *prop,
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pBuf_ISP,
    NVRAM_CAMERA_SHADING_STRUCT*const   pBuf_SD,
    NVRAM_CAMERA_3A_STRUCT*const   pBuf_3A, 
    NVRAM_LENS_PARA_STRUCT*const   pBuf_LN 
)
    : m_eSensorEnum(prop->sen_prop.m_sen_type)
    , m_u4SensorID(prop->sen_prop.m_sen_id)
    //
    , m_pNvramDrv(prop->nvram_prop.m_pnvramdrv)
    , m_pIspDrv(prop->isp_prop.m_pispdrv)
    //
    , m_rBufIf_ISP(*prop->nvram_prop.pbufIf_isp)
    , m_rBufIf_SD (*prop->nvram_prop.pbufIf_shd)
    , m_rBufIf_3A (*prop->nvram_prop.pbufIf_3a) 
    , m_rBufIf_LN (*prop->nvram_prop.pbufIf_ln) 
    //
    //
    , m_rBuf_ISP(*pBuf_ISP)
    ////
    , m_rISPComm(m_rBuf_ISP.ISPComm)
    , m_rISPRegs(m_rBuf_ISP.ISPRegs)
    , m_rISPRegsIdx(m_rBuf_ISP.ISPRegs.Idx)
    , m_rISPPca (m_rBuf_ISP.ISPPca)
    //
    , m_fgEnabled_OB(MTRUE)
    , m_u4Backup_OB(0)
    //
    ////
    , m_rBuf_SD (*pBuf_SD)
    //
    , m_rBuf_3A (*pBuf_3A) 
    //
    , m_rBuf_LN (*pBuf_LN) 
    ////
{
}

CctCtrl::
~CctCtrl()
{
    if (m_pNvramDrv) {
        m_pNvramDrv->destroyInstance();
        m_pNvramDrv = NULL;
    }

    if (m_pIspDrv) {
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }

}

MINT32 CCTIF::setCCTSensorDev(MINT32 sensor_dev)
{
    if((sensor_dev < SENSOR_DEV_NONE) || (sensor_dev > SENSOR_DEV_MAIN_3D))
        return CCTIF_UNSUPPORT_SENSOR_TYPE;

    mSensorDev = sensor_dev;

    return CCTIF_NO_ERROR;
}

