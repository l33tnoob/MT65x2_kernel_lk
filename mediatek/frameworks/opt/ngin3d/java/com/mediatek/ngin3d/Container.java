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

import android.util.Log;
import com.mediatek.ngin3d.presentation.Presentation;
import com.mediatek.ngin3d.presentation.PresentationEngine;
import com.mediatek.ngin3d.utils.JSON;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.List;
import java.util.Queue;
import java.util.Stack;

/**
 * A Container is a special Actor that (only) contains other Actors.
 * Containers are extremely useful for managing sets of Actors.  Note that
 * Containers may also include other Containers.
 */
public class Container extends Actor {
    /**
     * It is important to name attached property as ATTACHED_PROP_*. If its name begins with
     * typical PROP_*, it will be treated as class-owned property and will not be dispatched
     * to property chain.
     */
    static final Property<Container> ATTACHED_PROP_PARENT = new Property<Container>("parent", null);
    /**
     * @hide
     */
    public static final int DEPTH_FIRST_SEARCH = 0;
    /**
     * @hide
     */
    public static final int BREADTH_FIRST_SEARCH = 1;

    /**
     * @hide
     */
    public static final int SEARCH_BY_TAG = 0;
    /**
     * @hide
     */
    public static final int SEARCH_BY_ID = 1;

    /**
     * @hide
     */
    protected List<Actor> mChildren = new ArrayList<Actor>();
    /**
     * @hide
     */
    protected List<Actor> mPendingRm = new ArrayList<Actor>();

    /**
     * @hide
     */
    @Override
    protected Presentation createPresentation(PresentationEngine engine) {
        return engine.createContainer();
    }

    /**
     * @hide
     */
    @Override
    public void realize(PresentationEngine presentationEngine) {
        synchronized (this) {
            super.realize(presentationEngine);

            int size = mChildren.size(); // Use indexing rather than iterator to prevent frequent GC
            for (int i = 0; i < size; ++i) {
                final Actor child = mChildren.get(i);

                // This actor was assign to another Container and didn't un-realize yet
                // We need to unrealize it to update the scene graph in A3M
                if (child.getOwner() != this && child.getOwner() != null) {
                    child.unrealize();
                }

                if (!isRealized() || !child.isRealized()) {
                    // It is necessary to set
                    // +parent again to make the properties dirty if it was applied before the parent
                    // or child is re-realized.
                    child.setValue(ATTACHED_PROP_PARENT, this);
                    child.setPropertyChain(new PropertyChainNode());

                }

                child.setOwner(this);
                child.notifyOfLayer(getLayer());
                child.realize(presentationEngine);
            }

            // Process all removed actors
            size = mPendingRm.size();
            for (int i = 0; i < size; ++i) {
                Actor actor = mPendingRm.get(i);
                // We don't need to unrealize the actor if it's assigned to another Container.
                if (actor.getOwner() == this) {
                    actor.setValue(ATTACHED_PROP_PARENT, null);
                    actor.setOwner(null);
                    actor.notifyOfLayer(null);
                    actor.setPropertyChain(null);
                    actor.unrealize();
                }
            }
            mPendingRm.clear();
        }
    }

    /**
     * @hide
     */
    @Override
    public void unrealize() {
        synchronized (this) {
            for (Actor actor : mChildren) {
                actor.unrealize();
            }

            super.unrealize();
        }
    }

    /**
     * Notify the actor of the layer in which it resides.
     * @param layer Layer
     */
    @Override
    protected void notifyOfLayer(Layer layer) {
        super.notifyOfLayer(layer);
        synchronized (this) {
            for (int i = 0; i < mChildren.size(); ++i) {
                mChildren.get(i).notifyOfLayer(layer);
            }
        }
    }

    /**
     * Set the clipping rectangle.
     *
     * @param area Rectangular area outside of which the Actors are not drawn
     */
    @Override
    public void setDisplayArea(Box area) {
        synchronized (this) {
            for (Actor actor : mChildren) {
                actor.setDisplayArea(area);
            }

            super.setDisplayArea(area);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Base.PropertyChain

    @SuppressWarnings("PMD")
    class PropertyChainNode implements Base.PropertyChain {
        public boolean applyAttachedProperty(Base obj, Property property, Object value) {
            if (property.sameInstance(ATTACHED_PROP_PARENT)) {
                if (value != Container.this) {
                    throw new IllegalArgumentException("Unmatched child-parent!");
                }
                if (mPresentation == null) {
                    return false;
                }

                Actor actor = (Actor) obj;
                mPresentation.addChild(actor.getPresentation());
                return true;
            }

            return false;
        }

        public Object getInheritedProperty(Property property) {
            return null;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Parent-child relationship

    /**
     * Add the actors into this group.
     *
     * @param actors actors to add as children
     */
    protected void addChild(Actor... actors) {
        synchronized (this) {
            for (Actor child : actors) {
                if (child == null) {
                    continue;
                }
                // Avoid adding duplicated child
                if (mChildren.contains(child)) {
                    Log.w(TAG, "The actor is already in the group");
                    continue;
                }
                mChildren.add(child);

                if (!mPendingRm.remove(child)) {
                    // Attach PARENT property to the child
                    child.setValue(ATTACHED_PROP_PARENT, this);

                    // Listen for unhandled property so that we can apply PARENT
                    child.setPropertyChain(new PropertyChainNode());
                }
            }
        }

        requestRender();
    }

    /**
     * Remove the actor from this container.
     *
     * @param child child to remove
     */
    protected void removeChild(Actor child) {
        synchronized (this) {
            if (mChildren.remove(child)) {
                // Avoid adding duplicated child
                if (!mPendingRm.contains(child)) {
                    mPendingRm.add(child);
                }
            }
        }
        requestRender();
    }

    protected void removeAllChildren() {
        synchronized (this) {
            mPendingRm.addAll(mChildren);
            mChildren.clear();
        }
        requestRender();
    }

    /**
     * Gets the number of children in the Container.
     *
     * @return  the number of actors
     */
    public int getChildrenCount() {
        synchronized (this) {
            return mChildren.size();
        }
    }

    /**
     * Gets the number of descendants in the Container.
     *
     * @return  the number of actors
     */
    public int getDescendantCount() {
        synchronized (this) {
            int count = mChildren.size();
            for (Actor actor : mChildren) {
                if (actor instanceof Container) {
                    count += ((Container) actor).getDescendantCount();
                }
            }
            return count;
        }
    }

    /**
     * Gets the child in Container with index.
     * @return  the child with specific index
     */
    protected <T> T getChildByIndex(int index) {
        synchronized (this) {
            return (T) mChildren.get(index);
        }
    }

    /**
     * Gets the all children in Container.
     * @return  the List of children
     */
    protected List<Actor> getAllChildren() {
        synchronized (this) {
            return new ArrayList<Actor>(mChildren);
        }
    }

    /**
     * Gets the named actor from within the children of the Container.
     *
     * @param childName  Name of actor to look for.
     * @return  Actor object, or null if the actor is not found.
     */
    public Actor findChildByName(CharSequence childName) {
        synchronized (this) {
            for (Actor actor : mChildren) {
                if (actor.getName().equals(childName)) {
                    return actor;
                }
            }
            return null;
        }
    }

    /**
     * Gets the named actor from within the descendents of the Container.
     *
     * @param childName  Name of actor to look for.
     * @param searchMode  0 is depth-first search and 1 is breadth-first search, otherwise search first level only.
     * @return  Actor object, or null if the actor is not found.
     */
    public Actor findChildByName(CharSequence childName, int searchMode) {
        synchronized (this) {
            if (searchMode == BREADTH_FIRST_SEARCH) {
                return  findChildByBFS(childName);
            } else if (searchMode == DEPTH_FIRST_SEARCH) {
                return findChildByDFS(childName);
            } else {
                return findChildByName(childName);
            }
        }
    }

    /**
     * Gets the specific actor with the tag from children of Container.
     * @param tag  tag of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     *
     * @hide The Tag facility is hidden from the user API
     */
    public Actor findChildByTag(int tag) {
        synchronized (this) {
            for (Actor actor : mChildren) {
                if (actor.getTag() == tag) {
                    return actor;
                }
            }
            return null;
        }
    }

    /**
     * Gets the specific actor with the tag from descendant of Container.
     * @param tag  tag of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     *
     * @hide The Tag facility is hidden from the user API
     */
    public Actor findChildByTag(int tag, int searchMode) {
        synchronized (this) {
            if (searchMode == BREADTH_FIRST_SEARCH) {
                return findChildByBFS(tag, SEARCH_BY_TAG);
            } else if (searchMode == DEPTH_FIRST_SEARCH) {
                return findChildByDFS(tag, SEARCH_BY_TAG);
            } else {
                return findChildByTag(tag);
            }
        }
    }


    /**
     * Gets the actor with the specified ID from the children of the Container.
     * @param id  ID of actor.
     * @return  Actor object, or null if the actor is not found.
     */
    public Actor findChildById(int id) {
        synchronized (this) {
            for (Actor actor : mChildren) {
                if (actor.getId() == id) {
                    return actor;
                }
            }
            return null;
        }
    }

    /**
     * Gets the specific actor with the id from descendant of Container.
     * @param id  id of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     *
     * @hide The ID facility is hidden from the user API
     */
    protected Actor findChildById(int id, int searchMode) {
        synchronized (this) {
            if (searchMode == BREADTH_FIRST_SEARCH) {
                return findChildByBFS(id, SEARCH_BY_ID);
            } else if (searchMode == DEPTH_FIRST_SEARCH) {
                return findChildByDFS(id, SEARCH_BY_ID);
            } else {
                return findChildById(id);
            }
        }
    }


    /**
     * Use BFS way to gets the specific actor with the tag from descendant of Container.
     * @param tag  tag of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     */
    private Actor findChildByBFS(int tag, int attribute) {
        Queue<Container> queue = new ArrayDeque<Container>();
        queue.add(this);
        while (queue.size() > 0) {
            Container group = queue.remove();
            List<Actor> list = group.getAllChildren();
            for (Actor actor : list) {
                if (attribute == SEARCH_BY_TAG) {
                    if (actor.getTag() == tag) {
                        return actor;
                    }
                } else if (attribute == SEARCH_BY_ID) {
                    if (actor.getId() == tag) {
                        return actor;
                    }
                }
                if (actor instanceof Container) {
                    queue.add((Container) actor);
                }
            }
        }
        return null;
    }

    /**
     * Use BFS way to gets the specific actor with the name from descendant of Container.
     * @param childName  name of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     */
    private Actor findChildByBFS(CharSequence childName) {
        Queue<Container> queue = new ArrayDeque<Container>();
        queue.add(this);
        while (queue.size() > 0) {
            Container group = queue.remove();
            List<Actor> list = group.getAllChildren();
            for (Actor actor : list) {
                if (actor.getName().equals(childName)) {
                    return actor;
                }
                if (actor instanceof Container) {
                    queue.add((Container) actor);
                }
            }
        }
        return null;
    }

    /**
     * Use DFS way to gets the specific actor with the tag from descendant of Container.
     *
     * @param tag  tag of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     */
    private Actor findChildByDFS(int tag, int attribute) {
        Stack<Actor> stack = new Stack<Actor>();
        stack.push(this);
        while (stack.size() > 0) {
            Actor popped = stack.pop();
            if (attribute == SEARCH_BY_TAG) {
                if (popped.getTag() == tag) {
                    return popped;
                }
            } else if (attribute == SEARCH_BY_ID) {
                if (popped.getId() == tag) {
                    return popped;
                }
            }
            if (popped instanceof Container) {
                List<Actor> list = ((Container) popped).getAllChildren();
                for (Actor actor : list) {
                    stack.push(actor);
                }
            }
        }
        return null;
    }

    /**
     * Use DFS way to gets the specific actor with the name from descendant of Container.
     * @param childName  name of actor.
     * @return  actor object, or null if the specific actor is not existed in this Container.
     */
    private Actor findChildByDFS(CharSequence childName) {
        Stack<Actor> stack = new Stack<Actor>();
        stack.push(this);
        while (stack.size() > 0) {
            Actor popped = stack.pop();
            if (popped.getName().equals(childName)) {
                return popped;
            }
            if (popped instanceof Container) {
                List<Actor> list = ((Container) popped).getAllChildren();
                for (Actor actor : list) {
                    stack.push(actor);
                }
            }
        }
        return null;
    }

    protected void raiseChild(Actor actor, Actor sibling) {
        synchronized (this) {
            if (sibling != null && !mChildren.contains(sibling)) {
                throw new IllegalArgumentException("sibling does not exist in the list");
            }
            if (actor == null || !mChildren.contains(actor)) {
                throw new IllegalArgumentException("actor does not exist in the list");
            }

            mChildren.remove(actor);
            int pos = (sibling == null) ? mChildren.size() : mChildren.indexOf(sibling) + 1;
            mChildren.add(pos, actor);
        }
    }

    protected void lowerChild(Actor actor, Actor sibling) {
        synchronized (this) {
            if (sibling != null && !mChildren.contains(sibling)) {
                throw new IllegalArgumentException("sibling does not exist in the list");
            }
            if (actor == null || !mChildren.contains(actor)) {
                throw new IllegalArgumentException("actor does not exist in the list");
            }

            mChildren.remove(actor);
            int pos = (sibling == null) ? 0 : mChildren.indexOf(sibling);
            mChildren.add(pos, actor);
        }
    }

    /**
     * @hide Internal use
     */
    @Override
    public String dump() {
        StringBuffer buffer = new StringBuffer();
        synchronized (this) {
            buffer.append(super.dump());
            int count = mChildren.size();
            for (int i = 0; i < count; i++) {
                Actor actor = mChildren.get(i);
                buffer.append(",");
                buffer.append(actor.getClass().getSimpleName() + i + ":");
                buffer.append(JSON.wrap(actor.dump()));
            }
            JSON.wrap(buffer);
            buffer.insert(0, this.getClass().getSimpleName() + ":");
        }
        return buffer.toString();
    }

    /**
     * Sets the color of all Actors in this Container. The change is applied to
     * all the <i>current</i> descendents of this Actor.  The Container itself
     * has no useful sense of 'color'.  If Actors are subsequently moved into
     * this Container they do <i>not</i> acquire the Container's color until
     * another call to setColor is made.
     *
     * @param color The color value
     */
    @Override
    public void setColor(Color color) {
        super.setColor(color);

        for (int i = 0; i < mChildren.size(); ++i) {
            mChildren.get(i).setColor(color);
        }
    }

    /**
     * Stop animations of this container and animation of all its children recursively.
     */
    @Override
    public void stopAnimations() {
        super.stopAnimations();

        for (Actor actor : mChildren) {
            actor.stopAnimations();
        }
    }

    /**
     * Discover whether the container or any of its children are dirty (need redrawing).
     *
     * @return True if an Actor needs re-rendering ('dirty')
     */
    public boolean isDirty() {
        synchronized (this) {
            if (super.isDirty()) {
                return true;
            }
            int size = mChildren.size();
            for (int i = 0; i < size; ++i) {
                if (mChildren.get(i).isDirty()) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * Discover whether the animations applied to container are running or not.
     *
     * @return True if an animation is running.
     */
    public boolean isAnimationStarted() {
        synchronized (this) {
            if (super.isAnimationStarted()) {
                return true;
            }
            int size = mChildren.size();
            for (int i = 0; i < size; ++i) {
                if (mChildren.get(i).isAnimationStarted()) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * @hide Undocumented
     */
    public void reloadBitmapTexture() {
        synchronized (this) {
            int size = getChildrenCount(); // Use indexing rather than iterator to prevent frequent GC
            for (int i = 0; i < size; ++i) {
                Actor child = getChildByIndex(i);
                if (child instanceof Container) {
                    ((Container) child).reloadBitmapTexture();
                } else if (child instanceof Image) {
                    ((Image) child).loadAsync();
                }
            }
        }
    }

    /**
     * Touch and Actor property and make it dirty.
     *
     * @param propertyName  selected property name
     * @hide
     */
    public void touchProperty(String propertyName) {
        synchronized (this) {
            super.touchProperty(propertyName);
            int size = mChildren.size();
            for (int i = 0; i < size; ++i) {
                mChildren.get(i).touchProperty(propertyName);
            }
        }
    }

    /**
     * Add actors into this container.
     *
     * @param actors Actors to add as children
     */
    public void add(Actor... actors) {
        addChild(actors);
    }

    /**
     * Remove actor from this container.
     *
     * @param child The child Actor to remove
     */
    public void remove(Actor child) {
        removeChild(child);
    }

    /**
     * Remove ALL Actors from this container.
     */
    public void removeAll() {
        removeAllChildren();
    }

    /**
     * Gets the child in Container with index
     *
     * @param index  The index of child
     * @return The child with specific index
     */
    public <T> T getChild(int index) {
        return (T) getChildByIndex(index);
    }

    /**
     * Gets the all children in Container.
     *
     * @return List of children
     */
    public List<Actor> getChildren() {
        return getAllChildren();
    }

    /**
     * @hide Undocumented
     */
    public void raise(Actor child, Actor sibling) {
        raiseChild(child, sibling);
    }

    /**
     * @hide Undocumented
     */
    public void lower(Actor child, Actor sibling) {
        lowerChild(child, sibling);
    }

}
