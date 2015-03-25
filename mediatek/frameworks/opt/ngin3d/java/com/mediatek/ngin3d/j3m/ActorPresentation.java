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
 * Actor Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import android.graphics.Bitmap;
import android.util.Log;
import com.mediatek.j3m.Appearance;
import com.mediatek.j3m.Camera;
import com.mediatek.j3m.Ray;
import com.mediatek.j3m.SceneNode;
import com.mediatek.j3m.Shape;
import com.mediatek.j3m.Solid;
import com.mediatek.j3m.Texture2D;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.presentation.IActorNodePresentation;
import com.mediatek.ngin3d.presentation.Presentation;
import com.mediatek.ngin3d.presentation.PresentationHitTestResult;
import com.mediatek.ngin3d.presentation.RenderLayer;

import java.util.ArrayList;
import java.util.List;

/**
 * Base presentation object which represents an empty scene node.
 * This class can be extended to provide additional types of scene
 * node presentation.
 */
public class ActorPresentation implements Presentation {

    private static final String TAG = "ActorPresentation";

    private static final int MAX_RENDER_LAYER = 3;

    private J3mPresentationEngine mEngine;
    private SceneNode mRootSceneNode;
    private SceneNode mAnchorSceneNode;
    private SceneNode mSceneNode;
    private Shape mShape;
    private String mName;
    private Object mOwner;
    private boolean mHasDynamicMaterial;
    private boolean mHasDynamicMaterialDirty = true;
    private int mRenderLayer;

    private final List<ActorPresentation> mChildren =
            new ArrayList<ActorPresentation>();
    private final List<ActorNodePresentation> mNodes =
            new ArrayList<ActorNodePresentation>();

    private ActorPresentation mParent;
    private final Color mColor = new Color(255, 255, 255, 255);
    private float mOpacity = 1.0f;
    private float mParentOpacity = 1.0f;

    public ActorPresentation(J3mPresentationEngine engine) {
        mEngine = engine;
    }

    public Object getOwner() {
        return mOwner;
    }

    /**
     * Initialize with the owner object.
     *
     * @param owner The owner
     */
    public void initialize(Object owner) {
        onInitialize();
        mOwner = owner;
    }

    protected void onInitialize() {
        if (mAnchorSceneNode == null) {
            // Transformation are applied to the root scene node
            mRootSceneNode = mEngine.getJ3m().createSceneNode();
            mRootSceneNode.setParent(mEngine.getRootNode());

            // The anchor scene node is used to apply an anchor point offset.
            // The anchor point is the effective local origin of the node.
            mAnchorSceneNode = mEngine.getJ3m().createSceneNode();
            mAnchorSceneNode.setParent(mRootSceneNode);

            // The scene node is replaced by whatever type of object this
            // presentation is representing.  We create an empty dummy node
            // to start with.
            setSceneNode(mEngine.getJ3m().createSceneNode());
            mSceneNode.setParent(mAnchorSceneNode);
        }
    }

    /**
     * Checks if this object is initialized
     *
     * @return true if this object is initialized
     */
    public boolean isInitialized() {
        return mSceneNode != null;
    }

    /**
     * Un-initialize this object
     */
    public void uninitialize() {
        onUninitialize();
    }

    protected void onUninitialize() {
        if (mParent != null) {
            mParent.removeChild(this);
        }

        if (mRootSceneNode != null) {
            // Remove the nodes from the scene graph
            mRootSceneNode.setParent(null);
            mAnchorSceneNode.setParent(null);
            mSceneNode.setParent(null);
        }

        mEngine = null;
        mRootSceneNode = null;
        mAnchorSceneNode = null;
        mSceneNode = null;
        mOwner = null;
        mName = null;
    }

    /**
     * Sets the position of this object
     *
     * @param pos position setting using Point object
     */
    public void setPosition(Point pos) {
        if (pos.isNormalized) {
            mRootSceneNode.setPosition(
                    pos.x * mEngine.getWidth(),
                    pos.y * mEngine.getHeight(),
                    pos.z);
        } else {
            mRootSceneNode.setPosition(pos.x, pos.y, pos.z);
        }
    }

    /**
     * Gets the position with normalize argument.
     * When normalize is true means the position is using absolute coordinates,
     * false is for relational coordinate
     *
     * @param normalized boolean value for normalize setting
     * @return position point value
     */
    public Point getPosition(boolean normalized) {
        Point pos = new Point(
                mRootSceneNode.getPositionX(),
                mRootSceneNode.getPositionY(),
                mRootSceneNode.getPositionZ());

        if (normalized) {
            pos.x /= mEngine.getWidth();
            pos.y /= mEngine.getHeight();
            pos.isNormalized = true;
        }

        return pos;
    }

    /**
     * Sets the visible status for this object
     *
     * @param visible - 'true' to make scene node visible, 'false' otherwise
     */
    public void setVisible(boolean visible) {
        mRootSceneNode.setFlags(mEngine.getRenderFlags().VISIBLE, visible);
    }

    /**
     * Gets the visibility status of this object.
     *
     * @return true if visibility flag set, false otherwise
     */
    public boolean getVisible() {
        return mRootSceneNode.getFlags(mEngine.getRenderFlags().VISIBLE);
    }

    /**
     * Checks whether this node is currently visible. A node may be invisible
     * because either its visibility flag is set FALSE or if one of its
     * parents has a visibility flag set FALSE.
     *
     * @return TRUE if this node is visible.
     */
    public boolean getTrulyVisible() {
        return mRootSceneNode.getDerivedFlags(mEngine.getRenderFlags().VISIBLE);
    }

    /**
     * Finds a scene node within this object.
     * This function logs an error if the node cannot be found.
     *
     * @param name Name of scene node for which to search
     * @return Scene node, or null if no scene node with name exists
     */
    private SceneNode getSceneNode(String name) {
        SceneNode node = mSceneNode.find(name);

        if (node == null) {
            Log.e(TAG, "Cannot find node with name \"" + name + "\".");
        }

        return node;
    }

    /**
     * Sets the material for this presentation.
     *
     * @param name Name of the material to set
     */
    public void setMaterial(String name) {
        setMaterialRecursive(mSceneNode, name);
        mHasDynamicMaterialDirty = true;
    }

    /**
     * Sets the material for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the material
     * @param name Name of the material to set
     */
    public void setMaterial(String nodeName, String name) {
        setMaterialRecursive(getSceneNode(nodeName), name);
        mHasDynamicMaterialDirty = true;
    }

    /**
     * Sets an integer material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, int value) {
        setMaterialPropertyRecursive(mSceneNode, name, Integer.valueOf(value));
    }

    /**
     * Sets an integer material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String nodeName, String name, int value) {
        setMaterialPropertyRecursive(getSceneNode(nodeName), name,
                Integer.valueOf(value));
    }

    /**
     * Sets a floating point material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, float value) {
        setMaterialPropertyRecursive(mSceneNode, name, Float.valueOf(value));
    }

    /**
     * Sets a floating point material property for a node within this
     * presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String nodeName, String name, float value) {
        setMaterialPropertyRecursive(getSceneNode(nodeName), name,
                Float.valueOf(value));
    }

    /**
     * Sets a boolean material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, boolean value) {
        setMaterialPropertyRecursive(mSceneNode, name, Boolean.valueOf(value));

        // Property may be "DYNAMIC" (defer check to avoid code duplication).
        mHasDynamicMaterialDirty = true;
    }

    /**
     * Sets a boolean material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String nodeName, String name, boolean value) {
        setMaterialPropertyRecursive(getSceneNode(nodeName), name,
                Boolean.valueOf(value));

        // Property may be "DYNAMIC" (defer check to avoid code duplication).
        mHasDynamicMaterialDirty = true;
    }

    /**
     * Sets a point material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, Point value) {
        setMaterialPropertyRecursive(mSceneNode, name, value);
    }

    /**
     * Sets a point material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String nodeName, String name, Point value) {
        setMaterialPropertyRecursive(getSceneNode(nodeName), name, value);
    }

    /**
     * Returns a texture of the given name.
     * If the texture is not found, this function logs an error message.
     *
     * @param name Name of the texture to get
     * @return Texture with the specified name
     */
    private Texture2D getTexture2D(String name) {
        Texture2D texture = mEngine.getAssetPool().getTexture2D(name);

        if (texture == null) {
            Log.e(TAG, "Texture \"" + name + "\" cannot be found.");
        }

        return texture;
    }

    /**
     * Sets a texture material property.
     *
     * @param name Name of the property to set
     * @param textureName Name of the texture to set
     */
    public void setMaterialProperty(String name, String textureName) {
        Texture2D texture = getTexture2D(textureName);

        if (texture != null) {
            setMaterialPropertyRecursive(mSceneNode, name, texture);
        }
    }

    /**
     * Sets an integer material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param textureName Name of the texture to set
     */
    public void setMaterialProperty(String nodeName, String name,
            String textureName) {
        Texture2D texture = getTexture2D(textureName);

        if (texture != null) {
            setMaterialPropertyRecursive(getSceneNode(nodeName), name, texture);
        }
    }

    /**
     * Sets a texture material property.
     *
     * @param name Name of the property to set
     * @param bitmap object of the texture to set
     */
    public void setMaterialProperty(String name, Bitmap bitmap) {
        Texture2D texture = mEngine.getTextureCache().getTexture(bitmap);

        if (texture != null) {
            setMaterialPropertyRecursive(mSceneNode, name, texture);
        }
    }

    /**
     * Sets a texture material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param bitmap object of the texture to set
     */
    public void setMaterialProperty(String nodeName, String name,
            Bitmap bitmap) {
        Texture2D texture = mEngine.getTextureCache().getTexture(bitmap);

        if (texture != null) {
            setMaterialPropertyRecursive(getSceneNode(nodeName), name, texture);
        }
    }

    /**
     * Iterates over all scene nodes to determine whether any of them uses a
     * dynamic material.
     *
     * @param node Node to check
     */
    private void updateHasDynamicMaterial(SceneNode node) {
        if (Solid.class.isInstance(node)) {
            Solid solid = (Solid) node;

            // If DYNAMIC doesn't exist, or is false, this will return false.
            if (solid.getAppearance().getBoolean("DYNAMIC")) {
                mHasDynamicMaterial = true;
            }
        }

        int childCount = node.getChildCount();

        // Dynamic material flag is checked, as we can stop iterating when the
        // material is found to be dynamic.
        for (int i = 0; i < childCount && !mHasDynamicMaterial; ++i) {
            SceneNode childNode = node.getChild(i);
            updateHasDynamicMaterial(childNode);
        }
    }

    /**
     * Returns whether the actor is continuously changing.
     * This property can be inspected to determine whether the screen needs to
     * be updated continuously.
     *
     * @return True if the actor is dynamic
     */
    public boolean isDynamic() {
        if (mHasDynamicMaterialDirty) {
            mHasDynamicMaterial = false;
            updateHasDynamicMaterial(mSceneNode);
            mHasDynamicMaterialDirty = false;
        }

        return mHasDynamicMaterial;
    }

    /**
     * Sets the name of this objects
     *
     * @param name a string to be set for name
     */
    public void setName(String name) {
        mName = name;
    }

    /**
     * Gets the name of this object.
     *
     * @return name of this object.
     */
    public String getName() {
        return mName;
    }

    /**
     * Sets the rotation values for this object.
     *
     * @param rotation a rotation object for setting up the
     * rotation value of this object.
     */
    public void setRotation(Rotation rotation) {
        Quaternion q = rotation.getQuaternion();
        mRootSceneNode.setRotation(q.getQ0(), q.getQ1(), q.getQ2(), q.getQ3());
    }

    /**
     * Gets the rotation values of this object.
     *
     * @return the rotation value.
     */
    public Rotation getRotation() {
        return new Rotation(
                mRootSceneNode.getRotationA(),
                mRootSceneNode.getRotationB(),
                mRootSceneNode.getRotationC(),
                mRootSceneNode.getRotationD(),
                false);
    }

    /**
     * Sets the scale values for this object.
     *
     * @param scale a scale object for setting up the scale values
     * of this object
     */
    public void setScale(Scale scale) {
        mRootSceneNode.setScale(scale.x, scale.y, scale.z);
    }

    /**
     * Gets the scale values of this object.
     *
     * @return scale value
     */
    public Scale getScale() {
        return new Scale(
                mRootSceneNode.getScaleX(),
                mRootSceneNode.getScaleY(),
                mRootSceneNode.getScaleZ());
    }

    /**
     * Sets the anchor point values for this object.
     *
     * @param point a point object to be used for setting up
     * the anchor point of this object.
     */
    public void setAnchorPoint(Point point) {
        mAnchorSceneNode.setPosition(-point.x, -point.y, -point.z);
    }

    /**
     * Gets the anchor point values of this object.
     *
     * @return anchor point value
     */
    public Point getAnchorPoint() {
        return new Point(
                -mAnchorSceneNode.getPositionX(),
                -mAnchorSceneNode.getPositionY(),
                -mAnchorSceneNode.getPositionZ());
    }

    /**
     * Sets the diffuse color of this object.
     * The specified color will be recursively applied to the J3M scene graph
     * for this presentation, and will be recursively applied as a derived
     * color for all child presentations using the following equation:
     *
     *   derivedColor = parentColor * color
     *
     * @param color Color to set
     */
    public void setColor(Color color) {
        mColor.set(color);

        Utility.setColorRecursive(
                mEngine, mSceneNode,
                color.red / 255f,
                color.green / 255f,
                color.blue / 255f,
                color.alpha / 255f);
    }

    /**
     * Gets the diffuse color of this object.
     *
     * @see #setColor(Color)
     *
     * @return Color object.
     */
    public Color getColor() {
        return mColor;
    }

    /**
     * Sets the opacity of this object.
     *
     * @param opacity Opacity value (ranges from 0 to 255).
     */
    public void setOpacity(int opacity) {
        mOpacity = opacity / 255.0f;
        updateOpacity();
    }

    /**
     * Returns the opacity of this object.
     *
     * @return Opacity value (ranges from 0 to 255).
     */
    public int getOpacity() {
        return (int) (mOpacity * 255.0f);
    }

    /**
     * Informs the actor of the opacity of its parent.
     *
     * @param opacity Opacity
     */
    void notifyOfParentOpacity(float opacity) {
        mParentOpacity = opacity;
        updateOpacity();
    }

    /**
     * Applies the Actor's opacity to all child actors and nodes.
     */
    private void updateOpacity() {
        float opacity = mParentOpacity * mOpacity;

        // If this actor has no children, then it is not a Container, and we
        // will all the nodes it contains; otherwise, it is a Container, and we
        // update all of its child nodes.
        if (mChildren.isEmpty()) {
            Utility.setOpacityRecursive(mEngine, mSceneNode, opacity);
        } else {
            for (int i = 0; i < mChildren.size(); ++i) {
                mChildren.get(i).notifyOfParentOpacity(opacity);
            }
        }

        // Always update the ActorNodes, since they can be owned by Containers
        // and non-Containers, and update them after the rest of the objects.
        for (int i = 0; i < mNodes.size(); ++i) {
            mNodes.get(i).notifyOfParentOpacity(opacity);
        }
    }

    /**
     * Performs a raycast hit test on the scene using a screen space coordinate.
     * The nearest presentation object intersected will be returned, along with
     * additional details about the raycast test.
     *
     * @param screenPoint Screen space point on the screen to pick
     * @param layer layer to which this object belongs
     * @return The result of the hit test
     */
    public PresentationHitTestResult hitTest(Point screenPoint, RenderLayer layer) {
        // Abort early if this part of the scene graph is invisible.
        if (!getTrulyVisible()) {
            return null;
        }

        // This avoids creating a new ray object each time we do a hit test.
        Ray ray = mEngine.getHitTestRay();

        Camera camera;
        if (layer == null) {
            camera = mEngine.getCamera();
        } else {
            camera = ((LayerPresentation)layer).getCamera();
        }

        // Set up the ray once before recursing.
        ray.setToCameraRay(
                camera,
                mEngine.getWidth(),
                mEngine.getHeight(),
                screenPoint.x,
                screenPoint.y);

        PresentationHitTestResult nearest = new PresentationHitTestResult();
        raycast(ray, nearest);

        // If a hit occurred.
        ActorPresentation actorPresentation =
            (ActorPresentation) nearest.getActorPresentation();

        if (actorPresentation != null) {
            ActorNodePresentation subActorPresentation =
                (ActorNodePresentation) nearest.getActorNodePresentation();

            // Get the node shape if hit test was with node.
            Shape shape;
            if (subActorPresentation == null) {
                shape = actorPresentation.getShape();
            } else {
                shape = subActorPresentation.getShape();
            }

            // \todo Consider redesign to not require newing of Points.
            Point rayStart = new Point(
                    ray.getPositionX(),
                    ray.getPositionY(),
                    ray.getPositionZ());

            Point rayDirection = new Point(
                    ray.getDirectionX(),
                    ray.getDirectionY(),
                    ray.getDirectionZ());

            float distance = shape.getRaycastDistance();

            Point rayHit = new Point(
                    rayStart.x + distance * rayDirection.x,
                    rayStart.y + distance * rayDirection.y,
                    rayStart.z + distance * rayDirection.z);

            Point rayHitNormal = new Point(
                    shape.getRaycastNormalX(),
                    shape.getRaycastNormalY(),
                    shape.getRaycastNormalZ());

            nearest.setRay(rayStart, rayDirection, rayHit, rayHitNormal);
        }

        return nearest;
    }

    /**
     * Recursively performs raycast on all children of this node.
     * The raycast intersection nearest to the start of the ray is always
     * stored in the RaycastResult object recursively passed to this function.
     *
     * @param ray Ray used for intersection test
     * @param nearest Raycast result containing nearest intersection.
     */
    private void raycast(Ray ray, PresentationHitTestResult nearest) {
        // Since we checked isTrulyVisible() in hitTest(), we can just check the
        // normal visibility flag from now on.
        if (getVisible()) {
            // Perform raycast on self.
            raycast(ray, nearest, null);

            // Perform raycasts on nodes.
            for (ActorNodePresentation subActor : mNodes) {
                raycast(ray, nearest, subActor);
            }

            // Test all children
            for (int i = 0; i < getChildrenCount(); ++i) {
                ActorPresentation child = (ActorPresentation) getChild(i);
                child.raycast(ray, nearest);
            }
        }
    }

    /**
     * Performs a raycast on a node.
     */
    private void raycast(Ray ray, PresentationHitTestResult nearest,
            ActorNodePresentation subActor) {
        Shape shape = (subActor == null) ? getShape() : subActor.getShape();

        // Does ray intersect?
        if (shape != null) {
            if (shape.raycast(ray)) {
                // Is this the first or nearest collision?
                if (nearest.getActorPresentation() == null) {
                    nearest.setActorPresentation(this);
                    nearest.setActorNodePresentation(subActor);
                } else {
                    int layer = getRenderLayer();
                    float distance = shape.getRaycastDistance();

                    ActorPresentation actorPresentation =
                        (ActorPresentation) nearest.getActorPresentation();
                    ActorNodePresentation subActorPresentation =
                        (ActorNodePresentation) nearest.getActorNodePresentation();

                    int nearestLayer = actorPresentation.getRenderLayer();

                    Shape nearestShape;
                    if (subActorPresentation == null) {
                        nearestShape = actorPresentation.getShape();
                    } else {
                        nearestShape = subActorPresentation.getShape();
                    }

                    assert nearestShape != null;
                    float nearestDistance = nearestShape.getRaycastDistance();

                    // Higher layers are rendered on top, and so layer
                    // number trumps raycast distance.
                    if (layer >= nearestLayer
                            && (layer > nearestLayer
                             || distance < nearestDistance)) {
                        nearest.setActorPresentation(this);
                        nearest.setActorNodePresentation(subActor);
                    }
                }
            }
        }
    }

    /**
     * Sets the collision shape for this presentation.
     */
    protected void setShape(Shape shape) {
        mShape = shape;
    }

    /**
     * Returns the collision shape for this presentation.
     * This function will update the shape transform automatically.
     */
    protected Shape getShape() {
        if (mShape != null) {
            mShape.setTransform(mSceneNode);
        }

        return mShape;
    }

    /**
     * Add child to this scene node
     *
     * @param presentation The presentation
     */
    public void addChild(Presentation presentation) {
        assert presentation instanceof ActorPresentation;
        ActorPresentation actorPresentation =
                (ActorPresentation) presentation;
        actorPresentation.mRootSceneNode.setParent(mRootSceneNode);
        mChildren.add(actorPresentation);
        actorPresentation.mParent = this;

        // Apply derived opacity recursively to new child node
        actorPresentation.notifyOfParentOpacity(mOpacity * mParentOpacity);
    }

    /**
     * Removes child from this scene node
     *
     * @param presentation The presentation
     */
    public void removeChild(Presentation presentation) {
        assert presentation instanceof ActorPresentation;
        ActorPresentation actorPresentation =
                (ActorPresentation) presentation;

        if (mChildren.remove(actorPresentation)) {
            actorPresentation.mRootSceneNode.setParent(null);
            actorPresentation.mParent = null;
        }
    }

    /**
     * Removes all children from this scene node
     */
    public void removeAllChildren() {
        for (ActorPresentation actorPresentation : mChildren) {
            actorPresentation.mRootSceneNode.setParent(null);
            actorPresentation.mParent = null;
        }

        mChildren.clear();
    }

    /**
     * Gets the child through the child index
     *
     * @param index child index
     * @return a child presentation
     */
    public Presentation getChild(int index) {
        return mChildren.get(index);
    }

    /**
     * Gets the number of this class's children
     *
     * @return number of children
     */
    public int getChildrenCount() {
        return mChildren.size();
    }

    /**
     * Creates a node presentation pointing to a given node.
     * @param nodeName Name of the node the node will reference
     * @return Node
     */
    public IActorNodePresentation createActorNodePresentation(String nodeName) {
        ActorNodePresentation presentation = new ActorNodePresentation(
                this, nodeName);
        mNodes.add(presentation);
        return presentation;
    }

    /**
     * Returns the engine for this object.
     *
     * @return scene node
     */
    protected J3mPresentationEngine getEngine() {
        return mEngine;
    }

    /**
     * Gets the scene node of this object.
     *
     * @return scene node
     */
    protected SceneNode getSceneNode() {
        return mSceneNode;
    }

    /**
     * Sets the scene node of this object.
     */
    protected void setSceneNode(SceneNode node) {
        mSceneNode = node;

        // Ensure that render layer flags are set.
        setRenderZOrder(getRenderZOrder());
    }

    /**
     * Gets the root scene node of this object.
     *
     * @return Root scene node
     */
    protected SceneNode getRootSceneNode() {
        return mRootSceneNode;
    }

    /**
     * Gets the "anchor" scene node of this object.
     *
     * @return Anchor scene node
     */
    protected SceneNode getAnchorSceneNode() {
        return mAnchorSceneNode;
    }

    /**
     * Set render z order
     *
     * @param zOrder The value of z order
     */
    public void setRenderZOrder(int zOrder) {
        // Layers are rendered in increasing numerical order.  Negative zOrder
        // means z-order check is disabled, and non-z-order checked objects are
        // rendered first.  Z-orders are sorted in reverse numerical order.
        mRenderLayer = zOrder;

        if (mRenderLayer < 0) {
            mRenderLayer = MAX_RENDER_LAYER + 1;
        }

        mRenderLayer = MAX_RENDER_LAYER + 1 - mRenderLayer;

        boolean bit0 = (1 == mRenderLayer % 2);
        boolean bit1 = (1 == (mRenderLayer >> 1) % 2);

        mSceneNode.setFlags(
                mEngine.getRenderFlags().RENDER_LAYER_BIT_0, bit0);
        mSceneNode.setFlags(
                mEngine.getRenderFlags().RENDER_LAYER_BIT_1, bit1);
    }

    /**
     * Get render z order
     *
     * @return The value of z order
     */
    public int getRenderZOrder() {
        int zOrder = mRenderLayer;

        zOrder = MAX_RENDER_LAYER + 1 - zOrder;

        if (zOrder == MAX_RENDER_LAYER + 1) {
            zOrder = -1;
        }

        return zOrder;
    }

    /**
     * Get internal render layer.
     *
     * @return Render layer number
     */
    public int getRenderLayer() {
        return mRenderLayer;
    }

    /**
     * Set clip rect of node
     *
     * @param area the rectangle
     */
    public void setDisplayArea(Box area) {
        if (Solid.class.isInstance(mSceneNode)) {
            Solid solid = (Solid) mSceneNode;
            Appearance appearance = solid.getAppearance();

            if (area == null) {
                appearance.setScissorTestEnabled(false);
            } else {
                // The Box passed into the function is defined by the top-left
                // and bottom-right corners, using the top-left of the screen
                // as the origin.  The scissor rectangle function expects the
                // bottom-left corner, and the width and height, using the
                // bottom-left corner as the origin.
                appearance.setScissorTestEnabled(true);
                appearance.setScissorRectangle(
                        (int) area.x1,
                        (int) (mEngine.getHeight() - area.y2),
                        (int) area.x2,
                        (int) (area.y2 - area.y1));
            }
        }
    }

    /**
     * Request renderer render a frame.
     */
    public void requestRender() {
        mEngine.requestRender();
    }

    private void setMaterialRecursive(SceneNode sceneNode, String name) {
        // Scene node can be null if no material should be set
        if (sceneNode == null || sceneNode.getFlags(mEngine.getRenderFlags().DEBUG)) {
            return;
        }

        if (Solid.class.isInstance(sceneNode)) {
            Solid solid = (Solid) sceneNode;
            mEngine.getAssetPool().applyAppearance(solid.getAppearance(), name);
        }

        // Iteratively set material for all children
        int numChildren = sceneNode.getChildCount();
        for (int i = 0; i < numChildren; ++i) {
            SceneNode childSceneNode = sceneNode.getChild(i);
            setMaterialRecursive(childSceneNode, name);
        }
    }

    private void setMaterialPropertyRecursive(SceneNode sceneNode,
            String name, Object value) {

        // Scene node can be null if no property should be set
        if (sceneNode == null || sceneNode.getFlags(mEngine.getRenderFlags().DEBUG)) {
            return;
        }

        if (Solid.class.isInstance(sceneNode)) {
            Solid solid = (Solid) sceneNode;

            // This may be slightly slower than providing separate
            // functions for each data type, but it avoids code duplication
            // (Java generics are not as flexible as C++ templates).
            // Data types that are more likely to be used are checked first.
            if (Float.class.isInstance(value)) {
                solid.getAppearance().setFloat(name, (Float) value);

            } else if (Point.class.isInstance(value)) {
                Point point = (Point) value;
                solid.getAppearance().setVector3f(name,
                        point.x, point.y, point.z);

            } else if (Boolean.class.isInstance(value)) {
                solid.getAppearance().setBoolean(name, (Boolean) value);

            } else if (Integer.class.isInstance(value)) {
                solid.getAppearance().setInt(name, (Integer) value);

            } else if (Texture2D.class.isInstance(value)) {
                solid.getAppearance().setTexture2D(name, (Texture2D) value);

            } else {
                Log.e(TAG, "Unrecognised property type.");
            }
        }

        // Iteratively set material for all children
        int numChildren = sceneNode.getChildCount();
        for (int i = 0; i < numChildren; ++i) {
            SceneNode childSceneNode = sceneNode.getChild(i);
            setMaterialPropertyRecursive(childSceneNode, name, value);
        }
    }
}
