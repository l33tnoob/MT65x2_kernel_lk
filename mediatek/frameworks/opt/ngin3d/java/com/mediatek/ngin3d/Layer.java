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
 * MediaTek Inc. (C) 2013. All rights reserved.
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

import com.mediatek.ngin3d.presentation.Presentation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.presentation.RenderLayer;

/**
 * Render-Layer, used to create independent stacked layers in scenes.
 * <p>
 * Layer is a special Container that displays its Actors on the screen.
 * Each Layer object has its own camera settings. Layer objects are added to
 * the Stage object. They are drawn in the order that they are added to the
 * Stage.
 */
public class Layer extends Container {
    protected static final String TAG = "Layer";
    /*
     * Projection modes
     */
    /** Orthographic projection mode. */
    public static final int ORTHOGRAPHIC = 0;
    /** Perspective projection mode. */
    public static final int PERSPECTIVE = 1;

    /*
     * Motion blur quality levels
     */
    /** Low quality motion blur. */
    public static final int MOTION_BLUR_QUALITY_LOW = 0;
    /** High quality motion blur. */
    public static final int MOTION_BLUR_QUALITY_HIGH = 1;

    private RenderLayer mRenderLayer;
    private Point mPosition = new Point(0, 0, 0);

    /**
     * @hide
     */
    @Override
    protected Presentation createPresentation(PresentationEngine engine) {
        mRenderLayer = engine.createRenderLayer();
        return mRenderLayer;
    }

    /**
     * @hide
     */
    public RenderLayer getRenderLayer() {
        return mRenderLayer;
    }

    /**
     * @hide
     */
    static final Property<Point> PROP_CAMERA_POS = new Property<Point>(
            "camera", null);
    /**
     * @hide
     */
    static final Property<Rotation> PROP_CAMERA_ROT = new Property<Rotation>(
            "camera_rotation", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_FOV = new Property<Float>(
            "camera_fov", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_NEAR = new Property<Float>(
            "camera_near", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_FAR = new Property<Float>(
            "camera_far", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_CAMERA_WIDTH = new Property<Float>(
            "camera_width", null);
    /**
     * @hide
     */
    static final Property<Integer> PROP_PROJECTION_MODE = new Property<Integer>(
            "proj_mode", null);
    /**
     * @hide
     */
    static final Property<Boolean> PROP_CLEAR_DEPTH = new Property<Boolean>(
            "clear_depth", null);
    /**
     * @hide
     */
    static final Property<Plane> PROP_TARGET_IMAGE = new Property<Plane>(
            "target_image", null);
    /**
     * @hide
     */
    static final Property<String> PROP_NAMED_CAMERA = new Property<String>(
            "named_camera", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_FOCUS_DISTANCE = new Property<Float>(
            "focus_distance", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_FOCUS_RANGE = new Property<Float>(
            "focus_range", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_FOCUS_BLUR_FACTOR = new Property<Float>(
            "focus_blur_factor", null);
    /**
     * @hide
     */
    static final Property<Boolean> PROP_DEPTH_OF_FIELD_ENABLED
        = new Property<Boolean>("dof_enabled", null);
    /**
     * @hide
     */
    static final Property<Boolean> PROP_MOTION_BLUR_ENABLED
        = new Property<Boolean>("motion_blur_enabled", null);
    /**
     * @hide
     */
    static final Property<Float> PROP_MOTION_BLUR_FACTOR = new Property<Float>(
            "motion_blur_factor", null);
    /**
     * @hide
     */
    static final Property<Integer> PROP_MOTION_BLUR_QUALITY = new Property<Integer>(
            "motion_blur_quality", null);
    /**
     * @hide
     */
    static final Property<Dimension> PROP_VIEWPORT_SIZE = new Property<Dimension>(
            "viewport_size", null);


    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (property.sameInstance(PROP_CAMERA_POS)) {
            if (value != null) {
                Point position = (Point) value;
                mRenderLayer.setCameraPosition(position);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_ROT)) {
            if (value != null) {
                Rotation rotation = (Rotation) value;
                mRenderLayer.setCameraRotation(rotation);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_FOV)) {
            if (value != null) {
                Float fov = (Float) value;
                mRenderLayer.setCameraFov(fov);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_NEAR)) {
            if (value != null) {
                Float near = (Float) value;
                mRenderLayer.setCameraNear(near);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_FAR)) {
            if (value != null) {
                Float far = (Float) value;
                mRenderLayer.setCameraFar(far);
            }
            return true;
        } else if (property.sameInstance(PROP_CAMERA_WIDTH)) {
            if (value != null) {
                Float far = (Float) value;
                mRenderLayer.setCameraWidth(far);
            }
            return true;
        } else if (property.sameInstance(PROP_PROJECTION_MODE)) {
            if (value != null) {
                Integer mode = (Integer) value;
                mRenderLayer.setProjectionMode(mode);
            }
            return true;
        } else if (property.sameInstance(PROP_NAMED_CAMERA)) {
            if (value != null) {
                String name = (String) value;
                mRenderLayer.useNamedCamera(name);
            }
            return true;
        } else if (property.sameInstance(PROP_CLEAR_DEPTH)) {
            if (value != null) {
                mRenderLayer.setDepthClear((Boolean)value);
            }
            return true;
        } else if (property.sameInstance(PROP_TARGET_IMAGE)) {
            if (value != null) {
                ((Plane)value).attachToRenderLayer(mRenderLayer);
            }
            return true;
        } else if (property.sameInstance(PROP_FOCUS_DISTANCE)) {
            if (value != null) {
                Float focusDistance = (Float) value;
                mRenderLayer.setFocusDistance(focusDistance);
            }
            return true;
        } else if (property.sameInstance(PROP_FOCUS_RANGE)) {
            if (value != null) {
                Float focusRange = (Float) value;
                mRenderLayer.setFocusRange(focusRange);
            }
            return true;
        } else if (property.sameInstance(PROP_FOCUS_BLUR_FACTOR)) {
            if (value != null) {
                Float blurFactor = (Float) value;
                mRenderLayer.setFocusBlurFactor(blurFactor);
            }
            return true;
        } else if (property.sameInstance(PROP_DEPTH_OF_FIELD_ENABLED)) {
            if (value != null) {
                mRenderLayer.enableDepthOfField((Boolean)value);
            }
            return true;
        } else if (property.sameInstance(PROP_MOTION_BLUR_ENABLED)) {
            if (value != null) {
                mRenderLayer.enableMotionBlur((Boolean)value);
            }
            return true;
        } else if (property.sameInstance(PROP_MOTION_BLUR_FACTOR)) {
            if (value != null) {
                mRenderLayer.setMotionBlurFactor((Float)value);
            }
            return true;
        } else if (property.sameInstance(PROP_MOTION_BLUR_QUALITY)) {
            if (value != null) {
                mRenderLayer.setMotionBlurQuality((Integer)value);
            }
            return true;
        } else if (property.sameInstance(PROP_VIEWPORT_SIZE)) {
            if (value != null) {
                Dimension size = (Dimension) value;
                mRenderLayer.setViewport(0f, 0f, size.width, size.height);
            }
            return true;
        }

        return false;
    }

    @Override
    public void realize(PresentationEngine presentationEngine) {
        boolean isRealized = isRealized();
        super.realize(presentationEngine);
        // Avoid add this render layer more than twice
        if (!isRealized) {
            presentationEngine.addRenderLayer(getPresentation());
        }
    }

    /**
     * Returns the layer.
     */
    @Override
    public Layer getLayer() {
        return this;
    }

    /**
     * Sets the camera position and the point at which it is looking in unison.
     *
     * @param position Camera position
     * @param lookAt Camera focus point
     */
    public void setCamera(Point position, Point lookAt) {
        setCameraPosition(position);
        setCameraLookAt(lookAt);
    }

    /**
     * Configures the camera projection and clipping planes all at once.
     *
     * @param projectionMode ORTHOGRAPHIC or PERSPECTIVE
     * @param zNear distance from camera to near clipping plane
     * @param zFar distance from camera to far clipping plane
     */
    public void setProjection(int projectionMode, float zNear, float zFar) {
        setProjectionMode(projectionMode);
        setCameraNear(zNear);
        setCameraFar(zFar);
    }

    /**
     * Sets the camera position.
     * The camera will be set to the given position. The orientation is
     * unaffected.
     *
     * @param position Camera position
     */
    public void setCameraPosition(Point position) {
        mPosition = position;
        setValueInTransaction(PROP_CAMERA_POS, position);
    }

    /**
     * Sets the camera orientation.
     * The camera will oriented to the given rotation.
     *
     * @param rotation Camera orientation
     */
    public void setCameraRotation(Rotation rotation) {
        setValueInTransaction(PROP_CAMERA_ROT, rotation);
    }

    /**
     * Sets the camera orientation to look at a given point.
     * If you subsequently change the camera position it will not re-orient
     * itself to look at the given point.  The subsequent camera rotation will
     * have its "up" vector as close to (0,1,0) as possible.
     *
     * @param lookAt    Camera focus point
     */
    public void setCameraLookAt(Point lookAt) {
        setCameraLookAt(lookAt, new Point(0, 1, 0));
    }

    /**
     * Sets the camera orientation to look at a given point.
     * If you subsequently change the camera position it will not re-orient
     * itself to look at the given point.
     *
     * @param lookAt    Camera focus point
     * @param up        Vector to keep up (usually (0,1,0) or (0,-1,0))
     */
    public void setCameraLookAt(Point lookAt, Point up) {
        Vec3 to = new Vec3(lookAt.x - mPosition.x,
                lookAt.y - mPosition.y, lookAt.z - mPosition.z);
        Rotation rotation = Rotation.pointAt(new Vec3(0, 0, -1),
                to, new Vec3(0, 1, 0), new Vec3(up.x, up.y, up.z));
        setValueInTransaction(PROP_CAMERA_ROT, rotation);
    }

    /**
     * Set camera field of view (FOV) in degrees.
     * The field of view for the smaller screen dimension is specified (e.g. if
     * the screen is taller than it is wide, the horizontal FOV is specified).
     *
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
     * Set camera near clipping plane.
     *
     * @param near Camera near clipping plane
     */
    public void setCameraNear(float near) {
        setValueInTransaction(PROP_CAMERA_NEAR, near);
    }

    /**
     * Set camera far clipping plane.
     *
     * @param far Camera far clipping plane
     */
    public void setCameraFar(float far) {
        setValueInTransaction(PROP_CAMERA_FAR, far);
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
     * Set conventional projection mode, Perspective or Orthographic.
     * @see setUiPerspective
     *
     * @param mode Camera projection mode ORTHOGRAPHIC or PERSPECTIVE
     */
    public void setProjectionMode(int mode) {
        setValueInTransaction(PROP_PROJECTION_MODE, mode);
    }

    /**
     * Sets depth buffer clearing behaviour.
     * By default the depth buffer is cleared before each layer is rendered.
     * Call this method with false to prevent the z-buffer being cleared before
     * the layer is rendered.
     *
     * @param clear True if the depth buffer should be cleared
     */
    public void setDepthClear(boolean clear) {
        setValueInTransaction(PROP_CLEAR_DEPTH, clear);
    }

    /**
     * Set an image (a off-screen bitmap) as a render target.
     * After calling this method, the layer will be rendered to the supplied
     * image instead of the device.
     *
     * @param plane The image to use as a target
     */
    public void setTargetImage(Plane plane) {
        setValueInTransaction(PROP_TARGET_IMAGE, plane);
    }

    /**
     * Use a camera from the scene.
     * <p>
     * Use a camera created in a DCC tool and exported to a glo file. You should
     * pass in the name you gave the camera in 3ds Max or Blender. Pass an empty
     * string to revert to using the Layer's camera parameters.
     *
     * @param name Camera in scene to use
     */
    public void useNamedCamera(String name) {
        setValueInTransaction(PROP_NAMED_CAMERA, name);
    }

    /**
     * Set focus distance for depth of field (DOF) effect.
     *
     * @param distance Distance at which objects appear completely sharp
     */
    public void setFocusDistance(float distance) {
        setValueInTransaction(PROP_FOCUS_DISTANCE, distance);
    }

    /**
     * Set focus range for depth of field (DOF) effect.
     *
     * @param range Range over which objects appear relatively sharp
     */
    public void setFocusRange(float range) {
        setValueInTransaction(PROP_FOCUS_RANGE, range);
    }

    /**
     * Sets blur factor for depth of field (DOF) effect.
     * <p>
     * This parameter changes the factor by which the image is downsampled to
     * create the blurred image. Note that it is expensive to change this
     * factor, so it should generally be set once (before you enable depth of
     * field), and not changed every frame.
     * <p>
     * This value should usually be set to 2 or 4. Set the value to 2 for a
     * subtle effect which minimizes artifacts. Set the value to 4 for a
     * stronger effect. You can experiment with larger values for more extrem
     * effects.
     * <p>
     * The value is clamped to the range [2,16]. The default value is 4.
     *
     * @param blurFactor Amount to blur out-of-focus pixels
     */
    public void setFocusBlurFactor(float blurFactor) {
        setValueInTransaction(PROP_FOCUS_BLUR_FACTOR, blurFactor);
    }

    /**
     * Enable depth of field (DOF) effect.
     * @param enable True if DOF is to be enabled
     */
    public void enableDepthOfField(boolean enable) {
        setValueInTransaction(PROP_DEPTH_OF_FIELD_ENABLED, enable);
    }

    /**
     * Enable motion blur effect.
     * @param enable True if motion blur is to be enabled
     */
    public void enableMotionBlur(boolean enable) {
        setValueInTransaction(PROP_MOTION_BLUR_ENABLED, enable);
    }

    /**
     * Sets blur factor for motion blur effect.
     * <p>
     * This parameter changes the amount by which moving objects are blurred
     * when the motion blur effect is enabled. Values should normally be in the
     * range [0.5,1.0], but you can experiment with values outside of this range
     * for more subtle effects [0.0,0.5], or more extreme effects (>1.0). You
     * can even try -ve numbers if you're feeling adventurous.
     *
     * @param blurFactor Amount to blur moving objects
     */
    public void setMotionBlurFactor(float blurFactor) {
        setValueInTransaction(PROP_MOTION_BLUR_FACTOR, blurFactor);
    }

    /**
     * Sets quality for motion blur effect.
     * <p>
     * This parameter changes the quality of the motion blur effect. Low quality
     * uses fewer samples to produce the effect, which may result in higher
     * performance.
     *
     * @param quality One of MOTION_BLUR_QUALITY_LOW or MOTION_BLUR_QUALITY_HIGH
     */
    public void setMotionBlurQuality(int quality) {
        setValueInTransaction(PROP_MOTION_BLUR_QUALITY, quality);
    }

    /**
     * Use UI_PERSPECTIVE projection for this layer.
     * <p>
     * The camera will be positioned at the given (zPos) distance from the
     * X/Y plane and the field of view set such that the camera covers an
     * area of the plane so that x=0 corresponds to the left of the screen,
     * x=width corresponds to the right of the screen, y=0 corresponds to
     * the top of the screen and y=height corresponds to the bottom of the
     * screen.
     *
     * @param width     Width of screen
     * @param height    Height of screen
     * @param zPos      Distance of camera from X/Y plane
     */
    public void setUiPerspective(float width, float height, float zPos) {
        float smallerDim = Math.min(width, height);
        float zDistance = Math.abs(zPos);

        float fov = (float) Math.toDegrees(
            Math.atan((smallerDim / 2) / zDistance) * 2);
        setCameraFov(fov);

        float centX = width / 2;
        float centY = height / 2;

        Point cameraPosition = new Point(centX, centY, zPos);

        setCameraPosition(cameraPosition);

        float zDir = (zPos < 0) ? 1 : -1;
        Rotation rotation = Rotation.pointAt(new Vec3(0, 0, -1),
                new Vec3(0, 0, zDir),
                new Vec3(0, 1, 0),
                new Vec3(0, -1, 0));
        setCameraRotation(rotation);

        // Set near and far planes such that the z=0 plane is half-way between
        // them and the ratio near/far = 1/100
        setCameraNear(2 * zDistance / 101);
        setCameraFar(200 * zDistance / 101);
    }

    /**
     * Sets the size of the viewport into which to render.
     *
     * @param width width of the screen area
     * @param height height of the screen area
     */
    public void setViewportSize(int width, int height) {
        setValueInTransaction(PROP_VIEWPORT_SIZE,
                new Dimension((float)width, (float)height));
    }
}
