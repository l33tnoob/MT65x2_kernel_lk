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
 * Basic3d Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import android.util.Log;

import com.mediatek.j3m.Appearance;
import com.mediatek.j3m.Solid;
import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.Model3d;
import com.mediatek.ngin3d.utils.Ngin3dException;

/**
 * Presentation layer object representing a built-in primitive shape.
 * @hide
 */
public class Basic3dPresentation extends ActorPresentation
    implements Model3d {

    public static final String TAG = "Basic3dPresentation";

    private final int mType;
    private final boolean mIsYUp;

    private Texture2D mTexture;

    /**
     * Initializes this object with presentation engine and model type.
     * @param engine  the Engine to be used for initialization.
     * @param type  model type, could be sphere or cube.
     */
    public Basic3dPresentation(J3mPresentationEngine engine, int type, boolean isYUp) {
        super(engine);
        mType = type;
        mIsYUp = isYUp;
    }

    /**
     * Initializes this object
     */
    @Override
    public void onInitialize() {
        super.onInitialize();

        // Replace the default scene node with an appropriate object
        getSceneNode().setParent(null);

        if (mType == Model3d.SPHERE) {
            setSceneNode(getEngine().getAssetPool().createSphere(16, 32));

            if (mIsYUp) {
                // For use in y-axis-up configurations, rotate sphere by 180 degrees
                // so that it does not appear upside-down.
                float angle = (float) Math.PI / 2.0f;
                Quaternion q1 = new Quaternion(0, 1, 0, 0);
                Quaternion q2 = new Quaternion(
                    (float) Math.cos(angle / 2.0f),
                    (float) Math.sin(angle / 2.0f) * 0.0f,
                    (float) Math.sin(angle / 2.0f) * 1.0f,
                    (float) Math.sin(angle / 2.0f) * 0.0f);
                Quaternion q = q1.multiply(q2);
                getSceneNode().setRotation(
                    q.getQ0(),
                    q.getQ1(),
                    q.getQ2(),
                    q.getQ3());
            } else {
                float angle = -(float) Math.PI / 2.0f;
                getSceneNode().setRotation(
                    (float) Math.cos(angle / 2.0f),
                    (float) Math.sin(angle / 2.0f) * 0.0f,
                    (float) Math.sin(angle / 2.0f) * 1.0f,
                    (float) Math.sin(angle / 2.0f) * 0.0f);
            }

            // Set a collision shape.
            setShape(getEngine().getJ3m().createSphere());

        } else if (mType == Model3d.CUBE) {
            setSceneNode(getEngine().getAssetPool().createCube());
            if (mIsYUp) {
                // For use in y-axis-up configurations, rotate cube by 180 degrees
                // so that it does not appear upside-down.
                getSceneNode().setRotation(0, 1, 0, 0);
            }
        } else {
            throw new Ngin3dException("Unsupported model type " + mType);
        }

        Appearance appearance = ((Solid) getSceneNode()).getAppearance();
        appearance.setShaderProgram(
                getEngine().getAssetPool().getShaderProgram("ngin3d#quad.sp"));

        // Flip the UV coordinates vertically so that Basic3D shapes work
        // in y-axis-down configurations.  For use in y-axis-up configurations,
        // these actors must be rotated by 180 degrees so that they do not
        // appear upside-down.
        appearance.setVector4f("M_UV_OFFSET_SCALE", 0.f, 1.f, 1.f, -1.f);

        // Basic transparency blending
        appearance.setBlendFactors(
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA,
                Appearance.BlendFactor.ONE_MINUS_SRC_ALPHA);

        getSceneNode().setParent(getAnchorSceneNode());
    }

    /**
     * Un-initializes this object.
     */
    public void onUninitialize() {

        mTexture = null;

        super.onUninitialize();
    }

    ///////////////////////////////////////////////////////////////////////////
    // Model3d

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
    }

    /**
     * Sets the texture of this object.
     * @param src image source
     */
    public void setTexture(ImageSource src) {
        Texture2D texture = getEngine().getTextureCache().getTexture(src);
        replaceImageSource(src, texture);
    }
}

