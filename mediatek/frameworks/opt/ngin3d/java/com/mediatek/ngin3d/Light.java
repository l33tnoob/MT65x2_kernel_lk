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

package com.mediatek.ngin3d;

import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.presentation.ILightPresentation;

/**
 * Light source for 3D scenes.
 * <p>
 * Reminder: These lights only affect 3D models of type Glo3D. The in-built
 * graphics primitives of Plane, Cube, Sphere, etc. are 'self lit' and not
 * affected by these lights.
 */
public class Light extends Actor {
    /**
     * @hide
     */
    protected static final String TAG = "Light";

    /**
     * @hide
     */
    protected ILightPresentation createPresentation(PresentationEngine engine) {
        return engine.createLight();
    }

    /**
     * Returns the Actor's presentation cast to the instantiated type.
     *
     * @hide Presentation API should be internal only
     *
     * @return Presentation object
     */
    @Override
    public ILightPresentation getPresentation() {
        return (ILightPresentation) mPresentation;
    }

    /**
     * Light type property
     * @hide
     */
    public static final Property<Integer> PROP_TYPE =
        new Property<Integer>("light_type", null);

    /**
     * Diffuse color property
     * @hide
     */
    public static final Property<Color> PROP_COLOR =
        new Property<Color>("color", null);

    /**
     * Ambient level property
     * @hide
     */
    public static final Property<Float> PROP_AMBIENT_LEVEL =
        new Property<Float>("ambient_level", null);

    /**
     * Intensity property
     * @hide
     */
    public static final Property<Float> PROP_INTENSITY =
        new Property<Float>("intensity", null);

    /**
     * Near attenuation property
     * @hide
     */
    public static final Property<Float> PROP_ATTN_NEAR =
        new Property<Float>("attn_near", null);

    /**
     * Far attenuation property
     * @hide
     */
    public static final Property<Float> PROP_ATTN_FAR =
        new Property<Float>("attn_far", null);

    /**
     * Spot inner angle property
     * @hide
     */
    public static final Property<Float> PROP_SPOT_INNER =
        new Property<Float>("spot_inner", null);

    /**
     * Spot outer angle property
     * @hide
     */
    public static final Property<Float> PROP_SPOT_OUTER =
        new Property<Float>("spot_outer", null);

    /**
     * Is attenuated property
     * @hide
     */
    public static final Property<Boolean> PROP_IS_ATTN =
        new Property<Boolean>("spot_is_attn", null);

    /**
     * Applies a property to the node.
     *
     * @param property Property to apply
     * @param value Value to apply
     * @return True if the property was applied successfully
     * @hide
     */
    protected boolean applyValue(Property property, Object value) {
        if (super.applyValue(property, value)) {
            return true;
        }

        if (value == null) {
            return true;
        }
        if (property.sameInstance(PROP_TYPE)) {
            getPresentation().setType((Integer) value);
            return true;
        } else if (property.sameInstance(PROP_COLOR)) {
            getPresentation().setColor((Color) value);
            return true;
        } else if (property.sameInstance(PROP_AMBIENT_LEVEL)) {
            getPresentation().setAmbientLevel((Float) value);
            return true;
        } else if (property.sameInstance(PROP_INTENSITY)) {
            getPresentation().setIntensity((Float) value);
            return true;
        } else if (property.sameInstance(PROP_ATTN_NEAR)) {
            getPresentation().setAttenuationNear((Float) value);
            return true;
        } else if (property.sameInstance(PROP_ATTN_FAR)) {
            getPresentation().setAttenuationFar((Float) value);
            return true;
        } else if (property.sameInstance(PROP_IS_ATTN)) {
            getPresentation().setIsAttenuated((Boolean) value);
            return true;
        } else if (property.sameInstance(PROP_SPOT_INNER)) {
            getPresentation().setSpotInnerAngle((Float) value);
            return true;
        } else if (property.sameInstance(PROP_SPOT_OUTER)) {
            getPresentation().setSpotOuterAngle((Float) value);
            return true;
        }

        return false;
    }

    /**
     * Sets light type to Directional ("Sun" type).
     */
    public void setTypeDirectional() {
        setValue(PROP_TYPE, ILightPresentation.DIRECTIONAL);
    }

    /**
     * Sets light type to Point ("Omni", light bulb).
     */
    public void setTypePoint() {
        setValue(PROP_TYPE, ILightPresentation.POINT);
    }

    /**
     * Sets light type to Spot (spotlight with a defined angle).
     */
    public void setTypeSpot() {
        setValue(PROP_TYPE, ILightPresentation.SPOT);
    }

    /**
     * Sets light color.
     * @param color Color of the light
     */
    public void setColor(Color color) {
        setValue(PROP_COLOR, color);
    }

    /**
     * Sets the ambient level for the light.
     * This determines how much ambient light is emitted.  The level is
     * multiplied by the light color to determine the final ambient color.
     *
     * @param level Ambient level
     */
    public void setAmbientLevel(float level) {
        setValue(PROP_AMBIENT_LEVEL, level);
    }

    /**
     * @deprecated Use setColor() instead
     */
    public void setDiffuseColor(Color color) {
        setColor(color);
    }

    /**
     * @deprecated Use setColor() instead
     */
    public void setSpecularColor(Color color) {
        setColor(color);
    }

    /**
     * @deprecated Use setColor() and setAmbientLevel() instead
     */
    public void setAmbientColor(Color color) {
        setColor(color);
    }

    /**
     * Set light 'intensity' - a brightness multiplier.
     * @param intensity Amount by which light color should be multiplied
     */
    public void setIntensity(float intensity) {
        setValue(PROP_INTENSITY, intensity);
    }

    /**
     * Sets near attenuation distance.
     * @param distance Distance at which light starts to attenuate
     */
    public void setAttenuationNear(float distance) {
        setValue(PROP_ATTN_NEAR, distance);
    }

    /**
     * Sets far attenuation distance.
     * @param distance Distance beyond which the light has no effect
     */
    public void setAttenuationFar(float distance) {
        setValue(PROP_ATTN_FAR, distance);
    }

    /**
     * Sets spot inner attenuation angle.
     * @param angle Angle within which light is full strength
     */
    public void setSpotInnerAngle(float angle) {
        setValue(PROP_SPOT_INNER, angle);
    }

    /**
     * Sets spot outer attentuation angle.
     * @param angle Angle outside of which light has no effect
     */
    public void setSpotOuterAngle(float angle) {
        setValue(PROP_SPOT_OUTER, angle);
    }

    /**
     * Light attenuation enable/disable switch.
     * @param isAttenuated True if light level should drop with distance
     */
    public void setIsAttenuated(boolean isAttenuated) {
        setValue(PROP_IS_ATTN, isAttenuated);
    }
}
