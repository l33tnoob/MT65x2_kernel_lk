/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef _WIN32
#define LOGD(x)
#else
#include <utils/Log.h>
#undef LOG_TAG
#define LOG_TAG "OTP"
#endif

#define OTP_LOG(...) \
    do { \
        LOGD(__VA_ARGS__); \
    } while (0)

struct otp_ctl
{
    unsigned int QLength;
    unsigned int Offset;
    unsigned int Length;
    char *BufferPtr;
    unsigned int status;
};

#define OTP_MAGIC           'k'

#define OTP_GET_LENGTH 		_IOW(OTP_MAGIC, 1, int)
#define OTP_READ 	        _IOW(OTP_MAGIC, 2, int)
#define OTP_WRITE 			_IOW(OTP_MAGIC, 3, int)

int main()
{
    int fd, index;
    struct otp_ctl otpctl;

    fd = open("/dev/otp", O_RDONLY, 0);

    if(fd == -1)
    {
        OTP_LOG("Error: Cannot open the /dev/otp \n");
        return -1;
    }

    OTP_LOG("OTP: Start ioctl get length operations \n");
    ioctl(fd, (unsigned int)OTP_GET_LENGTH, (unsigned long) &otpctl);
    OTP_LOG("OTP: The Length of the NAND OTP area (%d byte) \n", otpctl.QLength);
    OTP_LOG("OTP: End ioctl get length operations \n");

    otpctl.BufferPtr = malloc(sizeof(char)*3000);

    OTP_LOG("OTP: start ioctl write operations \n");
    otpctl.Length = 3000;
    otpctl.Offset = 0;
    otpctl.status = 0;

    for(index=0; index<3000; index++)
        otpctl.BufferPtr[index]= index;

    ioctl(fd, (unsigned int)OTP_WRITE, (unsigned long) &otpctl);

    if(otpctl.status)
    {
        OTP_LOG("OTP: OTP operation error ! \n");
        goto error;
    }
    OTP_LOG("OTP: End ioctl write operations \n");
    OTP_LOG("OTP: Start ioctl read operations \n");
    otpctl.Length = 3000;
    otpctl.Offset = 0;
    otpctl.status = 0;

    for(index=0; index<3000; index++)
        otpctl.BufferPtr[index]= 0xff;

    ioctl(fd, (unsigned int)OTP_READ, (unsigned long) &otpctl);

    if(otpctl.status)
    {
        OTP_LOG("OTP: OTP operation error ! \n");
        goto error;
    }

    OTP_LOG("OTP: End ioctl read operations \n");

error:
    free(otpctl.BufferPtr);

    close(fd);
    return 0;
}
