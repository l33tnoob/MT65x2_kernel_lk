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

#ifndef _FTM_H_
#define _FTM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LOGD ALOGD
#define LOGE ALOGE
#define LOGI ALOGI
#define LOGW ALOGW


enum {
    FTM_TEST_UNKNOWN = 0,
    FTM_TEST_PASS,
    FTM_TEST_FAIL, 
    FTM_TEST_MAX,
};

enum{
	FTM_AUTO_ITEM = 1,
	FTM_MANUAL_ITEM = 2,
};

struct ftm_param {
    const char *name;
};

typedef struct {
	int fm_rssi;
} ftm_ata_fm;

typedef struct {
	char wifi_mac[33];
	char wifi_name[32];
	int wifi_rssi;
	int channel;
	int rate;
} ftm_ata_wifi;

typedef struct {
	char bt_mac[32];
	char bt_name[32];
	int bt_rssi;
} ftm_ata_bt;

typedef struct {
	char modem_ver[128];
	char sw_ver[128];
} ftm_ata_version;

typedef struct{
	int num;
	ftm_ata_bt bt[10];
} ftm_ata_bt_num;

typedef struct{
	float ratio;
	int offset;
	float drift;
	int mean;
	int sigma;
	int update_hz;
	int bitsync;
	int acquision;
	int svid;
} ftm_ata_gps;

typedef struct{
	int freqL;
    int amplL;
    int freqR;
    int amplR;
} ftm_ata_speaker;

typedef struct{
	int freqL;
    int amplL;
    int freqR;
    int amplR;
} ftm_ata_receiver;

typedef struct{
	int freqL;
    int amplL;
	int freqR;
    int amplR;
} ftm_ata_headset;

typedef struct{
	int current;
	int voltage;
} ftm_ata_battery;

typedef struct{
    float g_sensor_x;
    float g_sensor_y;
    float g_sensor_z;
    int accuracy;
} ftm_ata_gsensor;

typedef struct{
    int m_sensor_x;
    int m_sensor_y;
    int m_sensor_z;
    int accuracy;
} ftm_ata_msensor;

typedef struct{
    int als;
    int ps;
} ftm_ata_alsps;

typedef struct{
    float gyroscope_x;
    float gyroscope_y;
    float gyroscope_z;
    int accuracy;
} ftm_ata_gyroscope;

typedef struct{
    int freq;
    int ampl;
} ftm_ata_vibrator;

typedef struct{
	ftm_ata_fm fm;
	ftm_ata_wifi wifi;
	ftm_ata_bt_num bt;
	ftm_ata_version version;
	ftm_ata_gps gps;
	ftm_ata_speaker speaker;
	ftm_ata_receiver receiver;
	ftm_ata_headset headset;
	ftm_ata_headset headsetL;
	ftm_ata_headset headsetR;
	ftm_ata_battery battery;
    ftm_ata_gsensor gsensor;
    ftm_ata_msensor msensor;
    ftm_ata_alsps alsps;
    ftm_ata_gyroscope gyroscope;
	ftm_ata_vibrator vibrator;
} sp_ata_data;

enum{
	FM_ATA_WIFI = 0,
	FM_ATA_BT,
	FM_ATA_FM,
	FM_ATA_VERSION,
};

typedef int (*ftm_init_fn)(void);
typedef int (*ftm_entry_fn)(struct ftm_param *param, void *priv);

struct ftm_module {
    int           id;
    bool          visible;
	void         *priv;
	ftm_entry_fn  entry;
	int           test_result;
};

extern int  ftm_init(void);
extern int  ftm_register(struct ftm_module *mod, ftm_entry_fn entry, void *priv);
extern int  ftm_db_register(struct ftm_module *mod, ftm_entry_fn entry, void *priv);

extern void ftm_unregister(struct ftm_module *mod);
extern int  ftm_entry(int id, struct ftm_param *param);

extern struct ftm_module *ftm_alloc(int id, int extra);
extern void ftm_free(struct ftm_module *mod);

extern int ftm_set_prop(const char *name, const char *val);
extern char *ftm_get_prop(const char *name);

extern void ftm_set_result(struct ftm_module *mod, int result);
extern int ftm_get_result(struct ftm_module *mod);

extern struct ftm_module **ftm_get_modules(void);
extern struct ftm_module *ftm_get_module(int id);

extern int get_is_ata();
extern int is_USB_State_PlugIn();
extern void close_usb();
extern void open_usb();

#ifdef __cplusplus
}
#endif


#endif
