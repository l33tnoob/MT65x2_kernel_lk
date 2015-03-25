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

#define _MNL_PROCESS_6620_C_
#include "mnl_linux_6620.h"
#include "mtk_gps_driver_wrapper.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sched.h>
#include <linux/rtpm_prio.h>
//#include <linux/mtgpio.h>
#include <pthread.h>
#include "CFG_GPS_File.h"

//for sync supl state
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif 
//#define GPS_MNL_SUPL_NAME "gps.supl.status"
#include <linux/mtk_agps_common.h>

/*check chip version */
#define MT6620_GPS_CHIP_VERSION 0xFFFF6620 
#include <linux/mtcombo.h>

/*for FM TX compensation*/
#include <linux/fm.h>
#include "mtk_gps_sys_fp.h"
#include "SUPL_encryption.h"

/*for retry mnl callback*/
#define GPS_RETRY_NUM 10
#define GET_VER
/******************************************************************************
* Configuration
******************************************************************************/
#define MT6620_BETA
#define PMTK_SERVER_BACKLOG   5
/******************************************************************************
* Structure & enumeration
******************************************************************************/
typedef enum
{
    MNL_THREAD_UNKNOWN      = -1,
    MNL_THREAD_AGPSDISPATCH = 0,    
    MNL_THREAD_NUM,
    MNL_THREAD_LAST         = 0x7FFFFFFF
} MNL_THREAD_ID_T;
/*---------------------------------------------------------------------------*/
typedef struct _MNL_THREAD_T
{
    int                 snd_fd;
    MNL_THREAD_ID_T     thread_id;
    pthread_t           thread_handle;
    int (*thread_exit)(struct _MNL_THREAD_T *arg);
    int (*thread_active)(struct _MNL_THREAD_T *arg);
} MNL_THREAD_T;
/*---------------------------------------------------------------------------*/
typedef struct /*internal data structure for single process*/
{
    int dae_rcv_fd; /*receive message from daemon*/
    int dae_snd_fd; /*send message to daemon*/
    int sig_rcv_fd; /*receive message from queue containing internal signal*/
    int sig_snd_fd; /*send message to queue containing internal signal*/
    int (*notify_alive)(int snd_fd);
} MNL_SINPROC_T;

MNL_EPO_TIME_T mnl_epo_time = {
    .uSecond_start = 0,
    .uSecond_expire = 0,	
};
typedef struct
{
    double latitude;
    double longitude;
    float accuracy;   
}NetworkLocation;
/******************************************************************************
* Global variable
******************************************************************************/
static UINT32 delete_aiding_data;
/******************************************************************************
* static variable
******************************************************************************/
// Thread of Agent Dispatch task
static volatile int g_ThreadExitAgpsDispatch = 0;
static int condition = 0;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

/*****************************************************************************/
FILE *dbglog_fp = NULL;
MTK_GPS_BOOL enable_dbg_log = MTK_GPS_TRUE; /* MTK_GPS_FALSE;*/

struct sec_timeval
{
  unsigned int tv_sec;
  unsigned int tv_msec;
};

/*Monitor time++*/
struct sec_timeval g_gps_time_diff;
unsigned int        g_gps_time_diff_ms = 0;
struct sec_timeval g_gps_time_prev;
struct sec_timeval g_gps_time_now;

struct sec_timeval g_sys_time_diff;
unsigned int        g_sys_time_diff_ms = 0;
struct sec_timeval g_sys_time_prev;
struct sec_timeval g_sys_time_now;
int g_is_1Hz = 0;

/*Agps dispatcher state mode*/
typedef enum {
    ST_SI,
    ST_NI,
    ST_IDLE,
    ST_UNKNOWN,
}MTK_AGPS_DISPATCH_STATE;

MTK_AGPS_DISPATCH_STATE state_mode = ST_IDLE;
MTK_AGPS_DISPATCH_STATE last_state_mode = ST_UNKNOWN;

int AGENT_SET_ALARM = 0;
/*****************************************************************************/


MNL_CONFIG_T mnl_config =
{
    .init_speed = 38400,
    .link_speed = 921600,
    .debug_nmea = 1,
    .debug_mnl  = MNL_NEMA_DEBUG_SENTENCE, /*MNL_NMEA_DEBUG_NORMAL,*/
    .pmtk_conn  = PMTK_CONNECTION_SOCKET,
    .socket_port = 7000,
    .dev_dbg = DBG_DEV,
    .dev_dsp = DSP_DEV,
    .dev_gps = GPS_DEV,
    .bee_path = BEE_PATH,
    .epo_file = EPO_FILE,
    .epo_update_file = EPO_UPDATE_HAL,
    .delay_reset_dsp = 500,
    .nmea2file = 0,
    .dbg2file = 0,
    .nmea2socket = 1,
    .dbg2socket = 0,
    .timeout_init = 0,
    .timeout_monitor = 0,
    .timeout_wakeup = 0,
    .timeout_sleep = 0,
    .timeout_pwroff = 0,
    .timeout_ttff = 0,
    .EPO_enabled = 1,
    .BEE_enabled = 1,
    .SUPL_enabled = 1,
    .SUPLSI_enabled = 1,
    .EPO_priority = 64,
    .BEE_priority = 32,
    .SUPL_priority = 96,
    .AVAILIABLE_AGE = 2,
    .RTC_DRIFT = 30,
    .TIME_INTERVAL = 10,
	.u1AgpsMachine = 0	//default use spirent "0"
    .ACCURACY_SNR = 1

};

ap_nvram_gps_config_struct stGPSReadback;
int gps_nvram_valid = 0;

/*****************************************************************************/
int exit_thread_normal(MNL_THREAD_T *arg);
int thread_active_notify(MNL_THREAD_T *arg);
extern int mtk_gps_sys_init();
extern int mtk_gps_sys_uninit();

/*---------------------------------------------------------------------------*/
static MNL_THREAD_T mnl_thread[MNL_THREAD_NUM] = {
    {C_INVALID_FD, MNL_THREAD_AGPSDISPATCH,  C_INVALID_TID, exit_thread_normal, thread_active_notify},
};
/*****************************************************************************/
int send_active_notify(int snd_fd);
/*---------------------------------------------------------------------------*/
static MNL_SINPROC_T mnl_sinproc =
{
    .dae_rcv_fd = C_INVALID_FD,
    .dae_snd_fd = C_INVALID_FD,
    .sig_rcv_fd = C_INVALID_FD,
    .sig_snd_fd = C_INVALID_FD,
    .notify_alive = send_active_notify,
};
#define MNL_SO

void mtk_null(UINT16 a)
{
    return;
}
int SUPL_encrypt_wrapper(unsigned char *plain, 
    unsigned char *cipher, unsigned int length)
{
    return SUPL_encrypt(plain, cipher, length);
}

int SUPL_decrypt_wrapper(unsigned char *plain, 
    unsigned char *cipher, unsigned int length)
{
    return SUPL_decrypt(plain, cipher, length);
}

#ifdef MNL_SO
MTK_GPS_SYS_FUNCTION_PTR_T porting_layer_callback = 
{
	.sys_gps_mnl_callback = mtk_gps_sys_gps_mnl_callback,
	.sys_nmea_output_to_app = mtk_gps_sys_nmea_output_to_app,
	.sys_frame_sync_enable_sleep_mode = mtk_gps_sys_frame_sync_enable_sleep_mode,
	.sys_frame_sync_meas_req_by_network = mtk_gps_sys_frame_sync_meas_req_by_network,
	.sys_frame_sync_meas_req = mtk_gps_sys_frame_sync_meas_req,
	.sys_agps_disaptcher_callback = mtk_gps_sys_agps_disaptcher_callback,
	.sys_pmtk_cmd_cb = mtk_null,
	.encrypt = SUPL_encrypt_wrapper,
	.decrypt = SUPL_decrypt_wrapper,
};
#endif
/*****************************************************************************/
 //00, SUPL_enabled is false & SUPLSI_enabled is false
 //01, SUPL_enabled is false & SUPLSI_enabled is ture
 //10, SUPL_enabled is true & SUPLSI_enabled is false
 //11, SUPL_enabled is true & SUPLSI_enabled is true
 //default: 00
#if 0
void  
update_supl_prop()
{
    int count = 2;    
    char status[10] = {0};
    int ret = MTK_GPS_ERROR;
    MTK_AGPS_USER_PROFILE userprofile;

    while(count-- > 0)	
    {
		MNL_ERR("====To get property sstatus \n");	        
		if (property_get(GPS_MNL_SUPL_NAME, status, NULL)) 
		{        	
			MNL_ERR("====After get property, status is %s\n", status);            
			if (strcmp(status, "00") == 0) 
			{
			    mnl_config.SUPL_enabled = 0;
			    mnl_config.SUPLSI_enabled = 0;
			    ret = MTK_GPS_SUCCESS;
			    MNL_ERR("====status is %s\n", status);	            
			    break;
			}
			else if ( strcmp(status, "01") == 0)
			{
			    mnl_config.SUPL_enabled = 0;
			    mnl_config.SUPLSI_enabled = 1;
		           ret = MTK_GPS_SUCCESS;
			    MNL_ERR("====status is %s\n", status);	
			    break;
			}
			else if ( strcmp(status, "10") == 0)
			{
			    mnl_config.SUPL_enabled = 1;
			    mnl_config.SUPLSI_enabled = 0;
			    ret = MTK_GPS_SUCCESS;
			    MNL_ERR("====status is %s\n", status);
			    break;
			}
			else if (strcmp(status, "11") == 0)
			{
			    mnl_config.SUPL_enabled = 1;
			    mnl_config.SUPLSI_enabled = 1;
			    ret = MTK_GPS_SUCCESS;
			    MNL_ERR("====status is %s\n", status);			    
			    break;
			}
		}
		//usleep(100000);
    	}
	
    if (ret == MTK_GPS_ERROR)
    {
    MNL_ERR("====get failed, status is %s\n", status);			    
    }

    userprofile.EPO_enabled = mnl_config.EPO_enabled;
    userprofile.BEE_enabled = mnl_config.BEE_enabled;    
    userprofile.SUPL_enabled = mnl_config.SUPL_enabled;
    userprofile.SUPLSI_enabled = mnl_config.SUPLSI_enabled;
    userprofile.EPO_priority = mnl_config.EPO_priority;
    userprofile.BEE_priority = mnl_config.BEE_priority;
    userprofile.SUPL_priority = mnl_config.SUPL_priority;
    mtk_agps_set_param(MTK_MSG_AGPS_MSG_PROFILE, &userprofile, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);
}
#endif
/******************************************************************************
*   MNL Implementation
******************************************************************************/


/*****************************************************************************/
/*[SeanNote]Monitor time of two GPS UTC Time++*/
/* I treat it as x:bigger,y:smaller */
/*****************************************************************************/
// callback function from MNL main thread

static time_t last_send_time = 0;
static time_t current_time = 0;
static int callback_flags = 0;
static int tcxo_config_everset = 0;
	
INT32
mtk_gps_sys_gps_mnl_callback (MTK_GPS_NOTIFICATION_TYPE msg)
{
    switch (msg)
    {
        case MTK_GPS_MSG_FIX_READY:
        {
            if (tcxo_config_everset == 0){
                mtk_gps_set_tcxo_mode(TCXO_FLAG);                
                MNL_MSG("Default is bad TCXO mode !!!!");
                if (AIC_FLAG == MTK_AIC_ON){
                    mtk_gps_set_AIC_mode(AIC_FLAG);
                    MNL_MSG("AIC mode on!");
                }
                tcxo_config_everset = 1;
            }        
            //MNL_MSG("MTK_GPS_MSG_FIX_READY\n");
            double dfRtcD = 0.0, dfAge = 0.0;     
#if 0   
            UINT32 SVListBitMap = 0;
            static UINT32 EphBitMap = 0;
            UINT32 idex = 0;
            MTK_GPS_SV_INFO SV_INFO_DATA;

            memset(&SV_INFO_DATA, 0x00, sizeof(SV_INFO_DATA));
            if (!mtk_gps_get_sv_list(&SVListBitMap))
            {
                MNL_MSG("SVListBitMap: %X\n", SVListBitMap);
            }
            else
            {
                MNL_ERR("Get SV list fail\n");
            }
               
            if (!mtk_gps_get_sv_info(&SV_INFO_DATA))
            {
                MNL_ERR("Get SV info success\n");
            }
            else
            {
                MNL_ERR("Get SV info fail\n");
            }              
            
            for (idex = 0; idex < 32; idex++)
            {
                if(0 != EphBitMap){
                    MNL_ERR("Need re-adiding\n");
                    break;
                }
            
             //   MNL_ERR("EPH[%d]=%d\n", idex, SV_INFO_DATA.eph[idex]);
                if(((SVListBitMap & (0x00000001 << idex)) != 0) && (SV_INFO_DATA.eph[idex] == 0)){
                    MNL_ERR("SVID %d no EPH\n", idex+1);
                    EphBitMap|= (0x00000001 << idex);
                }
                
            }

            if (EphBitMap != 0){
                MNL_MSG("Set assist bitmap\n");
                mtk_gps_set_assist_bitmap(0x08);
                //Re-aiding request
                MNL_MSG("Re-aiding request\n");
		if (ST_IDLE == state_mode){
                    MNL_MSG("#AGNT# idle mode, send re-aiding req\n");
                    mtk_gps_sys_agps_disaptcher_callback (MTK_AGPS_CB_ASSIST_REQ, 0, NULL);
                }else{
                    //Re-aiding: only IDLE mode can send ASSIST req
                    MNL_MSG("#AGNT# SI or NI mode, ignore re-aiding req\n");
                }
                EphBitMap = 0;
                SVListBitMap = 0;
                memset(&SV_INFO_DATA, 0x00, sizeof(SV_INFO_DATA));
            }     
#endif     
            			
            callback_flags = 1;
            if (mnl_sinproc.notify_alive)
            {
                mnl_sinproc.notify_alive(mnl_sinproc.dae_snd_fd);
            }
			
            if(mtk_gps_get_rtc_info(&dfRtcD, &dfAge) == MTK_GPS_SUCCESS)
            {
                    MNL_MSG("MTK_GPS_MSG_FIX_READY, GET_RTC_OK, %.3lf, %.3lf\n", dfRtcD, dfAge);
		      MNL_MSG("Age = %d, RTCDiff = %d, Time_interval = %d\n", mnl_config.AVAILIABLE_AGE, 
			  	                                     mnl_config.RTC_DRIFT, mnl_config.TIME_INTERVAL);			
			if((dfAge <= mnl_config.AVAILIABLE_AGE) && (dfRtcD >= mnl_config.RTC_DRIFT ||
                                               dfRtcD <= -mnl_config.RTC_DRIFT) && dfRtcD < 5000) 
			{
			    int fd_fmsta = -1;
			    unsigned char buf[2];
			    int status = -1;	
			    fd_fmsta = open("/proc/fm", O_RDWR);
			    if(fd_fmsta < 0){
                             MNL_MSG("open /proc/fm error\n");				 
			    }else{
                                MNL_MSG("open /proc/fm success!\n");  
			           status = read(fd_fmsta, &buf, sizeof(buf));
                                if(status < 0){ 
                                MNL_MSG("read fm status fails = %s\n", strerror(errno));                               
                                } 
                                if(close(fd_fmsta) == -1){
                                 MNL_MSG("close fails = %s\n", strerror(errno));
                                }                    
                            }
			 				
			    if(buf[0] == '2')                                                 
			    {
			        INT32 time_diff;
				time(&current_time);
				time_diff = current_time - last_send_time;         
				 
			        if((0 == last_send_time) || (time_diff > mnl_config.TIME_INTERVAL))
			        {
                                  int fd_fmdev = -1;
                                  int ret = 0;
                                  struct fm_gps_rtc_info rtcInfo;

                                  fd_fmdev = open("dev/fm", O_RDWR);
                                  if(fd_fmdev < 0){
                                      MNL_MSG("open fm dev error\n");                                      
                                  } else {                                				
			              rtcInfo.retryCnt = 2;
                                      rtcInfo.ageThd = mnl_config.AVAILIABLE_AGE;
                                      rtcInfo.driftThd = mnl_config.RTC_DRIFT;
                                      rtcInfo.tvThd.tv_sec = mnl_config.TIME_INTERVAL;
                                      rtcInfo.age = dfAge;
                                      rtcInfo.drift = dfRtcD;
                                      rtcInfo.tv.tv_sec = current_time;

				      ret = ioctl(fd_fmdev, FM_IOCTL_GPS_RTC_DRIFT, &rtcInfo);

				      if(ret){
                                MNL_MSG("send rtc info failed, [ret=%d]\n", ret);                                          
                                      }
				      ret = close(fd_fmdev);
				      if(ret){
                                          MNL_MSG("close fm dev error\n");                                         
                                       }
			         }   
		            }
			}
	          }
            }
            else
            {
               MNL_MSG("MTK_GPS_MSG_FIX_READY,GET_RTC_FAIL\n");                        
            }
			
        }
        break;
        case MTK_GPS_MSG_FIX_PROHIBITED:
        {
        	  if (mnl_sinproc.notify_alive)
        	  {
        	  		mnl_sinproc.notify_alive(mnl_sinproc.dae_snd_fd);
        	  }
            MNL_MSG("MTK_GPS_MSG_FIX_PROHIBITED\n");
        }
        break;
        default:
        break;
    }

    return  MTK_GPS_SUCCESS;
}

/*****************************************************************************/

/*****************************************************************************/
void signal_handler(int status)
{
}
/*****************************************************************************/

/*****************************************************************************/
static const char hexTABLE[] =
{
   0x30, 0x31, 0x32, 0x33,  // Characters '0' - '3'
   0x34, 0x35, 0x36, 0x37,  // Characters '4' - '7'
   0x38, 0x39, 0x41, 0x42,  // Characters '8' - 'B'
   0x43, 0x44, 0x45, 0x46   // Characters 'C' - 'F'
};

void alarm_handler(){
    MNL_MSG("Send MTK_AGPS_SUPL_END to MNL");
    mtk_agps_set_param(MTK_MSG_AGPS_MSG_SUPL_TERMINATE, NULL, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);            

    MNL_MSG("++Receive MTK_AGPS_SUPL_END , last_state_mode = %d, state_mode = %d", last_state_mode, state_mode);
    last_state_mode = state_mode;
    state_mode = ST_IDLE;
    MNL_MSG("--Receive MTK_AGPS_SUPL_END , last_state_mode = %d, state_mode = %d", last_state_mode, state_mode);
    
    AGENT_SET_ALARM = 0;
    #if AGENT_LOG_ENABLE
        MNL_MSG("@#$^ [AGNT] RecvMsg (%d %d)\r\n", type, payload_len);
    #endif
}


/*****************************************************************************/
static void * thread_agpsdispatch_func(void * arg)
{
    MNL_MSG("thread_agpsdispatch_func, state mode = %d", state_mode);
    int agpsdispatch_sock = -1, left = 0;
    struct sockaddr_un local;
    struct sockaddr_un remote;
    socklen_t remotelen;
    UINT8 BufIn[MTK_AGPS_PMTK_MAX_SIZE];
    UINT16 type, payload_len;
    MTK_GPS_AGPS_AGENT_MSG *pMsg;    

    if ((agpsdispatch_sock = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        MNL_ERR("thread_agpsdispatch_func: socket open failed\n");
        return NULL;       
    }

    unlink(MTK_PROFILE2MNL);
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_LOCAL;
    strcpy(local.sun_path, MTK_PROFILE2MNL);

    if (bind(agpsdispatch_sock, (struct sockaddr *)&local, sizeof(local)) < 0 )
    {           
        MNL_ERR("thread_agpsdispatch_func: socket bind failed\n");
        close(agpsdispatch_sock);
        agpsdispatch_sock = -1;       
        return NULL;
    }
	
    int res = chmod(MTK_PROFILE2MNL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP);
    MNL_MSG("chmod res = %d, %s", res, strerror(errno));
    
	//set condtion to report the GPS driver status to SUPL
	pthread_mutex_lock(&mutex);
       condition++;
	if(condition == 2)
       {
           pthread_cond_signal(&cond);
       }
	pthread_mutex_unlock(&mutex);      

    while(!g_ThreadExitAgpsDispatch)
    {
        remotelen = sizeof(remote);
        left = recvfrom(agpsdispatch_sock, BufIn, sizeof(BufIn), 0, (struct sockaddr *)&remotelen, &remotelen);
        if (left < 0)
        {
            if (errno != EINTR && errno != EAGAIN)
                MNL_ERR("thread_agpsdispatch_func: recvfrom error\r\n");
            break;
        }

        type = (BufIn[3]<<8 |BufIn[2]);
        payload_len = (UINT16)(BufIn[5]<<8 |BufIn[4]);
        
        if(MTK_AGPS_SUPL_NI_REQ  == type)
        {
            /* update user profile config*/
            //update_supl_prop();
            //check & cancel timer
            if (1 == AGENT_SET_ALARM){
                MNL_MSG("Cancel timer and send NI to MNL");
                alarm(0);               
                AGENT_SET_ALARM = 0;
            }                        
            state_mode = ST_NI;   
            mtk_agps_set_param(MTK_MSG_AGPS_MSG_RESET_SM, NULL, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);            
            mtk_agps_set_param(MTK_MSG_AGPS_MSG_REQ_NI, NULL, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);

            #if AGENT_LOG_ENABLE
            MNL_MSG("@#$^ [AGNT] RecvMsg (%d %d)\r\n", type, payload_len);
            #endif            
            }
            else if(MTK_AGPS_SUPL_PMTK_DATA == type)
            {
            if((ST_IDLE == state_mode) && (ST_NI == last_state_mode)){
                #if AGENT_LOG_ENABLE
                    MNL_MSG("@#$^ [AGNT] Recv MTK_AGPS_SUPL_PMTK_DATA in ST_IDLE\r\n");
                #endif
                state_mode = ST_NI;   
                mtk_agps_set_param(MTK_MSG_AGPS_MSG_RESET_SM, NULL, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);            
                mtk_agps_set_param(MTK_MSG_AGPS_MSG_REQ_NI, NULL, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);
            }
            MTK_GPS_PARAM_PMTK_CMD pmtk_cmd;
            memset(&pmtk_cmd.pmtk[0], 0, MTK_PMTK_CMD_MAX_SIZE);
            memcpy(&pmtk_cmd.pmtk[0], (char*)&BufIn[6], payload_len);
            mtk_agps_set_param(MTK_MSG_AGPS_MSG_SUPL_PMTK, &pmtk_cmd.pmtk[0], MTK_MOD_DISPATCHER, MTK_MOD_AGENT);

                #if AGENT_LOG_ENABLE
            MNL_MSG("@#$^ [AGNT] RecvMsg (%d %d %s)\r\n", type, payload_len, pmtk_cmd.pmtk);
                #endif                         
                }
        else if(MTK_AGPS_SUPL_END == type)
        {
            /* ZQH: Send MTK_AGPS_SUPL_END to MNL after 30s to avoid the case: 
               when AGPSD failed to send aiding data to MNL, it(MNL) will send SI req per second */
            
            //setup 30s timer
            if (0 == AGENT_SET_ALARM){
                MNL_MSG("Set up signal & alarm!");
                signal(SIGALRM, alarm_handler);                
                alarm(30);
                AGENT_SET_ALARM = 1;                
            }else{
                MNL_MSG("MNL_AT_SET_ALARM == 1");
            }          
        }
        else 
        {
            MNL_MSG("thread_agpsdispatch_func: msg type invalid\r\n");                
        }
    }
    MNL_MSG("thread agps dispatch return\n");
    close(agpsdispatch_sock);
    agpsdispatch_sock = -1;  
    pthread_exit(NULL);
    return NULL;
}


/*****************************************************************************/
INT32
mtk_gps_sys_agps_disaptcher_callback (UINT16 type, UINT16 length, char *data)
{ 
    INT32 ret = MTK_GPS_SUCCESS;
    MNL_MSG("mtk_gps_sys_agps_disaptcher_callback, state mode = %d", state_mode);
    
    if (type == MTK_AGPS_CB_SUPL_PMTK || type == MTK_AGPS_CB_ASSIST_REQ)
    {  
        if (mnl_config.SUPL_enabled)
        {
            int sock2supl = -1;
            struct sockaddr_un local;
            mtk_agps_msg *pMsg = NULL;
            UINT16 total_length = length + sizeof(mtk_agps_msg);
            
            pMsg = (mtk_agps_msg *)malloc(total_length);
            if(pMsg)
            {
           
                if(type == MTK_AGPS_CB_SUPL_PMTK)
                {
                    pMsg->type = MTK_AGPS_SUPL_PMTK_DATA;
                }
                else if(type == MTK_AGPS_CB_ASSIST_REQ)
                {
                    if (state_mode == ST_IDLE || state_mode == ST_SI)
                    {
                        MNL_MSG("Dispatcher in IDLE or SI mode\n");
                        state_mode = ST_SI;
                        pMsg->type = MTK_AGPS_SUPL_ASSIST_REQ;
                    }
                    else
                    {
                        MNL_ERR("Dispatcher in %d mode, ignore current request\n", state_mode);
                        if(pMsg){
                            free(pMsg); 
                            pMsg = NULL;
                        }
                        MNL_ERR("free pMsg\n");
                        return MTK_GPS_ERROR; 
                    }
					
                }
                pMsg->srcMod = MTK_MOD_GPS;
                pMsg->dstMod = MTK_MOD_SUPL; 
                pMsg->length = length;
                if (pMsg->length != 0)
                {
                    memcpy(pMsg->data, data, length);
                #if AGENT_LOG_ENABLE
                    MNL_MSG("@#$^ [AGNT] SendMsg (%d %d %d %d %s)\r\n", 
                                        pMsg->srcMod, pMsg->dstMod, pMsg->type, pMsg->length, pMsg->data);                              
                #endif
                }
                
                else {     
                #if AGENT_LOG_ENABLE
                    MNL_MSG("@#$^ [AGNT] SendMsg (%d %d %d %d) no data\r\n", 
                                        pMsg->srcMod, pMsg->dstMod, pMsg->type, pMsg->length);
                #endif
                }

                if((sock2supl = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
                {
                    MNL_ERR("@#$^ [AGNT] SendMsg:open sock2supl fails\r\n");
                    free(pMsg); 
                    pMsg = NULL; 
                    return MTK_GPS_ERROR;
                }

                memset(&local, 0, sizeof(local));
                local.sun_family = AF_LOCAL;
                strcpy(local.sun_path, MTK_MNL2SUPL);

                if (sendto(sock2supl, (void *)pMsg, total_length, 0, (struct sockaddr*)&local, sizeof(local)) < 0)
                {
                    MNL_ERR("send message fail:%s\r\n", strerror(errno));
                    ret = MTK_GPS_ERROR;
                }
                close(sock2supl);
                if(pMsg)
                {
                    free(pMsg); 
                    pMsg = NULL;
                }
            }
        }
        else {
            MNL_MSG("mtk_sys_agps_disaptcher_callback: SUPL disable\r\n");    
            ret = MTK_GPS_ERROR;
        } 
    }    
    return ret;
}

/*****************************************************************************/
int
linux_gps_init (void)
{
    INT32 status = MTK_GPS_SUCCESS;
    static MTK_GPS_INIT_CFG init_cfg;
    static MTK_GPS_DRIVER_CFG driver_cfg;
    UINT8 clk_type = 0xff;  // for new 43EVK board
    FILE *parm_fp;
    MTK_GPS_BOOT_STATUS mnl_status = 0;
    ENUM_WMTHWVER_TYPE_T hw_ver = WMTHWVER_INVALID;

    memset(&init_cfg, 0, sizeof(MTK_GPS_INIT_CFG));
    MTK_AGPS_USER_PROFILE userprofile;
    // ====== default config ======
    init_cfg.if_type = MTK_IF_UART_NO_HW_FLOW_CTRL;
    init_cfg.pps_mode = MTK_PPS_DISABLE;        // PPS disabled
    init_cfg.pps_duty = 100;                    // pps_duty (100ms high)
    init_cfg.if_link_spd = 115200;              // 115200bps

    if(gps_nvram_valid == 1)
    {
        init_cfg.hw_Clock_Freq = stGPSReadback.gps_tcxo_hz;            // 26MHz TCXO,                         needs to modify nvram default value
	init_cfg.hw_Clock_Drift = stGPSReadback.gps_tcxo_ppb;                 // 0.5ppm TCXO
        init_cfg.Int_LNA_Config = stGPSReadback.gps_lna_mode;					// 0 -> Mixer in , 1 -> Internal LNA
	init_cfg.u1ClockType = stGPSReadback.gps_tcxo_type; //clk_type;          needs to modify nvram default value
	
    }
    else
    {
    	init_cfg.hw_Clock_Freq = 26000000;             // 26MHz TCXO
    	init_cfg.hw_Clock_Drift = 2000;                 // 0.5ppm TCXO
    	init_cfg.Int_LNA_Config = 0;                    // 0 -> Mixer in , 1 -> Internal LNA
     	init_cfg.u1ClockType = 0xFF; //clk_type;
  
    }
	if(init_cfg.hw_Clock_Drift == 0)
	{
		MNL_MSG("customer didn't set clock drift value, use default value\n");	
		init_cfg.hw_Clock_Drift = 2000;	
	}

    /*setting 1Hz/5Hz */
    if(g_is_1Hz)
    {
        init_cfg.fix_interval = 1000;               // 1Hz update rate
    }
    else 
    {
        init_cfg.fix_interval = 200;               // 5Hz update rate
    }


    init_cfg.datum = MTK_DATUM_WGS84;           // datum
    init_cfg.dgps_mode = MTK_AGPS_MODE_AUTO;    // enable SBAS

 	
    dsp_fd = open(mnl_config.dev_dsp, O_RDWR);
    if ( dsp_fd == -1)    
    {        
        MNL_MSG("open_port: Unable to open - %s \n", mnl_config.dev_dsp); 	   
        return MTK_GPS_ERROR;    
    } else {
        MNL_MSG("open dsp successfully\n");
    }
    
 
    init_cfg.reservedy = (void *)MTK_GPS_CHIP_KEY_MT6630;
    init_cfg.reservedx = MT6630_E1;
    if (mnl_config.ACCURACY_SNR == 1){
        init_cfg.reservedx |=(UINT32)0x80000000;
    }else if (mnl_config.ACCURACY_SNR == 2){
        init_cfg.reservedx |=(UINT32)0x40000000;
    }else if (mnl_config.ACCURACY_SNR == 3){
        init_cfg.reservedx |=(UINT32)0xC0000000;;
    }
    MNL_MSG("ACCURACY_SNR = %d\n", mnl_config.ACCURACY_SNR);
    MNL_MSG("Get chip version type (%p) \n", init_cfg.reservedy);
    MNL_MSG("Get chip version value (%d) \n", init_cfg.reservedx);		
    //close(dsp_fd);	

    strcpy(driver_cfg.nv_file_name, NV_FILE);
    strcpy(driver_cfg.dbg_file_name, LOG_FILE);
    strcpy(driver_cfg.nmeain_port_name, mnl_config.dev_dbg);
    strcpy(driver_cfg.nmea_port_name, mnl_config.dev_gps);
    strcpy(driver_cfg.dsp_port_name, mnl_config.dev_dsp);
    strcpy((char *)driver_cfg.bee_path_name, mnl_config.bee_path);
    driver_cfg.reserved   =   mnl_config.BEE_enabled;
    driver_cfg.DebugType    =   0;
    driver_cfg.u1AgpsMachine = mnl_config.u1AgpsMachine;
    strcpy(driver_cfg.epo_file_name, mnl_config.epo_file);
    strcpy(driver_cfg.epo_update_file_name,mnl_config.epo_update_file);
		
    driver_cfg.DebugType = mnl_config.dbg2file;

	driver_cfg.u1AgpsMachine = mnl_config.u1AgpsMachine;
	if(driver_cfg.u1AgpsMachine == 1)
		MNL_MSG("we use CRTU to test\n");
	else
		MNL_MSG("we use Spirent to test\n");

    status = mtk_gps_delete_nv_data(delete_aiding_data);
    MNL_MSG("u4Bitmap, %d\n", status);
#ifdef MNL_SO

	MTK_GPS_SYS_FUNCTION_PTR_T*  mBEE_SYS_FP = &porting_layer_callback;
    if(mtk_gps_sys_function_register(mBEE_SYS_FP) != MTK_GPS_SUCCESS)
    {
    	MNL_ERR("register callback for mnl error\n");
        status = MTK_GPS_ERROR;
   	    return status;  
    }
    driver_cfg.dsp_fd = dsp_fd;

#endif
    mnl_status = mtk_gps_mnl_run((const MTK_GPS_INIT_CFG*)&init_cfg , (const MTK_GPS_DRIVER_CFG*)&driver_cfg);
    MNL_MSG("Status (%d) \n",mnl_status );
    if (mnl_status != MNL_INIT_SUCCESS )		
    {
       status = MTK_GPS_ERROR;
	   return status;   
    }
    if(access(EPO_UPDATE_HAL, F_OK) == -1)
	{
        MNL_MSG("EPO file does not exist, no EPO yet\n");                
    }
	else
	{					
		MNL_MSG("there is a EPOHAL file, please mnl update EPO.DAT from EPOHAL.DAT\n");
		if (mtk_agps_agent_epo_file_update() == MTK_GPS_ERROR)
        {
            MNL_ERR("EPO file updates fail\n");
        }
    }

    MNL_MSG("dsp port (%s) \n",driver_cfg.dsp_port_name );
    MNL_MSG("nmea port (%s) \n",driver_cfg.nmea_port_name );
    MNL_MSG("nmea dbg port (%s) \n",driver_cfg.nmeain_port_name );
    MNL_MSG("dsp_dbg_file_name (%s) \n",driver_cfg.dbg_file_name );
    MNL_MSG("dbg_file_name (%s) \n",driver_cfg.dbg_file_name );
    MNL_MSG("nv_file_name (%s) \n",driver_cfg.nv_file_name );
    //MNL_MSG("socket_port (%d) \n", driver_cfg.socket_port);

#ifdef GPS_AGENT    
	if (pthread_create(&mnl_thread[MNL_THREAD_AGPSDISPATCH].thread_handle, 
                       NULL, thread_agpsdispatch_func, 
                       (void*)&mnl_thread[MNL_THREAD_AGPSDISPATCH])) 
	{
	   MNL_MSG("error creating dispatch thread for agps \n");
	   return MTK_GPS_ERROR;
	}
#endif
    //usleep(100000); // delay 100ms for init agent message queue
    //update_supl_prop();
    userprofile.EPO_enabled = mnl_config.EPO_enabled;
    userprofile.BEE_enabled = mnl_config.BEE_enabled;    
    userprofile.SUPL_enabled = mnl_config.SUPL_enabled;
    //userprofile.SUPLSI_enabled = mnl_config.SUPLSI_enabled;
    userprofile.EPO_priority = mnl_config.EPO_priority;
    userprofile.BEE_priority = mnl_config.BEE_priority;
    userprofile.SUPL_priority = mnl_config.SUPL_priority;
    mtk_agps_set_param(MTK_MSG_AGPS_MSG_PROFILE, &userprofile, MTK_MOD_DISPATCHER, MTK_MOD_AGENT);

    //set condtion to report the GPS driver status to SUPL 
    pthread_mutex_lock(&mutex);
    condition++;
    if(condition == 2)
    {
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    return  status;
}
/*****************************************************************************/
int
linux_gps_uninit (void)
{
    int idx ,err;
    for (idx = 0; idx < MNL_THREAD_NUM; idx++) {
        if (mnl_thread[idx].thread_handle == C_INVALID_TID)
            continue;
        if (!mnl_thread[idx].thread_exit)
            continue;

        if ((err = mnl_thread[idx].thread_exit(&mnl_thread[idx]))) {
            MNL_ERR("fails to thread_exit thread %d], err = %d\n", idx, err);
            return MTK_GPS_ERROR;
        }
    }
    MNL_MSG("close dsp_fd \n");
    close(dsp_fd);
    return MTK_GPS_SUCCESS;
}
/******************************************************************************
*   MNL Porting For Android Platform
******************************************************************************/
void linux_gps_load_property()
{
    enable_dbg_log = 0;
    nmea_debug_level = 0;

    if (!mnl_utl_load_property(&mnl_config)) {
        enable_dbg_log = mnl_config.debug_nmea;
        nmea_debug_level = mnl_config.debug_mnl;
    }
    
    //if file reads fail, to get property set value
    enable_dbg_log = mnl_config.debug_nmea;
    MNL_ERR("enable_dbg_log: %d", enable_dbg_log);
}
/*****************************************************************************/
int exit_thread_normal(MNL_THREAD_T *arg)
{   /* exit thread by pthread_kill -> pthread_join*/
    int err;
    if (!arg)
    {
        return MTK_GPS_ERROR;
    }
        
    if(arg->thread_id == MNL_THREAD_AGPSDISPATCH)
    {
        int sock2diapatch = -1;
        struct sockaddr_un local;
        mtk_agps_msg *pDummy_agps_msg = (mtk_agps_msg *)malloc(sizeof(mtk_agps_msg));
        MNL_MSG("agps dispatch thread return trigger\n");
        
        g_ThreadExitAgpsDispatch = 1;      
        if((sock2diapatch = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        {
            MNL_ERR("exit_thread_normal: open sock2supl fails\r\n");
            free(pDummy_agps_msg); 
            pDummy_agps_msg = NULL; 
            goto EXIT;
        }

        memset(&local, 0, sizeof(local));
        local.sun_family = AF_LOCAL;
        strcpy(local.sun_path, MTK_PROFILE2MNL);

        if (sendto(sock2diapatch, pDummy_agps_msg, sizeof(mtk_agps_msg), 0, (struct sockaddr*)&local, sizeof(local)) < 0)
        {
            MNL_ERR("send msg to dispatch fail:%s\r\n", strerror(errno));
        }
        close(sock2diapatch);
        if(pDummy_agps_msg){
            free(pDummy_agps_msg); 
            pDummy_agps_msg = NULL;
        }
    }
    

 EXIT:   
    if ((err = pthread_kill(arg->thread_handle, SIGUSR1)))
    {
        MNL_ERR("pthread_kill failed idx:%d, err:%d\n", arg->thread_id, err);
        return err;
    }  
    if ((err = pthread_join(arg->thread_handle, NULL)))
    {
        MNL_ERR("pthread_join failed idx:%d, err:%d\n", arg->thread_id, err);
        return err;
    }
    return 0;
}

/*****************************************************************************/
static int sig_send_cmd(int fd, char* cmd, int len)
{
    if (fd == C_INVALID_FD)
    {
        return 0;
    }
    else
    {
        int  ret;
        MNL_VER("sig_send_cmd (%d, 0x%x)\n", fd, (int)(*cmd));
        do {
            ret = write( fd, cmd, len );
        }while (ret < 0 && errno == EINTR);
        if (ret == len)
            return 0;
        else
        {
            MNL_ERR("sig_send_cmd fails: %d (%s)\n", errno, strerror(errno));
            return -1;
        }
    }

}
/*****************************************************************************/
void mtk_gps_sys_ttff_handler(int type)
{   /*the TTFF handler is called from PMTK handler*/
    char *msg = NULL;
    char buf[] = {MNL_CMD_RCV_TTFF};
    int err;

    if (type == MTK_GPS_START_HOT)
        msg = "HOT ";
    else if (type == MTK_GPS_START_WARM)
        msg = "WARM";
    else if (type == MTK_GPS_START_COLD)
        msg = "COLD";
    else if (type == MTK_GPS_START_FULL)
        msg = "FULL";
    else
        MNL_ERR("invalid TTFF type: %d\n", type);

    MNL_MSG("receive %s TTFF\n", msg);
    if ((err = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf))))
        MNL_MSG("send command 0x%X fails\n", (unsigned int)buf[0]);

}
/*****************************************************************************/
int thread_active_notify(MNL_THREAD_T *arg)
{
    if (!arg)
    {
        MNL_MSG("fatal error: null pointer!!\n");
        return -1;
    }
    if (arg->snd_fd != C_INVALID_FD)
    {
        char buf[] = {MNL_CMD_ACTIVE};
        return sig_send_cmd(arg->snd_fd, buf, sizeof(buf));
    }
    return 0;
}
/*****************************************************************************/

/*****************************************************************************/
int linux_gps_dev_init(void)
{
    struct termios termOptions;

    struct sched_param sched, test_sched;

#if CFG_DBG_INFO_GPIO_TOGGLE
/*[SeanNote]init gpio port ++ */
    MNL_MSG("open_port: %s", "/dev/mtgpio");
    gpio_fd = open("/dev/mtgpio", O_RDONLY);
    if (gpio_fd == -1)
    {
        MNL_ERR("open_port: Unable to open - %s", "/dev/mtgpio");
        /*the process should exit if fail to open UART device*/
        return MTK_GPS_ERROR;
    }
    else
    {
        /*testing*/
        usleep(200000);  // sleep 200 ms
        MNL_MSG("gpio test 1 ,%s", "/dev/mtgpio");
        ioctl(gpio_fd, GPIO_IOCSDATAHIGH, 20);
        usleep(200000);  // sleep 200 ms
        ioctl(gpio_fd, GPIO_IOCSDATALOW, 20);
        usleep(200000);  // sleep 200 ms
        MNL_MSG("gpio test 2 ,%s", "/dev/mtgpio");
        ioctl(gpio_fd, GPIO_IOCSDATAHIGH, 20);
        usleep(200000);  // sleep 200 ms
        ioctl(gpio_fd, GPIO_IOCSDATALOW, 20);
    }
/*[SeanNote]init gpio port -- */
#endif

#if CFG_DBG_INFO_UART_OUTPUT
    /* George: use GPS UART to output debug information */
    dbg_info_uart_fd = open(DBG_INFO_UART_DEV, O_RDWR | O_NOCTTY);
    /* Initialize UART2 to 921600bps */
    if (dbg_info_uart_fd == -1) {
        MNL_MSG("open_port: Unable to open dbg_info_uart_fd: %s", DBG_INFO_UART_DEV);
        /*the process should exit if fail to open UART device*/
        return MTK_GPS_ERROR;
    }
    else {
        fcntl(dbg_info_uart_fd, F_SETFL, 0);

        // Get the current options:
        tcgetattr(dbg_info_uart_fd, &termOptions);

        // Set 8bit data, No parity, stop 1 bit (8N1):
        termOptions.c_cflag &= ~PARENB;
        termOptions.c_cflag &= ~CSTOPB;
        termOptions.c_cflag &= ~CSIZE;
        termOptions.c_cflag |= CS8 | CLOCAL | CREAD;

        MNL_MSG("c_lflag=%x,c_iflag=%x,c_oflag=%x\n",termOptions.c_lflag,termOptions.c_iflag,
                                termOptions.c_oflag);
        //termOptions.c_lflag

        // Raw mode
        termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
        termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*raw input*/
        termOptions.c_oflag &= ~OPOST;  /*raw output*/

        tcflush(dbg_info_uart_fd,TCIFLUSH);//clear input buffer
        termOptions.c_cc[VTIME] = 100; /* inter-character timer unused */
        termOptions.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */
        /*
        * Set the new options for the port...
        */
        cfsetispeed(&termOptions, B921600);
        cfsetospeed(&termOptions, B921600);
        tcsetattr(dbg_info_uart_fd, TCSANOW, &termOptions);
    }
#endif /* end of CFG_DBG_INFO_UART_OUTPUT */
    return 0;
}
/*****************************************************************************/
int linux_gps_dev_uninit(void)
{
#if CFG_DBG_INFO_GPIO_TOGGLE
    /*[SeanNote]init gpio port ++ */
    if (gpio_fd != C_INVALID_FD)
        close(gpio_fd);
    /*[SeanNote]init gpio port -- */
#endif

#if CFG_DBG_INFO_UART_OUTPUT /* George: GPS debug info UART */
    if (dbg_info_uart_fd != C_INVALID_FD) {
        close(dbg_info_uart_fd);
        dbg_info_uart_fd = C_INVALID_FD;
    }
#endif
    return 0;
}
/*****************************************************************************/
static int epoll_register( int  epoll_fd, int  fd )
{
    struct epoll_event  ev;
    int                 ret, flags;

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
static int mnlcmd_handler(int fd) /*sent from mnld*/
{
    int ret;
    char cmd = MNL_CMD_UNKNOWN;
    int retry = 0;
    do {
        ret = read(  fd, &cmd, sizeof(cmd) );
    } while (ret < 0 && errno == EINTR);
    if (ret == 0)
    {
        MNL_MSG("mnlcmd_handler EOF"); /*it should not happen*/
        return 0;
    }
    else if (ret != sizeof(cmd))
    {
        MNL_MSG("mnlcmd_handler fails: %d %d(%s)\n", ret, errno, strerror(errno));
        return -1;
    }

    MNL_MSG("mnlcmd_handler(0x%X)\n", cmd);

    while ((0 == callback_flags) && (retry < GPS_RETRY_NUM))
    {
        usleep(200000);
	retry++;
    }
		
    if(1 != callback_flags)
        return -1;
	
    if (cmd == MNL_CMD_SLEEP)
    {
        if ((ret = mtk_gps_set_param(MTK_PARAM_CMD_SLEEP, NULL)))  
        {
            MNL_ERR("MNL sleep = %d\n", ret);
        }
        else
        {
            /*notify mnld that sleep command is successfully executed*/
            char buf[] = {MNL_CMD_SLEPT};
            ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf));
        }
        return ret;
    }
    else if (cmd == MNL_CMD_WAKEUP)
    {
        if ((ret = mtk_gps_set_param(MTK_PARAM_CMD_WAKEUP, NULL))) 
            MNL_ERR("MNL wakeup = %d\n", ret);
        return ret;
    }
    else if (cmd == MNL_CMD_RESTART_HOT)
    {
        MTK_GPS_PARAM_RESTART restart = {MTK_GPS_START_HOT};
        if ((ret = mtk_gps_set_param (MTK_PARAM_CMD_RESTART, &restart)))
            MNL_ERR("MNL hot start = %d\n", ret);
        return ret;
    }
    else if (cmd == MNL_CMD_RESTART_WARM)
    {
        MTK_GPS_PARAM_RESTART restart = {MTK_GPS_START_WARM};
        if ((ret = mtk_gps_set_param (MTK_PARAM_CMD_RESTART, &restart)))
            MNL_ERR("MNL warm start = %d\n", ret);
        return ret;
    }
    else if (cmd == MNL_CMD_RESTART_COLD)
    {
        MTK_GPS_PARAM_RESTART restart = {MTK_GPS_START_COLD};
        if ((ret = mtk_gps_set_param (MTK_PARAM_CMD_RESTART, &restart)))
            MNL_ERR("MNL cold start = %d\n", ret);
        return ret;
    }
    else if (cmd == MNL_CMD_RESTART_FULL)
    {
        MTK_GPS_PARAM_RESTART restart = {MTK_GPS_START_FULL};
        if ((ret = mtk_gps_set_param (MTK_PARAM_CMD_RESTART, &restart)))
            MNL_ERR("MNL full start = %d\n", ret);
        MNL_ERR("libmnlp send FULL restart command to mnl\n"); 
        return ret;
    }
    else if (cmd == MNL_CMD_RESTART_AGPS)
    {
        MTK_GPS_PARAM_RESTART restart = {MTK_GPS_START_AGPS};
        if ((ret = mtk_gps_set_param (MTK_PARAM_CMD_RESTART, &restart)))
            MNL_ERR("MNL agps start = %d\n", ret);
        MNL_ERR("libmnlp send AGPS restart command to mnl\n");    
        return ret;
    }
        
    else if (cmd == MNL_CMD_READ_EPO_TIME)
    {
        if (mtk_agps_agent_epo_read_utc_time(&mnl_epo_time.uSecond_start, &mnl_epo_time.uSecond_expire) == MTK_GPS_ERROR)
        {
            MNL_ERR("Get EPO start/expire time fail\n ");
            char buf[] = {MNL_CMD_READ_EPO_TIME_FAIL};
            ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf));
            MNL_ERR("Send msg to MNLD that get time fail\n ");
        }
	 else
	 {
	     MNL_ERR("Get EPO start/expire time successfully\n");
	    MNL_ERR("mnl_epo_time.uSecond_start = %ld, mnl_epo_time.uSecond_expire = %ld", mnl_epo_time.uSecond_start, 
		 	mnl_epo_time.uSecond_expire);	 
	     //send the msg to MNLD here	   
	     char buf[] = {MNL_CMD_READ_EPO_TIME_DONE};
            ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf));
            MNL_ERR("Send msg to MNLD that get time ok\n ");
            //send time to MNLD
            time_t time[2];
	     time[0] = mnl_epo_time.uSecond_start ;  
	     time[1] = mnl_epo_time.uSecond_expire;	    	 
	    ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, (char*)time, sizeof(time));
	     if(!ret)
	     {
	       MNL_MSG("Send time to MNLD successfully! ret = %d\n", ret);
               return MTK_GPS_SUCCESS;
	     }else{
	       MNL_MSG("Send time to MNLD failed, ret = %d\n", ret);
               return MTK_GPS_ERROR;          
	     }   				
	 }
	 return ret;
    }
    else if (cmd == MNL_CMD_UPDATE_EPO_FILE)
    {
        if (mtk_agps_agent_epo_file_update() == MTK_GPS_ERROR)
        {
            MNL_ERR("EPO file updates fail\n");
            char buf[] = {MNL_CMD_UPDATE_EPO_FILE_FAIL};
            ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf));
	    MNL_ERR("Send msg to mnld that EPO file updates fail\n");
        }
	 else
	 {
	     char buf[] = {MNL_CMD_UPDATE_EPO_FILE_DONE};
            ret = sig_send_cmd(mnl_sinproc.dae_snd_fd, buf, sizeof(buf));
	    MNL_ERR("Send msg to mnld that EPO file updates ok\n");
	 }
	 return ret;
    }	
    else
    {
        MNL_MSG("unknown command: 0x%2X\n", cmd);
        errno = -EINVAL;
        return errno;
    }
}
/*****************************************************************************/
#define ERR_FORCE_QUIT  0x0E01
/*****************************************************************************/
static int mnlctl_handler(int fd) /*sent from mnld*/
{
    int ret;
    char cmd = MNL_CMD_UNKNOWN;
    do {
        ret = read(  fd, &cmd, sizeof(cmd) );
    } while (ret < 0 && errno == EINTR);
    if (ret == 0)
    {
        MNL_MSG("%s EOF", __FUNCTION__); /*it should not happen*/
        return 0;
    }
    else if (ret != sizeof(cmd))
    {
        MNL_MSG("%s fails: %d %d(%s)\n", __FUNCTION__, ret, errno, strerror(errno));
        return -1;
    }

    MNL_MSG("%s(0x%X)\n", __FUNCTION__, cmd);


    if (cmd == MNL_CMD_QUIT)
    {
        return ERR_FORCE_QUIT;
    }
    else
    {
        MNL_MSG("unknown command: 0x%2X\n", cmd);
        errno = -EINVAL;
        return errno;
    }
}
/*****************************************************************************/
int send_active_notify(int snd_fd) {
    if ((snd_fd != C_INVALID_FD) && !(mnl_config.debug_mnl & MNL_NMEA_DISABLE_NOTIFY))
    {
        char buf[] = {MNL_CMD_ACTIVE};
        return sig_send_cmd(snd_fd, buf, sizeof(buf));
    }
    return -1;
}
/*****************************************************************************/
void linux_signal_handler(int signo)
{
    pthread_t self = pthread_self();
    if (signo == SIGTERM)
    {
        char buf[] = {MNL_CMD_QUIT};
        sig_send_cmd(mnl_sinproc.sig_snd_fd, buf, sizeof(buf));
    }
    MNL_MSG("Signal handler of %.8x -> %s\n", (unsigned int)self, sys_siglist[signo]);
}
/*****************************************************************************/
static int linux_setup_signal_handler(void)
{
    struct sigaction actions;
    int err;
    int s[2];

    /*the signal handler is MUST, otherwise, the thread will not be killed*/
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = linux_signal_handler;
    if ((err = sigaction(SIGTERM, &actions, NULL)))
    {
        MNL_MSG("register signal hanlder for SIGTERM: %s\n", strerror(errno));
        return -1;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, s))
        return -1;

    fcntl(s[0], F_SETFD, FD_CLOEXEC);
    fcntl(s[0], F_SETFL, O_NONBLOCK);
    fcntl(s[1], F_SETFD, FD_CLOEXEC);
    fcntl(s[1], F_SETFL, O_NONBLOCK);

    mnl_sinproc.sig_snd_fd = s[0];
    mnl_sinproc.sig_rcv_fd = s[1];
    return 0;
}
/*****************************************************************************/
static void* thread_cmd_func( void*  arg )
{
    int epoll_fd   = epoll_create(2);
    int epoll_cnt  = 0;
    int err;

    // register control file descriptors for polling
    if (mnl_sinproc.dae_rcv_fd != C_INVALID_FD)
    {
        epoll_register( epoll_fd, mnl_sinproc.dae_rcv_fd);
        epoll_cnt++;
    }
    if (mnl_sinproc.sig_rcv_fd != C_INVALID_FD)
    {
        epoll_register( epoll_fd, mnl_sinproc.sig_rcv_fd);
        epoll_cnt++;
    }
    if (!epoll_cnt)
    {
        MNL_ERR("thread_cmd_func exit due to zero epoll count\n");
        goto exit;
    }

    MNL_MSG("thread_cmd_func running: PPID[%d], PID[%d]\n", getppid(), getpid());

    // now loop
    for (;;)
    {
        struct epoll_event   events[2];
        int                  ne, nevents;

        nevents = epoll_wait( epoll_fd, events, 2, -1 );
        if (nevents < 0)
        {
            if (errno == EINTR)
            {
                MNL_MSG("epoll_wait() is interrupted, try again!!\n");
            }
            else
            {
                MNL_ERR("epoll_wait() return error: %s", strerror(errno));
                goto exit;
            }
        }
        else
        {
            MNL_MSG("epoll_wait() received %d events", nevents);
        }

        for (ne = 0; ne < nevents; ne++)
        {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0)
            {
                MNL_ERR("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto exit;
            }
            if ((events[ne].events & EPOLLIN) != 0)
            {
                int  fd = events[ne].data.fd;
                if (fd == mnl_sinproc.dae_rcv_fd)
                {
                    err = mnlcmd_handler(fd);
                    if (err)
                    {
                        MNL_ERR("mnlcmd_handler: %d\n", errno);
                    }
                }
                else if (fd == mnl_sinproc.sig_rcv_fd)
                {
                    err = mnlctl_handler(fd);
                    if (err == ERR_FORCE_QUIT)
                    {
                        MNL_ERR("receives ERR_FORCE_QUIT\n");
                        goto exit;
                    }
                    else if (err)
                    {
                        MNL_ERR("mnlctl_handler: %d\n", errno);
                    }
                }
            }
        }
    }
exit:
    if (epoll_fd != C_INVALID_FD)
        close(epoll_fd);
    return NULL;
}
/*****************************************************************************/
void get_gps_version()
{

    property_set("gps.gps.version", "MTK_GPS_MT6630");

    
    return;

    //write to file

}

/*****************************************************************************/
int single_process()
{
    int idx, ret,err;
    pthread_t thread_cmd;
    clock_t beg,end;

    MNL_MSG("Execute process: PPID = %.8d, PID = %.8d\n", getppid(), getpid());

    for (idx = 0; idx < MNL_THREAD_NUM; idx++)
    {
        mnl_thread[idx].thread_id = MNL_THREAD_UNKNOWN;
        mnl_thread[idx].thread_handle = C_INVALID_TID;
        mnl_thread[idx].thread_exit = NULL;
        mnl_thread[idx].thread_active = NULL;
        mnl_thread[idx].snd_fd = mnl_sinproc.dae_snd_fd;
    }

    /*initialize system resource (message queue, mutex) used by library*/
    if ((err = mtk_gps_sys_init()))
    {
        MNL_MSG("mtk_gps_sys_init err = %d\n",errno);
        goto exit_proc;
    }

    MNL_MSG("main running\n");

    /*initialize UART/GPS device*/
    if ((err = linux_gps_dev_init()))
    {
        MNL_MSG("linux_gps_dev_init err = %d\n", errno);
        goto exit_proc;
    }

    /*initialize library thread*/
    if ((err = linux_gps_init()))
    {
        MNL_MSG("linux_gps_init err = %d\n", errno);
        goto exit_proc;
    }

    if(mnl_config.SUPL_enabled == 1 && mnl_config.SUPLSI_enabled == 0)
    {
        //wait thread agent and dispatch here
        pthread_mutex_lock(&mutex);
        while(condition != 2)
        {
            MNL_MSG("wait thread agent and dispatch...\n");
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        //send GPS driver status to SUPL 
        int sock2supl = -1;
        struct sockaddr_un local;
        mtk_agps_msg *pMsg = NULL;          
        UINT16 length = sizeof(mtk_agps_msg); 
        pMsg = (mtk_agps_msg *)malloc(length);
        if(pMsg)
        {
            pMsg->type = MTK_AGPS_SUPL_MNL_STATUS;        
            pMsg->srcMod = MTK_MOD_GPS;
            pMsg->dstMod = MTK_MOD_SUPL; 
            pMsg->length = 1;
	  		pMsg->data[0] = '1'; 
			                             			
  			if((sock2supl = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1){
                MNL_ERR("open sock2supl fail%s\r\n", strerror(errno));
                free(pMsg); 
                pMsg = NULL; 
            }else{
	            memset(&local, 0, sizeof(local));
	            local.sun_family = AF_LOCAL;
	            strcpy(local.sun_path, MTK_MNL2SUPL);

				MNL_MSG("SendMsg MTK_AGPS_SUPL_MNL_STATUS to SUPL!");
				if (sendto(sock2supl, (void *)pMsg, length, 0, (struct sockaddr*)&local, sizeof(local)) < 0)
	            {
	                MNL_ERR("send message supl fail:%s\r\n", strerror(errno));
	            }
	            close(sock2supl); 
                free(pMsg); 
                pMsg = NULL;
            }     
         }
    }    
    
    if ((err = linux_setup_signal_handler()))
    {
        MNL_MSG("linux_setup_signal_handler err = %d\n", errno);
        goto exit_proc;
    }

    MNL_MSG("MNL running..\n");
    #ifdef GET_VER
    get_gps_version();
    #endif
    thread_cmd_func(NULL);

exit_proc:
    /*exiting*/
    /*finalize library*/
    // 1. DSP in
    // 2. PMTK in
    // 3. BEE
    // 4. MNL   
    MNL_MSG("MNL exiting \n");
    mtk_gps_mnl_stop();
     // AGPS Threads
    if ((ret = linux_gps_uninit()))
    {
        MNL_ERR("linux_gps_uninit err = %d\n", errno);
        err = (err) ? (err) : (ret);
    }
    
    #if 0 // Need to check the usage of this command
    if ((ret = mtk_gps_set_param(MTK_PARAM_CMD_RESET_DSP, NULL)))
    {
        MNL_ERR("mtk_gps_set_param err = %d\n", errno);
        err = (err) ? (err) : (ret);
    }
    #endif // Need to check the usage of this command
    
    #if 0
    beg = clock();
    usleep(mnl_config.delay_reset_dsp*1000); /*to wait until software reset*/
    end = clock();
    MNL_MSG("Reset delay: %.4f ms\n", (end-beg)*1000.0/CLOCKS_PER_SEC);
    #endif
    
    if ((ret = linux_gps_dev_uninit()))
    {
        MNL_ERR("mtk_gps_sys_dev_unint err = %d\n", errno);
        err = (err) ? (err) : (ret);
    }
    // AGPS Threads
    if ((ret = mtk_gps_sys_uninit()))
    {
        MNL_ERR("mtk_gps_sys_uninit err = %d=\n", errno);
        err = (err) ? (err) : (ret);
    }
    if (mnl_sinproc.sig_rcv_fd != C_INVALID_FD)
        close(mnl_sinproc.sig_rcv_fd);
    if (mnl_sinproc.sig_snd_fd != C_INVALID_FD)
        close(mnl_sinproc.sig_snd_fd);
    if (mnl_sinproc.dae_rcv_fd != C_INVALID_FD)
        close(mnl_sinproc.dae_rcv_fd);
    if (mnl_sinproc.dae_snd_fd != C_INVALID_FD)
        close(mnl_sinproc.dae_snd_fd);

    callback_flags = 0;
    tcxo_config_everset = 0;
    MNL_MSG("MNL exiting down\n");  
    return err;
}
/*****************************************************************************/
int main (int argc, char** argv)
{
    pid_t pid = C_INVALID_PID;
    struct sched_param sched, test_sched;
    int policy = 0xff;
    int err=0xff;

    MNL_MSG("mnl_process running: argc(%d)\n", argc);
#if defined(READ_PROPERTY_FROM_FILE)
    linux_gps_load_property();
#endif
    if ((argc == 3) || (argc == 4))
    {
        int fd0 = atoi(argv[1]);
        int fd1 = atoi(argv[2]);
        if ((fd0 > 0) && (fd1 > 0))
        {
            mnl_sinproc.dae_snd_fd = fd1;
            mnl_sinproc.dae_rcv_fd = fd0;
        }
        MNL_MSG("the pipe id is %d, %d\n", fd0, fd1);
        if (argc == 4)
            mnl_config.link_speed = atoi(argv[3]);
           delete_aiding_data = atoi(argv[3]);
    }

    if (argc >= 2)
    {
        MNL_ERR("argv[1]= %s\n", argv[1]);
    }

    if (argc >= 2 && !strncmp(argv[1],"1Hz=y", 5))
    {
        /*when 1st argument = "1Hz=y", enable 1Hz GPS Mode*/
        g_is_1Hz = 1;
    }
    else
    {
        /*default to adapt 5Hz mode*/
        //g_is_1Hz = 0;
        g_is_1Hz = 1;

        /*++ adjust priority when 5 Hz Mode++*/
        policy = sched_getscheduler(0);
        sched_getparam(0, &test_sched);
        MNL_ERR("Before %s policy = %d, priority = %d\n", "main" , policy, test_sched.sched_priority);

        sched.sched_priority = RTPM_PRIO_GPS_DRIVER;
        err = sched_setscheduler(0, SCHED_FIFO, &sched);
        if(err == 0){
            MNL_ERR("pthread_setschedparam SUCCESS \n");
            policy = sched_getscheduler(0);
            sched_getparam(0, &test_sched);
            MNL_ERR("After %s policy = %d, priority = %d\n", "main" ,policy , test_sched.sched_priority);
        }
        else{
            if(err == EINVAL) MNL_ERR("policy is not one of SCHED_OTHER, SCHED_RR, SCHED_FIFO\n");
            if(err == EINVAL) MNL_ERR("the  priority  value  specified by param is not valid for the specified policy\n");
            if(err == EPERM) MNL_ERR("the calling process does not have superuser permissions\n");
            if(err == ESRCH) MNL_ERR("the target_thread is invalid or has already terminated\n");
            if(err == EFAULT)  MNL_ERR("param points outside the process memory space\n");
            MNL_ERR("pthread_setschedparam FAIL \n");
        }
        /*--*/
    }

    return single_process();
}


