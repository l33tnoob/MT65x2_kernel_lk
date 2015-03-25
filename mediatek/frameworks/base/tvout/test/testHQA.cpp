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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <utils/misc.h>
#include <utils/Log.h>
#include <cutils/xlog.h>


#include "tvout_patterns.h"

#if defined(MTK_TVOUT_SUPPORT)
#include <linux/tv_out.h>
#endif


#define LOG_TAG "TV/TEST"


static unsigned char* patBufRGB = NULL;
static unsigned char* patBufYUV = NULL;


static int tvoutIoctl(int code, unsigned int value = 0)
{
	int fd = open("/dev/TV-out", O_RDWR, 0);
	int ret;
    if (fd >= 0) {
        ret = ioctl(fd, code, value);
        if (ret == -1) {
            XLOGE("[TVOut] [%s] failed. ioctlCode: %d, errno: %d",
                 __func__, code, errno);
            return 0;
        }
        close(fd);
    } else {
        XLOGE("[TVOut] [%s] open mtkfb failed. errno: %d", __func__, errno);
        return 0;
    }
    return ret;
}


static int showPattern(int id)
{
     unsigned int ret;

#if defined (MTK_TVOUT_SUPPORT)
     unsigned int    patBufSize;
     TVOutPatInfo    patInfo;
     TVOUT_HQA_BUF   tvBuf;
     unsigned char**  patBufAddr;

     TVOutPatterns* pattern = new(TVOutPatterns);
     if (pattern == NULL) {
         XLOGE("[TVOut] pattern can not be used");
         return false;
     }

     if (pattern->checkID(id) == false) {
         XLOGE("[TVOut] pattern id error %d", id);
         delete pattern;
         return false;
     }

     pattern->getPatternInfo(id, &patInfo);
     if (patInfo.fmt == TVOUT_FMT_RGB565) {
         patBufSize = TVOUT_PATTERN_RGB_SIZE;
         patBufAddr = &patBufRGB;
     } else if (patInfo.fmt == TVOUT_FMT_YUV420_PLANAR) {
         patBufSize = TVOUT_PATTERN_YUV_SIZE;

         patBufAddr = &patBufYUV;
     } else {
         XLOGE("[TVOut] unsupport pattern");
         delete pattern;
         return false;
     }

     if (*patBufAddr == NULL) {
         XLOGI("allocate pat buffer 0x%x", patBufSize);
         *patBufAddr = (unsigned char*)malloc(patBufSize);
         if (*patBufAddr == NULL) {
             XLOGE("[TVOut] JNI buffer can not be allocated");
             delete pattern;
             return false;
         }
     }

     XLOGD("%s() id=%d, patBuf=0x%x, size=0x%x\n", __func__, id, *patBufAddr, patBufSize);

     pattern->getUnzippedPattern(id, *patBufAddr, patBufSize);
     delete pattern;

     tvBuf.phy_addr = NULL;
     tvBuf.vir_addr = (void*)(*patBufAddr);
     tvBuf.format = patInfo.fmt;
     tvBuf.width = patInfo.w;
     tvBuf.height = patInfo.h;

     ret = tvoutIoctl(TVOUT_CTL_POST_HQA_BUFFER , (unsigned int)&tvBuf);
     if (ret < 0)
         return false;

#endif

     return true;


 }

static int leavePattern(void)
{
     unsigned int ret;

#if defined (MTK_TVOUT_SUPPORT)
     ret = tvoutIoctl(TVOUT_CTL_LEAVE_HQA_MODE);

     if (patBufRGB != NULL) {
         XLOGI("free RGB pat buffer");
         free(patBufRGB);
         patBufRGB = NULL;
     }

     if (patBufYUV != NULL) {
         XLOGI("free YUV pat buffer");
         free(patBufYUV);
         patBufYUV = NULL;
     }

     if (ret < 0)
         return false;

#endif
     return true;

 }


 int main(int argc, char* argv[])
 {

	int cmd;

    char s;
    char temp;
	while(s != 'q')
	{

		scanf("%c", &s);
        scanf("%c", &temp);
        XLOGD("get cmd");
        cmd = atoi(&s);
        if ( cmd >= 0 && cmd <= 6)
        {
            XLOGD("testHQA show pattern %d", cmd);
            showPattern(cmd);
            XLOGD("show pattern done");
        }
	}

    leavePattern();
 }

