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
#include <pthread.h>
#include <termios.h> 
#include <DfoDefines.h>

#include "libnvram.h"

#include "CFG_file_lid.h"
#include "CFG_PRODUCT_INFO_File.h"
#include "Custom_NvRam_LID.h"
#include "CFG_Wifi_File.h"
#include "CFG_BT_File.h"


#define TAG "[AT Command]"


#define BUF_SIZE 128
#define HALT_INTERVAL 20000

pthread_mutex_t ccci_mutex = PTHREAD_MUTEX_INITIALIZER;

#define CCCI_IOC_MAGIC 'C'
#define CCCI_IOC_ENTER_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 14) //CCI will not kill muxd/rild
#define CCCI_IOC_LEAVE_DEEP_FLIGHT _IO(CCCI_IOC_MAGIC, 15) //CCI will kill muxd/rild

#define rmmi_skip_spaces(source_string_ptr)                                  \
      while( source_string_ptr->string_ptr[ source_string_ptr->index ]       \
                                 == RMMI_SPACE )                             \
      {                                                                      \
        source_string_ptr->index++;                                          \
      }

#define RMMI_IS_LOWER( alpha_char )   \
  ( ( (alpha_char >= rmmi_char_a) && (alpha_char <= rmmi_char_z) ) ?  1 : 0 )

#define RMMI_IS_UPPER( alpha_char )   \
   ( ( (alpha_char >= RMMI_CHAR_A) && (alpha_char <= RMMI_CHAR_Z) ) ? 1 : 0 )

CMD_HDLR g_cmd_handler[] = 
						{ 
							{0, "AT+EABT", 0, 0, rmmi_eabt_hdlr}
							,{1, "AT+EAWIFI", 0, 0, rmmi_eawifi_hdlr}
							,{2, "AT+EANVBK", 0, 0, rmmi_eanvbk_hdlr}
							,{INVALID_ENUM, "AT+END", 0, 0, NULL}
						};


typedef enum {
	FALSE = 0,
	TRUE,
} _BOOL;
WIFI_CFG_PARAM_STRUCT g_wifi_nvram;
ap_nvram_btradio_mt6610_struct g_bt_nvram;
F_ID nvram_fd = {0};

static speed_t baud_bits[] = {
	0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600
};

void initTermIO(int portFd, int cmux_port_speed)
{
	struct termios uart_cfg_opt;
	tcgetattr(portFd, &uart_cfg_opt);
	tcflush(portFd, TCIOFLUSH);
	
	/*set standard buadrate setting*/
	speed_t speed = baud_bits[cmux_port_speed];
	cfsetospeed(&uart_cfg_opt, speed);
	cfsetispeed(&uart_cfg_opt, speed);
	
	uart_cfg_opt.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);   
	uart_cfg_opt.c_cflag |= CREAD | CLOCAL | CS8 ;
	
	/* Raw data */
	uart_cfg_opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	uart_cfg_opt.c_iflag &= ~(INLCR | IGNCR | ICRNL | IXON | IXOFF);
	uart_cfg_opt.c_oflag &=~(INLCR|IGNCR|ICRNL);
	uart_cfg_opt.c_oflag &=~(ONLCR|OCRNL);
	
	/* Non flow control */
	//uart_cfg_opt.c_cflag &= ~CRTSCTS; 			   /*clear flags for hardware flow control*/
	uart_cfg_opt.c_cflag |= CRTSCTS;
	uart_cfg_opt.c_iflag &= ~(IXON | IXOFF | IXANY); /*clear flags for software flow control*/		  
	
	// Set time out
	uart_cfg_opt.c_cc[VMIN] = 1;
	uart_cfg_opt.c_cc[VTIME] = 0;
	
	/* Apply new settings */
	if(tcsetattr(portFd, TCSANOW, &uart_cfg_opt)<0)
	{
		LOGD(TAG "set terminal parameter fail");
	}
	
	int status = TIOCM_DTR | TIOCM_RTS;
	ioctl(portFd, TIOCMBIS, &status);

}

bool is_support_modem(int modem)
{

    if(modem == 1)
    {
        if(MTK_ENABLE_MD1)
        {
            return true;
        }
    }else if(modem == 2)
    {
        if(MTK_ENABLE_MD2)
        {
            return true;
        }
    }else if(modem == 3){
        if(MTK_DT_SUPPORT)
        {
            return true;
        }
    }
    if(modem == 5)
    {

        if(MTK_ENABLE_MD5)
        {
            return true;
        }


    }

    return false;
}


int send_at (const int fd, const char *pCMD)
{
    int ret = 0, sent_len = 0;
    LOGD(TAG "Send AT CMD: %s\n", pCMD);
		
    while (sent_len != strlen(pCMD))
    {
    	//LOGD("send_at ccci_mutex try lock\n");
	   	if (pthread_mutex_lock (&ccci_mutex))
		{
			LOGE( "send_at pthread_mutex_lock ERROR!\n"); 
		}
		//LOGD("send_at ccci_mutex lock done\n");

		ret = write(fd, pCMD, strlen(pCMD));


		//LOGD("send_at ccci_mutex try unlock\n");
		if (pthread_mutex_unlock (&ccci_mutex))
		{
			LOGE( "send_at pthread_mutex_unlock ERROR!\n"); 
		}
		//LOGD("send_at ccci_mutex unlock done\n");
		
        if (ret<0)
        {
        	LOGE("ccci write fail! Error code = 0x%x\n", errno); 
            return ret;
        }
        else
        {	
            sent_len += ret;
            LOGD(TAG "[send_at] lenth = %d\n", sent_len);
        }
    }
    return 0;
		
}

int read_ack (const int fd, char *rbuff, int length)
{
	unsigned int has_read = 0;
	ssize_t      ret_val;

	if(-1 == fd)
		return -1;
	
	LOGD("Enter read_ack(): uart = %d\n", fd);
	memset (rbuff, 0, length);
	
#if 1
	while(has_read<length)
	{

loop:
		usleep(HALT_INTERVAL);
		LOGD("read_ack ccci_mutex try lock\n");
		if (pthread_mutex_lock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_lock ERROR!\n"); 
		}
		LOGD("read_ack ccci_mutex lock done\n");
		
		ret_val = read(fd, &rbuff[has_read], length);
		LOGD("read_ack ccci_mutex try unlock\n");
		if (pthread_mutex_unlock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_unlock ERROR!\n"); 
		}
		LOGD("read_ack ccci_mutex unlock done\n");
		LOGD("ret_val %d",ret_val);
		if(-1 == ret_val)
		{
			if (errno == EAGAIN)
			{
                LOGD("ccci can't read a byte!\n"); 
			}
			else
				LOGE("ccci read fail! Error code = 0x%x\n", errno); 
			
			//continue;  
			goto loop;
		}
		
		//if( (rbuff[has_read]!='\r')&&(rbuff[has_read]!='\n') )
		if(ret_val>2)
		{
		has_read += (unsigned int)ret_val;
		if (strstr(rbuff, "\r\n"))  break;
	}
		

	}
	LOGD("read_ack %s",rbuff);
	return has_read;
#endif

}

int wait4_ack2 (const int fd, char *pACK, char *pACK1,int timeout)
{
    char buf[BUF_SIZE] = {0};
	char *  p = NULL;
    int rdCount = 0, LOOP_MAX;
    int ret = -1;

    LOOP_MAX = timeout*1000/HALT_INTERVAL;

    LOGD(TAG "Wait for AT ACK...: %s; Special Pattern: %s\n", buf, (pACK==NULL)?"NULL":pACK);

    for(rdCount = 0; rdCount < LOOP_MAX; ++rdCount) 
    {
		memset(buf,'\0',BUF_SIZE);    
		if (pthread_mutex_lock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_lock ERROR!\n"); 
		}
		ret = read(fd, buf, BUF_SIZE);
		if (pthread_mutex_unlock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_unlock ERROR!\n"); 
		}
		
        LOGD(TAG "AT CMD ACK: %s.rdCount=%d\n", buf,rdCount);
        p = NULL;


		if ((pACK != NULL) && (pACK1 != NULL))  
        {

			p = strstr(buf, "ERROR");
        	if(p) 
            {
            	ret = -1; 
                break;
        	}
			p = strstr(buf, "NO CARRIER");
        	if(p) 
            {
            	ret = -1;
                break;
        	}
            p = strstr(buf, pACK);
            if(p) 
            {
              	ret = 0; 
                break; 
            }
            p = strstr(buf, pACK1);
            if(p != NULL)
            {
                ret = 0; 
                break;
            }
	
        }
		else
		{
			p = strstr(buf, "OK");
        	if(p) {
            LOGD(TAG "Char before OK are %c,%c.\n",*(p-2),*(p-1) );			
            if(*(p-2) == 'E' && *(p-1) == 'P'){
                LOGD(TAG "EPOK detected\n");			
            }else{	
                LOGD(TAG "OK response detected\n");			            
            	ret = 0; break;
        	}
        }
        	p = strstr(buf, "ERROR");
        	if(p) {
            	ret = -1; break;
        	}
			p = strstr(buf, "NO CARRIER");
        	if(p) {
            	ret = -1; break;
        	}
		}
	
        usleep(HALT_INTERVAL);

    }
	LOGD("ret = %d",ret);
    return ret;
}


int wait4_ack (const int fd, char *pACK, int timeout)
{
    char buf[BUF_SIZE] = {0};
	char *  p = NULL;
    int rdCount = 0, LOOP_MAX;
    int ret = -1;

    LOOP_MAX = timeout*1000/HALT_INTERVAL;

    LOGD(TAG "Wait for AT ACK...: %s; Special Pattern: %s\n", buf, (pACK==NULL)?"NULL":pACK);

    for(rdCount = 0; rdCount < LOOP_MAX; ++rdCount) 
    {
		memset(buf,'\0',BUF_SIZE);    
		if (pthread_mutex_lock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_lock ERROR!\n"); 
		}
		ret = read(fd, buf, BUF_SIZE);
		if (pthread_mutex_unlock (&ccci_mutex))
		{
			LOGE( "read_ack pthread_mutex_unlock ERROR!\n"); 
		}
		
        LOGD(TAG "AT CMD ACK: %s.rdCount=%d\n", buf,rdCount);
        p = NULL;


		if (pACK != NULL)  
        {
	          p = strstr(buf, pACK);
              if(p) {
              	ret = 0; break; 
              }
			  p = strstr(buf, "ERROR");
        	  if(p) {
            	ret = -1; break;
        	  }
			  p = strstr(buf, "NO CARRIER");
        	  if(p) {
            	ret = -1; break;
        	  }
	
        }
		else
		{
			p = strstr(buf, "OK");
        	if(p) {
            LOGD(TAG "Char before OK are %c,%c.\n",*(p-2),*(p-1) );			
            if(*(p-2) == 'E' && *(p-1) == 'P'){
                LOGD(TAG "EPOK detected\n");			
            }else{	
                LOGD(TAG "OK response detected\n");			            
            	ret = 0; break;
        	}
        }
        	p = strstr(buf, "ERROR");
        	if(p) {
            	ret = -1; break;
        	}
			p = strstr(buf, "NO CARRIER");
        	if(p) {
            	ret = -1; break;
        	}
		}
	
        usleep(HALT_INTERVAL);

    }
	LOGD("ret = %d",ret);
    return ret;
}

#ifdef MTK_ENABLE_MD1
int openDevice(void)
{
    int fd;
  
    fd = open("/dev/ttyC0", O_RDWR | O_NONBLOCK);
    if(fd < 0) {
        LOGD(TAG "Fail to open ttyC0: %s\n", strerror(errno));
        return -1;
    }
    // +EIND will always feedback +EIND when open device,
    // so move this to openDevice.	
    // +EIND
    //wait4_ack (fd, "+EIND", 3000);
    return fd;
}
#endif

int openDeviceWithDeviceName(char *deviceName)
{
    LOGD(TAG "%s - %s\n", __FUNCTION__, deviceName);
    int fd;
    fd = open(deviceName, O_RDWR | O_NONBLOCK);
    if(fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n", deviceName, strerror(errno));
        return -1;
    }

    return fd;
}

int ExitFlightMode_DualTalk(int fd,_BOOL bON)
{

	if (bON)
	{
		
		LOGD(TAG "[AT]Disable Sleep Mode:\n");
		if (send_at (fd, "AT+ESLP=0\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;

		LOGD(TAG "[AT]Reset SIM1:\n");
		if (send_at (fd, "AT+ESIMS\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;
		if (send_at (fd, "AT+CFUN=1\r\n")) goto err;
		wait4_ack (fd, NULL, 5000);
		if (send_at (fd, "AT+CREG=1\r\n")) goto err;
		wait4_ack (fd, "+CREG", 15000);

	}else
	{
		
		send_at (fd, "ATH\r\n");
		wait4_ack (fd, NULL, 3000);

		LOGD(TAG "[AT]Enable Sleep Mode:\n");
		if (send_at (fd, "AT+ESLP=1\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;
		if (send_at (fd, "AT+CFUN=4\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;
	}
	return 0;
	err:
	return -1;

}


int ExitFlightMode (int fd, _BOOL bON)
{
	static bInit = FALSE;

	LOGD(TAG "[AT]Detect MD active status:\n");
	do
	{
		send_at (fd, "AT\r\n");
	} while (wait4_ack (fd, NULL, 300));


	if (bON)
	{		
		LOGD(TAG "[AT]Disable Sleep Mode:\n");
		if (send_at (fd, "AT+ESLP=0\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;

#ifdef GEMINI
		if (bInit == FALSE)
		{
			LOGD(TAG "[AT]Reset SIM1:\n");
			if (send_at (fd, "AT+ESIMS\r\n")) goto err;
			if (wait4_ack (fd, NULL, 3000)) goto err;

			LOGD(TAG "[AT]Switch to UART2:\n");
			if (send_at (fd, "AT+ESUO=5\r\n")) goto err;
			if (wait4_ack (fd, NULL, 3000)) goto err;

			LOGD(TAG "[AT]Reset SIM2:\n");
			if (send_at (fd, "AT+ESIMS\r\n")) goto err;
			if (wait4_ack (fd, NULL, 2000)) goto err;
			
	
			LOGD(TAG "[AT]Switch to UART1:\n");
			if (send_at (fd, "AT+ESUO=4\r\n")) goto err;
			if (wait4_ack (fd, NULL, 3000)) goto err;
			bInit = TRUE;
}

		LOGD(TAG "[AT]Turn ON RF:\n");
		#if defined(MTK_GEMINI_3SIM_SUPPORT)

		if (send_at (fd, "AT+EFUN=7\r\n")) goto err;

		#elif defined(MTK_GEMINI_4SIM_SUPPORT)

		if (send_at (fd, "AT+EFUN=f\r\n")) goto err;

		#else
		if (send_at (fd, "AT+EFUN=3\r\n")) goto err;
		#endif
		if (send_at (fd, "AT+CREG=1\r\n")) goto err;

		
#else
		if (bInit == FALSE)
		{
			LOGD(TAG "[AT]Reset SIM1:\n");
			if (send_at (fd, "AT+ESIMS\r\n")) goto err;
			if (wait4_ack (fd, NULL, 3000)) goto err;
			bInit = TRUE;
		}
		
		LOGD(TAG "[AT]Turn ON RF:\n");
		if (send_at (fd, "AT+EFUN=1\r\n")) goto err;
		if (send_at (fd, "AT+CREG=1\r\n")) goto err;

#endif

		wait4_ack (fd, "+CREG", 15000);

	}else
	{
		LOGD(TAG "[AT]Enable Sleep Mode:\n");
		if (send_at (fd, "AT+ESLP=1\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;

#ifdef GEMINI
		LOGD(TAG "[AT]Switch to UART1:\n");
		if (send_at (fd, "AT+ESUO=4\r\n")) goto err;
		if (wait4_ack (fd, NULL, 3000)) goto err;
	
		LOGD(TAG "[AT]Turn OFF RF:\n");
		if (send_at (fd, "AT+EFUN=0\r\n")) goto err;
#else
		LOGD(TAG "[AT]Turn OFF RF:\n");
		if (send_at (fd, "AT+CFUN=4\r\n")) goto err;
#endif
		if (wait4_ack (fd, NULL, 3000)) goto err;
	}
	return 0;
err:
	return -1;
}


/* turn on/off flight mode function, this function is for flight mode power-off modem feature */
int ExitFlightMode_PowerOffModem(int fd, int ioctl_fd, _BOOL bON){
    LOGD(TAG "[AT]ExitFlightMode_PowerOffModem\n");

    if(bON) {		
        LOGD(TAG "[AT]CCCI_IOC_LEAVE_DEEP_FLIGHT \n");		
        int ret_ioctl_val = ioctl(ioctl_fd, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_LEAVE_DEEP_FLIGHT);
		
    } else {
        do
        {
            send_at (fd, "AT\r\n");
        } while (wait4_ack (fd, NULL, 300));

        LOGD(TAG "[AT]Enable Sleep Mode:\n");		
        if (send_at (fd, "AT+ESLP=1\r\n")) goto err;
        if (wait4_ack (fd, NULL, 3000)) goto err;

        LOGD(TAG "[AT]Power OFF Modem:\n");
        if (send_at (fd, "AT+EFUN=0\r\n")) goto err;
        wait4_ack (fd, NULL, 15000); 
        if (send_at (fd, "AT+EPOF\r\n")) goto err;
        wait4_ack (fd, NULL, 10000);
		
        LOGD(TAG "[AT]CCCI_IOC_ENTER_DEEP_FLIGHT \n");		

        int ret_ioctl_val = ioctl(ioctl_fd, CCCI_IOC_ENTER_DEEP_FLIGHT);
        LOGD("[AT]CCCI ioctl result: ret_val=%d, request=%d", ret_ioctl_val, CCCI_IOC_ENTER_DEEP_FLIGHT);
    }

err:
    return -1;
}

void closeDevice(int fd)
{
    close(fd);
}



/********************
       ATD112;
********************/
const char* dial112(const int fd)
{
    LOGD(TAG "%s start\n", __FUNCTION__);
	static _BOOL bCMD_Sent = FALSE;



    if(fd < 0)
	{
    	LOGD(TAG "Invalid fd\n");
    	return STR_ERROR;
    }
     

	LOGD(TAG "[AT]Dail Up 112:\n");
	send_at (fd, "ATH\r\n");
	wait4_ack (fd, NULL, 3000);

	LOGD(TAG ">>>>>>>>> ATH done:\n");

    if(send_at(fd, "AT+ECPI=4294967295;\r\n"))
    {

        LOGD(TAG ">>>>>>>>> send AT+ECPI\n");
		
		goto err;
        
    }

    if (wait4_ack (fd, NULL, 15000)) 
	{
		
			LOGD(TAG ">>>>>>>>> wait for OK error\n");
		
			goto err;
		
	}

	if (send_at (fd, "ATD112;\r\n"))
	{
		
			LOGD(TAG ">>>>>>>>> send ATD112 error\n");
		
			goto err;
		
	}
	LOGD(TAG ">>>>>>>>> send ATD112 done:\n");


	if (wait4_ack (fd, NULL, 15000)) 
		{
		
			LOGD(TAG ">>>>>>>>> wait for OK error\n");
		
			goto err;
		
	}
	LOGD(TAG ">>>>>>>>> wait for OK done:\n");
	
	if(wait4_ack2(fd, "+ECPI: 1,2", "ESPEECH", 15000))
		{
		
			LOGD(TAG ">>>>>>>>> wait for +ECPI: 1,2 error\n");
		
			goto err;
		
		}

	send_at (fd, "ATH\r\n");
	
	if(wait4_ack (fd, NULL, 3000))
	{
		
		LOGD(TAG ">>>>>>>>> wait for hang up error\n");

		goto err;
		

	}

	LOGD(TAG "%s end\n", __FUNCTION__);
	return STR_OK;
err:
	return STR_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  rmmi_int_validator_ext
 * DESCRIPTION
 *  
 * PARAMETERS
 *  error_cause             [?]         
 *  source_string_ptr       [?]         
 *  delimiter               [IN]        
 * RETURNS
 *  
 *****************************************************************************/
unsigned int rmmi_int_validator_ext(rmmi_validator_cause_enum *error_cause, rmmi_string_struct *source_string_ptr, unsigned char delimiter)
{
    unsigned int ret_val = RMMI_VALIDATOR_ERROR;
    unsigned int value = 0;
    unsigned int length;
    bool error_flag = false;
    bool some_char_found = false;

    //kal_trace(TRACE_FUNC, FUNC_RMMI_INT_VALIDATOR_ENTRY);
    length = strlen((char*)source_string_ptr->string_ptr);

    /* If there are some leading white spaces, ignore them */
    rmmi_skip_spaces(source_string_ptr);

    /*
     * we have to initial the error so that we can using again and
     * again even if any error occur. so we dont have to init before
     * enter this function
     */
    *error_cause = RMMI_PARSE_OK;

    /*
     * Start checking for the integer, till the delimiter which may
     * * be a comma, a dot etc.
     */

    while ((source_string_ptr->string_ptr[source_string_ptr->index]
            != delimiter)
           &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != S3) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_END_OF_STRING_CHAR) && (source_string_ptr->index < length))
    {
        /* It means we found something between two commas(,)  */
        some_char_found = true;

        /*
         * check whether the character is in 0 - 9 range. If so,
         * * store corresponding integer value for that character
         */
        if ((source_string_ptr->string_ptr[source_string_ptr->index]
             >= RMMI_CHAR_0) && (source_string_ptr->string_ptr[source_string_ptr->index] <= RMMI_CHAR_9))
        {
            value = value * 10 + (source_string_ptr->string_ptr[source_string_ptr->index] - RMMI_CHAR_0);
        }
        else    /* out of range, return immediately */
        {
            error_flag = true;
            break;
        }
        /* If the character is a valid part of integer, then continue */
        source_string_ptr->index++;
    }   /* end of the while loop */

    if (error_flag == true)
    {
        /*
         * Value is not in the valid range. It can also be due to
         * * white space in between two digits, because such white
         * * spaces are not allowed
         */
        /* mark for solve correct input but incorrect end for 1,2,2, */
        /* rmmi_result_code_fmttr (  RMMI_RCODE_ERROR,
           INVALID_CHARACTERS_IN_TEXT_ERRSTRING_ERR ); */
        ret_val = RMMI_VALIDATOR_ERROR;
        *error_cause = RMMI_PARSE_ERROR;
    }
    else if (some_char_found == false)
    {
        /* Nothing is present before the delimiter */
        ret_val = RMMI_VALIDATOR_ERROR;
        *error_cause =  RMMI_PARSE_NOT_FOUND;

        /*
         * Increment the string sliding index to point to the next
         * * character after delimiter, i.e. the next field in the
         * * command line
         */
        source_string_ptr->index++;
    }
    /*
     * If some thing is present and check for the valid range as
     * * specified by the calling function
     */
    else
    {
        ret_val = value;
        /*
         * Increment the string sliding index to point to the next
         * * character after delimiter, i.e. the next field in the
         * * command line
         */
        if (source_string_ptr->string_ptr[source_string_ptr->index] == delimiter)
        {
            source_string_ptr->index++;
            rmmi_skip_spaces(source_string_ptr);
            if (source_string_ptr->string_ptr[source_string_ptr->index] == S3&&
		  source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)
            {
                ret_val = RMMI_VALIDATOR_ERROR;
                *error_cause =  RMMI_PARSE_ERROR;
            }
        }
        else
        {
            source_string_ptr->index++;
        }
    }
    return ret_val;
}

void get_len(char *str, int *length)
{
    int i = 0;
    while(str[i] != RMMI_END_OF_STRING_CHAR)
    {
        i++;
    }
    *length = i;
    return;
}

static int convStrtoHex(char*  szStr, unsigned char* pbOutput, int dwMaxOutputLen, int*  pdwOutputLen){
    LOGD("Entry %s\n", __FUNCTION__);

    int   dwStrLen;        
    int   i = 0;
    unsigned char ucValue = 0;
    LOGD("before strlen,dwStrLen\n");
    while(szStr[i] != '\0')
    {
        LOGD("szStr[%d]:%c", i, szStr[i]);
        i++;
    }

    LOGD("after while loop\n");
    dwStrLen = strlen(szStr);
//	dwStrLen = i;
	LOGD("after strlen, dwStrLen = %d\n", dwStrLen);
//    LOGD("after strlen,dwStrLen %d\n", dwStrLen);

    if(dwMaxOutputLen < dwStrLen/2){
        return -1;
    }
    i = 0;
    for (i = 0; i < dwStrLen; i ++){
        
    LOGD("in for loop %c\n", szStr[i]);
        switch(szStr[i]){
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                ucValue = (ucValue * 16) + (szStr[i] -  '0');
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                ucValue = (ucValue * 16) + (szStr[i] -  'a' + 10);
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                ucValue = (ucValue * 16) + (szStr[i] -  'A' + 10);
                break;
            default:
                return -1;
                break;
        }

        if(i & 0x01){
            pbOutput[i/2] = ucValue;
            LOGD("int pbOutput:%d, ucValue:%d\n", pbOutput[i/2], ucValue);
            LOGD("int pbOutput:%02x, ucValue:%02x\n", pbOutput[i/2], ucValue);
            ucValue = 0;
        }
    }

    *pdwOutputLen = i/2;
    LOGD("Leave %s\n", __FUNCTION__);  

    return 0;
}

void get_write_value(rmmi_string_struct *cmd, char *write_value)
{
    LOGD("Entry %s\n", __FUNCTION__);
    int i = 0;
    while(cmd->string_ptr[cmd->index] != RMMI_DOUBLE_QUOTE)
    {
        cmd->index++;
    }
    cmd->index++;
    while(cmd->string_ptr[cmd->index] != RMMI_DOUBLE_QUOTE)
    {
        write_value[i] = cmd->string_ptr[cmd->index];
        i++;
        cmd->index++;
    }
    LOGD("Leave %s\n", __FUNCTION__);    
}


void rmmi_eabt_hdlr (rmmi_string_struct* cmd, char *addr)
{
    char output[bt_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
	unsigned char w_bt[bt_length];
    int length;
    int ret, i = 0;
    char value[13] = {0};
    memset(value, 0, sizeof(value));
	if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
    {   
        sprintf(cmd->result, "%s", return_err);
		return;
    }
	switch (cmd->cmd_mode)
	{
	    case   RMMI_SET_OR_EXECUTE_MODE: //AT+EABT=1\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_SET_OR_EXECUTE_MODE,cmd:%s,%c\n", cmd->string_ptr, cmd->string_ptr[cmd->index+2]);
            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			memset(&g_bt_nvram,0,rec_num*rec_size);
   			ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:read bt addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
                   g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
            );
   			NVM_CloseFileDesc(nvram_fd);
   			nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISWRITE);
   			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			memset(g_bt_nvram.addr,0,bt_length);
   			memset(w_bt,0,bt_length);
            get_write_value(cmd, value);
            length = strlen(value);
    	    if(length != 12)
		    {
			    LOGD("error:bt address length is not right!\n");
                sprintf(cmd->result, "%s", return_err);
			    return -1;
		    }
		    ret = convStrtoHex(value,output,bt_length,&length);
		    if(-1 == ret)
		    {
			    LOGD("error:convert bt address to hex fail\n");
                sprintf(cmd->result, "%s", return_err);
			    return -1;
		    }
            else
            {
                LOGD("BT Address:%s\n", output);
            }
   			for(i=0;i<bt_length;i++)
   			{	
   				g_bt_nvram.addr[i] = output[i];
   			}
   			LOGD("write bt addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
                    g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5],
                    output[0], output[1], output[2], output[3], output[4], output[5]
                    );
   			ret = write(nvram_fd.iFileDesc, &g_bt_nvram , rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:write wifi addr fail!\n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			NVM_CloseFileDesc(nvram_fd);
   			LOGD("write bt addr success!\n");
   			if(FileOp_BackupToBinRegion_All())
   			{
   				LOGD("backup nvram data to nvram binregion success!\n");
                sprintf(cmd->result, "%s", return_ok);
   			}
   			else
   			{
   				LOGD("error:backup nvram data to nvram binregion fail!\n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			sync();
			break;
    	case RMMI_READ_MODE:                    //AT+EABT?\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_READ_MODE");
             nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_BT_ADDR_LID, &rec_size, &rec_num, ISREAD);
  			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			if(sizeof(g_bt_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			memset(&g_bt_nvram,0,rec_num*rec_size);
   			ret = read(nvram_fd.iFileDesc, &g_bt_nvram, rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:read bt addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			LOGD("read pre bt addr:%02x%02x%02x%02x%02x%02x\n", 
                   g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5] 
            );
//            memcpy(addr, g_bt_nvram.addr, sizeof(g_bt_nvram.addr));
            sprintf(cmd->result, "%s%02x%02x%02x%02x%02x%02x%s%s", "\n\r+EABT:\"", g_bt_nvram.addr[0], g_bt_nvram.addr[1], g_bt_nvram.addr[2], g_bt_nvram.addr[3], g_bt_nvram.addr[4], g_bt_nvram.addr[5], "\"\n\r", return_ok);
   			NVM_CloseFileDesc(nvram_fd);
			break;
    	case RMMI_TEST_MODE:                     //AT+EABT=?\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_TEST_MODE");
            sprintf(cmd->result, "%s%s", "\r\n+EABT:(0,1)(1)", return_ok);
			break;
    	case RMMI_ACTIVE_MODE:                 //AT+EABT\r
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_ACTIVE_MODE");
            sprintf(cmd->result, "%s", return_err);
			break;
				
    	case RMMI_WRONG_MODE:
   		    LOGD(TAG "rmmi_eabt_hdlr:RMMI_WRONG_MODE");
            sprintf(cmd->result, "%s", return_err);
            break;
		default:
			return;
	}	return;
}
void rmmi_eawifi_hdlr (rmmi_string_struct* cmd, char *addr)
{
    char output[wifi_length] = {0};
    int rec_size = 0;
    int rec_num = 0;
	unsigned char w_wifi[wifi_length];
    int ret, length = 0, i = 0;
    char value[13] = {0};
	if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
		return;
	switch (cmd->cmd_mode)
	{
   		case   RMMI_SET_OR_EXECUTE_MODE: //AT+EAWIFI=1\r
   		    LOGD(TAG "rmmi_eawifi_hdlr:RMMI_SET_OR_EXECUTE_MODE");
            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
            printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    		if(1 != rec_num)
    		{
    			printf("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
    			return -1;
    		}
    		if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    		{
    			printf("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
    			return -1;
    		}
    		memset(&g_wifi_nvram,0,rec_num*rec_size);
    		ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    		if(-1 == ret||rec_num*rec_size != ret)
    		{
    			printf("error:read wifi mac addr fail!/n");
                sprintf(cmd->result, "%s", return_err);
    			return -1;
    		}
    		printf("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
    		g_wifi_nvram.aucMacAddress[5]);
    		NVM_CloseFileDesc(nvram_fd);

            nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISWRITE);
   			LOGD("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
   			if(1 != rec_num)
   			{
   				LOGD("error:unexpected record num %d\n",rec_num);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			if(sizeof(g_wifi_nvram) != rec_size)
   			{
   				LOGD("error:unexpected record size %d\n",rec_size);
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			memset(g_wifi_nvram.aucMacAddress,0,bt_length);
//   			memset(w_bt,0,bt_length);
            get_write_value(cmd, value);
            length = strlen(value);
    	    if(length != 12)
		    {
			    LOGD("error:bt address length is not right!\n");
                sprintf(cmd->result, "%s", return_err);
			    return -1;
		    }
		    ret = convStrtoHex(value,output,wifi_length,&length);
		    if(-1 == ret)
		    {
			    LOGD("error:convert wifi address to hex fail\n");
                sprintf(cmd->result, "%s", return_err);
			    return -1;
		    }
            else
            {
                LOGD("WIFI Address:%s\n", output);
            }
   			for(i=0;i<bt_length;i++)
   			{	
   				g_wifi_nvram.aucMacAddress[i] = output[i];
   			}
   			LOGD("write wifi addr:%02x%02x%02x%02x%02x%02x, value:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], 
                    g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
                    g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5],
                    output[0], output[1], output[2], output[3], output[4], output[5]
                    );
   			ret = write(nvram_fd.iFileDesc, &g_wifi_nvram , rec_num*rec_size);
   			if(-1 == ret||rec_num*rec_size != ret)
   			{
   				LOGD("error:write wifi addr fail!\n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			NVM_CloseFileDesc(nvram_fd);
   			LOGD("write wifi addr success!\n");
   			if(FileOp_BackupToBinRegion_All())
   			{
   				LOGD("backup nvram data to nvram binregion success!\n");
                sprintf(cmd->result, "%s", return_ok);
   			} 
   			else
   			{
   				LOGD("error:backup nvram data to nvram binregion fail!\n");
                sprintf(cmd->result, "%s", return_err);
   				return -1;
   			}
   			sync();
			break;
    		case   RMMI_READ_MODE:                    //AT+EAWIFI?\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_READ_MODE");
    			nvram_fd = NVM_GetFileDesc(AP_CFG_RDEB_FILE_WIFI_LID, &rec_size, &rec_num, ISREAD);
    			printf("rec_size=%d,rec_num=%d\n",rec_size,rec_num);
    			if(1 != rec_num)
    			{
    				LOGD("error:unexpected record num %d\n",rec_num);
                    sprintf(cmd->result, "%s", return_err);
    				return -1;
    			}
    			if(sizeof(WIFI_CFG_PARAM_STRUCT) != rec_size)
    			{
    				LOGD("error:unexpected record size %d\n",rec_size);
                    sprintf(cmd->result, "%s", return_err);
    				return -1;
    			}
    			memset(&g_wifi_nvram,0,rec_num*rec_size);
    			ret = read(nvram_fd.iFileDesc, &g_wifi_nvram, rec_num*rec_size);
    			if(-1 == ret||rec_num*rec_size != ret)
    			{
    				LOGD("error:read wifi mac addr fail!/n");
                    sprintf(cmd->result, "%s", return_err);
    				return -1;
    			}
    			LOGD("read wifi addr:%02x%02x%02x%02x%02x%02x\n", 
                    g_wifi_nvram.aucMacAddress[0], g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], g_wifi_nvram.aucMacAddress[4], 
    			g_wifi_nvram.aucMacAddress[5]);
//                memcpy(addr, g_wifi_nvram.aucMacAddress, sizeof(g_wifi_nvram.aucMacAddress));
                sprintf(cmd->result, "%s%02x%02x%02x%02x%02x%02x%s%s", "\r\n+EAWIFI:\"", g_wifi_nvram.aucMacAddress[0], 
                g_wifi_nvram.aucMacAddress[1], g_wifi_nvram.aucMacAddress[2], g_wifi_nvram.aucMacAddress[3], 
                g_wifi_nvram.aucMacAddress[4], g_wifi_nvram.aucMacAddress[5], "\"\r\n", return_ok);
    			NVM_CloseFileDesc(nvram_fd);
			break;
    		case   RMMI_TEST_MODE:                     //AT+EAWIFI=?\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_TEST_MODE");
                sprintf(cmd->result, "%s%s", "\r\n+EAWIFI:(0,1)(1)", return_ok);
			break;
    		case   RMMI_ACTIVE_MODE:                 //AT+EAWIFI\r
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_ACTIVE_MODE");
                sprintf(cmd->result, "%s", return_err);
			break;
				
    		case   RMMI_WRONG_MODE:
   		        LOGD(TAG "rmmi_eawifi_hdlr:RMMI_WRONG_MODE");
                sprintf(cmd->result, "%s", return_err);
                break;
		default:
            
                sprintf(cmd->result, "%s", return_err);
			    return;
	}

}
void rmmi_eanvbk_hdlr (rmmi_string_struct* cmd)
{
	 char *rsp_str = NULL;
	 rmmi_validator_cause_enum err_code;
	  unsigned int ret =  RMMI_VALIDATOR_ERROR;
	  
	if (cmd->cmd_owner != CMD_OWENR_AP || cmd->cmd_class != RMMI_EXTENDED_CMD)
		return;
	switch (cmd->cmd_mode)
	{
   		case   RMMI_SET_OR_EXECUTE_MODE: //AT+EANVBK=1\r
   		        LOGD(TAG "rmmi_eanvbk_hdlr:RMMI_SET_OR_EXECUTE_MODE");
   			ret = rmmi_int_validator_ext(&err_code, cmd, RMMI_COMMA);
			if (ret == RMMI_VALIDATOR_ERROR)
			{
			    LOGD("RMMI_VALIDATOR_ERROR\n");
                sprintf(cmd->result, "%s", return_err);
				goto err;
			}
			else if (ret != 1)
			{
			    LOGD("ret != 1\n");
                sprintf(cmd->result, "%s", return_err);
				goto err;
			} else
			{
			    LOGD("Backup nvram!\n");
   				if(FileOp_BackupToBinRegion_All())
   			    {
   				    LOGD("backup nvram data to nvram binregion success!\n");
    		        //sprintf(cmd->result, "%s%s", "\r\n+EANVBK:", return_ok);  //return Parameter
    		        sprintf(cmd->result, "%s", return_ok);
   			    }
   			    else
   			    {
   				    LOGD("error:backup nvram data to nvram binregion fail!\n");
                    sprintf(cmd->result, "%s", return_err);
   				    return -1;
   			    }
   			    sync(); 
			}
			break;
    		case   RMMI_TEST_MODE:                     //AT+EANVBK=?\r
    		    sprintf(cmd->result, "%s%s", "\r\n+EANVBK:(1)", return_ok);  //return Parameter
			break;
err:				
		case   RMMI_READ_MODE:                    //AT+EANVBK?\r
		case   RMMI_ACTIVE_MODE:                 //AT+EANVBK\r
    	case   RMMI_WRONG_MODE:
		default:
            sprintf(cmd->result, "%s", return_err);
	}

	//send ack to pc
	
}



int calc_hashvalue (char *string_ptr, unsigned int *hash_value1, unsigned int *hash_value2)

{
    unsigned int counter = 0;
    int index = 3; // at+XXXX => XXXX->hash value

    /* This variable is used to ensure that the unsigned ints coming on line for
    command are properly broken into set of 5 unsigned ints. So, the correct
    hash value can be calculated based on the parser expression and
    respectively stored in the variables hash_value1 and hash_value2 */
    unsigned int ascii;

    /* Variable used to store the calculated hash value of 2nd 5 unsigned intacters
    as a part of a extended command */
    unsigned int cmd_found = false;

    /* if command is found, cmd_index is the command enum */
    *hash_value1 = 0;

    /* Variable used to store the calculated hash value of 1st 5 unsigned intacters
       as a part of a extended command */
    *hash_value2 = 0;
    
   /* Parser for Extended AT commands */
    while ((string_ptr[index]
            != RMMI_EQUAL) &&
           (string_ptr[index]
            != RMMI_QUESTION_MARK) &&
           (string_ptr[index]
            != RMMI_COMMA) &&
           (string_ptr[index]
            != RMMI_SEMICOLON) &&
           (string_ptr[index]
            != RMMI_END_OF_STRING_CHAR)&&
           (counter <= RMMI_MAX_EXT_CMD_NAME_LEN) &&
           (string_ptr[index]
            != S3) && (string_ptr[index] != S4))
    {
	 
	 if (RMMI_IS_UPPER(string_ptr[index]))
        {
            ascii = string_ptr[index] - RMMI_CHAR_A;
        }

        else if (RMMI_IS_LOWER(string_ptr[index]))
        {
            ascii = string_ptr[index] - rmmi_char_a;
        }

    #ifdef __CS_SERVICE__
        else if (RMMI_IS_NUMBER(string_ptr[index]))
        {
            ascii = string_ptr[index] - RMMI_CHAR_0 + RMMI_NUMBER_OFFSET_IN_PARSER_TABLE;
        }
    #endif /* __CS_SERVICE__ */ 
        else  if ( string_ptr[index] == '+') 
        {
		//ascii = 1;
		 return false;
        }
	else
	{
            //rmmi_result_code_fmttr(RMMI_RCODE_ERROR, RMMI_ERR_UNRECOGNIZED_CMD);            
            return false;
            //break;
        }

        /**** [MAUI_01319443] mtk02514, 090120 *************************************************************
        *  The new hash value computed method is as follows.
        *  for AT+ABCDEFGH
        *  hash_value1 = hash(A)*38^4 + hash(B)*38^3 + hash(C)*38^2 + hash(D)*38^1 + hash(E)*38^0
        *                    = ((((hash(A)+0)*38 + hash(B))*38 + hash(C))*38 + hash(D))*38 + hash(E)  <== as following statements do.
        *  hash_value2 = hash(F)*38^2 + hash(G)*38^1 + hash(H)*38^0
        *                    = ((hash(F) + 0)*38 + hash(G))*38 + hash(H)  <== as following statements do.
        **********************************************************************************************/
        if (counter < RMMI_HASH_TABLE_SPAN)
            *hash_value1 = (*hash_value1)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);
        else
            *hash_value2 = (*hash_value2)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);

        counter++;

        /* Increment the index to get the next unsigned intacter */
        index++;

    }   /* End of while loop */
    return true;
}

int rmmi_calc_hashvalue (rmmi_string_struct *source_string_ptr, unsigned int *hash_value1, unsigned int *hash_value2)

{
    unsigned int counter = 0;
    //source_string_ptr->index = 0; //AT+, AT^

    /* This variable is used to ensure that the unsigned ints coming on line for
    command are properly broken into set of 5 unsigned ints. So, the correct
    hash value can be calculated based on the parser expression and
    respectively stored in the variables hash_value1 and hash_value2 */
    unsigned int ascii;

    /* Variable used to store the calculated hash value of 2nd 5 unsigned intacters
    as a part of a extended command */
    unsigned int cmd_found = false;

    /* if command is found, cmd_index is the command enum */
    *hash_value1 = 0;

    /* Variable used to store the calculated hash value of 1st 5 unsigned intacters
       as a part of a extended command */
    *hash_value2 = 0;
    
    /* Skip all leading white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /* Parser for Extended AT commands */
    while ((source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_EQUAL) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_QUESTION_MARK) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_COMMA) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_SEMICOLON) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != RMMI_END_OF_STRING_CHAR)&&
           (counter <= RMMI_MAX_EXT_CMD_NAME_LEN) &&
           (source_string_ptr->string_ptr[source_string_ptr->index]
            != S3) && (source_string_ptr->string_ptr[source_string_ptr->index] != S4))
    {
	
	 
        if (RMMI_IS_UPPER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr[source_string_ptr->index] - RMMI_CHAR_A;
        }

        else if (RMMI_IS_LOWER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr[source_string_ptr->index] - rmmi_char_a;
        }

    #ifdef __CS_SERVICE__
        else if (RMMI_IS_NUMBER(source_string_ptr->string_ptr[source_string_ptr->index]))
        {
            ascii = source_string_ptr->string_ptr
                [source_string_ptr->index] - RMMI_CHAR_0 + RMMI_NUMBER_OFFSET_IN_PARSER_TABLE;
        }
    #endif /* __CS_SERVICE__ */ 

        else if ( source_string_ptr->string_ptr[source_string_ptr->index] == '+') 
	 {
	 	//ascii = 1;
	 	return false;
        } else
        {
            //rmmi_result_code_fmttr(RMMI_RCODE_ERROR, RMMI_ERR_UNRECOGNIZED_CMD);            
            return false;
            //break;
        }

        /**** [MAUI_01319443] mtk02514, 090120 *************************************************************
        *  The new hash value computed method is as follows.
        *  for AT+ABCDEFGH
        *  hash_value1 = hash(A)*38^4 + hash(B)*38^3 + hash(C)*38^2 + hash(D)*38^1 + hash(E)*38^0
        *                    = ((((hash(A)+0)*38 + hash(B))*38 + hash(C))*38 + hash(D))*38 + hash(E)  <== as following statements do.
        *  hash_value2 = hash(F)*38^2 + hash(G)*38^1 + hash(H)*38^0
        *                    = ((hash(F) + 0)*38 + hash(G))*38 + hash(H)  <== as following statements do.
        **********************************************************************************************/
        if (counter < RMMI_HASH_TABLE_SPAN)
            *hash_value1 = (*hash_value1)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);
        else
            *hash_value2 = (*hash_value2)*(RMMI_HASH_TABLE_ROW+1)+(ascii+1);

        counter++;

        /* Increment the index to get the next unsigned intacter */
        source_string_ptr->index++;

        /* skip all leading white  spaces */
        rmmi_skip_spaces(source_string_ptr);

    }   /* End of while loop */
    return true; 
}


int cmd_handler_init ()
{
	int i;
	for (i=0; i<sizeof (g_cmd_handler); i++)
	{
		if (g_cmd_handler[i].cmd_index == INVALID_ENUM)
			break;
		calc_hashvalue (g_cmd_handler[i].cmd_string, 
							&g_cmd_handler[i].hash_value1, 
							&g_cmd_handler[i].hash_value2);
		
	}
	return 0;
}

unsigned int cmd_analyzer(unsigned int hash_value1, unsigned int hash_value2,  int *cmd_index_ptr)
{
    unsigned int ret_val = false;
    unsigned int col_index = 1;
    unsigned int row_index;

    if ((hash_value1 == 0) && (hash_value2 == 0))
    {
        return ret_val;
    }
    for (row_index = 0; row_index < sizeof (g_cmd_handler); row_index++)
    {
	    if (g_cmd_handler[row_index].cmd_index == INVALID_ENUM)
			break;
	    if ((hash_value1 == g_cmd_handler[row_index].hash_value1) &&
            (hash_value2 == g_cmd_handler[row_index].hash_value2))
        {
            *cmd_index_ptr = row_index;
            ret_val = true;
            break;
        }
    }

    return ret_val;
}



char custom_get_atcmd_symbol(void)
{
   return (CUSTOM_SYMBOL);
}

rmmi_cmd_mode_enum rmmi_find_cmd_mode(rmmi_string_struct *source_string_ptr)
{ 
   rmmi_cmd_mode_enum ret_val = RMMI_WRONG_MODE;

    /* Skip all leading white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /*
     * If not read mode, then check for the TEST/SET/EXECUTE mode.
     * * Symbol '=' is common for both SET/EXECUTE and TEST command;
     * * so first check for the '=' symbol.
     */
    if (source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_EQUAL)
    {
        /*
         * If we find '?' after the '=' symbol, then we decide that
         * * given command is TEST command. Else it is assumed to be
         * * either a SET or an EXECUTE command
         */
        source_string_ptr->index++;
        /* Skip white spaces after the '=' symbol */
        rmmi_skip_spaces(source_string_ptr);
        LOGD("The char after = is %c\n", source_string_ptr->string_ptr[source_string_ptr->index]);
        if (source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_QUESTION_MARK)
        {
            /*
             * Since question mark is also found, check whether the
             * * string is terminated properly by a termination character.
             * * White spaces may be present between the question mark and
             * * the termination character.
             */
            source_string_ptr->index++;
            rmmi_skip_spaces(source_string_ptr);

            if (source_string_ptr->string_ptr[source_string_ptr->index] == S3||
	         source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)
            {
                ret_val = RMMI_TEST_MODE;
            }
        }
        /* If didn't find '?' after the '=' symbol then we decide that
           given command is SET/EXECUTE command */
        else if(source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_0)
        {
            ret_val = RMMI_READ_MODE;
        }
        else
        {
            ret_val = RMMI_SET_OR_EXECUTE_MODE;
        }
    }   /* mtk00468 add for some extend command has no parameter */
    else if ((source_string_ptr->string_ptr[source_string_ptr->index] == S3) ||
             (source_string_ptr->string_ptr[source_string_ptr->index] == S4)||
	      source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_END_OF_STRING_CHAR)

    {
        ret_val = RMMI_ACTIVE_MODE;
    }

    return ret_val;
}
rmmi_cmd_type_enum rmmi_find_cmd_class(rmmi_string_struct *source_string_ptr)
{
    rmmi_cmd_type_enum ret_val = RMMI_INVALID_CMD_TYPE;
    source_string_ptr->index = 0;
    rmmi_skip_spaces(source_string_ptr);        // Skip all leading white spaces 

    /* Check if the first unsigned intacter is neither 'A' nor 'a' i.e. a invalid 
       command prefix */
    if ((source_string_ptr->string_ptr[source_string_ptr->index] != RMMI_CHAR_A) &&
        (source_string_ptr->string_ptr[source_string_ptr->index] != rmmi_char_a))
    {
        return ret_val;
    }

    /* Increment the index to get the next unsigned intacter */
    source_string_ptr->index++;

    /* Skip all white spaces */
    rmmi_skip_spaces(source_string_ptr);

    /* there are two possibilities of unsigned intacters may come after the
       unsigned intacter A. One is '/' and other one is 'T'. First we check for
       the unsigned intacter '/', if not found then check for the unsigned intacter 'T'. */
    if ((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_FORWARD_SLASH) &&
        (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
    {
        /* Skip all leading spaces, which are coming after the "A/".
           Finally check for the command line termination unsigned intacter */
        source_string_ptr->index++;
        rmmi_skip_spaces(source_string_ptr);

        if ((source_string_ptr->string_ptr[source_string_ptr->index] == S3) &&
            (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            ret_val = RMMI_PREV_CMD;
        }
        /* else, command line is invalid */
        else
        {
            ret_val = RMMI_INVALID_CMD_TYPE;
        }
    }
    /* We failed to find '/'.the second alternative is 'T'.Check whether
       the second non spaces unsigned intacter is 'T' or not */
    else if (((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_T) ||
              (source_string_ptr->string_ptr[source_string_ptr->index] == rmmi_char_t)) &&
             (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
    {
        /*
         * Skip all leading white space unsigned intacter which are coming after
         * * "AT".Again we can find two different unsigned intacter after "AT".One is
         * * '+' and other one is non '+' unsigned intacter.if we find '+' then we
         * * decided that the give command is a Extended command, otherwise
         * * Basic command.
         */
        /*
         * there's no need of check for command line termination, because that
         * * will be checked during the parsing of commands
         */
        source_string_ptr->index++;
        rmmi_skip_spaces(source_string_ptr);

        if ((source_string_ptr->string_ptr[source_string_ptr->index] == RMMI_CHAR_PLUS) &&
            (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            /* the '+' unsigned intacter is found,hence it is extended command */
            ret_val = RMMI_EXTENDED_CMD;
            source_string_ptr->index++; /* to get the next unsigned intacter */
        }
        else if ((source_string_ptr->string_ptr[source_string_ptr->index] == (custom_get_atcmd_symbol())) &&
                 (source_string_ptr->index <= MAX_MULTIPLE_CMD_INFO_LEN))
        {
            /* the special symbol defined by customer is found,hence it is customer-defined command */
            ret_val = RMMI_CUSTOMER_CMD;
            source_string_ptr->index++; /* to get the next unsigned intacter */
        }
        else
        {
            /* the non '+' unsigned intacter was not found; take it
               to be basic command */
            ret_val = RMMI_BASIC_CMD;
        }
    }

    /* We didn't find the either "AT" or "A/". Either the command was too long, 
       or the unsigned intacters were unrecognizable */
    else if (source_string_ptr->index >= MAX_MULTIPLE_CMD_INFO_LEN)
    {
        ret_val = RMMI_INVALID_CMD_TYPE;
    }
    /* unrecognizable command line prefix */
    else
    {
        ret_val = RMMI_INVALID_CMD_TYPE;
    }

    return ret_val;
}


int rmmi_cmd_analyzer (rmmi_string_struct *source_string_ptr)
{  
    unsigned int hash_value1, hash_value2;
    unsigned int cmd_found = 0;

    source_string_ptr ->cmd_owner = CMD_OWNER_INVALID;
    source_string_ptr->cmd_mode = RMMI_WRONG_MODE;
    source_string_ptr->cmd_class = rmmi_find_cmd_class(source_string_ptr);

    LOGD("RMMI_CMD_ANALYZER:%d\n", source_string_ptr->cmd_class);

    if (RMMI_INVALID_CMD_TYPE==source_string_ptr->cmd_class) 
    {
         cmd_found =  INVALID_ENUM;
	     goto err;
    }
    if (false==rmmi_calc_hashvalue (source_string_ptr, &hash_value1, &hash_value2))
    {
         cmd_found =  INVALID_ENUM;
	     goto err;
    }
    		
    cmd_found = cmd_analyzer(hash_value1, hash_value2, &source_string_ptr->cmd_index);

    source_string_ptr->cmd_mode = rmmi_find_cmd_mode (source_string_ptr);
    source_string_ptr->cmd_owner = (cmd_found==1) ? CMD_OWENR_AP:CMD_OWENR_MD;

err:		
   return cmd_found;

}


CMD_OWENR_ENUM rmmi_cmd_processor(unsigned char *cmd_str, char *result)
{
    LOGD(TAG "Entry rmmi_cmd_processor");
	unsigned int cmd_found = 0;
	rmmi_string_struct source_string_ptr;

	source_string_ptr.index = 0;
    source_string_ptr.result = result;
	source_string_ptr.string_ptr = cmd_str;
	cmd_found = rmmi_cmd_analyzer(&source_string_ptr);
    LOGD(TAG "cmd_found=%d", cmd_found);

	if (source_string_ptr.cmd_owner == CMD_OWENR_AP )
		g_cmd_handler[source_string_ptr.cmd_index].func(&source_string_ptr);
	
    LOGD(TAG "source_string_ptr.cmd_owner=%d", source_string_ptr.cmd_owner);
	return source_string_ptr.cmd_owner;

}



#if 0


const char * at_command_set(char *pCommand, const int fd)
{
	LOGD(TAG "%s start\n", __FUNCTION__);
	if(fd < 0) {
	  LOGD(TAG "Invalid fd\n");
	  return "ERROR";
	}
	//const int sz = 64;
	const int sz = 100;
	char buf[sz];
	memset(buf, 0, sz);
	const int HALT = 100000;
	
	sprintf(buf, "%s",pCommand);
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[AT]Set command: %s\n", buf);
	
	char *p = NULL;
	p = strstr(buf, "OK");
	if(p) {
	  LOGD(TAG "%s end with OK\n", __FUNCTION__);
	  return STR_OK;
	} else {
	  LOGD(TAG "%s end with ERROR\n", __FUNCTION__);
	  return STR_ERROR;
	}
}


const char * at_command_get(char *pCommand, char *pRet, const int len, const int fd)
{
	LOGD(TAG "%s start\n", __FUNCTION__);
	
	if(fd < 0) {
		LOGD(TAG "Invalid fd\n");
		return STR_ERROR;
	}
	
	// const int sz = 64;
	const int sz = 100;
	char buf[sz];
	memset(buf, 0, sz);
	const int HALT = 100000;
	
	sprintf(buf, pCommand);
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	LOGD(TAG "[AT]SN: %s\n", buf);
	
	const char *tok = "+EGMR: ";
	char *p = NULL;
	p = strstr(buf, tok);
	if(p) {
		if(len >= strlen(p)) {
		  strcpy(pRet, p);
		  LOGD(TAG "%s end with OK\n", __FUNCTION__);
		  return STR_OK;
		} else {
		  LOGD(TAG "Buffer is not enough\n");
		}
	} else {
		LOGD(TAG "Fail to get SN\n");
	}
	
	LOGD(TAG "%s end with ERROR\n", __FUNCTION__);
	return STR_ERROR;	
}

/********************
       AT+ESLP=0
********************/
const char* setSleepMode(const int fd)
{
	LOGD(TAG "%s start\n", __FUNCTION__);
	
	if(fd < 0) {
	  LOGD(TAG "Invalid fd\n");
	  return STR_ERROR;
	}
	
	const int sz = 64;
	char buf[sz];
	memset(buf, 0, sz);
	const int HALT = 100000;
	
	sprintf(buf, "AT+ESLP=0\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[AT]Set sleep mode: %s\n", buf);
	
	char *p = NULL;
	p = strstr(buf, "OK");
	if(p) {
	  LOGD(TAG "%s end with OK\n", __FUNCTION__);
	  return STR_OK;
	} else {
	  LOGD(TAG "%s end with ERROR\n", __FUNCTION__);
	  return STR_ERROR;
	}
}


/********************
       AT+EGMR=0,5
       param sn:   the buffer to save the serial number
       param len:  the length of buffer
********************/
const char* getSN(char *sn, const unsigned int len, const int fd)
{
	LOGD(TAG "%s start\n", __FUNCTION__);
	
	if(fd < 0) {
		LOGD(TAG "Invalid fd\n");
		return STR_ERROR;
	}
	
	// const int sz = 64;
	const int sz = 100;
	char buf[sz];
	memset(buf, 0, sz);
	const int HALT = 100000;
	
	sprintf(buf, "AT+EGMR=0,5\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	LOGD(TAG "[AT]SN: %s\n", buf);
	
	const char *tok = "+EGMR: ";
	char *p = NULL;
	p = strstr(buf, tok);
	if(p) {
		if(len >= strlen(p)) {
		  strcpy(sn, p);
		  LOGD(TAG "%s end with OK\n", __FUNCTION__);
		  return STR_OK;
		} else {
		  LOGD(TAG "Buffer is not enough\n");
		}
	} else {
		LOGD(TAG "Fail to get SN\n");
	}
	
	LOGD(TAG "%s end with ERROR\n", __FUNCTION__);
	return STR_ERROR;
}

/********************
       AT+EGMR=1,5
       param sn: new serial number
********************/
const char* setSN(const char* sn, const int fd)
{
	LOGD(TAG "%s start\n", __FUNCTION__);
	
	if(fd < 0) {
	  LOGD(TAG "Invalid fd\n");
	  return STR_ERROR;
	}
	
	// const int sz = 64;
	const int sz = 100;
	char buf[sz];
	memset(buf, 0, sz);
	const int HALT = 100000;
	
	sprintf(buf, "AT+EGMR=1,5,\"%s\"\r\n", sn);
	LOGD(TAG "[AT]%s\n", buf);
	write(fd, buf, strlen(buf));
	usleep(100000);
	read(fd, buf, sz);
	LOGD(TAG "[AT]%s\n", buf);
	
	char *p = NULL;
	p = strstr(buf, "OK");
	if(p) {
	  LOGD(TAG "%s end with OK\n", __FUNCTION__);
	  return STR_OK;
	} else {
	  LOGD(TAG "%s end with Error\n", __FUNCTION__);
	  return STR_ERROR;
	}
}

/********************
  to dial a specific number
********************/
const char* dial(const int fd, const char *number)
{
    LOGD(TAG "%s start\n", __FUNCTION__);

    if(fd < 0) {
    LOGD(TAG "Invalid fd\n");
    return STR_ERROR;
    }

    const int sz = 64 * 2;
    char buf[sz];
    memset(buf, 0, sz);
    const int HALT = 200000;
    char *p = NULL;
    const int rdTimes = 3;
    int rdCount = 0;

                
#if 0
    // +EIND will always feedback +EIND when open device,
    // so move this to openDevice.	
    // +EIND
    read(fd, buf, sz);
    usleep(HALT);
    LOGD(TAG "[at]%s\n", buf);
#endif
	
	sprintf(buf, "AT+ESIMS\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]Reset SIM1: %s\n", buf);
#ifdef GEMINI
	sprintf(buf, "AT+ESUO=5\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]Switch to UART2: %s\n", buf);
	
	sprintf(buf, "AT+ESIMS\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]Reset SIM2: %s\n", buf);
	
	sprintf(buf, "AT+ESUO=4\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]Switch to UART1: %s\n", buf);
	
	sprintf(buf, "AT+EFUN=3\r\n");
#else
	sprintf(buf, "AT+CFUN=1\r\n");
#endif
	write(fd, buf, strlen(buf));
	usleep(HALT);
	// read(fd, buf, sz);
	// usleep(HALT);
	// LOGD(TAG "[at]Turn on radio: %s\n", buf);
	for(rdCount = 0; rdCount < rdTimes; ++rdCount) 
	{
		read(fd, buf, sz);
		p = NULL;
		p = strstr(buf, "OK");
		if(p) {
		  break;
		}
		usleep(HALT);
	}
	if(!p) {
		LOGD(TAG "%s error in +EFUN\n",__FUNCTION__);
		return STR_ERROR;
	}
	
  sprintf(buf, "AT+CREG=2\r\n");
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]AT+CREG=2: %s\n", buf);
	
	int retry = 5;
	while(retry > 0) {
	    read(fd, buf, sz);
	    if(strstr(buf, "+CREG:")) {
	        LOGD(TAG "%s SIM 1 creg status: %s\n", __FUNCTION__, buf);
	        break;
	    } else {
	        LOGD(TAG "%s SIM 1 has not get creg status yet\n", __FUNCTION__);
	        retry -= 1;
	        usleep(1000000);
	    }
	}
	if(retry <= 0) {
	    LOGD(TAG "%s Fail to get network registration status\n", __FUNCTION__);
	    // return STR_ERROR;
	}
	
	sprintf(buf, "ATD%s;\r\n", number);
	write(fd, buf, strlen(buf));
	usleep(HALT);
	read(fd, buf, sz);
	usleep(HALT);
	LOGD(TAG "[at]ATD: %s\n", buf);
	for(rdCount = 0; rdCount < rdTimes; ++rdCount) 
	{
		p = NULL;
		p = strstr(buf, "OK");
		if(p) {
		  break;
		}
		usleep(HALT);
	}
	if(!p) {
		LOGD(TAG "%s error in ATD\n", __FUNCTION__);
		return STR_ERROR;
	}
	
	LOGD(TAG "%s end\n", __FUNCTION__);
	return STR_OK;
}
#endif
