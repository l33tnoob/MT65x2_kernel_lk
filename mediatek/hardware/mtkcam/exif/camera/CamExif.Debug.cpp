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
#define LOG_TAG "CamExifDebug"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/properties.h>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include "mtkcam/exif/IBaseCamExif.h"
#include "mtkcam/exif/CamExif.h"
//
#include <IBaseExif.h>
//
#include <debug_exif/aaa/dbg_aaa_param.h>
#include <debug_exif/aaa/dbg_isp_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
#include <debug_exif/eis/dbg_eis_param.h>
//

/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] "fmt, ::gettid(), __FUNCTION__, ##arg)


/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_CMN_INFO_S &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_CMN_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
void 
CamExif::
getCommonDebugInfo(DEBUG_CMN_INFO_S &a_rDbgCommonInfo)
{
    MINT32  ret = 0;
    MUINT32 u4ID = 0;

    // Cam Debug Info
    memset(&a_rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));

    // Cam Debug Version
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_VERSION, (MUINT32)CMN_DEBUG_TAG_VERSION);

    // Shot mode
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_SHOT_MODE, mCamDbgParam.u4ShotMode);

    // Camera mode: Normal, Engineer
    setDebugTag(a_rDbgCommonInfo, CMN_TAG_CAM_MODE, mCamDbgParam.u4CamMode);

    ret = sendCommand(CMD_REGISTER, DEBUG_CAM_CMN_MID, reinterpret_cast<MINT32>(&u4ID));
    if (ret != 0) {
        MY_LOGE("[getCommonDebugInfo] ERROR: CMD_REGISTER");
    }

    ret = sendCommand(CMD_SET_DBG_EXIF, u4ID, (MINT32) &a_rDbgCommonInfo, sizeof(DEBUG_CMN_INFO_S));
    if (ret != 0) {
        MY_LOGE("[getCommonDebugInfo] ERROR: ID_CMN");
    }

}
/*******************************************************************************
*
********************************************************************************/
void 
CamExif::
getDebugInfo(MUINT8** pDbgInfo, MUINT32 &rDbgSize, MINT32 const dbgModuleID)
{

    if (mi4DbgModuleType & dbgModuleID)
    {
        MUINT32 moduleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("[getDebugInfo] Get: ID(0x%04x), Size(%d), Addr(0x%x)\n", 
            dbgModuleID, mDbgInfo[moduleIndex].u4BufSize, (MUINT32)&mDbgInfo[moduleIndex].puDbgBuf);
        //
        *pDbgInfo = mDbgInfo[moduleIndex].puDbgBuf;
        rDbgSize = mDbgInfo[moduleIndex].u4BufSize;
    }
    else
    {
        MY_LOGW("[getDebugInfo] ID(0x%04x) did not exist.", dbgModuleID);
        //
        *pDbgInfo = NULL;
        rDbgSize = 0;        
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
appendDebugInfo(
    MINT32 const dbgModuleID,       //  [I] debug module ID
    MINT32 const dbgAppn,           //  [I] APPn
    MUINT8** const ppuAppnBuf,      //  [O] Pointer to APPn Buffer
    MUINT32* const pu4AppnSize      //  [O] Pointer to APPn Size
)
{
    MBOOL  ret = MTRUE;

    if (mi4DbgModuleType & dbgModuleID)
    {
        MUINT32 appnReturnSize = 0;
        MUINT8* pDbgModuleInfo = NULL;
        MUINT32 dbgModuleSize = 0;        
        MUINT32 dbgModuleIndex = mMapModuleID.valueFor(dbgModuleID);
        //
        MY_LOGI("[appendDebugInfo] Get: ID(0x%04x), Index(%d), Size(%d), Addr(0x%x)", 
            dbgModuleID, dbgModuleIndex, mDbgInfo[dbgModuleIndex].u4BufSize, (MUINT32)mDbgInfo[dbgModuleIndex].puDbgBuf);
        //
        pDbgModuleInfo = mDbgInfo[dbgModuleIndex].puDbgBuf; 
        dbgModuleSize = mDbgInfo[dbgModuleIndex].u4BufSize + 2; // Data(n bytes) + Data size(2 bytes)
        //
        if ( (dbgModuleSize > 2) && (dbgModuleSize < 64*1024) )
        {
            mpBaseExif->exifAppnMake(dbgAppn, *ppuAppnBuf, pDbgModuleInfo, dbgModuleSize, &appnReturnSize);
            //
            *ppuAppnBuf += appnReturnSize;
            *pu4AppnSize += appnReturnSize;
        }
        else
        {
            MY_LOGW("[appendDebugInfo] dbgModuleSize(%d)", dbgModuleSize);
            ret = MFALSE;
        }
    }
    else
    {
        MY_LOGW("[appendDebugInfo] ID(0x%04x) did not exist", dbgModuleID);
        ret = MFALSE;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
appendDebugExif(MUINT8* const puAppnBuf, MUINT32*const pu4AppnSize)
{
    MINT32  ret = MTRUE;
    MUINT8* pDst = puAppnBuf;
    MUINT32 u4AppnSize = 0;

    //// EIS debug info
    appendDebugInfo(ID_EIS, 4, &pDst, &u4AppnSize);

    //// CAM debug info: Common, MF, N3D, Sensor, Shading
    {
        // COMMON
        DEBUG_CMN_INFO_T rDbgCommonInfo;
        memset(&rDbgCommonInfo, 0, sizeof(DEBUG_CMN_INFO_T));
        getCommonDebugInfo(rDbgCommonInfo);
        //
        MUINT8* pDbgCMNInfo = NULL;
        MUINT32 dbgCMNSize = 0;
        getDebugInfo(&pDbgCMNInfo, dbgCMNSize, ID_CMN);

        // MF
        MUINT8* pDbgMFInfo = NULL;
        MUINT32 dbgMFSize = 0;
        getDebugInfo(&pDbgMFInfo, dbgMFSize, ID_MF);

        // N3D
        MUINT8* pDbgN3DInfo = NULL;
        MUINT32 dbgN3DSize = 0;
        getDebugInfo(&pDbgN3DInfo, dbgN3DSize, ID_N3D);

        // SENSOR
        MUINT8* pDbgSENSORInfo = NULL;
        MUINT32 dbgSENSORSize = 0;
        getDebugInfo(&pDbgSENSORInfo, dbgSENSORSize, ID_SENSOR);

        // SHADING
        MUINT8* pDbgSHADInfo = NULL;
        MUINT32 dbgSHADSize = 0;
        getDebugInfo(&pDbgSHADInfo, dbgSHADSize, ID_SHAD);


        //// CAM
        MUINT32 app5ReturnSize = 0;
        MUINT32 dbgCamHeaderSize = 0;
        MUINT8* pDbgCamInfo = NULL;
        MUINT32 dbgCamSize = 0;
        DEBUG_CAM_INFO_T rDbgCamInfo;
        memset(&rDbgCamInfo, 0, sizeof(DEBUG_CAM_INFO_T));
        dbgCamHeaderSize = sizeof(rDbgCamInfo.hdr);

        rDbgCamInfo.hdr.u4KeyID = DEBUG_CAM_KEYID;
        rDbgCamInfo.hdr.u4ModuleCount = MODULE_NUM(DEBUF_CAM_TOT_MODULE_NUM, DEBUF_CAM_TAG_MODULE_NUM);
        rDbgCamInfo.hdr.u4DbgCMNInfoOffset      = dbgCamHeaderSize;
        rDbgCamInfo.hdr.u4DbgMFInfoOffset       = rDbgCamInfo.hdr.u4DbgCMNInfoOffset + sizeof(DEBUG_CMN_INFO_T);
        rDbgCamInfo.hdr.u4DbgN3DInfoOffset      = rDbgCamInfo.hdr.u4DbgMFInfoOffset + sizeof(DEBUG_MF_INFO_T);
        rDbgCamInfo.hdr.u4DbgSENSORInfoOffset   = rDbgCamInfo.hdr.u4DbgN3DInfoOffset + sizeof(DEBUG_N3D_INFO_T);
        rDbgCamInfo.hdr.u4DbgSHADInfoOffset     = rDbgCamInfo.hdr.u4DbgSENSORInfoOffset + sizeof(DEBUG_SENSOR_INFO_T);

        if (pDbgCMNInfo != NULL) {
            memcpy(&rDbgCamInfo.rDbgCMNInfo, pDbgCMNInfo, dbgCMNSize);
        }
        if (pDbgMFInfo != NULL) {
            memcpy(&rDbgCamInfo.rDbgMFInfo, pDbgMFInfo, dbgMFSize);
        }
        if (pDbgN3DInfo != NULL) {
            memcpy(&rDbgCamInfo.rDbgN3DInfo, pDbgN3DInfo, dbgN3DSize);
        }
        if (pDbgSENSORInfo != NULL) {
            memcpy(&rDbgCamInfo.rDbgSENSORInfo, pDbgSENSORInfo, dbgSENSORSize);
        }
        if (pDbgSHADInfo != NULL) {
            memcpy(&rDbgCamInfo.rDbgSHADInfo, pDbgSHADInfo, dbgSHADSize);
        }

        pDbgCamInfo = (MUINT8*) &rDbgCamInfo;
        dbgCamSize  = sizeof(DEBUG_CAM_INFO_T) + 2; // Data(n bytes) + Data size(2 bytes)

        if ( pDbgCamInfo && dbgCamSize > 2)
        {
            mpBaseExif->exifAppnMake(5, pDst, pDbgCamInfo, dbgCamSize, &app5ReturnSize);
            //
            pDst += app5ReturnSize;
            u4AppnSize += app5ReturnSize;
        }
        else
        {
            MY_LOGE("[appendDebugExif] dbgCamSize(%d) < 0", dbgCamSize);
            goto lbExit;
        }
    }

    //// AAA debug info
    appendDebugInfo(ID_AAA, 6, &pDst, &u4AppnSize);

    //// ISP debug info
    appendDebugInfo(ID_ISP, 7, &pDst, &u4AppnSize);

    //// SHAD_ARRAY debug info
    appendDebugInfo(ID_SHAD_ARRAY, 8, &pDst, &u4AppnSize);

    if  (pu4AppnSize)
    {
        *pu4AppnSize = u4AppnSize;
    }

lbExit:
    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
CamExif::
sendCommand(MINT32 cmd, MINT32 parg1, MINT32 parg2, MINT32 parg3)
{
    MINT32 ret = 0;
    
    //1.Special command
    //MY_LOGI("[sendCommand] cmd: 0x%x \n", cmd);

    switch (cmd) 
    {
        case CMD_REGISTER:
        {
            MINT32 registerName = (MINT32) parg1;
            //
            switch (registerName) {
                case AAA_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_AAA;
                    break;
                case ISP_DEBUG_KEYID:
                    *(MINT32 *) parg2 = ID_ISP;
                    break;
                case DEBUG_EIS_MID:
                    *(MINT32 *) parg2 = ID_EIS;
                    break;
                case DEBUG_CAM_CMN_MID:
                    *(MINT32 *) parg2 = ID_CMN;
                    break;
                case DEBUG_CAM_MF_MID:
                    *(MINT32 *) parg2 = ID_MF;
                    break;
                case DEBUG_CAM_N3D_MID:
                    *(MINT32 *) parg2 = ID_N3D;
                    break;
                case DEBUG_CAM_SENSOR_MID:
                    *(MINT32 *) parg2 = ID_SENSOR;
                    break;
                case DEBUG_CAM_SHAD_MID:
                    *(MINT32 *) parg2 = ID_SHAD;
                    break;
                case DEBUG_CAM_SHAD_ARRAY_MID:
                    *(MINT32 *) parg2 = ID_SHAD_ARRAY;
                    break;
                default:
                    *(MINT32 *) parg2 = ID_ERROR;
                    MY_LOGE("[sendCommand] registerID: 0x%x \n", registerName);
                    ret = -1;
                    break;
            }
            break;
        }        
        case CMD_SET_DBG_EXIF:
        {
            MINT32 i4ID     = (MINT32) parg1;
            MUINT32 u4Addr  = (MUINT32) parg2;
            MUINT32 u4Size  = (MUINT32) parg3;
            //
            mi4DbgModuleType |= i4ID;
            //
            if (i4ID != ID_ERROR) {
                DbgInfo tmpDbgInfo;
                //
                tmpDbgInfo.u4BufSize    = u4Size;
                tmpDbgInfo.puDbgBuf     = new MUINT8[ u4Size ];
                memset(tmpDbgInfo.puDbgBuf, 0, u4Size);
                memcpy(tmpDbgInfo.puDbgBuf, (void*)u4Addr, u4Size);
                //
                MY_LOGI("[sendCommand] Set: ID(0x%04x), Size(%d), Addr(0x%x)", 
                    i4ID, tmpDbgInfo.u4BufSize, (MUINT32)&tmpDbgInfo.puDbgBuf);
                //
                mMapModuleID.add(i4ID, mDbgInfo.size());
                mDbgInfo.push_back(tmpDbgInfo);
            }
            else {
                MY_LOGE("[sendCommand] ID ERROR \n");
                ret = -1;
            }
            break;
        }
        default:
            ret = -1;
            break;
    }
    return ret;

}
