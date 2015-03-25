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
/*
** $Log: cam_cal_drv.h $
 *
 *
*/

#ifndef _CAM_CAL_DRV_H_
#define _CAM_CAL_DRV_H_

//seaniln 121017 for 658x #include "camera_custom_nvram.h"
#include "camera_custom_cam_cal.h"
#define DRV_CAM_CAL_SUPPORT (0)

#include "camera_custom_nvram.h"
#ifndef USING_MTK_LDVT
    #define CAM_CAL_SUPPORT
#endif
/* //SeanLin@121116 No Use>
typedef enum {
    CAM_CAL_NO_ERROR  = 0,            ///< The function work successfully
    CAM_CAL_UNKNOWN   = 0x80000000,   ///< Unknown error
    CAM_CAL_READ_PARAMETER_ERROR,     ///< CAM_CAL read parameter error
    CAM_CAL_PARAMETER_INVALIDITY,    ///< CAM_CAL write parameter error
    CAM_CAL_CAMERA_FILE_ERROR,        ///< CAM_CAL camera file error
    CAM_CAL_MEMORY_ALLOCATE_ERROR,    ///< CAM_CAL memory alocate error
    CAM_CAL_DATA_READ_ERROR,          ///< CAM_CAL data read error
    CAM_CAL_DATA_WRITE_ERROR,         ///< CAM_CAL data write error
    CAM_CAL_DEFAULT_DATA_READ_ERROR,  ///< CAM_CAL data read error
    CAM_CAL_DATA_VERSION_ERROR,       ///< CAM_CAL data version error
    CAM_CAL_BAD_SENSOR_ENUM,          ///< CAM_CAL bad sensor enum
} CAM_CAL_ERROR_ENUM;
typedef enum {

    CAM_CAL_MASK_SHADING = 0x01,
    CAM_CAL_MASK_DEFECT = 0x02,
    CAM_CAL_MASK_PREGAIN = 0x04,
    CAM_CAL_MASK_ALL  = 0x07
} CAM_CAL_MASK_ENUM;


*/ //SeanLin@121116 No Use<



/*******************************************************************************
*
********************************************************************************/

class CamCalDrvBase {

public:
    /////////////////////////////////////////////////////////////////////////
    //
    // createInstance () -
    //! \brief create instance
    //
    /////////////////////////////////////////////////////////////////////////
    static CamCalDrvBase* createInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    // destroyInstance () -
    //! \brief destroy instance
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void destroyInstance() = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // readCamCal () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
	virtual int GetCamCalCalData(unsigned long i4SensorDevId,
                          /*unsigned long u4SensorID,*/
                          CAMERA_CAM_CAL_TYPE_ENUM a_eCamCalDataType,
	                      void *a_pCamCalData) = 0;

protected:
    /////////////////////////////////////////////////////////////////////////
    //
    // ~CamCalDrvBase () -
    //! \brief descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~CamCalDrvBase() {}

private:
	
};





#endif  

