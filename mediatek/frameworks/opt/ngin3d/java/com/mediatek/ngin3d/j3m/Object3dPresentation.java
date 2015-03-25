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
 * Object3d Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import com.mediatek.j3m.AnimationController;
import com.mediatek.j3m.Model;
import com.mediatek.j3m.SceneNode;
import com.mediatek.ngin3d.presentation.IObject3d;
import com.mediatek.ngin3d.presentation.ObjectSource;
import com.mediatek.ngin3d.Rotation;

import android.util.Log;

/**
 * A presentation object which represents an instance of a Glo object.
 * @hide
 */

public class Object3dPresentation extends ActorPresentation
    implements IObject3d {

    private static final String TAG = "Object3dPresentation";

    private AnimationController mAnimation;
    private boolean mStarting;
    private float mProgress;
    private boolean mApplyProgress;

    /**
     * Initializes this object with J3M presentation engine
     * @param engine
     */
    public Object3dPresentation(J3mPresentationEngine engine) {
        super(engine);
    }

    /**
     * Called by the presentation engine to unintitialize the object
     */
    @Override
    protected void onUninitialize() {
        disableNewAnimation();
        mApplyProgress = false;
        super.onUninitialize();
    }

    /**
     * Initializes the Glo presentation from a generic source object
     *
     * @param src object source
     */
    public void setObjectSource(ObjectSource src) {
        super.onInitialize();

        // It isn't clear what the difference between loading from file,
        // and loaded from an asset file is, so they are the same here.
        if (src.srcType == ObjectSource.FILE
                || src.srcType == ObjectSource.ASSET) {
            getSceneNode().setParent(null);
            Log.v(TAG, "Loading file asset: " + (String) src.srcInfo);
            Model model = getEngine().getAssetPool().loadModel(
                    (String) src.srcInfo, getEngine().getRenderBlockParent());

            SceneNode node = null;
            mAnimation = null;

            if (model == null) {
                Log.e(TAG, "Failed to load file asset: " + (String) src.srcInfo);
            } else {
                node = model.getSceneNode();
                mAnimation = model.getAnimation();
            }

            if (node == null) {
                node = getEngine().getJ3m().createSceneNode();
            }

            node.setParent(getAnchorSceneNode());
            setSceneNode(node);

            // Looping is enabled by by default if a loop exists.
            if (mAnimation != null) {
                // \todo Make animation disabled by default in A3M.
                mAnimation.setEnabled(false);

                // If a loop is defined, enable looping.  Otherwise, define a
                // loop over the entire of the animation's duration, but don't
                // enable looping by default.
                if (mAnimation.hasLoop()) {
                    mAnimation.setLooping(true);
                } else {
                    mAnimation.setLoopRange(
                            mAnimation.getStart(), mAnimation.getEnd());
                    mAnimation.setLooping(false);
                }

                // Register the animation for update in render loop.
                getEngine().addAnimationController(mAnimation);
            }

        } // else if (src.srcType == ObjectSource.RES_ID) {
            // \todo implement
            // Loading from a Android resource (a file specified using an
            // ID from the generated R.java files) will most likely be done
            // by loading the resource in Java and passing the data to J3M.
        //}
    }

    private SceneNode getSceneNode(String name) {
        return getSceneNode().find(name);
    }

    public void setRotation(String sceneNodeName, Rotation rotation) {
        SceneNode node = getSceneNode(sceneNodeName);
        node.setRotation(
                rotation.getQuaternion().getQ0(),
                rotation.getQuaternion().getQ1(),
                rotation.getQuaternion().getQ2(),
                rotation.getQuaternion().getQ3());
    }

    public boolean isDynamic() {
        return (super.isDynamic() || (mAnimation != null && mAnimation.getEnabled()));
    }

    /*
     * New animation functions.
     */
    public void setAnimationPlaying(boolean flag) {
        if (mAnimation != null) {
            mAnimation.setEnabled(flag);
        }
    }

    public boolean isAnimationPlaying() {
        if (mAnimation == null) {
            return false;
        } else {
            return mAnimation.getEnabled();
        }
    }

    public void setAnimationProgress(float progress) {
        if (mAnimation != null) {
            boolean enabled = mAnimation.getEnabled();
            mAnimation.setEnabled(mApplyProgress);
            float offset = progress * mAnimation.getLength();
            mAnimation.seek(mAnimation.getStart() + offset);
            mAnimation.setEnabled(enabled);
        }
    }

    public float getAnimationProgress() {
        if (mAnimation == null) {
            return 0.0f;
        } else {
            float offset = mAnimation.getProgress() - mAnimation.getStart();
            return offset / mAnimation.getLength();
        }
    }

    public void setAnimationLoopEnabled(boolean flag) {
        if (mAnimation != null) {
            mAnimation.setLooping(flag);
        }
    }

    public boolean getAnimationLoopEnabled() {
        if (mAnimation == null) {
            return false;
        } else {
            return mAnimation.getLooping();
        }
    }

    public void setAnimationSpeed(float speed) {
        if (mAnimation != null) {
            mAnimation.setSpeed(speed);
        }
    }

    public float getAnimationLength() {
        if (mAnimation == null) {
            return 0.0f;
        } else {
            return mAnimation.getLength();
        }
    }

    public void disableNewAnimation() {
        if (mAnimation != null) {
            getEngine().removeAnimationController(mAnimation);
        }
    }

    public void enableApplyProgress() {
        mApplyProgress = true;
    }

    /*
     * The following is the old-style Glo animation system, and should be
     * removed once the new system has been universally adopted.
     */

    public void update(float progress) {
        if (mAnimation != null) {
            if (mStarting) {
                mProgress = progress;
                mStarting = false;
            }

            mAnimation.update(progress - mProgress);
            mProgress = progress;
        }
    }

}
