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

import android.graphics.Bitmap;

import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;

/**
 * Basic presentation.
 */
public interface Presentation {

    /**
     * Initialize a presentation before calling any other methods.
     * @param owner The owner
     */
    void initialize(Object owner);

    /**
     * Check if this presentation engine is initialized.
     * @return true if this presentation engine is initialized.
     */
    boolean isInitialized();

    /**
     * Gets the owner of this object.
     * @return the reference of owner.
     */
    Object getOwner();

    /**
     *  Sets the name of this objects
     * @param name a string to be set for name
     */
    void setName(String name);

     /**
     * Gets the name of this object.
     * @return  name of this object.
     */
    String getName();

    /**
     *  Sets the position of this object
     * @param pos  position setting using Point object
     */
    void setPosition(Point pos);

    /**
     * Gets the position with normalize argument. When normalize is true means the position is using absolute coordinates, false is for relational coordinate
     * @param normalized  boolean value for normalize setting
     * @return   position point value
     */
    Point getPosition(boolean normalized);

    /**
     * Sets the visible setting of this presentation engine.
     * @param visible the value of visible
     */
    void setVisible(boolean visible);

    /**
     *  Gets the visible status for this object
     * @return true if it is visible
     */
    boolean getVisible();

    /**
     *  Gets the truly visible status for this object
     * @return true if it is truly visible
     */
    boolean getTrulyVisible();

    /**
     * Sets the material for this presentation.
     *
     * @param name Name of the material to set
     */
    void setMaterial(String name);

    /**
     * Sets the material for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the material
     * @param name Name of the material to set
     */
    void setMaterial(String nodeName, String name);

    /**
     * Sets an integer material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String name, int value);

    /**
     * Sets an integer material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String nodeName, String name, int value);

    /**
     * Sets a floating point material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String name, float value);

    /**
     * Sets a floating point material property for a node within this
     * presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String nodeName, String name, float value);

    /**
     * Sets a boolean material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String name, boolean value);

    /**
     * Sets a boolean material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String nodeName, String name, boolean value);

    /**
     * Sets a point material property.
     *
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String name, Point value);

    /**
     * Sets a point material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param value Value to set
     */
    void setMaterialProperty(String nodeName, String name, Point value);

    /**
     * Sets a texture material property.
     *
     * @param name Name of the property to set
     * @param textureName Name of the texture to set
     */
    void setMaterialProperty(String name, String textureName);

    /**
     * Sets an integer material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param textureName Name of the texture to set
     */
    void setMaterialProperty(String nodeName, String name, String textureName);

    /**
     * Sets a texture material property.
     *
     * @param name Name of the property to set
     * @param bitmap object of the texture to set
     */
    void setMaterialProperty(String name, Bitmap bitmap);

    /**
     * Sets a texture material property for a node within this presentation.
     *
     * @param nodeName Name of the node for which to set the property
     * @param name Name of the property to set
     * @param bitmap object of the texture to set
     */
    void setMaterialProperty(String nodeName, String name, Bitmap bitmap);

    /**
     * Returns whether the actor is continuously changing.
     * This property can be inspected to determine whether the screen needs to
     * be updated continuously.
     *
     * @return True if the actor is dynamic
     */
    boolean isDynamic();

     /**
     *  Sets the rotation values for this object.
     * @param rotation   a rotation object for setting up the rotation value of this object.
     */
    void setRotation(Rotation rotation);

    /**
     * Gets the rotation values of this object.
     * @return  the rotation value.
     */
    Rotation getRotation();

    /**
     *  Sets the scale values for this object.
     * @param scale  a scale object for setting up the scale values of this object
     */
    void setScale(Scale scale);

     /**
     *  Gets the scale values of this object.
     * @return  scale value
     */
    Scale getScale();

    /**
     *  Sets the anchor point values for this object.
     * @param point   a point object to be used for setting up the anchor point of this object.
     */
    void setAnchorPoint(Point point);

    /**
     *  Gets the anchor point values of this object.
     * @return   anchor point value
     */
    Point getAnchorPoint();

    /**
     * Set the color tone of image. Will display solid color if image is not specified.
     * @param color color
     */
    void setColor(Color color);

     /**
     *  Gets the color value of this object
     * @return   color value
     */
    Color getColor();

    /**
     * Sets the alpha component of the diffuse color of this object.
     *
     * @see #setColor(Color)
     *
     * @param opacity Opacity value (ranges from 0 to 255).
     */
    void setOpacity(int opacity);

    /**
     * Returns the alpha component of the diffuse color of this object.
     *
     * @see #setColor(Color)
     *
     * @return Opacity value (ranges from 0 to 255).
     */
    int getOpacity();

    void setRenderZOrder(int zOrder);

    int getRenderZOrder();

    /**
     * Set the display area of this object.
     * @param area the area where object can be shown. If passing null value the display area will be cleared.
     */
    void setDisplayArea(Box area);

    /**
     * Performs a raycast hit test on the scene using a screen space coordinate.
     * The nearest presentation object intersected will be returned, along with
     * additional details about the raycast test.
     *
     * @param screenPoint Screen space point on the screen to pick
     * @param layer layer to which this object belongs
     * @return The result of the hit test
     */
    PresentationHitTestResult hitTest(Point screenPoint, RenderLayer layer);

    /**
     * Add a presentation as child of another in presentation tree. If the child is already added to
     * another parent, it will be removed from the old one first and then added to this presentation.
     *
     * @param child child presentation
     */
    void addChild(Presentation child);

    /**
     * Get child presentation at specified index.
     * @param index child index
     * @return child presentation
     */
    Presentation getChild(int index);

    /**
     * @return total count of child presentation
     */
    int getChildrenCount();

    /**
     * Creates a node presentation pointing to a given node.
     * @param nodeName Name of the node the node will reference
     * @return Node
     */
    IActorNodePresentation createActorNodePresentation(String nodeName);

    /**
     * Request renderer render a frame.
     */
    void requestRender();

    /**
     * Uninitialize the presentation. Should not call any methods after it is uninitialized.
     */
    void uninitialize();

}
