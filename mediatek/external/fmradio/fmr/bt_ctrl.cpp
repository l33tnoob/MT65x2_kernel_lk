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

#include "fmr.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "FMJNI_BTCTRL"
#endif

#define BT_SERIAL_PORT "/dev/ttyMT2"

enum{
    BT_PWR_ON,
    BT_PWR_OFF,
    BT_PWR_UNKNOWN,
    BT_PWR_MAX
};
//for BT host
static int bt_host_rfkill_path_get(int *bt_id, char **state_path);
static int bt_host_pwr_state_get(int *state);
static int bt_host_init_uart(char *dev);
//for MT6616 BT controller
static int bt_mt6616_reset(int fd);
static int bt_mt6616_nonsleepduration_set(int fd, uint32_t dwTime);
static int bt_mt6616_alwayssleep_set(int fd);
static void *bt_mt6616_forcesleep_thread(void *priv);
//for MT6626 BT controller
static int bt_mt6626_alwayssleep_set(int fd);
static int bt_mt6626_i2s_switch_1(int fd);
static int bt_mt6626_i2s_switch_2(int fd);
static void *bt_mt6626_forcesleep_thread(void *priv);


static int bt_host_rfkill_path_get(int *bt_id, char **state_path) 
{
    char path[128];
    char buf[32];
    int fd, id;
    ssize_t sz;

    FMR_ASSERT(bt_id);
    FMR_ASSERT(state_path);
    
    for(id = 0; id < 10 ; id++){
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if(fd < 0){
            LOGE("open(%s) failed\n", path);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if(sz >= 9 && (memcmp(buf, "bluetooth", 9) == 0)){
            *bt_id = id;
            break;
        }
    }

    if (id == 10)
        return -1;

    asprintf(state_path, "/sys/class/rfkill/rfkill%d/state", *bt_id);
    return 0;
}

static int bt_host_pwr_state_get(int *state) 
{
    int sz;
    int fd = -1;
    int ret = 0;
    char sta;
    int bt_rfkill_id = -1;
    char *bt_rfkill_state_path = NULL;

    FMR_ASSERT(state);
    
    if(bt_rfkill_id == -1){
        ret = bt_host_rfkill_path_get(&bt_rfkill_id, &bt_rfkill_state_path);
        if(ret){
            LOGE("get bt rfkill sate path failed\n");
            return ret;
        }
    }

    fd = open(bt_rfkill_state_path, O_RDONLY);
    if(fd < 0){
        LOGE("open(%s) failed\n", bt_rfkill_state_path);
        return fd;
    }
    sz = read(fd, &sta, 1);
    if(sz != 1){
        LOGE("read(%s) failed", bt_rfkill_state_path);
        close(fd);
        return -1;
    }

    switch(sta){
    case '1':
        *state = BT_PWR_ON;
        break;
    case '0':
        *state = BT_PWR_OFF;
        break;
    default:
        LOGE("unknown bt pwr state\n");
        ret = -1;
    }

    free(bt_rfkill_state_path);
	close(fd);
    return ret;
}

static int bt_host_init_uart(char *dev)
{
	struct termios ti;
	int fd, i;

    FMR_ASSERT(dev);
    
	fd = open(dev, O_RDWR | O_NOCTTY);
	if(fd < 0){
		LOGE("Can't open serial port\n");
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	if(tcgetattr(fd, &ti) < 0){
        LOGE("unable to get UART port setting\n");
		return -1;
	}

	cfmakeraw(&ti);

	ti.c_cflag |= CLOCAL;
	ti.c_cflag &= ~CRTSCTS;

    ti.c_lflag = 0;
    ti.c_cc[VTIME]    = 5; /* 0.5 sec */
    ti.c_cc[VMIN]     = 0;

	if(tcsetattr(fd, TCSANOW, &ti) < 0){
		LOGE("Can't set port settings\n");
		return -1;
	}

	/* Set initial baudrate */
	cfsetospeed(&ti, B115200);
	cfsetispeed(&ti, B115200);

    tcsetattr(fd, TCSANOW, &ti);
	tcflush(fd, TCIOFLUSH);

	return fd;
}

static int bt_mt6616_reset(int fd)
{
    uint32_t dwBytesWritten = 0;
    uint32_t dwBytesTotal = 0;
    uint32_t dwCounter = 0;
    uint32_t dwBytesRead = 7;
    uint8_t pAckEvent[7];
    int sz = 0;      
    uint8_t HCI_RESET[] = {0x01, 0x03, 0x0c, 0x0};
    
	sz = write(fd, HCI_RESET, sizeof(HCI_RESET)); 
	if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    }
    
    while((dwBytesTotal < 7) && (dwCounter < 5)){
        dwBytesRead = read(fd, &pAckEvent[dwBytesTotal], dwBytesRead);
        dwBytesTotal += dwBytesRead;  
        dwBytesRead = 7 - dwBytesTotal;
        dwCounter++;      
    }    
    if(dwBytesTotal < 7){
        LOGE("%s no ACK \n", __func__);        
        return -1;
    }
            	
    return 0;     
}

static int bt_mt6616_nonsleepduration_set(int fd, uint32_t dwTime)
{
    uint32_t dwBytesWritten = 0;
    uint32_t dwBytesTotal = 0;
    uint32_t dwCounter = 0;
    uint32_t dwBytesRead = 7;
    uint8_t pAckEvent[7];
    int sz = 0;
    
    uint32_t dwSlotNum = (dwTime<<3)/5;  
    uint8_t HCI_VS_SET_SLEEP[] =  {0x01, 0x7a, 0xfc, 0x07, 0x03, 0x40, 0x1f, 0x40, 0x1f, 0x00, 0x04};  
  
    HCI_VS_SET_SLEEP[4] = 0x03; //3-pin lower power mode
    HCI_VS_SET_SLEEP[5] = (uint8_t)(dwSlotNum&0xFF); 
    HCI_VS_SET_SLEEP[6] = (uint8_t)((dwSlotNum>>8)&0xFF); 
    HCI_VS_SET_SLEEP[7] = (uint8_t)(dwSlotNum&0xFF);
    HCI_VS_SET_SLEEP[8] = (uint8_t)((dwSlotNum>>8)&0xFF);
   
    sz = write(fd, HCI_VS_SET_SLEEP, sizeof(HCI_VS_SET_SLEEP)); 
	if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    }
    
    while((dwBytesTotal < 7) && (dwCounter < 5)){
        dwBytesRead = read(fd, &pAckEvent[dwBytesTotal], dwBytesRead);
        dwBytesTotal += dwBytesRead;  
        dwBytesRead = 7 - dwBytesTotal;
        dwCounter++;      
    }    
    if(dwBytesTotal < 7){
        LOGE("%s no ACK \n", __func__);        
        return -1;
    }
    	
    return 0;    
}

static int bt_mt6616_alwayssleep_set(int fd)
{
    int sz = 0;
    uint8_t HCI_VS_SET_ALWAYS_SLEEP[] = {0x01, 0xD8, 0xfc, 0x0};
    
    sz = write(fd, HCI_VS_SET_ALWAYS_SLEEP, sizeof(HCI_VS_SET_ALWAYS_SLEEP));
    if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    } 
        
    return 0;
}

static void *bt_mt6616_forcesleep_thread(void *priv)
{
    int bt_fd = bt_host_init_uart((char*)BT_SERIAL_PORT);
    if(bt_fd != -1){
        LOGD("+set bt force sleep, sleep 1\n");
        sleep(1);
        bt_mt6616_nonsleepduration_set(bt_fd, 5000);
        bt_mt6616_reset(bt_fd);
        bt_mt6616_alwayssleep_set(bt_fd); 
        LOGD("-set bt force sleep\n");   
        close(bt_fd);    
    }
    
    pthread_exit(NULL);
    return NULL;  
}

static int bt_mt6626_alwayssleep_set(int fd)
{
    uint32_t dwBytesWritten = 0;
    uint32_t dwBytesTotal = 0;
    uint32_t dwCounter = 0;
    uint32_t dwBytesRead = 7;
    uint8_t pAckEvent[7];
    int sz = 0;      
    uint8_t HCI_VS_SET_ALWAYS_SLEEP[] = 
        {0x01, 0xD8, 0xFC, 0x08, 0x03, 0x40, 0x1F, 0x40, 0x1F, 0x00, 0x04, 0x01};
    
	sz = write(fd, HCI_VS_SET_ALWAYS_SLEEP, sizeof(HCI_VS_SET_ALWAYS_SLEEP)); 
	if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    }
    
    while((dwBytesTotal < 7) && (dwCounter < 5)){
        dwBytesRead = read(fd, &pAckEvent[dwBytesTotal], dwBytesRead);
        dwBytesTotal += dwBytesRead;  
        dwBytesRead = 7 - dwBytesTotal;
        dwCounter++;      
    }    
    if(dwBytesTotal < 7){
        LOGE("%s no ACK \n", __func__);        
        return -1;
    }
            	
    return 0;     
}

static int bt_mt6626_i2s_switch_1(int fd)
{
    uint32_t dwBytesWritten = 0;
    uint32_t dwBytesTotal = 0;
    uint32_t dwCounter = 0;
    uint32_t dwBytesRead = 7;
    uint8_t pAckEvent[7];
    int sz = 0;      
    uint8_t HCI_VS_I2S_SWITCH_1[] = 
        {0x01, 0xD0, 0xFC, 0x08, 0x50, 0x01, 0x05, 0x80, 0x20, 0x00, 0x00, 0x00};
    
	sz = write(fd, HCI_VS_I2S_SWITCH_1, sizeof(HCI_VS_I2S_SWITCH_1)); 
	if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    }
    
    while((dwBytesTotal < 7) && (dwCounter < 5)){
        dwBytesRead = read(fd, &pAckEvent[dwBytesTotal], dwBytesRead);
        dwBytesTotal += dwBytesRead;  
        dwBytesRead = 7 - dwBytesTotal;
        dwCounter++;      
    }    
    if(dwBytesTotal < 7){
        LOGE("%s no ACK \n", __func__);        
        return -1;
    }
            	
    return 0;     
}

static int bt_mt6626_i2s_switch_2(int fd)
{
    uint32_t dwBytesWritten = 0;
    uint32_t dwBytesTotal = 0;
    uint32_t dwCounter = 0;
    uint32_t dwBytesRead = 7;
    uint8_t pAckEvent[7];
    int sz = 0;      
    uint8_t bt_mt6626_i2s_switch_2[] = 
        {0x01, 0xD0, 0xFC, 0x08, 0x60, 0x01, 0x05, 0x80, 0x00, 0x00, 0x00, 0x00};
    
	sz = write(fd, bt_mt6626_i2s_switch_2, sizeof(bt_mt6626_i2s_switch_2)); 
	if(sz < 0){
        LOGE("%s can't send command\n", __func__);
        return -1;
    }
    
    while((dwBytesTotal < 7) && (dwCounter < 5)){
        dwBytesRead = read(fd, &pAckEvent[dwBytesTotal], dwBytesRead);
        dwBytesTotal += dwBytesRead;  
        dwBytesRead = 7 - dwBytesTotal;
        dwCounter++;      
    }    
    if(dwBytesTotal < 7){
        LOGE("%s no ACK \n", __func__);        
        return -1;
    }
            	
    return 0;     
}

static void *bt_mt6626_forcesleep_thread(void *priv)
{
    int bt_fd = bt_host_init_uart((char*)BT_SERIAL_PORT);
    if(bt_fd != -1){
        LOGD("+set bt force sleep, sleep 1\n");
        sleep(1);
        bt_mt6626_alwayssleep_set(bt_fd);
        bt_mt6626_i2s_switch_1(bt_fd);
        bt_mt6626_i2s_switch_2(bt_fd); 
        LOGD("-set bt force sleep\n");   
        close(bt_fd);    
    }
    
    pthread_exit(NULL);
    return NULL;  
}

/*
 * bt_set_controller_force_sleep
 * If bt host is not at power on state, we should info bt firmware to close bt HW
 * and let bt firmware enter sleep mode
 * @chip -- chip value, 0x6616/0x6626
 */
int bt_set_controller_force_sleep(int chip)
{
	int ret = 0;
    pthread_t bt_fsleep_thd;
    void *(*work_thread)(void*) = NULL;
    int pwr_state = BT_PWR_UNKNOWN;

    ret = bt_host_pwr_state_get(&pwr_state);
    if(ret){
        LOGE("get bt power state failed\n");
        return ret;
    }

    switch(chip){
        case FM_CHIP_MT6616:
            work_thread = bt_mt6616_forcesleep_thread;
            break;
        case FM_CHIP_MT6626:
            work_thread = bt_mt6626_forcesleep_thread;
            break;
        default:
            LOGE("unknown chip type\n");
            return -1;
            break;
    }
    
    if(BT_PWR_OFF == pwr_state){
        LOGD("set bt force sleep\n");
        ret = pthread_create(&bt_fsleep_thd, NULL, work_thread, NULL);
        if(ret){
            LOGE("create work thread failed\n");
            return ret;
        }
        pthread_join(bt_fsleep_thd, NULL);
    }   
    return ret;
}

