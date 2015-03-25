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

#include "TransformTest.h"

float tmpFloats[9];

static void __createRotationMatrix(int rotation, int width, int height,
        SkMatrix &outMatrix) {
    // set the position of the image to the middle of the screen
    outMatrix.setRotate(rotation, width / 2, height / 2);
    outMatrix.postTranslate((DISPLAY_WIDTH - width) / 2,
            (DISPLAY_HEIGHT - height) / 2);
}


static void setSnapshotTransform(sp<SurfaceControl>& surfaceControl, SkMatrix &matrix, float alpha) {
    if (surfaceControl != NULL) {
#ifdef SK_SCALAR_IS_FIXED
        for (int i = 0; i < 6; i++) {
            tmpFloats[i] = SkFixedToFloat(matrix.get(i));
        }
        for (int j = 6; j < 9; j++) {
            tmpFloats[j] = SkFractToFloat(matrix.get(j));
        }
#else
        for (int i = 0; i < 9; i++) {
            tmpFloats[i] = matrix.get(i);
        }
#endif
        surfaceControl->setPosition(tmpFloats[MTRANS_X], tmpFloats[MTRANS_Y]);
        surfaceControl->setMatrix(tmpFloats[MSCALE_X], tmpFloats[MSKEW_Y],
                tmpFloats[MSKEW_X], tmpFloats[MSCALE_Y]);
//        LOGD("(trans_x,trans_y,scale_x,scale_y)=(%f,%f,%f,%f)",
//                tmpFloats[MSCALE_X], tmpFloats[MSKEW_Y],
//                tmpFloats[MSKEW_X], tmpFloats[MSCALE_Y]);
        surfaceControl->setAlpha(alpha);
    }
}


static void setRotation(sp<SurfaceControl>& surfaceControl, int degree)
{

    SkMatrix *snapshotMatrix = new SkMatrix();
    // computre rotation matrix
    __createRotationMatrix(degree, LAYER_WIDTH, LAYER_HEIGHT, *snapshotMatrix);
    // set surface with rotation matrix
    setSnapshotTransform(surfaceControl, *snapshotMatrix, 1.0f);

    delete snapshotMatrix;
}


status_t main(int argc, char **argv)
{
    sp<ProcessState> proc = ProcessState::self();
    ProcessState::self()->startThreadPool();

    sp<SurfaceComposerClient> client = new SurfaceComposerClient();

    // A LayerScreenshot is a special type of layer
    // that contains a screenshot of the screen acquired
    // when its created. It works just like LayerDim.
    //
    // Make sure to call compositionComplete()
    // after rendering into a FBO.
    sp<SurfaceControl> surfaceControl = client->createSurface(
            String8("test-transform2"),
            DISPLAY_HANDLE,
            LAYER_WIDTH,
            LAYER_HEIGHT,
            PIXEL_FORMAT_RGBA_8888,
            ISurfaceComposer::eFXSurfaceScreenshot & ISurfaceComposer::eFXSurfaceMask);
    // reference:
    // ScreenRotationAnimation.java
    // Surface.java
    // android_view_Surface.cpp
    // ISurfaceComposer.h
    int orientation = 0;
    int oldRotation = ROTATION_0;
    int newRotation = ROTATION_0;

    while (true) {
        int delta = newRotation - oldRotation;
        if (delta < 0) {
            delta += 4;
        }

        // rotate with animation
        int degree = oldRotation * 90;
        int toDegree = degree + delta * 90;
        LOGD("(delta,degree)=(%d,%d)", delta, degree);
        while (degree++ < toDegree) {
            SurfaceComposerClient::openGlobalTransaction();
            surfaceControl->setLayer(100000);
            setRotation(surfaceControl, degree);
            SurfaceComposerClient::closeGlobalTransaction();
            usleep(16667); // fps = 60
        }

        oldRotation = newRotation;
        orientation = (orientation + 1) % 4;
        switch(orientation) {
            case 0:
                newRotation = ROTATION_0;
                break;
            case 1:
                newRotation = ROTATION_90;
                break;
            case 2:
                newRotation = ROTATION_180;
                break;
            case 3:
                newRotation = ROTATION_270;
                break;
            default:
                LOGE("out of index");
        }
    }

    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}
