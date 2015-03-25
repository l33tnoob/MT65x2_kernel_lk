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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_NDEBUG 0
#define LOG_TAG "MemoryDumper"
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "MemoryDumper.h"
#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/SystemClock.h>
#include <utils/Vector.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get

#include <utils/misc.h>
#include <binder/IServiceManager.h>

namespace android {

const size_t READ_BLOCK_SIZE = 4096;

#define DUMPER_PATH "/sdcard"

void MemoryDumper::instantiate() {
    ALOGV("Instantiate memory dumper service...");
    const size_t FILENAME_SIZE = 64;
    char dumpFileName[FILENAME_SIZE];
    memset(dumpFileName, 0, FILENAME_SIZE);
    snprintf(dumpFileName, FILENAME_SIZE-1, DUMPER_PATH "/memstatus_%d", getpid()); 
    defaultServiceManager()->addService(
            String16("memory.dumper"), new MemoryDumper(dumpFileName));
}

MemoryDumper::MemoryDumper(const char* destFile)
    :m_fileName(destFile)
    ,m_dumpNo(0)
{
    ALOGV("MemoryDumper created, dump file=%s", (const char*)m_fileName);
}

MemoryDumper::~MemoryDumper()
{
    ALOGV("MemoryDumper destructor.");
}

#if defined(__arm__)
extern "C" void get_malloc_leak_info(uint8_t** info, size_t* overallSize,
        size_t* infoSize, size_t* totalMemory, size_t* backtraceSize);
extern "C" void free_malloc_leak_info(uint8_t* info);

//E5568c, this file is copied from MediaPlayerService.cpp: 
//      void memStatus(int fd, const Vector<String16>& args) 
//
bool MemoryDumper::dumpHeap()
{
    ALOGV("enter dumpHeap");
    bool isDumpSuccess = false;
    const size_t FILENAME_SIZE = 1024;
    char fileName[FILENAME_SIZE];
    memset(fileName, 0, FILENAME_SIZE);
    snprintf(fileName, FILENAME_SIZE-1, "%s.%d", (const char*)m_fileName, m_dumpNo);
    m_dumpNo++;
    FILE *f = fopen(fileName, "w+");
    if(f == NULL)
    {
        ALOGE("Open file failed: %s!", (const char*)m_fileName);
        return false;
    }
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    typedef struct {
        size_t size;
        size_t dups;
        intptr_t * backtrace;
    } AllocEntry;

    uint8_t *info = NULL;
    size_t overallSize = 0;
    size_t infoSize = 0;
    size_t totalMemory = 0;
    size_t backtraceSize = 0;

    get_malloc_leak_info(&info, &overallSize, &infoSize, &totalMemory, &backtraceSize);
    ALOGI("returned from get_malloc_leak_info, info=0x%x, overallSize=%d, infoSize=%d, totalMemory=%d, backtraceSize=%d", (int)info, overallSize, infoSize, totalMemory, backtraceSize);
    if (info) {
        uint8_t *ptr = info;
        size_t count = overallSize / infoSize;

        snprintf(buffer, SIZE, " Allocation count %i\n", count);
        result.append(buffer);
        snprintf(buffer, SIZE, " Total meory %i\n", totalMemory);
        result.append(buffer);

        AllocEntry * entries = new AllocEntry[count];

        for (size_t i = 0; i < count; i++) {
            // Each entry should be size_t, size_t, intptr_t[backtraceSize]
            AllocEntry *e = &entries[i];

            e->size = *reinterpret_cast<size_t *>(ptr);
            ptr += sizeof(size_t);

            e->dups = *reinterpret_cast<size_t *>(ptr);
            ptr += sizeof(size_t);

            e->backtrace = reinterpret_cast<intptr_t *>(ptr);
            ptr += sizeof(intptr_t) * backtraceSize;
        }

        // Now we need to sort the entries.  They come sorted by size but
        // not by stack trace which causes problems using diff.
        bool moved;
        do {
            moved = false;
            for (size_t i = 0; i < (count - 1); i++) {
                AllocEntry *e1 = &entries[i];                AllocEntry *e2 = &entries[i+1];

                bool swap = e1->size < e2->size;
                if (e1->size == e2->size) {
                    for(size_t j = 0; j < backtraceSize; j++) {
                        if (e1->backtrace[j] == e2->backtrace[j]) {
                            continue;
                        }
                        swap = e1->backtrace[j] < e2->backtrace[j];
                        break;
                    }
                }
                if (swap) {
                    AllocEntry t = entries[i];
                    entries[i] = entries[i+1];
                    entries[i+1] = t;
                    moved = true;
                }
            }
        } while (moved);

        for (size_t i = 0; i < count; i++) {
            AllocEntry *e = &entries[i];

            snprintf(buffer, SIZE, "size %8i, dup %4i, ", e->size, e->dups);
            result.append(buffer);
            for (size_t ct = 0; (ct < backtraceSize) && e->backtrace[ct]; ct++) {
                if (ct) {
                    result.append(", ");
                }
                snprintf(buffer, SIZE, "0x%08x", e->backtrace[ct]);
                result.append(buffer);
            }
            result.append("\n");
        }

        delete[] entries;
        free_malloc_leak_info(info);
        isDumpSuccess = true;
    }
    if(isDumpSuccess)
    {
        write(fileno(f), result.string(), result.size());
        String8 mapsfile(fileName);
        mapsfile.append(".maps");
        ALOGV("save maps file to: %s", (const char*)mapsfile);
        copyfile("/proc/self/maps", (const char*)mapsfile);
        String8 smapsfile(fileName);
        smapsfile.append(".smaps");
        ALOGV("save smaps file to: %s", (const char*)smapsfile);
        copyfile("/proc/self/smaps", (const char*)smapsfile);
        String8 statmfile(fileName);
        statmfile.append(".statm");
        ALOGV("save statm file to: %s", (const char*)statmfile);
        copyfile("/proc/self/statm", (const char*)statmfile);
    }
    fclose(f);
    ALOGV("exit memStatus, isDumpSuccess=%d", (int)isDumpSuccess);
    return isDumpSuccess;
}

#else

bool MemoryDumper::dumpHeap()
{
    ALOGV("MemoryDumper::dumpHeap not supported!.");
    return false;
}

#endif

bool MemoryDumper::saveMaps()
{
    ALOGV("enter saveMaps");
    String8 mapsfile = m_fileName;
    mapsfile.append(".maps");
    return copyfile("/proc/self/maps", (const char*)mapsfile);
}

bool MemoryDumper::copyfile(const char* sourceFile, const char* destFile)
{
    ALOGV("copy file, sourceFile=%s, destFile=%s", sourceFile, destFile);
    FILE* src = fopen(sourceFile, "r");
    if(src == NULL)
    {
        ALOGE("Open source file failed: %s!", sourceFile);
        return false;
    }
    FILE* dest = fopen(destFile, "w+");
    if(dest == NULL)
    {
        ALOGE("Open dest file failed: %s!", destFile);
        fclose(src);
        return false;
    }
    char buffer[READ_BLOCK_SIZE];
    int readNum = 0;
    while(!feof(src))
    {
        readNum = fread(buffer, 1, READ_BLOCK_SIZE, src);
        if(readNum > 0)
        {
            fwrite(buffer, 1, readNum, dest);
        }
        else
        {
            ALOGE("Read error, readNum=%d, errno=%d", readNum, errno);
            break;
        }
    }
    fclose(src);
    fclose(dest);
    ALOGV("exit copy file.");
    return true;
}

};
