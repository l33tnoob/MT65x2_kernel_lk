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

#ifndef _FLICKER_HAL_BASE_H_
#define _FLICKER_HAL_BASE_H_

#define HAL_FLICKER_AUTO_50HZ    50
#define HAL_FLICKER_AUTO_60HZ    60
#define HAL_FLICKER_AUTO_OFF       0

/*******************************************************************************
*
********************************************************************************/
enum FLKCmd_T {
	 FLKCmd_Update,
	 FLKCmd_GetDimenion,
	 FLKCmd_SetFLKMode,
	 FLKCmd_SetWindow,
	 FLKCmd_FlkEnable,
	 FLKCmd_FlkDISable,
	 FLKCmd_Uninit,

};


class FlickerHalBase {
public:
    static FlickerHalBase* createInstance(MINT32 i4SensorDev);
    virtual MVOID      destroyInstance() = 0;
    
protected:
    virtual ~FlickerHalBase(){};

public:
    virtual MINT32  enableFlickerDetection(MBOOL bEnableFlicker) = 0;    
    virtual MINT32 analyzeFlickerFrequency(MINT32 i4LMVcnt, MINT32 *i4LMV_x, MINT32 *i4LMV_y, MINT64 *i4vAFstat=NULL) = 0;    
//    virtual MRESULT analyzeFlickerFrequency(MINT32 i4LMVcnt, MINT32 *i4LMV_x, MINT32 *i4LMV_y) = 0;    
    virtual MINT32 setWindowInfo() = 0;
    virtual MINT32 getFlickerStatus(MINT32 *a_flickerStatus) = 0;
	virtual MBOOL sendCommand(FLKCmd_T eCmd,void* pi4Arg)=0;

};

#endif

