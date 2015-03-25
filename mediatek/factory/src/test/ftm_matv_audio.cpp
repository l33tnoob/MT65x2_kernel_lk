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

/*****************************************************************************
*                     C O M P I L E R   F L A G S
******************************************************************************
*/

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_MATV

/*****************************************************************************
*                E X T E R N A L   R E F E R E N C E S
******************************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include "kal_release.h"
#include "ftm_matv_audio.h"
///#include "AudioI2S.h"

#define TAG   "[MATV_I2S] "
/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/

/*****************************************************************************
*                        F U N C T I O N   D E F I N I T I O N
******************************************************************************
*/
void* I2SGetInstance()
{
    LOGD("I2SGetInstance");
    return (void*)android::AudioI2S::getInstance();
}

void I2SFreeInstance(void *handle)
{
    LOGD("I2SFreeInstance");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    i2sHandle->freeInstance();
}

unsigned int I2SOpen(void* handle)
{
    LOGD("I2SOpen");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->open();
}

bool I2SSet(void* handle, int type)
{
    LOGD("I2SSet");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->set(type);
}

bool I2SClose(void* handle, unsigned int Identity)
{
    LOGD("I2SClose");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->close(Identity);
}

bool I2SStart(void* handle, unsigned int Identity)
{
    LOGD("I2SStart");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->start(Identity,android::MATV);
}

bool I2SStop(void* handle, unsigned int Identity)
{
    LOGD("I2SStop");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->stop(Identity,android::MATV);
}

unsigned int I2SGetReadBufferSize(void* handle)
{
    LOGD("I2SGetReadBufferSize");
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->GetReadBufferSize();
}

unsigned int I2SRead(void* handle, unsigned int Identity,void* buffer, unsigned int buffersize)
{
    android::AudioI2S* i2sHandle = (android::AudioI2S*)handle;
    return i2sHandle->read(Identity, buffer, buffersize);
}

#ifdef __cplusplus
};
#endif

#endif
