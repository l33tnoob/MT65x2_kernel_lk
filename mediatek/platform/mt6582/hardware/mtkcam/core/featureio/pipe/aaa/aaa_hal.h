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
* @file aaa_hal.h
* @brief Declarations of 3A Hal Class (public inherited from Hal3ABase)
*/
#ifndef _AAA_HAL_H_
#define _AAA_HAL_H_

//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/hal/aaa_hal_base.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/featureio/flicker_hal_base.h>
using namespace android;

namespace NS3A
{


/*******************************************************************************
*
********************************************************************************/
/**  
 * @brief 3A Hal Class, all inherited function interfaces are described in class Hal3ABase, please refer to aaa_hal_base.h
 */
class Hal3A : public Hal3ABase
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3A();
    virtual ~Hal3A();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static Hal3A* createInstance(MINT32 const i4SensorDevId);
    static Hal3A* getInstance();
    virtual MVOID destroyInstance();
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINT32 const i4Arg = 0);

    virtual inline MINT32 getErrorCode() const
    {
        return m_errorCode;
    }

    virtual inline MBOOL getParams(Param_T &rParam) const
    {
        rParam = m_rParam;
        return MTRUE;
    }

    virtual MBOOL setParams(Param_T const &rNewParam);
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam);

    virtual inline MBOOL isReadyToCapture() const
    {
        return m_bReadyToCapture;
    }

    virtual MBOOL autoFocus();
    virtual MBOOL cancelAutoFocus();
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;
    virtual MBOOL setDebugInfo(IBaseCamExif *pIBaseCamExif) const;
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;
    virtual MBOOL setIspProfile(EIspProfile_T const eIspProfile);
	/**  
	* @brief Enable AF thread
	* @param [in] a_bEnable set 1 to enable AF thread
	*/
    virtual MRESULT EnableAFThread(MINT32 a_bEnable);
    virtual MBOOL setCallbacks(I3ACallBack* cb);
    virtual MINT32 getCaptureParams(MINT8 index, MINT32 i4EVidx, CaptureParam_T &a_rCaptureInfo);
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo);
	virtual MVOID setFDEnable(MBOOL a_sFacesEn);
    virtual MBOOL setFDInfo(MVOID* a_sFaces);
    virtual MBOOL setOTInfo(MVOID* a_sOT);    
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo);
    virtual MINT32 isNeedFiringFlash();
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo);
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo);
    virtual MVOID endContinuousShotJobs();
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter);
	virtual MINT32 getFlashFrameNumBeforeCapFrame();
	virtual MVOID onFireCapFlashIfNeeded();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	 //
	/**  
	* @brief init 3A hal
	* @param [in] i4SensorDev sensor device; please refer to halSensorDev_e in sensor_hal.h
	*/
    MRESULT init(MINT32 i4SensorDev);
	
	/**  
	* @brief uninit 3A hal
	*/
    MRESULT uninit();

	/**  
	* @brief set error code
	* @param [in] errorCode error code; please refer to aaa_error_code.h
	*/
    inline MVOID setErrorCode(MRESULT errorCode)
    {
        m_errorCode = errorCode;
    }
	/**  
	* @brief reset ready-to-capture bool flag to be false
	*/
    inline MVOID resetReadyToCapture()
    {
        m_bReadyToCapture = MFALSE;
    }
	/**  
	* @brief enable ready-to-capture bool flag to be true
	*/
    inline MVOID notifyReadyToCapture()
    {
        m_bReadyToCapture = MTRUE;
    }
	/**  
	* @brief get current sensor device from 3A Hal instance
	*/
    inline MINT32 getSensorDev()
    {
        return m_i4SensorDev;
    }

    private:
	/**  
	* @brief AF thread execution function
	*/
    static MVOID* AFThreadFunc(void *arg);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile int  m_Users;
    mutable Mutex m_Lock;
    MRESULT       m_errorCode;
    Param_T       m_rParam;
    MBOOL         m_bReadyToCapture;
    MINT32        m_i4SensorDev;
    MBOOL         m_bDebugEnable;
	FlickerHalBase* mpFlickerHal;
	MBOOL		  m_bFaceDetectEnable;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE/AWB thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
	/**  
	* @brief execute 3A commands in current 3A state
	* @param [in] r3ACmd 3A commands; please refer to aaa_hal_base.h
	*/
    MBOOL           postCommand(ECmd_T const r3ACmd, MINT32 const i4Arg = 0);
	/**  
	* @brief create AE/AWB thread
	*/
    MVOID           createThread();
	/**  
	* @brief destroy AE/AWB thread
	*/
    MVOID           destroyThread();
	/**  
	* @brief change AE/AWB thread setting
	*/
    MVOID           changeThreadSetting();
	/**  
	* @brief AE/AWB thread execution function
	*/
    static  MVOID*  onThreadLoop(MVOID*);
	/**  
	* @brief add 3A commands in command queue
	* @param [in] r3ACmd 3A commands; please refer to aaa_hal_base.h
	*/
    MVOID           addCommandQ(ECmd_T const & r3ACmd);
	/**  
	* @brief clear all ECmd_Update commands in current command queue
	*/
    MVOID           clearCommandQ();
	/**  
	* @brief get 3A command from the head of 3A command queue
	* @param [in] rCmd 3A commands; please refer to aaa_hal_base.h
	*/
    MBOOL           getCommand(ECmd_T &rCmd);
	/**  
	* @brief non-busy wait of Vsync signal
	*/
    MVOID           waitVSirq();

private:
    pthread_t       mThread;
    List<ECmd_T>    mCmdQ;
    Mutex           mModuleMtx;
	Mutex           mAFMtx;
    Condition       mCmdQCond;
    IspDrv*         mpIspDrv;
    sem_t           mSem;
	sem_t           semAFProcEnd;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3A framework log control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
	static MINT32 sm_3ALogEnable;
};

#include <sys/time.h>
#include <cutils/xlog.h>
class AaaTimer {
public:
	inline MINT32 getUsTime()
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);

		return tv.tv_sec * 1000000 + tv.tv_usec;
	}

	AaaTimer(const char* info, MINT32 sensorDevId, MBOOL enable)
		: mInfo(info), mIdx(sensorDevId), mEnable(enable)
	{
		if (mEnable) mStartTime = getUsTime();
	}

	MVOID start(const char* info, MINT32 sensorDevId, MBOOL enable) //used by global/static variables
	{
		mInfo = info;
		mIdx = sensorDevId;
		mEnable = enable;
		if (mEnable) mStartTime = getUsTime();
	}

	MVOID printTime()
	{
		if (!mEnable) return;
		MINT32 endTime = getUsTime();
		XLOGD("[Function: %s, SensorDevId: %d] =====> time(ms): %f\n", mInfo, mIdx, ((double)(endTime - mStartTime)) / 1000);
	}

	~AaaTimer()
	{
	}

protected:
	const char* mInfo;
	MINT32 mStartTime;
	MINT32 mIdx;
	MBOOL mEnable;
};

//define log control
#define EN_3A_FLOW_LOG     1
#define EN_3A_TIMER_LOG    2

}; // namespace NS3A

#endif

