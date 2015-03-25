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
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Empty;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Text;

import java.util.Random;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;

public class ContainerTest extends Ngin3dInstrumentationTestCase {
    Container mContainer;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContainer = new Container();
        mStage.add(mContainer);
    }

    @Override
    protected void tearDown() throws Exception {
        mStage.removeAll();
        super.tearDown();
    }

    @SmallTest
    public void testBasics() throws ExecutionException, InterruptedException {
        final Container container = new Container();
        Text text = new Text();
        Actor empty = new Empty();
        container.add(text);
        container.add(empty);
        assertEquals(2, container.getChildrenCount());

        assertSame(text, container.<Actor>getChild(0));
        container.<Text>getChild(0).setText("First");
        assertEquals("First", text.getText());

        assertSame(empty, container.<Actor>getChild(1));
        empty.setPosition(new Point(1, 1, 1));
        assertEquals(new Point(1, 1, 1), container.<Empty>getChild(1).getPosition());

        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                container.realize(mPresentationEngine);
                return true;
            }
        });

        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());

        assertFalse("Container should not be dirty after realized", container.isDirty());
        assertFalse("Text should not be dirty after its container is realized", text.isDirty());
        assertFalse("Actor should not be dirty after its container is realized", empty.isDirty());

        Container container2 = new Container();
        container.add(container2);
        task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                container.realize(mPresentationEngine);
                return true;
            }
        });

        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
        assertFalse("Container should not be dirty after its container is realized", container2.isDirty());
    }

    @SmallTest
    public void testAddRemove() {
        Actor actor = Image.createEmptyImage();
        mContainer.add(actor);
        assertEquals(mContainer.getChildren().size(), 1);

        mContainer.remove(actor);
        assertEquals(mContainer.getChildren().size(), 0);

        // Can add duplicate actor?
        // Can add null actor?
        // Will throw exception when actor to remove does not exist?
    }

    @SmallTest
    public void testAddNull() {
        Empty empty = new Empty();
        mContainer.add(empty, null);
        assertEquals(1, mContainer.getChildren().size());
    }

    @SmallTest
    public void testAddAgain() {
        Actor actor = Image.createEmptyImage();
        mContainer.add(actor);
        mContainer.remove(actor);
        mContainer.add(actor);
    }

    @SmallTest
    public void testFind() {
        Container childContainer1 = new Container();
        Container childContainer2 = new Container();
        Container childContainer3 = new Container();

        Actor grandchild1 = Image.createEmptyImage();
        Actor grandchild2 = Image.createEmptyImage();
        Actor grandchild3 = Image.createEmptyImage();
        mContainer.add(childContainer1);
        mContainer.add(childContainer2);
        mContainer.add(childContainer3);
        childContainer1.add(grandchild1);
        childContainer2.add(grandchild2);
        childContainer3.add(grandchild3);

        childContainer1.setName("childContainer1");
        childContainer2.setName("childContainer2");
        childContainer3.setName("childContainer3");
        grandchild1.setName("granchild1");
        grandchild2.setName("granchild2");
        grandchild3.setName("granchild3");

        Actor foundChild = mContainer.findChildByName(childContainer1.getName());
        assertEquals(childContainer1, foundChild);
        foundChild = mContainer.findChildByName(childContainer1.getName(), Container.BREADTH_FIRST_SEARCH);
        assertEquals(childContainer1, foundChild);

        Actor foundGrandchild = mContainer.findChildByName(grandchild1.getName());
        assertEquals(null, foundGrandchild);
        foundGrandchild = mContainer.findChildByName(grandchild1.getName(), Container.BREADTH_FIRST_SEARCH);
        assertEquals(grandchild1, foundGrandchild);

        childContainer1.setTag(100);
        childContainer2.setTag(200);
        childContainer3.setTag(300);
        grandchild1.setTag(110);
        grandchild2.setTag(220);
        grandchild3.setTag(330);
        foundChild = mContainer.findChildByTag(100);
        assertEquals(childContainer1, foundChild);
        foundChild = mContainer.findChildByTag(100, Container.BREADTH_FIRST_SEARCH);
        assertEquals(childContainer1, foundChild);
        foundChild = mContainer.findChildByTag(100, Container.DEPTH_FIRST_SEARCH);
        assertEquals(childContainer1, foundChild);

        foundChild = mContainer.findChildByTag(110);
        assertEquals(null, foundChild);
        foundChild = mContainer.findChildByTag(110, Container.BREADTH_FIRST_SEARCH);
        assertEquals(grandchild1, foundChild);
        foundChild = mContainer.findChildByTag(110, Container.DEPTH_FIRST_SEARCH);
        assertEquals(grandchild1, foundChild);

        foundChild = mContainer.findChildByTag(220);
        assertEquals(null, foundChild);
        foundChild = mContainer.findChildByTag(220, Container.BREADTH_FIRST_SEARCH);
        assertEquals(grandchild2, foundChild);
        foundChild = mContainer.findChildByTag(220, Container.DEPTH_FIRST_SEARCH);
        assertEquals(grandchild2, foundChild);

        foundChild = mContainer.findChildByTag(330);
        assertEquals(null, foundChild);
        foundChild = mContainer.findChildByTag(330, Container.BREADTH_FIRST_SEARCH);
        assertEquals(grandchild3, foundChild);
        foundChild = mContainer.findChildByTag(330, Container.DEPTH_FIRST_SEARCH);
        assertEquals(grandchild3, foundChild);

        grandchild3.setTag(200);
        foundChild = mContainer.findChildByTag(200);
        assertEquals(childContainer2, foundChild);
        foundChild = mContainer.findChildByTag(200, Container.BREADTH_FIRST_SEARCH);
        assertEquals(childContainer2, foundChild);
        foundChild = mContainer.findChildByTag(200, Container.DEPTH_FIRST_SEARCH);
        assertEquals(grandchild3, foundChild);

        assertEquals(3, mContainer.getChildrenCount());
        assertEquals(6, mContainer.getDescendantCount());
    }

    @SmallTest
    public void testRaise() {
        Actor actor1 = Image.createEmptyImage();
        mContainer.add(actor1);

        Actor actor2 = Image.createEmptyImage();
        mContainer.add(actor2);

        assertEquals(mContainer.getChildren().get(0), actor1);
        assertEquals(mContainer.getChildren().get(1), actor2);

        mContainer.raise(actor1, actor2);

        assertEquals(mContainer.getChildren().get(0), actor2);
        assertEquals(mContainer.getChildren().get(1), actor1);

        mContainer.remove(actor1);
        try {
            mContainer.raise(actor2, actor1);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        try {
            mContainer.raise(actor1, actor2);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    @SmallTest
    public void testLower() {
        Actor actor1 = Image.createEmptyImage();
        mContainer.add(actor1);

        Actor actor2 = Image.createEmptyImage();
        mContainer.add(actor2);

        Actor actor3 = Image.createEmptyImage();
        mContainer.add(actor3);

        mContainer.lower(actor3, actor2);

        assertEquals(mContainer.getChildren().get(0), actor1);
        assertEquals(mContainer.getChildren().get(1), actor3);
        assertEquals(mContainer.getChildren().get(2), actor2);

        mContainer.remove(actor3);

        try {
            mContainer.lower(actor2, actor3);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }

        try {
            mContainer.lower(actor3, actor2);
            fail("IllegalArgumentException expected.");
        } catch (IllegalArgumentException e) {
            // expected
        }
    }

    @SmallTest
    public void testGetChild() {

        Actor actor1 = Image.createEmptyImage();
        mContainer.add(actor1);

        Actor actor2 = Image.createEmptyImage();
        mContainer.add(actor2);

        assertEquals(mContainer.getChild(0), actor1);
        assertEquals(mContainer.getChild(1), actor2);
    }

    @SmallTest
    public void testRealize() throws ExecutionException, InterruptedException {
        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                mContainer.realize(mPresentationEngine);
                return true;
            }
        });
        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());
        assertTrue("Container should be realized successfully", mContainer.isRealized());
    }

    @SmallTest
    public void testPosition() throws ExecutionException, InterruptedException {
        mContainer.setPosition(new Point(100.f, 100.f));
        FutureTask<Boolean> task = new FutureTask<Boolean>(new Callable<Boolean>() {
            public Boolean call() {
                mContainer.realize(mPresentationEngine);
                return true;
            }
        });
        mStageView.runInGLThread(task);
        assertTrue("The test runs successfully", task.get());

        Point pos = mContainer.getPresentation().getPosition(false);
        assertEquals(100f, pos.x);
        assertEquals(100f, pos.y);
    }

    @SmallTest
    public void testGetChildrenCount() {

        Actor actor1 = Image.createEmptyImage();
        mContainer.add(actor1);
        assertEquals(mContainer.getChildrenCount(), 1);

        Actor actor2 = Image.createEmptyImage();
        mContainer.add(actor2);
        assertEquals(mContainer.getChildrenCount(), 2);
    }

    @SmallTest
    public void testRemoveAllChildren() {

        Actor actor1 = Image.createEmptyImage();
        mContainer.add(actor1);

        Actor actor2 = Image.createEmptyImage();
        mContainer.add(actor2);

        Actor actor3 = Image.createEmptyImage();
        mContainer.add(actor3);

        assertEquals(mContainer.getChildren().get(0), actor1);
        assertEquals(mContainer.getChildren().get(1), actor2);
        assertEquals(mContainer.getChildren().get(2), actor3);

        mContainer.removeAll();

        assertEquals(mContainer.getChildrenCount(), 0);
    }

    /**
     * The same test is done in PresentationTest while the presentation tree is realized.
     */
    @SmallTest
    public void testMultiThreadAccess() {
        final Random rnd = new Random(System.currentTimeMillis());
        Thread[] threads = new Thread[10];
        for (int i = 0; i < threads.length; ++i) {
            threads[i] = new Thread(new Runnable() {
                public void run() {
                    for (int j = 0; j < 200; ++j) {
                        try {
                            Thread.sleep(rnd.nextInt(10));
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                            return;
                        }
                        Actor a = new Empty();
                        mContainer.add(a);
                        mContainer.getChildrenCount();
                        mContainer.getChild(0);
                        mContainer.remove(a);
                    }
                }
            });
            threads[i].start();
        }

        for (int i = 0; i < threads.length; ++i) {
            try {
                threads[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        assertEquals(0, mContainer.getChildrenCount());
    }

    public void testOpacity() {
        Plane a = new Plane();
        a.setOpacity(100);
        mContainer.add(a);
        mContainer.setOpacity(150);
        assertThat(a.getOpacity(), is(100));
    }

    public void testColor() {
        Plane a = new Plane();
        a.setColor(new Color(100, 100, 100));
        mContainer.add(a);
        mContainer.setColor(new Color(150, 150, 150));
        assertThat(a.getColor().red, is(150));
    }

    public void testFindChildByTag() {
        Actor a = Image.createEmptyImage();
        a.setTag(10);
        mContainer.add(a);
        assertEquals(a, mContainer.findChildByTag(10));
    }

    public void testFindChildById() {
        Actor a = Image.createEmptyImage();
        a.setTag(1001);
        mContainer.add(a);

        int id = a.getId();
        assertEquals(a.getTag(), mContainer.findChildById(id).getTag());
    }

}
