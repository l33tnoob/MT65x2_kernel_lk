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


using namespace android;


status_t main(int argc, char** argv)
{
	printf("[Unit Test] SurfaceFlinger 3D display test !\n\n");

    sp<SurfaceComposerClient> client;    
    sp<SurfaceControl>        u;
    sp<SurfaceControl>        c;
    sp<Surface>               s;
    Surface::SurfaceInfo      i;
    SkBitmap                  sbs;
    SkBitmap                  cam;

    // ready the png image file
    if (false == SkImageDecoder::DecodeFile("/data/3D_SBS.png", &sbs) ||
        false == SkImageDecoder::DecodeFile("/data/camera.png", &cam)) {
        LOGE("fail load file");
        return INVALID_OPERATION;
    }

    // create layer env
    client = new SurfaceComposerClient();
    printf("screen (w, h) = (%d, %d)\n\n",
        (int)client->getDisplayWidth(0), (int)client->getDisplayHeight(0));

    // test set to side by side mode, and pull to topest layer in transaction
    printf("*** camera test ...\n");

    u = client->createSurface(String8("test-ui"), 0, 540, 960, PIXEL_FORMAT_BGRA_8888);
    c = client->createSurface(String8("test-camera"), 0, 540, 960, PIXEL_FORMAT_RGBX_8888);
    
    client->openGlobalTransaction();
    {
        u->setLayer(210000);
        c->setLayer(200000);
    }
	client->closeGlobalTransaction();

    s = u->getSurface();    // fill ui surface
    s->lock(&i);
    {
        memcpy(i.bits, cam.getPixels(), 544 * 960 * 4);
    }
    s->unlockAndPost();//*/


    ANativeWindow       *w;                                        // fill camera surface
    ANativeWindowBuffer *buf;
    void                *ptr;
    const Rect          rect(960, 540);

    s = c->getSurface();
    w = s.get();
    native_window_api_connect(w, NATIVE_WINDOW_API_CAMERA);
    native_window_set_buffers_dimensions(w, 960, 540);
    native_window_set_buffers_format(w, HAL_PIXEL_FORMAT_RGBX_8888);
    native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE | GRALLOC_USAGE_S3D_TOP_AND_BOTTOM);
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    w->dequeueBuffer(w, &buf);
    GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
    {
        memcpy(ptr, sbs.getPixels(), 960 * 540 * 4);
    }
    GraphicBufferMapper::getInstance().unlock(buf->handle);    // inlock to return buffer
    w->queueBuffer(w, buf);                                    // queue to display


    uint8_t j = 0xff;
    while (true) {
        client->openGlobalTransaction();
        {
            u->setAlpha(j--);
        }
	    client->closeGlobalTransaction();

        usleep(33 * 1000);
    }

    client->dispose();
    return NO_ERROR;
}


