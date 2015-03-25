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

/**
 * @file ispdrv_mgr.h
 * @brief ISP driver manager
 */

#ifndef _ISPDRV_MGR_H_
#define _ISPDRV_MGR_H_

#include <mtkcam/drv/isp_drv.h>

namespace NS3A
{
    
/**  
 * @brief enum for ISP drive mode
 */
typedef enum
{
    ISPDRV_MODE_ISP = 1,      ///< ISP direct access
    ISPDRV_MODE_CQ0 = 2,      ///< command queue 0
    ISPDRV_MODE_CQ1_SYNC = 4, ///< command queue 1 sync
    ISPDRV_MODE_CQ2_SYNC = 8  ///< command queue 2 sync
} ISPDRV_MODE_T;

/**  
 * @brief ISP register info
 */
typedef struct
{
    MUINT32     addr; ///< address
    MUINT32     val;  ///< value
} ISPREG_INFO_T;

/**  
 * @brief ISP driver manager
 */
class IspDrvMgr
{
public:
    /**  
     * @brief error code
     */
    typedef enum MERROR_ENUM
    {
        MERR_OK         = 0,          ///< OK
        MERR_UNKNOWN    = 0x80000000, ///< Unknown error
        MERR_BAD_ISP_DRV,             ///< ISP driver error
        MERR_BAD_ISP_ADDR             ///< wrong ISP addess
    } MERROR_ENUM_T;

public:     ////    Interfaces.

    /**  
     * @brief get ISP register start address
     * @param [in] eIspDrvMode ISP drive mode 
     */ 
    virtual volatile void*  getIspReg(ISPDRV_MODE_T eIspDrvMode) const = 0;

    /**  
     * @brief read register
     * @param [in] eIspDrvMode ISP drive mode 
     * @param [in] pRegInfos ISP register info
     * @param [in] count register count
     */ 
    virtual MBOOL           readRegs(ISPDRV_MODE_T eIspDrvMode, ISPREG_INFO_T*const pRegInfos, MUINT32 const count) = 0;

     /**  
     * @brief write register
     * @param [in] eCamModule camera module
     * @param [in] eIspDrvMode ISP drive mode 
     * @param [in] pRegInfos ISP register info
     * @param [in] count register count
     */ 
    virtual MBOOL           writeRegs(CAM_MODULE_ENUM eCamModule, ISPDRV_MODE_T eIspDrvMode, ISPREG_INFO_T*const pRegInfos, MUINT32 const count) = 0;

    /**  
     * @brief init function
     */
    virtual MERROR_ENUM_T   init() = 0;

    /**  
     * @brief uninit function
     */
    virtual MERROR_ENUM_T   uninit() = 0;

public:     ////
    /**  
     * @brief get instance
     */
    static IspDrvMgr&   getInstance();

protected:
    virtual ~IspDrvMgr() {}
};


/*******************************************************************************
*
*******************************************************************************/

};//  NS3A

#endif // _ISPDRV_MGR_H_

