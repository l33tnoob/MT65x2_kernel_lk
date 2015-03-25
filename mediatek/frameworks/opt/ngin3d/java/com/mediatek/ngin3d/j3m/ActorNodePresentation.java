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
 * Actor Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import android.util.Log;

import com.mediatek.j3m.SceneNode;
import com.mediatek.j3m.Shape;
import com.mediatek.j3m.Solid;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.presentation.IActorNodePresentation.CollisionType;
import com.mediatek.ngin3d.presentation.IActorNodePresentation;

/**
 * Node presentation object.
 * This object simply holds a sub-node of an actor presentation.
 */
public class ActorNodePresentation implements IActorNodePresentation {
    private static final String TAG = "ActorNodePresentation";

    private Object mOwner;
    private J3mPresentationEngine mEngine;
    private SceneNode mNode;
    private float mOpacity = 1.0f;
    private float mParentOpacity = 1.0f;
    private final SceneNode mShapeNode;
    private Shape mShape;
    private Solid mDebugSolid;
    private int mCollisionType = CollisionType.NONE;
    private boolean mCollisionVisible;

    public ActorNodePresentation(
            ActorPresentation actorPresentation,
            String nodeName) {

        mEngine = actorPresentation.getEngine();
        mShapeNode = mEngine.getJ3m().createSceneNode();
        mNode = actorPresentation.getSceneNode();

        if (nodeName != null) {
            mNode = mNode.find(nodeName);
        }

        if (mNode != null) {
            mShapeNode.setParent(mNode);
        }
    }

    /**
     * Initializes the presentation.
     */
    public void initialize(Object owner) {
        mOwner = owner;
    }

    /**
     * Un-initialize this object
     */
    public void uninitialize() {
        mOwner = null;
        mEngine = null;
        mNode = null;
    }

    /**
     * Returns the object which owns this presentation.
     * @return Owner
     */
    public Object getOwner() {
        return mOwner;
    }

    /**
     * Sets the local position of the node.
     * @param position Position of node
     */
    public void setPosition(Point position) {
        mNode.setPosition(position.x, position.y, position.z);
    }

    /**
     * Sets the rotation of the node.
     * @param position Rotation of node
     */
    public void setRotation(Rotation rotation) {
        Quaternion q = rotation.getQuaternion();
        mNode.setRotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());
    }

    /**
     * Sets the scale of the node.
     * @param scale Scale of node
     */
    public void setScale(Scale scale) {
        mNode.setScale(scale.x, scale.y, scale.z);
    }

    /**
     * Sets whether the node is visible.
     *
     * @param visible Visibility flag
     */
    public void setVisible(boolean visible) {
        mNode.setFlags(mEngine.getRenderFlags().VISIBLE, visible);
    }

    /**
     * Sets the color of the node.
     *
     * @param color Color
     */
    public void setColor(Color color) {
        Utility.setColorRecursive(
                mEngine, mNode,
                color.red / 255f,
                color.green / 255f,
                color.blue / 255f,
                color.alpha / 255f);
    }

    /**
     * Sets the opacity of the node.
     *
     * @param opacity Opacity
     */
    public void setOpacity(int opacity) {
        mOpacity = opacity / 255f;
        updateOpacity();
    }

    /**
     * Informs the node of the opacity of its parent.
     *
     * @param opacity Opacity
     */
    void notifyOfParentOpacity(float opacity) {
        mParentOpacity = opacity;
        updateOpacity();
    }

    /**
     * Recursively applies the opacity to all parts of this node.
     */
    private void updateOpacity() {
        Utility.setOpacityRecursive(mEngine, mNode, mOpacity * mParentOpacity);
    }

    /**
     * Sets the type of collision shape used by this node for hit tests.
     * @param type Type of collision shape
     */
    public void setCollisionShape(int type) {
        if (type != mCollisionType) {
            switch (type) {
            case CollisionType.NONE:
                mShape = null;
                break;

            case CollisionType.PLANE:
                mShape = mEngine.getJ3m().createPlane();
                break;

            case CollisionType.SQUARE:
                mShape = mEngine.getJ3m().createSquare();
                break;

            case CollisionType.SPHERE:
                mShape = mEngine.getJ3m().createSphere();
                break;

            default:
                Log.e(TAG, "Invalid collision shape type: " + type);
                return;
            }

            mCollisionType = type;
            updateDebugSolid();
        }
    }

    /**
     * Sets the position of the collision sphere relative to the node.
     * @param position Position of shape
     */
    public void setCollisionPosition(Point position) {
        mShapeNode.setPosition(position.x, position.y, position.z);
    }

    /**
     * Sets the rotation of the collision sphere relative to the node.
     * @param rotation Rotation of shape
     */
    public void setCollisionRotation(Rotation rotation) {
        Quaternion q = rotation.getQuaternion();
        mShapeNode.setRotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());
    }

    /**
     * Sets the scale of the collision sphere relative to the node.
     * @param scale Scale of shape
     */
    public void setCollisionScale(Scale scale) {
        mShapeNode.setScale(scale.x, scale.y, scale.z);
    }

    /**
     * Updates the visual representation of the collision shape.
     */
    private void updateDebugSolid() {
        // Remove old solid, if there was one.
        if (mDebugSolid != null) {
            mDebugSolid.setParent(null);
            mDebugSolid = null;
        }

        if (mCollisionVisible) {
            // Create a solid (or not) depending on the type of collision shape.
            switch (mCollisionType) {
            case CollisionType.NONE:
                break;

            // Use a bounded square for both bounded and unbounded collision
            // planes.  The user can scale the plane sideways if they want the
            // visual representation to be larger.
            case CollisionType.PLANE:
            case CollisionType.SQUARE:
                mDebugSolid = mEngine.getAssetPool().createSquare();
                break;

            case CollisionType.SPHERE:
                mDebugSolid = mEngine.getAssetPool().createSphere(10, 10);
                break;

            default:
                // Make sure all existing shapes are added to this list.
                assert false;
                break;
            }
        }

        // Add the debug shape to the scene graph.
        if (mDebugSolid != null) {
            mDebugSolid.setFlags(mEngine.getRenderFlags().DEBUG, true);
            mDebugSolid.setParent(mShapeNode);
        }
    }

    /**
     * Sets whether to render a representation of the collision shape.
     * @param visible Visibility flag
     */
    public void setCollisionVisible(boolean visible) {
        if (visible != mCollisionVisible) {
            mCollisionVisible = visible;
            updateDebugSolid();
        }
    }

    /**
     * Returns the collision shape used by the node.
     * This function will update the shape transform automatically.
     */
    Shape getShape() {
        if (mShape != null) {
            mShape.setTransform(mShapeNode);
        }

        return mShape;
    }

    /**
     * Request renderer render a frame.
     */
    public void requestRender() {
        mEngine.requestRender();
    }
}
