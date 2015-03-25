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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include "camera_custom_types.h"
#include "string.h"
#ifdef WIN32
#else
#include "camera_custom_nvram.h"
#endif
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"

// BEGIN <><2014.4.19><LiteON 3H7 flash custom> Jiangde
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
//adb for log
#include <cutils/xlog.h>
// END <><2014.4.19><LiteON 3H7 flash custom> Jiangde

#include <cutils/xlog.h>
#include "flash_feature.h"
#include "flash_param.h"
#include "flash_tuning_custom.h"
#include <kd_camera_feature.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int getDefaultStrobeNVRam_sub(void* data, int* ret_size);

int getDefaultStrobeNVRam_main(void* data, int* ret_size)
{
	//static NVRAM_CAMERA_STROBE_STRUCT strobeNVRam;
	NVRAM_CAMERA_STROBE_STRUCT* p;
	p = (NVRAM_CAMERA_STROBE_STRUCT*)data;

    // BEGIN <><2014.4.19><LiteON 3H7 flash custom> Jiangde
    char    *mod_cam = "/sys/devices/platform/image_sensor/camera_info";
    char    module_name[64] = {0};
    int     fd = open(mod_cam, O_RDONLY);
    int     res = read(fd, module_name, sizeof(module_name));
    char    *pLiteON3h7 = strstr(module_name,"liteon3h7ymipiraw");

    close(fd);
    XLOGD("HJDDbg3h7Cct flash_tuning_custom_cct.cpp.cpp, module_name=%s, pLiteON3h7=%p \n",module_name, pLiteON3h7); // pLiteON3h7 DO NOT print %s !!!

    if (NULL != pLiteON3h7)
    {
        XLOGD("HJDDbg3h7Cct flash_tuning_custom_cct.cpp.cpp, It is! pLiteON3h7=%s \n", pLiteON3h7); // pLiteON3h7 is NOT NULL, so it can print %s
    	static short engTab[]=
        {
        3114,9648,141,164,188,281,375,469,563,656,750,844,938,1032,1125,1219,1313,1406,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,2,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,3,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,4,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,5,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,6,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,7,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,8,
        };

    	//version
    	p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
    	//eng tab
    	memcpy(p->engTab.yTab, engTab, 256*sizeof(short));
    	p->engTab.exp =20000;
    	p->engTab.afe_gain = 1024;
    	p->engTab.isp_gain = 1024;
    	p->engTab.distance = 300; //mm
    	//tuning para
    	p->tuningPara[0].yTar = 288;
    	p->tuningPara[0].antiIsoLevel = -5;
    	p->tuningPara[0].antiExpLevel = -5;
    	p->tuningPara[0].antiStrobeLevel = -10;
    	p->tuningPara[0].antiUnderLevel = 8;//-1  -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[0].antiOverLevel = -2; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[0].foregroundLevel = -8;
    	p->tuningPara[0].isRefAfDistance = 0;
    	p->tuningPara[0].accuracyLevel = -10;

    	p->tuningPara[1].yTar = 288;
    	p->tuningPara[1].antiIsoLevel = -5;
    	p->tuningPara[1].antiExpLevel = -5;
    	p->tuningPara[1].antiStrobeLevel = -10;
    	p->tuningPara[1].antiUnderLevel = 5;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[1].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[1].foregroundLevel = 1;
    	p->tuningPara[1].isRefAfDistance = 0;
    	p->tuningPara[1].accuracyLevel = -10;

    	p->tuningPara[2].yTar = 288;
    	p->tuningPara[2].antiIsoLevel = -5;
    	p->tuningPara[2].antiExpLevel = -5;
    	p->tuningPara[2].antiStrobeLevel = -10;
    	p->tuningPara[2].antiUnderLevel = 5;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[2].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[2].foregroundLevel = 1;
    	p->tuningPara[2].isRefAfDistance = 0;
    	p->tuningPara[2].accuracyLevel = -10;

    	p->tuningPara[3].yTar = 288;
    	p->tuningPara[3].antiIsoLevel = -5;
    	p->tuningPara[3].antiExpLevel = -5;
    	p->tuningPara[3].antiStrobeLevel = -10;
    	p->tuningPara[3].antiUnderLevel = 5;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[3].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[3].foregroundLevel = 1;
    	p->tuningPara[3].isRefAfDistance = 0;
    	p->tuningPara[3].accuracyLevel = -10;           
    }
    else
    // END <><2014.4.19><LiteON 3H7 flash custom> Jiangde        
    {
    	static short engTab[]=
        {
        4091,9542,141,164,188,281,375,469,563,656,750,844,938,1032,1125,1219,1313,1406,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,1500,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,2,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,3,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,4,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,5,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,6,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,7,
    		1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,8,
        };

    	//version
    	p->u4Version = NVRAM_CAMERA_STROBE_FILE_VERSION;
    	//eng tab
    	memcpy(p->engTab.yTab, engTab, 256*sizeof(short));
    	p->engTab.exp =20000;
    	p->engTab.afe_gain = 1024;
    	p->engTab.isp_gain = 1024;
    	p->engTab.distance = 300; //mm
    	//tuning para
    	p->tuningPara[0].yTar = 188;
    	p->tuningPara[0].antiIsoLevel = -5;
    	p->tuningPara[0].antiExpLevel = -5;
    	p->tuningPara[0].antiStrobeLevel = -10;
    	p->tuningPara[0].antiUnderLevel = -1;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[0].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[0].foregroundLevel = 1;
    	p->tuningPara[0].isRefAfDistance = 0;
    	p->tuningPara[0].accuracyLevel = -10;

    	p->tuningPara[1].yTar = 188;
    	p->tuningPara[1].antiIsoLevel = -5;
    	p->tuningPara[1].antiExpLevel = -5;
    	p->tuningPara[1].antiStrobeLevel = -10;
    	p->tuningPara[1].antiUnderLevel = -1;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[1].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[1].foregroundLevel = 1;
    	p->tuningPara[1].isRefAfDistance = 0;
    	p->tuningPara[1].accuracyLevel = -10;

    	p->tuningPara[2].yTar = 188;
    	p->tuningPara[2].antiIsoLevel = -5;
    	p->tuningPara[2].antiExpLevel = -5;
    	p->tuningPara[2].antiStrobeLevel = -10;
    	p->tuningPara[2].antiUnderLevel = -1;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[2].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[2].foregroundLevel = 1;
    	p->tuningPara[2].isRefAfDistance = 0;
    	p->tuningPara[2].accuracyLevel = -10;

    	p->tuningPara[3].yTar = 188;
    	p->tuningPara[3].antiIsoLevel = -5;
    	p->tuningPara[3].antiExpLevel = -5;
    	p->tuningPara[3].antiStrobeLevel = -10;
    	p->tuningPara[3].antiUnderLevel = -1;// -2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[3].antiOverLevel = 3; //   2//LINE <ASNTZO-60> <DATE20130325> <enhance the brightness of flash light> wupingzhou
    	p->tuningPara[3].foregroundLevel = 1;
    	p->tuningPara[3].isRefAfDistance = 0;
    	p->tuningPara[3].accuracyLevel = -10;
    }
    
    //is eng level used (or by firmware)
	p->isTorchEngUpdate =0;
	p->isNormaEnglUpdate =0;
	p->isLowBatEngUpdate =0;
	p->isBurstEngUpdate =0;
	//eng level
	memset(&p->engLevel, 0, sizeof(FLASH_ENG_LEVEL));

	*ret_size = sizeof(NVRAM_CAMERA_STROBE_STRUCT);
	return 0;
}


int getDefaultStrobeNVRam(int sensorType, void* data, int* ret_size)
{

	if(sensorType==(int)DUAL_CAMERA_SUB_SENSOR)
	{
		XLOGD("getDefaultStrobeNVRam ln=%d sensorId=%d",__LINE__, sensorType);
		return getDefaultStrobeNVRam_sub(data, ret_size);
	}
	else //DUAL_CAMERA_MAIN_SENSOR
	{
		XLOGD("getDefaultStrobeNVRam ln=%d sensorId=%d",__LINE__, sensorType);
		return getDefaultStrobeNVRam_main(data, ret_size);
	}
}
