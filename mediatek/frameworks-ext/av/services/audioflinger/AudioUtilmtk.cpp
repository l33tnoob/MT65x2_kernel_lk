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

#define LOG_TAG "AudioUtilmtk"
#include"AudioUtilmtk.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <cutils/xlog.h>
#include <cutils/properties.h>



namespace android {

//class  AudioDump
void AudioDump::dump(const char * filepath, void * buffer, int count,const char * property)
{
	char value[PROPERTY_VALUE_MAX];
	int ret;
	property_get(property, value, "0");
	int bflag=atoi(value);
	if(bflag)
	{
	   ret = checkPath(filepath);
	   if(ret<0)
	   {
		   XLOGE("dump fail!!!");
	   }
	   else
	   {
		 FILE * fp= fopen(filepath, "ab+");
		 if(fp!=NULL)
		 {
			 fwrite(buffer,1,count,fp);
			 fclose(fp);
		 }
		 else
		 {
			 XLOGE("dump %s fail",property);
		 }
	   }
	}
}

int AudioDump::checkPath(const char * path)
{
	char tmp[PATH_MAX];
	int i = 0;

	while(*path)
	{
		tmp[i] = *path;

		if(*path == '/' && i)
		{
			tmp[i] = '\0';
			if(access(tmp, F_OK) != 0)
			{
				if(mkdir(tmp, 0770) == -1)
				{
					XLOGE("mkdir error! %s",(char*)strerror(errno));
					return -1;
				}
			}
			tmp[i] = '/';
		}
		i++;
		path++;
	}
	return 0;
}

// class hw

bool HwFSync::mUnderflow =false;

HwFSync::HwFSync()
    :mFd(-1)
{
}

HwFSync::~HwFSync()
{
    if(mFd != -1)
    {
        ::close(mFd);
        mFd = -1;
    }
}

void HwFSync::setFsync()
{
    if(mFd == -1){
        mFd = ::open("/dev/eac", O_RDWR);
        XLOGW("mfd =%d",mFd);
       if(mFd < 0)
       {
           XLOGE("setFsync fail to open eac");
       }
    }
    if(mFd >= 0 )
    {
        XLOGD("callback hw setFSync");
        memset(&action, 0, sizeof(action));
        action.sa_handler = callback;
        action.sa_flags = 0;
        sigaction(SIGIO, &action, NULL); //set up async handler
        fcntl(mFd, F_SETOWN, gettid()); //enable async notification
        fcntl(mFd, F_SETFL, fcntl(mFd, F_GETFL) | FASYNC|FD_CLOEXEC);
    }
}

//do  not use mutex to protect this value, use atomic if needed.
bool HwFSync::underflow()
{
   return mUnderflow;
}

 void HwFSync::callback(int signal)
{
    XLOGD("callback");
    if (signal==SIGIO)
    {
        mUnderflow = true;
        XLOGD("callback hw is under flow");
    }
}

void HwFSync::reset()
{
   mUnderflow = false;
}

void setCPU_MIN_Freq(const char *pfreq)
{
    FILE *fp= fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", "w");
    if(fp!=NULL)
    {
        fputs(pfreq,fp);
        fclose(fp);
    }
    else
    {
        XLOGE("Can't open /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
    }
}

}












