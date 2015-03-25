/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.player;

import java.io.IOException;
import java.util.List;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;

public class EffectCameraPlayer extends EffectPlayer {
    private int mCameraId = 0;
    private int mFps = 30;
    private Camera mCamera;
    private Camera.Parameters mCameraParameters;
    private int mDegree = 0;

    public EffectCameraPlayer(int width, int height) {
        super(width, height);
    }

    private int[] findClosestSize(int width, int height, Camera.Parameters parameters) {
        List<Camera.Size> previewSizes = parameters.getSupportedPreviewSizes();
        int closestWidth = -1;
        int closestHeight = -1;
        int smallestWidth = previewSizes.get(0).width;
        int smallestHeight = previewSizes.get(0).height;
        for (Camera.Size size : previewSizes) {
            // Best match defined as not being larger in either dimension than
            // the requested size, but as close as possible. The below isn't a
            // stable selection (reording the size list can give different
            // results), but since this is a fallback nicety, that's acceptable.
            if (size.width <= width && size.height <= height && size.width >= closestWidth
                && size.height >= closestHeight) {
                closestWidth = size.width;
                closestHeight = size.height;
            }
            if (size.width < smallestWidth && size.height < smallestHeight) {
                smallestWidth = size.width;
                smallestHeight = size.height;
            }
        }
        if (closestWidth == -1) {
            // Requested size is smaller than any listed size; match with smallest possible
            closestWidth = smallestWidth;
            closestHeight = smallestHeight;
        }

        mTool.log('d', "Requested resolution: (" + width + ", " + height + "). Closest match: (" + closestWidth + ", "
            + closestHeight + ").");

        int[] closestSize = {
            closestWidth, closestHeight
        };
        return closestSize;
    }

    private int[] findClosestFpsRange(int fps, Camera.Parameters params) {
        List<int[]> supportedFpsRanges = params.getSupportedPreviewFpsRange();
        int[] closestRange = supportedFpsRanges.get(0);
        for (int[] range : supportedFpsRanges) {
            if (range[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] < fps * 1000
                && range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] > fps * 1000
                && range[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] > closestRange[Camera.Parameters.PREVIEW_FPS_MIN_INDEX]
                && range[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] < closestRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX]) {
                closestRange = range;
            }
        }
        mTool.log('d', "Requested fps: " + fps + ".Closest frame rate range: ["
            + closestRange[Camera.Parameters.PREVIEW_FPS_MIN_INDEX] / 1000. + ","
            + closestRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX] / 1000. + "]");

        return closestRange;
    }

    public Camera getCamera() {
        return mCamera;
    }

    public void setCameraId(int id) {
        mCameraId = id;
    }

    private Camera.Parameters getCameraParameters() {
        boolean closeCamera = false;
        if (mCameraParameters == null) {
            if (mCamera == null) {
                mCamera = Camera.open(mCameraId);
                closeCamera = true;
            }
            mCameraParameters = mCamera.getParameters();

            if (closeCamera) {
                mCamera.release();
                mCamera = null;
            }
        }

        int closestSize[] = findClosestSize(mWidth, mHeight, mCameraParameters);
        mWidth = closestSize[0];
        mHeight = closestSize[1];
        mCameraParameters.setPreviewSize(mWidth, mHeight);

        int closestRange[] = findClosestFpsRange(mFps, mCameraParameters);

        mCameraParameters.setPreviewFpsRange(closestRange[Camera.Parameters.PREVIEW_FPS_MIN_INDEX],
                                             closestRange[Camera.Parameters.PREVIEW_FPS_MAX_INDEX]);

        return mCameraParameters;
    }

    public void setDisplayOrientation(int degree) {
        mDegree = degree;
    }

    public class StartGraphThread implements Runnable {
        public void run() {
            Thread.currentThread().setName("[" + mTool.getID() + "] Start Graph Thread - " + " CameraPlayer");
            synchronized (EffectCameraPlayer.this) {
                mTool.log('d', "showCamera() Srart-Thread Start ... ");

                isStartThreadRunning = true;

                mEffectGraphCore.graphClose();

                if (mEffectGraphCore != null && mEffectGraphCore.isGraphRunning == false) {
                    mEffectGraphCore.setResourceContext(mCntx, mEffectVideoUri);

                    if (mEffectGraphCore.graphCreate() == true) {
                        SurfaceTexture st = mEffectGraphCore.graphRun();

                        if (st != null) {
                            st.setDefaultBufferSize(mWidth, mHeight);

                            mCamera = Camera.open(mCameraId);

                            // Set parameters
                            getCameraParameters();
                            mCameraParameters.setPreviewSize(mWidth, mHeight);
                            mCamera.setParameters(mCameraParameters);
                            mCamera.setDisplayOrientation(mDegree);

                            try {
                                mCamera.setPreviewTexture(st);
                            } catch (IOException e) {
                                mCamera.release();
                                mCamera = null;
                                throw new RuntimeException("Could not bind camera surface texture: " + e.getMessage()
                                    + "!");
                            }
                            mCamera.startPreview();
                        }
                    }
                }
                isStartThreadRunning = false;
                mTool.log('d', "showCamera() Srart-Thread End ... ");
            }
        }
    }

    public void showCamera() {
        mTool.log('d', "showCamera() ");
        if (isStartThreadRunning == false) {
            submit(new StartGraphThread());
        }
    }

    @Override
    public synchronized void release() {
        super.release();
        if (null != mCamera) {
            mCamera.release();
        }
        mCamera = null;
    }
}
