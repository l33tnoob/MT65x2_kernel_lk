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
#define LOG_TAG "sensor_cctop"
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "cct_main.h"
#include "cct_imp.h"
#include "sensor_drv.h"
#include <mtkcam/acdk/cct_feature.h>


/*******************************************************************************
*
********************************************************************************/
#define SENSOR_CCTOP_LOG(fmt, arg...)    XLOGD(fmt, ##arg)
#define SENSOR_CCTOP_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPQuerySensor(MVOID *a_pCCTSensorInfoOut, MUINT32 *pRealParaOutLen)
{

    MINT32 err = CCTIF_NO_ERROR;
    halSensorRawImageInfo_t rawImageInfo;
    ACDK_CCT_SENSOR_INFO_STRUCT *pSensorEngInfoOut = (ACDK_CCT_SENSOR_INFO_STRUCT*)a_pCCTSensorInfoOut;


    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                       SENSOR_CMD_SET_SENSOR_DEV,
                                       0,
                                       0,
                                       0);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOReadSensorReg() error]\n");
        return err;
    }

    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev, SENSOR_CMD_GET_SENSOR_ID, (MINT32)&pSensorEngInfoOut->DeviceId);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (MINT32)&pSensorEngInfoOut->Type);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev, SENSOR_CMD_GET_RAW_INFO, (MINT32)&rawImageInfo, 0);

    pSensorEngInfoOut->StartPixelBayerPtn = (ACDK_SENSOR_OUTPUT_DATA_FORMAT_ENUM)rawImageInfo.u1Order;
    pSensorEngInfoOut->GrabXOffset = 0;     // phase out, no multi-shot
    pSensorEngInfoOut->GrabYOffset = 0;     // phase out, no multi-shot

    printf("[CCTOPQuerySensor] Id = 0x%x\n", pSensorEngInfoOut->DeviceId);
    printf("[CCTOPQuerySensor] Type = %d\n", pSensorEngInfoOut->Type);
    printf("[CCTOPQuerySensor] StartPixelBayerPtn = %d\n", pSensorEngInfoOut->Type);
    printf("[CCTOPQuerySensor] GrabXOffset = %d\n", pSensorEngInfoOut->GrabXOffset);
    printf("[CCTOPQuerySensor] GrabYOffset = %d\n", pSensorEngInfoOut->GrabYOffset);


    *pRealParaOutLen = sizeof(ACDK_CCT_SENSOR_INFO_STRUCT);

    return err;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_CCT_SENSOR_RESOLUTION_STRUCT *pSensorResolution = (ACDK_CCT_SENSOR_RESOLUTION_STRUCT *)pCCTSensorResOut;


    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                        (MINT32)&pSensorResolution->SensorPreviewWidth,
                                        (MINT32)&pSensorResolution->SensorPreviewHeight);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                        (MINT32)&pSensorResolution->SensorFullWidth,
                                        (MINT32)&pSensorResolution->SensorFullHeight,
                                        0);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                        (MINT32)&pSensorResolution->SensorVideoWidth,
                                        (MINT32)&pSensorResolution->SensorVideoHeight,
                                        0);

    printf("[CCTOPGetSensorRes] PreviewWidth = %d, PreviewHeight = %d\n", pSensorResolution->SensorPreviewWidth, pSensorResolution->SensorPreviewHeight);
    printf("[CCTOPGetSensorRes] SensorFullWidth = %d, SensorFullHeight = %d\n", pSensorResolution->SensorFullWidth, pSensorResolution->SensorFullHeight);
    printf("[CCTOPGetSensorRes] SensorVideoWidth = %d, SensorVideoHeight = %d\n", pSensorResolution->SensorVideoWidth, pSensorResolution->SensorVideoHeight);

    *pRealParaOutLen = sizeof(ACDK_CCT_SENSOR_RESOLUTION_STRUCT);

    return err;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetLSCSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen)
{
    #define ISP_RAW_WIDTH_PADD      0
    #define ISP_RAW_HEIGHT_PADD     0

    MINT32 err = CCTIF_NO_ERROR;
    PACDK_SENSOR_RESOLUTION_INFO_STRUCT pSensorResolution = (PACDK_SENSOR_RESOLUTION_INFO_STRUCT)pCCTSensorResOut;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION]\n");

    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                        (MINT32)&pSensorResolution->SensorPreviewWidth,
                                        (MINT32)&pSensorResolution->SensorPreviewHeight);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                        (MINT32)&pSensorResolution->SensorFullWidth,
                                        (MINT32)&pSensorResolution->SensorFullHeight,
                                        0);
    err |= m_pSensorHalObj->sendCommand((halSensorDev_e)mSensorDev,
                                        SENSOR_CMD_GET_SENSOR_VIDEO_RANGE,
                                        (MINT32)&pSensorResolution->SensorVideoWidth,
                                        (MINT32)&pSensorResolution->SensorVideoHeight,
                                        0);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPGetLSCSensorRes() error]\n");
        return err;
    }

    pSensorResolution->SensorFullWidth += ISP_RAW_WIDTH_PADD;
    pSensorResolution->SensorFullHeight += ISP_RAW_HEIGHT_PADD;
    pSensorResolution->SensorPreviewWidth += ISP_RAW_WIDTH_PADD;
    pSensorResolution->SensorPreviewHeight += ISP_RAW_HEIGHT_PADD;

    *pRealParaOutLen = sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetEngSensorGroupCount(MUINT32 *pGoupCntOut, MUINT32 *pRealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_GET_GROUP_COUNT;
    MUINT32 sensorParaLen = sizeof(MUINT32);

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT]\n");

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, pGoupCntOut,  &sensorParaLen);

    *pRealParaOutLen = sizeof(MUINT32);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPGetEngSensorGroupCount() error]\n");
        return err;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetEngSensorGroupPara(MUINT32 groupIdx, MVOID *pGroupInfoOut, MUINT32 *pRealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_GET_GROUP_INFO;
    MUINT32 sensorParaLen = sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT);
    ACDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfoOut = (ACDK_SENSOR_GROUP_INFO_STRUCT *)pGroupInfoOut;
    pSensorGroupInfoOut->GroupIdx = groupIdx;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA]\n");

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, (MUINT32 *)pSensorGroupInfoOut, &sensorParaLen);
    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPGetEngSensorGroupPara() error]\n");
        return err;
    }

    *pRealParaOutLen = sizeof(ACDK_SENSOR_GROUP_INFO_STRUCT);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetEngSensorPara(MVOID *pSensorItemInfoIn, MVOID *pSensorItemInfoOut, MUINT32 *pRealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;

    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_GET_ITEM_INFO;
    ACDK_SENSOR_ITEM_INFO_STRUCT *pInfoIn = (ACDK_SENSOR_ITEM_INFO_STRUCT *)pSensorItemInfoIn;
    ACDK_SENSOR_ITEM_INFO_STRUCT *pInfoOut = (ACDK_SENSOR_ITEM_INFO_STRUCT *)pSensorItemInfoOut;
    MUINT32 sensorParaLen = sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT);
    pInfoOut->GroupIdx = pInfoIn->GroupIdx;
    pInfoOut->ItemIdx = pInfoIn->ItemIdx;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_ENG_SENSOR_PARA]\n");

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, (MUINT32 *)pInfoOut, &sensorParaLen);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPGetEngSensorPara() error]\n");
        return err;
    }

    *pRealParaOutLen = sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT);

    return err ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPSetEngSensorPara(MVOID *pSensorItemInfoIn)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_SET_ITEM_INFO;
    ACDK_SENSOR_ITEM_INFO_STRUCT *pInfoIn = (ACDK_SENSOR_ITEM_INFO_STRUCT *)pSensorItemInfoIn;
    MUINT32 sensorParaLen = sizeof(ACDK_SENSOR_ITEM_INFO_STRUCT);

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_SET_ENG_SENSOR_PARA]\n");

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, (MUINT32 *)pInfoIn, &sensorParaLen);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPSetEngSensorPara() error]\n");
        return err;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetSensorPregain(MVOID *pSensorItemInfoIn, MVOID *pSensorItemInfoOut, MUINT32 *pRealParaOutLen)
{
    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_SENSOR_PREGAIN]\n");

    return CCTOPGetEngSensorPara(pSensorItemInfoIn, pSensorItemInfoOut, pRealParaOutLen);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPSetSensorPregain(MVOID *pSensorItemInfoIn)
{
    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_SET_SENSOR_PREGAIN]\n");

    return CCTOPSetEngSensorPara(pSensorItemInfoIn);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOReadSensorReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_GET_REGISTER;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    MUINT32 Data[2], sensorParaLen;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_READ_SENSOR_REG]\n");

    Data[0] = pSensorRegInfoIn->RegAddr;
    Data[1] = 0;

    sensorParaLen = 2 * sizeof(MUINT32);

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, &Data[0], &sensorParaLen);

    pSensorRegInfoOut->RegData = Data[1];

    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOReadSensorReg() error]\n");
        return err;
    }

    SENSOR_CCTOP_LOG("[CCTOReadSensorReg] regAddr = %x, regData = %x\n", Data[0], Data[1]);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPWriteSensorReg(MVOID *puParaIn)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SENSOR_FEATURE_ENUM eSensorFeature = SENSOR_FEATURE_SET_REGISTER;
    PACDK_CCT_REG_RW_STRUCT pSensorRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    MUINT32 Data[2], sensorParaLen;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_WRITE_SENSOR_REG]\n");

    Data[0] = pSensorRegInfoIn->RegAddr;
    Data[1] = pSensorRegInfoIn->RegData;

    sensorParaLen = 2 * sizeof(MUINT32);

    //err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CCT_FEATURE_CONTROL, (MUINT32 *)&eSensorFeature, &Data[0], &sensorParaLen);

    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPWriteSensorReg() error]\n");
        return err;
    }

    SENSOR_CCTOP_LOG("[CCTOPWriteSensorReg] regAddr = %x, regData = %x\n", Data[0], Data[1]);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::CCTOPGetSensorInfo(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    ACDK_SCENARIO_ID_ENUM *pScenarioId = (ACDK_SCENARIO_ID_ENUM *)puParaIn;
    ACDK_SENSOR_INFO_STRUCT *pSensorInfo = (ACDK_SENSOR_INFO_STRUCT *)puParaOut;
    ACDK_SENSOR_CONFIG_STRUCT sensorConfigData;

    SENSOR_CCTOP_LOG("[ACDK_CCT_OP_GET_SENSOR_INFO]\n");

    //err = m_pSensorDrvObj->getInfo(*pScenarioId, pSensorInfo, &sensorConfigData);
    if (err != SENSOR_NO_ERROR) {
        SENSOR_CCTOP_ERR("[CCTOPGetSensorInfo() error]\n");
        return err;
    }

    *pu4RealParaOutLen = sizeof(ACDK_SENSOR_INFO_STRUCT);

    return err;
}


