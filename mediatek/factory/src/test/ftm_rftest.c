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
#include <pthread.h>
#include <fcntl.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include "hardware/ccci_intf.h"

extern int send_at (const int fd, const char *pCMD);
extern int wait4_ack (const int fd, char *pACK, int timeout);
#define DEVICE_NAME ccci_get_node_name(USR_FACTORY_RF, MD_SYS1)

#ifdef FEATURE_FTM_RF

#define TAG    "[RF] "

static void *rf_update_thread(void *priv);
int rf_test_entry(struct ftm_param *param, void *priv);
int rf_test_init(void);

static item_t rf_items[] = {
    item(-1, NULL),
};

struct rf_factory {
    char info[1024];
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct itemview *iv;
    pthread_t update_thread;
    bool exit_thread;
    bool test_done;
};

#define mod_to_rf(p)  (struct rf_factory*)((char*)(p) + sizeof(struct ftm_module))


static void *rf_update_thread(void *priv)
{
    struct rf_factory *rf = (struct rf_factory*)priv;
    struct itemview *iv = rf->iv;
    int rssi_level = 0;
	
    LOGD(TAG "%s: Start\n", __FUNCTION__);

    int fd = -1;
    fd = open(DEVICE_NAME, O_RDWR);
    if(fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n",DEVICE_NAME, strerror(errno));
        goto err;
    }
    LOGD(TAG "%s has been opened...\n",DEVICE_NAME);
  
    const int rdTimes = 3;
    int rdCount = 0;
	int ret = 0;
    int retryCount = 0;
    memset(rf->info, 0, sizeof(rf->info) / sizeof(*(rf->info)));
    if(!rf->test_done) {
        bool ret = false;
        rf->test_done = true;

        const int BUF_SIZE = 256;
        char cmd_buf[BUF_SIZE];
        char rsp_buf[BUF_SIZE];
        const int HALT_TIME = 200000;//0.2s (200000ms)

        LOGD(TAG "[AT]AT polling first:\n");
        do
        {
            send_at (fd, "AT\r\n");
        } while (wait4_ack (fd, NULL, 300));

        LOGD(TAG "[AT]Disable Sleep Mode:\n");
        send_at (fd, "AT+ESLP=0\r\n");
        if (wait4_ack (fd, NULL, 3000))goto err;		

        while(1){
            /* Set Band to GSM 900 */
            LOGD(TAG "\n");
            send_at (fd, "AT+EPBSE=2\r\n");
            LOGD(TAG "Send AT+EPBASE=2 to set Band GSM 900, retryCount=%d \n",retryCount);			
            if (wait4_ack (fd, NULL, 3000)){
				if(retryCount == 5){
                    LOGD(TAG "EPBSE fail too many times\n");					
                    goto err;
				}else{
                    retryCount++;
                    usleep(HALT_TIME);				
				}					
            }else{
               LOGD(TAG "AT+EPBSE=2 got OK\n");					            
                break;
            }				  
        }

        /* Reboot modem to make new band setting work */
        send_at (fd, "AT+EPON\r\n");
        LOGD(TAG "Send AT+EPON to reboot modem \n");			
        if (wait4_ack (fd, NULL, 5000))goto err;


        /* Wait modem ready URC +EIND:128 */
        LOGD(TAG "AT+EPON OK ,Wait modem ready +EIND:128 \n");			
        LOGD(TAG "[AT]Sleep:\n");
        usleep(10 * HALT_TIME);		

        /* Reopen ccci FD. otherwise cannot access CCCI normally after rebooting modem */
        close(fd);
        fd = open(DEVICE_NAME, O_RDWR);
        if(fd < 0) {
            LOGD(TAG "Fail to open %s: %s\n",DEVICE_NAME, strerror(errno));
            goto err;
        }
        LOGD(TAG "%s has been reopened...\n",DEVICE_NAME);
		
        retryCount = 0;
	
        while(retryCount < 100){
            char *p = NULL;
			
            memset(rsp_buf, 0, sizeof(rsp_buf));		  
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------Wait EIND URC start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------Wait ENID URC end------\n");	

            retryCount++;
            p = strstr(rsp_buf, "+EIND:");						

            if(p!= NULL){
                LOGD(TAG "+EIND:128 detected\n");	
				break;
			}			
 	        usleep(HALT_TIME);						
        }		
		
        LOGD(TAG "[AT]Disable Sleep Mode:\n");
        send_at (fd, "AT+ESLP=0\r\n");
        if (wait4_ack (fd, NULL, 3000))goto err;		
		
        /* To trun on RF . AT+CFUN=1 can be used for single or mulitple SIM project */
        LOGD(TAG "Send AT+CFUN=1 to turn on RF \n");			
        send_at (fd, "AT+CFUN=1\r\n");		
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+CFUN=1 OK ,Start to test RF \n");	

        /* To set GSM only modem. To make sure GSM 900 work */
        LOGD(TAG "Send AT+ERAT=0 to set GSM only \n");			
        send_at (fd, "AT+ERAT=0\r\n");		
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+ERAT=0 OK ,Start to test RF \n");	

        /* Start RF test */		
        retryCount = 0;
		
        while(retryCount < 100){
            const char *tok = "+ECSQ";
            const char *minus_tok = "-";			
            char *p = NULL;
            char *minus_p = NULL;
            rssi_level = 0;
			
            /* Check RF RSSI level. The verdict of RF test is the RSSI level shall be greater than -80dbm  */
            LOGD(TAG "\n");
  	        usleep(HALT_TIME);

            memset(cmd_buf, 0, sizeof(cmd_buf));		  
            strcpy(cmd_buf, "AT+ECSQ\r\n");
            write(fd, cmd_buf, strlen(cmd_buf));
			
            LOGD(TAG "Send AT+ECSQ to check RF, retryCount=%d \n",retryCount);			

            memset(rsp_buf, 0, sizeof(rsp_buf));		  
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------AT+ECSQ start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+ECSQ end------\n");			
            retryCount++;
            p = strstr(rsp_buf, tok);						

            if(p!= NULL){
                LOGD(TAG "p=%s\n", p);				
                minus_p = strstr(p, minus_tok);							
				if(minus_p != NULL){
					/* (qdbm value) /4 = dbm value */
					LOGD(TAG "rssi_level str =%s\n", minus_p+1);									
					rssi_level = atoi(minus_p+1) / 4; 
				}					
            }			

            /* AT+ECSQ might got null immeidate response or rssi_in_qdbm=1 ,which means modem did not have any measurement result yet. keep retry polling */
            LOGD(TAG "rssi_level=%d\n", rssi_level);				
            if(rssi_level < 80 && rssi_level!=0){
                ret = 1;
                LOGD(TAG "rssi_level pass in RF test");				
                break;
            }else if(rssi_level >= 80){
                ret = 0;
                LOGD(TAG "rssi_level fail in RF test");				
				break;
            }
        }

        if(ret) {
            LOGD(TAG "RF Test result pass\n");			
            sprintf(rf->info + strlen(rf->info),
                "%s: %s. -%d dbm \n", uistr_rf_test, uistr_info_pass,rssi_level);
            close(fd);
            fd = -1;

            LOGD(TAG "%s: Exit\n", __FUNCTION__);
            iv->redraw(iv);
			
            return NULL;			
        } else {
            LOGD(TAG "RF Test result fail\n");			        
            goto err;  			            
        }			
		  	
        LOGD(TAG "redraw\n");
        iv->redraw(iv);
    } // end if(!sim->test_done)

err:
    LOGD(TAG "%s: FAIL\n",__FUNCTION__);
	
    sprintf(rf->info + strlen(rf->info),
         "%s: %s. -%d dbm \n", uistr_rf_test, uistr_info_fail,rssi_level);    			
	
    close(fd);
    fd = -1;

    LOGD(TAG "redraw\n");
    iv->redraw(iv);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return NULL;
}

int rf_test_entry(struct ftm_param *param, void *priv)
{
  bool exit = false;
  int  passCount = 0;
  struct rf_factory *rf = (struct rf_factory*)priv;
  struct itemview *iv = NULL;

  LOGD(TAG "%s: Start\n", __FUNCTION__);
  strcpy(rf->info,"");
  init_text(&rf->title, param->name, COLOR_YELLOW);
  init_text(&rf->text, &rf->info[0], COLOR_YELLOW);

  if(NULL == rf->iv) {
    iv = ui_new_itemview();
    if(!iv) {
      LOGD(TAG "No memory for item view");
      return -1;
    }
    rf->iv = iv;
  }
  iv = rf->iv;
  iv->set_title(iv, &rf->title);
  iv->set_items(iv, rf_items, 0);
  iv->set_text(iv, &rf->text);
  iv->start_menu(iv,0);
  iv->redraw(iv);

  rf->exit_thread = false;

  pthread_create(&rf->update_thread, NULL, rf_update_thread, priv);


  strcpy(rf->info, "");
  rf->test_done = false;
  while (strlen(rf->info) == 0) {
  	usleep(200000);
    if (strstr(rf->info, uistr_info_pass)) {
  	   passCount++;
  	}
  }
  LOGD(TAG "passCount = %d\n", passCount);

  //Exit RF Test thread
  rf->exit_thread = true;
  rf->test_done = true;

  pthread_join(rf->update_thread, NULL);

  //Check test result
  if (passCount == 1) {
     rf->mod->test_result = FTM_TEST_PASS;
  } else {
     rf->mod->test_result = FTM_TEST_FAIL;
  }

  LOGD(TAG "%s: End\n", __FUNCTION__);

  return 0;
}

int rf_test_init(void)
{
  int ret = 0;
  struct ftm_module *mod;
  struct rf_factory *rf;

  LOGD(TAG "%s: Start\n", __FUNCTION__);

  mod = ftm_alloc(ITEM_RF_TEST, sizeof(struct rf_factory));
  if(!mod) {
    return -ENOMEM;
  }
  rf = mod_to_rf(mod);
  rf->mod = mod;
  rf->test_done = true;

  ret = ftm_register(mod, rf_test_entry, (void*)rf);
  if(ret) {
    LOGD(TAG "register rf_test_entry failed (%d)\n", ret);
  }

  return ret;
}


#endif
