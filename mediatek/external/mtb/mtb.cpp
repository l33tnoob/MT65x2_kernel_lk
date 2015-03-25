/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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



#ifdef MTB_SUPPORT
#define LOG_TAG "MediaTekTraceBridge"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <utils/misc.h>
#include <utils/Timers.h>
#include <cutils/xlog.h>
#include <cutils/trace.h>
#include <cutils/properties.h>
#include "constants.h"
#include "mtb_tracer_factory.h"
#include "mtb.h"


#define MTB_INTERNAL_DEBUG 0

static uint32_t mtb_trace_type = 0;
static uint64_t mtb_atrace_tags = 0;
static pthread_mutex_t mtb_mutex = PTHREAD_MUTEX_INITIALIZER;

#if MTB_INTERNAL_DEBUG
inline long long unsigned getNowus() {
    return systemTime(SYSTEM_TIME_MONOTONIC) / 1000ll;
}

class TimeScale{
public:
inline TimeScale(const char* name)
{
    mTime = ::getNowus();
    mName = name;
}

inline ~TimeScale() {
    mTime = ::getNowus() - mTime;
    SXLOGD("Run function %s cost %lld us", mName, mTime);
}

private:
    long long mTime;
    const char* mName;
};

#define TIME_SCALE() TimeScale __scale(__FUNCTION__)
#endif


int mtb_trace_begin(uint64_t tag, const char *name, uint32_t pid)
{
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld", __FUNCTION__, tag);
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_BEGIN, name, pid, NULL);

    return 0;
}

int mtb_trace_end(uint64_t tag, const char* name, uint32_t pid)
{ 
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld", __FUNCTION__, tag);
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_END, name, pid, NULL);
    return 0;
}

int mtb_trace_oneshot(uint64_t tag, uint32_t type, const char *name, uint32_t pid)
{
#if MTB_INTERNAL_DEBUG
    TIME_SCALE();
#endif
    SXLOGV("Enter function %s, tag = %lld, type = %s, name = %s", __FUNCTION__, tag, type, name);
    
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_ONESHOT, name, pid, type);
    
    return 0;
}

int mtb_trace_init(uint32_t type, uint64_t tag)
{
    SXLOGD("Enter function %s, type = %ld, tag = %lld", __FUNCTION__, type, tag);
    mtb_trace_type = type & MTB_TRACE_TYPE_VALID_MASK;
    mtb_atrace_tags = tag;
    
    tracer_load();
    tracer_interface(mtb_trace_type, tag, TRACER_INTERFACE_INIT, NULL, 0,NULL);
	return 0;
}

/**
 * debug.mtb.tags.tracetype. Can be used as a sysprop change callback.
 */
void mtb_trace_update_types() {
    char mtb_tracetype[PROPERTY_VALUE_MAX];
    char atrace_tags[PROPERTY_VALUE_MAX];
    uint32_t type;
    uint64_t atags;
    
    pthread_mutex_lock(&mtb_mutex);
    property_get("debug.mtb.tracetype", mtb_tracetype, "0");
    property_get("debug.atrace.tags.enableflags", atrace_tags, "0");
    type = (uint32_t)(strtoll(mtb_tracetype, NULL, 0) & MTB_TRACE_TYPE_VALID_MASK);
    atags = strtoll(atrace_tags, NULL, 0);
   
    if ((!type || !atags) || ((mtb_trace_type == type) && (mtb_atrace_tags == atags))) {
        pthread_mutex_unlock(&mtb_mutex);
        return;
    }

    mtb_trace_init(type, atags);
    atrace_update_tags();
    pthread_mutex_unlock(&mtb_mutex);
}

uint32_t mtb_trace_get_types() {
    return mtb_trace_type;
}

static void mtb_atrace_init() __attribute__((constructor));

static void mtb_atrace_init()
{
    ::android::add_sysprop_change_callback(mtb_trace_update_types, 0);
}

#endif
