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
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <dirent.h>
#include <linux/kdev_t.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "UsbSelect.h"


#define LOG_TAG "dongled"


UsbSelect *UsbSelect::sInstance = NULL;



UsbSelect *UsbSelect::Instance() 
{
    if (!sInstance) {
        sInstance = new UsbSelect();
    }
    return sInstance;
}

UsbSelect::UsbSelect()
{
	  //lastVID = {0,0,0,0,0};
  
}

/*
Description: search the value of key in the buf
input:
      
output:
*/
bool UsbSelect::searchParam(const char *buf, const char *key, char *value, int len)
{
    bool ret=false;
    char *p, *str;
    int  i;

    memset (value, 0, len);
    p = strstr (buf, key);
    
    if (!p) {
        
        SLOGE(" not found the value of %s", key);
        return false;
    }    

    str = strstr(p, "=");
    if (!str) {
        
        SLOGE(" not found the value of %s", key);
        return false;
    }

    i = 0;
    str++;
    while ((*str != 0) && (*str != 0x0d) && (*str != 0x0a) && (*str != ',') && (i < len)) {
        
        value[i] = *str;
        i++;
        str++;
    }

    return true;
 
}

/*
Description: search the  device in dongleselect.conf  
input: 
       
output:true(the input device is found in devicesTable)
       false(not found)
*/
bool UsbSelect::findDevice(char *vid, char *pid, struct usbdevice *tmpdevice)
{
    FILE *file;
    char buf[256];
    char vendorid[5], productid[5], libpath[30];
    bool found = false, ret = false;

    if (NULL == vid || NULL == pid || NULL == tmpdevice) {
        SLOGE(" param is wrong");
    }
    SLOGE("idvendor:%s, idproduct:%s\n", vid, pid);
    
    file = fopen ("/system/etc/dongleselect.conf", "r");
    
    if (file) {
        while (fgets(buf, 256, file)) {
            memset (vendorid, 0, sizeof(vendorid));
            ret = searchParam(buf, "VID", vendorid, sizeof(vendorid));
            if (!ret) {
                SLOGE("It's not VID");
                continue;
            }
            
            if (strtoul(vid, NULL, 16) != strtoul(vendorid, NULL, 16)) {
                continue;
            }

            memset (productid, 0, sizeof(productid));
            ret = searchParam(buf, "PID", productid, sizeof(productid));
            if (!ret) {
                SLOGE("It's not PID");
                continue;
            }

            if (strtoul(pid, NULL, 16) != strtoul(productid, NULL, 16) && strcmp(productid, "0")) {
                continue;
            }
            
            memset (libpath, 0, sizeof(libpath));
            ret = searchParam(buf, "LIBPATH", libpath, sizeof(libpath));
            if (!ret) {
                SLOGE("It's not LIBPATH");
                continue;
            }

            found = true;
            memset(tmpdevice->VID, 0, sizeof(tmpdevice->VID));
            memset(tmpdevice->PID, 0, sizeof(tmpdevice->PID));
            memset(tmpdevice->libpath, 0, sizeof(tmpdevice->libpath));
            strcpy(tmpdevice->VID, vendorid);
            strcpy(tmpdevice->PID, productid);
            strcpy(tmpdevice->libpath, libpath);
            
            break;
        }
    }

    return found;
}


/*
Description: handle the NetlinkEvent of usb
input: 
        evt: NetlinkEvent
output:
*/
int UsbSelect::handleUsbAutoSelect(NetlinkEvent *evt)
{
    const char *pStr;
    const char *buf;
    char vid[5];
    char pid[5];
    bool found = false;
    struct usbdevice device;

    memset(vid, 0, 5);
    memset(pid, 0, 5);
   
    if (NULL == evt) {
        SLOGE("NetlinkEvent is null");
        return -1;
    }
    
    buf = evt->findParam("PRODUCT");
    
    if (NULL == buf) {
        SLOGE("NetlinkEvent has not the Param of PRODUCT ");
        return -1;
    }
    SLOGE("The PRODUCT is: %s ", buf);
    
    memcpy(vid, buf, 4);
    vid[5] = '\0';
    pStr = buf + 5;

    if (NULL == pStr) {
        SLOGE("NetlinkEvent has not the Param of pid ");
        return -1;
    }

    memcpy(pid, pStr, 4);
    pid[5] = '\0';

    if (strtoul(lastDevice.VID, NULL, 16) == strtoul(vid, NULL, 16) &&
        strtoul(lastDevice.PID, NULL, 16) == strtoul(pid, NULL, 16)) {
        
        SLOGE("NetlinkEvent is the same of last one");
       
    } else {
    
        found = findDevice(vid, pid, &device);
        
        if (found) {
            if (0 != strcmp(device.libpath, lastDevice.libpath)) {
                
                strcpy(lastDevice.VID, vid);
                strcpy(lastDevice.PID, pid);
                memset(lastDevice.libpath, 0, 30);
                strcpy(lastDevice.libpath, device.libpath);
                
                SLOGE("new 3Gdongle insert, the libpath is: %s", device.libpath);
                
                property_set("rild.libpath", device.libpath);
                
                // not switch user for huawei dongle in rild.
                if(strtoul("12D1", NULL, 16) == strtoul(vid, NULL, 16))
                {
                    property_set("rild.mark_switchuser", "1");
                }
                		
                SLOGE("restart Rild");
                //killRild();
                property_set("ctl.restart", "ril-daemon");//restart rild 

                return 0;
                
            } else {
            
                SLOGE("The 3Gdongle is the same factory of last one");
                return 1;
                
            }
         } else {  
            SLOGE("The 3Gdongle(vid = %s,pid = %s) is not found", vid, pid);
            return 2;
            
         }

    
    }
    
    return 3;

}

/*
Description: poll the usb devices 
input: 
output:
*/

/*
Description: kill the rild process
input: 
output:
*/
void UsbSelect::killRild()
{

    char szBuf[256] = {0}; 
    char cmd[64] = {0};
    char *p_pid = NULL;
    FILE *pFile = NULL;
    int  pid = 0;
    int count = 5;

    sprintf(cmd, "ps | grep %s", "/system/bin/rild");
    pFile = popen(cmd, "r");

    if (pFile != NULL){
        while(fgets(szBuf, sizeof(szBuf), pFile)) {   //find the pid of rild 
            if(strstr(szBuf, "/system/bin/rild")) {
                p_pid = strstr(szBuf, " "); 
                pid = strtoul(p_pid, NULL, 10);
                SLOGE("--- rild PID = %d ---",pid);
                break;
            }
        }  
    }

    pclose(pFile); 

    if(pid){
        kill(pid, SIGKILL);  
        while(--count >= 0) { //Wait rild exit
            kill(pid, SIGHUP);
            if(errno == ESRCH) {
            	break;
            }
            sleep(1);
        }
    }

}







