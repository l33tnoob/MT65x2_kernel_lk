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

#include <SkImageDecoder.h>
#include <SkBitmap.h>

#include <cutils/xlog.h>


using namespace android;


status_t main(int argc, char** argv)
{
	printf("[Unit Test] SurfaceFlinger 3D display test !\n\n");

    sp<SurfaceComposerClient> client;    
    sp<SurfaceControl>        c;
    sp<Surface>               s;
    Surface::SurfaceInfo      i;
    SkBitmap                  sbs;
    SkBitmap                  tab;

    // ready the png image file
    if (false == SkImageDecoder::DecodeFile("/data/3D_SBS.png", &sbs) ||
        false == SkImageDecoder::DecodeFile("/data/3D_TAB.png", &tab)) {
        XLOGE("fail load file");
        return INVALID_OPERATION;
    }

    // create layer env
    client = new SurfaceComposerClient();
    printf("screen (w, h) = (%d, %d)\n\n",
        (int)client->getDisplayWidth(0), (int)client->getDisplayHeight(0));



    // test set to side by side mode, and pull to topest layer in transaction
    printf("*** side by side test ...\n");

    c = client->createSurface(String8("test-S3D_image"), 0, sbs.width(), sbs.height(), PIXEL_FORMAT_RGBA_8888);
    client->openGlobalTransaction();
    {
        c->setLayer(200000);
        c->setPosition(
            (client->getDisplayWidth(0) - sbs.width()) / 2,
            (client->getDisplayHeight(0) - sbs.height()) / 2);
    }
	client->closeGlobalTransaction();

    printf("    set to SBS mode\n");
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayerSideBySide, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s = c->getSurface();
    s->lock(&i);
    {
        memcpy(i.bits, sbs.getPixels(), sbs.width() * sbs.height() * sbs.bytesPerPixel());
    }
    s->unlockAndPost();

    sleep(5);

    printf("    set back to 2D mode\n");
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayer2D, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s = c->getSurface();
    s->lock(&i);
    {
        memcpy(i.bits, sbs.getPixels(), sbs.width() * sbs.height() * sbs.bytesPerPixel());
    }
    s->unlockAndPost();

    sleep(3);




    // test set to side by side mode, and pull to topest layer in transaction
    printf("*** top and bottom test ...\n");

    c = client->createSurface(String8("test-S3D_image"), 0, tab.width(), tab.height(), PIXEL_FORMAT_RGBA_8888);
    client->openGlobalTransaction();
    {
        c->setLayer(200000);
        c->setPosition(
            (client->getDisplayWidth(0) - tab.width()) / 2,
            (client->getDisplayHeight(0) - tab.height()) / 2);
    }
	client->closeGlobalTransaction();

    printf("    set to TAB mode\n");
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayerTopAndBottom, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s = c->getSurface();
    s->lock(&i);
    {
        memcpy(i.bits, tab.getPixels(), tab.width() * tab.height() * tab.bytesPerPixel());
    }
    s->unlockAndPost();
    
    sleep(5);

    printf("    set back to 2D mode\n");
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayer2D, ISurfaceComposer::eLayerS3DMask);
    }
	client->closeGlobalTransaction();

    s->lock(&i);
    {
        memcpy(i.bits, tab.getPixels(), tab.width() * tab.height() * tab.bytesPerPixel());
    }
    s->unlockAndPost();

    sleep(5);

    client->dispose();
    return NO_ERROR;
}

