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
/*
** $Log: mav_hal.h $
 *
*/
/**
 * @file mav_hal.h
 * @brief Header file for mav_hal.cpp.
 */

#ifndef _MAV_HAL_H_
#define _MAV_HAL_H_

#include <mtkcam/featureio/3DF_hal_base.h>
#include <mtkcam/algorithm/libmav/MTKMav.h>
#include <mtkcam/algorithm/libmotion/MTKMotion.h>
#include <mtkcam/algorithm/libwarp/MTKWarp.h>


class MTKMav;
class MTKMotion;
class MTKWarp;

/*******************************************************************************
*
********************************************************************************/
/**
 * @brief MAV Hal. Interface to MAV Drv.
 */
class halMAV: public hal3DFBase 
{
public:
    //
    static hal3DFBase* getInstance();
    virtual void destroyInstance();

    /**
     * @brief MAV Hal constructor.
     *
     * @note N/A.
     *
     * @return N/A.
     */
    halMAV(); 

    /**
     * @brief MAV Hal descontrustor.
     *
     * @note N/A.
     *
     * @return N/A.
     */
    virtual ~halMAV();

    /**
     * @brief Init MAV.
     *
     * @note N/A.
     *
     * @param [in] MavInitInData    Input data for MAV algorithm initialization.
     * @param [in] MotionInitInData Input data for Motion algorithm initialization.
     * @param [in] WarpInitInData   Input data for Warping algorithm initialization.
     * @param [in] Pano3DInitInData Input data for Pano3D algorithm initialization.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfInit(void* MavInitInData,void* MotionInitInData,void* WarpInitInData,void* Pano3DInitInData);

    /**
     * @brief Uninit MAV.
     *
     * @note N/A.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfUninit();
    
    /**
     * @brief MAV main function.
     *
     * @note N/A.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHalMavMain();
    
    /**
     * @brief Add input image for MAV.
     *
     * @note N/A.
     *
     * @param [in] pParaIn Input parameters.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfAddImg(MavPipeImageInfo* pParaIn);

    /**
     * @brief Get MAV result by feature control function.
     *
     * @note N/A.
     *
     * @param [out] pParaOut Output parameters.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfGetMavResult(void* pParaOut);  

    /**
     * @brief MAV merge image and return result
     *
     * @note N/A.
     *
     * @param [out] MavResult Output MAV result.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfMerge(MUINT32 *MavResult);

    /**
     * @brief Do motion calculation algorithm.
     *
     * @note N/A.
     *
     * @param [in] InputData Input image address for Motion Algorithm.
     * @param [out] MotionResult Output Motion Result.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfDoMotion(void* InputData, MUINT32* MotionResult, MUINT32 u4SrcImgWidth, MUINT32 u4SrcImgHeight);

    /**
     * @brief Do warp image.
     *
     * @note N/A.
     *
     * @param [in] pParaIn Input parameters (image address, size, crop offset).
     * @param [in] MavResult MAV result.
     * @param [in] ImgNum Number of warped images.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfWarp(MavPipeImageInfo* pParaIn,MUINT32 *MavResult,MUINT8 ImgNum);
    
    /**
     * @brief Do warp image using clipped size.
     *
     * @note N/A.
     *
     * @param [in] MavResult MAV result.
     * @param [in] ImgNum Number of warped images.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfCrop(MUINT32 *MavResult,MUINT8 ImgNum);
 
    /**
     * @brief Check warp image success or not.
     *
     * @note N/A.
     *
     * @param [out] MavResult A flag to indicate MAV result.
     * @param [out] ClipWidth Clip Width.
     * @param [out] ClipHeight Clip Height.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfGetResult(MUINT32& MavResult,MUINT32& ClipWidth, MUINT32& ClipHeight);  
    
    /**
     * @brief Get algorithm working buffer size.
     *
     * @note N/A.
     *
     * @param [in] SrcWidth Source image width.
     * @param [in] SrcHeight Source image height.
     * @param [out] WorkingSize Output working buffer size.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfGetWokSize(int SrcWidth, int SrcHeight, MUINT32 &WorkingSize); 	
    
    /**
     * @brief Set algorithm working buffer addr.
     *
     * @note N/A.
     *
     * @param [in] WorkingBuff Input working buffer addr.
     *
     * @return
     * - S_MAV_OK indicates success.
     * - E_MAV_ERR indicates failure.
     * - Other value is error code.
     */
    virtual MINT32 mHal3dfSetWokBuff(void* WorkingBuff);
        
protected:


private:
    MTKMav* m_pMTKMavObj;
    MTKMotion* m_pMTKMotionObj;
    MTKMotion* m_pMTKPanoMotionObj;
    MTKWarp* m_pMTKWarpObj;
    MUINT32* SrcImg;
    MAVMotionResultInfo MAVPreMotionResult;
    MUINT8 FrameCunt;
};

#endif

