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

package com.mediatek.effect.filterpacks.io;

import com.mediatek.effect.filterpacks.MyUtility;

import android.annotation.SuppressLint;
import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.GenerateFinalPort;
import android.filterfw.core.GLEnvironment;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;
import android.filterfw.format.ImageFormat;

import android.filterfw.geometry.Quad;
import android.filterfw.geometry.Point;

import android.view.Surface;
import android.view.TextureView;

import android.graphics.SurfaceTexture;

/**
 * @hide
 */
public class TextureViewOutFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);
    private boolean mLogVerbose;

    private final int RENDERMODE_STRETCH   = 0;
    private final int RENDERMODE_FIT       = 1;
    private final int RENDERMODE_FILL_CROP = 2;
    private final int RENDERMODE_CUSTOMIZE = 3;

    /** Required. Sets the destination surfaceTexture.
     */
    @GenerateFinalPort(name = "textureView")
    private TextureView mTextureView = null;
    private SurfaceTexture mSurfaceTexture = null;

    /** Required. Sets the width of the output surfaceTexture images */
    @GenerateFinalPort(name = "width")
    private int mScreenWidth;

    /** Required. Sets the height of the output surfaceTexture images */
    @GenerateFinalPort(name = "height")
    private int mScreenHeight;


    /** Optional. Control how the incoming frames are rendered onto the
     * output. Default is FIT.
     * RENDERMODE_STRETCH: Just fill the output surfaceView.
     * RENDERMODE_FIT: Keep aspect ratio and fit without cropping. May
     * have black bars.
     * RENDERMODE_FILL_CROP: Keep aspect ratio and fit without black
     * bars. May crop.
     */
    @GenerateFieldPort(name = "renderMode", hasDefault = true)
    private String mRenderModeString;

    @GenerateFieldPort(name = "sourceQuad", hasDefault = true)
    private Quad mSourceQuad = new Quad(new Point(0.0f, 1.0f),
                                        new Point(1.0f, 1.0f),
                                        new Point(0.0f, 0.0f),
                                        new Point(1.0f, 0.0f));

    @GenerateFieldPort(name = "targetQuad", hasDefault = true)
    private Quad mTargetQuad = new Quad(new Point(0.0f, 0.0f),
                                        new Point(1.0f, 0.0f),
                                        new Point(0.0f, 1.0f),
                                        new Point(1.0f, 1.0f));

    private int mSurfaceId;

    private ShaderProgram mProgram;
    private GLFrame mScreen;
    private int mRenderMode = RENDERMODE_FIT;
    private float mAspectRatio = 1.f;

    private int mFrameCount;

    public TextureViewOutFilter(String name) {
        super(name);
        mLogVerbose = true;//Log.isLoggable(TAG, Log.VERBOSE);
        mTool.log('d', getClass().getSimpleName() + "() " + name);
        mTool.setIDandIncrease(mCount);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    @Override
    public synchronized void setupPorts() {
        mTool.log('d', "setupPorts() mTextureView: " + mTextureView);

        if (mTextureView != null) {
            mSurfaceTexture = mTextureView.getSurfaceTexture();
        }

        mTool.log('d', "setupPorts() mSurfaceTexture: " + mSurfaceTexture);

        // Make sure we have a SurfaceView
        if (mSurfaceTexture == null) {
            throw new RuntimeException("Null SurfaceTexture passed to SurfaceTextureTarget");
        }

        // Add input port - will accept anything that's 4-channel.
        addMaskedInputPort("frame", ImageFormat.create(ImageFormat.COLORSPACE_RGBA));
    }

    public void updateRenderMode() {
        if (mLogVerbose)
            mTool.log('d', "updateRenderMode() Thread: " + Thread.currentThread());
        if (mRenderModeString != null) {
            if (mRenderModeString.equals("stretch")) {
                mRenderMode = RENDERMODE_STRETCH;
            } else if (mRenderModeString.equals("fit")) {
                mRenderMode = RENDERMODE_FIT;
            } else if (mRenderModeString.equals("fill_crop")) {
                mRenderMode = RENDERMODE_FILL_CROP;
            } else if (mRenderModeString.equals("customize")) {
                mRenderMode = RENDERMODE_CUSTOMIZE;
            } else {
                throw new RuntimeException("Unknown render mode '" + mRenderModeString + "'!");
            }
        }
        updateTargetRect();
    }

    @Override
    public void prepare(FilterContext context) {
        if (mLogVerbose)
            mTool.log('d', "prepare() Thread: " + Thread.currentThread());

        // Create identity shader to render, and make sure to render
        // upside-down, as textures
        // are stored internally bottom-to-top.
        mProgram = ShaderProgram.createIdentity(context);
        mProgram.setSourceRect(0, 1, 1, -1);
        mProgram.setClearColor(0.0f, 0.0f, 0.0f);

        updateRenderMode();

        // Create a frame representing the screen
        MutableFrameFormat screenFormat = new MutableFrameFormat(FrameFormat.TYPE_BYTE,
            FrameFormat.TARGET_GPU);
        screenFormat.setBytesPerSample(4);
        screenFormat.setDimensions(mScreenWidth, mScreenHeight);
        mScreen = (GLFrame) context.getFrameManager().newBoundFrame(screenFormat,
            GLFrame.EXISTING_FBO_BINDING, 0);
    }

    @SuppressLint("NewApi")
    @Override
    public synchronized void open(FilterContext context) {
        // Set up SurfaceTexture internals
        if (mSurfaceTexture == null) {
            mTool.log('e', "open() SurfaceTexture is null!!");
            throw new RuntimeException("Could not register SurfaceTexture: " + mSurfaceTexture);
        }

        GLEnvironment gl = context.getGLEnvironment();

        mTool.log('d', "open() mSurfaceTexture: " + mSurfaceTexture);
        mSurfaceTexture.setDefaultBufferSize(mScreenWidth, mScreenHeight);
        //mSurfaceId = gl.registerSurfaceTexture(mSurfaceTexture, mScreenWidth, mScreenHeight);
        Surface surface = new Surface(mSurfaceTexture);
        mSurfaceId = gl.registerSurface(surface);
        mTool.log('d', "open() new surface: " + surface + ", mSurfaceId: " + mSurfaceId);
        surface.release();

        mFrameCount = 0;
        if (mSurfaceId <= 0) {
            throw new RuntimeException("Could not register SurfaceTexture: " + mSurfaceTexture);
        }
    }


    // Once the surface is unregistered, we still need the surfacetexture reference.
    // That is because when the the filter graph stops and starts again, the app
    // may not set the mSurfaceTexture again on the filter. In some cases, the app
    // may not even know that the graph has re-started. So it is difficult to enforce
    // that condition on an app using this filter. The only case where we need
    // to let go of the mSurfaceTexure reference is when the app wants to shut
    // down the graph on purpose, such as in the disconnect call.
    @Override
    public synchronized void close(FilterContext context) {
        if (mSurfaceId > 0) {
            context.getGLEnvironment().unregisterSurfaceId(mSurfaceId);
            mSurfaceId = -1;
        }
    }

    // This should be called from the client side when the surfacetexture is no longer
    // valid. e.g. from onPause() in the application using the filter graph.
    // In this case, we need to let go of our surfacetexture reference.
    public synchronized void disconnect(FilterContext context) {
        if (mLogVerbose) mTool.log('d', "disconnect()");
        if (mSurfaceTexture == null) {
            mTool.log('w', "SurfaceTexture is already null. Nothing to disconnect.");
            return;
        }
        mSurfaceTexture = null;
        // Make sure we unregister the surface as well if a surface was registered.
        // There can be a situation where the surface was not registered but the
        // surfacetexture was valid. For example, the disconnect can be called before
        // the filter was opened. Hence, the surfaceId may not be a valid one here,
        // and need to check for its validity.
        if (mSurfaceId > 0) {
            context.getGLEnvironment().unregisterSurfaceId(mSurfaceId);
            mSurfaceId = -1;
        }
    }

    @Override
    public synchronized void process(FilterContext context) {
        // Surface is not registered. Nothing to render into.
        if (mSurfaceId <= 0) {
            return;
        }
        GLEnvironment glEnv = context.getGLEnvironment();

        // Get input frame
        Frame input = pullInput("frame");
        boolean createdFrame = false;

        float currentAspectRatio =
          (float)input.getFormat().getWidth() / input.getFormat().getHeight();
        if (currentAspectRatio != mAspectRatio) {
            if (mLogVerbose) {
                mTool.log('d', "process() New aspect ratio: " + currentAspectRatio +
                    ", previously: " + mAspectRatio + ". Thread: " + Thread.currentThread());
            }
            mAspectRatio = currentAspectRatio;
            updateTargetRect();
        }

        // See if we need to copy to GPU
        Frame gpuFrame = null;
        int target = input.getFormat().getTarget();
        if (target != FrameFormat.TARGET_GPU) {
            gpuFrame = context.getFrameManager().duplicateFrameToTarget(input,
                                                                        FrameFormat.TARGET_GPU);
            createdFrame = true;
        } else {
            gpuFrame = input;
        }

        // Activate our surface
        glEnv.activateSurfaceWithId(mSurfaceId);

        // Process
        mProgram.process(gpuFrame, mScreen);

        glEnv.setSurfaceTimestamp(input.getTimestamp());

        // And swap buffers
        glEnv.swapBuffers();

        if (createdFrame) {
            gpuFrame.release();
        }

        mFrameCount++;
    }

    @Override
    public void fieldPortValueUpdated(String name, FilterContext context) {
        if (mLogVerbose) mTool.log('d', "fieldPortValueUpdated() FPVU. Thread: " + Thread.currentThread());
        updateRenderMode();
    }

    @Override
    public void tearDown(FilterContext context) {
        if (mScreen != null) {
            mScreen.release();
        }
    }

    private void updateTargetRect() {
        if (mLogVerbose) mTool.log('d', "updateTargetRect() Thread: " + Thread.currentThread());
        if (mScreenWidth > 0 && mScreenHeight > 0 && mProgram != null) {
            float screenAspectRatio = (float)mScreenWidth / mScreenHeight;
            float relativeAspectRatio = screenAspectRatio / mAspectRatio;
            if (mLogVerbose) {
                mTool.log('d', "updateTargetRect() UTR. screen w = " + (float)mScreenWidth + " x screen h = " +
                    (float)mScreenHeight + " Screen AR: " + screenAspectRatio +
                    ", frame AR: "  + mAspectRatio + ", relative AR: " + relativeAspectRatio);
            }

            if (relativeAspectRatio == 1.0f && mRenderMode != RENDERMODE_CUSTOMIZE) {
                mProgram.setTargetRect(0, 0, 1, 1);
                mProgram.setClearsOutput(false);
            } else {
                switch (mRenderMode) {
                    case RENDERMODE_STRETCH:
                        mTargetQuad.p0.set(0f, 0.0f);
                        mTargetQuad.p1.set(1f, 0.0f);
                        mTargetQuad.p2.set(0f, 1.0f);
                        mTargetQuad.p3.set(1f, 1.0f);
                        mProgram.setClearsOutput(false);
                        break;
                    case RENDERMODE_FIT:
                        if (relativeAspectRatio > 1.0f) {
                            // Screen is wider than the camera, scale down X
                            mTargetQuad.p0.set(0.5f - 0.5f / relativeAspectRatio, 0.0f);
                            mTargetQuad.p1.set(0.5f + 0.5f / relativeAspectRatio, 0.0f);
                            mTargetQuad.p2.set(0.5f - 0.5f / relativeAspectRatio, 1.0f);
                            mTargetQuad.p3.set(0.5f + 0.5f / relativeAspectRatio, 1.0f);

                        } else {
                            // Screen is taller than the camera, scale down Y
                            mTargetQuad.p0.set(0.0f, 0.5f - 0.5f * relativeAspectRatio);
                            mTargetQuad.p1.set(1.0f, 0.5f - 0.5f * relativeAspectRatio);
                            mTargetQuad.p2.set(0.0f, 0.5f + 0.5f * relativeAspectRatio);
                            mTargetQuad.p3.set(1.0f, 0.5f + 0.5f * relativeAspectRatio);
                        }
                        mProgram.setClearsOutput(true);
                        break;
                    case RENDERMODE_FILL_CROP:
                        if (relativeAspectRatio > 1) {
                            // Screen is wider than the camera, crop in Y
                            mTargetQuad.p0.set(0.0f, 0.5f - 0.5f * relativeAspectRatio);
                            mTargetQuad.p1.set(1.0f, 0.5f - 0.5f * relativeAspectRatio);
                            mTargetQuad.p2.set(0.0f, 0.5f + 0.5f * relativeAspectRatio);
                            mTargetQuad.p3.set(1.0f, 0.5f + 0.5f * relativeAspectRatio);
                        } else {
                            // Screen is taller than the camera, crop in X
                            mTargetQuad.p0.set(0.5f - 0.5f / relativeAspectRatio, 0.0f);
                            mTargetQuad.p1.set(0.5f + 0.5f / relativeAspectRatio, 0.0f);
                            mTargetQuad.p2.set(0.5f - 0.5f / relativeAspectRatio, 1.0f);
                            mTargetQuad.p3.set(0.5f + 0.5f / relativeAspectRatio, 1.0f);
                        }
                        mProgram.setClearsOutput(true);
                        break;
                    case RENDERMODE_CUSTOMIZE:
                        ((ShaderProgram) mProgram).setSourceRegion(mSourceQuad);
                        break;
                }
                if (mLogVerbose) mTool.log('d', "updateTargetRect() UTR. quad: " + mTargetQuad);
                ((ShaderProgram) mProgram).setTargetRegion(mTargetQuad);
            }
        }
    }
}
