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

#define _MNLD_6620_C_
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "mnl_common_6620.h"
#include "mtk_gps.h"
#include <cutils/properties.h>
#include <linux/mtk_agps_common.h>
#include <sys/stat.h>

#include <unistd.h>
//for read NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"


#ifdef GPS_PROPERTY
#undef GPS_PROPERTY
#endif
#define GPS_PROPERTY "/data/misc/GPS_CHIP.cfg"
static 	ap_nvram_gps_config_struct stGPSReadback;

/*---------------------------------------------------------------------------*/
#if defined(ANDROID)
#define LOG_TAG "MNLD"
#include <cutils/sockets.h>
#include <cutils/xlog.h>     /*logging in logcat*/
#endif 

/******************************************************************************
* Macro & Definition
******************************************************************************/
#define C_CMD_BUF_SIZE 32
#define SOCKET_MNL "mnld"
UINT32 assist_data_bit_map = FLAG_HOT_START;
static int in_stop_proc = 0;	/*this flag is used to indicate if in stopping procedure*/
/*---------------------------------------------------------------------------*/
#if defined(ANDROID)
#define MND_MSG(fmt, arg ...) XLOGD("%s: " fmt, __FUNCTION__ ,##arg)
#define MND_ERR(fmt, arg ...) XLOGE("%s: " fmt, __FUNCTION__ ,##arg)
#define MND_TRC(f)            XLOGD("%s\n", __FUNCTION__) 
#define MND_VER(...)          do {} while(0) 
#else
#define MND_MSG(...) printf(LOG_TAG":" __VA_ARGS__)
#define MND_ERR(...) printf(LOG_TAG":" __VA_ARGS__)
#endif
/*---------------------------------------------------------------------------*/
#define EPOLL_NUM 5
/*---------------------------------------------------------------------------*/
enum {
    GPS_PWRCTL_UNSUPPORTED  = 0xFF,
    GPS_PWRCTL_OFF          = 0x00,
    GPS_PWRCTL_ON           = 0x01,
    GPS_PWRCTL_RST          = 0x02,
    GPS_PWRCTL_OFF_FORCE    = 0x03,
    GPS_PWRCTL_RST_FORCE    = 0x04,
    GPS_PWRCTL_MAX          = 0x05,
};
enum {
    GPS_PWR_UNSUPPORTED     = 0xFF,
    GPS_PWR_RESUME          = 0x00,
    GPS_PWR_SUSPEND         = 0x01,
    GPS_PWR_MAX             = 0x02,
};
enum {
    GPS_STATE_UNSUPPORTED   = 0xFF,
    GPS_STATE_PWROFF        = 0x00, /*cleanup/power off, default state*/
    GPS_STATE_INIT          = 0x01, /*init*/
    GPS_STATE_START         = 0x02, /*start navigating*/
    GPS_STATE_STOP          = 0x03, /*stop navigating*/
    GPS_STATE_DEC_FREQ      = 0x04, 
    GPS_STATE_SLEEP         = 0x05,
    GPS_STATE_MAX           = 0x06,
};
enum {
    GPS_PWRSAVE_UNSUPPORTED = 0xFF,
    GPS_PWRSAVE_DEC_FREQ    = 0x00,
    GPS_PWRSAVE_SLEEP       = 0x01,
    GPS_PWRSAVE_OFF         = 0x02,
    GPS_PWRSAVE_MAX         = 0x03,
};
#define MNL_ATTR_PWRCTL  "/sys/class/gpsdrv/gps/pwrctl"
#define MNL_ATTR_SUSPEND "/sys/class/gpsdrv/gps/suspend"
#define MNL_ATTR_STATE   "/sys/class/gpsdrv/gps/state"
#define MNL_ATTR_PWRSAVE "/sys/class/gpsdrv/gps/pwrsave"
#define MNL_ATTR_STATUS  "/sys/class/gpsdrv/gps/status"
#define  MNL_CONFIG_STATUS      "persist.radio.mnl.prop"
/*---------------------------------------------------------------------------*/
typedef enum {
    MNL_ALARM_UNSUPPORTED   = 0xFF,
    MNL_ALARM_INIT          = 0x00,
    MNL_ALARM_MONITOR       = 0x01,
    MNL_ALARM_WAKEUP        = 0x02,
    MNL_ALARM_TTFF          = 0x03,
    MNL_ALARM_DEC_FREQ      = 0x04,
    MNL_ALARM_SLEEP         = 0x05,
    MNL_ALARM_PWROFF        = 0x06,
    MNL_ALARM_MAX           = 0x07,
} MNL_ALARM_TYPE;
/*---------------------------------------------------------------------------*/
enum {
    MNL_ALARM_IDX_WATCH     = 0x00,
    MNL_ALARM_IDX_PWRSAVE   = 0x01,
    MNL_ALARM_IDX_MAX       = 0x02
};
/*---------------------------------------------------------------------------*/
enum { /*restart reason*/
    MNL_RESTART_NONE            = 0x00, /*recording the 1st of mnld*/
    MNL_RESTART_TIMEOUT_INIT    = 0x01, /*restart due to timeout*/
    MNL_RESTART_TIMEOUT_MONITOR = 0x02, /*restart due to timeout*/
    MNL_RESTART_TIMEOUT_WAKEUP  = 0x03, /*restart due to timeout*/
    MNL_RESTART_TIMEOUT_TTFF    = 0x04, /*restart due to TTFF timeout*/
    MNL_RESTART_FORCE           = 0x05, /*restart due to external command*/    
};
/*---------------------------------------------------------------------------*/
typedef struct
{
    int period;
    int cmd;
    int idx;
} MNL_ALARM_T;
/*---------------------------------------------------------------------------*/
#define MNL_TIMEOUT_INIT    15
#define MNL_TIMEOUT_MONITOR 5
#define MNL_TIMEOUT_WAKEUP  3
#define MNL_TIMEOUT_TTFF    10
#define MNL_TIMEOUT_DEC_REQ 300
#define MNL_TIMEOUT_SLEEP   2
//#define MNL_TIMEOUT_PWROFF  30
#define MNL_TIMEOUT_PWROFF  1

/*---------------------------------------------------------------------------*/
static MNL_CONFIG_T mnld_cfg = {
    .timeout_init  = MNL_TIMEOUT_INIT,
    .timeout_monitor = MNL_TIMEOUT_MONITOR,
    .timeout_sleep = MNL_TIMEOUT_SLEEP,
    .timeout_pwroff = MNL_TIMEOUT_PWROFF,
    .timeout_wakeup = MNL_TIMEOUT_WAKEUP,
    .timeout_ttff = MNL_TIMEOUT_TTFF,
};
/*---------------------------------------------------------------------------*/
static MNL_ALARM_T mnl_alarm[MNL_ALARM_MAX] = { 
    /*the order should be the same as MNL_ALARM_TYPE*/
    {MNL_TIMEOUT_INIT,     MNL_CMD_TIMEOUT_INIT,    MNL_ALARM_IDX_WATCH},
    {MNL_TIMEOUT_MONITOR,  MNL_CMD_TIMEOUT_MONITOR, MNL_ALARM_IDX_WATCH},
    {MNL_TIMEOUT_WAKEUP,   MNL_CMD_TIMEOUT_WAKEUP,  MNL_ALARM_IDX_WATCH},
    {MNL_TIMEOUT_TTFF,     MNL_CMD_TIMEOUT_TTFF,    MNL_ALARM_IDX_WATCH},      
    {MNL_TIMEOUT_DEC_REQ,  MNL_CMD_DEC_FREQ,        MNL_ALARM_IDX_PWRSAVE}, 
    {MNL_TIMEOUT_SLEEP,    MNL_CMD_SLEEP,           MNL_ALARM_IDX_PWRSAVE},  
    {MNL_TIMEOUT_PWROFF,   MNL_CMD_PWROFF,          MNL_ALARM_IDX_PWRSAVE},  
};
/*---------------------------------------------------------------------------*/
typedef struct
{
    int type;
    struct sigevent evt;
    struct itimerspec expire;
    timer_t id[MNL_ALARM_IDX_MAX];        
} MNL_TIMER_T;
/*---------------------------------------------------------------------------*/
static MNL_TIMER_T mnl_timer = {
    .type = MNL_ALARM_UNSUPPORTED,
    .expire = {{0,0}, {0,0}},
    .id = {C_INVALID_TIMER, C_INVALID_TIMER},
};
/*---------------------------------------------------------------------------*/
typedef struct {
    int cur_accept_socket;
    int epoll_fd;
    int sig_rcv_fd;  /*pipe for signal handler*/
    int sig_snd_fd;  
    int mnl_rcv_fd;  /*pipe for mnl daemon*/
    int mnl_snd_fd;
    unsigned char pwrctl;    
    unsigned char suspend;    
    unsigned char state;
    unsigned char pwrsave;
}MNLD_DATA_T;
/*---------------------------------------------------------------------------*/
typedef struct {
    int         init;
    pid_t       pid;
    int         count;
    int         terminated;
}MNLD_MONITOR_T;
/*****************************************************************************/
static MNLD_DATA_T mnld_data = {
    .cur_accept_socket = C_INVALID_SOCKET,
    .epoll_fd   = C_INVALID_FD,
    .sig_rcv_fd = C_INVALID_FD,
    .sig_snd_fd = C_INVALID_FD,
    .mnl_rcv_fd = C_INVALID_FD,
    .mnl_snd_fd = C_INVALID_FD,
    .pwrctl = GPS_PWRCTL_UNSUPPORTED,
    .suspend = GPS_PWR_UNSUPPORTED,
    .state = GPS_STATE_UNSUPPORTED,
    .pwrsave = GPS_PWRSAVE_UNSUPPORTED
};
static MNLD_MONITOR_T mnld_monitor = {
    .init   = 0,
    .pid    = C_INVALID_PID,
    .count  = 0,
    .terminated = 0,
};
//CMCC: GPS.LOG
static int ttff = 0;
time_t start_time;
static int get_prop()
{
    //Read property
    char result[PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    if(property_get(MNL_CONFIG_STATUS, result, NULL)){
        ret = result[6] - '0';
      	MND_MSG("gps.log: %s, %d\n", &result[6], ret);       	 
    }else{
        ret = 0;
        MND_MSG("Config is not set yet, use default value");
    }
    
    return ret;
}
/*****************************************************************************/
static int epoll_add(int epoll_fd, int fd)
{
    struct epoll_event  ev;
    int                 ret, flags;

    if (epoll_fd == C_INVALID_FD)
        return -1;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}
/*****************************************************************************/
static int epoll_del( int epoll_fd, int fd)
{
    struct epoll_event  ev;
    int                 ret;

    if (epoll_fd == C_INVALID_FD)
        return -1;
    
    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}
/*****************************************************************************/
static int epoll_init(void)
{
    int epoll_fd = epoll_create(EPOLL_NUM);
    MNLD_DATA_T *obj = &mnld_data;
    
    if (epoll_fd < 0)
        return -1;   
    
    if (obj->cur_accept_socket != C_INVALID_FD) {
        if (epoll_add(epoll_fd, obj->cur_accept_socket))
           return -1;
    }

    if (obj->sig_rcv_fd != C_INVALID_FD) {
        if (epoll_add(epoll_fd, obj->sig_rcv_fd))
            return -1;
    }

    if (obj->mnl_rcv_fd != C_INVALID_FD) {
        if (epoll_add(epoll_fd, obj->mnl_rcv_fd))
            return -1;
    }

    obj->epoll_fd = epoll_fd;
    return 0;
}
/*****************************************************************************/
static void epoll_destroy(void)
{
    MNLD_DATA_T *obj = &mnld_data;

    if ((obj) && (obj->epoll_fd != C_INVALID_FD)) {
        if (close(obj->epoll_fd))
            MND_ERR("close(%d) : %d (%s)\n", obj->epoll_fd, errno, strerror(errno));
    }
}
/*****************************************************************************/
static int send_cmd_ex(int fd, char* cmd, int len, char* caller)
{
    if (fd == C_INVALID_FD) {
        return 0;
    } else {    
        int  ret;
        MND_MSG("%s (%d, 0x%x)\n", caller, fd, (int)(*cmd));
        do { 
            ret = write( fd, cmd, len ); 
        }while (ret < 0 && errno == EINTR);
        
        if (ret == len)
            return 0;
        else {
            MND_ERR("%s fails: %d (%s)\n", caller, errno, strerror(errno));
            return -1;
        }
    }    

}
/*****************************************************************************/
#define mnl_send_cmd(cmd, len) send_cmd_ex(mnld_data.mnl_snd_fd, cmd, len, "mnl_send_cmd")
#define slf_send_cmd(cmd, len) send_cmd_ex(mnld_data.sig_snd_fd, cmd, len, "slf_send_cmd")
/*****************************************************************************/
void mnl_alarm_handler(sigval_t v)
{    
    char buf[] = {(char)v.sival_int};
    MND_MSG("mnl_alarm_handler:%d\n", (int)(*buf));
    slf_send_cmd(buf, sizeof(buf));
}
/*****************************************************************************/
static inline int mnl_alarm_stop(int alarm_idx) 
{
    int err = 0;
    MNL_TIMER_T *obj = &mnl_timer;
    
    if (alarm_idx >= MNL_ALARM_IDX_MAX) {
        err = -1;   /*out-of-range*/
    } else if (obj->id[alarm_idx] != C_INVALID_TIMER) {
        if ((err = timer_delete(obj->id[alarm_idx]))) {
            MND_ERR("timer_delete(%.8X) = %d (%s)\n", (int)obj->id[alarm_idx], errno, strerror(errno));
            return -1;
        }
        obj->id[alarm_idx] = C_INVALID_TIMER;
        obj->type = MNL_ALARM_UNSUPPORTED;
    } else {
        /*the alarm is already stopped*/
    }
    return err;
}
/*****************************************************************************/
static inline int mnl_alarm_stop_watch() 
{
    MND_TRC();
    return mnl_alarm_stop(MNL_ALARM_IDX_WATCH);
}
/*****************************************************************************/
static int mnl_alarm_stop_all()
{
    int idx, err;
    MNL_TIMER_T *obj = &mnl_timer;
    for (idx = 0; idx < MNL_ALARM_IDX_MAX; idx++) {
        if (obj->id[idx] != C_INVALID_TIMER) {
            if ((err = timer_delete(obj->id[idx]))) {
                MND_ERR("timer_delete(%d) = %d (%s)\n", (int)obj->id, errno, strerror(errno));
                return -1;
            }
            obj->id[idx] = C_INVALID_TIMER;
            obj->type = MNL_ALARM_UNSUPPORTED;
        }
    }
    return 0;
}
/*****************************************************************************/
static int mnl_set_alarm(int type)
{
    int err = 0;
    MNL_TIMER_T *obj = &mnl_timer;
    MNL_ALARM_T *ptr;
    if (type >= MNL_ALARM_MAX) {
        MND_ERR("invalid alarm type: %d\n", type);
        return -1;
    }
    ptr = &mnl_alarm[type];
    if (ptr->idx >= MNL_ALARM_IDX_MAX) {
        MND_ERR("invalid alarm index: %d\n", type);
        return -1;        
    }
    if (obj->id[ptr->idx] != C_INVALID_TIMER) {
        if (obj->type != type) 
        {
            //MND_MSG("timer_delete(0x%.8X)\n", obj->id);        
            if ((err = timer_delete(obj->id[ptr->idx]))) {
                MND_ERR("timer_delete(%d) = %d (%s)\n", (int)obj->id, errno, strerror(errno));
                return -1;
            }
            obj->id[ptr->idx] = C_INVALID_TIMER;
            obj->type = MNL_ALARM_UNSUPPORTED;

        }
    }
    if (obj->id[ptr->idx] == C_INVALID_TIMER) {
        memset(&obj->evt, 0x00, sizeof(obj->evt));
        obj->evt.sigev_value.sival_int = ptr->cmd;
        obj->evt.sigev_notify = SIGEV_THREAD;
        obj->evt.sigev_notify_function = mnl_alarm_handler;       
        obj->type = type;
        if ((err = timer_create(CLOCK_PROCESS_CPUTIME_ID, &obj->evt, &obj->id[ptr->idx]))) {
            MND_ERR("timer_create = %d(%s)\n", errno, strerror(errno));
            return -1;
        }
        //MND_MSG("timer_create(0x%.8X)\n", obj->id);        
    }
    
    /*setup on-shot timer*/
    obj->expire.it_interval.tv_sec = 0;
    obj->expire.it_interval.tv_nsec = 0;
    obj->expire.it_value.tv_sec = ptr->period;
    obj->expire.it_value.tv_nsec = 0;
    if ((err = timer_settime(obj->id[ptr->idx], 0, &obj->expire, NULL))){
        MND_ERR("timer_settime = %d(%s)\n", errno, strerror(errno));
        return -1;
    }
    MND_MSG("(%d, 0x%.8X, %d)\n", ptr->idx, obj->id[ptr->idx], ptr->period);
    return 0;    
}
/*****************************************************************************/
static int mnl_read_attr(const char *name, unsigned char *attr) 
{
    int fd = open(name, O_RDWR);
    unsigned char buf;
    int err = 0;
    
    if (fd == -1) {
        MND_ERR("open %s err = %s\n", name, strerror(errno));
        return err;    
    }
    do {
        err = read(fd, &buf, sizeof(buf));
    } while(err < 0 && errno == EINTR);
    if (err != sizeof(buf)) { 
        MND_ERR("read fails = %s\n", strerror(errno));
        err = -1;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        MND_ERR("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-1);
    }
    if (!err)
        *attr = buf - '0';
    else 
        *attr = 0xFF;
    return err;
}
/*****************************************************************************/
static int mnl_write_attr(const char *name, unsigned char attr) 
{
    int err, fd = open(name, O_RDWR);
    char buf[] = {attr + '0'};
    
    if (fd == -1) {
        MND_ERR("open %s err = %s\n", name, strerror(errno));
        return -errno;
    }
    do { err = write(fd, buf, sizeof(buf) ); }
    while (err < 0 && errno == EINTR);
    
    if (err != sizeof(buf)) { 
        MND_ERR("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        MND_ERR("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    MND_MSG("write '%d' to %s okay\n", attr, name);    
    return err;
}
/*****************************************************************************/
static int mnl_set_pwrctl(unsigned char pwrctl) 
{    
    if (pwrctl < GPS_PWRCTL_MAX) {                
        return mnl_write_attr(MNL_ATTR_PWRCTL, pwrctl);
    } else {
        MND_ERR("invalid pwrctl = %d\n", pwrctl);
        errno = -EINVAL;
        return -1;
    }
}
/*****************************************************************************/
static int mnl_get_pwrctl(unsigned char *pwrctl) 
{
    return mnl_read_attr(MNL_ATTR_PWRCTL, pwrctl);
}
/*****************************************************************************/
static int mnl_set_suspend(unsigned char suspend)
{
    if (suspend < GPS_PWR_MAX) {
        return mnl_write_attr(MNL_ATTR_SUSPEND, suspend);
    } else {
        MND_ERR("invalid suspend = %d\n", suspend);
        errno = -EINVAL;
        return -1;
    }
}
/*****************************************************************************/
static int mnl_get_suspend(unsigned char *suspend) 
{
    return mnl_read_attr(MNL_ATTR_SUSPEND, suspend);
}
/*****************************************************************************/
static int mnl_set_state(unsigned char state)
{
    int err;
    if (state < GPS_STATE_MAX) {
        if ((err = mnl_write_attr(MNL_ATTR_STATE, state)))
            return err;
        mnld_data.state = state;
        return 0;
    } else {
        MND_ERR("invalid state = %d\n", state);
        errno = -EINVAL;
        return -1;
    }
}
/*****************************************************************************/
static int mnl_get_state(unsigned char *state) 
{
    return mnl_read_attr(MNL_ATTR_STATE, state);
}
/*****************************************************************************/
static int mnl_set_pwrsave(unsigned char pwrsave)
{
    if (pwrsave < GPS_PWRSAVE_MAX) {
        return mnl_write_attr(MNL_ATTR_PWRSAVE, pwrsave);
    } else {
        MND_ERR("invalid pwrsave = %d\n", pwrsave);
        errno = -EINVAL;
        return -1;
    }
}
/*****************************************************************************/
static int mnl_get_pwrsave(unsigned char *pwrsave) 
{
    return mnl_read_attr(MNL_ATTR_PWRSAVE, pwrsave);
}
/*****************************************************************************/
static int mnl_set_status(char *buf, int len)
{
    const char *name = MNL_ATTR_STATUS;
    int err, fd = open(name, O_RDWR);
    
    if (fd == -1) {
        MND_ERR("open %s err = %s\n", name, strerror(errno));
        return -errno;
    }
    do { 
        err = write(fd, buf, len ); 
    } while (err < 0 && errno == EINTR);
    
    if (err != len) { 
        MND_ERR("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        MND_ERR("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    return err;  
}
/*****************************************************************************/
//CMCC: get timestamp
void get_time_stamp(char time_str1[], char time_str2[]){
    struct tm *tm_pt;
    time_t time_st;
    struct timeval tv;
    
    MND_MSG("Get time now");
    time(&time_st);
    gettimeofday(&tv, NULL);
    tm_pt = gmtime(&time_st);
    
    MND_MSG("time_st = %ld, tv.tv_sec = %ld", time_st, tv.tv_sec);
    
    //tm_pt = gmtime(&tv.tv_sec);
    tm_pt = localtime(&tv.tv_sec);
    //char date[20]={0};
    //MND_MSG("Get time now 1");
    memset(time_str1, 0, sizeof(char)*30);
    memset(time_str2, 0, sizeof(char)*30);
    //MND_MSG("Get time now 2");            
    sprintf(time_str1, "%d%02d%02d%02d%02d%02d.%1d", tm_pt->tm_year+1900, tm_pt->tm_mon+1, tm_pt->tm_mday, tm_pt->tm_hour, tm_pt->tm_min, tm_pt->tm_sec,tv.tv_usec/100000);
    sprintf(time_str2, "%d%02d%02d%02d%02d%02d.%03d", tm_pt->tm_year+1900, tm_pt->tm_mon+1, tm_pt->tm_mday, tm_pt->tm_hour, tm_pt->tm_min, tm_pt->tm_sec,tv.tv_usec/1000);
    //strcpy(time_stp, date);
    //MND_MSG("Get time now 3");    
}
/*****************************************************************************/
//CMCC: get ttff
int get_ttff_time(){
    time_t current_time;
    time(&current_time);
    ttff = current_time - start_time;
    return ttff;
}
//CMCC:get file size
unsigned long get_file_size(const char* filename){
    struct stat buf;
    if(stat("/sdcard/GPS.LOG", &buf)<0){
        MND_ERR("get file size error: %s", strerror(errno)); 
        return 0; 
    } //else{
        //if (buf.st_size < 64*1024){
        //    MND_MSG("file size = %lu", buf.st_size);
        //}else{
        //    MND_MSG("File size is lager than 64K");
        //    truncate(file_sd, 0);
        //}
    //}
    return (unsigned long)buf.st_size;
}
//CMCC: write file
int write_gps_log_to_sdcard(char* content){
    int fd_sd = 0;
    int ret = 0;
    char file_sd[] = "/sdcard/GPS.LOG";
    unsigned long len = 0;
 	int retry = 0;
    if(access(file_sd, 0) == -1){
        MND_MSG("file does not exist!\n");
    }else{
        MND_MSG("GPS.LOG file exist");
        //int res = chmod(file_sd, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP);
        //MND_MSG("chmod res = %d, %s", res, strerror(errno));;
    }
    
    len = get_file_size(file_sd);
    MND_MSG("file size = %lu", len);
    if (len >= (unsigned long)65535){
    	  MND_MSG("File size is lager than 64K");
        truncate(file_sd, 0);            
    }else{
        MND_MSG("file size = %lu", len);
    }
		 
    fd_sd = open(file_sd, O_RDWR | O_CREAT | O_APPEND | O_NONBLOCK, S_IRUSR | S_IROTH | S_IRGRP);
    if (fd_sd != -1){
    	  do {
            ret = write(fd_sd, content, strlen(content));
            MND_ERR("write to /sdcard/ fail, retry: %d, %s", retry, strerror(errno));
            retry++;
        }while((retry < 2) && (ret < 0)); 
        close(fd_sd);	
    } else {
        MND_ERR("open file_sd fail: %s, %d!", strerror(errno), errno);
        ret = -1;
    }
    return ret;
}
/*****************************************************************************/
static int mnl_attr_init() 
{
    int err;
    char buf[48];
    time_t tm;
    struct tm *p;
    
    time(&tm);
    p = localtime(&tm);    
    if(p == NULL)
    {
        return -1;
    }
    snprintf(buf, sizeof(buf), "(%d/%d/%d %d:%d:%d) - %d/%d",
            p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
            0, MNL_RESTART_NONE);
    if ((err = mnl_set_status(buf, sizeof(buf))))
        return err;
    if ((err = mnl_set_pwrctl(GPS_PWRCTL_OFF)))
        return err;
    if ((err = mnl_set_suspend(GPS_PWR_RESUME)))
        return err;
    if ((err = mnl_set_state(GPS_STATE_PWROFF)))
        return err;
    if ((err = mnl_set_pwrsave(GPS_PWRSAVE_SLEEP)))
        return err;
    return 0;    
}
/*****************************************************************************/
static int mnl_set_active()
{
    if (mnld_data.state == GPS_STATE_SLEEP || mnld_data.state == GPS_STATE_PWROFF) {
        MND_MSG("ignore active: state(%d)\n", mnld_data.state);
        return 0;
    } else {
        return mnl_set_alarm(MNL_ALARM_MONITOR);    
    }
}
/*****************************************************************************/
static int mnl_init() 
{
    int err;
    int s[2];
    MNLD_DATA_T *obj = &mnld_data;

    if ((err = mnl_attr_init()))
        return err;
        
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, s))
        return -1;
    
    fcntl(s[0], F_SETFD, FD_CLOEXEC);
    fcntl(s[0], F_SETFL, O_NONBLOCK);
    fcntl(s[1], F_SETFD, FD_CLOEXEC);
    fcntl(s[1], F_SETFL, O_NONBLOCK);
    
    obj->sig_snd_fd = s[0];
    obj->sig_rcv_fd = s[1];

    /*setup property*/
   /* if (!mnl_utl_load_property(&mnld_cfg)) {*/
        mnl_alarm[MNL_ALARM_INIT].period = mnld_cfg.timeout_init;
        mnl_alarm[MNL_ALARM_MONITOR].period = mnld_cfg.timeout_monitor;
        mnl_alarm[MNL_ALARM_SLEEP].period = mnld_cfg.timeout_sleep;
        mnl_alarm[MNL_ALARM_PWROFF].period = mnld_cfg.timeout_pwroff;
        mnl_alarm[MNL_ALARM_WAKEUP].period = mnld_cfg.timeout_wakeup;
        mnl_alarm[MNL_ALARM_TTFF].period = mnld_cfg.timeout_ttff;
/*    }*/
    return 0;
    
}
/*****************************************************************************/
static int launch_daemon_thread(void) 
{
    pid_t pid;
    int p2c[2] = {C_INVALID_FD,C_INVALID_FD};
    int c2p[2] = {C_INVALID_FD,C_INVALID_FD};
    MND_TRC();
        
        
    if (pipe(p2c) < 0 || pipe(c2p) < 0) {
        MND_ERR("create pipe: %d (%s)\n", errno, strerror(errno));
        goto error;
    } else {
        int flags;
        flags = fcntl(p2c[0], F_GETFD);
        fcntl(p2c[0], F_SETFD, (flags & ~FD_CLOEXEC));
        flags = fcntl(p2c[1], F_GETFD);
        fcntl(p2c[1], F_SETFD, (flags & ~FD_CLOEXEC));
        flags = fcntl(c2p[0], F_GETFD);
        fcntl(c2p[0], F_SETFD, (flags & ~FD_CLOEXEC));
        flags = fcntl(c2p[1], F_GETFD);
        fcntl(c2p[1], F_SETFD, (flags & ~FD_CLOEXEC));
    }
    if ((pid = fork()) < 0) {
        MND_ERR("fork fails: %d (%s)\n", errno, strerror(errno));
        goto error;
    } else if (pid == 0) { /*child process*/
        int err;
        char fd0[12] = {0};
        char fd1[12] = {0};
		char char_assist_data_bit_map[12] = {0};
		char *argv[] = {"/system/xbin/libmnla", "libmnlp", fd0, fd1, char_assist_data_bit_map};
		/*here we should know which combo chip is using*/
		snprintf(fd0, sizeof(fd0), "%d", p2c[0]);
        snprintf(fd1, sizeof(fd1), "%d", c2p[1]);
		snprintf(char_assist_data_bit_map, sizeof(char_assist_data_bit_map), "%d", assist_data_bit_map);

		char chip_id[100]={0};
		int fd;
		if((fd = open(GPS_PROPERTY, O_RDONLY)) == -1)
			MND_ERR("open % error, %s\n", GPS_PROPERTY, strerror(errno));	
		if(read(fd, chip_id, sizeof(chip_id)) == -1)
			MND_ERR("open % error, %s\n", GPS_PROPERTY, strerror(errno));
		close(fd);

		MND_MSG("chip_id is %s\n", chip_id);
		if (strcmp(chip_id, "0x6620") ==0 )
		{
			MND_MSG("we get MT6620\n");
			char *mnl6620 = "/system/xbin/libmnlp_mt6620";
			argv[0] = mnl6620;	
		}
		else if(strcmp(chip_id, "0x6628") == 0)
		{
			MND_MSG("we get MT6628\n");
			char *mnl6628 = "/system/xbin/libmnlp_mt6628";
			argv[0] = mnl6628;			
		}	
		else if(strcmp(chip_id, "0x6630") == 0)
		{
			MND_MSG("we get MT6630\n");
			char *mnl6630 = "/system/xbin/libmnlp_mt6630";
			argv[0] = mnl6630;				
		}
		else if(strcmp(chip_id, "0x6572") == 0)
		{
			MND_MSG("we get MT6572\n");
			char *mnl6572 = "/system/xbin/libmnlp_mt6572";
			argv[0] = mnl6572;				
		}
		else if(strcmp(chip_id, "0x6571") == 0)
		{
			MND_MSG("we get MT6571\n");
			char *mnl6571 = "/system/xbin/libmnlp_mt6571";
			argv[0] = mnl6571;				
		}
		else if(strcmp(chip_id, "0x6582") == 0)
		{
			MND_MSG("we get MT6582\n");
			char *mnl6582 = "/system/xbin/libmnlp_mt6582";
			argv[0] = mnl6582;				
		}
                else if(strcmp(chip_id, "0x6592") == 0)
		{
			MND_MSG("we get MT6592\n");
			char *mnl6592 = "/system/xbin/libmnlp_mt6592";
			argv[0] = mnl6592;				
		}

		else if(strcmp(chip_id, "0x3332") == 0)
		{
			MND_MSG("we get MT3332\n");
			char *mnl3332 = "/system/xbin/libmnlp_mt3332";
			argv[0] = mnl3332;				
		}

		else
		{
			MND_ERR("chip is unknown, chip id is %s\n", chip_id);
			goto error; 
		}


        close(p2c[1]); /*close the write channel in p2c because the pipe is used for reading data from parent*/
        close(c2p[0]); /*close the read channel in c2p because the pipe is used for writing data to parent*/
        

        MND_MSG("execute: %s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
        err = execl(argv[0], argv[1], argv[2], argv[3], argv[4], NULL);
        if (err == -1){
            MND_MSG("execl error: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    } else { /*parent process*/
        MNLD_DATA_T *obj = &mnld_data;
        close(p2c[0]); /*close the read channel in p2c because the pipe is used for writing data to child*/
        close(c2p[1]); /*close the write channel in c2p because the pipe is used for reading data from child*/         

        mnld_monitor.pid = pid;
        obj->mnl_rcv_fd = c2p[0];
        obj->mnl_snd_fd = p2c[1];
        if (epoll_add(obj->epoll_fd, obj->mnl_rcv_fd)) {
            MND_MSG("add mnl_rcv_fd fails: %d (%s)\n", errno, strerror(errno));
            return -1;
        }            
        MND_MSG("mnl_pid = %d\n", pid);
        mnl_set_alarm(MNL_ALARM_INIT);           
        return 0;
    }
error:
    if (p2c[0] != C_INVALID_FD) { close(p2c[0]); }
    if (p2c[1] != C_INVALID_FD) { close(p2c[1]); }
    if (c2p[0] != C_INVALID_FD) { close(c2p[0]); }
    if (c2p[1] != C_INVALID_FD) { close(c2p[1]); }
    return -1;
}
/*****************************************************************************/
static int kill_mnl_process(void)
{
    if (mnld_monitor.pid == C_INVALID_PID) {
        MND_MSG("no mnl process created, ignore\n");
        return 0;
    } else {
        pid_t child_pid;
        int err, cnt = 0, max = 10;
        MNLD_DATA_T *obj = &mnld_data;
        
        MND_MSG("kill pid: %d\n", mnld_monitor.pid);

        kill(mnld_monitor.pid, SIGTERM);	            
        while (!mnld_monitor.terminated) {
            if (cnt++ < max) {  
                /*timeout: 1 sec; 
                  notice that libmnlp needs some sleep time after MTK_PARAM_CMD_RESET_DSP*/
                usleep(100000);
                continue;
            } else {
                kill(mnld_monitor.pid, SIGKILL);	
                usleep(100000);            
            }
        }
        MND_MSG("waiting counts: %d\n", cnt);
        child_pid = wait(&err);
        if (child_pid == -1)
            MND_MSG("wait error: %s\n",strerror(errno));
        MND_MSG("child process : %d is killed\n", child_pid);
        if (WIFEXITED(err))
            MND_MSG("Normal termination with exit status = %d\n", WEXITSTATUS(err));
        if (WIFSIGNALED(err))
            MND_MSG("Killed by signal %d%s\n", WTERMSIG(err), WCOREDUMP(err) ? "dump core" : "");
        if (WIFSTOPPED(err))
            MND_MSG("Stopped by signal = %d\n", WSTOPSIG(err));        
        mnld_monitor.pid = C_INVALID_PID;  
        in_stop_proc = 0;		
        mnld_monitor.terminated = 0;             
        epoll_del(obj->epoll_fd, obj->mnl_rcv_fd);    /*mnl process is killed, remove the fd out of epoll_wait queue*/
        close(obj->mnl_rcv_fd);
        close(obj->mnl_snd_fd);
        obj->mnl_rcv_fd = C_INVALID_FD;
        obj->mnl_snd_fd = C_INVALID_FD;
        
        mnl_set_pwrctl(GPS_PWRCTL_OFF);
        mnl_set_state(GPS_STATE_PWROFF);

        // Kill done, Update GPS_MNL_PROCESS_STATUS prop
        property_set(GPS_MNL_PROCESS_STATUS, "0000");
        MND_MSG("libmnlp is killed. Set %s 0000\n", GPS_MNL_PROCESS_STATUS);
        assist_data_bit_map = FLAG_HOT_START;

		char cmd = MNL_CMD_MNL_DIE;
        if(obj->cur_accept_socket != -1) 
		{
	   		if(send(obj->cur_accept_socket, &cmd, sizeof(cmd), 0) != sizeof(cmd))
	        	MND_ERR("Send to HAL failed, %s\n", strerror(errno));
			else
				MND_MSG("Send to HAL successfully\n");	    	
        }
        return 0;
    }
    return -1;
}
/*****************************************************************************/
static int stop_mnl_process(void)
{
    int err;
    unsigned char pwrsave;

    MND_TRC();
    if ((err = mnl_set_state(GPS_STATE_STOP)))
        return err;    
    if ((err = mnl_get_pwrsave(&pwrsave)))
        return err;
        
    if (pwrsave == GPS_PWRSAVE_SLEEP) {        
        if ((err = mnl_set_alarm(MNL_ALARM_SLEEP)))
            return err;
    } else {
        if ((err = mnl_set_alarm(MNL_ALARM_PWROFF)))
            return err;
    }
    return 0;    
}
/*****************************************************************************/
#define restart_mnl_process(X) restart_mnl_process_ex(X, __LINE__)
/*****************************************************************************/
static int restart_mnl_process_ex(unsigned int reborn, unsigned int line)
{
    int err;
    char buf[48];
    time_t tm;
    struct tm *p;
    
    time(&tm);
    p = localtime(&tm);
    if(p == NULL)
    {
        return -1;
    }
    MND_MSG("(%d,%d) (%d/%d/%d %d:%d:%d)",reborn, line,
        p->tm_year+1900, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    
    if ((err = kill_mnl_process()))
        return err;
    /*because the LDO (V3GTX) is shared with modem, 
      only GPS_PWRCTL_RST_FORCE will actually reset hardware*/
	if ((err = mnl_set_pwrctl(GPS_PWRCTL_RST_FORCE)))   
		return err;
    if ((err = launch_daemon_thread()))
        return err;

    mnld_monitor.count++;

    snprintf(buf, sizeof(buf), "(%d/%d/%d %d:%d:%d) - %d/%d",
            p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
            mnld_monitor.count, reborn);
    return mnl_set_status(buf, strlen(buf));    
}
/*****************************************************************************/
static int start_mnl_process(void) 
{
    int err = 0;
    MND_TRC();    
    if ((err = mnl_alarm_stop_all())) /*if current state is going to pwrsave*/
        return err; 
    if (mnld_monitor.pid == C_INVALID_PID) {
        if ((err = mnl_set_pwrctl(GPS_PWRCTL_RST))) /*if current state is power off*/
            return err;
        if ((err = launch_daemon_thread()))
            return err;
        return mnl_set_state(GPS_STATE_START);
    } else {
        unsigned char state = GPS_STATE_UNSUPPORTED;
        unsigned char pwrctl = GPS_PWRCTL_UNSUPPORTED;
        err = mnl_get_pwrctl(&pwrctl);
        if ((err) || (pwrctl >= GPS_STATE_MAX)) {
            MND_ERR("mnl_get_pwrctl() = %d, %d\n", err, pwrctl);
            return -1;            
        }
        err = mnl_get_state(&state);
        if ((err) || (state >= GPS_STATE_MAX)) {
            MND_ERR("mnl_get_state() = %d, %d\n", err, state);
            return -1;
        }
        MND_MSG("start: pwrctl (%d), state (%d)\n", pwrctl, state);

        if (pwrctl == GPS_PWRCTL_OFF) {
            if ((err = mnl_set_pwrctl(GPS_PWRCTL_ON))) /*if current state is power off*/
                return err;            
            return restart_mnl_process(MNL_RESTART_FORCE);
        }
        if (state == GPS_STATE_SLEEP) {
            char buf[] = {MNL_CMD_WAKEUP};
            if ((err = mnl_send_cmd(buf, sizeof(buf))))
                return err;
            if ((err = mnl_set_alarm(MNL_ALARM_WAKEUP)))
                return err;
            return mnl_set_state(GPS_STATE_START);                        
        } 
                
        if(state == GPS_STATE_STOP) //LiChunhui, for sync GPS state
        {
            MND_MSG("state from STOP to START again\n");
            mnl_set_state(GPS_STATE_START);
        }
                
        MND_MSG("mnl_daemon is already started!!\n");
    }
    return 0;
}
/*****************************************************************************/
static void sighlr(int signo) 
{
    int err = 0;
    pthread_t self = pthread_self();
    //MND_MSG("Signal handler of %.8x -> %s\n", (unsigned int)self, sys_siglist[signo]);        
    if (signo == SIGUSR1) {
        char buf[] = {MNL_CMD_ACTIVE};
        err = slf_send_cmd(buf, sizeof(buf));
    } else if (signo == SIGALRM) {
        char buf[] = {MNL_CMD_TIMEOUT};
        err = slf_send_cmd(buf, sizeof(buf));
    } else if (signo == SIGUSR2) {
        char buf[] = {MNL_CMD_TIMEOUT};
        err = slf_send_cmd(buf, sizeof(buf));
    } else if (signo == SIGCHLD) {
        mnld_monitor.terminated = 1;
    }
}
/*****************************************************************************/
static int setup_signal_handler(void) 
{
    struct sigaction actions;   
    int err;
    
    /*the signal handler is MUST, otherwise, the thread will not be killed*/
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = sighlr;    
    if ((err = sigaction(SIGUSR1, &actions, NULL))) {
        MND_MSG("register signal hanlder for SIGUSR1: %s\n", strerror(errno));
        return -1;
    }
    if ((err = sigaction(SIGUSR2, &actions, NULL))) {
        MND_MSG("register signal hanlder for SIGUSR2: %s\n", strerror(errno));
        return -1;
    }
    if ((err = sigaction(SIGALRM, &actions, NULL))) {
        MND_MSG("register signal handler for SIGALRM: %s\n", strerror(errno));
        return -1;
    }  
    if ((err = sigaction(SIGCHLD, &actions, NULL))) {
        MND_MSG("register signal handler for SIGALRM: %s\n", strerror(errno));
        return -1;
    }      
    return 0;
} 
/*****************************************************************************/
static int socket_handler(int sock, int* eof)
{   
    int ret;
    char cmd = MNL_CMD_UNKNOWN;
    *eof = 0;
    
    char str[256] = {0};
    char time_str1[30] = {0};
    char time_str2[30] = {0};
    char position[32] = {0};
    
    int retry = 0;
     
    do {
        ret = read( sock, &cmd, sizeof(cmd) );
    } while (ret < 0 && errno == EINTR);
    if (ret == 0) {
        MND_MSG("remote socket closed!!");
        *eof = 1;
        return 0;
    } else if (ret != sizeof(cmd)) {
        MND_MSG("fails: %d %d(%s)\n", ret, errno, strerror(errno));
        return -1;
    }
    
    MND_MSG("args: %d\n", cmd);

    if (cmd == MNL_CMD_INIT) {
        if ((ret = mnl_set_pwrctl(GPS_PWRCTL_OFF))) /*default power off*/
            return ret;
        return mnl_set_state(GPS_STATE_INIT);
    } else if (cmd == MNL_CMD_CLEANUP) {
        ret = kill_mnl_process();
        return ret; 
    } else if (cmd == MNL_CMD_START) {
        int status_ret = 0;
        status_ret = start_mnl_process();
        if (status_ret == 0){
            property_set(GPS_MNL_PROCESS_STATUS, "1111");
            MND_MSG("mnl_daemon run. Set %s to 1111\n", GPS_MNL_PROCESS_STATUS);
        }
         time(&start_time);
    if(get_prop()){
        //Log start
        //CMCC: Gps start
        memset(str, 0x00, sizeof(str));
        memset(time_str1, 0x00, sizeof(time_str1));
        memset(time_str2, 0x00, sizeof(time_str2));
      
        get_time_stamp(time_str1, time_str2);
        //time(&start_time);
        sprintf(str, "[%s]0x00000000: %s #gps start\r\n", time_str2, time_str2);        
        MND_MSG("write begin: %s", str);
        if (write_gps_log_to_sdcard(str) < 0)
            MNL_ERR("/sdcard/GPS.LOG write start fail");
        MND_MSG("write done"); 
    }     
        return status_ret;
    } else if (cmd == MNL_CMD_STOP) {
       // return stop_mnl_process();
        ttff = 0;
        in_stop_proc = 1;
       mnl_alarm_stop_all();
       if(get_prop()){
        //Log Stop
        memset(str, 0x00, sizeof(str));
        memset(time_str1, 0x00, sizeof(time_str1));
        memset(time_str2, 0x00, sizeof(time_str2));
        get_time_stamp(time_str1, time_str2);
        sprintf(str, "[%s]0x00000001: %s #gps stop\r\n", time_str2, time_str2);
        MND_MSG("write begin: %s", str);
        if (write_gps_log_to_sdcard(str) < 0)
            MNL_ERR("/sdcard/GPS.LOG write stop fail");
        MND_MSG("write done");
    }
       return kill_mnl_process();
    } else if (cmd == MNL_CMD_RESTART) {
        return restart_mnl_process(MNL_RESTART_FORCE);
    } else if ((cmd == MNL_CMD_RESTART_HOT)  || (cmd == MNL_CMD_RESTART_WARM) ||
               (cmd == MNL_CMD_RESTART_COLD) || (cmd == MNL_CMD_RESTART_FULL) ||
               (cmd == MNL_CMD_RESTART_AGPS)) {
        char buf[] = {cmd};
		if (mnld_monitor.pid == C_INVALID_PID || in_stop_proc == 1) 
		{
			switch(cmd)
			{
				case MNL_CMD_RESTART_HOT:
					MND_MSG("hot start\n");
					assist_data_bit_map = FLAG_HOT_START;
					break;
				case MNL_CMD_RESTART_WARM:
					MND_MSG("warm start\n");
					assist_data_bit_map = FLAG_WARM_START;	
					break;
				case MNL_CMD_RESTART_COLD:
					MND_MSG("cold start\n");
					assist_data_bit_map = FLAG_COLD_START;	
					break;
				case MNL_CMD_RESTART_FULL:
					MND_MSG("full start\n");
					assist_data_bit_map = FLAG_FULL_START;	
					break;
				case MNL_CMD_RESTART_AGPS:
					assist_data_bit_map = FLAG_AGPS_START;	
					break;
				default:
					MND_MSG("unknowned delete aiding data\n");
					break;
			}
			return 0;
		}
		else
		{
			MND_MSG("MNLD send command to libmnlp: %d\n", (int)(*buf));
        	return mnl_send_cmd(buf, sizeof(buf));
		}
    } else if (cmd == MNL_CMD_READ_EPO_TIME) {
        char buf[] = {cmd};
	 return mnl_send_cmd(buf, sizeof(buf)); 
    } else if (cmd == MNL_CMD_UPDATE_EPO_FILE) {
        char buf[] = {cmd};
	 return mnl_send_cmd(buf, sizeof(buf));
    } else if(cmd == MNL_CMD_GPS_LOG_WRITE){
    	 if (ttff == 0){
            ttff = get_ttff_time();
            MND_MSG("TTFF = %d", ttff);
        }else{
            MND_MSG("Already get ttff: %d", ttff);
        }
     if(get_prop()){
        //read position        
        do {
            usleep(100000);
            ret = read(sock, &position, sizeof(position) );
            MND_MSG("read: %d, pos = %s", ret, position);            
            retry++;
        } while ((ret < 0) && (retry < 5));
        
        //write sdcard        
        memset(time_str1, 0x00, sizeof(time_str1));
        memset(time_str2, 0x00, sizeof(time_str2));

        get_time_stamp(time_str1, time_str2);
        /*
        if (ttff == 0){
            ttff = get_ttff_time();
            MND_MSG("TTFF = %d", ttff);
        }else{
            MND_MSG("Already get ttff: %d", ttff);
        }*/
        sprintf(str, "[%s]0x00000002: %s, %s, %d #position(time_stamp, lat, lon, ttff)\r\n", time_str2, time_str2, position, ttff);
        MND_MSG("gps postion  = %s", str);
        MND_MSG("/sdcard/GPS.LOG write: %s", str);
        if (write_gps_log_to_sdcard(str)<0)
            MNL_ERR("/sdcard/GPS.LOG write position fail");
        MND_MSG("/sdcard/GPS.LOG write done");
    }
        return 0;    	       
    } else {
        MND_MSG("unknown command: 0x%2X\n", cmd);
        errno = -EINVAL;
        return errno;
    }
}
/*****************************************************************************/
static int sigrcv_handler(int fd)   /*sent from signal handler or internal event*/
{
    int err;
    char cmd = MNL_CMD_UNKNOWN;
    do {
        err = read(  fd, &cmd, sizeof(cmd) );
    } while (err < 0 && errno == EINTR);
    if (err == 0) {
        MND_ERR("EOF"); /*it should not happen*/
        return 0;
    } else if (err != sizeof(cmd)) {
        MND_ERR("fails: %d %d(%s)\n", err, errno, strerror(errno));
        return -1;
    }
    
    MND_MSG("arg: %d\n", cmd);

    if (cmd == MNL_CMD_ACTIVE) {
        if ((err = mnl_set_active()))
            return err;
        return 0;
    } else if (cmd == MNL_CMD_TIMEOUT_INIT) {
        return restart_mnl_process(MNL_RESTART_TIMEOUT_INIT);
    } else if (cmd == MNL_CMD_TIMEOUT_MONITOR) {
        return restart_mnl_process(MNL_RESTART_TIMEOUT_MONITOR);
    } else if (cmd == MNL_CMD_TIMEOUT_WAKEUP) {
        return restart_mnl_process(MNL_RESTART_TIMEOUT_WAKEUP);
    } else if (cmd == MNL_CMD_TIMEOUT_TTFF) {
        return restart_mnl_process(MNL_RESTART_TIMEOUT_TTFF);
    } else if (cmd == MNL_CMD_SLEEP) {
        unsigned char state;
        err = mnl_get_state(&state);
        if ((err) || (state >= GPS_STATE_MAX)) 
        {
            MND_ERR("mnl_get_state() = %d, %d\n", err, state);
            return -1;
        }
        if (state == GPS_STATE_STOP)
        {
        char buf[] = {MNL_CMD_SLEEP};
        if ((err = mnl_send_cmd(buf, sizeof(buf))))
            return err;
        if ((err = mnl_alarm_stop_all()))
            return err;
        if ((err = mnl_set_state(GPS_STATE_SLEEP)))
            return err;
        /*enable timer for entering next power-saving stage*/
        if ((err = mnl_set_alarm(MNL_ALARM_PWROFF)))
            return err;        
        }else{
            MND_MSG("GSP(%u) isn't in stop state. Can't go to sleep\n", state);
        }    
        return 0;    
    } else if (cmd == MNL_CMD_WAKEUP) {
        char buf[] = {MNL_CMD_WAKEUP};
        if ((err = mnl_send_cmd(buf, sizeof(buf))))
            return err;
        if ((err = mnl_set_state(GPS_STATE_START)))
            return err;
        return 0;  
    } else if (cmd == MNL_CMD_PWROFF) {
        if ((err = mnl_alarm_stop_all()))
            return err;    
        if ((err = kill_mnl_process()))
            return err;     
        return 0;    
    } else {
        MND_ERR("unknown command: 0x%2X\n", cmd);
        errno = -EINVAL;
        return errno;
    }
}
/*****************************************************************************/
static int mnlrcv_handler(int fd) /*sent from libmnlx*/
{
    int ret;
    char cmd = MNL_CMD_UNKNOWN;
    time_t time[2];
    int cnt = 0, max = 5;
	
    #if 1	
    /*send EPO msg to HAL*/	
    MNLD_DATA_T *obj = &mnld_data;
    #endif
	
    do {
        ret = read(  fd, &cmd, sizeof(cmd) );
    } while (ret < 0 && errno == EINTR);
    if (ret == 0) {
        MND_ERR("EOF"); /*it should not happen*/
        return 0;
    } else if (ret != sizeof(cmd)) {
        MND_ERR("fails: %d %d(%s)\n", ret, errno, strerror(errno));
        return -1;
    }
    
    MND_VER("mnlrcv_handler(%d)\n", cmd);

    if (cmd == MNL_CMD_ACTIVE) {
        if ((ret = mnl_set_alarm(MNL_ALARM_MONITOR)))
            return ret;
        return 0;
    } else if (cmd == MNL_CMD_SLEPT) {
        /*since libmnlx is slept, stop watch alarm*/
        return mnl_alarm_stop_watch();
    } else if (cmd == MNL_CMD_RCV_TTFF) {
        return mnl_set_alarm(MNL_ALARM_TTFF);
    } else if (cmd == MNL_CMD_READ_EPO_TIME_DONE) {
        //read time here, mnl_epo_time.uSecond_start, mnl_epo_time.uSecond_expire
	do {
            MND_MSG("Read EPO time");
            usleep(100000);
            ret = read(  fd, time, sizeof(time) );
	    MND_MSG("Read time from libmnlp, ret = %d, cnt = %d\n", ret, cnt);		
        } while (ret < 0 && cnt++ < max);
        MND_MSG("MNL_CMD_READ_EPO_TIME_DONE\n");
        
	//time[0] = mnl_epo_time.uSecond_start;
	//time[1] = mnl_epo_time.uSecond_expire;
	//MND_MSG("mnl_epo_time.uSecond_start = %d, mnl_epo_time.uSecond_expire = %d\n", mnl_epo_time.uSecond_start, mnl_epo_time.uSecond_expire);
	MND_MSG("time[0] = %ld, time[1] = %ld\n", time[0], time[1]);
	//send to HAL
	if(ret == sizeof(time)) {
        if(obj->cur_accept_socket != -1) {
             //send time read ok to HAL first
             ret = send(obj->cur_accept_socket, &cmd, sizeof(cmd), 0);
             if(ret != sizeof(cmd)) {
	          MND_ERR("Send time ok to HAL failed\n");
                 return MTK_GPS_ERROR;
	      }else{
                 MND_MSG("Send time ok to HAL successfully\n");
		   MND_MSG("sizeof(time) = %d\n", sizeof(time));		
		   ret = send(obj->cur_accept_socket, time, sizeof(time), 0);
	          if(ret != sizeof(time)){
	              MND_MSG("Send time to HAL failed\n");
                     return MTK_GPS_ERROR;
	          }else{
                     MND_MSG("Send time to HAL successfully, ret = %d \n", ret);
                     return MTK_GPS_SUCCESS;            
	          }   		
                 //return MTK_GPS_SUCCESS;             
	      }             
        } else {
	     MND_ERR("obj->cur_accept_socket == -1\n");
            return MTK_GPS_ERROR;
	 }
    }else{
        MND_ERR("Read time from libmnlp failed\n");
        cmd = MNL_CMD_READ_EPO_TIME_FAIL;
        if(obj->cur_accept_socket != -1) {
            ret = send(obj->cur_accept_socket, &cmd, sizeof(cmd), 0);
	    if(ret != sizeof(cmd)){
	        MND_ERR("Send to HAL failed\n");
                return MTK_GPS_ERROR;
	    }else{
                MND_MSG("Send to HAL successfully\n");
                return MTK_GPS_SUCCESS;             
	    }   
        }else{
            MND_ERR("obj->cur_accept_socket == -1\n");
            return MTK_GPS_ERROR; 
        }
        return -1;
    }
    } else if (cmd == MNL_CMD_READ_EPO_TIME_FAIL) {
        //send msg to HAL get time fail
        if(obj->cur_accept_socket != -1) {
	     ret = send(obj->cur_accept_socket, &cmd, sizeof(cmd), 0);
	     if(ret != sizeof(cmd)){
	         MND_ERR("Send to HAL failed\n");
                return MTK_GPS_ERROR;
	     }else{
                MND_MSG("Send to HAL successfully\n");
                return MTK_GPS_SUCCESS;             
	     }   
	 }else{
	     MND_ERR("obj->cur_accept_socket == -1\n");
            return MTK_GPS_ERROR; 
        }
    } else if ((cmd == MNL_CMD_UPDATE_EPO_FILE_DONE) || (cmd == MNL_CMD_UPDATE_EPO_FILE_FAIL)) {
        if (cmd == MNL_CMD_UPDATE_EPO_FILE_DONE) {
            MND_MSG("MNL_CMD_UPDATE_EPO_FILE_DONE\n");
        } else if (cmd == MNL_CMD_UPDATE_EPO_FILE_FAIL) {
            MND_MSG("MNL_CMD_UPDATE_EPO_FILE_FAIL\n"); 
        }
	 	
        //send to HAL that update done		
        if(obj->cur_accept_socket != -1) {
	     ret = send(obj->cur_accept_socket, &cmd, sizeof(cmd), 0);
	 }else{
	     MND_ERR("obj->cur_accept_socket == -1\n");
            return MTK_GPS_ERROR; 
	 }
	  
	 if(ret != sizeof(cmd))
	 {
	     MND_ERR("Send to HAL failed\n");
            return MTK_GPS_ERROR;
	 }else{
            MND_MSG("Send to HAL successfully\n");
            return MTK_GPS_SUCCESS;             
	 }   
    }	else {
        MND_ERR("unknown command: 0x%2X\n", cmd);
        errno = -EINVAL;
        return -1;
    }
}
/*****************************************************************************/
#define ERR_REMOTE_HANGUP   0x0F01
#define ERR_MNL_DIED        0x0F02
/*****************************************************************************/
static int process() 
{
    struct epoll_event   events[EPOLL_NUM];
    int                  ne, nevents;
    MNLD_DATA_T          *obj = &mnld_data;
    int eof = 0;

    nevents = epoll_wait( obj->epoll_fd, events, EPOLL_NUM, -1 );
    if (nevents < 0) {
        if (errno != EINTR)
            MND_ERR("epoll_wait() unexpected error: %s", strerror(errno));
        return -1;
    } else {
        //MND_MSG("epoll_wait() received %d events", nevents);
    }

    for (ne = 0; ne < nevents; ne++) {
        if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
            MND_ERR("wait: (%d %d %d), event: 0x%X from %d", obj->cur_accept_socket, 
                obj->sig_rcv_fd, obj->mnl_rcv_fd, events[ne].events, events[ne].data.fd);
            //MND_ERR("EPOLLERR or EPOLLHUP after epoll_wait(): %d\n", events[ne].data.fd);
            if (events[ne].data.fd == obj->cur_accept_socket) {
                /*current socket connection is hang-up, stop current session and 
                  wait another connection*/
                return ERR_REMOTE_HANGUP;
            } else if (events[ne].data.fd == obj->mnl_rcv_fd) {
                /*mnl process is already died*/
                return ERR_MNL_DIED;
            }
            return -1;
        }
        if ((events[ne].events & EPOLLIN) != 0) {
            int  fd = events[ne].data.fd;
            if (fd == obj->cur_accept_socket)
                return socket_handler(fd, &eof);
            if (fd == obj->sig_rcv_fd)
                return sigrcv_handler(fd);
            if (fd == obj->mnl_rcv_fd)
                return mnlrcv_handler(fd);
        }
    } 
    return -1;  /*nothing is hanlded*/
}
/*****************************************************************************/
void power_on_3332()
{
	int err;
	err = mnl_write_attr(MNL_ATTR_PWRCTL, GPS_PWRCTL_RST_FORCE);    
	if(err != 0)    
	{        
		MND_ERR("GPS_Open: GPS power-on error: %d\n", err);       
		return ;    
	}
	usleep(1000*100);
	return;
}
/*****************************************************************************/
void power_off_3332()
{
	int err;
	err = mnl_write_attr(MNL_ATTR_PWRCTL, GPS_PWRCTL_OFF);    
	if(err != 0)    
	{        
		MND_ERR("GPS_Open: GPS power-on error: %d\n", err);       
		return ;    
	}
	usleep(1000*100);
	return;
}

/*******************************************************************/
static int read_NVRAM()
{
    //int gps_nvram_fd = 0;
    F_ID gps_nvram_fd;
    int rec_size;
    int rec_num;
    int i;


    memset(&stGPSReadback, 0, sizeof(stGPSReadback));

	gps_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_GPS_LID, &rec_size, &rec_num, ISREAD);
	if(gps_nvram_fd.iFileDesc > 0)/*>0 means ok*/
	{
    	if(read(gps_nvram_fd.iFileDesc, &stGPSReadback , rec_size*rec_num) < 0)
			MND_ERR("read NVRAM error, %s\n", strerror(errno));;
        NVM_CloseFileDesc(gps_nvram_fd);
    
		if(strlen(stGPSReadback.dsp_dev) != 0)
		{
    
             MND_MSG("GPS NVRam (%d * %d) : \n", rec_size, rec_num);
             MND_MSG("dsp_dev : %s\n", stGPSReadback.dsp_dev);
           
		}
         else
         {
             MND_ERR("GPS NVRam mnl_config.dev_dsp == NULL \n");
			 return -1;
         }
     }
     else
     {
         MND_ERR("GPS NVRam gps_nvram_fd == %d \n", gps_nvram_fd);
		 return -1;
     }
	if(strcmp(stGPSReadback.dsp_dev, "/dev/stpgps") == 0)
	{
		MND_ERR("not 3332 UART port\n");
		return 1;
	}
	return 0;
}

/*****************************************************************************/
static int init_3332_interface(const int fd)

{		
	
	struct termios termOptions;
//	fcntl(fd, F_SETFL, 0);

	// Get the current options:
	tcgetattr(fd, &termOptions);

	// Set 8bit data, No parity, stop 1 bit (8N1):
	termOptions.c_cflag &= ~PARENB;
	termOptions.c_cflag &= ~CSTOPB;
	termOptions.c_cflag &= ~CSIZE;
	termOptions.c_cflag |= CS8 | CLOCAL | CREAD;

	MND_MSG("GPS_Open: c_lflag=%x,c_iflag=%x,c_oflag=%x\n",termOptions.c_lflag,termOptions.c_iflag,
							termOptions.c_oflag);
	//termOptions.c_lflag

	// Raw mode
	termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
	termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*raw input*/
	termOptions.c_oflag &= ~OPOST;  /*raw output*/

	tcflush(fd,TCIFLUSH);//clear input buffer
	termOptions.c_cc[VTIME] = 10; /* inter-character timer unused, wait 1s, if no data, return */
	termOptions.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */

   // Set baudrate to 38400 bps
	cfsetispeed(&termOptions, B115200);	/*set baudrate to 115200, which is 3332 default bd*/
	cfsetospeed(&termOptions, B115200);

	tcsetattr(fd, TCSANOW, &termOptions);	

	return 0;
}
/*****************************************************************************/
static int hw_test_3332(const int fd)
{
	ssize_t bytewrite, byteread;
	char buf[6] = {0};
	char cmd[] = {0xAA,0xF0,0x6E,0x00,0x08,0xFE,0x1A,0x00,0x00,0x00,0x00,
				0x00,0xC3,0x01,0xA5,0x02,0x00,0x00,0x00,0x00,0x5A,0x45,0x00,
				0x80,0x04,0x80,0x00,0x00,0x1A,0x00,0x00,0x00,0x00,0x00,0x05,0x00,
				0x96,0x00,0x6F,0x3C,0xDE,0xDF,0x8B,0x6D,0x04,0x04,0x00,0xD2,0x00,
				0xB7,0x00,0x28,0x00,0x5D,0x4A,0x1E,0x00,0xC6,0x37,0x28,0x00,0x5D,
				0x4A,0x8E,0x65,0x00,0x00,0x01,0x00,0x28,0x00,0xFF,0x00,0x80,0x00,
				0x47,0x00,0x64,0x00,0x50,0x00,0xD8,0x00,0x50,0x00,0xBB,0x00,0x03,
				0x00,0x3C,0x00,0x6F,0x00,0x89,0x00,0x88,0x00,0x02,0x00,0xFB,0x00,
				0x01,0x00,0x00,0x00,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x7A,0x16,0xAA,0x0F};
	char ack[] = {0xaa,0xf0,0x0e,0x00,0x31,0xfe};

	
	bytewrite = write(fd, cmd, sizeof(cmd));
	if (bytewrite == sizeof(cmd))
	{
		usleep(500*000);
		byteread = read(fd, buf, sizeof(buf));
		MND_MSG("ack:%02x %02x %02x %02x %02x %02x\n",
				 buf[0],buf[1], buf[2], buf[3], buf[4], buf[5]);
		if((byteread == sizeof(ack)) && (memcmp(buf, ack, sizeof(ack)) == 0))
		{ 
			MND_MSG("it's 3332\n"); 
			return 0;	/*0 means 3332,   1 means other GPS chips*/
		}
		return 1;
	}
	else
	{
		MND_ERR("write error, write API return is %d, error message is %s\n", bytewrite, strerror(errno));
		return 1;
	}
}

/*****************************************************************************/
static int hand_shake()
{
	int fd;
	int ret;
	int nv;
	nv = read_NVRAM();

	if(nv == 1)
		return 1;
	else if(nv == -1)
		return -1;
	else
		MND_MSG("read NVRAM ok\n");
		
	fd = open(stGPSReadback.dsp_dev, O_RDWR | O_NOCTTY);
	if (fd == -1) 
    {
		MND_ERR("GPS_Open: Unable to open - %s, %s\n", stGPSReadback.dsp_dev, strerror(errno));
        return -1; 
	}
	init_3332_interface(fd);	/*set UART parameter*/
		
	ret = hw_test_3332(fd);	/*is 3332? 	0:yes  	1:no*/
	close(fd);
	return ret;
	
}

/*****************************************************************************/
static int confirm_if_3332()
{
	int ret;
	power_on_3332();
	ret = hand_shake();
	power_off_3332();
	return ret;
}

/*****************************************************************************/
void chip_detector()
{	
	
	int get_time = 5;
	int res;
	char chip_id[PROPERTY_VALUE_MAX];/*combo chip ID*/
	char gps_id[PROPERTY_VALUE_MAX];/*GPS chip ID*/
/*
	while(get_time-- != 0 && (property_get(GPS_PROPERTY, gps_id, NULL) <= 0))
	{
		usleep(100000);
	}	
*/
	int fd = -1;
	fd = open(GPS_PROPERTY, O_RDWR|O_CREAT, 0600);
	if(fd == -1)
	{
		MND_ERR("open %s error, %s\n", GPS_PROPERTY, strerror(errno));	
		return;
	}
	int read_len;
	char buf[100] = {0};
	read_len = read(fd, buf, sizeof(buf));
	if(read_len == -1)
	{
		MND_ERR("read %s error, %s\n", GPS_PROPERTY, strerror(errno));	
		goto exit_chip_detector;
	}	
	else if(read_len != 0) /*print chip id then return*/
	{				
		MND_MSG("gps is %s\n", buf);
		goto exit_chip_detector;
	}
	else
		MND_MSG("we need to known which GPS chip is in use\n");
#if 0		
	if(strcmp(gps_id, "0xffff") != 0)	/*not default value, so just return*/
	{
		MND_MSG("gps is %s\n", gps_id);
		return;
	}
#endif
	while(get_time-- != 0 && (property_get("persist.mtk.wcn.combo.chipid", chip_id, NULL) <= 0))
	{
		usleep(100000);
	}

	MND_MSG("combo_chip_id is %s\n", chip_id);
	/*get chip from combo chip property, if 6620 or 6572 just set GPS chip as the same value*/
	if (strcmp(chip_id, "0x6620") ==0 )
	{
		MND_MSG("we get MT6620\n");
		if(write(fd, "0x6620", 10) == -1)		
			MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
		
		goto exit_chip_detector;
/*
		if(property_set(GPS_PROPERTY, "0x6620") < 0)
			MND_ERR("set_property error, %s\n", strerror(errno));
		return;
*/
	}
	if (strcmp(chip_id, "0x6572") ==0 )
	{
		MND_MSG("we get MT6572\n");
		if(write(fd, "0x6572", 10) == -1)		
			MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
		
		goto exit_chip_detector;
	}
	/*detect if there is 3332, yes set GPS property to 3332, then else read from combo chip to see which GPS chip used*/
	res = confirm_if_3332();	/*0 means 3332, 1 means not 3332, other value means error*/

	if(res == 0)
	{
		if(write(fd, "0x3332", 10) == -1)		
			MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
		
		goto exit_chip_detector;
	}
	else if (res == 1)
	{
		/*we can not distinguish 6628T and 6628Q yet*/
		if (strcmp(chip_id, "0x6628") ==0 )
		{
			MND_MSG("we get MT6628\n");
			if(write(fd, "0x6628", 10) == -1)		
				MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
			
			goto exit_chip_detector;
		}
		if (strcmp(chip_id, "0x6582") ==0 )
		{
			MND_MSG("we get MT6582\n");
			if(write(fd, "0x6582", 10) == -1)		
				MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
			
			goto exit_chip_detector;
		}
		if (strcmp(chip_id, "0x6571") ==0 )
		{
			MND_MSG("we get MT6571\n");
			if(write(fd, "0x6571", 10) == -1)		
				MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
			
			goto exit_chip_detector;
		}
		if (strcmp(chip_id, "0x6592") ==0 )
		{
			MND_MSG("we get MT6592\n");
			if(write(fd, "0x6592", 10) == -1)		
				MND_ERR("write % error, %s\n", GPS_PROPERTY, strerror(errno));			
			
			goto exit_chip_detector;
		}
	}
	else
		MND_ERR("this should never be showed\n");

exit_chip_detector:
	close(fd);
	return;
}
/*****************************************************************************/
int main (void)
{    
    int err;
    struct sockaddr addr;
    socklen_t alen = sizeof(addr);
    int lsocket, s, count;    
    char buf[C_CMD_BUF_SIZE];
    MNLD_DATA_T *obj = &mnld_data;
         
    lsocket = android_get_control_socket(SOCKET_MNL);
    if (lsocket < 0) {
        MND_ERR("fail to get socket from environment: %s\n",strerror(errno));
        exit(1);
    }
    if (listen(lsocket, 5)) {
        MND_ERR("listen on socket failed: %s\n", strerror(errno));
        exit(1);
    }
    if (setup_signal_handler()) {
        MND_ERR("setup_signal_handler: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    if (mnl_init()) {
        MND_ERR("mnl_init: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }    
    MND_MSG("listening..\n");
	chip_detector();
    while(1) {
        int eof, s = accept(lsocket, &addr, &alen);
        if (s < 0) {
            MND_ERR("Accept failed!! :%s\n", strerror(errno));
            continue;
        }
        obj->cur_accept_socket = s;
        fcntl(s, F_SETFD, FD_CLOEXEC);

        if (epoll_init()) {
            MND_ERR("epoll_init: %d (%s)\n", errno, strerror(errno));
            exit(1);
        }
        MND_MSG("new connection\n");
        for (;;) {
            err = process();
            if (err == ERR_REMOTE_HANGUP) {
                MND_ERR("remote hangup (cleanup?), wait for new connection\n");
                break;
            } else if (err == ERR_MNL_DIED) {
                MND_ERR("mnl process died, kill it\n");
                kill_mnl_process();
            } else if (errno == EINTR) {
                continue;
            } else if (err) {
                MND_ERR("process data error: %d (%s)\n", errno, strerror(errno));
            }
        }
        MND_MSG("closing connection\n");        
        close(s);
        epoll_destroy();
    }    
    MND_MSG("exit mnld \n!!");
    return 0;
}
