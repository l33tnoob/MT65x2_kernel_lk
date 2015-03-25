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

#include "multimediaFactoryTest.h"
#include <cutils/log.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dlfcn.h>

extern     int DpFactoryModeTest(void * a_pArg);

#define NO_ERROR    0
#define EFAIL       -1

pthread_t DDPEMITestThread;
static void *MT6589_DDPEMITestThread(void * a_pArg)
{
   
    unsigned long *result = (unsigned long *)a_pArg;
 

    if( DpFactoryModeTest(a_pArg) == 0 )
    {
        *result = NO_ERROR;
    } else
    {
        *result = EFAIL;
    }

    return NULL;
    
}


pthread_t MT6589MFVEMITestThread;
static void *MT6589_MFVEMITestThread(void * a_pArg)
{
    unsigned long *u4ThreadMDPResult = (unsigned long *)a_pArg;
    struct timeval t1 , t2;
    unsigned long long u4Time1, u4Time2;
    unsigned long *result = (unsigned long *)a_pArg;
	
    const char* const lib_path = "/system/lib/libmfvfactory.so";
    void *dso = NULL;
    int (*mainfactory)();

    ALOGD("MT6589MFVEMITestThread");

    if (access(lib_path, R_OK))
    {
        ALOGE("access (%s) fail", lib_path);
        return 0;
    }

    dso = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);
    if (dso == 0)
    {
        const char* err = dlerror();
        ALOGE("load (%s) fail: %s", lib_path, err ? err:"unknown");
        return 0;
    }

    ALOGD("loaded %s", lib_path);

    mainfactory = (int (*)())dlsym(dso, "main_factory");
    ALOGD("loaded mainfactory:%x", (unsigned int)mainfactory);

    gettimeofday(&t1,NULL);
    u4Time1 = (t1.tv_sec*1000000 + t1.tv_usec);
	
    if (!mainfactory)
    {
        ALOGE("can't find main_factory() in %s", lib_path);
        return 0;
    }

	if((*mainfactory)() <= 0)
	{
	        *u4ThreadMDPResult = EFAIL;	
            ALOGE("MFV test failed!!");	
	}
	else
	{
        	*u4ThreadMDPResult = NO_ERROR;
            ALOGD("MFV test pass!!");			
	}

	gettimeofday(&t2,NULL);
	u4Time2 = (t2.tv_sec*1000000 + t2.tv_usec);
    u4Time1 = u4Time2 - u4Time1;
    ALOGD("vcodec time=%lld\n", u4Time1);
	
    ALOGD("MT6589MFVEMITestThread End");
	
    return NULL;
}


#define MT6589MDPEMITEST_TIMEOUT 5000000//5 seconds
//#define MT6575MDPEMITEST_TIMEOUT 600000000//600 seconds

#define MHAL_UNKNOWN_ERROR 0x80000000;

pthread_t MT6589MFGEMITestThread;
static void *MT6589_MFGEMITestThread(void * a_pArg)
{
    const char* const lib_path = "/system/vendor/lib/libBLPP.so";
    void *dso = NULL;
    int (*blppstart)(int);
    int idx = 0;

    struct timeval t1 , t2;
    unsigned long u4Time1, u4Time2;

    *(unsigned long *)a_pArg = NO_ERROR;

    if (access(lib_path, R_OK))
    {
        ALOGE("access (%s) fail", lib_path);
        return 0;
    }

    dso = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);
    if (dso == 0)
    {
        const char* err = dlerror();
        ALOGE("load (%s) fail: %s", lib_path, err ? err:"unknown");
        return 0;
    }

    ALOGD("loaded %s", lib_path);

    blppstart = (int (*)(int))dlsym(dso, "BLPPStart");
    ALOGD("loaded blppstart:%x", (unsigned int)blppstart);
    if (!blppstart)
    {
        ALOGE("can't find BLPPStart() in %s", lib_path);
        return 0;
    }

    gettimeofday(&t1,NULL);
    u4Time1 = (t1.tv_sec*1000000 + t1.tv_usec);

    while (1)
    {
        if((*blppstart)(idx) < 0)
        {
            *(unsigned long *)a_pArg = MHAL_UNKNOWN_ERROR;
            ALOGE("MFG test failed!!");
        }
        ALOGD("MFG test ok");
        gettimeofday(&t2,NULL);
        u4Time2 = (t2.tv_sec*1000000 + t2.tv_usec);
        if(u4Time2 > u4Time1 + MT6589MDPEMITEST_TIMEOUT)
        {
            ALOGD("MFG test success, time up");
            break;
        }

        if (++idx > 4) idx = 0;
    }
    ALOGD("%s test end", lib_path);
    dlclose(dso);

    return NULL;
}


int mHalFactory(void *a_pInBuffer)
{
    unsigned long u4MDPResult = NO_ERROR;
    unsigned long u4MFVResult = NO_ERROR;
    unsigned long u4MFGResult = NO_ERROR;


    pthread_create(&DDPEMITestThread, NULL , MT6589_DDPEMITestThread , &u4MDPResult);
   
    pthread_create(&MT6589MFVEMITestThread, NULL , MT6589_MFVEMITestThread , &u4MFVResult);

    pthread_create(&MT6589MFGEMITestThread, NULL , MT6589_MFGEMITestThread , &u4MFGResult);

    pthread_join(DDPEMITestThread , NULL);
    
    pthread_join(MT6589MFVEMITestThread , NULL);

    pthread_join(MT6589MFGEMITestThread , NULL);
/*
if((NO_ERROR == u4MDPResult) && (NO_ERROR == u4MFVResult))
{
   PLOG("Test success!!! good");
}
*/
    return ((NO_ERROR == u4MDPResult) && (NO_ERROR == u4MFVResult) && (NO_ERROR == u4MFGResult)? NO_ERROR : EFAIL);
}
/*
int main()  
{
    MHAL_ERROR_ENUM ret;
	ret = (MHAL_ERROR_ENUM)mHalFactory(NULL);
	if (ret == NO_ERROR)
		printf("Test success!\n");
	else 
		printf("Test fail!\n");
	return 0;
}
*/
