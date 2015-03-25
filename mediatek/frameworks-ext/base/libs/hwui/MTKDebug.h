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

#ifndef MTK_HWUI_DEBUG_H
#define MTK_HWUI_DEBUG_H

#include <SkBitmap.h>

#include <utils/Singleton.h>
#include <stdio.h>
#include "utils/SortedList.h"
#include <utils/Vector.h>
#include <utils/String8.h>
#include <cutils/properties.h>
#include <cutils/xlog.h>

// Turn on to check for OpenGL errors on each frame
extern int g_HWUI_debug_opengl;
// Turn on to display informations about the GPU
extern int g_HWUI_debug_extensions;
// Turn on to enable initialization information
extern int g_HWUI_debug_init;
// Turn on to enable memory usage summary on each frame
extern int g_HWUI_debug_memory_usage;
// Turn on to enable debugging of cache flushes
extern int g_HWUI_debug_cache_flush;
// Turn on to enable layers debugging when rendered as regions
extern int g_HWUI_debug_layers_as_regions;
// Turn on to enable debugging when the clip is not a rect
extern int g_HWUI_debug_clip_regions;
// Turn on to display debug info about vertex/fragment shaders
extern int g_HWUI_debug_programs;
// Turn on to display info about layers
extern int g_HWUI_debug_layers;
// Turn on to display info about render buffers
extern int g_HWUI_debug_render_buffers;
// Turn on to make stencil operations easier to debug
// (writes 255 instead of 1 in the buffer, forces 8 bit stencil)
extern int g_HWUI_debug_stencil;
// Turn on to display debug info about 9patch objects
extern int g_HWUI_debug_patches;
// Turn on to display vertex and tex coords data about 9patch objects
// This flag requires DEBUG_PATCHES to be turned on
extern int g_HWUI_debug_patches_vertices;
// Turn on to display vertex and tex coords data used by empty quads
// in 9patch objects
// This flag requires DEBUG_PATCHES to be turned on
extern int g_HWUI_debug_patches_empty_vertices;
// Turn on to display debug info about shapes
extern int g_HWUI_debug_paths;
// Turn on to display debug info about textures
extern int g_HWUI_debug_textures;
// Turn on to display debug info about the layer renderer
extern int g_HWUI_debug_layer_renderer;
// Turn on to log draw operation batching and deferral information
extern int g_HWUI_debug_defer;
// Turn on to enable additional debugging in the font renderers
extern int g_HWUI_debug_font_renderer;
// Turn on to dump display list state
extern int g_HWUI_debug_display_list;
// Turn on to insert an event marker for each display list op
extern int g_HWUI_debug_display_ops_as_events;
// Turn on to highlight drawing batches and merged batches with different colors
extern int g_HWUI_debug_merge_behavior;

//MTK debug dump functions
extern int g_HWUI_debug_texture_tracker;
extern int g_HWUI_debug_duration;
extern int g_HWUI_debug_dumpDisplayList;
extern int g_HWUI_debug_dumpDraw;
extern int g_HWUI_debug_dumpTexture;
extern int g_HWUI_debug_dumpAlphaTexture;
extern int g_HWUI_debug_layer;
extern int g_HWUI_debug_enhancement;

//MTK sync with egl trace
extern int g_HWUI_debug_egl_trace;

enum {
    /*
        Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!Critical!!
        If want to modify the below log index, you SHALL also update the responding the array in setDebugLog()"
        Or, it would cuase a build failure!!!
            int* pDebugArray[e_HWUI_DEBUG_END] = {
                &g_HWUI_debug_opengl,
                &g_HWUI_debug_extensions,
                ...

    */
    e_HWUI_DEBUG_BEGIN = 0,
    e_HWUI_DEBUG_OPENGL = e_HWUI_DEBUG_BEGIN,
    e_HWUI_DEBUG_EXTENSIONS,
    e_HWUI_DEBUG_INIT,
    e_HWUI_DEBUG_MEMORY_USAGE,
    e_HWUI_DEBUG_CACHE_FLUSH,
    e_HWUI_DEBUG_LAYERS_AS_REGIONS,
    e_HWUI_DEBUG_CLIP_REGIONS,
    e_HWUI_DEBUG_PROGRAMS,
    e_HWUI_DEBUG_LAYERS,
    e_HWUI_DEBUG_RENDER_BUFFERS,
    e_HWUI_DEBUG_STENCIL,
    e_HWUI_DEBUG_PATCHES,
    e_HWUI_DEBUG_PATCHES_VERTICES,
    e_HWUI_DEBUG_PATCHES_EMPTY_VERTICES,
    e_HWUI_DEBUG_PATHS,
    e_HWUI_DEBUG_TEXTURES,
    e_HWUI_DEBUG_LAYER_RENDERER,
    e_HWUI_DEBUG_FONT_RENDERER,
    e_HWUI_DEBUG_DEFER,
    e_HWUI_DEBUG_DISPLAY_LIST,
    e_HWUI_DEBUG_DISPLAY_OPS_AS_EVENTS,
    e_HWUI_DEBUG_MERGE_BEHAVIOR,

    e_HWUI_DEBUG_TEXTURE_TRACKER,
    e_HWUI_DEBUG_DURATION,
    e_HWUI_DEBUG_DUMPDISPLAYLIST,
    e_HWUI_DEBUG_DUMPDRAW,
    e_HWUI_DEBUG_DUMPTEXTURE,
    e_HWUI_DEBUG_DUMPALPHATEXTURE,
    e_HWUI_DEBUG_DUMPLAYER,
    e_HWUI_DEBUG_ENHANCEMENT,
    e_HWUI_DEBUG_EGL_TRACE,

    e_HWUI_DEBUG_END
};


static void setDebugLog(bool enable) {
    int* pDebugArray[e_HWUI_DEBUG_END] = {
        &g_HWUI_debug_opengl,
        &g_HWUI_debug_extensions,
        &g_HWUI_debug_init,
        &g_HWUI_debug_memory_usage,
        &g_HWUI_debug_cache_flush,
        &g_HWUI_debug_layers_as_regions,
        &g_HWUI_debug_clip_regions,
        &g_HWUI_debug_programs,
        &g_HWUI_debug_layers,
        &g_HWUI_debug_render_buffers,
        &g_HWUI_debug_stencil,
        &g_HWUI_debug_patches,
        &g_HWUI_debug_patches_vertices,
        &g_HWUI_debug_patches_empty_vertices,
        &g_HWUI_debug_paths,
        &g_HWUI_debug_textures,
        &g_HWUI_debug_layer_renderer,
        &g_HWUI_debug_font_renderer,
        &g_HWUI_debug_defer,
        &g_HWUI_debug_display_list,
        &g_HWUI_debug_display_ops_as_events,
        &g_HWUI_debug_merge_behavior,
        &g_HWUI_debug_texture_tracker,
        &g_HWUI_debug_duration,
        &g_HWUI_debug_dumpDisplayList,
        &g_HWUI_debug_dumpDraw,
        &g_HWUI_debug_dumpTexture,
        &g_HWUI_debug_dumpAlphaTexture,
        &g_HWUI_debug_layer,
        &g_HWUI_debug_enhancement,
        &g_HWUI_debug_egl_trace
    };
    const char* properties[e_HWUI_DEBUG_END] = {
        "debug.hwui.log.opengl",
        "debug.hwui.log.ext",
        "debug.hwui.log.init",
        "debug.hwui.log.mem",
        "debug.hwui.log.cache_flush",
        "debug.hwui.log.layersAsRegions",
        "debug.hwui.log.clip_regions",
        "debug.hwui.log.programs",
        "debug.hwui.log.layers",
        "debug.hwui.log.render_buffers",
        "debug.hwui.log.stencil",
        "debug.hwui.log.patches",
        "debug.hwui.log.patches_vtx",
        "debug.hwui.log.patchesEmptyVtx",
        "debug.hwui.log.paths",
        "debug.hwui.log.tex",
        "debug.hwui.log.layer_renderer",
        "debug.hwui.log.font_renderer",
        "debug.hwui.log.defer",
        "debug.hwui.log.displaylist",
        "debug.hwui.log.display_events",
        "debug.hwui.log.merge_behavior",

        "debug.hwui.log.texture_tracker",    // log gl textures' life
        "debug.hwui.log.duration",           // sync with DisplayListLogBuffer
        "debug.hwui.dump.displaylist",       // dump rendering result per frame
        "debug.hwui.dump.draw",              // dump rendering result per draw operation
        "debug.hwui.dump.tex",               // dump texture returned from textureCache
        "debug.hwui.dump.fonttex",           // dump texture for fonts, aka g_HWUI_debug_dumpAlphaTexture
        "debug.hwui.dump.layer",             // dump layer, the result of fbo
        "debug.hwui.enhancement",            // mtk enhancements
        "debug.egl.trace"                    // sync with DevelopmentSettings
    };
    char value[PROPERTY_VALUE_MAX];
    uint32_t flags;

    if (enable) {
        for (uint32_t i = 0; i < e_HWUI_DEBUG_END; i++) {
            //ALOGD("getDebugLog: %s %d/%d", properties[i], i, e_HWUI_DEBUG_END);
            property_get(properties[i], value, "");
            if (value[0] != '\0') {
                ALOGD("setDebugLog: %s=%s", properties[i], value);
                //must check "1" because egl_trace property is systrace/error/1
                *pDebugArray[i] = (strcmp(value, "1") == 0) ? 1 : 0;
            }
        }
    } else {
        for (uint32_t i = 0; i < e_HWUI_DEBUG_END; i++) {
            *pDebugArray[i] = 0;
        }
    }
}

#if defined(MTK_DEBUG_RENDERER)

bool dumpDisplayList(int width, int height, int level);
bool dumpDraw(int width, int height, bool newFrame);
bool dumpTexture(int texture, int width, int height, SkBitmap *bitmap);
bool dumpAlphaTexture(int width, int height, uint8_t *data, const char *prefix, SkBitmap::Config format);
bool dumpLayer(int width, int height, int fbo);



namespace android {
namespace uirenderer {

class TextureTracker: public Singleton<TextureTracker> {
    TextureTracker();
    ~TextureTracker();

    friend class Singleton<TextureTracker>;

public:
    void startMark(String8 name);
    void endMark();
    void add(int textureId, int w, int h, int format, int type, String8 name, const char* comment = NULL);
    void add(String8 name, int textureId, int w, int h, int format, int type, String8 purpose, const char* comment = NULL);
    void remove(int textureId, const char* comment = NULL);
    void update(int textureId, bool ghost, String8 name = String8());
    static int estimateMemory(int w, int h, int format, int type);

    void dumpMemoryUsage(String8 &log);

    int load3dUsage();

private:
    struct TextureEntry {
       TextureEntry():
           mName(String8("none")), mId(0), mWidth(0), mHeight(0), mFormat(0), mType(0), mMemory(0), mGhost(true), mPurpose(String8("none")) {
       }

       TextureEntry(String8 name, int id, int w, int h, int format, int type, String8 purpose):
           mName(name), mId(id), mWidth(w), mHeight(h), mFormat(format), mType(type), mGhost(false), mPurpose(purpose) {
           mMemory = TextureTracker::estimateMemory(w, h, format, type);
       }

       TextureEntry(int id):
           mName(String8("none")), mId(id), mWidth(0), mHeight(0), mFormat(0), mType(0),mMemory(0), mGhost(true), mPurpose(String8("none")) {
       }

       bool operator<(const TextureEntry& rhs) const {
           return mId < rhs.mId;
       }

       bool operator==(const TextureEntry& rhs) const {
           return mId == rhs.mId;
       }

       String8 mName;
       int mId;
       int mWidth;
       int mHeight;
       int mFormat;
       int mType;
       int mMemory;
       bool mGhost;
       String8 mPurpose;


   }; // struct TextureEntry

   Vector<String8> mViews;
   int mPid;
   SortedList<TextureEntry> mMemoryList;
   mutable Mutex mLock;


};

}; // namespace uirenderer
}; // namespace android

#endif

#endif // MTK_HWUI_DEBUG_H
