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
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PorterDuffXfermode;
import android.graphics.Bitmap.Config;
import android.graphics.PorterDuff.Mode;
import android.graphics.Shader.TileMode;
import android.graphics.drawable.Drawable;
import android.util.LongSparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.FrameLayout;
import android.widget.Gallery;

import com.mediatek.xlog.Xlog;
import com.mediatek.internal.R;

import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * New added class for new common control BookmarkView.
 * 
 * It is a special adapter that implemented only for BookmarkView, maintains the
 * title, detail information and image content of each bookmark item, the image
 * will be scaled with reflection when getView. User can add or delete item by
 * calling add()/remove APIs, bookmark view will handle the update issue. 
 * 
 * If you want a delete icon for convenience, you can just call setDeleteMode(true) 
 * to set the adapter in delete mode.
 *
 * @hide
 */
public class BookmarkAdapter extends BaseAdapter {
    private static final String TAG = "BookmarkAdapter";
    private static final boolean DBG = true;

    private static final int DEFAULT_DISPLAY_WIDTH = 120;
    private static final int DEFAULT_DISPLAY_HEIGHT = 160;

    private static final float DEFAULT_REFLECTION = 0.25f;
    private static final int DEFAULT_REFLECTION_GAP = 4;
    private static final int DELICON_MARGIN_SIZE = 10;

    // TODO: Use fixed size seems less flexible and the image may be cut,
    // consider using fill_parent.
    private int mDispWidth = DEFAULT_DISPLAY_WIDTH;
    private int mDispHeight = DEFAULT_DISPLAY_HEIGHT;
    private float mReflection = DEFAULT_REFLECTION;
    
    private Matrix mTransMatrix = new Matrix();
    
    private Paint mNormalPaint = new Paint();
    private Paint mGradientPaint = new Paint();

    private Context mContext;
    private LayoutInflater mInflater;

    /**
     * Store the total height of the child, equals <b>mDispHeight * (1 + mReflection)</b>, 
     * use it to save time instead calculating it in getView every time.
     */
    private int mTotalDispHeight;

    // Whether to add delete icon at the top-right corner.
    private boolean mDeleteMode = false;
    private Drawable mDeleteDrawable;

    private final Object mLock = new Object();

    private List<BookmarkItem> mBookmarkItems;
    private final LongSparseArray<WeakReference<Bitmap>> mBitmapCache = new LongSparseArray<WeakReference<Bitmap>>();

    public BookmarkAdapter(Context context, List<BookmarkItem> bookmarkItems) {
        mContext = context;
        mBookmarkItems = bookmarkItems;
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;

        if (convertView != null) {
            holder = (ViewHolder) convertView.getTag();
        } else {
            convertView = mInflater.inflate(R.layout.coverflow_item_layout, null, false);
            holder = new ViewHolder();
            holder.container = (FrameLayout) convertView.findViewById(R.id.containerView);
            holder.content = (ImageView) convertView.findViewById(R.id.contentView);
            holder.delIcon = (ImageView) convertView.findViewById(R.id.deleteView);
            convertView.setTag(holder);
        }

        loadImage(holder.content, position);
        if (mDeleteMode) {
            // This may lead bitmap exceed the control, seems like image cut,
            // user need to fix the size issue.
            holder.container.setLayoutParams(new Gallery.LayoutParams(mDispWidth
                    + DELICON_MARGIN_SIZE, mTotalDispHeight + DELICON_MARGIN_SIZE));
            holder.delIcon.setVisibility(View.VISIBLE);
            holder.delIcon.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    BookmarkAdapter.this.remove(position);
                }
            });
        } else {
            holder.container.setLayoutParams(
                    new Gallery.LayoutParams(mDispWidth, mTotalDispHeight));
            holder.delIcon.setVisibility(View.INVISIBLE);
            holder.delIcon.setOnClickListener(null);
        }

        return convertView;
    }

    /**
     * We use a weak reference cache to save bitmap instances of each position
     * instead of load it every time we getView to save time, we also decode
     * bitmap from image resource only once, then we will store the reference
     * for next time.
     * 
     * @param child
     * @param position
     */
    private void loadImage(ImageView child, int position) {
        Bitmap cacheBitmap = getCachedBitmap(mBitmapCache, position);
        if (DBG) {
            Xlog.d(TAG, "loadImage: position = " + position + ",cacheBitmap = " + cacheBitmap);
        }
        if (cacheBitmap != null) {
            child.setImageBitmap(cacheBitmap);
            return;
        }
        
        final BookmarkItem item = mBookmarkItems.get(position);
        // Use the content image first, only need to decode image from resource
        // id once.
        if (item.mContent == null && item.mResId != -1) {
            item.mContent = BitmapFactory.decodeResource(mContext.getResources(), item.mResId);
        }

        Bitmap originalImage = item.mContent;
        if (originalImage == null) {
            Xlog.e(TAG, "No image valid for bookmark item " + position);
            return;
        }

        final int width = originalImage.getWidth();
        final int height = originalImage.getHeight();
        if (DBG) {
            Xlog.d(TAG, "loadImage: position = " + position + ",width = " + width + ",height = "
                    + height + ",mDispWidth = " + mDispWidth + ",mDispHeight = " + mDispHeight);
        }

        // Create a bitmap by scaling the origin image to fit the view size.
        mTransMatrix.reset();
        mTransMatrix.postScale((float) mDispWidth / width, (float) mDispHeight / height);
        
        // The scaledBitmap is the same with originalImage, so we won't recycle
        // it, application should recycle the originalImage indeed.
        Bitmap scaledBitmap = Bitmap.createBitmap(originalImage, 0, 0, 
                width, height, mTransMatrix, true);

        // Whether to draw reflection, float variable should not use "=".
        if (mReflection > 0.0f || mReflection < 0.0f) {
            mTransMatrix.reset();
            mTransMatrix.preScale(1, -1);

            Bitmap reflectedBitmap = Bitmap.createBitmap(scaledBitmap, 0,
                    (int) (mDispHeight * (1 - mReflection)), mDispWidth,
                    (int) (mDispHeight * mReflection), mTransMatrix, false);
            Bitmap bitmapWithReflection = Bitmap.createBitmap(mDispWidth, mTotalDispHeight,
                    Config.ARGB_8888);

            Canvas canvas = new Canvas(bitmapWithReflection);
            // Draw the origin bitmap.
            canvas.drawBitmap(scaledBitmap, 0, 0, null);
            // Draw a rectangle to separate the origin bitmap and the reflection bitmap.          
            canvas.drawRect(0, mDispHeight, mDispWidth, mDispHeight + DEFAULT_REFLECTION_GAP,
                    mNormalPaint);
            // Draw reflection bitmap.
            canvas.drawBitmap(reflectedBitmap, 0, mDispHeight + DEFAULT_REFLECTION_GAP, null);

            canvas.drawRect(0, mDispHeight, mDispWidth, mTotalDispHeight + DEFAULT_REFLECTION_GAP,
                    mGradientPaint);
            child.setImageBitmap(bitmapWithReflection);

            synchronized (mLock) {
                if (DBG) {
                    Xlog.d(TAG, "loadImage cache reflection bitmap: position = " + position
                            + ",bitmapWithReflection = " + bitmapWithReflection);
                }
                mBitmapCache.put(position, new WeakReference<Bitmap>(bitmapWithReflection));
            }

            // Recycle reflected bitmap.
            reflectedBitmap.recycle();
            reflectedBitmap = null;
        } else {
            child.setImageBitmap(scaledBitmap);
            synchronized (mLock) {
                if (DBG) {
                    Xlog.d(TAG, "loadImage cache scaled bitmap: position = " + position
                            + ",scaledBitmap = " + scaledBitmap);
                }
                mBitmapCache.put(position, new WeakReference<Bitmap>(scaledBitmap));
            }
        }
    }   

    @Override
    public int getCount() {
        if (mBookmarkItems != null) {
            return mBookmarkItems.size();
        }
        return 0;
    }

    @Override
    public Object getItem(int position) {
        if (mBookmarkItems != null) {
            return mBookmarkItems.get(position);
        }
        return null;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    /**
     * Adds the specified object at the end of the array.
     * 
     * @param item The object to add at the end of the array.
     */
    public void add(BookmarkItem item) {
        mBookmarkItems.add(item);
        notifyDataSetChanged();
    }

    /**
     * Inserts the specified bookmark item at the specified index in the array.
     * 
     * @param item The bookmark item to insert into the array.
     * @param index The index at which the object must be inserted.
     */
    public void insert(BookmarkItem item, int index) {
        mBookmarkItems.add(index, item);
        notifyDataSetChanged();
    }

    /**
     * Replace the specified bookmark item at the specified index using the
     * given item in the array.
     * 
     * @param item the item to be set to index.
     * @param index the index should not be negative value, or else exception will happens.
     * @return the replaced item.
     */
    public BookmarkItem replace(BookmarkItem item, int index) {
        BookmarkItem replaced = null;
        if (item != null && mBookmarkItems.size() > index) {
            replaced = mBookmarkItems.get(index);
            mBookmarkItems.set(index, item);
            notifyDataSetChanged();
        }
        return replaced;
    }

    /**
     * Removes the specified object from the array.
     * 
     * @param item the object to remove.
     * @return whether the remove operation successful.
     */
    public boolean remove(BookmarkItem item) {
        boolean succ = mBookmarkItems.remove(item);
        if (succ) {
            notifyDataSetChanged();
        }
        return succ;
    }

    /**Removes the specified object from the array.
     * 
     * @param index the index of the object to remove.
     * @return the removed object.
     */
    public Object remove(int index) {
        Object removed = mBookmarkItems.remove(index);
        if (removed != null) {
            notifyDataSetChanged();
        }
        return removed;
    }

    /**
     * Remove all elements from the list.
     */
    public void clear() {
        mBookmarkItems.clear();
        notifyDataSetInvalidated();
    }

    /**
     * Sorts the content of this adapter using the specified comparator.
     * 
     * @param comparator The comparator used to sort the objects contained in
     *            this adapter.
     */
    public void sort(Comparator<BookmarkItem> comparator) {
        Collections.sort(mBookmarkItems, comparator);
        notifyDataSetChanged();
    }
    
    /**
     * Returns the context associated with this array adapter. The context is
     * used to create views from the resource passed to the constructor.
     * 
     * @return The Context associated with this adapter.
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Set whether the delete icon is needed.
     * 
     * @param delMode
     */
    public void setDeleteMode(boolean delMode) {
        mDeleteMode = delMode;
    }

    /**
     * Get whether the adapter is in delete mode.
     * 
     * @return
     */
    public boolean isDeleteMode() {
        return mDeleteMode;
    }   

    /**
     * Set the delete icon drawable source.
     * 
     * @param deleteIcon
     */
    public void setDeleteIcon(final Drawable deleteIcon) {
        mDeleteDrawable = deleteIcon;
    }
    
    /**
     * When data of bookmark adapter changes, the bitmap cache need to be
     * cleared. It is also highly recommend application call this to destroy
     * bitmap cache when onDestroy().
     */
    public void clearBitmapCache() {
        synchronized (mLock) {
            final int size = mBitmapCache.size();
            if (DBG) {
                Xlog.d(TAG, "clearBitmapCache: size = " + size);
            }
            Bitmap bmp = null;
            for (int i = 0; i < size; i++) {
                bmp = mBitmapCache.valueAt(i).get();
                if (DBG) {
                    Xlog.d(TAG, "clearBitmapCache: i = " + i + ",bmp = " + bmp + ",recycled = "
                            + ((bmp != null) ? bmp.isRecycled() : true));
                }
                if (bmp != null && bmp.isRecycled()) {
                    bmp.recycle();
                }
            }
            mBitmapCache.clear();
        }        
    }
    
    /**
     * Recycle bitmap contents in bookmark items. Application should call this
     * function to recycle bitmaps if bookmark items are initialized with
     * resource id instead of bitmap content.
     */
    public void recycleBitmapContents() {
        if (mBookmarkItems != null) {
            final int size = mBookmarkItems.size();
            BookmarkItem item = null;
            for (int i = 0; i < size; i++) {
                item = mBookmarkItems.get(i);
                if (item.mContent != null && !item.mContent.isRecycled()) {
                    item.mContent.recycle();
                }
            }
        }
    }
    
    /**
     * Set the displayed size of the image.
     * 
     * @param width the width of the view to display image.
     * @param height the height of the view to display image.
     *
     * @internal
     */
    public void setImageDispSize(final int width, final int height) {
        mDispWidth = width;
        mDispHeight = height;
        mTotalDispHeight = (int) (mDispHeight * (1 + mReflection));

        // Create a linear gradient shader to implement transition effect.
        final LinearGradient shader = new LinearGradient(0, mDispHeight, 0, mTotalDispHeight
                + DEFAULT_REFLECTION_GAP, 0x70ffffff, 0x00ffffff, TileMode.CLAMP);
        mGradientPaint.setShader(shader);

        // Set the Xfermode.
        mGradientPaint.setXfermode(new PorterDuffXfermode(Mode.DST_IN));
    }

    /**
     * Set image reflection rate shows below the origin image.
     * 
     * @param reflect
     *
     * @internal
     */
    public void setImageReflection(final float reflect) {
        mReflection = reflect;
        mTotalDispHeight = (int) (mDispHeight * (1 + mReflection));
    } 

    /**
     * Get bitmap from bitmap cache.
     * 
     * @param bmpCache
     * @param key
     * @return
     */
    private Bitmap getCachedBitmap(final LongSparseArray<WeakReference<Bitmap>> bmpCache, final long key) {
        synchronized (mLock) {
            final WeakReference<Bitmap> wr = bmpCache.get(key);
            if (wr != null) { // We have the key.
                Bitmap entry = wr.get();
                if (entry != null) {
                    return entry;
                } else { // Our entry has been purged.
                    bmpCache.delete(key);
                }
            }
        }
        return null;
    }

    private void dumpBitmapCacheInfo() {
        synchronized (mLock) {
            final int size = mBitmapCache.size();
            Xlog.d(TAG, "dumpBitmapCacheInfo: size = " + size);
            Bitmap bmp = null;
            for (int i = 0; i < size; i++) {
                bmp = mBitmapCache.valueAt(i).get();
                Xlog.d(TAG, "dumpBitmapCacheInfo: i = " + i + ",bmp = " + bmp + ",recycled = "
                        + ((bmp != null) ? bmp.isRecycled() : true));
            }
        }
    }

    static class ViewHolder {
        FrameLayout container;
        ImageView content;
        ImageView delIcon;
    }
}
