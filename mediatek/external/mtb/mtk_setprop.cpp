/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <binder/IBinder.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>

#include <cutils/properties.h>
#include <utils/String8.h>
#define LOG_TAG "mtk_setprop"
#include <cutils/log.h>

using namespace android;

// Poke all the binder-enabled processes in the system to get them to re-read
// their system properties.
static bool pokeBinderServices()
{
    sp<IServiceManager> sm = defaultServiceManager();
    Vector<String16> services = sm->listServices();
    ALOGE("binder service size : %d", services.size());
    for (size_t i = 0; i < services.size(); i++) {
        sp<IBinder> obj = sm->checkService(services[i]);
        String8 s(services[i]);
        ALOGE("binder service %s", s.string());
        if (obj != NULL) {
            Parcel data;
            if (obj->transact(IBinder::SYSPROPS_TRANSACTION, data,
                    NULL, 0) != OK) {
                if (false) {
                    // XXX: For some reason this fails on tablets trying to
                    // poke the "phone" service.  It's not clear whether some
                    // are expected to fail.
                    String8 svc(services[i]);
                    fprintf(stderr, "error poking binder service %s\n",
                        svc.string());
                    return false;
                }
            }
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    ALOGE("Enter mtk_setprop");
    if(argc != 3) {
        fprintf(stderr,"usage: setprop <key> <value>\n");
        return 1;
    }

    if(property_set(argv[1], argv[2])){
        fprintf(stderr,"could not set property\n");
        return 1;
    }

    pokeBinderServices();

    ALOGE("Exit mtk_setprop");
    return 0;
}
