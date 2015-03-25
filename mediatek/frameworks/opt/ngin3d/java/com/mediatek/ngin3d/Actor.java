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

import android.util.Log;
import android.graphics.Bitmap;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.AnimationGroup;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.ngin3d.presentation.IActorNodePresentation;
import com.mediatek.ngin3d.presentation.Presentation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.presentation.PresentationHitTestResult;
import com.mediatek.ngin3d.presentation.RenderLayer;
import com.mediatek.ngin3d.utils.Ngin3dException;
import com.mediatek.ngin3d.utils.JSON;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

/**
 * Base abstract class for all graphics scene objects.
 * MAGE uses a concept of a Stage (a visual scene with one or more cameras etc.)
 * and Actors which are the entities that may be put on the stage.  All
 * graphical objects are Actors including obvious things like 3D objects and
 * lights, and also convenience classes such as simple geometry (cubes, planes),
 * Image bitmaps and text.
 *
 */
public class Actor extends Base {
    /**
     * @hide
     */
    protected static final String TAG = "Ngin3d";

    private static int sSerial;

    /**
     * Unique id of this actor
     *
     * @hide
     */
    protected int mId;

    /**
     * Tag of this actor
     *
     * @hide
     */
    protected int mTag;

    /**
     * Reactive status of this actor
     *
     * @hide
     */
    protected boolean mReactive;

    /**
     * Owner of this actor
     *
     * @hide
     */
    protected Object mOwner;

    /**
     * Layer in which this actor is located
     *
     * @hide
     */
    private Layer mLayer;

    /**
     * Presentation of this actor
     *
     * @hide
     */
    protected Presentation mPresentation;

    /**
     * Nodes of this actor
     *
     * @hide
     */
    private final Map<String, ActorNode> mNodes = new HashMap<String, ActorNode>();

    /**
     * Store properties that was locked, any modification of these properties will cause exception.
     *
     * @hide
     */
    protected ArrayList<Property> mLockedProperties;

    /**
     * Initialize this actor
     */
    protected Actor() {
        mId = sSerial++;
        mReactive = true;
    }

    /**
     * Get the actor ID.
     *
     * @return Actor ID
     */
    public int getId() {
        return mId;
    }

    /**
     * Set the actor tag.
     *
     * @param tag actor tag
     * @hide Internal use by animation sub-package
     */
    public void setTag(int tag) {
        mTag = tag;
    }

    /**
     * Get the actor tag.
     *
     * @return actor tag
     * @hide Internal use by animation sub-package
     */
    public int getTag() {
        return mTag;
    }

    /**
     * Set the actor owner.
     *
     * @param owner actor owner
     * @hide Dangerous to have external user set this!!
     */
    public void setOwner(Object owner) {
        mOwner = owner;
    }

    /**
     * Get the actor owner - the container.
     *
     * @return A generic object that is the 'owner' of this Actor - cast to Container
     */
    public Object getOwner() {
        return mOwner;
    }

    /**
     * Notify the actor of the layer in which it resides.
     *
     * @param layer Layer
     */
    protected void notifyOfLayer(Layer layer) {
        mLayer = layer;
    }

    /**
     * Returns the render layer in which this actor exists.
     *
     * @return actor layer
     */
    public Layer getLayer() {
        return mLayer;
    }

    /**
     * @hide
     */
    protected void applyBatchValues() {
        // Do nothing by default
    }

    /**
     * @hide Presentation should be internal interface
     */
    protected Presentation createPresentation(PresentationEngine engine) {
        throw new Ngin3dException("Should be overrided to create Presentation object");
    }

    /**
     * @hide
     */
    @Override
    protected void setPropertyChain(PropertyChain chain) {
        super.setPropertyChain(chain);
    }

    /**
     * Get the Presentation of this actor
     *
     * @return presentation
     * @hide Presentation should be internal interface
     */
    public Presentation getPresentation() {
        return mPresentation;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Property handling

    /**
     *
     */
    protected static final int MAX_OPACITY = 255;

    /**
     * Name property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<String> PROP_NAME =
        new Property<String>("name", "noname");

    /**
     * Rotation property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Rotation> PROP_ROTATION =
        new Property<Rotation>("rotation", new ImmutableRotation(0.f, 0.f, 0.f),
            Property.FLAG_ANIMATABLE);

    /**
     * Scale property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Scale> PROP_SCALE =
        new Property<Scale>("scale", new ImmutableScale(1, 1, 1),
            Property.FLAG_ANIMATABLE);

    /**
     * Visible property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Boolean> PROP_VISIBLE =
        new Property<Boolean>("visible", true);

    /**
     * Position property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Point> PROP_POSITION =
        new Property<Point>("position", new ImmutablePoint(0.f, 0.f, 0.f),
            Property.FLAG_ANIMATABLE);

    /**
     * Color property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Color> PROP_COLOR =
        new Property<Color>("color", null);

    /**
     * Opacity property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Integer> PROP_OPACITY =
        new Property<Integer>("opacity", MAX_OPACITY);

    /**
     * The display area property of this actor
     *
     * @hide Property values should be internal
     */
    public static final Property<Box> PROP_DISPLAY_AREA =
        new Property<Box>("display_area", null);

    /**
     * Material property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL = "material";

    /**
     * Integer property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_INT =
        "material_property_int";

    /**
     * Floating point property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_FLOAT =
        "material_property_float";

    /**
     * Boolean property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_BOOLEAN =
        "material_property_boolean";

    /**
     * Point property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_POINT =
        "material_property_point";

    /**
     * Integer property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_TEXTURE =
        "material_property_texture";

    /**
     * Bitmap property name
     *
     * @hide Property values should be internal
     */
    public static final String PROPNAME_MATERIAL_PROPERTY_BITMAP =
        "material_property_bitmap";

    /**
     * Apply the value of specified property to presentation tree.
     *
     * @param property property object
     * @param value    property value
     * @return true if the properties are applied successfully
     */
    protected boolean applyValue(Property property, Object value) {
        if (property instanceof KeyPathProperty) {
            KeyPathProperty kp = (KeyPathProperty) property;
            String propertyName = kp.getKey(0);
            String nodeName = "";

            // In the case where the property should be applied to the entire
            // actor, the node name will be empty, and so there will be no
            // entry in the key path (the key path string will be of the format
            // "PROPERTY." or "PROPERTY.." as opposed to "PROPERTY.nodeName" or
            // "PROPERTY.nodeName.materialProperty").
            if (kp.getKeyPathLength() >= 2) {
                nodeName = kp.getKey(1);
            }

            if (propertyName.equals(PROPNAME_MATERIAL)) {
                if (value != null) {
                    String materialName = (String) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterial(materialName);
                    } else {
                        mPresentation.setMaterial(nodeName, materialName);
                    }
                }
                return true;
            }

            if (kp.getKeyPathLength() < 3) {
                return false;
            }

            String matPropName = kp.getKey(2);

            if (propertyName.equals(PROPNAME_MATERIAL_PROPERTY_INT)) {
                if (value != null) {
                    Integer matPropValue = (Integer) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;

            } else if (propertyName.equals(PROPNAME_MATERIAL_PROPERTY_FLOAT)) {
                if (value != null) {
                    Float matPropValue = (Float) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;

            } else if (propertyName.equals(
                PROPNAME_MATERIAL_PROPERTY_BOOLEAN)) {
                if (value != null) {
                    Boolean matPropValue = (Boolean) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;

            } else if (propertyName.equals(PROPNAME_MATERIAL_PROPERTY_POINT)) {
                if (value != null) {
                    Point matPropValue = (Point) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;

            } else if (propertyName.equals(
                PROPNAME_MATERIAL_PROPERTY_TEXTURE)) {
                if (value != null) {
                    String matPropValue = (String) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;

            } else if (propertyName.equals(
                PROPNAME_MATERIAL_PROPERTY_BITMAP)) {
                if (value != null) {
                    Bitmap matPropValue = (Bitmap) value;
                    if (nodeName.isEmpty()) {
                        mPresentation.setMaterialProperty(
                            matPropName, matPropValue);
                    } else {
                        mPresentation.setMaterialProperty(
                            nodeName, matPropName, matPropValue);
                    }
                }
                return true;
            }

        } else if (property.sameInstance(PROP_POSITION)) {
            Point pos = (Point) value;
            mPresentation.setPosition(pos);
            return true;
        } else if (property.sameInstance(PROP_ROTATION)) {
            Rotation rotation = (Rotation) value;
            mPresentation.setRotation(rotation);
            return true;
        } else if (property.sameInstance(PROP_SCALE)) {
            Scale scale = (Scale) value;
            mPresentation.setScale(scale);
            return true;
        } else if (property.sameInstance(PROP_VISIBLE)) {
            mPresentation.setVisible(asBoolean(value));
            return true;
        } else if (property.sameInstance(PROP_NAME)) {
            String name = (String) value;
            mPresentation.setName(name);
            return true;
        } else if (property.sameInstance(PROP_COLOR)) {
            if (value != null) {
                Color color = (Color) value;
                mPresentation.setColor(color);
            }
            return true;
        } else if (property.sameInstance(PROP_OPACITY)) {
            Integer opacity = (Integer) value;
            mPresentation.setOpacity(opacity);
            return true;
        } else if (property.sameInstance(PROP_DISPLAY_AREA)) {
            Box area = (Box) value;
            mPresentation.setDisplayArea(area);
            return true;
        }

        return false;
    }

    /**
     * Updates actor properties to match the internal state of the actor.
     */
    protected void refreshState() {
        // No properties are currently set, but function may be overridden by
        // subclasses.
    }

    /**
     * Flag for {@link #getPresentationValue(Property)}: returned value should be normalized
     */
    protected static final int FLAG_NORMALIZED = 0x01;

    /**
     * For the same property, there are two values: the logic one and the visual one. The visual
     * value can be retrieved from presentation tree using this method.
     *
     * @param property property object, such as Actor.PROP_POSITION, to retrieve
     * @return visual value. Will return null if the value does not exist.
     * @hide Presentation interface and internal Property's should be hidden from external user
     */
    public Object getPresentationValue(Property property) {
        return getPresentationValue(property, 0);
    }

    /**
     * The same as the one with single property parameter except that you can specify an additional format modifier.
     *
     * @param property property object
     * @param flags    modifiers for value format, e.g. {@link #FLAG_NORMALIZED}
     * @return visual value. Will return null if the value does not exist.
     * @hide Presentation interface and internal Property's should be hidden from external user
     */
    public Object getPresentationValue(Property property, int flags) {
        if (mPresentation == null) {
            return null;
        }

        if (property.sameInstance(PROP_POSITION)) {
            return mPresentation.getPosition((flags & FLAG_NORMALIZED) != 0);
        } else if (property.sameInstance(PROP_ROTATION)) {
            return mPresentation.getRotation();
        } else if (property.sameInstance(PROP_SCALE)) {
            return mPresentation.getScale();
        } else if (property.sameInstance(PROP_VISIBLE)) {
            return mPresentation.getVisible();
        } else {
            Log.w(TAG, "Unknown property name: " + property.getName());
        }

        return null;
    }

    private void checkPropertyLocked(Property property) {
        if (isPropertyLocked(property)) {
            throw new Ngin3dException(this + ": Property" + property + "is locked, can not be modified");
        }
    }

    /**
     * Add the new value of property into the active transaction.
     *
     * @param property the property in the transaction to be set.
     * @param newValue the value to be set to the property in the transaction.
     * @return true if the property is set successfully
     * @hide Property values should be internal
     */
    protected final <T> boolean setValueInTransaction(Property<T> property, T newValue) {
        // Check the property is locked or not, if true it will throw an exception.
        checkPropertyLocked(property);

        // Add value modification to active transaction.
        Transaction transaction = Transaction.getActive();
        if (transaction == null) {
            // Set the logic value
            return setValue(property, newValue);
        } else {
            setValue(property, newValue, false);
            transaction.addPropertyModification(this, property, newValue);
            return true;
        }
    }

    /**
     * Set the new property value for the actor object.
     *
     * @param property the property to be set
     * @param newValue the new value to be set to the property.
     * @return true if the property is set successfully
     * @hide Property values should be internal
     */
    @Override
    public final <T> boolean setValue(Property<T> property, T newValue, boolean dirty) {
        // Check the property is locked or not, if true it will throw an exception.
        checkPropertyLocked(property);

        if (super.setValue(property, newValue, dirty)) {
            requestRender();
            return true;
        }
        return false;
    }

    /**
     * Set the new property value for the actor object.
     *
     * @param property the property to be set
     * @param newValue the new value to be set to the property.
     * @return true if the property is set successfully
     * @hide Property values should be internal
     */
    @Override
    public final <T> boolean setValue(Property<T> property, T newValue) {
        return setValue(property, newValue, true);
    }

    /**
     * Override because setting a key path value (dynamic property) will
     * usually affect some visual aspect of the Actor, and so should cause the
     * scene to re-render.
     *
     * @hide
     */
    @Override
    public <T> boolean setKeyPathValue(String keyPath, Object value) {
        if (super.setKeyPathValue(keyPath, value)) {
            requestRender();
            return true;
        }
        return false;
    }

    /**
     * Renew this actor using new property value.
     *
     * @hide
     */
    public void requestRender() {
        if (mPresentation != null) {
            mPresentation.requestRender();
        }
    }

    /**
     * Return the value is dirty.
     *
     * @return true is the dirty value is set
     * @hide Internal state handling
     */
    public boolean isDirty() {
        return dirtyValueExists()
            || (mPresentation != null && mPresentation.isDynamic())
            || dirtyActorNodeExists();
    }

    /**
     * Returns whether any of the nodes are dirty.
     *
     * @return True if at least one node is dirty
     * @hide Internal state handling
     */
    private boolean dirtyActorNodeExists() {
        for (ActorNode node : mNodes.values()) {
            if (node.isDirty()) {
                return true;
            }
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Property accessors

    /**
     * Set the name of this actor.
     * The 'name' is an arbitrary text string associated with the Actor.
     *
     * @param name New name for the actor
     */
    public void setName(String name) {
        setValue(PROP_NAME, name);
    }

    /**
     * Get the name of this actor
     *
     * @return The name of actor
     */
    public CharSequence getName() {
        return getValue(PROP_NAME);
    }

    /**
     * Set the position of the Actor.
     * The origin of the Actor (as defined by the artist) will be positioned at
     * this point in space.
     *
     * @param pos The new position as a Point
     */
    public void setPosition(Point pos) {
        setValueInTransaction(PROP_POSITION, pos);
    }

    /**
     * Get the position of the actor.
     *
     * @return Actor position
     */
    public Point getPosition() {
        Point point = getValue(PROP_POSITION);
        // If it's immutable point, create a normal point to replace it
        if (point instanceof ImmutablePoint) {
            point = new Point(point.x, point.y, point.z, point.isNormalized);
            setPosition(point);
        }
        return point;
    }

    /**
     * Set the visibility flag for this actor.
     * Whether an Actor is visible depends on this flag <b>and</b> on
     * whether the container it is in is visible, etc.  Therefore, setting
     * this false will make the Actor invisible, but setting it true may
     * not necessarily make the Actor visible.
     *
     * @param visible New setting of visibility flag, false = invisible
     */
    public void setVisible(boolean visible) {
        setValueInTransaction(PROP_VISIBLE, visible);
    }

    /**
     * Gets the Actor's visibility flag state.
     *
     * @return Current setting of visibility flag, false = invisible
     */
    public boolean getVisible() {
        return getValue(PROP_VISIBLE);
    }

    /**
     * Gets the the Actor's true visibility.
     * The Actor and all the nested containers it is in are checked for visibility.
     * If the result of this is that the Actor is visible this method returns true.
     * If any flag in the set is false (invisible) the Actor will not be visible
     * and the method returns false.
     *
     * @return Whether the Actor is truly visible
     */
    public boolean getTrulyVisible() {
        if (mPresentation == null) {
            return false;
        }
        return mPresentation.getTrulyVisible();
    }

    /**
     * Sets the material for this Actor.
     * Materials are defined by material (.mat) files, which can either be
     * provided by the user, or built into ngin3d.  Built-in material files are
     * prefixed with the "ngin3d" namespace (e.g. "ngin3d#example.mat").
     *
     * @param name Name of the new material to set
     */
    public void setMaterial(String name) {
        setMaterial("", name);
    }

    /**
     * Sets the material for a node within this Actor.
     * Actors may be constructed from a hierachy of components, especially if the
     * Actor is a 3D object created by 3ds Max or Blender. This method allows
     * a material to be assigned to a specific node in that hierachy. You will need
     * to know the name of the node; ask the artist.
     *
     * @param nodeName Name of the node for which to set the material
     * @param name     Name of the material to set
     */
    public void setMaterial(String nodeName, String name) {
        setKeyPathValue(PROPNAME_MATERIAL + "." + nodeName, name);
    }

    /**
     * Sets an name / integer-value pair material property.
     *
     * @param name  Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, int value) {
        setMaterialProperty("", name, value);
    }

    /**
     * Sets an integer material property for a node within this Actor.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name     Name of the property to set
     * @param value    Value to set
     */
    public void setMaterialProperty(String nodeName, String name, int value) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_INT + "." + nodeName + "."
            + name, value);
    }

    /**
     * Sets a floating point material property.
     * Example robot.setMaterialProperty("ASPECT_RATIO", (float) WIDTH / HEIGHT);
     *
     * @param name  Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, float value) {
        setMaterialProperty("", name, value);
    }

    /**
     * Sets a floating point material property for a node within this
     * presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name     Name of the property to set
     * @param value    Value to set
     */
    public void setMaterialProperty(String nodeName, String name, float value) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_FLOAT + "." + nodeName + "."
            + name, value);
    }

    /**
     * Sets a boolean material property.
     *
     * @param name  Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, boolean value) {
        setMaterialProperty("", name, value);
    }

    /**
     * Sets a boolean material property for a node within this Actor.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name     Name of the property to set
     * @param value    Value to set
     */
    public void setMaterialProperty(String nodeName, String name,
                                    boolean value) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_BOOLEAN + "." + nodeName
            + "." + name, value);
    }

    /**
     * Sets a point material property.
     *
     * @param name  Name of the property to set
     * @param value Value to set
     */
    public void setMaterialProperty(String name, Point value) {
        setMaterialProperty("", name, value);
    }

    /**
     * Sets a point material property for a node within this Actor.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name     Name of the property to set
     * @param value    Value to set
     */
    public void setMaterialProperty(String nodeName, String name, Point value) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_POINT + "." + nodeName + "."
            + name, value);
    }

    /**
     * Sets a texture material property.
     * Example robot.setMaterialProperty("TEXTURE", "onions.jpg");
     *
     * @param name        Name of the property to set
     * @param textureName Name of the texture to set
     */
    public void setMaterialProperty(String name, String textureName) {
        setMaterialProperty("", name, textureName);
    }

    /**
     * Sets an integer material property for a node within this Actor.
     *
     * @param nodeName    Name of the node for which to set the property
     * @param name        Name of the property to set
     * @param textureName Name of the texture to set
     */
    public void setMaterialProperty(String nodeName, String name,
                                    String textureName) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_TEXTURE + "." + nodeName
            + "." + name, textureName);
    }

    /**
     * Sets a texture property for a node within this Actor, using a bitmap.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name     Name of the property to set
     * @param bitmap   Object of the bitmap texture to set
     */
    public void setMaterialProperty(String nodeName, String name,
                                    Bitmap bitmap) {
        setKeyPathValue(PROPNAME_MATERIAL_PROPERTY_BITMAP + "." + nodeName
            + "." + name, bitmap);
    }

    /**
     * Set the Actor's absolute rotation about its origin.
     *
     * @param rotation New Rotation value
     */
    public void setRotation(Rotation rotation) {
        setValueInTransaction(PROP_ROTATION, rotation);
    }

    /**
     * Get the Actor's current rotation value.
     *
     * @return Current rotation
     */
    public Rotation getRotation() {
        return getValue(PROP_ROTATION);
    }

    /**
     * Set the Actor's scale factor.
     * The scaling is done relative to the Actor's origin.
     *
     * @param scale New Scale value to be applied
     */
    public void setScale(Scale scale) {
        setValueInTransaction(PROP_SCALE, scale);
    }

    /**
     * Get the current scale of the actor
     *
     * @return Current scale factor
     */
    public Scale getScale() {
        return getValue(PROP_SCALE);
    }

    /**
     * Sets a flag indicating that the Actor is 'reactive' = dynamic.
     * Dynamic Actors cause the whole scene to be re-rendered continuously
     * because it is assumed that this Actor's state will have changed and
     * have affected the scene.
     *
     * @param reactive True if the object is reactive
     */
    public void setReactive(boolean reactive) {
        mReactive = reactive;
    }

    /**
     * Gets the Actors current 'reactive' flag
     *
     * @return True if the object is marked as reactive
     */
    public boolean getReactive() {
        return mReactive;
    }

    /**
     * This method is used for debug only.
     * Make properties in locked status. The locked properties can't be modified or it will cause exception.
     *
     * @param properties the properties that will be locked.
     * @hide Debug only
     */
    public void lockProperty(Property... properties) {
        getLockProperties().addAll(Arrays.asList(properties));
    }

    /**
     * This method is used for debug only.
     * Make properties in unlocked status. The unlocked properties can't be modified or it will cause exception.
     *
     * @param properties the properties that will be unlocked.
     * @hide Debug only
     */
    public void unlockProperty(Property... properties) {
        getLockProperties().removeAll(Arrays.asList(properties));
    }

    /**
     * This method is used for debug only.
     * Get the lock status of specific property.
     *
     * @return the property is locked or not
     * @hide Debug only
     */
    public boolean isPropertyLocked(Property property) {
        return mLockedProperties != null && mLockedProperties.contains(property);
    }

    private ArrayList<Property> getLockProperties() {
        if (mLockedProperties == null) {
            mLockedProperties = new ArrayList<Property>();
        }
        return mLockedProperties;
    }

    /**
     * Set the color of the actor.
     * Setting a color of a Container will recursively set the color of its
     * children.  The color of an Actor will remain the same when moving it
     * between Containers (the color of the container only takes effect at the
     * moment it is set).
     *
     * @param color New color value to be used for actor
     */
    public void setColor(Color color) {
        setValueInTransaction(PROP_COLOR, color);
    }

    /**
     * Get the color property of this actor
     *
     * @return Current color; White if none has yet been set
     */
    public Color getColor() {
        Color value = getValue(PROP_COLOR);

        if (value == null) {
            value = Color.WHITE;
        }

        return value;
    }

    /**
     * Sets the opacity of this actor (0 transparent - 255 opaque).
     * The rendered opacity of an actor is equal to the product of its opacity
     * and all of its parents' opacities.  In other words, opacity accumulates
     * multiplicatively down the Actor hierarchy.  Unlike with color, setting
     * the opacity of a Container does not set the opacity for its children.
     * <p>
     * Reducing opacity from the maximum 255 makes an object slightly transparent.
     * Beware that using transparency substantially increases the load on the GPU
     * and will have an effect on the frame rate of the application.
     *
     * @param opacity Opacity to set
     */
    public void setOpacity(int opacity) {
        if (opacity < 0 || opacity > MAX_OPACITY) {
            throw new IllegalArgumentException("Invalid opacity value: " + opacity);
        }
        setValue(PROP_OPACITY, opacity);
    }

    /**
     * Returns the opacity of this actor.
     *
     * @return Opacity (0-255) of Actor
     */
    public int getOpacity() {
        return getValue(PROP_OPACITY);
    }

    /**
     * Set the clipping rectangle of the Actor.
     * The rectangle is specified in screen pixels, using the coordinates of
     * top-left and bottom-right (origin of the screen is at the top left).
     * Beware that this is the <b>inverse of glScissor</b>, the openGL equivalent
     * method.
     *
     * @param area New clipping rectangle
     */
    public void setDisplayArea(Box area) {
        setValueInTransaction(PROP_DISPLAY_AREA, area);
    }

    /**
     * Get the clipping rectangle of the Actor.
     *
     * @return Current clipping rectangle
     */
    public Box getDisplayArea() {
        return getValue(PROP_DISPLAY_AREA);
    }

    ///////////////////////////////////////////////////////////////////////////
    // public methods

    /**
     * Realize this actor.
     *
     * @param presentationEngine an initialized PresentationEngine to be used for realizing actor
     * @hide Presentation should be an internal interface
     */
    public void realize(PresentationEngine presentationEngine) {
        if (mPresentation == null) {
            mPresentation = createPresentation(presentationEngine);
            mPresentation.initialize(this);

            // Whether dirty or not, force to apply all KeyPath Properties
            applyAllKeyPathProperties();

            // There are dependencies between static properties, ex: PROP_SRC_RECT depends on PROP_IMG_SRC
            // In the end we have to use applyAllProperties() to apply all properties with dependencies
            // which applyAllExistingValues doesn't consider it.
            applyAllProperties();

            // continue because there maybe some attached properties...
        }

        // Apply all properties marked as dirty, and then update the properties
        // to match the current actor state.
        applyAllDirtyValues();
        refreshState();

        // Realize nodes.
        for (ActorNode node : mNodes.values()) {
            node.realize(mPresentation);
        }
    }

    /**
     * Check if this actor is realized.
     *
     * @hide Internal knowledge
     * @return True if the actor is realized
     */
    public boolean isRealized() {
        return (mPresentation != null);
    }

    /**
     * Un-realize this actor.
     *
     * @hide Internal operation
     */
    public void unrealize() {
        finishAnimations();

        // Un-realize nodes.
        for (ActorNode node : mNodes.values()) {
            node.unrealize();
        }

        if (mPresentation != null) {
            mPresentation.uninitialize();
            mPresentation = null;
        }
    }

    /**
     * Performs a raycast hit test on the tree under this actor, returning only
     * the Actor which was hit by the test, if any.
     *
     * @param screenPoint Point (in pixels) on the screen to test
     * @return The actor that is at the point given, or null otherwise
     */
    public Actor hitTest(Point screenPoint) {
        HitTestResult result = hitTestFull(screenPoint);
        return result.getActor();
    }

    /**
     * Performs a hit test on the tree under this actor, returning full details
     * about any hit that may occur.
     *
     * @param screenPoint Point (in pixels) on the screen to test
     * @return Details about the hit test and its result
     */
    public HitTestResult hitTestFull(Point screenPoint) {
        HitTestResult result = new HitTestResult();

        if (mPresentation != null) {
            Layer layer = getLayer();
            RenderLayer renderLayer = (layer == null) ? null : layer.getRenderLayer();

            PresentationHitTestResult hit = mPresentation.hitTest(screenPoint, renderLayer);

            // Check if a hit occurred.
            Presentation actorPresentation = hit.getActorPresentation();
            if (actorPresentation != null) {
                Actor actor = (Actor) actorPresentation.getOwner();

                while (actor != null && !actor.getReactive() && actor != this) {
                    actor = (Actor) actor.getOwner();
                }

                result.setActor(actor);

                // Check if the hit occurred with a node.
                IActorNodePresentation nodePresentation = hit.getActorNodePresentation();
                if (nodePresentation != null) {
                    result.setNode((ActorNode) nodePresentation.getOwner());
                }
            }

            result.setRay(
                hit.getRayStart(),
                hit.getRayDirection(),
                null,
                hit.getRayHit(),
                hit.getRayHitNormal());
        }

        return result;
    }

    /**
     * Get the root ActorNode for this Actor.
     *
     * @return Root ActorNode
     */
    public ActorNode getNode() {
        return getNode(null);
    }

    /**
     * Get a specific named ActorNode within this Actor.
     * If the specified node does not exist, it is created.
     *
     * @param nodeName Name of the node to reference
     * @return The corresponding ActorNode
     */
    public ActorNode getNode(String nodeName) {
        // Nodes are cached in a map so that node references remain unique.
        ActorNode node = mNodes.get(nodeName);

        if (node == null) {
            node = new ActorNode(nodeName);
            mNodes.put(nodeName, node);
        }

        return node;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Animations

    private final Map<String, Animation> mAnimationMap = new HashMap<String, Animation>();

    /**
     * Register an ngin3D-animation for specified key.
     *
     * @param key       Typically property name.
     * @param animation the animation
     * @hide Internal use by animation package
     */
    @SuppressWarnings("PMD")
    public void onAnimationStarted(String key, Animation animation) {
        synchronized (mAnimationMap) {
            Animation existing = mAnimationMap.get(key);
            if (existing != animation) {
                if (existing != null) {
                    existing.stop();
                }
                mAnimationMap.put(key, animation);
            }
        }
    }

    /**
     * Register an ngin3D-animation for specified key.
     *
     * @param key       Typically property name.
     * @hide Internal use by animation package
     */
    public void onAnimationStopped(String key) {
        if (mAnimationMap != null) {
            synchronized (mAnimationMap) {
                mAnimationMap.remove(key);
            }
        }
    }

    /**
     * Stops all ngin3D-animations that are currently started on this actor.
     */
    public void stopAnimations() {
        if (mAnimationMap != null) {
            synchronized (mAnimationMap) {
                ArrayList<Animation> animations = new ArrayList<Animation>(mAnimationMap.values());
                for (Animation animation : animations) {
                    animation.stop();
                }
            }
        }
    }

    /**
     * Finishes all ngin3D-animations that are currently started on this actor.
     * @hide Internal use by animation package
     */
    public void finishAnimations() {
        if (mAnimationMap != null) {
            synchronized (mAnimationMap) {
                ArrayList<Animation> animations = new ArrayList<Animation>(mAnimationMap.values());
                for (Animation animation : animations) {
                    animation.complete();
                }
            }
        }
    }

    /**
     * Checks if the ngin3D-animation in this actor is started.
     *
     * @return true if the animation in this actor is started.
     */
    public boolean isAnimationStarted() {
        if (mAnimationMap != null) {
            synchronized (mAnimationMap) {
                return !mAnimationMap.isEmpty();
            }
        }
        return false;
    }

    /**
     * Animate changes to one or more actors. Note that the default animation
     * duration <code>BasicAnimation.DEFAULT_DURATION</code> will be used.
     *
     * @param animations a runnable to change actor properties
     * @hide Access method needed by internal .animation package
     */
    public static void animate(Runnable animations) {
        animate(animations, null);
    }

    /**
     * Animate changes to one or more actors with specified completion handler.
     *
     * @param animations a runnable to change actor properties
     * @param completion completion a runnable that will be executed when the animation sequence ends.
     * @hide Access method needed by internal .animation package
     */
    public static void animate(Runnable animations, Runnable completion) {
        animate(BasicAnimation.DEFAULT_DURATION, animations, completion);
    }

    /**
     * Animate changes to one or more actors with specified duration and completion handler.
     *
     * @param duration   duration of animations in milliseconds
     * @param animations a runnable to change actor properties
     * @param completion a runnable that will be executed when the animation sequence ends.
     * @hide Access method needed by internal .animation package
     */
    public static ImplicitAnimation animate(int duration, Runnable animations, Runnable completion) {
        ImplicitAnimation animation;

        try {
            animation = Transaction.beginImplicitAnimation();

            Transaction.setAnimationDuration(duration);
            Transaction.setCompletion(completion);

            animations.run();
        } finally {
            Transaction.commit();
        }

        return animation;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Debugging

    /**
     * Dump all of the property of this actor to log.
     * @hide Internal use
     */
    public String dump() {
        return dumpProperties(!mAnimationMap.isEmpty());
    }

    /**
     * Dump all of the property of the animation in this actor to log.
     * @hide Internal use
     */
    public String dumpAnimation() {
        StringBuilder builder = new StringBuilder();
        int index = 0;
        if (!mAnimationMap.isEmpty()) {
            ArrayList<Animation> animations = new ArrayList<Animation>(mAnimationMap.values());
            for (Animation animation : animations) {
                builder.append(animation.getClass().getSimpleName() + index + ":");
                String temp = "";
                if (animation instanceof AnimationGroup) {
                    temp = temp + wrapAnimationGroup(animation);
                } else {
                    temp = temp + wrapSingleAnimation(animation);
                    temp = temp.substring(0, temp.length() - 1);
                    temp = JSON.wrap(temp);
                }
                builder.append(temp);
                builder.append(",");
                index++;
            }
        }

        if (builder.length() > 0) {
            // To compatible with JSON format
            builder.deleteCharAt(builder.length() - 1);
            return JSON.wrap(builder.toString());
        }
        return null;
    }

    private String wrapAnimationGroup(Animation animation) {
        StringBuilder builder = new StringBuilder();
        Log.w(TAG, "wrapAnimationGroup: " + ((AnimationGroup) animation).getAnimationCount());

        for (int index = 0; index < ((AnimationGroup) animation).getAnimationCount(); index++) {
            Animation ani = ((AnimationGroup) animation).getAnimation(index);

            builder.append(ani.getClass().getSimpleName() + index + ":");
            Log.w(TAG, "wrapAnimationGroup  -- string: " + builder.toString());

            String temp = "";
            if (ani instanceof AnimationGroup) {
                temp = temp + wrapAnimationGroup(ani);
            } else {
                temp = temp + wrapSingleAnimation(animation);
                temp = temp.substring(0, temp.length() - 1);
                temp = JSON.wrap(temp);
            }
            builder.append(temp);
            builder.append(",");
        }

        if (builder.length() > 0) {
            // To compatible with JSON format
            builder.deleteCharAt(builder.length() - 1);
            return JSON.wrap(builder.toString());
        }
        return null;
    }

    private String wrapSingleAnimation(Animation animation) {

        String property = "";

        if (animation instanceof PropertyAnimation) {
            Log.w(TAG, "wrapSingleAnimation  -- getPropertyName: " + ((PropertyAnimation) animation).getPropertyName());
            property = property + ((PropertyAnimation) animation).getPropertyName();
        }

        return (wrapProperty("Property", property)
            + wrapProperty("AutoReverse", Boolean.toString(((BasicAnimation) animation).getAutoReverse()))
            + wrapProperty("Loop", Boolean.toString(((BasicAnimation) animation).getLoop()))
            + wrapProperty("Duration", Integer.toString(((BasicAnimation) animation).getDuration()))
            + wrapProperty("Direction", Integer.toString(((BasicAnimation) animation).getDirection()))
            + wrapProperty("TimeScale", Float.toString(((BasicAnimation) animation).getTimeScale())));
    }

    private String wrapProperty(String name, String value) {
        return name + ":" + value + ",";
    }

    /**
     * Get a text description for the Actor.
     *
     * @return Descriptive string
     */
    @Override
    public String toString() {
        return "Actor{" + "mId=" + mId + ", mTag=" + mTag + ", mReactive=" + mReactive
            + ", mOwner=" + mOwner + ", mPresentation=" + mPresentation + '}';
    }
}
