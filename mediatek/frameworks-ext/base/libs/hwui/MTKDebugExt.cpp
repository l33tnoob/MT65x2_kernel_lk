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

#include "MTKDebug.h"

int g_HWUI_debug_opengl = 1;
int g_HWUI_debug_extensions = 0;
int g_HWUI_debug_init = 0;
int g_HWUI_debug_memory_usage = 0;
int g_HWUI_debug_cache_flush = 1;
int g_HWUI_debug_layers_as_regions = 0;
int g_HWUI_debug_clip_regions = 0;
int g_HWUI_debug_programs = 0;
int g_HWUI_debug_layers = 0;
int g_HWUI_debug_render_buffers = 0;
int g_HWUI_debug_stencil = 0;
int g_HWUI_debug_patches = 0;
int g_HWUI_debug_patches_vertices = 0;
int g_HWUI_debug_patches_empty_vertices = 0;
int g_HWUI_debug_paths = 0;
int g_HWUI_debug_textures = 0;
int g_HWUI_debug_layer_renderer = 0;
int g_HWUI_debug_font_renderer = 0;
int g_HWUI_debug_defer = 0;
int g_HWUI_debug_display_list = 0;
int g_HWUI_debug_display_ops_as_events = 0;
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

bool dumpDisplayList(int width, int height, int level)
{
    return true;
}

bool dumpDraw(int width, int height, bool newFrame)
{
    return true;
}

bool dumpTexture(int texture, int width, int height, SkBitmap *bitmap)
{
    return true;
}

bool dumpAlphaTexture(int width, int height, uint8_t *data, const char *prefix, SkBitmap::Config format)
{
    return true;
}

bool dumpLayer(int width, int height, int fbo)
{
    return true;
}

