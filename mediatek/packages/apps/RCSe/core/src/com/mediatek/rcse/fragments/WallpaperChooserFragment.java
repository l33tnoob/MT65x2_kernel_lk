/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.fragments;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Gallery;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.SpinnerAdapter;

import com.mediatek.rcse.api.Logger;
import com.orangelabs.rcs.R;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import java.util.ArrayList;

public class WallpaperChooserFragment extends DialogFragment implements
        AdapterView.OnItemSelectedListener, AdapterView.OnItemClickListener {
    private static final String TAG = "WallpaperChooserDialogFragment";

    private Bitmap mBitmap = null;

    private ArrayList<Integer> mThumbs;
    private ArrayList<Integer> mImages;
    private WallpaperLoader mLoader;
    private RCSeWallpaperDrawable mWallpaperDrawable = new RCSeWallpaperDrawable();

    public static WallpaperChooserFragment newInstance() {
        WallpaperChooserFragment fragment = new WallpaperChooserFragment();
        fragment.setCancelable(true);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    private void cancelLoader() {
        if (mLoader != null && mLoader.getStatus() != WallpaperLoader.Status.FINISHED) {
            mLoader.cancel(true);
            mLoader = null;
        }
    }

    @Override
    public void onDetach() {
        super.onDetach();
        Logger.d(TAG, "onDetach.");
        cancelLoader();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Logger.d(TAG, "onDestroy: mLoader = " + mLoader + ", this = " + this);
        cancelLoader();
    }

    @Override
    public void onDismiss(DialogInterface dialog) {
        super.onDismiss(dialog);
        Activity activity = getActivity();
        Logger.d(TAG, "onDismiss() : activity = " + activity + ", dialog = " + dialog);
        if (activity != null) {
            activity.finish();
        }
    }

    /*
     * This will only be called when in XLarge mode, since this Fragment is
     * invoked like a dialog in that mode
     */
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Logger.d(TAG, "onCreateDialog: savedInstanceState = " + savedInstanceState);
        findWallpapers();

        return null;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Logger.d(TAG, "onCreateView: container = " + container);
        findWallpapers();

        View view = inflater.inflate(R.layout.wallpaper_chooser_fragment, container, false);
        view.setBackgroundDrawable(mWallpaperDrawable);

        final Gallery gallery = (Gallery) view.findViewById(R.id.gallery);
        gallery.setCallbackDuringFling(false);
        gallery.setOnItemSelectedListener(this);
        gallery.setAdapter(new ImageAdapter(getActivity()));

        final View setButton = view.findViewById(R.id.set);
        setButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                selectWallpaper(gallery.getSelectedItemPosition());
            }
        });
        return view;
    }

    private void selectWallpaper(int position) {
        Logger.d(TAG, "selectWallpaper: position = " + position + ", this = " + this);
        RcsSettings.getInstance().setChatWallpaper(String.valueOf((mImages.get(position))));
        Activity activity = getActivity();
        activity.finish();
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Logger.d(TAG, "onItemClick");
        selectWallpaper(position);
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        Logger.d(TAG, "onItemSelected");
        if (mLoader != null && mLoader.getStatus() != WallpaperLoader.Status.FINISHED) {
            mLoader.cancel();
        }
        mLoader = (WallpaperLoader) new WallpaperLoader().execute(position);
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        Logger.d(TAG, "onNothingSelected");
    }

    private void findWallpapers() {
        Logger.d(TAG, "findWallpapers()");
        final Resources resources = getResources();
        final String packageName = resources.getResourcePackageName(R.array.wallpapers_resid);
        addWallpapers(resources, packageName, R.array.wallpapers_resid);
    }

    private void addWallpapers(Resources resources, String packageName, int arryId) {
        Logger.d(TAG, "addWallpapers()");
        final String[] extras = resources.getStringArray(arryId);
        final int length = extras.length;
        mThumbs = new ArrayList<Integer>(length);
        mImages = new ArrayList<Integer>(length);
        for (String extra : extras) {
            int res = resources.getIdentifier(extra, "drawable", packageName);
            if (res != 0) {
                final int thumbRes = resources.getIdentifier(extra + "_small", "drawable",
                        packageName);
                if (thumbRes != 0) {
                    mThumbs.add(thumbRes);
                    mImages.add(res);
                    Logger.d(TAG, "add: [" + packageName + "]: " + extra + " (" + res + ","
                            + thumbRes + ")");
                }
            }
        }
    }

    private class ImageAdapter extends BaseAdapter implements ListAdapter, SpinnerAdapter {
        private LayoutInflater mLayoutInflater;

        ImageAdapter(Activity activity) {
            mLayoutInflater = activity.getLayoutInflater();
        }

        public int getCount() {
            return mThumbs.size();
        }

        public Object getItem(int position) {
            return position;
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            View view;

            if (convertView == null) {
                view = mLayoutInflater.inflate(R.layout.wallpaper_item, parent, false);
            } else {
                view = convertView;
            }

            ImageView image = (ImageView) view.findViewById(R.id.wallpaper_image);

            int thumbRes = mThumbs.get(position);
            image.setImageResource(thumbRes);
            Drawable thumbDrawable = image.getDrawable();
            if (thumbDrawable != null) {
                thumbDrawable.setDither(true);
            } else {
                Log.e(TAG, "Error decoding thumbnail resId=" + thumbRes + " for wallpaper #"
                        + position);
            }

            return view;
        }
    }

    class WallpaperLoader extends AsyncTask<Integer, Void, Bitmap> {
        BitmapFactory.Options mOptions;

        WallpaperLoader() {
            mOptions = new BitmapFactory.Options();
            mOptions.inDither = false;
            mOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
        }

        @Override
        protected Bitmap doInBackground(Integer... params) {
            if (isCancelled() || !isAdded()) {
                // If the fragment is not added(attached) to an activity, return
                // null.
                return null;
            }
            try {
                return BitmapFactory.decodeResource(getResources(), mImages.get(params[0]),
                        mOptions);
            } catch (OutOfMemoryError e) {
                Logger.e(TAG, "WallpaperLoader decode resource out of memory " + e.getMessage());
                return null;
            }
        }

        @Override
        protected void onPostExecute(Bitmap b) {
            if (b == null) {
                return;
            }

            if (!isCancelled() && !mOptions.mCancel) {
                // Help the GC
                if (mBitmap != null) {
                    mBitmap.recycle();
                }

                View v = getView();
                if (v != null) {
                    mBitmap = b;
                    mWallpaperDrawable.setBitmap(b);
                    v.postInvalidate();
                } else {
                    mBitmap = null;
                    mWallpaperDrawable.setBitmap(null);
                }
                mLoader = null;
            } else {
                b.recycle();
            }
        }

        void cancel() {
            mOptions.requestCancelDecode();
            super.cancel(true);
        }
    }

    /**
     * Custom drawable that centers the bitmap fed to it.
     */
    static class RCSeWallpaperDrawable extends Drawable {

        Bitmap mBitmap;
        int mIntrinsicWidth;
        int mIntrinsicHeight;

        /* package */void setBitmap(Bitmap bitmap) {
            mBitmap = bitmap;
            if (mBitmap == null) {
                return;
            }
            mIntrinsicWidth = mBitmap.getWidth();
            mIntrinsicHeight = mBitmap.getHeight();
        }

        @Override
        public void draw(Canvas canvas) {
            if (mBitmap == null) {
                return;
            }
            int width = canvas.getWidth();
            int height = canvas.getHeight();
            int x = (width - mIntrinsicWidth) >> 1;
            int y = (height - mIntrinsicHeight) >> 1;
            canvas.drawBitmap(mBitmap, x, y, null);
        }

        @Override
        public int getOpacity() {
            return android.graphics.PixelFormat.OPAQUE;
        }

        @Override
        public void setAlpha(int alpha) {
            // Do nothing
        }

        @Override
        public void setColorFilter(ColorFilter cf) {
            // Do nothing
        }
    }
}
