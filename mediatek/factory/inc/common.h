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
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FACTORY_COMMON_H
#define FACTORY_COMMON_H

#include <stdio.h>
#include <utils/Log.h>
#include "cust.h"
#include "uistrings.h"

#ifndef bool
#define bool    int
#define false   0
#define true    1
#endif

#undef LOG_TAG
#define LOG_TAG "FTM"

enum {
    ITEM_MAIN_CAMERA,
    ITEM_MAIN2_CAMERA,
    ITEM_SUB_CAMERA,
    ITEM_STROBE,
    ITEM_GPS,
    ITEM_NFC,
    ITEM_FM,
    ITEM_FMTX,
    ITEM_FLASH,
    ITEM_MEMCARD,
    ITEM_RTC,
    ITEM_LCD,
	ITEM_LCM,
    ITEM_LED,
    ITEM_LOOPBACK,
    ITEM_LOOPBACK1,
    ITEM_LOOPBACK2,
    ITEM_LOOPBACK3,
    ITEM_BT,
    ITEM_WIFI,
    ITEM_KEYS,
    ITEM_LOOPBACK_PHONEMICSPK,
    ITEM_WAVEPLAYBACK,
    ITEM_ACOUSTICLOOPBACK,
    ITEM_GSENSOR,
    ITEM_GS_CALI,
    ITEM_MSENSOR,
    ITEM_ALSPS,
    ITEM_HEADSET,
    ITEM_HEADSET_DEBUG,
    ITEM_USB,
    ITEM_OTG,
    ITEM_CLRFLASH,
    ITEM_CHARGER,
    ITEM_TOUCH,
    ITEM_TOUCH_AUTO,
    ITEM_SIM,
    ITEM_VIBRATOR,
    ITEM_RECEIVER,
    ITEM_RECEIVER_DEBUG,
    ITEM_SIMCARD,
    ITEM_IDLE,
    ITEM_TVOUT,
    ITEM_JOGBALL,
    ITEM_OFN,
    ITEM_MATV_NORMAL,
    ITEM_MATV_AUTOSCAN,
    ITEM_MUI_TEST,
    ITEM_FULL_TEST,
    ITEM_ITEM_TEST,
    ITEM_AUTO_TEST,
    ITEM_DEBUG_TEST,
    ITEM_VERSION,
    ITEM_REPORT,
    ITEM_UPDATE,
    ITEM_REBOOT,
    ITEM_BAROMETER,
    ITEM_GYROSCOPE,
    ITEM_GYROSCOPE_CALI,
    ITEM_SPK_OC,
    ITEM_SIGNALTEST,
    ITEM_CMMB,
    ITEM_EMMC,
    ITEM_EMI,
    ITEM_CLREMMC,
    ITEM_HDMI,
    ITEM_RF_TEST,
    ITEM_RECEIVER_PHONE,
    ITEM_HEADSET_PHONE,
    ITEM_LOOPBACK_PHONEMICSPK_PHONE,
    ITEM_VIBRATOR_PHONE,
    ITEM_MICBIAS,
    ITEM_HOTKNOT,
    ITEM_MAX_IDS
};

#if 0
#define LOGE(...) ui_print("E:" __VA_ARGS__)
#define LOGW(...) fprintf(stderr, "W:" __VA_ARGS__)
#define LOGI(...) fprintf(stderr, "I:" __VA_ARGS__)

#if 0
#define LOGV(...) fprintf(stderr, "V:" __VA_ARGS__)
#define LOGD(...) fprintf(stderr, "D:" __VA_ARGS__)
#else
#define LOGV(...) do {} while (0)
#define LOGD(...) do {} while (0)
#endif
#endif

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

#endif  // FACTORY_COMMON_H
