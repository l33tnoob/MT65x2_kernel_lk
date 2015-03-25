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
/** \file
 * Layer Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import com.mediatek.j3m.AngularUnits;
import com.mediatek.j3m.Appearance;
import com.mediatek.j3m.Camera;
import com.mediatek.j3m.RenderBlock;
import com.mediatek.j3m.RenderBlockBase;
import com.mediatek.j3m.RenderBlockGroup;
import com.mediatek.j3m.RenderTarget;
import com.mediatek.j3m.Renderer;
import com.mediatek.j3m.SceneNode;
import com.mediatek.j3m.Solid;
import com.mediatek.j3m.Texture;
import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.RenderLayer;

import java.util.ArrayList;
import java.util.List;

/**
 * j3m implementation of RenderLayer interface
 * @hide
 */

public class LayerPresentation extends ActorPresentation implements RenderLayer {

    private static final String TAG = "LayerPresentation";

    private static final float FOCUS_RANGE_MIN = 0.001f;

    private RenderBlockGroup mRenderBlockGroup;
    private RenderBlock mRenderBlock;
    private Camera mCamera;
    private final Renderer mRenderer;

    private float mFocusDistance = 1111f;
    private float mFocusRange = 500f;
    private float mBlurFactor = 4f;

    private float mMotionBlurFactor = 1f;
    private int mMotionBlurQuality = MOTION_BLUR_QUALITY_HIGH;

    private RenderBlockGroup mDepthOfFieldGroup;
    private Appearance       mDepthOfFieldAppearance;

    private RenderBlockGroup mMotionBlurGroup;
    private Renderer         mMotionBlurRenderer;
    private Appearance       mMotionBlurAppearance;

    /**
     * Initializes this object with A3M presentation engine.
     * @param engine    Presentation engine
     * @param renderer  Renderer to use
     */
    public LayerPresentation(J3mPresentationEngine engine, Renderer renderer) {
        super(engine);
        mRenderer = renderer;
    }

    @Override
    public void onInitialize() {
        super.onInitialize();
        getRootSceneNode().setParent(getEngine().getRenderBlockParent());

        mCamera = getEngine().getJ3m().createCamera();
        mRenderBlockGroup = getEngine().getJ3m().createRenderBlockGroup();
        mRenderBlock = getEngine().getJ3m().createRenderBlock(
                mRenderer, getRootSceneNode(), mCamera);
        mRenderBlockGroup.addBlock(mRenderBlock);
        mRenderBlock.setColourClear(false);
        mRenderBlock.setDepthClear(true);
    }

    @Override
    public void onUninitialize() {
        mCamera = null;
        mRenderBlockGroup = null;
        mRenderBlock = null;

        super.onUninitialize();
    }

    public RenderBlockBase getRenderBlock() {
        return mRenderBlockGroup;
    }

    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Sets position of camera.
     *
     * @param pos    Camera position
     */
    public void setCameraPosition(Point pos) {
        mCamera.setPosition(pos.x, pos.y, pos.z);
    }

    /**
     * Sets rotation of camera.
     *
     * @param rot    Camera rotation
     */
    public void setCameraRotation(Rotation rot) {
        Quaternion q = rot.getQuaternion();
        mCamera.setRotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());
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
        mCamera.setFov(AngularUnits.DEGREES, fov);
    }

    /**
     * Sets the near and far clipping distances.
     * Note these are distances from the camera, in the forward direction of the
     * camera axis; they are NOT planes positioned along the global Z axis
     * despite often called Znear and Zfar.
     *
     * @param near Objects nearer than this are clipped
     * @param far Objects further away than this are clipped
     */
    public void setClipDistances(float near, float far) {
        mCamera.setNear(near);
        mCamera.setFar(far);
        setDepthOfFieldParams();
    }

    /**
     * Set camera near clipping plane.
     *
     * @param near Camera near clipping plane
     */
    public void setCameraNear(float near) {
        mCamera.setNear(near);
        setDepthOfFieldParams();
    }

    /**
     * Set camera far clipping plane.
     *
     * @param far Camera far clipping plane
     */
    public void setCameraFar(float far) {
        mCamera.setFar(far);
        setDepthOfFieldParams();
    }

    /**
     * Set projection mode.
     *
     * @param mode Projection mode
     */
    public void setProjectionMode(int mode) {
        mCamera.setProjectionType(mode);
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
        mCamera.setWidth(width);
    }

    /**
     * Sets the currently active camera.
     * Passing an empty string activates the default camera.
     *
     */
    public void useNamedCamera(String name) {
        if (name.isEmpty()) {
            mRenderBlock.setCamera(mCamera);
        } else {
            SceneNode node = getRootSceneNode().find(name);

            if (Camera.class.isInstance(node)) {
                mRenderBlock.setCamera((Camera)node);
            }
        }
    }

    /**
     * Sets z-buffer clearing behaviour.
     * Call this method with true will cause the z-buffer to be cleared before
     * the layer is rendered.
     *
     * @param clear True if the z buffer should be cleared
     */
    public void setDepthClear(boolean clear) {
        mRenderBlock.setDepthClear(clear);
    }

    /**
     * Returns a list of names of cameras in the scene.
     *
     */
    public String[] getGloCameraNames() {
        List<String> names = new ArrayList<String>();
        compileGloCameraNames(names, getRootSceneNode());
        String[] namesArray = new String[names.size()];
        names.toArray(namesArray);
        return namesArray;
    }

    /**
     * Traverses the scene graph and compiles a list of all the cameras.
     *
     */
    private void compileGloCameraNames(List<String> names, SceneNode node) {
        int childCount = node.getChildCount();
        for (int i = 0; i < childCount; ++i) {
            SceneNode child = node.getChild(i);

            String name = child.getName();
            if ((!name.isEmpty()) && Camera.class.isInstance(child)) {
                names.add(name);
            }

            compileGloCameraNames(names, child);
        }
    }

    /**
     * Sets destination for rendering operations.
     *
     * @param imageDisplay Image to render into
     */
    public void setRenderTarget(ImageDisplay imageDisplay) {
        PlanePresentation pp = (PlanePresentation)imageDisplay;
        pp.attachToRenderBlock(mRenderBlock);
    }

    /**
     * Sets focus distance for depth of field effect.
     *
     * @param focusDistance Distance at which objects are sharpest
     */
    public void setFocusDistance(float focusDistance) {
        mFocusDistance = focusDistance;
        setDepthOfFieldParams();
    }

    /**
     * Sets focus range for depth of field effect.
     *
     * @param focusRange Range within which objects are quite sharp
     */
    public void setFocusRange(float focusRange) {
        mFocusRange = focusRange;
        setDepthOfFieldParams();
    }

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
    public void setFocusBlurFactor(float blurFactor) {
        mBlurFactor = (float)Math.max(Math.min(blurFactor, 16f), 2f);

        // If the effect has already been enabled, re-enable it to re-size
        if (mDepthOfFieldGroup != null) {
            enableDepthOfField(false);
            enableDepthOfField(true);
        }
    }

    /**
     * Enable depth of field effect.
     *
     * @param enable True to enable, false to disable
     */
    public void enableDepthOfField(boolean enable) {
        if (enable) {
            if (mDepthOfFieldGroup == null) {
                enableMotionBlur(false);
                initDepthOfFieldEffect();
                mRenderBlock.setColourClear(true);
                mRenderBlock.setBackgroundColour(0f, 0f, 0f, 0f);
                setDepthOfFieldParams();
            }
        } else {
            if (mDepthOfFieldGroup != null) {
                mRenderBlock.setRenderTarget(null);
                mRenderBlock.setColourClear(false);
                mRenderBlockGroup.removeBlock(mDepthOfFieldGroup);
                mDepthOfFieldGroup = null;
                mDepthOfFieldAppearance = null;
            }
        }
    }

    /**
     * Enable motion blur effect.
     *
     * @param enable True to enable, false to disable
     */
    public void enableMotionBlur(boolean enable) {
        if (enable) {
            if (mMotionBlurGroup == null) {
                enableDepthOfField(false);
                initMotionBlurEffect();
                mRenderBlock.setColourClear(true);
                setMotionBlurParams();
            }
        } else {
            if (mMotionBlurGroup != null) {
                mRenderBlock.setRenderTarget(null);
                mRenderBlock.setColourClear(false);
                mRenderBlockGroup.removeBlock(mMotionBlurGroup);
                mMotionBlurGroup = null;
                mMotionBlurRenderer = null;
                mMotionBlurAppearance = null;
            }
        }
    }

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
    public void setMotionBlurFactor(float blurFactor) {
        mMotionBlurFactor = blurFactor;
        setMotionBlurParams();
    }

    /**
     * Sets quality for motion blur effect.
     *
     * This parameter changes the quality of the motion blur effect. Low quality
     * uses fewer samples to produce the effect, which may result in higher
     * performance.
     *
     * @param quality One of MOTION_BLUR_QUALITY_LOW or MOTION_BLUR_QUALITY_HIGH
     */
    public void setMotionBlurQuality(int quality) {
        mMotionBlurQuality = quality;
        setMotionBlurParams();
    }

    /**
     * Sets the portion of the screen to which to render.
     *
     * @param left left edge of the screen area
     * @param bottom bottom edge of the screen area
     * @param width width of the screen area
     * @param height height of the screen area
     */
    public void setViewport(float left, float bottom, float width, float height)
    {
        mRenderBlock.setViewport(left, bottom, width, height);
    }

    private final class PostProcessStage {
        public PostProcessStage(Appearance appearance, Texture2D texture) {
            this.appearance = appearance;
            this.texture = texture;
        }
        public final Appearance appearance;
        public final Texture2D texture;
    }

    private final PostProcessStage addPostProcessStage(int xRes, int yRes,
        RenderBlockGroup group) {

        Texture2D targetTexture = getEngine().getAssetPool().createTexture2D(
            xRes, yRes, Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE,
            new byte[xRes * yRes * 4]);

        RenderTarget target = getEngine().getJ3m().createRenderTarget(
            targetTexture, null, false, false);

        Solid quad = getEngine().getAssetPool().createSquare();

        RenderBlock block = getEngine().getJ3m().createRenderBlock(
            mRenderer, quad, null);
        block.setRenderTarget(target);

        group.addBlock(block);

        return new PostProcessStage(quad.getAppearance(), targetTexture);
    }


    private void initDepthOfFieldEffect() {
        mDepthOfFieldGroup = getEngine().getJ3m().createRenderBlockGroup();

        int width = getEngine().getWidth();
        int height = getEngine().getHeight();
        int widthLowRes = (int)((float)width / mBlurFactor);
        int heightLowRes = (int)((float)height / mBlurFactor);


        /* Set up the default render block to render into a texture.
         * This is a full resolution texture used both as a source for the
         * first blur stage and a source for the final image (blended with
         * the final blurred image).
         */

        Texture2D depthTexture = getEngine().getAssetPool().createTexture2D(
            width, height, Texture.Format.DEPTH, Texture.Type.UNSIGNED_SHORT,
            new byte[width * height * 2]);
        Texture2D sharpTexture = getEngine().getAssetPool().createTexture2D(
            width, height, Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE,
            new byte[width * height * 4]);
        RenderTarget sharpTarget = getEngine().getJ3m().createRenderTarget(
            sharpTexture, depthTexture, true, false);
        mRenderBlock.setRenderTarget(sharpTarget);


        /* Set up the first (horizontal) blur stage, taking the sharp
         * texture as input and rendering to a lower resolution texture.
         */

        PostProcessStage hStage = addPostProcessStage(widthLowRes, heightLowRes, mDepthOfFieldGroup);

        getEngine().getAssetPool().applyAppearance(hStage.appearance, "ngin3d#blur.mat");
        hStage.appearance.setTexture2D("M_DIFFUSE_TEXTURE", sharpTexture);
        hStage.appearance.setVector2f("M_SAMPLE_MULT", 1.0f / widthLowRes, 0.f);


        /* Set up the second (vertical) blur stage, taking the horizontally
         * blurred texture as input and rendering to a lower resolution texture.
         */

        PostProcessStage vStage = addPostProcessStage(widthLowRes, heightLowRes, mDepthOfFieldGroup);

        getEngine().getAssetPool().applyAppearance(vStage.appearance, "ngin3d#blur.mat");
        vStage.appearance.setTexture2D("M_DIFFUSE_TEXTURE", hStage.texture);
        vStage.appearance.setVector2f("M_SAMPLE_MULT", 0.f, 1.0f / heightLowRes);


        /* Set up the depth of field stage, taking the final blurred texture
         * and the original sharp as input and rendering to the device.
         */

        Solid quadDepthOfField = getEngine().getAssetPool().createSquare();
        getEngine().getAssetPool().applyAppearance(quadDepthOfField.getAppearance(),
            "ngin3d#depthoffield.mat");
        mDepthOfFieldAppearance = quadDepthOfField.getAppearance();
        mDepthOfFieldAppearance.setTexture2D("M_SHARP_TEXTURE", sharpTexture);
        mDepthOfFieldAppearance.setTexture2D("M_BLURRED_TEXTURE", vStage.texture);
        mDepthOfFieldAppearance.setTexture2D("M_DEPTH_TEXTURE", depthTexture);

        mDepthOfFieldAppearance.setBlendFactors(
            Appearance.BlendFactor.ONE,
            Appearance.BlendFactor.ZERO,
            Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA,
            Appearance.BlendFactor.ONE);

        RenderBlock depthOfFieldBlock = getEngine().getJ3m().createRenderBlock(
            mRenderer, quadDepthOfField, null);
        depthOfFieldBlock.setColourClear(false);

        mDepthOfFieldGroup.addBlock(depthOfFieldBlock);

        mRenderBlockGroup.addBlock(mDepthOfFieldGroup);
    }

    private void setDepthOfFieldParams() {
        if (mDepthOfFieldAppearance != null) {
            mDepthOfFieldAppearance.setFloat("M_FOCUS_DISTANCE", mFocusDistance);
            mDepthOfFieldAppearance.setFloat("M_FOCUS_RANGE_RECIPROCAL",
                2.f / Math.max(mFocusRange, FOCUS_RANGE_MIN));
            float near = mCamera.getNear();
            float far = mCamera.getFar();
            mDepthOfFieldAppearance.setFloat("M_NEAR_X_FAR", near * far);
            mDepthOfFieldAppearance.setFloat("M_FAR", far);
            mDepthOfFieldAppearance.setFloat("M_NEAR_MINUS_FAR", near - far);
        }
    }

    private void initMotionBlurEffect() {
        mMotionBlurGroup = getEngine().getJ3m().createRenderBlockGroup();

        int width = getEngine().getWidth();
        int height = getEngine().getHeight();


        /* Set up the default render block to render into a texture.
         * The texture will be used as a source for the second stage.
         */

        Texture2D sharpTexture = getEngine().getAssetPool().createTexture2D(
            width, height, Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE,
            new byte[width * height * 4]);
        RenderTarget sharpTarget = getEngine().getJ3m().createRenderTarget(
            sharpTexture, null, true, false);
        mRenderBlock.setRenderTarget(sharpTarget);
        mRenderBlock.setColourClear(true);
        mRenderBlock.setBackgroundColour(0.f, 0.f, 0.f, 0.f);

        /* Set up another renderblock to render velocities into.
         */

        Texture2D velocityTexture = getEngine().getAssetPool().createTexture2D(
            width, height, Texture.Format.RGBA, Texture.Type.UNSIGNED_BYTE,
            new byte[width * height * 4]);
        mMotionBlurRenderer =
                getEngine().getJ3m().createMotionBlurRenderer(getEngine().getAssetPool());
        RenderBlock velocityBlock = getEngine().getJ3m().createRenderBlock(
            mMotionBlurRenderer, getRootSceneNode(), mCamera);
        RenderTarget velocityTarget = getEngine().getJ3m().createRenderTarget(
            velocityTexture, null, true, false);
        velocityBlock.setRenderTarget(velocityTarget);
        velocityBlock.setColourClear(true);
        velocityBlock.setBackgroundColour(0.5f, 0.5f, 0.f, 1.f);

        mMotionBlurGroup.addBlock(velocityBlock);

        /* Set up the motion blur stage, taking the velocity texture
         * and the original sharp as input and rendering to the device.
         */

        Solid quad = getEngine().getAssetPool().createSquare();
        mMotionBlurAppearance = quad.getAppearance();
        mMotionBlurAppearance.setShaderProgram(
                getEngine().getAssetPool().getShaderProgram("a3m#motionblur.sp$HQMB"));
        mMotionBlurAppearance.setTexture2D("M_SHARP_TEXTURE", sharpTexture);
        mMotionBlurAppearance.setTexture2D("M_VELOCITY_TEXTURE", velocityTexture);

        mMotionBlurAppearance.setBlendFactors(
            Appearance.BlendFactor.ONE,
            Appearance.BlendFactor.ZERO,
            Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA,
            Appearance.BlendFactor.ONE);

        RenderBlock block = getEngine().getJ3m().createRenderBlock(
            mRenderer, quad, null);
        block.setColourClear(false);

        mMotionBlurGroup.addBlock(block);

        mRenderBlockGroup.addBlock(mMotionBlurGroup);
    }

    private void setMotionBlurParams() {
        if (mMotionBlurRenderer != null) {
            mMotionBlurRenderer.setProperty("MOTION_BLUR_FACTOR", mMotionBlurFactor);
        }
        if (mMotionBlurAppearance != null) {
            if (mMotionBlurQuality == MOTION_BLUR_QUALITY_HIGH) {
                mMotionBlurAppearance.setShaderProgram(
                        getEngine().getAssetPool().getShaderProgram("a3m#motionblur.sp$HQMB"));
            } else {
                mMotionBlurAppearance.setShaderProgram(
                        getEngine().getAssetPool().getShaderProgram("a3m#motionblur.sp"));
            }
        }
    }
}
