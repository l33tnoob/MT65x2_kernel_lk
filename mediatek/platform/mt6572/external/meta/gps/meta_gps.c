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

#define C_INVALID_PID  (-1)   /*invalid process id*/
#define C_INVALID_TID  (-1)   /*invalid thread id*/
#define C_INVALID_FD   (-1)   /*invalid file handle*/
#define C_INVALID_SOCKET (-1) /*invalid socket id*/

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

int META_GPS_Open()
{
    int err;
    pid_t pid;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

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
        int err;

        META_LOG("META_GPS_Open: execute: %s\n", "/system/xbin/libmnlp");
        int get_time = 5;
		char chip_id[PROPERTY_VALUE_MAX];
		while(get_time-- != 0 && (property_get("persist.mtk.wcn.combo.chipid", chip_id, NULL) !=0))
		{
			usleep(100000);
		}
		META_LOG("chip_id is %s\n", chip_id);
		if (strcmp(chip_id, "0x6620") ==0 )
		{
			META_LOG("we get MT6620\n");
			err = execl("/system/xbin/libmnlp_mt6620", "libmnlp", NULL);
		}
		else if(strcmp(chip_id, "0x6628") == 0)
		{
			META_LOG("we get MT6628\n");
			err = execl("/system/xbin/libmnlp_mt6628", "libmnlp", NULL);		
		}	
		else if(strcmp(chip_id, "0x6630") == 0)
		{
			META_LOG("we get MT6630\n");
			err = execl("/system/xbin/libmnlp_mt6630", "libmnlp", NULL);				
		}
		else if(strcmp(chip_id, "0x6572") == 0)
		{
			META_LOG("we get MT6572\n");
			err = execl("/system/xbin/libmnlp_mt6572", "libmnlp", NULL);				
		}
        if (err == -1)
        {
            META_LOG("META_GPS_Open: execl error: %s\n", strerror(errno));
            return (-3);
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
    
    // power off GPS chip
    err = mnl_write_attr("/sys/class/gpsdrv/gps/pwrctl", 0);
    if(err != 0)
    {
        META_LOG("GPS power-off error: %d\n", err);
    }
    META_LOG("META_GPS_Close() 6\n");

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
