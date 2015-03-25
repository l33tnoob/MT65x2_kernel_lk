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

package com.mediatek.ngin3d.tests;

import android.test.suitebuilder.annotation.SmallTest;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Point;
import junit.framework.TestCase;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;

/**
 * Add description here.
 */
public class PointTest extends TestCase {

    @SmallTest
    public void testEquals() {
        Point a = new Point(1, 1);
        Point b = new Point(1, 1, 0);
        Point c = new Point(1, 1, 1);
        Point d = new Point(1, 1, 1, true);

        assertThat(a, is(equalTo(b)));
        assertThat(b, is(equalTo(a)));
        assertThat(a, not(equalTo(c)));
        assertThat(c, not(equalTo(a)));
        assertThat(b, not(equalTo(c)));
        assertThat(c, not(equalTo(a)));
        assertThat(c, not(equalTo(d)));
    }

    public void testPointConstructor() {
        Point p;

        p = new Point();
        assertTrue(p.x == 0.0f);
        assertTrue(p.y == 0.0f);
        assertTrue(p.z == 0.0f);
        assertFalse(p.isNormalized);

        p = new Point(true);
        assertTrue(p.x == 0.0f);
        assertTrue(p.y == 0.0f);
        assertTrue(p.z == 0.0f);
        assertTrue(p.isNormalized);

        p = new Point(1.0f, 2.0f);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 0.0f);
        assertFalse(p.isNormalized);

        p = new Point(1.0f, 2.0f, true);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 0.0f);
        assertTrue(p.isNormalized);

        p = new Point(1.0f, 2.0f, 3.0f);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 3.0f);
        assertFalse(p.isNormalized);

        p = new Point(1.0f, 2.0f, 3.0f, true);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 3.0f);
        assertTrue(p.isNormalized);

        Point q;

        q = new Point(p);
        assertTrue(q.x == 1.0f);
        assertTrue(q.y == 2.0f);
        assertTrue(q.z == 3.0f);
        assertTrue(q.isNormalized);

        q = new Point(2.0f, p);
        assertTrue(q.x == 2.0f * 1.0f);
        assertTrue(q.y == 2.0f * 2.0f);
        assertTrue(q.z == 2.0f * 3.0f);
        assertTrue(q.isNormalized);
    }

    public void testPointSet() {
        Point p = new Point(4.0f, 5.0f, 6.0f, true);

        p.set(1.0f, 2.0f);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 6.0f);
        assertTrue(p.isNormalized);

        p.set(0.0f, 1.0f, false);
        assertTrue(p.x == 0.0f);
        assertTrue(p.y == 1.0f);
        assertTrue(p.z == 6.0f);
        assertFalse(p.isNormalized);

        p.set(1.0f, 2.0f, 3.0f);
        assertTrue(p.x == 1.0f);
        assertTrue(p.y == 2.0f);
        assertTrue(p.z == 3.0f);
        assertFalse(p.isNormalized);

        p = new Point(0.0f, 1.0f, 2.0f, true);
        assertTrue(p.x == 0.0f);
        assertTrue(p.y == 1.0f);
        assertTrue(p.z == 2.0f);
        assertTrue(p.isNormalized);

        Point q = new Point();

        q.set(p);
        assertTrue(q.x == 0.0f);
        assertTrue(q.y == 1.0f);
        assertTrue(q.z == 2.0f);
        assertTrue(q.isNormalized);

        q.set(2.0f, p);
        assertTrue(q.x == 2.0f * 0.0f);
        assertTrue(q.y == 2.0f * 1.0f);
        assertTrue(q.z == 2.0f * 2.0f);
        assertTrue(q.isNormalized);
    }
}
