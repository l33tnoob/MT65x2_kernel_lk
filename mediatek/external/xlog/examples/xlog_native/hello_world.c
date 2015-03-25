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

#define LOG_TAG "xlogtest/native"
#define LOG_NDEBUG 0

#include <stdio.h>
#include <limits.h>
#include <math.h>

#include <cutils/log.h>
#include <cutils/xlog.h>

int main(int argc, char *argv[])
{
  ALOGV("Verbose - Hello World\n");

  ALOGD("Debug - Hello World\n");

  ALOGI("Info - Hello World\n");

  ALOGW("Warning - Hello World\n");

  ALOGE("Error - Hello World\n");

  SLOGV("S-Verbose - Hello World\n");

  SLOGD("S-Debug - Hello World\n");

  SLOGI("S-Info - Hello World\n");

  SLOGW("S-Warning - Hello World\n");

  SLOGE("S-Error - Hello World\n");

  XLOGV("X-Verbose - Hello World\n");

  XLOGD("X-Debug - Hello World\n");

  XLOGI("X-Info - Hello World\n");

  XLOGW("X-Warning - Hello World\n");

  XLOGE("X-Error - Hello World\n");

  SXLOGV("S-X-Verbose - Hello World\n");

  SXLOGD("S-X-Debug - Hello World\n");

  SXLOGI("S-X-Info - Hello World\n");

  SXLOGW("S-X-Warning - Hello World\n");

  SXLOGE("S-X-Error - Hello World\n");

  xlog_printf(ANDROID_LOG_DEBUG, LOG_TAG, "Hello world by using xlog_printf\n");

  sxlog_printf(ANDROID_LOG_DEBUG, LOG_TAG, "Hello world by using xlog_printf\n");

  xlog_buf_printf(LOG_ID_MAIN, ANDROID_LOG_DEBUG, LOG_TAG, "Hello world by using xlog_buf_printf\n");

  return 0;
}
