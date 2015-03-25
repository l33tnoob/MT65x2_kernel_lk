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

#include "atci_service.h"
#include "atci_generic_cmd_table.h"
#include <cutils/sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>


#define SOCKET_NAME_ATCI "atci-service"
#define SOCKET_NAME_ATCI_FRAMEWORK "atci-serv-fw"

int s_fdAtci_generic_listen = -1;
int s_fdAtci_generic_command = -1;

int s_fdAtci_mmi_listen = -1;
int s_fdAtci_mmi_command = -1;


#define MAX_DATA_SIZE 1024

static pthread_t s_tid_atci_service;
static pthread_t s_tid_atci_data_service;

static pthread_t s_tid_atci_mmi_service;
static pthread_t s_tid_atci_mmi_data_service;

pthread_cond_t start_atci_thread_cond;
pthread_mutex_t start_atci_data_thread_mutex;

pthread_cond_t start_atci_mmi_thread_cond;
pthread_mutex_t start_atci_mmi_data_thread_mutex;

//pthread_cond_t wait_for_atci_connect_cond;
//pthread_mutex_t wait_for_atci_connect_mutex;

int main_exit_signal = 0;
void setMainExit() {
    ALOGD("atci setMainExit !!! %d", main_exit_signal);
    main_exit_signal++;
    pthread_exit(0);
}
void* waitForAtciData(void *param){
    int ret = 0 ;
    fd_set rfds;
    FD_ZERO(&rfds);

    ALOGD("atci start to wait for atci data !!!");

    for (;;) {
        if (s_fdAtci_generic_command > 0) {
            FD_SET(s_fdAtci_generic_command, &rfds);
            ret = select(s_fdAtci_generic_command+1, &rfds, NULL, NULL, NULL);
            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;
                ALOGE("waitForAtciData atci-usb fail to select. error (%d)", errno);
                setMainExit();
                //exit(-1);
            }

            if (FD_ISSET(s_fdAtci_generic_command, &rfds)) {
                int recvLen = 0;
                char data[MAX_DATA_SIZE+1];
                memset(data, 0, sizeof(data));
                recvLen = recv(s_fdAtci_generic_command, data, MAX_DATA_SIZE, 0);
                if(recvLen > 0) {
                    ALOGD("The receive data from ATCID is %s with len:%d", data, recvLen);
                    process_generic_command(data);
                }
                else {
                    ALOGD("atci_service recvLen <= 0, atci_service exit, waitForAtciData, recvLen = %d", recvLen);
                    if(s_fdAtci_mmi_listen != -1)
                        close(s_fdAtci_mmi_listen);
                    if(s_fdAtci_generic_listen != -1)
                        close(s_fdAtci_generic_listen);
                    setMainExit();
                    //exit(-1);
                }
            }
            else {
                ALOGD("atci service is exit, waitForAtciData");
                if(s_fdAtci_mmi_listen != -1)
                    close(s_fdAtci_mmi_listen);
                if(s_fdAtci_generic_listen != -1)
                    close(s_fdAtci_generic_listen);
                setMainExit();
                //exit(-1);
            }
            
        } else {
            pthread_mutex_lock(&start_atci_data_thread_mutex);
            pthread_cond_wait(&start_atci_thread_cond, &start_atci_data_thread_mutex);
            pthread_mutex_unlock(&start_atci_data_thread_mutex);
        }
    }    
}

void startAtciServiceThread(){
    int ret = 0;

    ALOGD("create startAtciServiceThread thread");
    ret = pthread_create(&s_tid_atci_data_service, NULL, waitForAtciData, NULL);
    if (ret != 0) {
        ALOGE("fail to create atci service data thread errno:%d", errno);
        return;
    }
    ALOGD("create atci service data thread successfully");
}


void* waitForAtcidConnect(void *param){
    int ret;    
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    fd_set rfds;   
    ALOGI("atci start to wait for atcid connect!!!");

    for(;;) {
        FD_ZERO(&rfds);
        FD_SET(s_fdAtci_generic_listen, &rfds);
        ALOGI("wait client connect");
        ret = select(s_fdAtci_generic_listen+1, &rfds, NULL, NULL, NULL);
        
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            ALOGE(" waitForAtcidConnect fail to select. error (%d)", errno);
            setMainExit();
            //exit(-1);
        }

        if (FD_ISSET(s_fdAtci_generic_listen, &rfds)) {
            ALOGD("select atci service socket successfully!!!");            
            s_fdAtci_generic_command = accept(s_fdAtci_generic_listen, (struct sockaddr *)&peeraddr, &socklen);
            if (s_fdAtci_generic_command < 0) {
                ALOGE("fail to accept atci service socket. errno:%d", errno);
                setMainExit();
                //exit(-1);
            }
            ALOGD("accept atci service socket '%d' successfully!!!", s_fdAtci_generic_command);

            ret = fcntl(s_fdAtci_generic_command, F_SETFL, O_NONBLOCK);
            if (ret < 0) {
                ALOGE("fail to set atci-usb client socket O_NONBLOCK. errno: %d", errno);
            }
            
            ALOGI("atci: new atci-service connection");
            pthread_cond_signal(&start_atci_thread_cond); //Notify atci data thread to read data from ATCI dispatcher
            //pthread_mutex_lock(&wait_for_atci_connect_mutex);
            //pthread_cond_wait(&wait_for_atci_connect_cond, &wait_for_atci_connect_mutex);
            //pthread_mutex_unlock(&wait_for_atci_connect_mutex);
        }
        else {
            ALOGD("atci service is exit, waitForAtcidConnect");
            if(s_fdAtci_mmi_listen != -1)
                close(s_fdAtci_mmi_listen);
            if(s_fdAtci_generic_listen != -1)
                close(s_fdAtci_generic_listen);
            setMainExit();
            //exit(-1);
        }
    }
}

void startAtciServThread(){
    int ret = 0;

    ALOGD("create atci service socket thread");
    ret = pthread_create(&s_tid_atci_service, NULL, waitForAtcidConnect, NULL);
    if (ret != 0) {
        ALOGE("fail to create atci-usb server socket. errno:%d", errno);
        return;
    }
    
    ALOGD("create atci service socket thread successfully");
}

void process_mmi_response(char* data, int dataLen){
    int sendLen = 0;
                
    if(dataLen > 0){
        sendLen = send(s_fdAtci_generic_command, data, dataLen, 0);
        if(sendLen != dataLen){
            ALOGE("Data lost when send command response to atcid socket. errno = %d", errno);
        }
    }else{
        ALOGE("No data to send");
    }
}

void* receiveAtciFrameworkData(void *param){
    int ret = 0 ;
    fd_set rfds;
    FD_ZERO(&rfds);

    ALOGD("atci service is ready to receive data from framework !!!");

    for (;;) {
        if (s_fdAtci_mmi_command > 0) {
            FD_ZERO(&rfds);
            FD_SET(s_fdAtci_mmi_command, &rfds);
            ret = select(s_fdAtci_mmi_command+1, &rfds, NULL, NULL, NULL);
            if (ret < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;
                ALOGE("receiveAtciFrameworkData atci-usb fail to select. error (%d)", errno);
                setMainExit();
                //exit(-1);
            }

            if (FD_ISSET(s_fdAtci_mmi_command, &rfds)) {
                int recvLen = 0;
                char data[MAX_DATA_SIZE+1];
                memset(data, 0, sizeof(data));
                recvLen = recv(s_fdAtci_mmi_command, data, MAX_DATA_SIZE, 0);
                if(recvLen > 0) {
                    ALOGD("The rx data from android framework is %s with len:%d", data, recvLen);
                    process_mmi_response(data, recvLen);
                }
                else {
                    //ALOGD("atci_service recvLen <= 0, atci_service exit, receiveAtciFrameworkData, recvLen = %d", recvLen);
                    FD_CLR(s_fdAtci_mmi_command,&rfds);
                    if(-1 != s_fdAtci_mmi_command)
                    {
                        close(s_fdAtci_mmi_command);
                        s_fdAtci_mmi_command = -1;
                    }
#if 0					
                    if(s_fdAtci_mmi_listen != -1)
                        close(s_fdAtci_mmi_listen);
                    if(s_fdAtci_generic_listen != -1)
                        close(s_fdAtci_generic_listen);
                    setMainExit();
                    //exit(-1);
#endif					
                }
            }
            else {
                ALOGD("atci service is exit, receiveAtciFrameworkData");
                if(s_fdAtci_mmi_listen != -1)
                    close(s_fdAtci_mmi_listen);
                if(s_fdAtci_generic_listen != -1)
                    close(s_fdAtci_generic_listen);
                setMainExit();
                //exit(-1);
            }
        } else {
            pthread_mutex_lock(&start_atci_mmi_data_thread_mutex);
            pthread_cond_wait(&start_atci_mmi_thread_cond, &start_atci_mmi_data_thread_mutex);
            pthread_mutex_unlock(&start_atci_mmi_data_thread_mutex);
        }
    }    
}

void startAtciServiceFrameworkThread(){
    int ret = 0;

    ALOGD("create startAtciServiceFrameworkThread thread");
    ret = pthread_create(&s_tid_atci_mmi_data_service, NULL, receiveAtciFrameworkData, NULL);
    if (ret != 0) {
        ALOGE("fail to create atci service data thread errno:%d", errno);
        return;
    }
    ALOGD("create atci service data thread successfully");
}

void* waitForAtcidFrameworkConnect(void *param){
    int ret;    
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    fd_set rfds;   
    ALOGI("atci start to wait for atcid connect!!!");

    for(;;) {
        FD_ZERO(&rfds);
        FD_SET(s_fdAtci_mmi_listen, &rfds);
        ALOGI("wait for frmaework client connect");
        ret = select(s_fdAtci_mmi_listen+1, &rfds, NULL, NULL, NULL);
        
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;
            ALOGE("waitForAtcidFrameworkConnect fail to select. error (%d)", errno);
            setMainExit();
            //exit(-1);
        }

        if (FD_ISSET(s_fdAtci_mmi_listen, &rfds)) {
            ALOGD("select atci service framework socket successfully!!!");            
            s_fdAtci_mmi_command = accept(s_fdAtci_mmi_listen, (struct sockaddr *)&peeraddr, &socklen);
            if (s_fdAtci_mmi_command < 0) {
                ALOGE("fail to accept atci framework service socket. errno:%d", errno);
                setMainExit();
                //exit(-1);
            }
            ALOGD("accept atci framework service socket '%d' successfully!!!", s_fdAtci_mmi_command);

            ret = fcntl(s_fdAtci_mmi_command, F_SETFL, O_NONBLOCK);
            if (ret < 0) {
                ALOGE("fail to set atci framework client socket O_NONBLOCK. errno: %d", errno);
            }
            
            ALOGI("atci: new atci-service framework connection");
            pthread_cond_signal(&start_atci_mmi_thread_cond); //Notify atci data thread to read data from ATCI dispatcher
            //pthread_mutex_lock(&wait_for_atci_connect_mutex);
            //pthread_cond_wait(&wait_for_atci_connect_cond, &wait_for_atci_connect_mutex);
            //pthread_mutex_unlock(&wait_for_atci_connect_mutex);
        }
        else {
            ALOGD("atci service is exit, waitForAtcidFrameworkConnect");
            if(s_fdAtci_mmi_listen != -1)
                close(s_fdAtci_mmi_listen);
            if(s_fdAtci_generic_listen != -1)
                close(s_fdAtci_generic_listen);
            setMainExit();
            //exit(-1);
        }
    }
}


void startAtciServFrameworkThread(){
    int ret = 0;

    ALOGD("create atci service socket thread for framework service");
    ret = pthread_create(&s_tid_atci_mmi_service, NULL, waitForAtcidFrameworkConnect, NULL);
    if (ret != 0) {
        ALOGE("fail to create atci-framework server socket. errno:%d", errno);
        return;
    }
    
    ALOGD("create atci framework service socket thread successfully");
}

/*
Initial the name socket to listen the socket request from atcid process
*/

void initSocket(){
    int ret = 0;
    
    //Initalize the ATCI socket for native ATCI service
    s_fdAtci_generic_listen = android_get_control_socket(SOCKET_NAME_ATCI);
    if (s_fdAtci_generic_listen < 0) {
        ALOGE("Failed to get socket '" SOCKET_NAME_ATCI "' erron:%d", errno);
    }else{
        ret = listen(s_fdAtci_generic_listen, 4);
        if (ret < 0) {
            ALOGE("Failed to listen on control socket '%d': %s",
                 s_fdAtci_generic_listen, strerror(errno));
        }
    }

    //Initialize the ATCI framework for framework ATCI service
    s_fdAtci_mmi_listen = android_get_control_socket(SOCKET_NAME_ATCI_FRAMEWORK);
	
    if (s_fdAtci_mmi_listen < 0) {
        ALOGE("Failed to get socket '" SOCKET_NAME_ATCI_FRAMEWORK "' erron:%d", errno);
    }else{
        ret = listen(s_fdAtci_mmi_listen, 4);
        if (ret < 0) {
            ALOGE("Failed to listen on control socket on framework '%d': %s",
                 s_fdAtci_mmi_listen, strerror(errno));
        }
    }
    while(1){
    if(pthread_cond_init(&start_atci_mmi_thread_cond, NULL)){
	if (errno == EAGAIN){
	   ALOGE("mmi cond init fail for EAGAIN"); 
	   continue;
	}
	else{
	   ALOGE("mmi cond init fail, have to exit");
	   exit(-1);
	}
    }
	break;
   }
    while(1){
	if(pthread_cond_init(&start_atci_thread_cond, NULL)){
	if (errno == EAGAIN){ 
           ALOGE("atci cond init fail for EAGAIN");
           continue;
        }
        else{
           ALOGE("atci cond init fail, have to exit");
           exit(-1);
        }
    }
    break;
   }
   while(1){
	if(pthread_mutex_init(&start_atci_data_thread_mutex, NULL)){
	if (errno == EAGAIN){ 
           ALOGE("atci mutex init fail for EAGAIN");
           continue;
        }
        else{
           ALOGE("atci mutex init fail, have to exit");
           exit(-1);
        }
    }
    break;
   }
   while(1){
	if(pthread_mutex_init(&start_atci_mmi_data_thread_mutex, NULL)){
	if (errno == EAGAIN){ 
           ALOGE("atci mmi mutex init fail for EAGAIN");
           continue;
        }
        else{
           ALOGE("atci mmi mutex init fail, have to exit");
           exit(-1);
        }
    }
    break;
   }
}

void release_lock()
{
    ALOGD("Start release_lock");

    pthread_mutex_unlock(&start_atci_data_thread_mutex);
    pthread_mutex_unlock(&start_atci_mmi_data_thread_mutex);

    pthread_cond_signal(&start_atci_thread_cond);	
    pthread_cond_signal(&start_atci_mmi_thread_cond);	
	
    pthread_mutex_destroy(&start_atci_data_thread_mutex); 
    pthread_mutex_destroy(&start_atci_mmi_data_thread_mutex);
	
    pthread_cond_destroy(&start_atci_thread_cond);
    pthread_cond_destroy(&start_atci_mmi_thread_cond);
	
    ALOGD("End release_lock");	
}
/*
* Purpose:  The main program loop
* Input:      argc - number of input arguments
*                argv - array of strings (input arguments)
* Return:    0
*/
int main(int argc,char *argv[]){
    
    ALOGD("Start to run atci service");
    initSocket();
    
    startAtciServThread();
    startAtciServiceThread();
    
    startAtciServFrameworkThread();
    startAtciServiceFrameworkThread();
    while (main_exit_signal == 0) {
        sleep(5);
    }
    ALOGD("exit atci service %d", main_exit_signal);
    if (main_exit_signal != 0) {
        release_lock();
        exit(-1);
    }
    pthread_join(s_tid_atci_service, NULL);
    pthread_join(s_tid_atci_data_service, NULL);
    pthread_join(s_tid_atci_mmi_service, NULL);
    pthread_join(s_tid_atci_mmi_data_service, NULL);

    return 0;
}