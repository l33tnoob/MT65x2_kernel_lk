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

#include <sys/types.h>
#include <sys/stat.h>
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/stat.h>
#include <sys/ioctl.h>

#define LOG_TAG "OTG_TEST"
#include "usb_jni.h"

#define OTG_TEST_DEV	"/dev/mt_otg_test"
int fd = -1;

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeInit
  (JNIEnv * env, jobject obj){

    fd = open(OTG_TEST_DEV, O_RDWR);
    if(fd<0){
        OTG_LOG("otg_test_init::create and open file fail!\n");
        return JNI_FALSE;
     } else {
        OTG_LOG("otg_test_init::create and open file OK!\n");
		unsigned int cmd = HOST_CMD_ENV_INIT;
		int ret = -1;
		int temp = 0;
		ret = ioctl(fd, cmd, &temp);
//		if(ret < 0)
//		{
//			OTG_LOG("otg_test_cmd_start::fail and cmd=%d\n",cmd);
//			return JNI_FALSE;
//		}

     }
    return JNI_TRUE;    
    
}

JNIEXPORT void JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeDeInit
  (JNIEnv * env, jobject obj){

	if(fd != -1){
		unsigned int cmd = HOST_CMD_ENV_EXIT;
		int ret = -1;
		int temp = 0;
		ret = ioctl(fd, cmd, &temp);
   		close(fd);
	}
    return;
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeStartTest
  (JNIEnv * env, jobject obj, jint test_nr){
		if(fd == -1)
		{
			OTG_LOG("FD == -1\n");
			return JNI_FALSE;
		}
    unsigned int cmd = 0;
    int ret = -1;
    int temp = 0;
    OTG_LOG("otg_test_cmd_start::test_nr=%d\n",test_nr);
    switch(test_nr){
        case ENABLE_VBUS:
            cmd = OTG_CMD_E_ENABLE_VBUS;
            break;
        case ENABLE_SRP:
            cmd = OTG_CMD_E_ENABLE_SRP;
            break;
        case DETECT_SRP:
            cmd = OTG_CMD_E_START_DET_SRP;
            break;
        case DETECT_VBUS:
            cmd = OTG_CMD_E_START_DET_VBUS;
            break;
        case TD_5_9:
        	cmd = OTG_CMD_P_B_UUT_TD59;
        	break;
        case A_UUT:
            cmd = OTG_CMD_P_A_UUT;
            break;
        case B_UUT:
            cmd = OTG_CMD_P_B_UUT;
            break;
        case TEST_SE0_NAK:
            cmd = HOST_CMD_TEST_SE0_NAK;
            break;
        case TEST_J:
            cmd = HOST_CMD_TEST_J;
            break;
        case TEST_K:
            cmd = HOST_CMD_TEST_K;
            break;
        case TEST_PACKET:
            cmd = HOST_CMD_TEST_PACKET;
            break;
        case SUSPEND_RESUME:
            cmd = HOST_CMD_SUSPEND_RESUME;
            break;
        case GET_DESCRIPTOR:
            cmd = HOST_CMD_GET_DESCRIPTOR;
            break;   
        case SET_FEATURE:
            cmd = HOST_CMD_SET_FEATURE;
            break;  
        }
    ret = ioctl(fd, cmd, &temp);
    if(ret < 0)
    {
    	OTG_LOG("otg_test_cmd_start::fail and cmd=%d\n",cmd);
    	return JNI_FALSE;
    }
    else
    {
    	OTG_LOG("otg_test_cmd_start::OK and cmd=%d\n",cmd);
    	return JNI_TRUE;
    }
    
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeStopTest
  (JNIEnv * env, jobject obj, jint test_nr){
	if(fd == -1)
		{
			OTG_LOG("FD == -1\n");
			return JNI_FALSE;
		}
    unsigned int cmd;
     unsigned int stop_cmd;
     int ret = -1;
    OTG_LOG("otg_test_cmd_stop::test_nr=%d\n",test_nr);
    
    stop_cmd = OTG_STOP_CMD;
    if(0 == write(fd,&stop_cmd,sizeof(unsigned int)))
    {
        OTG_LOG("otg_test_cmd_stop::stop cmd OK\n");
        ret = 0;
    }
    else
    {
        OTG_LOG("otg_test_cmd_stop::stop cmd fail\n");
    }
    if(ret < 0)
    {
    	return JNI_FALSE;
    }
    else
    {
    	return JNI_TRUE;
    }
}

JNIEXPORT jint JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeGetMsg
  (JNIEnv * env, jobject obj){
	if(fd == -1)
		{
			OTG_LOG("FD == -1\n");
			return 0;
		}
    int msg;
    if(0 == read(fd,(unsigned int*)&msg,sizeof(int)))
    	{
        OTG_LOG("otg_test_msg_get::get msg OK,0x%x\n",(char*)msg);
      }
    else
    	{
        OTG_LOG("otg_test_msg_get::get msg fail\n");
        msg = 0;
      }
    return (char*)msg;
    
}

JNIEXPORT jboolean JNICALL Java_com_mediatek_engineermode_usb_UsbDriver_nativeCleanMsg
  (JNIEnv * env, jobject obj){
    int ret;
    OTG_LOG("UsbDriver_nativeCleanMsg");
	if(fd == -1)
    {
			OTG_LOG("FD == -1\n");
			return JNI_FALSE;
    }
    unsigned int init_msg = OTG_INIT_MSG;
    ret = write(fd, &init_msg, sizeof(int));
    if(ret < 0)
    {
        OTG_LOG("UsbDriver_nativeCleanMsg fail\n");
        return JNI_FALSE;
    }
    else
    {

        OTG_LOG("UsbDriver_nativeCleanMsg OK\n");
        return JNI_TRUE;
    }
    
}

