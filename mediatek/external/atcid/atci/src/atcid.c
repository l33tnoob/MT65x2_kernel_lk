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

#include "atcid.h"
#include "atcid_serial.h"

#include <sys/socket.h>
#include <cutils/sockets.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*misc global vars */
Serial serial;
static int main_exit_signal=0;  /* 1:main() received exit signal */

/*
* Purpose:  The main program loop
* Input:      argc - number of input arguments
*                argv - array of strings (input arguments)
* Return:    0
*/
int main(int argc,char *argv[]) {
    int rc=-1;
    int i = 0;

    LOGATCI(LOG_INFO, "atcid-daemon start!!!");

    //Initial the parameter for serial dervice
    initSerialDevice(&serial);
    strcpy(serial.devicename[0],TTY_GS0);
    strcpy(serial.devicename[1],TTY_GS1);
    
    for(i = 0; i < MAX_DEVICE_VCOM_NUM; i++) {
        if ((serial.fd[i] = open_serial_device(&serial, serial.devicename[i])) == INVALIDE_SOCKET_FD) {
            LOGATCI(LOG_ERR, "Could not open serial device [%d] and start atci service", i);
            return ATCID_OPEN_SERIAL_DEV_ERR;
        }
    }
#if 0 /* For CTS security test, we can not use INET socket to listen on a port but only can use local socket instead.*/
    if(INVALIDE_SOCKET_FD == (serial.fd[ADB_SKT_SERVER_NUM] = socket(AF_INET, SOCK_STREAM, 0))) {
        LOGATCI(LOG_ERR, "Could not open serial device [%d] and start atci service", i);
        return ATCID_OPEN_SERIAL_DEV_ERR;
    }
#endif
    serial.fd[ADB_SKT_SERVER_NUM] = android_get_control_socket(ADB_ATCI_SOCKET);
    if (serial.fd[ADB_SKT_SERVER_NUM] < 0) {
        LOGATCI(LOG_ERR, "Failed to get socket '%s'", ADB_ATCI_SOCKET);
    } else {
        if(ATCID_SUCCESS != adb_socket_listen(serial.fd[ADB_SKT_SERVER_NUM])) {
            LOGATCI(LOG_ERR, "Could not open adb socket [%d] and start atci service", i);
            return ATCID_OPEN_SERIAL_DEV_ERR;
        }
    }
    
    readerLoop((void*) &serial);

    return 0;
}