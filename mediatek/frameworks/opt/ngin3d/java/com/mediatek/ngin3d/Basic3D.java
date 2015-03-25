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

import android.content.res.Resources;
import android.graphics.Bitmap;
import com.mediatek.ngin3d.presentation.ImageDisplay;
import com.mediatek.ngin3d.presentation.ImageSource;
import com.mediatek.ngin3d.presentation.Model3d;
import com.mediatek.ngin3d.presentation.PresentationEngine;

/**
 * Abstract base class for Basic-3D Actors such as Cube, Sphere, etc.
 */
public abstract class Basic3D  extends Actor {

    ///////////////////////////////////////////////////////////////////////////
    // Property handling

    /**
     * @hide properties should be internal
     */
    public static final Property<ImageSource> PROP_IMG_SRC = new Property<ImageSource>("image_source", null);


    private final int mType;
    private final boolean mIsYUp;

    /**
     * Constructor taking type of basic-3D object.
     * This should not be used directly, only via the Cube, Sphere classes.
     * @param type Model type (Cube etc)
     * @param isYUp True if the model has its y-axis pointing up
     */
    Basic3D(int type, boolean isYUp) {
        mType = type;
        mIsYUp = isYUp;
    }

    /**
     * @hide Presentation API should be internal only
     */
    @Override
    protected Model3d createPresentation(PresentationEngine engine) {
        return engine.createModel3d(mType, mIsYUp);
    }

    /**
     * Returns the Actor's presentation cast to the instantiated type.
     *
     * @hide Presentation API should be internal only
     *
     * @return Presentation object
     */
    @Override
    public Model3d getPresentation() {
        return (Model3d) mPresentation;
    }


    protected boolean applyValue(Property property, Object value) {
        if (property.sameInstance(PROP_IMG_SRC)) {
            ImageSource src = (ImageSource) value;
            if (src == null) {
                return false;
            }
            getPresentation().setTexture(src);
            return true;
        } else if (property.sameInstance(PROP_SCALE)) {
            Scale scale = (Scale) value;
            getPresentation().setScale(new Scale(scale.x, -scale.y, scale.z));
            return true;
        }

        if (super.applyValue(property, value)) {
            return true;
        }

        return false;
    }

    /**
     * Sets the texture image of this 3D object from file name reference.
     * @param filename File name of image.
     */
    public void setImageFromFile(String filename) {
        if (filename == null) {
            throw new NullPointerException("filename cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.FILE, filename));
    }

    /**
     * Sets the texture image of the 3D object from bitmap data.
     * @param bitmap  Bitmap data of the image.
     */
    public void setImageFromBitmap(Bitmap bitmap) {
        if (bitmap == null) {
            throw new NullPointerException("bitmap cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.BITMAP, bitmap));
    }

    /**
     * Sets the texture image of the 3D object from android resource manager.
     * @param resources  Android resource manager.
     * @param resId    Resource id
     */
    public void setImageFromResource(Resources resources, int resId) {
        if (resources == null) {
            throw new NullPointerException("resources cannot be null");
        }
        setValue(PROP_IMG_SRC, new ImageSource(ImageSource.RES_ID,
            new ImageDisplay.Resource(resources, resId)));
    }
}
