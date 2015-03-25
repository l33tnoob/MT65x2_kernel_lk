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
 * Plane Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import android.util.Log;

import com.mediatek.j3m.RenderBlock;
import com.mediatek.j3m.RenderTarget;
import com.mediatek.j3m.Solid;
import com.mediatek.j3m.Texture;
import com.mediatek.j3m.Texture2D;
import com.mediatek.j3m.Appearance;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;

/**
 * Represents a quad in 3D space
 * @hide
 */
public class PlanePresentation extends ActorPresentation
    implements ImageDisplay {

    public static final String TAG = "PlanePresentation";

    private final Dimension mApparentSize = new Dimension(1, 1);
    protected final boolean mIsYUp;

    private Texture2D mTexture;
    private RenderBlock mRenderBlockForAttachment;
    private Dimension mSize = new Dimension(1, 1);
    private Scale mScale = new Scale(1, 1, 1);
    private float mAspectRatio = 1;
    private boolean mKeepAspectRatio; // false by default
    private int mRepeatX = 1;
    private int mRepeatY = 1;
    private Box mSourceRect;
    private int mFilterQuality = Image.FILTER_QUALITY_MEDIUM;
    private boolean mUseMipmaps = true;

    /**
     * Initializes this object with J3M presentation engine
     */
    public PlanePresentation(J3mPresentationEngine engine, boolean isYUp) {
        super(engine);
        mIsYUp = isYUp;
    }

    @Override
    public void onInitialize() {
        super.onInitialize();

        // Replace the default scene node with the quad
        getSceneNode().setParent(null);
        setSceneNode(getEngine().getAssetPool().createSquare());
        getSceneNode().setParent(getAnchorSceneNode());

        // Offset the quad so it its origin is at its top-left corner
        getSceneNode().setPosition(0.5f, 0.5f, 0.0f);

        // Rotate the plane 180 degrees around the x-axis so that it works in
        // y-axis down configurations.  For use in y-axis-up configurations,
        // these actors must be rotated by 180 degrees so that they do not
        // appear upside-down.
        if (!mIsYUp) {
            getSceneNode().setRotation(0, 1, 0, 0);
        }

        Solid solid = (Solid) getSceneNode();
        Appearance appearance = solid.getAppearance();
        appearance.setShaderProgram(
                getEngine().getAssetPool().getShaderProgram("ngin3d#quad.sp"));

        // Basic transparency blending
        appearance.setBlendFactors(
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA);
        appearance.setDepthWriteEnabled(false);

        // Ensure that UV mapping shader uniform is initialised.
        applyTextureMapping();

        // Set a collision shape.
        setShape(getEngine().getJ3m().createSquare());
    }

    @Override
    public void onUninitialize() {

        mTexture = null;
        mSize = null;
        mScale = null;

        super.onUninitialize();
    }

    private void replaceImageSource(ImageSource src, Texture2D texture) {
        if (texture == null) {
            if (src.srcType == ImageSource.RES_ID) {
                Log.e(TAG, "failed to load image source: "
                        + getEngine().getResources().getResourceName(
                            ((ImageDisplay.Resource) src.srcInfo).resId) + "; "
                        + src);
            } else {
                Log.e(TAG, "Failed to load image source " + src);
            }
            return;
        }

        Texture2D replaced = mTexture;
        if (replaced != null && replaced.equals(texture)) {
            return;
        }

        ((Solid) getSceneNode()).getAppearance().setTexture2D(
            "M_DIFFUSE_TEXTURE", texture);

        mTexture = texture;

        if (mRenderBlockForAttachment != null) {
            Texture2D depth = null;
            RenderTarget target = getEngine().getJ3m().createRenderTarget(
                mTexture, depth, true, false);
            mRenderBlockForAttachment.setRenderTarget(target);
            mRenderBlockForAttachment = null;
        }

        mAspectRatio = (float) mTexture.getWidth() / mTexture.getHeight();

        setSize(new Dimension(mTexture.getWidth(), mTexture.getHeight()));

        // Clear source rectangle for new texture
        mSourceRect = null;
        applyTextureMapping();
        updateApparentSize();
        updateTextureFilter();
    }

    /**
     * Sets a new image source for this object.
     *
     * @param src image source
     */
    public void setImageSource(ImageSource src) {
        Texture2D texture = getEngine().getTextureCache().getTexture(src);
        replaceImageSource(src, texture);
    }

    /**
     * Get OpenGL ID ("name" but an integer) for use with live video textures.
     * Exposing OpenGL data at this level is undesirable but currently
     * necessary.  Do not rely on this being available in the long term.
     *
     * @return The 'name' of texture or -1 on failure
     */
    public int getTexName() {
        if (mTexture == null) {
            return -1;
        }
        return mTexture.getOpenGlTextureId();
    }

    /**
     * Sets which rectangular area of the texture is mapped to this object.
     * Calling this function will overwrite the effect of any prior calls to
     * {@link #setRepeat(int,int)}.
     *
     * @param rect
     *            the rectangle to display. If null is specified, the entire
     *            source image will be displayed.
     */
    public void setSourceRect(Box rect) {
        if (rect != null) {
            mSourceRect = new Box(rect);
            applyTextureMapping();
        }
    }

    /**
     * Gets the dimension of the image source of this object.
     *
     * @return a dimension object with the setting of this object
     */
    public Dimension getSourceDimension() {
        if (mTexture == null) {
            return new Dimension(0, 0);
        } else {
            return new Dimension(mTexture.getWidth(), mTexture.getHeight());
        }
    }

    /**
     * Sets the scale values for this object.
     *
     * @param scale a scale object for setting up the scale values of
     * this object
     */
    @Override
    public void setScale(Scale scale) {
        mScale = scale;
        applyScale();
    }

    /**
     * Gets the scale values of this object.
     *
     * @return scale value
     */
    @Override
    public Scale getScale() {
        return mScale;
    }

    /**
     * Combines scale and size and applies to underlying object.
     */
    private void applyScale() {
        getRootSceneNode().setScale(
                mScale.x * mApparentSize.width,
                mScale.y * mApparentSize.height,
                mScale.z);
    }

    /**
     * Updates the apparent size of the object depending on its settings.
     * The apparent size of an object depends on its actual size, and whether
     * it is synced to the aspect ratio.
     */
    private void updateApparentSize() {
        mApparentSize.width = mSize.width;
        mApparentSize.height = mSize.height;

        if (mKeepAspectRatio) {
            // Enforce the aspect ratio, changing the larger dimension to
            // meet the smaller dimension
            if (mAspectRatio > mSize.width / mSize.height) {
                mApparentSize.height = mSize.width / mAspectRatio;
            } else {
                mApparentSize.width = mSize.height * mAspectRatio;
            }
        }

        applyScale();
    }

    /**
     * Sets the size of this object using dimension object.
     *
     * @param size a dimension object with size setting
     */
    public void setSize(Dimension size) {
        if (size.width >= 1 && size.height >= 1) {
            mSize.width = (int) size.width;
            mSize.height = (int) size.height;

            updateApparentSize();
        }
    }

    /**
     * Gets the apparent size of this object.
     *
     * @return Apparent size
     */
    public Dimension getSize() {
        return mApparentSize;
    }

    public int getFilterQuality() {
        return mFilterQuality;
    }

    public void setFilterQuality(int quality) {
        mFilterQuality = quality;
        updateTextureFilter();
    }

    /**
     * Sets whether the object must have the same aspect ratio as its texture.
     * When using {@link #setSize(Dimension)}, if this value is set, the size
     * of the image will be reduced to match the aspect ratio of the texture.
     *
     * This option defaults to false.
     *
     * @param flag Boolean flag
     */
    public void setKeepAspectRatio(boolean flag) {
        if (flag != mKeepAspectRatio) {
            mKeepAspectRatio = flag;
            updateApparentSize();
        }
    }

    /**
     * Checks whether the object must have the same aspect ratio as its texture.
     *
     * @return Boolean flag
     */
    public boolean isKeepAspectRatio() {
        return mKeepAspectRatio;
    }

    /**
     * Applies texture mapping to the rectangle, depending on the current mode.
     */
    private void applyTextureMapping() {
        float offsetU = 0.0f;
        float offsetV = 1.0f;
        float scaleU = mRepeatX;
        float scaleV = -mRepeatY;

        if (mSourceRect != null) {
            // Incoming coordinates are in pixels - top left then bottom right.
            // These need to be converted to OpenGL 0->1 coordinates.
            Dimension dim = getSourceDimension(); // Size of tex, not of rect
            if (dim.width == 0.f) {
                dim.width = 1.f;
            }
            if (dim.height == 0.f) {
                dim.height = 1.f;
            }

            offsetU = mSourceRect.x1 / dim.width;
            offsetV = mSourceRect.y2 / dim.height;
            scaleU = (mSourceRect.x2 - mSourceRect.x1) / dim.width;
            scaleV = (mSourceRect.y1 - mSourceRect.y2) / dim.height;
        }

        ((Solid) getSceneNode()).getAppearance().setVector4f("M_UV_OFFSET_SCALE",
            offsetU, offsetV, scaleU, scaleV);

    }

    /**
     * Sets the x and y repeat times of the image in this object.
     * Calling this function will overwrite any source rectangle set by calls to
     * {@link #setSourceRect(Box)}.
     *
     * @param x x axis repeating times
     * @param y y axis repeating times
     */
    public void setRepeat(int x, int y) {
        if (x != 0 && y != 0) {
            mRepeatX = x;
            mRepeatY = y;
            mSourceRect = null;
            applyTextureMapping();
        }
    }

    /**
     * Gets the repeating times of x axis.
     *
     * @return a value of x repeating times
     */
    public int getRepeatX() {
        return mRepeatX;
    }

    /**
     * Gets the repeating times of y axis.
     *
     * @return a value of y repeating times
     */
    public int getRepeatY() {
        return mRepeatY;
    }

    /**
     * Enable mipmap of the object or not.
     *
     * @param enable true for enable and false for disable
     */
    public void enableMipmap(boolean enable) {
        mUseMipmaps = enable;
        updateTextureFilter();
    }

    /**
     * Check mipmap of the object is enabled or not
     *
     * @return true for enable and false for disable.
     */
    public boolean isMipmapEnabled() {
        return mUseMipmaps;
    }

    /**
     * Make the rect drawable regardless of orientation of the normal.
     * Makes the 'back' of the rectangle visible, whereas it would
     * normally be culled to enhance performance
     *
     * @param enable true for enable and false for disable
     */
    public void enableDoubleSided(boolean enable) {
        if (enable) {
            // Disable culling so both sides are seen
            ((Solid) getSceneNode()).getAppearance().setCullingMode(
                Appearance.CullingMode.NONE);
        } else {
            // Restore the default that back-facing polygons are culled
            ((Solid) getSceneNode()).getAppearance().setCullingMode(
                Appearance.CullingMode.BACK);
        }
    }

    public void attachToRenderBlock(RenderBlock block) {
        if (mTexture == null) {
            mRenderBlockForAttachment = block;
        } else {
            Texture2D depth = null;
            RenderTarget target = getEngine().getJ3m().createRenderTarget(
                mTexture, depth, true, false);
            block.setRenderTarget(target);
        }
    }

    private void updateTextureFilter() {
        if (mTexture == null) {
            return;
        }

        if (mFilterQuality == Image.FILTER_QUALITY_LOW) {
            mTexture.setMagFilter(Texture.FilterMode.NEAREST);
        } else {
            mTexture.setMagFilter(Texture.FilterMode.LINEAR);
        }

        if (mUseMipmaps) {
            if (mFilterQuality == Image.FILTER_QUALITY_HIGH) {
                mTexture.setMinFilter(Texture.FilterMode.LINEAR_MIPMAP_LINEAR);
            } else if (mFilterQuality == Image.FILTER_QUALITY_MEDIUM) {
                mTexture.setMinFilter(Texture.FilterMode.LINEAR_MIPMAP_NEAREST);
            } else {
                mTexture.setMinFilter(
                        Texture.FilterMode.NEAREST_MIPMAP_NEAREST);
            }

        } else if (mFilterQuality == Image.FILTER_QUALITY_LOW) {
            mTexture.setMinFilter(Texture.FilterMode.NEAREST);
        } else {
            mTexture.setMinFilter(Texture.FilterMode.LINEAR);
        }
    }

}
