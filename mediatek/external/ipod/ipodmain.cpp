/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/reboot.h>
#include <utils/Log.h>
#include <cust_kpd.h>
#include "ipodmain.h"

#define LIB_FULL_NAME "/system/lib/libipod.so"

//extern int (*ipod_chglights)[];

pthread_mutex_t mutex;

// add prefer power and backlight keys in below arrays.
int pwrkeys[] = {KPD_PWRKEY_MAP, KEY_ENDCALL};
int bklkeys[] = {KEY_HOME, KPD_PWRKEY_MAP, KEY_HP};

 /* Specific customization 
  * int params[] = {...}
  * p0: desired LIBIPOD version
  * p1: backlight skew timer (msec)
  * p2: charging animatoin total duration (msec)
  * p3: charging animation callback duration (msec) for re-drawing the framebuffer.
  * p4: bit mask for boot types without showing logos
  * p5: bit mask for logos not to show
  * 
  * <P1>
  * For thebacklight skew timer, it must be implemented with lights_chgon func.
  * for example, if the requirement is led on for 2 sec, and off for 3 sec,
  * then we can have 2 ways to implement it
  * 1. set skew in 3000 (ms), and when calling into lights_chgon(), 
  *  first turn on the led, and then sleep 2000ms, then turn off the led before return from lights_chgon().
  *    The flow will be:
  *    ...-> lights_chgon() -> turn on light -> sleep 2000ms -> turn off light ->...callback after 3000...->...
  *   
  * 2. set skew in 2000 (ms), and when calling into lights_chgon(), 
  *    first turn off the led, and then sleep 3000ms, then turn on the led before return from lights_chgon().
  *    The flow will be:
  *    ...-> lights_chgon() -> turn off light -> sleep 3000ms -> turn on light ->...callback after 2000...->...
  *
  * <p4> bit mask for boot types without showing logos
  * Currently support alarm, normal boot.You can OR them if needed. Set it to "0" if you don't need to config it.
  *    normal boot: 1<<0
  *    alarm boot:  1<<5
  *  
  * <p5> bit mask for logos not to show
  *    logo1: 1<<0
  *    logo2: 1<<1
  *    logo3: 1<<2
  *
  * <p6> backlight level
  *    range from 0~255
  *
  * <P7>
  *     power off time 
  *
  * <P8>
  *     Backlight on delay. 
  *     Sometimes the SurfaceFlinger may update the LCM behind the logo/charging anim. 
  *     This parameter is used to delay some time (in ms) to do the backlighting.
  * 
  * <P9> Shutdown voltage, unit: mV
  *      0: default configuration, 3400mV
  *      Others: customization voltage. Must between 3000 and 4000.
  *      Note, use default value will take battery capacity into consideration. 
  *      That is, IPO will do shutdown when voltage < 3400 and capacity = 0
  *      Other customization will bypass capacity check.
  * <P10> Power on voltage gate, unit: mV
  *       0: default configuration, 3450mV
  *       Others: customization voltage gate. Must between 3000 and 4000. 
  *       Note when IPO detects the battery voltage under this value, IPO won't let device power on.
  *       Normally, it should be <P9> + 50mV
  */

#define MASK_TYPE_NORMAL (1 << 0)
#define MASK_TYPE_ALARM  (1 << 5)

#define MASK_LOGO1 (1 << 0)
#define MASK_LOGO2 (1 << 1)
#define MASK_LOGO3 (1 << 2)

long params[PARAM_AMOUNTS] = { 
				  29,   /*p0, DO NOT CHNAGE IT */
				  0,    /*p1*/
 				  6000, /*p2*/
 				  200,  /*p3*/
 				  0,    /*p4*/
 				  0,    /*p5*/
 				  150,  /*p6, backlight level*/
 				  0,    /*p7, power off time*/
 				  100,  /*p8, backlight on delay*/
 				  0,    /*p9, for ipo shutdown voltage customization*/
 				  0,    /*p10, for ipo power on voltage gate*/
 				  0,    /*p11, tablet wifi only mode*/
                  0,    /*p12, charging mode*/
                  0,    /*p13, fast reboot mode*/
                  0,    /*p14, reset modem? sync with modem via sys.mdrst prop*/
                  0,    /*p15, ipo-h mode */
                  0,    /*p16, logging to kernel log*/
 				  };

void (*ipod_start)(int *, int, int *, int) = NULL;
void (*ipod_init_fb)(void (*pFunc1)(void), void (*pFunc2)(void), void (*pFunc3)(void), void (*pFunc4)(int, int), void(*pFunc5)(void)) = NULL;

//void (*ipod_chglight)(int(*pFunc[])()) = NULL;
void (*ipod_chgcontrol)(long *, int (*pFunc1)(void), int (*pFunc2)(void), int (*pFunc3)(void), int (*pFunc4)(int, int, int)) = NULL;

void (*ipod_trigger_chganim)(int) = NULL;

typedef void (*Func1)(int);
typedef void (*Func3)(int *, int, int *, int);
typedef void (*Func4)(long *, int (*pFunc1)(void), int (*pFunc2)(void), int (*pFunc3)(void), int (*pFunc4)(int, int, int));
typedef void (*Func5)(void (*pFunc1)(void), void (*pFunc2)(void), void (*pFunc3)(void), void (*pFunc4)(int, int), void(*pFunc5)(void));

void loadlib()
{
        void *handle, * func;

        handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
        if (handle == NULL) {
                SXLOGE("Can't load library: %s", dlerror());
                sleep(3); // delay to collect log before shutting down
                reboot(RB_POWER_OFF);                
        }

        func = dlsym(handle, "ipod_init_fb");
		ipod_init_fb = reinterpret_cast<Func5>(func);
		
        if (ipod_init_fb == NULL) {
                SXLOGE("ipod_init_fb error: %s", dlerror());
                dlclose(handle);
                sleep(3); // delay to collect log before shutting down
                reboot(RB_POWER_OFF);
        }

		func = dlsym(handle, "ipod_start");
        ipod_start = reinterpret_cast<Func3>(func);
        if (ipod_start == NULL) {
                SXLOGE("ipod_start error: %s", dlerror());
                dlclose(handle);
                sleep(3); // delay to collect log before shutting down
                reboot(RB_POWER_OFF);
        }

        func = dlsym(handle, "ipod_chgcontrol");
        ipod_chgcontrol = reinterpret_cast<Func4>(func);
        if (ipod_chgcontrol == NULL) {
                SXLOGE("ipod_chgcontrol error: %s", dlerror());
                dlclose(handle);
                sleep(3); // delay to collect log before shutting down
                reboot(RB_POWER_OFF);
        }

		func = dlsym(handle, "ipod_trigger_chganim");
		ipod_trigger_chganim = reinterpret_cast<Func1>(func);
        if (ipod_trigger_chganim == NULL) {
                SXLOGE("ipod_trigger_chganim error: %s", dlerror());
                dlclose(handle);
                sleep(3); // delay to collect log before shutting down
                reboot(RB_POWER_OFF);
        }
        SXLOGI("loadlib success!");
}

extern void checkIPOHMode(void);
int main(int argc, char *argv[]) 
{    
	pthread_mutex_init(&mutex, NULL);
	
	loadlib();


    updateFastRebootMode();

#ifdef MTK_KERNEL_POWER_OFF_CHARGING
    updateChargingMode();
#if defined(MTK_IPO_POWERPATH_SUPPORT) || defined(MTK_BATLOWV_NO_PANEL_ON_EARLY)  
	updateLogoBacklightVoltageMode(params);
#endif
#endif

    // not KPOC mode
	if(params[PARAM_CHARGING_MODE] != 1){
        // IPO-H mode
        checkIPOHMode();
#ifdef MTK_TB_WIFI_ONLY
	    updateTbWifiOnlyMode();
#else
	    radiooff_check();
#endif
        finish_shutdown();
    }

    params[PARAM_RESET_MODEM] = MTK_RESET_MODEM;
    SXLOGI("PARAM_RESET_MODEM: %d", params[PARAM_RESET_MODEM]);
    //set IPO backlight on delay 
    setIPOBklOnDelay();

	/* init fb sub-system*/
	ipod_init_fb(bootlogo_init, bootlogo_deinit, 
		 bootlogo_show_boot, bootlogo_show_charging, bootlogo_show_kernel);

	/* init specific config, charging anim, lights func */
	ipod_chgcontrol(params, lights_chgon, lights_chgfull, lights_chgexit, status_cb);
	
	/*input the customization key*/
	ipod_start(pwrkeys, sizeof(pwrkeys)/sizeof(int), bklkeys, sizeof(bklkeys)/sizeof(int)); 
	
	return 0;
}
