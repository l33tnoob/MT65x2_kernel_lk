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

//! \file  eis_hal_base.h

#ifndef _EIS_HAL_BASE_H_
#define _EIS_HAL_BASE_H_

#include <mtkcam/featureio/EIS_Type.h>
#include <mtkcam/v1/hwscenario/IhwScenarioType.h>

//! EIS ratio
#define EIS_FACTOR 120

/**
  * @struct eisHal_config_s
  * @brief EIS config structure
  * @details The width and height are assigned to image size which is sent to EIS HW
*/
typedef struct eisHal_config_s
{
    MUINT32 imageWidth;
    MUINT32 imageHeight; 
} eisHal_config_t;

/**
  *@class EisHalBase 
  *@brief EIS class used by scenario
*/
class EisHalBase 
{
public:

    /**       
         *@brief EisHalBase constructor
       */
    EisHalBase() {}; 

    /**
         *@brief Create EisHalBase object
         *@param[in] userName : user name,i.e. who create EIS object
         *@return
         *-EisHalBase object
       */
    static EisHalBase* createInstance(char const *userName);

    /**
         *@brief Destroy EisHalBase object   
         *@param[in] userName : user name,i.e. who destroy EIS object
       */
    virtual MVOID destroyInstance(char const *userName) = 0;

    /**
         *@brief Config EIS
         *@details Call before pass1
         *@param[in] a_ehwMode : hardware scenario
         *@param[in] a_sEisConfig : size of image sent to EIS (TG out size or size after CDRZ)
         *
       */
    virtual MVOID  configEIS(NSHwScenario::EhwMode a_ehwMode, eisHal_config_t a_sEisConfig) = 0;    

    /**       
         *@brief Execute EIS
         *@return
         *-0 indicates success, otherwise indicates fail
       */
    virtual MINT32 doEIS() = 0;

    /**       
         *@brief Get EIS algorithm result (CMV)
         *@param[in,out] a_CMV_X_Int : EIS algo result of X-direction integer part
         *@param[in,out] a_CMV_X_Flt  : EIS algo result of X-direction float part
         *@param[in,out] a_CMV_Y_Int : EIS algo result of Y-direction integer part
         *@param[in,out] a_CMV_Y_Flt  : EIS algo result of Y-direction float part
         *@param[in,out] a_TarWidth    : EIS width crop size
         *@param[in,out] a_TarHeight   : EIS height crop size
       */
    virtual MVOID getEISResult(MUINT32 &a_CMV_X_Int, 
                                   MUINT32 &a_CMV_X_Flt,
                                   MUINT32 &a_CMV_Y_Int, 
                                   MUINT32 &a_CMV_Y_Flt,
                                   MUINT32 &a_TarWidth,
                                   MUINT32 &a_TarHeight) = 0;

    /**       
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[in,out] a_GMV_X : x-direction global motion vector between two frames
         *@param[in,out] a_GMV_Y  : y-direction global motion vector between two frames
       */
    virtual MVOID getEISGmv(MUINT32 &a_GMV_X, MUINT32 &a_GMV_Y) = 0;

    /**       
         *@brief Return EIS HW statistic result      
         *@param[in,out] a_pEIS_Stat : EIS_STATISTIC_STRUCT  
       */
    virtual MVOID getEISStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat) = 0;
    
protected:

    /**       
         *@brief EisHalBase destructor
       */
    virtual ~EisHalBase() {};    
};


/**
  *@class EisNone 
  *@brief Not support EIS
*/
class EisNone : public EisHalBase
{
public:
    
    EisNone() {}; 

    static EisNone* createInstance() {return new EisNone;};

    virtual MVOID  configEIS(NSHwScenario::EhwMode a_ehwMode, eisHal_config_t a_sEisFactor) {};    

    virtual MINT32 doEIS() {return 0;};

    virtual MVOID getEISResult(MUINT32 &a_CMV_X_Int, 
                                   MUINT32 &a_CMV_X_Flt,
                                   MUINT32 &a_CMV_Y_Int, 
                                   MUINT32 &a_CMV_Y_Flt,
                                   MUINT32 &a_TarWidth,
                                   MUINT32 &a_TarHeight) {a_CMV_X_Int = 0;a_CMV_X_Flt = 0;a_CMV_Y_Int = 0; a_CMV_Y_Flt = 0;a_TarWidth = 0; a_TarHeight = 0;};

    virtual MVOID getEISGmv(MUINT32 &a_GMV_X,MUINT32 &a_GMV_Y) {a_GMV_X = 0;a_GMV_Y = 0;};

    virtual MVOID getEISStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat) {};
    
private:
    virtual MVOID destroyInstance(char const *userName)  { delete this; };	
    virtual ~EisNone() {};    	
}; 


#endif

