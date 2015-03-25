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


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "ftm.h"

#define TAG         "[FTM] "
#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)

struct ftm_prop_head {
    struct ftm_prop *head;
    struct ftm_prop *tail;
};

struct ftm_prop {
    char *name;
    char *val;
    struct ftm_prop *next;
};

extern ftm_init_fn ftm_init_funcs[];
extern ftm_init_fn ftm_init_debug_funcs[];

static struct ftm_module *ftm_mods[ITEM_MAX_IDS];
static struct ftm_module *ftm_db_mods[ITEM_MAX_IDS];

static struct ftm_prop_head ftm_props = {NULL, NULL};

static int ftm_default_entry(struct ftm_param *param,void *priv)
{
    struct ftm_module *mod = (struct ftm_module *)priv;

    LOGD(TAG "id = %d is not handled\n", mod->id);

    return 0;
}

int ftm_init(void)
{
    int i, ret = 0;
    char linebuf[512];
    struct ftm_module *mod;
    size_t nbytes;
    ftm_init_fn *initfn = &ftm_init_funcs[0];
	ftm_init_fn *initdbfn = &ftm_init_debug_funcs[0];

    memset(&ftm_mods[0], 0, ITEM_MAX_IDS * sizeof(struct ftm_module *));

	memset(&ftm_db_mods[0], 0, ITEM_MAX_IDS * sizeof(struct ftm_module *));

    /* init ftm modules */
    while (*initfn != NULL) {
        (*initfn)(); initfn++;
    }

	while (*initdbfn != NULL) {
		LOGD(TAG "initdbfn");
        (*initdbfn)(); initdbfn++;
    }

	
	LOGD(TAG "%s end\n", __FUNCTION__);

    return 0;
}

struct ftm_module *ftm_alloc(int id, int extra)
{
    struct ftm_module *mod = NULL;

    if (id < 0 || id > ITEM_MAX_IDS - 1)
        return NULL;

    mod = malloc(sizeof(struct ftm_module) + extra);

    if (mod) {
        memset(mod, 0, sizeof(struct ftm_module) + extra);
        mod->id = id;
        mod->entry = ftm_default_entry;
        mod->priv  = (void*)mod;
        mod->test_result = FTM_TEST_UNKNOWN;        
    }

    return mod;
}

void ftm_free(struct ftm_module *mod)
{
    if (mod) {
        if (mod->id < 0 || mod->id > ITEM_MAX_IDS - 1)
            return;
        free(mod);
    }
}

int ftm_entry(int id, struct ftm_param *param)
{    
    if (id < 0 || id > ITEM_MAX_IDS - 1)
        return -EINVAL;

    if (ftm_mods[id] && ftm_mods[id]->entry)
        return ftm_mods[id]->entry(param, ftm_mods[id]->priv);

    return -ENOENT;
}

int ftm_debug_entry(int id, struct ftm_param *param)
{    
    if (id < 0 || id > ITEM_MAX_IDS - 1)
        return -EINVAL;
	LOGD(TAG "%s\n", __FUNCTION__);

    if (ftm_db_mods[id] && ftm_db_mods[id]->entry)
        return ftm_db_mods[id]->entry(param, ftm_db_mods[id]->priv);

    return -ENOENT;
}


int ftm_register(struct ftm_module *mod, ftm_entry_fn entry, void *priv)
{
    if (mod == NULL || entry == NULL)
        return -EINVAL;
    if (mod->id < 0 || mod->id > ITEM_MAX_IDS - 1)
        return -EINVAL;

    mod->test_result = FTM_TEST_UNKNOWN;
    mod->visible = true;
    mod->entry = entry;
    mod->priv  = priv;
    ftm_mods[mod->id] = mod;

    return 0;
}

int ftm_db_register(struct ftm_module *mod, ftm_entry_fn entry, void *priv)
{
    if (mod == NULL || entry == NULL)
        return -EINVAL;
    if (mod->id < 0 || mod->id > ITEM_MAX_IDS - 1)
        return -EINVAL;
	
	LOGD(TAG "ftm_db_register mod->id=%d",mod->id);

    mod->test_result = FTM_TEST_UNKNOWN;
    mod->visible = true;
    mod->entry = entry;
    mod->priv  = priv;
	
    ftm_db_mods[mod->id] = mod;

    return 0;
}


void ftm_unregister(struct ftm_module *mod)
{
    if (mod == NULL)
        return;
    if (mod->id < 0 || mod->id > ITEM_MAX_IDS - 1)
        return;
    ftm_mods[mod->id] = NULL;
}

void ftm_db_unregister(struct ftm_module *mod)
{
    if (mod == NULL)
        return;
    if (mod->id < 0 || mod->id > ITEM_MAX_IDS - 1)
        return;
    ftm_db_mods[mod->id] = NULL;
}




struct ftm_module **ftm_get_modules(void)
{
    return ftm_mods;
}

struct ftm_module *ftm_get_module(int id)
{
    int i;

    for (i = 0; i < ITEM_MAX_IDS; i++) {
        if (!ftm_mods[i])
            continue;
        if (id == ftm_mods[i]->id)
            return ftm_mods[i];
    }
    
    return NULL;
}

void ftm_set_result(struct ftm_module *mod, int result)
{
    mod->test_result = result;
}

int ftm_get_result(struct ftm_module *mod)
{
    return mod->test_result;
}

int ftm_set_prop(const char *name, const char *val)
{
    struct ftm_prop *p, *t;
    int namelen, valuelen;

    if (!name)
        return -EINVAL;

    namelen  = strlen(name) + 1;
    valuelen = val ? strlen(val) + 1 : 0;

    p = ftm_props.head;
    t = ftm_props.tail;

    while (p) {
        if (p->name && !strcmp(p->name, name)) {
            LOGD(TAG "%s: EXIST(%s)\n", __FUNCTION__, name);
            strncpy(p->val, val, valuelen);
            return -EEXIST;
        }
        if (p == t)
            break;
        p = p->next;
    }

    p = malloc(sizeof(struct ftm_prop) + namelen + valuelen);

    if (!p)
        return -ENOMEM;

    p->name = (char *)(p + 1);
    p->val  = val ? p->name + namelen : NULL;
    p->next = NULL;

    if (p->name)
        strncpy(p->name, name, namelen);
    if (p->val)
        strncpy(p->val, val, valuelen);

    if (ftm_props.head) {
        ftm_props.tail->next = p;
        ftm_props.tail = p;
    } else {
        ftm_props.head = p;
        ftm_props.tail = p;
    }
    
    return 0;
}

char *ftm_get_prop(const char *name)
{
    struct ftm_prop *p;
    struct ftm_prop *t;

    p = ftm_props.head;
    t = ftm_props.tail;

    if (p == NULL || name == NULL)
        return NULL;

    do {
        if (!strcmp(name, p->name))
            return p->val;
        if (p == t)
            break;
        p = p->next;
    } while (p);

    return NULL;
}

void ftm_dump_prop(void)
{
    struct ftm_prop *p;
    struct ftm_prop *t;

    LOGD(TAG "%s\n", __FUNCTION__);
    p = ftm_props.head;
    t = ftm_props.tail;
    
    while (p) {
        LOGD(TAG "<PROP> %s=%s\n", p->name, p->val ? p->val : "null");
        if (p == t)
            break;
        p = p->next;
    }

    return;
}

