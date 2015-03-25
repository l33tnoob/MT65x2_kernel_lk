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

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <android_runtime/AndroidRuntime.h>
#include "jni.h"
#include "custom_prop.h"

typedef struct _ItemHead{
    struct _ItemHead *next;
    struct _ItemHead *prev;
} ItemHead;

typedef struct _ConfigItem {
    ItemHead head;
    char module[MAX_MODULE_LEN];
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    char prop[PROPERTY_KEY_MAX];
} ConfigItem;

typedef enum {
    STATE_PREFIX,
    STATE_KEY,
    STATE_VALUE,
    STATE_PROP,
    STATE_COUNT
} ParseLineState;

typedef struct {
    int  state;
    char *write;
    char *prop;
    ConfigItem *output;
} Parser;

typedef int (*StateHandler)(const char *in, Parser *parser);
typedef int (*PropFinder)(char *buff);

typedef struct {
    const char *name;
    PropFinder finder;
} CustomProp;

static ConfigItem confItems[MAX_ITEM_NUM+1];
static ItemHead itemList;
static Parser confParser;
static int initDone;
static pthread_mutex_t parseMutex = PTHREAD_MUTEX_INITIALIZER;

static int parse_state_prefix(const char *in, Parser *parser);
static int parse_state_key(const char *in, Parser *parser);
static int parse_state_value(const char *in, Parser *parser);
static int parse_state_property(const char *in, Parser *parser);

static JNIEnv *load_jni_env();
static jclass load_interface(JNIEnv *env);

static int get_kernel_version(char *buff);
static int get_browser_version(char *buff);
static int get_release_date(char *buff);
static int dummy_prop_hdlr(char *buff);

static const StateHandler StateHdlrs[STATE_COUNT] = {
    parse_state_prefix,
    parse_state_key,
    parse_state_value,
    parse_state_property
};

static const CustomProp CustomProperties[] = {
    {"apc.kernel.version",   get_kernel_version},
    {"apc.browser.version",  get_browser_version},
    {"apc.release.date",     get_release_date},
    {0, dummy_prop_hdlr}
};

using namespace android;

static int dummy_prop_hdlr(char *buff)
{
    return 0;
}

static int get_kernel_version(char *buff)
{
    struct utsname name;
    int len;

    ALOGI("[custom_prop]get_kernel_version");
    if (uname(&name) < 0)
        return 0;

    ALOGI("[custom_prop]kenel_version[%s]", name.release);
    len = strlen(name.release);
    memcpy(buff, name.release, len);

    return len;
}

static int get_browser_version(char *buff)
{
    JNIEnv *env;
    jclass clazz;
    jmethodID method;
    jstring result;
    const char *str;
    int len;

    if (!(env = load_jni_env()))
        return 0;

    if (!(clazz = load_interface(env)))
        return 0;

/*    clazz = env->FindClass("com/mediatek/custom/CustomPropInterface");
    ALOGI("[custom_prop]FindClass[%d]", (int)clazz);
    if (!clazz)
        goto fail1;
*/
    method = env->GetStaticMethodID(clazz, "getBrowserVersion", "()Ljava/lang/String;");
    ALOGI("[custom_prop]getMethod[%d]", (int)method);
    if (!method)
        return 0;

    result = (jstring)env->CallStaticObjectMethod(clazz, method);
    ALOGI("[custom_prop]callMethod[%d]", (int)result);
    if (!result)
        return 0;

    str = env->GetStringUTFChars(result, 0);
    ALOGI("[custom_prop]browser_version[%s]", str);
    len = strlen(str);
    memcpy(buff, str, len);

    env->ReleaseStringUTFChars(result, str);
    
    return len;
}

static int get_release_date(char *buff)
{
    JNIEnv *env;
    jclass clazz;
    jmethodID method;
    jstring result, in;
    const char *str;
    char date[PROPERTY_VALUE_MAX];
    int len;

    if (!(env = load_jni_env()))
        return 0;
    
    if (!(clazz = load_interface(env)))
        return 0;
    
/*  clazz = env->FindClass("com/mediatek/custom/CustomPropInterface");
    ALOGI("[custom_prop]FindClass[%d]", (int)clazz);
    if (!clazz)
        goto fail2;
*/
    method = env->GetStaticMethodID(clazz, "getReleaseDate", "(Ljava/lang/String;)Ljava/lang/String;");
    ALOGI("[custom_prop]getMethod[%d]", (int)method);
    if (!method)
        return 0;

    property_get("ro.build.date", date, 0);
    in = env->NewStringUTF(date);
    result = (jstring)env->CallStaticObjectMethod(clazz, method, in);
    ALOGI("[custom_prop]callMethod[%d]", (int)result);
    env->DeleteLocalRef(in);
    if (!result)
        return 0;

    str = env->GetStringUTFChars(result, 0);
    ALOGI("[custom_prop]release_date[%s]", str);
    len = strlen(str);
    memcpy(buff, str, len);

    env->ReleaseStringUTFChars(result, str);
    
    return len;
}

static JNIEnv *load_jni_env()
{
    void *handle;
    JNIEnv *env;
    JNIEnv* (*runtime_hdlr)();
    
    handle = dlopen("/system/lib/libcustom_jni.so", RTLD_NOW);
    if (!handle)
        return 0;

    runtime_hdlr = (JNIEnv* (*)())dlsym(handle, "getJNIEnv");
    if (!runtime_hdlr) {
        dlclose(handle);
        return 0;
    }

    env = runtime_hdlr();

    ALOGI("[custom_prop]getEnv[%d]", (int)(env));

    return env;
}

static jclass load_interface(JNIEnv *env)
{
    jclass clazz, intrfc;
    jmethodID method;
    jboolean success;
    
    clazz = env->FindClass("com/mediatek/custom/CustomProperties");
    ALOGI("[custom_prop]FindClass[%d]", (int)clazz);
    
    method = env->GetStaticMethodID(clazz, "loadInterface", "()Ljava/lang/Class;");
    ALOGI("[custom_prop]getMethod[%d]", (int)method);
    if (!method)
        return 0;

    intrfc = (jclass)env->CallStaticObjectMethod(clazz, method);
    ALOGI("[custom_prop]callMethod[%d]", (int)intrfc);

    return intrfc;
}

static ConfigItem *new_config_item()
{
    ConfigItem *node;
    int i;

    for (i = 0; i < MAX_ITEM_NUM; i++) {
        if (!confItems[i].head.next && !confItems[i].head.prev) {
            memset(&confItems[i], 0, sizeof(confItems[0]));
            break;
        }
    }

    if (i == MAX_ITEM_NUM)
        return 0;

    if (!itemList.prev) {
        itemList.prev = itemList.next = &itemList;
    }

    confItems[i].head.next = &itemList;
    confItems[i].head.prev = itemList.prev;
    itemList.prev->next = &confItems[i].head;
    itemList.prev = &confItems[i].head;

    return &confItems[i];
}

static void delete_config_item(ItemHead *item)
{
    if (!itemList.prev || itemList.prev == &itemList)
        return;

    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next = item->prev = 0;
}

static ConfigItem *search_config_item(const char *module, const char *key)
{
    ConfigItem *item, *find = 0;

    if (!itemList.prev || itemList.prev == &itemList)
        return 0;

    for (item = (ConfigItem *)itemList.next; &item->head != &itemList; item = (ConfigItem *)item->head.next) {
        if (strcmp(item->key, key) == 0) {
            if (!module || strlen(module) == 0)
                return item;
            if (strlen(item->module) == 0 || item->module[0] == '*') {
                find = item;
                continue;
            }
            if (strcmp(item->module, module) == 0)
                return item;
        }
    }

    if (find) return find;

    return 0;
}

static void reset_parser(Parser *parser, int keep_output)
{
    ALOGI("[custom_prop]reset_parser->keep[%d]", keep_output);

    if (keep_output) {
        *parser->write = '\0';
    }
    else if (parser->output) {
        delete_config_item(&parser->output->head);
    }

    parser->output = new_config_item();
    parser->write = parser->output->key;
    parser->state = STATE_PREFIX;
}


static int parse_state_prefix(const char *in, Parser *parser)
{
    int len = 0;

    switch (*in) {
    case '\n':
    case '\r':
    case '\t':
    case ' ':
        len++;
        break;

    case '#':
        while (in[len] && in[len] != '\n')
            len++;
        break;

    default:
        parser->state = STATE_KEY;
        break;
    }

    return len;
}


static int parse_state_key(const char *in, Parser *parser)
{
    ConfigItem *item = parser->output;
    int len = 1;

    switch (*in) {
    case '\r':
    case '\t':
    case ' ':
        break;

    case '\n':
        reset_parser(parser, 0);
        break;

    case '.':
        *parser->write = '\0';
        strncpy(item->module, item->key, sizeof(item->module)-1);
        parser->write = item->key;

        /* treat multiple '.' as single '.' */
        while (in[len] == '.')
            len++;

        break;

    case '=':
        ALOGI("[custom_prop]enter STATE_VALUE");
        parser->state = STATE_VALUE;
        *parser->write = '\0';
        parser->write = item->value;
        /* skip white space right following '=' */
        while (in[len] == '\t' || in[len] == ' ')
            len++;
        break;

    default:
        *parser->write++ = *in;
        break;
    }
    
    return len;
}

static int parse_state_value(const char *in, Parser *parser)
{
    ConfigItem *item = parser->output;
    int len = 1;

    switch (*in) {
    case '\r':
        break;

    case '\n':
        reset_parser(parser, 1);
        break;

    case '@':
        ALOGI("[custom_prop]enter STATE_PROP");
        parser->state = STATE_PROP;
        parser->prop = item->prop;
        break;

    default:
        *parser->write++ = *in;
        break;
    }
    
    return len;
}

static int parse_state_property(const char *in, Parser *parser)
{
    ConfigItem *item = parser->output;
    int len = 1, i;

    switch (*in) {
    case '\n':
    case '\r':
    case '\t':
    case ' ':
    case '@':
        *parser->prop = '\0';
        ALOGI("[custom_prop]prop name:%s", item->prop);
        for (i = 0; CustomProperties[i].name; i++) {
            if (strcmp(item->prop, CustomProperties[i].name) == 0) {
                parser->write += CustomProperties[i].finder(parser->write);
                break;
            }
        }
        if (!CustomProperties[i].name) {
            parser->write += property_get(item->prop, parser->write, 0);
        }
        parser->prop = 0;
        if ('\n' == *in)
            reset_parser(parser, 1);
        else {
            ALOGI("[custom_prop]enter STATE_VALUE");
            parser->state = STATE_VALUE;
        }
        break;

    default:
        *parser->prop++ = *in;
        break;
    }

    return len;
}

static int parse_data(const char *data, Parser *parser)
{
    const char *in = data;
    int len;

    reset_parser(parser, 0);

    while (*in) {
        len = StateHdlrs[parser->state](in, parser);
        in += len;
    }

    /* handle config item without CR/LF ending */
    if (parser->state == STATE_VALUE) {
        if (parser->prop) {
            *parser->prop = '\0';
            parser->write += property_get(parser->prop, parser->write, 0);
        }
        *parser->write = '\0';
    }
    else
        reset_parser(parser, 0);

    return 1;
}


static int parse_file(const char *name)
{
    int fd, size, result = 0;
    char *data = 0;

    fd = open(name, O_RDONLY);
    if (fd < 0) return 0;

    size = lseek(fd, 0, SEEK_END);
    if (size < 0) goto parse_fail;

    if (lseek(fd, 0, SEEK_SET) != 0)
        goto parse_fail;

    data = (char *)malloc(size+1);
    if (!data) goto parse_fail;

    if (read(fd, data, size) != size)
        goto parse_fail;

    data[size] = '\0';

    result = parse_data(data, &confParser);

parse_fail:
    if (data)
        free(data);
    close(fd);

    return result;
}

int custom_get_string(const char *module, const char *key, char *value, const char *default_value)
{
    ConfigItem *item;

    if (module != 0 && key != 0)
        ALOGI("[custom_prop]custom_get_string->module[%s],key[%s]", module,key);
    else if (key != 0)
        ALOGI("[custom_prop]custom_get_string->module[null],key[%s]", key);
    else if (module != 0)
        ALOGI("[custom_prop]custom_get_string->module[%s],key[null]", module);

    pthread_mutex_lock(&parseMutex);
    if (!initDone) {
        if (parse_file("/system/etc/custom.conf"))
            initDone = 1;
        else {
            pthread_mutex_unlock(&parseMutex);
            goto find_fail;
        }
    }
    pthread_mutex_unlock(&parseMutex);

    item = search_config_item(module, key);

    if (!item)
        goto find_fail;

    strcpy(value, item->value);

    return strlen(item->value);

find_fail:
    if (default_value) {
        strcpy(value, default_value);
        return strlen(default_value);
    }

    return -1;
}


