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
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Property;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import com.mediatek.ngin3d.animation.Animation;
import com.mediatek.ngin3d.animation.BasicAnimation;
import com.mediatek.ngin3d.animation.Mode;
import com.mediatek.ngin3d.animation.PropertyAnimation;
import com.mediatek.ngin3d.utils.Ngin3dException;
import junit.framework.TestCase;

import static android.test.MoreAsserts.assertNotEqual;
import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;

public class PropertyAnimationTest extends TestCase {
    @SmallTest
    public void testStartEndValue() {
        Point start = new Point(0, 0);
        Point end = new Point(480, 800);
        Actor target = new Empty();
        PropertyAnimation ani = new PropertyAnimation(target, "position", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        assertEquals(target, ani.getTarget());
        assertEquals(false, ani.getAutoReverse());
        assertEquals(false, ani.getLoop());
        assertEquals(BasicAnimation.DEFAULT_DURATION, ani.getDuration());
        assertEquals(Mode.LINEAR, ani.getMode());
        ani.setProgress(0.5f);
        assertThat(target.getPosition().x, is(240f));
        assertThat(target.getPosition().y, is(400f));
        ani.setProgress(1f);
        assertThat(target.getPosition().x, is(480f));
        assertThat(target.getPosition().y, is(800f));
        assertEquals(target.getPosition(), end);

    }

    @SmallTest
    public void testFloatValue() {
        Float start = new Float(0);
        Float end = new Float(1);
        Text text = new Text("test");
        PropertyAnimation ani = new PropertyAnimation(text, "text_size", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        ani.setProgress(0.5f);
        assertThat(text.getTextSize(), is(0.5f));
        ani.setProgress(1f);
        assertThat(text.getTextSize(), is(1f));

    }

    @SmallTest
    public void testIntegerValue() {
        Integer start = new Integer(0);
        Integer end = new Integer(255);
        Plane plane = new Plane();
        PropertyAnimation ani = new PropertyAnimation(plane, "opacity", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        ani.setProgress(0.5f);
        assertThat(plane.getOpacity(), is(127));
        ani.setProgress(1f);
        assertThat(plane.getOpacity(), is(255));
    }

    @SmallTest
    public void testScaleValue() {
        Scale start = new Scale(1, 1);
        Scale end = new Scale(2, 2);
        Empty empty = new Empty();
        PropertyAnimation ani = new PropertyAnimation(empty, "scale", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        ani.setProgress(0.5f);
        assertEquals(empty.getScale(), new Scale(1.5f, 1.5f));
        ani.setProgress(1f);
        assertEquals(empty.getScale(), end);

    }

    @SmallTest
    public void testRotationValue() {
        Rotation start = new Rotation(0, 0, 0);
        Rotation end = new Rotation(180, 180, 360);
        Empty empty = new Empty();
        PropertyAnimation ani1= new PropertyAnimation(empty, "rotation", start, end);
        assertEquals(start, ani1.getStartValue());
        assertEquals(end, ani1.getEndValue());
        ani1.setProgress(0.5f);
        assertEquals(empty.getRotation(), new Rotation(90, 90, 180));
        ani1.setProgress(1f);
        assertEquals(empty.getRotation(), end);
        empty.stopAnimations();

        start.set(0, 1, 0, 0);
        end.set(0, 1, 0, 360);
        PropertyAnimation ani2= new PropertyAnimation(empty, "rotation", start, end);
        ani2.setProgress(0.5f);
        assertEquals(empty.getRotation(), new Rotation(0, 1, 0, 180));
        ani2.setProgress(1f);
        assertEquals(empty.getRotation(), end);
    }

    @SmallTest
    public void testColorValue() {
        Color start = new Color(Color.BLACK.getRgb());
        Color end = new Color(Color.WHITE.getRgb());
        Empty empty = new Empty();
        PropertyAnimation ani = new PropertyAnimation(empty, "color", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        ani.setProgress(0f);
        assertEquals(empty.getColor(), start);
        ani.setProgress(1f);
        assertEquals(empty.getColor(), end);
    }

    @SmallTest
    public void testCameraValue() {
        Stage.Camera start = new Stage.Camera(new Point(0, 0, 0), new Point(240, 480, 0));
        Stage.Camera end = new Stage.Camera(new Point(10, 10, 10), new Point(240, 480, 1000));
        Stage stage = new Stage();
        PropertyAnimation ani = new PropertyAnimation(stage, "camera", start, end);
        assertEquals(start, ani.getStartValue());
        assertEquals(end, ani.getEndValue());
        ani.setProgress(0f);
        assertEquals(stage.getCamera().position, new Point(0, 0, 0));
        ani.setProgress(1f);
        assertEquals(stage.getCamera().position, new Point(10, 10, 10));
    }

    @SmallTest
    public void testInvalidValue() {
        try {
            Scale start = new Scale(1, 1);
            Scale end = new Scale(2, 2);
            PropertyAnimation ani = new PropertyAnimation(new Empty(), "SCALE", start, end);
            fail("Should throw exception because of wrong property name.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        try {
            Double start = 0.0;
            Double end = 1.0;
            PropertyAnimation ani = new PropertyAnimation(new Empty(), "position", start, end);
            fail("Should throw exception because Double animation is not supported.");
        } catch (Ngin3dException e) {
            // expected
        }

        try {
            PropertyAnimation ani = new PropertyAnimation(new Text("test"), "text_size", 1.0f, 1);
            fail("Should throw exception when different values of types are passed.");
        } catch (ClassCastException e) {
            // expected
        }

        try {
            PropertyAnimation ani = new PropertyAnimation(new Empty(), "x", new Float(0));
            fail("Should throw exception when only one value is passed.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        // The third one should be ignored.
        PropertyAnimation ani = new PropertyAnimation(new Text("test"), "text_size", new Float(0), new Float(1), new Float(2));
    }

    public void testSetTarget() {
        Rotation start = new Rotation(0, 0, 0);
        Rotation end = new Rotation(180, 180, 360);
        PropertyAnimation ani = new PropertyAnimation(new Empty(), "rotation", start, end);
        Actor a = new Empty();

        Exception exception = null;
        try {
            ani.setTarget(a);
        } catch (RuntimeException e) {
            exception = e;
        } catch (Exception e) {
            fail("Unexpected exception");
        }

        ani.setTargetVisible(true);
        assertTrue(ani.getTarget().getVisible());
    }

    public void testConstructor() {
        Rotation rotStart = new Rotation(0, 0, 0);
        Rotation rotEnd = new Rotation(180, 180, 360);
        Actor empty = new Empty();
        PropertyAnimation rotation1 = new PropertyAnimation("rotation", rotStart, rotEnd);
        rotation1.setTarget(empty);
        PropertyAnimation rotation2 = new PropertyAnimation(Actor.PROP_ROTATION, rotStart, rotEnd);
        rotation2.setTarget(empty);

        Scale scaleStart = new Scale(1, 1, 1);
        Scale scaleEnd = new Scale(10, 10, 30);
        PropertyAnimation scale1 = new PropertyAnimation("size", scaleStart, scaleEnd);
        try {
            scale1.setTarget(empty);
            fail("Should throw exception when target has no property 'size'.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        PropertyAnimation scale2 = new PropertyAnimation(Plane.PROP_SIZE, scaleStart, scaleEnd);
        try {
            scale2.setTarget(empty);
            fail("Should throw exception when target has no property 'size'.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        Property<Scale> property = null;
        try {
            PropertyAnimation nullProp = new PropertyAnimation(property, scaleStart, scaleEnd);
            fail("Should throw exception when property is null.");
        } catch (IllegalArgumentException e) {
            // expected
        }
        String propertyName = null;
        try {
            PropertyAnimation nullProp = new PropertyAnimation(propertyName, scaleStart, scaleEnd);
            fail("Should throw exception when property name is null.");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    public void testClone() throws CloneNotSupportedException {
        Rotation rotStart = new Rotation(0, 0, 0);
        Rotation rotEnd = new Rotation(180, 180, 360);
        Actor empty = new Empty();
        PropertyAnimation rotation = new PropertyAnimation("rotation", rotStart, rotEnd);
        rotation.setName("rotation");
        rotation.setTarget(empty);
        PropertyAnimation clone = rotation.clone();
        assertEquals(null, clone.getTarget());
        assertEquals("", clone.getName());

        rotation.setDuration(5);
        clone.setDuration(2);
        assertNotEqual(clone.getDuration(), rotation.getDuration());

        rotation.setDirection(Animation.BACKWARD);
        clone.setDirection(Animation.FORWARD);
        assertNotEqual(clone.getDirection(), rotation.getDirection());

        rotation.setTimeScale(1.5f);
        clone.setTimeScale(2f);
        assertNotEqual(clone.getTimeScale(), rotation.getTimeScale());

        rotation.setProgress(0.5f);
        clone.setProgress(0.8f);
        assertNotEqual(clone.getProgress(), rotation.getProgress());

        rotation.setLoop(true);
        clone.setLoop(false);
        assertNotEqual(clone.getLoop(), rotation.getLoop());
    }
}
