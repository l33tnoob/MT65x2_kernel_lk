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
/** \file
 * Ngin3D Presentation Layer for J3M Engine
 */
package com.mediatek.ngin3d.j3m;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.util.Log;
import com.mediatek.j3m.AngularUnits;
import com.mediatek.j3m.AnimationController;
import com.mediatek.j3m.AssetPool;
import com.mediatek.j3m.Camera;
import com.mediatek.j3m.J3m;
import com.mediatek.j3m.Ray;
import com.mediatek.j3m.RenderBlock;
import com.mediatek.j3m.RenderBlockGroup;
import com.mediatek.j3m.Renderer;
import com.mediatek.j3m.SceneNode;
import com.mediatek.j3m.Version;
import com.mediatek.ja3m.A3mJ3m;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Transaction;
import com.mediatek.ngin3d.animation.MasterClock;
import com.mediatek.ngin3d.presentation.Graphics2d;
import com.mediatek.ngin3d.presentation.Graphics3d;
import com.mediatek.ngin3d.presentation.ILightPresentation;
import com.mediatek.ngin3d.presentation.IObject3d;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.Model3d;
import com.mediatek.ngin3d.presentation.Presentation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.presentation.RenderLayer;
import com.mediatek.ngin3d.presentation.VideoDisplay;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Implementation of presentation engine using J3M.
 *
 * @hide
 */
public class J3mPresentationEngine implements PresentationEngine {
    private static final String TAG = "J3mPresentationEngine";

    // Follows format (major, minor, patch)
    private static final int[] MINIMUM_A3M_VERSION = {1, 0, 0};

    private static final int RENDER_LAYER_COUNT = 4;

    private final Stage mStage;
    private final TextureCache mTextureCache;
    private Resources mResources;
    private J3m mJ3m;
    private RenderBlock[] mRenderBlocks = new RenderBlock[RENDER_LAYER_COUNT];
    private RenderBlockGroup mRenderBlockGroup;
    private Renderer mRenderer;
    private RenderFlags mRenderFlags;
    private AssetPool mAssetPool;
    private SceneNode mTrueRootNode;
    private SceneNode mRootNode;
    private Ray mHitTestRay;
    private final Set<AnimationController> mAnimationControllers =
        new HashSet<AnimationController>();

    // Camera device, local copy of position, and scene node it is pointed at
    private Camera mCamera;
    private Point mCameraPos = new Point(0.0f, 0.0f, 0.0f);
    private Point mCameraLookAt = new Point(0.0f, 0.0f, -1.0f);
    private float mCameraFov = 40.0f;
    private float mCameraWidth;

    // screen dimensions set on initialisation
    private int mWidth;
    private int mHeight;

    private int mProjectionMode = Stage.UI_PERSPECTIVE;

    // Z-clipping values
    private float mZNear = 2.f;
    private float mZFar = 3000.f;

    // Stereo: Distance in camera-Z to the plane where stereo cameras match
    private float mCameraFocalLength;
    // Distance between 'eye's.  Set zero to disable by default.
    private float mCameraEyeSeparation;

    private boolean mRenderingPaused;

    // Timing-related
    private static final int NANOSECS_PER_SECOND = 1000000000;
    private boolean mIsReady;
    private long mLastTime;
    private long mTimeOrigin;
    private int mMaxFPS;
    private FpsLimiter mFpsLimiter;
    private int mFrameCount;
    private long mFrameCountingStartTimeNs;
    private double mFPS;

    protected RenderCallback mRenderCallback;

    /**
     * Imports Stage object to initialize this J3M Presentation Engine.
     *
     * @param stage stage object to be used for this engine
     */
    public J3mPresentationEngine(Stage stage) {
        mStage = stage;
        mTextureCache = new TextureCache(this);
    }

    /**
     * Initializes this  with specific width and height
     *
     * @param width     in pixels
     * @param height    in pixels
     * @param resources Resources
     * @param cacheDir  Folder for caching items
     * @param libDir customer lib folder, for widget used only
     */
    public void initialize(int width, int height, Resources resources,
                           String cacheDir, String libDir) {
        Log.d(TAG, "J3mPresentationEngine initialize");
        if (libDir == null) {
            System.loadLibrary("a3m");
            System.loadLibrary("ja3m");
        } else {
            System.load(libDir + "/libja3m.so");
        }

        mWidth = width;
        mHeight = height;

        mJ3m = new A3mJ3m();

        Version minimumVersion = mJ3m.createVersion(
                MINIMUM_A3M_VERSION[0],
                MINIMUM_A3M_VERSION[1],
                MINIMUM_A3M_VERSION[2]);

        Version currentVersion = mJ3m.getVersion();

        if (currentVersion.isLessThan(minimumVersion)) {
            Log.w(TAG, "A3M version is out-of-date: "
                    + currentVersion.getMajor() + "."
                    + currentVersion.getMinor() + "."
                    + currentVersion.getPatch() + " < "
                    + minimumVersion.getMajor() + "."
                    + minimumVersion.getMinor() + "."
                    + minimumVersion.getPatch());
        }

        Log.i(TAG, "Using A3M version: "
                + currentVersion.getMajor() + "."
                + currentVersion.getMinor() + "."
                + currentVersion.getPatch() + " ("
                + currentVersion.getExtra() + ")");

        Log.i(TAG, "A3M Build Information:\n" + mJ3m.getBuildInfo());

        mRenderFlags = new RenderFlags(mJ3m);
        mAssetPool = mJ3m.createAssetPool();
        mHitTestRay = mJ3m.createRay();

        mCameraPos = new Point(0.0f, 0.0f, -1.0f);

        // If Resources were passed, register them with the asset pool
        if (resources != null) {
            mAssetPool.registerSource(resources);
            mAssetPool.registerSource(resources.getAssets());

            mResources = resources;
        }

        mAssetPool.registerSource("//sdcard/ngin3d/assets");

        if (cacheDir != null) {
            mAssetPool.setCacheSource(cacheDir);
        }

        mFpsLimiter = new FpsLimiter();

        // Info: The existance of both TrueRootNode and RootNode allows us
        // to map between different coordinate systems.  For example it's used
        // later to map between Perspective (Y-up) and UI-Perspective (Y-down)
        mTrueRootNode = mJ3m.createSceneNode();
        mRootNode = mJ3m.createSceneNode();
        mRootNode.setParent(mTrueRootNode);

        // Setting up a nominal camera on initialisation prevents a null
        // camera being given to the renderer.
        mCamera = mJ3m.createCamera();
        mCamera.setParent(mRootNode);

        mRenderer = mJ3m.createRenderer(mAssetPool);
        mRenderBlockGroup = mJ3m.createRenderBlockGroup();

        // Create the render layers.
        for (int i = 0; i < RENDER_LAYER_COUNT; ++i) {
            mRenderBlocks[i] = mJ3m.createRenderBlock(
                    mRenderer, mRootNode, mCamera);
            mRenderBlockGroup.addBlock(mRenderBlocks[i]);

            if (i > 0) {
                mRenderBlocks[i].setColourClear(false);
            }
        }

        mRenderBlocks[0].setRenderFlags(mRenderFlags.VISIBLE.or(
                    mRenderFlags.RENDER_LAYER_BIT_0.inverse()).or(
                    mRenderFlags.RENDER_LAYER_BIT_1.inverse()), mRenderFlags.VISIBLE);

        mRenderBlocks[1].setRenderFlags(mRenderFlags.VISIBLE.or(
                    mRenderFlags.RENDER_LAYER_BIT_0).or(
                    mRenderFlags.RENDER_LAYER_BIT_1.inverse()), mRenderFlags.VISIBLE);

        mRenderBlocks[2].setRenderFlags(mRenderFlags.VISIBLE.or(
                    mRenderFlags.RENDER_LAYER_BIT_0.inverse()).or(
                    mRenderFlags.RENDER_LAYER_BIT_1), mRenderFlags.VISIBLE);

        mRenderBlocks[3].setRenderFlags(mRenderFlags.VISIBLE.or(
                    mRenderFlags.RENDER_LAYER_BIT_0).or(
                    mRenderFlags.RENDER_LAYER_BIT_1), mRenderFlags.VISIBLE);

        // Initialise camera setup
        updateCamera();

        enableMipMaps(true);
        mStage.realize(this);

        mTimeOrigin = System.nanoTime();
        mLastTime = mTimeOrigin;

        mIsReady = true;
    }

    public void initialize(int width, int height) {
        initialize(width, height, null, null);
    }

    public void initialize(int width, int height, Resources resources) {
        initialize(width, height, resources, null);
    }

    public void initialize(int width, int height, Resources resources, String cacheDir) {
        initialize(width, height, resources, cacheDir, null);
    }

    /**
     * Uninitialize this object.
     */
    public void uninitialize() {
        Log.d(TAG, "J3mPresentationEngine uninitialize ");
        mIsReady = false;
        mStage.unrealize();

        // Explicitly release device resources
        if (mAssetPool != null) {
            mAssetPool.release();
        }

        // Do the last tick so that animation can complete.
        MasterClock.getDefault().tick();

        // Make sRenderCallback of Transaction null to avoid memory leakage.
        setRenderCallback(null);
        mRenderer = null;
        mAssetPool = null;
        mTrueRootNode = null;
        mRootNode = null;
        mCamera = null;
        mCameraPos = null;
    }

    // ------------------------------------------------------------------------
    // Camera and projections
    // ------------------------------------------------------------------------

    // Update the projection following a change of screen size (i.e.
    // orientation) or camera position (posn.z used in FoV calc), etc.
    private void applyProjection() {

        switch (mProjectionMode) {
        case Stage.ORTHOGRAPHIC:
            setOrthographicProjection();
            break;

        case Stage.PERSPECTIVE:
            setClassicPerspectiveProjection();
            break;

        case Stage.UI_PERSPECTIVE:
            setUiPerspectiveProjection();
            break;

        default:
            break;
        }

        // Update common parameters.
        mCamera.setNear(mZNear);
        mCamera.setFar(mZFar);
    }

    /**
     * Set new projection mode
     */
    public void setProjectionMode(int mode) {
        mProjectionMode = mode;
        updateCamera();
    }

    /**
     * Set the near and far clipping distances
     */
    public void setClipDistances(float zNear, float zFar) {
        mZNear = zNear;
        mZFar = zFar;
        updateCamera();
    }

    /**
     * Set camera Z position.
     * Mainly relevant to the UI_PERSPECTIVE view where X and Y are fixed
     */
    public void setCameraZ(float zCamera) {
        mCameraPos.z = zCamera;
        updateCamera();
    }

    /**
     * Set camera field of view in degrees.
     */
    public void setCameraFov(float fov) {
        mCameraFov = (float)Math.toRadians(fov);
        updateCamera();
    }

    /**
     * Set camera orthographic frustum width.
     */
    public void setCameraWidth(float width) {
        mCameraWidth = width;
        updateCamera();
    }

    private void setOrthographicProjection() {
        mCamera.setProjectionType(Camera.ProjectionType.ORTHOGRAPHIC);

        // To maintain backwards compatibility, automatically set the camera
        // width to equal the screen width in pixels if the width is zero.
        mCamera.setWidth(mCameraWidth > 0.0f ? mCameraWidth : mWidth);
    }

    /**
     * Operations necessary when switching to a "UI" perspective projection.
     * Mainly - calculate the field of view to pass to the engine via
     * the camera properties, and recalculate the projection matrix.
     */
    private void setUiPerspectiveProjection() {

        // Ensure camera exists
        if (mCamera == null) {
            Log.e(TAG, "No camera defined in setUiPerspectiveProjection");
            return;
        }

        // Initially assume portrait
        float smallerDim = (float) mWidth;

        // correct if landscape
        if (mWidth > mHeight) {
            smallerDim = (float) mHeight;
        }

        float distToScreen = Math.abs(mCameraPos.z);

        float fov = (float) (Math.atan((smallerDim / 2)
                    / Math.abs(distToScreen)) * 2);

        mCamera.setProjectionType(Camera.ProjectionType.PERSPECTIVE);
        mCamera.setFov(AngularUnits.RADIANS, fov);
    }

    /**
     * Operations necessary when switching to a vanilla perspective projection.
     */
    private void setClassicPerspectiveProjection() {
        // FOV is set by the client in PERSPECTIVE projection mode
        mCamera.setFov(AngularUnits.RADIANS, mCameraFov);
    }

    /**
     * Updates the camera according to the current projection mode.
     */
    private void updateCamera() {

        // Abort if there's no camera
        if (mCamera == null) {
            Log.e(TAG, "No camera defined in updateCamera");
            return;
        }

        float cameraPositionX = 0.0f;
        float cameraPositionY = 0.0f;
        float cameraPositionZ = 0.0f;
        float targetPositionX = 0.0f;
        float targetPositionY = 0.0f;
        float targetPositionZ = 0.0f;
        float cameraUpX = 0.0f;
        float cameraUpY = 1.0f;
        float cameraUpZ = 0.0f;

        switch (mProjectionMode) {
        case Stage.ORTHOGRAPHIC:
        case Stage.PERSPECTIVE:
            // Camera setup is fully specified by client
            cameraPositionX = mCameraPos.x;
            cameraPositionY = mCameraPos.y;
            cameraPositionZ = mCameraPos.z;
            targetPositionX = mCameraLookAt.x;
            targetPositionY = mCameraLookAt.y;
            targetPositionZ = mCameraLookAt.z;
            break;

        case Stage.UI_PERSPECTIVE:
            // Camera is fixed mid-screen
            float centX = (float) mWidth / 2;
            float centY = (float) mHeight / 2;

            cameraPositionX = centX;
            cameraPositionY = centY;
            cameraPositionZ = mCameraPos.z;
            targetPositionX = centX;
            targetPositionY = centY;
            targetPositionZ = 0.0f;

            // Rotate the camera upside-down
            cameraUpY = -1.0f;
            break;
        default:
            break;
        }

        // Set positions of camera and look-at point
        mCamera.setPosition(cameraPositionX, cameraPositionY, cameraPositionZ);
        mCamera.point(
                targetPositionX - cameraPositionX,
                targetPositionY - cameraPositionY,
                targetPositionZ - cameraPositionZ,
                cameraUpX, cameraUpY, cameraUpZ);

        // Camera position used in calculation of FoV etc.
        applyProjection();
    }

    /**
     * Positions and aims the camera for this stage.
     * Only the camera Z position is used in UI_PERSPECTIVE projection mode.
     *
     * @param pos    camera position
     * @param lookAt camera focus point position
     */
    public void setCamera(Point pos, Point lookAt) {
        mCameraPos = pos;
        mCameraLookAt = lookAt;

        updateCamera();
    }

    /**
     * Sets the currently active debug camera.
     * Passing an empty string activates the default camera.
     *
     * @deprecated This method is wrongly named and was not intended to be
     * public, so and may change in the future.
     */
    public void setDebugCamera(String name) {
        if (name.isEmpty()) {
            for (RenderBlock block : mRenderBlocks) {
                block.setCamera(mCamera);
            }
        } else {
            SceneNode node = mRootNode.find(name);

            if (Camera.class.isInstance(node)) {
                for (RenderBlock block : mRenderBlocks) {
                    block.setCamera((Camera) node);
                }
            }
        }
    }

    /**
     * Returns a list of names of cameras in the scene.
     *
     * @deprecated This method is wrongly named and was not intended to be
     * public, so and may change in the future.
     */
    public String[] getDebugCameraNames() {
        List<String> names = new ArrayList<String>();
        compileDebugCameraNames(names, mRootNode);
        String[] namesArray = new String[names.size()];
        names.toArray(namesArray);
        return namesArray;
    }

    /**
     * Traverses the scene graph and compiles a list of all the cameras.
     *
     * This function is deprecated and marked for removal in the near future.
     *
     * @deprecated
     */
    private void compileDebugCameraNames(List<String> names, SceneNode node) {
        int childCount = node.getChildCount();
        for (int i = 0; i < childCount; ++i) {
            SceneNode child = node.getChild(i);

            String name = child.getName();
            if ((!name.isEmpty()) && Camera.class.isInstance(child)) {
                names.add(name);
            }

            compileDebugCameraNames(names, child);
        }
    }

    /**
     * Sets the object size with new value.
     * Typically part of the response to an onSurfaceChanged()
     *
     * @param width  in pixels
     * @param height in pixels
     */
    public void resize(int width, int height) {
        mWidth = width;
        mHeight = height;
        updateCamera();

        // Resize the viewport to match the screen size.
        for (RenderBlock block : mRenderBlocks) {
            block.setViewport(0, 0, mWidth, mHeight);
        }
    }

    /**
     * Adds an AnimationController to be updated.
     */
    public void addAnimationController(AnimationController controller) {
        mAnimationControllers.add(controller);
    }

    /**
     * Removes an AnimationController being updated.
     */
    public void removeAnimationController(AnimationController controller) {
        mAnimationControllers.remove(controller);
    }

    // ------------------------------------------------------------------------
    // Scene effects
    // ------------------------------------------------------------------------

    /**
     * Sets the global fog density.
     *
     * @param density Fog density
     */
    public void setFogDensity(float density) {
        mRenderer.setProperty("FOG_DENSITY", density);
    }

    /**
     * Sets the global fog color.
     *
     * @param color Fog color
     */
    public void setFogColor(Color color) {
        mRenderer.setProperty("FOG_COLOUR",
                color.red / 255.0f, color.green / 255.0f,
                color.blue / 255.0f, color.alpha / 255.0f);
    }

    // ------------------------------------------------------------------------
    // Rendering
    // ------------------------------------------------------------------------

    /**
     * Process all transactions and render the scene
     *
     * @return true if the render process is successful.
     */
    public boolean render() {
        if (mRenderingPaused) {
            return false;
        }

        // Calculate the time elapsed since the last render.
        long time = System.nanoTime();
        float dt = (time - mLastTime) / 1e9f;
        mLastTime = time;

        // Tick the clock to do animation and make Stage dirty
        MasterClock.getDefault().tick();

        // Update all animations.
        for (AnimationController controller : mAnimationControllers) {
            controller.update(dt);

            if (controller.getEnabled() && controller.isFinished()) {
                // Do not update object to start of animation.
                controller.stop(false);
            }
        }

        // Apply transaction for animations.
        Transaction.applyOperations();

        // Check stage is dirty or is there any animation running.
        boolean dirty = mStage.isDirty() || mStage.isAnimationStarted();

        // Apply all property changes into scene graph
        mStage.applyChanges(this);

        // Flush unused assets and resources
        mAssetPool.flush();

        Color bkgColor = mStage.getBackgroundColor();

        mRenderBlocks[0].setBackgroundColour(
                bkgColor.red / 255.0f, bkgColor.green / 255.0f,
                bkgColor.blue / 255.0f, bkgColor.alpha / 255.0f);

        float updateTime = (float) (System.nanoTime() - mTimeOrigin)
            / (float) NANOSECS_PER_SECOND;
        mRenderBlockGroup.update(updateTime);
        mRenderBlockGroup.render();

        // Calculate and fix frame rate
        mFpsLimiter.calculateAndFixFrameTime();

        long now = System.nanoTime();
        if (mFrameCountingStartTimeNs == 0) {
            mFrameCountingStartTimeNs = now;
        } else if ((now - mFrameCountingStartTimeNs) > NANOSECS_PER_SECOND) {
            mFPS = (double) mFrameCount * NANOSECS_PER_SECOND / (now - mFrameCountingStartTimeNs);
            mFrameCountingStartTimeNs = now;
            mFrameCount = 0;
        }
        ++mFrameCount;

        return dirty;
    }

    /**
     * Pause the rendering
     */
    public void pauseRendering() {
        mRenderingPaused = true;
        MasterClock.getDefault().pause();
    }

    /**
     * Resume the rendering.
     */
    public void resumeRendering() {
        mRenderingPaused = false;
        mLastTime = System.nanoTime();
        MasterClock.getDefault().resume();
    }

    /**
     * Check the rendering status
     *
     * @return the rendering is pause or not
     */
    public boolean isRenderingPaused() {
        return mRenderingPaused;
    }

    public void setRenderCallback(RenderCallback render) {
        Transaction.setRenderCallback(render);
        mRenderCallback = render;
    }

    /**
     * Sends a request to the engine to do the render process.
     */
    public void requestRender() {
        if (mRenderCallback != null) {
            mRenderCallback.requestRender();
        }
    }

    // ------------------------------------------------------------------------
    // Admin, Control, Getter/Setter
    // ------------------------------------------------------------------------

    /**
     * Returns the root J3M object.
     */
    public J3m getJ3m() {
        return mJ3m;
    }

    /**
     * Returns the presentation layer render flags.
     */
    public RenderFlags getRenderFlags() {
        return mRenderFlags;
    }

    /**
     * Returns a cached ray object which may be used for hit tests.
     */
    public Ray getHitTestRay() {
        return mHitTestRay;
    }

    /**
     * Returns the currently active camera.
     */
    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Returns the root scene node
     */
    public SceneNode getRootNode() {
        return mRootNode;
    }

    /**
     * Returns the root scene node
     */
    public SceneNode getRenderBlockParent() {
        return mTrueRootNode;
    }

    /**
     * Returns the asset pool object, used to load assets in J3M
     */
    public AssetPool getAssetPool() {
        return mAssetPool;
    }

    /**
     * Returns the texture cache, from which textures are loaded
     */
    public TextureCache getTextureCache() {
        return mTextureCache;
    }

    /**
     * Dump the properties of this object out.
     */
    public void dump() {
        // \todo implement
    }

    /**
     * Gets the width of the screen
     *
     * @return width value
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Gets the height of the screen
     *
     * @return height value
     */
    public int getHeight() {
        return mHeight;
    }

    public int getTotalCImageBytes() {
        // \todo implement
        return 0;
    }

    public int getTotalTextureBytes() {
        // \todo implement
        return 0;
    }

    /**
     * Return the Android Resources container.
     *
     * @return Android resources
     */
    protected Resources getResources() {
        return mResources;
    }

    /**
     * Create a special scene node with empty presentation engine setting.
     *
     * @return a new scene node presentation.
     */
    public Presentation createEmpty() {
        return new ActorPresentation(this);
    }

    /**
     * Create a container.
     *
     * @return a new scene node presentation.
     */
    public Presentation createContainer() {
        return new ActorPresentation(this);
    }

    /**
     * Create a image display object.
     *
     * @return a new rectangular scene node presentation.
     */
    public ImageDisplay createImageDisplay(boolean isYUp) {
        return new PlanePresentation(this, isYUp);
    }

    /**
     * Create a video display object.
     *
     * @return a new rectangular scene node presentation.
     */
    public VideoDisplay createVideoDisplay(boolean isYUp) {
        return new VideoPresentation(this, isYUp);
    }

    /**
     * Create basic 3D model.
     *
     * @param type model type, such as Model3d.CUBE or Model3d.Sphere
     * @return a new model 3D presentation.
     */
    public Model3d createModel3d(int type, boolean isYUp) {
        return new Basic3dPresentation(this, type, isYUp);
    }

    /**
     * Create object 3D model.
     *
     * @return a new model 3D presentation.
     */
    public IObject3d createObject3d() {
        return new Object3dPresentation(this);
    }

    /**
     * Create a 2D object for drawing.
     *
     * @return new 2D presentation object for graphic
     */
    public Graphics2d createGraphics2d(boolean isYUp) {
        return new Canvas2dPresentation(this, isYUp);
    }

    /**
     * Create a 3D object for drawing.
     *
     * @return new 3D presentation object for graphic
     */
    public Graphics3d createGraphics3d() {
        return new Canvas3dPresentation(this);
    }

    /**
     * Create a Render Layer.
     *
     * @return new object object representing a Render Layer
     */
    public RenderLayer createRenderLayer() {
        return new LayerPresentation(this, mRenderer);
    }


    /**
     * Create a Light.
     *
     * @return new object object representing a light
     */
    public ILightPresentation createLight() {
        return new LightPresentation(this);
    }

    public Object getScreenShot() {
        // Grab backbuffer into a byte array
        byte[] pixels = mJ3m.getPixels(0, 0, mWidth, mHeight);

        // Copy the pixel data into a buffer
        ByteBuffer buffer = ByteBuffer.allocate(pixels.length);
        buffer.put(pixels);
        buffer.rewind();

        // Construct a bitmap from the buffer
        Bitmap bitmap = Bitmap.createBitmap(mWidth, mHeight,
            Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(buffer);

        // Invert bitmap in Y by scaling by -1. Android and OpenGL use
        // different bitmap origins (top-left vs. bottom-left)
        Matrix matrix = new Matrix();
        matrix.setScale(1, -1, mWidth * 0.5f, mHeight * 0.5f);
        Bitmap flipBitmap = Bitmap.createBitmap(mWidth, mHeight,
            Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(flipBitmap);
        canvas.drawBitmap(bitmap, matrix, new Paint());

        return flipBitmap;
    }

    public void enableMipMaps(boolean enable) {
        // mVideoDriver.setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, enable);
    }

    /*
     * This is the 'ideal' ratio for the distance between the cameras and
     * the distance from camera(s) to the focal plane.
     */
    private static final float STEREO_EYE_DIST_FACTOR = 30f;

    /**
     * Enable/Configure stereocopic display
     * From http://paulbourke.net/miscellaneous/stereographics/stereorender/
     *
     * The degree of the stereo effect depends on both the distance of the
     * camera to the projection plane and the separation of the left and right
     * camera. Too large a separation can be hard to resolve and is known as
     * hyperstereo. A good ballpark separation of the cameras is 1/20 of the
     * distance to the projection plane, this is generally the maximum
     * separation for comfortable viewing. Another constraint in general
     * practice is to ensure the negative parallax (projection plane behind
     * the object) does not exceed the eye separation.
     *
     * A common measure is the parallax angle defined as
     * theta = 2 atan(DX / 2d) where DX is the horizontal separation of a
     * projected point between the two eyes and d is the distance of the eye
     * from the projection plane. For easy fusing by the majority of people,
     * the absolute value of theta should not exceed 1.5 degrees for all points
     * in the scene. Note theta is positive for points behind the scene and
     * negative for points in front of the screen. It is not uncommon to
     * restrict the negative value of theta to some value closer to zero since
     * negative parallax is more difficult to fuse especially when objects cut
     * the boundary of the projection plane.
     *
     * @param enable enable stereoscopic 3d effect.
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     * @param intensity Adjust the level of stereo separation. Normally 1.0,
     *                  1.1 increases the effect by 10%, for example.
     */
    public void enableStereoscopic3D(boolean enable, float focalDistance, float intensity) {
        if (enable) {
            mCameraFocalLength = focalDistance;
            mCameraEyeSeparation = (focalDistance / STEREO_EYE_DIST_FACTOR) * intensity;
        } else {
            mCameraEyeSeparation = 0.0f; // disables stereo
        }

        // Configure for stereo projection here
        mRenderBlockGroup.setStereo(mCameraFocalLength, mCameraEyeSeparation);
    }

    public boolean isStereo3dMode() {
        return (mCameraEyeSeparation != 0.0f);
    }

    /**
     * Checks whether the presentation has been initialized
     *
     * @return true if initialized
     */
    public boolean isReady() {
        return mIsReady;
    }

    // ------------------------------------------------------------------------
    // Framerate
    // ------------------------------------------------------------------------

    /**
     * get FPS.
     */
    public double getFPS() {
        return mFPS;
    }

    /**
     * Record the maximum of FPS number.
     *
     * @param fps fps number
     */
    public void setMaxFPS(int fps) {
        mMaxFPS = fps;
    }

    /**
     * Get time the last frame cost.
     *
     * @return the time of frame interval
     */
    public int getFrameInterval() {
        return mFpsLimiter.waitForFrameTime();
    }

    /*
     * This class is used to log and calculate the frame time
     */
    class FpsLimiter {
        private int mLastFrameTime;
        private long mTickTime;

        void update() {
            long now = MasterClock.getTime();
            if (mTickTime != 0) {
                mLastFrameTime = (int) (now - mTickTime);
            }
            mTickTime = now;
        }

        int waitForFrameTime() {
            synchronized (this) {
                try {
                    mFpsLimiter.wait();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            return mLastFrameTime;
        }

        private void calculateAndFixFrameTime() {
            synchronized (this) {
                mFpsLimiter.update();
                mFpsLimiter.notifyAll();
            }

            if (mMaxFPS > 0) {
                int period = 1000 / mMaxFPS;
                if (mLastFrameTime < period) {
                    try {
                        Thread.sleep((period - mLastFrameTime));
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }

    public void addRenderLayer(Presentation presentation) {
        assert presentation instanceof LayerPresentation;
        LayerPresentation layerPresentation =
                (LayerPresentation) presentation;
        mRenderBlockGroup.addBlock(layerPresentation.getRenderBlock());
        layerPresentation.getRootSceneNode().setParent(mTrueRootNode);
    }

    public int getLightZOrder() {
        return mJ3m.getLightZOrder();
    }
}
