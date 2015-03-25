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

import android.annotation.TargetApi;
import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.GenerateFinalPort;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;
import android.filterfw.format.ImageFormat;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.os.SystemClock;
import android.opengl.Matrix;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.player.EffectUiHandler;

import android.util.Log;

/**
 * @hide
 */
@TargetApi(16)
public class SurfaceTextureInFilter extends Filter{
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    /** User-visible parameters */

    /**
     * Whether the filter will always wait for a new video frame, or whether it
     * will output an old frame again if a new frame isn't available. Defaults
     * to true.
     */
    @GenerateFinalPort(name = "waitForNewFrame", hasDefault = true)
    private boolean mWaitForNewFrame = true;

    /**
     * Orientation. This controls the output orientation of the video. Valid
     * values are 0, 90, 180, 270
     */
    @GenerateFieldPort(name = "orientation", hasDefault = true)
    private int mOrientation = 0;

    /** The width of the output image frame. If the texture width for the
     * SurfaceTexture source is known, use it here to minimize resampling. */
    @GenerateFieldPort(name = "width")
    private int mWidth;

    /** The height of the output image frame. If the texture height for the
     * SurfaceTexture source is known, use it here to minimize resampling. */
    @GenerateFieldPort(name = "height")
    private int mHeight;

    @GenerateFieldPort(name = "inwidth", hasDefault = true)
    private int mInputWidth = 0;

    @GenerateFieldPort(name = "inheight", hasDefault = true)
    private int mInputHeight = 0;

    @GenerateFieldPort(name = "truncate", hasDefault = true)
    private boolean mIsTruncate = false;

    @GenerateFieldPort(name = "effectplayer", hasDefault = true)
    private MediaPlayer mMediaPlayer = null;

    @GenerateFieldPort(name = "ignoreframe", hasDefault = true)
    private boolean mIgnoreMainFrameStreem = false;

    private GLFrame mGLMediaFrame;
    private SurfaceTexture mSurfaceTexture = null;
    private ShaderProgram mFrameExtractorProgram;
    private MutableFrameFormat mOutputFormat;

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

    // The following transforms enable rotation of the decoded source.
    // These are multiplied with the transform obtained from the
    // SurfaceTexture to get the final transform to be set on the media source.
    private static final float[] mSourceCoords_0 = {
            1, 1, 0, 1,
            0, 1, 0, 1,
            1, 0, 0, 1,
            0, 0, 0, 1
    };
    private static final float[] mSourceCoords_90 = {
            1, 0, 0, 1,
            1, 1, 0, 1,
            0, 0, 0, 1,
            0, 1, 0, 1
    };
    private static final float[] mSourceCoords_180 = {
            0, 0, 0, 1,
            1, 0, 0, 1,
            0, 1, 0, 1,
            1, 1, 0, 1
    };
    private static final float[] mSourceCoords_270 = {
            0, 1, 0, 1,
            0, 0, 0, 1,
            1, 1, 0, 1,
            1, 0, 0, 1
    };

    private static final int[][] mAngleIndex = {
        { 0,  4,  8, 12},
        { 4, 12,  0,  8},
        {12,  8,  4,  0},
        { 8,  0,  12, 4}
    };


    private boolean mGotSize;
    private boolean mOrientationUpdated;
    private boolean mCompleted;

    private boolean mLogVerbose;
    private final String TAG = this.getClass().getSimpleName();

    private boolean mNewFrameAvailable = false;

    private int mProcessMaxFrameCount = -1;
    private int mFrameCount = 0;
    private int mLastFrameCount = 0;
    private long mLastTime = 0;
    private long mPeriod = 500;
    private float mFps = 0;
    private EffectUiHandler mUIHandler = null;

    private long mStartTimestampNs = 0;

    public SurfaceTextureInFilter(String name) {
        super(name);
        mLogVerbose = Log.isLoggable(TAG, Log.VERBOSE);
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
        mTool.log('d', "setupPorts()");

        // Add input port
        addOutputPort("video", ImageFormat.create(ImageFormat.COLORSPACE_RGBA,
                FrameFormat.TARGET_GPU));
    }

    private void createFormats() {
        mOutputFormat = ImageFormat.create(mWidth, mHeight,
                ImageFormat.COLORSPACE_RGBA, FrameFormat.TARGET_GPU);
    }

    private void setSourceRect(float sourceCoords[], int angleIndex, float sourceCoords_out[]) {
        float localInputWidth  = mInputWidth;
        float localInputHeight = mInputHeight;

        if (mOrientation == 90 || mOrientation == 270) {
            localInputWidth = mInputHeight;
            localInputHeight = mInputWidth;
        }

        if (mIsTruncate && (0 != mInputWidth && 0 != mInputHeight)) {
            float outputAspectRatio = (float) mWidth / mHeight;
            float inputAspectRatio = (float) localInputWidth / localInputHeight;
            float xOffset = 0.0f;
            float yOffset = 0.0f;

            if (outputAspectRatio <= inputAspectRatio) {
                xOffset = (localInputWidth - localInputHeight * outputAspectRatio) / 2.0f;
                xOffset = Math.abs(xOffset / localInputWidth);
            } else {
                yOffset = (localInputHeight - localInputWidth / outputAspectRatio) / 2.0f;
                yOffset = Math.abs(yOffset / localInputHeight);
            }

            if (mOrientation == 90 || mOrientation == 270) {
                float tmp = xOffset;
                xOffset = yOffset;
                yOffset = tmp;
            }

            // 1,1
            sourceCoords_out[mAngleIndex[angleIndex][0]]    = sourceCoords[mAngleIndex[angleIndex][0]]   - xOffset;
            sourceCoords_out[mAngleIndex[angleIndex][0]+1]  = sourceCoords[mAngleIndex[angleIndex][0]+1] - yOffset;
            sourceCoords_out[2]  = sourceCoords[2];
            sourceCoords_out[3]  = sourceCoords[3];

            // 0,1
            sourceCoords_out[mAngleIndex[angleIndex][1]]   = sourceCoords[mAngleIndex[angleIndex][1]]   + xOffset;
            sourceCoords_out[mAngleIndex[angleIndex][1]+1] = sourceCoords[mAngleIndex[angleIndex][1]+1] - yOffset;
            sourceCoords_out[6] = sourceCoords[6];
            sourceCoords_out[7] = sourceCoords[7];

            // 1,0
            sourceCoords_out[mAngleIndex[angleIndex][2]]   = sourceCoords[mAngleIndex[angleIndex][2]]   - xOffset;
            sourceCoords_out[mAngleIndex[angleIndex][2]+1] = sourceCoords[mAngleIndex[angleIndex][2]+1] + yOffset;
            sourceCoords_out[10] = sourceCoords[10];
            sourceCoords_out[11] = sourceCoords[11];

            // 0,0
            sourceCoords_out[mAngleIndex[angleIndex][3]]   = sourceCoords[mAngleIndex[angleIndex][3]]   + xOffset;
            sourceCoords_out[mAngleIndex[angleIndex][3]+1] = sourceCoords[mAngleIndex[angleIndex][3]+1] + yOffset;
            sourceCoords_out[14] = sourceCoords[14];
            sourceCoords_out[15] = sourceCoords[15];
        } else {
            for (int i = 0; i < sourceCoords.length; i++) {
                sourceCoords_out[i] = sourceCoords[i];
            }
        }
    }

    @Override
    protected void prepare(FilterContext context) {
        if (mLogVerbose) {
            mTool.log('d', "Preparing ... " + this);
        }

        mFrameExtractorProgram = new ShaderProgram(context, mFrameShader);
        // SurfaceTexture defines (0,0) to be bottom-left. The filter framework
        // defines (0,0) as top-left, so do the flip here.
        mFrameExtractorProgram.setSourceRect(0, 1, 1, -1);

        createFormats();

        if (mSurfaceTexture == null) {
            mGLMediaFrame = (GLFrame) context.getFrameManager().newBoundFrame(
                    mOutputFormat,
                    GLFrame.EXTERNAL_TEXTURE,
                    0);

            mSurfaceTexture = new SurfaceTexture(mGLMediaFrame.getTextureId());
        }
    }

    @Override
    public synchronized void open(FilterContext context) {
        if (mLogVerbose) {
            mTool.log('d', "Opening ... " + this);
        }

        if (mSurfaceTexture == null) {
            mGLMediaFrame = (GLFrame) context.getFrameManager().newBoundFrame(
                    mOutputFormat,
                    GLFrame.EXTERNAL_TEXTURE,
                    0);

            mSurfaceTexture = new SurfaceTexture(mGLMediaFrame.getTextureId());
        }

        this.notifyAll();

        mProcessMaxFrameCount = -1;
        mFrameCount = 0;
        mGotSize = false;
        mCompleted = false;
        mOrientationUpdated = true;
    }

    @Override
    public synchronized void process(FilterContext context) {
        // Note: process is synchronized by its caller in the Filter base class
        if (mLogVerbose) {
            mTool.log('d', "process() " + context);
        }

        if (mCompleted) {
            closeOutputPort("video");
            mTool.log('d', "the filter already completed !");
            return;
        }

        // wait new frame available
        if (mWaitForNewFrame == true) {
            if (mLogVerbose) {
                mTool.log('d', "Waiting for new frame");
            }

            int waitCount = 0;
            while (!mNewFrameAvailable) {
                if (waitCount >= 10) {
                    if (mCompleted) {
                        closeOutputPort("video");
                    }
                    mTool.log('d', "No new frame to be processed. count(" + waitCount + ")");
                    return;
                }

                try {
                    this.wait(100);
                } catch (InterruptedException e) {
                    if (mLogVerbose) {
                        mTool.log('d', "Interrupted");
                    }
                }
                waitCount++;
            }
            mNewFrameAvailable = false;
            if (mLogVerbose) {
                mTool.log('d', "Got new frame");
            }
        }

        if (mCompleted) {
            mTool.log('d', "the filter already completed ! ignore this frame");
            return;
        }

        if (mSurfaceTexture == null) {
            return;
        }

        mSurfaceTexture.updateTexImage();

        if (mOrientationUpdated) {

            if (mIgnoreMainFrameStreem) {
                mStartTimestampNs = System.nanoTime();
                if (mMediaPlayer != null) {
                    mMediaPlayer.pause();
                    mWaitForNewFrame = false;
                }
            }

            float[] surfaceTransform = new float[16];
            mSurfaceTexture.getTransformMatrix(surfaceTransform);

            float[] sourceCoordsin = new float[16];
            float[] sourceCoords   = new float[16];
            switch (mOrientation) {
                default:
                case 0:
                    setSourceRect(mSourceCoords_0, 0, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                            surfaceTransform, 0,
                            sourceCoordsin, 0);
                    break;
                case 90:
                    setSourceRect(mSourceCoords_90, 1, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                            surfaceTransform, 0,
                            sourceCoordsin, 0);
                    break;
                case 180:
                    setSourceRect(mSourceCoords_180, 2, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                            surfaceTransform, 0,
                            sourceCoordsin, 0);
                    break;
                case 270:
                    setSourceRect(mSourceCoords_270, 3, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                            surfaceTransform, 0,
                            sourceCoordsin, 0);
                    break;
            }

            mFrameExtractorProgram.setSourceRegion(sourceCoords[4], sourceCoords[5],
                        sourceCoords[0], sourceCoords[1],
                        sourceCoords[12], sourceCoords[13],
                        sourceCoords[8], sourceCoords[9]);
            mOrientationUpdated = false;
        }

        Frame output = context.getFrameManager().newFrame(mOutputFormat);

        mFrameExtractorProgram.process(mGLMediaFrame, output);

        long timestamp = 0;

        if (mIgnoreMainFrameStreem) {
            timestamp = System.nanoTime() - mStartTimestampNs;
        } else {
            timestamp = mSurfaceTexture.getTimestamp();
        }

        if (mLogVerbose) {
            mTool.log('d', "Timestamp: " + (timestamp / 1000000000.0) + " s");
        }
        output.setTimestamp(timestamp);

        pushOutput("video", output);

        output.release();

        long current = SystemClock.elapsedRealtime();
        mFrameCount++;

        if (mLastTime == 0) {
            mLastTime = current;
            mLastFrameCount = mFrameCount;
        }

        if ((current - mLastTime) > mPeriod) {
            mFps = (float)(mFrameCount - mLastFrameCount) / ((float) (current - mLastTime) / 1000f);
            mLastTime = current;
            mLastFrameCount = mFrameCount;

            if (mUIHandler != null) {
                mUIHandler.setText(String.format("%.02f", mFps) + " fps");
            }
        }

        if (mProcessMaxFrameCount != -1) {
            if (mFrameCount >= mProcessMaxFrameCount) {
                mCompleted = true;
                if (mLogVerbose)
                    mTool.log('d', "process() set mCompleted! " + mFrameCount + "/" + mProcessMaxFrameCount);
                }
        }
    }

    @Override
    public synchronized void close(FilterContext context) {
        mGotSize = false;
        mCompleted = false;

        mSurfaceTexture.release();
        mSurfaceTexture = null;
        if (mLogVerbose) {
            mTool.log('d', this + " closed()");
        }
    }

    @Override
    public void tearDown(FilterContext context) {
        if (mGLMediaFrame != null) {
            mGLMediaFrame.release();
        }
    }

    public void fieldPortValueUpdated(String name, FilterContext context) {
        if (mLogVerbose) {
            mTool.log('d', "Parameter update");
        }

        if (name.equals("width") || name.equals("height") ) {
            mOutputFormat.setDimensions(mWidth, mHeight);
        } else if (name.equals("orientation") && mGotSize) {
            if (mOrientation == 0 || mOrientation == 180) {
                mOutputFormat.setDimensions(mWidth, mHeight);
            } else {
                mOutputFormat.setDimensions(mHeight, mWidth);
            }
            mOrientationUpdated = true;
        }
    }

    public void setProcessMaxFrameCount(int count) {
        mProcessMaxFrameCount = count;
    }

    public synchronized void setCompleted(boolean isComplete){
        mTool.log('w', "setCompleted() " + mCompleted);
        if (mCompleted == false) {
            closeOutputPort("video");
        }
        mCompleted = true;
    }

    public void setNewFrameAvailable(boolean isNew) {
        synchronized(this) {
            mNewFrameAvailable = isNew;
            this.notify();
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        mTool.log('w', "getSurfaceTexture(): " + mSurfaceTexture);
        return mSurfaceTexture;
    }

    public int getTextureId() {
        mTool.log('d', "getTextureId(): " + mGLMediaFrame);
        if (mGLMediaFrame != null) {
            return mGLMediaFrame.getTextureId();
        }
        return 0;
    }

    public void setHandler(EffectUiHandler tv) {
        mUIHandler = tv;
    }
}
