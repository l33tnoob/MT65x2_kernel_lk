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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <cutils/properties.h>

#include "bt_em.h"

typedef unsigned long DWORD;
typedef unsigned long* PDWORD;
typedef unsigned long* LPDWORD;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned long HANDLE;
typedef void VOID;
typedef void* LPCVOID;
typedef void* LPVOID;
typedef void* LPOVERLAPPED;
typedef unsigned char* PUCHAR;
typedef unsigned char* PBYTE;
typedef unsigned char* LPBYTE;

#define TRUE           1
#define FALSE          0


#define LOG_TAG           "BT_RELAYER "
#include <cutils/log.h>

#define BT_RELAYER_DEBUG  1
#define ERR(f, ...)       ALOGE("%s: " f, __func__, ##__VA_ARGS__)
#define WAN(f, ...)       ALOGW("%s: " f, __func__, ##__VA_ARGS__)
#if BT_RELAYER_DEBUG
#define DBG(f, ...)       ALOGD("%s: " f, __func__, ##__VA_ARGS__)
#define TRC(f)            ALOGW("%s #%d", __func__, __LINE__)
#else
#define DBG(...)          ((void)0)
#define TRC(f)            ((void)0)
#endif

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static int serial_fd = -1;

static pthread_t txThread; // PC->BT moniter thread
static pthread_t rxThread; // BT->PC moniter thread

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static int uart_speed(int s)
{
    switch (s) {
    case 9600:
         return B9600;
    case 19200:
         return B19200;
    case 38400:
         return B38400;
    case 57600:
         return B57600;
    case 115200:
         return B115200;
    case 230400:
         return B230400;
    case 460800:
         return B460800;
    case 500000:
         return B500000;
    case 576000:
         return B576000;
    case 921600:
         return B921600;
    default:
         return B57600;
    }
}

/* Initialize serial port to PC */
static int init_serial(int port, int speed)
{
    struct termios ti;
    int fd;
    int baudenum;
    char dev[20] = {0};
    char usb_prop[PROPERTY_VALUE_MAX];
    
    if (port < 4){ //serial port UART
        sprintf(dev, "/dev/ttyMT%d", port);
    }
    else{ //serial port USB
        sprintf(dev, "/dev/ttyGS2");
    }
    
    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        ERR("Can't open serial port %s\n", dev);
        return -1;
    }
    
    tcflush(fd, TCIOFLUSH);
	  
    if (tcgetattr(fd, &ti) < 0) {
        ERR("Can't get serial port setting\n");
        close(fd);
        return -1;
    }
    
    cfmakeraw(&ti);
    
    if (port < 4){ //serial port UART
        ti.c_cflag |= CLOCAL;
        ti.c_lflag = 0;
        
        ti.c_cflag &= ~CRTSCTS;
        ti.c_iflag &= ~(IXON | IXOFF | IXANY);
        
        /* Set baudrate */
        baudenum = uart_speed(speed);
        if ((baudenum == B57600) && (speed != 57600)) {
            ERR("Serial port baudrate not supported!\n");
            close(fd);
            return -1;
        }
        
        cfsetospeed(&ti, baudenum);
        cfsetispeed(&ti, baudenum);
    }
    else{ //serial port USB
        /* Set USB property to acm_third: add 1 acm port to /dev/ttyGS2 */
        property_get("sys.usb.config", usb_prop, NULL);
        if(0 != strcmp(usb_prop, "acm_third")){
            if(property_set("sys.usb.config", "acm_third") < 0){
                ERR("Can't set USB property to open a VCOM\n");
                close(fd);
                return -1;
            }
            else{
                DBG("Set USB property to open a VCOM\n");
            }
        }
    }
    
    if (tcsetattr(fd, TCSANOW, &ti) < 0) {
        ERR("Can't set serial port setting\n");
        close(fd);
        return -1;
    }
    
    tcflush(fd, TCIOFLUSH);
    
    return fd;
}


static
int write_data_to_pc(int fd, unsigned char *buffer, unsigned long len)
{
    int bytesWritten = 0;
    int bytesToWrite = len;
    
    if (fd < 0) 
        return -1;
    
    // Send len bytes data in buffer
    while (bytesToWrite > 0){
        bytesWritten = write(fd, buffer, bytesToWrite);
        if (bytesWritten < 0){
            if(errno == EINTR || errno == EAGAIN)
                continue;
            else
                return -1;
        }
        bytesToWrite -= bytesWritten;
        buffer += bytesWritten;
    }

    return 0;
}

static
int read_data_from_pc(int fd, unsigned char *buffer, unsigned long len)
{
    int bytesRead = 0;
    int bytesToRead = len;
    
    int ret = 0;
    struct timeval tv;
    fd_set readfd;
    
    tv.tv_sec = 5;  //SECOND
    tv.tv_usec = 0;   //USECOND
    FD_ZERO(&readfd);
    
    if (fd < 0) 
        return -1;
    
    // Hope to receive len bytes
    while (bytesToRead > 0){
    
        FD_SET(fd, &readfd);
        ret = select(fd + 1, &readfd, NULL, NULL, &tv);
        
        if (ret > 0){
            bytesRead = read(fd, buffer, bytesToRead);
            if (bytesRead < 0){
                if(errno == EINTR || errno == EAGAIN)
                    continue;
                else
                    return -1;
            }
            else{
                bytesToRead -= bytesRead;
                buffer += bytesRead;
            }
        }
        else if (ret == 0){
            return -1; // read com port timeout 5000ms
        }
        else if ((ret == -1) && (errno == EINTR)){
            continue;
        }
        else{
            return -1;
        }
    }

    return 0;
}

static BOOL RELAYER_write(
    int  fd,
    unsigned char *peer_buf, 
    int  peer_len)
{

    if (peer_buf == NULL){
        ERR("NULL write buffer\n");
        return FALSE;
    }
    
    if ((peer_buf[0] != 0x04) && (peer_buf[0] != 0x02) && (peer_buf[0] != 0x03)){
        ERR("Invalid packet type 0x%02x to PC\n", peer_buf[0]);
        return FALSE;    
    }
    
    if (write_data_to_pc(fd, peer_buf, peer_len) < 0){
        return FALSE;
    }
    
    return TRUE;
}

static BOOL RELAYER_read(
    int  fd,
    unsigned char *peer_buf, 
    int  peer_len,
    int *piResultLen)
{
    
    UCHAR ucHeader = 0;
    int iLen = 0, pkt_len = 0, count = 0;
    
    if (peer_buf == NULL){
        ERR("NULL read buffer\n");
        return FALSE;
    }

LOOP:
    if(read_data_from_pc(fd, &ucHeader, sizeof(ucHeader)) < 0){
        count ++;
        if (count < 3){
            goto LOOP;
        }
        else{
            *piResultLen = iLen;
            return FALSE;
        }
    }
    
    peer_buf[0] = ucHeader;
    iLen ++;
    
    switch (ucHeader)
    {
      case 0x01:
        // HCI command
        if(read_data_from_pc(fd, &peer_buf[1], 3) < 0){
            ERR("Read command header failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 3;
        pkt_len = (int)peer_buf[3];
        if((iLen + pkt_len) > peer_len){
            ERR("Large packet from PC! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(read_data_from_pc(fd, &peer_buf[4], pkt_len) < 0){
            ERR("Read command param failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;
        
      case 0x02:
        // ACL data
        if(read_data_from_pc(fd, &peer_buf[1], 4) < 0){
            ERR("Read ACL header failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 4;
        pkt_len = (((int)peer_buf[4]) << 8);
        pkt_len += peer_buf[3];//little endian
        if((iLen + pkt_len) > peer_len){
            ERR("Large packet from PC! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(read_data_from_pc(fd, &peer_buf[5], pkt_len) < 0){
            ERR("Read ACL data failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;
        
      case 0x03:
        // SCO data
        if(read_data_from_pc(fd, &peer_buf[1], 3) < 0){
            ERR("Read SCO header failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 3;
        pkt_len = (int)peer_buf[3];
        if((iLen + pkt_len) > peer_len){
            ERR("Large packet from PC! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(read_data_from_pc(fd, &peer_buf[4], pkt_len) < 0){
            ERR("Read SCO data failed\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;
        
      default:
        // Filter PC garbage data
        ERR("Invalid packet type %02x from PC\n", ucHeader);
        *piResultLen = 0;
        return FALSE;
    }
    
    return TRUE;
}


static void thread_exit(int signo)
{
    pthread_t tid = pthread_self();
    DBG("Thread %lu exits\n", tid);
    pthread_exit(0);
}

static void *bt_tx_monitor(void *ptr)
{
    UCHAR ucTxBuf[512];
    int iPktLen;
    
    DBG("Thread %lu starts\n", txThread);
    
    while (1){
        memset(ucTxBuf, 0, sizeof(ucTxBuf));
        iPktLen = 0;
        
        // Receive HCI packet from PC
        if(RELAYER_read(serial_fd, ucTxBuf, sizeof(ucTxBuf), &iPktLen))
        {
            DBG("Receive packet from PC\n");
            // Send the packet to BT Controller
            if(EM_BT_write(ucTxBuf, iPktLen)){
                sched_yield();
                continue;
            }
            else{
                ERR("Send packet to BT Controller failed\n");
                pthread_kill(rxThread, SIGRTMIN);
                break;
            }   
        }
        else{
            if (iPktLen == 0){
                continue;
            }
            else{
                ERR("Receive packet from PC failed\n");
                pthread_kill(rxThread, SIGRTMIN);
                break;
            }
        }
    }
    return 0;
}

static void *bt_rx_monitor(void *ptr)
{
    UCHAR ucRxBuf[512];
    int iPktLen;
    
    DBG("Thread %lu starts\n", rxThread);
    
    while (1){
        memset(ucRxBuf, 0, sizeof(ucRxBuf));
        int iPktLen = 0;
        
        // Receive HCI packet from BT Controller
        if(EM_BT_read(ucRxBuf, sizeof(ucRxBuf), &iPktLen))
        {
            DBG("Send packet to PC\n");            
            // Send the packet to PC
            if(RELAYER_write(serial_fd, ucRxBuf, iPktLen)){
                sched_yield();
                continue;
            }
            else{
                ERR("Send packet to PC failed\n");
                pthread_kill(txThread, SIGRTMIN);
                break;
            }
        }
        else{
            if (iPktLen == 0){
                continue;
            }
            else{
                ERR("Receive packet from BT Controller failed\n");
                pthread_kill(txThread, SIGRTMIN);
                break;
            }
        }
    }
    return 0;
}

BOOL RELAYER_start(int serial_port, int serial_speed)
{
    TRC();
    
    if (EM_BT_init()){
        DBG("BT device power on success\n");
    }
    else{
        ERR("BT device power on failed\n");
        return FALSE;
    }
    
    serial_fd = init_serial(serial_port, serial_speed);
    if (serial_fd < 0){
        ERR("Initialize serial port to PC failed\n");
        EM_BT_deinit();
        return FALSE;
    }
    
    signal(SIGRTMIN, thread_exit);
    /* Create Tx monitor thread */
    pthread_create(&txThread, NULL, bt_tx_monitor, (void*)NULL);
    /* Create RX monitor thread */
    pthread_create(&rxThread, NULL, bt_rx_monitor, (void*)NULL);
    
    DBG("BT Relayer mode start\n");
    
    return TRUE;
}

void RELAYER_exit()
{
    TRC();
    
    /* Wait until thread exit */
    pthread_kill(txThread, SIGRTMIN);
    pthread_join(txThread, NULL);
    pthread_kill(rxThread, SIGRTMIN);
    pthread_join(rxThread, NULL);
    
    close(serial_fd);
    serial_fd = -1;
    
    EM_BT_deinit();           	
}
