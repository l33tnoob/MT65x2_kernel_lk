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
 *      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file  eis_drv_base.h
 
#ifndef _EIS_DRV_BASE_H_
#define _EIS_DRV_BASE_H_

#include <mtkcam/featureio/EIS_Type.h>

/**
  *@class EisDrvBase 
  *@brief EIS driver class used by EIS_Hal
*/
class EisDrvBase 
{
public:
    
    /**
         *@brief Create EisDrvBase object
         *@return
         *-EisDrvBase object
       */
    static EisDrvBase *createInstance();
    
    /**
         *@brief Destroy EisDrvBase object
       */
    virtual MVOID   destroyInstance() = 0;

protected:

    /**       
         *@brief EisDrvBase destructor
       */
    virtual ~EisDrvBase() {};
    
public:

    /**
         *@brief Enable EIS
         *@param[in] a_Enable : 1(MTRUE) - enable EIS , 0(MFALSE) - disable EIS
       */
    virtual MVOID enableEIS(MBOOL a_Enable) = 0; 
    
    /**
         *@brief Return EIS is enable or not
         *@return
         *-0 : disable, 1 : enable
       */
    virtual MBOOL isEISEnable() = 0;
   
    /**
         *@brief Select EIS input source
         *@details This setting is only valid at non N3D case. In N3D case, EIS connect to SGG ouput
         *@param[in] a_bEisSel : 0 (before CDRZ), 1 (after CDRZ)
       */
    virtual MVOID setEISSel(MBOOL a_EisSel) = 0;
    
    /**
         *@brief Select EIS process type        
         *@param[in] a_EisRawSel : 0 (EIS at CDP, YUV), 1 (EIS at RAW)
       */
    virtual MVOID setEISRawSel(MBOOL a_EisRawSel) = 0;
    
    /**
         *@brief Select EIS db load
         *@param[in] a_bEisSel : 0 (no change), 1 (from TG1 db load, raw_db_load1)
       */
    virtual MVOID setEIS_DB_SEL(MBOOL a_EisDB) = 0;
    
    /**
         *@brief Set down sample ratio of EIS module
         *@details horizontal ratio = vertical ratio. default is 1
         *@param[in] a_DS : 1,2,4 
       */
    virtual MVOID setEISFilterDS(MINT32 a_DS) = 0;
    
    /**
         *@brief Set number of RPs        
         *@param[in] a_RPNum_H : 1~16
         *@param[in] a_RPNum_V : 1~8
       */
    virtual MVOID setRPNum(MINT32 a_RPNum_H, MINT32 a_RPNum_V) = 0;    

    /**
         *@brief Set knee point and clipping point of AD knee function
         *@param[in] a_Knee : 0~15
         *@param[in] a_Clip : 0~15
       */
    virtual MVOID setADKneeClip(MINT32 a_Knee, MINT32 a_Clip) = 0;    

    /**
         *@brief Set number of macroblock 
         *@param[in] a_MBNum_H : 1~4
         *@param[in] a_MBNum_V : 1~8
       */
    virtual MVOID setMBNum(MINT32 a_MBNum_H, MINT32 a_MBNum_V) = 0;

    /**
         *@brief Set EIS horizontal filter (FIR & IIR) property
         *@param[in] a_Gain : horizontal gain. value = 0 (1x), 1 (2x), 3 (1/2x, valid when DS_IIR_V = 1)
         *@param[in] a_IIRGain : horizontal IIR gain. value = 3 (1/8x), 4(1/16x)
         *@param[in] a_FIRGain : horizontal FIR gain. value = 16, 32
       */
    virtual MVOID setFilter_H(MINT32 a_Gain, MINT32 a_IIRGain, MINT32 a_FIRGain) = 0;

    /**
         *@brief Set EIS verticall filter (IIR) property
         *@param[in] a_IIRGain : vertical IIR gain. value = 3 (1/8x), 4(1/16x)      
       */
    virtual MVOID setFilter_V(MINT32 a_IIRGain) = 0;
    
    /**
         *@brief Enable/Disable update RP info
         *@param[in] a_Enable : 1(enable) / 0(disable)
       */
    virtual MVOID setWRPEnable(MBOOL a_Enable) = 0;

    /**
         *@brief To indicate the first frame
         *@param[in] a_First : 1 (is the first frame) / 0 (is not the first frame)
       */
    virtual MVOID setFirstFrame(MBOOL a_First) = 0;

    /**
         *@brief Set LMV thresholds
         *@param[in] a_Center_X   : LMV X threshold for the central 4 macro blocks
         *@param[in] a_Surrond_X : LMV X threshold for the surrounding 12 macroblocks
         *@param[in] a_Center_Y   : LMV Y threshold for the central 4 macro blocks
         *@param[in] a_Surrond_Y : LMV Y threshold for the surrounding 12 macroblocks
       */
    virtual MVOID setLMV_TH(MINT32 a_Center_X, MINT32 a_Surrond_X, MINT32 a_Center_Y, MINT32 a_Surrond_Y) = 0;

    /**
         *@brief Set search window offset upper bound
         *@param[in] a_FLOffsetMax_H : 0~15
         *@param[in] a_FLOffsetMax_V : 0~65
       */
    virtual MVOID setFLOffsetMax(MINT32 a_FLOffsetMax_H, MINT32 a_FLOffsetMax_V) = 0;
   
    /**
         *@brief Set serch window offset
         *@param[in] a_FLOffset_H : -15~15
         *@param[in] a_FLOffset_V : -64~65
       */
    virtual MVOID setFLOffset(MINT32 a_FLOffset_H, MINT32 a_FLOffset_V) = 0;

    /**
         *@brief Set macroblock horizontal offset
         *@details Offset between the start of frame and the start of the 1st macroblock
         *@param[in] a_MBOffset_H     
       */
    virtual MVOID setMBOffset_H(MINT32 a_MBOffset_H) = 0;

    /**
         *@brief Set macroblock vertical offset
         *@details Offset between the start of frame and the start of the 1st macroblock
         *@param[in] a_MBOffset_V     
       */
    virtual MVOID setMBOffset_V(MINT32 a_MBOffset_V) = 0; 
   
    /**
         *@brief Set macroblock horizontal interval
         *@details Interval between the 1st RPs of two neighbor macroblock
         *@param[in] a_MBInterval_H     
       */
    virtual MVOID setMBInterval_H(MINT32 a_MBInterval_H) = 0;
  
    /**
         *@brief Set macroblock vertical interval
         *@details Interval between the 1st RPs of two neighbor macroblock
         *@param[in] a_MBInterval_V     
       */
    virtual MVOID setMBInterval_V(MINT32 a_MBInterval_V) = 0;

    /**
         *@brief Set EIS image control register         
         *@param[in] a_ImgWidth : width of image which is sent to EIS HW
         *@param[in] a_ImgHeight : height of image which is sent to EIS HW
       */
    virtual MVOID setEISImage(MINT32 a_ImgWidth, MINT32 a_ImgHeight) = 0;
    
    /**
         *@brief  Return search window offset limitation
         *@param[in,out] a_FLOffsetMax_H : max float offset in horizontal direction
         *@param[in,out] a_FLOffsetMax_V : max float offset in vertical direction
       */
    virtual MVOID getFLOffsetMax(MINT32 &a_FLOffsetMax_H, MINT32 &a_FLOffsetMax_V) = 0;    

    /**
         *@brief  Return down sample raio
         *@param[in,out] a_DS_H : horizontal down sample raio
         *@param[in,out] a_DS_V : vertical down sample raio
       */
    virtual MVOID getDSRatio(MINT32 &a_DS_H, MINT32 &a_DS_V) = 0;    

    /**
         *@brief  Return statistic of EIS HW
         *@param[in,out] a_pEIS_Stat : EIS_STATISTIC_T struct
       */
    virtual MVOID getStatistic(EIS_STATISTIC_T *a_pEIS_Stat) = 0;

    /**
         *@brief  Return register setting status
         *@return
         *-0 : success, 1 : fail
       */ 
    virtual MBOOL configStatus() = 0;

    /**
         *@brief  Reset register setting status        
       */ 
    virtual MVOID resetConfigStatus() = 0;

    /**
         *@brief  Dump EIS register setting
         *@details Debug usage
       */
    virtual MVOID dumpReg() = 0;
};

#endif // _EIS_DRV_BASE_H_

