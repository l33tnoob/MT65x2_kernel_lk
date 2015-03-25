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
import android.filterfw.core.FilterSurfaceView;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.GenerateFinalPort;
import android.filterfw.core.GLEnvironment;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;
import android.filterfw.format.ImageFormat;
import android.graphics.SurfaceTexture;

import android.view.Surface;
import android.view.SurfaceHolder;

/**
 * @hide
 */
public class SurfaceViewOutFilter extends Filter implements SurfaceHolder.Callback {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);
    private boolean mLogVerbose;

    private final int RENDERMODE_STRETCH = 0;
    private final int RENDERMODE_FIT = 1;
    private final int RENDERMODE_FILL_CROP = 2;

    /**
     * Required. Sets the destination filter surface view for this node.
     */
    @GenerateFinalPort(name = "surfaceView")
    private FilterSurfaceView mSurfaceView;

    /**
     * Optional. Control how the incoming frames are rendered onto the output.
     * Default is FIT. RENDERMODE_STRETCH: Just fill the output surfaceView.
     * RENDERMODE_FIT: Keep aspect ratio and fit without cropping. May have
     * black bars. RENDERMODE_FILL_CROP: Keep aspect ratio and fit without black
     * bars. May crop.
     */
    @GenerateFieldPort(name = "renderMode", hasDefault = true)
    private String mRenderModeString;

    private boolean mIsBound = false;

    private ShaderProgram mProgram;
    private GLFrame mScreen;
    private int mRenderMode = RENDERMODE_FIT;
    private float mAspectRatio = 1.f;

    private int mScreenWidth;
    private int mScreenHeight;

    private GLFrame mGLMediaFrame;
    private SurfaceTexture mSurfaceTexture = null;
    private int mSurfaceId = -1;
    private ShaderProgram mFrameExtractorProgram;
    // This is an identity shader; not using the default identity
    // shader because reading from a SurfaceTexture requires the
    // GL_OES_EGL_image_external extension.
    private final String mFrameShader =
            "#extension GL_OES_EGL_image_external : require\n" +
            "precision mediump float;\n" +
            "uniform samplerExternalOES tex_sampler_0;\n" +
            "varying vec2 v_texcoord;\n" +
            "void main() {\n" +
            "  gl_FragColor = texture2D(tex_sampler_0, v_texcoord);\n" +
            "}\n";

    public SurfaceViewOutFilter(String name) {
        super(name);
        mLogVerbose = true;// Log.isLoggable(TAG, Log.VERBOSE);
        mTool.log('d', getClass().getSimpleName() + "() " + name);
        mTool.setIDandIncrease(mCount);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    @Override
    public void setupPorts() {
        // Make sure we have a SurfaceView
        if (mSurfaceView == null) {
            throw new RuntimeException("NULL SurfaceView passed to SurfaceRenderFilter");
        }

        // Add input port
        addMaskedInputPort("frame", ImageFormat.create(ImageFormat.COLORSPACE_RGBA));
    }

    public void updateRenderMode() {
        if (mRenderModeString != null) {
            if (mRenderModeString.equals("stretch")) {
                mRenderMode = RENDERMODE_STRETCH;
            } else if (mRenderModeString.equals("fit")) {
                mRenderMode = RENDERMODE_FIT;
            } else if (mRenderModeString.equals("fill_crop")) {
                mRenderMode = RENDERMODE_FILL_CROP;
            } else {
                throw new RuntimeException("Unknown render mode '" + mRenderModeString + "'!");
            }
        }
        updateTargetRect();
    }

    @Override
    public void prepare(FilterContext context) {
        // Create identity shader to render, and make sure to render
        // upside-down, as textures
        // are stored internally bottom-to-top.
        mProgram = ShaderProgram.createIdentity(context);
        // mProgram = new ShaderProgram(context, mFrameShader);
        mProgram.setSourceRect(0, 1, 1, -1);
        mProgram.setClearsOutput(true);
        mProgram.setClearColor(0.0f, 0.0f, 0.0f);

        updateRenderMode();

        // Create a frame representing the screen
        MutableFrameFormat screenFormat = ImageFormat.create(mSurfaceView.getWidth(),
            mSurfaceView.getHeight(), ImageFormat.COLORSPACE_RGBA, FrameFormat.TARGET_GPU);
        mScreen = (GLFrame) context.getFrameManager().newBoundFrame(screenFormat,
            GLFrame.EXISTING_FBO_BINDING, 0);

        mFrameExtractorProgram = new ShaderProgram(context, mFrameShader);
        // SurfaceTexture defines (0,0) to be bottom-left. The filter framework
        // defines (0,0) as top-left, so do the flip here.
        mFrameExtractorProgram.setSourceRect(0, 1, 1, -1);
    }

    @SuppressLint("NewApi")
    @Override
    public void open(FilterContext context) {
        // Bind surface view to us. This will emit a surfaceCreated and
        // surfaceChanged call that
        // will update our screen width and height.
        mSurfaceView.unbind();
        mSurfaceView.bindToListener(this, context.getGLEnvironment());



        // Set up SurfaceTexture internals
        if (mSurfaceTexture == null) {
            mGLMediaFrame = (GLFrame) context.getFrameManager().newBoundFrame(
                ImageFormat.create(mSurfaceView.getWidth(), mSurfaceView.getHeight(),
                    ImageFormat.COLORSPACE_RGBA, FrameFormat.TARGET_GPU), GLFrame.EXTERNAL_TEXTURE,
                0);
            mSurfaceTexture = new SurfaceTexture(mGLMediaFrame.getTextureId());
        }

        GLEnvironment gl = context.getGLEnvironment();
        mTool.log('d', "open() mSurfaceTexture: " + mSurfaceTexture);
        mSurfaceTexture.setDefaultBufferSize(mScreenWidth, mScreenHeight);
        Surface surface = new Surface(mSurfaceTexture);
        mSurfaceId = gl.registerSurface(surface);
        mTool.log('d', "open() new surface: " + surface + ", mSurfaceId: " + mSurfaceId);
        surface.release();

        if (mSurfaceId <= 0) {
            throw new RuntimeException("Could not register SurfaceTexture: " + mSurfaceTexture);
        }
    }

    @Override
    public synchronized void process(FilterContext context) {
        // Make sure we are bound to a surface before rendering
        if (!mIsBound) {
            mTool.log('d', "process() Ignoring frame as there is no surface to render to!");
            return;
        }

        GLEnvironment glEnv = mSurfaceView.getGLEnv();
        if (glEnv != context.getGLEnvironment()) {
            throw new RuntimeException("Surface created under different GLEnvironment!");
        }

        // Get input frame
        Frame input = pullInput("frame");
        boolean createdFrame = false;

        float currentAspectRatio = (float) input.getFormat().getWidth()
            / input.getFormat().getHeight();
        if (currentAspectRatio != mAspectRatio) {
            if (mLogVerbose)
                mTool.log('d', "process() New aspect ratio: " + currentAspectRatio + ", previously: " + mAspectRatio);
            mAspectRatio = currentAspectRatio;
            updateTargetRect();
        }

        // See if we need to copy to GPU
        Frame gpuFrame = null;
        int target = input.getFormat().getTarget();
        if (target != FrameFormat.TARGET_GPU) {
            gpuFrame = context.getFrameManager().duplicateFrameToTarget(input, FrameFormat.TARGET_GPU);
            createdFrame = true;
        } else {
            gpuFrame = input;
        }

        glEnv.activateSurfaceWithId(mSurfaceView.getSurfaceId());
        mProgram.process(gpuFrame, mScreen);

        glEnv.swapBuffers();

        if (createdFrame) {
            gpuFrame.release();
        }
    }

    @Override
    public void fieldPortValueUpdated(String name, FilterContext context) {
        updateTargetRect();
    }

    @Override
    public void close(FilterContext context) {
        mSurfaceView.unbind();

        mSurfaceTexture.release();
        mSurfaceTexture = null;

        if (mSurfaceId > 0) {
            context.getGLEnvironment().unregisterSurfaceId(mSurfaceId);
            mSurfaceId = -1;
        }
    }

    @Override
    public void tearDown(FilterContext context) {
        if (mScreen != null) {
            mScreen.release();
        }

        if (mGLMediaFrame != null) {
            mGLMediaFrame.release();
        }
    }

    // @Override
    public synchronized void surfaceCreated(SurfaceHolder holder) {
        mIsBound = true;
    }

    // @Override
    public synchronized void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // If the screen is null, we do not care about surface changes (yet).
        // Once we have a
        // screen object, we need to keep track of these changes.
        if (mScreen != null) {
            mScreenWidth = width;
            mScreenHeight = height;
            mScreen.setViewport(0, 0, width, height);
            updateTargetRect();
            if (mLogVerbose)
                mTool.log('d', "surfaceChanged(" + holder + "): " + mScreenWidth + "x" + mScreenHeight);
        }
    }

    // @Override
    public synchronized void surfaceDestroyed(SurfaceHolder holder) {
        mIsBound = false;
    }

    private void updateTargetRect() {
        if (mScreenWidth > 0 && mScreenHeight > 0 && mProgram != null) {
            float screenAspectRatio = (float) mScreenWidth / mScreenHeight;
            float relativeAspectRatio = screenAspectRatio / mAspectRatio;

            switch (mRenderMode) {
                case RENDERMODE_STRETCH:
                    mProgram.setTargetRect(0, 0, 1, 1);
                    mFrameExtractorProgram.setTargetRect(0, 0, 1, 1);
                    break;
                case RENDERMODE_FIT:
                    if (relativeAspectRatio > 1.0f) {
                        // Screen is wider than the camera, scale down X
                        mProgram.setTargetRect(0.5f - 0.5f / relativeAspectRatio, 0.0f,
                            1.0f / relativeAspectRatio, 1.0f);

                        mFrameExtractorProgram.setTargetRect(0.5f - 0.5f / relativeAspectRatio, 0.0f,
                            1.0f / relativeAspectRatio, 1.0f);
                    } else {
                        // Screen is taller than the camera, scale down Y
                        mProgram.setTargetRect(0.0f, 0.5f - 0.5f * relativeAspectRatio, 1.0f,
                            relativeAspectRatio);

                        mFrameExtractorProgram.setTargetRect(0.0f, 0.5f - 0.5f * relativeAspectRatio, 1.0f,
                            relativeAspectRatio);
                    }
                    break;
                case RENDERMODE_FILL_CROP:
                    if (relativeAspectRatio > 1) {
                        // Screen is wider than the camera, crop in Y
                        mProgram.setTargetRect(0.0f, 0.5f - 0.5f * relativeAspectRatio, 1.0f,
                            relativeAspectRatio);

                        mFrameExtractorProgram.setTargetRect(0.0f, 0.5f - 0.5f * relativeAspectRatio, 1.0f,
                            relativeAspectRatio);
                    } else {
                        // Screen is taller than the camera, crop in X
                        mProgram.setTargetRect(0.5f - 0.5f / relativeAspectRatio, 0.0f,
                            1.0f / relativeAspectRatio, 1.0f);

                        mFrameExtractorProgram.setTargetRect(0.5f - 0.5f / relativeAspectRatio, 0.0f,
                            1.0f / relativeAspectRatio, 1.0f);
                    }
                    break;
            }
        }
    }
}
