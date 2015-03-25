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
import com.mediatek.ngin3d.EulerOrder;
import com.mediatek.ngin3d.Vec3;
import com.mediatek.ngin3d.Quaternion;
import com.mediatek.ngin3d.Rotation;
import junit.framework.TestCase;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.closeTo;
import static org.hamcrest.Matchers.is;

public class RotationTest extends TestCase {

    private final double DEGREE_TOLERANCE = 0.00001;
    private final double QUATERNION_TOLERANCE = 0.001;
    private final double VECTOR_TOLERANCE = 0.001;


    @SmallTest
    public void testRotation() {
        Rotation rot4 = new Rotation(0.1f, 0.2f, 0.3f);
        float[] xyz = rot4.getEulerAngles();
        assertEquals(0.1f, xyz[0]);
        assertEquals(0.2f, xyz[1]);
        assertEquals(0.3f, xyz[2]);

        Rotation rot5 = new Rotation(0.1f, 0.2f, 0.3f, 90.0f);
        double angle = rot5.getAxisAngle();
        assertEquals(new Vec3(0.1f, 0.2f, 0.3f), rot5.getAxis());
        assertEquals(90.0, angle);
    }

    @SmallTest
    public void testChangeMode() {
        Rotation rot1 = new Rotation(1.0f, 1.0f, 1.0f);
        assertEquals(Rotation.MODE_XYZ_EULER, rot1.getMode());

        rot1.set(1.0f, 1.0f, 1.0f, 45.0f);
        assertEquals(Rotation.MODE_AXIS_ANGLE, rot1.getMode());
    }

    public void testDefaultAngle() {
        Rotation rotation = new Rotation();
        float[] xyz2 = rotation.getEulerAngles();
        assertThat((double) xyz2[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)xyz2[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)xyz2[2], closeTo(0, DEGREE_TOLERANCE));

        float[] xyz3 = rotation.getEulerAngles(EulerOrder.ZYX);
        assertThat((double)xyz3[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)xyz3[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)xyz3[2], closeTo(0, DEGREE_TOLERANCE));

        Quaternion quaternion = rotation.getQuaternion();
        float angle = quaternion.getAxisAngle();
        Vec3 v = quaternion.getAxis();
        assertThat((double)angle, closeTo(0, DEGREE_TOLERANCE));
        assertEquals(v, Vec3.X_AXIS);

        assertThat((double)quaternion.getQ0(), closeTo(1, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ1(), closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(0, DEGREE_TOLERANCE));

    }

    public void testQuaternionToEulerWithSmallAngles() {
        Rotation rotation = new Rotation(0.1f, 0.2f, 0.3f);

        Quaternion quaternion = rotation.getQuaternion();
        // These compared quaternion value came from Blender and blender gave accurate value when angle is small.
        assertThat((double)quaternion.getQ0(), closeTo(1, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ1(), closeTo(0.000868, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(0.001748, DEGREE_TOLERANCE));
        assertThat((double)quaternion.getQ3(), closeTo(0.002616, DEGREE_TOLERANCE));


        float[] euler = quaternion.getEulerAngles();
        assertThat((double)euler[0], closeTo(0.1, DEGREE_TOLERANCE));
        assertThat((double)euler[1], closeTo(0.2, DEGREE_TOLERANCE));
        assertThat((double)euler[2], closeTo(0.3, DEGREE_TOLERANCE));

    }

    public void testQuaternionToEulerWithNormalAngles() {
        Rotation rotation = new Rotation(10f, 20f, 30f);

        Quaternion quaternion = rotation.getQuaternion();
        // These compared quaternion value came from Blender hence it needs more tolerance.
        assertThat((double)quaternion.getQ0(), closeTo(0.9515, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ1(), closeTo(0.03813, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(0.1893, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ3(), closeTo(0.2392, QUATERNION_TOLERANCE));

        float[] euler = quaternion.getEulerAngles();
        assertThat((double)euler[0], closeTo(10, DEGREE_TOLERANCE));
        assertThat((double)euler[1], closeTo(20, DEGREE_TOLERANCE));
        assertThat((double)euler[2], closeTo(30, DEGREE_TOLERANCE));

    }

    public void testQuaternionToEulerWithNegativeAngles() {
        Rotation rotation = new Rotation(-10f, -20f, -30f);

        Quaternion quaternion = rotation.getQuaternion();
        // These compared quaternion value came from Blender hence it needs more tolerance.
        assertThat((double)quaternion.getQ0(), closeTo(0.9437, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ1(), closeTo(-0.1276, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(-0.1448, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ3(), closeTo(-0.26853, QUATERNION_TOLERANCE));

        float[] euler = quaternion.getEulerAngles();

        assertThat((double)euler[0], closeTo(-10, DEGREE_TOLERANCE));
        assertThat((double)euler[1], closeTo(-20, DEGREE_TOLERANCE));
        assertThat((double)euler[2], closeTo(-30, DEGREE_TOLERANCE));

    }

    public void testAxisAngleToEulerAngle() {
        Rotation rotation = new Rotation(0, 1, 0, 30);

        Quaternion quaternion = rotation.getQuaternion();
        // These compared quaternion value came from Blender hence it needs more tolerance.
        assertThat((double)quaternion.getQ0(), closeTo(0.9659, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ1(), closeTo(0, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ2(), closeTo(0.2588, QUATERNION_TOLERANCE));
        assertThat((double)quaternion.getQ3(), closeTo(0, QUATERNION_TOLERANCE));

        float[] euler = quaternion.getEulerAngles();
        assertThat((double)euler[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)euler[1], closeTo(30, DEGREE_TOLERANCE));
        assertThat((double)euler[2], closeTo(0, DEGREE_TOLERANCE));

        float angle = quaternion.getAxisAngle();
        Vec3 v = quaternion.getAxis();

        assertThat((double)angle, closeTo(30, DEGREE_TOLERANCE));
        assertEquals(v, Vec3.Y_AXIS);
    }

    public void testRotationApplyTo() {
        Rotation rotation = new Rotation(new Vec3(0, 0, 1), 90);

        Quaternion quaternion = rotation.getQuaternion();

        Vec3 rotated = quaternion.applyTo(new Vec3(1, 0, 0));
        assertThat((double)rotated.x, closeTo(0, VECTOR_TOLERANCE));
        assertThat((double)rotated.y, closeTo(1, VECTOR_TOLERANCE));
        assertThat((double)rotated.z, closeTo(0, VECTOR_TOLERANCE));
    }

    public void testAxisAngleToEulerAngleWithOrder() {
        Rotation rotation = new Rotation(0, 1, 0, 30);

        Quaternion quaternion = rotation.getQuaternion();
        float[] XZY = quaternion.getEulerAngles(EulerOrder.XZY);
        assertThat((double)XZY[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)XZY[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)XZY[2], closeTo(30, 0.3));

        float[] YZX = quaternion.getEulerAngles(EulerOrder.YZX);
        assertThat((double)YZX[0], closeTo(30, DEGREE_TOLERANCE));
        assertThat((double)YZX[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)YZX[2], closeTo(0, DEGREE_TOLERANCE));

        float[] YXZ = quaternion.getEulerAngles(EulerOrder.YXZ);
        assertThat((double)YXZ[0], closeTo(30, DEGREE_TOLERANCE));
        assertThat((double)YXZ[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)YXZ[2], closeTo(0, DEGREE_TOLERANCE));

        float[] ZYX = quaternion.getEulerAngles(EulerOrder.ZYX);
        assertThat((double)ZYX[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)ZYX[1], closeTo(30, DEGREE_TOLERANCE));
        assertThat((double)ZYX[2], closeTo(0, DEGREE_TOLERANCE));

        float[] ZXY = quaternion.getEulerAngles(EulerOrder.ZXY);
        assertThat((double)ZXY[0], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)ZXY[1], closeTo(0, DEGREE_TOLERANCE));
        assertThat((double)ZXY[2], closeTo(30, DEGREE_TOLERANCE));
    }

    public void testEulerToEulerAngleWithOrder() {
        Rotation rotation = new Rotation(10f, 20f, 30f);

        Quaternion quaternion = rotation.getQuaternion();

        float[] XYZ = quaternion.getEulerAngles(EulerOrder.XYZ);
        assertThat((double)XYZ[0], closeTo(10, QUATERNION_TOLERANCE));
        assertThat((double)XYZ[1], closeTo(20, QUATERNION_TOLERANCE));
        assertThat((double)XYZ[2], closeTo(30, QUATERNION_TOLERANCE));

        float[] XZY = quaternion.getEulerAngles(EulerOrder.XZY);
        assertThat((double)XZY[0], closeTo(-1.17, QUATERNION_TOLERANCE));
        assertThat((double)XZY[1], closeTo(28.024, QUATERNION_TOLERANCE));
        assertThat((double)XZY[2], closeTo(22.796, QUATERNION_TOLERANCE));

        float[] YZX = quaternion.getEulerAngles(EulerOrder.YZX);
        assertThat((double)YZX[0], closeTo(24.944, QUATERNION_TOLERANCE));
        assertThat((double)YZX[1], closeTo(26.165, QUATERNION_TOLERANCE));
        assertThat((double)YZX[2], closeTo(10.475, QUATERNION_TOLERANCE));

        float[] YXZ = quaternion.getEulerAngles(EulerOrder.YXZ);
        assertThat((double)YXZ[0], closeTo(20.283, QUATERNION_TOLERANCE));
        assertThat((double)YXZ[1], closeTo(9.391, QUATERNION_TOLERANCE));
        assertThat((double)YXZ[2], closeTo(26.548, QUATERNION_TOLERANCE));

        float[] ZYX = quaternion.getEulerAngles(EulerOrder.ZYX);
        assertThat((double)ZYX[0], closeTo(28.451, QUATERNION_TOLERANCE));
        assertThat((double)ZYX[1], closeTo(22.242, QUATERNION_TOLERANCE));
        assertThat((double)ZYX[2], closeTo(-1.116, QUATERNION_TOLERANCE));

        float[] ZXY = quaternion.getEulerAngles(EulerOrder.ZXY);
        assertThat((double)ZXY[0], closeTo(28.029, QUATERNION_TOLERANCE));
        assertThat((double)ZXY[1], closeTo(-1.033, QUATERNION_TOLERANCE));
        assertThat((double)ZXY[2], closeTo(22.245, QUATERNION_TOLERANCE));
    }

    public void testEulerToAxisAngle() {
        Rotation rotation = new Rotation(0, 30f, 0);

        Vec3 v = rotation.getAxis();
        float angle = rotation.getAxisAngle();

        assertThat((double)angle, closeTo(30, DEGREE_TOLERANCE));
        assertEquals(v, Vec3.Y_AXIS);
    }
}
