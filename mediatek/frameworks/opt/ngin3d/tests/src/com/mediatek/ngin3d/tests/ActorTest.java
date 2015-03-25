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
import com.mediatek.ngin3d.Box;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.ImmutablePoint;
import com.mediatek.ngin3d.KeyPathProperty;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Rotation;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.animation.PropertyAnimation;

import java.security.InvalidParameterException;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;

public class ActorTest extends Ngin3dTest {
    @SmallTest
    public void testActorProperties() {
        Actor actor = new Empty();

        Point point = new Point(0, 0, 0);
        assertEquals(point, actor.getPosition());
        Actor hitActor = actor.hitTest(point);
        assertEquals(null, hitActor);

        point = new Point(1, 1, 1);
        actor.setPosition(point);
        assertEquals(point, actor.getPosition());
        hitActor = actor.hitTest(point);
        assertEquals(null, hitActor);

        assertEquals(true, actor.getVisible());
        actor.setVisible(false);
        assertEquals(false, actor.getVisible());
        actor.setVisible(true);
        assertEquals(true, actor.getVisible());

        Rotation rotation = new Rotation(0, 0, 0);
        actor.setRotation(rotation);
        assertEquals(rotation, actor.getRotation());
        Scale scale = new Scale(2, 2);
        actor.setScale(scale);
        assertEquals(scale, actor.getScale());

        actor.setReactive(true);
        assertEquals(true, actor.getReactive());
        actor.setReactive(false);
        assertEquals(false, actor.getReactive());

        actor.setTag(1);
        assertEquals(1, actor.getTag());

        Box area = new Box(0, 0, 50, 50);
        actor.setDisplayArea(area);
        assertEquals(area, actor.getDisplayArea());
        actor.setDisplayArea(null);
        assertEquals(null, actor.getDisplayArea());

        Container c = new Container();
        actor.setOwner(c);
        assertEquals(c, actor.getOwner());

        assertThat(actor.getId(), is(not(0)));

        actor.dump();

        String string = actor.toString().substring(0, 5);
        assertEquals(string, "Actor");

        PropertyAnimation animation = new PropertyAnimation(actor, Actor.PROP_POSITION, new Point(0, 0), new Point(800, 480));
        animation.start();
        actor.stopAnimations();
        assertFalse(animation.isStarted());
    }

    @SmallTest
    public void testDumpProperties() {
        Actor empty = new Empty();
        mStage.add(empty);
        empty.dumpProperties();
    }

    @SmallTest
    public void testKeyPathProperties() {
        Actor empty = new Empty();

        try {
            empty.setKeyPathValue("", 0);
            fail("Should throw InvalidParameterException when empty key path is passed.");
        } catch (InvalidParameterException e) {
            // expected
        }

        assertNull(empty.getKeyPathValue("foo"));
        empty.setKeyPathValue("foo", 1);
        assertEquals(1, empty.getKeyPathValue("foo"));

        assertNull(empty.getKeyPathValue("foo.bar"));
        empty.setKeyPathValue("foo.bar", 2.0f);
        assertEquals(2.0f, empty.getKeyPathValue("foo.bar"));

        KeyPathProperty kpp = new KeyPathProperty("foo.bar.baz");
        assertEquals("baz", kpp.getLastKey());
        assertEquals("foo", kpp.getFirstKey());
        assertEquals("foo.bar", kpp.getParentKeyPath());
        assertEquals("foo", kpp.getKey(0));
        assertEquals("bar", kpp.getKey(1));
        assertEquals("baz", kpp.getKey(2));
        assertEquals(3, kpp.getKeyPathLength());

        try {
            kpp.getKey(-1);
            fail("Should throw IndexOutOfBoundsException when invalid index is passed.");
        } catch (IndexOutOfBoundsException e) {
            // expected
        }

        try {
            kpp.getKey(3);
            fail("Should throw IndexOutOfBoundsException when invalid index is passed.");
        } catch (IndexOutOfBoundsException e) {
            // expected
        }
    }

}
