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

import android.content.Context;
import android.content.res.AssetFileDescriptor;
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
import android.net.Uri;
import android.opengl.Matrix;
import android.util.Log;
import android.view.Surface;

import java.io.IOException;
import java.lang.IllegalArgumentException;

import com.mediatek.effect.filterpacks.MyUtility;

/**
 * @hide
 */
public class MediaSourceInFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    private static final int CLEAR_MOTION_KEY = 1700;
    private static final int CLEAR_MOTION_DISABLE = 1;

    /** User-visible parameters */

    /** The source URL for the media source. Can be an http: link to a remote
     * resource, or a file: link to a local media file
     */
    @GenerateFieldPort(name = "sourceUrl", hasDefault = true)
    private String mSourceUrl = "";

    /** An open asset file descriptor to a local media source. Default is null */
    @GenerateFieldPort(name = "sourceAsset", hasDefault = true)
    private AssetFileDescriptor mSourceAsset = null;

    /** The context for the MediaPlayer to resolve the sourceUrl.
     * Make sure this is set before the sourceUrl to avoid unexpected result.
     * If the sourceUrl is not a content URI, it is OK to keep this as null. */
    @GenerateFieldPort(name = "context", hasDefault = true)
    private Context mContext = null;

    /** Whether the media source is a URL or an asset file descriptor. Defaults
     * to false.
     */
    @GenerateFieldPort(name = "sourceIsUrl", hasDefault = true)
    private boolean mSelectedIsUrl = false;

    /** Whether the filter will always wait for a new video frame, or whether it
     * will output an old frame again if a new frame isn't available. Defaults
     * to true.
     */
    @GenerateFinalPort(name = "waitForNewFrame", hasDefault = true)
    private boolean mWaitForNewFrame = true;

    /** Whether the media source should loop automatically or not. Defaults to
     * true.
     */
    @GenerateFieldPort(name = "loop", hasDefault = true)
    private boolean mLooping = true;

    /** Volume control. Currently sound is piped directly to the speakers, so
     * this defaults to mute.
     */
    @GenerateFieldPort(name = "volume", hasDefault = true)
    private float mVolume = 0.f;

    /** Orientation. This controls the output orientation of the video. Valid
     * values are 0, 90, 180, 270
     */
    @GenerateFieldPort(name = "orientation", hasDefault = true)
    private int mOrientation = 0;

    private MediaPlayer mMediaPlayer;
    private GLFrame mMediaFrame;
    private SurfaceTexture mSurfaceTexture;
    private ShaderProgram mFrameExtractor;
    private MutableFrameFormat mOutputFormat;

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

    @GenerateFieldPort(name = "init_offset", hasDefault = true)
    private int mInitOffsetTime = 0;

    // Total timeouts will be PREP_TIMEOUT*PREP_TIMEOUT_REPEAT
    private static final int PREP_TIMEOUT = 100; // ms
    private static final int PREP_TIMEOUT_REPEAT = 300;
    private static final int NEWFRAME_TIMEOUT = 100; //ms
    private static final int NEWFRAME_TIMEOUT_REPEAT = 10;

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
    // Currently, given a device orientation, the MediaSource rotates in such a way
    // that the source is displayed upright. A particular use case
    // is "Background Replacement" feature in the Camera app
    // where the MediaSource rotates the source to align with the camera feed and pass it
    // on to the backdropper filter. The backdropper only does the blending
    // and does not have to do any rotation
    // (except for mirroring in case of front camera).
    // TODO: Currently the rotations are spread over a bunch of stages in the
    // pipeline. A cleaner design
    // could be to cast away all the rotation in a separate filter or attach a transform
    // to the frame so that MediaSource itself need not know about any rotation.
    private static final float[] mSourceCoords_0 = { 1, 1, 0, 1,
                                                     0, 1, 0, 1,
                                                     1, 0, 0, 1,
                                                     0, 0, 0, 1 };
    private static final float[] mSourceCoords_270 = { 0, 1, 0, 1,
                                                      0, 0, 0, 1,
                                                      1, 1, 0, 1,
                                                      1, 0, 0, 1 };
    private static final float[] mSourceCoords_180 = { 0, 0, 0, 1,
                                                       1, 0, 0, 1,
                                                       0, 1, 0, 1,
                                                       1, 1, 0, 1 };
    private static final float[] mSourceCoords_90 = { 1, 0, 0, 1,
                                                       1, 1, 0, 1,
                                                       0, 0, 0, 1,
                                                       0, 1, 0, 1 };

    private static final int[][] mAngleIndex = {
        { 0,  4,  8, 12},
        { 4, 12,  0,  8},
        {12,  8,  4,  0},
        { 8,  0,  12, 4}
    };

    private boolean mGotSize;
    private boolean mPrepared;
    private boolean mPlaying;
    private boolean mNewFrameAvailable;
    private boolean mOrientationUpdated;
    private boolean mPaused;
    private boolean mCompleted;

    private final boolean mLogVerbose;

    public MediaSourceInFilter(String name) {
        super(name);
        mLogVerbose = Log.isLoggable(getClass().getSimpleName(), Log.VERBOSE);
        mTool.log('d', getClass().getSimpleName() + "() " + name);
        mTool.setIDandIncrease(mCount);
        mNewFrameAvailable = false;
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    @Override
    public void setupPorts() {
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
        if (mLogVerbose) mTool.log('d', "Preparing MediaSource");

        mFrameExtractor = new ShaderProgram(context, mFrameShader);
        // SurfaceTexture defines (0,0) to be bottom-left. The filter framework
        // defines (0,0) as top-left, so do the flip here.
        mFrameExtractor.setSourceRect(0, 1, 1, -1);

        createFormats();
    }

    @Override
    public void open(FilterContext context) {
        if (mLogVerbose) {
            mTool.log('d', "Opening MediaSource");
            if (mSelectedIsUrl) {
                mTool.log('d', "Current URL is " + mSourceUrl);
            } else {
                mTool.log('d', "Current source is Asset!");
            }
        }

        mMediaFrame = (GLFrame)context.getFrameManager().newBoundFrame(
                mOutputFormat,
                GLFrame.EXTERNAL_TEXTURE,
                0);

        mSurfaceTexture = new SurfaceTexture(mMediaFrame.getTextureId());

        if (!setupMediaPlayer(mSelectedIsUrl)) {
          throw new RuntimeException("Error setting up MediaPlayer!");
        }

        mOrientationUpdated = true;
    }

    @Override
    public synchronized void process(FilterContext context) {
        // Note: process is synchronized by its caller in the Filter base class
        if (mLogVerbose) mTool.log('d', "Processing new frame");

        if (mMediaPlayer == null) {
            // Something went wrong in initialization or parameter updates
            throw new NullPointerException("Unexpected null media player!");
        }

        if (mCompleted) {
            // Video playback is done, so close us down
            closeOutputPort("video");
            return;
        }

        if (!mPlaying) {
            int waitCount = 0;
            if (mLogVerbose) mTool.log('d', "Waiting for preparation to complete");
            while (!mGotSize || !mPrepared) {
                try {
                    mTool.log('d', "mGotSize:" + mGotSize + ", mPrepared:" + mPrepared);
                    this.wait(PREP_TIMEOUT);
                } catch (InterruptedException e) {
                    // ignoring
                }
                if (mCompleted) {
                    // Video playback is done, so close us down
                    closeOutputPort("video");
                    return;
                }
                waitCount++;
                if (waitCount == PREP_TIMEOUT_REPEAT) {
                    mMediaPlayer.release();
                    throw new RuntimeException("MediaPlayer timed out while preparing!");
                }
            }

            mTool.log('d', "MediaPlayer starting playback offset(" + mInitOffsetTime + ")");
            mMediaPlayer.seekTo(mInitOffsetTime);
            mMediaPlayer.start();
        }

        // Use last frame if paused, unless just starting playback, in which case
        // we want at least one valid frame before pausing
        if (!mPaused || !mPlaying) {
            if (mWaitForNewFrame) {
                if (mLogVerbose) mTool.log('d', "Waiting for new frame");

                int waitCount = 0;
                while (!mNewFrameAvailable) {
                    if (waitCount == NEWFRAME_TIMEOUT_REPEAT) {
                        if (mCompleted) {
                            // Video playback is done, so close us down
                            mTool.log('d', "closeOutputPort(video);");
                            closeOutputPort("video");
                            mTool.log('d', "the filter already completed !");
                            return;
                        } else {
                            mTool.log('d', "ignore this.wait()");
                            return;
                            //throw new RuntimeException("Timeout waiting for new frame!");
                        }
                    }
                    try {
                        this.wait(NEWFRAME_TIMEOUT);
                    } catch (InterruptedException e) {
                        if (mLogVerbose) mTool.log('d', "interrupted");
                        // ignoring
                    }
                    waitCount++;
                }
                mNewFrameAvailable = false;
                if (mLogVerbose) mTool.log('d', "Got new frame");
            }

            if (mCompleted) {
                mTool.log('d', "the filter already completed ! ignore this frame");
                return;
            }

            mSurfaceTexture.updateTexImage();
        }

        if (mOrientationUpdated) {
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
                    setSourceRect(mSourceCoords_90, 0, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                                      surfaceTransform, 0,
                                      sourceCoordsin, 0);
                    break;
                case 180:
                    setSourceRect(mSourceCoords_180, 0, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                                      surfaceTransform, 0,
                                      sourceCoordsin, 0);
                    break;
                case 270:
                    setSourceRect(mSourceCoords_270, 0, sourceCoordsin);
                    Matrix.multiplyMM(sourceCoords, 0,
                                      surfaceTransform, 0,
                                      sourceCoordsin, 0);
                    break;
            }
            if (mLogVerbose) {
                mTool.log('d', "OrientationHint = " + mOrientation);
                String temp = String.format("SetSourceRegion: %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
                        sourceCoords[4], sourceCoords[5],sourceCoords[0], sourceCoords[1],
                        sourceCoords[12], sourceCoords[13],sourceCoords[8], sourceCoords[9]);
                mTool.log('d', temp);
            }
            mFrameExtractor.setSourceRegion(sourceCoords[4], sourceCoords[5],
                    sourceCoords[0], sourceCoords[1],
                    sourceCoords[12], sourceCoords[13],
                    sourceCoords[8], sourceCoords[9]);
            mOrientationUpdated = false;
        }

        Frame output = context.getFrameManager().newFrame(mOutputFormat);
        mFrameExtractor.process(mMediaFrame, output);

        long timestamp = mSurfaceTexture.getTimestamp();
        if (mLogVerbose) mTool.log('d', "Timestamp: " + (timestamp / 1000000000.0) + " s");
        output.setTimestamp(timestamp);

        pushOutput("video", output);
        output.release();

        mPlaying = true;
    }

    public synchronized void setCompleted(){
        mTool.log('w', "setCompleted() " + mCompleted);
        if (mCompleted == false) {
            closeOutputPort("video");
        }
        mCompleted = true;
    }

    @Override
    public void close(FilterContext context) {
        try {
            if (mMediaPlayer.isPlaying()) {
                mMediaPlayer.stop();
            }
        } catch (IllegalStateException e) {
            // ignore state error when closed
            e.printStackTrace();
        }

        mPrepared = false;
        mGotSize = false;
        mPlaying = false;
        mPaused = false;
        mCompleted = false;
        mNewFrameAvailable = false;

        mMediaPlayer.release();
        mMediaPlayer = null;
        mSurfaceTexture.release();
        mSurfaceTexture = null;
        if (mLogVerbose) mTool.log('d', "MediaSource closed");
    }

    @Override
    public void tearDown(FilterContext context) {
        if (mMediaFrame != null) {
            mMediaFrame.release();
        }
    }

    // When updating the port values of the filter, users can update sourceIsUrl to switch
    //   between using URL objects or Assets.
    // If updating only sourceUrl/sourceAsset, MediaPlayer gets reset if the current player
    //   uses Url objects/Asset.
    // Otherwise the new sourceUrl/sourceAsset is stored and will be used when users switch
    //   sourceIsUrl next time.
    @Override
    public void fieldPortValueUpdated(String name, FilterContext context) {
        if (mLogVerbose) mTool.log('d', "Parameter update");
        if (name.equals("sourceUrl")) {
           if (isOpen()) {
                if (mLogVerbose) mTool.log('d', "Opening new source URL");
                if (mSelectedIsUrl) {
                    setupMediaPlayer(mSelectedIsUrl);
                }
            }
        } else if (name.equals("sourceAsset") ) {
            if (isOpen()) {
                if (mLogVerbose) mTool.log('d', "Opening new source FD");
                if (!mSelectedIsUrl) {
                    setupMediaPlayer(mSelectedIsUrl);
                }
            }
        } else if (name.equals("loop")) {
            if (isOpen()) {
                mMediaPlayer.setLooping(mLooping);
            }
        } else if (name.equals("sourceIsUrl")) {
            if (isOpen()){
                if (mSelectedIsUrl){
                    if (mLogVerbose) mTool.log('d', "Opening new source URL");
                } else {
                    if (mLogVerbose) mTool.log('d', "Opening new source Asset");
                }
                setupMediaPlayer(mSelectedIsUrl);
            }
        } else if (name.equals("volume")) {
            if (isOpen()) {
                mMediaPlayer.setVolume(mVolume, mVolume);
            }
        } else if (name.equals("orientation") && mGotSize) {
            if (mOrientation == 0 || mOrientation == 180) {
                mOutputFormat.setDimensions(mWidth, mHeight);
            } else {
                mOutputFormat.setDimensions(mHeight, mWidth);
            }
            mOrientationUpdated = true;
        }
    }

    synchronized public void pauseVideo(boolean pauseState) {
        if (isOpen()) {
            if (pauseState && !mPaused) {
                mMediaPlayer.pause();
            } else if (!pauseState && mPaused) {
                mMediaPlayer.start();
            }
        }
        mPaused = pauseState;
    }

    synchronized public void seekTo(int msec) {
        if (isOpen()) {
            try {
                mMediaPlayer.seekTo(msec);
            } catch(IllegalStateException e) {
                e.printStackTrace();
                // just ignored
            }
        }
    }

    /** Creates a media player, sets it up, and calls prepare */
    synchronized private boolean setupMediaPlayer(boolean useUrl) {
        mPrepared = false;
        mGotSize = false;
        mPlaying = false;
        mPaused = false;
        mCompleted = false;
        mNewFrameAvailable = false;

        if (mLogVerbose) mTool.log('d', "Setting up playback.");

        if (mMediaPlayer != null) {
            // Clean up existing media players
            if (mLogVerbose) mTool.log('d', "Resetting existing MediaPlayer.");
            mMediaPlayer.reset();
        } else {
            // Create new media player
            if (mLogVerbose) mTool.log('d', "Creating new MediaPlayer.");
            mMediaPlayer = new MediaPlayer();
        }

        if (mMediaPlayer == null) {
            throw new RuntimeException("Unable to create a MediaPlayer!");
        }

        // Set up data sources, etc
        try {
            if (useUrl) {
                if (mLogVerbose) mTool.log('d', "Setting MediaPlayer source to URI " + mSourceUrl);
                if (mContext == null) {
                    mMediaPlayer.setDataSource(mSourceUrl);
                } else {
                    mMediaPlayer.setDataSource(mContext, Uri.parse(mSourceUrl.toString()));
                }
            } else {
                if (mLogVerbose) mTool.log('d', "Setting MediaPlayer source to asset " + mSourceAsset);
                mMediaPlayer.setDataSource(mSourceAsset.getFileDescriptor(), mSourceAsset.getStartOffset(), mSourceAsset.getLength());
            }
        } catch(IOException e) {
            e.printStackTrace();
            mMediaPlayer.release();
            mMediaPlayer = null;
            if (useUrl) {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to URL %s!", mSourceUrl), e);
            } else {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to asset %s!", mSourceAsset), e);
            }
        } catch(IllegalArgumentException e) {
            e.printStackTrace();
            mMediaPlayer.release();
            mMediaPlayer = null;
            if (useUrl) {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to URL %s!", mSourceUrl), e);
            } else {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to asset %s!", mSourceAsset), e);
            }
        } catch (SecurityException e) {
            e.printStackTrace();
            mMediaPlayer.release();
            mMediaPlayer = null;
            if (useUrl) {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to URL %s!", mSourceUrl), e);
            } else {
                throw new RuntimeException(String.format("Unable to set MediaPlayer to asset %s!", mSourceAsset), e);
            }
        }

        mMediaPlayer.setLooping(mLooping);
        mMediaPlayer.setVolume(mVolume, mVolume);

        // Bind it to our media frame
        Surface surface = new Surface(mSurfaceTexture);
        mMediaPlayer.setSurface(surface);
        surface.release();

        // Connect Media Player to callbacks

        mMediaPlayer.setOnVideoSizeChangedListener(onVideoSizeChangedListener);
        mMediaPlayer.setOnPreparedListener(onPreparedListener);
        mMediaPlayer.setOnCompletionListener(onCompletionListener);

        // Connect SurfaceTexture to callback
        mSurfaceTexture.setOnFrameAvailableListener(onMediaFrameAvailableListener);

        if (mLogVerbose) mTool.log('d', "Preparing MediaPlayer.");

        mMediaPlayer.setParameter(CLEAR_MOTION_KEY, CLEAR_MOTION_DISABLE);

        mMediaPlayer.prepareAsync();
        return true;
    }

    private MediaPlayer.OnVideoSizeChangedListener onVideoSizeChangedListener =
            new MediaPlayer.OnVideoSizeChangedListener() {
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
            if (mLogVerbose) mTool.log('d', "MediaPlayer sent dimensions: " + width + " x " + height);
            mInputWidth = width;
            mInputHeight = height;
            synchronized(MediaSourceInFilter.this) {
                mGotSize = true;
                MediaSourceInFilter.this.notify();
            }
        }
    };

    private MediaPlayer.OnPreparedListener onPreparedListener =
            new MediaPlayer.OnPreparedListener() {
        public void onPrepared(MediaPlayer mp) {
            if (mLogVerbose) mTool.log('d', "MediaPlayer is prepared");
            synchronized(MediaSourceInFilter.this) {
                mPrepared = true;
                MediaSourceInFilter.this.notify();
            }
        }
    };

    private MediaPlayer.OnCompletionListener onCompletionListener =
            new MediaPlayer.OnCompletionListener() {
        public void onCompletion(MediaPlayer mp) {
            if (mLogVerbose) mTool.log('d', "MediaPlayer has completed playback");
            synchronized(MediaSourceInFilter.this) {
                mCompleted = true;
            }
        }
    };

    private SurfaceTexture.OnFrameAvailableListener onMediaFrameAvailableListener =
            new SurfaceTexture.OnFrameAvailableListener() {
        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
            if (mLogVerbose) mTool.log('d', "New frame from media player");
            synchronized(MediaSourceInFilter.this) {
                if (mLogVerbose) mTool.log('d', "New frame: notify");
                mNewFrameAvailable = true;
                MediaSourceInFilter.this.notify();
                if (mLogVerbose) mTool.log('d', "New frame: notify done");
            }
        }
    };

}
