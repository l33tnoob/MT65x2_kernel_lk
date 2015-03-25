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

#define S3D_USAGE    (GRALLOC_USAGE_S3D_LR_SWAPPED|GRALLOC_USAGE_S3D_SIDE_BY_SIDE|GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_HW_TEXTURE)
//define S3D_USAGE    (GRALLOC_USAGE_S3D_TOP_AND_BOTTOM|GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_SW_READ_RARELY|GRALLOC_USAGE_HW_TEXTURE)
#define NORMAL_USAGE (GRALLOC_USAGE_SW_WRITE_OFTEN|GRALLOC_USAGE_HW_TEXTURE|GRALLOC_USAGE_SW_READ_RARELY)
#define DRAW_FRAME   300

using namespace android;


// global controls
sp<SurfaceComposerClient> client;    
sp<SurfaceControl>        c;
sp<Surface>               s;
ANativeWindow             *w;
int32_t                   sw;
int32_t                   sh;
int32_t                   casenum;

status_t showTestVideo() {
    const uint32_t fw = 864;
    const uint32_t fh = 320;
    const uint32_t fs = 864 * 320 * 3 / 2;

    ANativeWindowBuffer *buf;
    void                *ptr;
    const Rect          rect(fw, fh);
    uint32_t            i;
	FILE                *f;

    f = fopen("/data/S3D_video_300f.yuv", "rb");
    if (0 > f) {
        printf("!!! File open fail !!!\n\n");
        fclose(f);
        return INVALID_OPERATION;
	}

    // set surface for video play back
    native_window_api_connect(w, NATIVE_WINDOW_API_MEDIA);
    native_window_set_buffers_dimensions(w, fw, fh);
    native_window_set_buffers_format(w, HAL_PIXEL_FORMAT_I420);
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    //native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface S3D auto detection to S3D\n\n", casenum);
    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, S3D_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(40 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface S3D auto detection to 2D\n\n", casenum);
    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, NORMAL_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(40 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface:2D, Buffer:2D\n\n", casenum);
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayer2D, ISurfaceComposer::eLayerS3DMask);
    }
    client->closeGlobalTransaction();

    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, NORMAL_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(40 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface:3D, Buffer:2D\n\n", casenum);
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayerSideBySide, ISurfaceComposer::eLayerS3DMask);
    }
    client->closeGlobalTransaction();

    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, NORMAL_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(40 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface:2D, Buffer:3D\n\n", casenum);
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayer2D, ISurfaceComposer::eLayerS3DMask);
    }
    client->closeGlobalTransaction();

    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, S3D_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(33 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] Surface:3D, Buffer:3D\n\n",casenum);
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayerSideBySide, ISurfaceComposer::eLayerS3DMask);
    }
    client->closeGlobalTransaction();

    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, S3D_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(33 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] back to auto detection to 2D\n\n",casenum);
    client->openGlobalTransaction();
    {
        c->setFlags(ISurfaceComposer::eLayerUnknown, ISurfaceComposer::eLayerS3DMask);
    }
    client->closeGlobalTransaction();

    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, NORMAL_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(33 * 1000);
    }

    //----------------------------------------------------------------------------
    casenum += 1;
    printf("[case %d] back to auto detection to S3D\n\n",casenum);
    native_window_set_buffer_count(w, 8);
    native_window_set_usage(w, S3D_USAGE);
    for(i = 0; i < DRAW_FRAME; i++) {
        w->dequeueBuffer(w, &buf);
        GraphicBufferMapper::getInstance().lock(buf->handle, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, &ptr);
        {
    	    rewind(f);
	    	fseek(f, i * fs, SEEK_SET);
		    fread(ptr, fs, 1, f);
        }
        GraphicBufferMapper::getInstance().unlock(buf->handle);
        w->queueBuffer(w, buf);

	    usleep(33 * 1000);
    }

	/*printf("[Unit Test] Post video buffer with 180 rotation !\n\n");
    client->openTransaction();
	client->setOrientation(0, ISurfaceComposer::eOrientation180, 0);
    client->closeTransaction();
    for(i=0; i<300; i++)
    {
    	// rewind image file
		rewind(fp_3d);
		fseek(fp_3d, i * size/2, SEEK_SET);
		// read out image file to store in PMEM
		fread(heapPmem->getBase() + (i%2) * size/2, size/2, 1, fp_3d);
		// post to SF
		isurface->postBuffer((i%2) * size/2);	
	    usleep(33*1000);
    }

	printf("[Unit Test] Set Surface Size 960 x 540 !\n\n");
    client->openTransaction();
	surfaceCtl->setSize(960, 540);
    client->closeTransaction();


	printf("[Unit Test] Post video buffer with 90 rotation !\n\n");
    client->openTransaction();
	client->setOrientation(0, ISurfaceComposer::eOrientation90, 0);
    client->closeTransaction();
    for(i=0; i<300; i++)
    {
    	// rewind image file
		rewind(fp_3d);
		fseek(fp_3d, i * size/2, SEEK_SET);
		// read out image file to store in PMEM
		fread(heapPmem->getBase() + (i%2) * size/2, size/2, 1, fp_3d);
		// post to SF
		isurface->postBuffer((i%2) * size/2);	
	    usleep(33*1000);
    }


	printf("[Unit Test] Post video buffer with 270 rotation !\n\n");
    client->openTransaction();
	client->setOrientation(0, ISurfaceComposer::eOrientation270, 0);
    client->closeTransaction();
    for(i=0; i<300; i++)
    {
    	// rewind image file
		rewind(fp_3d);
		fseek(fp_3d, i * size/2, SEEK_SET);
		// read out image file to store in PMEM
		fread(heapPmem->getBase() + (i%2) * size/2, size/2, 1, fp_3d);
		// post to SF
		isurface->postBuffer((i%2) * size/2);	
	    usleep(33*1000);
    }



	//*/
    fclose(f);
    return NO_ERROR;
}


status_t main(int argc, char** argv) {
    printf("[Unit Test] SurfaceFlinger S3D video test !\n\n");

    client = new SurfaceComposerClient();
    sw = (int)client->getDisplayWidth(0);
    sh = (int)client->getDisplayHeight(0);
    printf("screen (w, h) = (%d, %d)\n\n", sw, sh);
    
    c = client->createSurface(String8("test-S3D_video"), 0, 340, 560, PIXEL_FORMAT_RGB_565);
    s = c->getSurface();
    w = s.get();

    client->openGlobalTransaction();
    {
        c->setLayer(200000);
        c->setPosition(100, 200);
    }
    client->closeGlobalTransaction();

    casenum = 0;
    showTestVideo();

    client->dispose();
    return NO_ERROR;
}
