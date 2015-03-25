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

import com.mediatek.effect.filterpacks.ProcessDoneListener;
import com.mediatek.effect.filterpacks.VideoEventFilter;

import android.content.Context;
import android.filterfw.GraphEnvironment;
import android.filterfw.core.Filter;
import android.filterfw.core.FilterGraph;
import android.filterfw.core.FilterSurfaceView;
import android.filterfw.core.GLEnvironment;
import android.filterfw.core.GraphRunner;
import android.filterfw.core.GraphRunner.OnRunnerDoneListener;
import android.filterfw.io.GraphIOException;
import android.graphics.SurfaceTexture;
import android.media.CamcorderProfile;
import android.util.Log;
import android.view.TextureView;

import java.util.Arrays;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.mediatek.effect.filterpacks.MyUtility;
import com.mediatek.effect.filterpacks.ve.VideoEvent;
import com.mediatek.effect.filterpacks.ve.VideoScenario;
import com.mediatek.effect.filterpacks.io.MediaEncoderOutFilter;
import com.mediatek.effect.filterpacks.io.MediaSourceInFilter;
import com.mediatek.effect.filterpacks.io.SurfaceTextureInFilter;
import com.mediatek.effect.filterpacks.io.TextureViewOutFilter;

/**
 * @hide
 */
public class EffectCore {
    private static int[] mCount = {0};
    private MyUtility mTool = new MyUtility(getClass().getSimpleName(), mCount);

    public static final int FILTER_BLACKWHITE       = 1;
    public static final int FILTER_COLORTEMPERATURE = 2;
    public static final int FILTER_CROSSPROCESS     = 3;
    public static final int FILTER_DOCUMENTARY      = 4;
    public static final int FILTER_DUOTONE          = 5;
    public static final int FILTER_FILLLIGHT        = 6;
    public static final int FILTER_FISHEYE          = 7;
    public static final int FILTER_ROTATION         = 8;
    public static final int FILTER_FLIP             = 9;
    public static final int FILTER_GRAIN            = 10;
    public static final int FILTER_LOMOISH          = 11;
    public static final int FILTER_NEGATIVE         = 12;
    public static final int FILTER_POSTERIZE        = 13;
    public static final int FILTER_SATURATE         = 14;
    public static final int FILTER_SEPIA            = 15;
    public static final int FILTER_SHARPEN          = 16;
    public static final int FILTER_TINT             = 17;
    public static final int FILTER_GRAY             = 18;
    public static final int FILTER_VIGNETTE         = 19;

    // normal maximum
    public static final int MAX_NORMAL_FILTER_EFFECT = FILTER_VIGNETTE;

    public static final int FILTER_GF_SQUEEZE       = 20;
    public static final int FILTER_GF_BIG_EYES      = 21;
    public static final int FILTER_GF_BIG_MOUTH     = 22;
    public static final int FILTER_GF_SMALL_MOUTH   = 23;
    public static final int FILTER_GF_BIG_NOSE      = 24;
    public static final int FILTER_GF_SMALL_EYES    = 25;

    // should be equal to the latest effect item
    public static final int MAX_FILTER_EFFECT       = FILTER_GF_SMALL_EYES;

    // special filter !!
    public static final int FILTER_RANDOMIZE_EFFECT = 26;
    public static final int FILTER_VIDEO_TRANSITION = 27;
    public static final int FILTER_PROCESS_MAX      = FILTER_VIDEO_TRANSITION;

    private int mFilterEffectSettingValue = 0;
    private int mFilterRealEffect = 0;

    private GraphEnvironment mGraphEnv = null;
    private GraphRunner mRunner = null;
    private String mGraphString = "";

    private MediaEncoderOutFilter mEncoderFilter = null;
    private SurfaceTextureInFilter mSourceFilter = null;
    private SurfaceTexture mSurfaceTexture = null; // input SurfaceTexture

    private FilterSurfaceView mSurfaceView = null;
    private TextureView mTextureView = null;
    private TextureView mTextureViewOriginalImg = null;

    private EffectUiHandler mUIHandler = null;

    private ProcessDoneListener mProcessDone = null;

    /*
     * Output size
     */
    private int mWidth;
    private int mHeight;

    /*
     * Intput size
     */
    private int mInputWidth = 0;
    private int mInputHeight = 0;
    private boolean mIgnoreMainFrameStreem = false;

    private CamcorderProfile mRecorderProfile = null;
    private String mRecordingPath = null;
    private boolean mIsFromMediaPlayer = false;

    private VideoScenario mTransitionScenario = new VideoScenario();

    public boolean isGraphRunning = false;
    protected ExecutorService mExecutorService = null;

    private EffectPlayer mEffectPlayer;

    public EffectCore(int width, int height, EffectPlayer player) {
        super();
        mTool.log('d', getClass().getSimpleName() + "() " + width + "x" + height);
        mTool.setIDandIncrease(mCount);

        setOntputSize(width, height);

        mEffectPlayer = player;

        mExecutorService = Executors.newFixedThreadPool(2);
    }

    @Override
    public void finalize() throws Throwable {
        mTool.log('d', "~" + getClass().getSimpleName() + "() " + mWidth + "x" + mHeight);
        if (null != mExecutorService) {
            mExecutorService.shutdownNow();
            mExecutorService = null;
        }
        super.finalize();
    }

    public Context mCntx;
    public String mEffectVideoUri;
    public void setResourceContext(Context cntx, String video) {
        mCntx = cntx;
        mEffectVideoUri = video;
    }

    public static boolean isSupport(int effect) {
        final String TAG = "EffectCoreUtility";

        Log.d(TAG, "Effect: " + effect);
        if (FILTER_GF_SQUEEZE <= effect && effect <= FILTER_GF_SMALL_EYES) {
            boolean Support = isAvailable("com.google.android.filterpacks.facedetect.GoofyRenderFilter");
            Log.d(TAG, "Effect: " + effect + ", isSupport: " + Support);
            return Support;
        }
        return true;
    }

    public static boolean isGMSGoofySupport() {
        return isSupport(FILTER_GF_SMALL_EYES);
    }

    public void checkEffect() {
        // set the effect filter
        if (getGraphEffectSetting() == FILTER_RANDOMIZE_EFFECT) {
            if (isGMSGoofySupport() == true) {
                setEffectInternal((int) (Math.random() * MAX_FILTER_EFFECT) + 1);
            } else {
                setEffectInternal((int) (Math.random() * MAX_NORMAL_FILTER_EFFECT) + 1);
            }
        } else {
            setEffectInternal(getGraphEffectSetting());
        }
    }

    private void setEffectInternal(int effect) {
        mTool.log('d', "setEffectInternal (" + effect + ")");

        if (FILTER_GF_SQUEEZE <= effect && effect <= FILTER_GF_SMALL_EYES) {
            boolean aaa = isGMSGoofySupport();
            mTool.log('d', "Google Facedetect Packs: " + aaa);
            if (!aaa) {
                mFilterRealEffect = 0;
                return;
            }
        }
        mFilterRealEffect = effect;
    }

    public boolean setEffect(int effect) {
        mTool.log('d', "setEffect (" + effect + ")");

        if (FILTER_GF_SQUEEZE <= effect && effect <= FILTER_GF_SMALL_EYES) {
            boolean aaa = isGMSGoofySupport();
            mTool.log('d', "Google Facedetect Packs: " + aaa);
            if (!aaa) {
                mTool.log('d', "setEffect changed to randomization !");
                mFilterEffectSettingValue = FILTER_RANDOMIZE_EFFECT;
                return true;
            }
        } else if (effect > FILTER_PROCESS_MAX/*FILTER_RANDOMIZE_EFFECT*/) {
            return false;
        }

        mFilterEffectSettingValue = effect;
        return true;
    }

    private boolean hasViewForOutput() {
        if (mSurfaceView != null || mTextureView != null) {
            return true;
        }
        return false;
    }

    private String getGraphOutputViewString(int width, int height, String outName) {
        String outputFilter = "";

        if (mSurfaceView != null) {
            outputFilter = "@filter SurfaceViewOutFilter " + outName.trim() + " {\n"
                + "  surfaceView = $OutputSurfaceView;\n"
                + "}\n\n";
        } else if (mTextureView != null) {
            outputFilter = "@filter TextureViewOutFilter " + outName.trim() + " {\n"
                + "  textureView = $OutputTextureView;\n"
                + "  width = " + width + ";\n"
                + "  height = " + height + ";\n"
                + "}\n\n";
        }

        return outputFilter;
    }

    public FilterSurfaceView getSurfaceView() {
        return mSurfaceView;
    }

    public TextureView getTextureView() {
        return mTextureView;
    }

    public TextureView getTextureViewOriginal() {
        return mTextureViewOriginalImg;
    }

    public int getGraphEffectSetting() {
        return mFilterEffectSettingValue;
    }

    public int getGraphEffect() {
        return mFilterRealEffect;
    }

    public String getGraphEffectName() {
        return getGraphEffectName(mFilterRealEffect);
    }

    public static String getGraphEffectName(int filter) {
        String name = "";
        switch(filter) {
            case FILTER_BLACKWHITE:
                name = "BLACKWHITE";
                break;

            case FILTER_COLORTEMPERATURE:
                name = "COLORTEMPERATURE";
                break;

            case FILTER_CROSSPROCESS:
                name = "CROSSPROCESS";
                break;

            case FILTER_DOCUMENTARY:
                name = "DOCUMENTARY";
                break;

            case FILTER_DUOTONE:
                name = "DUOTONE";
                break;

            case FILTER_FILLLIGHT:
                name = "FILLLIGHT";
                break;

            case FILTER_FISHEYE:
                name = "FISHEYE";
                break;

            case FILTER_ROTATION:
                name = "ROTATION";
                break;

            case FILTER_FLIP:
                name = "FLIP";
                break;

            case FILTER_GRAIN:
                name = "GRAIN";
                break;

            case FILTER_LOMOISH:
                name = "LOMO";
                break;

            case FILTER_NEGATIVE:
                name = "NEGATIVE";
                break;

            case FILTER_POSTERIZE:
                name = "POSTERIZE";
                break;

            case FILTER_SATURATE:
                name = "SATURATE";
                break;

            case FILTER_SEPIA:
                name = "SEPIA";
                break;

            case FILTER_SHARPEN:
                name = "SHARPEN";
                break;

            case FILTER_TINT:
                name = "TINT";
                break;

            case FILTER_GRAY:
                name = "GRAY";
                break;

            case FILTER_VIGNETTE:
                name = "VIGNETTE";
                break;

            case FILTER_GF_SQUEEZE:
                name = "SQUEEZE";
                break;

            case FILTER_GF_BIG_EYES:
                name = "BIG_EYES";
                break;

            case FILTER_GF_BIG_MOUTH:
                name = "BIG_MOUTH";
                break;

            case FILTER_GF_SMALL_MOUTH:
                name = "SMALL_MOUTH";
                break;

            case FILTER_GF_BIG_NOSE:
                name = "BIG_NOSE";
                break;

            case FILTER_GF_SMALL_EYES:
                name = "SMALL_EYES";
                break;

            case FILTER_RANDOMIZE_EFFECT:
                name = "RANDOMIZE";
                break;

            case FILTER_VIDEO_TRANSITION:
                name = "TRANSITION";
                break;
        }

        return name;
    }


    private String getGraphEffectStringByIndex(int filter, int width, int height) {
        String graph = "";
        switch(filter) {
            case FILTER_BLACKWHITE:
                graph = "@filter BlackWhiteFilter filterA {\n" +
                    "  black = 0.3f;" +
                    "  white = 0.5f;" +
                    "}\n\n";
                break;

            case FILTER_COLORTEMPERATURE:
                graph = "@filter ColorTemperatureFilter filterA {\n" +
                    "  scale = 0.9f;" +
                    "}\n\n";
                break;

            case FILTER_CROSSPROCESS:
                graph = "@filter CrossProcessFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_DOCUMENTARY:
                graph = "@filter DocumentaryFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_DUOTONE:
                graph = "@filter DuotoneFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_FILLLIGHT:
                graph = "@filter FillLightFilter filterA {\n" +
                    "  strength = 0.9f;" +
                    "}\n\n";
                break;

            case FILTER_FISHEYE:
                graph = "@filter FisheyeFilter filterA {\n" +
                    "  scale = 0.9f;" +
                    "}\n\n";
                break;

            case FILTER_ROTATION:
                graph = "@filter FixedRotationFilter filterA {\n" +
                    "  rotation = 180;" +
                    "}\n\n";
                break;

            case FILTER_FLIP:
                graph = "@filter FlipFilter filterA {\n" +
                    "  horizontal = true;" +
                    "}\n\n";
                break;

            case FILTER_GRAIN:
                graph = "@filter GrainFilter filterA {\n" +
                    "  strength = 0.8f;" +
                    "}\n\n";
                break;

            case FILTER_LOMOISH:
                graph = "@filter LomoishFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_NEGATIVE:
                graph = "@filter NegativeFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_POSTERIZE:
                graph = "@filter PosterizeFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_SATURATE:
                graph = "@filter SaturateFilter filterA {\n" +
                    "  scale = 0.9f;" +
                    "}\n\n";
                break;

            case FILTER_SEPIA:
                graph = "@filter SepiaFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_SHARPEN:
                graph = "@filter SharpenFilter filterA {\n" +
                    "  scale = 0.9f;" +
                    "}\n\n";
                break;

            case FILTER_TINT:
                graph = "@filter TintFilter filterA {\n" +
                    "  tint = 65280;" + // 0x 00 00 FF 00
                    "}\n\n";
                break;

            case FILTER_GRAY:
                graph = "@filter ToGrayFilter filterA {\n" +
                    "}\n\n";
                break;

            case FILTER_VIGNETTE:
                graph = "@filter VignetteFilter filterA {\n" +
                    "  scale = 0.7f;" +
                    "}\n\n";
                break;

            case FILTER_VIDEO_TRANSITION:
                graph = "@filter VideoEventFilter filterA {\n" +
                    "}\n" +
                    "\n" +
                    "@filter MediaSourceInFilter videoeffect {\n" +
                    "  sourceUrl = \"no_file_specified\";\n" +
                    "  waitForNewFrame = true;\n" +
                    "  width = " + width + ";\n" +
                    "  height = " + height + ";\n" +
                    "  orientation = $orientation;\n" +
                    "}\n" +
                    "\n";
                break;
        }

        return graph;
    }

    public static final boolean isAvailable(String filterName) {
        ClassLoader contextClassLoader = Thread.currentThread().getContextClassLoader();
        Class<?> filterClass;
        // First see if a class of that name exists
        try {
            if (contextClassLoader == null) {
                Thread t = new Thread("isAvailable");
                contextClassLoader = t.getClass().getClassLoader();
            }
            filterClass = contextClassLoader.loadClass(filterName);
        } catch (ClassNotFoundException e) {
            return false;
        }

        // Then make sure it's a subclass of Filter.
        try {
            filterClass.asSubclass(Filter.class);
        } catch (ClassCastException e) {
            return false;
        }
        return true;
    }

    private void graphConstruct(int width, int height, int orientation, int orientationOrg) {
        mGraphString = "@setting autoBranch = \"synced\";\n"
            + "@import android.filterpacks.ui;\n"
            + "@import android.filterpacks.base;\n"
            + "@import android.filterpacks.imageproc;\n"
            + "@import android.filterpacks.videosrc;\n"
            + "@import android.filterpacks.videosink;\n"
            + "@import com.mediatek.effect.filterpacks;\n"
            + "@import com.mediatek.effect.filterpacks.io;\n"
            + "\n"
            + "@external OutputSurfaceView;\n"
            + "@external OutputTextureView;\n"
            + "@external OutputTextureView1;\n"
            + "@set orientation = " + orientation + ";\n\n"
            + "@set orientationOrg = " + orientationOrg + ";\n\n"
            + "@filter SurfaceTextureInFilter source {\n"
            + "  waitForNewFrame = true;\n"
            + "  width = " + width + ";\n"
            + "  height = " + height + ";\n"
            + "  orientation = $orientation;\n"
            + "}\n"
            + "\n"
            + getGraphOutputViewString(width, height, "display")
            + getGraphEffectStringByIndex(mFilterRealEffect, width, height)
            + "\n";

        if (mFilterRealEffect <= 0) {
            mGraphString += "@connect source[video] => display[frame];\n\n";
        } else {
            mGraphString += "@connect source[video] => filterA[image];\n";

            if (mFilterRealEffect == FILTER_VIDEO_TRANSITION) {
                mGraphString += "@connect videoeffect[video] => filterA[video];\n";
                //mGraphString += "@connect videoedge[video] => filterA[edge];\n";
            }

            if (hasViewForOutput()) {
                mGraphString += "@connect filterA[image] => display[frame];\n\n";
            }
        }

        if (FILTER_GF_SQUEEZE <= mFilterRealEffect && mFilterRealEffect <= FILTER_GF_SMALL_EYES) {
            mGraphString = "@setting autoBranch = \"synced\";\n"
                + "@import android.filterpacks.videosrc;\n"
                + "@import android.filterpacks.videosink;\n"
                + "@import android.filterpacks.ui;\n"
                + "@import android.filterpacks.base;\n"
                + "@import android.filterpacks.imageproc;\n"
                + "@import android.filterpacks.videosink;\n"
                + "@import com.mediatek.effect.filterpacks;\n"
                + "@import com.mediatek.effect.filterpacks.io;\n"
                + "@import com.google.android.filterpacks.facedetect;\n\n"
                + "@setting autoBranch = \"synced\";\n\n"
                + "\n"
                + "@external OutputSurfaceView;\n"
                + "@external OutputTextureView;\n"
                + "@external OutputTextureView1;\n"
                + "@set orientation = " + orientation + ";\n"
                + "@set orientationOrg = " + orientationOrg + ";\n\n"
                + "\n"
                + "@filter SurfaceTextureInFilter source {\n"
                + "  waitForNewFrame = true;\n"
                + "  width = " + width + ";\n"
                + "  height = " + height + ";\n"
                + "  orientation = $orientation;\n"
                + "}\n"
                + "\n"
                + getGraphOutputViewString(width, height, "display")
                + "@filter ToPackedGrayFilter toPackedGray {\n"
                + "    owidth = 320;\n"
                + "    oheight = 240;\n"
                + "    keepAspectRatio = true;\n"
                + "}\n"
                + "\n"
                + "@filter MultiFaceTrackerFilter faceTracker {\n"
                + "    numChannelsDetector = 3;\n"
                + "    quality = 0.0f;\n"
                + "    smoothness = 0.2f;\n"
                + "    minEyeDist = 25.0f;\n"
                + "    rollRange = 45.0f;\n"
                + "    numSkipFrames = 9;\n"
                + "    trackingError = 1.0;\n"
                + "    mouthOnlySmoothing = 0;\n"
                + "    useAffineCorrection = 1;\n"
                + "    patchSize = 15;\n"
                + "}\n"
                + "\n"
                + "@filter GoofyFastRenderFilter goofyrenderer {\n"
                + "    distortionAmount = 1.0;\n"
                + "}\n"
                + "\n"
                + "@filter FixedRotationFilter rotate {\n"
                + "    rotation = 0;\n"
                + "}\n"
                + "\n"
                + "@filter FaceMetaFixedRotationFilter metarotate {\n"
                + "    rotation = 0;\n"
                + "}\n"
                + "\n"
                + "@connect source[video] => rotate[image];\n"
                + "@connect rotate[image] => toPackedGray[image];\n"
                + "@connect toPackedGray[image] => faceTracker[image];\n"
                + "\n"
                + "@connect source[video] => goofyrenderer[image];\n"
                + "\n"
                + "@connect faceTracker[faces] => metarotate[faces];\n"
                + "@connect metarotate[faces] => goofyrenderer[faces];\n"
                + "\n";

            if (hasViewForOutput()) {
                mGraphString += "@connect goofyrenderer[outimage] => display[frame];\n\n";
            }
        }

        if (mTextureViewOriginalImg != null) {
            mGraphString +=
                "@filter TextureViewOutFilter displayWithoutEffect {\n" +
                "  textureView = $OutputTextureView1;\n" +
                "  width = " + width + ";\n" +
                "  height = " + height + ";\n" +
                "}\n" +
                "\n" +
                "@connect source[video] => displayWithoutEffect[frame];\n";
        }

        if (null != mRecordingPath) {
            mGraphString +=
                "@filter MediaEncoderOutFilter recorder {\n" +
                "  orientationHint = $orientationOrg;\n" +
                "  recording = true;\n" +
                "  width = " + width + ";\n" +
                "  height = " + height + ";\n" +
                "  outputFile = \"" + mRecordingPath.trim() + "\";\n" +
                "}\n" +
                "\n";
            if (mFilterRealEffect <= 0) {
                mGraphString += "@connect source[video] => recorder[videoframe];\n";
            } else {
                mGraphString += "@connect filterA[image] => recorder[videoframe];\n";
            }
        }
    }

    public boolean hasTargetOutput() {
        boolean result = true;

        if (mTextureView == null && mSurfaceView == null) {
            result = false;
        }

        return result;
    }

    public Filter getFilter(String name) {
        FilterGraph graph;
        Filter ret = null;

        if (null != mGraphEnv) {
            graph = mGraphEnv.getGraph(0);
            ret = graph.getFilter(name);
        }

        return ret;
    }

    public synchronized boolean graphCreate() {
        mTool.log('d', "graphCreate()");

        checkEffect();

        if (hasTargetOutput() == false) {
            mTool.log('w', "There is no target view for output");
            if (null != mRecordingPath) {
                mTool.log('w', "Only Recorder to " + mRecordingPath);
            } else {
                mTool.log('w', "Effect Stopped !");
                return false;
            }
        }

        int orientation = 0;
        int orientationVideoSrc = 0;
        if (mTransitionScenario.containsKey("orientation")) {
            try {
                orientation = Integer.parseInt(mTransitionScenario.get("orientation").toString());
                orientationVideoSrc = 0;
            } catch (NumberFormatException e) {
                orientation = 0;
            }
        }
        graphConstruct(mWidth, mHeight, orientationVideoSrc, orientation);
        FilterGraph graph;
        try {
            mGraphEnv = new GraphEnvironment();
            mGraphEnv.createGLEnvironment();
            mGraphEnv.addReferences("OutputSurfaceView", mSurfaceView);
            mGraphEnv.addReferences("OutputTextureView", mTextureView);
            mGraphEnv.addReferences("OutputTextureView1", mTextureViewOriginalImg);

            graph = mGraphEnv.getGraphReader().readGraphString(mGraphString);
            int graphId = mGraphEnv.addGraph(graph);
            mRunner = mGraphEnv.getRunner(graphId, GraphEnvironment.MODE_SYNCHRONOUS);
            mRunner.setDoneCallback(new OnRunnerDoneListener() {
                public void onRunnerDone(int result) {
                    mTool.log('d', "Graph runner done (" + mRunner + ") result:" + result);
                    if (result == GraphRunner.RESULT_ERROR) {
                        mTool.log('e', "Error running filter graph!");
                        Exception e = null;
                        if (mRunner != null) {
                            e = mRunner.getError();
                            e.printStackTrace();
                            mTool.log('w', "Graph runner err " + e.getMessage());
                        }
                    } else {
                        mRunner.setDoneCallback(null);
                        mRunner.close();
                    }

                    if (null != mProcessDone) {
                        submit(new Runnable() {
                            @Override
                            public void run() {
                                Thread.currentThread().setName("[" + mTool.getID() + "] OnProcessDone Call Back");
                                mProcessDone.onProcessDone(new Boolean(true));
                            }
                        });
                    }

                    graphCleanResource();
                }
            });

            mSourceFilter = (SurfaceTextureInFilter)graph.getFilter("source");
            if (null != mSourceFilter) {
                mSourceFilter.setInputValue("inwidth", mInputWidth);
                mSourceFilter.setInputValue("inheight", mInputHeight);
                mSourceFilter.setHandler(mUIHandler);

                if (mTransitionScenario.containsKey("truncate")) {
                    mSourceFilter.setInputValue("truncate", true);
                }

                if (mIgnoreMainFrameStreem) {
                    mTransitionScenario.put("IgnoreMainFrame", 1);
                    mSourceFilter.setInputValue("width", 16);
                    mSourceFilter.setInputValue("height", 16);
                    mSourceFilter.setInputValue("ignoreframe", true);
                    mSourceFilter.setInputValue("effectplayer", mEffectPlayer);
                }
            }

            mEncoderFilter = (MediaEncoderOutFilter)graph.getFilter("recorder");
            if (null != mEncoderFilter) {
                mEncoderFilter.setInputValue("isFromMediaPlayer", mIsFromMediaPlayer);
                mEncoderFilter.setInputValue("recordingProfile", mRecorderProfile);
                if (null != mRecorderProfile) {
                    mEncoderFilter.setInputValue("width", 0);
                    mEncoderFilter.setInputValue("height", 0);
                }
                mEncoderFilter.setInputValue("effectplayer", mEffectPlayer);

                if (mTransitionScenario.containsKey("livephoto")) {
                    mEncoderFilter.setInputValue("livephoto", true);
                }
            }

            if (FILTER_GF_SQUEEZE <= mFilterRealEffect && mFilterRealEffect <= FILTER_GF_SMALL_EYES) {
                Filter rotateFilter = mRunner.getGraph().getFilter("rotate");
                Filter metaRotateFilter = mRunner.getGraph().getFilter("metarotate");
                if (null != rotateFilter && null != metaRotateFilter) {
                    rotateFilter.setInputValue("rotation", 0);
                    int reverseDegrees = (360 - 0) % 360;
                    metaRotateFilter.setInputValue("rotation", reverseDegrees);

                    Filter goofyFilter = mRunner.getGraph().getFilter("goofyrenderer");
                    goofyFilter.setInputValue("currentEffect", (mFilterRealEffect - FILTER_GF_SQUEEZE));
                }
            } else if (mFilterRealEffect == EffectCameraPlayer.FILTER_VIDEO_TRANSITION) {
                Filter videoeffect = mRunner.getGraph().getFilter("videoeffect");

                if (null != videoeffect) {
                    if (null != mCntx) {
                        videoeffect.setInputValue("context", mCntx);
                    }

                    if (mTransitionScenario.containsKey("video2")) {
                        mEffectVideoUri = mTransitionScenario.get("video2") + "";
                    }

                    mTool.log('d', "EffectVideoUri: " + mEffectVideoUri);

                    videoeffect.setInputValue("sourceIsUrl", true);
                    videoeffect.setInputValue("sourceUrl", mEffectVideoUri);

                    if (mTransitionScenario.containsKey("truncate")) {
                        videoeffect.setInputValue("truncate", true);
                    }

                    if (mTransitionScenario.containsKey("video2_init_offset")) {
                        videoeffect.setInputValue("init_offset",
                            Integer.parseInt(mTransitionScenario.get("video2_init_offset") + ""));
                    }
                }

                VideoEventFilter transition = (VideoEventFilter)mRunner.getGraph().getFilter("filterA");
                transition.setInputValue("effectplayer", mEffectPlayer);
                transition.setInputValue("mediasourcefilter", videoeffect);

                transition.setScenario(mTransitionScenario);

                if (null != mEncoderFilter) {
                    if (mTransitionScenario.containsKey("THEENDTIME")) {
                        long endtime = Long.valueOf(mTransitionScenario.get("THEENDTIME") + "");
                        mEncoderFilter.setInputValue("endtime", endtime);
                    }
                    if (mTransitionScenario.containsKey("outputvideo")) {
                        String path = mTransitionScenario.get("outputvideo") + "";
                        mEncoderFilter.setInputValue("outputFile", path);
                    }
                    if (mTransitionScenario.containsKey("outputvideo_fps")) {
                        int fps = Integer.parseInt(mTransitionScenario.get("outputvideo_fps") + "");
                        mEncoderFilter.setInputValue("framerate", fps);
                    }
                    if (mTransitionScenario.containsKey("outputvideo_bitrate")) {
                        int bitrate = Integer.parseInt(mTransitionScenario.get("outputvideo_bitrate") + "");
                        mEncoderFilter.setInputValue("bitrate", bitrate);
                    }
                }
            }
        } catch (RuntimeException e) {
            mTool.log('w', "Could not read graph: " + e.getMessage());
            e.printStackTrace();
            return false;
        } catch (GraphIOException e) {
            mTool.log('w', "Could not read graph: " + e.getMessage());
            e.printStackTrace();
            return false;
        }

        mTool.log('d', "graphCreate() done");
        return true;
    }

    public synchronized SurfaceTexture graphRun() {
        Runnable EffectThread = new Runnable() {
            public void run() {
                Thread.currentThread().setName("[" + mTool.getID() + "] Effect Thread");
                Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
                GraphRunner LocalRunner = mRunner;
                isGraphRunning = true;
                mTool.log('d', "graphRun() Start " + LocalRunner);
                try {
                    LocalRunner.run();
                } catch (RuntimeException e) {
                    mTool.log('e', "Could not run graph: " + e.getMessage());
                    e.printStackTrace();
                } finally {
                    mTool.log('e', "graphRun() Stopped " + LocalRunner);
                    isGraphRunning = false;
                    LocalRunner = null;
                }
            }
        };

        if (isGraphRunning == false) {
            isGraphRunning = true;
            submit(EffectThread);
        }

        int waitCount = 0;
        mSurfaceTexture = null;

        if (mSourceFilter != null) {
            while (true) {
                mSurfaceTexture = mSourceFilter.getSurfaceTexture();

                if (mSurfaceTexture != null) {
                    break;
                }

                if (waitCount >= 100) {
                    mTool.log('e', "Get OpenGL SurfaceTexture error (" + waitCount + " times)");
                    return null;
                }

                synchronized (mSourceFilter) {
                    try {
                        mSourceFilter.wait(100);
                    } catch (InterruptedException e) {
                        mTool.log('w', "Get OpenGL SurfaceTexture error (" + waitCount + " times)");
                    }
                }
                waitCount++;
            }

            // Connect SurfaceTexture to callback
            mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
                public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                    if (mSourceFilter != null) {
                        mSourceFilter.setNewFrameAvailable(true);
                    }
                }
            });
        }

        return mSurfaceTexture;
    }

    /**
     *  Blocking for closing Graph
     */
    public void graphClose() {
        mTool.log('d', "graphClose()");

        setGraphCompleted();

        while (isGraphRunning == true) {
            mTool.log('d', "waiting for EffetThread released !! " + "isGraphRunning: " + isGraphRunning);
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (mRunner != null) {
            mTool.log('d', "mRunner.close()");
            mRunner.close();
            mTool.log('d', "mRunner.close() done");
        }

        mTool.log('d', "graphClose() done");
    }

    public synchronized void graphCleanResource() {
        mTool.log('d', "graphCleanResource()");

        if (mRunner != null) {
            graphCleanResourceDisconnectDisplay();
        }

        if (mGraphEnv != null) {
            mTool.log('d', "Tear down graph");

            // Tear down old graph if available
            try {
                GLEnvironment glEnv = mGraphEnv.getContext().getGLEnvironment();
                if (glEnv != null && !glEnv.isActive()) {
                    glEnv.activate();
                }
                mRunner.getGraph().tearDown(mGraphEnv.getContext());
                if (glEnv != null && glEnv.isActive()) {
                    glEnv.deactivate();
                }
            } catch (RuntimeException e) {
                mTool.log('d', "RuntimeException " + e.getMessage());
                e.printStackTrace();
            }
        }

        if (mRunner != null) {
            mRunner.close();
        }
        mRunner = null;

        mGraphEnv = null;

        mSourceFilter = null;
        mSurfaceTexture = null;
        mTextureView = null;
        mTextureViewOriginalImg = null;

        mTool.log('d', "graphCleanResource() done");
    }

    public void graphCleanResourceDisconnectDisplay() {
        mTool.log('d', "disconnect display of the graph");
        if (mRunner == null)
            return;

        Filter display = mRunner.getGraph().getFilter("display");
        if (display != null && (display instanceof TextureViewOutFilter)) {
            mTool.log('d', "Disconnecting the graph from: " + mTextureView);
            TextureViewOutFilter outFilter = (TextureViewOutFilter)display;
            outFilter.disconnect(mGraphEnv.getContext());
        }

        display = mRunner.getGraph().getFilter("displayWithoutEffect");
        if (display != null && (display instanceof TextureViewOutFilter)) {
            mTool.log('d', "Disconnecting the graph from: " + mTextureViewOriginalImg);
            TextureViewOutFilter outFilter = (TextureViewOutFilter)display;
            outFilter.disconnect(mGraphEnv.getContext());
        }
    }

    public void submit(Runnable task) {
        if (null != mExecutorService) {
            mExecutorService.submit(task);
        }
    }

    public SurfaceTexture getInputSurfaceTexture() {
        SurfaceTexture localSurfaceTexture = mSurfaceTexture;
        mTool.log('d', "getInputSurfaceTexture(): " + localSurfaceTexture);
        return localSurfaceTexture;
    }

    public int getInputTextureId() {
        int localTextureId = 0;
        if (mSourceFilter != null) {
            localTextureId = mSourceFilter.getTextureId();
        }
        mTool.log('d', "getInputTextureId(): " + mSourceFilter + " ID(" + localTextureId + ")");
        return localTextureId;
    }

    public void setProcessMaxFrameCount(int count) {
        if (mSourceFilter != null)
            mSourceFilter.setProcessMaxFrameCount(count);
    }

    private void setGraphCompleted() {
        if (mEncoderFilter != null)
            mEncoderFilter.setInputValue("recording", false);

        if (mSourceFilter != null)
            mSourceFilter.setCompleted(true);

        if (mRunner != null) {
            FilterGraph graph = mRunner.getGraph();
            if (graph != null) {
                Filter filter = graph.getFilter("videoeffect");
                if (filter instanceof MediaSourceInFilter) {
                    MediaSourceInFilter videoeffect = (MediaSourceInFilter) filter;
                    if (videoeffect != null)
                        videoeffect.setCompleted();
                }
            }
        }
    }

    /*
     * Setting variable/parameter
     */

    public void setScenario(VideoScenario scenario) {
        if (scenario != null)
            mTransitionScenario = scenario;
    }

    public void setIsFromMediaPlayer(boolean is) {
        mIsFromMediaPlayer = is;
    }

    public void setRecordingPath(String path, CamcorderProfile profile) {
        mRecordingPath = path;
        mRecorderProfile = profile;
    }

    public void setInputSizeToFitOutputSize(int width, int height) {
        mInputWidth = width;
        mInputHeight = height;
    }

    public void setIgnoreMainFrameStreem(boolean ignore) {
        mIgnoreMainFrameStreem = ignore;
    }

    public void setOntputSize(int width, int height) {
        mWidth = width;
        mHeight = height;
    }

    public void setOutputSurfaceView(FilterSurfaceView sv) {
        mSurfaceView = sv;
    }

    public void setOutputTextureView(TextureView tv) {
        mTextureView = tv;
    }

    public void setOutputTextureViewWithoutEffect(TextureView tv) {
        mTextureViewOriginalImg = tv;
    }

    public void setHandler(EffectUiHandler tv) {
        mUIHandler = tv;
    }

    public void setProcessDoneCallBack(ProcessDoneListener callback) {
        mProcessDone = callback;
    }
}

