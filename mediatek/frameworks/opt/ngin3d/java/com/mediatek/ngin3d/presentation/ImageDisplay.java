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

import android.content.res.Resources;
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.utils.JSON;

/**
 * Provide image decoding and display.
 */
public interface ImageDisplay extends Presentation {

    /**
     * A Inner class to contain resource information  for other class to use.
     */
    public static class Resource implements JSON.ToJson {
        public Resources resources;
        public int resId;

        public Resource(Resources resources, int resId) {
            this.resources = resources;
            this.resId = resId;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;

            Resource resource = (Resource) o;

            if (resId != resource.resId) return false;
            if (resources == null ? resource.resources != null : !resources.equals(resource.resources)) return false;

            return true;
        }

        @Override
        public int hashCode() {
            int result = resources == null ? 0 : resources.hashCode();
            result = 31 * result + resId;
            return result;
        }

        @Override
        public String toString() {
            return String.format("Resource:{resources:\"%s\", resId:%d}", resources, resId);
        }

        public String toJson() {
            return String.format("{Resource:{resources:\"%s\", resId:%d}}", resources, resId);
        }
    }

    /**
     * Specify the image to display.
     *
     * @param src image source
     */
    void setImageSource(ImageSource src);

    /**
     * To display only part of source image.
     *
     * @param rect the rectangle to display. If null is specified, the entire source image will be displayed.
     */
    void setSourceRect(Box rect);

    /**
     * Query the dimension of image
     *
     * @return image dimension. return (0, 0) if image is not available.
     */
    Dimension getSourceDimension();

    /**
     * Specify the width/height to display the image.
     *
     * @param size in pixels
     */
    void setSize(Dimension size);

    /**
     * Gets the size of this object.
     * @return   size of this object.
     */
    Dimension getSize();

    /**
     * Specify the opacity of image when alpha source is set to image.
     *
     * @param opacity The value of opacity
     */
    void setOpacity(int opacity);

    /**
     * Get opacity.
     *
     * @return opacity of image
     */
    int getOpacity();

    /**
     * Get the value of filter quality
     * @return The value of filter quality
     */
    int getFilterQuality();

    void setFilterQuality(int quality);

    /**
     * Sets this object to keep the aspect ratio of the image.
     * @param kar  a boolean value to indicate the status.
     */
    void setKeepAspectRatio(boolean kar);

    /**
     * Checks this object if it keeps the aspect ratio of the image.
     * @return  true if the image of this object keeps the aspect ratio.
     */
    boolean isKeepAspectRatio();

     /**
     * Sets the x and y repeat times of the image in this object.
     * @param x  x axis repeating times
     * @param y  y axis repeating times
     */
    void setRepeat(int x, int y);

     /**
     * Gets the repeating times of x axis.
     * @return  a value of x repeating times
     */
    int getRepeatX();

     /**
     * Gets the repeating times of y axis.
     * @return  a value of y repeating times
     */
    int getRepeatY();

    /**
     * Enable mipmap of the object or not.
     *
     * @param enable true for enable and false for disable
     */
    void enableMipmap(boolean enable);

    /**
     * Check mipmap of the object is enabled or not
     *
     * @return true for enable and false for disable.
     */
    boolean isMipmapEnabled();

    /**
     * Make the rect drawable regardless of orientation of the normal.
     * Makes the 'back' of the rectangle visible, whereas it would
     * normally be culled to enhance performance
     *
     * @param enable true for enable and false for disable
     */
    void enableDoubleSided(boolean enable);

    /**
     * Get texture name
     *
     * @return the name of texture
     */
    int getTexName();
}
