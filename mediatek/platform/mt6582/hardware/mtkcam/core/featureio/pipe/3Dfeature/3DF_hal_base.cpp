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
#define LOG_TAG "3DF_hal_base"
     
#include <stdlib.h>
#include <stdio.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include "mav_hal.h"
#include "pano3d_hal.h"
#include <mtkcam/featureio/3DF_hal_base.h>
     
/*******************************************************************************
*
********************************************************************************/
hal3DFBase* 
hal3DFBase::createInstance(Hal3DFObject_e eobject)
{
    if (eobject == HAL_MAV_OBJ_NORMAL) {
        return halMAV::getInstance();
    }
    if (eobject == HAL_PANO3D_OBJ_NORMAL) {   
    	  return halPANO3D::getInstance();
    }	
    else {
        return hal3DFTmp::getInstance();
    }

    return NULL;
}

MINT32 hal3DFBase::mHal3dfAddImg(MavPipeImageInfo* pParaIn)
{                      
	 return 0;           
}	                     
                       
MINT32 hal3DFBase::mHal3dfGetMavResult(void* pParaOut)
{                      
	 return 0;           
}	
                       
MINT32 hal3DFBase::mHal3dfMerge(MUINT32 *MavResult)
{                      
	 return 0;               
}	                     
                       
MINT32 hal3DFBase::mHal3dfDoMotion(void* InputData,MUINT32* MotionResult, MUINT32 u4SrcImgWidth, MUINT32 u4SrcImgHeight)
{                      
	 return 0;           
}	    	               
                       
MINT32 hal3DFBase::mHal3dfWarp(MavPipeImageInfo* pParaIn,MUINT32 *MavResult,MUINT8 ImgNum)
{                      
	 return 0;           
}	                                       

MINT32 hal3DFBase::mHal3dfCrop(MUINT32 *MavResult,MUINT8 ImgNum)
{                      
	 return 0;           
}	

MINT32 hal3DFBase::mHal3dfGetResult(MUINT32& MavResult, MUINT32& ClipWidth, MUINT32& ClipHeight)  
{
	 return 0;
}  

MINT32 hal3DFBase::mHal3dfStitch(MUINT32 *MavResult,MUINT8 ImgNum)
{                      
	 return 0;           
}	                                       

MINT32 hal3DFBase::mHal3dfGetStitchResult(void* Pano3dResult)  
{
	 return 0;
} 
  
MINT32 hal3DFBase::mHal3dfGetWokSize(int SrcWidth, int SrcHeight, MUINT32 &WorkingSize)
{
	 return 0;
}      

MINT32 hal3DFBase::mHal3dfSetWokBuff(void* WorkingBuff) 
{
	 return 0;
}  

/*******************************************************************************
*
********************************************************************************/
hal3DFBase*
hal3DFTmp::
getInstance()
{
    CAM_LOGD("[hal3DFTmp] getInstance \n");
    static hal3DFTmp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
hal3DFTmp::
destroyInstance() 
{
}

