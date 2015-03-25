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

package com.mediatek.ngin3d.presentation;

import android.content.res.Resources;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;

/**
 * Interface to presentation engine.
 */
public interface PresentationEngine {
    /**
     * Initialize presentation engine with viewport dimension.
     *
     * @param width  in pixels
     * @param height in pixels
     */
    void initialize(int width, int height);

    /**
     * Initialize presentation engine with viewport dimension and Resources.
     *
     * @param width  in pixels
     * @param height in pixels
     * @param resources Resources
     */
    void initialize(int width, int height, Resources resources);

    /**
     * Initialize presentation engine with viewport dimension and Resources.
     *
     * @param width  in pixels
     * @param height in pixels
     * @param resources Resources
     * @param cacheDir The binary shader cache directory
     */
    void initialize(int width, int height, Resources resources, String cacheDir);

    /**
     * Initialize presentation engine with viewport dimension and Resources.
     *
     * @param width  in pixels
     * @param height in pixels
     * @param resources Resources
     * @param cacheDir The binary shader cache directory
     * @param libDir customer lib folder, for widget used only
     */
    void initialize(int width, int height, Resources resources, String cacheDir, String libDir);

    /**
     * Can be used to know whether the required resource, such as OpenGL context, is ready or not.
     *
     * @return true if the context is ready. false otherwise.
     */
    boolean isReady();

    /**
     * Sets the currently active debug camera.
     * Passing an empty string activates the default camera.
     *
     * This function is deprecated and marked for removal in the near future.
     *
     * @deprecated
     * @param name Camera name (an empty string activates the default camera)
     */
    void setDebugCamera(String name);

    /**
     * Returns a list of names of cameras in the scene.
     *
     * This function is deprecated and marked for removal in the near future.
     *
     * @deprecated
     * @return List of cameras
     */
    String[] getDebugCameraNames();

    /**
     * Sets a virtual camera of this object for seeing stage.
     *
     * @param pos    camera position
     * @param lookAt camera focus point position
     */
    void setCamera(Point pos, Point lookAt);

    /**
     * Set camera Z position. This is normally only used for the
     * UI_PERSPECTIVE projection where X and Y are fixed relative
     * to the screen size.
     * However, for completeness it will also affect the Z position
     * when in PERSPECTIVE mode.
     *
     * @param zCamera Z component of camera position
     */
    void setCameraZ(float zCamera);

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
     * Set camera orthographic frustum width.
     * The width of the frustum for the horizonal screen dimension is
     * specified.  If a width of zero or less is given, the frustum width will
     * be equal to the screen width in pixels.
     *
     * This parameter is only used by the ORTHOGRAPHIC projection.
     *
     * @param width Camera frustum width
     */
    void setCameraWidth(float width);

    /**
     * Sets the near and far clipping distances. Note these are distances
     * from the camera, in the forward direction of the camera axis; they
     * are NOT planes positioned along the global Z axis despite often called
     * Znear and Zfar.
     *
     * @param near objects nearer than this are clipped
     * @param far objects further away than this are clipped
     */
    void setClipDistances(float near, float far);

    /**
     * Record the maximum of FPS number.
     *
     * @param fps fps number
     */
    void setMaxFPS(int fps);

    /**
     * Get time the last frame cost.
     *
     * @return the time of frame interval
     */
    int getFrameInterval();

    /**
     * Enable/disable stereoscopic 3d display mode
     * @param enable enable stereoscopic 3d effect.
     * @param focalDistance the distance between the camera and the object in
     *        the world space you would like to focus on.
     * @param intensity Adjust the level of stereo separation. Normally 1.0,
     *                  1.1 increases the effect by 10%, for example.
     */
    void enableStereoscopic3D(boolean enable, float focalDistance, float intensity);

    /**
     * Get the screen shot of current render frame.
     * @return An Object representing the render frame.
     */
    Object getScreenShot();

    /**
     * Register with real renderer's requestRender
     */
    public interface RenderCallback {
        void requestRender();
    }

    /**
     * Specify the callback for rendering.
     *
     * @param renderCallback
     */
    void setRenderCallback(RenderCallback renderCallback);

    /**
     * Render the presentation.
     *
     * @return true if still dirty
     */
    boolean render();

    /**
     * Resize the presentation area.
     *
     * @param width  in pixels
     * @param height in pixels
     */
    void resize(int width, int height);

    /**
     * Sets the global fog density.
     *
     * @param density Fog density
     */
    void setFogDensity(float density);

    /**
     * Sets the global fog color.
     *
     * @param color Fog color
     */
    void setFogColor(Color color);

    /**
     * Deinitialize the context.
     */
    void uninitialize();

    /**
     * Dump debug information.
     */
    void dump();

    /**
     * Gets the width of this presentation engine object.
     *
     * @return width value
     */
    int getWidth();

    /**
     * Gets the height of this presentation engine object.
     *
     * @return height value
     */
    int getHeight();

    /**
     * @return total memory usage of CImage in bytes.
     */
    int getTotalCImageBytes();

    /**
     * @return total memory usage of OpenGL texture.
     */
    int getTotalTextureBytes();

    /**
     * Create a special scene node with empty presentation engine setting.
     *
     * @return a new scene node presentation.
     */
    Presentation createEmpty();

    /**
     * Create a container.
     *
     * @return a new scene node presentation.
     */
    Presentation createContainer();

    /**
     * Create a image display object.
     *
     * @return a new rectangular scene node presentation.
     */
    ImageDisplay createImageDisplay(boolean isYUp);

    /**
     * Create a video display object.
     *
     * @return a new rectangular scene node presentation.
     */
    VideoDisplay createVideoDisplay(boolean isYUp);

    /**
     * Create basic 3D model.
     *
     * @param type model type, such as Model3d.CUBE or Model3d.Sphere
     * @return newly created model
     */
    Model3d createModel3d(int type, boolean isYUp);

    /**
     * Create object 3D model.
     *
     * @return newly created model
     */
    IObject3d createObject3d();

    /**
     * Create a 2D object for drawing.
     *
     * @return new object that can be used to draw 2D graphics
     */
    Graphics2d createGraphics2d(boolean isYUp);

    /**
     * Create a 3D object for drawing.
     *
     * @return new object that can be used to draw 3D graphics
     */
    Graphics3d createGraphics3d();

    /**
     * Create a Render Layer.
     *
     * @return new object object representing a Render Layer
     */
    RenderLayer createRenderLayer();

    /**
     * Create a Light.
     *
     * @return new object object representing a light
     */
    ILightPresentation createLight();

    /**
     * Pause the rendering
     */
    void pauseRendering();

    /**
     * Resume the rendering
     */
    void resumeRendering();

    /**
     * Check the rendering status
     * @return the rendering is pause or not
     */
    boolean isRenderingPaused();

    /**
     * get FPS
     */
    double getFPS();

    /**
     * Set default projection mode
     */
    void setProjectionMode(int mode);

    /**
     * Add a Layer
     */
    void addRenderLayer(Presentation presentation);

    /**
     * Get Z order of lights in experimental renderer.
     *
     * @return Z light order
     */
    public int getLightZOrder();
}
