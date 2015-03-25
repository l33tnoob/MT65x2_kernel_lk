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
 * Video Presentation for J3M
 */
package com.mediatek.ngin3d.j3m;

import com.mediatek.j3m.Appearance;
import com.mediatek.j3m.Solid;
import com.mediatek.ngin3d.presentation.VideoDisplay;

/**
 * Represents a video quad in 3D space
 *
 * @hide
 */
public class VideoPresentation extends PlanePresentation
    implements VideoDisplay {
    /**
     * Initializes this object with J3M presentation engine
     */
    public VideoPresentation(J3mPresentationEngine engine, boolean isYUp) {
        super(engine, isYUp);
    }

    @Override
    public void onInitialize() {
        super.onInitialize();

        Solid solid = (Solid) getSceneNode();
        Appearance appearance = solid.getAppearance();
        appearance.setShaderProgram(
            getEngine().getAssetPool().getShaderProgram("ngin3d#vidquad.sp"));
    }

    /**
     * Set the 4x4 texture coordinate transform matrix associated with the
     * texture image set by the most recent call to updateTexImage.
     *
     * The matrix is stored in column-major order so that it may be passed
     * directly to OpenGL ES via the glLoadMatrixf or glUniformMatrix4fv
     * functions.
     *
     * @param matrix the array into which the 4x4 matrix will be stored.
     *               The array must have exactly 16 elements.
     */
    public void setTextureTransform(float[] matrix) {
        if (matrix.length != 16) {
            throw new IllegalArgumentException(
                "The length of 4*4 matrix array should be 16 rather than "
                    + matrix.length);
        }
        ((Solid) getSceneNode()).getAppearance().setMatrix4f(
            "M_UV_TRANSFORM_MATRIX", matrix);
    }

}
