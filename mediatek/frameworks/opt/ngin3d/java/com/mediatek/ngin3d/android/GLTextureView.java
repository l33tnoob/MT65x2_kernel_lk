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

import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.SurfaceTexture;
import android.opengl.GLUtils;
import android.os.SystemProperties;
import android.util.AttributeSet;
import android.util.Log;
import android.view.TextureView;
import android.view.View;

import com.mediatek.ngin3d.Ngin3d;
import com.mediatek.ngin3d.presentation.PresentationEngine;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;
import java.util.ArrayList;

/**
 *
 */
/*package*/ class GLTextureView extends TextureView implements TextureView.SurfaceTextureListener {

    private static final String TAG = "GLTextureView";

    private static final boolean LOG_ATTACH_DETACH = true;
    private static final boolean LOG_THREADS = false;
    private static final boolean LOG_PAUSE_RESUME = true;
    private static final boolean LOG_SURFACE = true;
    private static final boolean LOG_RENDERER = true;
    private static final boolean LOG_RENDERER_DRAW_FRAME = false;
    private static final boolean LOG_TEXTURE_UPDATE_FRAME = false;
    private static final boolean LOG_TEXTURE_VIEW = true;
    private static final boolean LOG_EGL = true;
    // Work-around for bug 2263168
    private static final boolean DRAW_TWICE_AFTER_SIZE_CHANGED = true;
    /**
     * The renderer only renders
     * when the surface is created, or when {@link #requestRender} is called.
     */
    public static final int RENDERMODE_WHEN_DIRTY = 0;
    /**
     * The renderer is called continuously to re-render the scene.
     */
    public static final int RENDERMODE_CONTINUOUSLY = 1;

    protected boolean mShowFPS;
    protected PresentationEngine mPresentationEngine;

    public GLTextureView(Context context) {
        this(context, null);
    }

    public GLTextureView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSurfaceTextureListener(this);
        int debugOptions = SystemProperties.getInt("debug.ngin3d.enable", 0);
        mShowFPS = ((Ngin3d.DEBUG_SHOW_FPS & debugOptions) != 0);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (LOG_ATTACH_DETACH) {
            Log.d(TAG, "onAttachedToWindow reattach =" + mDetached + "this: " + this);
        }

        if (mDetached && (mRenderer != null)) {

            int renderMode = RENDERMODE_CONTINUOUSLY;
            if (mGLThread != null) {
                renderMode = mGLThread.getRenderMode();
            }
            mGLThread = new GLThread(mRenderer);
            if (renderMode != RENDERMODE_CONTINUOUSLY) {
                mGLThread.setRenderMode(renderMode);
            }
            mGLThread.start();
        }

        mDetached = false;
    }

    @Override
    protected void onDetachedFromWindow() {
        if (LOG_ATTACH_DETACH) {
            Log.d(TAG, "onDetachedFromWindow");
        }

        if (mGLThread != null) {
            mGLThread.requestExitAndWait();
        }
        mDetached = true;
        super.onDetachedFromWindow();
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w,  h, oldw, oldh);
        Log.d(TAG, "onSizeChanged, w: " + w + " h: " + h + " oldw: " + oldw + " oldh: " + oldh);
        mGLThread.onWindowResize(w, h);
        mGLThread.setSurfaceTextureReady();
        mGLThread.requestRender();
    }

    // Override this method for debug purpose
    @Override
    public void setVisibility(int visibility) {
        super.setVisibility(visibility);
        Log.d(TAG, "setVisibility, visibility is:" + visibility);
    }

    @Override
    protected void onVisibilityChanged(View changedView, int visibility) {
        super.onVisibilityChanged(changedView, visibility);
        if (visibility == VISIBLE) {
            mGLThread.requestRender();
        }
        Log.d(TAG, "onVisibilityChanged, visibility is:" + visibility);
    }

    public void runInGLThread(Runnable runnable) {
        if (Thread.currentThread() == mGLThread) {
            runnable.run();
        } else {
            queueEvent(runnable);
        }
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        if (LOG_TEXTURE_VIEW) {
            Log.d(TAG, "onSurfaceTextureAvailable");
        }
        mGLThread.surfaceCreated(surface, width, height);
        // Handle Low Memory case: Make sure GLthread is resume after surface texture available
        mGLThread.onResume();

    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        if (LOG_TEXTURE_VIEW) {
            Log.d(TAG, "onSurfaceTextureSizeChanged");
        }
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        if (LOG_TEXTURE_VIEW) {
            Log.d(TAG, "onSurfaceTextureDestroyed");
        }

        // Handle Low Memory case: Make sure GLthread is paused before destroy surface
        mGLThread.onPause();
        mGLThread.surfaceDestroyed();
        return false;
    }

    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        if (LOG_TEXTURE_UPDATE_FRAME) {
            Log.d(TAG, "onSurfaceTextureUpdated");
        }

        mGLThread.setSurfaceTextureReady();
    }

    /**
     * Inform the view that the activity is paused. The owner of this view must
     * call this method when the activity is paused. Calling this method will
     * pause the rendering thread.
     * Must not be called before a renderer has been set.
     */
    public void onPause() {
        if (LOG_TEXTURE_VIEW) {
            Log.d(TAG, "onPause from activity");
        }
        mGLThread.onPause();
    }

    /**
     * Inform the view that the activity is resumed. The owner of this view must
     * call this method when the activity is resumed. Calling this method will
     * recreate the OpenGL display and resume the rendering
     * thread.
     * Must not be called before a renderer has been set.
     */
    public void onResume() {
        if (LOG_TEXTURE_VIEW) {
            Log.d(TAG, "onResume from activity");
        }
        mGLThread.onResume();
    }

    /**
     * Queue a runnable to be run on the GL rendering thread. This can be used
     * to communicate with the Renderer on the rendering thread.
     * Must not be called before a renderer has been set.
     * @param r the runnable to be run on the GL rendering thread.
     */
    public void queueEvent(Runnable r) {
        mGLThread.queueEvent(r);
    }

    public void setRenderer(Renderer renderer) {
        checkRenderThreadState();
        if (mEGLConfigChooser == null) {
            mEGLConfigChooser = new SimpleEGLConfigChooser(true);
        }
        if (mEGLContextFactory == null) {
            mEGLContextFactory = new DefaultContextFactory();
        }
        if (mEGLWindowSurfaceFactory == null) {
            mEGLWindowSurfaceFactory = new DefaultWindowSurfaceFactory();
        }

        mRenderer = renderer;
        mGLThread = new GLThread(renderer);
        mGLThread.start();
    }

    /**
     * Install a custom EGLContextFactory.
     * <p>If this method is
     * called, it must be called before {@link #setRenderer(Renderer)}
     * is called.
     * <p>
     * If this method is not called, then by default
     * a context will be created with no shared context and
     * with a null attribute list.
     */
    public void setEGLContextFactory(EGLContextFactory factory) {
        checkRenderThreadState();
        mEGLContextFactory = factory;
    }

    /**
     * Install a custom EGLWindowSurfaceFactory.
     * <p>If this method is
     * called, it must be called before {@link #setRenderer(Renderer)}
     * is called.
     * <p>
     * If this method is not called, then by default
     * a window surface will be created with a null attribute list.
     */
    public void setEGLWindowSurfaceFactory(EGLWindowSurfaceFactory factory) {
        checkRenderThreadState();
        mEGLWindowSurfaceFactory = factory;
    }

    /**
     * Install a custom EGLConfigChooser.
     * <p>If this method is
     * called, it must be called before {@link #setRenderer(Renderer)}
     * is called.
     * <p>
     * If no setEGLConfigChooser method is called, then by default the
     * view will choose an EGLConfig that is compatible with the current
     * android.view.Surface, with a depth buffer depth of
     * at least 16 bits.
     * @param configChooser
     */
    public void setEGLConfigChooser(EGLConfigChooser configChooser) {
        checkRenderThreadState();
        mEGLConfigChooser = configChooser;
    }

    /**
     * Install a config chooser which will choose a config
     * as close to 16-bit RGB as possible, with or without an optional depth
     * buffer as close to 16-bits as possible.
     * <p>If this method is
     * called, it must be called before {@link #setRenderer(Renderer)}
     * is called.
     * <p>
     * If no setEGLConfigChooser method is called, then by default the
     * view will choose an RGB_565 surface with a depth buffer depth of
     * at least 16 bits.
     *
     * @param needDepth
     */
    public void setEGLConfigChooser(boolean needDepth) {
        setEGLConfigChooser(new SimpleEGLConfigChooser(needDepth));
    }

    /**
     * Install a config chooser which will choose a config
     * with at least the specified depthSize and stencilSize,
     * and exactly the specified redSize, greenSize, blueSize and alphaSize.
     * <p>If this method is
     * called, it must be called before {@link #setRenderer(Renderer)}
     * is called.
     * <p>
     * If no setEGLConfigChooser method is called, then by default the
     * view will choose an RGB_565 surface with a depth buffer depth of
     * at least 16 bits.
     *
     */
    public void setEGLConfigChooser(int redSize, int greenSize, int blueSize,
                                    int alphaSize, int depthSize, int stencilSize) {
        setEGLConfigChooser(new ComponentSizeChooser(redSize, greenSize,
            blueSize, alphaSize, depthSize, stencilSize));
    }

    /**
     * Inform the default EGLContextFactory and default EGLConfigChooser
     * which EGLContext client version to pick.
     * <p>Use this method to create an OpenGL ES 2.0-compatible context.
     */
    public void setEGLContextClientVersion(int version) {
        checkRenderThreadState();
        mEGLContextClientVersion = version;
    }

    /**
     * Set the rendering mode. When renderMode is
     * RENDERMODE_CONTINUOUSLY, the renderer is called
     * repeatedly to re-render the scene. When renderMode
     * is RENDERMODE_WHEN_DIRTY, the renderer only rendered when the surface
     * is created, or when {@link #requestRender} is called. Defaults to RENDERMODE_CONTINUOUSLY.
     * <p>
     * Using RENDERMODE_WHEN_DIRTY can improve battery life and overall system performance
     * by allowing the GPU and CPU to idle when the view does not need to be updated.
     * <p>
     *
     * @param renderMode one of the RENDERMODE_X constants
     * @see #RENDERMODE_CONTINUOUSLY
     * @see #RENDERMODE_WHEN_DIRTY
     */
    public void setRenderMode(int renderMode) {
        mGLThread.setRenderMode(renderMode);
    }

    /**
     * Get the current rendering mode. May be called
     * from any thread. Must not be called before a renderer has been set.
     * @return the current rendering mode.
     * @see #RENDERMODE_CONTINUOUSLY
     * @see #RENDERMODE_WHEN_DIRTY
     */
    public int getRenderMode() {
        return mGLThread.getRenderMode();
    }

    /**
     * Request that the renderer render a frame.
     * This method is typically used when the render mode has been set to
     * {@link #RENDERMODE_WHEN_DIRTY}, so that frames are only rendered on demand.
     * May be called
     * from any thread. Must not be called before a renderer has been set.
     */
    public void requestRender() {
        mGLThread.requestRender();
    }


    protected void setSurfaceTextureReady() {
        mGLThread.setSurfaceTextureReady();
    }

    // ----------------------------------------------------------------------

    /**
     * An interface used to wrap a GL interface.
     */
    public interface GLWrapper {
        /**
         * Wraps a gl interface in another gl interface.
         *
         * @param gl a GL interface that is to be wrapped.
         * @return either the input argument or another GL object that wraps the input argument.
         */
        GL wrap(GL gl);
    }

    /**
     * A generic renderer interface.
     */
    public interface Renderer {
        /**
         * Called when the surface is created or recreated.
         * <p/>
         * Called when the rendering thread
         * starts and whenever the EGL context is lost. The EGL context will typically
         * be lost when the Android device awakes after going to sleep.
         * <p/>
         * Since this method is called at the beginning of rendering, as well as
         * every time the EGL context is lost, this method is a convenient place to put
         * code to create resources that need to be created when the rendering
         * starts, and that need to be recreated when the EGL context is lost.
         * Textures are an example of a resource that you might want to create
         * here.
         * <p/>
         * Note that when the EGL context is lost, all OpenGL resources associated
         * with that context will be automatically deleted. You do not need to call
         * the corresponding "glDelete" methods such as glDeleteTextures to
         * manually delete these lost resources.
         * <p/>
         *
         * @param gl the GL interface. Use <code>instanceof</code> to
         * test if the interface supports GL11 or higher interfaces.
         * @param config the EGLConfig of the created surface. Can be used
         *               to create matching pbuffers.
         */
        void onSurfaceCreated(GL10 gl, EGLConfig config);

        /**
         * Called when the surface changed size.
         * <p/>
         * Called after the surface is created and whenever
         * the OpenGL ES surface size changes.
         * <p/>
         * Typically you will set your viewport here. If your camera
         * is fixed then you could also set your projection matrix here:
         * <pre class="prettyprint">
         * void onSurfaceChanged(GL10 gl, int width, int height) {
         * gl.glViewport(0, 0, width, height);
         * // for a fixed camera, set the projection too
         * float ratio = (float) width / height;
         * gl.glMatrixMode(GL10.GL_PROJECTION);
         * gl.glLoadIdentity();
         * gl.glFrustumf(-ratio, ratio, -1, 1, 1, 10);
         * }
         * </pre>
         * @param gl the GL interface. Use <code>instanceof</code> to
         * test if the interface supports GL11 or higher interfaces.
         * @param width
         * @param height
         */
        void onSurfaceChanged(GL10 gl, int width, int height);

        /**
         * Called to draw the current frame.
         * <p/>
         * This method is responsible for drawing the current frame.
         * <p/>
         * The implementation of this method typically looks like this:
         * <pre class="prettyprint">
         * void onDrawFrame(GL10 gl) {
         * gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
         * //... other gl calls to render the scene ...
         * }
         * </pre>
         * @param gl the GL interface. Use <code>instanceof</code> to
         * test if the interface supports GL11 or higher interfaces.
         */
        void onDrawFrame(GL10 gl);

        /**
         * Called when the Glthread is paused.
         * <p/>
         * This method is responsible for notify GLThread is paused and
         * even free the resource.
         * <p/>
         * The implementation of this method typically looks like this:
         * <pre class="prettyprint">
         * void onPaused() {
         * Release(resource);
         * //... other uninitialized call
         * }
         * </pre>
         */
        void onPaused();
    }

    /**
     * An interface for customizing the eglCreateContext and eglDestroyContext calls.
     */
    public interface EGLContextFactory {
        EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig);
        void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context);
    }

    private class DefaultContextFactory implements EGLContextFactory {
        private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config) {
            int[] attribList = {EGL_CONTEXT_CLIENT_VERSION, mEGLContextClientVersion,
                EGL10.EGL_NONE };
            return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT,
                mEGLContextClientVersion == 0 ? null : attribList);
        }

        public void destroyContext(EGL10 egl, EGLDisplay display,
                                   EGLContext context) {
            if (!egl.eglDestroyContext(display, context)) {
                Log.e("DefaultContextFactory", "display:" + display + " context: " + context);
                if (LOG_THREADS) {
                    Log.i("DefaultContextFactory", "tid=" + Thread.currentThread().getId());
                }
                throw new RuntimeException("eglDestroyContext failed: ");
            }
        }
    }

    /**
     * An interface for customizing the eglCreateWindowSurface and eglDestroySurface calls.
     * <p>
     * This interface must be implemented by clients wishing to call
     */
    public interface EGLWindowSurfaceFactory {
        /**
         *  @return null if the surface cannot be constructed.
         */
        EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config,
                                       Object nativeWindow);
        void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface);
    }

    private static class DefaultWindowSurfaceFactory implements EGLWindowSurfaceFactory {

        public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display,
                                              EGLConfig config, Object nativeWindow) {
            EGLSurface result = null;
            try {
                result = egl.eglCreateWindowSurface(display, config, nativeWindow, null);
            } catch (IllegalArgumentException e) {
                // This exception indicates that the surface flinger surface
                // is not valid. This can happen if the surface flinger surface has
                // been torn down, but the application has not yet been
                // notified via SurfaceHolder.Callback.surfaceDestroyed.
                // In theory the application should be notified first,
                // but in practice sometimes it is not. See b/4588890
                Log.e(TAG, "eglCreateWindowSurface", e);
            }
            return result;
        }

        public void destroySurface(EGL10 egl, EGLDisplay display,
                                   EGLSurface surface) {
            egl.eglDestroySurface(display, surface);
        }
    }

    /**
     * An EGL helper class.
     */

    private class EglHelper {
        public EglHelper() {
            // Do nothing
        }

        /**
         * Initialize EGL for a given configuration spec.
         */
        public void start(SurfaceTexture mSurface) {
            if (LOG_EGL) {
                Log.d("EglHelper", "start() tid=" + Thread.currentThread().getId());
            }
            /*
             * Get an EGL instance
             */
            mEgl = (EGL10) EGLContext.getEGL();

            /*
             * Get to the default display.
             */
            mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

            if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
                throw new RuntimeException("eglGetDisplay failed");
            }

            /*
             * We can now initialize EGL for that display
             */
            int[] version = new int[2];
            if (!mEgl.eglInitialize(mEglDisplay, version)) {
                throw new RuntimeException("eglInitialize failed");
            }

            mEglConfig = mEGLConfigChooser.chooseConfig(mEgl, mEglDisplay);
            if (mEglConfig == null) {
                throw new RuntimeException("eglConfig not initialized");
            }

            /*
            * Create an EGL context. We want to do this as rarely as we can, because an
            * EGL context is a somewhat heavy object.
            */
            mEglContext = mEGLContextFactory.createContext(mEgl, mEglDisplay, mEglConfig);

            if (mEglContext == null || mEglContext == EGL10.EGL_NO_CONTEXT) {
                mEglContext = null;
                throwEglException("createContext");
            }
            if (LOG_EGL) {
                Log.d("EglHelper", "createContext " + mEglContext + " tid=" + Thread.currentThread().getId());
            }

            mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay,
                mEglConfig, mSurface, null);
            if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
                int error = mEgl.eglGetError();
                if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                    Log.e("EglHelper", "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.");
                }
                throw new RuntimeException("createWindowSurface failed "
                    + GLUtils.getEGLErrorString(error));
            }
            if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface,
                mEglContext)) {
                throw new RuntimeException("eglMakeCurrent failed "
                    + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
        }

        public void purgeBuffers() {
            mEgl.eglMakeCurrent(mEglDisplay,
                EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE,
                EGL10.EGL_NO_CONTEXT);

            if (LOG_EGL) {
                Log.d("EglHelper", "mEglDisplay= " + mEglDisplay + " mEglSurface= " + mEglSurface + " mEglContext=" + mEglContext);
            }
            mEgl.eglMakeCurrent(mEglDisplay,
                mEglSurface, mEglSurface,
                mEglContext);
        }

        /**
         * Display the current render surface.
         * @return false if the context has been lost.
         */
        public boolean swap() {
            if (!mEgl.eglSwapBuffers(mEglDisplay, mEglSurface)) {

                /*
                 * Check for EGL_CONTEXT_LOST, which means the context
                 * and all associated data were lost (For instance because
                 * the device went to sleep). We need to sleep until we
                 * get a new surface.
                 */
                int error = mEgl.eglGetError();
                switch(error) {
                case EGL11.EGL_CONTEXT_LOST:
                    return false;
                case EGL10.EGL_BAD_NATIVE_WINDOW:
                    // The native window is bad, probably because the
                    // window manager has closed it. Ignore this error,
                    // on the expectation that the application will be closed soon.
                    Log.e("EglHelper", "eglSwapBuffers returned EGL_BAD_NATIVE_WINDOW. tid=" + Thread.currentThread().getId());
                    break;
                default:
                    throwEglException("eglSwapBuffers", error);
                }
            }
            return true;
        }

        public void destroySurface() {
            if (LOG_EGL) {
                Log.d("EglHelper", "destroySurface()  tid=" + Thread.currentThread().getId());
            }

            if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE) {
                mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_SURFACE,
                    EGL10.EGL_NO_CONTEXT);
                mEGLWindowSurfaceFactory.destroySurface(mEgl, mEglDisplay, mEglSurface);
                mEglSurface = null;
            }
        }

        public void finish() {
            if (LOG_EGL) {
                Log.d("EglHelper", "finish() tid=" + Thread.currentThread().getId());
            }

            if (mEglContext != null) {
                mEGLContextFactory.destroyContext(mEgl, mEglDisplay, mEglContext);
                mEglContext = null;
            }
            if (mEglDisplay != null) {
                mEgl.eglTerminate(mEglDisplay);
                mEglDisplay = null;
            }
        }

        private void throwEglException(String function) {
            throwEglException(function, mEgl.eglGetError());
        }

        private void throwEglException(String function, int error) {
            String message = function + " failed: " + error;
            if (LOG_THREADS) {
                Log.e("EglHelper", "throwEglException tid=" + Thread.currentThread().getId() + " " + message);
            }
            throw new RuntimeException(message);
        }

        EGL10 mEgl;
        EGLDisplay mEglDisplay;
        EGLSurface mEglSurface;
        EGLConfig mEglConfig;
        EGLContext mEglContext;
    }

    /**
     * A generic GL Thread. Takes care of initializing EGL and GL. Delegates
     * to a Renderer instance to do the actual drawing. Can be configured to
     * render continuously or on request.
     *
     * All potentially blocking synchronization is done through the
     * GL_THREAD_MANAGER object. This avoids multiple-lock ordering issues.
     *
     */
    private class GLThread extends Thread {
        private static final String LOG_TAG = "GLThread";
        GLThread(Renderer renderer) {
            super();
            mRenderer = renderer;
            mRequestRender = true;
            mSurfaceTextureReady = true;
            mRenderMode = RENDERMODE_CONTINUOUSLY;
        }

        @Override
        public void run() {
            setName("GLThread " + getId());
            if (LOG_THREADS) {
                Log.i("GLThread", "starting tid=" + getId());
            }

            try {
                guardedRun();
            } catch (InterruptedException e) {
                // fall thru and exit normally
                e.printStackTrace();
            } finally {
                if (LOG_THREADS) {
                    Log.i("GLThread", "threadExiting tid=" + getId());
                }
                GL_THREAD_MANAGER.threadExiting(this);
            }
        }

        /*
        * This private method should only be called inside a
        * synchronized(GL_THREAD_MANAGER) block.
        */
        private void stopEglSurfaceLocked() {
            if (mHaveEglSurface) {
                mHaveEglSurface = false;
                mEglHelper.destroySurface();
            }
        }

        /*
         * This private method should only be called inside a
         * synchronized(GL_THREAD_MANAGER) block.
         */
        private void stopEglContextLocked() {
            if (mHaveEglContext) {
                mEglHelper.finish();
                mHaveEglContext = false;
                GL_THREAD_MANAGER.releaseEglContextLocked(this);
            }
        }
        private void guardedRun() throws InterruptedException {
            mEglHelper = new EglHelper();
            mHaveEglContext = false;
            mHaveEglSurface = false;
            try {
                GL10 gl = null;
                boolean createEglContext = false;
                boolean lostEglContext = false;
                boolean sizeChanged = false;
                boolean wantRenderNotification = false;
                boolean doRenderNotification = false;
                boolean askedToReleaseEglContext = false;
                Runnable event = null;

                while (true) {
                    synchronized (GL_THREAD_MANAGER) {
                        while (true) {
                            if (mShouldExit) {
                                return;
                            }

                            if (!mEventQueue.isEmpty()) {
                                event = mEventQueue.remove(0);
                                break;
                            }

                            // Update the pause state.
                            if (mPaused != mRequestPaused) {
                                mPaused = mRequestPaused;
                                GL_THREAD_MANAGER.notifyAll();
                                if (LOG_PAUSE_RESUME) {
                                    Log.i("GLThread", "mPaused is now " + mPaused + " tid=" + getId());
                                }
                            }

                            // Do we need to give up the EGL context?
                            if (mShouldReleaseEglContext) {
                                if (LOG_SURFACE) {
                                    Log.i("GLThread", "releasing EGL context because asked to tid=" + getId());
                                }
                                stopEglSurfaceLocked();
                                stopEglContextLocked();
                                mShouldReleaseEglContext = false;
                                askedToReleaseEglContext = true;
                            }

                            // Have we lost the EGL context?
                            if (lostEglContext) {
                                stopEglSurfaceLocked();
                                stopEglContextLocked();
                                lostEglContext = false;
                            }

                            // Do we need to release the EGL surface?
                            if (mHaveEglSurface && mPaused) {
                                if (LOG_SURFACE) {
                                    Log.i("GLThread", "releasing EGL surface because paused tid=" + getId());
                                }
                                stopEglSurfaceLocked();
                                if (GL_THREAD_MANAGER.shouldReleaseEGLContextWhenPausing()) {
                                    stopEglContextLocked();
                                    if (LOG_SURFACE) {
                                        Log.i("GLThread", "releasing EGL context because paused tid=" + getId());
                                    }
                                }
                                if (GL_THREAD_MANAGER.shouldTerminateEGLWhenPausing()) {
                                    mEglHelper.finish();
                                    if (LOG_SURFACE) {
                                        Log.i("GLThread", "terminating EGL because paused tid=" + getId());
                                    }
                                }

                                // Notify render that GLThread is going to pause
                                mRenderer.onPaused();
                            }

                            // Have we lost the surface view surface?
                            if ((!mHasSurface) && (!mWaitingForSurface)) {
                                if (LOG_SURFACE) {
                                    Log.i("GLThread", "noticed surfaceView surface lost tid=" + getId());
                                }
                                if (mHaveEglSurface) {
                                    stopEglSurfaceLocked();
                                }
                                mWaitingForSurface = true;

                                // Notify the wait in surfaceDestroyed() to avoid ANR
                                // with low memory case, Activity Manager destroys hardware resource of SurfaceTexture directly.
                                GL_THREAD_MANAGER.notifyAll();
                            }

                            // Have we acquired the surface view surface?
                            if (mHasSurface && mWaitingForSurface) {
                                if (LOG_SURFACE) {
                                    Log.i("GLThread", "noticed surfaceView surface acquired tid=" + getId());
                                }
                                mWaitingForSurface = false;
                                GL_THREAD_MANAGER.notifyAll();
                            }

                            if (doRenderNotification) {
                                if (LOG_SURFACE) {
                                    Log.i("GLThread", "sending render notification tid=" + getId());
                                }
                                wantRenderNotification = false;
                                doRenderNotification = false;
                                mRenderComplete = true;
                                GL_THREAD_MANAGER.notifyAll();
                            }
                            // Ready to draw?
                            if (readyToDraw()) {
                                // If we don't have an EGL context, try to acquire one.
                                if (!mHaveEglContext) {
                                    if (askedToReleaseEglContext) {
                                        askedToReleaseEglContext = false;
                                    } else if (GL_THREAD_MANAGER.tryAcquireEglContextLocked(this)) {
                                        try {
                                            mEglHelper.start(mSurface);
                                        } catch (RuntimeException t) {
                                            GL_THREAD_MANAGER.releaseEglContextLocked(this);
                                            throw t;
                                        }
                                        mHaveEglContext = true;
                                        createEglContext = true;

                                        GL_THREAD_MANAGER.notifyAll();
                                    }
                                }

                                if (mHaveEglContext && !mHaveEglSurface) {
                                    mHaveEglSurface = true;
                                    sizeChanged = true;
                                }

                                if (mHaveEglSurface) {
                                    if (mSizeChanged) {
                                        sizeChanged = true;
                                        wantRenderNotification = true;
                                        if (LOG_SURFACE) {
                                            Log.i("GLThread", "noticing that we want render notification tid=" + getId());
                                        }

                                        if (DRAW_TWICE_AFTER_SIZE_CHANGED) {
                                            // We keep mRequestRender true so that we draw twice after the size changes.
                                            // (Once because of mSizeChanged, the second time because of mRequestRender.)
                                            // This forces the updated graphics onto the screen.
                                        } else {
                                            mRequestRender = false;
                                            mSurfaceTextureReady = false;
                                        }
                                        mSizeChanged = false;
                                    } else {
                                        mRequestRender = false;
                                        mSurfaceTextureReady = false;
                                    }
                                    GL_THREAD_MANAGER.notifyAll();
                                    break;
                                }
                            }

                            // By design, this is the only place in a GLThread thread where we wait().
                            if (LOG_THREADS) {
                                Log.i("GLThread", "waiting tid=" + getId()
                                    + " mHaveEglContext: " + mHaveEglContext
                                    + " mHaveEglSurface: " + mHaveEglSurface
                                    + " mPaused: " + mPaused
                                    + " mHasSurface: " + mHasSurface
                                    + " mSurfaceTextureReady: " + mSurfaceTextureReady
                                    + " mWaitingForSurface: " + mWaitingForSurface
                                    + " mWidth: " + mWidth
                                    + " mHeight: " + mHeight
                                    + " mRequestRender: " + mRequestRender
                                    + " mRenderMode: " + mRenderMode);
                            }
                            GL_THREAD_MANAGER.wait();

                        }
                    } // end of synchronized(GL_THREAD_MANAGER)

                    if (event != null) {
                        event.run();
                        event = null;
                        continue;
                    }

                    if (!mHasSurface) {
                        continue;
                    }

                    if (createEglContext) {
                        if (LOG_RENDERER) {
                            Log.w("GLThread", "onSurfaceCreated");
                        }
                        gl = (GL10) mEglHelper.mEglContext.getGL();
                        mRenderer.onSurfaceCreated(gl, mEglHelper.mEglConfig);
                        createEglContext = false;
                    }

                    if (sizeChanged) {
                        if (LOG_RENDERER) {
                            Log.w("GLThread", "onSurfaceChanged(" + mWidth + ", " + mHeight + ")");
                        }
                        mEglHelper.purgeBuffers();
                        mRenderer.onSurfaceChanged(gl, mWidth, mHeight);
                        sizeChanged = false;
                    }

                    if (LOG_RENDERER_DRAW_FRAME) {
                        Log.w("GLThread", "onDrawFrame tid=" + getId());
                    }

                    mRenderer.onDrawFrame(gl);

                    if (!mEglHelper.swap()) {
                        if (LOG_SURFACE) {
                            Log.i("GLThread", "egl context lost tid=" + getId());
                        }
                        lostEglContext = true;
                    }

                    if (wantRenderNotification) {
                        doRenderNotification = true;
                    }
                }

            } finally {
                /*
                 * clean-up everything...
                 */
                synchronized (GL_THREAD_MANAGER) {
                    stopEglSurfaceLocked();
                    stopEglContextLocked();
                }
            }
        }

        public boolean ableToDraw() {
            return mHaveEglContext && mHaveEglSurface && readyToDraw();
        }

        private boolean readyToDraw() {
            return (!mPaused) && mHasSurface
                && (mWidth > 0) && (mHeight > 0)
                && (mRequestRender || (mRenderMode == RENDERMODE_CONTINUOUSLY))
                && mSurfaceTextureReady;
        }

        public void setRenderMode(int renderMode) {
            if (!((RENDERMODE_WHEN_DIRTY <= renderMode) && (renderMode <= RENDERMODE_CONTINUOUSLY))) {
                throw new IllegalArgumentException("renderMode");
            }

            synchronized (GL_THREAD_MANAGER) {
                mRenderMode = renderMode;
                GL_THREAD_MANAGER.notifyAll();
            }
        }

        public int getRenderMode() {
            synchronized (GL_THREAD_MANAGER) {
                return mRenderMode;
            }
        }

        public void requestRender() {
            synchronized (GL_THREAD_MANAGER) {
                mRequestRender = true;
                GL_THREAD_MANAGER.notifyAll();
            }
        }

        /*
         * We need to sync OpneGL render streaming with SurfaceTexture updated
         */
        public void setSurfaceTextureReady() {
            synchronized (GL_THREAD_MANAGER) {
                mSurfaceTextureReady = true;
                GL_THREAD_MANAGER.notifyAll();
            }
        }

        public void surfaceCreated(SurfaceTexture surfaceTexture, int width, int height) {
            synchronized (GL_THREAD_MANAGER) {
                if (LOG_THREADS) {
                    Log.i("GLThread", "surfaceCreated tid=" + getId());
                }
                mWidth = width;
                mHeight = height;
                mSurface = surfaceTexture;
                mSurfaceTextureReady = true;
                mHasSurface = true;
                GL_THREAD_MANAGER.notifyAll();
                while ((mWaitingForSurface) && (!mExited)) {
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void surfaceDestroyed() {
            synchronized (GL_THREAD_MANAGER) {
                if (LOG_THREADS) {
                    Log.i("GLThread", "surfaceDestroyed tid=" + getId());
                }

                mHasSurface = false;
                mSurfaceTextureReady = false;
                GL_THREAD_MANAGER.notifyAll();
                while ((!mWaitingForSurface) && (!mExited)) {
                    Log.i("Main thread", "surfaceDestroyed waiting for !mWaitingForSurface.");
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }
        public void onPause() {
            synchronized (GL_THREAD_MANAGER) {
                if (LOG_PAUSE_RESUME) {
                    Log.i("GLThread", "onPause tid=" + getId());
                }
                mSurfaceTextureReady = true;
                mRequestPaused = true;
                GL_THREAD_MANAGER.notifyAll();
                while ((!mExited) && (!mPaused)) {
                    if (LOG_PAUSE_RESUME) {
                        Log.i("Main thread", "onPause waiting for mPaused.");
                    }
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onResume() {
            synchronized (GL_THREAD_MANAGER) {
                if (LOG_PAUSE_RESUME) {
                    Log.i("GLThread", "onResume tid=" + getId());
                }
                mRequestPaused = false;
                mRequestRender = true;
                mRenderComplete = false;
                GL_THREAD_MANAGER.notifyAll();
                // No need to wait really resume if there is no surface,
                while ((!mExited) && mPaused && (!mRenderComplete) && (mHasSurface)) {
                    if (LOG_PAUSE_RESUME) {
                        Log.i("Main thread", "onResume waiting for !mPaused.");
                    }
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void onWindowResize(int w, int h) {
            synchronized (GL_THREAD_MANAGER) {
                mWidth = w;
                mHeight = h;
                mSizeChanged = true;
                mRequestRender = true;
                mSurfaceTextureReady = true;
                mRenderComplete = false;
                GL_THREAD_MANAGER.notifyAll();

                // Wait for thread to react to resize and render a frame
                while (!mExited && !mPaused && !mRenderComplete
                    && (mGLThread != null && mGLThread.ableToDraw())) {
                    if (LOG_SURFACE) {
                        Log.i("Main thread", "onWindowResize waiting for render complete from tid=" + mGLThread.getId());
                    }
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
        }

        public void requestExitAndWait() {
            // don't call this from GLThread thread or it is a guaranteed
            // deadlock!
            synchronized (GL_THREAD_MANAGER) {
                mShouldExit = true;
                GL_THREAD_MANAGER.notifyAll();
                while (!mExited) {
                    try {
                        GL_THREAD_MANAGER.wait();
                    } catch (InterruptedException ex) {
                        Thread.currentThread().interrupt();
                    }
                }
            }
            mSurface = null;
        }

        public void requestReleaseEglContextLocked() {
            mShouldReleaseEglContext = true;
            GL_THREAD_MANAGER.notifyAll();
        }

        /**
         * Queue an "event" to be run on the GL rendering thread.
         * @param r the runnable to be run on the GL rendering thread.
         */
        public void queueEvent(Runnable r) {
            if (r == null) {
                throw new IllegalArgumentException("r must not be null");
            }
            synchronized (GL_THREAD_MANAGER) {
                mEventQueue.add(r);
                GL_THREAD_MANAGER.notifyAll();
            }
        }

        // Once the thread is started, all accesses to the following member
        // variables are protected by the GL_THREAD_MANAGER monitor
        private boolean mShouldExit;
        private boolean mExited;
        private boolean mRequestPaused;
        private boolean mPaused;
        private boolean mHasSurface;
        private boolean mWaitingForSurface;
        private boolean mHaveEglContext;
        private boolean mHaveEglSurface;
        private boolean mShouldReleaseEglContext;
        private int mWidth;
        private int mHeight;
        private int mRenderMode;
        private boolean mRequestRender;
        private boolean mRenderComplete;
        private final ArrayList<Runnable> mEventQueue = new ArrayList<Runnable>();

        // End of member variables protected by the GL_THREAD_MANAGER monitor.

        private final Renderer mRenderer;
        private EglHelper mEglHelper;

        private SurfaceTexture mSurface;
        // Sync with SurfaceTexture
        private boolean mSurfaceTextureReady;
    }

    private void checkRenderThreadState() {
        if (mGLThread != null) {
            throw new IllegalStateException(
                "setRenderer has already been called for this instance.");
        }
    }

    private static class GLThreadManager {
        private static final String TAG = "GLThreadManager";

        @SuppressWarnings("PMD")
        public void threadExiting(GLThread thread) {
            synchronized (this) {
                if (LOG_THREADS) {
                    Log.i("GLThread", "exiting tid=" + thread.getId());
                }
                thread.mExited = true;
                if (mEglOwner == thread) {
                    mEglOwner = null;
                }
                notifyAll();
            }
        }

        /*
         * Tries once to acquire the right to use an EGL
         * context. Does not block. Requires that we are already
         * in the GL_THREAD_MANAGER monitor when this is called.
         *
         * @return true if the right to use an EGL context was acquired.
         */
        @SuppressWarnings("PMD")
        public boolean tryAcquireEglContextLocked(GLThread thread) {
            if (mEglOwner == thread || mEglOwner == null) {
                mEglOwner = thread;
                notifyAll();
                return true;
            }
            checkGLESVersion();
            if (mMultipleGLESContextsAllowed) {
                return true;
            }
            // Notify the owning thread that it should release the context.
            // TODO: implement a fairness policy. Currently
            // if the owning thread is drawing continuously it will just
            // reacquire the EGL context.
            if (mEglOwner != null) {
                mEglOwner.requestReleaseEglContextLocked();
            }
            return false;
        }

        /*
         * Releases the EGL context. Requires that we are already in the
         * GL_THREAD_MANAGER monitor when this is called.
         */
        @SuppressWarnings("PMD")
        public void releaseEglContextLocked(GLThread thread) {
            if (mEglOwner == thread) {
                mEglOwner = null;
            }
            notifyAll();
        }

        public boolean shouldReleaseEGLContextWhenPausing() {
            // Release the EGL context when pausing even if
            // the hardware supports multiple EGL contexts.
            // Otherwise the device could run out of EGL contexts.
            synchronized (this) {
                return LIMITED_GLES_CONTEXTS;
            }
        }

        public boolean shouldTerminateEGLWhenPausing() {
            synchronized (this) {
                checkGLESVersion();
                return !mMultipleGLESContextsAllowed;
            }
        }

        private void checkGLESVersion() {
            if (!mGLESVersionCheckComplete) {
                mGLESVersion = SystemProperties.getInt(
                    "ro.opengles.version",
                    ConfigurationInfo.GL_ES_VERSION_UNDEFINED);
                if (mGLESVersion >= GLES20) {
                    mMultipleGLESContextsAllowed = true;
                }
                if (LOG_SURFACE) {
                    Log.w(TAG, "checkGLESVersion mGLESVersion ="
                        + " " + mGLESVersion + " mMultipleGLESContextsAllowed = " + mMultipleGLESContextsAllowed);
                }
                mGLESVersionCheckComplete = true;
            }
        }

        private boolean mGLESVersionCheckComplete;
        private int mGLESVersion;
        private boolean mMultipleGLESContextsAllowed;
        private static final boolean LIMITED_GLES_CONTEXTS = true;
        private static final int GLES20 = 0x20000;
        private GLThread mEglOwner;
    }

    private static final GLThreadManager GL_THREAD_MANAGER = new GLThreadManager();
    private boolean mSizeChanged = true;

    private GLThread mGLThread;
    private Renderer mRenderer;
    private boolean mDetached;
    private EGLConfigChooser mEGLConfigChooser;
    private EGLContextFactory mEGLContextFactory;
    private EGLWindowSurfaceFactory mEGLWindowSurfaceFactory;
    private int mEGLContextClientVersion;

}
