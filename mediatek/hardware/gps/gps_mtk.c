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


#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <sys/ioctl.h>
#include <sys/time.h>
//#include <linux/mtgpio.h>
//for EPO file
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <private/android_filesystem_config.h>

#define EPO_FILE "/data/misc/EPO.DAT"
#define EPO_FILE_HAL "/data/misc/EPOHAL.DAT"
#define MTK_EPO_SET_PER_DAY  4
#define MTK_EPO_MAX_DAY      30
#define MTK_EPO_ONE_SV_SIZE  72
#define MTK_EPO_SET_MAX_SIZE 2304  //72*32, One SET
#define MTK_EPO_MAX_SET (MTK_EPO_MAX_DAY * MTK_EPO_SET_PER_DAY)
#define MTK_EPO_EXPIRED 15*24*60*60 //15 days
#define BUF_SIZE MTK_EPO_SET_MAX_SIZE

#define  LOG_TAG  "gps_mt3326"
#include <cutils/xlog.h>
#include <cutils/sockets.h>
#include <cutils/properties.h>
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#endif 

#include <hardware/gps.h>

/* the name of the controlled socket */
#define  GPS_CHANNEL_NAME       "/dev/gps"
#define  GPS_MNL_DAEMON_NAME    "mnld"
#define  GPS_MNL_DAEMON_PROP    "init.svc.mnld"

#define  GPS_AT_COMMAND_SOCK    "/data/server"
#define  MNL_CONFIG_STATUS      "persist.radio.mnl.prop" 

#define  GPS_OP   "AT%GPS"      
#define  GNSS_OP  "AT%GNSS"     
#define  GPS_AT_ACK_SIZE        40

#define  GPS_DEBUG  1
#define  NEMA_DEBUG 0   /*the flag works if GPS_DEBUG is defined*/

#if GPS_DEBUG
#define  TRC(f)       XLOGD("%s", __func__)
#define  ERR(f, ...)  XLOGE("%s: line = %d" f, __func__,__LINE__, ##__VA_ARGS__)
#define  WAN(f, ...)  XLOGW("%s: line = %d" f, __func__, __LINE__,##__VA_ARGS__)
#define DBG(f, ...) XLOGD("%s: line = %d" f, __func__, __LINE__,##__VA_ARGS__)
#define VER(f, ...) ((void)0) //((void)0)//
#else
#  define DBG(...)    ((void)0)
#  define VER(...)    ((void)0)
#endif
#define SEM 0
#define NEED_IPC_WITH_CODEC 1
#if (NEED_IPC_WITH_CODEC)
#include <private/android_filesystem_config.h>
#define EVDO_CODEC_SOC		"/data/misc/codec_sock"
#endif
static int flag_unlock = 0;
#define GPS_MNL_PROCESS_STATUS "gps.mnl_process.status"

/*****************************************************************************/
/*    MT3326 device control                                                  */   
/*****************************************************************************/
enum { 
    MNL_CMD_UNKNOWN = -1,
    /*command send from GPS HAL*/    
    MNL_CMD_INIT            = 0x00,
    MNL_CMD_CLEANUP         = 0x01,
    MNL_CMD_STOP            = 0x02,
    MNL_CMD_START           = 0x03,
    MNL_CMD_RESTART         = 0x04,    /*restart MNL process*/
    MNL_CMD_RESTART_HOT     = 0x05,    /*restart MNL by PMTK command: hot start*/
    MNL_CMD_RESTART_WARM    = 0x06,    /*restart MNL by PMTK command: warm start*/
    MNL_CMD_RESTART_COLD    = 0x07,    /*restart MNL by PMTK command: cold start*/
    MNL_CMD_RESTART_FULL    = 0x08,    /*restart MNL by PMTK command: full start*/
    MNL_CMD_RESTART_AGPS    = 0x09,    /*restart MNL by PMTK command: AGPS start*/

   MNL_CMD_READ_EPO_TIME = 0x33,
   MNL_CMD_UPDATE_EPO_FILE = 0x34,
   
    MNL_CMD_GPS_LOG_WRITE = 0x40,
   
    /*please see mnld.c for other definition*/
};

enum {
    HAL_CMD_STOP_UNKNOWN = -1,
    HAL_CMD_READ_EPO_TIME_DONE = 0x35,
    HAL_CMD_UPDATE_EPO_FILE_DONE = 0x36,
    
    HAL_CMD_READ_EPO_TIME_FAIL = 0x37,
    HAL_CMD_UPDATE_EPO_FILE_FAIL = 0x38,
    HAL_CMD_MNL_DIE = 0x41,
};

#define M_START 0
#define M_STOP 1
#define M_CLEANUP 2
#define M_INIT	3
#define M_THREAD_EXIT 4
typedef struct sync_lock
{
	pthread_mutex_t mutx;
	pthread_cond_t con;
	int condtion;
}SYNC_LOCK_T;
static SYNC_LOCK_T lock_for_sync[] = {{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0}, 
								{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
								{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
								{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0},
								{PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER, 0}};

const char* gps_native_thread = "GPS NATIVE THREAD";
static GpsCallbacks callback_backup;
static int start_flag = 0;
//for different SV parse
typedef enum{
    GPS_SV = 0,
    GLONASS_SV,
    BDS_SV,
    GALILEO_SV,
}SV_TYPE;

//zqh: download EPO by request
static GpsXtraCallbacks mGpsXtraCallbacks;
typedef struct
{
    int length;
    char* data;	
}EpoData;
EpoData epo_data;
static int started = 0;
/*---------------------------------------------------------------------------*/
typedef struct {    
    int sock; 
} MT3326_GPS;
/*****************************************************************************/
static MT3326_GPS mt3326_gps = {
    .sock = -1,
};
static int gps_nmea_end_tag = 0;
struct sockaddr_un cmd_local;
struct sockaddr_un remote;
socklen_t remotelen;
remotelen = sizeof(remote);

/*****************************************************************************/
/*AT command test state*/
static int MNL_AT_TEST_FLAG = 0;  
enum {
    MNL_AT_TEST_UNKNOWN = -1,
    MNL_AT_TEST_START       = 0x00,
    MNL_AT_TEST_STOP        = 0x01,
    MNL_AT_TEST_INPROGRESS   = 0x02,
    MNL_AT_TEST_DONE       = 0x03,
};
int MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
typedef struct {
    int test_num;
    int prn_num;
    int time_delay;
}HAL_AT_TEST_T;

static HAL_AT_TEST_T hal_test_data = {
    .test_num = 0,
    .prn_num = 0,
    .time_delay = 0,
};

static time_t start_time;
static time_t end_time;
int* Dev_CNr = NULL;
int prn[32] = {0};
int snr[32] = {0};
/* 
* Test result array: error_code, theta(0), phi(0), Success_Num, 
* Complete_Num, Avg_CNo, Dev_CNo, Avg_Speed(0)
*/
static int result[8] = {1, 0, 0, 0, 0, 0, 0, 0};
static int test_num = 0;
static int CNo, DCNo;
static int Avg_CNo = 0;
static int Dev_CNo = 0; 
static int Completed_Num = 0;
static int Success_Num = 0;
static int Err_Code = 1;
int test_mode_flag = 1; //0: USB mode, 1: SMS mode

static int get_prop()
{
    //Read property
    char result[PROPERTY_VALUE_MAX] = {0};
    int ret = 0;
    if(property_get(MNL_CONFIG_STATUS, result, NULL)){
        ret = result[6] - '0';
      	DBG("gps.log: %s, %d\n", &result[6], ret);       	 
    }else{
        ret = 0;
        DBG("Config is not set yet, use default value");
    }
    
    return ret;
}

static void get_condition(SYNC_LOCK_T *lock)
{
	
	while (!lock->condtion)
		DBG("ret cond wait = %d\n" ,pthread_cond_wait(&(lock->con), &(lock->mutx)));
	
	lock->condtion = 0;

	return;
}

static void release_condition(SYNC_LOCK_T *lock)
{
								
	lock->condtion= 1;	
	DBG ("ret cond_signal = %d\n", pthread_cond_signal(&(lock->con)));
					
	return;
}

struct prop_info {
    unsigned volatile serial;
    char value[PROP_VALUE_MAX];
    char name[0];
};

typedef struct prop_info prop_info;
/*****************************************************************************/
int mt3326_start_daemon() { /*gps driver must exist before running the function*/
    int start = 0;
    char status[PROPERTY_VALUE_MAX] = {0};
    int count = 100, len;
#ifdef HAVE_LIBC_SYSTEM_PROPERTIES    
    const prop_info *pi = NULL;
    unsigned serial = 0;
#endif     
    TRC();

#ifdef HAVE_LIBC_SYSTEM_PROPERTIES
    pi = __system_property_find(GPS_MNL_DAEMON_PROP);
    if (pi != NULL)
        serial = pi->serial;
#endif

	property_set("ctl.start", GPS_MNL_DAEMON_NAME);
	sched_yield();

    while (count-- > 0) {
 #ifdef HAVE_LIBC_SYSTEM_PROPERTIES
        if (pi == NULL) {
            pi = __system_property_find(GPS_MNL_DAEMON_PROP);
        }
        if (pi != NULL) {
            __system_property_read(pi, NULL, status);
            if (strcmp(status, "running") == 0) {
                DBG("running\n");
                return 0;
            } else if (pi->serial != serial &&
                    strcmp(status, "stopped") == 0) {
                return -1;
            }
        }
#else
        if (property_get(GPS_MNL_DAEMON_PROP, status, NULL)) {
            if (strcmp(status, "running") == 0)
                return 0;
        }
#endif
        WAN("[%02d] '%s'\n", count, status);
        usleep(100000);
    }
    ERR("start daemon timeout!!\n");
    return -1;
}
/*****************************************************************************/
int mt3326_stop_daemon() {
    char status[PROPERTY_VALUE_MAX] = {0};
    int count = 50;

    TRC();
    if (property_get(GPS_MNL_DAEMON_PROP, status, NULL) && 
        strcmp (status, "stopped") == 0) {
        property_set(GPS_MNL_PROCESS_STATUS, "0000");
        DBG("daemon is already stopped!!");
        return 0;
    }

    property_set("ctl.stop", GPS_MNL_DAEMON_NAME);
    sched_yield();

    while (count-- > 0) {
        if (property_get(GPS_MNL_DAEMON_PROP, status, NULL)) {
            if (strcmp(status, "stopped") == 0) {
                property_set(GPS_MNL_PROCESS_STATUS, "0000");
                DBG("daemon is stopped, set %s 0000", GPS_MNL_PROCESS_STATUS);                
                return 0;
            }
        }
        usleep(100000);
    }
    ERR("stop daemon timeout!!\n");
    return -1;
}
/*****************************************************************************/
int mt3326_daemon_init() {
    int err = -1, lsocket;
    int count = 10;
    
    if (mt3326_start_daemon() < 0) {
        ERR("start daemon fail: %s\n", strerror(errno));
        return -1;
    }

    lsocket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (lsocket < 0) {
        ERR("fail to create socket: %s\n", strerror(errno));
        return -1;
    }
    
    while (count-- > 0) {
        err = socket_local_client_connect(lsocket, GPS_MNL_DAEMON_NAME, 
                                    ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
        /*connect success*/
        if (err >= 0)   
            break;
        /*connect retry*/
        WAN("[%02d] retry\n", count);
        err = mt3326_start_daemon();
        if (err < 0)
            break;
        usleep(100000);    
    }
    if (err < 0) {
        ERR("socket_local_client_connect fail: %s\n", strerror(errno));
        return -1;
    }    
    mt3326_gps.sock = lsocket;
    DBG("socket : %d\n", lsocket);
    return 0;
}
/*****************************************************************************/
int mt3326_daemon_cleanup() {
    TRC();
    if (mt3326_gps.sock != -1) 
        close(mt3326_gps.sock);    
    return mt3326_stop_daemon();
}
/*****************************************************************************/
int mt3326_daemon_send(char* cmd, int len) {
    DBG("send: %d,%d\n", (int)(*cmd), len);
    int ret;
    if (mt3326_gps.sock != -1) {
        ret = send(mt3326_gps.sock, cmd, len, 0);
        if (ret != len)
            ERR("send fails: %d(%s)\n", errno, strerror(errno));
        return (ret != len) ? (-1) : (0);        
    }
    errno = -EINVAL;        
    return -1;
}
/*****************************************************************************/
int mt3326_init() 
{
    int err;
    char buf[] = {MNL_CMD_INIT};
    TRC();    
    if ((err = mt3326_daemon_init()))
        return err;
    return mt3326_daemon_send(buf, sizeof(buf));    
}
/*****************************************************************************/
int mt3326_cleanup()
{
    int err, res;
    char buf[] = {MNL_CMD_CLEANUP};
    if ((err = mt3326_daemon_send(buf, sizeof(buf))))
        ERR("mt3326_cleanup send cleanup fails\n");
	DBG("let MNLD die\n");
	usleep(100000);
    res = mt3326_daemon_cleanup();
    //DBG("cleanup: %d, %d\n", err, res);
    return 0;
}
/*****************************************************************************/
int mt3326_start()
{
    char buf[] = {MNL_CMD_START};
    int err;
    int idx = 0, max = 5;
    TRC();
    err = mt3326_daemon_send(buf, sizeof(buf));
    /*sometimes, the mnld is restarted, so */
    while (err && (errno == EPIPE) && (idx++ < max)) {
        sleep(1);    
        if ((err = mt3326_daemon_init()))
            err = mt3326_daemon_send(buf, sizeof(buf));
    }
    return err;
}
/*****************************************************************************/
int mt3326_stop()
{
    char buf[] = {MNL_CMD_STOP};
    TRC();
    return mt3326_daemon_send(buf, sizeof(buf));
}
/*****************************************************************************/
int mt3326_restart(unsigned char cmd)
{
    char buf[] = {cmd};
    TRC();
    return mt3326_daemon_send(buf, sizeof(buf));
}
/*****************************************************************************/

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
    const char*  p;
    const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  24

typedef struct {
    int     count;
    Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end )
{
    int    count = 0;
    char*  q;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentecne
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (q >= p) {
            if (count < MAX_NMEA_TOKENS) {
                t->tokens[count].p   = p;
                t->tokens[count].end = q;
                count += 1;
            }
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}


static int
str2int( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    int   sign = 1;

    if (*p == '-')
    {
        sign = -1;
        p++;
        len = end - p;
    }

    for ( ; len > 0; len--, p++ )
    {
        int  c;

        if (p >= end)
            goto Fail;

        c = *p - '0';
        if ((unsigned)c >= 10)
            goto Fail;

        result = result*10 + c;
    }
    return  sign*result;

Fail:
    return -1;
}

static double
str2float( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy( temp, p, len );
    temp[len] = 0;
    return strtod( temp, NULL );
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

#define  NMEA_MAX_SIZE  83
/*maximum number of SV information in GPGSV*/
#define  NMEA_MAX_SV_INFO 4 
#define  LOC_FIXED(pNmeaReader) ((pNmeaReader->fix_mode == 2) || (pNmeaReader->fix_mode ==3))
typedef struct {
    int     pos;
    int     overflow;
    int     utc_year;
    int     utc_mon;
    int     utc_day;
    int     utc_diff;
    GpsLocation  fix;

    /*
     * The fix flag extracted from GPGSA setence: 1: No fix; 2 = 2D; 3 = 3D
     * if the fix mode is 0, no location will be reported via callback
     * otherwise, the location will be reported via callback
     */
    int     fix_mode;           
    /*
     * Indicate that the status of callback handling.
     * The flag is used to report GPS_STATUS_SESSION_BEGIN or GPS_STATUS_SESSION_END:
     * (0) The flag will be set as true when callback setting is changed via nmea_reader_set_callback
     * (1) GPS_STATUS_SESSION_BEGIN: receive location fix + flag set + callback is set
     * (2) GPS_STATUS_SESSION_END:   receive location fix + flag set + callback is null
     */
    int     cb_status_changed;  
    int     sv_count;           /*used to count the number of received SV information*/    
    GpsSvStatus  sv_status;  
    GpsCallbacks callbacks;
    GpsTestResult test_result;
    char    in[ NMEA_MAX_SIZE+1 ];
} NmeaReader;


static void
nmea_reader_update_utc_diff( NmeaReader* const r )
{
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    unsigned long  time_local, time_utc;

    gmtime_r( &now, &tm_utc );
    localtime_r( &now, &tm_local );


    time_local = mktime(&tm_local);


    time_utc = mktime(&tm_utc);

    r->utc_diff = time_utc - time_local;
}


static void
nmea_reader_init( NmeaReader* const r )
{
    memset( r, 0, sizeof(*r) );

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
    r->utc_diff = 0;

    r->sv_count = 0;
    r->fix_mode = 0;    /*no fix*/
    r->cb_status_changed = 0;
    memset((void*)&r->sv_status, 0x00, sizeof(r->sv_status));
    memset((void*)&r->in, 0x00, sizeof(r->in));
    
    nmea_reader_update_utc_diff( r );
}

GpsStatus sta;
static void
nmea_reader_set_callback( NmeaReader* const r, GpsCallbacks* const cbs)
{
    if (!r) {           /*this should not happen*/
        return; 
    } else if (!cbs) {  /*unregister the callback */
        if(MNL_AT_TEST_FLAG) {            
            DBG("**GPS AT Command test mode, unregister the test_cb!!");
            if(r->callbacks.test_cb != NULL) {
                DBG("**Unregister test_callback!!");
                r->callbacks.test_cb = NULL;
            } else {
                DBG("**test_callback is NULL");
            }
        } 

        return ;
    } else {            /*register the callback*/    
        if(MNL_AT_TEST_FLAG) {            
            r->callbacks.test_cb = cbs->test_cb;
            DBG("**GPS AT Command test mode, register test_cb = %p", r->callbacks.test_cb);
        } 

		else
		{
			r->fix.flags = 0;
			r->sv_count = r->sv_status.num_svs = 0;
		}
    }
}


static int
nmea_reader_update_time( NmeaReader* const r, Token  tok )
{
    int        hour, minute;
    double     seconds;
    struct tm  tm;
    time_t     fix_time;

    if (tok.p + 6 > tok.end)
        return -1;
        
    memset((void*)&tm, 0x00, sizeof(tm));
    if (r->utc_year < 0) {
        // no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r( &now, &tm );
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    hour    = str2int(tok.p,   tok.p+2);
    minute  = str2int(tok.p+2, tok.p+4);
    seconds = str2float(tok.p+4, tok.end);

    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = (int) seconds;
    tm.tm_year = r->utc_year - 1900;
    tm.tm_mon  = r->utc_mon - 1;
    tm.tm_mday = r->utc_day;
    tm.tm_isdst = -1;

    if (mktime(&tm) == (time_t)-1)
        ERR("mktime error: %d %s\n", errno, strerror(errno));
        
    //Add by ZQH to recalculate the utc_diff when the time zone is reset     
    nmea_reader_update_utc_diff( r );
        
    fix_time = mktime( &tm ) - r->utc_diff;
    r->fix.timestamp = (long long)fix_time * 1000;
    return 0;
}

static int
nmea_reader_update_date( NmeaReader* const r, Token  date, Token  time )
{
    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        ERR("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    return nmea_reader_update_time( r, time );
}


static double
convert_from_hhmm( Token  tok )
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}


static int
nmea_reader_update_latlong( NmeaReader* const r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi )
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        ERR("latitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        ERR("longitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    return 0;
}

static int    
nmea_reader_update_at_test_result(NmeaReader* const r,
                                  int Err_Code,
                                  int Success_Num,
                                  int Completed_Num,
                                  int Avg_CNo,
                                  int Dev_CNo)
{
    r->test_result.error_code = Err_Code;
    r->test_result.theta = 0;
    r->test_result.phi = 0;
    r->test_result.success_num = Success_Num;
    r->test_result.completed_num = Completed_Num;
    r->test_result.avg_cno = Avg_CNo;
    r->test_result.dev_cno = Dev_CNo;
    r->test_result.avg_speed = 0;    
    if (r->callbacks.test_cb) {
        r->callbacks.test_cb( &r->test_result );
        DBG("**AT command test set callback!!");
    } else {
        VER("**AT Command test: no test result callback !!");
    }    
    return 0;    
}


static int
nmea_reader_update_altitude( NmeaReader* const r,
                             Token        altitude,
                             Token        units )
{
    double  alt;
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    r->fix.altitude = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_bearing( NmeaReader* const r,
                            Token        bearing )
{
    double  alt;
    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    r->fix.bearing  = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_speed( NmeaReader* const r,
                          Token        speed )
{
    double  alt;
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_SPEED;

    //Modify by ZQH to convert the speed unit from knot to m/s
    //r->fix.speed    = str2float(tok.p, tok.end);
    r->fix.speed = str2float(tok.p, tok.end) / 1.942795467;
    return 0;
}

//Add by LCH for accuracy
static int
nmea_reader_update_accuracy( NmeaReader* const r,
                             Token accuracy )
{
    double  alt;
    Token   tok = accuracy;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
    r->fix.accuracy = str2float(tok.p, tok.end);
    return 0;
}

static void
gps_at_command_test_proc(NmeaReader* const r){
    //For AT command test
    int i = 0;
    int j = 0;
    int time_diff;
    time_t current_time;

    if (MNL_AT_TEST_STATE != MNL_AT_TEST_DONE){
        DBG("**AT Command test mode!!");
        time(&current_time);
        time_diff = current_time - start_time;
        if(time_diff >= hal_test_data.time_delay){
            if(test_num < hal_test_data.test_num){
                DBG("**AT Command Continue");         
                for (i = 0; i < r->sv_count; i++){        
                    if (prn[i] == hal_test_data.prn_num){        
                        DBG("**AT Command test SvID: %d", prn[i]); 
                        if (snr[i] != 0){                 
                            Err_Code = 0;                            
                            CNo += snr[i]*10;                 
                            Dev_CNr[Success_Num] = snr[i]*10;
                            Success_Num++;
                            Avg_CNo = CNo / Success_Num;
                            DBG("CNo = %d, Avg_CNo /= %d, Success_Num = %d", CNo, Avg_CNo, Success_Num);
                        } else {
                            DBG("**SNR is 0, ignore!!!");
                        }           

                        if (Success_Num != 0){                 
                            for(j = 0; j < Success_Num; j++){
                                DCNo += (Dev_CNr[j]-Avg_CNo) * (Dev_CNr[j]-Avg_CNo);
                                DBG("Dev_CNr[%d] = %d, Dev_CNo2 += %d", j, Dev_CNr[j], DCNo);
                            }
                            Dev_CNo = DCNo / Success_Num;
                            DCNo = 0;
                            Dev_CNo = sqrt(Dev_CNo);                 
                        }
                        DBG("**AT Command find SvID: %d, exit",prn[i]);
                        break;
                    }else {
                        DBG("**AT Command ignore SvID: %d", prn[i]);
                    }
                }
                test_num++;
                Completed_Num++;
                DBG("**AT Command %d times, Err_Code = %d, Avg_CNo = %d, Completed_Num = %d, Success_Num = %d, Dev_CNo = %d",
                                                  test_num, Err_Code, Avg_CNo, Completed_Num, Success_Num, Dev_CNo);

                if (Completed_Num != 0){
                    DBG("**AT Command test %d times, update result!!", Completed_Num);
                    nmea_reader_update_at_test_result(r, Err_Code, Success_Num, Completed_Num, Avg_CNo, Dev_CNo); 
                }
            }else {             
                /*
                 * Test result array: error_code, theta(0), phi(0), Success_Num,
                 * Complete_Num, Avg_CNo, Dev_CNo, Avg_Speed(0)
                */
                //static int result[8] = {1, 0, 0, 0, 0, 0, 0, 0};
                result[0] = Err_Code;
                result[3] = Success_Num;
                result[4] = Completed_Num;
                result[5]= Avg_CNo;
                result[6] = Dev_CNo;
                MNL_AT_TEST_STATE = MNL_AT_TEST_DONE;

                DBG("**AT Command test_start done, Success_Num = %d, Completed_Num = %d, Avg_CNo = %d, Dev_CNo = %d, Err_Code = %d,test_num = %d, MNL_AT_TEST_STATE = %d", 
                              Success_Num, Completed_Num, Avg_CNo, Dev_CNo, Err_Code, test_num, MNL_AT_TEST_STATE);
                
                if((MNL_AT_TEST_STATE == MNL_AT_TEST_DONE)&&(1 == MNL_AT_TEST_FLAG)){
                    DBG("** AT Command test done, stop GPS driver **");
                    mt3326_gps_test_stop();
                }                
            }
        }  else {
            DBG("time_diff is %d", time_diff);
        }
    }else {
        DBG("**AT Command test, test mode is MNL_AT_TEST_DONE");
    }
}

static int
nmea_reader_update_sv_status( NmeaReader* r, int sv_index,
                              int id, Token elevation,
                              Token azimuth, Token snr) 
{   
    //int prn = str2int(id.p, id.end);
    int prn = id;    
    if ((prn <= 0) || (prn > GPS_MAX_SVS) || (r->sv_count >= GPS_MAX_SVS)) {
        VER("sv_status: ignore (%d)", prn);
        return 0;
    }
    sv_index = r->sv_count+r->sv_status.num_svs;
    r->sv_status.sv_list[sv_index].prn = prn;
    r->sv_status.sv_list[sv_index].snr = str2float(snr.p, snr.end);
    r->sv_status.sv_list[sv_index].elevation = str2int(elevation.p, elevation.end);
    r->sv_status.sv_list[sv_index].azimuth = str2int(azimuth.p, azimuth.end);
    r->sv_count++;
    VER("sv_status(%2d): %2d, %2f, %3f, %2f", sv_index, r->sv_status.sv_list[sv_index].prn, r->sv_status.sv_list[sv_index].elevation,
                                         r->sv_status.sv_list[sv_index].azimuth, r->sv_status.sv_list[sv_index].snr);        
    return 0;
}

static void
nmea_reader_parse( NmeaReader* const r )
{
   /* we received a complete sentence, now parse it to generate
    * a new GPS fix...
    */
    NmeaTokenizer  tzer[1];
    Token          tok;
    Token          mtok;
    SV_TYPE sv_type = 0;

#if NEMA_DEBUG    
    DBG("Received: '%.*s'", r->pos, r->in);
#endif     
    if (r->pos < 9) {
        ERR("Too short. discarded. '%.*s'", r->pos, r->in);
        return;
    }

    nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
#if NEMA_DEBUG
    {
        int  n;
        DBG("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer,n);
            DBG("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        ERR("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
        return;
    }

    // ignore first two characters.
    mtok.p = tok.p; //Mark the first two char for GPS,GLONASS,BDS SV parse.
    if(!memcmp(mtok.p, "BD", 2)){
        sv_type = BDS_SV;
        DBG("BDS SV type");
    }
    tok.p += 2;
    if ( !memcmp(tok.p, "GGA", 3) ) {
        // GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
        Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer,10);

        nmea_reader_update_time(r, tok_time);
        nmea_reader_update_latlong(r, tok_latitude,
                                      tok_latitudeHemi.p[0],
                                      tok_longitude,
                                      tok_longitudeHemi.p[0]);
        nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

    } else if ( !memcmp(tok.p, "GSA", 3) ) {
//    	DBG("[debug mask] and mask = %d\n ", r->sv_status.used_in_fix_mask);
        Token tok_fix = nmea_tokenizer_get(tzer, 2);    
        int idx, max = 12; /*the number of satellites in GPGSA*/

        r->fix_mode = str2int(tok_fix.p, tok_fix.end);   
//			memset(r->sv_status.used_in_fix_mask, 0, sizeof(r->sv_status.used_in_fix_mask));
//        r->sv_status.used_in_fix_mask = 0;
        if (LOC_FIXED(r)) { /* 1: No fix; 2: 2D; 3: 3D*/
//				DBG("[debug mask] has been fixed and mask = %d\n ", r->sv_status.used_in_fix_mask);
            for (idx = 0; idx < max; idx++) {
                Token tok_satellite = nmea_tokenizer_get(tzer, idx+3);
                if (tok_satellite.p == tok_satellite.end) {                    
                    DBG("GSA: found %d active satellites\n", idx);					
                    break;
                }
                int sate_id = str2int(tok_satellite.p, tok_satellite.end);
                if (sv_type == BDS_SV){				
                    sate_id += 200;
                    DBG("It is BDGSA: %d", sate_id);
                }
                if (sate_id >= 1 && sate_id <= 32) 
				{
                    r->sv_status.used_in_fix_mask[0] |= ( 1 << (sate_id-1) );
				//	DBG("[debug mask] satellite is valid & mask[0] = 0x%x\n ", r->sv_status.used_in_fix_mask[0]);
                } 
				else if (sate_id >= 33 && sate_id <= 64)
                {
                	r->sv_status.used_in_fix_mask[1] |= ( 1 << (sate_id-33) );
				//	DBG("[debug mask] satellite is valid & mask[1] = 0x%x\n ", r->sv_status.used_in_fix_mask[1]);
                }
				else if(sate_id >= 65 && sate_id <= 96)
				{
					r->sv_status.used_in_fix_mask[2] |= ( 1 << (sate_id-65) );
				//	DBG("[debug mask] satellite is valid & mask[2] = 0x%x\n ", r->sv_status.used_in_fix_mask[2]);
				}
				else if(sate_id >= 97 && sate_id <= 128)
				{
					r->sv_status.used_in_fix_mask[3] |= ( 1 << (sate_id-97) );
				//	DBG("[debug mask] satellite is valid & mask[3] = 0x%x\n ", r->sv_status.used_in_fix_mask[3]);
				}
				else if(sate_id >= 129 && sate_id <= 160)
				{
					r->sv_status.used_in_fix_mask[4] |= ( 1 << (sate_id-129) );
				//	DBG("[debug mask] satellite is valid & mask[4] = 0x%x\n ", r->sv_status.used_in_fix_mask[4]);
				}
				else if(sate_id >= 161 && sate_id <= 192)
				{
					r->sv_status.used_in_fix_mask[5] |= ( 1 << (sate_id-161) );
				//	DBG("[debug mask] satellite is valid & mask[5] = 0x%x\n ", r->sv_status.used_in_fix_mask[5]);
				}
				else if(sate_id >= 193 && sate_id <= 224)
				{
					r->sv_status.used_in_fix_mask[6] |= ( 1 << (sate_id-193) );
				//	DBG("[debug mask] satellite is valid & mask[6] = 0x%x\n ", r->sv_status.used_in_fix_mask[6]);
				}
				else if(sate_id >= 225 && sate_id <= 256)
				{
					r->sv_status.used_in_fix_mask[7] |= ( 1 << (sate_id-225) );
				//	DBG("[debug mask] satellite is valid & mask[7] = 0x%x\n ", r->sv_status.used_in_fix_mask[7]);
				}
                else
				{
				   	memset(r->sv_status.used_in_fix_mask, 0, sizeof(r->sv_status.used_in_fix_mask));
					VER("[debug mask] satellite is invalid & mask is set to 0\n ");
					break;
				}                   
                }
            }
        }
 //       VER("GPGSA: mask 0x%x", r->sv_status.used_in_fix_mask);
    else if ( !memcmp(tok.p, "RMC", 3) ) {
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
        Token  tok_speed         = nmea_tokenizer_get(tzer,7);
        Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
        Token  tok_date          = nmea_tokenizer_get(tzer,9);

        VER("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
        if (tok_fixStatus.p[0] == 'A')
        {
            nmea_reader_update_date( r, tok_date, tok_time );

            nmea_reader_update_latlong( r, tok_latitude,
                                           tok_latitudeHemi.p[0],
                                           tok_longitude,
                                           tok_longitudeHemi.p[0] );

            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed  ( r, tok_speed );
        }
    } else if ( !memcmp(tok.p, "GSV", 3) ) {
        Token tok_num = nmea_tokenizer_get(tzer,1); //number of messages
        Token tok_seq = nmea_tokenizer_get(tzer,2); //sequence number
        Token tok_cnt = nmea_tokenizer_get(tzer,3); //Satellites in view
        int num = str2int(tok_num.p, tok_num.end);
        int seq = str2int(tok_seq.p, tok_seq.end);
        int cnt = str2int(tok_cnt.p, tok_cnt.end);
        int sv_base = (seq - 1)*NMEA_MAX_SV_INFO;
        int sv_num = cnt - sv_base;
        int idx, base = 4, base_idx;
        if (sv_num > NMEA_MAX_SV_INFO) 
            sv_num = NMEA_MAX_SV_INFO;
        if (seq == 1)   /*if sequence number is 1, a new set of GPGSV will be parsed*/
            r->sv_count = 0;
        for (idx = 0; idx < sv_num; idx++) {            
            base_idx = base*(idx+1);
            Token tok_id  = nmea_tokenizer_get(tzer, base_idx+0);
            int sv_id = str2int(tok_id.p, tok_id.end);			
            if (sv_type == BDS_SV){				
                sv_id += 200;
                DBG("It is BDS SV: %d", sv_id);
            }
            Token tok_ele = nmea_tokenizer_get(tzer, base_idx+1);
            Token tok_azi = nmea_tokenizer_get(tzer, base_idx+2);
            Token tok_snr = nmea_tokenizer_get(tzer, base_idx+3);
            prn[r->sv_count] = str2int(tok_id.p, tok_id.end);            
            snr[r->sv_count] = str2int(tok_snr.p, tok_snr.end);       
            nmea_reader_update_sv_status(r, sv_base+idx, sv_id, tok_ele, tok_azi, tok_snr);
        }
        if (seq == num) {
 //           if (r->sv_count <= cnt) { 
                r->sv_status.num_svs += r->sv_count;
            //    r->sv_status.almanac_mask = 0;
				memset(r->sv_status.almanac_mask, 0, sizeof(r->sv_status.almanac_mask));
             //   r->sv_status.ephemeris_mask = 0;
				memset(r->sv_status.almanac_mask, 0, sizeof(r->sv_status.almanac_mask));
                 if (1 == MNL_AT_TEST_FLAG){                    
                    gps_at_command_test_proc(r);                  
                }  
//            } else {
//                ERR("GPGSV incomplete (%d/%d), ignored!", r->sv_count, cnt);
//                r->sv_count = r->sv_status.num_svs = 0;                
 //           }
        }        
    }
    //Add for Accuracy, LCH
    else if ( !memcmp(tok.p, "ACCURACY", 8)) {
        if((r->fix_mode == 3)||(r->fix_mode == 2)) {
        //if(LOC_FIXED(r)) {
        Token  tok_accuracy = nmea_tokenizer_get(tzer,1);
        nmea_reader_update_accuracy(r, tok_accuracy);
            DBG("GPS get accuracy from driver:%f\n", r->fix.accuracy);
        }
        else {
            DBG("GPS get accuracy failed, fix mode:%d\n", r->fix_mode);
        } 
    }
    else {
        tok.p -= 2;
        VER("unknown sentence '%.*s", tok.end-tok.p, tok.p);
    }
    if (!LOC_FIXED(r)) {
        VER("Location is not fixed, ignored callback\n");
    } else if (r->fix.flags != 0 && gps_nmea_end_tag) {
#if NEMA_DEBUG
        char   temp[256];
        char*  p   = temp;
        char*  end = p + sizeof(temp);
        struct tm   utc;

        p += snprintf( p, end-p, "sending fix" );
        if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
            p += snprintf(p,end-p, " accuracy=%g", r->fix.accuracy);
            DBG("GPS accuracy=%g\n", r->fix.accuracy);
        }
        gmtime_r( (time_t*) &r->fix.timestamp, &utc );
        p += snprintf(p, end-p, " time=%s", asctime( &utc ) );
        VER(temp);
#endif
    if(get_prop()){   
        char pos[32] = {0};
        sprintf(pos, "%lf, %lf", r->fix.latitude, r->fix.longitude); 

        DBG("gps postion str = %s", pos);
             
	      
        char buff[] = {MNL_CMD_GPS_LOG_WRITE};
        mt3326_daemon_send(buff, sizeof(buff));
        DBG("Send position to MNLD");
        //sleep 
        mt3326_daemon_send(pos, sizeof(pos));
	      	      
        /*
        if (write_gps_log_to_sdcard(str) < 0)
            write_gps_log_to_data(str);
        */
        DBG("write stop");
    }
     

        callback_backup.location_cb(&r->fix);
            r->fix.flags = 0;
        }
    if (r->sv_status.num_svs != 0 && gps_nmea_end_tag) 
       {
            int idx;
                   
            for(idx = 0; idx < 8; idx++)
            {
            	DBG("sv_status.used_in_fix_mask[%d] = 0x%x\n", idx, r->sv_status.used_in_fix_mask[idx]);
			}
            callback_backup.sv_status_cb(&r->sv_status);
            r->sv_count = r->sv_status.num_svs = 0;
            DBG("clear sv_status.used_in_fix_mask\n\n");
            memset(r->sv_status.used_in_fix_mask, 0, sizeof(r->sv_status.used_in_fix_mask));
        } 
    
}


static void
nmea_reader_addc( NmeaReader* const r, int  c )
{
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= (int) sizeof(r->in)-1 ) {
        r->overflow = 1;
        r->pos      = 0;
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos       += 1;

    if (c == '\n') 
	{

    	nmea_reader_parse( r );
        
//        DBG("the structure include nmea_cb address is %p\n", r);
//   	    DBG("nmea_cb address is %p\n", r->callbacks.nmea_cb);
        callback_backup.nmea_cb( r->fix.timestamp, r->in, r->pos );
        
        r->pos = 0;
    }
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2,
    CMD_RESTART = 3,
    CMD_DOWNLOAD = 4,
    
    CMD_TEST_START = 10,
    CMD_TEST_STOP = 11,    
};


/* this is the state of our connection to the daemon */
typedef struct {
    int                     init;
    int                     fd;
    GpsCallbacks            callbacks;
    pthread_t               thread;
    pthread_t               thread_epo;
    int                     control[2];
    int                     sockfd;
    int                     test_time;
    int                     epoll_hd;
    int                     flag;
    int                     start_flag;
//  int                     thread_exit_flag;
#if NEED_IPC_WITH_CODEC
    int                     sock_codec;
#endif
    int                     epo_data_updated;
    int                     thread_epo_exit_flag;
} GpsState;

static GpsState  _gps_state[1];

static void
gps_state_done( GpsState*  s )
{
    char   cmd = CMD_QUIT;

    write( s->control[0], &cmd, 1 );
	get_condition(&lock_for_sync[M_CLEANUP]);
    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;
    close( s->fd ); s->fd = -1;
	close(s->sockfd); s->sockfd = -1;
	close(s->epoll_hd); s->epoll_hd = -1;
#if NEED_IPC_WITH_CODEC
	close(s->sock_codec);s->sock_codec = -1;
#endif
    s->init = 0;
	s->test_time -= 1;
	return;
}


static void
gps_state_start( GpsState*  s )
{
    char  cmd = CMD_START;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_test_start( GpsState*  s ) 
{
    char cmd = CMD_TEST_START;
    int ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_TEST_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}
static void
gps_state_test_stop( GpsState*  s )
{
    char cmd = CMD_TEST_STOP;
    int ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_TEST_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));    
}

static void
gps_state_stop( GpsState*  s )
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_state_restart( GpsState*  s )
{
    char  cmd = CMD_RESTART;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_RESTART command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

static void
gps_download_epo(GpsState* s)
{
    char cmd = CMD_DOWNLOAD;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        ERR("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}

void
at_command_send_ack(char* ack, int len){
    GpsState*  s = _gps_state;
    
    if (sendto(s->sockfd, ack, len, 0, (struct sockaddr*)&remote, remotelen) < 0){
        DBG("** AT Command send ack to USB failed **");
    }else{
        DBG("** AT Command send ack to USB sucess **");
    }
}

void 
at_command_send_cno(char* cmdline){
    GpsState*  s = _gps_state;
    char* command = cmdline;
    
    if(0 == Avg_CNo){
        DBG("** AT Command, Avg_CNo is invalid **");
        char buff[] = "ERROR";
        at_command_send_ack(buff, sizeof(buff));
    } else {
        DBG("** AT Command, Avg_CNo is valid **");
        char buff[10];
        sprintf(buff,"%d", Avg_CNo/10); // unit of AT%GNSS, AT%GNSS=? is 1dB        
        if(!memcmp(command, GNSS_OP, 7)) {
            DBG("** GNSS test, report Avg_CNo and NA**");
            //need to test
            int size = strlen(buff);
            strcpy(buff+size, " NA");
            DBG("** result = %s**", buff);
    }
        at_command_send_ack(buff, sizeof(buff));
    }
}

int 
at_command_parse_test_num(char* cmdline){
    unsigned long int res;
    char* command = cmdline;
    char** pos = (char**)malloc(sizeof(char)*strlen(command));    
                    
    if(!memcmp(command, GNSS_OP, 7)){
        //AT%GNSS=n
        res = strtoul(command+8, pos, 10);
    } else {
        //AT%GPS=n
        res = strtoul(command+7, pos, 10);
    }
        
    if ((res != 0)&&((**pos)=='\0')){
        DBG("** AT Command Parse: get test_num = %d success!**", res);
        return res;
    } else {
        DBG("** AT Command Parse: the test num may incorrect**");
        return -1;
    }
    
}

void
at_gps_command_parser(char* cmdline)
{
    char* command = cmdline;
    test_mode_flag = 0;
    
    if (!memcmp(command+6, "=", 1)){
        if((!memcmp(command+7, "?", 1))&&(!memcmp(command+8, "\0", 1))){
            //AT%GPS=?
            DBG("** AT Command Parse: AT%%GPS=?**");            
            at_command_send_cno(command);
        } else {
            //AT%GPS=n
            DBG("** AT Command Parse: AT%%GPS=num**");
            int test_num = at_command_parse_test_num(command);
            if (test_num >= 0){
                int ret = mt3326_gps_test_start(test_num, 1, test_num+10);                
                if (0 == ret){
                    DBG("** AT Command gps test start success **");
                    char buff[] = "GPS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    DBG("** AT Command gps test start fail **");
                    char buff[] = "GPS ERROR";
                    at_command_send_ack(buff, sizeof(buff));
                }
                
            }else{
                char buff[] = "GPS ERROR";
                at_command_send_ack(buff, sizeof(buff));                
            }
        }
    }else if (!memcmp(command+6, "?", 1) && (!memcmp(command+7, "\0", 1))){
        //AT%GPS?
        DBG("** AT Command Parse: AT%%GPS? **");        
        int ret = mt3326_gps_test_inprogress();

        if (MNL_AT_TEST_INPROGRESS == ret){
            DBG("** AT Command test is inprogress **");
            char buff[] = "GPS Test In Progress";
            at_command_send_ack(buff, sizeof(buff)); 
        } else if (MNL_AT_TEST_DONE == ret){
            DBG("** AT Command test done");
            char buff[GPS_AT_ACK_SIZE];
            sprintf(buff,"<%d, %d, %d, %d, %d, %d, %d, %d>", 
                result[0], result[1],  result[2],result[3],result[4],result[5],result[6],result[7]);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            DBG("** AT Command test status unknown");
            char buff[] = "ERROR";
            at_command_send_ack(buff, sizeof(buff));
        }
    } else if (!memcmp(command+6, "\0", 1)){
        //AT%GPS
        DBG("** AT Command Parse: AT%%GPS **");        
        at_command_send_cno(command);
    } else {
        DBG("** AT Command Parse: illegal command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

void
at_gnss_command_parser(char* cmdline)
{
    char* command = cmdline;
    test_mode_flag = 0;
    
    if (!memcmp(command+7, "=", 1)){
        if((!memcmp(command+8, "?", 1))&&(!memcmp(command+9, "\0", 1))){
            //AT%GNSS=?
            DBG("** AT Command Parse: AT%%GNSS=?**");            
            at_command_send_cno(command);
        } else {
            //AT%GNSS=n
            DBG("** AT Command Parse: AT%%GNSS=n**");
            int test_num = at_command_parse_test_num(command);
            if (test_num >= 0){
                int ret = mt3326_gps_test_start(test_num, 1, test_num+10);                
                if (0 == ret){
                    DBG("** AT Command gps test start success **");
                    char buff[] = "GNSS TEST START OK";
                    at_command_send_ack(buff, sizeof(buff));
                } else {
                    DBG("** AT Command gps test start fail **");
                    char buff[] = "GNSS ERROR";
                    at_command_send_ack(buff, sizeof(buff));
                }
            }else{
                char buff[] = "GNSS ERROR";
                at_command_send_ack(buff, sizeof(buff));
            }
        }
    }else if (!memcmp(command+7, "?", 1) && (!memcmp(command+8, "\0", 1))){
        //AT%GNSS?
        DBG("** AT Command Parse: AT%%GNSS? **");
        int ret = mt3326_gps_test_inprogress();
        if (MNL_AT_TEST_INPROGRESS == ret){
            DBG("** AT Command test is inprogress **");
            char buff[] = "GNSS Test In Progress";
            at_command_send_ack(buff, sizeof(buff)); 
        } else if (MNL_AT_TEST_DONE == ret){
            DBG("** AT Command test done");
            char buff[GPS_AT_ACK_SIZE];            
            sprintf(buff,"<%d, %d, %d, %d, %d, %d, %d, %d>, [NA]", 
                result[0], result[1],  result[2],result[3],result[4],result[5],result[6],result[7]);
            at_command_send_ack(buff, sizeof(buff));
        } else {
            DBG("** AT Command test status unknown");                     
            char buff[] = "ERROR";
            at_command_send_ack(buff, sizeof(buff));
        }
    }else if (!memcmp(command+7, "\0", 1)){
        //AT%GNSS
        DBG("** AT Command Parse: AT%%GNSS **");        
        at_command_send_cno(command);
    } else {
        DBG("** AT Command Parse: illegal command **");
        char buff[] = "GNSS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}



static void
at_command_parser(char* cmdline)
{   
    char* command = cmdline;
    DBG("** AT Command, receive command %s**", command);
    /* begin to parse the command */
    if (!memcmp(command, GPS_OP, 6)){
        at_gps_command_parser(command);
    }else if(!memcmp(command, GNSS_OP, 7)){
        at_gnss_command_parser(command);
    }else {
        DBG("** AT Command Parse: Not GPS/GNSS AT Command **");
        char buff[] = "GPS ERROR";
        at_command_send_ack(buff, sizeof(buff));
    }
}

static int
epoll_register( int  epoll_fd, int  fd )
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
	if(ret < 0)
		ERR("epoll ctl error, error num is %d\n, message is %s\n", errno, strerror(errno));
    return ret;
}


static int
epoll_deregister( int  epoll_fd, int  fd )
{
    int  ret;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
    } while (ret < 0 && errno == EINTR);
    return ret;
}

/*for reducing the function call to get data from kernel*/
static char buff[1024];
/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
void
gps_state_thread( void*  arg )
{
    GpsState*   state = (GpsState*) arg;
	state->test_time += 1;
//	state->thread_exit_flag=0;
    NmeaReader  reader[1];
#if NEED_IPC_WITH_CODEC	
    char buf_for_codec[1024];
#endif
    int         started    = 0;
    int         gps_fd     = state->fd;
    int         control_fd = state->control[1];
    int         atc_fd = state->sockfd;

	int epoll_fd = state->epoll_hd;
    int         test_started = 0;

    nmea_reader_init( reader );
#if NEED_IPC_WITH_CODEC
	int sock_codec, size_codec;
	struct sockaddr_un un;
	socklen_t client_addr_len;
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, EVDO_CODEC_SOC);
	if ((state->sock_codec = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{	
		ERR("create socket for communicate with codec error, message %s\n", strerror(errno));
		return;
	}	
	unlink(EVDO_CODEC_SOC);
	size_codec = sizeof(un.sun_family)+strlen(un.sun_path);
	if(bind(state->sock_codec, (struct sockaddr *)&un, size_codec) < 0)
	{
		ERR("bind fail, message = %s\n", strerror(errno));
		return;
	}
	if(listen(state->sock_codec, 5) == -1)
	{
		ERR("listern error, message is %s\n", strerror(errno));
		return;	
	}
	DBG("listen done\n");	
	int a = chmod(EVDO_CODEC_SOC, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP);
	DBG("chmod res = %d\n", a);	//770<--mode

	if(chown("/data/misc/codec_sock", -1, AID_INET))
	{
		ERR("chown error: %s", strerror(errno));
	}

	epoll_register(epoll_fd, state->sock_codec);
#endif
    // register control file descriptors for polling
    if(epoll_register( epoll_fd, control_fd ) < 0)
    	ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    if(epoll_register( epoll_fd, gps_fd) < 0)
    	ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));
    if(epoll_register(epoll_fd, atc_fd) < 0)
    	ERR("epoll register control_fd error, error num is %d\n, message is %s\n", errno, strerror(errno));

    DBG("gps thread running: PPID[%d], PID[%d]\n", getppid(), getpid());
	release_condition(&lock_for_sync[M_INIT]);
	DBG("HAL thread is ready, realease lock, and CMD_START can be handled\n");
#if SEM
	sem_t *sem;
	sem = sem_open("/data/misc/read_dev_gps", O_CREAT, S_IRWXU|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH,1);
	if(sem == SEM_FAILED)
	{			
        ERR("init semaphore FAIL, error message is %s \n", strerror(errno));
		return ;
	}
	else
		DBG("create semaphore ok\n");
#endif
    // now loop
    for (;;) {
#if NEED_IPC_WITH_CODEC
        struct epoll_event   events[3];
#else
        struct epoll_event   events[2];
#endif
        int                  ne, nevents;
#if NEED_IPC_WITH_CODEC
        nevents = epoll_wait( epoll_fd, events, 3, -1 );
#else
        nevents = epoll_wait( epoll_fd, events, 2, -1 );
#endif
        if (nevents < 0) {
            if (errno != EINTR)
                ERR("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }
        VER("gps thread received %d events", nevents);
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                ERR("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto Exit;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;

                if (fd == control_fd)
                {
                    char  cmd = 255;
                    int   ret;
                    DBG("gps control fd event");
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        DBG("gps thread quitting on demand");
                        goto Exit;
                    }
                    else if (cmd == CMD_START) {
                        if (!started) {
                            DBG("gps thread starting  location_cb=%p", &callback_backup);
                            started = 1;
                            nmea_reader_set_callback( reader, &state->callbacks);
                        }
                    }
                    else if (cmd == CMD_TEST_START) {                        
                        if ((!test_started)&&(1 == test_mode_flag)) {
                            DBG("**AT Command test_start: test_cb=%p", state->callbacks.test_cb);
                            test_started = 1;
                            nmea_reader_set_callback(reader, &state->callbacks);
                        }                        
                    }
                    else if (cmd == CMD_TEST_STOP) {                        
                        if(test_started) {
                            DBG("**AT Command test_stop");
                            test_started = 0;
                            nmea_reader_set_callback(reader, NULL);
                        }                       
                    }
                    else if (cmd == CMD_STOP) {
                        if (started) {
                            DBG("gps thread stopping");
                            started = 0;
                            nmea_reader_set_callback( reader, NULL );
							release_condition(&lock_for_sync[M_STOP]);
							DBG("CMD_STOP has been receiving from HAL thread, release lock so can handle CLEAN_UP\n");
                        }
                    }
                    else if (cmd == CMD_RESTART) {
                        reader->fix_mode = 0;
                    }
                    else if (cmd == CMD_DOWNLOAD){
                        DBG("Send download request in HAL.");
                        mGpsXtraCallbacks.download_request_cb();
                    }
                }
                else if (fd == gps_fd)
                {
                	if(!flag_unlock)
                	{
                		release_condition(&lock_for_sync[M_START]);
						flag_unlock = 1;							
						DBG("got first NMEA sentence, release lock to set state ENGINE ON, SESSION BEGIN");
					}
                    VER("gps fd event");
                    for (;;) {
                        int  nn, ret;
#if SEM
						if(sem_wait(sem) != 0)
						{
							ERR("sem wait error, message is %s \n", strerror(errno));
							close(fd);
							return ;
						}
						else
							DBG("get semaphore, can read now\n");
#endif
                        ret = read( fd, buff, sizeof(buff) );
#if NEED_IPC_WITH_CODEC
                        memset(buf_for_codec, 0, sizeof(buf_for_codec));
                        memcpy(buf_for_codec, buff, sizeof(buff));
#endif
#if SEM
                        if(sem_post(sem) != 0)
						{
							ERR("sem post error, message is %s\n", strerror(errno));
							close(fd);
							return ;
						}
						else
							DBG("post semaphore, read done\n");
#endif
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ERR("error while reading from gps daemon socket: %s: %p", strerror(errno), buff);
                            break;
                        }
                        DBG("received %d bytes:\n", ret);
                        gps_nmea_end_tag = 0;
                        for (nn = 0; nn < ret; nn++)
                        {
                            if(nn == (ret-1))
                                gps_nmea_end_tag = 1;
                                
                            nmea_reader_addc( reader, buff[nn] );
                        }
                    }
                    VER("gps fd event end");
                }
                else if (fd == atc_fd)
                {
                    char cmd[20];
                    DBG("** AT Command received **");
                    /* receive and parse ATCM here */
                    for (;;) {
                        int  i, ret;
                        ret = recvfrom(fd, cmd, sizeof(cmd), 0, (struct sockaddr *)&remote, &remotelen);                        
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                ERR("error while reading AT Command socket: %s: %p", strerror(errno), cmd);
                            break;
                        }
                        DBG("received %d bytes: %.*s", ret, ret, cmd);
                        cmd[ret] = 0x00;
                        at_command_parser(cmd);             //need redefine
                    }
                    DBG("** AT Command event done **");

                }
#if NEED_IPC_WITH_CODEC
				else if(fd == state->sock_codec)
				{
					client_addr_len = sizeof(un);
					int accept_ret = accept(state->sock_codec, (struct sockaddr*)&un, &client_addr_len);
					if(accept_ret == -1)
					{	
						ERR("accept error, message is %s\n", strerror(errno));
						continue;
					}	
					DBG("accept done\n");
					int cmd, write_len;
					GpsLocation tLocation;

					if(recv(accept_ret, &cmd, sizeof(cmd),0)<0)
					{
						ERR("read from codec error, message = %s\n", strerror(errno));
						continue;
					}
					DBG("read done, cmd: %d\n", cmd);
					switch(cmd)
					{
					case 1://need NMEA sentences
						write_len = send(accept_ret, buf_for_codec, sizeof(buff),0);
						DBG("write %d bytes to codec\n", write_len);
						break;
					case 2: //For AGPS location froward
						DBG("Snd to UI");
						char ack_buf[10] = {0};
						strcpy(ack_buf, "cmd2_ack");
						write_len = send(accept_ret, ack_buf, sizeof(ack_buf), 0);
						DBG("wait rcv location data");
						if(recv(accept_ret, &tLocation, sizeof(tLocation), 0) < 0)
						{
							ERR("read from codec error, message = %s\n", strerror(errno));		
						}else{
							if(callback_backup.location_cb){
								DBG("Update location data to UI");
								callback_backup.location_cb(&tLocation);
							}else{
								DBG("Location CB is null");
							}
						}
						break;
					default:
						ERR("unknonwn codec message, codec send %d to me\n", cmd);
						break;	
					}
					close(accept_ret);
				}
#endif
                else
                {
                    ERR("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }
    }
Exit:
	DBG("HAL thread is exiting, release lock to clean resources\n");
	release_condition(&lock_for_sync[M_CLEANUP]);
    return;
}


static void
gps_state_init( GpsState*  state )
{
    state->control[0] = -1;
    state->control[1] = -1;
    state->fd         = -1;

    state->fd = open( GPS_CHANNEL_NAME, O_RDONLY); //support poll behavior
	int epoll_fd   = epoll_create(2);

	state->epoll_hd = epoll_fd;
    if (state->fd < 0) {
        ERR("no gps hardware detected: %s:%d, %s", GPS_CHANNEL_NAME, state->fd, strerror(errno));
        return;
    }

    if ( mt3326_init() != 0 ) {
        ERR("could not initiaize mt3326 !!");
        goto Fail;
    }
    
    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        ERR("could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }

    /* Create socket with generic service for AT Command */    
    if ((state->sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        ERR("gps_state_init: hal2usb socket create failed\n");
        goto Fail;      
    }

    unlink(GPS_AT_COMMAND_SOCK);
    memset(&cmd_local, 0, sizeof(cmd_local));
    cmd_local.sun_family = AF_LOCAL;
    strcpy(cmd_local.sun_path, GPS_AT_COMMAND_SOCK);

    if (bind(state->sockfd, (struct sockaddr *)&cmd_local, sizeof(cmd_local)) < 0 )
    {           
        ERR("gps_state_init: hal2usb socket bind failed\n");
        state->sockfd = -1;       
        goto Fail;
    }

    
    state->thread = callback_backup.create_thread_cb(gps_native_thread, gps_state_thread, state);
    if (!state->thread){
        ERR("could not create gps thread: %s", strerror(errno));
        goto Fail;
    }

    DBG("gps state initialized, the thread is %d\n", state->thread);
    return;

Fail:
    gps_state_done( state );
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/


static int
mt3326_gps_init(GpsCallbacks* callbacks)
{
    GpsState*  s = _gps_state;
	if(s->init)
		return 0;
    s->callbacks = *callbacks;
    callback_backup = *callbacks;

    gps_state_init(s);
	get_condition(&lock_for_sync[M_INIT]);
	usleep(1000*1);
	s->init = 1;
    return 0;
}

static void
mt3326_gps_cleanup(void)
{
    GpsState*  s = _gps_state;

    TRC();
    
    if (mt3326_cleanup() != 0) 
          ERR("mt3326 cleanup error!!");
    if(s->start_flag)
    get_condition(&lock_for_sync[M_STOP]);	//make sure gps_stop has set state to GPS_STATUS_ENGINE_OFF by callback function
    if (s->init)
        gps_state_done(s);
    
    s->thread_epo_exit_flag = 1;   
    get_condition(&lock_for_sync[M_THREAD_EXIT]);
    
    DBG("mt3326_gps_cleanup done");
    return NULL;
}

int
mtk_epo_is_expired(){
    long long uTime[3];
	memset(uTime, 0, sizeof(uTime));
    time_t time_st;	    	
    
    time(&time_st);
    mtk_gps_epo_file_time_hal(uTime);

    DBG("current time: %ld, current time:%s", time_st, ctime(&time_st));
    DBG("EPO start time: %lld, EPO start time: %s", uTime[0], ctime(&uTime[0]));
	
    if ((time_st - uTime[0]) >= MTK_EPO_EXPIRED){
        DBG("EPO file is expired");
        return 1;
	}else{
        DBG("EPO file is valid, no need update");
        return 0;
    }
}

int
mt3326_gps_start()
{
    GpsState*  s = _gps_state;
    int err;
    
    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }
    if ((err = mt3326_start())) {
        ERR("mt3326_start err = %d", err);
        return -1;
    }
    if(access(EPO_FILE_HAL, 0) == -1)
    {
        DBG("no EPOHAL file, the EPO.DAT is not exsited, or is the latest one\n");
        //check if EPO.DAT existed
        if(access(EPO_FILE, 0) == -1)
        {
            //request download
            DBG("Both EPOHAL.DAT and EPO.DAT are not existed, download request 1");
            gps_download_epo(s);
        }
        else
        {
            //check if EPO.DAT is expired
            if(mtk_epo_is_expired())
            {
                DBG("EPOHAL.DAT is not existed and EPO.DAT expired, download request 2");
                gps_download_epo(s);
            }
        }
    }
    else
    {
        //to check if EPOHAL.DAT is expired.
        if (mtk_epo_is_expired())
        {
            DBG("EPOHAL is expired, download request 3");
            gps_download_epo(s);
        }
        else
        {
            DBG("EPOHAL is existed and no expired, tell agent to update");
            char buf[] = {MNL_CMD_UPDATE_EPO_FILE};
            char cmd = HAL_CMD_STOP_UNKNOWN;
            err = mt3326_daemon_send(buf, sizeof(buf));
            if(-1 == err)
            {
                ERR("Request update epo file fail\n");
            }
            else
            {
                DBG("Request update epo file successfully\n");
                err = read(mt3326_gps.sock, &cmd, sizeof(cmd));
                if(cmd == HAL_CMD_UPDATE_EPO_FILE_DONE)
                {
                    DBG("Update EPO file successfully\n");
                    unlink(EPO_FILE_HAL);	
                }
                else if (cmd == HAL_CMD_UPDATE_EPO_FILE_FAIL)
                {
                    ERR("Update EPO file fail\n");		
                }     	 	
            }
        }
    }
    get_condition(&lock_for_sync[M_START]);
    DBG("HAL thread has initialiazed\n");
    gps_state_start(s);
                                                                                   
    sta.status = GPS_STATUS_ENGINE_ON;
    DBG("sta.status = GPS_STATUS_ENGINE_ON\n");
    callback_backup.status_cb(&sta);
    sta.status = GPS_STATUS_SESSION_BEGIN;
    DBG("sta.status = GPS_STATUS_SESSION_BEGIN\n");
    callback_backup.status_cb(&sta);
    
    s->start_flag = 1;
    return 0;
}

/*for GPS AT command test*/
int mt3326_gps_test_start(int test_num, int prn_num, int time_delay){

    GpsState*  s = _gps_state;
    int err;   
    
    hal_test_data.test_num = test_num;
    hal_test_data.prn_num = prn_num;
    hal_test_data.time_delay = time_delay;    
    time(&start_time);

    Avg_CNo = 0; // ithis code is moved from stop function to here to keep avg value for AT%GPS(GNSS) or AT%GPS=?(GNSS=?)
    Dev_CNr = (int*)malloc(sizeof(int)*hal_test_data.test_num);
    memset(Dev_CNr, 0, test_num*sizeof(int));
    
    if(0 == hal_test_data.test_num) {
        ERR("%s: test number is 0!!", __FUNCTION__);
        return -1;
    }

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    MNL_AT_TEST_STATE = MNL_AT_TEST_INPROGRESS;
    MNL_AT_TEST_FLAG = 1;

    if ((err = mt3326_start())) {
        ERR("mt3326_start err = %d", err);
        MNL_AT_TEST_STATE = MNL_AT_TEST_UNKNOWN;
        MNL_AT_TEST_FLAG = 0;
        return -1;
    }

    TRC();
    gps_state_test_start(s);
    return 0;       
}

int
mt3326_gps_stop()
{
    GpsState*  s = _gps_state;
    int err;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }
    if ((err = mt3326_stop())) {
        ERR("mt3326_stop err = %d", err);
        return -1;
    }

    TRC();
    gps_state_stop(s);
	char cmd;
	int ret;
	ret = read(mt3326_gps.sock, &cmd, sizeof(cmd));
	if(cmd == HAL_CMD_MNL_DIE)
     	DBG("mnl die\n"); 
	
	flag_unlock = 0;	
    DBG("GPS normal mode, unregister callbacks!!");
       
    //GpsStatus sta;                                                                                                   //Modify to global
    sta.status = GPS_STATUS_SESSION_END;
    callback_backup.status_cb(&sta);
    DBG("sta.status = GPS_STATUS_SESSION_END\n");
    sta.status = GPS_STATUS_ENGINE_OFF;
    DBG("sta.status = GPS_STATUS_ENGINE_OFF\n");
    callback_backup.status_cb(&sta);   	
	
	s->start_flag = 0;
    return 0;
}


int
mt3326_gps_test_stop() 
{
    GpsState*  s = _gps_state;
    int err;
    test_mode_flag = 1;

    if (!s->init) {
        ERR("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }
    if ((err = mt3326_stop())) {
        ERR("mt3326_stop err = %d", err);
        return -1;
    }

    TRC();
    gps_state_test_stop(s);
    
    hal_test_data.test_num = 0;
    hal_test_data.prn_num = 0;
    hal_test_data.time_delay = 0;
    MNL_AT_TEST_FLAG = 0;

    //release the variable
    Success_Num = 0;
    Completed_Num = 0;
    CNo = 0;
    DCNo = 0;
    //Avg_CNo = 0; // initialization of this variable will be set on start function due to keep this value for AT%GPS(GNSS) or AT%GPS=?(GNSS=?)
    Dev_CNo = 0;
    Err_Code = 1;
    test_num = 0;
    
    free(Dev_CNr);  
    return 0;
}

int 
mt3326_gps_test_inprogress()
{    
    int ret = -1;

    if (MNL_AT_TEST_STATE == MNL_AT_TEST_DONE){
        DBG("**AT Command test done!!");
        ret = MNL_AT_TEST_DONE;        
    }else if (MNL_AT_TEST_STATE == MNL_AT_TEST_INPROGRESS){        
        DBG("**AT Command test is in progress!!");
        ret = MNL_AT_TEST_INPROGRESS;        
    } else {
        DBG("**AT Command test status unknown!!");
        ret = MNL_AT_TEST_UNKNOWN;        
    }        
    return ret;
}

static int
mt3326_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    TRC();
    return 0;
}

static int
mt3326_gps_inject_location(double latitude, double longitude, float accuracy)
{
    return 0;
}

static void
mt3326_gps_delete_aiding_data(GpsAidingData flags)
{    
    /*workaround to trigger hot/warm/cold/full start*/
    #define FLAG_HOT_START  GPS_DELETE_RTI
    #define FLAG_WARM_START GPS_DELETE_EPHEMERIS     
    #define FLAG_COLD_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC | GPS_DELETE_HEALTH)
    #define FLAG_FULL_START (GPS_DELETE_ALL)
    #define FLAG_AGPS_START (GPS_DELETE_EPHEMERIS | GPS_DELETE_ALMANAC | GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO | GPS_DELETE_UTC)
    GpsState*  s = _gps_state;

    DBG("%s:0x%X\n", __FUNCTION__, flags);

    gps_state_restart(s);

    if (flags == FLAG_HOT_START)
    {
        DBG("Send MNL_CMD_RESTART_HOT in HAL\n");
        mt3326_restart(MNL_CMD_RESTART_HOT);
    }
    else if (flags == FLAG_WARM_START)
    {
        DBG("Send MNL_CMD_RESTART_WARM in HAL\n");
        mt3326_restart(MNL_CMD_RESTART_WARM);
    }
    else if (flags == FLAG_COLD_START)
    {
        DBG("Send MNL_CMD_RESTART_AGPS/COLD in HAL\n");
        mt3326_restart(MNL_CMD_RESTART_COLD);
    }else if (flags == FLAG_FULL_START)
    {
        DBG("Send MNL_CMD_RESTART_FULL in HAL\n");
        mt3326_restart(MNL_CMD_RESTART_FULL);
    }
    else if(flags == FLAG_AGPS_START)
    {
        DBG("Send MNL_CMD_RESTART_AGPS in HAL\n");	
        mt3326_restart(MNL_CMD_RESTART_AGPS);
    }
}

static int mt3326_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
            uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
    // FIXME - support fix_frequency
    // only standalone supported for now.
    TRC();
    return 0;
}
void thread_epo_file_update(void* arg){
	
    GpsState* s = (GpsState *)arg;    	
    DBG("EPO thread start");    	
    while(1){
        usleep(100000);        
        if (s->thread_epo_exit_flag == 1){
            DBG("EPO thread exit\n");
            break;
        }
		
        if(s->epo_data_updated == 1){
            DBG("Write EPO data to file now");
            s->epo_data_updated = 0;
            mtk_gps_epo_file_update();
        }		
    }
    //pthread_exit(NULL);    
    release_condition(&lock_for_sync[M_THREAD_EXIT]);
    DBG("EPO thread exit done");    
    return NULL;
}


//zqh: download EPO by request
int mtk_gps_epo_interface_init (GpsXtraCallbacks* callbacks){
    TRC();
    int ret = -1;
    GpsState*  s = _gps_state;
    if(s->init){
        mGpsXtraCallbacks = *callbacks;
		ret = 0;
    }

    //start thread to write data to file
    ret = pthread_create(&s->thread_epo, NULL, thread_epo_file_update, s);
    if(0 != ret){
        ERR("EPO thread create fail: %s\n", strerror(errno));    
        return ret;
    }
    s->thread_epo_exit_flag = 0;    
    DBG("mtk_gps_epo_interface_init done");
    return ret;
}


int mtk_gps_inject_epo_data ( char* data, int length ){

    GpsState* s = _gps_state;		
    if(length <= 0){
        ERR("EPO data lengh error!!");
        return -1;
    }
    
    epo_data.length = length;
    epo_data.data = data;
	
    s->epo_data_updated = 1;
    DBG("length = %d, epo_data.length = %d", length, epo_data.length);
    
    DBG("EPO download done, epo_data_updated = %d\n", s->epo_data_updated);
    return 0;
}

static const GpsXtraInterface mtkGpsXtraInterface = {
    sizeof(GpsXtraInterface),
    mtk_gps_epo_interface_init,
    mtk_gps_inject_epo_data,
};
//zqh: download EPO by request end


static const void*
mt3326_gps_get_extension(const char* name)
{
    TRC();
    //zqh: for EPO file download by request
    if (!strcmp(name, GPS_XTRA_INTERFACE))
        return (void*)(&mtkGpsXtraInterface);
    return NULL;
}

int
mtk_gps_sys_read_lock(int fd, off_t offset, int whence, off_t len){   

    struct flock lock;
    	
    lock.l_type = F_RDLCK;                                                       
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
	
    if (fcntl(fd, F_SETLK, &lock) < 0 ){
		return -1;
    }
	
    return 0;	
}

int
mtk_gps_sys_write_lock(int fd, off_t offset, int whence, off_t len){
	struct flock lock;
	
	lock.l_type = F_WRLCK;
	lock.l_start = offset;
	lock.l_whence = whence;
	lock.l_len = len;
	if (fcntl(fd, F_SETLK, &lock) < 0){
		return -1;
	}
	
	return 0;
}

static unsigned int
mtk_gps_sys_get_file_size() {
    unsigned int fileSize;
	int res_epo, res_epo_hal;
    struct stat st;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_FILE_HAL;
    char epofile[32] = {0};
	res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_FILE_HAL, F_OK);
	if(res_epo < 0 && res_epo_hal < 0)
	{
		DBG("no EPO data yet\n");
		return -1;	
	}
	if(res_epo == 0) /*EPO.DAT is here*/
	{
		DBG("find EPO.DAT here\n");
		strcpy(epofile, epo_file);
	}
	else if(res_epo_hal == 0) /*EPOHAL.DAT is here*/
	{
		DBG("find EPOHAL.DAT here\n");
		strcpy(epofile, epo_file_hal);
	}
	else
		ERR("unknown error happened\n");
    if(stat(epofile, &st) < 0) {
		ERR("Get file size error, return\n");
		return 0;
    	}
	
    fileSize = st.st_size;	
    DBG("EPO file size: %d\n", fileSize);
    return fileSize;
       	
    //fseek(pFile, 0L, SEEK_END); //reset the current pointer to the end of file
    //fileSize = ftell(pFile); 
    //fseek(pFile, 0, SEEK_SET);

    //return fileSize;	
}
void GpsToUtcTime(int i2Wn, double dfTow, time_t* uSecond)
{
    struct tm target_time;
    int iYearsElapsed;     // Years since 1980.
    unsigned int iDaysElapsed;      // Days elapsed since Jan 1, 1980.
    double dfSecElapsed;
    unsigned int fgLeapYear; 
    int pi2Yr;
    int pi2Mo;
    int pi2Day;
    int pi2Hr;
    int pi2Min;
    double pdfSec; 	
    int i;  
  

  // Number of days into the year at the start of each month (ignoring leap
  // years).
  unsigned int doy[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

  // Convert time to GPS weeks and seconds
  iDaysElapsed = i2Wn * 7 + ((int)dfTow / 86400) + 5;
  dfSecElapsed = dfTow - ((int)dfTow / 86400) * 86400;


  // decide year
  iYearsElapsed = 0;    // from 1980
  while(iDaysElapsed >= 365)
  {
    if((iYearsElapsed % 100) == 20) // if year % 100 == 0
    {
      if((iYearsElapsed % 400) == 20) // if year % 400 == 0
      {
        if (iDaysElapsed >= 366)
        {
          iDaysElapsed -= 366;
        }
        else
        {
          break;
        }
      }
      else
      {
        iDaysElapsed -= 365;
      }
    }
    else if((iYearsElapsed % 4) == 0) // if year % 4 == 0
    {
        if (iDaysElapsed >= 366)
        {
          iDaysElapsed -= 366;
        }
        else
        {
          break;
        }
    }
    else
    {
        iDaysElapsed -= 365;
    }
    iYearsElapsed++;
  }
  pi2Yr = 1980 + iYearsElapsed;


  // decide month, day
  fgLeapYear = 0;
  if((iYearsElapsed % 100) == 20) // if year % 100 == 0
  {
    if((iYearsElapsed % 400) == 20) // if year % 400 == 0
    {
      fgLeapYear = 1;
    }
  }
  else if((iYearsElapsed % 4) == 0) // if year % 4 == 0
  {
    fgLeapYear = 1;
  }

  if(fgLeapYear)
  {
    for(i = 2; i < 12; i++)
    {
      doy[i] += 1;
    }
  }
  for(i = 0; i < 12; i++)
  {
    if(iDaysElapsed < doy[i])
    {
      break;
    }
  }
  pi2Mo = i;
  pi2Day =iDaysElapsed - doy[i-1] + 1;


  // decide hour, min, sec
  pi2Hr = dfSecElapsed / 3600;
  pi2Min = ((int)dfSecElapsed % 3600) / 60;
  pdfSec = dfSecElapsed - ((int)dfSecElapsed / 60) * 60;

    //change the UTC time to seconds
    memset(&target_time, 0, sizeof(target_time));
    target_time.tm_year = pi2Yr - 1900;
    target_time.tm_mon = pi2Mo - 1;
    target_time.tm_mday = pi2Day;
    target_time.tm_hour = pi2Hr;
    target_time.tm_min = pi2Min;
    target_time.tm_sec = pdfSec;
    target_time.tm_isdst = -1;
    DBG("target_time.tm_year = %d, month = %d, day = %d, hour = %d, min = %d, sec = %d, tm_isdst = %d\n", 
        target_time.tm_year, target_time.tm_mon, target_time.tm_mday, target_time.tm_hour, target_time.tm_min, target_time.tm_sec, target_time.tm_isdst);
    *uSecond = mktime(&target_time);
    if (*uSecond < 0){
		ERR("Convert UTC time to seconds fail, return\n");
    }
    
}


static int
mtk_gps_sys_epo_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond){      //no file lock                  
    char szBuf[MTK_EPO_ONE_SV_SIZE];    
    int pi2WeekNo; 
    unsigned int pu4Tow;
    	
	
    //if (fread(szBuf, 1, MTK_EPO_ONE_SV_SIZE, pFile) != MTK_EPO_ONE_SV_SIZE) {
    if(read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE){
        return -1;
    }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);                                         
    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;
    
    TRC();
    DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);//to get UTC second	
    return 0;	
}


static int
mtk_gps_sys_epo_period_end(int fd, unsigned int *u4GpsSecs, time_t* uSecond) {        //no file lock   
    int fileSize;
    char szBuf[MTK_EPO_ONE_SV_SIZE]; 
    int pi2WeekNo; 
    unsigned int pu4Tow;	

    fileSize = mtk_gps_sys_get_file_size();
    if(fileSize < MTK_EPO_ONE_SV_SIZE) {
		return -1;
    }

    lseek(fd, (fileSize - MTK_EPO_ONE_SV_SIZE), SEEK_SET);

    if(read(fd, szBuf, MTK_EPO_ONE_SV_SIZE) != MTK_EPO_ONE_SV_SIZE) {
   	return -1;
   }

    *u4GpsSecs = (((*(unsigned int*)(&szBuf[0])) & 0x00FFFFFF) *3600);                                     
    (*u4GpsSecs) += 21600;

    pi2WeekNo = (*u4GpsSecs) / 604800;
    pu4Tow = (*u4GpsSecs) % 604800;
    
    TRC();
    DBG("pi2WeekNo = %d, pu4Tow = %d\n", pi2WeekNo, pu4Tow);
    GpsToUtcTime(pi2WeekNo, pu4Tow, uSecond);		

    return 0;	 
}

int
mtk_gps_epo_file_time_hal(long long uTime[]) {
    
    TRC();
    struct stat filestat;
    int fd = 0;
    int addLock, res_epo,res_epo_hal;	
    unsigned int u4GpsSecs_start; //GPS seconds
    unsigned int u4GpsSecs_expire;
    char *epo_file = EPO_FILE;
    char *epo_file_hal = EPO_FILE_HAL;
    char epofile[32] = {0};
    time_t uSecond_start;   //UTC seconds
    time_t uSecond_expire;	
    res_epo = access(EPO_FILE, F_OK);
    res_epo_hal = access(EPO_FILE_HAL, F_OK);
	if(res_epo < 0 && res_epo_hal < 0)
	{
		DBG("no EPO data yet\n");
		return -1;	
	}
	if( res_epo_hal== 0) /*EPOHAL.DAT is here*/
	{
		DBG("find EPOHAL.DAT here\n");
		strcpy(epofile, epo_file_hal);
	}
	else if(res_epo == 0) /*EPO.DAT is here*/
	{
		DBG("find EPO.DAT here\n");
		strcpy(epofile, epo_file);
	}
	else
		ERR("unknown error happened\n");

    //open file
    fd = open(epofile, O_RDONLY);
    if(-1 == fd) {
		ERR("Open EPO fail, return\n");
		return -1;
    	}

   //Add file lock 
    if(mtk_gps_sys_read_lock(fd, 0, SEEK_SET, 0) < 0){
   	ERR("Add read lock failed, return\n");
       close(fd);
	return -1;
   }
       
    //EPO start time
    if(mtk_gps_sys_epo_period_start(fd, &u4GpsSecs_start, &uSecond_start)) {
        ERR("Get EPO file start time error, return\n");
        close(fd);
        return -1;                                                                                  
    }else{
        uTime[0] = (long long)uSecond_start;   	    
        DBG("The Start time of EPO file is %lld", uTime[0]);
        DBG("The start time of EPO file is %s", ctime(&uTime[0]));
    }
			   
    //download time	
    stat(epofile, &filestat);
    uTime[1] = (long long)(filestat.st_mtime);
    //uTime[1] = uTime[1] - 8 * 3600;    
    DBG("Download time of EPO file is %lld", uTime[1]);
    DBG("Download time of EPO file is %s\n", ctime(&uTime[1]));		
    
    //EPO file expire time    
    if(mtk_gps_sys_epo_period_end(fd, &u4GpsSecs_expire, &uSecond_expire)){
        ERR("Get EPO file expire time error, return\n");
        close(fd);	
        return -1;
    }else {
        uTime[2] = (long long)uSecond_expire;
        DBG("The expire time of EPO file is %lld", uTime[2]);
        DBG("The expire time of EPO file is %s", ctime(&uTime[2]));
    }

    close(fd);
    return 0;
}


int
mtk_gps_epo_file_update_hal(){
    TRC();
    int fd_write, res;    
    ssize_t bytes_read;
    ssize_t bytes_write;
    char* data = epo_data.data;
    int length = epo_data.length;
    
    int result;
    DBG("length = %d", length);
    DBG("Update EPO HAL file...\n");
  
    fd_write = open(EPO_FILE_HAL, O_WRONLY | O_TRUNC | O_CREAT | O_NONBLOCK | O_APPEND, 0641);
    if(fd_write < 0){
        ERR("open /data/misc/EPOHAL.DAT error!\n");		
	return -1;
    }
 
    if(mtk_gps_sys_write_lock(fd_write, 0, SEEK_SET, 0) < 0){
        ERR("Add read lock failed, return\n");
        close(fd_write);	   
        return -1;
    }

    //start copy new data from framework to EPOHAL.DAT	
    while(length > 0){                                              
        bytes_write = write(fd_write,data,length);		
        if(bytes_write < 0){
            ERR("Write EPOHAL.DAT error: %s\n", strerror(errno));
            length = 0;			
            data = NULL;
            close(fd_write);
            return -1;
        }

        DBG("bytes_write = %d\n", bytes_write);
        DBG("EPO data: %s", data);
		    
        if(bytes_write <= length) {                        
            data = data + length; 		
            length = length - bytes_write;
            DBG("Remain length: %d\n", length); 
        }
    }

    data = NULL;
    length = 0;

    //release write lock
    close(fd_write);	
    if ((res = chmod(EPO_FILE_HAL, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0)
        DBG("chmod res = %d, %s", res, strerror(errno));
    DBG("Update EPO HAL file done\n");
    return 0;
 }


int mtk_gps_epo_file_time(long long uTime[])
{
    TRC();
    char cmd = HAL_CMD_STOP_UNKNOWN;
    int ret;
    struct stat filestat; 
    DBG("sta.status = %d\n", sta.status);	
    //if(sta.status == GPS_STATUS_ENGINE_ON || sta.status == GPS_STATUS_SESSION_BEGIN) {    
    if(started){
	 DBG("GPS driver is running, read epo time via GPS driver\n");         
        //send cmd to MNLD        
        char buf[] = {MNL_CMD_READ_EPO_TIME};   
        ret = mt3326_daemon_send(buf, sizeof(buf));   
        if (-1 == ret){
            ERR("Request read epo time fail\n");
	     return ret;		
        }else{
	     //send cmd success, wait to read from socket
	     DBG("Request read epo time successfully\n");
	     //usleep(100000);                                                          
	     
            ret = read(mt3326_gps.sock, &cmd, sizeof(cmd));
            if(cmd == HAL_CMD_READ_EPO_TIME_DONE){
                DBG("HAL_CMD_READ_EPO_TIME_DONE\n");
	         time_t msg[2];

	         do { 
	             ret = read(mt3326_gps.sock, msg, sizeof(msg));
                    DBG("Read EPO time from mt3326_gps.sock, ret = %d\n", ret);
	         } while (ret < 0 && errno == EINTR);

	         DBG("msg[0] = %ld, msg[1] = %ld", msg[0], msg[1]);
            
	         if(ret == 0) {
                    ERR("Remote socket closed\n");
	             return -1;	   
	         }
            
	         if(ret == sizeof(msg)) {		
                    //start time
	             uTime[0] = msg[0];
	             DBG("Start time of EPO file is %lld", uTime[0]);
	             DBG("Start time of EPO file is %s\n", ctime(&uTime[0]));
		  
                    //download time
                    stat(EPO_FILE, &filestat);
                    uTime[1] = (long long)(filestat.st_mtime);
                    //uTime[1] = uTime[1] - 8 * 3600;    
                    DBG("Download time of EPO file is %lld", uTime[1]);
                    DBG("Download time of EPO file is %s\n", ctime(&uTime[1]));	

	             //expire time	
	             uTime[2] = msg[1];
	             DBG("Download time of EPO file is %lld", uTime[2]);
                    DBG("Download time of EPO file is %s\n", ctime(&uTime[2]));
	             return 0;
	         } else {
                    ERR("Read time fail\n");
	             return -1;		   
	         }
          	  //return 0;		
             }else if(cmd == HAL_CMD_READ_EPO_TIME_FAIL){
                ERR("Read EPO time fail\n");
	         return -1;			 
	     }	
          }        
      }else{
        //GPS driver is not running, read epo file time in HAL.
        DBG("GPS driver is not running, read epo file time in HAL\n");
        ret = mtk_gps_epo_file_time_hal(uTime);
    }
    return ret;	
}
	
int mtk_gps_epo_file_update(){
    int ret;
    int res;
    DBG("sta.status = %d\n", sta.status);	
    
    ret = mtk_gps_epo_file_update_hal();
    if(ret < 0)
    {
        ERR("Update EPOHAL.DAT error\n");
    	return -1;
    }
    //if(sta.status == GPS_STATUS_ENGINE_ON || sta.status == GPS_STATUS_SESSION_BEGIN) {
    if(started){
        //send cmd to MNLD        
        char cmd = HAL_CMD_STOP_UNKNOWN;
 
        DBG("GPS driver is running, update epo file via GPS driver\n");
	char buf[] = {MNL_CMD_UPDATE_EPO_FILE};
	ret = mt3326_daemon_send(buf, sizeof(buf));
	      
        if(-1 == ret){
            ERR("Request update epo file fail\n");
	 }else{
	     //send cmd success, wait to read from socket
            DBG("Request update epo file successfully\n");
            //usleep(100000);                                                         
			
	     ret = read(mt3326_gps.sock, &cmd, sizeof(cmd));
	     if(cmd == HAL_CMD_UPDATE_EPO_FILE_DONE){
                DBG("Update EPO file successfully\n");
                unlink(EPO_FILE_HAL);
		  return 0;		
            }else if (cmd == HAL_CMD_UPDATE_EPO_FILE_FAIL){
                ERR("Update EPO file fail\n");
	         return -1;			
	     }     	 	
	 }	 	
    }
    return ret;
}

static const GpsInterface  mt3326GpsInterface = {
    sizeof(GpsInterface),
    mt3326_gps_init,
    mt3326_gps_start,
    mt3326_gps_stop,
    mt3326_gps_cleanup,
    mt3326_gps_inject_time,
    mt3326_gps_inject_location,
    mt3326_gps_delete_aiding_data,
    mt3326_gps_set_position_mode,
    mt3326_gps_get_extension,
    //mtk_gps_epo_file_time,
    //mtk_gps_epo_file_update,    
    mt3326_gps_test_start,
    mt3326_gps_test_stop,
    mt3326_gps_test_inprogress,
};

const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
	  DBG("gps__get_gps_interface HAL\n");
    return &mt3326GpsInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
	  DBG("open_gps HAL 1\n");
    struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
//    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    DBG("open_gps HAL 2\n");
    dev->get_gps_interface = gps__get_gps_interface;
    DBG("open_gps HAL 3\n");
    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};


struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "Hardware GPS Module",
    .author = "The MTK GPS Source Project",
    .methods = &gps_module_methods,
};

/*
const GpsInterface* gps_get_hardware_interface()
{
    TRC();	
    return &mt3326GpsInterface;
}
*/

