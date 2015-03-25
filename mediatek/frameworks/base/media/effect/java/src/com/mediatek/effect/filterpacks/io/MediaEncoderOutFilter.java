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

import android.filterfw.core.Filter;
import android.filterfw.core.FilterContext;
import android.filterfw.core.Frame;
import android.filterfw.core.FrameFormat;
import android.filterfw.core.GenerateFieldPort;
import android.filterfw.core.GLFrame;
import android.filterfw.core.MutableFrameFormat;
import android.filterfw.core.ShaderProgram;
import android.filterfw.format.ImageFormat;
import android.filterfw.geometry.Point;
import android.filterfw.geometry.Quad;
import android.media.MediaRecorder;
import android.media.CamcorderProfile;
import android.filterfw.core.GLEnvironment;
import android.filterpacks.videosink.MediaRecorderStopException;
import android.graphics.Bitmap;

import java.io.IOException;
import java.io.FileDescriptor;
import java.util.Timer;
import java.util.TimerTask;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.player.EffectPlayer;
import com.mediatek.media.MediaRecorderEx;

/**
 * @hide
 */
public class MediaEncoderOutFilter extends Filter {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    /** User-visible parameters */

    /** Recording state. When set to false, recording will stop, or will not
     * start if not yet running the graph. Instead, frames are simply ignored.
     * When switched back to true, recording will restart. This allows a single
     * graph to both provide preview and to record video. If this is false,
     * recording settings can be updated while the graph is running.
     */
    @GenerateFieldPort(name = "recording", hasDefault = true)
    private boolean mRecording = true;

    /** Filename to save the output. */
    @GenerateFieldPort(name = "outputFile", hasDefault = true)
    private String mOutputFile = new String("/sdcard/MediaEncoderOut.mp4");

    /** File Descriptor to save the output. */
    @GenerateFieldPort(name = "outputFileDescriptor", hasDefault = true)
    private FileDescriptor mFd = null;

    /** Input audio source. If not set, no audio will be recorded.
     * Select from the values in MediaRecorder.AudioSource
     */
    @GenerateFieldPort(name = "audioSource", hasDefault = true)
    private int mAudioSource = NO_AUDIO_SOURCE;

    /** Media recorder info listener, which needs to implement
     * MediaRecorder.OnInfoListener. Set this to receive notifications about
     * recording events.
     */
    @GenerateFieldPort(name = "infoListener", hasDefault = true)
    private MediaRecorder.OnInfoListener mInfoListener = null;

    /**
     * Add switch for disable audio in recording.
     */
    @GenerateFieldPort(name = "muteAudio", hasDefault = true)
    private boolean mMuteAudio = true;

    /**
     * Media recorder camera device released listener, which needs to implement
     * MediaRecorder.OnInfoListener. Set this to receive notifications about
     * mediaRecorder has released camera device now.
     */
    @GenerateFieldPort(name = "releasedListener", hasDefault = true)
    private MediaRecorder.OnInfoListener mCameraReleasedListener = null;

    /** Media recorder error listener, which needs to implement
     * MediaRecorder.OnErrorListener. Set this to receive notifications about
     * recording errors.
     */
    @GenerateFieldPort(name = "errorListener", hasDefault = true)
    private MediaRecorder.OnErrorListener mErrorListener = null;

    /** Media recording done callback, which needs to implement OnRecordingDoneListener.
     * Set this to finalize media upon completion of media recording.
     */
    @GenerateFieldPort(name = "recordingDoneListener", hasDefault = true)
    private OnRecordingDoneListener mRecordingDoneListener = null;

    /** Orientation hint. Used for indicating proper video playback orientation.
     * Units are in degrees of clockwise rotation, valid values are (0, 90, 180,
     * 270).
     */
    @GenerateFieldPort(name = "orientationHint", hasDefault = true)
    private int mOrientationHint = 0;

    /** Camcorder profile to use. Select from the profiles available in
     * android.media.CamcorderProfile. If this field is set, it overrides
     * settings to width, height, framerate, outputFormat, and videoEncoder.
     */
    @GenerateFieldPort(name = "recordingProfile", hasDefault = true)
    private CamcorderProfile mProfile = null;

    /** Frame width to be encoded, defaults to 320.
     * Actual received frame size has to match this */
    @GenerateFieldPort(name = "width", hasDefault = true)
    private int mWidth = 0;

    /** Frame height to to be encoded, defaults to 240.
     * Actual received frame size has to match */
    @GenerateFieldPort(name = "height", hasDefault = true)
    private int mHeight = 0;

    /** Stream framerate to encode the frames at.
     * By default, frames are encoded at 30 FPS*/
    @GenerateFieldPort(name = "framerate", hasDefault = true)
    private int mFps = 30;

    /** Stream framerate to encode the frames at.
     * By default, frames are encoded at 30 FPS*/
    @GenerateFieldPort(name = "bitrate", hasDefault = true)
    private int mBitRate = 8000000;

    /** The output format to encode the frames in.
     * Choose an output format from the options in
     * android.media.MediaRecorder.OutputFormat */
    @GenerateFieldPort(name = "outputFormat", hasDefault = true)
    private int mOutputFormat = MediaRecorder.OutputFormat.MPEG_4;

    /** The videoencoder to encode the frames with.
     * Choose a videoencoder from the options in
     * android.media.MediaRecorder.VideoEncoder */
    @GenerateFieldPort(name = "videoEncoder", hasDefault = true)
    private int mVideoEncoder = MediaRecorder.VideoEncoder.H264;

    /** The input region to read from the frame. The corners of this quad are
     * mapped to the output rectangle. The input frame ranges from (0,0)-(1,1),
     * top-left to bottom-right. The corners of the quad are specified in the
     * order bottom-left, bottom-right, top-left, top-right.
     */
    @GenerateFieldPort(name = "inputRegion", hasDefault = true)
    private Quad mSourceRegion;

    /** The maximum filesize (in bytes) of the recording session.
     * By default, it will be 0 and will be passed on to the MediaRecorder.
     * If the limit is zero or negative, MediaRecorder will disable the limit*/
    @GenerateFieldPort(name = "maxFileSize", hasDefault = true)
    private long mMaxFileSize = 0;

    /** The maximum duration (in milliseconds) of the recording session.
     * By default, it will be 0 and will be passed on to the MediaRecorder.
     * If the limit is zero or negative, MediaRecorder will record indefinitely*/
    @GenerateFieldPort(name = "maxDurationMs", hasDefault = true)
    private int mMaxDurationMs = 0;

    @GenerateFieldPort(name = "livephoto", hasDefault = true)
    private boolean mIsLivePhoto = false;

    /** TimeLapse Interval between frames.
     * By default, it will be 0. Whether the recording is timelapsed
     * is inferred based on its value being greater than 0 */
    @GenerateFieldPort(name = "timelapseRecordingIntervalUs", hasDefault = true)
    private long mTimeBetweenTimeLapseFrameCaptureUs = 0;

    @GenerateFieldPort(name = "isFromMediaPlayer", hasDefault = true)
    private boolean mIsFromMediaPlayer = false;

    @GenerateFieldPort(name = "effectplayer", hasDefault = true)
    private EffectPlayer mEffectPlayer = null;
    @GenerateFieldPort(name = "endtime", hasDefault = true)
    private long mEndTime = 0;

    // End of user visible parameters

    private static final int NO_AUDIO_SOURCE = -1;

    private int mSurfaceId;
    private ShaderProgram mProgram;
    private GLFrame mScreen;

    private boolean mRecordingActive = false;
    private long mTimestampNs = 0;
    private long mLastTimeLapseFrameRealTimestampNs = 0;
    private int mNumFramesEncoded = 0;
    // Used to indicate whether recording is timelapsed.
    // Inferred based on (mTimeBetweenTimeLapseFrameCaptureUs > 0)
    private boolean mCaptureTimeLapse = false;

    private boolean mLogVerbose;

    private boolean isSubmitFirstClose = false;
    private boolean isRunFirstClose = false;

    // Our hook to the encoder
    private MediaRecorder mMediaRecorder;

    /** Callback to be called when media recording completes. */

    public interface OnRecordingDoneListener {
        public void onRecordingDone();
    }

    public MediaEncoderOutFilter(String name) {
        super(name);
        mLogVerbose = true; //Log.isLoggable(TAG, Log.VERBOSE);
        mTool.log('d', getClass().getSimpleName() + "() " + name);
        mTool.setIDandIncrease(mCount);

        Point bl = new Point(0, 0);
        Point br = new Point(1, 0);
        Point tl = new Point(0, 1);
        Point tr = new Point(1, 1);
        mSourceRegion = new Quad(bl, br, tl, tr);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "()");
        super.finalize();
    }

    @Override
    public void setupPorts() {
        // Add input port- will accept RGBA GLFrames
        addMaskedInputPort("videoframe", ImageFormat.create(ImageFormat.COLORSPACE_RGBA,
                                                      FrameFormat.TARGET_GPU));
    }

    @Override
    public void fieldPortValueUpdated(String name, FilterContext context) {
        if (mLogVerbose) mTool.log('d', "Port " + name + " has been updated");
        if (name.equals("recording")) return;
        if (name.equals("inputRegion")) {
            if (isOpen()) updateSourceRegion();
            return;
        }
        // TODO: Not sure if it is possible to update the maxFileSize
        // when the recording is going on. For now, not doing that.
        if (isOpen() && mRecordingActive) {
            throw new RuntimeException("Cannot change recording parameters"
                                       + " when the filter is recording!");
        }
    }

    private void updateSourceRegion() {
        // Flip source quad to map to OpenGL origin
        Quad flippedRegion = new Quad();
        flippedRegion.p0 = mSourceRegion.p2;
        flippedRegion.p1 = mSourceRegion.p3;
        flippedRegion.p2 = mSourceRegion.p0;
        flippedRegion.p3 = mSourceRegion.p1;
        mProgram.setSourceRegion(flippedRegion);
    }

    // update the MediaRecorderParams based on the variables.
    // These have to be in certain order as per the MediaRecorder
    // documentation
    private void updateMediaRecorderParams() {
        mCaptureTimeLapse = mTimeBetweenTimeLapseFrameCaptureUs > 0;
        final int GRALLOC_BUFFER = 2;
        mMediaRecorder.setVideoSource(GRALLOC_BUFFER);
        if (!mCaptureTimeLapse && (mAudioSource != NO_AUDIO_SOURCE) && !mMuteAudio) {
            mMediaRecorder.setAudioSource(mAudioSource);
        }
        if (mProfile != null) {
            //mMediaRecorder.setProfile(mProfile);
            mMediaRecorder.setOutputFormat(mProfile.fileFormat);
            mMediaRecorder.setVideoFrameRate(mProfile.videoFrameRate);
            // If width and height are set larger than 0, then those
            // overwrite the ones in the profile.
            if (mWidth > 0 && mHeight > 0) {
                mMediaRecorder.setVideoSize(mWidth, mHeight);
            } else {
            	mMediaRecorder.setVideoSize(mProfile.videoFrameWidth, mProfile.videoFrameHeight);
            }
            mMediaRecorder.setVideoEncodingBitRate(mProfile.videoBitRate);
            mMediaRecorder.setVideoEncoder(mProfile.videoCodec);
            if (!mMuteAudio && !mCaptureTimeLapse) {
                mMediaRecorder.setAudioEncodingBitRate(mProfile.audioBitRate);
                mMediaRecorder.setAudioChannels(mProfile.audioChannels);
                mMediaRecorder.setAudioSamplingRate(mProfile.audioSampleRate);
                mMediaRecorder.setAudioEncoder(mProfile.audioCodec);
            }
            mFps = mProfile.videoFrameRate;
        } else {
            mMediaRecorder.setVideoEncodingBitRate(mBitRate);
            mMediaRecorder.setOutputFormat(mOutputFormat);
            mMediaRecorder.setVideoEncoder(mVideoEncoder);
            mMediaRecorder.setVideoSize(mWidth, mHeight);
            mMediaRecorder.setVideoFrameRate(mFps);
        }
        mMediaRecorder.setOrientationHint(mOrientationHint);
        mMediaRecorder.setOnInfoListener(mInfoListener);
        //mMediaRecorder.setOnCameraReleasedListener(mCameraReleasedListener);
        mMediaRecorder.setOnErrorListener(mErrorListener);
        if (mFd != null) {
            mMediaRecorder.setOutputFile(mFd);
        } else {
            mMediaRecorder.setOutputFile(mOutputFile);
        }
        try {
            mMediaRecorder.setMaxFileSize(mMaxFileSize);
        } catch (Exception e) {
            // Following the logic in  VideoCamera.java (in Camera app)
            // We are going to ignore failure of setMaxFileSize here, as
            // a) The composer selected may simply not support it, or
            // b) The underlying media framework may not handle 64-bit range
            // on the size restriction.
            mTool.log('w', "Setting maxFileSize on MediaRecorder unsuccessful! "
                    + e.getMessage());
        }
        mMediaRecorder.setMaxDuration(mMaxDurationMs);

        if (mIsLivePhoto) {
            MediaRecorderEx.setLivePhotoTag(mMediaRecorder, 1);
        }
    }

    @Override
    public void prepare(FilterContext context) {
        if (mLogVerbose) mTool.log('d', "Preparing");

        mProgram = ShaderProgram.createIdentity(context);

        mRecordingActive = false;
    }

    @Override
    public void open(FilterContext context) {
        if (mLogVerbose) mTool.log('d', "Opening");
        updateSourceRegion();
        if (mRecording) startRecording(context);
        isSubmitFirstClose = false;
        isRunFirstClose = false;
        mContext = context;
    }

    private void startRecording(FilterContext context) {
        mTool.log('d', "Starting recording");

        // Create a frame representing the screen
        MutableFrameFormat screenFormat = new MutableFrameFormat(
                              FrameFormat.TYPE_BYTE, FrameFormat.TARGET_GPU);
        screenFormat.setBytesPerSample(4);

        int width, height;
        boolean widthHeightSpecified = mWidth > 0 && mHeight > 0;
        // If width and height are specified, then use those instead
        // of that in the profile.
        if (mProfile != null && !widthHeightSpecified) {
            width = mProfile.videoFrameWidth;
            height = mProfile.videoFrameHeight;
        } else {
            width = mWidth;
            height = mHeight;
        }
        screenFormat.setDimensions(width, height);
        mScreen = (GLFrame)context.getFrameManager().newBoundFrame(
                           screenFormat, GLFrame.EXISTING_FBO_BINDING, 0);

        // Initialize the media recorder

        mMediaRecorder = new MediaRecorder();
        updateMediaRecorderParams();

        mMediaRecorder.setOnInfoListener(new MediaRecorder.OnInfoListener() {
            @Override
            public void onInfo(MediaRecorder mr, int what, int extra) {
                mTool.log('d', "onInfo(" + what + ", " + extra + ")");
                if (what > 100000) {
                    mEffectPlayer.submit(new Runnable() {
                        @Override
                        public void run() {
                            Thread.currentThread().setName("[" + mTool.getID() + "] Encoder close Thread");
                            mTool.log('w', "Encoder close Thread part 2: close graph");
                            mEffectPlayer.graphClose();
                        }
                    });
                }
            }
        });

        try {
            mMediaRecorder.prepare();
        } catch (IllegalStateException e) {
            throw e;
        } catch (IOException e) {
            throw new RuntimeException("IOException in"
                    + "MediaRecorder.prepare()!", e);
        } catch (Exception e) {
            throw new RuntimeException("Unknown Exception in"
                    + "MediaRecorder.prepare()!", e);
        }
        // Make sure start() is called before trying to
        // register the surface. The native window handle needed to create
        // the surface is initiated in start()
        mMediaRecorder.start();
        recorderStartTimestampNs = System.nanoTime(); // set start timestamp
        mSurfaceId = context.getGLEnvironment().
                registerSurfaceFromMediaRecorder(mMediaRecorder);
        if (mLogVerbose) mTool.log('d', "Open: registering surface " + mSurfaceId + " from Mediarecorder");
        mNumFramesEncoded = 0;
        mRecordingActive = true;
        // extra code is -1 means MediaRecorder has been started
        // and Application needs to listen MediaRecorder.MEDIA_RECORDER_INFO_CAMERA_RELEASE info.
        if (mCameraReleasedListener != null) {
            mCameraReleasedListener.onInfo(mMediaRecorder,
                    MediaRecorder.MEDIA_RECORDER_ERROR_UNKNOWN, -1);
        }
    }

    public boolean skipFrameAndModifyTimestamp(long timestampNs) {
        if (mLogVerbose) mTool.log('d', "skipFrameAndModifyTimestamp(" + timestampNs + ")" +
                ", recorderStartTimestampNs=" + recorderStartTimestampNs);
        // skip pre-time frame if media recorder has been started.
        if (timestampNs < recorderStartTimestampNs) {
            return true;
        }
        // first frame- encode. Don't skip
        if (mNumFramesEncoded == 0) {
            mLastTimeLapseFrameRealTimestampNs = timestampNs;
            mTimestampNs = timestampNs;
            if (mLogVerbose) mTool.log('d', "timelapse: FIRST frame, last real t= "
                    + mLastTimeLapseFrameRealTimestampNs +
                    ", setting t = " + mTimestampNs );
            return false;
        }

        // Workaround to bypass the first 2 input frames for skipping.
        // The first 2 output frames from the encoder are: decoder specific info and
        // the compressed video frame data for the first input video frame.
        if (mNumFramesEncoded >= 2 && timestampNs <
            (mLastTimeLapseFrameRealTimestampNs +  1000L * mTimeBetweenTimeLapseFrameCaptureUs)) {
            // If 2 frames have been already encoded,
            // Skip all frames from last encoded frame until
            // sufficient time (mTimeBetweenTimeLapseFrameCaptureUs) has passed.
            if (mLogVerbose) mTool.log('d', "timelapse: skipping intermediate frame");
            return true;
        } else {
            // Desired frame has arrived after mTimeBetweenTimeLapseFrameCaptureUs time:
            // - Reset mLastTimeLapseFrameRealTimestampNs to current time.
            // - Artificially modify timestampNs to be one frame time (1/framerate) ahead
            // of the last encoded frame's time stamp.
            if (mLogVerbose) mTool.log('d', "timelapse: encoding frame, Timestamp t = " + timestampNs +
                    ", last real t= " + mLastTimeLapseFrameRealTimestampNs +
                    ", interval = " + mTimeBetweenTimeLapseFrameCaptureUs);
            mLastTimeLapseFrameRealTimestampNs = timestampNs;
            mTimestampNs = mTimestampNs + (1000000000L / (long)mFps);
            if (mLogVerbose) mTool.log('d', "timelapse: encoding frame, setting t = "
                    + mTimestampNs + ", delta t = " + (1000000000L / (long)mFps) +
                    ", fps = " + mFps );
            return false;
        }
    }

    @Override
    public synchronized void process(FilterContext context) {
        GLEnvironment glEnv = context.getGLEnvironment();
        // Get input frame
        Frame input = pullInput("videoframe");

        // Check if recording needs to start
        if (!mRecordingActive && mRecording) {
            startRecording(context);
        }
        // Check if recording needs to stop
        if (mRecordingActive && !mRecording) {
            stopRecording(context);
        }

        if (!mRecordingActive) {
            //closeOutputPort("videoframe");
            this.getInputPort("videoframe").close();
            return;
        }

        if (isSubmitFirstClose == true) {
            return;
        }

        if (mCaptureTimeLapse) {
            if (skipFrameAndModifyTimestamp(input.getTimestamp())) {
                return;
            }
        } else {
            mTimestampNs = input.getTimestamp();
        }

        if (mIsFromMediaPlayer) {
            // finished all the effect recording
            if (mEndTime > 0 && mEndTime < mTimestampNs) {
                mTool.log('w', mTimestampNs + " stop recording !");
                isSubmitFirstClose = true;
            }
            mTimestampNs = mTimestampNs + recorderStartTimestampNs;
        }

        // Activate our surface
        glEnv.activateSurfaceWithId(mSurfaceId);

        // Process
        mProgram.process(input, mScreen);

        // Set timestamp from input
        glEnv.setSurfaceTimestamp(mTimestampNs);

        // And swap buffers
        glEnv.swapBuffers();
        mNumFramesEncoded++;

        // to be finished
        if (isSubmitFirstClose == true && isRunFirstClose == false) {
            stopPrepare();
            mTool.log('w', "Encoder close Thread part 1: wait OnInfo()");
            isRunFirstClose = true;
        }
    }

    private FilterContext mContext;
    public synchronized void stopPrepare() {
        GLEnvironment glEnv = mContext.getGLEnvironment();

        Bitmap bb = mScreen.getBitmap();
        bb.recycle();

        mTool.log('e', String.format("Unregistering surface %d", mSurfaceId));
        glEnv.unregisterSurfaceId(mSurfaceId);

        mScreen.release();
        mScreen = null;
    }

    private synchronized void stopRecording(FilterContext context) {
        mTool.log('d', "Stopping recording");

        mRecordingActive = false;
        mNumFramesEncoded = 0;
        recorderStartTimestampNs = 0; // reset media recorder start time

        if (isSubmitFirstClose != true) {
            GLEnvironment glEnv = context.getGLEnvironment();
            // The following call will switch the surface_id to 0
            // (thus, calling eglMakeCurrent on surface with id 0) and
            // then call eglDestroy on the surface. Hence, this will
            // call disconnect the SurfaceMediaSource, which is needed to
            // be called before calling Stop on the mediarecorder
            mTool.log('d', String.format("Unregistering surface %d", mSurfaceId));
            glEnv.unregisterSurfaceId(mSurfaceId);
        }

        try {
            mMediaRecorder.stop();
        } catch (RuntimeException e) {
            mTool.log('e', "MediaRecorder.stop() failed!");
            throw new MediaRecorderStopException("MediaRecorder.stop() failed!", e);
        }
        mMediaRecorder.release();
        mMediaRecorder = null;

        if (isSubmitFirstClose != true) {
            mScreen.release();
            mScreen = null;
        }

        // Use an EffectsRecorder callback to forward a media finalization
        // call so that it creates the video thumbnail, and whatever else needs
        // to be done to finalize media.
        if (mRecordingDoneListener != null) {
            mRecordingDoneListener.onRecordingDone();
        }
    }

    @Override
    public void close(FilterContext context) {
        mTool.log('d', "Closing");
        if (mRecordingActive) stopRecording(context);
        mContext = null;
    }

    @Override
    public void tearDown(FilterContext context) {
        mTool.log('d', "tearDown()");
        // Release all the resources associated with the MediaRecorder
        // and GLFrame members
        if (mMediaRecorder != null) {
            mMediaRecorder.release();
        }
        if (mScreen != null) {
            mScreen.release();
        }
    }

    private long recorderStartTimestampNs = 0;
}
