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

package com.mediatek.ngin3d;

import android.content.res.Resources;
import android.util.Log;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.utils.JSON;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * Container for all Actors that are to be displayed.
 *
 * Stage is a special Container that displays its Actors on the screen.
 * Note that the Layer class may be used to construct a multiple-layer
 * stage scene.
 * @see Layer
 *
 */
public class Stage extends Container {
    private PresentationEngine mPresentationEngine;
    private final UiHandler mUiHandler;
    private static final ThreadLocal<Stage> THREAD_LOCAL_STAGE = new ThreadLocal<Stage>();

    /*
     * Projection modes
     */
    /** Orthographic projection mode. */
    public static final int ORTHOGRAPHIC = 0;
    /** Perspective projection mode. */
    public static final int PERSPECTIVE = 1;
    /** Perspective projection mode with fixed parameters to simplify UI applications. */
    public static final int UI_PERSPECTIVE = 2;
    /** Legacy projection mode name
     * @deprecated Use UI_PERSPECTIVE - LHC will be deleted without notice.
     */
    public static final int UI_PERSPECTIVE_LHC = 2;

    private static final int PM_MAX_LEGAL = 3; // highest legal value

    /**
     * It is important to name attached property as ATTACHED_PROP_*. If its name begins with
     * typical PROP_*, it will be treated as class-owned property and will not be dispatched
     * to property chain.
     */
    private static final Property<Stage> ATTACHED_PROP_ADD_LAYER
        = new Property<Stage>("layer", null);

    /**
     * Construct an empty stage [DEPRECATED].
     *
     * @deprecated Use Stage(UiHandler) instead.
     */
    @Deprecated
    public Stage() {
        // A dummy UI handle that run the specified runnable directly.
        this(new UiHandler() {
            public void post(Runnable runnable) {
                runnable.run();
            }
        });
    }

    /**
     * Construct an empty stage with specified UI handler.
     *
     * @param uiHandler Handler to run specified runnable in UI thread
     */
    public Stage(UiHandler uiHandler) {
        super();
        mUiHandler = uiHandler;
    }

    ///////////////////////////////////////////////////////////////////////////
    // public methods

    /**
     * Called to apply property changes to presentation engine for each frame rendering.
     *
     * @param presentationEngine presentation engine
     * @hide Presentation API should be encapsulated
     */
    public void applyChanges(PresentationEngine presentationEngine) {
        super.realize(presentationEngine);
    }

    /**
     * PresentationEngine will call this to initialize Stage.
     *
     * @param presentationEngine presentation engine
     * @hide Presentation API should be encapsulated
     */
    public void realize(PresentationEngine presentationEngine) {
        mPresentationEngine = presentationEngine;
        // Sometimes We need tasks to run on UI thread when current thread is GL Thread.
        // So we have to access UI handler when the executing Thread is GL.
        // This stage get the UI handler when constructing, and this(realize) method is invoked in GL thread, so we store
        // the instance of this stage into ThreadLocal here.
        registerCurrentThread();

        super.realize(presentationEngine);
        reloadBitmapTexture();
    }

    /**
     * Invoke the method to register caller thread to have this stage.
     * In order to make Animation callback run on UI thread, you have to
     * register the thread which starts animation.
     * @hide
     */
    public void registerCurrentThread() {
        // Remember this stage in TLS so that rendering thread can get it later.
        THREAD_LOCAL_STAGE.set(this);
    }

    /**
     * For rendering thread to get current UI handler.
     *
     * @return UI handler
     * @hide Internal use only
     */
    public static UiHandler getUiHandler() {
        Stage stage = THREAD_LOCAL_STAGE.get();
        if (stage == null) {
            return null;
        } else {
            return stage.mUiHandler;
        }

    }

    /**
     * Gets the width of the stage window.
     *
     * @return Stage width
     */
    public int getWidth() {
        return mPresentationEngine.getWidth();
    }

    /**
     * Gets the height of the stage window.
     *
     * @return Stage height
     */
    public int getHeight() {
        return mPresentationEngine.getHeight();
    }

    /**
     * Get the duration of the last frame.
     *
     * @return Frame interval
     */
    public int getFrameInterval() {
        if (mPresentationEngine == null) {
            return 0;
        }
        return mPresentationEngine.getFrameInterval();
    }

    /**
     * Get Z order of lights in experimental renderer.
     *
     * @return Z light order
     */
    public int getLightZOrder() {
        if (mPresentationEngine == null) {
            return -1;
        }
        return mPresentationEngine.getLightZOrder();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Projections

    /**
     * Projection configuration.
     * @hide (Public only for the purpose of automated testing)
     */
    public static class ProjectionConfig implements JSON.ToJson {
        public int mode;
        public float zNear;
        public float zFar;
        public float zStage;

        public ProjectionConfig(int mode, float zNear, float zFar, float zStage) {
            this.mode = mode;
            this.zNear = zNear;
            this.zFar = zFar;
            this.zStage = zStage;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            ProjectionConfig that = (ProjectionConfig) o;

            if (that.mode != mode) return false;
            if (Float.compare(that.zFar, zFar) != 0) return false;
            if (Float.compare(that.zNear, zNear) != 0) return false;
            if (Float.compare(that.zStage, zStage) != 0) return false;

            return true;
        }

        @Override
        public int hashCode() {
            int result = mode;
            result = 31 * result + (zFar == +0.0f ? 0 : Float.floatToIntBits(zFar));
            result = 31 * result + (zNear == +0.0f ? 0 : Float.floatToIntBits(zNear));
            result = 31 * result + (zStage == +0.0f ? 0 : Float.floatToIntBits(zStage));
            return result;
        }

        @Override
        public String toString() {
            return "{Proj mode : " + mode + ", zNear : " + zNear
                + ", zFar : " + zFar + ", zStage : " + zStage + "}";
        }

        public String toJson() {
            return "{Proj mode : " + mode + ", zNear : " + zNear
                + ", zFar : " + zFar + ", zStage : " + zStage + "}";
        }

    }

    /**
     * Camera for the Stage.
     * Read-only methods. The camera must be configured via the Stage methods.
     */
    public static class Camera implements JSON.ToJson {
        /**
         * @hide Access via configuration methods
         */
        public Point position;
        /**
         * @hide Access via configuration methods
         */
        public Point lookAt;

        /**
         * @hide Access via configuration methods
         */
        public Camera(Point position, Point lookAt) {
            this.position = new Point(position);
            this.lookAt = new Point(lookAt);
        }

        @Override
        public String toString() {
            return "position : " + position + ", lookAt : " + lookAt;
        }

        public String toJson() {
            return "{position : " + position.toJson() + ", lookAt : " + lookAt.toJson() + "}";
        }
    }

    /**
     * Stereo3D configuration for the Stage.
     * Read-only methods. The Stereo-3D configuration must be done via the Stage methods
     */
    public static class Stereo3D implements JSON.ToJson {
        /**
         * @hide Access via configuration methods
         */
        public boolean enable;
        /**
         * @hide Access via configuration methods
         */
        public float focalDistance;
        /**
         * @hide Access via configuration methods
         */
        public float intensity;

        /**
         * @hide Access via configuration methods
         */
        public Stereo3D(boolean enable, float focalDistance, float intensity) {
            this.enable = enable;
            this.focalDistance = focalDistance;
            this.intensity = intensity;
        }

        @Override
        public String toString() {
            return "enable : " + enable + ", focalDistance : " + focalDistance + ", intensity : " + intensity;
        }

        public String toJson() {
            return "{enable : " + enable + ", focalDistance : " + focalDistance + ", intensity : " + intensity + "}";
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    // Property handling


    // Default projection is camera fixed looking at a planar 'stage'.
    // The value of -1111 for the default camera z position is a remnant from
    // AfterEffects default position.  Demo code based on this still relies on
    // this default value.  New code must NOT rely on this as it may be changed
    // when all legacy code is updated or disused.
    /**
     * @hide
     */
    static final Property<ProjectionConfig> PROP_PROJECTION = new Property<ProjectionConfig>(
            "projection", new ProjectionConfig(UI_PERSPECTIVE, 2f, 3000f, -1111f));
    /**
     * @hide
     */
    static final Property<String> PROP_DEBUG_CAMERA = new Property<String>(
            "active_camera", null, PROP_PROJECTION);
    /**
     * @hide
     */
    static final Property<Camera> PROP_CAMERA = new Property<Camera>(
            "camera", new Camera(new Point(0, 0, 0), new Point(0, 0, -1)), PROP_PROJECTION);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_FOV = new Property<Float>(
            "camera_fov", null, PROP_PROJECTION);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_WIDTH = new Property<Float>(
            "camera_width", null, PROP_PROJECTION);
    /**
     * @hide
     */
    static final Property<Color> PROP_BACKGROUND_COLOR = new Property<Color>(
            "background_color", Color.BLACK);
    /**
     * @hide
     */
    static final Property<Float> PROP_FOG_DENSITY = new Property<Float>(
            "fog_density", 0.0f);
    /**
     * @hide
     */
    static final Property<Color> PROP_FOG_COLOR = new Property<Color>(
            "fog_color", Color.BLACK);
    /**
     * @hide
     */
    static final Property<Integer> PROP_MAX_FPS = new Property<Integer>(
            "max_fps", 0);
    /**
     * @hide
     */
    static final Property<Stereo3D> PROP_STEREO3D = new Property<Stereo3D>(
            "stereo3d", null);

    /**
     * @hide
     */
    @Override
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property.sameInstance(PROP_DEBUG_CAMERA)) {
            if (value != null) {
                String name = (String) value;
                mPresentationEngine.setDebugCamera(name);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA)) {
            if (value != null) {
                Camera camera = (Camera) value;
                mPresentationEngine.setCamera(camera.position, camera.lookAt);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_FOV)) {
            if (value != null) {
                Float fov = (Float) value;
                mPresentationEngine.setCameraFov(fov);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_WIDTH)) {
            if (value != null) {
                Float width = (Float) value;
                mPresentationEngine.setCameraWidth(width);
            }
            return true;
        } else if (property.sameInstance(PROP_PROJECTION)) {
            if (value != null) {
                ProjectionConfig p = (ProjectionConfig) value;
                mPresentationEngine.setClipDistances(p.zNear, p.zFar);
                if (p.mode == UI_PERSPECTIVE) {
                    mPresentationEngine.setCameraZ(p.zStage);
                }
                mPresentationEngine.setProjectionMode(p.mode);
            }
            return true;
        } else if (property.sameInstance(PROP_BACKGROUND_COLOR)) {
            // Not necessary to apply cause PresentationEngine should read the background color automatically.
            return true;
        } else if (property.sameInstance(PROP_FOG_DENSITY)) {
            mPresentationEngine.setFogDensity((Float) value);
            return true;
        } else if (property.sameInstance(PROP_FOG_COLOR)) {
            mPresentationEngine.setFogColor((Color) value);
            return true;
        } else if (property.sameInstance(PROP_MAX_FPS)) {
            Integer fps = (Integer) value;
            mPresentationEngine.setMaxFPS(fps);
            return true;
        } else if (property.sameInstance(PROP_STEREO3D)) {
            if (value != null) {
                Stereo3D stereo3D = (Stereo3D) value;
                mPresentationEngine.enableStereoscopic3D(stereo3D.enable, stereo3D.focalDistance, stereo3D.intensity);
            }
            return true;
        }

        return false;
    }


    /**
     * Configures the type of projection in use.
     * <p>
     * Orthogonal and Perspective are as classical graphics rendering.
     * http://en.wikipedia.org/wiki/3D_projection
     * <p>
     * UI-perspective is a UI-application projection where the camera position
     * and orientation is fixed, mid screen and pointing down the Z axis at the
     * XY plane where the UI objects are located. Equates to watching action
     * on a theatre stage.
     * <p>
     * Use of setCamera() is only legal if the mode is PERSPECTIVE.  For the
     * other modes the position of the camera is either irrelevant (ORTHO) or
     * fixed (UI).
     *
     * @param projectionMode ORTHOGRAPHIC, PERSPECTIVE or UI_PERSPECTIVE
     * @param zNear distance from camera to near clipping plane
     * @param zFar distance from camera to far clipping plane
     * @param zStage Z position of camera for STAGE mode
     */
    public void setProjection(int projectionMode, float zNear, float zFar, float zStage) {
        if (projectionMode > PM_MAX_LEGAL || projectionMode < 0) {
            throw new Ngin3dException("Illegal projection mode " + projectionMode);
        } else {
            setValue(PROP_PROJECTION,
                new ProjectionConfig(projectionMode, zNear, zFar, zStage));
        }
    }

    /**
     * Query the current projection mode.
     *
     * @return Projection mode
     */
    public Object getProjection() {
        return getValue(PROP_PROJECTION);
    }

    /**
     * Returns the camera projection configuration.
     *
     * @return Projection configuration
     */
    private ProjectionConfig getProjectionConfig() {
        return (ProjectionConfig) getProjection();
    }

    /**
     * Set projection mode.
     *
     * @param mode Camera projection mode ORTHOGRAPHIC or PERSPECTIVE
     */
    public void setProjectionMode(int mode) {
        ProjectionConfig config = getProjectionConfig();
        setProjection(mode, config.zNear, config.zFar, config.zStage);
    }

    /**
     * Set camera near clipping plane.
     * This defines the position of a plane in front of the camera regardless where
     * the camera is positioned.
     * (This is often called Z-near it is nothing to do with the world Z axis).
     * All graphical objects nearer to the camera than this are 'clipped' and may be
     * entirely invisible.
     *
     * @param near Camera near clipping plane
     */
    public void setCameraNear(float near) {
        ProjectionConfig config = getProjectionConfig();
        setProjection(config.mode, near, config.zFar, config.zStage);
    }

    /**
     * Set camera far clipping plane.
     * This defines the position of a plane in front of the camera regardless where
     * the camera is positioned.
     * (This is often called Z-far it is nothing to do with the world Z axis).
     * All graphical objects further from the camera than this are 'clipped' and may be
     * entirely invisible.
     *
     *
     * @param far Camera far clipping plane
     */
    public void setCameraFar(float far) {
        ProjectionConfig config = getProjectionConfig();
        setProjection(config.mode, config.zNear, far, config.zStage);
    }

    /**
     * Select a named camera from those defined by an artist in a Glo file.
     * Passing an empty string activates the default camera.
     *
     * @param name Camera name (an empty string activates the default camera)
     * @deprecated This method will be renamed in future, it is not a debug feature.
     */
    public void setDebugCamera(String name) {
        if (mPresentationEngine != null) {
            mPresentationEngine.setDebugCamera(name);
        }
    }

    /**
     * Returns a list of names of cameras available.
     *
     * @deprecated This method will be renamed in future, it is not a debug feature.
     */
    public String[] getDebugCameraNames() {
        if (mPresentationEngine == null) {
            return null;
        } else {
            return mPresentationEngine.getDebugCameraNames();
        }
    }

    /**
     * Sets the camera position and the point at which it is looking in unison.
     *
     * @param position Camera position
     * @param lookAt Camera focus point
     */
    public void setCamera(Point position, Point lookAt) {
        setValueInTransaction(PROP_CAMERA, new Camera(position, lookAt));
    }

    /**
     * Sets the camera position.
     * The look-at point is unaffected.
     *
     * @param position Camera position
     */
    public void setCameraPosition(Point position) {
        setCamera(position, getCamera().lookAt);
    }

    /**
     * Sets the camera position and orientation.
     * If you subsequently change the camera position it will re-orient itself
     * to look at the given point. The subsequent camera rotation will have its
     * "up" vector as close to (0,1,0) as possible.
     *
     * @param lookAt Camera focus point
     */
    public void setCameraLookAt(Point lookAt) {
        setCamera(getCamera().position, lookAt);
    }

    /**
     * Set camera field of view (FOV) in degrees.
     * The field of view for the smaller screen dimension is specified (e.g. if
     * the screen is taller than it is wide, the horizontal FOV is specified).
     * <p>
     * This parameter is only used by the PERSPECTIVE projection. In the 'UI'
     * projections the FOV is derived from the camera Z position and the screen
     * width (pixels) which are considered to be in the same coordinate space.
     *
     * @param fov Camera field of view in degrees
     */
    public void setCameraFov(float fov) {
        setValueInTransaction(PROP_CAMERA_FOV, fov);
    }

    /**
     * Set camera width.
     * Sets the width of the viewing frustum in world-units when using an
     * ORTHOGRAPHIC projection.
     *
     * When using an PERSPECTIVE projection, this parameter has no visible
     * effect.
     *
     * @param width Width of viewing frustum
     */
    public void setCameraWidth(float width) {
        setValueInTransaction(PROP_CAMERA_WIDTH, width);
    }

    /**
     * Gets the virtual camera property of this object.
     *
     * @return Camera property
     */
    public Camera getCamera() {
        return getValue(PROP_CAMERA);
    }

    /**
     * Sets the background color of this stage object.
     *
     * @param bkgColor background color
     */
    public void setBackgroundColor(Color bkgColor) {
        setValue(PROP_BACKGROUND_COLOR, bkgColor);
    }

    /**
     * Gets the background color of this stage object.
     *
     * @return background color property
     */
    public Color getBackgroundColor() {
        return getValue(PROP_BACKGROUND_COLOR);
    }

    /**
     * Sets the global fog density.
     * When fog density is greater than zero (zero is the default), the scene
     * will gradually fade towards the fog color as it gets further from the
     * camera.  The higher the fog density, the faster the fog color will fade
     * in with distance.
     *
     * @param density Fog density
     */
    public void setFogDensity(float density) {
        setValue(PROP_FOG_DENSITY, density);
    }

    /**
     * Sets the global fog color.
     *
     * @param color Fog color
     */
    public void setFogColor(Color color) {
        setValue(PROP_FOG_COLOR, color);
    }

    /**
     * Sets a Stereo3D configuration.
     *
     * @param enable enable stereoscopic 3d effect.
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     */
    public void setStereo3D(boolean enable, float focalDistance) {
        setStereo3D(enable, focalDistance, 1);
    }

    /**
     * Sets a Stereo3D configuration, with optional Intensity.
     *
     * @param enable enable stereoscopic 3d effect.
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     * @param intensity Adjust the level of stereo separation. Normally 1.0,
     *                  1.1 increases the effect by 10%, for example.
     */
    public void setStereo3D(boolean enable, float focalDistance, float intensity) {
        setValue(PROP_STEREO3D, new Stereo3D(enable, focalDistance, intensity));
    }

    /**
     * Gets the Stereo3D configuration.
     *
     * @return the Stereo3D object
     */
    public Stereo3D getStereo3D() {
        return getValue(PROP_STEREO3D);
    }

    /**
     * Add a TextureAtlas into this stage using android resource information and JSON file.
     *
     * @param res      android resource
     * @param imageId  android resource id
     * @param scriptId JSON file id
     */
    public void addTextureAtlas(Resources res, int imageId, int scriptId) {
        TextureAtlas.getDefault().add(res, imageId, scriptId);
    }

    /**
     * Add a TextureAtlas into this stage using android asset information and JSON file.
     *
     * @param res      android resource
     * @param asset  android asset name
     * @param scriptId JSON file id
     */
    public void addTextureAtlas(Resources res, String asset, int scriptId) {
        TextureAtlas.getDefault().add(res, asset, scriptId);
    }

    /**
     * Set the maximum Frames-per-second rate.
     * @param fps Max FPS setting
     */
    public void setMaxFPS(int fps) {
        setValue(PROP_MAX_FPS, fps);
    }

    /**
     * Query the current 'Max FPS' setting.
     * @return Current Max FPS setting.
     */
    public int getMaxFPS() {
        return getValue(PROP_MAX_FPS);
    }

    /** dump function
     * @hide
     */
    public String dump() {
        String dump = super.dump();
        dump = JSON.wrap(dump);
        Log.d(TAG, dump);
        return dump;
    }

}
