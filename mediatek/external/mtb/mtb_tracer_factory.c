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

#define LOG_TAG "TracerFacotry"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <cutils/xlog.h>
#include "constants.h"
#include "mmp_vp.h"
#include "mmp_wfd.h"
#include "met_vp.h"
#include "mtb_tracer_factory.h"

static tracer_list_t *gTracerList = NULL; 
static pthread_mutex_t gTracerLock = PTHREAD_MUTEX_INITIALIZER; 
static int gLoadDone;

static tracer_t gTracerInfo[] = {
    {
        type : MTB_TRACE_MMP,
        name : "Multimedia Profile",
        load_tracer : tracer_load
    },
    {
        type : MTB_TRACE_MET,
        name : "Mediatek Embedded Technology",
        load_tracer : tracer_load
    }
};

static struct tracer_interface_s gTracerIf[] = {
    {
        tag : MTB_TAG_VIDEO,
        type : MTB_TRACE_MMP,
        init : mmp_vp_init,
        begin : mmp_vp_begin,
        end : mmp_vp_end,
        oneshot : mmp_vp_oneshot
    },
    {
        tag : MTB_TAG_WFD,
        type : MTB_TRACE_MMP,
        init : mmp_wfd_init,
        begin : mmp_wfd_begin,
        end : mmp_wfd_end,
        oneshot : mmp_wfd_oneshot
    },
    {
        tag : MTB_TAG_VIDEO,
        type : MTB_TRACE_MET,
        init : met_vp_init,
        begin : met_vp_begin,
        end : met_vp_end,
        oneshot : met_vp_oneshot
    }
};

int32_t tracer_load()
{
    struct tracer_interface_s *s;
    int i,j;
    int info_size = sizeof(gTracerInfo)/sizeof(gTracerInfo[0]);
    int if_size = sizeof(gTracerIf)/sizeof(gTracerIf[0]);

    if (gLoadDone) {
        return 0;
    }

    tracer_list_t *t = NULL;
    tracer_list_t *p = NULL;
    for (i=0; i<info_size; i++) {
        t = malloc(sizeof(tracer_list_t));
        t->tracer = &gTracerInfo[i];
        t->next = NULL;
        if (!gTracerList) {
            gTracerList = t;
        }
        else {
            p->next = t;
        }
        p = t;

        tracer_item_list_t *l = NULL;
        tracer_item_list_t *h = NULL;
        for (j=0; j<if_size; j++) {
            if (gTracerIf[j].type == t->tracer->type) {
                l = malloc(sizeof(tracer_item_list_t));
                l->trif = &gTracerIf[j];
                l->next = NULL;

                if (!h) {
                    t->items = l;
                }
                else {
                    h->next = l;
                }
                h = l;
            }
        }

    }
    gLoadDone = 1;
    return 0;
}

int tracer_interface(uint32_t type, uint64_t tag, trif_t trif,
                     const char *name, uint32_t pid, uint32_t sub_type ) {
    do {
        tracer_list_t *tl = gTracerList;
        while (tl) {
            if (tl->tracer->type & type) {
                tracer_item_list_t *ti = tl->items;
                while (ti) {
                    if (ti->trif->tag & tag) {
                        switch (trif) {
                            case TRACER_INTERFACE_INIT:
                            {
                                ti->trif->init(&ti->trif);
                                break;
                            }
                            case TRACER_INTERFACE_BEGIN:
                            {
                                ti->trif->begin(&ti->trif, name, pid );
                                break;
                            }
                            case TRACER_INTERFACE_END:
                            {
                                ti->trif->end(&ti->trif, name, pid);
                                break;
                            }
                            case TRACER_INTERFACE_ONESHOT:
                            {
                                ti->trif->oneshot(&ti->trif, sub_type, name, pid);
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    ti = ti->next;
                }
            }
            tl = tl->next;
        }
    }while(0);
    
    return 0;
}
