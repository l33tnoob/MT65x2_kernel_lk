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



#ifdef MTK_MMPROFILE_SUPPORT
#define LOG_TAG "MediaTekTraceBridge"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cutils/xlog.h>
#include <linux/mmprofile.h>
#include "constants.h"
#include "mtb_tracer.h"
#include "mmp_wfd.h"
#include "mmp_hash.h"

#define THREAD_MAX 10

MMP_Event MMP_WFD_ROOT;
MMP_Event MMP_WFD_MESSAGE;
MMP_Event MMP_WFD_ADATA;
MMP_Event MMP_WFD_VDATA;
MMP_Event MMP_WFD_SPECIAL;

static struct mmp_event_info mmp_wfd_hashTable[MMP_EVENT_HASHTABLE_LENGTH];

struct event_list {
    int32_t pid;
    char name[16];
    struct event_list *next;
};

static struct event_list* mmp_wfd_events = NULL;
static pthread_mutex_t mmp_wfd_mutex;

static struct event_list* event_create(uint32_t number);
static struct event_list* event_search(int32_t pid);
static void event_add(int32_t pid, char* name);
static void event_remove(int32_t pid);

struct event_list* event_create(uint32_t number) {
    struct event_list* head = NULL;
    struct event_list* p = NULL;
    int i;

    for (i=0; i<number; i++) {
        struct event_list* node = (struct event_list*)malloc(sizeof(struct event_list));
        node->pid = -1;
        node->next = NULL;

        if (!head) {
            head = node;
            p = node;
        }
        else {
            p->next = node;
            p = node;
        }
        SXLOGD("event_create, p = %p",p);
    }

    pthread_mutex_init(&mmp_wfd_mutex, NULL);
    return head;
}

struct event_list* event_search(int32_t pid) {
    struct event_list* p = NULL;
    
    if (!mmp_wfd_events) return p;
    
    pthread_mutex_lock(&mmp_wfd_mutex); 
    p = mmp_wfd_events;
    while(p != NULL) {
        if (p->pid == pid) {
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&mmp_wfd_mutex);
    return p;
}

void event_add(int32_t pid, char* name) {
    struct event_list* p = NULL;

    if (!mmp_wfd_events) return;
    
    pthread_mutex_lock(&mmp_wfd_mutex);
    p = mmp_wfd_events;

    while(p != NULL) {
        if (p->pid == -1) {
            p->pid = pid;
            strcpy(p->name, name);
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&mmp_wfd_mutex);
}

void event_remove(int32_t pid) {
    struct event_list* p = NULL;

    if (!mmp_wfd_events) return;
    
    pthread_mutex_lock(&mmp_wfd_mutex);
    p = mmp_wfd_events;

    while(p != NULL) {
        if (p->pid == pid) {
            p->pid = -1;
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&mmp_wfd_mutex);
}

static inline void generate_event_name(char* str, char* name, uint32_t length)
{
    char *p = strchr(str, ',');
    size_t size = p ? (p - str) : strlen(str);
    size = (size > length) ? length : size;
    
    strncpy(name, str, size);
    name[size] = '\0';
}

int32_t  mmp_wfd_init(tracer_handle_t self)
{
    SXLOGD("Enter function %s", __FUNCTION__);
    MMP_WFD_ROOT = MMProfileRegisterEvent(MMP_RootEvent, "WFD");
    MMProfileEnableEvent(MMP_WFD_ROOT, 1); 

    MMP_WFD_MESSAGE = MMProfileRegisterEvent(MMP_WFD_ROOT, "message");
    MMProfileEnableEvent(MMP_WFD_MESSAGE,1); 

    MMP_WFD_ADATA = MMProfileRegisterEvent(MMP_WFD_ROOT, "audiodata");
    MMProfileEnableEvent(MMP_WFD_ADATA, 1); 
    
    MMP_WFD_VDATA = MMProfileRegisterEvent(MMP_WFD_ROOT, "videodata");
    MMProfileEnableEvent(MMP_WFD_VDATA, 1); 
    
    MMP_WFD_SPECIAL = MMProfileRegisterEvent(MMP_WFD_ROOT, "special");
    MMProfileEnableEvent(MMP_WFD_SPECIAL, 1); 

    mmp_wfd_events = event_create(THREAD_MAX);

    return 0;

}

int32_t mmp_wfd_begin(tracer_handle_t self, const char *name, uint32_t pid) {
    MMP_Event event;
    char event_name[16];
    generate_event_name(name, event_name, 15);
    
    if ((event = MMProfileFindEvent(MMP_WFD_MESSAGE, event_name)) == 0 ) {
            event = MMProfileRegisterEvent(MMP_WFD_MESSAGE, event_name);
            MMProfileEnableEvent(event, 1); 
    }
    MMProfileLogMetaStringEx(event, MMProfileFlagStart, pid, 0, name);
    event_add((int32_t)pid, event_name);

    return 0;
}

int32_t mmp_wfd_end(tracer_handle_t self, const char* name, uint32_t pid) {
    MMP_Event event;
    struct event_list *p = event_search((int32_t)pid);
    if (p && (event = MMProfileFindEvent(MMP_WFD_MESSAGE, p->name))) {
        MMProfileLogMetaStringEx(event, MMProfileFlagEnd, pid, 0, p->name);
        event_remove((int32_t)pid);
    }

    return 0;
}

int32_t mmp_wfd_oneshot(tracer_handle_t self, uint32_t type, const char *name, uint32_t pid) {

    MMP_Event event;

    SXLOGV("Enter function %s, type = %d, name = %s", __FUNCTION__, type, name);
    switch(type) {
        case ATRACE_ONESHOT_ADATA:
            event = MMP_WFD_ADATA;
            break;
        case ATRACE_ONESHOT_VDATA:
            event = MMP_WFD_VDATA;
            break;
        case ATRACE_ONESHOT_SPECIAL:
        { 
            const char *p = name;
            char event_name[16];
            int i = 0;
            while (*p != '\0') {
                if (*p == ',' || i > 14) 
                    break;
                event_name[i] = *p++;
                i++;
            }
            event_name[i] = '\0';
            
            struct mmp_event_info *info;
            if ((info = mmp_hash_search(event_name, mmp_wfd_hashTable)) == NULL) {
                event = MMProfileRegisterEvent(MMP_WFD_SPECIAL, event_name);
                MMProfileEnableEvent(event, 1); 
                mmp_hash_insert(event_name, event,mmp_wfd_hashTable);
            }
            else {
                event = info->id;
            }
            break;
        }
        default:
            SXLOGE("Not Support ATRACE ONE SHOT type: %d", type);
            return -1;
    }

    MMProfileLogMetaStringEx(event, MMProfileFlagPulse, (unsigned int)(*self)->tag, 0, name);
    return 0;
}

#endif











