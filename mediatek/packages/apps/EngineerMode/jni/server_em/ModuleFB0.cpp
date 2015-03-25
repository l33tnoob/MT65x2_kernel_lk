/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "EMFB0"

#include <binder/IBinder.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

#include <linux/mtkfb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/xlog.h>
#include "ModuleFB0.h"
#include "RPCClient.h"

using namespace android;
static int LCMPowerON();
static int LCMPowerOFF();

static int LCMGetTm()
{
	int power_state = 1;
    int fd = open("/dev/graphics/fb0", O_RDWR);

    ioctl(fd, MTKFB_GET_POWERSTATE, &power_state);

    close(fd);
    if(power_state == 1)
        return 1;
    else
        return 0;
}

static int LCMSetTm(int level)
{
    /*
    int power_state = 1;
    int fd = open("/dev/graphics/fb0", O_RDWR);

    bool on = (level==1? true:false);
    if(on)
        ioctl(fd, MTKFB_POWERON, &power_state);
    else
        ioctl(fd, MTKFB_POWEROFF, &power_state);

    close(fd);

    return level;
    */
    bool on = (level==1? true:false);
    if(on)
         LCMPowerON();
    else
         LCMPowerOFF();
    return level;
}

static int LCMGetMipiClock()
{
	int clock = 0;
    int fd = open("/dev/graphics/fb0", O_RDWR);

    ioctl(fd, MTKFB_GET_CURR_UPDATESPEED, &clock);

    close(fd);

    return  clock;
}


///the return value is real cycle applied
static int LCMSetMipiClock(int clock)
{
	int  applied_clock = 0;

    int fd = open("/dev/graphics/fb0", O_RDWR);

    ioctl(fd, MTKFB_CHANGE_UPDATESPEED, &clock);
    ioctl(fd, MTKFB_GET_CURR_UPDATESPEED, &applied_clock);

    close(fd);

    return applied_clock;
}


/// ///0 DBI, 1 DPI, 2 MIPI
static int LCMGetInterfaceType()
{
	int  type = 0;

    int fd = open("/dev/graphics/fb0", O_RDWR);

    ioctl(fd, MTKFB_GET_INTERFACE_TYPE, &type);

    close(fd);

    return type;

}




static int LCMPowerON() {
	int fd = open("dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		XLOGE("OPEN Failed");
		return -1;
	} else {
		// turn on lcm
		int ret = -1;
		if (ioctl(fd, MTKFB_POWERON, &ret) == -1) {
			XLOGE("ioctl Failed");
			close(fd);
			return -1;
		}
		close(fd);

		// unblank SurfaceFlinger
		sp<SurfaceComposerClient> client = new SurfaceComposerClient();
		sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
				ISurfaceComposer::eDisplayIdMain);
		SurfaceComposerClient::unblankDisplay(display);
		return 0;
	}

}
static int LCMPowerOFF() {
	int fd = open("dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		XLOGE("OPEN Failed");
		return -1;
	} else {
		// blank SurfaceFlinger
		sp<SurfaceComposerClient> client = new SurfaceComposerClient();
		sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
				ISurfaceComposer::eDisplayIdMain);
		SurfaceComposerClient::blankDisplay(display);

		// turn off lcm
		int ret = -1;
		if (ioctl(fd, MTKFB_POWEROFF, &ret) == -1) {
			XLOGE("ioctl Failed");
			close(fd);
			return -1;
		}
		close(fd);
		return 0;
	}

}

static int LCDWriteCycleGetMinVal() {
	int fd = open("dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		XLOGE("OPEN Failed");
		return -1;
	} else {
		int ret = -1;
		if (ioctl(fd, MTKFB_GET_DEFAULT_UPDATESPEED, &ret) == -1) {
			XLOGE("ioctl Failed");
			close(fd);
			return -1;
		}
		close(fd);
		return ret;
	}

}

static int LCDWriteCycleGetCurrentVal() {
	int fd = open("dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		XLOGE("OPEN Failed");
		return -1;
	} else {
		int ret = -1;
		if (ioctl(fd, MTKFB_GET_CURR_UPDATESPEED, &ret) == -1) {
			XLOGE("ioctl Failed");
			close(fd);
			return -1;
		}
		close(fd);
		return ret;
	}

}

static int LCDWriteCycleSetVal(int level) {
	int fd = open("dev/graphics/fb0", O_RDWR, 0);
	if (fd < 0) {
		XLOGE("OPEN Failed");
		return -1;
	} else {
		int ret = level;
		if (ioctl(fd, MTKFB_CHANGE_UPDATESPEED, &ret) == -1) {
			XLOGE("ioctl Failed");
			close(fd);
			return -1;
		}
		close(fd);
		return 0;
	}

}

int ModuleFB0::FB0_IOCTL(RPCClient* msgSender)
{
	int paraNum = msgSender->ReadInt();		
	
	int param[2] = {0}; //ioctlNum, param1
	
	int idx = 0;
	for(idx=0; idx<paraNum; idx++)
	{
		int T = msgSender->ReadInt();
		if(T != PARAM_TYPE_INT)
		{
			//error
			return -1;
		}
		int L = msgSender->ReadInt();
		int V = msgSender->ReadInt();
				
		param[idx] = V;				
	}
	
	if(param[0]>=FB0_MAX ||param[0]<=0)
	{
		msgSender->PostMsg((char*)"Parameter range overflow");	
		return -1;
	}
		
		int valReturn =	-1;
		switch(param[0])
		{
			case FB0_LCDWriteCycleGetMinVal:
				valReturn = LCDWriteCycleGetMinVal();
				break;
			case FB0_LCDWriteCycleGetCurrentVal:
				valReturn = LCDWriteCycleGetCurrentVal();
				break;
			case FB0_LCDWriteCycleSetVal:
				valReturn = LCDWriteCycleSetVal(param[1]);
				break;
			case FB0_LCMPowerON:
				valReturn = LCMPowerON();
				break;
			case FB0_LCMPowerOFF:
				valReturn = LCMPowerOFF();
				break;	

			case FB0_LCM_Get_Tm:
				valReturn = LCMGetTm();
				break;
			case FB0_LCM_Set_Tm:
				valReturn = LCMSetTm(param[1]);
				break;

			case FB0_LCM_Get_MIPI_clock:
				valReturn = LCMGetMipiClock();
				break;
			case FB0_LCM_Set_MIPI_clock:
				valReturn = LCMSetMipiClock(param[1]);
				break;
			case FB0_LCM_Get_Interface_Type:
				valReturn = LCMGetInterfaceType();
				break;
			default:
				break;
		}
		if(-1 == valReturn)
		{
			msgSender->PostMsg((char*)ERROR_CODE);
		}
		else
		{
			char b[64] = {0};
			snprintf(b,63, "%d", valReturn);		
			msgSender->PostMsg(b);
		}		
	
	return 0;
}


