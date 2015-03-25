/*
 * Copyright (C) 2011 The Android Open Source Project
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
 *
 */

/*
 * Hardware Composer Color Equivalence
 *
 * Synopsis
 *   hwc_colorequiv [options] eFmt
 *
 *     options:
         -v - verbose
 *       -s <0.##, 0.##, 0.##> - Start color (default: <0.0, 0.0, 0.0>
 *       -e <0.##, 0.##, 0.##> - Ending color (default: <1.0, 1.0, 1.0>
 *       -r fmt - reference graphic format
 *       -D #.## - End of test delay
 *
 *     graphic formats:
 *       RGBA8888 (reference frame default)
 *       RGBX8888
 *       RGB888
 *       RGB565
 *       BGRA8888
 *       RGBA5551
 *       RGBA4444
 *       YV12
 *
 * Description
 *   Renders a horizontal blend in two frames.  The first frame is rendered
 *   in the upper third of the display and is called the reference frame.
 *   The second frame is displayed in the middle third and is called the
 *   equivalence frame.  The primary purpose of this utility is to verify
 *   that the colors produced in the reference and equivalence frames are
 *   the same.  The colors are the same when the colors are the same
 *   vertically between the reference and equivalence frames.
 *
 *   By default the reference frame is rendered through the use of the
 *   RGBA8888 graphic format.  The -r option can be used to specify a
 *   non-default reference frame graphic format.  The graphic format of
 *   the equivalence frame is determined by a single required positional
 *   parameter.  Intentionally there is no default for the graphic format
 *   of the equivalence frame.
 *
 *   The horizontal blend in the reference frame is produced from a linear
 *   interpolation from a start color (default: <0.0, 0.0, 0.0> on the left
 *   side to an end color (default <1.0, 1.0, 1.0> on the right side.  Where
 *   possible the equivalence frame is rendered with the equivalent color
 *   from the reference frame.  A color of black is used in the equivalence
 *   frame for cases where an equivalent color does not exist.
 */

#include <algorithm>
#include <assert.h>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <libgen.h>
#include <sched.h>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <list>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <ui/FramebufferNativeWindow.h>
#include <ui/GraphicBuffer.h>

#define LOG_TAG "hwcColorEquivTest"
#include <utils/Log.h>
#include <testUtil.h>

#include <hardware/hwcomposer.h>

#include "surfaceTestLib.h"
#include <SkImageEncoder.h>
#include <SkBitmap.h>


using namespace std;
using namespace android;


#define CAPTURE_BY_LCD


static char cmds[256];

#ifdef CAPTURE_BY_LCD
char golden_name[128]={"_gold.bgra"};
char data_name[128]={".bgra"};
#else
char golden_name[128]={"_gold.rgba"};
char data_name[128]={".rgba"};
#endif

bool doCheck(uint32_t size, char *cmd_path) {
    bool result = true;

    //printf("file size:%d\n", size);

    uint8_t *gold = NULL, *sample = NULL;
    uint32_t gold_width, gold_height;
    uint32_t sample_width, sample_height;
    FILE *file = NULL;

    strcpy(cmds, cmd_path);
    strcat(cmds, golden_name);

    printf("gold file:%s\n", cmds);

    file = fopen(cmds, "rb");

    if(NULL != file) {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        gold = (uint8_t *)malloc(sizeof(uint8_t) * size);
        fread(gold, size, 1, file);
        uint32_t *temp = (uint32_t*)gold;
        gold_width = temp[0];
        gold_height = temp[1];
        fclose(file);
    } else {
        printf("file open fail:%s\n", strerror(errno));
        return false;
    }

    printf("read gold pattern done, width = %d, height = %d\n",gold_width, gold_height);

    strcpy(cmds, cmd_path);
    strcat(cmds, data_name);

    printf("output path:%s\n", cmds);

    file = fopen(cmds, "rb");

    if(NULL != file) {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        sample = (uint8_t *)malloc(sizeof(uint8_t) * size);
        fread(sample, size, 1, file);
        uint32_t *temp = (uint32_t*)sample;
        sample_width = temp[0];
        sample_height = temp[1];
        fclose(file);
    } else {
        printf("file open fail:%s\n", strerror(errno));
        free(gold);
        return false;
    }

    printf("read output pattern done, width = %d, height = %d\n",sample_width, sample_height);

    if(sample_width != 0 && sample_height != 0)
    {
        uint32_t cmp_width = sample_width < gold_width ? sample_width : gold_width;
        uint32_t cmp_height = sample_height < gold_height ? sample_height : gold_height;
        uint8_t *f1 = gold + 12, *f2 = sample + 12;

        for(uint32_t i=0; i < cmp_height; i++)
        {
            if(memcmp(f1,f2,cmp_width*4)!=0)
            {
                result = false;
                break;
            }
            f1 += gold_width * 4;
            f2 += sample_width * 4;
        }
    }
    else
    {
        result = false;
    }

    free(gold);
    free(sample);

    return result;
}

bool captureScreen(bool saveGold, char *goldenPath, char *cmd_path)
{
#ifdef CAPTURE_BY_LCD
    sprintf(cmds, "system/bin/lcdc_screen_cap > ");
    if(goldenPath)
    {
        strcat(cmds, goldenPath);
        printf("lcd capture to %s \n", goldenPath);
    }
    else
    {
        strcat(cmds, cmd_path);
        if(saveGold)
            strcat(cmds, golden_name);
        else
            strcat(cmds, data_name);
       
        printf("lcd capture to %s ...\n", cmd_path);
    }

    system(cmds);

    printf("cmds %s \n", cmds);

    if(saveGold)
    {        
        FILE *file = NULL;
        uint8_t *gold = NULL;
        
        strcpy(cmds, cmd_path);
        strcat(cmds, golden_name);

        file = fopen(cmds, "rb");
        
        if(NULL != file) {
            fseek(file, 0, SEEK_END);
            int size = ftell(file);
            fseek(file, 0, SEEK_SET);
            gold = (uint8_t *)malloc(sizeof(uint8_t) * size);
            fread(gold, size, 1, file);
            uint32_t *temp = (uint32_t*)gold;
            uint32_t gold_width = temp[0];
            uint32_t gold_height = temp[1];
            fclose(file);
            
            SkBitmap b;
            strcpy(cmds, cmd_path);
            strcat(cmds, "_gold.png"); 
            printf("saving file as PNG at %s , width=%d, height=%d\n", cmds, gold_width, gold_height);
            b.setConfig(SkBitmap::kARGB_8888_Config, gold_width, gold_height);
            b.setPixels(gold+12);
{//temp solution
            char *ptemp = (char*)gold+15;
            for(int i=0;i<gold_width * gold_height;i++)
            {
                *ptemp=0xFF;
                ptemp+=4;
            }
}
            SkImageEncoder::EncodeFile(cmds, b,
                    SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);

            free(gold);
        } else {
            printf("file open fail:%s\n", strerror(errno));
        }

    }
#else

    const String16 name("SurfaceFlinger");
    sp<ISurfaceComposer> composer;
    getService(name, &composer);

    sp<IMemoryHeap> heap;
    uint32_t w, h;
    PixelFormat f;
    //char cmds[256];
    status_t err = composer->captureScreen(0, &heap, &w, &h, &f, 0, 0, 0,2500000);
    if (err != NO_ERROR) {
        fprintf(stderr, "screen capture failed: %s\n", strerror(-err));
        return false;
    }

    printf("screen capture success: w=%u, h=%u, format=%d, pixels=%p\n",
            w, h, f,heap->getBase());

    saveFile(heap->getBase(),w*h*4,saveGold);

    strcpy(cmds, cmd_path);
    strcat(cmds, ".png");   

    printf("saving file as PNG at %s ...\n", cmds);

    SkBitmap b;
    b.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    b.setPixels(heap->getBase());
    SkImageEncoder::EncodeFile(cmds, b,
            SkImageEncoder::kPNG_Type, SkImageEncoder::kDefaultQuality);


#endif

    return true;
}

#ifndef CAPTURE_BY_LCD

bool saveFile(void *pdata, uint32_t size, bool saveGold)
{
    FILE *file = NULL;
    //char cmds[256];
    strcpy(cmds, cmd_path);

    if(saveGold)
    {
        strcat(cmds, golden_name);
    }else    
    {
        strcat(cmds, data_name);            
    }

    printf("save raw file:%s\n", cmds);

    file = fopen(cmds, "wb");

    if(NULL != file) {
        fwrite(pdata, size, 1, file);
        fclose(file);
    } else {
        printf("file open fail:%s\n", strerror(errno));
        return false;
    }

    printf("write file done\n");   

    return true;
}
#endif
