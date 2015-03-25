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
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cutils/properties.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

//for read NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
//#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"


#ifdef FEATURE_FTM_GPS

#ifdef GPS_PROPERTY
#undef GPS_PROPERTY
#endif
#define GPS_PROPERTY "/data/misc/GPS_CHIP_FTM.cfg"
static 	ap_nvram_gps_config_struct stGPSReadback;


#define MNL_ATTR_PWRCTL  "/sys/class/gpsdrv/gps/pwrctl"
#define MNL_ATTR_SUSPEND "/sys/class/gpsdrv/gps/suspend"
#define MNL_ATTR_STATE   "/sys/class/gpsdrv/gps/state"
#define MNL_ATTR_PWRSAVE "/sys/class/gpsdrv/gps/pwrsave"
#define MNL_ATTR_STATUS  "/sys/class/gpsdrv/gps/status"

enum {
    GPS_PWRCTL_UNSUPPORTED  = 0xFF,
    GPS_PWRCTL_OFF          = 0x00,
    GPS_PWRCTL_ON           = 0x01,
    GPS_PWRCTL_RST          = 0x02,
    GPS_PWRCTL_OFF_FORCE    = 0x03,
    GPS_PWRCTL_RST_FORCE    = 0x04,
    GPS_PWRCTL_MAX          = 0x05,
};

#define TAG             "[GPS]   "
#define INFO_SIZE       1024
#define NMEA_SIZE       10240

unsigned char nmea_buf[NMEA_SIZE];

#define NUM_CH  (20)
#define PSEUDO_CH (32)
#define Knot2Kmhr (1.8532)
static int timeout = 60;
static init_flag = 0;
typedef struct SVInfo
{
    int SVid;            // PRN
    int SNR;
    int elv;             // elevation angle : 0~90
    int azimuth;         // azimuth : 0~360
    unsigned char Fix;   // 0:None , 1:FixSV
} SVInfo;

typedef struct ChInfo
{
    int SVid;            // PRN
    int SNR;             // SNR
    unsigned char Status;// Status(0:Idle, 1:Search, 2:Tracking)
} ChInfo;

typedef struct GPSInfo
{
    int year;
    int mon;
    int day;
    int hour;
    int min;
    float sec;

    float Lat; // Position, +:E,N -:W,S
    float Lon;
    float Alt;
    unsigned char FixService;  // NoFix:0, SPS:1, DGPS:2, Estimate:6
    unsigned char FixType;     // None:0, 2D:1, 3D:2
    float Speed;  // km/hr
    float Track;  // 0~360
    float PDOP;   //DOP
    float HDOP;
    float VDOP;

    int SV_cnt;
    int fixSV[NUM_CH];
}GPSInfo;
extern char *ftm_get_prop(const char *name);
GPSInfo g_gpsInfo;
SVInfo  g_svInfo[NUM_CH];
ChInfo  g_chInfo[PSEUDO_CH];

int ttff = 0;
int fixed = 0;
int httff = 0;
int cttff = 0;

int ttff_check_res = 0;
enum {
    ITEM_PASS,
    ITEM_FAIL,
    ITEM_HTTFF,
    ITEM_CTTFF
};

static item_t gps_items[] = {
    //item(ITEM_PASS,   uistr_pass),
    //item(ITEM_FAIL,   uistr_fail),
    //item(ITEM_HTTFF,  uistr_info_gps_hot_restart),
    //item(ITEM_CTTFF,  uistr_info_gps_cold_restart),
    item(-1, NULL),
};

struct gps_desc {
    char         info[INFO_SIZE];
    char        *mntpnt;
    bool         exit_thd;

    text_t title;
    text_t text;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define mod_to_gps(p)  (struct gps_desc*)((char*)(p) + sizeof(struct ftm_module))

#define C_INVALID_PID  (-1)   /*invalid process id*/
#define C_INVALID_TID  (-1)   /*invalid thread id*/
#define C_INVALID_FD   (-1)   /*invalid file handle*/
#define C_INVALID_SOCKET (-1) /*invalid socket id*/

#define MND_ERR(fmt, arg ...) LOGD(TAG"%s: " fmt, __FUNCTION__ ,##arg)
#define MND_MSG(fmt, arg ...) LOGD(TAG"%s: " fmt, __FUNCTION__ ,##arg)

pid_t mnl_pid = C_INVALID_PID;
int sockfd = C_INVALID_SOCKET;
pthread_t gps_meta_thread_handle = C_INVALID_TID;

static int mnl_write_attr(const char *name, unsigned char attr) 
{
    int err, fd = open(name, O_RDWR);
    char buf[] = {attr + '0'};
    
    if (fd == -1) {
        LOGD(TAG"open %s err = %s\n", name, strerror(errno));
        return -errno;
    }
    do { err = write(fd, buf, sizeof(buf) ); }
    while (err < 0 && errno == EINTR);
    
    if (err != sizeof(buf)) { 
        LOGD(TAG"write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        LOGD(TAG"close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    LOGD(TAG"write '%d' to %s okay\n", attr, name);
    return err;
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
		usleep(500*1000);
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
	
	int fd = -1;
	fd = open(GPS_PROPERTY, O_RDWR|O_CREAT, 0606);
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
#if 0
		if(property_set(GPS_PROPERTY, "0x6620") < 0)
			MND_ERR("set_property error, %s\n", strerror(errno));
		return;
#endif
	}

	/*detect if there is 3332, yes set GPS property to 3332, then else read from combo chip to see which GPS chip used*/
	res = confirm_if_3332();	/*0 means 3332, 1 means not 3332, other value means error*/

	if(res == 0)
	{
		MND_MSG("we get MT3332\n");

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
		if (strcmp(chip_id, "0x6572") ==0 )
		{
			MND_MSG("we get MT6572\n");
			if(write(fd, "0x6572", 10) == -1)		
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
static int GPS_Open()
{
    short err;
    pid_t pid;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int mt3326_fd;
    struct termios termOptions;
    char *argv[] = {"/system/xbin/libmnla", "libmnlp"};
    unsigned char query[11]     = {0x04, 0x24, 0x0b, 0x00, 0x08, 0xff, 0x19, 0x00, 0xe5, 0x0d, 0x0a};
    unsigned char response[11]  = {0x04, 0x24, 0x0b, 0x00, 0x1d, 0xff, 0x01, 0xaa, 0x42, 0x0d, 0x0a};
    unsigned char buf[20] = {0};
    int nRead = 0, nWrite = 0;
    chip_detector();


    LOGD(TAG"GPS_Open() 1\n");
    // power on GPS chip
#if defined(MTK_GPS_MT3332)
    err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 4);
    if(err != 0)
    {
        LOGD(TAG"GPS_Open: GPS power-on error: %d\n", err);
        return (-1);
    }
#endif


    // run gps driver (libmnlp)
    if ((pid = fork()) < 0) 
    {
        LOGD(TAG"GPS_Open: fork fails: %d (%s)\n", errno, strerror(errno));
        return (-2);
    } 
    else if (pid == 0)  /*child process*/
    {
        int err;

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
		else if(strcmp(chip_id, "0x6582") == 0)
		{
			MND_MSG("we get MT6582\n");
			char *mnl6582 = "/system/xbin/libmnlp_mt6582";
			argv[0] = mnl6582;				
		}
		else if(strcmp(chip_id, "0x6571") == 0)
		{
			MND_MSG("we get MT6571\n");
			char *mnl6571 = "/system/xbin/libmnlp_mt6571";
			argv[0] = mnl6571;				
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
			return -1;
		}        

        MND_MSG("execute: %s \n", argv[0]);
        err = execl(argv[0], "libmnlp", "1Hz=y", NULL);
        if (err == -1){
            MND_MSG("execl error: %s\n", strerror(errno));
            return -1;
        }
        return 0;
    } 
    else  /*parent process*/
    {
        mnl_pid = pid;
        LOGD(TAG"GPS_Open: mnl_pid = %d\n", pid);
    }

    // create socket connection to gps driver
    portno = 7000;
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        LOGD(TAG"GPS_Open: ERROR opening socket");
        return (-4);
    }
    /*
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        LOGD(TAG"GPS_Open: ERROR, no such host\n");
        return (-5);
    }
	*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
/*    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);*/
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr.sin_port = htons(portno);
	
	int try_time = 5;
   	do
   	{
   		LOGD(TAG"GPS_Open: try connecting");
   		sleep(1);// sleep 5sec for libmnlp to finish initialization
    }while(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 && --try_time);
    if (try_time == 0) 
    {
         LOGD(TAG"GPS_Open: ERROR connecting");
         return (-6);
    }	
	init_flag = 1;
    LOGD(TAG"GPS_Open() 2, set init_flag = 1\n");

    return 0;
}

static void GPS_Close()
{    
    int err;
    
    LOGD(TAG"GPS_Close() 1\n");

    // disconnect to gps driver
    if(sockfd != C_INVALID_SOCKET)
    {
        LOGD(TAG"GPS_Close() 2\n");
        close(sockfd);
        LOGD(TAG"GPS_Close() 3\n");
        sockfd = C_INVALID_SOCKET;
    }
    LOGD(TAG"GPS_Close() 4\n");

    // kill gps driver (libmnlp)
    if(mnl_pid != C_INVALID_PID)
    {
        LOGD(TAG"GPS_Close() 5\n");
        kill(mnl_pid, SIGKILL);
        usleep(500000); //500ms
    }
    LOGD(TAG"GPS_Close() 6\n");
    
    // power off GPS chip
#if defined(MTK_GPS_MT3332)
    err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 0);
    if(err != 0)
    {
        LOGD(TAG"GPS power-off error: %d\n", err);
    }
    LOGD(TAG"GPS_Close() 6\n");
#endif
	unlink(GPS_PROPERTY);
    return;
}

unsigned char CheckSum(char *buf, int size)
{
   int i;
   char chksum=0, chksum2=0;

   if(size < 5)
      return false;

   chksum = buf[1];
   for(i = 2; i < (size - 2); i++)
   {
      if(buf[i] != '*')
      {
        chksum ^= buf[i];
      }
      else
      {
        if(buf[i + 1] >= 'A')
        {
          chksum2 = (buf[i+1]-'A'+10)<<4;
        }
        else
        {
          chksum2 = (buf[i+1]-'0')<<4;
        }

        if(buf[i + 2] >= 'A')
        {
          chksum2 += buf[i+2]-'A'+10;
        }
        else
        {
          chksum2 += buf[i+2]-'0';
        }
        break;
      }
    }

   /* if not found character '*' */
   if(i == (size - 2))
   {
      return (false);
   }

   if(chksum == chksum2)
   {
     return (true);
   }
   else
   {
     return (false);
   }
}

bool FetchField(char *start, char *result)
{
   char *end;

   if(start == NULL)
      return false;

   end = strstr( start, ",");
   // the end of sentence
   if(end == NULL)
      end = strstr(start, "*");

   if(end-start>0)
   {
     strncpy( result, start, end-start);
     result[end-start]='\0';
   }
   else   // no data
   {
     result[0]='\0';
     return false;
   }

   return true;
}
void GLL_Parse( char *head)
{
   // $GPGLL,2446.367638,N,12101.356226,E,144437.000,A,A*56
   char *start, result[20], tmp[20], *point;
   short len=0;
   char FixService;

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // Position(Lat)
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField(start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lat = (float)(atof(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lat += (float)(atof(tmp)/60.0);
      }

      // N or S
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField(start, result))
      {
         if(*result=='S')
            g_gpsInfo.Lat = -g_gpsInfo.Lat;
      }

      // Position(Lon)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField(start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lon = (float)(atof(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lon += (float)(atof(tmp)/60.0);
      }

      // E or W
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         if(*result=='W')
            g_gpsInfo.Lon = -g_gpsInfo.Lon;
      }

      // UTC Time
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         // Hour
         strncpy( tmp, result, 2);
         tmp[2]='\0';
         g_gpsInfo.hour = atoi(tmp);
         // Min
         strncpy( tmp, result+2, 2);
         tmp[2]='\0';
         g_gpsInfo.min = atoi(tmp);
         // Sec
         strncpy( tmp, result+4, strlen(result)-4);
         tmp[strlen(result)-4]='\0';
         g_gpsInfo.sec = (float)(atof(tmp));
      }   

      // The positioning system Mode Indicator and Status fields shall not be null fields.
      // Data valid
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      if(!FetchField( start, result))
         return;
      
      if(*result=='A')
      {
         // Fix Type
         if(g_gpsInfo.FixType == 0)
            g_gpsInfo.FixType = 1;   // Assume 2D, if there's no other info.

         // Fix Service
         start = strstr( start, ",");
         if(start != NULL)
             start = start +1;
         else
            return;

         if(!FetchField( start, result))
            return;

         FixService = *result;

         switch(FixService)
         {
            case 'A':
            {
               g_gpsInfo.FixService = 1;
               break;
            }
            case 'D':
            {
               g_gpsInfo.FixService = 2;
               break;
            }
            case 'E':
            {
               g_gpsInfo.FixService = 6;
               break;
            }
         }
      }
      else // Data invalid
      {
         g_gpsInfo.FixType = 0;    // NoFix
         g_gpsInfo.FixService = 0; // NoFix
      }
   }
}
//---------------------------------------------------------------------------
void RMC_Parse( char *head)
{
   // $GPRMC,073446.000,A,2446.3752,N,12101.3708,E,0.002,22.08,121006,,,A*6C

   char *start, result[20], tmp[20], *point;
   short len=0;

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // UTC time : 161229.487
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, tmp))
      {
         // Hour
         strncpy( result, tmp, 2);
         result[2]='\0';
         g_gpsInfo.hour = atoi(result);
         // Min
         strncpy( result, tmp+2, 2);
         result[2]='\0';
         g_gpsInfo.min = atoi(result);
         // Sec
         strncpy( result, tmp+4, strlen(tmp)-4);
         result[strlen(tmp)-4]='\0';
         g_gpsInfo.sec = (float)(atof(result));
      }

      // valid
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;

      if(*result == 'A')
      {
         if(g_gpsInfo.FixType == 0)
            g_gpsInfo.FixType = 1;      // Assume 2D

         if(g_gpsInfo.FixService == 0)
            g_gpsInfo.FixService = 1;   // Assume SPS
      }
      else
      {
         g_gpsInfo.FixType = 0;    // NoFix
         g_gpsInfo.FixService = 0; // NoFix
      }

      // Position(Lat) : 3723.2475(N)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lat = (float)(atoi(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lat += (float)(atof(tmp)/60.0);
      }
	  else  //Can not fetch Lat field
	  {
	     g_gpsInfo.Lat = 0;
	  }

      // N or S
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result) && g_gpsInfo.Lat!=0)
      {
         if(*result=='S')
            g_gpsInfo.Lat = -g_gpsInfo.Lat;
      }

      // Position(Lon) : 12158.3416(W)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lon = (float)(atoi(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lon += (float)(atof(tmp)/60.0);
      }
	  else  //Can not fetch Lat field
	  {
	     g_gpsInfo.Lon = 0;
	  }

      // E or W
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result) && g_gpsInfo.Lat!=0)
      {
         if(*result=='W')
            g_gpsInfo.Lon = -g_gpsInfo.Lon;
      }

      // Speed : 0.13
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.Speed = (float)(atof(result) * Knot2Kmhr);
      }

      // Track : 309.62
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.Track = (float)(atof(result));
      }

      // Date : 120598
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         // Day
         strncpy(tmp, result, 2);
         tmp[2]='\0';
         g_gpsInfo.day=atoi(tmp);

         // Month
         strncpy(tmp, result+2, 2);
         tmp[2]='\0';
         g_gpsInfo.mon=atoi(tmp);

         // Year
         strncpy(tmp, result+4, 2);
         tmp[2]='\0';
         g_gpsInfo.year=atoi(tmp)+2000;
      }

      // skip Magnetic variation
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      // mode indicator
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      if(!FetchField( start, result))
         return;

      if(g_gpsInfo.FixType > 0)
      {
         switch(result[0])
         {
            case 'A':
            {
               g_gpsInfo.FixService = 1;
               break;
            }
            case 'D':
            {
               g_gpsInfo.FixService = 2;
               break;
            }
            case 'E':
            {
               g_gpsInfo.FixService = 6;
               break;
            }
         }
      }
   }
}
//---------------------------------------------------------------------------
void VTG_Parse( char *head)
{
   //$GPVTG,159.16,T,,M,0.013,N,0.023,K,A*34
   char *start, result[20];
   char FixService;

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // Track
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.Track = (float)(atof(result));
      }

      // ignore
      start = strstr( start, ",");     // T
      if(start != NULL)
         start = start +1;
      else
         return;

      start = strstr( start, ",");     // NULL
      if(start != NULL)
         start = start +1;
      else
         return;

      start = strstr( start, ",");     // M
      if(start != NULL)
         start = start +1;
      else
         return;

      // Speed
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.Speed = (float)(atof(result) * Knot2Kmhr);
      }

      // ignore
      start = strstr( start, ",");     // N
      if(start != NULL)
         start = start +1;
      else
         return;

      start = strstr( start, ",");     // 0.023
      if(start != NULL)
         start = start +1;
      else
         return;

      start = strstr( start, ",");     // K
      if(start != NULL)
         start = start +1;
      else
         return;

      // Fix Service
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      if(!FetchField( start, result))
         return;
      FixService = *result;

      if(FixService != 'N')
      {
         if(g_gpsInfo.FixType == 0)
            g_gpsInfo.FixType = 1;  //Assume 2D

         switch(FixService)
         {
            case 'A':
            {
               g_gpsInfo.FixService = 1;
               break;
            }
            case 'D':
            {
               g_gpsInfo.FixService = 2;
               break;
            }
            case 'E':
            {
               g_gpsInfo.FixService = 6;
               break;
            }
         }
      }
      else // NoFix
      {
         g_gpsInfo.FixType = 0;    // NoFix
         g_gpsInfo.FixService = 0; // NoFix
      }
   }
}
//---------------------------------------------------------------------------
void GSA_Parse( char *head)
{
   // $GPGSA,A,3,03,19,27,23,13,16,15,11,07,,,,1.63,0.95,1.32*03
   char *start, result[20];
   short sv_cnt=0, i;

   if(CheckSum(head, strlen(head)))
   {
      //Fix SV
      memset(&g_gpsInfo.fixSV, 0, sizeof(g_gpsInfo.fixSV));

      //Valid
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;

      if((*result == 'A') || (*result == 'M'))
      {
         // Fix Type
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;
         if(!FetchField( start, result))
            return;
         g_gpsInfo.FixType = atoi(result)-1;

         if(g_gpsInfo.FixType > 0)          // Fix
         {
            if(g_gpsInfo.FixService == 0)
               g_gpsInfo.FixService = 1;    //Assume SPS FixSerivce
         }
         else
         {
            g_gpsInfo.FixType = 0;    // NoFix
            g_gpsInfo.FixService = 0; // NoFix
         }
      }
      else
      {
         g_gpsInfo.FixType = 0;    // NoFix
         g_gpsInfo.FixService = 0; // NoFix
      }

      for(i=0 ; i<12 ; i++)
      {
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;

         FetchField( start, result);

         if(strlen(result)>0)
            g_gpsInfo.fixSV[sv_cnt++] = atoi(result);
      }

      //PDOP
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      g_gpsInfo.PDOP = (float)(atof(result));

      //HDOP
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      g_gpsInfo.HDOP = (float)(atof(result));

      //VDOP
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      g_gpsInfo.VDOP = (float)(atof(result));
   }
}
//---------------------------------------------------------------------------
void GSV_Parse( char *head)
{
   // $GPGSV,3,1,09,03,63,020,43,19,76,257,37,27,14,320,30,23,39,228,37*79
   // $GPGSV,3,2,09,13,38,274,38,16,31,058,37,15,16,055,34,11,16,192,32*76
   // $GPGSV,3,3,09,07,15,043,26*40

   char *start, result[20];
   short sv_cnt=0, base, i;

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // ignore
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      //first Message
      if(*(start+2)=='1')
      {
         memset( &g_svInfo, 0, sizeof(g_svInfo));
         //g_fgSVUpdate = false;
      }

      // Last Message
      //if(*start == *(start+2))
      //   g_fgSVUpdate = true;

      //base  //sentence number.
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      base = (atoi(result)-1)*4;

      //total
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      g_gpsInfo.SV_cnt = atoi(result);
      if(g_gpsInfo.SV_cnt == 0)
      {
         return;
      }

      for( i=0 ; i<4 ; i++)
      {
         //SVid
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;
         FetchField( start, result);
         if(strlen(result)>0)
            g_svInfo[base+sv_cnt].SVid = atoi(result);
         else
            g_svInfo[base+sv_cnt].SVid = 0;

         //elev
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;
         FetchField( start, result);
         if(strlen(result)>0)
            g_svInfo[base+sv_cnt].elv = atoi(result);
         else
            g_svInfo[base+sv_cnt].elv = 0;

         //azimuth
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;
         FetchField( start, result);
         if(strlen(result)>0)
            g_svInfo[base+sv_cnt].azimuth = atoi(result);
         else
            g_svInfo[base+sv_cnt].azimuth = 0;

         //SNR
         start = strstr( start, ",");
         if(start != NULL)
            start = start +1;
         else
            return;
         if(*start == '*')
            g_svInfo[base+sv_cnt].SNR = 0;
         else
         {
            FetchField( start, result);
            if(strlen(result)>0)
               g_svInfo[base+sv_cnt].SNR = atoi(result);
            else
               g_svInfo[base+sv_cnt].SNR = 0;
         }

         sv_cnt++;

         if(base+sv_cnt == g_gpsInfo.SV_cnt)
            break;
      }
   }
}
//---------------------------------------------------------------------------
void GGA_Parse( char *head)
{
   //$GPGGA,144437.000,2446.367638,N,12101.356226,E,1,9,0.95,155.696,M,15.057,M,,*58
   char *start, result[20], tmp[20], *point;
   short len=0;

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // UTC time : 144437.000
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      if(FetchField( start, result))
      {
         // Hour
         strncpy( tmp, result, 2);
         tmp[2]='\0';
         g_gpsInfo.hour = atoi(tmp);
         // Min
         strncpy( tmp, result+2, 2);
         tmp[2]='\0';
         g_gpsInfo.min = atoi(tmp);
         // Sec
         strncpy( tmp, result+4, strlen(result)-4);
         tmp[strlen(result)-4]='\0';
         g_gpsInfo.sec = (float)(atof(tmp));
      }

      // Position(Lat)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lat = (float)(atoi(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lat += (float)(atof(tmp)/60.0);
      }

      // N or S
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         if(*result=='S')
            g_gpsInfo.Lat = -g_gpsInfo.Lat;
      }

      // Position(Lon)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         point = strstr( result, ".");
         len = (point-2)-result;
         strncpy(tmp, result, len);
         tmp[len]='\0';
         g_gpsInfo.Lon = (float)(atoi(tmp));
         strncpy(tmp, result+len, strlen(result)-len);
         tmp[strlen(result)-len]='\0';
         g_gpsInfo.Lon += (float)(atof(tmp)/60.0);
      }

      // E or W
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         if(*result=='W')
            g_gpsInfo.Lon = -g_gpsInfo.Lon;
      }

      //GPS Fix Type and Service
      // 0: NoFix, 1:SPS, 2:DGPS, 6:Estimate
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(!FetchField( start, result))
         return;
      g_gpsInfo.FixService = atoi(result);

      // Fix
      if(g_gpsInfo.FixService > 0)
      {
         if(g_gpsInfo.FixType == 0)
            g_gpsInfo.FixType = 1; // Assume 2D
      }

      start = strstr( start, ",");   // Number of SV in use , ex :9
      if(start != NULL)
         start = start +1;
      else
         return;

      // HDOP
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.HDOP = (float)(atof(result));
      }

      //Altitude (mean sea level)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
      {
         g_gpsInfo.Alt = (float)(atof(result));
      }

      //Altitude unit (bypass)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      //Altitude (Geoidal separation)
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      
      if(FetchField( start, result))
      {
         g_gpsInfo.Alt += (float)(atof(result));
      }   
   }
}
//---------------------------------------------------------------------------
void ZDA_Parse( char *head)
{
   // $GPZDA,000007.123,06,01,2000,,*50
   char *start, result[20], tmp[20];

   // check checksum
   if(CheckSum(head, strlen(head)))
   {
      // UTC time : 000007.123
      start = strstr( head, ",");
      if(start != NULL)
         start = start +1;
      else
         return;

      if(FetchField( start, result))
      {
         // Hour
         strncpy( tmp, result, 2);
         tmp[2]='\0';
         g_gpsInfo.hour = atoi(tmp);

         // Min
         strncpy( tmp, result+2, 2);
         tmp[2]='\0';
         g_gpsInfo.min = atoi(tmp);

         // Sec
         strncpy( tmp, result+4, strlen(result)-4);
         tmp[strlen(result)-4]='\0';
         g_gpsInfo.sec = (float)(atof(tmp));
      }

      // Day
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
         g_gpsInfo.day = atoi(result);

      // Month
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
         g_gpsInfo.mon = atoi(result);

      // Year
      start = strstr( start, ",");
      if(start != NULL)
         start = start +1;
      else
         return;
      if(FetchField( start, result))
         g_gpsInfo.year = atoi(result);
   }
}

void NMEA_Parse(char *TempBuf)
{
    char *GP;
    char type[4];
    //char TempBuf[500];
    //strcpy(TempBuf,g_NMEAbuf);
    GP=strtok(TempBuf, "\r\n");

    memset(&g_gpsInfo, 0, sizeof(g_gpsInfo));
    memset(g_svInfo, 0, (sizeof(SVInfo)*NUM_CH));
    memset(g_chInfo, 0, (sizeof(ChInfo)*PSEUDO_CH));

    if(GP == NULL)
        return;

    do
    {
        // Channel Status
        if(strncmp(GP, "$PMTKCHN", 8) == 0)
        {
            //Channel_Parse(GP);
        }
        //Ack Parse
        else if(strncmp(GP, "$PMTK", 5) == 0)
        {
            //Ack_Parse(GP);
        }
        // NMEA Parse
        else if((strncmp(GP, "$GP", 3) == 0) && (strlen(GP) > 10))
        {
            // skip "$GP" char to fetch Message Type
            strncpy ( type, GP+3, 3);
            type[3]='\0';

            if(strcmp( type, "GLL")==0)
            {
                GLL_Parse( GP);
            }
            else if(strcmp( type, "RMC")==0)
            {
                RMC_Parse( GP);
            }
            else if(strcmp( type, "VTG")==0)
            {
                VTG_Parse( GP);
            }
            else if(strcmp( type, "GSA")==0)
            {
                GSA_Parse( GP);
            }
            else if(strcmp( type, "GSV")==0)
            {
                GSV_Parse( GP);
            }
            else if(strcmp( type, "GGA")==0)
            {
                GGA_Parse( GP);
            }
            else if(strcmp( type, "ZDA")==0)
            {
                ZDA_Parse( GP);
            }
        }
    }while( (GP = strtok( NULL, "\r\n")) != NULL );
}

static void gps_update_info(struct gps_desc *gps, char *info)
{
    char *ptr;
    short i = 0;
    short read_leng = 0;
    short total_leng = 0;

    memset(info, '\n', INFO_SIZE);
    info[INFO_SIZE-1] = 0x0;

    if(sockfd != C_INVALID_SOCKET)
    {
        memset(nmea_buf, 0, NMEA_SIZE);
        //do
        //{
            LOGD(TAG"read from sockfd 1\n");
            read_leng = read(sockfd, &nmea_buf[total_leng], (NMEA_SIZE - total_leng));
            total_leng += read_leng;
            LOGD(TAG"read_leng=%d, total_leng=%d\n", read_leng, total_leng);
        //}while((read_leng > 0) /*|| ((total_leng > 0) && (nmea_buf[total_leng-1] != '\n'))*/);
        
        if (read_leng <= 0) 
        {
            LOGD(TAG"ERROR reading from socket\n");
            sprintf(gps->info, "%s\n", uistr_info_gps_error);
            gps->exit_thd = true;
        }
        else if(total_leng > 0)
        {
            NMEA_Parse((char*)&nmea_buf[0]);

            ptr  = info;            
            if(((g_gpsInfo.FixType != 0) && (ttff != 0)/*avoid prev second's NMEA*/) || (fixed == 1)) // 2D or 3D fixed
            {
                ptr += sprintf(ptr, "%s: %d\n", uistr_info_gps_fixed, ttff);
                fixed = 1;
                //for auto test
                LOGD(TAG"Fix success");
             // ttff_check_res = 1;				               
            }
            else if((g_gpsInfo.FixType != 0) && (ttff == 0)) //skip prev second's NMEA, clear data
            {
                ptr += sprintf(ptr, "%s: %d\n", uistr_info_gps_ttff, ttff++);
                memset(&g_gpsInfo, 0, sizeof(g_gpsInfo));
                memset(g_svInfo, 0, (sizeof(SVInfo)*NUM_CH));
                memset(g_chInfo, 0, (sizeof(ChInfo)*PSEUDO_CH));
            }
            else    // no fix
            {
                ptr += sprintf(ptr, "%s: %d\n", uistr_info_gps_ttff, ttff++);
            }

            for(i = 0; i < g_gpsInfo.SV_cnt; i++)
            {
                ptr += sprintf(ptr, "%s[%d] : %d\n", uistr_info_gps_svid, g_svInfo[i].SVid, g_svInfo[i].SNR);
                ttff_check_res = 1;
            }
        }
    }

    return;
}

static void *gps_update_iv_thread(void *priv)
{
    struct gps_desc *gps = (struct gps_desc *)priv;
    struct itemview *iv = gps->iv;
    short count = 1, chkcnt = 10;
    int init_status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    //init GPS driver
    memset(gps->info, '\n', INFO_SIZE);
    sprintf(gps->info, "%s\n", uistr_info_gps_init);
    iv->redraw(iv);
    sleep(1);
    init_status = GPS_Open();
    if(init_status != 0)    // GPS init fail
    {
        memset(gps->info, '\n', INFO_SIZE);
        sprintf(gps->info, "%s (%d)\n", uistr_info_gps_error, init_status);
        iv->redraw(iv);
    }
    else
    {
        //init GPS driver done
        ttff = 0;
        fixed = 0;
        memset(gps->info, '\n', INFO_SIZE);
        iv->redraw(iv);
        
        while (1) {
            usleep(100000); // wake up every 0.1sec
            chkcnt--;

            if (gps->exit_thd)
            {
                LOGD(TAG "%s, gps->exit_thd = true\n", __FUNCTION__);
                break;
            }

            if(httff == 1)
            {
                httff = 0;
                write(sockfd, "$PMTK101*32\r\n", sizeof("$PMTK101*32\r\n"));
                ttff = 0;
                fixed = 0;
                memset(gps->info, '\n', INFO_SIZE);
                gps->info[INFO_SIZE-1] = 0x0;
                iv->redraw(iv);
            }

            if(cttff == 1)
            {
                cttff = 0;
                write(sockfd, "$PMTK103*30\r\n", sizeof("$PMTK103*30\r\n") );
                ttff = 0;
                fixed = 0;
                memset(gps->info, '\n', INFO_SIZE);
                gps->info[INFO_SIZE-1] = 0x0;
                iv->redraw(iv);
            }

            if (chkcnt > 0)
                continue;

            chkcnt = 10;

            gps_update_info(gps, gps->info);
            iv->redraw(iv);
        }
    }
    //close GPS driver
    GPS_Close();
    //close GPS driver done
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

	return NULL;
}

int gps_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    char *pTimeout=NULL;
    int chosen;
    bool exit = false;
    struct gps_desc *gps = (struct gps_desc *)priv;
    struct itemview *iv;
	pTimeout = ftm_get_prop("GPS.TIMEOUT");
	if(pTimeout != NULL )
	{
		timeout=atoi(pTimeout);
		LOGD(TAG "timeout value is %d\n", timeout);	
	}
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&gps->title, param->name, COLOR_YELLOW);
    init_text(&gps->text, &gps->info[0], COLOR_YELLOW);
    
    gps_update_info(gps, gps->info);
   
    gps->exit_thd = false;

    if (!gps->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        gps->iv = iv;
    }
    
    iv = gps->iv;
    iv->set_title(iv, &gps->title);
    iv->set_items(iv, gps_items, 0);
    iv->set_text(iv, &gps->text);
    
	iv->start_menu(iv,0);
    pthread_create(&gps->update_thd, NULL, gps_update_iv_thread, priv);
    int wait_time = 10;
    int init_try_time = 0;
    while(!init_flag && wait_time)
    {
    	LOGD(TAG" init_flag = %d, try time = %d\n", init_flag, ++init_try_time);
    	sleep(1);
    	--wait_time;	
    }
    init_flag = 0;
    wait_time = 10;
    init_try_time = 0;
    LOGD(TAG" init_flag = %d\n", init_flag);
	iv->redraw(iv);
    do{
        //Auto test: only CTTFF
        cttff = 1;
        do{
            usleep(1000000); //check status per sec
            LOGD(TAG"after %d sec from test starting, we still can not see any SV\n", ttff);
        }while(ttff <= timeout && !ttff_check_res);

        if(ttff_check_res == 1){
            LOGD(TAG"Test pass"); 
            gps->mod->test_result = FTM_TEST_PASS;
	      }else{
            LOGD(TAG"Test fail");
            gps->mod->test_result = FTM_TEST_FAIL;		
        }
		if(unlink("/data/misc/mtkgps.dat")!=0 )
			LOGD(TAG "unlink mtkgps.dat error, error is %s\n", strerror(errno));
		    ttff_check_res = 0;
    	unlink(GPS_PROPERTY);

     }while(0);
     
    gps->exit_thd = true;
    LOGD(TAG "%s, gps->exit_thd = true\n", __FUNCTION__);    
#if 0
    do {
        chosen = iv->run(iv, &exit);
        LOGD(TAG "%s, chosen = %d\n", __FUNCTION__, chosen);
        switch (chosen) {
        case ITEM_HTTFF:
            httff = 1;
            break;
        case ITEM_CTTFF:
            cttff = 1;
            break;
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                gps->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                gps->mod->test_result = FTM_TEST_FAIL;
            }           
            exit = true;
            break;
        }
        
        if (exit) {
            gps->exit_thd = true;
            LOGD(TAG "%s, gps->exit_thd = true\n", __FUNCTION__);
            break;
        }        
    } while (1);
#endif
    pthread_join(gps->update_thd, NULL);

    return 0;
}

int gps_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct gps_desc *gps;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_GPS, sizeof(struct gps_desc));
    gps  = mod_to_gps(mod);

    gps->mod      = mod;

    if (!mod)
    {
        return -ENOMEM;
    }

    ret = ftm_register(mod, gps_entry, (void*)gps);

    return ret;
}

#endif

