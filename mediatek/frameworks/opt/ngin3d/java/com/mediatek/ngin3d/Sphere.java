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
import com.mediatek.ngin3d.presentation.Model3d;

/**
 * A basic 3D textured Unit Sphere.
 *
 * The default Sphere is created with a negative Y axis ("y-down")
 * for use in UI_PERSPECTIVE scenes.
 * <p>
 * <b>Important:</b>  These built-in primitives are <b>not</b> affected
 * by lights in the scene.  It is not expected that they are
 * used in true '3D scenes' for advanced graphics apps.
 *
 * @deprecated The built-in primitives are not suitable for end-user applications.
 */
@Deprecated
public final class Sphere extends Basic3D {
    private Sphere(boolean isYUp) {
        super(Model3d.SPHERE, isYUp);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Public methods

    /**
     * Creates a sphere with texture from file.
     * @param filename  Texture image file name.
     * @return Sphere object
     */
    public static Sphere createFromFile(String filename) {
        return createFromFile(filename, false);
    }

    /**
     * Creates a sphere with texture from file.
     * @param filename  Texture image file name.
     * @param isYUp   true for creating a Y-up sphere, default is Y-down
     * @return Sphere object
     */
    public static Sphere createFromFile(String filename, boolean isYUp) {
        Sphere sphere = new Sphere(isYUp);
        sphere.setImageFromFile(filename);
        return sphere;
    }

    /**
     * Creates a sphere with texture from bitmap data.
     * @param bitmap  Texture image bitmap data
     * @return Sphere object
     */
    public static Sphere createFromBitmap(Bitmap bitmap) {
        return createFromBitmap(bitmap, false);
    }

    /**
     * Creates a sphere with texture from bitmap data.
     * @param bitmap  Texture image bitmap data
     * @param isYUp   True for creating a Y-up model, default is Y-down
     * @return Sphere object
     */
    public static Sphere createFromBitmap(Bitmap bitmap, boolean isYUp) {
        Sphere sphere = new Sphere(isYUp);
        sphere.setImageFromBitmap(bitmap);
        return sphere;
    }

    /**
     * Creates a sphere with texture from android resource manager.
     * @param resources  Android resource manager.
     * @param resId  Resource id
     * @return Sphere object
     */
    public static Sphere createFromResource(Resources resources, int resId) {
        return createFromResource(resources, resId, false);
    }

    /**
     * Creates a sphere with texture from android resource manager.
     * @param resources  Android resource manager.
     * @param resId  Resource id
     * @param isYUp   True for creating a Y-up model, default is Y-down
     * @return Sphere object
     */
    public static Sphere createFromResource(Resources resources, int resId, boolean isYUp) {
        Sphere sphere = new Sphere(isYUp);
        sphere.setImageFromResource(resources, resId);
        return sphere;
    }

}

