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

package com.mediatek.ngin3d.android;

import android.os.SystemProperties;
import android.util.Log;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.presentation.PresentationEngine;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Stage for Wallpaper applications.
 */
public class StageWallpaperService extends GLWallpaperService {
    private boolean mDebugWallpaper;
    private static final String TAG = "StageWallpaper";

    public StageWallpaperService() {
        super();
    }

    public class StageRenderer implements Renderer {
        private PresentationEngine mPresentationEngine;
        private final PresentationEngine.RenderCallback mRenderCallback;
        private final Stage mStage;
        private Text mTextFPS;
        private final boolean mShowFPS;

        public StageRenderer(Stage stage) {
            this(stage, null);
        }

        public StageRenderer(Stage stage, PresentationEngine.RenderCallback renderCallback) {
            this(stage, renderCallback, false);
        }

        public StageRenderer(Stage stage, PresentationEngine.RenderCallback renderCallback, boolean showFPS) {
            super();
            mStage = stage;
            mRenderCallback = renderCallback;
            mShowFPS = showFPS;
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            if (mDebugWallpaper) {
                Log.d(TAG, "onSurfaceCreated, this: " + this + " tid:" + Thread.currentThread().getId());
            }
        }

        public Boolean isSurfaceReady() {
            Log.d(TAG, "isSurfaceReady(), mPresentationEngine:" + mPresentationEngine);
            if (mPresentationEngine == null) {
                return false;
            }
            return mPresentationEngine.isReady();
        }

        private final Object mSurfaceReadyLock = new Object();

        public void waitSurfaceReady() {
            synchronized (mSurfaceReadyLock) {
                while (!isSurfaceReady()) {
                    try {
                        mSurfaceReadyLock.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            if (mDebugWallpaper) {
                Log.d(TAG, "onSurfaceChanged - width: " + width + "height: " + height + " id: " + this
                    + " tid:" + Thread.currentThread().getId());
            }
            if (mPresentationEngine == null) {
                String cacheDir = null;
                if (getCacheDir() != null) {
                    cacheDir = getCacheDir().getAbsolutePath();
                }
                mPresentationEngine = Ngin3d.createPresentationEngine(mStage);
                mPresentationEngine.initialize(width, height, getResources(), cacheDir);
                mPresentationEngine.setRenderCallback(mRenderCallback);
                if (mShowFPS) {
                    setupFPSText();
                }
                synchronized (mSurfaceReadyLock) {
                    mSurfaceReadyLock.notifyAll();
                }
            } else {
                /** The actors' position might be normalized values and the real position values depend on Width/Height.
                 *  We need make position property dirty to recalculate correct position of actors after surface changed
                 */
                mStage.touchProperty("position");

                // "display area" function need use the height of view to calculate correct rectangle.
                // Make display_area dirty when surface size changing
                mStage.touchProperty("display_area");

                // Elsewhere, the viewport is set by the service rather than the
                // presentation engine, for consistency set the glViewport here.
                gl.glViewport(0, 0, width, height);

                // Inform the presentation engine of the change
                mPresentationEngine.resize(width, height);
            }

            if (mShowFPS) {
                mTextFPS.setPosition(new Point(width, 0));  // show it at right-bottom corner
            }
            mRenderCallback.requestRender();
        }

        public void onDrawFrame(GL10 gl) {
            if (mDebugWallpaper) {
                Log.d(TAG, "onDrawFrame , this: " + this + " tid:" + Thread.currentThread().getId());
            }
            if (mPresentationEngine == null)
                return;

            if (mShowFPS) {
                Log.d(TAG, "StageWallpaperService FPS:" + mPresentationEngine.getFPS());
                mTextFPS.setText(String.format("FPS: %.2f", mPresentationEngine.getFPS()));
                mPresentationEngine.render();
            } else {
                if (mPresentationEngine.render()) {
                    mRenderCallback.requestRender();
                }
            }

            // Dump the stage for debug purpose if the debug option is enabled
            int debugOptions = SystemProperties.getInt("debug.ngin3d.enable", 0);
            if ((Ngin3d.DEBUG_DUMP_STAGE & debugOptions) != 0) {
                mStage.dump();
            }
        }

        public void onDestroy() {
            if (mDebugWallpaper) {
                Log.d(TAG, "onDestroy, this: " + this + " tid:" + Thread.currentThread().getId());
            }
            if (mPresentationEngine != null) {
                mPresentationEngine.uninitialize();
            }
        }

        private void setupFPSText() {
            mTextFPS = new Text("");
            mTextFPS.setAnchorPoint(new Point(1.f, 0.f));
            mTextFPS.setPosition(new Point(0, 0));
            mTextFPS.setTextColor(Color.YELLOW);
            mStage.add(mTextFPS);
        }

        /**
         * Get the presentation engine of this object
         *
         * @hide Presentation API should be internal only
         *
         * @return presentation engine
         */
        public PresentationEngine getPresentationEngine() {
            return mPresentationEngine;
        }

        /**
         * Pause the rendering
         */
        public void pauseRendering() {
            if (mPresentationEngine != null) {
                mPresentationEngine.pauseRendering();
            }
        }

        /**
         * Resume the rendering
         */
        public void resumeRendering() {
            // adjust all timelines by current tick time
            if (mPresentationEngine != null) {
                mPresentationEngine.resumeRendering();
            }
        }

    }

    /**
     * Wallpaper stage 'engine', the stage-equivalent for wallpaper apps.
     */
    public class StageEngine extends GLEngine {
        private final Stage mStage;
        private StageRenderer mRenderer;
        private boolean mShowFPS;

        /**
         * StageEngine default constructor. Anti-aliasing is true
         * and provides default stage.
         */
        public StageEngine() {
            this(null, true);
        }

        /**
         * Initialize StageEngine with anti-alias configuration.
         *
         * @param antiAlias   enable anti-aliasing if true
         */
        public StageEngine(boolean antiAlias) {
            this(null, antiAlias);
        }

        /**
         * Initialize StageEngine with specific stage.
         *
         * @param stage   Stage class object
         */
        public StageEngine(Stage stage) {
            this(stage, true);
        }

        /**
         * Initialize StageEngine with specific stage and anti-alias configuration.
         *
         * @param stage   Stage class object
         * @param antiAlias   enable anti-aliasing if true
         */
        public StageEngine(Stage stage, boolean antiAlias) {
            super();

            if (stage == null) {
                mStage = new Stage(AndroidUiHandler.create());
            } else {
                mStage = stage;
            }
            int debugOptions = SystemProperties.getInt("debug.ngin3d.enable", 0);
            mShowFPS = ((Ngin3d.DEBUG_SHOW_FPS & debugOptions) != 0);
            mDebugWallpaper = ((Ngin3d.DEBUG_WALLPAPER_SERVICE & debugOptions) != 0);
            PresentationEngine.RenderCallback mRenderCallback = new PresentationEngine.RenderCallback() {
                public void requestRender() {
                    StageEngine.this.requestRender();
                }
            };
            if (antiAlias) {
                setEGLConfigChooser(new MultisampleConfigChooser());
            }
            setRenderer(new StageRenderer(mStage, mRenderCallback, mShowFPS));

            if (mShowFPS) {
                setRenderMode(RENDERMODE_CONTINUOUSLY);
            } else {
                setRenderMode(RENDERMODE_WHEN_DIRTY);
            }
        }

        @Override
        public void onDestroy() {
            if (mDebugWallpaper) {
                Log.d(TAG, "StageEngine onDestroy, this: " + this + " tid:" + Thread.currentThread().getId());
            }
            if (mRenderer != null) {
                mRenderer.onDestroy();
                mRenderer = null;
            }
            super.onDestroy();

        }

        @Override
        public void onPause() {
            if (mDebugWallpaper) {
                Log.d(TAG, "StageEngine onPause, this: " + this + " tid:" + Thread.currentThread().getId());
            }
            if (mShowFPS) {
                setRenderMode(RENDERMODE_WHEN_DIRTY);
            }
            if (mRenderer != null) {
                mRenderer.pauseRendering();
            }
            super.onPause();
        }

        @Override
        public void onResume() {
            if (mDebugWallpaper) {
                Log.d(TAG, "StageEngine onResume, this: " + this + " tid:" + Thread.currentThread().getId());
            }
            super.onResume();
            if (mRenderer != null) {
                // resume rendering and animations
                mRenderer.resumeRendering();
            }
            if (mShowFPS) {
                setRenderMode(RENDERMODE_CONTINUOUSLY);
            }
            requestRender();
        }

        public Stage getStage() {
            return mStage;
        }

        /**
         * @hide Should be internal.
         */
        public final void setRenderer(StageRenderer renderer) {
            if (mDebugWallpaper) {
                Log.d(TAG, "setRenderer: " + renderer);
            }
            super.setRenderer(renderer);
            mRenderer = renderer;
        }

        /**
         * Get the presentation engine of this object
         *
         * @hide Presentation API should be internal only
         *
         * @return presentation engine
         */
        public PresentationEngine getPresentationEngine() {
            return mRenderer.getPresentationEngine();
        }

        public void waitSurfaceReady() {
            Log.d(TAG, "waitSurfaceReady");
            mRenderer.waitSurfaceReady();
        }

    }
}
