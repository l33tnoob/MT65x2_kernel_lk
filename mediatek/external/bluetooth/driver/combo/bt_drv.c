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
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "CFG_BT_File.h"
#include "CFG_BT_Default.h"
#include "bt_drv.h"
#include "os_dep.h"


/* For BT_HOT_OP_SET_FWASSERT */
#define COMBO_IOC_MAGIC              0xb0
#define COMBO_IOCTL_FW_ASSERT        _IOWR(COMBO_IOC_MAGIC, 0, void*)

/* For BT_AUDIO_OP_GET_CONFIG */
struct audio_t {
  int chip_id;
  AUDIO_CONFIG audio_conf;
};

/**************************************************************************
 *                  G L O B A L   V A R I A B L E S                       *
***************************************************************************/

// mtk bt library
static void *glib_handle = NULL;
typedef int (*INIT)(void);
typedef int (*UNINIT)(int fd);
typedef int (*WRITE)(int fd, unsigned char *buffer, unsigned long len);
typedef int (*READ)(int fd, unsigned char *buffer, unsigned long len);
typedef int (*NVRAM)(unsigned char *ucNvRamData);
typedef int (*GETID)(unsigned long *pChipId);


INIT    mtk = NULL;
UNINIT  bt_restore = NULL;
WRITE   write_com_port = NULL;
READ    read_com_port = NULL;
NVRAM   bt_read_nvram = NULL;
GETID   bt_get_combo_id = NULL;


/* Audio interface & Codec information Mapping */
struct audio_t audio_conf_map[] = {
  { 0x6620,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },

#if defined(MTK_MERGE_INTERFACE_SUPPORT)
  { 0x6628,    { MERGE_INTERFACE,  SYNC_8K,  SHORT_FRAME,  0 } },
#else
  { 0x6628,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },
#endif

  { 0x6572,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },

  { 0x6582,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },

  { 0x6592,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },

  { 0x6571,    { CVSD_REMOVAL,     SYNC_8K,  SHORT_FRAME,  0 } },

#if defined(MTK_MERGE_INTERFACE_SUPPORT)
  { 0x6630,    { MERGE_INTERFACE,  SYNC_8K,  SHORT_FRAME,  0 } },
#else
  { 0x6630,    { PCM,              SYNC_8K,  SHORT_FRAME,  0 } },
#endif

  { 0,         { 0 } }
};

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

static void wait_whole_chip_reset_complete(int bt_fd)
{
    UCHAR temp;
    int   res;
    
    do {
        res = read(bt_fd, &temp, 1);
        if (res < 0){
            if (errno == 88)
                usleep(200000);
            else if (errno == 99)
                break;
            else if (errno != EINTR && errno != EAGAIN)
                break;
        }
        else{
            break; // impossible case
        }
    } while(1);
}

int mtk_bt_enable(int flag, void *func_cb)
{
    const char *errstr;
    int bt_fd = -1;
    
    LOG_TRC();
    
    glib_handle = dlopen("libbluetooth_mtk.so", RTLD_LAZY);
    if (!glib_handle){
        LOG_ERR("%s\n", dlerror());
        goto error;
    }
    
    dlerror(); /* Clear any existing error */
    
    mtk = dlsym(glib_handle, "mtk");
    bt_restore = dlsym(glib_handle, "bt_restore");
    write_com_port = dlsym(glib_handle, "write_com_port");
    read_com_port = dlsym(glib_handle, "read_com_port");
    
    if ((errstr = dlerror()) != NULL){
        LOG_ERR("Can't find function symbols %s\n", errstr);
        goto error;
    }
    
    bt_fd = mtk();
    if (bt_fd < 0)
        goto error;

    LOG_DBG("BT is enabled success\n");
    
    return bt_fd;

error:
    if (glib_handle){
        dlclose(glib_handle);
        glib_handle = NULL;
    }
    return -1;
}

int mtk_bt_disable(int bt_fd)
{
    LOG_TRC();

    if (!glib_handle){
        LOG_ERR("mtk bt library is unloaded!\n");
        return -1;
    }
    
    bt_restore(bt_fd);
    dlclose(glib_handle);
    glib_handle = NULL;
    
    return 0;
}

int mtk_bt_write(int bt_fd, unsigned char *buffer, unsigned long len)
{
    int ret_val;
    
    LOG_DBG("buffer %x, len %d\n", buffer, len);

    if (!glib_handle){
        LOG_ERR("mtk bt library is unloaded!\n");
        return -1;
    }
    
    ret_val = write_com_port(bt_fd, buffer, len);
    
    if (ret_val < 0 && (ret_val == -88)){
        // whole chip reset, wait it complete (errno 99)
        wait_whole_chip_reset_complete(bt_fd);
        ret_val = -99;
    }
    
    return ret_val;
}

int mtk_bt_read(int bt_fd, unsigned char *buffer, unsigned long len)
{
    int ret_val;
    
    LOG_DBG("buffer %x, len %d\n", buffer, len);

    if (!glib_handle){
        LOG_ERR("mtk bt library is unloaded!\n");
        return -1;
    }
    
    ret_val = read_com_port(bt_fd, buffer, len);
    
    if (ret_val < 0 && (ret_val == -88)){
        // whole chip reset, wait it complete (errno 99)
        wait_whole_chip_reset_complete(bt_fd);
        ret_val = -99;
    }
    
    return ret_val;	
}

void mtk_bt_op(BT_REQ req, BT_RESULT *result)
{
    result->status = FALSE;
    
    switch(req.op)
    {
      case BT_COLD_OP_GET_ADDR:
      {
        const char *errstr;
        unsigned char nvram[sizeof(ap_nvram_btradio_struct)];
        unsigned char ucDefaultAddr[6] = {0};
        unsigned long chipId;
        
        LOG_DBG("BT_COLD_OP_GET_ADDR\n");
        
        glib_handle = dlopen("libbluetooth_mtk.so", RTLD_LAZY);
        if (!glib_handle){
            LOG_ERR("%s\n", dlerror());
            return;
        }
        
        dlerror(); /* Clear any existing error */
        
        bt_read_nvram = dlsym(glib_handle, "bt_read_nvram");
        bt_get_combo_id = dlsym(glib_handle, "bt_get_combo_id");
        
        if ((errstr = dlerror()) != NULL){
            LOG_ERR("Can't find function symbols %s\n", errstr);
            dlclose(glib_handle);
            glib_handle = NULL;
            return;
        }
        
        if(bt_read_nvram(nvram) < 0){
            LOG_ERR("Read Nvram data fails\n");
            dlclose(glib_handle);
            glib_handle = NULL;
            return;
        }
        
        /* Get combo chip id */
        if(bt_get_combo_id(&chipId) < 0){
            LOG_ERR("Get combo chip id fails\n");
            dlclose(glib_handle);
            glib_handle = NULL;
            return;
        }
        
        dlclose(glib_handle);
        glib_handle = NULL;
        
        switch(chipId)
        {
          case 0x6620:
            memcpy(ucDefaultAddr, stBtDefault_6620.addr, 6);
            break;
          case 0x6628:
            memcpy(ucDefaultAddr, stBtDefault_6628.addr, 6);
            break;
          case 0x6572:
            memcpy(ucDefaultAddr, stBtDefault_6572.addr, 6);
            break;
          case 0x6582:
            memcpy(ucDefaultAddr, stBtDefault_6582.addr, 6);
            break;
          case 0x6592:
            memcpy(ucDefaultAddr, stBtDefault_6592.addr, 6);
            break;
          case 0x6571:
            memcpy(ucDefaultAddr, stBtDefault_6571.addr, 6);
            break;
          case 0x6630:
            memcpy(ucDefaultAddr, stBtDefault_6630.addr, 6);
            break;
          default:
            LOG_ERR("Unknown combo chip id\n");
            return;
        }
        
        result->status = TRUE;
        if (0 == memcmp(nvram, ucDefaultAddr, 6))
        {
            LOG_DBG("Nvram BD address default value\n");
            result->param.addr[0] = 0;  //default address
            memcpy(&result->param.addr[1], nvram, 6);
        }
        else {
            LOG_DBG("Nvram BD address has valid value\n");
            result->param.addr[0] = 1;  //valid address
            memcpy(&result->param.addr[1], nvram, 6);
        }
        break;
      }
      case BT_HOT_OP_SET_FWASSERT:
      {
        LOG_DBG("BT_HOT_OP_SET_FWASSERT\n");
        
        // req.param.assert.fd should be the fd returned by mtk_bt_enable
        if (req.param.assert.fd < 0){
            LOG_ERR("Invalid bt fd!\n");
            return;
        }
        
        if (ioctl(req.param.assert.fd, COMBO_IOCTL_FW_ASSERT, \
            req.param.assert.reason) < 0){
            LOG_ERR("Set COMBO FW ASSERT fails\n");
            return;
        }
        
        result->status = TRUE;
        break;
      }
      case BT_AUDIO_OP_GET_CONFIG:
      {
        const char *errstr;
        unsigned long chipId;
        int i;
        
        LOG_DBG("BT_AUDIO_OP_GET_CONFIG\n");
        
        glib_handle = dlopen("libbluetooth_mtk.so", RTLD_LAZY);
        if (!glib_handle){
            LOG_ERR("%s\n", dlerror());
            return;
        }
        
        dlerror(); /* Clear any existing error */
        
        bt_get_combo_id = dlsym(glib_handle, "bt_get_combo_id");
        
        if ((errstr = dlerror()) != NULL){
            LOG_ERR("Can't find function symbols %s\n", errstr);
            dlclose(glib_handle);
            glib_handle = NULL;
            return;
        }
        
        /* Get combo chip id */
        if(bt_get_combo_id(&chipId) < 0){
            LOG_ERR("Get combo chip id fails\n");
            dlclose(glib_handle);
            glib_handle = NULL;
            return;
        }
        
        dlclose(glib_handle);
        glib_handle = NULL;
        
        /* Return the specific audio config on current chip */
        for(i = 0; audio_conf_map[i].chip_id; i++){
            if(audio_conf_map[i].chip_id == chipId){
                memcpy(&result->param.audio_conf, &audio_conf_map[i].audio_conf, 
                    sizeof(AUDIO_CONFIG));
                result->status = TRUE;
                return;
            }
        }
        
        result->status = FALSE;
        break;
      }
      default:
        LOG_DBG("Unknown operation %d\n", req.op);
        break;
    }
    
    return;
}
