/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBLFLASHLIGHT_H_
#define _MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBLFLASHLIGHT_H_
#if 1
//


/*******************************************************************************
 *
 ******************************************************************************/
#define CUSTOM_FLASHLIGHT   "flashlight"
FTABLE_DEFINITION(CUSTOM_FLASHLIGHT)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================
#if 1
    if  (1 == facing)
    {
        MY_LOGD("facing=1");
	    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
	        KEY_AS_(MtkCameraParameters::KEY_FLASH_MODE),
	        SCENE_AS_DEFAULT_SCENE(
	            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
	            ITEM_AS_VALUES_(
	            CameraParameters::FLASH_MODE_OFF,
	            )
	        ),
	    )
        return  true;
    }
#endif
    //==========================================================================
#if     defined(CUSTOM_FLASHLIGHT_TYPE_constant_flashlight)
    if  (NSSensorType::eSensorType_RAW==u4SensorType)
    {
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FLASH_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            CameraParameters::FLASH_MODE_ON,
            CameraParameters::FLASH_MODE_AUTO,
            CameraParameters::FLASH_MODE_RED_EYE,
            CameraParameters::FLASH_MODE_TORCH,
            )
        ),
        //......................................................................
        #if 1   //  SCENE HDR
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_HDR,
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            )
        )
        #endif
        //......................................................................
        //......................................................................
        #if 1   //  SCENE Fireworks
        SCENE_AS_(MtkCameraParameters::SCENE_MODE_FIREWORKS,
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            )
        )
        #endif
        //......................................................................
    )
    }
    else
    {
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FLASH_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            CameraParameters::FLASH_MODE_ON,
            //CameraParameters::FLASH_MODE_AUTO,
            CameraParameters::FLASH_MODE_RED_EYE,
            CameraParameters::FLASH_MODE_TORCH,
            )
        ),
    )
    }
    //==========================================================================
#elif   defined(CUSTOM_FLASHLIGHT_TYPE_peak_flashlight)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FLASH_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            CameraParameters::FLASH_MODE_ON,
            CameraParameters::FLASH_MODE_AUTO,
            CameraParameters::FLASH_MODE_RED_EYE,
            )
        ),
    )
    //==========================================================================
#elif   defined(CUSTOM_FLASHLIGHT_TYPE_torch_flashlight)
    #warning "[torch_flashlight]"
    //==========================================================================
#elif   defined(CUSTOM_FLASHLIGHT_TYPE_dummy_flashlight)
    #warning "[dummy_flashlight]"
    //==========================================================================
#else
    #warning "[else flashlight]"
#endif
    //==========================================================================
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()


/*******************************************************************************
 *
 ******************************************************************************/
#define CUSTOM_AFLAMP       "aflamp"
FTABLE_DEFINITION(CUSTOM_AFLAMP)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
    //==========================================================================
#if 1
    if  (1 == facing)
    {
        MY_LOGD("facing=1");
        return  true;
    }
#endif
    //==========================================================================
#if     defined(CUSTOM_FLASHLIGHT_TYPE_constant_flashlight)
    if  (NSSensorType::eSensorType_RAW==u4SensorType)
    {
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_AF_LAMP_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            CameraParameters::FLASH_MODE_ON,
            CameraParameters::FLASH_MODE_AUTO,
            )
        ),
    )
    }
    else
    {
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_AF_LAMP_MODE),
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FLASH_MODE_OFF),
            ITEM_AS_VALUES_(
            CameraParameters::FLASH_MODE_OFF,
            CameraParameters::FLASH_MODE_ON,
            //CameraParameters::FLASH_MODE_AUTO,
            )
        ),
    )
    }
    //==========================================================================
#else
#endif
    //==========================================================================
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()
/*******************************************************************************
 *
 ******************************************************************************/


/*******************************************************************************
 *
 ******************************************************************************/
#endif
#endif //_MTK_CUSTOM_PROJECT_HAL_IMGSENSOR_SRC_CONFIGFTBLFLASHLIGHT_H_

