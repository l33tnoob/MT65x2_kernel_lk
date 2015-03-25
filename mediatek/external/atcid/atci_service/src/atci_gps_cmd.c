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

#include "atci_gps_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define AT_COMMAND_SERVER_SOCK "/data/server"
#define AT_COMMAND_CLIENT_SOCK "/data/client"
#define RES_SIZE 30

int sock_fd = -1;

int gps_send_at_command(char * command){
    struct sockaddr_un client;
    struct sockaddr_un server;

    int left;
    char* cmdline = command;
    char result[RES_SIZE];    

    /* Create socket with GPS HAL */
    if((sock_fd = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
    {
        ALOGE("pas_gps_handler: open sock_fd failed\r\n"); 
        return -1;
    }
    
    unlink(AT_COMMAND_CLIENT_SOCK);
    memset(&client, 0, sizeof(client));
    client.sun_family = AF_LOCAL;
    strcpy(client.sun_path, AT_COMMAND_CLIENT_SOCK);
    if (bind(sock_fd, (struct sockaddr *)&client, sizeof(client)) < 0 )
    {
        ALOGE("Bind client error: %s\n", strerror(errno));
        return -1;
    }
    
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_LOCAL;
    strcpy(server.sun_path, AT_COMMAND_SERVER_SOCK);

    /* Send cmdline to HAL*/
    if (sendto(sock_fd, cmdline, strlen(cmdline), 0, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        ALOGE("send command to HAL fail:%s\r\n", strerror(errno));
    }else{
        ALOGD("send command to HAL successfully !!\n");        
    }
    
    return 0;
}

void gps_receive_at_result(char* response){   
    int left = 0;
    char result[30];
   
    ALOGD("Receive GPS AT test result\n");
    left = recvfrom(sock_fd, result, sizeof(result), 0, NULL, NULL);
    if (left < 0){
        ALOGE("recv test ack from GPS driver fail: %s\n", strerror(errno));                
    } else {
        ALOGD("recv test ack from GPS driver success: %d\n", left);
        
        if (!strcmp(result," ")){ //in case that test num is not correct
            sprintf(response,"\r\n%s\r\n", result);
        }else{
            sprintf(response,"\r\n%s\r\nOK\r\n\r\n", result);
        }
    }
}

int pas_gps_handler(char* cmdline, ATOP_t at_op, char* response){

    ALOGD("handle cmdline:%s, at_op: %d", cmdline, at_op);
    char test_command[20] = "AT%GPS=";

    switch(at_op){        
        case AT_ACTION_OP:
            //AT%GPS
            ALOGD("Action operation: AT%%GPS\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //Avg_CNo or ERROR
            break;
        case AT_TEST_OP:
            //AT%GPS=?           
            ALOGD("Test operation: AT%%GPS=?\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //Avg_CNo or ERROR
            break;
        case AT_READ_OP:
            //AT%GPS?
            ALOGD("Read operation: AT%%GPS?\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //GPS Test In Progress or Array                        
            break;
        case AT_SET_OP:
            //only test_num
            strcpy(test_command+7, cmdline);
            ALOGD("Set operation: AT%%GPS=N\n");
            if (-1 == gps_send_at_command(test_command))
                return -1;
            gps_receive_at_result(response);                     //GPS Test Start OK                  
            break;
        default:
            break;
    }
    
    return 0;
}

int pas_gnss_handler(char* cmdline, ATOP_t at_op, char* response){

    ALOGD("handle cmdline:%s, at_op: %d", cmdline, at_op);
    char test_command[30] = "AT%GNSS=";
    int num;

    switch(at_op){        
        case AT_ACTION_OP:
            //AT%GNSS
            ALOGD("Action operation: AT%%GNSS\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //Avg_CNo or ERROR
            break;
        case AT_TEST_OP:
            //AT%GNSS=?            
            ALOGD("Test operation: AT%%GNSS=?\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //Avg_CNo or ERROR
            break;
        case AT_READ_OP:
            //AT%GNSS?
            ALOGD("Read operation: AT%%GNSS?\n");
            if (-1 == gps_send_at_command(cmdline))
                return -1;
            gps_receive_at_result(response);                     //GPS Test In Progress or Array                        
            break;
        case AT_SET_OP:
            //only test_num
            strcpy(test_command+8, cmdline);
            ALOGD("Set operation: AT%%GNSS=%s\n", cmdline);
            if (-1 == gps_send_at_command(test_command))
                return -1;
            gps_receive_at_result(response);                     //GPS Test Start OK                  
            break;
        default:
            break;
    }
    
    return 0;
}




