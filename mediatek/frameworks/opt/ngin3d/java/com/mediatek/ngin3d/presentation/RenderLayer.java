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

package com.mediatek.ngin3d.presentation;

import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;


/**
 * Class used to render part of a scene using its own camera.
 */
public interface RenderLayer extends Presentation {

    /*
     * Motion blur quality levels
     */
    /** Low quality motion blur. */
    public static final int MOTION_BLUR_QUALITY_LOW = 0;
    /** High quality motion blur. */
    public static final int MOTION_BLUR_QUALITY_HIGH = 1;


    /**
     * Sets position of camera.
     *
     * @param pos    Camera position
     */
    void setCameraPosition(Point pos);

    /**
     * Sets rotation of camera.
     *
     * @param rot    Camera rotation
     */
    void setCameraRotation(Rotation rot);

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
    void setCameraFov(float fov);

    /**
     * Sets the near and far clipping distances.
     * Note these are distances from the camera, in the forward direction of the
     * camera axis; they are NOT planes positioned along the global Z axis
     * despite often called Znear and Zfar.
     *
     * @param near Objects nearer than this are clipped
     * @param far Objects further away than this are clipped
     */
    void setClipDistances(float near, float far);

    /**
     * Set camera near clipping plane.
     *
     * @param near Camera near clipping plane
     */
    void setCameraNear(float near);

    /**
     * Set camera far clipping plane.
     *
     * @param far Camera far clipping plane
     */
    void setCameraFar(float far);

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
    void setCameraWidth(float width);

    /**
     * Set projection mode
     *
     * @param mode Camera projection mode ORTHOGRAPHIC or PERSPECTIVE
     */
    void setProjectionMode(int mode);

    /**
     * Sets the currently active camera.
     * Passing an empty string activates the default camera.
     *
     * @param name Camera node name
     */
    void useNamedCamera(String name);

    /**
     * Sets z-buffer clearing behaviour.
     * Call this method with true will cause the z-buffer to be cleared before
     * the layer is rendered.
     *
     * @param clear True if the z buffer should be cleared
     */
    void setDepthClear(boolean clear);

    /**
     * Returns a list of names of cameras in the scene.
     *
     */
    String[] getGloCameraNames();

    /**
     * Sets destination for rendering operations.
     *
     * @param imageDisplay Image to render into
     */
    void setRenderTarget(ImageDisplay imageDisplay);

    /**
     * Enable depth of field effect.
     *
     * @param enable True to enable, false to disable
     */
    void enableDepthOfField(boolean enable);

    /**
     * Sets focus distance for depth of field effect.
     *
     * @param focusDistance Distance at which objects are sharpest
     */
    void setFocusDistance(float focusDistance);

    /**
     * Sets focus range for depth of field effect.
     *
     * @param focusRange Range within which objects are quite sharp
     */
    void setFocusRange(float focusRange);

    /**
     * Sets blur factor for depth of field effect.
     *
     * This parameter changes the factor by which the image is downsampled to
     * create the blurred image. Note that it is expensive to change this
     * factor, so it should generally be set once (before you enable depth of
     * field), and not changed every frame.
     *
     * This value should usually be set to 2 or 4. Set the value to 2 for a
     * subtle effect which minimizes artifacts. Set the value to 4 for a
     * stronger effect. You can experiment with larger values for more extrem
     * effects.
     *
     * The value is clamped to the range [2,16]. The default value is 4.
     *
     * @param blurFactor Amount to blur out-of-focus pixels
     */
    void setFocusBlurFactor(float blurFactor);

    /**
     * Enable motion blur effect.
     *
     * @param enable True to enable, false to disable
     */
    void enableMotionBlur(boolean enable);

    /**
     * Sets blur factor for motion blur effect.
     *
     * This parameter changes the amount by which moving objects are blurred
     * when the motion blur effect is enabled. Values should normally be in the
     * range [0.5,1.0], but you can experiment with values outside of this range
     * for more subtle effects [0.0,0.5], or more extreme effects (>1.0). You
     * can even try -ve numbers if you're feeling adventurous.
     *
     * @param blurFactor Amount to blur moving objects
     */
    void setMotionBlurFactor(float blurFactor);

    /**
     * Sets quality for motion blur effect.
     *
     * This parameter changes the quality of the motion blur effect. Low quality
     * uses fewer samples to produce the effect, which may result in higher
     * performance.
     *
     * @param quality One of MOTION_BLUR_QUALITY_LOW or MOTION_BLUR_QUALITY_HIGH
     */
    void setMotionBlurQuality(int quality);

    /**
     * Sets the portion of the screen to which to render.
     *
     * @param left left edge of the screen area
     * @param bottom bottom edge of the screen area
     * @param width width of the screen area
     * @param height height of the screen area
     */
    void setViewport(float left, float bottom, float width, float height);
}
