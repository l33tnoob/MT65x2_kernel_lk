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

package android.widget;

import android.annotation.Widget;
import android.content.Context;
import android.graphics.Camera;
import android.graphics.Matrix;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewParent;
import android.view.animation.Transformation;

import com.mediatek.xlog.Xlog;

/**
 * New added class for new common control BookmarkView.
 * 
 * A view that shows items in a center-locked, and transform each image base on
 * the distance between the center of child image view and center of the view, 
 * farther distance with smaller view.
 *
 * @hide
 */
@Widget
public class BounceCoverFlow extends BounceGallery {
    private static final String TAG = "BounceCoverFlow";
    private static final boolean DBG = false;

    private static int DEFAULT_MAX_ROTATION = 60;
    private static float DEFAULT_MAX_ZOOM = 400.0f;

    private int mMaxRotationAngle = DEFAULT_MAX_ROTATION;
    private float mMaxZoom = DEFAULT_MAX_ZOOM;

    private final Camera mCamera = new Camera();

    public BounceCoverFlow(Context context) {
        this(context, null);
    }

    public BounceCoverFlow(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BounceCoverFlow(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setStaticTransformationsEnabled(true);
    }

    /**
     * Get center of the cover flow.
     * 
     * @return
     */
    private int getCenterOfCoverflow() {
        return ((getMeasuredWidth() - mPaddingLeft - mPaddingRight) >> 1) + mPaddingLeft;
    }

    /**
     * Set max zoom out value.
     * 
     * @param maxZoom controls the maximum zoom of the central image
     *
     * @internal
     */
    public void setMaxZoomOut(final float maxZoom) {
        mMaxZoom = maxZoom;
    }

    /**
     * Set max rotation angle, it must be bigger than 0.
     * 
     * @param maxAngle
     */
    public void setMaxRotationAngle(final int maxAngle) {
        mMaxRotationAngle = maxAngle;
    }

    @Override
    protected boolean getChildStaticTransformation(View child, Transformation t) {
        final int childCenter = getCenterOfChildWithScroll(child);
        final int childWidth = child.getWidth();
        int rotationAngle = 0;

        t.clear();
        t.setTransformationType(Transformation.TYPE_MATRIX);

        if (childCenter == getCenterOfCoverflow()) {
            transformImageBitmap(child, t, 0);
        } else { // other images
            int distToCenter = getCenterOfCoverflow() - childCenter;
            rotationAngle = (int) Math.abs((((float) distToCenter / childWidth) * mMaxRotationAngle));
            if (DBG) {
                Xlog.d(TAG, "getChildStaticTransformation: distToCenter = " + distToCenter
                        + ",childCenter = " + childCenter + ",center cover flow = "
                        + getCenterOfCoverflow() + ",rotationAngle = " + rotationAngle
                        + ",mScrollX = " + mScrollX + ",child = " + child);
            }
            if (rotationAngle > mMaxRotationAngle) {
                rotationAngle = mMaxRotationAngle;
            }
            transformImageBitmap(child, t, rotationAngle);
        }

        return true;
    }

    /**
     * Transform image bitmap, zoom the child, first move the center of the
     * child to (0, 0), then translate the child in Z-axis, restore the center
     * of child to (width/2, height/2) at last.
     * 
     * @param child the transformed child.
     * @param t
     * @param rotation the coefficient of rotate angle.
     */
    private void transformImageBitmap(final View child, final Transformation t, final int rotationAngle) {
        if (DBG) {
            Xlog.d(TAG, "transformImageBitmap: rotation " + rotationAngle);
        }

        mCamera.save();

        final Matrix imageMatrix = t.getMatrix();
        final int imageHeight = child.getHeight();
        final int imageWidth = child.getWidth();

        final float zoomAmount = rotationAngle * Math.abs((float) mMaxZoom / mMaxRotationAngle);
        if (DBG) {
            Xlog.d(TAG, "transformImageBitmap: zoomAmount " + zoomAmount + ",rotationAngle = "
                    + rotationAngle + ",child = " + child);
        }

        mCamera.translate(0.0f, 0.0f, zoomAmount);
        mCamera.getMatrix(imageMatrix);

        imageMatrix.preTranslate(-((imageWidth) / 2), -(imageHeight / 2));
        imageMatrix.postTranslate(((imageWidth) / 2), (imageHeight / 2));

        mCamera.restore();
    }

    /**
     * Utility to keep mNextSelectedPosition and mNextSelectedRowId in sync
     * 
     * @param position Intended value for mSelectedPosition the next time we go
     * through layout
     *
     * @hide
     */
    public void setNextSelectedPositionInt(int position) {
        super.setNextSelectedPositionInt(position);
    }
}
