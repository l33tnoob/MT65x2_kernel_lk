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
#include <utils/Errors.h>
#include <utils/Log.h>
#include <fcntl.h>
#include "../inc/nvram_drv.h"
#include "nvram_drv_imp.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "camera_custom_AEPlinetable.h"
#include <aaa_types.h>
#include "flash_param.h"
#include "flash_tuning_custom.h"

#ifdef NVRAM_SUPPORT
#include "camera_custom_msdk.h"
#endif

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
#undef LOG_TAG
#define LOG_TAG "NvramDrv"

#define NVRAM_DRV_LOG(fmt, arg...)    ALOGD(LOG_TAG " "fmt, ##arg)
#define NVRAM_DRV_ERR(fmt, arg...)    ALOGE(LOG_TAG "Err: %5d: "fmt, __LINE__, ##arg)

#define INVALID_HANDLE_VALUE (-1)

/*******************************************************************************
*
********************************************************************************/
static unsigned long const g_u4NvramDataSize[CAMERA_DATA_TYPE_NUM] =
{
    sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    sizeof(NVRAM_CAMERA_3A_STRUCT),
    sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    sizeof(NVRAM_LENS_PARA_STRUCT),
    sizeof(AE_PLINETABLE_T),
    sizeof(NVRAM_CAMERA_STROBE_STRUCT),
    sizeof(CAMERA_TSF_TBL_STRUCT)
};
    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrvBase::createInstance()
{
    return NvramDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrv::getInstance()
{
    static NvramDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
NvramDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::NvramDrv()
    : NvramDrvBase()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::~NvramDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData,
	unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[readNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	if ((a_eSensorType > DUAL_CAMERA_MAIN_SECOND_SENSOR) ||
		(a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
		//(a_eNvramDataType > CAMERA_DATA_AE_PLINETABLE) ||
		(a_eNvramDataType >= CAMERA_DATA_TYPE_NUM) ||
		(a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
		(a_pNvramData == NULL) ||
		(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType]))
	{
		NVRAM_DRV_LOG("[readNvram] error: line=%d",__LINE__);
        return NVRAM_READ_PARAMETER_ERROR;
    }



    Mutex::Autolock lock(mLock);

    switch(a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
    case CAMERA_NVRAM_DATA_3A:
    case CAMERA_NVRAM_DATA_SHADING:
	case CAMERA_NVRAM_DATA_LENS:
	case CAMERA_NVRAM_DATA_STROBE:
		err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
		    NVRAM_DRV_ERR("readNvramData() error: ==> readDefaultData()\n");
            int para1;
		    if(a_eNvramDataType==CAMERA_NVRAM_DATA_STROBE)
		    	para1=a_eSensorType;
		    else
		    	para1=a_u4SensorID;
            err = readDefaultData(para1, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
		        NVRAM_DRV_ERR("readDefaultData() error:\n");
	        }
            break;
	    }

		if (checkDataVersion(a_eNvramDataType, a_pNvramData) != NVRAM_NO_ERROR) {
			int para1;
		    if(a_eNvramDataType==CAMERA_NVRAM_DATA_STROBE)
		    	para1=a_eSensorType;
		    else
		    	para1=a_u4SensorID;
			err = readDefaultData(para1, a_eNvramDataType, a_pNvramData);
			if (err != NVRAM_NO_ERROR) {
		        NVRAM_DRV_ERR("readDefaultData() error:\n");
	        }
		}
        break;
    case CAMERA_DATA_AE_PLINETABLE:
            err = readDefaultData(a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
		        NVRAM_DRV_ERR("readDefaultData() AE Pline table error:\n");
	     }
    	break;
    case CAMERA_DATA_TSF_TABLE:
        err = readDefaultData(a_u4SensorID, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
            NVRAM_DRV_ERR("readDefaultData() TSF table error:\n");
        }
        break;
    default:
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData,
	unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[writeNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	if ((a_eSensorType > DUAL_CAMERA_MAIN_SECOND_SENSOR) ||
		(a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
		(a_eNvramDataType > CAMERA_NVRAM_DATA_LENS && (a_eNvramDataType!=CAMERA_NVRAM_DATA_STROBE)) ||
		(a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
		(a_pNvramData == NULL) ||
		(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])) {
        return NVRAM_WRITE_PARAMETER_ERROR;
    }

    Mutex::Autolock lock(mLock);

    err = writeNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::checkDataVersion(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[checkDataVersion]\n");

    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP) { // ISP
        PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraNvramData = (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[ISP] NVRAM data version = %d; F/W data version = %d\n", pCameraNvramData->Version, NVRAM_CAMERA_PARA_FILE_VERSION);

		if (pCameraNvramData->Version != NVRAM_CAMERA_PARA_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
	else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A) { // 3A
		PNVRAM_CAMERA_3A_STRUCT p3ANvramData = (PNVRAM_CAMERA_3A_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[3A] NVRAM data version = %d; F/W data version = %d\n", p3ANvramData->u4Version, NVRAM_CAMERA_3A_FILE_VERSION);

		if (p3ANvramData->u4Version != NVRAM_CAMERA_3A_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
	else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING) { // Shading
		PNVRAM_CAMERA_SHADING_STRUCT pShadingNvramData = (PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Shading] NVRAM data version = %d; F/W data version = %d\n", pShadingNvramData->Shading.Version, NVRAM_CAMERA_SHADING_FILE_VERSION);

		if (pShadingNvramData->Shading.Version != NVRAM_CAMERA_SHADING_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS) { // Lens
		PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Lens] NVRAM data version = %d; F/W data version = %d\n", pLensNvramData->Version, NVRAM_CAMERA_LENS_FILE_VERSION);

		if (pLensNvramData->Version != NVRAM_CAMERA_LENS_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
		}
    }
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_STROBE) { // strobe
		PNVRAM_CAMERA_STROBE_STRUCT pStrobeNvramData = (PNVRAM_CAMERA_STROBE_STRUCT)a_pNvramData;
        NVRAM_DRV_LOG("[Strobe] NVRAM data version = %d; F/W data version = %d\n", pStrobeNvramData->u4Version, NVRAM_CAMERA_STROBE_FILE_VERSION);
		if (pStrobeNvramData->u4Version != NVRAM_CAMERA_STROBE_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
		}
    }
    else {
		NVRAM_DRV_ERR("checkDataVersion(): incorrect data type\n");
}

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvramData(
	CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
	F_ID rNvramFileID;
	int i4FileInfo;
	int i4RecSize;
    int i4RecNum;
//seanlin 121221 avoid camera has not inited>
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    if (!bCustomInit) {
        cameraCustomInit();
	    LensCustomInit();
		bCustomInit = 1;
	}
//[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
//seanlin 121221 avoid camera has not inited<
    NVRAM_DRV_LOG("[readNvramData] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
		break;
	case CAMERA_NVRAM_DATA_3A:
		i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
		break;
	case CAMERA_NVRAM_DATA_LENS:
		i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
		break;
	case CAMERA_NVRAM_DATA_STROBE:
		i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
		break;
	default :
	    NVRAM_DRV_ERR("readNvramData(): incorrect data type\n");
		return NVRAM_READ_PARAMETER_ERROR;
		break;
	}

#ifdef NVRAM_SUPPORT

	rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISREAD);
	if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
		NVRAM_DRV_ERR("readNvramData(): create NVRAM file fail\n");
		return NVRAM_CAMERA_FILE_ERROR;
	}

    if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
	}

    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
	}

	read(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

	NVM_CloseFileDesc(rNvramFileID);

#endif

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvramData(
	CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
	F_ID rNvramFileID;
	int i4FileInfo;
	int i4RecSize;
    int i4RecNum;

    NVRAM_DRV_LOG("[writeNvramData] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
		break;
	case CAMERA_NVRAM_DATA_3A:
		i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
		break;
	case CAMERA_NVRAM_DATA_LENS:
		i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
		break;
	case CAMERA_NVRAM_DATA_STROBE:
		i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
		break;
	default:
	    NVRAM_DRV_ERR("writeNvramData(): incorrect data type\n");
		return NVRAM_WRITE_PARAMETER_ERROR;
		break;
	}

#ifdef NVRAM_SUPPORT

    rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISWRITE);
	if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
	    NVRAM_DRV_ERR("writeNvramData(): create NVRAM file fail\n");
		return NVRAM_CAMERA_FILE_ERROR;
	}

	if (a_eSensorType == DUAL_CAMERA_MAIN_SECOND_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
	}

	if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize*2, SEEK_SET);
	}

    write(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

	NVM_CloseFileDesc(rNvramFileID);
#endif

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readDefaultData(
	unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{

//    static bool bCustomInit = 0; //[ALPS00424402] [CCT6589] Len shading page --> Save to NVRAM --> CCT reboot failed
    NVRAM_DRV_LOG("[readDefaultData] sensor ID = %ld; NVRAM data type = %d\n", a_u4SensorID, a_eNvramDataType);

#ifdef NVRAM_SUPPORT

    if (!bCustomInit) {
        cameraCustomInit();
	    LensCustomInit();
		bCustomInit = 1;
	}

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		GetCameraDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_3A:
		GetCameraDefaultPara(a_u4SensorID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL);
		break;
	case CAMERA_NVRAM_DATA_LENS:
		GetLensDefaultPara((PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
		{
			PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
			pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
		}
		break;
	case CAMERA_DATA_AE_PLINETABLE:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,(PAE_PLINETABLE_STRUCT)a_pNvramData);
		break;

	case CAMERA_NVRAM_DATA_STROBE:
		int sz;
		int ret;
									//a_eSensorType
		//ret = getDefaultStrobeNVRam(1, a_pNvramData, &sz);
		ret = cust_getDefaultStrobeNVRam_V2(a_u4SensorID, a_pNvramData, &sz);
		break;

    case CAMERA_DATA_TSF_TABLE:
        if (0 != GetCameraTsfDefaultTbl(a_u4SensorID, (PCAMERA_TSF_TBL_STRUCT)a_pNvramData))
        {
            return NVRAM_DEFAULT_DATA_READ_ERROR;
        }
        break;
	default:
		break;
	}

#endif

    return NVRAM_NO_ERROR;
}


int nvGetFlickerPara(MUINT32 SensorId, int SensorMode, void* buf)
{
	NVRAM_DRV_LOG("nvGetFlickerPara id=%d mode=%d", SensorId, SensorMode);
	int err;
	err = msdkGetFlickerPara(SensorId, SensorMode, buf);
	if(err!=0)
		NVRAM_DRV_LOG("nvGetFlickerPara error:=%d", err);
	return err;
}
