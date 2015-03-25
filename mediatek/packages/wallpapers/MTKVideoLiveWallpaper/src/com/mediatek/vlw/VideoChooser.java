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

package com.mediatek.vlw;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import com.mediatek.widget.BookmarkAdapter;
import com.mediatek.widget.BookmarkItem;
import com.mediatek.widget.BookmarkView;
import android.widget.BounceCoverFlow;

import com.mediatek.xlog.Xlog;

import java.util.ArrayList;

public class VideoChooser extends Activity {
    private static final String TAG = "VideoChooser";
    
    static final float DEFAULT_SCALE = 1.0f;
    private static final boolean DEBUG = false;
    private static final int MAX_VIDEO_COUNT = 4;
    
    private static final float COVER_MULTIPLE_IMAGE = 0.25f;
    private static final float COVER_EXTEND_IMAGE = 0.5f;
    private static final float IMAGE_REFLECTION = 0.25f;
    private static final float MAX_ZOOM_OUT = 350;
    
    private static final String SCHEMA = "android.resource://";
    private static final String PATH = "/raw/";

    private final Object mLock = new Object();
    private ArrayList<Uri> mUris;

    private BookmarkView mBookmark;
    private ArrayList<BookmarkItem> mBookmarkItems;
    private BookmarkAdapter mAdapter;
    
    private int mSelectedPos;
    private int mImgWidth;
    private int mImgHeight;
    private int mThumbIndex;
    
    // loading thumb of video
    private final AsyncTask<Void, Integer, Void> mTask = new AsyncTask<Void, Integer, Void>() {

        @Override
        protected Void doInBackground(Void... params) {
            int index = 0;
            Bitmap bmp = null;
            String title = null;
            
            for (Uri uri : mUris) {
                if (isCancelled()) {
                    break;
                }
                BookmarkItem item = mBookmarkItems.get(index);
                MediaMetadataRetriever retriever = new MediaMetadataRetriever();
                try {
                    retriever.setDataSource(VideoChooser.this, uri);
                    bmp = retriever.getFrameAtTime();
                    title = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
                    Xlog.i(TAG, "doInBackground: " + index + ", uri: " + uri
                            + ", bmp: " + bmp + ", title: " + title);
                    if (bmp != null) {
                        Bitmap scaledBitmap = createVideoThumbnail(bmp,
                                mImgWidth, mImgHeight);
                        item.setContentBitmap(scaledBitmap);
                    }
                } catch (IllegalArgumentException ex) {
                    // Assume this is a corrupt video file
                    Xlog.e(TAG, "error: create video thumbnails failed", ex);
                } finally {
                    retriever.release();
                }

                if (title == null) {
                    title = uri.getLastPathSegment();
                    item.setTitleString(title);
                }
                if (bmp != null || title != null) {
                    publishProgress(index);
                }

                ++index;
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            // Update current retrieved video thumbnai index
            mThumbIndex = values[0];
            mAdapter.notifyDataSetChanged();
        }

        @Override
        protected void onPostExecute(Void result) {
            mBookmark.getCoverFlow().setSelection(mSelectedPos);
            super.onPostExecute(result);
        }

    };

    private final OnItemSelectedListener mSelectedListener = new OnItemSelectedListener() {

        @Override
        public void onItemSelected(AdapterView<?> parent, View view,
                int position, long id) {
            mSelectedPos = position;
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
            // Indicate nothing selected when we click the "set" button and
            // return.
            mSelectedPos = -1;
        }

    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.chooser);
        initViews();
    }

    @Override
    protected void onDestroy() {
        mTask.cancel(true);
        
        Bitmap bmp = null;
        for (BookmarkItem item : mBookmarkItems) {
            bmp = item.getContentBitmap();
            if (bmp != null) {
                bmp.recycle();
            }
        }
        super.onDestroy();
    }

    public int getRetrievedThumbIndex() {
        return mThumbIndex;
    }
    
    private void initViews() {
        mUris = new ArrayList<Uri>();
        mBookmarkItems = new ArrayList<BookmarkItem>();

        mBookmark = (BookmarkView) findViewById(R.id.bookmark);
        mImgWidth = getResources().getDimensionPixelSize(R.dimen.thumb_disp_width);
        mImgHeight = getResources().getDimensionPixelSize(R.dimen.thumb_disp_height);
        
        // After this, mUri.size() is the total videos count.
        findVideo();
        mAdapter = new BookmarkAdapter(this, mBookmarkItems);
        mBookmark.setBookmarkAdapter(mAdapter);
        mBookmark.getTitleView().setTextSize(
                getResources().getDimensionPixelSize(R.dimen.title_size));
        mBookmark.setImageDispSize(mImgWidth, mImgHeight);
        mBookmark.setCoverFlowSpacing(
                -(int) (COVER_MULTIPLE_IMAGE * mImgWidth + COVER_EXTEND_IMAGE));
        mBookmark.setImageReflection(IMAGE_REFLECTION);
        
        BounceCoverFlow gallery = mBookmark.getCoverFlow();
        gallery.setSelection(mSelectedPos);
        gallery.setEmptyView(null);
        gallery.setMaxZoomOut(MAX_ZOOM_OUT);
        gallery.setOnItemSelectedListener(mSelectedListener);

        // Update video frame thumbnails.
        mTask.execute();
    }

    /**
     * Get the thumbnail of a video.
     */
    private Bitmap createVideoThumbnail(Bitmap bm, int width, int height) {
        if (bm == null) {
            return bm;
        }

        bm.setDensity(DisplayMetrics.DENSITY_DEVICE);

        // This is the final bitmap we want to return.
        // XXX We should get the pixel depth from the system (to match the
        // physical display depth), when there is a way.
        Bitmap newbm = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565);
        newbm.setDensity(DisplayMetrics.DENSITY_DEVICE);
        Canvas c = new Canvas(newbm);
        c.setDensity(DisplayMetrics.DENSITY_DEVICE);
        Rect targetRect = new Rect();
        targetRect.left = 0;
        targetRect.top = 0;
        targetRect.right = bm.getWidth();
        targetRect.bottom = bm.getHeight();

        int deltaw = width - targetRect.right;
        int deltah = height - targetRect.bottom;
        if (DEBUG) {
            Xlog.i(TAG, "deltaw: " + deltaw + ", deltah: " + deltah + ", width: " + width + ", height: " + height
                    + ", targetRect.right: " + targetRect.right + ", targetRect.bottom: " + targetRect.bottom);
        }

            // We need to scale up so it covers the entire area.
        float scale = DEFAULT_SCALE;
        if (deltaw > 0 || deltah > 0) {
            if (deltaw > deltah) {
                scale = width / (float) targetRect.right;
            } else {
                scale = height / (float) targetRect.bottom;
            }
        } else {
            if (Math.abs(deltaw) < Math.abs(deltah)) {
                scale = width / (float) targetRect.right;
            } else {
                scale = height / (float) targetRect.bottom;
            }
        }
        
        targetRect.right = (int) (targetRect.right * scale);
        targetRect.bottom = (int) (targetRect.bottom * scale);
        deltaw = width - targetRect.right;
        deltah = height - targetRect.bottom;

        targetRect.offset(deltaw / 2, deltah / 2);
        Paint paint = new Paint();
        paint.setFilterBitmap(false);
        paint.setDither(true);
        c.drawBitmap(bm, null, targetRect, paint);

        bm.recycle();
        return newbm;
    }

    private void findVideo() {
        final Resources resources = getResources();
        final String packageName = getPackageName();

        addVideo(resources, packageName, R.array.default_video);
        addVideo(resources, packageName, R.array.extra_video);
        
        // Init bookmark view widget.
        Bitmap bm = BitmapFactory.decodeResource(getResources(),
                R.drawable.default_video_thumb);
        String title = getResources().getString(R.string.default_video_title);
        int size = mUris.size();
        for (int i = 0; i < size; i++) {
            String info = String.valueOf(i + 1) + "/" + String.valueOf(size);
            BookmarkItem item = new BookmarkItem(bm, title, info);
            mBookmarkItems.add(item);
        }
    }

    private void addVideo(Resources resources, String packageName, int list) {
        final String[] extras = resources.getStringArray(list);
        Uri uri = null;
        String uriString = SCHEMA + getPackageName() + PATH;
        String tmp = null;

        for (String extra : extras) {
            if (extra != null) {
                tmp = uriString.concat(extra);
                uri = Uri.parse(tmp);
                mUris.add(uri);
                if (DEBUG) {
                    Xlog.i(TAG, "add default video uri: " + uri);
                }
            }
        }
    }

    // button hook
    @SuppressWarnings({ "UnusedDeclaration" })
    public void selectVideo(View v) {
        Intent result = new Intent();

        Uri uri = mUris.get(mSelectedPos);
        result.setData(uri);
        setResult(RESULT_OK, result);
        mTask.cancel(true);
        finish();
    }
}
