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

package com.mediatek.widget;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.database.DataSetObserver;
import android.graphics.Color;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.BounceGallery;
import android.widget.BounceGallery.OnSelectionChangeListener;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.BounceCoverFlow;


import com.mediatek.internal.R;
import com.mediatek.xlog.Xlog;

import java.util.ArrayList;
import java.util.List;

/**
 *  New added class for new common control BookmarkView.
 * 
 *  A view maintain a list of image, title and detail informations, the image
 *  will be contained in a CoverFlow. When user move or fling the CoverFlow, the 
 *  title and detail information will also be updated.
 *
 *  @hide
 */
public class BookmarkView extends FrameLayout {
    private static final String TAG = "BookmarkView";
    private static final boolean DBG = true;

    private static final float DEFAULT_REFLECTION = 0.25f;
    private static final float DEFAULT_MAX_ZOOM = 400.0f;

    private float mMaxZoom = DEFAULT_MAX_ZOOM;
    private float mImageReflection = DEFAULT_REFLECTION;
    
    private int mInfoColor = Color.WHITE;
    private int mTitleColor = Color.WHITE;
    
    /**
     * Horizontal spacing between items.
     */
    private int mSpaceBetweenItems;
    private int mSpaceBetweenIndicators;

    private int mImageDispWidth;
    private int mImageDispHeight;

    private int mItemCount;
    private int mOldItemCount;
    private int mCurrentSelectedPosition = -1;
    private LayoutInflater mInflater;
    private TextView mTitleView;
    private TextView mInfoView;
    private BounceCoverFlow mCoverflow;
    private LinearLayout mIndicators;
    
    private BookmarkAdapter mBookmarkAdapter;
    /* Bookmark data set observer to watch and handle adapter data change. */
    private BookmarkDataSetObserver mBookmarkDataSetObserver;
    private List<ImageView> mRecycledIndicators = new ArrayList<ImageView>();

    public BookmarkView(Context context) {
        this(context, null);
    }

    public BookmarkView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public BookmarkView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs);
        TypedArray a = context.obtainStyledAttributes(attrs,
                com.mediatek.internal.R.styleable.BookmarkView);
        final Resources resources = context.getResources();
        mImageDispWidth = a.getDimensionPixelSize(com.mediatek.internal.R.styleable.BookmarkView_imageDispWidth,
                resources.getDimensionPixelSize(com.mediatek.internal.R.dimen.bookmark_bitmap_width));
        mImageDispHeight = a.getDimensionPixelSize(com.mediatek.internal.R.styleable.BookmarkView_imageDispHeight,
                resources.getDimensionPixelSize(com.mediatek.internal.R.dimen.bookmark_bitmap_height));

        mSpaceBetweenIndicators = a.getDimensionPixelSize(com.mediatek.internal.R.styleable.BookmarkView_dotGap, 
                resources.getDimensionPixelSize(com.mediatek.internal.R.dimen.bookmark_dot_gap));
        mSpaceBetweenItems = a.getDimensionPixelSize(com.mediatek.internal.R.styleable.BookmarkView_spaceBetweenItems, 
                resources.getDimensionPixelSize(com.mediatek.internal.R.dimen.bookmark_spacing));

        mMaxZoom = a.getFloat(com.mediatek.internal.R.styleable.BookmarkView_maxZoomOut, DEFAULT_MAX_ZOOM);
        mImageReflection = a.getFloat(
                com.mediatek.internal.R.styleable.BookmarkView_imageReflection, DEFAULT_REFLECTION);

        mTitleColor = a.getColor(com.mediatek.internal.R.styleable.BookmarkView_titleColor,
                Color.WHITE);
        mInfoColor = a.getColor(com.mediatek.internal.R.styleable.BookmarkView_infoColor,
                Color.WHITE);

        a.recycle();

        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mInflater.inflate(com.mediatek.internal.R.layout.bookmarkview_layout, this, true);
    }
    
    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        
        mTitleView = (TextView) findViewById(R.id.bookmarkTitle);
        mInfoView = (TextView) findViewById(R.id.bookmarkInfo);

        mCoverflow = (BounceCoverFlow) findViewById(R.id.bookmarkCoverflow);
        mCoverflow.setGravity(Gravity.CENTER_VERTICAL);

        mIndicators = (LinearLayout) findViewById(R.id.bookmarkIndicators);

        mCoverflow.setSpacing(mSpaceBetweenItems);
        mCoverflow.setMaxZoomOut(mMaxZoom);
        mTitleView.setTextColor(mTitleColor);
        mTitleView.setTextColor(mInfoColor);

        mCoverflow.setOnSelectionChangeListener(mSelectionChangeListener);
    }

    /**
     * Set adapter to bookmark view, set the image display size and reflection
     * which used in bookmark adapter.
     * 
     * @param adapter
     *
     * @internal
     */
    public void setBookmarkAdapter(final BookmarkAdapter adapter) {
        if (DBG) {
            Xlog.d(TAG, "setBookmarkAdapter adapter = " + adapter + ",mBookmarkDataSetObserver = "
                    + mBookmarkDataSetObserver);
        }
        if (null != mBookmarkAdapter) {
            mBookmarkAdapter.unregisterDataSetObserver(mBookmarkDataSetObserver);
        }

        mCoverflow.setAdapter(adapter);
        mBookmarkAdapter = adapter;
        if (null != mBookmarkAdapter) {
            mBookmarkDataSetObserver = new BookmarkDataSetObserver();
            mBookmarkAdapter.registerDataSetObserver(mBookmarkDataSetObserver);
            mBookmarkAdapter.setImageDispSize(mImageDispWidth, mImageDispHeight);
            mBookmarkAdapter.setImageReflection(mImageReflection);

            mOldItemCount = mItemCount;
            mItemCount = mBookmarkAdapter.getCount();
        }
    }

    /**
     * Sets the spacing between items in a cover flow.
     * 
     * @param spacing The spacing in pixels between items in the BounceGallery.
     *
     * @internal
     */
    public void setCoverFlowSpacing(final int spacing) {
        mCoverflow.setSpacing(spacing);
    }

    /**
     * Set the max zoom out to transform cover flow images.
     * 
     * @param maxZoomout The max zoom out.
     */
    public void setCoverFlowMaxZoomOut(final float maxZoomout) {
        mCoverflow.setMaxZoomOut(maxZoomout);
    }

    /**
     * Set the displayed size of the image in cover flow.
     * 
     * @param dispWidth the width of the view to display image in cover flow.
     * @param dispHeight the height of the view to display image in cover flow.
     *
     * @internal
     */
    public void setImageDispSize(final int dispWidth, final int dispHeight) {
        mImageDispWidth = dispWidth;
        mImageDispHeight = dispHeight;
        if (null != mBookmarkAdapter) {
            mBookmarkAdapter.setImageDispSize(dispWidth, dispHeight);
        }
    }

    /**
     * Set the reflection ratio of the image displayed in cover flow.
     * 
     * @param reflection
     *
     * @internal
     */
    public void setImageReflection(final float reflection) {
        if (null != mBookmarkAdapter) {
            mBookmarkAdapter.setImageReflection(reflection);
        }
    }

    /**
     * Set overscroll and overfling distance.
     * 
     * @param distance
     */
    public void setGalleryOverScrollDistance(final int distance) {
        mCoverflow.setOverScrollDistance(distance);
    }

    /**
     * Get the cover flow view of this bookmark.
     * 
     * @return
     *
     * @internal
     */
    public BounceCoverFlow getCoverFlow() {
        return mCoverflow;
    }

    /**
     * Get the title view of the bookmark.
     * 
     * @return the title view.
     *
     * @internal
     */
    public TextView getTitleView() {
        return mTitleView;
    }

    /**
     * Get the information view of the bookmark.
     * 
     * @return the information view.
     */
    public TextView getInfoView() {
        return mInfoView;
    }

    /**
     * Get layout of indicators.
     * @return
     */
    public ViewGroup getIndicatorsLayout() {
        return mIndicators;
    }
    
    /**
     * Get item count of bookmark items in adapter.
     * 
     * @return the item count of bookmark items.
     */
    public int getItemCount() {
        return mItemCount;
    }
    
    /**
     * Get current selected position of bookmark.
     * 
     * @return current selected position.
     */
    public int getCurrentPosition() {
        return mCurrentSelectedPosition;
    }

    /**
     * This function used to refresh information, called when the selection
     * changed or user force update, used to refresh the title and info string.
     * 
     * @param force whether force to update.
     */
    public void refreshInfo(boolean force) {
        if (null != mBookmarkAdapter) {
            // Clear title and info text if data invalid.
            if (mBookmarkAdapter.getCount() == 0) {
                Xlog.d(TAG, "refreshInfo and data invalid.");
                mTitleView.setText("");
                mInfoView.setText("");
                mCurrentSelectedPosition = 0;
                return;
            }

            int selectedPos = mCoverflow.getSelectedItemPosition();
            if (DBG) {
                Xlog.d(TAG, "refreshInfo: new selectedPos = " + selectedPos
                        + ", old selected pos = " + mCurrentSelectedPosition + ",force = " + force);
            }
            if (force || mCurrentSelectedPosition != selectedPos) {
                final BookmarkItem item = (BookmarkItem) mBookmarkAdapter.getItem(selectedPos);
                mTitleView.setText(item.mTitle);
                mInfoView.setText(item.mInfo);
                mCurrentSelectedPosition = selectedPos;
            }
        }
    }

    /**
     * Refresh indicators, add or remove indicator view from indicators layout,
     * change image resource of the indicators.
     */
    private void refreshIndicators() {
        int indicatorCnt = mIndicators.getChildCount();
        int realCount = indicatorCnt;
        if (indicatorCnt > mItemCount) {
            realCount = mItemCount;
            ImageView recycleView = null;
            for (int i = indicatorCnt - 1; i >= mItemCount; i--) {
                recycleView = (ImageView) mIndicators.getChildAt(i);
                mIndicators.removeViewAt(i);
                
                // Request recycled view to re-layout.
                recycleView.forceLayout();
                recycleView.dispatchStartTemporaryDetach();                
                mRecycledIndicators.add(recycleView);
            }
        } else if (indicatorCnt < mItemCount) {
            ImageView indicator = null;
            for (int i = indicatorCnt; i < mItemCount; i++) {                
                if (mRecycledIndicators.size() > 0) {
                    indicator = mRecycledIndicators.get(0);
                } else {
                    indicator = null;
                }

                if (null != indicator) {
                    // Get view from recycled list.
                    if (DBG) {
                        Xlog.d(TAG, "Get indicator from recycled list:indicator = " + indicator);
                    }
                    mRecycledIndicators.remove(0);
                } else {
                    if (DBG) {
                        Xlog.d(TAG, "Get indicator by inflating layout resource:indicator = "
                                + indicator);
                    }
                    indicator = (ImageView) mInflater.inflate(R.layout.bookmarkview_indicator_layout, null,
                            false);
                }
                indicator.setPadding(0, 0, mSpaceBetweenIndicators, 0);
                mIndicators.addView(indicator);
            }
        }
        if (DBG) {
            Xlog.d(TAG, "refreshIndicators indicatorCnt = " + indicatorCnt + ",mItemCount = "
                    + mItemCount + ",mIndicators = " + mIndicators + ",mCurrentSelectedPosition = "
                    + mCurrentSelectedPosition + ",realCount = " + realCount
                    + ",mRecycledIndicators.size = " + mRecycledIndicators.size());
        }

        // TODO: shall we refresh indicators only when current selected item changed.
        // Set all indicator to non-present.
        for (int i = 0; i < realCount; i++) {
            ((ImageView) mIndicators.getChildAt(i)).getDrawable().setLevel(0);
        }

        // Set the current indicator resource to present.
        if (mIndicators.getChildCount() > mCurrentSelectedPosition) {
            ((ImageView) mIndicators.getChildAt(mCurrentSelectedPosition)).getDrawable().setLevel(1);
        }
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (DBG) {
            Xlog.d(TAG, "onAttachedToWindow:mBookmarkAdapter = " + mBookmarkAdapter
                    + ",mBookmarkDataSetObserver = " + mBookmarkDataSetObserver);
        }        
        if (null != mBookmarkAdapter && null == mBookmarkDataSetObserver) {
            mBookmarkDataSetObserver = new BookmarkDataSetObserver();
            mBookmarkAdapter.registerDataSetObserver(mBookmarkDataSetObserver);

            // Data may have changed while we were detached, update it.
            mOldItemCount = mItemCount;
            mItemCount = mBookmarkAdapter.getCount();
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (DBG) {
            Xlog.d(TAG, "onAttachedToWindow:mBookmarkAdapter = " + mBookmarkAdapter
                    + ",mRecycledIndicators.size = " + mRecycledIndicators.size());
        }
        // Detach all views remain in the recycled list.
        mRecycledIndicators.clear();

        if (null != mBookmarkAdapter) {
            mBookmarkAdapter.unregisterDataSetObserver(mBookmarkDataSetObserver);
            mBookmarkDataSetObserver = null;
        }
    }

    private final BounceGallery.OnSelectionChangeListener mSelectionChangeListener = new OnSelectionChangeListener() {
        public void onSelectionChanged() {
            if (DBG) {
                Xlog.d(TAG, "BounceGallery selection changed.");
            }
            refreshInfo(false);
            refreshIndicators();
        }
    };

    final class BookmarkDataSetObserver extends DataSetObserver {
        @Override
        public void onChanged() {
            mBookmarkAdapter.clearBitmapCache();
            mOldItemCount = mItemCount;
            mItemCount = mBookmarkAdapter.getCount();
            if (DBG) {
                Xlog.d(TAG, "Bookmark data changes: mItemCount = " + mItemCount + ",mOldItemCount = "
                        + mOldItemCount + ",mCurrentSelectedPosition = " + mCurrentSelectedPosition);
            }

            // If the current selected position exceed the total count, set it
            // to the last one.
            if (mCurrentSelectedPosition > mItemCount - 1) {
                mCoverflow.setNextSelectedPositionInt(mItemCount - 1);
            }

            /*
             * Since maybe only bookmark item like title change, we need to
             * force refresh informations even the number of data didn't change.
             */
            refreshInfo(true);
            refreshIndicators();
        }

        @Override
        public void onInvalidated() {
            if (DBG) {
                Xlog.d(TAG, "Bookmark data invalidate:mItemCount = " + mItemCount
                        + ",mOldItemCount = " + mOldItemCount);
            }
            mBookmarkAdapter.clearBitmapCache();
            refreshInfo(true);
            refreshIndicators();
        }
    }
}
