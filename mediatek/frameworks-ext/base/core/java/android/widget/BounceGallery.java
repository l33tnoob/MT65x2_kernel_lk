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
 * Copyright (C) 2007 The Android Open Source Project
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

package android.widget;

import android.annotation.Widget;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.view.SoundEffectConstants;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.animation.Transformation;
import android.view.animation.Interpolator;
import android.view.WindowManager;

import com.android.internal.R;

import com.mediatek.xlog.Xlog;

/**
 * New added class for new common control BookmarkView.
 *
 * A view the same as Gallery except adding overscroll effect when moving it, not change
 * code in the origin gallery because CTS issue, remove multi touch and long press related code.
 *
 * @hide
 */
@Widget
public class BounceGallery extends AbsSpinner {
    private static final String TAG = "BounceGallery";
    private static final boolean DBG = true;
    private static final boolean DBG_MOTION = true;
    private static final boolean DBG_KEY = true;
    private static final boolean DBG_LAYOUT = true;

    private static final int OVER_DIST_SCALED_RAT = 3;
    private static final float DEFAULT_UNSELECTED_ALPHA = 0.5f;

    /**
     * Indicates that we are not in the middle of a touch gesture
     */
    private static final int TOUCH_MODE_REST = -1;

    /**
     * Indicates we just received the touch event and we are waiting to see if the it is a tap or a
     * scroll gesture.
     */
    private static final int TOUCH_MODE_DOWN = 0;

    /**
     * Indicates the touch gesture is a scroll
     */
    private static final int TOUCH_MODE_SCROLL = 1;

    /**
     * Indicates the view is in the process of being flung
     */
    private static final int TOUCH_MODE_FLING = 2;

    /**
     * Indicates the touch gesture is an overscroll - a scroll beyond the beginning or end.
     */
    private static final int TOUCH_MODE_OVERSCROLL = 3;

    /**
     * Indicates the view is being flung outside of normal content bounds
     * and will spring back.
     */
    private static final int TOUCH_MODE_OVERFLING = 4;

    private int mTouchMode = TOUCH_MODE_REST;

    /**
     * Horizontal spacing between items.
     */
    private int mSpacing;

    /**
     * How long the transition animation should run when a child view changes
     * position, measured in milliseconds.
     */
    private int mAnimationDuration = 400;

    /**
     * The alpha of items that are not selected.
     */
    private float mUnselectedAlpha;

    /**
     * Left most edge of a child seen so far during layout.
     */
    private int mLeftMost;

    /**
     * Right most edge of a child seen so far during layout.
     */
    private int mRightMost;

    private int mGravity;

    /**
     * The position of the item that received the user's down touch.
     */
    private int mDownTouchPosition;

    /**
     * The view of the item that received the user's down touch.
     */
    private View mDownTouchView;

    /**
     * The last CheckForLongPress runnable we posted, if any
     */
    private CheckForLongPress mPendingCheckForLongPress;

    /**
     * The last CheckForTap runnable we posted, if any
     */
    private Runnable mPendingCheckForTap;

    private AdapterContextMenuInfo mContextMenuInfo;

    /**
     * When fling runnable runs, it resets this to false. Any method along the
     * path until the end of its run() can set this to true to abort any
     * remaining fling. For example, if we've reached either the leftmost or
     * rightmost item, we will set this to true.
     */
    private boolean mShouldStopFling;

    /**
     * The currently selected item's child.
     */
    private View mSelectedChild;

    /**
     * Whether to continuously callback on the item selected listener during a
     * fling.
     */
    private boolean mShouldCallbackDuringFling = true;

    /**
     * Whether to callback when an item that is not selected is clicked.
     */
    private boolean mShouldCallbackOnUnselectedItemClick = true;

    /**
     * If true, do not callback to item selected listener.
     */
    private boolean mSuppressSelectionChanged;

    /**
     * If true, we have received the "invoke" (center or enter buttons) key
     * down. This is checked before we action on the "invoke" key up, and is
     * subsequently cleared.
     */
    private boolean mReceivedInvokeKeyDown;

    /**
     * The x coord of the last motion event.
     */
    private int mLastMotionX;

    private int mTouchSlop;
    private int mMinimumVelocity;
    private int mMaximumVelocity;
    /**
     * Maximum distance to overscroll by during edge effects
     */
    private int mOverscrollDistance;

    /**
     * Maximum distance to overfling during edge effects
     */
    private int mOverflingDistance;

    private int mDistanceLeft;
    private boolean mNeedOverscroll;
    /**
     * Used for determining when to cancel out of overscroll.
     */
    private int mDirection;

    /**
     * Determines speed during touch scrolling
     */
    private VelocityTracker mVelocityTracker;

    private OnSelectionChangeListener mSelectionChangeListener;

    /**
     * Executes the delta scrolls from a fling or scroll movement.
     */
    private FlingRunnable mFlingRunnable = new FlingRunnable();

    /**
     * Sets mSuppressSelectionChanged = false. This is used to set it to false
     * in the future. It will also trigger a selection changed.
     */
    private Runnable mDisableSuppressSelectionChangedRunnable = new Runnable() {
        public void run() {
            mSuppressSelectionChanged = false;
            selectionChanged();
        }
    };

    public BounceGallery(Context context) {
        this(context, null);
    }

    public BounceGallery(Context context, AttributeSet attrs) {
        this(context, attrs, R.attr.galleryStyle);
    }

    public BounceGallery(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        WindowManager windowManager = (WindowManager) mContext.getSystemService(
                Context.WINDOW_SERVICE);

        final ViewConfiguration viewConfig = ViewConfiguration.get(context);
        mOverscrollDistance = viewConfig.getScaledOverscrollDistance() / OVER_DIST_SCALED_RAT;
        mOverflingDistance = viewConfig.getScaledOverflingDistance() / OVER_DIST_SCALED_RAT;

        mTouchSlop = viewConfig.getScaledTouchSlop();
        mMinimumVelocity = viewConfig.getScaledMinimumFlingVelocity();
        mMaximumVelocity = viewConfig.getScaledMaximumFlingVelocity();
        setHorizontalScrollBarEnabled(false);

        // Obtain attributes set from View and use it to initialize scroll bars.
        TypedArray a = context.obtainStyledAttributes(R.styleable.View);
        initializeScrollbars(a);
        a.recycle();

        // Obtain attributes set from Gallery.
        a = context.obtainStyledAttributes(attrs, com.android.internal.R.styleable.Gallery,
                defStyle, 0);

        int index = a.getInt(com.android.internal.R.styleable.Gallery_gravity, -1);
        if (index >= 0) {
            setGravity(index);
        }

        int animationDuration = a.getInt(
                com.android.internal.R.styleable.Gallery_animationDuration, -1);
        if (animationDuration > 0) {
            setAnimationDuration(animationDuration);
        }

        int spacing = a.getDimensionPixelOffset(
                com.android.internal.R.styleable.Gallery_spacing, 0);
        setSpacing(spacing);

        float unselectedAlpha = a.getFloat(
                com.android.internal.R.styleable.Gallery_unselectedAlpha,
                DEFAULT_UNSELECTED_ALPHA);
        setUnselectedAlpha(unselectedAlpha);

        a.recycle();

        // We draw the selected item last (because otherwise the item to the right overlaps it)
        mGroupFlags |= FLAG_USE_CHILD_DRAWING_ORDER;
        mGroupFlags |= FLAG_SUPPORT_STATIC_TRANSFORMATIONS;
    }

    /**
     * Whether or not to callback on any {@link #getOnItemSelectedListener()}
     * while the items are being flinged. If false, only the final selected item
     * will cause the callback. If true, all items between the first and the
     * final will cause callbacks.
     *
     * @param shouldCallback Whether or not to callback on the listener while
     *            the items are being flinged.
     *
     * @internal
     */
    public void setCallbackDuringFling(boolean shouldCallback) {
        mShouldCallbackDuringFling = shouldCallback;
    }

    /**
     * Whether or not to callback when an item that is not selected is clicked.
     * If false, the item will become selected (and re-centered). If true, the
     * {@link #getOnItemClickListener()} will get the callback.
     *
     * @param shouldCallback Whether or not to callback on the listener when a
     *            item that is not selected is clicked.
     * @hide
     */
    public void setCallbackOnUnselectedItemClick(boolean shouldCallback) {
        mShouldCallbackOnUnselectedItemClick = shouldCallback;
    }

    /**
     * Sets how long the transition animation should run when a child view
     * changes position. Only relevant if animation is turned on.
     *
     * @param animationDurationMillis The duration of the transition, in
     *        milliseconds.
     *
     * @attr ref android.R.styleable#Gallery_animationDuration
     */
    public void setAnimationDuration(int animationDurationMillis) {
        mAnimationDuration = animationDurationMillis;
    }

    /**
     * Sets the spacing between items in a BounceGallery.
     *
     * @param spacing The spacing in pixels between items in the BounceGallery.
     *
     * @internal
     */
    public void setSpacing(int spacing) {
        mSpacing = spacing;
    }

    /**
     * Sets the alpha of items that are not selected in the BounceGallery.
     *
     * @param unselectedAlpha the alpha for the items that are not selected.
     *
     * @attr ref android.R.styleable#Gallery_unselectedAlpha
     */
    public void setUnselectedAlpha(float unselectedAlpha) {
        mUnselectedAlpha = unselectedAlpha;
    }

    @Override
    protected boolean getChildStaticTransformation(View child, Transformation t) {
        t.clear();
        t.setAlpha(child == mSelectedChild ? 1.0f : mUnselectedAlpha);
        return true;
    }

    @Override
    protected int computeHorizontalScrollExtent() {
        /* Only 1 item is considered to be selected. */
        return 1;
    }

    @Override
    protected int computeHorizontalScrollOffset() {
        /* Current scroll position is the same as the selected position. */
        return mSelectedPosition;
    }

    @Override
    protected int computeHorizontalScrollRange() {
        /* Scroll range is the same as the item count. */
        return mItemCount;
    }

    @Override
    protected boolean checkLayoutParams(ViewGroup.LayoutParams p) {
        return p instanceof Gallery.LayoutParams;
    }

    @Override
    protected ViewGroup.LayoutParams generateLayoutParams(ViewGroup.LayoutParams p) {
        return new Gallery.LayoutParams(p);
    }

    @Override
    public ViewGroup.LayoutParams generateLayoutParams(AttributeSet attrs) {
        return new Gallery.LayoutParams(getContext(), attrs);
    }

    @Override
    protected ViewGroup.LayoutParams generateDefaultLayoutParams() {
        /*
         * BounceGallery expects Gallery.LayoutParams.
         */
        return new Gallery.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);

        /*
         * Remember that we are in layout to prevent more layout request from
         * being generated.
         */
        mInLayout = true;
        layout(0, false);
        mInLayout = false;
    }

    @Override
    int getChildHeight(View child) {
        return child.getMeasuredHeight();
    }

    /**
     * Tracks a motion scroll. In reality, this is used to do just about any
     * movement to items (touch scroll, arrow-key scroll, set an item as selected).
     *
     * @param deltaX Change in X from the previous event.
     */
    void trackMotionScroll(int deltaX) {
        if (getChildCount() == 0) {
            Xlog.d(TAG, "trackMotionScroll with no children.");
            return;
        }

        boolean toLeft = deltaX < 0;

        int limitedDeltaX = getLimitedMotionScrollAmount(toLeft, deltaX);
        if (limitedDeltaX != deltaX) {
            /* The above call returned a limited amount, so stop any scrolls/flings. */
            if (mTouchMode == TOUCH_MODE_FLING) {
                mNeedOverscroll = true;
                mDistanceLeft = deltaX - limitedDeltaX;
            }

            if (DBG_MOTION) {
                Xlog.d(TAG, "trackMotionScroll: may need over scroll, mTouchMode = " + mTouchMode +
                        ",deltaX = " + deltaX + ",limitedDeltaX = " + limitedDeltaX +
                        ",mDistanceLeft = " + mDistanceLeft + ",mFirstPosition = " + mFirstPosition);
            }
        }

        offsetChildrenLeftAndRight(limitedDeltaX);

        detachOffScreenChildren(toLeft);

        if (toLeft) {
            /* If moved left, there will be empty space on the right. */
            fillToGalleryRight();
        } else {
            /* Similarly, empty space on the left. */
            fillToGalleryLeft();
        }

        /* Clear unused views. */
        mRecycler.clear();
        setSelectionToCenterChild();
        invalidate();
    }

    int getLimitedMotionScrollAmount(boolean motionToLeft, int deltaX) {
        int extremeItemPosition = !motionToLeft ? (isRtL() ? mItemCount - 1 : 0) : (isRtL() ? 0 : mItemCount - 1);

        View extremeChild = getChildAt(extremeItemPosition - mFirstPosition);

        if (extremeChild == null) {
            return deltaX;
        }

        int extremeChildCenter = getCenterOfView(extremeChild);
        int galleryCenter = getCenterOfGallery();

        if (motionToLeft) {
            if (extremeChildCenter <= galleryCenter) {

                /* The extreme child is past his boundary point! */
                return 0;
            }
        } else {
            if (extremeChildCenter >= galleryCenter) {

                /* The extreme child is past his boundary point! */
                return 0;
            }
        }

        int centerDifference = galleryCenter - extremeChildCenter;

        return motionToLeft
                ? Math.max(centerDifference, deltaX)
                : Math.min(centerDifference, deltaX);
    }

    /**
     * Offset the horizontal location of all children of this view by the
     * specified number of pixels.
     *
     * @param offset the number of pixels to offset
     */
    private void offsetChildrenLeftAndRight(int offset) {
        for (int i = getChildCount() - 1; i >= 0; i--) {
            getChildAt(i).offsetLeftAndRight(offset);
        }
    }

    /**
     * @return The center of this BounceGallery.
     */
    private int getCenterOfGallery() {
        return (getWidth() - mPaddingLeft - mPaddingRight) / 2 + mPaddingLeft;
    }

    /**
     * @return The center of the given view.
     */
    private static int getCenterOfView(View view) {
        return view.getLeft() + view.getWidth() / 2;
    }

    /**
     * Get center of the child view.
     *
     * @param child the child view.
     * @return the center of the child plus the horizontal scroll value.
     */
    protected int getCenterOfChildWithScroll(View child) {
        return (getCenterOfView(child) - mScrollX);
    }

    /**
     * Detaches children that are off the screen (i.e.: Gallery bounds).
     *
     * @param toLeft Whether to detach children to the left of the BounceGallery, or
     *            to the right.
     */
    private void detachOffScreenChildren(boolean toLeft) {
        int numChildren = getChildCount();
        int firstPosition = mFirstPosition;
        int start = 0;
        int count = 0;

        if (toLeft) {
            final int galleryLeft = mPaddingLeft;
            View child = null;
            for (int i = 0; i < numChildren; i++) {
                child = getChildAt(i);
                if (child.getRight() >= galleryLeft) {
                    break;
                } else {
                    count++;
                    mRecycler.put(firstPosition + i, child);
                }
            }
        } else {
            final int galleryRight = getWidth() - mPaddingRight;
            View child = null;
            for (int i = numChildren - 1; i >= 0; i--) {
                child = getChildAt(i);
                if (child.getLeft() <= galleryRight) {
                    break;
                } else {
                    start = i;
                    count++;
                    mRecycler.put(firstPosition + i, child);
                }
            }
        }

        detachViewsFromParent(start, count);

        if (toLeft) {
            mFirstPosition += count;
        }
    }

    /**
     * Scrolls the items so that the selected item is in its 'slot' (its center
     * is the gallery's center).
     */
    private void scrollIntoSlots() {
        if (getChildCount() == 0 || mSelectedChild == null) return;

        int selectedCenter = getCenterOfChildWithScroll(mSelectedChild);
        int targetCenter = getCenterOfGallery();

        int scrollAmount = targetCenter - selectedCenter;
        if (DBG_MOTION) {
            Xlog.d(TAG, "scrollIntoSlots:mSelectedChild = " + mSelectedChild + ",selectedCenter = "
                    + selectedCenter + ",targetCenter = " + targetCenter + ",scrollAmount = " + scrollAmount);
        }
        if (scrollAmount != 0) {
            mFlingRunnable.startUsingDistance(scrollAmount);
        } else {
            onFinishedMovement();
        }
    }

    private void onFinishedMovement() {
        if (DBG_MOTION) {
            Xlog.d(TAG, "onFinishedMovement: mSelectedPosition = " + mSelectedPosition);
        }
        if (mSuppressSelectionChanged) {
            mSuppressSelectionChanged = false;

            /* We haven't been callbacking during the fling, so do it now. */
            super.selectionChanged();
        }

        invalidate();
    }

    @Override
    void selectionChanged() {
        if (!mSuppressSelectionChanged) {
            super.selectionChanged();
        }

        if (mSelectionChangeListener != null) {
            mSelectionChangeListener.onSelectionChanged();
        }
        if (DBG_MOTION) {
            Xlog.d(TAG, "selectionChanged mSelectedPosition = " + mSelectedPosition);
        }
    }

    /**
     * Looks for the child that is closest to the center and sets it as the
     * selected child.
     */
    private void setSelectionToCenterChild() {
        View selView = mSelectedChild;
        if (mSelectedChild == null) return;

        int galleryCenter = getCenterOfGallery();

        // Common case where the current selected position is correct.
        if (selView.getLeft() <= galleryCenter && selView.getRight() >= galleryCenter) {
            return;
        }

        int closestEdgeDistance = Integer.MAX_VALUE;
        int newSelectedChildIndex = 0;
        for (int i = getChildCount() - 1; i >= 0; i--) {
            View child = getChildAt(i);
            if (child.getLeft() <= galleryCenter && child.getRight() >= galleryCenter) {
                // This child is in the center.
                newSelectedChildIndex = i;
                break;
            }

            int childClosestEdgeDistance = Math.min(Math.abs(child.getLeft() - galleryCenter),
                    Math.abs(child.getRight() - galleryCenter));
            if (childClosestEdgeDistance < closestEdgeDistance) {
                closestEdgeDistance = childClosestEdgeDistance;
                newSelectedChildIndex = i;
            }
        }

        int newPos = mFirstPosition + newSelectedChildIndex;
        if (DBG_MOTION) {
            Xlog.d(TAG, "setSelectionToCenterChild: newPos = " + newPos + ",newSelectedChildIndex = " +
                    newSelectedChildIndex + ",mSelectedPosition = " + mSelectedPosition);
        }

        if (newPos != mSelectedPosition) {
            setSelectedPositionInt(newPos);
            setNextSelectedPositionInt(newPos);
            checkSelectionChanged();
        }
    }

    /**
     * Creates and positions all views for this BounceGallery.
     * <p>
     * We layout rarely, most of the time {@link #trackMotionScroll(int)} takes
     * care of repositioning, adding, and removing children.
     *
     * @param delta Change in the selected position. +1 means the selection is
     *            moving to the right, so views are scrolling to the left. -1
     *            means the selection is moving to the left.
     */
    @Override
    void layout(int delta, boolean animate) {
        int childrenLeft = mSpinnerPadding.left;
        int childrenWidth = mRight - mLeft - mSpinnerPadding.left - mSpinnerPadding.right;

        if (mDataChanged) {
            handleDataChanged();
        }

        /* Handle an empty bounce gallery by removing all views. */
        if (mItemCount == 0) {
            resetList();
            return;
        }

        /* Update to the new selected position. */
        if (mNextSelectedPosition >= 0) {
            setSelectedPositionInt(mNextSelectedPosition);
        }

        /* All views go in recycler while we are in layout. */
        recycleAllViews();

        /* Clear out old views. */
        detachAllViewsFromParent();

        /*
         * These will be used to give initial positions to views entering the
         * bounce gallery as we scroll.
         */
        mRightMost = 0;
        mLeftMost = 0;

        /* Make selected view and center it. */

        /*
         * mFirstPosition will be decreased as we add views to the left later
         * on. The 0 for x will be offset in a couple lines down.
         */
        mFirstPosition = mSelectedPosition;
        View sel = makeAndAddView(mSelectedPosition, 0, 0, true);

        /* Put the selected child in the center. */
        int selectedOffset = childrenLeft + (childrenWidth / 2) - (sel.getWidth() / 2);
        sel.offsetLeftAndRight(selectedOffset);

        fillToGalleryRight();
        fillToGalleryLeft();

        /* Flush any cached views that did not get reused above. */
        mRecycler.clear();

        invalidate();
        checkSelectionChanged();

        mDataChanged = false;
        mNeedSync = false;
        setNextSelectedPositionInt(mSelectedPosition);

        updateSelectedItemMetadata();
    }

    /**
     * @return true if layout is RtL
     */
    private boolean isRtL() {
        return getLayoutDirection() == LAYOUT_DIRECTION_RTL;
    }

    /**
     * Fill the left space of bounce gallery with children.
     */
    private void fillToGalleryLeft() {
        int itemSpacing = mSpacing;
        int galleryLeft = mPaddingLeft;

        /* Set state for initial iteration. */
        int numChildren = getChildCount();
        View prevIterationView = getChildAt(isRtL() ? numChildren - 1 : 0);
        int numItems = mItemCount;
        int curPosition = 0;
        int curRightEdge = 0;

        if (prevIterationView != null) {
            curPosition = isRtL() ? mFirstPosition + numChildren : mFirstPosition - 1;
            curRightEdge = prevIterationView.getLeft() - itemSpacing;
        } else {
            /* No children available! */
            curPosition = isRtL() ? mItemCount - 1 : 0;
            curRightEdge = mRight - mLeft - mPaddingRight;
            mShouldStopFling = true;
        }

        if (DBG_LAYOUT) {
            Xlog.d(TAG, "fillToGalleryLeft:curRightEdge = " + curRightEdge + ",galleryLeft = "
                    + galleryLeft + ",curPosition = " + curPosition + ",mSelectedPosition = "
                    + mSelectedPosition + ",mFirstPosition = " + mFirstPosition);
        }

        while (curRightEdge > galleryLeft && (isRtL() ? (curPosition < numItems) : (curPosition >= 0))) {
            prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                    curRightEdge, false);

            /* Remember some state. */
            if (!isRtL()) mFirstPosition = curPosition;

            /* Set state for next iteration. */
            curRightEdge = prevIterationView.getLeft() - itemSpacing;
            curPosition--;
        }
    }

    private void fillToGalleryRight() {
        int itemSpacing = mSpacing;
        int galleryRight = mRight - mLeft - mPaddingRight;
        int numChildren = getChildCount();
        int numItems = mItemCount;

        if (numChildren == 0) {
            Xlog.d(TAG, "No child when fill gallery right!");
            return;
        }
        /* Set state for initial iteration. */
        View prevIterationView = getChildAt(isRtL() ? 0 : numChildren - 1);

        int curPosition = 0;
        int curLeftEdge = 0;

        if (prevIterationView != null) {
            curPosition = isRtL() ? mFirstPosition - 1 : mFirstPosition + numChildren;
            curLeftEdge = prevIterationView.getRight() + itemSpacing;
        } else {
            if (isRtL())
                curPosition = 0;
            else
                mFirstPosition = curPosition = mItemCount - 1;

            curLeftEdge = mPaddingLeft;
            mShouldStopFling = true;
        }

        if (DBG_LAYOUT) {
            Xlog.d(TAG, "fillToGalleryRight: curLeftEdge = " + curLeftEdge + ",galleryRight = "
                    + galleryRight + ",curPosition = " + curPosition + ",mSelectedPosition = "
                    + mSelectedPosition + ",mFirstPosition = " + mFirstPosition + ",numItems = "
                    + numItems);
        }

        while (curLeftEdge < galleryRight && (isRtL() ? (curPosition >= 0) : (curPosition < numItems))) {
            prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                    curLeftEdge, true);

            if (isRtL()) mFirstPosition = curPosition;

            /* Set state for next iteration. */
            curLeftEdge = prevIterationView.getRight() + itemSpacing;
            curPosition++;
        }
    }

    /**
     * Obtain a view, either by pulling an existing view from the recycler or by
     * getting a new one from the adapter. If we are animating, make sure there
     * is enough information in the view's layout parameters to animate from the
     * old to new positions.
     *
     * @param position Position in the gallery for the view to obtain
     * @param offset Offset from the selected position
     * @param x X-coordintate indicating where this view should be placed. This
     *            will either be the left or right edge of the view, depending
     *            on the fromLeft paramter
     * @param fromLeft Are we posiitoning views based on the left edge? (i.e.,
     *            building from left to right)?
     * @return A view that has been added to the gallery
     */
    private View makeAndAddView(int position, int offset, int x, boolean fromLeft) {
        View child = null;

        if (!mDataChanged) {
            child = mRecycler.get(position);
            if (child != null) {
                /* Can reuse an existing view. */
                int childLeft = child.getLeft();

                /*
                 * Remember left and right edges of where views have been placed.
                 */
                mRightMost = Math.max(mRightMost, childLeft + child.getMeasuredWidth());
                mLeftMost = Math.min(mLeftMost, childLeft);

                /* Position the view. */
                setUpChild(child, offset, x, fromLeft);

                return child;
            }
        }

        /* Nothing found in the recycler -- ask the adapter for a view. */
        child = mAdapter.getView(position, null, this);

        /* Position the view. */
        setUpChild(child, offset, x, fromLeft);

        return child;
    }

    /**
     * Helper for makeAndAddView to set the position of a view and fill out its
     * layout paramters.
     *
     * @param child The view to position
     * @param offset Offset from the selected position
     * @param x X-coordintate indicating where this view should be placed. This
     *            will either be the left or right edge of the view, depending
     *            on the fromLeft paramter
     * @param fromLeft Are we posiitoning views based on the left edge? (i.e.,
     *            building from left to right)?
     */
    private void setUpChild(View child, int offset, int x, boolean fromLeft) {
        /*
         * Respect layout params that are already in the view. Otherwise make
         * some up....
         */
        Gallery.LayoutParams lp = (Gallery.LayoutParams) child.getLayoutParams();
        if (lp == null) {
            lp = (Gallery.LayoutParams) generateDefaultLayoutParams();
        }

        addViewInLayout(child, fromLeft ? (isRtL() ? 0 : -1) : (isRtL() ? -1 : 0), lp);

        child.setSelected(offset == 0);

        int childHeightSpec = ViewGroup.getChildMeasureSpec(mHeightMeasureSpec, mSpinnerPadding.top
                + mSpinnerPadding.bottom, lp.height);
        int childWidthSpec = ViewGroup.getChildMeasureSpec(mWidthMeasureSpec, mSpinnerPadding.left
                + mSpinnerPadding.right, lp.width);

        child.measure(childWidthSpec, childHeightSpec);

        /* Position vertically based on gravity setting. */
        int childTop = calculateTop(child, true);
        int childBottom = childTop + child.getMeasuredHeight();

        int childLeft = 0;
        int childRight = 0;
        int width = child.getMeasuredWidth();
        if (fromLeft) {
            childLeft = x;
            childRight = childLeft + width;
        } else {
            childLeft = x - width;
            childRight = x;
        }

        child.layout(childLeft, childTop, childRight, childBottom);
    }

    /**
     * Figure out vertical placement based on mGravity
     *
     * @param child Child to place
     * @return Where the top of the child should be
     */
    private int calculateTop(View child, boolean duringLayout) {
        int myHeight = duringLayout ? getMeasuredHeight() : getHeight();
        int childHeight = duringLayout ? child.getMeasuredHeight() : child.getHeight();

        int childTop = 0;

        switch (mGravity) {
            case Gravity.TOP:
                childTop = mSpinnerPadding.top;
                break;
            case Gravity.CENTER_VERTICAL:
            int availableSpace = myHeight - mSpinnerPadding.bottom
                    - mSpinnerPadding.top - childHeight;
                childTop = mSpinnerPadding.top + (availableSpace / 2);
                break;
            case Gravity.BOTTOM:
                childTop = myHeight - mSpinnerPadding.bottom - childHeight;
                break;
            default:
                break;
        }
        return childTop;
    }

    /**
     * Called when a touch event's action is MotionEvent.ACTION_UP or
     * MotionEvent.ACTION_CANCEL.
     */
    void onUpOrCancel() {
        if (DBG_MOTION) {
            Xlog.d(TAG, "onUpOrCancel: finished = " + mFlingRunnable.mScroller.isFinished()
                    + ",mSelectedPosition = " + mSelectedPosition + ",mVelocityTracker = "
                    + mVelocityTracker);
        }

        removeRunnables(mPendingCheckForTap);
        removeRunnables(mPendingCheckForLongPress);

        if (mFlingRunnable.mScroller.isFinished()) {
            scrollIntoSlots();
        }

        dispatchUnpress();

        if (mVelocityTracker != null) {
            mVelocityTracker.recycle();
            mVelocityTracker = null;
        }
    }

    private void dispatchPress(View child) {
        if (child != null) {
            child.setPressed(true);
        }

        setPressed(true);
    }

    private void dispatchUnpress() {
        for (int i = getChildCount() - 1; i >= 0; i--) {
            getChildAt(i).setPressed(false);
        }

        setPressed(false);
    }

    @Override
    public void dispatchSetSelected(boolean selected) {
        /*
         * We don't want to pass the selected state given from its parent to its
         * children since this widget itself has a selected state to give to its
         * children.
         */
    }

    @Override
    protected void dispatchSetPressed(boolean pressed) {
        /* Show the pressed state on the selected child. */
        if (mSelectedChild != null) {
            mSelectedChild.setPressed(pressed);
        }
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        // Gallery steals all key events
        return event.dispatch(this, null, null);
    }

    /**
     * Handles left, right, and clicking.
     *
     * @see android.view.View#onKeyDown
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_LEFT:
                if (movePrevious()) {
                    playSoundEffect(SoundEffectConstants.NAVIGATION_LEFT);
                }
                return true;

            case KeyEvent.KEYCODE_DPAD_RIGHT:
                if (moveNext()) {
                    playSoundEffect(SoundEffectConstants.NAVIGATION_RIGHT);
                }
                return true;

            case KeyEvent.KEYCODE_DPAD_CENTER:
            case KeyEvent.KEYCODE_ENTER:
                mReceivedInvokeKeyDown = true;

            // Fall through...
            default:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_CENTER:
            case KeyEvent.KEYCODE_ENTER:
                if (mReceivedInvokeKeyDown && mItemCount > 0) {
                    dispatchPress(mSelectedChild);
                    postDelayed(new Runnable() {
                        public void run() {
                            dispatchUnpress();
                        }
                    }, ViewConfiguration.getPressedStateDuration());

                    int selectedIndex = mSelectedPosition - mFirstPosition;
                    performItemClick(getChildAt(selectedIndex), mSelectedPosition, mAdapter
                            .getItemId(mSelectedPosition));
                }

                // Clear the flag.
                mReceivedInvokeKeyDown = false;
                return true;

            default:
                break;
        }

        return super.onKeyUp(keyCode, event);
    }

    private boolean movePrevious() {
        if (mItemCount > 0 && mSelectedPosition > 0) {
            scrollToChild(mSelectedPosition - mFirstPosition - 1);
            return true;
        } else {
            return false;
        }
    }

    private boolean moveNext() {
        if (mItemCount > 0 && mSelectedPosition < mItemCount - 1) {
            scrollToChild(mSelectedPosition - mFirstPosition + 1);
            return true;
        } else {
            return false;
        }
    }

    private boolean scrollToChild(int childPosition) {
        View child = getChildAt(childPosition);
        if (DBG_MOTION || DBG_KEY) {
            Xlog.d(TAG, "scrollToChild: childPosition = " + childPosition + ",mSelectedPosition = "
                    + mSelectedPosition + ",mFirstPosition = " + mFirstPosition + ",mScrollX = "
                    + mScrollX + ",child = " + child);
        }
        if (child != null) {
            int distance = getCenterOfGallery() - getCenterOfView(child);
            if (DBG_MOTION || DBG_KEY) {
                Xlog.d(TAG, "scrollToChild: childPosition = " + childPosition + ",centerGallery = "
                        + getCenterOfGallery() + ",center view = " + getCenterOfView(child)
                        + ",distance = " + distance);
            }
            mFlingRunnable.startUsingDistance(distance);
            return true;
        }

        return false;
    }

    @Override
    void setSelectedPositionInt(int position) {
        super.setSelectedPositionInt(position);

        /* Updates any metadata we keep about the selected item. */
        updateSelectedItemMetadata();
    }

    private void updateSelectedItemMetadata() {
        View oldSelectedChild = mSelectedChild;

        View child = getChildAt(mSelectedPosition - mFirstPosition);
        if (child == null) {
            return;
        }

        mSelectedChild = child;
        child.setSelected(true);
        child.setFocusable(true);

        if (hasFocus()) {
            child.requestFocus();
        }

        /*
         * We unfocus the old child down here so the above hasFocus check
         * returns true.
         */
        if (oldSelectedChild != null) {
            /* Make sure its drawable state doesn't contain 'selected'. */
            oldSelectedChild.setSelected(false);
            /*
             * Make sure it is not focusable anymore, since otherwise arrow keys
             * can make this one be focused
             */
            oldSelectedChild.setFocusable(false);
        }
    }

    /**
     * Describes how the child views are aligned.
     *
     * @param gravity
     * @attr ref android.R.styleable#Gallery_gravity
     *
     * @internal
     */
    public void setGravity(int gravity) {
        if (mGravity != gravity) {
            mGravity = gravity;
            requestLayout();
        }
    }

    @Override
    protected int getChildDrawingOrder(int childCount, int i) {
        int ret = i;
        /*
         * Adjust the drawing order of children, like if there are 5 children
         * 0,1,2,3,4, and 2 is the Selected, make the drawing order to be 0, 4,
         * 1, 3, 2.
         */
        int selectedIndex = mSelectedPosition - mFirstPosition;
        /* Just to be safe. */
        if (selectedIndex < 0) return ret;

        int notSymmNum = 2 * selectedIndex - childCount + 1;
        if (notSymmNum == 0) { /* 0, 1, 2, 3, 4, and 2 is selected. */
            ret = getSemmChildDrawingOrder(childCount, i, selectedIndex);
        } else if (notSymmNum < 0){ /* 0, 1, 2, 3, 4, and 0 or 1 is selected, assume 1 is selected. */
            if (i > 2 * selectedIndex) { /* 3 and 4. */
                ret = childCount - 1 - i;
            } else {
                ret = getSemmChildDrawingOrder(childCount, i, selectedIndex);
            }
        } else { /* 0, 1, 2, 3, 4, and 3 or 4 is selected, assume 3 is selected. */
            if (i < notSymmNum) { /* 0 and 1. */
                ret = i;
            } else {
                ret = getSemmChildDrawingOrder(childCount, i, selectedIndex);
            }
        }

        return ret;
    }

    /**
     * Adjust the drawing order of a symmetry bounce gallery.
     *
     * @param childCount
     * @param i
     * @param selectedIndex
     * @return
     */
    private int getSemmChildDrawingOrder(int childCount, int i, int selectedIndex) {
        if (i == selectedIndex) { /* 2. */
            return childCount - 1;
        } else if (i >= selectedIndex) { /* 3 and 4. */
            return childCount - ((i - selectedIndex) * 2);
        } else { /* 0 and 1. */
            return childCount - 1 - ((selectedIndex - i) * 2);
        }
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);

        /*
         * The gallery shows focus by focusing the selected item. So, give focus
         * to our selected item instead. We steal keys from our selected item
         * elsewhere.
         */
        if (gainFocus && mSelectedChild != null) {
            mSelectedChild.requestFocus(direction);
        }
    }

    @Override
    protected ContextMenuInfo getContextMenuInfo() {
        return mContextMenuInfo;
    }

    @Override
    public boolean showContextMenuForChild(View originalView) {
        final int longPressPosition = getPositionForView(originalView);
        if (longPressPosition < 0) {
            return false;
        }

        final long longPressId = mAdapter.getItemId(longPressPosition);
        return performLongPress(originalView, longPressPosition, longPressId);
    }

    @Override
    public boolean showContextMenu() {
        if (isPressed() && mSelectedPosition >= 0) {
            int index = mSelectedPosition - mFirstPosition;
            View v = getChildAt(index);
            return performLongPress(v, mSelectedPosition, mSelectedRowId);
        }

        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        final int action = ev.getAction();
        final int y = (int) ev.getY();
        final int x = (int) ev.getX();

        // Do not handle data changed.
        if (mVelocityTracker == null) {
            mVelocityTracker = VelocityTracker.obtain();
            Xlog.d(TAG, "onTouchEvent new mVelocityTracker = " + mVelocityTracker);
        }
        mVelocityTracker.addMovement(ev);

        boolean handled = false;
        int deltaX = 0;

        switch (action & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_DOWN: {
                if (DBG_MOTION) {
                    Xlog.d(TAG, "Touch down: mTouchMode = " + mTouchMode + ",x = " + x
                            + ",mLastMotionX = " + mLastMotionX + ",mDownTouchPosition = "
                            + mDownTouchPosition + ",mVelocityTracker = " + mVelocityTracker);
                }
                mLastMotionX = x;
                handleTouchDown(x, y);
                handled = true;
                break;
            }

            case MotionEvent.ACTION_MOVE: {
                deltaX = x - mLastMotionX;
                handleTouchMove(x, y, deltaX);
                handled = true;
                break;
            }

            case MotionEvent.ACTION_UP: {
                if (DBG_MOTION) {
                    Xlog.d(TAG, "Touch up: mTouchMode = " + mTouchMode + ",mLastMotionX = "
                            + mLastMotionX + ",x = " + x + ",deltaX = " + deltaX + ",mScrollX = "
                            + mScrollX + ",mDownTouchPosition = " + mDownTouchPosition);
                }
                handleTouchUp();
                if (mVelocityTracker != null) {
                    mVelocityTracker.recycle();
                    mVelocityTracker = null;
                }
                handled = true;
                break;
            }

            case MotionEvent.ACTION_CANCEL: {
                // Abnormal case, print log to record.
                Xlog.d(TAG, "Touch cancel: mTouchMode = " + mTouchMode + ",mDownTouchView = "
                        + mDownTouchView);

                mTouchMode = TOUCH_MODE_REST;
                if (mDownTouchView != null && mDownTouchView.isPressed()) {
                    mDownTouchView.setPressed(false);
                }
                onUpOrCancel();
                handled = true;
                break;
            }
        }
        return handled;
    }

    /**
     * Check if we have moved far enough that it looks more like a scroll than a
     * tap.
     *
     * @param deltaX
     * @return true if it is a scroll, else false.
     */
    private boolean startScrollIfNeeded(int deltaX) {
        final int distance = Math.abs(deltaX);
        final boolean overscroll = mScrollX != 0;
        if (DBG_MOTION) {
            Xlog.d(TAG, "startScrollIfNeeded: distance = " + distance + ",mScrollX = " + mScrollX
                    + ",mTouchSlop = " + mTouchSlop + ",mPendingCheckForLongPress = "
                    + mPendingCheckForLongPress);
        }
        if (overscroll || distance > mTouchSlop) {
            mTouchMode = overscroll ? TOUCH_MODE_OVERSCROLL : TOUCH_MODE_SCROLL;
            removeRunnables(mPendingCheckForLongPress);
            setPressed(false);
            final View motionView = getChildAt(mDownTouchPosition - mFirstPosition);
            if (motionView != null) {
                motionView.setPressed(false);
            }
            requestDisallowInterceptTouchEvent(true);
            return true;
        }
        return false;
    }

    private void handleTouchDown(final int x, final int y) {
        switch (mTouchMode) {
            case TOUCH_MODE_OVERFLING: {
                mFlingRunnable.stop(false);
                if (mScrollX == 0) {
                    mTouchMode = TOUCH_MODE_SCROLL;
                } else {
                    mTouchMode = TOUCH_MODE_OVERSCROLL;
                }
                break;
            }

            case TOUCH_MODE_FLING: {
                mTouchMode = TOUCH_MODE_SCROLL;
                /* Get the item's view that was touched. */
                mDownTouchPosition = pointToPosition((int) x, (int) y);
                if (mDownTouchPosition >= 0) {
                    mDownTouchView = getChildAt(mDownTouchPosition - mFirstPosition);
                    mDownTouchView.setPressed(true);
                }
                break;
            }

            default: {
                /* Reset the multiple-scroll tracking state. */
                mTouchMode = TOUCH_MODE_DOWN;

                /* Get the item's view that was touched. */
                mDownTouchPosition = pointToPosition((int) x, (int) y);
                if (mDownTouchPosition >= 0) {
                    mDownTouchView = getChildAt(mDownTouchPosition - mFirstPosition);
                    mDownTouchView.setPressed(true);
                }

                if (mPendingCheckForTap == null) {
                    mPendingCheckForTap = new CheckForTap();
                }
                postDelayed(mPendingCheckForTap, ViewConfiguration.getTapTimeout());
                break;
            }
        }
    }

    private void handleTouchMove(final int x, final int y, int deltaX) {
        switch (mTouchMode) {
            case TOUCH_MODE_DOWN: {
                if (DBG_MOTION) {
                    Xlog.d(TAG, "Touch move from touch down: mLastMotionX = " + mLastMotionX
                            + ",x = " + x + ",mDownTouchPosition = " + mDownTouchPosition
                            + ",deltaX = " + deltaX + ",mVelocityTracker = " + mVelocityTracker);
                }
                /*
                 * Check if we have moved far enough that it looks more like a
                 * scroll than a tap.
                 */
                startScrollIfNeeded(deltaX);
                break;
            }

            case TOUCH_MODE_SCROLL: {
                if (x != mLastMotionX) {
                    trackMotionScroll((int) deltaX);

                    int firstPosition = mFirstPosition;
                    int lastPosition = firstPosition + getChildCount();
                    /*
                     * If the gallery has fling to the edge when scrolling, set
                     * touch mode to overscroll mode.
                     */

                    if (isRtL()) {
                         if (deltaX < 0 && firstPosition == 0
                                && getCenterOfView(getChildAt(0)) <= getCenterOfGallery()) {
                            mTouchMode = TOUCH_MODE_OVERSCROLL;
                          } else if (deltaX > 0
                                && lastPosition == mItemCount
                                && getCenterOfView(getChildAt(getChildCount() - 1)) >= getCenterOfGallery()) {
                            mTouchMode = TOUCH_MODE_OVERSCROLL;
                        }
                    } else {
                        if (deltaX > 0 && firstPosition == 0
                                && getCenterOfView(getChildAt(0)) >= getCenterOfGallery()) {
                            mTouchMode = TOUCH_MODE_OVERSCROLL;
                        } else if (deltaX < 0
                          && lastPosition == mItemCount
                          && getCenterOfView(getChildAt(getChildCount() - 1)) <= getCenterOfGallery()) {
                            mTouchMode = TOUCH_MODE_OVERSCROLL;
                        }
                    }

                    if (DBG_MOTION) {
                        Xlog.d(TAG, "Touch move from scroll: mTouchMode = " + mTouchMode
                                + ",mLastMotionX = " + mLastMotionX + ",x = " + x
                                + ",mDownTouchPosition = " + mDownTouchPosition + ",deltaX = "
                                + deltaX + ",mFirstPosition = " + mFirstPosition
                                + ",mVelocityTracker = " + mVelocityTracker);
                    }

                    mLastMotionX = x;
                }
                break;
            }

            case TOUCH_MODE_OVERSCROLL: {
                final int oldScroll = mScrollX;
                final int newScroll = oldScroll - deltaX;
                int newDirection = (int) Math.signum(mScrollX);
                boolean scrollDirectionWillChange = (oldScroll * newScroll < 0);

                if (DBG_MOTION) {
                    Xlog.d(TAG, "Touch move from over scroll: mScrollX = " + mScrollX
                            + ",mLastMotionX = " + mLastMotionX + ",x = " + x + ",deltaX = "
                            + deltaX + ",mDownTouchPosition = " + mDownTouchPosition
                            + ",newScroll = " + newScroll + ",mFirstPosition = " + mFirstPosition
                            + ", mDirection = " + mDirection + ",mVelocityTracker = "
                            + mVelocityTracker);
                }

                if (mDirection == 0) {
                    mDirection = newDirection;
                }

                if (mDirection != newDirection || scrollDirectionWillChange) {
                    /*
                     * Deal with moving left when in overscroll mode and then
                     * moving right, first set the scroll x to zero, and then
                     * change the touch mode to normal scroll mode.
                     */
                    deltaX = -newScroll;
                    mScrollX = 0;

                    /*
                     * No need to do all this work if we're not going to move
                     * anyway.
                     */
                    if (deltaX != 0) {
                        trackMotionScroll((int) deltaX);
                    }
                    mTouchMode = TOUCH_MODE_SCROLL;
                } else {
                    /* Still in over scroll mode. */
                    overScrollBy(-deltaX, 0, mScrollX, 0, 0, 0, mOverscrollDistance, 0, true);
                    invalidate();
                }

                mDirection = newDirection;
                mLastMotionX = x;
                break;
            }
        }
    }

    private void handleTouchUp() {
        switch (mTouchMode) {
            case TOUCH_MODE_DOWN: {
                if (mDownTouchPosition >= 0) {
                    final View child = getChildAt(mDownTouchPosition - mFirstPosition);
                    if (child != null && !child.hasFocusable() && child.isPressed()) {
                        child.setPressed(false);
                    }

                    /*
                     * An item tap should make it selected, so scroll to this
                     * child.
                     */
                    scrollToChild(mDownTouchPosition - mFirstPosition);

                    /*
                     * Also pass the click so the client knows, if it wants to.
                     */
                    if (mShouldCallbackOnUnselectedItemClick
                            || mDownTouchPosition == mSelectedPosition) {
                        performItemClick(mDownTouchView, mDownTouchPosition, mAdapter
                                .getItemId(mDownTouchPosition));
                    }
                }
                break;
            }

            case TOUCH_MODE_SCROLL: {
                if (!mShouldCallbackDuringFling) {
                    /*
                     * We want to suppress selection changes, so remove any
                     * future code to set mSuppressSelectionChanged = false.
                     */
                    removeCallbacks(mDisableSuppressSelectionChangedRunnable);

                    /* This will get reset once we scroll into slots. */
                    if (!mSuppressSelectionChanged)
                        mSuppressSelectionChanged = true;
                }

                final VelocityTracker flingVelocityTracker = mVelocityTracker;
                flingVelocityTracker.computeCurrentVelocity(1000, mMaximumVelocity);
                final int velocityX = (int) flingVelocityTracker.getXVelocity(0);
                if (DBG_MOTION) {
                    Xlog.d(TAG, "Touch up from scroll: velocityX = " + velocityX
                            + ",mMinimumVelocity = " + mMinimumVelocity + ",mVelocityTracker = "
                            + mVelocityTracker);
                }
                /* Fling the bounce gallery. */
                if (Math.abs(velocityX) > mMinimumVelocity) {
                    if (mFlingRunnable == null) {
                        mFlingRunnable = new FlingRunnable();
                    }
                    mFlingRunnable.startUsingVelocity((int) -velocityX);
                } else {
                    mTouchMode = TOUCH_MODE_REST;
                    onUpOrCancel();
                }
                break;
            }

            case TOUCH_MODE_OVERSCROLL: {
                if (mFlingRunnable == null) {
                    mFlingRunnable = new FlingRunnable();
                }

                final VelocityTracker velocityTracker = mVelocityTracker;
                velocityTracker.computeCurrentVelocity(1000, mMaximumVelocity);
                final int initialVelocity = (int) velocityTracker.getXVelocity();

                if (Math.abs(initialVelocity) > mMinimumVelocity) {
                    mFlingRunnable.startOverfling(-initialVelocity);
                } else {
                    mFlingRunnable.startSpringback();
                }
                break;
            }

            case TOUCH_MODE_REST: {
                /*
                 * When touch the list item and press the key, touch mode will
                 * change to TOUCH_MODE_REST.
                 */
                Xlog.d(TAG, "Touch up from TOUCH_MODE_REST: mDownTouchView = " + mDownTouchView);
                if (mDownTouchView != null && mDownTouchView.isPressed()) {
                    mDownTouchView.setPressed(false);
                }
                break;
            }
        }
    }

    @Override
    protected void onOverScrolled(int scrollX, int scrollY, boolean clampedX, boolean clampedY) {
        if (DBG_MOTION) {
            Xlog.d(TAG, "onOverScrolled: scrollX = " + scrollX + ",clampedX = " + clampedX
                    + ",mScrollX = " + mScrollX);
        }
        if (mScrollX != scrollX) {
            mScrollX = scrollX;
            invalidateParentIfNeeded();

            awakenScrollBars();
        }
    }

    /**
     * Set the distance that can be over scrolled.
     *
     * @param overscrollDistance the over scroll distance, should be positive value.
     */
    public void setOverscrollDistance(int overscrollDistance) {
        mOverscrollDistance = overscrollDistance;
    }

    /**
     * Set the distance that can be over fling.
     *
     * @param overflingDistance the over fling distance, should be positive value.
     */
    public void setOverflingDistance(int overflingDistance) {
        mOverflingDistance = overflingDistance;
    }

    /**
     * Set listener called when gallery selection changes.
     *
     * @param listener
     */
    public void setOnSelectionChangeListener(OnSelectionChangeListener listener) {
        mSelectionChangeListener = listener;
    }

    /**
     * Remove runnable from message queue.
     *
     * @param runnable
     */
    private void removeRunnables(Runnable runnable) {
        final Handler handler = getHandler();
        if (DBG) {
            Xlog.d(TAG, "removeRunnables runnable = " + runnable + ",handler = " + handler);
        }
        if (handler != null) {
            handler.removeCallbacks(runnable);
        }
    }

    /**
     * Set overscroll and overfling distance.
     *
     * @param distance
     */
    public void setOverScrollDistance(final int distance) {
        mOverscrollDistance = distance;
        mOverflingDistance = distance;
    }

    private boolean performLongPress(View view, int position, long id) {
        boolean handled = false;

        if (mOnItemLongClickListener != null) {
            handled = mOnItemLongClickListener.onItemLongClick(this, mDownTouchView,
                    mDownTouchPosition, id);
        }

        if (!handled) {
            mContextMenuInfo = new AdapterContextMenuInfo(view, position, id);
            handled = super.showContextMenuForChild(this);
        }

        if (handled) {
            performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
        }

        return handled;
    }

    final class CheckForTap implements Runnable {
        public void run() {
            if (DBG) {
                Xlog.d(TAG, "CheckForTap: mTouchMode = " + mTouchMode + ",mFirstPosition = "
                        + mFirstPosition + ",mDataChanged = " + mDataChanged
                        + ",mDownTouchPosition = " + mDownTouchPosition + ",this = " + this);
            }

            if (mTouchMode == TOUCH_MODE_DOWN) {
                final View child = getChildAt(mDownTouchPosition - mFirstPosition);
                if (DBG) {
                    Xlog.d(TAG, "CheckForTap: child = " + child + ",this = " + this);
                }

                if (child != null && !mDataChanged) {
                    child.setPressed(true);
                    setPressed(true);

                    final int longPressTimeout = ViewConfiguration.getLongPressTimeout();
                    final boolean longClickable = isLongClickable();
                    if (DBG) {
                        Xlog.d(TAG, "CheckForTap longClickable = " + longClickable
                                + ",mFirstPosition = " + mFirstPosition + ",mDownTouchPosition = "
                                + mDownTouchPosition + ",this = " + this);
                    }

                    if (longClickable) {
                        if (mPendingCheckForLongPress == null) {
                            mPendingCheckForLongPress = new CheckForLongPress();
                        }
                        mPendingCheckForLongPress.rememberWindowAttachCount();
                        postDelayed(mPendingCheckForLongPress, longPressTimeout);
                    }
                }
            }
        }
    }

    /**
     * A base class for Runnables that will check that their view is still
     * attached to the original window as when the Runnable was created.
     */
    private class WindowRunnnable {
        private int mOriginalAttachCount;

        public void rememberWindowAttachCount() {
            mOriginalAttachCount = getWindowAttachCount();
        }

        public boolean sameWindow() {
            return hasWindowFocus() && getWindowAttachCount() == mOriginalAttachCount;
        }
    }

    private class CheckForLongPress extends WindowRunnnable implements Runnable {
        public void run() {
            final int motionPosition = mDownTouchPosition;
            final View child = getChildAt(motionPosition - mFirstPosition);
            if (DBG) {
                Xlog.d(TAG, "CheckForLongPress mTouchMode = " + mTouchMode + ",mFirstPosition = "
                        + mFirstPosition + ",mDataChanged = " + mDataChanged
                        + ",mDownTouchPosition = " + mDownTouchPosition + ",this = " + this);
            }

            if (child != null) {
                final int longPressPosition = mDownTouchPosition;
                final long longPressId = mAdapter.getItemId(mDownTouchPosition);

                boolean handled = false;
                if (sameWindow() && !mDataChanged) {
                    handled = performLongPress(child, longPressPosition, longPressId);
                }
                if (handled) {
                    mTouchMode = TOUCH_MODE_REST;
                    setPressed(false);
                    child.setPressed(false);
                }
            }
        }
    }

    /**
     * Responsible for fling behavior. Use {@link #startUsingVelocity(int)} to
     * initiate a fling. Each frame of the fling is handled in {@link #run()}. A
     * FlingRunnable will keep re-posting itself until the fling is done.
     */
    private class FlingRunnable implements Runnable {
        /**
         * Tracks the decay of a fling scroll
         */
        private OverScroller mScroller;

        /**
         * X value reported by mScroller on the previous fling
         */
        private int mLastFlingX;

        private boolean mUsingDistance;

        public FlingRunnable() {
            mScroller = new OverScroller(getContext());
        }

        private void startCommon() {
            // Remove any pending flings
            removeCallbacks(this);
        }

        public void startUsingVelocity(int initialVelocity) {
            mUsingDistance = false;
            if (initialVelocity == 0) return;

            startCommon();
            if (DBG_MOTION) {
                Xlog.d(TAG, "startUsingVelocity: initialVelocity = " + initialVelocity);
            }

            int initialX = initialVelocity < 0 ? Integer.MAX_VALUE : 0;
            mLastFlingX = initialX;
            mTouchMode = TOUCH_MODE_FLING;
            mScroller.fling(initialX, 0, initialVelocity, 0,
                    0, Integer.MAX_VALUE, 0, Integer.MAX_VALUE);
            post(this);
        }

        public void startUsingDistance(int distance) {
            mUsingDistance = true;
            if (distance == 0) return;

            startCommon();

            if (DBG_MOTION) {
                Xlog.d(TAG, "startUsingDistance: distance = " + distance + ",mScrollX = " + mScrollX);
            }
            mLastFlingX = 0;
            mScroller.startScroll(0, 0, -distance, 0, mAnimationDuration);
            mTouchMode = TOUCH_MODE_FLING;
            post(this);
        }

        void startSpringback() {
            mUsingDistance = false;
            if (mScroller.springBack(mScrollX, 0, 0, 0, 0, 0)) {
                mTouchMode = TOUCH_MODE_OVERFLING;
                invalidate();
                post(this);
            } else {
                mTouchMode = TOUCH_MODE_REST;
            }
        }

        void startOverfling(int initialVelocity) {
            mUsingDistance = false;
            final int min = mScrollX > 0 ? Integer.MIN_VALUE : 0;
            final int max = mScrollX > 0 ? 0 : Integer.MAX_VALUE;
            mScroller.fling(mScrollX, 0, initialVelocity, 0, min, max, 0, 0, getWidth(), 0);
            mTouchMode = TOUCH_MODE_OVERFLING;
            if (DBG_MOTION) {
                Xlog.d(TAG, "startOverfling: mScrollX = " + mScrollX + ",initialVelocity = "
                        + initialVelocity + ", min = " + min + ",max = " + max + ",mTouchMode = "
                        + mTouchMode);
            }

            invalidate();
            post(this);
        }

        public void stop(boolean scrollIntoSlots) {
            removeCallbacks(this);
            endFling(scrollIntoSlots);
        }

        private void endFling(boolean scrollIntoSlots) {
            if (DBG_MOTION) {
                Xlog.d(TAG, "endFling: scrollIntoSlots = " + scrollIntoSlots + ",mTouchMode = "
                        + mTouchMode + ",mScrollX = " + mScrollX + ",mVelocityTracker = "
                        + mVelocityTracker);
            }
            /*
             * Force the scroller's status to finished (without setting its
             * position to the end)
             */
            mScroller.forceFinished(true);
            mUsingDistance = false;

            if (scrollIntoSlots) {
                mTouchMode = TOUCH_MODE_REST;
                scrollIntoSlots();
            }
        }

        public void run() {
            if (mItemCount == 0 || getChildCount() == 0) {
                endFling(true);
                return;
            }

            mShouldStopFling = false;

            switch (mTouchMode) {
                case TOUCH_MODE_FLING: {
                    final OverScroller scroller = mScroller;
                    boolean more = scroller.computeScrollOffset();
                    final int x = scroller.getCurrX();

                    /*
                     * Flip sign to convert finger direction to list items
                     * direction (e.g. finger moving down means list is moving
                     * towards the top)
                     */
                    int delta = mLastFlingX - x;

                    if (DBG_MOTION) {
                        Xlog.d(TAG, "Before Fling run: mLastFlingX = " + mLastFlingX + ",x = " + x
                                + ",delta = " + delta + ",more = " + more + ",mFirstPosition = "
                                + mFirstPosition + ",current vel = " + mScroller.getCurrVelocity());
                    }

                    /*
                     * Pretend that each frame of a fling scroll is a touch
                     * scroll.
                     */
                    if (delta > 0) {
                        /*
                         * Moving towards the left. Use first view as
                         * mDownTouchPosition.
                         */
                        mDownTouchPosition = mFirstPosition;

                        /* Don't fling more than 1 screen. */
                        delta = Math.min(getWidth() - mPaddingLeft - mPaddingRight - 1, delta);
                    } else {
                        /*
                         * Moving towards the right. Use last view as
                         * mDownTouchPosition.
                         */
                        int offsetToLast = getChildCount() - 1;
                        mDownTouchPosition = mFirstPosition + offsetToLast;

                        /* Don't fling more than 1 screen. */
                        delta = Math.max(-(getWidth() - mPaddingRight - mPaddingLeft - 1), delta);
                    }

                    trackMotionScroll(delta);

                    if (DBG_MOTION) {
                        Xlog.d(TAG, "Flinging: mScrollX = " + mScrollX + ",mLastFlingX = "
                                + mLastFlingX + ",x = " + x + ",mFirstPosition = " + mFirstPosition
                                + ",mDownTouchPosition = " + mDownTouchPosition + ",delta = "
                                + delta + ",mDistanceLeft = " + mDistanceLeft
                                + ", mNeedOverscroll = " + mNeedOverscroll + ",mUsingDistance = "
                                + mUsingDistance);
                    }

                    if (mNeedOverscroll && !mUsingDistance) {
                        overScrollBy(-mDistanceLeft, 0, mScrollX, 0, 0, 0,
                                mOverflingDistance, 0, false);
                        mNeedOverscroll = false;
                        if (more) {
                            mScroller.notifyHorizontalEdgeReached(mScrollX, 0, mOverflingDistance);
                            mTouchMode = TOUCH_MODE_OVERFLING;
                            if (DBG_MOTION) {
                                Xlog.d(TAG, "Flinging after overscroll: mScrollX = " + mScrollX
                                        + ",mLastFlingX = " + mLastFlingX
                                        + ",mScroller.getCurrVelocity() = "
                                        + mScroller.getCurrVelocity());
                            }
                            invalidate();
                            post(this);
                        }
                        break;
                    }

                    if (DBG_MOTION) {
                        Xlog.d(TAG, "Flinging: mScrollX = " + mScrollX + ",mLastFlingX = "
                                + mLastFlingX + ",x = " + x + ",mShouldStopFling = "
                                + mShouldStopFling + ",more = " + more + ", mNeedOverscroll = "
                                + mNeedOverscroll);
                    }
                    if (more && !mShouldStopFling) {
                        mLastFlingX = x;
                        post(this);
                    } else {
                        endFling(true);
                    }
                    break;
                }

                case TOUCH_MODE_OVERFLING: {
                    final OverScroller scroller = mScroller;

                    if (scroller.computeScrollOffset()) {
                        final int scrollX = mScrollX;
                        final int deltaX = scroller.getCurrX() - scrollX;
                        if (DBG_MOTION) {
                            Xlog.d(TAG, "OverFlinging: mScrollX = " + mScrollX + ",mLastFlingX = "
                                    + mLastFlingX + ",curx = " + scroller.getCurrX()
                                    + ",mFirstPosition = " + mFirstPosition
                                    + ",mDownTouchPosition = " + mDownTouchPosition + ",deltaX = "
                                    + deltaX);
                        }

                        if (overScrollBy(deltaX, 0, scrollX, 0, 0, 0, mOverflingDistance, 0, false)) {
                            Xlog.d(TAG, "OverFlinging: startSpringback: mScrollX = " + mScrollX);
                            startSpringback();
                        } else {
                            invalidate();
                            post(this);
                        }
                    } else {
                        Xlog.d(TAG, "Over fling end here, so we finish the movement: mScrollX = "
                                + mScrollX);
                        endFling(true);
                    }
                    break;
                }

                default:
                    return;
            }
        }
    }

    /**
     * Interface definition for a callback to be invoked when the selection of
     * gallery changes.
     */
    public static interface OnSelectionChangeListener {
        /**
         * Called when the selection changes.
         */
        void onSelectionChanged();
    }
}
