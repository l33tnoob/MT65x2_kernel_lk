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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include "unistd.h"
#include <errno.h>
#include <termios.h>
#include <pthread.h>

#define MAX_UART_DEVICE 4

typedef struct __baudrate_mpping{
	unsigned int		ul_baud_rate;
	speed_t			linux_baud_rate;
}BAUD_RATE_SETTING;

pthread_mutex_t uart_mutex = PTHREAD_MUTEX_INITIALIZER;
extern int usb_plug_in;
extern int usb_com_port;
static BAUD_RATE_SETTING speeds_mapping[] = {
    {0		,B0		},
    {50		,B50		},
    {75		,B75		},
    {110	,B110		},
    {134	,B134,		},
    {150	,B150		},
    {200	,B200		},
    {300	,B300		},
    {600	,B600		},
    {1200	,B1200		},
    {1800	,B1800		},
    {2400	,B2400		},
    {4800	,B4800		},
    {9600	,B9600		},
    {19200	,B19200		},
    {38400	,B38400		},
    {57600	,B57600		},
    {115200	,B115200	},
    {230400	,B230400	},
    {460800	,B460800	},
    {500000	,B500000	},
    {576000	,B576000	},
    {921600	,B921600	},
    {1000000	,B1000000	}, 
    {1152000	,B1152000	}, 
    {1500000	,B1500000	}, 
    {2000000	,B2000000	}, 
    {2500000	,B2500000	}, 
    {3000000	,B3000000	}, 
    {3500000	,B3500000	}, 
    {4000000	,B4000000	},
};

static speed_t get_speed(unsigned int baudrate) 
{
	unsigned int idx;
	for (idx = 0; idx < sizeof(speeds_mapping)/sizeof(speeds_mapping[0]); idx++){
		if (baudrate == (unsigned int)speeds_mapping[idx].ul_baud_rate){
			return speeds_mapping[idx].linux_baud_rate;
		}
	}
	return CBAUDEX;        
}

#if 1
int set_baudrate_length_parity_stopbits(int fd, unsigned int new_baudrate, int length, char parity_c, int stopbits)
{
    struct termios uart_cfg_opt;
	speed_t speed;
	struct serial_struct ss;
	char  using_custom_speed = 0;
	
	if(-1==fd)
		return -1;

	/* Get current uart configure option */
	if(-1 == tcgetattr(fd, &uart_cfg_opt))
		return -1;

	tcflush(fd, TCIOFLUSH);

	/* Baud rate setting section */
	speed = get_speed(new_baudrate);
	if(CBAUDEX != speed){
		/*set standard buadrate setting*/
		cfsetospeed(&uart_cfg_opt, speed);
		cfsetispeed(&uart_cfg_opt, speed);
		printf("Standard baud\r\n");
	}else{
		printf("Custom baud\r\n");
		using_custom_speed = 1;
	}
	/* Apply baudrate settings */
	if(-1==tcsetattr(fd, TCSANOW, &uart_cfg_opt))
		return -1;
    
	/* Set time out */
	uart_cfg_opt.c_cc[VTIME] = 1;
	uart_cfg_opt.c_cc[VMIN] = 0;

	/*if((ioctl(fd,TIOCGSERIAL,&ss)) < 0)
		return -1;

	if(using_custom_speed){
		ss.flags |= ASYNC_SPD_CUST;  
        	ss.custom_divisor = 1<<31|new_baudrate;
        }else
        	ss.flags &= ~ASYNC_SPD_CUST;    

	if((ioctl(fd, TIOCSSERIAL, &ss)) < 0)
		return -1;//*/

	/* Data length setting section */
	uart_cfg_opt.c_cflag &= ~CSIZE;
	switch(length)
	{
	default:
	case 8:
		uart_cfg_opt.c_cflag |= CS8;
		break;
	case 5:
		uart_cfg_opt.c_cflag |= CS5;
		break;
	case 6:
		uart_cfg_opt.c_cflag |= CS6;
		break;
	case 7:
		uart_cfg_opt.c_cflag |= CS7;
		break;
	}

	/* Parity setting section */
	uart_cfg_opt.c_cflag &= ~(PARENB|PARODD);
	switch(parity_c)
	{
	default:
	case 'N':
	case 'n':
		uart_cfg_opt.c_iflag &= ~INPCK;
		break;
	case 'O':
	case 'o':
		uart_cfg_opt.c_cflag |= (PARENB|PARODD);
		uart_cfg_opt.c_iflag |= INPCK;
		break;
	case 'E':
	case 'e':
		uart_cfg_opt.c_cflag |= PARENB;
		uart_cfg_opt.c_iflag |= INPCK;
		break;
	}

	/* Stop bits setting section */
	if(2==stopbits)
		uart_cfg_opt.c_cflag |= CSTOPB;
	else
		uart_cfg_opt.c_cflag &= ~CSTOPB;

	/* Using raw data mode */
	uart_cfg_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	uart_cfg_opt.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
	uart_cfg_opt.c_oflag &=~(INLCR|IGNCR|ICRNL);
	uart_cfg_opt.c_oflag &=~(ONLCR|OCRNL);

	/* Apply new settings */
	if(-1==tcsetattr(fd, TCSANOW, &uart_cfg_opt))
		return -1;

	tcflush(fd,TCIOFLUSH);

	/* All setting applied successful */
	printf("setting apply done\r\n");
	return 0;
}
#endif
int open_uart_port(int uart_id, int baudrate, int length, char parity_c, int stopbits)
{
	int fd=-1;
	char dev[20]={0};
	/* The range of uart_id should from 1 to MAX_UART_DEVICE */ 
	if(uart_id == 0 || uart_id>MAX_UART_DEVICE)
		return fd;

	//sprintf(dev, "/dev/ttyMT%d", (int)(uart_id-1) );
	snprintf(dev,sizeof(dev),"%s%d","/dev/ttyMT", (int)(uart_id-1));
	/* Open device now */
	fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK);

	/* Check if the device handle is valid */
	if(-1 == fd)
		return fd;
	/* Apply settings */
	if(-1 == set_baudrate_length_parity_stopbits(fd, baudrate, length, parity_c, stopbits))
		return -1;
	/* Open success */
	return fd;
}

int open_usb_port(int uart_id, int baudrate, int length, char parity_c, int stopbits)
{
	int fd=-1;
	char dev[20]={0};
	/* The range of uart_id should from 1 to MAX_UART_DEVICE */ 
	if(uart_id == 0 || uart_id>MAX_UART_DEVICE)
		return fd;

	//sprintf(dev, "/dev/ttyMT%d", (int)(uart_id-1) );
	snprintf(dev,sizeof(dev),"%s%d","/dev/ttyGS", (int)(uart_id-1));
	/* Open device now */
	fd = open(dev, O_RDWR|O_NOCTTY|O_NONBLOCK);

	/* Check if the device handle is valid */
	if(-1 == fd)
		return fd;
	/* Apply settings */
	if(-1 == set_baudrate_length_parity_stopbits(fd, baudrate, length, parity_c, stopbits))
		return -1;
	/* Open success */
	return fd;
}


int  read_a_line(int fd, char rbuff[], unsigned int length)
{
	unsigned int has_read=0;
	ssize_t      ret_val;

	if(-1 == fd)
		return -1;
	
	LOGD("Enter read_a_line(): uart = %d\n", fd);
	memset (rbuff, 0, length);
	
	while(has_read<length){
		
loop:
	    usleep(20000);
		//LOGD("read_a_line uart_mutex try lock\n");
		if (pthread_mutex_lock (&uart_mutex))
		{
			LOGE( "read_a_line pthread_mutex_lock ERROR!\n"); 
		}
		//LOGD("read_a_line uart_mutex lock done\n");
		
		ret_val = read(fd, &rbuff[has_read], 1);

		//LOGD("read_a_line uart_mutex try unlock\n");
		if (pthread_mutex_unlock (&uart_mutex))
		{
			LOGE( "read_a_line pthread_mutex_unlock ERROR!\n"); 
		}
		//LOGD("read: %c, %d, %d", rbuff[has_read], ret_val, has_read);
		//LOGD("read_a_line uart_mutex unlock done\n");
		if(-1 == ret_val){
			if (errno == EAGAIN){
                //LOGD("UART1 can't read a byte!\n"); 
			}else
				LOGE("uart read fail! Error code = 0x%x\n", errno); 
			//continue;  
			goto loop;
		}
		
		if((rbuff[has_read]=='\r') || (rbuff[has_read]=='\n'))
            break;
		else
			has_read += (unsigned int)ret_val;

	}
	return has_read+1;
}

int  read_a_line_test(int fd, char rbuff[], unsigned int length)
{
	unsigned int has_read=0;
	ssize_t      ret_val;

	if(-1 == fd)
		return -1;
	
	LOGD("Enter read_a_line(): uart = %d\n", fd);
	memset (rbuff, 0, length);
	
	while(has_read<length){
		
loop:
    
//        LOGE("usb_plus_in = %d\n", usb_plus_in);
        if(usb_plug_in == 0)
        {
            break;
        }
	    usleep(20000);
		//LOGD("read_a_line uart_mutex try lock\n");
		if (pthread_mutex_lock (&uart_mutex))
		{
			LOGE( "read_a_line pthread_mutex_lock ERROR!\n"); 
		}
		//LOGD("read_a_line uart_mutex lock done\n");
		
		ret_val = read(usb_com_port, &rbuff[has_read], 1);

		//LOGD("read_a_line uart_mutex try unlock\n");
		if (pthread_mutex_unlock (&uart_mutex))
		{
			LOGE( "read_a_line pthread_mutex_unlock ERROR!\n"); 
		}
		//LOGD("read: %c, %d, %d", rbuff[has_read], ret_val, has_read);
		//LOGD("read_a_line uart_mutex unlock done\n");
		if(-1 == ret_val){
			if (errno == EAGAIN){
                //LOGD("UART1 can't read a byte!\n"); 
			}else
				LOGE("uart read fail! Error code = 0x%x\n", errno); 
			//continue;  
			goto loop;
		}
		
		if((rbuff[has_read]=='\r') || (rbuff[has_read]=='\n'))
            break;
		else
			has_read += (unsigned int)ret_val;

	}
	return has_read+1;
}


int write_chars(int fd, char wbuff[], unsigned int length)
{
    unsigned int has_write = 0;
    unsigned int refer_time,curr_time;
    ssize_t      ret_val;
    
    if(-1 == fd)
    	return -1;
    LOGD( "Enter write_chars()\n");
	
    /* Get refer time */
    refer_time = time(NULL);
    
    while(has_write < length){

#if 1
		//LOGD("write_chars uart_mutex try lock\n");
		if (pthread_mutex_lock (&uart_mutex))
		{
			LOGE( "write_chars pthread_mutex_lock ERROR!\n"); 
		}
		//LOGD("write_chars uart_mutex lock done\n");
#endif 
    	ret_val = write(fd, &wbuff[has_write], length-has_write);
#if 1
		//LOGD("write_chars uart_mutex try unlock\n");

		if (pthread_mutex_unlock (&uart_mutex))
		{
			LOGE( "write_chars pthread_mutex_unlock ERROR!\n"); 
		}
		//LOGD("write_chars uart_mutex unlock done\n");
#endif 

		if (-1==ret_val){
			LOGE( "write_chars write ERROR! Error code = 0x%x\n", errno); 
    		return -1;
    	}
    	has_write += (unsigned int)ret_val;
    	curr_time = (unsigned int)time(NULL);
    	if(curr_time - refer_time > 1)
    		break;
    }
    return has_write;
}
