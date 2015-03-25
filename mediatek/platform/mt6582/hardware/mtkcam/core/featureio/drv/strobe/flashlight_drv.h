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
/*
** $Log: flashlight_drv.h $
 *
 *
*/

#ifndef _FLASHLIGHT_DRV_H_
#define _FLASHLIGHT_DRV_H_

#include <utils/threads.h>
using namespace android;

/*******************************************************************************
*
********************************************************************************/
class FlashlightDrv : public StrobeDrv
{
private:

    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    FlashlightDrv();


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual ~FlashlightDrv();


public:

    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    static StrobeDrv* getInstance();


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual void destroyInstance();


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual FLASHLIGHT_TYPE_ENUM getFlashlightType() const;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int init(unsigned long sensorDev);

    virtual int initTemp(unsigned long sensorDev);


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int uninit();
    int uninitNoLock();


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setFire(unsigned long a_fire) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setLevel(unsigned long a_level) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setTimeus(unsigned long a_timeus) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setStartTimeus(unsigned long a_timeus) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setState(unsigned long a_state) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setFlashlightModeConf(unsigned long a_strobeMode) ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setCaptureFlashlightConf(unsigned long a_strobeWidth) ;


    /*******************************************************************************
       * Author : Cotta
       * Functionality : commadn control
       *
       ********************************************************************************/
    virtual int sendCommand(unsigned int cmd, unsigned int pArg1, unsigned int *pArg2, unsigned int *pArg3);

	virtual int isOn(int* a_isOn);
    virtual int setOnOff(int a_isOn);
    virtual int setStep(int step);
	virtual int setDuty(int duty);
	virtual int getFlashReg(int shift, unsigned short* reg);
	virtual int setFlashReg(int shift, unsigned short reg, unsigned short mask);
	virtual int getCoolDownTime(int* ms);


	virtual int lockSensedV();
	virtual int unlockSensedV();
	virtual int mapDutyStep(int peakI, int aveI, int* duty, int* step);

	virtual int getVBat(int* vbat);
	virtual int setTimeOutTime(int ms);


	virtual int hasFlashHw();
	virtual int setPreOn();
	virtual int getPreOnTimeMs(int* ms);

	virtual int setReg(int reg, int val);
	virtual int getReg(int reg, int* val);

	virtual int getDuty(int* duty);
	virtual int getStep(int* step);

	virtual int getPartId(int sensorDev);


protected:

private:

    /*******************************************************************************
       * Author : Cotta
       * Functionality : set value of sensor capture delay
       *
       ********************************************************************************/
    int setCaptureDelay(unsigned int value);


    /*******************************************************************************
       * Author : Cotta
       * Functionality : get value of strobe WDT. unit : ms
       *
       ********************************************************************************/
    int getStrobeWDTValue(unsigned int *pValue);

    int m_sensorDev;
    int m_fdSTROBE;
    int m_flashType;
    int m_strobeMode;
    volatile int mUsers;
    mutable Mutex mLock;

	int m_duty;
	int m_step;
    int m_isOn;
    int m_bTempInit;
    int m_preOnTime;
};

#endif

