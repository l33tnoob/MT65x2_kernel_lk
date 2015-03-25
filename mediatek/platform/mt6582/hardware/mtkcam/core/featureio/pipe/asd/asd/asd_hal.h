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
** $Log: asd_hal.h $
 *
*/ 

#ifndef _ASD_HAL_H_
#define _ASD_HAL_H_

#include <mtkcam/featureio/asd_hal_base.h>
#include <mtkcam/algorithm/libasd/MTKAsd.h>
#include "asd_aaa_param.h"

class MTKAsd;

/*******************************************************************************
*
********************************************************************************/
class halASD: public halASDBase 
{
public:
    //
    static halASDBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halASD(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halASD();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAUTORAMAInit () -
    //! \brief init autorama
    //
    /////////////////////////////////////////////////////////////////////////     
    virtual MINT32 mHalAsdInit(void* AAAData,void* working_buffer,MUINT8 SensorType, MINT32 width, MINT32 height);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAsdUnInit () -
    //! \brief uninit autorama
    //
    /////////////////////////////////////////////////////////////////////////      
    virtual MINT32 mHalAsdUnInit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAsdDoSceneDet () -
    //! \brief Do Scene Det
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalAsdDoSceneDet(void* src, MUINT16 imgw, MUINT16 imgh);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalAsdDoSceneDet () -
    //! \brief Asd Decider
    //
    /////////////////////////////////////////////////////////////////////////      
    virtual MINT32 mHalAsdDecider(void* AAAData,void* FDResult,mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene);
    
    
protected:


private:
    MTKAsd* m_pMTKAsdObj;   
    ASD_INIT_INFO					          gMyAsdInitInfo;
    ASD_SCD_SET_ENV_INFO_STRUCT		  gMyAsdEnvInfo;					
    ASD_DECIDER_REF_STRUCT			    gMyDeciderEnvInfo;
    ASD_DECIDER_TUNING_PARA_STRUCT	gMyDeciderTuningInfo;   // tuning parameters       
};
 
#endif

