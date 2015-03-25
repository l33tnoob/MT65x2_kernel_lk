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

#define LOG_TAG "OpenGLRenderer"



#include <SkImageEncoder.h>
#include "MTKDebug.h"
#include <GLES3/gl3.h>
#include <unistd.h>



#define TTLOGD(...) \
{                            \
    if (g_HWUI_debug_texture_tracker) \
        XLOGD(__VA_ARGS__); \
}


int g_HWUI_debug_opengl = 1;
int g_HWUI_debug_extensions = 0;
int g_HWUI_debug_init = 0;
int g_HWUI_debug_memory_usage = 0;
int g_HWUI_debug_cache_flush = 1;
int g_HWUI_debug_layers_as_regions = 0;
int g_HWUI_debug_clip_regions = 0;
int g_HWUI_debug_programs = 0;
int g_HWUI_debug_layers = 1;
int g_HWUI_debug_render_buffers = 0;
int g_HWUI_debug_stencil = 0;
int g_HWUI_debug_patches = 0;
int g_HWUI_debug_patches_vertices = 0;
int g_HWUI_debug_patches_empty_vertices = 0;
int g_HWUI_debug_paths = 0;
int g_HWUI_debug_textures = 1;
int g_HWUI_debug_layer_renderer = 1;
int g_HWUI_debug_font_renderer = 0;
int g_HWUI_debug_defer = 0;
int g_HWUI_debug_display_list = 0;
int g_HWUI_debug_display_ops_as_events = 1;
int g_HWUI_debug_merge_behavior = 0;

//MTK debug dump functions
int g_HWUI_debug_texture_tracker = 0;
int g_HWUI_debug_duration = 0;
int g_HWUI_debug_dumpDisplayList = 0;
int g_HWUI_debug_dumpDraw = 0;
int g_HWUI_debug_dumpTexture = 0;
int g_HWUI_debug_dumpAlphaTexture = 0;
int g_HWUI_debug_layer = 0;
int g_HWUI_debug_enhancement = 1;

//MTK sync with egl trace
int g_HWUI_debug_egl_trace = 0;


#if defined(MTK_DEBUG_RENDERER)

static bool getProcessName(char* psProcessName, int size)
{
    FILE *f;
    char *slash;

    if (!psProcessName)
        return false;

    f = fopen("/proc/self/cmdline", "r");
    if (!f)
    {
        XLOGE("Can't get application name");
        return false;
    }

    if (fgets(psProcessName, size, f) == NULL)
    {
        XLOGE("ame : fgets failed");
        fclose(f);
        return false;
    }

    fclose(f);

    if ((slash = strrchr(psProcessName, '/')) != NULL)
    {
        memmove(psProcessName, slash+1, strlen(slash));
    }

    return true;
}

static bool dumpImage(int width, int height, const char *filename)
{
    size_t size = width * height * 4;
    GLbyte *buf = (GLbyte*)malloc(size);
    GLenum error;
    bool bRet = true;

    if (!buf)
    {
        XLOGE("%s: failed to allocate buffer (%d bytes)\n", __FUNCTION__, size);
        return false;
    }

    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
    bitmap.setPixels(buf, NULL);

    XLOGI("%s: %dx%d, %s\n", __FUNCTION__, width, height, filename);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels());

    if ((error = glGetError()) != GL_NO_ERROR)
    {
        XLOGE("%s: get GL error 0x%x \n", __FUNCTION__, error);
        bRet = false;
        goto Exit;
    }

    if (!SkImageEncoder::EncodeFile(filename, bitmap, SkImageEncoder::kPNG_Type, 100))
    {
        XLOGE("%s: Failed to encode image %s\n", __FUNCTION__, filename);
        bRet = false;
        goto Exit;
    }

Exit:
    free(buf);
    return bRet;
}


bool dumpDisplayList(int width, int height, int level)
{
    static int frame = 0;
    static int count = 0;
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    if (level == 0)
    {
        count = 0;
        frame++;
    }
    sprintf(file, "/data/data/%s/dp_%04d_%04d.png", pch, frame, count++);
    return dumpImage(width, height, file);
}

bool dumpDraw(int width, int height, bool newFrame)
{
    static int frame = 0;
    static int count = 0;
    char procName[256];
    char file[512];


    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    if (newFrame)
    {
        count = 0;
        frame++;
        return false;
    } else {
        sprintf(file, "/data/data/%s/draw_%04d_%04d.png", pch, frame, count++);
        return dumpImage(width, height, file);
    }

}

bool dumpTexture(int texture, int width, int height, SkBitmap *bitmap)
{
    char procName[256];
    char file[512];

    if (!g_HWUI_debug_dumpTexture)
        return false;

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    sprintf(file, "/data/data/%s/tex_%d_%d_%d_%p.png", pch, texture, width, height, bitmap);
    if (!SkImageEncoder::EncodeFile(file, *bitmap, SkImageEncoder::kPNG_Type, 100))
    {
        XLOGE("%s: Fail to dump texture: %s", __FUNCTION__, file);
        return false;
    }

    XLOGI("%s: %dx%d, %s", __FUNCTION__, width, height, file);
    return true;
}

bool dumpAlphaTexture(int width, int height, uint8_t *data, const char *prefix, SkBitmap::Config format)
{
    static int count = 0;
    char procName[256];
    char file[512];
    SkBitmap bitmap;
    SkBitmap bitmapCopy;

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    sprintf(file, "/data/data/%s/%s_%04d.png", procName, prefix, count++);
    XLOGI("%s: %dx%d %s\n", __FUNCTION__, width, height, file);

    bitmap.setConfig(format, width, height);
    bitmap.setPixels(data, NULL);

    if (!bitmap.copyTo(&bitmapCopy, SkBitmap::kARGB_8888_Config))
    {
        XLOGD("%s: Failed to copy data", __FUNCTION__);
        return false;
    }

    if (!SkImageEncoder::EncodeFile(file, bitmapCopy, SkImageEncoder::kPNG_Type, 100))
    {
        XLOGE("%s: Failed to encode image %s\n", __FUNCTION__, file);
        return false;
    }

    return true;
}

bool dumpLayer(int width, int height, int fbo)
{
    static int frame = 0;
    char procName[256];
    char file[512];

    if (!getProcessName(procName, sizeof(procName)))
        return false;

    char *pch;
    pch = strtok(procName, ":"); // truncate, e.g: com.android.systemui:screenshot
    pch = pch == NULL ? procName : pch;

    sprintf(file, "/data/data/%s/layer_%d_%d_%d_%04d.png", pch, fbo, width, height, frame);

    frame++;
    return dumpImage(width, height, file);
}


namespace android {

#ifdef USE_OPENGL_RENDERER
using namespace uirenderer;
ANDROID_SINGLETON_STATIC_INSTANCE(TextureTracker);
#endif

namespace uirenderer {

///////////////////////////////////////////////////////////////////////////////
// Constructors/destructor
///////////////////////////////////////////////////////////////////////////////

TextureTracker::TextureTracker() {
    TTLOGD("[TT]TextureTracker +");
    mPid = getpid();
}

TextureTracker::~TextureTracker() {
    TTLOGD("[TT]TextureTracker -");
}


///////////////////////////////////////////////////////////////////////////////
// Monitoring
///////////////////////////////////////////////////////////////////////////////

void TextureTracker::startMark(String8 name) {
    Mutex::Autolock _l(mLock);
    mViews.push(name);
}

void TextureTracker::endMark() {
    Mutex::Autolock _l(mLock);
    mViews.pop();
}

void TextureTracker::add(int textureId, int w, int h, int format, int type, String8 purpose, const char* comment) {
    Mutex::Autolock _l(mLock);

    if (mViews.size() == 0) {
        if (comment != NULL) {
            XLOGE("[TT]add error %s %d %d %d 0x%x 0x%x %s", comment, textureId, w, h, format, type, purpose.string());
        } else {
            XLOGE("[TT]add error %d %d %d 0x%x 0x%x %s", textureId, w, h, format, type, purpose.string());
        }
        return;
    }
    TextureEntry entry(mViews.top(), textureId, w, h, format, type, purpose);
    mMemoryList.add(entry);


    if (comment != NULL) {
        TTLOGD("[TT]%s %s %d %d %d 0x%x 0x%x => %d %s", comment, entry.mName.string(), textureId, w, h, format, type, entry.mMemory, purpose.string());
    }

}

void TextureTracker::add(String8 name, int textureId, int w, int h, int format, int type, String8 purpose, const char* comment) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(name, textureId, w, h, format, type, purpose);
    mMemoryList.add(entry);

    if (comment != NULL) {
        TTLOGD("[TT]%s %s %d %d %d 0x%x 0x%x => %d %s", comment, name.string(), textureId, w, h, format, type, entry.mMemory, purpose.string());
    }
}

void TextureTracker::remove(int textureId, const char* comment) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(textureId);
    ssize_t index = mMemoryList.indexOf(entry);

    if (index >= 0) {
        entry = mMemoryList.itemAt(index);
        mMemoryList.removeAt(index);

        TTLOGD("[TT]%s %s %d", comment, entry.mName.string(), textureId);
    } else {
        TTLOGD("[TT]%s already %d", comment, textureId);
    }

}

void TextureTracker::update(int textureId, bool ghost, String8 name) {
    Mutex::Autolock _l(mLock);
    TextureEntry entry(textureId);
    ssize_t index = mMemoryList.indexOf(entry);

    if (index >= 0) {
        TextureEntry& item = mMemoryList.editItemAt(index);
        TTLOGD("[TT]update before %s %d %d %d %d %d\n", item.mName.string(), item.mId, item.mWidth, item.mHeight,
                        item.mMemory, item.mGhost);

        item.mGhost = ghost;

        if (name.isEmpty()) {
            if (!ghost) {
                item.mName = mViews.top();
            }
        } else {
            item.mName = name;
        }


        entry = mMemoryList.itemAt(index);
        TTLOGD("[TT]update after %s %d %d %d %d %d\n", entry.mName.string(), entry.mId, entry.mWidth, entry.mHeight,
                    entry.mMemory, entry.mGhost);
    } else {
        TTLOGD("[TT]update not found %d", textureId);
    }

}


int TextureTracker::estimateMemory(int w, int h, int format, int type) {

    int power2[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int width = 0;
    int height = 0;
    int i;
    for (i = 0; i < 14; ++i) {
        if (power2[i] >= w)
            break;
    }
    if (i < 14) {
        width = power2[i];
    }

    for (i = 0; i < 14; ++i) {
        if (power2[i] >= h)
            break;
    }
    if (i < 14) {
        height = power2[i];
    }


    int bytesPerPixel = 0;

    switch (type) {
        case GL_UNSIGNED_BYTE:
            switch (format) {
                case GL_RGBA:
                    bytesPerPixel = 4;
                    break;
                case GL_RGB:
                    bytesPerPixel = 3;
                    break;
                case GL_LUMINANCE_ALPHA:
                    bytesPerPixel = 2;
                    break;
                case GL_ALPHA:
                case GL_LUMINANCE:
                    bytesPerPixel = 1;
                    break;
                default:
                    XLOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
                    break;
            }
            break;
        case GL_UNSIGNED_SHORT_4_4_4_4: // GL_RGBA format
        case GL_UNSIGNED_SHORT_5_5_5_1: // GL_RGBA format
        case GL_UNSIGNED_SHORT_5_6_5:   // GL_RGB
            bytesPerPixel = 2;
            break;
        case GL_FLOAT:
            switch (format) {
                case GL_RED:
                    bytesPerPixel = 2;
                    break;
                case GL_RGBA:
                    bytesPerPixel = 8;
                    break;
                default:
                    XLOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
                    break;
            }
            break;
        default:
            XLOGE("[TT]estimateMemory Error!! type:0x%x, format:0x%x", type, format);
            break;
    }

    int size = 0;

    if (bytesPerPixel != 0) {
        size = width * height * bytesPerPixel;
        size = size < 4096 ? 4096 : size;
    }

    return size;

}

void TextureTracker::dumpMemoryUsage(String8 &log) {

    log.appendFormat("\nTextureTracker:\n");

    int sum = 0;
    SortedList<String8> list;
    size_t count = mMemoryList.size();
    for (size_t i = 0; i < count; i++) {
        const String8& current = mMemoryList.itemAt(i).mName;
        size_t tmp = list.size();
        bool found = false;
        for (size_t j = 0; j < tmp; j++) {
            if (current == list.itemAt(j)) {
                found = true;
                break;
            }
        }
        if (!found) {
            list.add(current);
        }
    }
    size_t tmp = list.size();

    for (size_t i = 0; i < tmp; i++) {
        const String8& current = list.itemAt(i);
        String8 tmpString;
        int tmpsum = 0;
        for (size_t j = 0; j < count; j++) {
            const TextureEntry& entry = mMemoryList.itemAt(j);
            if (entry.mName == current) {
                String8 format;
                String8 type;
                char s[64];

                switch (entry.mFormat) {
                    case GL_RGBA:
                        format = String8("GL_RGBA");
                        break;
                    case GL_RGB:
                        format = String8("GL_RGB");
                        break;
                    case GL_ALPHA:
                        format = String8("GL_ALPHA");
                        break;
                    case GL_LUMINANCE:
                        format = String8("GL_LUMINANCE");
                        break;
                    case GL_LUMINANCE_ALPHA:
                        format = String8("GL_LUMINANCE_ALPHA");
                        break;
                    default:
                        sprintf(s, "0x%x", entry.mFormat);
                        format = String8(s);
                        break;
                }

                switch (entry.mType) {
                    case GL_UNSIGNED_BYTE:
                        type = String8("GL_UNSIGNED_BYTE");
                        break;
                    case GL_UNSIGNED_SHORT_4_4_4_4:
                        type = String8("GL_UNSIGNED_SHORT_4_4_4_4");
                        break;
                    case GL_UNSIGNED_SHORT_5_5_5_1:
                        type = String8("GL_UNSIGNED_SHORT_5_5_5_1");
                        break;
                    case GL_UNSIGNED_SHORT_5_6_5:
                        type = String8("GL_UNSIGNED_SHORT_5_6_5");
                        break;
                    case GL_FLOAT:
                        type = String8("GL_FLOAT");
                        break;
                    default:
                        sprintf(s, "0x%x", entry.mType);
                        type = String8(s);
                        break;
                }

                tmpString.appendFormat("        %d (%d, %d) (%s, %s) %d <%s> %s\n", entry.mId, entry.mWidth,
                                        entry.mHeight, format.string(), type.string(), entry.mMemory,
                                        entry.mPurpose.string(), entry.mGhost ? "g" : "");
                tmpsum += entry.mMemory;
            }
        }

        sum += tmpsum;
        log.appendFormat("%s: %d bytes, %.2f KB, %.2f MB\n", current.string(), tmpsum, tmpsum / 1024.0f, tmpsum / 1048576.0f);
        log.append(tmpString);
        log.append("\n");
    }


     int rss = load3dUsage();
     log.appendFormat("\nTotal monitored:\n  %d bytes, %.2f KB, %.2f MB\n", sum, sum / 1024.0f, sum / 1048576.0f);
//     log.appendFormat("Physical allocated:\n  %d bytes, %.2f KB, %.2f MB\n", rss, rss / 1024.0f, rss / 1048576.0f);
//     log.appendFormat("Coverage rate:\n  %.2f %%\n", 100 * ((float)sum) / rss);


}



int TextureTracker::load3dUsage()
{
/*    char tmp[128];
    char buf[32];
    FILE *fp;
    long int li;

    sprintf(tmp, "/proc/gpu/mem_usage/%d", mPid);
    fp = fopen(tmp, "r");
    if (fp == 0) return -1;

//    uint64_t start = systemTime(SYSTEM_TIME_MONOTONIC);
    fgets(buf, sizeof(buf) - 1, fp);
//    uint64_t end = systemTime(SYSTEM_TIME_MONOTONIC);
//    ALOGW("load_3d_usage fgets time = %d", (int) ((end - start) / 1000));

    li = atol(buf); // from Bytes to KB // / 1024

    fclose(fp);

    return (int)li;*/
    return 0;
}

}; // namespace uirenderer
}; // namespace android

#endif
