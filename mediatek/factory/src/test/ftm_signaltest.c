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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include <termios.h> /* POSIX terminal control definitions */


#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "utils.h"

#include "hardware/ccci_intf.h"


//#ifdef FEATURE_FTM_SIGNALTEST

#define TAG "[Signal Test] "

#define SLEEPMODE "AT+ESLP=0"
#define EMER_CALL "ATD112;"
#define GET_SN "AT+EGMR=0,5"
// Set SN should be "AT+EGMR=1,5,'SN'"
#define SET_SN "AT+EGMR=1,5"
#define AT "AT"
#define ATE0 "ATE0"
#define ATH "ATH"
#define BUF_SIZE 128
int g_mdFlag=1;
enum {
#if 0
#if defined(MTK_ENABLE_MD1)
    #if defined(MTK_ENABLE_MD2)
		ITEM_CALL_FOR_MODEM_75,
		ITEM_CALL_FOR_MODEM_52,
    #elif defined(MTK_DT_SUPPORT)
			ITEM_CALL_FOR_MODEM_75,
			ITEM_CALL_FOR_MODEM_52,
#else
    ITEM_EMG_CALL,
#endif
#else
	#if defined(MTK_DT_SUPPORT)
	    ITEM_CALL_FOR_MODEM_75,
		ITEM_CALL_FOR_MODEM_52,
	#else
    ITEM_EMG_CALL,
#endif
#endif
#endif
    ITEM_PASS,
    ITEM_FAIL,
};


typedef enum {
	FALSE = 0,
	TRUE,
} _BOOL;



static item_t sigtest_items[] = {
//#ifdef MTK_DT_SUPPORT	
//    item(ITEM_CALL_FOR_MODEM_75,   "Call Test - Modem 1(MT6575, SIM1)"),
//    item(ITEM_CALL_FOR_MODEM_52,   "Call Test - Modem 2(MT6252, SIM2)"),
//#else
//    item(ITEM_EMG_CALL, uistr_info_emergency_call),
//#endif   
//    item(ITEM_PASS,   uistr_pass),
//    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

struct sigtest {  
    char         info[1024];
    bool         exit_thd;
    int 	 fd_atmd;

	int fd_atmd2;

    int fd_atmd_dt;

    int 	 fd_atmd5;

    text_t title;
    text_t text;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define mod_to_sigtest(p)  (struct sigtest*)((char*)(p) + sizeof(struct ftm_module))
#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)
#define HALT_INTERVAL 50000


pthread_t g_AP_UART_USB_RX;
pthread_t g_AP_CCCI_RX;



int g_fd_atcmd = -1, g_fd_uart = -1;
int g_fd_atcmdmd2=-1;
int g_fd_atcmdmd_dt = -1;
int g_fd_atcmd5 = -1;

int g_hUsbComPort = -1;


extern int wait4_ack (const int fd, char *pACK, int timeout);
extern int read_ack (const int fd, char *rbuff, int length);
extern int send_at (const int fd, const char *pCMD);
extern int open_usb_port(int uart_id, int baudrate, int length, char parity_c, int stopbits);
extern int ExitFlightMode (int fd, _BOOL bON);

extern int ExitFlightMode_DualTalk(int fd,_BOOL bON);
extern int openDeviceWithDeviceName(char *deviceName);

extern bool is_support_modem(int modem);



int AT_Pre_Process (char *buf_cmd)
{
#if 0
	if(!strcmp(buf_cmd, SLEEPMODE)){
            LOGD(TAG "SLEEP_MODE");
            pret = setSleepMode(fd_atcmd);
            if(!strcmp(pret, STR_OK))
                write_chars(fd_uart, "OK\r",3);
            else
               write_chars(fd_uart, "ERROR\r",6);
        }
        else if(!strcmp(buf_cmd, EMER_CALL)){
            LOGD(TAG "EMERCALL");
            pret = dial112(fd_atcmd);
            if(!strcmp(pret, STR_OK))
                write_chars(fd_uart, "OK\r",3);
            else
               write_chars(fd_uart, "ERROR\r",6);
        } 
        else if(!strcmp(buf_cmd, GET_SN)) {
            LOGD(TAG "GET_SN");
            pret = getSN(buf_ret, sizeof(buf_ret), fd_atcmd);
            if(!strcmp(pret, STR_OK))
                write_chars(fd_uart, buf_ret, sizeof(buf_ret));
            else
               write_chars(fd_uart, "ERROR\r",6);
        }
         else if(strstr(buf_cmd, SET_SN) || !strcmp(buf_cmd, ATH)) {
            LOGD(TAG "%s\n", buf_cmd); 
            // Add '\r\n' tp command buf.
            buf_cmd[len] = '\r';
            buf_cmd[len+1] = '\n';
            buf_cmd[len+2] = '\0';
            pret = at_command_set(buf_cmd, fd_atcmd);
            if(!strcmp(pret, STR_OK))
                write_chars(fd_uart, "OK\r",3);
            else
               write_chars(fd_uart, "ERROR\r",6);
        }
        else if(!strcmp(buf_cmd, AT) || !strcmp(buf_cmd, ATE0)) {
            // AT and ATE0 are commands for test command between
            // ATE tool and target. Just return "OK\r" when receive.
            LOGD(TAG "%s\n", buf_cmd);
            write_chars(fd_uart, "OK\r",3);
        }               
        else{
            LOGD(TAG "Unsupported command\n");
            write_chars(fd_uart, "ERROR\r",6);            
        }
#endif

	#ifdef MTK_DT_SUPPORT
		if(!strcmp(buf_cmd, "AT+SWITCH"))
		{
            LOGD(TAG "AT+SWITCH");	
			if(g_mdFlag==1)
				g_mdFlag=2;
			else if(g_mdFlag==2)
				g_mdFlag=1;
			else
				LOGD(TAG "Unsupported MD Flag\n");
        }
	#endif
	return 0;
}

int ACK_Pre_Process (char *buf_cmd)
{
	if (strstr(buf_cmd, "RING"))
	{
		LOGD(TAG "MT call: RING\n");
		send_at(g_fd_atcmd, "ATA\r");
		wait4_ack (g_fd_atcmd, NULL, 1000);
	}
	return 0;
}


void * AP_UART_USB_RX (void* lpParameter)
{
	char buf_cmd [BUF_SIZE] = {0};
	int len;
    int wr_len;
    char result[64] = {0};
	
	LOGD(TAG "Enter AP_UART_USB_RX()\n");
    cmd_handler_init ();

	for (;;)
	{
		len = read_a_line(g_fd_uart, buf_cmd, BUF_SIZE);            
		if (len>0)
		{
			buf_cmd[len] = '\0';
        	LOGD(TAG "AP_UART_USB_RX Command: %s, Len: %d\n" ,buf_cmd, len);
			AT_Pre_Process (buf_cmd);
            LOGD("buf_cmd:%s\n", buf_cmd);
#if 0
			#ifdef MTK_DT_SUPPORT
				
				if(g_mdFlag==1)
			send_at (g_fd_atcmd, buf_cmd);
				if(g_mdFlag==2)
					send_at (g_fd_atcmdmd2, buf_cmd);
			#else
				send_at (g_fd_atcmd, buf_cmd);
			#endif
#endif

            CMD_OWENR_ENUM owner = rmmi_cmd_processor(buf_cmd, result);
            LOGD(TAG "result:%s\n", result);
            if(owner == CMD_OWENR_AP)
            {
                wr_len = write_chars (g_fd_uart, result, strlen(result));
			    if (wr_len != strlen(result))
				LOGE(TAG "AP_CCCI_RX: wr_len != rd_len\n");
            }
            else
            {

			if(g_mdFlag == 1)
			{
				#if defined(MTK_EXTERNAL_MODEM_SLOT)
				if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "1"))
				{
					#ifndef EVDO_DT_SUPPORT
						send_at(g_fd_atcmdmd_dt, buf_cmd);
					#endif
				}
				else{

                    if(is_support_modem(1)){
						send_at(g_fd_atcmd, buf_cmd);

					}else if(is_support_modem(2)){
					    send_at(g_fd_atcmdmd2, buf_cmd);

                    }
				}
				#else
				

					if(is_support_modem(1)){
						send_at(g_fd_atcmd, buf_cmd);

                    }else if(is_support_modem(2)){
					    send_at(g_fd_atcmdmd2, buf_cmd);

                    }
				#endif
			}
		    else if(g_mdFlag == 2)
		    {
				#if defined(MTK_EXTERNAL_MODEM_SLOT)
				if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "2"))
				{
					#ifndef EVDO_DT_SUPPORT
						send_at(g_fd_atcmdmd_dt, buf_cmd);
					#endif
				}else{

					if(is_support_modem(1)){
						send_at(g_fd_atcmd, buf_cmd);

                    }else if(is_support_modem(2)){
					    send_at(g_fd_atcmdmd2, buf_cmd);

                    }
				}
				#else
				

					if(is_support_modem(1)){
						send_at(g_fd_atcmd, buf_cmd);

                    }else if(is_support_modem(2)){
					    send_at(g_fd_atcmdmd2, buf_cmd);

                    }
				#endif
			}
		}
 	}
 	}

	//pthread_exit (NULL);
}
void * AP_CCCI_RX (void* lpParameter)
{
	char buf_ack [BUF_SIZE] = {0};
	char buf_log [BUF_SIZE] = {0};
	int rd_len=0, wr_len=0;
	
	LOGD(TAG "Enter AP_CCCI_RX()\n");
	
	for (;;)
	{
#if 0
		#ifdef MTK_ENABLE_MD2
			if(g_mdFlag==1)
				rd_len = read_ack (g_fd_atcmd, buf_ack, BUF_SIZE);
			if(g_mdFlag==2)
				rd_len = read_ack (g_fd_atcmdmd2, buf_ack, BUF_SIZE);
		#else
		rd_len = read_ack (g_fd_atcmd, buf_ack, BUF_SIZE);
		#endif
#endif

if(g_mdFlag == 1)
					{
			#if defined(MTK_EXTERNAL_MODEM_SLOT)
			if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "1"))
			{
				#ifndef EVDO_DT_SUPPORT
					rd_len = read_ack(g_fd_atcmdmd_dt, buf_ack, BUF_SIZE);
				#endif
			}
			else
			{

				if(is_support_modem(1)){
					rd_len = read_ack(g_fd_atcmd, buf_ack, BUF_SIZE);

                }else if(is_support_modem(2)){
					rd_len = read_ack(g_fd_atcmdmd2, buf_ack, BUF_SIZE);

                }
			}
			#else

				if(is_support_modem(1)){
					rd_len = read_ack(g_fd_atcmd, buf_ack, BUF_SIZE);

                }else if(is_support_modem(2)){
					rd_len = read_ack(g_fd_atcmdmd2, buf_ack, BUF_SIZE);

                }
			#endif
		}
		else if(g_mdFlag == 2)
		{
			#if defined(MTK_EXTERNAL_MODEM_SLOT)
			if(!strcmp(MTK_EXTERNAL_MODEM_SLOT, "2"))
			{
				#ifndef EVDO_DT_SUPPORT
					rd_len = read_ack(g_fd_atcmdmd_dt, buf_ack, BUF_SIZE);
				#endif
			}
			else
			{

				if(is_support_modem(1)){
					rd_len = read_ack(g_fd_atcmd, buf_ack, BUF_SIZE);

                }else if(is_support_modem(2)){
					rd_len = read_ack(g_fd_atcmdmd2, buf_ack, BUF_SIZE);

                }
			}
			#else
			

				if(is_support_modem(1)){
					rd_len = read_ack(g_fd_atcmd, buf_ack, BUF_SIZE);

                }else if(is_support_modem(2)){
					rd_len = read_ack(g_fd_atcmdmd2, buf_ack, BUF_SIZE);

                }
			#endif
		}
		
		if (rd_len>0)
		{
			memcpy (buf_log, buf_ack, rd_len);
			buf_log[rd_len] = '\0';
			LOGD(TAG "AP_CCCI_RX: %s, rd_len = %d\n", buf_log, rd_len);

			ACK_Pre_Process(buf_log);
			
			wr_len = write_chars (g_fd_uart, buf_ack, rd_len);
			if (wr_len != rd_len)
				LOGE(TAG "AP_CCCI_RX: wr_len != rd_len\n");
		}
	}
	//pthread_exit(NULL);
}



void New_Thread ()
{
	LOGD(TAG "Enter New_Thread() \n");
	if(pthread_create (&g_AP_UART_USB_RX, NULL, AP_UART_USB_RX, NULL) != 0)
	{
		LOGD (TAG "main:Create AP_UART_USB_RX thread failed");
		return;
	}

	if(pthread_create (&g_AP_CCCI_RX, NULL, AP_CCCI_RX, NULL) != 0)
	{
		LOGD (TAG "main:Create AP_CCCI_RX thread failed");
		return;
	}
	LOGD(TAG "Exit New_Thread() \n");
}

int Wait_Thread (pthread_t arg)
{   /* exit thread by pthread_kill -> pthread_join*/
    int err;
    if ((err = pthread_kill(arg, SIGUSR1)))
        return err;

    if ((err = pthread_join(arg, NULL)))
        return err;
    return 0;
}

int Free_Thread ()
{   /* exit thread by pthread_kill -> pthread_join*/
    int err;
    if ( 0 != (err = Wait_Thread(g_AP_UART_USB_RX)) )
        return err;

    if ( 0 != (err = Wait_Thread(g_AP_CCCI_RX)) )
        return err;

    return 0;
}


int COM_Init (int *fd_atcmd, int *fd_uart, int *hUsbComPort)
{
    char dev_node[32];
    if(is_support_modem(1))
    {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS1));
        fd_atcmd = openDeviceWithDeviceName(dev_node);
    }

    else if(is_support_modem(2))
    {

		snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
        fd_atcmd = openDeviceWithDeviceName(dev_node);

    }



    if((is_support_modem(1) || is_support_modem(2)) && (*fd_atcmd == -1)) {
		LOGE(TAG "Open ccci port fail\r\n" );
        return *fd_atcmd;
    }
    
    //ATE Tool use 115200 baud rate
    *fd_uart = open_uart_port(UART_PORT1, 115200, 8, 'N', 1);
	if(*fd_uart == -1) {
        LOGE(TAG "Open uart port %d fail\r\n" ,UART_PORT1);
        return *fd_uart;
    }
    else
        LOGD(TAG "Open uart port %d success\r\n" ,UART_PORT1);
  	
           
	//*hUsbComPort = open("/dev/ttyGS0",O_RDWR | O_NOCTTY | /*O_NONBLOCK | */O_NDELAY);
	*hUsbComPort = open_usb_port(UART_PORT1, 115200, 8, 'N', 1);
	if(*hUsbComPort == -1)
	{
		LOGE(TAG "Open usb fail\r\n");
		return *hUsbComPort;
	}
	else
	{
		//initTermIO(*hUsbComPort);
		LOGD(TAG "Open usb success\r\n");
	}

	return 0;
}

int COM_DeInit (int *fd_atcmd, int *fd_uart, int *hUsbComPort)
{
	    //release the handle
    if (*fd_atcmd != -1)
    {
        close(*fd_atcmd);
		*fd_atcmd = -1;
    }
    if (*fd_uart != -1)
    {
        close(*fd_uart);
		*fd_uart = -1;
    }
    if (*hUsbComPort != -1)
    {
        close(*hUsbComPort);
		*hUsbComPort = -1;
    }
	return 0;
}


int sigtest_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen, i;
    bool exit = false;
    struct sigtest *st = (struct sigtest *)priv;
    struct itemview *iv;
    int ret;
    const char *pret;
	int test_result_temp = FTM_TEST_PASS;
    char dev_node1[32];
    char dev_node2[32];
    char dev_node5[32];
    LOGD(TAG "%s\n", __FUNCTION__);

if(is_support_modem(1)){

    snprintf(dev_node1, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS1));
    st->fd_atmd = openDeviceWithDeviceName(dev_node1);
    
    if (-1 == st->fd_atmd) {
				LOGD(TAG "Fail to open CCCI interface\n");
				return 0;
    }
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup

}

if(is_support_modem(2)){

    snprintf(dev_node2, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
    st->fd_atmd2 = openDeviceWithDeviceName(dev_node2);
    
    if (-1 == st->fd_atmd2) {
				LOGD(TAG "Fail to open CCCI interface\n");
				return 0;
    }
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup

}

#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
        #if defined(PURE_AP_USE_EXTERNAL_MODEM)
            st->fd_atmd_dt = openDeviceWithDeviceName("/dev/ttyUSB1");
        #else
	        st->fd_atmd_dt = openDeviceWithDeviceName("/dev/ttyMT0");
        #endif
		//st->fd_atmd_dt= openDeviceWithDeviceName(CCCI_MODEM_MT6252);
		if (-1 == st->fd_atmd_dt) {
					LOGD(TAG "Fail to open CCCI interface\n");
					return 0;
		}
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
#endif

    if(is_support_modem(5)){

        snprintf(dev_node5, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS5));
        st->fd_atmd5 = openDeviceWithDeviceName(dev_node5);
        
        if (-1 == st->fd_atmd5) {
				LOGD(TAG "Fail to open /dev/eemcs_mux interface\n");
				return 0;
        }
	    for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup

    }

	for (i = 0; i<50; i++) usleep(50000); //sleep 1s wait for modem bootup

if(is_support_modem(1)){
	ExitFlightMode (st->fd_atmd, TRUE);
}


if(is_support_modem(2)){
	ExitFlightMode_DualTalk (st->fd_atmd2, TRUE);
}

#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
	ExitFlightMode_DualTalk(st->fd_atmd_dt, TRUE);
#endif

    for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup

    if(is_support_modem(5)){
    	ExitFlightMode (st->fd_atmd5, TRUE);
    }

    init_text(&st->title, param->name, COLOR_YELLOW);
    init_text(&st->text, &st->info[0], COLOR_YELLOW);   

 
    memset(&st->info[0], 0, sizeof(st->info));
    sprintf(st->info, "%s\n", uistr_info_emergency_call_testing);
    st->exit_thd = false;

    if (!st->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        st->iv = iv;
    }
    
    iv = st->iv;
    iv->set_title(iv, &st->title);
    iv->set_items(iv, sigtest_items, 0);
    iv->set_text(iv, &st->text); 
	iv->start_menu(iv,0);

	iv->redraw(iv);

#if 0
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
#ifdef MTK_DT_SUPPORT
			  case ITEM_CALL_FOR_MODEM_75:
					pret = dial112(st->fd_atmd);
					if(!strcmp(pret, "OK")) {
		                 LOGD(TAG "Dial 112 Success\n");
		                 sprintf(st->info, "%s\n", "Dial 112 Success!\n");
		           }
					else {
						LOGD(TAG "Dial 112 Fail\n");
						sprintf(st->info, "%s\n", "Dial 112 Fail!\n");
					}
					iv->redraw(iv);
					break;

			  case ITEM_CALL_FOR_MODEM_52:
					pret = dial112(st->fd_atmd2);
					if(!strcmp(pret, "OK")) {
		                 LOGD(TAG "Dial 112 Success\n");
		                 sprintf(st->info, "%s\n", "Dial 112 Success!\n");
		           }
					else {
						LOGD(TAG "Dial 112 Fail\n");
						sprintf(st->info, "%s\n", "Dial 112 Fail!\n");
					}
					iv->redraw(iv);
					break;
#else
            case ITEM_EMG_CALL:
               	   LOGD(TAG "Come to EMG Call...\n");

//mtk70828
				   sprintf(st->info, "%s\n", "Dial 112 on going ...!\n");
				   iv->redraw(iv);
//mtk70828
		           pret = dial112(st->fd_atmd);
		           if(!strcmp(pret, "OK")) {
		                 LOGD(TAG "Dial 112 Success\n");
		                 sprintf(st->info, "%s\n", "Dial 112 Success!\n");
		           }
					else {
						LOGD(TAG "Dial 112 Fail\n");
						sprintf(st->info, "%s\n", "Dial 112 Fail!\n");
					}
					iv->redraw(iv);
                             
                break;
#endif
            case ITEM_PASS:
            case ITEM_FAIL:
                if (chosen == ITEM_PASS) {
                    st->mod->test_result = FTM_TEST_PASS;
                } else if (chosen == ITEM_FAIL) {
                    st->mod->test_result = FTM_TEST_FAIL;
                }          
                exit = true;
                break;
        }
        
        if (exit) {
            st->exit_thd = true;
            // Need to add ATH command else the loop back test will have noise.
			ExitFlightMode (st->fd_atmd, FALSE);
				
#ifdef MTK_DT_SUPPORT
	ExitFlightMode_DualTalk (st->fd_atmd2, FALSE);
#endif
            break;
        }        
    } while (1);
#endif

if(is_support_modem(1)){
	LOGD(TAG "Come to EMG Call IN MTK_ENABLE_MD1\n");
	pret = dial112(st->fd_atmd);
	if(!strcmp(pret, "OK"))
	{
		LOGD(TAG "Dial 112 Success\n");
		sprintf(st->info, "%s\n", uistr_info_emergency_call_success_in_modem1);
		st->mod->test_result = FTM_TEST_PASS;
		test_result_temp = FTM_TEST_PASS;
	}
	else 
	{
		LOGD(TAG "Dial 112 Fail IN MTK_ENABLE_MD1\n");
		sprintf(st->info, "%s\n", uistr_info_emergency_call_fail_in_modem1);
		st->mod->test_result = FTM_TEST_FAIL;
		test_result_temp = FTM_TEST_FAIL;
	}
	ExitFlightMode (st->fd_atmd, FALSE);
    closeDevice(st->fd_atmd);
    iv->redraw(iv);
}


if(is_support_modem(2)){
	LOGD(TAG "Come to EMG Call\n");
		pret = dial112(st->fd_atmd2);
		if(!strcmp(pret, "OK"))
		{
			LOGD(TAG "Dial 112 Success\n");
			sprintf(st->info, "%s\n", uistr_info_emergency_call_success_in_modem2);
			if(test_result_temp == FTM_TEST_PASS)
			{
				st->mod->test_result = FTM_TEST_PASS;
			}
			else
			{
				st->mod->test_result = FTM_TEST_FAIL;
			}
		}
		else 
		{
			LOGD(TAG "Dial 112 Fail in MTK_ENABLE_MD2\n");
			sprintf(st->info, "%s\n", uistr_info_emergency_call_fail_in_modem2);
			st->mod->test_result = FTM_TEST_FAIL;
		}
		ExitFlightMode (st->fd_atmd2, FALSE);
		
	closeDevice (st->fd_atmd2);
    iv->redraw(iv);
}


#if defined(MTK_EXTERNAL_MODEM_SLOT) && !defined(EVDO_DT_SUPPORT)
		LOGD(TAG "Come to EMG Call\n");
			pret = dial112(st->fd_atmd_dt);
			if(!strcmp(pret, "OK"))
			{
				LOGD(TAG "Dial 112 Success\n");
				sprintf(st->info, "%s\n", uistr_info_emergency_call_success_in_modem2);
				if(test_result_temp == FTM_TEST_PASS)
			    {
	    			st->mod->test_result = FTM_TEST_PASS;
		    	}
		    	else
		    	{
				    st->mod->test_result = FTM_TEST_FAIL;
		    	}
			}
			else 
			{
				LOGD(TAG "Dial 112 Fail in MTK_ENABLE_MD2\n");
				sprintf(st->info, "%s\n", uistr_info_emergency_call_fail_in_modem2);
				st->mod->test_result = FTM_TEST_FAIL;
			}
			ExitFlightMode (st->fd_atmd_dt, FALSE);
			
		closeDevice (st->fd_atmd_dt);
        iv->redraw(iv);
#endif

	

   if(is_support_modem(5)){
	 LOGD(TAG "Come to EMG Call IN MTK_ENABLE_MD5\n");
	 pret = dial112(st->fd_atmd5);
	 if(!strcmp(pret, "OK"))
	 {
		LOGD(TAG "Dial 112 Success\n");
		sprintf(st->info, "%s\n", uistr_info_emergency_call_success_in_modem5);
		st->mod->test_result = FTM_TEST_PASS;
		test_result_temp = FTM_TEST_PASS;
	 }
	 else 
	 {
		LOGD(TAG "Dial 112 Fail IN MTK_ENABLE_MD5\n");
		sprintf(st->info, "%s\n", uistr_info_emergency_call_fail_in_modem5);
		st->mod->test_result = FTM_TEST_FAIL;
		test_result_temp = FTM_TEST_FAIL;
	 }
	 ExitFlightMode (st->fd_atmd5, FALSE);
     closeDevice(st->fd_atmd5);
     iv->redraw(iv);
   }	

    return 0;
}

struct ftm_module* sigtest_init(void)
{
    int ret;
    struct ftm_module *mod;
    struct sigtest *st;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_SIGNALTEST, sizeof(struct sigtest));
    st  = mod_to_sigtest(mod);

    st->mod = mod;
    
    if (!mod)
        return NULL;

    ret = ftm_register(mod, sigtest_entry, (void*)st);

    if(ret == 0) {
        LOGD(TAG "ftm_register success!\n");
        return mod;
    }
    else {
        LOGD(TAG "ftm_register fail!\n");
        return NULL;
    }
            
}

/// The below is for ATE signal test.
void ate_signal(void)
{
    struct itemview ate;
    text_t ate_title, info;
    char buf[100];
    char buf_cmd[100];
    char buf_ret[100];
    int len, i;
    const char *pret;
    char dev_node[32];


	int flag=0;
    
    ui_init_itemview(&ate);
    init_text(&info, buf, COLOR_YELLOW);
    ate.set_text(&ate, &info);
    len = sprintf(buf, "%s", "ATE Signaling Test\nEmergency call is not started\n");
    //sprintf(buf+len, "%s", "Emergency call is not started\n");
    ate.redraw(&ate);


if(is_support_modem(1))
{
	if(-1 == COM_Init (&g_fd_atcmd, &g_fd_uart, &g_hUsbComPort))
	{
		LOGE(TAG "COM_Init init fail!\n");
		return;
	}
	g_fd_uart = g_hUsbComPort;
}
	
	
if(is_support_modem(2)){
	if(g_fd_uart != -1 && g_hUsbComPort != -1)
	{

        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, MD_SYS2));
        g_fd_atcmdmd2 = openDeviceWithDeviceName(dev_node);
        
	if(g_fd_atcmdmd2 == -1)
	{
		LOGE(TAG "Open md2 fail\r\n");
		return;
	}
	}
	else
	{
	    if(-1 == COM_Init (&g_fd_atcmdmd2, &g_fd_uart, &g_hUsbComPort))
	    {
	    	LOGE(TAG "COM_Init init fail!\n");
	    	return;
	    }
	g_fd_uart = g_hUsbComPort;

    }
}

	
#if defined(MTK_DT_SUPPORT) && !defined(EVDO_DT_SUPPORT)
		if(g_fd_uart != -1 && g_hUsbComPort != -1)
		{
			g_fd_atcmdmd_dt= openDeviceWithDeviceName("/dev/ttyMT0");
			if(g_fd_atcmdmd_dt== -1)
			{
				LOGE(TAG "Open md2 fail\r\n");
				return;
			}
		}
		else
		{
			if(-1 == COM_Init (&g_fd_atcmdmd2, &g_fd_uart, &g_hUsbComPort))
			{
				LOGE(TAG "COM_Init init fail!\n");
				return;
			}
			g_fd_uart = g_hUsbComPort;
		}
#endif

	
	for (i = 0; i<30; i++) usleep(50000); //sleep 1s wait for modem bootup
	LOGD(TAG "fd_atcmd = %d,  fd_uart = %d, hUsbComPort = %d\r\n", g_fd_atcmd, g_fd_uart, g_hUsbComPort);

	if(is_support_modem(1)){
	ExitFlightMode (g_fd_atcmd, TRUE);
    }


	if(is_support_modem(2)){
	ExitFlightMode_DualTalk (g_fd_atcmdmd2, TRUE);
    }

	#if defined(MTK_DT_SUPPORT) && !defined(EVDO_DT_SUPPORT)
	ExitFlightMode_DualTalk(g_fd_atcmdmd_dt, TRUE);
	#endif
	New_Thread ();

    while(1) {

         usleep(HALT_INTERVAL*20);
    }
	
	Free_Thread ();

	if(is_support_modem(1)){
	COM_DeInit (&g_fd_atcmd, &g_fd_uart, &g_hUsbComPort);

	    if(is_support_modem(2)){
		    if(g_fd_atcmdmd2 != -1)
		    {
		        close(g_fd_atcmdmd2);
		    	g_fd_atcmdmd2 = -1;
		    }
       }


		#if defined(MTK_DT_SUPPORT) && !defined(EVDO_DT_SUPPORT)
			if(g_fd_atcmdmd_dt != -1)
			{
				close(g_fd_atcmdmd_dt);
				g_fd_atcmdmd_dt = -1;
			}
		#endif

     }else if(is_support_modem(2)){
		COM_DeInit(&g_fd_atcmdmd2, &g_fd_uart, &g_hUsbComPort);

     }
	return;

}
//#endif
