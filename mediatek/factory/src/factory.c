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


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <unistd.h>
#include <pthread.h>


#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"

#include "libnvram.h"
#include "CFG_file_info_custom.h"
#include "hardware/ccci_intf.h"



#define BUFSZ      2048
#define TAG        "[MAIN] "

#define FTM_CUST_FILE1  "/sdcard/factory.ini"
#define FTM_CUST_FILE2  "/etc/factory.ini"
//add for save test report
#define TEST_REPORT_SAVE_FILE "/data/testreport.log"

#define START "AT+START"
#define STOP "AT+STOP"
#define REQUEST_DATA "AT+REQUESTDATA"
#define VERSION "AT+VERSION"
#define READ_BARCODE "AT+READBARCODE"
#define WRITE_BARCODE "AT+WRITEBARCODE"
#define VIBRATOR_ENABLE "/sys/class/timed_output/vibrator/enable"
#define MAX_RETRY_COUNT 20
// add for idle current auto test
int usb_com_port = -1;
int usb_status = 0;
int usb_plug_in = 1;


#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))
pthread_mutex_t at_command_mutex = PTHREAD_MUTEX_INITIALIZER;

extern bool is_support_modem(int modem);

static item_t ftm_menu_items[] = {
    //item(ITEM_MUI_TEST,"Mini-UI Test"),
    item(ITEM_AUTO_TEST, uistr_auto_test),
    item(ITEM_FULL_TEST, uistr_full_test),
    item(ITEM_ITEM_TEST, uistr_item_test),
    item(ITEM_REPORT,    uistr_test_report),
    item(ITEM_DEBUG_TEST,uistr_debug_test),
#ifdef FEATURE_FTM_CLEARFLASH
    item(ITEM_CLRFLASH,  uistr_clear_flash),
#endif
#ifdef FEATURE_FTM_CLEAREMMC
    item(ITEM_CLREMMC,  uistr_clear_emmc),
#endif
    item(ITEM_VERSION,   uistr_version),
    item(ITEM_REBOOT,    uistr_reboot),
    item(ITEM_MAX_IDS,   NULL),
};

static item_t ftm_auto_test_items[] = {
#ifdef FEATURE_FTM_TOUCH
	item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif

#ifdef FEATURE_FTM_LCM
    item(ITEM_LCM,     uistr_lcm),
#endif

#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
    item(ITEM_SIGNALTEST, uistr_sig_test),
#endif

#ifdef FEATURE_FTM_BATTERY
    item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif

#ifdef FEATURE_FTM_FLASH
   item(ITEM_FLASH,   uistr_nand_flash),
#endif

#ifdef FEATURE_FTM_RTC
    item(ITEM_RTC,     uistr_rtc),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    item(ITEM_BT, uistr_bluetooth),
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
    item(ITEM_WIFI, uistr_wifi), //no uistr for wifi
#endif
#endif

#ifdef FEATURE_FTM_EMMC
    item(ITEM_EMMC,   uistr_emmc),
#endif

#ifdef FEATURE_FTM_MEMCARD
    item(ITEM_MEMCARD, uistr_memory_card),
#endif

#ifdef FEATURE_FTM_SIM
    item(ITEM_SIM, uistr_sim_detect),
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
	item(ITEM_GPS,	   uistr_gps),
#endif
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
        item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif
#ifdef FEATURE_FTM_SUB_CAMERA
        item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif



#ifdef FEATURE_FTM_AUDIO
    item(ITEM_LOOPBACK_PHONEMICSPK,uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef RECEIVER_HEADSET_AUTOTEST
#ifdef FEATURE_FTM_AUDIO
    item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#endif

#ifdef FEATURE_FTM_MATV
    //item(ITEM_MATV_NORMAL,  "MATV HW Test"),
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#ifdef FEATURE_FTM_RF
    item(ITEM_RF_TEST,  uistr_rf_test),
#endif
#ifdef MTK_HOTKNOT_SUPPORT
    item(ITEM_HOTKNOT, uistr_hotknot),  
#endif

    item(ITEM_MAX_IDS, NULL),

};
static item_t pc_control_items[] = {
    item(ITEM_FM,      "AT+FM"),
    item(ITEM_MEMCARD,      "AT+MEMCARD"),
    item(ITEM_SIM,      "AT+SIM"),
    item(ITEM_GPS,      "AT+GPS"),
    item(ITEM_EMMC,      "AT+EMMC"),
    item(ITEM_WIFI,	"AT+WIFI"),
    item(ITEM_LOOPBACK_PHONEMICSPK,      "AT+RINGTONE"),
    item(ITEM_SIGNALTEST,      "AT+SIGNALTEST"),
    item(ITEM_RTC,      "AT+RTC"),
    item(ITEM_CHARGER,      "AT+CHARGER"),
    item(ITEM_BT,      "AT+BT"),
    item(ITEM_MAIN_CAMERA, "AT+MAINCAMERA"),
    item(ITEM_SUB_CAMERA, "AT+SUBCAMERA"),
    item(ITEM_KEYS, "AT+KEY"),
    item(ITEM_MATV_AUTOSCAN, "AT+MATV"),
    item(ITEM_TOUCH_AUTO, "AT+TOUCH"),
    #ifdef FEATURE_FTM_FLASH
    item(ITEM_CLRFLASH, "AT+FLASH"),
	#endif
	#ifdef FEATURE_FTM_EMMC
	item(ITEM_CLREMMC,"AT+FLASH"),
	#endif
    item(ITEM_VIBRATOR, "AT+VIBRATOR"),
    item(ITEM_LED, "AT+LED"),
#ifdef FEATURE_FTM_RECEIVER
    item(ITEM_RECEIVER, "AT+RECEIVER"),
#endif
    item(ITEM_HEADSET, "AT+HEADSET"),
    item(ITEM_CMMB, "AT+CMMB"),
    item(ITEM_GSENSOR, "AT+GSENSOR"),
    item(ITEM_MSENSOR, "AT+MSENSOR"),
    item(ITEM_ALSPS, "AT+ALSPS"),
    item(ITEM_GYROSCOPE, "AT+GYROSCOPE"),
    item(ITEM_IDLE, "AT+IDLE"),
    #ifdef FEATURE_FTM_LCM
    item(ITEM_LCM, "AT+LCM"),
    #endif
	item(ITEM_VIBRATOR_PHONE, "AT+PVIBRATOR"),
    item(ITEM_RECEIVER_PHONE, "AT+PRECEIVER"),
    item(ITEM_HEADSET_PHONE, "AT+PHEADSET"),
    item(ITEM_LOOPBACK_PHONEMICSPK_PHONE, "AT+PLOOPBACK"),
    item(ITEM_MICBIAS, "AT+MICBIAS"),
	item(ITEM_MAX_IDS, NULL),
};
static item_t ftm_debug_test_items[] = {

#ifdef FEATURE_FTM_AUDIO
#ifdef FEATURE_FTM_RECEIVER
		item(ITEM_RECEIVER_DEBUG, uistr_info_audio_receiver_debug),
#endif
#endif
	item(ITEM_MAX_IDS, NULL),
};


static item_t ftm_test_items[] = {
#ifdef FEATURE_FTM_KEYS
    item(ITEM_KEYS,    uistr_keys),
#endif
#ifdef FEATURE_FTM_JOGBALL
    item(ITEM_JOGBALL, uistr_jogball),
#endif
#ifdef FEATURE_FTM_OFN
    item(ITEM_OFN,     uistr_ofn),
#endif
#ifdef FEATURE_FTM_TOUCH
    item(ITEM_TOUCH,   uistr_touch),
    item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif
#ifdef FEATURE_FTM_LCD
    item(ITEM_LCD,     uistr_backlight_level),
#endif

#ifdef FEATURE_FTM_LCM
    item(ITEM_LCM,     uistr_lcm),
#endif

#ifdef FEATURE_FTM_FLASH
    item(ITEM_FLASH,   uistr_nand_flash),
#endif
#ifdef FEATURE_FTM_EMMC
    item(ITEM_EMMC,   uistr_emmc),
#endif
#ifndef FEATURE_FTM_WIFI_ONLY
#ifdef FEATURE_FTM_MEMCARD
    item(ITEM_MEMCARD, uistr_memory_card),
#endif
#ifdef FEATURE_FTM_SIMCARD
    item(ITEM_SIMCARD, uistr_sim_card),
#endif
#ifdef FEATURE_FTM_SIM
    item(ITEM_SIM, uistr_sim_detect),
#endif
#endif
//#ifdef FEATURE_FTM_SIGNALTEST
#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
    item(ITEM_SIGNALTEST, uistr_sig_test),
#endif
//#endif
#ifdef FEATURE_FTM_VIBRATOR
    item(ITEM_VIBRATOR, uistr_vibrator),
    item(ITEM_VIBRATOR_PHONE, uistr_vibrator_phone), // Phone Level Test
#endif
#ifdef FEATURE_FTM_LED
    item(ITEM_LED,     uistr_led),
#endif
#ifdef FEATURE_FTM_RTC
    item(ITEM_RTC,     uistr_rtc),
#endif

#ifdef FEATURE_FTM_AUDIO
    item(ITEM_LOOPBACK_PHONEMICSPK, uistr_info_audio_loopback_phone_mic_speaker),
#ifdef FEATURE_FTM_RECEIVER
    item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
//    item(ITEM_LOOPBACK, uistr_info_audio_loopback),
#ifdef FEATURE_FTM_ACSLB
    item(ITEM_ACOUSTICLOOPBACK, uistr_info_audio_acoustic_loopback),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_HEADSET_LOOPBACK
    item(ITEM_LOOPBACK1, uistr_info_audio_loopback_phone_mic_headset),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_SPEAKER_LOOPBACK
    item(ITEM_LOOPBACK2, uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef FEATURE_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
    item(ITEM_LOOPBACK3, uistr_info_audio_loopback_headset_mic_speaker),
#endif
#ifdef FEATURE_FTM_WAVE_PLAYBACK
    item(ITEM_WAVEPLAYBACK, uistr_info_audio_loopback_waveplayback),
  #endif
    item(ITEM_MICBIAS, uistr_info_audio_micbias),
    // Phone Level Test
    item(ITEM_RECEIVER_PHONE, uistr_info_audio_receiver_phone),
    item(ITEM_HEADSET_PHONE, uistr_info_headset_phone),
    item(ITEM_LOOPBACK_PHONEMICSPK_PHONE, uistr_info_audio_loopback_phone_mic_speaker_phone),
#endif //FEATURE_FTM_AUDIO

#ifdef FEATURE_FTM_HEADSET
    item(ITEM_HEADSET, uistr_info_headset),
#endif
#ifdef FEATURE_FTM_SPK_OC
    item(ITEM_SPK_OC, uistr_info_speaker_oc),
#endif
#ifdef FEATURE_FTM_OTG
    item(ITEM_OTG, "OTG"),
#endif
#ifdef FEATURE_FTM_USB
    item(ITEM_USB, "USB"),
#endif
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    item(ITEM_GSENSOR, uistr_g_sensor),
#endif
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    item(ITEM_GS_CALI, uistr_g_sensor_c),
#endif
#ifdef CUSTOM_KERNEL_MAGNETOMETER
    item(ITEM_MSENSOR, uistr_m_sensor),
#endif
#ifdef CUSTOM_KERNEL_ALSPS
    item(ITEM_ALSPS, uistr_als_ps),
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
    item(ITEM_BAROMETER, uistr_barometer),
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
    item(ITEM_GYROSCOPE, uistr_gyroscope),
    item(ITEM_GYROSCOPE_CALI, uistr_gyroscope_c),
#endif
#ifdef FEATURE_FTM_MAIN_CAMERA
    item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif
#ifdef FEATURE_FTM_MAIN2_CAMERA
    item(ITEM_MAIN2_CAMERA,  uistr_main2_sensor),
#endif
#ifdef FEATURE_FTM_SUB_CAMERA
    item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif


#ifdef FEATURE_FTM_STROBE
    item(ITEM_STROBE, uistr_strobe),
#endif
#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
    item(ITEM_GPS,     uistr_gps),
#endif
#endif

#ifdef MTK_NFC_SUPPORT
    item(ITEM_NFC,    uistr_nfc),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
    item(ITEM_FMTX, uistr_info_fmt_title),
#endif
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    item(ITEM_BT, uistr_bluetooth),
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
    item(ITEM_WIFI, uistr_wifi),
#endif
#endif

#if 1
#ifdef FEATURE_FTM_MATV
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif

#if 0
    //item(ITEM_MATV_NORMAL,  "MATV HW Test"),
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#endif

#ifdef FEATURE_FTM_BATTERY
    //item(ITEM_CHARGER, "Battery & Charger"),
   item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif
#ifdef FEATURE_FTM_IDLE
    item(ITEM_IDLE,    uistr_idle),
#endif
#ifdef FEATURE_FTM_TVOUT
    item(ITEM_TVOUT,     uistr_info_tvout_item),
#endif
#ifdef FEATURE_FTM_CMMB
    item(ITEM_CMMB, uistr_cmmb),
#endif
#ifdef FEATURE_FTM_EMI
    item(ITEM_EMI, uistr_system_stability),
#endif
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "HDMI"),
#endif
#ifdef FEATURE_FTM_RF
    item(ITEM_RF_TEST,  uistr_rf_test),
#endif
    item(ITEM_MAX_IDS, NULL),
};

static item_t ftm_cust_items[ITEM_MAX_IDS];
static item_t ftm_cust_auto_items[ITEM_MAX_IDS];
char at_command[128] = {0};
#ifdef FEATURE_FTM_VIBRATOR
extern bool vibrator_test_exit;
#endif
#ifdef FEATURE_FTM_LED
extern bool keypadled_test_exit;
extern bool led_test_exit;
#endif
#ifdef FEATURE_FTM_AUDIO
extern bool bMicbias_exit;
#endif


enum {
    ITEM_MUI_ITEMVIEW,
    ITEM_MUI_TEXTVIEW,
    ITEM_MUI_IMAGEVIEW,
    ITEM_MUI_PAINTVIEW,
    ITEM_MUI_MAX_IDS,
};

enum {
    ITEM_TEST_PASS,
    ITEM_TEST_FAIL,
    ITEM_BACK,
};

//add for saving test report
enum {
    TEST_REPORT_UNTEST,
	TEST_REPORT_PASS,
	TEST_REPORT_FAIL,
};


static text_t miniui_title = {
    .string = "Mini-UI Test",
    .color  = COLOR_YELLOW,
};

static item_t miniui_item[] = {
    item(ITEM_MUI_ITEMVIEW,  "ItemView Test"),
    item(ITEM_MUI_TEXTVIEW,  "TextView Test"),
    item(ITEM_MUI_IMAGEVIEW, "ImageView Test"),
    item(ITEM_MUI_PAINTVIEW, "PaintView Test"),
    item(ITEM_MUI_MAX_IDS, NULL),
};


extern int wait4_ack (const int fd, char *pACK, int timeout);
extern int read_ack (const int fd, char *rbuff, int length);
extern int send_at (const int fd, const char *pCMD);

static int g_nr_lines = 0;
char test_data[128];
int status = 0;


sp_ata_data return_data;
// add for idle current auto test
int is_USB_State_PlugIn(void)
{
    int type = 0;
    char buf[11];
    int bytes_read = 0;
    int res = 0;
    int fd = open("/sys/class/android_usb/android0/state", O_RDONLY);
    if (fd != -1)
    {
        memset(buf, 0, 11);
        while (bytes_read < 10)
        {
            res = read(fd, buf + bytes_read, 10);
            if (res > 0)
                bytes_read += res;
            else
                break;
        }
        close(fd);
        type = strcmp(buf,"CONFIGURED");

        LOGD("[TST_DRV]Query USB State OK.");
    }
    else
    {
        LOGD("[TST_DRV]Failed to open:/sys/class/android_usb/android0/state");
    }
         
	return (type == 0);     
}

void close_usb()
{
    close(usb_com_port);
    usb_status = 0;
    usb_plug_in = 0;
    LOGD("close_usb!\n");
}

void open_usb()
{
    usb_com_port = open_usb_port(UART_PORT1, 115200, 8, 'N', 1);
	if(usb_com_port == -1)
	{
		LOGE(TAG "Open usb fail\r\n");
	}
	else
	{
		//initTermIO(*hUsbComPort);
		LOGD(TAG "Open usb success\r\n");
	}
    LOGD("Open usb!\n");
}

static int get_AT_command(char *origin_at_command)
{
    char *ptr = NULL;
	char *p;
	char *temp_at_command = origin_at_command;
	int result = 0;
	int i = 0;
	int len = strlen(origin_at_command);
	p = origin_at_command;
	ptr = strchr(temp_at_command, '=');
	if(ptr == NULL)
	{
	    LOGD(TAG "ptr is null\n");
        pthread_mutex_lock (&at_command_mutex);
		strcpy(at_command, origin_at_command);
        pthread_mutex_unlock (&at_command_mutex);
		result = 0;
	}
	else
	{
		if(!strncmp(++ptr, "CLOSE", strlen("CLOSE")))
		{
			*(--ptr) = '\0';

			result = 1;
		}
		else
		{
	    	for (i = 0; i < len; i++, p++)
			{
           		if ((*p == '+') && ((i + 1) < len))
				{
                *p = '\0';
                break;
            }
        }
	    LOGD(TAG "ptr is not null\n");
//		strcpy(at_command, ++p);
        *(--ptr) = '\0';
		ftm_set_prop(++p, ++ptr);
			result = 2;
        }

	}
//    LOGD(TAG "%s\n");
	return result;
}

//add for saving test report
static FILE * open_file(char* filename)
{
    FILE *fp;
    if (NULL == (fp = fopen(filename, "wb")))
    {
        fprintf(fp, "test report");
    }
	return fp;
}

//add for saving test report
static int write_test_report(item_t *items, FILE *fp)
{

    int i = 0, test_report_len = 0, write_result = -1;
	char test_report[1024] = {0};
	char *get_test_report = test_report;
	char result[] = { ' ', 'O', 'X' };
	int state = 0;

    while (i < ITEM_MAX_IDS && items->name) {

//		LOGD(TAG "items.name=%s item.background=%d", items->name, items->background);
		if(items->background == 0)
		{
            state = TEST_REPORT_UNTEST;
		}
		else if (items->background == COLOR_GREEN)
		{
            state = TEST_REPORT_PASS;
		}
		else if (items->background == COLOR_RED)
		{
            state = TEST_REPORT_FAIL;
		}
//		LOGD(TAG "state = %d", state);
        if(strncmp(items->name, uistr_info_test_report_back, strlen(uistr_info_test_report_back)))
        {
		    get_test_report = test_report + test_report_len;
            test_report_len += snprintf(get_test_report, 40, "%s=%c\n", items->name+4,
				result[state]);
        }
//		LOGD(TAG "%s", get_test_report);
//	    LOGD(TAG "%s", test_report);

        i++;
        items++;
    }

    LOGD(TAG "before write");
	LOGD(TAG "%s", test_report);
    write_result = fputs(test_report, fp);
	LOGD(TAG "The result of fputs is %d", write_result);

	return 0;
}


int  strcmp_def(
        const char * src,
        const char * dst
        )
{
        int ret = 0 ;

        while( ! (ret = *(unsigned char *)src - *(unsigned char *)dst) && *dst)
                ++src, ++dst;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
}


static item_t *get_item_list(void)
{
    item_t *items;

	LOGD(TAG "get_item_list");

    items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

    return items;
}

static item_t *get_debug_item_list(void)
{
    item_t *items;

	LOGD(TAG "get_debug_item_list");

    items = ftm_debug_test_items;

    return items;
}


static item_t *get_manual_item_list(void)
{
    item_t *items;
	item_t *items_auto;
	int i = 0;
	int j =0;
	LOGD(TAG "get_manual_item_list");

    items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	items_auto = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

	while (items_auto[i].name)
	{
		for(j =0;items[j].name != NULL ;j++)
		{
			if(strcmp(items[j].name,items_auto[i].name)==0)
			{
				items[j].mode = FTM_AUTO_ITEM;
				LOGD(TAG "%s",items[j].name);
			}
		}
		i++;
	}

    return items;
}

static item_t *get_auto_item_list(void)
{
    item_t *items;

    items = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

    return items;
}

static const char *get_item_name(item_t *item, int id)
{
    int i;

    while (item->name) {
        if (item->id == id)
            return item->name;
        item++;
    }
    return NULL;
}

static int get_item_id(item_t *item, char *name)
{
    int i;

    while (item->name)
	{
		if(strlen(item->name)==strlen(name))
		{
        	if (!strncasecmp(item->name, name, strlen(item->name)))
            return item->id;
		}
        item++;
    }
    return -1;
}

static int paintview_key_handler(int key, void *priv)
{
    int handled = 0;
    struct paintview *pv = (struct paintview *)priv;

    switch (key) {
    case UI_KEY_BACK:
        pv->exit(pv);
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}


static int textview_key_handler(int key, void *priv)
{
    int handled = 0;
    int avail_lines = get_avail_textline();
    struct textview *tv = (struct textview *)priv;

    switch (key) {
    case UI_KEY_BACK:
        tv->exit(tv);
        break;
    case UI_KEY_UP:
    case UI_KEY_VOLUP:
        LOGE("textview_key_handler: key up\n");
        LOGE("textview_key_handler: avail_lines=%d, tv->m_nr_lines=%d, tv->m_start=%d, tv->m_end=%d,", \
                                    avail_lines, tv->m_nr_lines, tv->m_start, tv->m_end);
        if (avail_lines < tv->m_nr_lines && tv->m_start > 0) {
            tv->m_start--;
            tv->m_end--;
            tv->m_redraw = 1;
            LOGE("textview_key_handler: key up redraw\n");
        }
        break;
    case UI_KEY_DOWN:
    case UI_KEY_VOLDOWN:
        LOGE("textview_key_handler: key down\n");
        LOGE("textview_key_handler: avail_lines=%d, tv->m_nr_lines=%d, tv->m_start=%d, tv->m_end=%d,", \
                                    avail_lines, tv->m_nr_lines, tv->m_start, tv->m_end);
        if (avail_lines < tv->m_nr_lines && tv->m_end < tv->m_nr_lines) {
            tv->m_start++;
            tv->m_end++;
            tv->m_redraw = 1;
            LOGE("textview_key_handler: key down redraw\n");
        }
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}

static int imageview_key_handler(int key, void *priv)
{
    int handled = 0;
    struct imageview *imv = (struct imageview *)priv;

    switch (key) {
    case UI_KEY_BACK:
        imv->exit(imv);
        break;
    default:
        handled = -1;
        break;
    }
    return handled;
}

static char *trimspace(char *s)
{
    char *e;

    while (isspace(*s)) s++;

    e = s + strlen(s) - 1;

    while (e > s && isspace(*e)) e--;

    *(e + 1) = '\0';

    return s;
}


int get_is_ata(){
    LOGD(TAG "status........................... = %d\n", status);
    return status;
}


static int read_config(char *filename)
{
    char  buf[BUFSZ]={0};
    char *name, *val, *p;
    int   num = 0, i, id, len, limit, auto_id;
    int   auto_num = 0;
    item_t *items;
    item_t *auto_items;
    FILE *fp;

    if (NULL == (fp = fopen(filename, "r")))
        return num;

    memset(ftm_cust_items, 0, sizeof(ftm_cust_items));
    memset(ftm_cust_auto_items, 0, sizeof(ftm_cust_auto_items));
    items = ftm_cust_items;
    auto_items = ftm_cust_auto_items;
    limit = ARRAY_SIZE(ftm_cust_items) - 1;

    while (fgets(buf, BUFSZ, fp)) {
        if (NULL != (val = strstr(buf, "//")))
            *val = '\0';
        if (NULL != (val = strchr(buf, ';')))
            *val = '\0';

        len = strlen(buf);
        if (!len)
            continue;

        name = p = buf;
        val  = NULL;

        for (i = 0; i < len; i++, p++) {
            if ((*p == '=') && ((i + 1) < len)) {
                *p = '\0';
                val = p + 1;
                break;
            }
        }
        if (i == len)
            continue;

        if (name)
            name = trimspace(name);
        if (val)
            val = trimspace(val);

        if (strcasestr(name, "menuitem") && num < limit) {
            if (val && ((id = get_item_id(&ftm_test_items[0], val)) >= 0)) {
                LOGD(TAG "set menuitem[%d]: %s (%d)\n", num, val, id);
                items[num].id   = id;
                items[num].name = strdup(val);
				if((auto_id = get_item_id(&ftm_auto_test_items[0],val)) >= 0)
				{
				    LOGD(TAG "output id=%d, val=%s\n", auto_id, val);
                    auto_items[auto_num].id = auto_id;
					auto_items[auto_num].name = strdup(val);
					auto_items[auto_num].mode = FTM_AUTO_ITEM;
					items[num].mode = FTM_AUTO_ITEM;
					auto_num++;
				}
				LOGD(TAG "items[%d].mode=%d\n", num, items[num].mode);
                num++;
            }
        } else {
            LOGD(TAG "set prop: %s=%s\n", name, val ? val : "null");
            ftm_set_prop(name, val);
        }
    }
    items[num].id   = -1;
    items[num].name = NULL;

    fclose(fp);

    return num;
}

static int create_report(item_t *item, item_t *rpt_items, int maxitems, char *buf, int size)
{
    struct ftm_module *mod;
    int i = 0, len = 0;
    char *ptr = buf;
    char result[] = { ' ', 'O', 'X' };
    color_t bgc[] = { 0, COLOR_GREEN, COLOR_RED };
    //handle of testreport.log
	FILE *fp = NULL;

    while (i < maxitems && item->name) {
        mod = ftm_get_module(item->id);
        if (mod && mod->visible && len < size) {
            ptr = buf + len;
            len += sprintf(ptr, "[%c] %s ",
                (mod->test_result >= FTM_TEST_MAX) ?
                result[FTM_TEST_UNKNOWN] : result[mod->test_result], item->name);
            ptr[len++] = '\0';
            rpt_items[i].id = mod->id;
            rpt_items[i].name = ptr;
            rpt_items[i].background = (mod->test_result >= FTM_TEST_MAX) ?
                0 : bgc[mod->test_result];
            i++;
        }
        item++;
    }

    //add for saving test report
    fp = open_file(TEST_REPORT_SAVE_FILE);

	if(fp == NULL)
	{
	    LOGD(TAG "TEST_REPORT_SAVE_FILE is null");
	}
	else
	{
	    LOGD(TAG "TEST_REPORT_SAVE_FILE is not null");
        write_test_report(rpt_items, fp);
		fclose(fp);
	}
    //add for saving test report

    if (i < maxitems - 1) {
        rpt_items[i].id   = ITEM_MAX_IDS;
        rpt_items[i].name = uistr_info_test_report_back;
    }
    return ++i;
}

void getIMEI(int sim, int fd,char *result) {
    unsigned int i=0,j=0;
    char buf[64]={0};
	strcpy(result, "unknown");
	int count = 0;
    if(sim==1) strcpy(buf, "AT+EGMR=0,7\r\n");
    else if(sim==2) strcpy(buf, "AT+EGMR=0,10\r\n");
	else if(sim==3) strcpy(buf, "AT+EGMR=0,11\r\n");
	else strcpy(buf, "AT+EGMR=0,12\r\n");

retry:
	send_at(fd, buf);
	memset(buf,'\0',64);
	read_ack(fd,buf,64);
	LOGD("buf %s",buf);
    buf[63]=0;
    i=0, j=0;
	if(strlen(buf)>14)
	{
        for(i=0;i<strlen(buf)-14;i++)
		{
            for(j=i;j<i+15;j++)
			{
            if(buf[j]<'0' || buf[j]>'9') break;
            }
            if(j==i+15) break; else i=j;
        }
        buf[j]=0;
        if(j==i+15)
        {
			strcpy(result, &(buf[i]));
			count = 3;
	    }
	    else
	    {
			count++;
		}
	}
	else
	{
        count++;
	}

	LOGD(TAG "COUNT = %d\n", count);

	if(count < 3)
	{
		LOGD(TAG "go to retry");
		strcpy(buf, "AT\r\n");
        goto retry;
	}

	LOGE("getIMEI %s",result);
}


int getModemVersion(int fd,char *result)
{
	const int BUF_SIZE = 128;
	char buf[BUF_SIZE];
	memset(buf,'\0',BUF_SIZE);
	const int HALT_TIME = 100 * 1000;
	int count = 0;
	char *p = NULL;

	strcpy(buf, "AT+CGMR\r\n");
	strcpy(result, "unknow\n");

retry:
	send_at(fd, buf);
	memset(buf,'\0',BUF_SIZE);
	read_ack(fd,buf,BUF_SIZE);
	LOGD("buf %s",buf);

	//const char *tok = "+CGMR: ";
	//p = strstr(buf, tok);
	p = strchr(buf, ' '); // find the first space char.
	if(p) {
		strcpy(result, ++p);
		count = 3;
	} else {
		strcpy(buf, "AT\r\n");
		count++;
	}

	if(count < 3)
	{
		LOGD(TAG "COUNT in modem IS %d\n", count);
		goto retry;
	}

	LOGE(TAG "getModemVersion result = %s\n", result);
	return 0;
}

int write_barcode(int fd, char* barcode)
{
    char buf[128] = {0};
    int result = 0;
    if((fd == -1) || (barcode == NULL))
    {
        return -1;
    }
    if(strlen(barcode) > 113)
    {
        // barcode is too long, buf will leak
        return -1;
    }
    sprintf(buf, "AT+EGMR=1,5,\"%s\"\r\n", barcode);
    send_at(fd, buf);

    memset(buf, 0, 128);

    result = wait4_ack (fd, NULL, 3000);

    return result;
}

/*
* Caution: here we execute UART command to retrieve the barcode number from modem. And notice that we rely on the fact that there are double quotations(i.e.") in
* the bar-code returned by UART command.
* If the format of the bar-code number changes, the following code will probably NOT do what it is supposed to do:(
*/
int getBarcode(int fd,char *result)
{
	const int BUF_SIZE = 128;
	char buf[BUF_SIZE];
	const int HALT_TIME = 100 * 1000;
	int count = 0;
	char *p = NULL;

	strcpy(buf, "AT+EGMR=0,5\r\n");
	strcpy(result, "unknown");

retry:
	send_at(fd, buf);
	memset(buf,'\0',BUF_SIZE);
	read_ack(fd,buf,BUF_SIZE);
	LOGD("buf %s",buf);

	p = strchr(buf, '\"'); // find the first double quotation mark.
	if(p) {
		strcpy(result, ++p);
		count = 3;
	} else {
		strcpy(buf, "AT\r\n");
		count++;
	}

	if(count < 3)
	{
        LOGD(TAG "COUNT IN BARCODE IS %d\n", count);
		goto retry;
	}
	LOGE("getBarcode result = %s\n", result);
	return 0;
}

static void print_verinfo(char *info, int *len, char *tag, char *msg)
{
    char buf[256];
    int _len = *len;
    int tag_len = strlen(tag);

	int max_len = gr_fb_width() / CHAR_WIDTH *2;
    int msg_len = strlen(msg);

    int buf_len = gr_fb_width() / CHAR_WIDTH;

    _len += sprintf(info + _len, "%s", tag);
    _len += sprintf(info + _len, ": ");

    if(msg_len>max_len-tag_len-2) {
        _len += sprintf(info+_len,"\n    ");
        g_nr_lines++;
    }

    while(msg_len>0) {
        buf_len = max_len - 4;
        buf_len = (msg_len > buf_len ? buf_len : msg_len);
        strncpy(buf, msg, buf_len);
        buf[buf_len] = 0;

        _len += sprintf(info + _len, "%s", buf);
        _len += sprintf(info + _len, "\n");
        g_nr_lines++;
        msg_len-=buf_len;
        msg = &(msg[buf_len]);
        while(msg_len>0 && msg[0]==' ') {
            msg_len--;
            msg = &(msg[1]);
        }
#if 1
        if(msg_len>0) {
            for(buf_len=0; buf_len < 4; buf_len++) buf[buf_len]=' ';
            buf[buf_len]=0;
            //_len += sprintf(info+_len, buf);
            // Fix Anroid 2.3 build error
	    _len += sprintf(info + _len, "%s", buf);
        }
#endif
    }
    *len = _len;
    //LOGE("In factory mode: g_nr_lines = %d\n", g_nr_lines);
}

#if defined(FEATURE_FTM_3GDATA_SMS) || defined(FEATURE_FTM_3GDATA_ONLY) || defined(FEATURE_FTM_WIFI_ONLY)

static bool get_barcode_from_nvram(char *barcode_result)
{

    int read_nvram_ready_retry = 0;
	F_ID fid;
	int rec_size = 0;
	int rec_num = 0;
	int barcode_lid = AP_CFG_REEB_PRODUCT_INFO_LID;
	PRODUCT_INFO *barcode_struct;
	bool isread = true;
	char nvram_init_val[128] = {0};
    LOGD(TAG "Entry get_barcode_from_nvram");
	while(read_nvram_ready_retry < MAX_RETRY_COUNT)
	{
		read_nvram_ready_retry++;
		property_get("nvram_init", nvram_init_val, NULL);
		if(strcmp(nvram_init_val, "Ready") == 0)
		{
			break;
		}
		else
		{
			usleep(500*1000);
		}
	}

	if(read_nvram_ready_retry >= MAX_RETRY_COUNT)
	{
		LOGD(TAG "Get nvram restore ready failed!");
		return false;
	}

	barcode_struct= (PRODUCT_INFO *)malloc(sizeof(PRODUCT_INFO));
	if(barcode_struct == NULL)
	{
		return false;
	}

	fid = NVM_GetFileDesc(barcode_lid, &rec_size, &rec_num, isread);

	if(fid.iFileDesc < 0)
	{
		LOGD(TAG "fid.iFileDesc < 0");
		return false;
	}

	if(rec_size != read(fid.iFileDesc, barcode_struct, rec_size))
	{
		free(barcode_struct);
		return false;
	}
	if(strlen(barcode_struct->barcode) > 0)
	{
		strcpy(barcode_result, barcode_struct->barcode);
	}else
	{
		strcpy(barcode_result, "unknown");
	}

	free(barcode_struct);
	if(!NVM_CloseFileDesc(fid))
	{
		return false;
	}
    LOGD("The size of barcode_struct:%d\n", sizeof(barcode_struct));
    LOGD("Barcode is %s\n", barcode_result);
	return true;
}
#endif

static int create_verinfo(char *info, int size)
{

    int fd=-1;
	int fd2=-1;
	int fd_dt = -1;
	int fd5=-1;
    char val[128]={0};
    char dev_node1[32];
    char dev_node2[32];
    char dev_node5[32];
    int len = 0;
	unsigned int i;
    char ver[128]={0};
    char imei1[64]={0};
	char imei2[64]={0};
	char imei3[128]={0};
	char imei4[128]={0};
    char modem_ver[128] = "unknown";
	char modem_ver2[128] = "unknown";
	char modem_ver_dt[128] = "unknown";
    char barcode[128] = "unknown";
	char barcode2[128] = "unknown";
	char barcode_dt[128] = "unknown";

    char kernel_ver[256] = "unknown";
    char uboot_ver[128]  = "unknown";
    char uboot_build_ver[128]  = "unknown";
    char kernel_build_ver[128] = "unknown";
    char rootfs_build_ver[128]  = "unknown";
    int kernel_ver_fd = -1;
    int kernel_cli_fd = -1;
    char buffer[1024];
    char *ptr= NULL, *pstr = NULL;
    int reslt=0;
	int move_bit = 0;
	g_nr_lines = 0;


	if(is_support_modem(1))
	{
		LOGD(TAG "MTK_ENABLE_MD1\n");

		snprintf(dev_node1, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS1));
        fd = openDeviceWithDeviceName(dev_node1);
		if(-1 == fd) {
			LOGD(TAG "Fail to open CCCI interface\n");
			return 0;
		}
		for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
		send_at (fd, "AT\r\n");
		wait4_ack (fd, NULL, 3000);
	}

	if(is_support_modem(2)){
		LOGD(TAG "MTK_ENABLE_MD2\n");
		
		snprintf(dev_node2, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
        fd2 = openDeviceWithDeviceName(dev_node2);
        
		if(-1 == fd2) {
			LOGD(TAG "Fail to open ttyMT0 interface\n");
			return 0;
		}
		initTermIO(fd2,5);
		for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
		send_at (fd2, "AT\r\n");
		wait4_ack (fd2, NULL, 3000);
	}

//MT6582LTE_SUPPORT
if(is_support_modem(5)){
	LOGD(TAG "MTK_ENABLE_MD5\n");
	
	snprintf(dev_node5, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS5));
    fd5 = openDeviceWithDeviceName(dev_node5);
    
	if(-1 == fd5) {
		LOGD(TAG "Fail to open /dev/eemcs_muxd interface\n");
		return 0;
	}
	//initTermIO(fd5,5);
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd5, "AT\r\n");
	wait4_ack (fd5, NULL, 3000);
}


#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
	LOGD(TAG "MTK_DT_SUPPORT\n");
#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    fd_dt = openDeviceWithDeviceName("/dev/ttyUSB1");
    initTermIO(fd_dt, 5);
#else
	fd_dt= openDeviceWithDeviceName("/dev/ttyMT0");
#endif
	if(-1 == fd_dt) {
		LOGD(TAG "Fail to open ttyMT0 interface\n");
		return 0;
	}
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	send_at (fd_dt, "AT\r\n");
	wait4_ack (fd_dt, NULL, 3000);
#endif

#ifdef MTK_EXTERNAL_MODEM_SLOT
	if (!strcmp(MTK_EXTERNAL_MODEM_SLOT,"1"))
	{

		if(is_support_modem(1)){
			getIMEI(1, fd, imei2);

        }else if(is_support_modem(2)){
			getIMEI(1, fd2, imei2);
        }


#ifndef EVDO_DT_SUPPORT
		getIMEI(1, fd_dt, imei1);
#endif

	}
	else if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "2"))
	{
        if(is_support_modem(1)){
		    getIMEI(1, fd, imei1);

        }else if(is_support_modem(2)){
		    getIMEI(1, fd2, imei1);
        }

#ifndef EVDO_DT_SUPPORT
		getIMEI(1, fd_dt, imei2);
#endif

	}

#else

	if(is_support_modem(1))
    {
		getIMEI(1, fd, imei1);
#ifdef GEMINI

		if(is_support_modem(2)){
			getIMEI(1, fd2, imei2);
		}else{

			getIMEI(2, fd, imei2);
#if defined(MTK_GEMINI_3SIM_SUPPORT)
			getIMEI(3,fd, imei3);
#elif defined(MTK_GEMINI_4SIM_SUPPORT)
			getIMEI(3,fd, imei3);
			getIMEI(4,fd, imei4);
#endif
		}
#endif

	}
    else if(is_support_modem(2)){
		getIMEI(1, fd2, imei1);
#ifdef GEMINI
		getIMEI(2, fd2, imei2);
#endif

	}
    else if(is_support_modem(5)){
		getIMEI(1, fd5, imei1);
		#ifdef GEMINI
			getIMEI(2, fd5, imei2);
		#endif
     }

#endif


if(is_support_modem(1)){
    reslt = getModemVersion(fd,modem_ver);
    ptr = strchr(modem_ver, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver[strlen(modem_ver)-1] == '\r')
	{
        modem_ver[strlen(modem_ver)-1] = 0;
	}

    reslt = getBarcode(fd,barcode);
    ptr = strchr(barcode, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode) <= 0)
		strcpy(barcode, "unknown");
	closeDevice(fd);
}

if(is_support_modem(2)){
    reslt = getModemVersion(fd2,modem_ver2);
    ptr = strchr(modem_ver2, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver2[strlen(modem_ver2)-1] == '\r')
	{
        modem_ver2[strlen(modem_ver2)-1] = 0;
	}
    reslt = getBarcode(fd2,barcode2);
    ptr = strchr(barcode2, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode2) <= 0)
		strcpy(barcode2, "unknown");
    closeDevice(fd2);
}


if(is_support_modem(5)){
    reslt = getModemVersion(fd5,modem_ver);
    ptr = strchr(modem_ver, '\n');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(modem_ver[strlen(modem_ver)-1] == '\r')
	{
        modem_ver[strlen(modem_ver)-1] = 0;
	}

    reslt = getBarcode(fd5,barcode);
    ptr = strchr(barcode, '\"');
    if (ptr != NULL) {
        *ptr = 0;
    }
	if(strlen(barcode) <= 0)
		strcpy(barcode, "unknown");
	closeDevice(fd5);
}
 

#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
		reslt = getModemVersion(fd_dt,modem_ver_dt);
		ptr = strchr(modem_ver_dt, '\n');
		if (ptr != NULL) {
			*ptr = 0;
		}
		if(modem_ver_dt[strlen(modem_ver_dt)-1] == '\r')
		{
			modem_ver_dt[strlen(modem_ver_dt)-1] = 0;
		}
		reslt = getBarcode(fd_dt,barcode_dt);
		ptr = strchr(barcode_dt, '\"');
		if (ptr != NULL) {
			*ptr = 0;
		}
		if(strlen(barcode_dt) <= 0)
			strcpy(barcode_dt, "unknown");
		closeDevice(fd_dt);
#endif

#if defined(FEATURE_FTM_3GDATA_SMS) || defined(FEATURE_FTM_3GDATA_ONLY) || defined(FEATURE_FTM_WIFI_ONLY)

    get_barcode_from_nvram(barcode);
#endif

    kernel_ver_fd = open("/proc/version",O_RDONLY);
    if(kernel_ver_fd!=-1) {
        read(kernel_ver_fd, kernel_ver, 256);
        close(kernel_ver_fd);
    }

    kernel_cli_fd = open("/proc/cmdline",O_RDONLY);
    if(kernel_cli_fd!=-1) {
        read(kernel_cli_fd,buffer,128);
        ptr = buffer;
        pstr = strtok(ptr, ", =");
        while(pstr != NULL) {
            if(!strcmp(pstr, "uboot_build_ver")) {
                pstr = strtok(NULL, ", =");
                strcpy(uboot_build_ver, pstr);
            }
            if(!strcmp(pstr, "uboot_ver")) {
                pstr = strtok(NULL, ", =");
                strcpy(uboot_ver, pstr);
            }
            pstr = strtok(NULL, ", =");
        }
        close(kernel_cli_fd);
    }

    if(uboot_build_ver[strlen(uboot_build_ver)-1]=='\n') uboot_build_ver[strlen(uboot_build_ver)-1]=0;
    if(kernel_ver[strlen(kernel_ver)-1]=='\n') kernel_ver[strlen(kernel_ver)-1]=0;

    property_get("ro.mediatek.platform", val, "unknown");
    print_verinfo(info, &len,  "BB Chip     ", val);
    property_get("ro.product.device", val, "unknown");
    print_verinfo(info, &len,  "MS Board.   ", val);

    #ifdef FEATURE_FTM_3GDATA_SMS
    #elif defined FEATURE_FTM_3GDATA_ONLY
    #elif defined FEATURE_FTM_WIFI_ONLY
    #elif defined GEMINI
        #ifndef EVDO_DT_SUPPORT
            print_verinfo(info, &len,  "IMEI1       ", imei1);
            print_verinfo(info, &len,  "IMEI2       ", imei2);
            #if defined(MTK_GEMINI_3SIM_SUPPORT)
		        print_verinfo(info, &len,  "IMEI3       ", imei3);
	        #elif defined(MTK_GEMINI_4SIM_SUPPORT)
		        print_verinfo(info, &len,  "IMEI3       ", imei3);
                print_verinfo(info, &len,  "IMEI4       ", imei4);
	        #endif
         #else
            #ifdef MTK_EXTERNAL_MODEM_SLOT
                if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "1"))
                {
                    print_verinfo(info, &len, "IMEI        ", imei2);
                }
                else if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "2"))
                {
                    print_verinfo(info, &len, "IMEI        ", imei1);
                }
            #endif
         #endif
    #else
    print_verinfo(info, &len,  "IMEI        ", imei1);
    #endif

	if (!reslt) 
    {

		if(is_support_modem(1)){
            print_verinfo(info, &len,  "Modem Ver.  ", modem_ver);
			sprintf(return_data.version.modem_ver,"%s", modem_ver);

        }

		if(is_support_modem(2)){
		    print_verinfo(info, &len,  "Modem2 Ver.  ", modem_ver2);
        }

		if(is_support_modem(5)){
		    print_verinfo(info, &len,  "Modem Ver.  ", modem_ver);
        }

#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
		print_verinfo(info, &len, "Modem2 Ver.", modem_ver_dt);
#endif

		if(is_support_modem(1)){
            print_verinfo(info, &len,  "Bar code    ", barcode);

        }

		if(is_support_modem(2)){
		    print_verinfo(info, &len,  "Bar code2    ", barcode2);

        }
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
        print_verinfo(info, &len,  "Bar code2  ", barcode_dt);
#endif

		if(is_support_modem(5)){
            print_verinfo(info, &len,  "Bar code    ", barcode);
    }

    } else {
        LOGE(TAG "Fail to open device uart modem\n");
    }

        #if defined(FEATURE_FTM_3GDATA_SMS) || defined(FEATURE_FTM_3GDATA_ONLY) || defined(FEATURE_FTM_WIFI_ONLY)

        LOGD(TAG "Entry barcode in wifi only");
        print_verinfo(info, &len,  "Bar code    ", barcode);
		#endif

    property_get("ro.build.date", val, "TBD");
    print_verinfo(info, &len,  "Build Time  ", val);
//    print_verinfo(info, &len,  "UBoot Ver.  ", uboot_ver);

    ptr = &(kernel_ver[0]);
    for(i=0;i<strlen(kernel_ver);i++) {
        if(kernel_ver[i]>='0' && kernel_ver[i]<='9') {
            ptr = &(kernel_ver[i]);
            break;
        }
    }
    print_verinfo(info, &len,  "Kernel Ver. ", ptr);
    property_get("ro.build.version.release", val, "unknown");
    print_verinfo(info, &len,  "Android Ver.", val);
    property_get("ro.mediatek.version.release", val, "unknown");
    print_verinfo(info, &len,  "SW Ver.     ", val);
	sprintf(return_data.version.sw_ver,"%s", val);
	property_get("ro.custom.build.version",val,"unknown");
	print_verinfo(info, &len,  "Custom Build Verno.", val);

    return 0;
}

static int miniui_test(int chosen_item)
{
    bool   quit;
    struct itemview iv;
    struct textview vi;
    struct imageview imv;
    struct paintview pv;
    text_t title;
    text_t text;
    text_t rbtn;

    item_t items[] = {
        item(ITEM_TEST_PASS,  "Test Pass"),
        item(ITEM_TEST_FAIL,  "Test Fail"),
        item(ITEM_BACK     ,  "Back"),
        item(-1            ,  NULL),
    };

    ctext_t ctexts[] = {
        { "AAA", COLOR_YELLOW, 20, 40 },
        { "BBB", COLOR_GREEN, 20, 80 },
        { "CCC", COLOR_RED, 20, 120 },
        { "DDD", COLOR_BLUE, 20, 160 },
    };

    point_t my_point[] = {
        {100, 200, 10, COLOR_PINK },
        {240, 300, 10, COLOR_RED },
        {340, 400, 10, COLOR_GREEN },
    };

    line_t my_line[] = {
        {0, 400, 480, 400, 5, COLOR_PINK },
        {0, 200, 480, 200, 5, COLOR_RED },
        {0, 600, 480, 600, 5, COLOR_GREEN },
    };

    circle_t my_circle[] = {
        {240, 400, 30, COLOR_PINK },
        {240, 400, 50, COLOR_RED },
        {240, 400, 80, COLOR_GREEN },
        {240, 400, 100, COLOR_BLUE },
        {240, 400, 150, COLOR_WHITE },
    };

    switch (chosen_item) {
    case ITEM_MUI_ITEMVIEW:
        LOGD(TAG "ITEM_MUI_ITEMVIEW");
        quit = false;
        init_text(&title, "ItemView Test", COLOR_YELLOW);
        ui_init_itemview(&iv);
        iv.set_title(&iv, &title);
        iv.set_items(&iv, items, 0);
        do {
            chosen_item = iv.run(&iv, &quit);
            switch (chosen_item) {
            case ITEM_TEST_PASS:
                LOGD(TAG "ITEM_TEST_PASS");
                init_text(&text, "Test Pass!!!\nAAA\nBBB\nCCC\n", COLOR_YELLOW);
                iv.set_text(&iv, &text);
                iv.redraw(&iv);
                break;
            case ITEM_TEST_FAIL:
                LOGD(TAG "ITEM_TEST_FAIL");
                init_text(&text, "Test Fail!!!\nAAA\nBBB\nCCC\n", COLOR_YELLOW);
                iv.set_text(&iv, &text);
                iv.redraw(&iv);
                break;
            case ITEM_BACK:
                LOGD(TAG "ITEM_BACK");
                quit = true;
                break;
            }
        } while (!quit);
        break;
    case ITEM_MUI_TEXTVIEW:
        LOGD(TAG "ITEM_MUI_TEXTVIEW");
        init_text(&title, "TextView Test", COLOR_YELLOW);
        init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
        ui_init_textview(&vi, textview_key_handler, &vi);
        vi.set_title(&vi, &title);
        vi.set_btn(&vi, NULL, NULL, &rbtn);
        vi.set_ctext(&vi, ctexts, 4);
        vi.run(&vi);
        break;
    case ITEM_MUI_IMAGEVIEW:
        LOGD(TAG "ITEM_MUI_IMAGEVIEW");
        init_text(&title, "ImageView Test", COLOR_YELLOW);
        init_text(&rbtn, "Back", COLOR_YELLOW);
        ui_init_imageview(&imv, imageview_key_handler, &imv);
        imv.set_title(&imv, &title);
        imv.set_image(&imv, "/res/images/mtk_factory.png", 0, 0);
        imv.run(&imv);
        break;
    case ITEM_MUI_PAINTVIEW:
        LOGD(TAG "ITEM_MUI_PAINTVIEW");
        /* Initial the title info. */
        init_text(&title, "PaintView Test", COLOR_YELLOW);
        init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
        /* Initial the paintview function pointers */
        ui_init_paintview(&pv, paintview_key_handler, &pv);
        pv.set_title(&pv, &title);
        pv.set_point(&pv, my_point, 3);
        pv.set_line(&pv, my_line, 3);
        pv.set_circle(&pv, my_circle, 5);
        pv.set_btn(&pv, NULL, NULL, &rbtn);
        pv.run(&pv);
        break;
    }
    return 0;
}

static int item_test_report(item_t *items, char *buf, int bufsz)
{
    int    num;
    int    chosen_item;
    bool   quit;
    struct itemview triv; /* test report item view */
    item_t rpt_items[ITEM_MAX_IDS + 1];
    text_t tr_title;
    struct ftm_param param;

    init_text(&tr_title, uistr_test_report, COLOR_YELLOW);

    ui_init_itemview(&triv);

    quit = false;
    memset(rpt_items, 0, sizeof(item_t) * (ITEM_MAX_IDS + 1));
    num = create_report(items, rpt_items, ITEM_MAX_IDS, buf, bufsz);
    triv.set_title(&triv, &tr_title);
    triv.set_items(&triv, rpt_items, 0);
    while (!quit) {
        chosen_item = triv.run(&triv, &quit);
        if (chosen_item == ITEM_MAX_IDS)
            break;
        param.name = get_item_name(items, chosen_item);
        ftm_entry(chosen_item, &param);
        create_report(items, rpt_items, ITEM_MAX_IDS, buf, bufsz);
    }
    return 0;
}

static int full_test_mode(char *buf, int bufsz)
{
    int i = 0;
    item_t *items;
    struct ftm_module *mod;
    struct ftm_param param;
    //handle of testreport.log
    FILE *fp = NULL;
    //add for saving test report
	item_t rpt_items[ITEM_MAX_IDS + 1];
    int stopmode = 0;
    char *stopprop = ftm_get_prop("FTM.FailStop");

    if (stopprop && !strncasecmp(stopprop, "yes", strlen("yes")))
        stopmode = 1;

    LOGD(TAG "full_test_mode: %d", stopmode);

//    items = get_manual_item_list();

    items = ftm_cust_items;

	LOGD(TAG "get_manual_item_list end");

    while (items[i].name)
	{
		LOGD(TAG "name = %s,id = %d,mode=%d",items[i].name,items[i].id,items[i].mode);
		if(items[i].mode != FTM_AUTO_ITEM)
		{
			LOGD(TAG "%s:%d", items[i].name, items[i].id);

			switch (items[i].id)
			{
			case ITEM_IDLE: /* skip items */
				break;
			case ITEM_REPORT:
				item_test_report(items, buf, bufsz);
				break;
			default:
				mod = ftm_get_module(items[i].id);
				if (mod && mod->visible)
				{
					param.name = items[i].name;
					ftm_entry(items[i].id, &param);
					if (stopmode && mod->test_result != FTM_TEST_PASS)
						continue;
				}
				break;
			}
		}
		i++;
	}

    //add for saving test report
    fp = open_file(TEST_REPORT_SAVE_FILE);
	if(fp != NULL)
	{
		create_report(items, rpt_items, ITEM_MAX_IDS , buf, bufsz);
        write_test_report(rpt_items, fp);
		fclose(fp);
	}
    //add for saving test report

    return 0;
}

static int auto_test_mode(char *buf, int bufsz)
{
    int i = 0;
    item_t *items, *cust_items;
    struct ftm_module *mod;
    struct ftm_param param;
    //handle of testreport.log
    FILE *fp = NULL;
    //add for saving test report
    item_t rpt_items[ITEM_MAX_IDS + 1];
    int stopmode = 0;
    char *stopprop = ftm_get_prop("FTM.FailStop");

    if (stopprop && !strncasecmp(stopprop, "yes", strlen("yes")))
        stopmode = 1;

    LOGD(TAG "auto_test_mode: %d", stopmode);

    items = get_auto_item_list();
    //add for saving test report
    cust_items = get_item_list();
    memset(rpt_items, 0, sizeof(item_t) * (ITEM_MAX_IDS + 1));

    while (items[i].name) {
        LOGD(TAG "%s:%d", items[i].name, items[i].id);
        switch (items[i].id) {
        case ITEM_IDLE: /* skip items */
            break;
        case ITEM_REPORT:
            item_test_report(items, buf, bufsz);
            break;
        default:
            mod = ftm_get_module(items[i].id);
            if (mod && mod->visible) {
                param.name = items[i].name;
                ftm_entry(items[i].id, &param);
                if (stopmode && mod->test_result != FTM_TEST_PASS)
                    continue;
            }
            break;
        }
        i++;
    }

    //add for saving testreport.log
    fp = open_file(TEST_REPORT_SAVE_FILE);
	if(fp != NULL)
	{
		create_report(cust_items, rpt_items, ITEM_MAX_IDS , buf, bufsz);
//        write_test_report(rpt_items, fp);
		fclose(fp);
	}
    //add for saving testreport.log

    return 0;
}

static int item_test_mode(char *buf, int bufsz)
{
    int chosen_item = 0;
    bool exit = false;
    struct itemview itv;  /* item test menu */
    struct ftm_param param;
    text_t  title;
    item_t *items;
    //handle of testreport.log
	FILE *fp = NULL;
    //add for saving test report
	item_t rpt_items[ITEM_MAX_IDS + 1];

    LOGD(TAG "item_test_mode");

    items = get_item_list();

    ui_init_itemview(&itv);
    init_text(&title, uistr_item_test, COLOR_YELLOW);

    itv.set_title(&itv, &title);
    itv.set_items(&itv, items, 0);

    while (1) {
        chosen_item = itv.run(&itv, &exit);
        if (exit == true)
            break;
        switch (chosen_item) {
        case ITEM_REPORT:
            item_test_report(items, buf, bufsz);
            break;
        default:
            param.name = get_item_name(items, chosen_item);
            ftm_entry(chosen_item, &param);
            LOGD(TAG "ITEM TEST ftm_entry before");
            //add for saving test report
			fp = open_file(TEST_REPORT_SAVE_FILE);
			if(fp != NULL)
			{
			    create_report(items, rpt_items, ITEM_MAX_IDS , buf, bufsz);
				fclose(fp);
			}
            //add for saving test report
            break;
        }
    }
    return 0;
}

static int debug_test_mode(char *buf, int bufsz)
{
    int chosen_item = 0;
    bool exit = false;
    struct itemview itv;  /* item test menu */
    struct ftm_param param;
    text_t  title;
    item_t *items;

    LOGD(TAG "debug_test_mode");

    items = get_debug_item_list();

    ui_init_itemview(&itv);
    init_text(&title, uistr_item_test, COLOR_YELLOW);

    itv.set_title(&itv, &title);
    itv.set_items(&itv, items, 0);

    while (1) {
        chosen_item = itv.run(&itv, &exit);
        if (exit == true)
            break;
        switch (chosen_item) {
        default:
			LOGD(TAG "chosen_item=%d",chosen_item);
            param.name = get_item_name(items, chosen_item);
            ftm_debug_entry(chosen_item, &param);
            break;
        }
    }
    return 0;
}


static int test_module()
{

		int arg = 0;
		int id = -1;
		int write_len = 0;
		struct ftm_param param;
		char test_result[128] = {0};
		item_t *items;
		struct ftm_module *mod;
		char *prop_name = NULL;
		char *prop_val = NULL;
		char result[3][16] = {"not test\r\n", "pass\r\n", "fail\r\n" };
		char temp_at_command[128] = {0};
		int i = 0;
		char p[16] = {0};
		items = get_item_list();

		char buf[8] = {0};
        int at_command_len = 0;
		strcpy(buf, "quit");

		while(1)
		{
		    pthread_mutex_lock (&at_command_mutex);
            at_command_len = strlen(at_command);
            strcpy(p, at_command);
            memset(at_command, 0, sizeof(at_command));
			if(at_command_len <= 3){
		    pthread_mutex_unlock (&at_command_mutex);
				continue;
            }

            if(!memcmp(buf, p, 4))
            {
		    pthread_mutex_unlock (&at_command_mutex);
//                memset(at_command, 0, sizeof(at_command));
                break;
            }
			LOGD(TAG "at command:%d, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, %s,%d \n",
				strlen(at_command), at_command[0], at_command[1], at_command[2], at_command[3],
				buf[0], buf[1], buf[2], buf[3], at_command, strcmp(buf, at_command));
		    pthread_mutex_unlock (&at_command_mutex);

			id = get_item_id(pc_control_items, p);
			LOGD(TAG "test item id is %d\n", id);
			if(id >= 0)
			{
				LOGD(TAG "before get_item_name");
				param.name = get_item_name(items, id);
				if(param.name == NULL)
				{
					param.name = get_item_name(ftm_menu_items, id);
				}
				LOGD(TAG "after get_item_name is %s\n", param.name);
				if(param.name != NULL)
				{
					mod = ftm_get_module(id);
					if (mod && mod->visible)
					{
						ftm_entry(id, &param);
                        LOGD(TAG "test_result:%d\n", mod->test_result);
                        if(mod->test_result >= FTM_TEST_MAX)
                        {
                            strcpy(test_result, result[0]);
                        }
                        else
                        {
						strcpy(test_result, result[mod->test_result]);
					}
				}
			}
			}
			else
			{
				strcpy(test_result, "Cannot find the module!\r\n");
			}
            LOGD(TAG "before write data to pc\n");
            while(usb_status != 1)
            {
                sleep(1);
            }
			write_len = write(usb_com_port, test_result, strlen(test_result));
            LOGD(TAG "after write data to pc\n");
			if(write_len != strlen(test_result))
			{
				LOGD(TAG "write data to pc fail\n");
			}
//			memset(at_command, 0, sizeof(at_command));

		}
		LOGD(TAG "test_result is %s, the %s\n", test_result, test_data);
		return 0;
}

static int is_pc_control(int fd)
{
    struct timeval startime, endtime;
	double time_use = 0;
	int read_from_usb = 0;
	char USB_read_buffer[BUFSZ] = {0};
	bool pc_control = false;
	double max_time = 1500000;
	gettimeofday(&startime, 0);
	LOGD(TAG "time_use = %lf\n", time_use);

	if(usb_com_port == -1)
	{
		return 0;
	}

	while(time_use < max_time)
	{
//		LOGD(TAG "time_use = %lf\n", time_use);
        if(usb_com_port != -1)
        {
            read_from_usb = read(usb_com_port, USB_read_buffer, sizeof(USB_read_buffer));
//			LOGD(TAG "read_from_usb = %d, USB_read_buffer = %s\n", fd, USB_read_buffer);
		}
		if(read_from_usb == -1)
		{
			gettimeofday(&endtime, 0);
			time_use = 1000000 * (endtime.tv_sec - startime.tv_sec) +
				endtime.tv_usec - startime.tv_usec;
            continue;
		}
		else if (read_from_usb > 0)
		{
			if(strncmp(USB_read_buffer, START, strlen(START)) == 0)
			{
                LOGD(TAG "start\n");
                int len = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
				if(len != strlen("pass\r\n"))
				{
					LOGD(TAG "write pass fail in is_pc_control");
				}
				else
				{
					LOGD(TAG "write pass in is_pc_control");
					pc_control = true;
                    usb_status = 1;
					break;
				}
			}
		}
	}
//	write(fd,"ok",strlen("ok"));
	return pc_control;
}

char ** trans_verinfo(const char *str, int *line)
{
    char **pstrs = (char**)malloc(g_nr_lines * sizeof(char*));
    int  len     = strlen(str) + 1;
    int  row     = 0;
    const char *start  = str;
    const char *end    = str;

    if (!pstrs) {
        LOGE("In factory mode: malloc failed\n");
        return NULL;
    }

    while (len--) {
        if ('\n' == *end) {
            pstrs[row] = (char*)malloc((end - start + 1) * sizeof(char));

            if (!pstrs[row]) {
                LOGE("In factory mode: malloc failed\n");
                return NULL;
            }

            strncpy(pstrs[row], start, end - start);
            pstrs[row][end - start] = '\0';
            start = end + 1;
            row++;
        }
        end++;
    }

    *line = row;
    return pstrs;
}

void tear_down(char **pstr, int row)
{
    int i;

    for (i = 0; i < row; i++) {
        if (pstr[i]) {
            free(pstr[i]);
            pstr[i] = NULL;
        }
    }
	
    if (pstr) {
        free(pstr);
        pstr = NULL;
    }
}

//MTKBEGIN  [mtk0625][DualTalk]
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
#define EXT_MD_IOC_MAGIC			'E'
#define EXT_MD_IOCTL_LET_MD_GO		_IO(EXT_MD_IOC_MAGIC, 1)
#define EXT_MD_IOCTL_REQUEST_RESET	_IO(EXT_MD_IOC_MAGIC, 2)
#define EXT_MD_IOCTL_POWER_ON_HOLD	_IO(EXT_MD_IOC_MAGIC, 3)

int boot_modem(int is_reset)
{
    LOGD(TAG "%s\n", __FUNCTION__);

	int ret;
	int ext_md_ctl_n0, ext_md_ctl_n1;

	ext_md_ctl_n0 = open("/dev/ext_md_ctl0", O_RDWR);
	if(ext_md_ctl_n0 <0) {
        LOGD(TAG "open ext_md_ctl0 fail");
		return	ext_md_ctl_n0;
	}
	ret = ioctl(ext_md_ctl_n0, EXT_MD_IOCTL_POWER_ON_HOLD, NULL);
	if (ret < 0) {
        LOGD(TAG "power on modem fail");
		return	ret;
	}

	ext_md_ctl_n1 = open("/dev/ext_md_ctl1", O_RDWR);
	if(ext_md_ctl_n1 <0) {
        LOGD(TAG "open ext_md_ctl_n1 fail");
		return	ext_md_ctl_n1;
	}
	ret = ioctl(ext_md_ctl_n1, EXT_MD_IOCTL_LET_MD_GO, NULL);
	if (ret < 0) {
        LOGD(TAG "EXT_MD_IOCTL_LET_MD_GO fail");
		return	ret;
	}

	return	ret;
}
#endif  /* MTK_DT_SUPPORT */
//MTKEND    [mtk80625][DualTalk]

void *read_data_thread_callback(int fd)
{
	int read_from_usb = 0;
	char USB_read_buffer[BUFSZ] = {0};
	bool exit = false;

	while(1)
    {
        if(usb_plug_in == 0)
        {
            LOGD("FACTORY.C usb_plug_in == 0\n");
            continue;
        }
        else if(is_USB_State_PlugIn())
        {
            open_usb();
        }
		if(usb_com_port != -1)
    	{
            read_from_usb = read_a_line_test(usb_com_port, USB_read_buffer, sizeof(USB_read_buffer));
		}
        else
    	{
            continue;
		}
		if(read_from_usb == -1)
		{
            continue;
		}
		else if(read_from_usb > 0)
		{
            LOGD(TAG "read from usb is %s\n",USB_read_buffer);
			if(read_from_usb > 3)
			{
			USB_read_buffer[read_from_usb-1] = 0;
            pthread_mutex_lock (&at_command_mutex);
			LOGD(TAG "-----------> AT COMMAND = %s\n", at_command);
            pthread_mutex_unlock (&at_command_mutex);

		    if(strncmp(USB_read_buffer, STOP, strlen(STOP)) == 0){
                LOGD(TAG "stop\n");
                pthread_mutex_lock (&at_command_mutex);
				strcpy(at_command, "quit");
				LOGD(TAG "compare at_command and quit:%d\n", strncmp(at_command, "quit", strlen("quit")));
                pthread_mutex_unlock (&at_command_mutex);
                int n = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
				if(n != strlen("pass\r\n"))
			    {
				    LOGD(TAG "Write stop pass fail\n");
			    }
				else
				{
					LOGD(TAG "Write stop pass successfully\n");
				}
				close(usb_com_port);
				break;
		    }
			else if(strncmp(USB_read_buffer, START, strlen(START)) == 0)
			{
                LOGD(TAG "start\n");
                int n = write(usb_com_port, "pass\r\n", strlen("pass\r\n"));
                usb_status = 1;
				if(n != strlen("pass\r\n"))
			    {
				    LOGD(TAG "Write start pass fail\n");
			    }
				else
				{
					 LOGD(TAG "Write start pass successfully\n");
				}
			}
			else if(strncmp(USB_read_buffer, REQUEST_DATA, strlen(REQUEST_DATA)) == 0)
			{

				LOGD(TAG "name:%s, mac:%s, rssi:%d, channel:%d ,rate%d\n", return_data.wifi.wifi_name,
					return_data.wifi.wifi_mac, return_data.wifi.wifi_rssi,
					return_data.wifi.channel, return_data.wifi.rate);
				int i = 0;
				for(i = 0 ; i < return_data.bt.num; i++)
				{
				LOGD(TAG "bt_mac:%s, rssi:%d\n", return_data.bt.bt[i].bt_mac, return_data.bt.bt[i].bt_rssi);
				}
				char temp_buf[2048] = {0};
				memcpy(temp_buf, &return_data, sizeof(return_data));
				strcpy(temp_buf+sizeof(return_data), "\r\n");
			    int n = write(usb_com_port, temp_buf, sizeof(temp_buf));
			   // int n = write(fd, return_data.wifi.wifi_name, sizeof(return_data.wifi.wifi_name));
			    if(n != sizeof(temp_buf))
			    {
				    LOGD(TAG "Write test_data fail,%d\n",  sizeof(temp_buf));
			    }
				else
				{
					 LOGD(TAG "Write test_data successfully,%d\n", sizeof(temp_buf));
				}
			}
			else if(strncmp(USB_read_buffer, VERSION, strlen(VERSION))==0)
			{
				display_version();
				int n = write(usb_com_port, "pass\r\n", sizeof("pass\r\n"));
				if(n != sizeof("pass\r\n"))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}
			}
            else if(strncmp(USB_read_buffer, READ_BARCODE, strlen(READ_BARCODE))==0)
            {
                int ccci_handle = -1;
                int i = 0;
                char result[BUFSZ] = {0};
                char dev_node[32] = {0};
                if(is_support_modem(1))
                {
	                LOGD(TAG "MTK_ENABLE_MD1\n");
                    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS1));
	                //ccci_handle = openDevice();
                }
                else if(is_support_modem(2))
                {
                    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
                    //ccci_handle = openDeviceWithDeviceName("/dev/ccci2_tty0");
                }
                else if(is_support_modem(5))
                {
                    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS5));
                }
                ccci_handle = openDeviceWithDeviceName(dev_node);
                if(-1 == ccci_handle) 
                {
            	    LOGD(TAG "Fail to open ttyMT0 interface\n");
		                return 0;
                }
	            for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
                send_at (ccci_handle, "AT\r\n");
            	wait4_ack (ccci_handle, NULL, 3000);
                getBarcode(ccci_handle,result);
                char *ptr = strchr(result, '\"');
                if (ptr != NULL) 
                {
                    *ptr = 0;
                    if(strlen(result) <= 1)
                    {
                        strcpy(result, "unknown\r\n");
                    }
                }
                else
                {
                    strcpy(result, "fail\r\n");
                }
                int n = write(usb_com_port, result, sizeof(result));
				if(n != sizeof(result))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}
                if(-1 != ccci_handle)
                {
                    closeDevice(ccci_handle);
                }
            }
            else if(strncmp(USB_read_buffer, WRITE_BARCODE, strlen(WRITE_BARCODE))==0)
            {
                LOGD(TAG "Entry write barcode!\n");
                int ccci_handle = -1;
                int i = 0;
                int result = -1;
                char *barcode = strchr(USB_read_buffer, '=');
                char return_result[16] = {0};
                char dev_node[32] = {0};
                if(barcode == NULL)
                {
                    LOGD(TAG "barcode is null!\n");               
                }
                else
                {
                    barcode++;
                    LOGD(TAG "%s\n", barcode);
                    if(is_support_modem(1))
                    {
    	                LOGD(TAG "MTK_ENABLE_MD1\n");
                        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS1));
                    }
                    else if(is_support_modem(2))
                    {
                        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
                    }
                    else if(is_support_modem(5))
                    {
                        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS5));
                    }
                    ccci_handle = openDeviceWithDeviceName(dev_node);
	                if(-1 == ccci_handle) 
                    {
 		                    LOGD(TAG "Fail to open CCCI interface\n");
		                    return 0;
                    }
	                for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	                send_at (ccci_handle, "AT\r\n");
	                wait4_ack (ccci_handle, NULL, 3000);

                    result = write_barcode(ccci_handle, barcode);
                }
                if(result == 0)
                {
                    strncpy(return_result, "pass\r\n", strlen("pass\r\n"));
				}
				else
				{
                    strncpy(return_result, "fail\r\n", strlen("fail\r\n"));
				}
                int n = write(usb_com_port, return_result, strlen(return_result));
                
                if(n != strlen(return_result))
				{
					LOGD(TAG "Write test_data in version fail\n");
				}
				else
				{
					LOGD(TAG "Write test_data in version successfully\n");
				}
               
                if(-1 != ccci_handle)
                {
                    closeDevice(ccci_handle);
                }
            }
			else
			{
               LOGD(TAG "module\n");
               //test_module(fd, USB_read_buffer);
               int ret = 0;
			   int id = -1;
			   ret = get_AT_command(USB_read_buffer);
			   if(ret == 1)
			   {
					id = get_item_id(pc_control_items, USB_read_buffer);
					switch(id)
					{
						case ITEM_VIBRATOR:
							#ifdef FEATURE_FTM_VIBRATOR
							vibrator_test_exit = true;
							#endif
							break;
						case ITEM_LED:
							#ifdef FEATURE_FTM_LED
							keypadled_test_exit = true;
							led_test_exit = true;
							#endif
							break;
						case ITEM_MICBIAS:
							#ifdef FEATURE_FTM_AUDIO
							bMicbias_exit = true;
							#endif
							break;
						default:
							break;
					}
			   }
			   else if(ret ==2)
			   {

					int n = write(usb_com_port, "pass\r\n", sizeof("pass\r\n"));
					if(n != sizeof("pass\r\n"))
					{
						LOGD(TAG "Write test_data in version fail\n");
					}
					else
					{
						LOGD(TAG "Write test_data in version successfully\n");
					}

			   }
		    }
				}

			LOGD(TAG "BOOL IS %d\n", exit);
		}
	}//while

	pthread_exit(NULL);
	return NULL;
}


int display_version()
{
   char *buf = NULL;
   struct textview vi;	 /* version info */
   text_t vi_title;
   int nr_line;
   text_t info;
   int avail_lines = 0;
   text_t rbtn;
   buf = malloc(BUFSZ);
   init_text(&vi_title, uistr_version, COLOR_YELLOW);
   init_text(&info, buf, COLOR_YELLOW);
   init_text(&info, buf, COLOR_YELLOW);

   avail_lines = get_avail_textline();
   init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
   ui_init_textview(&vi, textview_key_handler, &vi);
   vi.set_btn(&vi, NULL, NULL, &rbtn);
   create_verinfo(buf, BUFSZ);
   LOGE("after create_verinfo");
   vi.set_title(&vi, &vi_title);
   vi.set_text(&vi, &info);
   vi.m_pstr = trans_verinfo(info.string, &nr_line);
   vi.m_nr_lines = g_nr_lines;
   LOGE("g_nr_lines is %d, avail_lines is %d\n", g_nr_lines, avail_lines);
   vi.m_start = 0;
   vi.m_end = (nr_line < avail_lines ? nr_line : avail_lines);
   LOGE("vi.m_end is %d\n", vi.m_end);
   vi.redraw(&vi);
   LOGE("Before tear_down\n");
   tear_down(vi.m_pstr, nr_line);
   if (buf)
	    free(buf);
   LOGE("The version is %s\n", test_data);
   return 0;
}

static int pc_control_mode(int fd)
{
	LOGD(TAG "CALL pc_control_mode1");
	test_module();
	return 0;
}

int main(int argc, char **argv)
{
	int exit = 0;
	int    sel=0;
	int nr_line=0;
	int avail_lines = 0;
	bool   quit=false;
	char  *buf = NULL;

	//int n;
    struct ftm_param param;
    struct itemview fiv;  /* factory item menu */
    struct itemview miv;  /* mini-ui item menu */
    struct textview vi;   /* version info */
    //struct itemview ate;  /* ATE factory mode*/
    item_t *items;
    text_t ftm_title;
    int bootMode;
	int g_fd_atcmd = -1, g_fd_uart = -1;
    int g_hUsbComPort = -1;
    //add for saving test report
	item_t rpt_items[ITEM_MAX_IDS + 1];

    text_t vi_title;
    text_t ate_title;
    text_t rbtn;
    text_t info;
	pthread_t read_thread;

    ui_init();

    /* CHECKME! should add this fuctnion to avoid UI not displayed */
 	//ui_print("factory mode\n");
    show_slash_screen(uistr_factory_mode, 1000);

    bootMode = getBootMode();

    if(ATE_FACTORY_BOOT == bootMode)
    {
        ui_print("Enter ATE factory mode...\n");

        ate_signal();

        while(1){}
    }
    else if(FACTORY_BOOT == bootMode)
    {
		buf = malloc(BUFSZ);
		if (NULL == buf)
		{
		    ui_print("Fail to get memory!\n");
		}

		ftm_init();
		avail_lines = get_avail_textline();
		if (!read_config(FTM_CUST_FILE1))
			read_config(FTM_CUST_FILE2);

#if 0
		if(-1 == COM_Init (&g_fd_atcmd, &g_fd_uart, &g_hUsbComPort))
		{
			LOGE(TAG "COM_Init init fail!\n");
			//		return 0;
		}
		usb_com_port = g_hUsbComPort;

		if(g_fd_atcmd != -1)
		{
			close(g_fd_atcmd);
			g_fd_atcmd = -1;
		}

#endif

		usb_com_port = open("dev/ttyGS0", O_RDWR | O_NOCTTY | O_NDELAY);

        LOGD(TAG "Open USB dev/ttyGS0 %s.\n", (-1==usb_com_port)? "failed":"success");

		if(is_pc_control(usb_com_port))
		{
			pthread_create(&read_thread, NULL, read_data_thread_callback, usb_com_port);
			LOGD(TAG "after create pthread");
			status = 1;
			pc_control_mode(usb_com_port);
			LOGD(TAG "pc control stops in if()!\n");
			status = 0;
		}

		LOGD(TAG "pc control stops!\n");

		ui_init_itemview(&fiv);
		ui_init_itemview(&miv);
		ui_init_textview(&vi, textview_key_handler, &vi);

		init_text(&ftm_title, uistr_factory_mode, COLOR_YELLOW);
		init_text(&vi_title, uistr_version, COLOR_YELLOW);
		init_text(&rbtn, uistr_key_back, COLOR_YELLOW);
		init_text(&info, buf, COLOR_YELLOW);

		items = ftm_menu_items;
		fiv.set_title(&fiv, &ftm_title);
		fiv.set_items(&fiv, items, 0);
		vi.set_btn(&vi, NULL, NULL, &rbtn);
 
#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
		boot_modem(0);
#endif  /* MTK_DT_SUPPORT */ 
		
		while (!exit) 
		{
			int chosen_item = fiv.run(&fiv, NULL);
			switch (chosen_item) 
			{
			case ITEM_MUI_TEST:
				quit = false;
				miv.set_title(&miv, &miniui_title);
				miv.set_items(&miv, miniui_item, 0);
				while (!quit)
					miniui_test(miv.run(&miv, &quit));
				break;
			case ITEM_FULL_TEST:
				full_test_mode(buf, BUFSZ);
				break;
			case ITEM_AUTO_TEST:
				auto_test_mode(buf, BUFSZ);
				item_test_report(get_auto_item_list(), buf, BUFSZ);
				//add for saving test report 
		        memset(rpt_items, 0, sizeof(item_t) * (ITEM_MAX_IDS + 1));
				create_report(get_item_list(), rpt_items, ITEM_MAX_IDS , buf, BUFSZ);
				//add for saving test report
				break;
			case ITEM_ITEM_TEST:
				item_test_mode(buf, BUFSZ);
				break;
			case ITEM_DEBUG_TEST:
				debug_test_mode(buf, BUFSZ);
				break;
			case ITEM_REPORT:
				item_test_report(get_item_list(), buf, BUFSZ);
				break;
			case ITEM_VERSION:
				create_verinfo(buf, BUFSZ);
				vi.set_title(&vi, &vi_title);
				vi.set_text(&vi, &info);
				vi.m_pstr = trans_verinfo(info.string, &nr_line);
				vi.m_nr_lines = g_nr_lines;
				LOGE("g_nr_lines is %d, avail_lines is %d\n", g_nr_lines, avail_lines);
				vi.m_start = 0;
				vi.m_end = (nr_line < avail_lines ? nr_line : avail_lines);
				LOGE("vi.m_end is %d\n", vi.m_end);
				vi.run(&vi);
				tear_down(vi.m_pstr, g_nr_lines);
				break;
			case ITEM_REBOOT:
				exit = 1;
				fiv.exit(&fiv);
				break;
			default:
				param.name = get_item_name(items, chosen_item);
				ftm_entry(chosen_item, &param);
				break;
			}
		}//end while

		if (buf)
			free(buf);

		ui_printf("Entering factory reset mode...\n");
		ui_printf("Rebooting...\n");
		sync();
		reboot(RB_AUTOBOOT);

		return EXIT_SUCCESS;
	}
	else
	{
		LOGE(TAG "Unsupported Factory mode\n");
	}
	
	return EXIT_SUCCESS;
}
