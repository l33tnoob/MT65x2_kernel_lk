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

#define LOG_TAG "ATV_CLI_TS"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <media/ATVCtrl.h>
#include <binder/MemoryBase.h>


namespace android
{

static sp<IATVCtrlService> spATVCtrlService;

const sp<IATVCtrlService>& getMATVService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> binder;

    do
    {
        binder = sm->getService(String16("media.ATVCtrlService"));

        if (binder != 0)
        {
            break;
        }

        ALOGW("MediaPlayerService not published, waiting...");
        usleep(500000); // 0.5 s
    }
    while (true);

    spATVCtrlService = interface_cast<IATVCtrlService>(binder);

    if (spATVCtrlService == 0)
    {
        ALOGE("no ATVCtrlService!?\n");
    }

    return spATVCtrlService;
}

int MATV_CLI_GetService(void)
{
    int ret = 0;
    const sp<IATVCtrlService>& atvcs = getMATVService();

    if (atvcs != 0)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

void MATV_CLI(char c)
{
    int ret = 0;
    spATVCtrlService->CLI(c);
}

};//namespace android

using namespace android;

int main()
{
    char c;
    printf("Please enter cli command\n");
    MATV_CLI_GetService();

    while (1)
    {
        c = getchar();

        if (c == '\n')
        {
            c = 0x0d;
        }

        MATV_CLI(c);
    }

    return 0;
}


