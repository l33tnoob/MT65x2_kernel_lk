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
 * A basic 3D textured Unit Cube.
 *
 * The default Cube is created with a negative Y axis ("y-down")
 * for use in UI_PERSPECTIVE scenes.
 * <p>
 * <b>Important:</b>  These built-in primitives are <b>not</b> affected
 * by lights in the scene.  It is not expected that they are
 * used in true '3D scenes' for advanced graphics apps.
 *
 * @deprecated The built-in primitives are not suitable for end-user applications.
 */
@Deprecated
public final class Cube extends Basic3D {
    private Cube(boolean isYUp) {
        super(Model3d.CUBE, isYUp);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Public methods

    /**
     * Creates a cube with texture image is from named file.
     * @param filename  Texture image file name.
     * @return  Cube object
     */
    public static Cube createFromFile(String filename) {
        return createFromFile(filename, false);
    }

    /**
     * Creates a cube which texture image is from file name reference.
     * @param filename  Texture image file name
     * @param isYUp   True for creating a Y-up cube, default is Y-down
     * @return  Cube object
     */
    public static Cube createFromFile(String filename, boolean isYUp) {
        Cube cube = new Cube(isYUp);
        cube.setImageFromFile(filename);
        return cube;
    }

    /**
     * Creates a cube which texture image is from bitmap data.
     * @param bitmap   Texture image bitmap data
     * @return  Cube object
     */
    public static Cube createFromBitmap(Bitmap bitmap) {
        return createFromBitmap(bitmap, false);
    }

    /**
     * Creates a cube which texture image is from bitmap data.
     * @param bitmap   Texture image bitmap data
     * @param isYUp   True for creating a Y-up cube, default is Y-down
     * @return  Cube object
     */
    public static Cube createFromBitmap(Bitmap bitmap, boolean isYUp) {
        Cube cube = new Cube(isYUp);
        cube.setImageFromBitmap(bitmap);
        return cube;
    }

    /**
     * Creates a cube which texture image is from android resource manager.
     * @param resources  Android resource manager.
     * @param resId  Resource id
     * @return  Cube object
     */
    public static Cube createFromResource(Resources resources, int resId) {
        return createFromResource(resources, resId, false);
    }

    /**
     * Creates a cube which texture image is from android resource manager.
     * @param resources  Android resource manager.
     * @param resId  Resource id
     * @param isYUp   True for creating a Y-up model, default is Y-down
     * @return  Cube object
     */
    public static Cube createFromResource(Resources resources, int resId, boolean isYUp) {
        Cube cube = new Cube(isYUp);
        cube.setImageFromResource(resources, resId);
        return cube;
    }
}

