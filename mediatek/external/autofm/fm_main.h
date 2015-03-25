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

#ifndef __FM_MAIN_H__
#define __FM_MAIN_H__

#define FM_NAME             "fm"
#define FM_DEVICE_NAME      "/dev/fm"

#define FM_VOL_MAX           0x2B	// 43 volume(0-15)

#define FM_RDS_ENABLE		0x01 // 1: enable RDS, 0:disable RDS
#define FM_RDS_DATA_READY   (1 << 0)

// errno
#define FM_SUCCESS      0
#define FM_FAILED       1
#define FM_EPARM        2
#define FM_BADSTATUS    3
#define FM_TUNE_FAILED  4
#define FM_SEEK_FAILED  5
#define FM_BUSY         6
#define FM_SCAN_FAILED  7

struct fm_tune_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint16_t freq; // IN/OUT parameter
};

struct fm_seek_parm {
    uint8_t err;
    uint8_t band;
    uint8_t space;
    uint8_t hilo;
    uint8_t seekdir;
    uint8_t seekth;
    uint16_t freq; // IN/OUT parameter
};

struct fm_scan_parm {
    uint8_t  err;
    uint8_t  band;
    uint8_t  space;
    uint8_t  hilo;
    uint16_t freq; // OUT parameter
    uint16_t ScanTBL[16]; //need no less than the chip
    uint16_t ScanTBLSize; //IN/OUT parameter
};

struct fm_cqi 
{
    uint16_t ch;
    uint16_t rssi;
    uint16_t reserve;
};

struct fm_cqi_req 
{
    uint16_t ch_num;
    int buf_size;
    char *cqi_buf;
};

struct fm_ch_rssi{
    uint16_t freq;
    uint16_t rssi;
};

struct fm_rssi_req{
    uint16_t num;
    uint16_t read_cnt;
    struct fm_ch_rssi cr[16*16];
};

struct fm_rds_tx_parm {
    uint8_t err;
    uint16_t pi;
    uint16_t ps[12]; // 4 ps
    uint16_t other_rds[87];  // 0~29 other groups
    uint8_t other_rds_cnt; // # of other group
};

typedef struct fm_rds_tx_req{
    unsigned char pty;         // 0~31 integer
    unsigned char rds_rbds;    // 0:RDS, 1:RBDS
    unsigned char dyn_pty;     // 0:static, 1:dynamic
    unsigned short pi_code;    // 2-byte hex
    unsigned char ps_buf[8];     // hex buf of PS
    unsigned char ps_len;      // length of PS, must be 0 / 8"
    unsigned char af;          // 0~204, 0:not used, 1~204:(87.5+0.1*af)MHz
    unsigned char ah;          // Artificial head, 0:no, 1:yes
    unsigned char stereo;      // 0:mono, 1:stereo
    unsigned char compress;    // Audio compress, 0:no, 1:yes
    unsigned char tp;          // traffic program, 0:no, 1:yes
    unsigned char ta;          // traffic announcement, 0:no, 1:yes
    unsigned char speech;      // 0:music, 1:speech
}fm_rds_tx_req;

#define TX_SCAN_MAX 10
#define TX_SCAN_MIN 1
struct fm_tx_scan_parm {
    uint8_t  err;
    uint8_t  band;	//87.6~108MHz
    uint8_t  space;
    uint8_t  hilo;
    uint16_t freq; 	// start freq, if less than band min freq, then will use band min freq
    uint8_t	 scandir;
    uint16_t ScanTBL[TX_SCAN_MAX]; 	//need no less than the chip
    uint16_t ScanTBLSize; //IN: desired size, OUT: scan result size 
};

struct fm_gps_rtc_info{
    int             err;            //error number, 0: success, other: err code
    int             retryCnt;       //GPS mnl can decide retry times
    int             ageThd;         //GPS 3D fix time diff threshold
    int             driftThd;       //GPS RTC drift threshold
    struct timeval  tvThd;          //time value diff threshold
    int             age;            //GPS 3D fix time diff
    int             drift;          //GPS RTC drift
    union{
        unsigned long stamp;        //time stamp in jiffies
        struct timeval  tv;         //time stamp value in RTC
    };
    int             flag;           //rw flag
};

typedef enum
{
	FM_I2S_ON = 0,
	FM_I2S_OFF
}fm_i2s_state;

typedef enum
{
	FM_I2S_MASTER = 0,
	FM_I2S_SLAVE
}fm_i2s_mode;

typedef enum
{
	FM_I2S_32K = 0,
	FM_I2S_44K,
	FM_I2S_48K
}fm_i2s_sample;

struct fm_i2s_setting{
    int onoff;
    int mode;
    int sample;
};

typedef enum{
    FM_RX = 0,
    FM_TX = 1
}FM_PWR_T;

enum group_idx {
    mono=0,
    stereo,
    RSSI_threshold,
    HCC_Enable,
    PAMD_threshold,
    Softmute_Enable,
    De_emphasis,
    HL_Side,
    Demod_BW,
    Dynamic_Limiter,
    Softmute_Rate,
    AFC_Enable,
    Softmute_Level,
    Analog_Volume,
    GROUP_TOTAL_NUMS
};
	
enum item_idx {
    Sblend_OFF=0,
    Sblend_ON,  
    ITEM_TOTAL_NUMS
};

struct fm_ctl_parm {
    uint8_t err;
    uint8_t addr;
    uint16_t val;
    uint16_t rw_flag;//0:write, 1:read
};
struct fm_top_rw_parm {
	uint8_t err;
	uint8_t rw_flag;//0:write, 1:read
	uint16_t addr;
	uint32_t val;
};
struct fm_host_rw_parm {
	uint8_t err;
	uint8_t rw_flag;//0:write, 1:read
	uint32_t addr;
	uint32_t val;
};

struct fm_em_parm {
	uint16_t group_idx;
	uint16_t item_idx;
	uint32_t item_value;	
};

#endif //__FM_MAIN_H__

