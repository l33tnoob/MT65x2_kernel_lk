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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_BATTERY

#define TAG                 "[Batt&Charging] "

#define BUF_LEN 16
#define BATT_VOLT_FILE "/sys/class/power_supply/battery/batt_vol"
#define BATT_TEMP_FILE "/sys/class/power_supply/battery/batt_temp"
#define CHARGER_STATUS_FILE "/sys/class/power_supply/battery/status"
#define AC_ONLINE_FILE "/sys/class/power_supply/ac/online"
#define USB_ONLINE_FILE "/sys/class/power_supply/usb/online"

/* IOCTO */
#define ADC_CHANNEL_READ 		_IOW('k', 4, int)
#define BAT_STATUS_READ 		_IOW('k', 5, int)
#define Set_Charger_Current _IOW('k', 6, int)
#define AVG_BAT_SEN_READ _IOW('k', 7, int)
#define FGADC_CURRENT_READ _IOW('k', 8, int)
#define BAT_THREAD_CTRL _IOW('k', 9, int)

/* ADC CHannel */
#define ADC_I_SEN 0
#define ADC_BAT_SEN 1
#define ADC_CHARGER 3

#ifdef BATTERY_TYPE_B61UN
#define ADC_BAT_TEMP 5 
#endif

#ifdef BATTERY_TYPE_BLP509
#define ADC_BAT_TEMP 8
#endif

#ifdef BATTERY_TYPE_Z3
#define ADC_BAT_TEMP 30 // magic number
#endif

#ifdef FEATURE_FTM_PMIC_632X
#define ADC_BAT_FG_CURRENT 66 // magic number
#endif

/* Charging Current Setting */
#if defined(BATTERY_TYPE_Z3)
typedef enum
{
	Cust_CC_70MA = 0,
	Cust_CC_200MA,
	Cust_CC_400MA,
	Cust_CC_450MA,
	Cust_CC_550MA,
	Cust_CC_650MA,
	Cust_CC_700MA,
	Cust_CC_800MA,
	#if defined(FEATURE_FTM_PMIC_632X)
	Cust_CC_900MA,
	Cust_CC_1000MA,
	Cust_CC_1100MA,
	Cust_CC_1200MA,
	Cust_CC_1300MA,
	Cust_CC_1400MA,
	Cust_CC_1500MA,
	Cust_CC_1600MA,
	#endif
}cust_charging_current_enum;
#endif

#if !defined(BATTERY_TYPE_Z3)
typedef enum
{
	Cust_CC_50MA = 0,
	Cust_CC_90MA,
	Cust_CC_150MA,
	Cust_CC_225MA,
	Cust_CC_300MA,
	Cust_CC_450MA,
	Cust_CC_650MA,
	Cust_CC_800MA
}cust_charging_current_enum;
#endif


#define USB_CHARGER_CURRENT		Cust_CC_450MA      
#define AC_CHARGER_CURRENT		Cust_CC_650MA

int ADC_COUNT = 5;

enum {
	ITEM_AC_CHARGER,
	ITEM_USB_CHARGER,
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t battery_items[] = {
#if 1
    //auto test
    item(-1, NULL),
#else
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
#endif
};

#if 1
//auto test
#define AUTO_TEST_VBAT_VALUE 3000
#define AUTO_TEST_THD_VALUE 4250
#define AUTO_TEST_VCHR_VALUE 4000
#define AUTO_TEST_VBAT_TEMP_MIN	 5
#define AUTO_TEST_VBAT_TEMP_MAX	 45
#endif


extern sp_ata_data return_data;

struct batteryFTM {
    int    bat_voltage;	/* battery_sense */
	int    charger_voltage;
    int    current_charging; 
	int    bat_temperature;
	int    adc_vbat_3_2;
	int    adc_vbat_4_2;
	int    adc_vbat_current;	
    char    info[1024];
	bool   is_charging;	
	bool   is_calibration;
	bool   charger_exist;
    bool   exit_thd;
    pthread_t    batt_update_thd;
    struct ftm_module    *mod;
    struct textview    tv;
	struct itemview *iv;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

#define mod_to_batteryFTM(p)     (struct batteryFTM*)((char*)(p) + sizeof(struct ftm_module))

/********************************************
*											*
*	Convert ADC voltage to Battery Temprature	*
*											*
********************************************/
#ifdef BATTERY_TYPE_Z3
#define RBAT_PULL_UP_R 			24000
#define RBAT_PULL_UP_VOLT 		2500
#define TBAT_OVER_CRITICAL_LOW 	68237
#else
#define RBAT_PULL_UP_R 			24000
#define RBAT_PULL_UP_VOLT 		2800
#define TBAT_OVER_CRITICAL_LOW 	68237
#endif

typedef struct{
	int BatteryTemp;
	int TemperatureR;
}BATT_TEMPERATURE;

/* convert register to temperature  */
int BattThermistorConverTemp(int Res)
{
	int i=0;
	int RES1=0,RES2=0;
	int TBatt_Value=-200,TMP1=0,TMP2=0;

	#ifdef BATTERY_TYPE_Z3
	BATT_TEMPERATURE Batt_Temperature_Table[] = {
		{-20,68237},
		{-15,53650},
		{-10,42506},
		{ -5,33892},
		{  0,27219},
		{  5,22021},
		{ 10,17926},
		{ 15,14674},
		{ 20,12081},
		{ 25,10000},
		{ 30,8315},
		{ 35,6948},
		{ 40,5834},
		{ 45,4917},
		{ 50,4161},
		{ 55,3535},
		{ 60,3014}
	};
	#else
	BATT_TEMPERATURE Batt_Temperature_Table[] = {
		{-20,483954},
		{-15,360850},
		{-10,271697},
		{ -5,206463},
		{  0,158214},
		{  5,122259},
		{ 10,95227},
		{ 15,74730},
		{ 20,59065},
		{ 25,47000},
		{ 30,37643},
		{ 35,30334},
		{ 40,24591},
		{ 45,20048},
		{ 50,16433},
		{ 55,13539},
		{ 60,11210}
	};
	#endif

	LOGD(TAG "###### %d <-> %d ######\r\n", Batt_Temperature_Table[9].BatteryTemp, Batt_Temperature_Table[9].TemperatureR);
	
	if(Res>=Batt_Temperature_Table[0].TemperatureR)
	{
		#ifdef CONFIG_DEBUG_MSG_NO_BQ27500
		printk("Res>=%d\n", Batt_Temperature_Table[0].TemperatureR);
		#endif
		TBatt_Value = -20;
	}
	else if(Res<=Batt_Temperature_Table[16].TemperatureR)
	{
		#ifdef CONFIG_DEBUG_MSG_NO_BQ27500
		printk("Res<=%d\n", Batt_Temperature_Table[16].TemperatureR);
		#endif
		TBatt_Value = 60;
	}
	else
	{
		RES1=Batt_Temperature_Table[0].TemperatureR;
		TMP1=Batt_Temperature_Table[0].BatteryTemp;

		for(i=0;i<=16;i++)
		{
			if(Res>=Batt_Temperature_Table[i].TemperatureR)
			{
				RES2=Batt_Temperature_Table[i].TemperatureR;
				TMP2=Batt_Temperature_Table[i].BatteryTemp;
				break;
			}
			else
			{
				RES1=Batt_Temperature_Table[i].TemperatureR;
				TMP1=Batt_Temperature_Table[i].BatteryTemp;
			}
		}
		
		TBatt_Value = (((Res-RES2)*TMP1)+((RES1-Res)*TMP2))/(RES1-RES2);
	}

	#ifdef CONFIG_DEBUG_MSG_NO_BQ27500
	printk("BattThermistorConverTemp() : TBatt_Value = %d\n",TBatt_Value);
	#endif

	return TBatt_Value;	
}

/* convert ADC_bat_temp_volt to register */
int BattVoltToTemp(int dwVolt)
{
	int TRes;
	int dwVCriBat = (TBAT_OVER_CRITICAL_LOW*RBAT_PULL_UP_VOLT)/(TBAT_OVER_CRITICAL_LOW+RBAT_PULL_UP_R); //~2000mV
	int sBaTTMP = -100;

	if(dwVolt > dwVCriBat)
		TRes = TBAT_OVER_CRITICAL_LOW;
	else
		TRes = (RBAT_PULL_UP_R*dwVolt)/(RBAT_PULL_UP_VOLT-dwVolt);		

	/* convert register to temperature */
	sBaTTMP = BattThermistorConverTemp(TRes);

	//#ifdef CONFIG_DEBUG_MSG_NO_BQ27500
	LOGD(TAG "ft-BattVoltToTemp() : TBAT_OVER_CRITICAL_LOW = %d\n", TBAT_OVER_CRITICAL_LOW);
	LOGD(TAG "ft-BattVoltToTemp() : RBAT_PULL_UP_VOLT = %d\n", RBAT_PULL_UP_VOLT);
	LOGD(TAG "ft-BattVoltToTemp() : dwVolt = %d\n", dwVolt);
	LOGD(TAG "ft-BattVoltToTemp() : TRes = %d\n", TRes);
	LOGD(TAG "ft-BattVoltToTemp() : sBaTTMP = %d\n", sBaTTMP);
	//#endif
	
	return sBaTTMP;
}

/********************************************
*											*
*	Battery Factory Mode Related Functions		*
*											*
********************************************/
/*Input : current level*/ 
/*Output : none*/
int set_Charger_Current(int charging_current_level)
{
	int fd = -1;
	int ret = 0;
	int current_level_in_data[1] = {0};
	current_level_in_data[0] = charging_current_level;
		
	#ifdef FEATURE_FTM_PMIC_632X	
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}
	#else	
	fd = open("/dev/MT_pmic_adc_cali",O_RDWR, 0);
	if (fd == -1) {
		LOGD(TAG "get_BAT_status - Can't open /dev/MT_pmic_adc_cali\n");
		return -1;
	}
	#endif
	
	ret = ioctl(fd, Set_Charger_Current, current_level_in_data);
	close(fd);
	
	//LOGD(TAG "Set_Charger_Current : %d\n", current_level_in_data[0]);

	return 0;
}

/*Input : none*/ 
/*Output : Is_CAL*/
int get_BAT_status(void)
{
	int fd = -1;
	int ret = 0;
	int battery_in_data[1] = {0}; 
	
	#ifdef FEATURE_FTM_PMIC_632X	
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}
	#else
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_BAT_status - Can't open /dev/MT_pmic_adc_cali\n");
		return -1;
	}
	#endif
	
	ret = ioctl(fd, BAT_STATUS_READ, battery_in_data);
	close(fd);

	//LOGD(TAG "read CAL value : %d\n", battery_in_data[0]);
	
	return battery_in_data[0];
}

int get_BAT_vol(void)
{
	int fd = -1;
	int ret = 0;
	int batsen_in_data[1] = {0};

	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_BAT_vol - Can't open /dev/MT_pmic_adc_cali\n");
		return -1;
	}
	ret = ioctl(fd, AVG_BAT_SEN_READ, batsen_in_data);
	close(fd);

	return batsen_in_data[0];
}

int get_FG_current(void)
{
	int fd = -1;
	int ret = 0;
	int fgadc_in_data[1] = {0};

	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_FG_current - Can't open /dev/MT_pmic_adc_cali\n");
		return -1;
	}
	ret = ioctl(fd, FGADC_CURRENT_READ, fgadc_in_data);
	close(fd);

	return fgadc_in_data[0];
}

/*Input : ChannelNUm, Counts*/ 
/*Output : Sum, Result (0:success, 1:failed)*/
int get_ADC_channel(int adc_channel, int adc_count)
{
	int fd = -1;
	int ret = 0;
	int adc_in_data[2] = {1,1}; 
	
	#ifdef FEATURE_FTM_PMIC_632X	
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}
	#else
	fd = open("/dev/MT_pmic_adc_cali",O_RDONLY, 0);
	if (fd == -1) {
		LOGD(TAG "get_ADC_channel - Can't open /dev/MT_pmic_adc_cali\n");
	}
	#endif
	
	adc_in_data[0] = adc_channel;
	adc_in_data[1] = adc_count;
	ret = ioctl(fd, ADC_CHANNEL_READ, adc_in_data);
	close(fd);
	if (adc_in_data[1]==0) { 
		LOGD(TAG "read channel[%d] %d times : %d\n", adc_channel, adc_count, adc_in_data[0]);		
		return adc_in_data[0];
	}
	
	return -1;
}

static void battery_update_info(struct batteryFTM *batt, char *info)
{
    char *ptr;
	int temp = 0;

	#ifdef FEATURE_FTM_PMIC_632X
	int battery_fg_current = 0;
	bool pmic_is_connect = 0;
	#endif

	temp = get_ADC_channel(ADC_CHARGER, ADC_COUNT); 
	if (temp != -1) {		
		batt->charger_voltage = (temp/ADC_COUNT); /* Charger_Voltage */
		if ( batt->charger_voltage >= 4100 ) {
			batt->charger_exist = true;
			batt->is_charging = true;
		} else {
			batt->charger_voltage = 0;
			batt->charger_exist = false;
			batt->is_charging = false;
		}
	} else {
		batt->charger_voltage = -1;
		batt->charger_exist = false;
		batt->is_charging = false;
	}

    //batt->bat_voltage = get_BAT_vol();
    batt->bat_voltage = get_v_bat_sen();
    if (batt->bat_voltage != -1) {		
#ifdef FEATURE_FTM_PMIC_632X
        batt->adc_vbat_current = ((batt->bat_voltage)*1024)/(4*1200);
#else
	    batt->adc_vbat_current = ((batt->bat_voltage)*1024)/(2*2800);		 
#endif
    } else {		
	    batt->adc_vbat_current = -1;
    }

    //batt->current_charging = get_FG_current();
    batt->current_charging = get_ADC_channel(ADC_BAT_FG_CURRENT, ADC_COUNT);
	temp = get_BAT_status();
	if (temp != -1) {
		batt->is_calibration = (temp==1) ? true : false;		
	} else {
		batt->is_calibration = false;
	}

	batt->bat_temperature = 25;

	#ifdef BATTERY_TYPE_B61UN
	temp = get_ADC_channel(ADC_BAT_TEMP, ADC_COUNT); 
	if (temp != -1) {		
		temp = (temp/ADC_COUNT); 
		batt->bat_temperature = BattVoltToTemp(temp);
	} else {
		batt->bat_temperature = -100;
	}
	#endif

	#ifdef BATTERY_TYPE_BLP509
	temp = get_ADC_channel(ADC_BAT_TEMP, ADC_COUNT); 
	if (temp != -1) {		
		temp = (temp/ADC_COUNT); 
		batt->bat_temperature = BattVoltToTemp(temp);
	} else {
		batt->bat_temperature = -100;
	}
	#endif

	#ifdef BATTERY_TYPE_Z3
	temp = get_ADC_channel(ADC_BAT_TEMP, ADC_COUNT); 
		#ifdef FEATURE_FTM_PMIC_632X
			batt->bat_temperature = temp/ADC_COUNT;
		#else
	if (temp != -1) {		
		temp = (temp/ADC_COUNT); 
		batt->bat_temperature = BattVoltToTemp(temp);
	} else {
		batt->bat_temperature = -101;
	}
	#endif
	#endif

	#ifdef FEATURE_FTM_PMIC_632X
	temp = get_ADC_channel(ADC_BAT_FG_CURRENT, ADC_COUNT); 
	if (temp != -1) {				
		battery_fg_current = temp;
	} else {
		battery_fg_current = -1;
	}	
	#endif

	#ifdef FEATURE_FTM_PMIC_632X
	if( batt->adc_vbat_current > 0 )
	{
		pmic_is_connect = true;
	}
	else
	{
		pmic_is_connect = false;
	}
	#endif

	/* preare text view info */
        ptr  = info;
        ptr += sprintf(ptr, "%s : %d %s \n", uistr_info_title_battery_val, batt->bat_voltage, uistr_info_title_battery_mv);
        ptr += sprintf(ptr, "%s : %d %s \n", uistr_info_title_battery_temp, batt->bat_temperature, uistr_info_title_battery_c);    
        ptr += sprintf(ptr, "%s : %s \n", uistr_info_title_battery_chr, (batt->is_charging) ? uistr_info_title_battery_yes : uistr_info_title_battery_no);
        ptr += sprintf(ptr, "%s : %d %s \n", uistr_info_title_battery_chr_val, batt->charger_voltage, uistr_info_title_battery_mv);  
        #ifdef FEATURE_FTM_PMIC_632X
        ptr += sprintf(ptr, "%s: %d %s \n", uistr_info_title_battery_fg_cur, battery_fg_current, uistr_info_title_battery_ma);
        ptr += sprintf(ptr, "%s : %s \n", uistr_info_title_battery_pmic_chip, (pmic_is_connect) ? uistr_info_title_battery_connect : uistr_info_title_battery_no_connect);
        #endif

  return;
}

static int battery_key_handler(int key, void *priv) 
{
    int handled = 0, exit = 0;
    struct batteryFTM *batt = (struct batteryFTM *)priv;
    struct textview *tv = &batt->tv;
    struct ftm_module *fm = batt->mod;
    
    switch (key) {
    case UI_KEY_RIGHT:
        exit = 1;
        break;
    case UI_KEY_LEFT:        
        fm->test_result = FTM_TEST_FAIL;
        exit = 1;
        break;
    case UI_KEY_CENTER:
        fm->test_result = FTM_TEST_PASS;
        exit = 1;
        break;
    default:
        handled = -1;
        break;
    }
    if (exit) {
        LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        batt->exit_thd = true;
        tv->exit(tv);        
    }
    return handled;
}

static void *battery_update_thread(void *priv)
{
    struct batteryFTM *batt = (struct batteryFTM *)priv;
    struct textview *tv = &batt->tv;
    //struct statfs stat;
    int count = 1, chkcnt = 10;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(100000);
        chkcnt--;

        if (batt->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        battery_update_info(batt, batt->info);
        tv->redraw(tv);
        chkcnt = 10;
    }
	
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
	return NULL;
}

static void *battery_update_iv_thread(void *priv)
{
    struct batteryFTM *batt = (struct batteryFTM *)priv;
    struct itemview *iv = batt->iv;
    int count = 1, chkcnt = 10;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    
    while (1) {
        usleep(100000);
        chkcnt--;

        if (batt->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        battery_update_info(batt, batt->info);
        iv->redraw(iv);
        chkcnt = 10;
    }
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
	return NULL;
}

int get_v_charger(void)
{
	int temp=0;	
	
	temp = get_ADC_channel(ADC_CHARGER, ADC_COUNT); 
	temp = (temp/ADC_COUNT);	
	
	return temp;
}

int get_v_bat_sen(void)
{
	int temp=0;	
	
	temp = get_ADC_channel(ADC_BAT_SEN, ADC_COUNT); 
	temp = (temp/ADC_COUNT);
	
	return temp;
}

static int bat_thread_ctrl(int value)
{
    int fd = -1;
    int ret = 0;
    int bat_thread_ctrl_in_data[1] = {0};
    bat_thread_ctrl_in_data[0] = value;
    fd = open("/dev/MT_pmic_adc_cali",O_RDWR, 0);
    if (fd == -1) {
        LOGD(TAG "get_BAT_status - Can't open /dev/MT_pmic_adc_cali\n");
        return -1;
    }
    ret = ioctl(fd, BAT_THREAD_CTRL, bat_thread_ctrl_in_data);
    close(fd);
    return 1;
}

int battery_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct batteryFTM *batt = (struct batteryFTM *)priv;
    struct textview *tv;    
	struct itemview *iv;
	//auto test
	int temp_v_bat=0;
	int temp_v_chr=0;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&batt->title, param->name, COLOR_YELLOW);
    init_text(&batt->text, &batt->info[0], COLOR_YELLOW);
    init_text(&batt->left_btn, "Fail", COLOR_YELLOW);
    init_text(&batt->center_btn, "Pass", COLOR_YELLOW);
    init_text(&batt->right_btn, "Back", COLOR_YELLOW);

    battery_update_info(batt, batt->info);

    /* show text view */
    batt->exit_thd = false;     

#if 0
    pthread_create(&batt->batt_update_thd, NULL, battery_update_thread, priv);

    tv = &batt->tv;
    ui_init_textview(tv, battery_key_handler, (void*)batt);
    tv->set_title(tv, &batt->title);
    tv->set_text(tv, &batt->text);
    tv->set_btn(tv, &batt->left_btn, &batt->center_btn, &batt->right_btn);
    tv->run(tv);

    pthread_join(batt->batt_update_thd, NULL);
#else
	if (!batt->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        batt->iv = iv;
    }
    
    iv = batt->iv;
    iv->set_title(iv, &batt->title);
    iv->set_items(iv, battery_items, 0);
    iv->set_text(iv, &batt->text);
	iv->start_menu(iv,0);
    
    #if 1
	iv->redraw(iv);

	return_data.battery.current = 0;
	return_data.battery.voltage = 0;

    //check charger voltage, if no charger, return fail
    temp_v_chr = get_v_charger();
  if(temp_v_chr < AUTO_TEST_VCHR_VALUE)
  {
    LOGD(TAG "[FTM_BAT 0] %d < %d => no charger, return fail\n", temp_v_chr, AUTO_TEST_VCHR_VALUE);		
    batt->mod->test_result = FTM_TEST_FAIL;
    return 0;
  }

	//auto test - V_bat
	temp_v_bat = get_v_bat_sen();
        return_data.battery.voltage = temp_v_bat;
	if(temp_v_bat < AUTO_TEST_VBAT_VALUE)
	{
		LOGD(TAG "[FTM_BAT 1] %d,%d,%d\n", 
			temp_v_bat, 
			AUTO_TEST_VBAT_VALUE, AUTO_TEST_THD_VALUE);
		
	    batt->mod->test_result = FTM_TEST_FAIL;
		return 0;
	}
	if(temp_v_bat > AUTO_TEST_THD_VALUE)
	{
		LOGD(TAG "[FTM_BAT 2] %d,%d,%d\n", 
			temp_v_bat, 
			AUTO_TEST_VBAT_VALUE, AUTO_TEST_THD_VALUE);
    
	    batt->mod->test_result = FTM_TEST_FAIL;		
		return 0;
	}
  
#ifdef FEATURE_FTM_VBAT_TEMP_CHECK
    //auto test - V bat temp	
    temp_v_bat_temp = get_v_bat_temp();
    if(temp_v_bat_temp < AUTO_TEST_VBAT_TEMP_MIN || temp_v_bat_temp > AUTO_TEST_VBAT_TEMP_MAX)
    {
       LOGD(TAG "[FTM_BAT 5] VBatTemp = %d , return fail\n", temp_v_chr);		
        batt->mod->test_result = FTM_TEST_FAIL;
    return 0;
}
#endif
  
    batt->mod->test_result = FTM_TEST_PASS;		
    return 0;

    #else
    pthread_create(&batt->batt_update_thd, NULL, battery_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                batt->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                batt->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        }
        
        if (exit) {
            batt->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(batt->batt_update_thd, NULL);
#endif	
#endif	

    return 0;
}

int battery_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct batteryFTM *batt;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_CHARGER, sizeof(struct batteryFTM));
    batt = mod_to_batteryFTM(mod);

	/* init */
    batt->mod = mod;
	batt->bat_voltage = 0;
	batt->charger_voltage = 0;
	batt->current_charging = 0;
	batt->bat_temperature = 0;
	#ifdef FEATURE_FTM_PMIC_632X
	batt->adc_vbat_3_2 = 683;
	batt->adc_vbat_4_2 = 896;
	#else
	batt->adc_vbat_3_2 = 584;
	batt->adc_vbat_4_2 = 767;
	#endif
	batt->adc_vbat_current = 0;
	batt->is_charging = false;
	batt->is_calibration = false;
	batt->charger_exist = false;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, battery_entry, (void*)batt);

    return ret;
}

#endif

