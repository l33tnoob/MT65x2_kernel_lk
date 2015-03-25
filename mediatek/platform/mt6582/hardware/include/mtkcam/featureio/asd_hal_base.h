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
** $Log: fd_hal_base.h $
 *
*/

#ifndef _ASD_HAL_BASE_H_
#define _ASD_HAL_BASE_H_

#include <mtkcam/featureio/pipe_types.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>

/*****************************************************************************
	ASD Defines and State Machine / Proc Mode  -->  Notice: drv-common-inc-MTKAsd.h
******************************************************************************/
#define MHAL_ASD_NUM_OF_SCENE  		(2)					// Will be fixed in final shipment
#define MHAL_ASD_IM_WIDTH    		(160)				// default max image width
#define MHAL_ASD_IM_HEIGHT   		(120)				// default max image height
#define MHAL_ASD_IMAGE_SIZE 			(ASD_IM_WIDTH*ASD_IM_HEIGHT*2)	// Image buffer size
#define MHAL_ASD_WORKING_BUFFER_SIZE (ASD_IMAGE_SIZE*9)	// working buffer size W*H*2*9
#define MHAL_ASD_DECIDER_LOG_BYTE_PER_CYCLE 256
#define MHAL_ASD_DECIDER_LOG_MAX_CYCLE 601
#define MHAL_ASD_DECIDER_BUFFER_SIZE (((ASD_DECIDER_LOG_BYTE_PER_CYCLE*ASD_DECIDER_LOG_MAX_CYCLE+ASD_IMAGE_SIZE+31)>>5)<<5)
#define MHAL_ASD_BUFFER_SIZE 	 (((ASD_IMAGE_SIZE+ASD_WORKING_BUFFER_SIZE+ASD_DECIDER_BUFFER_SIZE+31)>>5)<<5)

typedef enum HalASDObject_s {
    HAL_ASD_OBJ_NONE = 0,
    HAL_ASD_OBJ_AUTO,
    HAL_ASD_OBJ_UNKNOWN = 0xFF,
} HalASDObject_e;

typedef enum
{
    mhal_ASD_DECIDER_UI_AUTO=0,
    mhal_ASD_DECIDER_UI_N   =1,
    mhal_ASD_DECIDER_UI_B   =2,
    mhal_ASD_DECIDER_UI_P   =3,
    mhal_ASD_DECIDER_UI_L   =4,
    mhal_ASD_DECIDER_UI_NB  =5,
    mhal_ASD_DECIDER_UI_NP  =6,
    mhal_ASD_DECIDER_UI_NL  =7,
    mhal_ASD_DECIDER_UI_BP  =8,
    mhal_ASD_DECIDER_UI_BL  =9,
    mhal_ASD_DECIDER_UI_PL  =10,
    mhal_ASD_DECIDER_UI_NBP =11,
    mhal_ASD_DECIDER_UI_NBL =12,
    mhal_ASD_DECIDER_UI_NPL =13,
    mhal_ASD_DECIDER_UI_BPL =14,
    mhal_ASD_DECIDER_UI_NBPL=15,
    mhal_ASD_DECIDER_UI_SCENE_NUM
} mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM;

/*******************************************************************************
*
********************************************************************************/
class halASDBase {
public:
    //
    static halASDBase* createInstance(HalASDObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halASDBase() {};
protected:
    
public: 
    virtual MINT32 mHalAsdInit(void* AAAData,void* working_buffer,MUINT8 SensorType, MINT32 width, MINT32 height) {return 0;}
    //virtual MINT32 mHalAsdUninit() {return 0;}
    virtual MINT32 mHalAsdUnInit() {return 0;}
    
	  virtual MINT32 mHalAsdDoSceneDet(void* src, MUINT16 imgw, MUINT16 imgh) {return 0;}
	  virtual MINT32 mHalAsdDecider(void* AAAData,void* FDResult,mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene) {return 0;}  
};

class halASDTmp : public halASDBase {
public:
    //
    static halASDBase* getInstance();
    virtual void destroyInstance();
    //
    halASDTmp() {}; 
    virtual ~halASDTmp() {};
};

#endif

