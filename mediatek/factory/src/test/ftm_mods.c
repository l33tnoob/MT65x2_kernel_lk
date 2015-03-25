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


#include <stdio.h>
#include "common.h"
#include "ftm.h"

extern int mcard_init(void);
extern int battery_init(void);
extern int gsensor_init(void);
extern int gs_cali_init(void);
extern int msensor_init(void);
extern int flash_init(void);
extern int clear_flash_init(void);
extern int camera_main_init(void);
extern int camera_main2_init(void);
extern int camera_sub_init(void);
extern int strobe_init(void);
extern int audio_init(void);
extern int spk_init(void);
extern int keys_init(void);
extern int lcd_init(void);
extern int lcm_init(void);
extern int headset_init(void);
extern int bt_init(void);
extern int rtc_init(void);
extern int touch_init(void);
extern int touch_auto_init(void);
extern int gps_init(void);
extern int idle_init(void);
extern int led_init(void);
extern int vibrator_init(void);
extern int wifi_init(void);
extern int jogball_init(void);
extern int ofn_init(void);
extern int fm_init();
extern int fmtx_init();
extern int MATV_init(void);
extern int alsps_init(void);
extern int tvout_init(void);
extern int gyro_init(void);
extern int gyro_cali_init(void);
extern int sim_init(void);
extern int sigtest_init(void);
extern int barometer_init(void);
extern int otg_init(void);
extern int cmmb_init(void);
extern int usb_init(void);
extern int nfc_init(void);
extern int emmc_init(void);
extern int clear_emmc_init(void);
extern int emi_init(void);
extern int hdmi_init(void);
extern int audio_debug_init(void);
extern int rf_test_init(void);
//extern int headset_debug_init(void);
extern int hotknot_init(void);

ftm_init_fn ftm_init_debug_funcs[] = {
/*
#ifdef FEATURE_FTM_HEADSET
    headset_debug_init,
#endif	
*/
#ifdef FEATURE_FTM_AUDIO
    audio_debug_init,
#endif

	NULL,

};

ftm_init_fn ftm_init_funcs[] = {
#ifdef FEATURE_FTM_MEMCARD
    mcard_init,
#endif

#ifdef FEATURE_FTM_BATTERY
    battery_init,
#endif

#ifdef CUSTOM_KERNEL_ACCELEROMETER
    gsensor_init,
    gs_cali_init,
#endif

#ifdef FEATURE_FTM_IDLE
	idle_init,
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    msensor_init,
#endif

#ifdef FEATURE_FTM_FLASH
    flash_init,
#endif

#ifdef FEATURE_FTM_CLEARFLASH
    clear_flash_init,
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
    camera_main_init,
#endif 

#ifdef FEATURE_FTM_MAIN2_CAMERA
    camera_main2_init,
#endif 


#ifdef FEATURE_FTM_SUB_CAMERA
    camera_sub_init, 
#endif 


#ifdef FEATURE_FTM_STROBE
		strobe_init,
#endif

#ifdef FEATURE_FTM_AUDIO
    audio_init,
#endif

#ifdef FEATURE_FTM_SPK_OC
    spk_init,
#endif

#ifdef FEATURE_FTM_KEYS
    keys_init,
#endif

#ifdef FEATURE_FTM_LCD
    lcd_init,
#endif

#ifdef FEATURE_FTM_LCM
    lcm_init,
#endif

#ifdef FEATURE_FTM_HEADSET
    headset_init,
#endif

#ifdef FEATURE_FTM_USB
    usb_init,
#endif
#ifdef FEATURE_FTM_OTG
    otg_init,
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    bt_init,
#endif
#endif

#ifdef FEATURE_FTM_RTC
    rtc_init,
#endif

#ifdef FEATURE_FTM_TOUCH
    touch_init,
    touch_auto_init,
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
    gps_init,
#endif
#endif


#ifdef MTK_NFC_SUPPORT
    nfc_init,
#endif


#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
    fmtx_init,
#endif
#endif

#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    fm_init,
#endif
#endif
#endif

#ifdef FEATURE_FTM_LED
    led_init,
#endif

#ifdef FEATURE_FTM_SIM
    sim_init,
#endif

#ifdef FEATURE_FTM_VIBRATOR
    vibrator_init,
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
	wifi_init,
#endif
#endif

#ifdef FEATURE_FTM_JOGBALL
    jogball_init,
#endif    

#ifdef FEATURE_FTM_OFN
    ofn_init,
#endif    

#ifdef FEATURE_FTM_MATV
    MATV_init,
#endif
#ifdef CUSTOM_KERNEL_ALSPS
    alsps_init,
#endif    
#ifdef FEATURE_FTM_TVOUT
    tvout_init,
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
    barometer_init,
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
    gyro_init,
    gyro_cali_init,
#endif 
//#ifdef FEATURE_FTM_SIGNALTEST
    sigtest_init,
//#endif

#ifdef FEATURE_FTM_CMMB
    cmmb_init,
#endif
#ifdef FEATURE_FTM_EMMC
	emmc_init,
#endif
#ifdef FEATURE_FTM_CLEAREMMC
	clear_emmc_init,
#endif

#ifdef FEATURE_FTM_EMI
    emi_init,
#endif
#ifdef FEATURE_FTM_HDMI
    hdmi_init,
#endif
#ifdef FEATURE_FTM_RF
    rf_test_init,
#endif
#ifdef MTK_HOTKNOT_SUPPORT
    hotknot_init,
#endif
    NULL,
};
