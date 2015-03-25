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
** $Log: strobe_drv.h $
 *
 *
*/

#ifndef _STROBE_DRV_H_
#define _STROBE_DRV_H_

/*******************************************************************************
   * Strobe Command
   *
   ********************************************************************************/
typedef enum
{
    CMD_STROBE_SET_CAP_DELAY = 0,
    CMD_STROBE_GET_WDT_VALUE,
    CMD_STROBE_MAX
} CMD_STROBE_ENUM;



class StrobeDrv
{
protected:

    /*******************************************************************************
       * Functionality : mhal strobe base descontrustor
       *
       ********************************************************************************/
    virtual ~StrobeDrv() = 0;

public:

    /*******************************************************************************
       * Strobe Error Code
       *
       ********************************************************************************/
    typedef enum
    {
        STROBE_NO_ERROR = 0,                  ///< The function work successfully
        STROBE_UNKNOWN_ERROR = 0x80000000,    ///< Unknown error
        STROBE_FILE_ERR,
        STROBE_FILE_ERR2,


        STROBE_ERR_PARA_INVALID,
    } STROBE_ERROR_ENUM;

    /*******************************************************************************
       * Strobe Type
       *
       ********************************************************************************/
    typedef enum
    {
        FLASHLIGHT_NONE = 0,
        FLASHLIGHT_LED_ONOFF,       // LED always on/off
        FLASHLIGHT_LED_CONSTANT,    // CONSTANT type LED
        FLASHLIGHT_LED_PEAK,        // peak strobe type LED
        FLASHLIGHT_LED_TORCH,       // LED turn on when switch FLASH_ON
        FLASHLIGHT_XENON_SCR,       // SCR strobe type Xenon
        FLASHLIGHT_XENON_IGBT       // IGBT strobe type Xenon
    }   FLASHLIGHT_TYPE_ENUM;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    static StrobeDrv*   createInstance();


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual void        destroyInstance() = 0 ;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual FLASHLIGHT_TYPE_ENUM getFlashlightType() const = 0;


    /*******************************************************************************
       * Functionality : initialization
       *
       ********************************************************************************/
    virtual int init(unsigned long sensorDev)= 0;

    virtual int initTemp(unsigned long sensorDev)= 0;


    /*******************************************************************************
       * Functionality : uninitialization
       *
       ********************************************************************************/
    virtual int uninit()= 0;


    /*******************************************************************************
       * Functionality : send IOCTL command to kernel driver to turn on/off the strobe
       *
       ********************************************************************************/
    virtual int setFire(unsigned long a_fire) = 0;


    /*******************************************************************************
       * Functionality : send IOCTL command to kernel driver to set duty of strobe
       *
       ********************************************************************************/
    virtual int setLevel(unsigned long a_level) = 0;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setTimeus(unsigned long a_timeus) = 0;


    /*******************************************************************************
       * Functionality :
       *
       ********************************************************************************/
    virtual int setStartTimeus(unsigned long a_timeus) = 0;


    /*******************************************************************************
       * Functionality : send IOCTL command to kernel driver to set state of strobe
       *
       ********************************************************************************/
    virtual int setState(unsigned long a_state) = 0;


    /*******************************************************************************
       * Functionality : control strobe depend on strobe mode
       *
       ********************************************************************************/
    virtual int setFlashlightModeConf(unsigned long a_strobeMode) = 0 ;


    /*******************************************************************************
       * Functionality : control strobe depend on capture parameter of strobe
       *
       ********************************************************************************/
    virtual int setCaptureFlashlightConf(unsigned long a_strobeWidth) = 0;


    /*******************************************************************************
       * Author : Cotta
       * Functionality : commadn control
       *
       ********************************************************************************/
    virtual int sendCommand(unsigned int cmd, unsigned int pArg1, unsigned int *pArg2, unsigned int *pArg3) = 0;


	virtual int isOn(int* a_isOn) =0;
    virtual int setOnOff(int a_isOn) =0;
    virtual int setStep(int step)=0;
	virtual int setDuty(int duty)=0;
	virtual int getFlashReg(int shift, unsigned short* reg)=0;
	virtual int setFlashReg(int shift, unsigned short reg, unsigned short mask)=0;
	virtual int getCoolDownTime(int* ms)=0;

	virtual int lockSensedV()=0;
	virtual int unlockSensedV()=0;
	virtual int mapDutyStep(int peakI, int aveI, int* duty, int* step)=0;

	virtual int getVBat(int* vbat)=0;
	virtual int setTimeOutTime(int ms)=0;

	virtual int hasFlashHw()=0;


	virtual int setPreOn()=0;
	virtual int getPreOnTimeMs(int* ms)=0;

	virtual int setReg(int reg, int val)=0;
	virtual int getReg(int reg, int* val)=0;

	virtual int getDuty(int* duty)=0;
	virtual int getStep(int* step)=0;

	virtual int getPartId(int sensorDev)=0;



protected:

	//int m_preOnTime;

private:

};

#endif

