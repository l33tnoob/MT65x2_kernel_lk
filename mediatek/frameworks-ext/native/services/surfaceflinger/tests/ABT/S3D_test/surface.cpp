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

#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

#include <ui/GraphicBufferMapper.h>

#include <SkImageDecoder.h>
#include <SkBitmap.h>
#include "../common/surfaceTestLib.h"

#define DRAW_FHD_W 1080
#define DRAW_FHD_H 1920

using namespace android;
   
sp<SurfaceControl>        u;//keep reference
sp<SurfaceControl>        c;//keep reference
sp<SurfaceControl>        background;//keep reference

static char cmd_path[128]={"/data/s3d_test"};
static char cmd_path1[128]={"/data/s3d_test1"};
static char cmd_path2[128]={"/data/s3d_test2"};

static void usage(const char* pname)
{
    fprintf(stderr,
            "usage: %s [-g] [FILENAME]\n"
            "   -g: save golden data\n"
            "If FILENAME is not given, the results will be saved to /data/s3d_test_gold.bgra.\n",
            pname
    );
}

void s3d_clear_surface(void)
{
    u.clear();
    c.clear();
}

status_t s3d_camera_test(void)
{
 	printf("[Unit Test] SurfaceFlinger 3D display test !\n\n");

    sp<SurfaceComposerClient> client;    
    //sp<SurfaceControl>        u;
    //sp<SurfaceControl>        c;
    sp<Surface>               s;
    Surface::SurfaceInfo      i;
    SkBitmap                  sbs;
    SkBitmap                  cam;

    // ready the png image file
    if (false == SkImageDecoder::DecodeFile("/data/3D_Camera_SBS.png", &sbs) ||
        false == SkImageDecoder::DecodeFile("/data/camera.png", &cam)) {
        printf("fail load file");
        return INVALID_OPERATION;
    }

    // create layer env
    client = new SurfaceComposerClient();
    printf("screen (w, h) = (%d, %d)\n\n",
        (int)client->getDisplayWidth(0), (int)client->getDisplayHeight(0));

    // test set to side by side mode, and pull to topest layer in transaction
    printf("*** camera test ...\n");

    u = client->createSurface(String8("test-ui"), 0, DRAW_FHD_W, DRAW_FHD_H, PIXEL_FORMAT_BGRA_8888);
    c = client->createSurface(String8("test-camera"), 0, DRAW_FHD_W, DRAW_FHD_H, PIXEL_FORMAT_RGBX_8888);
    
    client->openGlobalTransaction();
    {
        u->setLayer(210000);
        c->setLayer(200000);
    }
	client->closeGlobalTransaction();

    ANativeWindow       *w;                                        // fill camera surface
    ANativeWindowBuffer *buf;
    void                *ptr;
    const Rect          rect0(544, 960);
    const Rect          rect1(960, 540);

    s = u->getSurface();    // fill ui surface
    s->lock(&i);
    {        
        printf("lock ui, i.s=%d, i.h=%d\n",i.s, i.h);
        memset(i.bits, 0, i.s * i.h * 4);
        memcpy(i.bits, cam.getPixels(), 544 * 960 * 4);//buffer stride is bigger then ...
    }
    s->unlockAndPost();


    s = c->getSurface();
    w = s.get();
    native_window_api_connect(w, NATIVE_WINDOW_API_CAMERA);
    native_window_set_buffers_dimensions(w, 960, 540);
    native_window_set_buffers_format(w, HAL_PIXEL_FORMAT_RGBX_8888);
    native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_S3D_TOP_AND_BOTTOM);
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    w->dequeueBuffer(w, &buf);
    GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect1, &ptr);
    {
        memcpy(ptr, sbs.getPixels(), 960 * 540 * 4);
    }
    GraphicBufferMapper::getInstance().unlock(buf->handle);    // inlock to return buffer
    w->queueBuffer(w, buf);                                    // queue to display


    uint8_t j = 0x80;

    client->openGlobalTransaction();
    {
        u->setAlpha(j);
    }
    client->closeGlobalTransaction();

    sleep(1);

    client->dispose();
    return NO_ERROR;   
}


status_t s3d_image_sbs_test(void)
{
	printf("[Unit Test] SurfaceFlinger 3D display test !\n\n");

    sp<SurfaceComposerClient> client;    
    //sp<SurfaceControl>        c;
    sp<Surface>               s;
    Surface::SurfaceInfo      i;
    SkBitmap                  sbs;

    // ready the png image file
    if (false == SkImageDecoder::DecodeFile("/data/3D_SBS.png", &sbs)) {
        printf("fail load file");
        return INVALID_OPERATION;
    }

    // create layer env
    client = new SurfaceComposerClient();
    printf("screen (w, h) = (%d, %d)\n\n",
        (int)client->getDisplayWidth(0), (int)client->getDisplayHeight(0));



    // test set to side by side mode, and pull to topest layer in transaction
    printf("*** side by side test ...\n");
    c = client->createSurface(
            String8("test-S3D_background"),
            0,
            DRAW_FHD_W,
            DRAW_FHD_H,
            PIXEL_FORMAT_RGBA_8888,
            ISurfaceComposer::eFXSurfaceDim & ISurfaceComposer::eFXSurfaceMask);

    u = client->createSurface(String8("test-S3D_image"), 0, sbs.width(), sbs.height(), PIXEL_FORMAT_RGBA_8888);
    //printf("tempc weakcount = %d\n", tempc->getWeakRefs()->getWeakCount());
    //c = tempc;
    //printf("after asign, tempc weakcount = %d, c weakcount = %d\n", tempc->getWeakRefs()->getWeakCount(),c->getWeakRefs()->getWeakCount());
    client->openGlobalTransaction();
    {
        c->setLayer(200000);
        c->setPosition(0, 0);
        c->setAlpha(1.0f);    // black background        
        u->setLayer(210000);
        u->setPosition(0, 0);
    }
	client->closeGlobalTransaction();

    printf("    set to SBS mode\n");
    client->openGlobalTransaction();
    {
        u->setFlags(ISurfaceComposer::eLayerSideBySide, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s = u->getSurface();
    s->lock(&i);
    {
        memcpy(i.bits, sbs.getPixels(), sbs.width() * sbs.height() * sbs.bytesPerPixel());
    }
    s->unlockAndPost();

    sleep(1);

    client->dispose();
    return NO_ERROR;
}

status_t s3d_image_tnb_test(void)
{
	printf("[Unit Test] SurfaceFlinger 3D display test !\n\n");

    sp<SurfaceComposerClient> client;    
    //sp<SurfaceControl>        c;
    sp<Surface>               s;
    Surface::SurfaceInfo      i;
    SkBitmap                  tab;

    // ready the png image file
    if (false == SkImageDecoder::DecodeFile("/data/3D_TAB.png", &tab)) {
        printf("fail load file");
        return INVALID_OPERATION;
    }

    // create layer env
    client = new SurfaceComposerClient();

    printf("*** top and bottom test ...\n");
    c = client->createSurface(
            String8("test-S3D_background"),
            0,
            DRAW_FHD_W,
            DRAW_FHD_H,
            PIXEL_FORMAT_RGBA_8888,
            ISurfaceComposer::eFXSurfaceDim & ISurfaceComposer::eFXSurfaceMask);

    u = client->createSurface(String8("test-S3D_image1"), 0, tab.width(), tab.height(), PIXEL_FORMAT_RGBA_8888);
    client->openGlobalTransaction();
    {
        c->setLayer(200000);
        c->setPosition(0, 0);
        c->setAlpha(1.0f);    // black background           
        u->setLayer(240000);
        u->setPosition(0, 0);
    }
	client->closeGlobalTransaction();

    printf("    set to TAB mode\n");
    client->openGlobalTransaction();
    {
        u->setFlags(ISurfaceComposer::eLayerTopAndBottom, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s = u->getSurface();
    s->lock(&i);
    {
        memcpy(i.bits, tab.getPixels(), tab.width() * tab.height() * tab.bytesPerPixel());
    }
    s->unlockAndPost();

    sleep(1);
    
    client->dispose();
    return NO_ERROR;
}

status_t main(int argc, char** argv)
{
    status_t result = NO_ERROR;
    const char* pname = argv[0];
    int c;
    bool saveGolden = false;
    while ((c = getopt(argc, argv, "gh")) != -1) {
        switch (c) {
            case 'g':
                saveGolden = true;
                break;
            case '?':
            case 'h':
                usage(pname);
                return 1;
        }
    }
    printf("saveGolden =%d,argc=%d,optind=%d\n",saveGolden,argc,optind);
    argc -= optind;
    argv += optind;

    if((result = s3d_camera_test()) != NO_ERROR)
    {
        printf("test case fail\n");
        return result;
    }

    captureScreen(saveGolden, argv[0], cmd_path);
    if(!saveGolden)
    {
        bool check = doCheck(0, cmd_path);
        printf("check done\n\n");

        if(true != check) {
            printf("test case fail\n");
            return UNKNOWN_ERROR;
        }
    }

    s3d_clear_surface();
    
    if((result = s3d_image_sbs_test()) != NO_ERROR)
    {
        printf("test case fail\n");
        return result;
    }   

    captureScreen(saveGolden, argv[0], cmd_path1);
    if(!saveGolden)
    {
        bool check = doCheck(0, cmd_path1);
        printf("check done\n\n");

        if(true != check) {
            printf("test case fail\n");
            return UNKNOWN_ERROR;
        }
    }

    s3d_clear_surface();

    
    if((result = s3d_image_tnb_test()) != NO_ERROR)
    {
        printf("test case fail\n");
        return result;
    }   

    captureScreen(saveGolden, argv[0], cmd_path2);
    if(!saveGolden)
    {
        bool check = doCheck(0, cmd_path2);
        printf("check done\n\n");

        if(true != check) {
            printf("test case fail\n");
            return UNKNOWN_ERROR;
        }
    }

    s3d_clear_surface();

    printf("test case pass\n");

    return NO_ERROR;
}


