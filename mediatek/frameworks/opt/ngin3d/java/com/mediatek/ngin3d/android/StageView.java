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

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Process;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.animation.AnimationLoader;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.utils.Ngin3dException;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

/**
 * A view that can display Ngin3d stage contents.
 */
public class StageView extends GLSurfaceView implements GLSurfaceView.Renderer {

    private static final String TAG = "StageView";
    private static final float DEFAULT_CAM_DIST = 1111.f;
    protected final Stage mStage;
    private Text mTextFPS;
    private final PresentationEngine mPresentationEngine;
    private Resources mResources;
    private String mCacheDir;
    private boolean mShowFPS;
    private Thread mGLThread;
    private static float sPixelDensity = -1f;
    private String mLibDir;

    public StageView(Context context) {
        this(context, (Stage) null);
    }

    public StageView(Context context, AttributeSet attrs) {
        this(context, attrs, new Stage(), true);
    }

    public StageView(Context context, AttributeSet attrs, boolean antiAlias) {
        this(context, attrs, new Stage(), antiAlias);
    }

    /**
     * Initialize this object with android context and Stage class object.
     *
     * @param context android context
     * @param stage   Stage class object
     */
    public StageView(Context context, Stage stage) {
        this(context, null, stage, true);
    }

    /**
     * Initialize this object with android context and Stage class object.
     *
     * @param context android context
     * @param stage   Stage class object
     * @param antiAlias   enable anti-aliasing if true
     */
    public StageView(Context context, Stage stage, boolean antiAlias) {
        this(context, null, stage, antiAlias);
    }

    private StageView(Context context, AttributeSet attrs, Stage stage, boolean antiAlias) {
        super(context, attrs);
        if (stage == null) {
            mStage = new Stage(AndroidUiHandler.create());
        } else {
            mStage = stage;
        }

        setEGLContextClientVersion(2);
        if (antiAlias) {
            setEGLConfigChooser(new MultisampleConfigChooser());
        } else {
            setEGLConfigChooser(8, 8, 8, 8, 16, 8);
        }

        getHolder().setFormat(PixelFormat.TRANSLUCENT);
        setZOrderOnTop(true);

        setRenderer(this);
        mResources = context.getResources();
        mPresentationEngine = Ngin3d.createPresentationEngine(mStage);

        if (context.getCacheDir() != null) {
            mCacheDir = context.getCacheDir().getAbsolutePath();
        }
        int debugOptions = SystemProperties.getInt("debug.ngin3d.enable", 0);
        mShowFPS = ((Ngin3d.DEBUG_SHOW_FPS & debugOptions) != 0);

        // Add text to show FPS
        if (mShowFPS) {
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            setupFPSText();
        } else {
            setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }

    private void setupFPSText() {
        mTextFPS = new Text("");
        mTextFPS.setAnchorPoint(new Point(1.f, 0.f));
        mTextFPS.setPosition(new Point(0, 0));
        mTextFPS.setTextColor(Color.YELLOW);
        mStage.add(mTextFPS);
    }

    public void runInGLThread(Runnable runnable) {
        if (Thread.currentThread() == mGLThread) {
            runnable.run();
        } else {
            queueEvent(runnable);
        }
    }

    public static float dpToPixel(Context context, float dp) {
        synchronized (StageView.class) {
            if (sPixelDensity < 0) {
                DisplayMetrics metrics = new DisplayMetrics();
                ((Activity) context).getWindowManager()
                    .getDefaultDisplay().getMetrics(metrics);
                sPixelDensity =  metrics.density;
            }
            return sPixelDensity * dp;
        }
    }

    public static int dpToPixel(Context context, int dp) {
        return (int)(dpToPixel(context, (float) dp) + .5f);
    }

    /**
     * This method can change the cache path where binary shaders are stored.
     * Must invoke the method in the constructor to apply the new cache directory or
     * default cache path (application's cache directory) will be used.
     *
     * @param cacheDir cache directory that binary
     */
    public void setCacheDir(String cacheDir) {
        mCacheDir = cacheDir;
    }

    /**
     * This method can change the cache path where binary shaders, animation cache and
     * symbolic library are stored.
     * Especially for widget (run on launcher process) and lockscreen 3D view(run on lockscreen process),
     * we have to set cache directory to launcher or lockscreen application's directory.
     * Only StageTextureView support this method since only this class can be used as widget and 3D lockscreen.
     * Must invoke the method in the constructor to apply the new cache directory or
     * default cache path (application's cache directory) will be used.
     *
     * @param context a context instance
     * @param cacheDir cache directory that binary
     */
    public void setCacheDir(Context context, String cacheDir) {
        if (context == null) {
            throw new Ngin3dException("The context can not be null");
        }
        mCacheDir = cacheDir;

        // Set keyframe animation cache directory
        AnimationLoader.setCacheDir(new File(mCacheDir));

        // Setup symbolic of JNI shared library
        mLibDir = mCacheDir + context.getPackageName();

        // If symbolic is created already, no need to create it again.
        File symbolic = new File(mLibDir + "/libja3m.so");
        if (symbolic.exists()) {
            return;
        }

        File libDir = new File(mLibDir);

        if (libDir.mkdirs()) {
            try {
                // Create link
                java.lang.Process process = Runtime.getRuntime().exec("ln -s /system/lib/libja3m.so " + mLibDir + "/libja3m.so");
                // The command is running in a separate native process.
                // We have to wait for the execution accomplished.
                process.waitFor();
            } catch (IOException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        } else {
            if (!libDir.exists()) {
                // Make sure library directory is created, if fail to create (ex: no permission),
                // set mLibDir to null and presentation engine will load shared library from default path.
                mLibDir = null;
            }
        }
    }

    /**
     * Get the original stage object of this class
     *
     * @return original stage object
     */
    public final Stage getStage() {
        return mStage;
    }


    /**
     * Get FPS
     *
     * @return FPS value
     */
    public double getFPS() {
        return mPresentationEngine.getFPS();
    }

    /**
     * Get the presentation engine of this object
     *
     * @hide Presentation regarded as internal interface
     *
     * @return presentation engine
     */
    public PresentationEngine getPresentationEngine() {
        return mPresentationEngine;
    }

    /**
     * Enable/disable stereoscopic 3d display mode
     *
     * If enable stereoscopic 3d mode by this method, the default
     * focal distance is default Camera Z position(1111) in UI_PERSPECTIVE mode.
     * The value of 1111 is a remnant from AfterEffects default position.
     *
     * @param enable       true to enable and false to disable stereo 3d mode
     */
    public void enableStereoscopic3D(boolean enable) {
        mStage.setStereo3D(enable, DEFAULT_CAM_DIST);
    }

    /**
     * Enable/disable stereoscopic 3d display mode
     *
     * @param enable       true if you want to enable stereo 3d display mode
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     */
    public void enableStereoscopic3D(boolean enable, float focalDistance) {
        mStage.setStereo3D(enable, focalDistance);
    }

    /**
     * Enable/disable stereoscopic 3d display mode
     *
     * @param enable       true if you want to enable stereo 3d display mode
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     * @param intensity Adjust the level of stereo separation. Normally 1.0,
     *                  1.1 increases the effect by 10%, for example.
     */
    public void enableStereoscopic3D(boolean enable, float focalDistance, float intensity) {
        mStage.setStereo3D(enable, focalDistance, intensity);
    }

    /**
     * Controls whether the surface view's surface is used as a stereo 3d
     * surface. Uses flags specified in android.view.WindowManager.LayoutParams
     *
     * @param flag Stereo 3D display flags that can be bitwise-ored to enable
              Stereo 3D and either side-by-side or top-and-bottom format
     * @param mask mask for the Stereo 3D flags
     */
    public void setStereoscopic3dFlags(int flag, int mask) {
        /* setFlagsEx is platform specific and may not always be present so we
         * cannot just inherit this method.
         * Instead this uses reflection to try and set the flags at runtime
         * and report if the class is not found.
         */
        try {
            // load the GLSurfaceView at runtime
            Class cls = Class.forName("android.opengl.GLSurfaceView");

            // int parameter
            Class[] paramInt = new Class[2];
            paramInt[0] = Integer.TYPE;
            paramInt[1] = Integer.TYPE;

            // call the setFlagsEx method
            Method method = cls.getDeclaredMethod("setFlagsEx", paramInt);
            method.invoke(this, flag, mask);
        } catch (ClassNotFoundException ex) {
            Log.w(TAG, "android.opengl.GLSurfaceView not found ");
        } catch (NoSuchMethodException ex) {
            Log.w(TAG, "setFlagsEx not found in GLSurfaceView");
        } catch (InvocationTargetException ex) {
            Log.w(TAG, "Got exception when invoking setFlagsEx");
        } catch (IllegalAccessException ex) {
            Log.w(TAG, "Illegal to access setFlagsEx");
        }
    }

    /**
     * Get the the screen shot of current render frame
     *
     * @return A Bitmap object representing the render frame
     */
    @Deprecated
    public Bitmap getScreenShot() {
        FutureTask<Object> task = new FutureTask<Object>(new Callable<Object>() {
            public Object call() {
                return mPresentationEngine.getScreenShot();
            }
        });

        runInGLThread(task);
        Object obj = null;
        try {
            obj = task.get();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (ExecutionException e) {
            e.printStackTrace();
        }

        if (obj instanceof Bitmap) {
            return (Bitmap) obj;
        }
        return null;
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.v(TAG, "onSurfaceCreated()");

        // Increase the priority of the render thread
        Process.setThreadPriority(Process.THREAD_PRIORITY_DISPLAY);
        mGLThread = Thread.currentThread();

        final int w = getWidth();
        final int h = getHeight();

        // Set render callback so that the presentation engine can trigger renders.
        mPresentationEngine.setRenderCallback(new PresentationEngine.RenderCallback() {
            public void requestRender() {
                StageView.this.requestRender();
            }
        });

        mPresentationEngine.initialize(w, h, mResources, mCacheDir, mLibDir);

        synchronized (mSurfaceReadyLock) {
            mSurfaceReadyLock.notifyAll();
        }

        if (Ngin3d.DEBUG) {
            mPresentationEngine.dump();
        }
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        Log.v(TAG, "onSurfaceChanged(width = " + width + ", height = " + height + ")");

        gl.glViewport(0, 0, width, height);
        /** The actors' position might be normalized values and the real position values depend on Width/Height.
         *  We need make position property dirty to recalculate correct position of actors after surface changed
         */
        mStage.touchProperty("position");

        // "display area" function need use the height of view to calculate correct rectangle.
        // Make display_area dirty when surface size changing
        mStage.touchProperty("display_area");

        mPresentationEngine.resize(width, height);

        if (mShowFPS) {
            mTextFPS.setPosition(new Point(width, 0));  // show it at right-bottom corner
        } else {
            requestRender();
        }
    }

    public static final long INVALID_TIME = -1;
    private long mFirstOnDrawFrameTime = INVALID_TIME;

    public long getFirstOnDrawTime() {
        return mFirstOnDrawFrameTime;
    }

    public void onDrawFrame(GL10 gl) {
        if (mFirstOnDrawFrameTime == INVALID_TIME) {
            mFirstOnDrawFrameTime = SystemClock.uptimeMillis();
            Log.d(TAG, "onDrawFrame() invoked @" + mFirstOnDrawFrameTime);
        }

        if (mShowFPS) {
            mTextFPS.setText(String.format("FPS: %.2f", mPresentationEngine.getFPS()));
            mPresentationEngine.render();
        } else {
            if (mPresentationEngine.render()) {
                requestRender();
            }
        }

        // Dump the stage for debug purpose if the debug option is enabled
        int debugOptions = SystemProperties.getInt("debug.ngin3d.enable", 0);
        if ((Ngin3d.DEBUG_DUMP_STAGE & debugOptions) != 0) {
            mStage.dump();
        }
    }

    @Override
    public void onPause() {
        // pause rendering and animations
        pauseRendering();
        // Uninitialize presentation engine before GLSurface paused the rendering thread.
        FutureTask<Void> task = new FutureTask<Void>(new Callable<Void>() {
            public Void call() {
                mPresentationEngine.uninitialize();
                return null;
            }
        });
        runInGLThread(task);
        super.onPause();
    }

    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume from activity");
        // resume rendering and animations
        resumeRendering();
    }

    /**
     * Pause the rendering
     */
    public void pauseRendering() {
        if (mShowFPS) {
            setRenderMode(RENDERMODE_WHEN_DIRTY);
        }

        mPresentationEngine.pauseRendering();
    }

    /**
     * Resume the rendering
     */
    public void resumeRendering() {
        // adjust all timelines by current tick time
        mPresentationEngine.resumeRendering();

        if (mShowFPS) {
            setRenderMode(RENDERMODE_CONTINUOUSLY);
        } else {
            requestRender();
        }
    }

    public Boolean isSurfaceReady() {
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
}
