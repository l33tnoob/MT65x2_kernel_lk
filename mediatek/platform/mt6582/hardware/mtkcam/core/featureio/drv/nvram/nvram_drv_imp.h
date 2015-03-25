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
** $Log: nvram_drv_imp.h $
 *
 *
*/

#ifndef _NVRAM_DRV_IMP_H_
#define _NVRAM_DRV_IMP_H_

#include <utils/threads.h>

using namespace android; 

/*******************************************************************************
*
********************************************************************************/

class NvramDrv : public NvramDrvBase
{
public:
    /////////////////////////////////////////////////////////////////////////
    //
    // getInstance () -
    //! \brief get instance
    //
    /////////////////////////////////////////////////////////////////////////
    static NvramDrvBase* getInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    // destroyInstance () -
    //! \brief destroy instance
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void destroyInstance();

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
	                      unsigned long a_u4NvramDataSize);

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
	                       unsigned long a_u4NvramDataSize);

private:
    /////////////////////////////////////////////////////////////////////////
    //
    // NvramDrv () -
    //! \brief constructor
    //
    /////////////////////////////////////////////////////////////////////////
    NvramDrv();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~NvramDrv () -
    //! \brief descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~NvramDrv();

    /////////////////////////////////////////////////////////////////////////
    //
    // checkDataVersion () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    int checkDataVersion(CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	                     void *a_pNvramData);

	/////////////////////////////////////////////////////////////////////////
    //
    // readNvramData () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    int readNvramData(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                      CAMERA_DATA_TYPE_ENUM a_eCameraDataType,
	                  void *a_pNvramData);

	/////////////////////////////////////////////////////////////////////////
    //
    // writeNvramData () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    int writeNvramData(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                       CAMERA_DATA_TYPE_ENUM a_eCameraDataType,
	                   void *a_pNvramData);

    /////////////////////////////////////////////////////////////////////////
    //
    // readDefaultData () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    int readDefaultData(unsigned long u4SensorID,
                        CAMERA_DATA_TYPE_ENUM a_eCameraDataType,
	                    void *a_pNvramData);

    /////////////////////////////////////////////////////////////////////////
    //
    // testNvramDrv () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    void testNvramDrv(CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
                      unsigned long u4SensorID,
                      CAMERA_DATA_TYPE_ENUM a_eCameraDataType,
                      unsigned long a_u4NvramDataSize,
                      unsigned char *a_pNvramData);

private: 
    mutable Mutex mLock; 
    

};

#endif  //  _NVRAM_DRV_IMP_H_

