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
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

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


#define LOG_TAG         "BT_EM "
#include <cutils/log.h>

#define BT_EM_DEBUG     1
#define ERR(f, ...)     ALOGE("%s: " f, __func__, ##__VA_ARGS__)
#define WAN(f, ...)     ALOGW("%s: " f, __func__, ##__VA_ARGS__)
#if BT_EM_DEBUG
#define DBG(f, ...)     ALOGD("%s: " f, __func__, ##__VA_ARGS__)
#define TRC(f)          ALOGW("%s #%d", __func__, __LINE__)
#else
#define DBG(...)        ((void)0)
#define TRC(f)          ((void)0)
#endif

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

static int   bt_fd = -1;

/* Used to read serial port */
static pthread_t rxThread;

// mtk bt library
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*UNINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buffer, unsigned long len);
typedef int (*READ)(int fd, unsigned char *buffer, unsigned long len);

INIT    mtk = NULL;
UNINIT  bt_restore = NULL;
WRITE   bt_send_data = NULL;
READ    bt_receive_data = NULL;

/*
//default patch
//--------------------------------6620--------------------------------
#if defined MTK_MT6620
static char COMBO_BUILT_IN_PATCH_FILE_NAME_E1[] = "/system/etc/firmware/mt6620_patch_hdr.bin";
static char COMBO_BUILT_IN_PATCH_FILE_NAME_E3[] = "/system/etc/firmware/mt6620_patch_e3_hdr.bin";
static char COMBO_BUILT_IN_PATCH_FILE_NAME_E6[] = "/system/etc/firmware/mt6620_patch_e6_hdr.bin";
//--------------------------------6628--------------------------------
#elif defined MTK_MT6628
static char COMBO_BUILT_IN_PATCH_FILE_NAME_E1[] = "/system/etc/firmware/mt6628_patch_e1_hdr.bin";
static char COMBO_BUILT_IN_PATCH_FILE_NAME_E2[] = "/system/etc/firmware/mt6628_patch_e2_hdr.bin"; 
#endif

static BT_HW_ECO  bt_hw_eco;
*/
/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

BOOL EM_BT_init(void)
{
    const char *errstr;
    
    TRC();
    
    glib_handle = dlopen("libbluetooth_mtk.so", RTLD_LAZY);
    if (!glib_handle){
        ERR("%s\n", dlerror());
        goto error;
    }
    
    dlerror(); /* Clear any existing error */
    
    mtk = dlsym(glib_handle, "mtk");
    bt_restore = dlsym(glib_handle, "bt_restore");
    bt_send_data = dlsym(glib_handle, "bt_send_data");
    bt_receive_data = dlsym(glib_handle, "bt_receive_data");
    
    if ((errstr = dlerror()) != NULL){
        ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }
    
    bt_fd = mtk();
    if (bt_fd < 0)
        goto error;

    DBG("BT is enabled success\n");
    
    return TRUE;

error:
    if (glib_handle){
        dlclose(glib_handle);
        glib_handle = NULL;
    }
    
    return FALSE;
}

void EM_BT_deinit(void)
{
    TRC();
    
    if (!glib_handle){
        ERR("mtk bt library is unloaded!\n");
    }
    else{
        if (bt_fd < 0){
            ERR("bt driver fd is invalid!\n");
        }
        else{
            bt_restore(bt_fd);
            bt_fd = -1;
        }
        dlclose(glib_handle);
        glib_handle = NULL;
    }
    
    return;
}

BOOL EM_BT_write(
    unsigned char *peer_buf, 
    int  peer_len)
{
    int i;
    
    TRC();
    
    if (peer_buf == NULL){
        ERR("NULL write buffer\n");
        return FALSE;
    }
    
    if ((peer_buf[0] != 0x01) && (peer_buf[0] != 0x02) && (peer_buf[0] != 0x03)){
        ERR("Invalid packet type 0x%02x\n", peer_buf[0]);
        return FALSE;    
    }
    
    if (!glib_handle){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }
    
    if (bt_send_data(bt_fd, peer_buf, peer_len) < 0){
        return FALSE;
    }
    
    // Dump tx packet
    DBG("write:\n");
    for (i = 0; i < peer_len; i++) {
        DBG("%02x\n", peer_buf[i]);
    }
    
    return TRUE;
}

BOOL EM_BT_read(
    unsigned char *peer_buf, 
    int  peer_len,
    int *piResultLen)
{
    UCHAR ucHeader = 0;
    int iLen = 0, pkt_len = 0, count = 0;
    int i;
	  
    TRC();
	  
    if (peer_buf == NULL){
        ERR("NULL read buffer\n");
        return FALSE;
    }
    
    if (!glib_handle){
        ERR("mtk bt library is unloaded!\n");
        return FALSE;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return FALSE;
    }

LOOP:
    if(bt_receive_data(bt_fd, &ucHeader, sizeof(ucHeader)) < 0){
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
      case 0x04:
        DBG("Receive HCI event\n");
        if(bt_receive_data(bt_fd, &peer_buf[1], 2) < 0){
            ERR("Read event header fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 2;
        pkt_len = (int)peer_buf[2];
        if((iLen + pkt_len) > peer_len){
            ERR("Read buffer overflow! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(bt_receive_data(bt_fd, &peer_buf[3], pkt_len) < 0){
            ERR("Read event param fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;
           
      case 0x02:
        DBG("Receive ACL data\n");
        if(bt_receive_data(bt_fd, &peer_buf[1], 4) < 0){
            ERR("Read ACL header fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 4;
        pkt_len = (((int)peer_buf[4]) << 8);
        pkt_len += peer_buf[3];//little endian
        if((iLen + pkt_len) > peer_len){
            ERR("Read buffer overflow! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(bt_receive_data(bt_fd, &peer_buf[5], pkt_len) < 0){
            ERR("Read ACL data fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;
        
      case 0x03:
        DBG("Receive SCO data\n");
        if(bt_receive_data(bt_fd, &peer_buf[1], 3) < 0){
            ERR("Read SCO header fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += 3;
        pkt_len = (int)peer_buf[3];
        if((iLen + pkt_len) > peer_len){
            ERR("Read buffer overflow! packet len %d\n", iLen + pkt_len);
            *piResultLen = iLen;
            return FALSE;
        }
        
        if(bt_receive_data(bt_fd, &peer_buf[4], pkt_len) < 0){
            ERR("Read SCO data fails\n");
            *piResultLen = iLen;
            return FALSE;
        }
        
        iLen += pkt_len;
        *piResultLen = iLen;
        break;                
        
      default:
        ERR("Unexpected BT packet header %02x\n", ucHeader);
        *piResultLen = iLen;
        return FALSE;
    }
    
    // Dump rx packet
    DBG("read:\n");
    for (i = 0; i < iLen; i++) {
        DBG("%02x\n", peer_buf[i]);
    }
    
    // If debug event, drop and retry
    if ((peer_buf[0] == 0x04) && (peer_buf[1] == 0xE0)){
        memset(peer_buf, 0, peer_len);
        iLen = 0;
        count = 0;
        goto LOOP;
    }
    
    return TRUE;
}

static void thread_exit(int signo)
{
    pthread_t tid = pthread_self();
    DBG("Thread %lu exits\n", tid);
    pthread_exit(0);
}

static void *BT_Rx_Thread(void *ptr)
{
    UCHAR ucRxBuf[512];
    int len = 512;
    int iResultLen;
    
    DBG("Thread %lu starts\n", rxThread);
    while(1){
        EM_BT_read(ucRxBuf, len, &iResultLen);
    }
    
    return 0;
}

void EM_BT_polling_start()
{
    DBG("Test start! Keep polling event from Controller\n");
    
    signal(SIGRTMIN, thread_exit);
    pthread_create(&rxThread, NULL, BT_Rx_Thread, NULL);
    return;
}

void EM_BT_polling_stop()
{
    DBG("Test about to end! Stop polling event from Controller\n");
    
    pthread_kill(rxThread, SIGRTMIN);
    /* Wait until thread exit */
    pthread_join(rxThread, NULL);
    signal(SIGRTMIN, SIG_DFL);
    return;
}


void EM_BT_getChipInfo(BT_CHIP_ID *chip_id, BT_HW_ECO *eco_num)
{
#if 0
    BT_CHIP_ID id;
      
    UCHAR HCI_VS_GET_HW_VER[] = 
              {0x01, 0xD1, 0xFC, 0x04, 0x00, 0x00, 0x00, 0x80};
    UCHAR pAckEvent[11];
    UCHAR ucEvent[] = {0x04, 0x0E, 0x08, 0x01, 0xD1, 0xFC, 0x00};
    
    TRC();
    
#ifdef MTK_MT6611
    id = BT_CHIP_ID_MT6611;
#elif defined MTK_MT6612
    id = BT_CHIP_ID_MT6612;
#elif defined MTK_MT6616
    id = BT_CHIP_ID_MT6616;
#elif defined MTK_MT6620
    id = BT_CHIP_ID_MT6620;
#elif defined MTK_MT6622
    id = BT_CHIP_ID_MT6622;
#elif defined MTK_MT6626
    id = BT_CHIP_ID_MT6626;
#elif defined MTK_MT6628
    id = BT_CHIP_ID_MT6628;
#endif
    *chip_id = id;
    *eco_num = BT_HW_ECO_UNKNOWN;
    
    // Try to get chip HW ECO
    if (!glib_handle){
        ERR("mtk bt library is unloaded!\n");
        return;
    }
    if (bt_fd < 0){
        ERR("bt driver fd is invalid!\n");
        return;
    }
    
    if(bt_send_data(bt_fd, HCI_VS_GET_HW_VER, sizeof(HCI_VS_GET_HW_VER)) < 0){
        ERR("Send get HW version command fails errno %d\n", errno);
        return;
    }
    
    if(bt_receive_data(bt_fd, pAckEvent, sizeof(pAckEvent)) < 0){
        ERR("Receive event fails errno %d\n", errno);
        return;
    }
    
    if(memcmp(pAckEvent, ucEvent, sizeof(ucEvent))){
        ERR("Receive unexpected event\n");
    }
    else{
        DBG("event 0xbbaa: %02x %02x\n", pAckEvent[8], pAckEvent[7]);
        
    #ifdef MTK_COMBO_SUPPORT
        /* Combo chip */
        switch (pAckEvent[7]){
        #ifdef MTK_MT6620
          case 0x00:
               bt_hw_eco = BT_HW_ECO_E1;
               break;
          case 0x01:
               bt_hw_eco = BT_HW_ECO_E2;
               break;
          case 0x10:
               bt_hw_eco = BT_HW_ECO_E3;
               break;
          case 0x11:
               bt_hw_eco = BT_HW_ECO_E4;
               break;
          case 0x30:
               bt_hw_eco = BT_HW_ECO_E6;
               break;
          case 0x31:
               bt_hw_eco = BT_HW_ECO_E7;
               break;
        #else
          case 0x00:
               bt_hw_eco = BT_HW_ECO_E1;
               break;
          case 0x10:
               bt_hw_eco = BT_HW_ECO_E2;
               break;
        #endif
          default:
               bt_hw_eco = BT_HW_ECO_UNKNOWN;
               break;
        }
    #else
        /* Standalone BT chip */
        switch (pAckEvent[7]){
          case 0x00:
               bt_hw_eco = BT_HW_ECO_E1;
               break;
          case 0x01:
               bt_hw_eco = BT_HW_ECO_E2;
               break;
          case 0x02:
               bt_hw_eco = BT_HW_ECO_E3;
               break;
          case 0x03:
               bt_hw_eco = BT_HW_ECO_E4;
               break;
          default:
               bt_hw_eco = BT_HW_ECO_UNKNOWN;
               break;
        }
    #endif
        *eco_num = bt_hw_eco;
    }
#endif    
    return;
}

void EM_BT_getPatchInfo(char *patch_id, unsigned long *patch_len)
{
#if 0
    DWORD   dwPatchExtLen = 0;
    FILE*   pPatchExtFile = NULL;
    DWORD   dwPatchLen = 0;
    FILE*   pPatchFile = NULL;
    size_t  szReadLen = 0;
    char    patch_hdr[30] = "N/A";
    
    TRC();

#ifdef MTK_COMBO_SUPPORT
    DBG("Load Combo firmware patch\n");
    
    switch (bt_hw_eco){
    #ifdef MTK_MT6620
      case BT_HW_ECO_E1:
      case BT_HW_ECO_E2:
           pPatchFile = fopen(COMBO_BUILT_IN_PATCH_FILE_NAME_E1, "rb");
           if(pPatchFile != NULL) {
               WAN("Open %s\n", COMBO_BUILT_IN_PATCH_FILE_NAME_E1);
           }
           else {
               ERR("Can't get valid patch file\n");
           }
           break;
      case BT_HW_ECO_E3:
      case BT_HW_ECO_E4:
      case BT_HW_ECO_E5:
      	   pPatchFile = fopen(COMBO_BUILT_IN_PATCH_FILE_NAME_E3, "rb");
           if(pPatchFile != NULL){
               WAN("Open %s\n", COMBO_BUILT_IN_PATCH_FILE_NAME_E3);
           }
           else {
               ERR("Can't get valid patch file\n");
           }
      	   break;
      case BT_HW_ECO_E6:
      case BT_HW_ECO_E7:
      	   pPatchFile = fopen(COMBO_BUILT_IN_PATCH_FILE_NAME_E6, "rb");
           if(pPatchFile != NULL){
               WAN("Open %s\n", COMBO_BUILT_IN_PATCH_FILE_NAME_E6);
           }
           else {
               ERR("Can't get valid patch file\n");
           }
      	   break;
    #else
      case BT_HW_ECO_E1:
           pPatchFile = fopen(COMBO_BUILT_IN_PATCH_FILE_NAME_E1, "rb");
           if(pPatchFile != NULL) {
               WAN("Open %s\n", COMBO_BUILT_IN_PATCH_FILE_NAME_E1);
           }
           else {
               ERR("Can't get valid patch file\n");
           }
           break;
      case BT_HW_ECO_E2:
           pPatchFile = fopen(COMBO_BUILT_IN_PATCH_FILE_NAME_E2, "rb");
           if(pPatchFile != NULL) {
               WAN("Open %s\n", COMBO_BUILT_IN_PATCH_FILE_NAME_E2);
           }
           else {
               ERR("Can't get valid patch file\n");
           }
           break;
    #endif
      default:
      	   ERR("No ECO version, don't known which patch to load\n");
      	   break;
    }
    
    if(pPatchFile != NULL){
       if(fseek(pPatchFile, 0, SEEK_END) != 0){
            ERR("fseek patch file fails errno: %d\n", errno);
        }
        else{
            dwPatchLen = ftell(pPatchFile);
            DBG("Patch file size %d\n", (int)dwPatchLen);
            if (dwPatchLen <= 28){
                // patch header needs 28 bytes at least
                ERR("Patch error len!\n");
            }
            
            /* back to file beginning */
            rewind(pPatchFile);
            
            memset(patch_hdr, 0, sizeof(patch_hdr));
            szReadLen = fread(patch_hdr, 1, 16, pPatchFile);
            if (szReadLen < 16){
               patch_hdr[szReadLen] = '\0';
            }
            else{
               patch_hdr[14] = ' ';
               szReadLen = fread(patch_hdr + 15, 1, 4, pPatchFile);
               szReadLen += 15;
               patch_hdr[szReadLen] = '\0';
            }
            
            DBG("Patch hdr: %s\n", patch_hdr);
        }
    }
    
    *patch_len = dwPatchLen;
    strcpy(patch_id, patch_hdr);
    
#else
    DBG("Load BT firmware patch\n");

#if BT_PATCH_EXT_ENABLE
    pPatchExtFile = fopen(BT_UPDATE_PATCH_EXT_FILE_NAME, "rb"); 
    
    /* if there is no adhoc file, use built-in patch file under system etc firmware */
    if(pPatchExtFile == NULL){
        pPatchExtFile = fopen(BT_BUILT_IN_PATCH_EXT_FILE_NAME, "rb");
        if(pPatchExtFile != NULL) {
            WAN("Open %s\n", BT_BUILT_IN_PATCH_EXT_FILE_NAME);
        }
        else {
            ERR("Can't get valid patch ext file\n");
        }
    }
    else {
        WAN("Open %s\n", BT_UPDATE_PATCH_EXT_FILE_NAME);
    }
    
    /* file exists */
    if(pPatchExtFile != NULL){
        if(fseek(pPatchExtFile, 0, SEEK_END) != 0){
            ERR("fseek patch ext file fails errno: %d\n", errno);
        }else{
            dwPatchExtLen = ftell(pPatchExtFile);
            DBG("Patch ext file size %d\n", (int)dwPatchExtLen);
        }
    }
#endif

    /* Use data directory first. for future update test convinience */
    pPatchFile = fopen(BT_UPDATE_PATCH_FILE_NAME, "rb"); 
    
    /* if there is no adhoc file, use built-in patch file under system etc firmware */
    if(pPatchFile == NULL){
        pPatchFile = fopen(BT_BUILT_IN_PATCH_FILE_NAME, "rb");
        if(pPatchFile != NULL) {
            WAN("Open %s\n", BT_BUILT_IN_PATCH_FILE_NAME);
        }
        else {
            ERR("Can't get valid patch file\n");
        }
    }
    else {
        WAN("Open %s\n", BT_UPDATE_PATCH_FILE_NAME);
    }
    
    /* file exists */
    if(pPatchFile != NULL){
        if(fseek(pPatchFile, 0, SEEK_END) != 0){
            ERR("feek patch file fails errno: %d\n", errno);
        }else{            
            dwPatchLen = ftell(pPatchFile);
            DBG("Patch file size %d\n", (int)dwPatchLen);
        }
    }
    
    *patch_len = dwPatchLen + dwPatchExtLen;
    // Standalone chip no patch header info
    strcpy(patch_id, "N/A");
#endif
#endif
    return;
}
