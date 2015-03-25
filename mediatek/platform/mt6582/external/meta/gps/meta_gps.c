/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   meta_gps.cpp
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Implement GPS interface for META mode.
 *
 * Author:
 * -------
 *  LiChunhui (MTK80143)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * Mar 20 2009 mtk80143
 * [DUMA00111323] [GPS] modify for GPS META
 * Add for GPS meta
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
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
#include "FT_Public.h"
//#include "type.h"
#include "meta_common.h"
#include "WM2Linux.h"
#include "meta_gps_para.h"

//for read NVRAM
#include "libnvram.h"
#include "CFG_GPS_File.h"
#include "CFG_GPS_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"

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

#define C_INVALID_PID  (-1)   /*invalid process id*/
#define C_INVALID_TID  (-1)   /*invalid thread id*/
#define C_INVALID_FD   (-1)   /*invalid file handle*/
#define C_INVALID_SOCKET (-1) /*invalid socket id*/

#define MND_ERR META_LOG
#define MND_MSG META_LOG

static GPS_CNF gps_cnf1;
static GPS_CNF gps_cnf2;

pid_t mnl_pid = C_INVALID_PID;
int sockfd = C_INVALID_SOCKET;
pthread_t gps_meta_thread_handle = C_INVALID_TID;

static int mnl_write_attr(const char *name, unsigned char attr) 
{
    int err, fd = open(name, O_RDWR);
    char buf[] = {attr + '0'};
    
    if (fd == -1) {
        META_LOG("open %s err = %s\n", name, strerror(errno));
        return -errno;
    }
    do { err = write(fd, buf, sizeof(buf) ); }
    while (err < 0 && errno == EINTR);
    
    if (err != sizeof(buf)) { 
        META_LOG("write fails = %s\n", strerror(errno));
        err = -errno;
    } else {
        err = 0;    /*no error*/
    }
    if (close(fd) == -1) {
        META_LOG("close fails = %s\n", strerror(errno));
        err = (err) ? (err) : (-errno);
    }
    META_LOG("write '%d' to %s okay\n", attr, name);    
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
#if 0
		if(property_set(GPS_PROPERTY, "0x6620") < 0)
			MND_ERR("set_property error, %s\n", strerror(errno));
		return;
#endif
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
	}
	else
		MND_ERR("this should never be showed\n");

exit_chip_detector:
	close(fd);
	return;
}

/*****************************************************************************/
int META_GPS_Open()
{
    int err;
    pid_t pid;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *argv[] = {"/system/xbin/libmnla", "libmnlp"};
	chip_detector();
    META_LOG("META_GPS_Open() 1\n");
    // power on GPS chip
    err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 4);
    if(err != 0)
    {
        META_LOG("META_GPS_Open: GPS power-on error: %d\n", err);
        return (-1);
    }

    // run gps driver (libmnlp)
    if ((pid = fork()) < 0) 
    {
        META_LOG("META_GPS_Open: fork fails: %d (%s)\n", errno, strerror(errno));
        return (-2);
    } 
    else if (pid == 0)  /*child process*/
    {
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
        META_LOG("META_GPS_Open: mnl_pid = %d\n", pid);
    }

    // create socket connection to gps driver
    portno = 7000;
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
    {
        META_LOG("META_GPS_Open: ERROR opening socket");
        return (-4);
    }
 /*
    server = gethostbyname("127.0.0.1");
    if (server == NULL) {
        META_LOG("META_GPS_Open: ERROR, no such host\n");
        return (-5);
    }
*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
//    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    sleep(3);  // sleep 5sec for libmnlp to finish initialization

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
         META_LOG("META_GPS_Open: ERROR connecting");
         return (-6);
    }	

    // run GPS_MetaThread
    if (pthread_create(&gps_meta_thread_handle, NULL, GPS_MetaThread, NULL)) 
    {
        META_LOG("META_GPS_Open: error creating dsp thread \n");
        return (-7);
    }
    META_LOG("META_GPS_Open() 2\n");

    return 0;
}

void META_GPS_Close()
{    
    int err;
    
    META_LOG("META_GPS_Close() 1\n");
    // close GPS_MetaThread
#if 0
    if ((err = pthread_kill(gps_meta_thread_handle, SIGUSR1))) 
    {
        META_LOG("META_GPS_Close: pthread_kill 1 error(%d) \n", err);
    }
    META_LOG("META_GPS_Close() 2\n");
    if ((err = pthread_join(gps_meta_thread_handle, NULL)))
    {
        META_LOG("META_GPS_Close: pthread_kill 2 error(%d) \n", err);
    }
    META_LOG("META_GPS_Close() 3\n");
#endif

    // disconnect to gps driver
    if(sockfd != C_INVALID_SOCKET)
    {
        close(sockfd);
        sockfd = C_INVALID_SOCKET;
    }
    META_LOG("META_GPS_Close() 4\n");

    // kill gps driver (libmnlp)
    if(mnl_pid != C_INVALID_PID)
    {
        kill(mnl_pid, SIGKILL);
    }
    META_LOG("META_GPS_Close() 5\n");
    unlink("/data/misc/mtkgps.dat");
    // power off GPS chip
    err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 0);
    if(err != 0)
    {
        META_LOG("GPS power-off error: %d\n", err);
    }
    META_LOG("META_GPS_Close() 6\n");

	unlink(GPS_PROPERTY);
    META_LOG("META_GPS_Close() 7\n");
    return;
}


void META_GPS_OP(GPS_REQ *req, char *peer_buff, unsigned short peer_len) 
{
    memset(&gps_cnf1, 0, sizeof(GPS_CNF));	
    gps_cnf1.header.id = FT_GPS_CNF_ID;
    gps_cnf1.header.token = req->header.token;
    gps_cnf1.op = req->op;
    memset(&gps_cnf2, 0, sizeof(GPS_CNF));	
    gps_cnf2.header.id = FT_GPS_CNF_ID;
    gps_cnf2.header.token = req->header.token;
    gps_cnf2.op = req->op;

    META_LOG("META_GPS_OP() 1, (%d)\n", req->op);
    switch(req->op) 
    {
        case GPS_OP_OPEN:
            META_LOG("META_GPS_OP(), GPS_OP_OPEN 1\n");
            if(META_GPS_Open() != 0)    // open fail
            {
                META_LOG("META_GPS_OP(), GPS_OP_OPEN fail\n");
                META_GPS_Close();
                META_LOG("Can't open gps driver \r\n");
                gps_cnf1.gps_status = FALSE;
                gps_cnf1.status = META_FAILED;	 
            }
            else
            {	        	 
                META_LOG("META_GPS_OP(), GPS_OP_OPEN OK\n");
                gps_cnf1.gps_status = TRUE; 
                gps_cnf1.status = META_SUCCESS;     	
            }
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);                	   
            META_LOG("META_GPS_OP(), GPS_OP_OPEN 2\n");
            break;

        case GPS_OP_CLOSE:	
            META_LOG("META_GPS_OP(), GPS_OP_CLOSE 1\n");
            META_GPS_Close();
            gps_cnf1.gps_status = TRUE;
            gps_cnf1.status = META_SUCCESS;
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);		   
            META_LOG("META_GPS_OP(), GPS_OP_CLOSE 2\n");
            break;

        case GPS_OP_SEND_CMD:
            META_LOG("META_GPS_OP(), GPS_OP_SEND_CMD\n");
            if(sockfd != C_INVALID_SOCKET)
            {
                int n = write(sockfd, req->cmd.buff, req->cmd.len);
                if (n < 0) 
                {
                     META_LOG("ERROR writing to socket\r\n");
                }
                META_LOG("META_GPS_OP(), GPS_OP_SEND_CMD: %s\r\n", req->cmd.buff);
            }  

            gps_cnf1.gps_status = TRUE;
            gps_cnf1.status = META_SUCCESS;
            META_LOG("GPS_OP_SEND_CMD, gps_cnf.status:%d\r\n", gps_cnf1.status);
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);

            break;   

        default:
            META_LOG("META_GPS_OP(), default 1\n");
            gps_cnf1.gps_status = FALSE;
            gps_cnf1.status = META_FAILED;
            WriteDataToPC(&gps_cnf1, sizeof(GPS_CNF), NULL, 0);
            META_LOG("META_GPS_OP(), default 2\n");
            break;
    }
    META_LOG("META_GPS_OP() 2\n");
}

void *GPS_MetaThread(void *arg)
{
    int read_leng = 0;
    int cnt = 0;
    char *ptr;
    unsigned char buf[10240];
    cnt = 0;
 
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        read_leng = 0;
        read_leng = read(sockfd, buf, sizeof(buf));
        if (read_leng < 0) 
        {
            META_LOG("ERROR reading from socket");
            return (void *)(-1);
        }
        else if(read_leng > 0)
        {
            ptr=strtok(buf, "\r\n");
            if(ptr == NULL)
            {
                continue;
            }
            
            do
            {
                /*if(strncmp(ptr, "$PMTK", 5) == 0)
                {
                    if((ptr[5] >= '0' && ptr[5] <= '9') &&
                       (ptr[6] >= '0' && ptr[6] <= '9') &&
                       (ptr[7] >= '0' && ptr[7] <= '9')) // $PMTK000~$PMTK999*/
                    //{
                        META_LOG("GPS_MetaThread: %s", ptr);
                        strncpy(&gps_cnf2.gps_ack.buff[cnt], ptr, (sizeof(gps_cnf2.gps_ack.buff) - cnt));
                        cnt += strlen(ptr);
                        gps_cnf2.gps_ack.buff[cnt++] = '\r';
                        gps_cnf2.gps_ack.buff[cnt++] = '\n';
                   // }
                //}
            }while((ptr=strtok(NULL, "\r\n")) != NULL);
            
            if(cnt > 0)
            {
                gps_cnf2.gps_ack.len = cnt;
                gps_cnf2.gps_status = TRUE;
                gps_cnf2.status = META_SUCCESS;
                META_LOG("GPS_MetaThread, status:%d, gps_cnf.gps_ack.len:%d\r\n", gps_cnf2.status, gps_cnf2.gps_ack.len);
                WriteDataToPC(&gps_cnf2, sizeof(GPS_CNF), NULL, 0);   
                cnt = 0;
            }
        }
    }
    
    return (void *)0;
}
