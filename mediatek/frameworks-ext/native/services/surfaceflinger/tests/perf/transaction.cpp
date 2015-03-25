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


#include <cutils/memory.h>
#include <math.h>

#include "transaction.h"


namespace android {

TransactionTest::TransactionTest(const sp<SurfaceComposerClient>& client,
                 const struct TestParamInput& input) :
    mIndex(input.index),
    mLayerCount(input.layerCount),
    mDispWidth(input.dispWidth),
    mDispHeight(input.dispHeight) {
    mSurfaceControl = client->createSurface(
                String8::format("test-surface-%d", mIndex),
                mDispWidth, mDispHeight, PIXEL_FORMAT_RGBX_8888);
    mSurface = mSurfaceControl->getSurface();
}

TransactionTest::~TransactionTest() {

}


SizeTest::SizeTest(const sp<SurfaceComposerClient>& client,
           const struct TestParamInput& input) :
    TransactionTest(client, input),
    mSteps(30),
    mBufWidth(128),
    mBufHeight(128),
    mConnected(false) {
    if (mIndex > 0) {
        mMaxWidth = (mDispWidth / 2) + (mDispWidth / 2 * ((mLayerCount-1) - mIndex) / (mLayerCount-1));
        mMaxHeight = (mDispHeight / 2) + (mDispHeight / 2 * ((mLayerCount-1) - mIndex) / (mLayerCount-1));
    } else {
        /* index=0, bottom layer */
        mMaxWidth = mDispWidth;
        mMaxHeight = mDispHeight;
    }

    mWidth = mMaxWidth;
    mHeight = mMaxHeight;

    mCurrStep = mSteps;
    mDirection = SHRINK;
}

SizeTest::~SizeTest() {
    if (mConnected) {
        ANativeWindow *window = mSurface.get();

        /* disconnect as unregister */
        native_window_api_disconnect(window, NATIVE_WINDOW_API_CPU);
    }
}

int SizeTest::Init() {
    /* config surface attrbution */
    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(START_LAYER_ORDER + mIndex);
    mSurfaceControl->setSize(mWidth, mHeight);
    mSurfaceControl->setPosition(0.0, 0.0);
    mSurfaceControl->setMatrix(1.0, 0.0, 0.0, 1.0);
    mSurfaceControl->setAlpha(1.0);
    SurfaceComposerClient::closeGlobalTransaction();

    /* start to config window buffer */
    ANativeWindow *window = mSurface.get();

    /* set api connection type as register */
    native_window_api_connect(window, NATIVE_WINDOW_API_CPU);

    /* set buffer size */
    native_window_set_buffers_dimensions(window, mBufWidth, mBufHeight);

    /* set format */
    native_window_set_buffers_format(window, HAL_PIXEL_FORMAT_RGBX_8888);

    /* set usage software write-able and hardware texture bind-able */
    native_window_set_usage(window, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    /* set scaling to match window display size */
    native_window_set_scaling_mode(window, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    /* alloc buffer and fill it */
    ANativeWindowBuffer *buf;
    sp<GraphicBuffer> gb;
    const Rect rect(mBufWidth, mBufHeight);
    void *ptr;

    int err;
    int fenceFd = -1;
    err = window->dequeueBuffer(window, &buf, &fenceFd);
    if (err != NO_ERROR) {
        printf("dequeue buffer failed for surface%d\n", mIndex);
        native_window_api_disconnect(window, NATIVE_WINDOW_API_CPU);
        return err;
    }
    sp<Fence> fence(new Fence(fenceFd));
    fence->wait(Fence::TIMEOUT_NEVER);

    gb = new GraphicBuffer(buf, false);
    gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
    uint32_t *pbuf = (uint32_t *)ptr;
    uint32_t c = (mLayerCount - mIndex) * 0xFF / mLayerCount;
    uint32_t color = 0xFF000000 | (c << 16) | (c << 8) | c; /* gray */
    for (int y = 0; y < mBufHeight; y++) {
        if ((y > 0) && (y < (mBufHeight-1))) {
            for (int x = 0; x < mBufWidth; x++) {
                if ((x > 0) && (x < (mBufWidth-1))) {
                    *pbuf = color;
                } else {
                    *pbuf = 0xFF0000FF; /* red edge */
                }
                pbuf++;
            }
        } else {
            android_memset32(pbuf, 0xFF0000FF, mBufWidth*4);
            pbuf += mBufWidth;
        }
    }
    gb->unlock();

    err = window->queueBuffer(window, buf, -1);
    if (err != NO_ERROR) {
        printf("queue buffer failed for surface%d\n", mIndex);
        native_window_api_disconnect(window, NATIVE_WINDOW_API_CPU);
        return err;
    }

    mConnected = true;

    return OK;
}

void SizeTest::Test() {
    if (mDirection == ENLARGE) {
        mCurrStep++;
        if (mCurrStep >= mSteps)
            mDirection = SHRINK;
    } else { /* SHRINK */
        mCurrStep--;
        if (mCurrStep <= 0)
            mDirection = ENLARGE;
    }

    mWidth = (mMaxWidth / 2) + (mMaxWidth / 2 * mCurrStep / mSteps);
    mHeight = (mMaxHeight / 2) + (mMaxHeight / 2 * mCurrStep / mSteps);

    /* resize */
    mSurfaceControl->setSize(mWidth, mHeight);
}


PositionTest::PositionTest(const sp<SurfaceComposerClient>& client,
               const struct TestParamInput& input) :
    TransactionTest(client, input),
    mSteps(30) {
    mWidth = (mDispWidth / 2) + (mDispWidth / 2 * ((mLayerCount-1) - mIndex) / mLayerCount);
    mHeight = (mDispHeight / 2) + (mDispHeight / 2 * ((mLayerCount-1) - mIndex) / mLayerCount);

    mPositionRight = mDispWidth - mWidth;
    mPositionBottom = mDispHeight - mHeight;

    mCurrX = 0;
    mCurrY = 0;
        
    mCurrStep = 0;
    mDirection = DIR_RIGHT; /* move to light */
}

PositionTest::~PositionTest() {

}

int PositionTest::Init() {
    /* config surface attrbution */
    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(START_LAYER_ORDER + mIndex);
    mSurfaceControl->setSize(mWidth, mHeight);
    mSurfaceControl->setPosition(mCurrX * 1.0, mCurrY * 1.0);
    mSurfaceControl->setMatrix(1.0, 0.0, 0.0, 1.0);
    mSurfaceControl->setAlpha(1.0);
    SurfaceComposerClient::closeGlobalTransaction();

    ANativeWindow_Buffer anwb;
    mSurface->lock(&anwb, NULL);

    /* fill the buffer of surface */
    uint32_t *pbuf = (uint32_t *)anwb.bits;
    uint32_t c = (mLayerCount - mIndex) * 0xFF / mLayerCount;
    uint32_t color = 0xFF000000 | (c << 16) | (c << 8) | c; /* gray */
    for (int32_t y = 0; y < anwb.height; y++) {
        if ((y > 2) && (y < ((anwb.height - 1) - 2))) {
            for (int32_t x = 0; x < anwb.stride; x++) {
                if ((x > 2) && (x < ((anwb.width - 1) - 2))) {
                    *pbuf = color;
                } else {
                    *pbuf = 0xFF0000FF; /* red edge */
                }
                pbuf++;
            }
        } else {
            android_memset32(pbuf, 0xFF0000FF, anwb.stride * 4);
            pbuf += anwb.stride;
        }
    }

    mSurface->unlockAndPost();

    return 0;
}

void PositionTest::Test() {
    if (mDirection == DIR_RIGHT) {
        mCurrStep++;
        /* re-calculate position x */
        mCurrX = mPositionRight * mCurrStep / mSteps;
        if (mCurrStep >= mSteps) {
            mCurrStep = 0;
            mDirection = DIR_DOWN;
        }
    } else if (mDirection == DIR_DOWN) {
        mCurrStep++;
        /* re-calculate position y */
        mCurrY = mPositionBottom * mCurrStep / mSteps;
        if (mCurrStep >= mSteps) {
            mCurrStep = mSteps;
            mDirection = DIR_LEFT;
        }
    } else if (mDirection == DIR_LEFT) {
        mCurrStep--;
        /* re-calculate position x */
        mCurrX = mPositionRight * mCurrStep / mSteps;
        if (mCurrStep <= 0) {
            mCurrStep = mSteps;
            mDirection = DIR_UP;
        }
    } else { /* DIR_UP */
        mCurrStep--;
        /* re-calculate position y */
        mCurrY = mPositionBottom * mCurrStep / mSteps;
        if (mCurrStep <= 0) {
            mCurrStep = 0;
            mDirection = DIR_RIGHT;
        }
    }

    /* set new position */
    mSurfaceControl->setPosition(mCurrX * 1.0, mCurrY * 1.0);
}


MatrixTest::MatrixTest(const sp<SurfaceComposerClient>& client,
               const struct TestParamInput& input) :
    TransactionTest(client, input),
    mMaxAngle(45.0),
    mSteps(30) {
    if (mIndex > 0) {
        mWidth = (mDispWidth / 2) + (mDispWidth / 2 * ((mLayerCount-1) - mIndex) / (mLayerCount-1));
        mHeight = (mDispHeight / 2) + (mDispHeight / 2 * ((mLayerCount-1) - mIndex) / (mLayerCount-1));
    } else {
        /* index=0, bottom layer */
        mWidth = mDispWidth;
        mHeight = mDispHeight;
    }

    mAngle = 0.0;
    mCurrStep = 0;

    if ((mIndex % 2) == 0)
        mDirection = CLOCKWISE;
    else
        mDirection = COUNTER_CLOCKWISE;
}

MatrixTest::~MatrixTest() {
 
}

int MatrixTest::Init() {
    /* config surface attrbution */
    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(START_LAYER_ORDER + mIndex);
    mSurfaceControl->setSize(mWidth, mHeight);
    mSurfaceControl->setPosition(0.0, 0.0);
    mSurfaceControl->setMatrix(1.0, 0.0, 0.0, 1.0);
    mSurfaceControl->setAlpha(1.0);
    SurfaceComposerClient::closeGlobalTransaction();

    ANativeWindow_Buffer anwb;
    mSurface->lock(&anwb, NULL);

    /* fill the buffer of surface */
    uint32_t *pbuf = (uint32_t *)anwb.bits;
    uint32_t c = (mLayerCount - mIndex) * 0xFF / mLayerCount;
    uint32_t color = 0xFF000000 | (c << 16) | (c << 8) | c; /* gray */
    for (int32_t y = 0; y < anwb.height; y++) {
        if ((y > 2) && (y < ((anwb.height - 1) - 2))) {
            for (int32_t x = 0; x < anwb.stride; x++) {
                if ((x > 2) && (x < ((anwb.width - 1) - 2))) {
                    *pbuf = color;
                } else {
                    *pbuf = 0xFF0000FF; /* red edge*/
                }
                pbuf++;
            }
        } else {
            android_memset32(pbuf, 0xFF0000FF, anwb.stride * 4);
            pbuf += anwb.stride;
        }
    }

    mSurface->unlockAndPost();

    return 0;
}

void MatrixTest::Test() {
    if (mDirection == CLOCKWISE) {
        mCurrStep++;
        if (mCurrStep >= mSteps)
            mDirection = COUNTER_CLOCKWISE;
    } else { /* COUNTER_CLOCKWISE */
        mCurrStep--;
        if (mCurrStep <= (-1 * mSteps))
            mDirection = CLOCKWISE;
    }

    /* calculate angle */
    mAngle = mMaxAngle * mCurrStep / mSteps;

    /* convert angle to radian */
    float radian = mAngle * M_PI / 180.0;

    float dsdx = cosf(radian);
    float dtdx = sinf(radian);
    float dsdy = -1.0 * dtdx;
    float dtdy = dsdx;

    /* set martix for rotation */
    mSurfaceControl->setMatrix(dsdx, dtdx, dsdy, dtdy);
}


AlphaTest::AlphaTest(const sp<SurfaceComposerClient>& client,
             const struct TestParamInput& input) :
    TransactionTest(client, input),
    mSteps(30) {
    if (mLayerCount > 1) {
        mWidth = (mDispWidth / 2) + (mDispWidth / 2 * mIndex / (mLayerCount-1));
        mHeight = (mDispHeight / 2) + (mDispHeight / 2 * mIndex / (mLayerCount-1));
    } else {
        /* only one surface */
        mWidth = mDispWidth;
        mHeight = mDispHeight;
    }

    mLeft = (mDispWidth - mWidth) / 2;
    mTop = (mDispHeight - mHeight) / 2;

    mAlpha = 1.0;
    mCurrStep = mSteps;

    mDirection = TRANSPARENT;
}

AlphaTest::~AlphaTest() {

}

int AlphaTest::Init() {
    /* config surface attrbution */
    SurfaceComposerClient::openGlobalTransaction();
    mSurfaceControl->setLayer(START_LAYER_ORDER + mIndex);
    mSurfaceControl->setSize(mWidth, mHeight);
    mSurfaceControl->setPosition(mLeft * 1.0, mTop * 1.0);
    mSurfaceControl->setMatrix(1.0, 0.0, 0.0, 1.0);
    mSurfaceControl->setAlpha(mAlpha);
    SurfaceComposerClient::closeGlobalTransaction();

    ANativeWindow_Buffer anwb;
    mSurface->lock(&anwb, NULL);

    /* fill the buffer of surface */
    uint32_t *pbuf = (uint32_t *)anwb.bits;
    uint32_t c = (mLayerCount - mIndex) * 0xFF / mLayerCount;
    uint32_t color = 0xFF000000 | (c << 16) | (c << 8) | c; /* gray */
    for (int32_t y = 0; y < anwb.height; y++) {
        if ((y > 2) && (y < ((anwb.height - 1) - 2))) {
            for (int32_t x = 0; x < anwb.stride; x++) {
                if ((x > 2) && (x < ((anwb.width - 1) - 2))) {
                    *pbuf = color;
                } else {
                    *pbuf = 0xFF0000FF; /* red edge */
                }
                pbuf++;
            }
        } else {
            android_memset32(pbuf, 0xFF0000FF, anwb.stride * 4);
            pbuf += anwb.stride;
        }
    }

    mSurface->unlockAndPost();

    return 0;
}

void AlphaTest::Test() {
    if (mDirection == OPAQUE) {
        mCurrStep++;
        if (mCurrStep >= mSteps)
            mDirection = TRANSPARENT;
    } else { /* TRANSPARENT */
        mCurrStep--;
        if (mCurrStep <= 0)
            mDirection = OPAQUE;
    }

    mAlpha = mCurrStep * 1.0 / mSteps;

    /*
     C: layer count
     I: layer index
     A: original alpha
      C-(I+1)     I+1             C - (I+1) + (I+1)xA         C - (1-A)x(I+1)             (1-A)x(I+1)
     --------- + ----- x A  ==>  ---------------------  ==>  -----------------  ==>  1 - -------------
         C         C                        C                         C                        C
    */
    mAlpha = 1.0 - ((1.0 - mAlpha) * (mIndex + 1) / mLayerCount);

    mSurfaceControl->setAlpha(mAlpha);
}

} // namespace android
