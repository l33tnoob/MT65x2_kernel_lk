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
#include "wmt_ioctl.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/endian.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <linux/serial.h> /* struct serial_struct  */
#include <pthread.h>
#include <sched.h>
#define FIBER_STACK 8192

pthread_mutex_t mutex;

typedef enum DRV_INDEX
{
  DRV_BT = 0,
  DRV_FM,  
  DRV_GPS,
  DRV_WIFI,
  DRV_MAX,
}ENUM_DRV_INDEX;

typedef enum ARG_DRV_INDEX
{
  ARG_DRV_BT = 1 << DRV_BT,
  ARG_DRV_FM = 1 << DRV_FM,  
  ARG_DRV_GPS = 1 << DRV_GPS,
  ARG_DRV_WIFI = 1 << DRV_WIFI,
  ARG_DRV_ALL = ARG_DRV_BT + ARG_DRV_FM + ARG_DRV_GPS + ARG_DRV_WIFI,
}ENUM_ARG_DRV_INDEX;

#define is_bt_mask_on(a) ((a) & ARG_DRV_BT)
#define is_fm_mask_on(a) ((a) & ARG_DRV_FM)
#define is_gps_mask_on(a) ((a) & ARG_DRV_GPS)
#define is_wifi_mask_on(a) ((a) & ARG_DRV_WIFI)

#define QUICK_FUNC_ON_OFF_SUPPORT
#ifdef QUICK_FUNC_ON_OFF_SUPPORT
	#define MIN_INTERVAL 1
#else 
	#define MIN_FUNC_ON_TIME 4
	#define MIN_FUNC_OFF_TIME 1
	#define MIN_INTERVAL  MIN_FUNC_ON_TIME + MIN_FUNC_OFF_TIME
#endif

#define MAX_FUNC_ON_TIME 9
#define MAX_FUNC_OFF_TIME 3
#define MAX_INTERVAL  MAX_FUNC_ON_TIME + MAX_FUNC_OFF_TIME
//enum bool {false = 0, true = !false};

char *src_name[]={
	"BT",  
	"FM", 
	"GPS", 
	"WIFI",
	"UNKNOWN DEVICE",
	};

static int g_wmt_fd = -1;
static int g_reference = 0;
static int g_count = 2;
int g_on2off_interval = MAX_INTERVAL;
int g_off2on_interval = MAX_INTERVAL;
volatile int g_state[5] = {0};

void dump_state(void)
{
	//printf("%s:++, pid =%d\n",  __FUNCTION__, getpid());
	printf("%d: %s:g_state:[BT]%d, [FM]:%d, [GPS]:%d, [WIFI]:%d\n", getpid(),  __FUNCTION__, g_state[0], g_state[1], g_state[2], g_state[3]);
	//printf("%s:--, pid =%d\n",  __FUNCTION__, getpid());
}

int read_reference(ENUM_DRV_INDEX index)
{
	int ref = 0;
	volatile int flag = 0;
	pthread_mutex_lock(&mutex);
	if(index >= 4)
	{
		if(g_state[0] || g_state[1] || g_state[2] || g_state[3])
		{
			flag = 1;
		}
		else
		{
			flag = 0;
		}
	}
	else
	{
		flag = g_state[index];
	}
	pthread_mutex_unlock(&mutex);
	return flag;
}

void get_reference(ENUM_DRV_INDEX index)
{
	pthread_mutex_lock(&mutex);
	g_reference++;
	g_state[index] = 1;
	dump_state();
	pthread_mutex_unlock(&mutex);
}


void put_reference(ENUM_DRV_INDEX index)
{
	pthread_mutex_lock(&mutex);
	g_reference--;
	g_state[index] = 0;
	dump_state();
	pthread_mutex_unlock(&mutex);
}

void func_on_off(ENUM_DRV_INDEX index)
{
  pid_t pid = -1;
  int count = g_count;
//  printf("%s:++, index =%d\n",  __FUNCTION__, index);
  if(DRV_MAX > index && g_wmt_fd > 0)
  {
    while(count--)
    {
    	//turn on  src_name[index] function
    	if (0 == ioctl(g_wmt_fd, WMT_IOCTL_FUNC_ONOFF_CTRL, 0x80000000 | index))
    	{
    		printf("pid:%d, turn on %s success.\n",   getpid(), src_name[index]);
			//exit(0);
    	}
    	else
    	{
    		printf("pid:%d, turn on %s fail.\n",   getpid(), src_name[index]);
    		exit(-1);
    	}
    	
    	//printf("%s:-- pid: %d, finish turn on %s\n",  __FUNCTION__, getpid(), src_name[index]);
    	/*
    	//turn off  src_name[index] function
    	*/
    	sleep(g_on2off_interval);
    	if ( 0 == ioctl(g_wmt_fd, WMT_IOCTL_FUNC_ONOFF_CTRL, 0x00000000 | index))
    	{
    		printf("pid:%d, turn off %s success.\n",   getpid(), src_name[index]);
    		//exit(0);
    	}
    	else
    	{
    		printf("pid:%d, turn off %s fail.\n",   getpid(), src_name[index]);
    		exit(-1);
    	}
    	printf("%d:%s test:left count = %d.\n",  getpid(), src_name[index], count);
    	sleep(g_off2on_interval);
    	
    }
  }
  else
  {
  	printf("pid:%d, undnown device with index:%d.\n",  getpid(), index);
  }
 // printf("%s:--, index =%d\n",  __FUNCTION__, index);
 //exit(-2);
 //return;
  exit(0);
}


static void sig_child_term(int sig)
{
    printf("%s ++.\n", __FUNCTION__);
    int pid = -1;
    int stat;
    while((pid = waitpid(0, &stat, WNOHANG)) != 0)
    {
    	printf("%s:pid = %d, exit event.\n", __FUNCTION__, pid);
    }
    printf("%s:pid = %d.\n", __FUNCTION__, pid);
    printf("%s --.\n", __FUNCTION__);
}

int main(int argc, char *argv[])
{
	
	int bitmask = ARG_DRV_ALL;
	
	int i = 0;
	int status;
	void *stack[8];
	struct sigaction sa;
	if(argc != 5)
	{
		printf("wmt_concurrency usage:\n\
		wmt_concurrency looptimes  bitmask  on2offtime off2ontime\n\
		  -looptimes	on<->off switch times (<1000000)\n\
		  -bitmask\n\
		    -1:BT on<->off test\n\
		    -2:FM on<->off test\n\
		    -4:GPS on<->off test\n\
		    -8:WIFI on<->off test\n\
		    -x: can be combination of the upper 4 bitmasks\n\
		  -on2offtime	(0~12s)\n\
		    -function on hold time before turn off\n\
		  -off2ontime	(0~12s)\n\
		    -function off hold time before turn on\n\
		");
		return -1;
	}
	if ((argc > 1) && (argv[1] != NULL)) {
        
        g_count = atoi(argv[1]);
        printf("%s:argv[1] g_count param = %d\n",  __FUNCTION__, g_count);
        if(g_count < 0)
	    {
	    	g_count = 2;
	    }
	   	g_count = g_count > 1000000 ? 1000000 : g_count;
	    printf("%s:g_count = %d\n",  __FUNCTION__, g_count);
    }
    if ((argc > 2) && (argv[2] != NULL)) {
        
        bitmask = atoi(argv[2]);
        printf("%s:argv[2] bitmask param = %d\n",  __FUNCTION__, bitmask);
        if(bitmask <= 0 || bitmask > ARG_DRV_ALL)
    	{
    		bitmask = ARG_DRV_ALL;
    	}
    	printf("%s:bitmask = %d\n",  __FUNCTION__, bitmask);
    }
    
    if ((argc > 3) && (argv[3] != NULL)) {
        
        g_on2off_interval = atoi(argv[3]);
        printf("%s:argv[3] g_on2off_interval param = %d\n",  __FUNCTION__, g_on2off_interval);
        if(g_on2off_interval < MIN_INTERVAL)
    	{
    		g_on2off_interval = MIN_INTERVAL;
    	}
    	if(g_on2off_interval > MAX_INTERVAL)
    	{
    		g_on2off_interval = MAX_INTERVAL;
    	}
		printf("%s:g_on2off_interval = %d\n",  __FUNCTION__, g_on2off_interval);
    }
    if ((argc > 4) && (argv[4] != NULL)) {
        
        g_off2on_interval = atoi(argv[4]);
        printf("%s:argv[4] g_off2on_interval param = %d\n",  __FUNCTION__, g_off2on_interval);
        if(g_off2on_interval < MIN_INTERVAL)
    	{
    		g_off2on_interval = MIN_INTERVAL;
    	}
    	if(g_off2on_interval > MAX_INTERVAL)
    	{
    		g_off2on_interval = MAX_INTERVAL;
    	}
		printf("%s:g_off2on_interval = %d\n",  __FUNCTION__, g_off2on_interval);
    }
    
	for (i = sizeof(g_state)/sizeof(g_state[0]); i > 0; )
	{
		g_state[--i] = 0;
	}

	printf("pid = %d\n", getpid());
	for(i = sizeof(stack)/sizeof(void *); i > 0; )
	{
		stack[--i] = malloc(FIBER_STACK);
		if(stack[i] == NULL)
		{
			printf("pid = %d, malloc error\n", getpid());
			goto out;
		}
	}

    g_wmt_fd = open("/dev/stpwmt", O_RDWR | O_NOCTTY);	
    printf("%s:argc = %d\n", __FUNCTION__,  argc);
    
    if(pthread_mutex_init(&mutex, NULL) != 0)
    {
    	printf("%s:pthread_mutex_init fail\n",  __FUNCTION__);
    	goto out;	
    }
    if(g_wmt_fd > 0)
    {
    	memset(&sa, 0, sizeof(sa));
    	//signal(SIGCHLD, sig_child_term);
		sa.sa_handler = sig_child_term;
		sa.sa_flags = SA_NOCLDSTOP;
		sigaction(SIGCHLD, &sa, 0);
    	int sleepCOunter = g_count;
    	    	
    		if(is_bt_mask_on(bitmask) && read_reference(DRV_BT) == 0)
    		{
    			clone(&func_on_off, (char *)stack[0] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_BT);
    			//clone(&func_off, (char *)stack[4] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_BT);
    		}
    		
    		if(is_wifi_mask_on(bitmask) && read_reference(DRV_WIFI) == 0)
    		{
    			clone(&func_on_off, (char *)stack[1] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_WIFI);
    			//clone(&func_off, (char *)stack[5] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_WIFI);
    		}	
    		if(is_fm_mask_on(bitmask) && read_reference(DRV_FM) == 0)
    		{
    			clone(&func_on_off, (char *)stack[2] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_FM);
    			//clone(&func_off, (char *)stack[6] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_FM);
    		}
    		if(is_gps_mask_on(bitmask) && read_reference(DRV_GPS) == 0)
    		{
    			clone(&func_on_off, (char *)stack[3] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_GPS);
  				//clone(&func_off, (char *)stack[7] + FIBER_STACK, CLONE_VM | CLONE_FS | CLONE_FILES /*| CLONE_SIGHAND*/, (void *)DRV_GPS);
  			}
    		//printf("%s:left g_count = %d.\n",  __FUNCTION__, g_count);
    		sleep(sleepCOunter * (g_on2off_interval + g_off2on_interval));
    }
out:
	
	for(i = sizeof(stack)/sizeof(void *); i > 0; )
	{
		printf("%s:pid = %d, free stack information.\n",  __FUNCTION__, getpid());
		if (NULL != stack[--i])
		{
			free(stack[i]);
		}
		stack[i] = NULL;
	}
    if(g_wmt_fd > 0)
    {
    	close(g_wmt_fd);
    	g_wmt_fd  = -1;
    }
    printf("%s:pid = %d, exit.\n",  __FUNCTION__, getpid());
    return 0;
}
