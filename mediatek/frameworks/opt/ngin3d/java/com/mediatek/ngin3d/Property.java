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

import java.util.Arrays;

/**
 * Represents properties (position, color, etc) of Actors in the scene.
 * A property can have name, default value, and dependency on another property.
 * If property B depends on property A, the value of property will be applied before value of property B.
 */
public class Property<T> {
    protected String mName;
    protected T mDefaultValue;
    protected Property[] mDependsOn;
    protected int mFlags;

    public static final int FLAG_ANIMATABLE = 0x0001;

    public Property(String name, T defaultValue, Property... dependsOn) {
        mName = name;
        mDefaultValue = defaultValue;
        mDependsOn = dependsOn;
    }

    public Property(String name, T defaultValue, int flags, Property... dependsOn) {
        mName = name;
        mDefaultValue = defaultValue;
        mFlags = flags;
        mDependsOn = dependsOn;
    }

    public boolean isAnimatable() {
        return (mFlags & FLAG_ANIMATABLE) != 0;
    }

    public boolean isKeyPath() {
        return false;
    }

    public String getName() {
        return mName;
    }

    public T defaultValue() {
        return mDefaultValue;
    }

    public boolean dependsOn(Property other) {
        if (this == other) {
            return false;
        }

        for (Property dep : mDependsOn) {
            if (dep.dependsOn(other)) {
                return true;
            }
        }

        return false;
    }

    public void addDependsOn(Property... dependsOn) {
        Property[] merged = new Property[mDependsOn.length + dependsOn.length];
        System.arraycopy(mDependsOn, 0, merged, 0, mDependsOn.length);
        System.arraycopy(dependsOn, 0, merged, mDependsOn.length, dependsOn.length);
        mDependsOn = merged;
    }

    Property[] getDependsOn() {
        return mDependsOn;
    }

    @Override
    public int hashCode() {
        int result = mName == null ? 0 : mName.hashCode();
        result = 31 * result + (mDefaultValue == null ? 0 : mDefaultValue.hashCode());
        result = 31 * result + (mDependsOn == null ? 0 : Arrays.hashCode(mDependsOn));
        result = 31 * result + mFlags;
        return result;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        Property property = (Property) o;

        if (mFlags != property.mFlags) return false;
        if (mDefaultValue == null ? property.mDefaultValue != null : !mDefaultValue.equals(property.mDefaultValue)) return false;
        if (!Arrays.equals(mDependsOn, property.mDependsOn)) return false;
        if (mName == null ? property.mName != null : !mName.equals(property.mName)) return false;

        return true;
    }

    @Override
    public String toString() {
        return mName;
    }

    public final boolean sameInstance(Property another) {
        return this == another;
    }
}
