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
** $Log: nvram_drv.h $
 *
 *
*/

#ifndef _NVRAM_DRV_H_
#define _NVRAM_DRV_H_
#include "kd_camera_feature.h"
#include "camera_custom_nvram.h"

#ifndef USING_MTK_LDVT
    #define NVRAM_SUPPORT
#endif

typedef enum {
    NVRAM_NO_ERROR  = 0,            ///< The function work successfully
    NVRAM_UNKNOWN   = 0x80000000,   ///< Unknown error
    NVRAM_READ_PARAMETER_ERROR,     ///< NVRAM read parameter error
    NVRAM_WRITE_PARAMETER_ERROR,    ///< NVRAM write parameter error
    NVRAM_CAMERA_FILE_ERROR,        ///< NVRAM camera file error
    NVRAM_MEMORY_ALLOCATE_ERROR,    ///< NVRAM memory alocate error
    NVRAM_DATA_READ_ERROR,          ///< NVRAM data read error
    NVRAM_DATA_WRITE_ERROR,         ///< NVRAM data write error
    NVRAM_DEFAULT_DATA_READ_ERROR,  ///< NVRAM data read error
    NVRAM_DATA_VERSION_ERROR,       ///< NVRAM data version error
    NVRAM_BAD_SENSOR_ENUM,          ///< NVRAM bad sensor enum
} NVRAM_ERROR_ENUM;


/*******************************************************************************
*
********************************************************************************/
namespace NSNvram
{

typedef CAMERA_DUAL_CAMERA_SENSOR_ENUM  ESensorEnum_T;
typedef CAMERA_DATA_TYPE_ENUM           ENvramDrvCmd_T;

template <class Buf_T>
struct BufIF
{
    virtual ~BufIF() {}

    /////////////////////////////////////////////////////////////////////
    //
    //! \brief Get a reference to the buffer.
    //
    /////////////////////////////////////////////////////////////////////
    virtual
    Buf_T*
    getRefBuf(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    ) = 0;

    /////////////////////////////////////////////////////////////////////
    //
    //! \brief Refresh the buffer by reading nvram (buffer <- nvram).
    //
    /////////////////////////////////////////////////////////////////////
    virtual
    MINT32
    refresh(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    ) = 0;

    /////////////////////////////////////////////////////////////////////
    //
    //! \brief Flush the buffer to nvram (buffer -> nvram).
    //
    /////////////////////////////////////////////////////////////////////
    virtual
    MINT32
    flush(
        ESensorEnum_T const eSensorEnum, MUINT32 const u4SensorID
    ) = 0;

};
};  //NSNvram


/*******************************************************************************
*
********************************************************************************/

class NvramDrvBase {

public:
    /////////////////////////////////////////////////////////////////////////
    //
    // createInstance () -
    //! \brief create instance
    //
    /////////////////////////////////////////////////////////////////////////
    static NvramDrvBase* createInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    // destroyInstance () -
    //! \brief destroy instance
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void destroyInstance() = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // readNvram () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int readNvram(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                          unsigned long u4SensorID,
                          CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	                      void *a_pNvramData,
	                      unsigned long a_u4NvramDataSize) = 0;

    /////////////////////////////////////////////////////////////////////////
    //
    // writeNvram () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual int writeNvram(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                           unsigned long u4SensorID,
                           CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	                       void *a_pNvramData,
	                       unsigned long a_u4NvramDataSize) = 0;

public:     ////    Buffer I/F
    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    template <class Buf_T> NSNvram::BufIF<Buf_T>* getBufIF() const;

protected:
    /////////////////////////////////////////////////////////////////////////
    //
    // ~NvramDrvBase () -
    //! \brief descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~NvramDrvBase() {}
};


#endif  //  _NVRAM_DRV_H_

