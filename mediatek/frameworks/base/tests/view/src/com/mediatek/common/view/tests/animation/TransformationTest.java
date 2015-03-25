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

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.common.view.tests.animation;

import android.graphics.Matrix;
import android.test.AndroidTestCase;
import android.view.animation.Transformation;
import dalvik.annotation.TestLevel;
import dalvik.annotation.TestTargetClass;
import dalvik.annotation.TestTargetNew;
import dalvik.annotation.TestTargets;
import dalvik.annotation.ToBeFixed;

@TestTargetClass(Transformation.class)
public class TransformationTest extends AndroidTestCase {

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "Transformation",
        args = {}
    )
    public void testConstructor() {
        new Transformation();
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "compose",
        args = {Transformation.class}
    )
    @ToBeFixed(bug = "1695243", explanation = "{@link Transformation#compose(Transformation t)}"
            + "needs to update Javadoc to declare how it composed.")
    public void testCompose() {
        final Transformation t1 = new Transformation();
        final Transformation t2 = new Transformation();
        t1.setAlpha(0.5f);
        t2.setAlpha(0.4f);
        t1.getMatrix().setScale(3, 1);
        t2.getMatrix().setScale(3, 1);
        t1.setTransformationType(Transformation.TYPE_MATRIX);
        t2.setTransformationType(Transformation.TYPE_ALPHA);
        t2.compose(t1);

        Matrix expectedMatrix = new Matrix();
        expectedMatrix.setScale(9, 1);
        assertEquals(expectedMatrix, t2.getMatrix());
        assertEquals(0.4f * 0.5f, t2.getAlpha());
        assertEquals(Transformation.TYPE_ALPHA, t2.getTransformationType());

        t1.setTransformationType(Transformation.TYPE_IDENTITY);
        t2.compose(t1);
        expectedMatrix = new Matrix();
        expectedMatrix.setScale(27, 1);
        assertEquals(expectedMatrix, t2.getMatrix());
        assertEquals(0.4f * 0.5f * 0.5f, t2.getAlpha());
        assertEquals(Transformation.TYPE_ALPHA, t2.getTransformationType());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "clear",
        args = {}
    )
    public void testClear() {
        final Transformation t1 = new Transformation();
        final Transformation t2 = new Transformation();
        t2.set(t1);
        assertTransformationEquals(t1, t2);

        // Change the t2
        t2.setAlpha(0.0f);
        t2.getMatrix().setScale(2, 3);
        t2.setTransformationType(Transformation.TYPE_ALPHA);
        assertTransformationNotSame(t1, t2);

        // Clear the change
        t2.clear();
        assertTransformationEquals(t1, t2);
    }

    private void assertTransformationNotSame(Transformation expected, Transformation actual) {
        assertNotSame(expected.getAlpha(), actual.getAlpha());
        assertFalse(expected.getMatrix().equals(actual.getMatrix()));
        assertNotSame(expected.getTransformationType(), actual.getTransformationType());
    }

    private void assertTransformationEquals(Transformation expected, Transformation actual) {
        assertEquals(expected.getAlpha(), actual.getAlpha());
        assertEquals(expected.getMatrix(), actual.getMatrix());
        assertEquals(expected.getTransformationType(), actual.getTransformationType());
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setTransformationType",
            args = {int.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getTransformationType",
            args = {}
        )
    })
    public void testAccessTransformationType() {
        final Transformation transformation = new Transformation();

        // From Javadoc of {@link Transformation#clear()}, we see the default type is TYPE_BOTH.
        assertEquals(Transformation.TYPE_BOTH, transformation.getTransformationType());

        transformation.setTransformationType(Transformation.TYPE_IDENTITY);
        assertEquals(Transformation.TYPE_IDENTITY, transformation.getTransformationType());

        transformation.setTransformationType(Transformation.TYPE_ALPHA);
        assertEquals(Transformation.TYPE_ALPHA, transformation.getTransformationType());

        transformation.setTransformationType(Transformation.TYPE_MATRIX);
        assertEquals(Transformation.TYPE_MATRIX, transformation.getTransformationType());

        transformation.setTransformationType(Transformation.TYPE_BOTH);
        assertEquals(Transformation.TYPE_BOTH, transformation.getTransformationType());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "set",
        args = {Transformation.class}
    )
    public void testSet() {
        final Transformation t1 = new Transformation();
        t1.setAlpha(0.0f);
        final Transformation t2 = new Transformation();
        t2.set(t1);
        assertTransformationEquals(t1, t2);
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "setAlpha",
            args = {float.class}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "getAlpha",
            args = {}
        )
    })
    public void testAccessAlpha() {
        final Transformation transformation = new Transformation();

        transformation.setAlpha(0.0f);
        assertEquals(0.0f, transformation.getAlpha());

        transformation.setAlpha(0.5f);
        assertEquals(0.5f, transformation.getAlpha());

        transformation.setAlpha(1.0f);
        assertEquals(1.0f, transformation.getAlpha());
    }

    @TestTargets({
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "toString",
            args = {}
        ),
        @TestTargetNew(
            level = TestLevel.COMPLETE,
            method = "toShortString",
            args = {}
        )
    })
    public void testToString() {
        assertNotNull(new Transformation().toString());
        assertNotNull(new Transformation().toShortString());
    }

    @TestTargetNew(
        level = TestLevel.COMPLETE,
        method = "getMatrix",
        args = {}
    )
    public void testGetMatrix() {
        final Matrix expected = new Matrix();
        final Transformation transformation = new Transformation();
        assertEquals(expected, transformation.getMatrix());
    }
}
