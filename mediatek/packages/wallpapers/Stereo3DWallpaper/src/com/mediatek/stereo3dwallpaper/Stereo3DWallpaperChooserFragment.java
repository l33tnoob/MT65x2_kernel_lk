/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.stereo3dwallpaper;

import android.app.Activity;
import android.app.DialogFragment;
import android.app.WallpaperManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.PaintFlagsDrawFilter;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.preference.PreferenceManager;
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

import java.util.ArrayList;

public class Stereo3DWallpaperChooserFragment extends DialogFragment implements
    AdapterView.OnItemSelectedListener, AdapterView.OnItemClickListener {

    private static final String TAG = "Stereo3DWallpaperChooserFragment";
    private static final String WALLPAPER_SERVICE_NAME
        = "com.mediatek.stereo3dwallpaper.Stereo3DWallpaperService";
    protected static final String VISIBILITY_CHANGED
        = "com.mediatek.stereo3dwallpaper.ACTION_VISIBILITY_CHANGED";

    private ArrayList<Integer> mImages;
    private ArrayList<Integer> mThumbs;
    private Bitmap mBitmap;
    private ComponentName mWallpaperComponent;

    private Resources mResources;
    private SharedPreferences mPreferences;
    private WallpaperLoader mLoader;
    private WallpaperManager mWallpaperManager;
    private final WallpaperDrawable mWallpaperDrawable = new WallpaperDrawable();

    public static Stereo3DWallpaperChooserFragment newInstance() {
        Stereo3DWallpaperChooserFragment fragment = new Stereo3DWallpaperChooserFragment();
        fragment.setCancelable(true);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Stereo3DLog.log(TAG, "onCreate");

        mResources = getResources();

        // initialize wallpaper components
        mWallpaperManager = (WallpaperManager)getActivity().getSystemService(
                                Context.WALLPAPER_SERVICE);
        mWallpaperComponent = new ComponentName("com.mediatek.stereo3dwallpaper",
                                                Stereo3DWallpaperService.class.getName());
        mPreferences = PreferenceManager.getDefaultSharedPreferences(getActivity());
    }

    @Override
    public void onDestroy() {
        if (mLoader != null && mLoader.getStatus() != WallpaperLoader.Status.FINISHED) {
            mLoader.cancel(true);
            mLoader = null;
        }
        super.onDestroy();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Stereo3DLog.log(TAG, "onCreateView");

        findWallpapers();

        View view = inflater.inflate(R.layout.wallpaper_chooser, container, false);
        view.setBackgroundDrawable(mWallpaperDrawable);

        final Gallery gallery = (Gallery) view.findViewById(R.id.gallery);
        gallery.setCallbackDuringFling(false);
        gallery.setOnItemSelectedListener(this);
        gallery.setAdapter(new ImageAdapter(getActivity()));

        View setButton = view.findViewById(R.id.set);
        setButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                setWallpaper(gallery.getSelectedItemPosition());
            }
        });

        return view;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        setWallpaper(position);
    }

    @Override
    public void onItemSelected(AdapterView parent, View v, int position, long id) {
        if (mLoader != null && mLoader.getStatus() != WallpaperLoader.Status.FINISHED) {
            mLoader.cancel();
        }
        mLoader = (WallpaperLoader) new WallpaperLoader().execute(position);
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {
        // TODO Auto-generated method stub
    }

    /**
     * This method retrieves all wallpapers listed in the array.
     */
    private void findWallpapers() {
        mThumbs = new ArrayList<Integer>(26);
        mImages = new ArrayList<Integer>(26);

        final String packageName = mResources.getResourcePackageName(R.array.wallpapers);
        addWallpapers(packageName, R.array.wallpapers);
    }

    /**
     * This method adds all wallpapers to the thumbs and images array lists.
     * @param packageName the package name of the app
     * @param list the array list defined in the resource
     */
    private void addWallpapers(String packageName, int list) {
        final String[] extras = mResources.getStringArray(list);

        for (String extra : extras) {
            int res = mResources.getIdentifier(extra, "drawable", packageName);

            if (res != 0) {
                final int thumbRes = mResources.getIdentifier(extra + "_small",
                                     "drawable", packageName);

                if (thumbRes != 0) {
                    mThumbs.add(thumbRes);
                    mImages.add(res);
                }
            }
        }
    }

    /**
     * This method sets the wallpaper.
     * @param v the view
     * @param position the position of the selected wallpaper in the image array list
     */
    private void setWallpaper(int position) {
        Stereo3DLog.log(TAG, "Set stereo3D wallpaper");
        setPrefs(mImages.get(position));
        Activity activity = getActivity();

        try {
            // When Stereo3DWallpaperChooserFragment sets a different wallpaper, setWallpaperComponent
            // must be called each time in order to force WallpaperManagerService to create a new window.
            // This is to avoid half black screen shown during orientation transition.
            mWallpaperManager.getIWallpaperManager().setWallpaperComponent(mWallpaperComponent);
            activity.setResult(activity.RESULT_OK);
        } catch (RemoteException e) {
            Stereo3DLog.log(TAG, "Failed to set wallpaper: " + e);
        }

        activity.finish();
    }

    /**
     * This method sets the new wallpaper id to the share preference
     * @param id the wallpaper id
     */
    private void setPrefs(int id) {
        SharedPreferences.Editor editor = mPreferences.edit();
        editor.putInt("wallpaper_id", id);
        editor.commit();
    }

    private class ImageAdapter extends BaseAdapter implements ListAdapter, SpinnerAdapter {
        private final LayoutInflater mLayoutInflater;

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

        public View getView(int position, View cv, ViewGroup parent) {
            View convertView = cv;

            if (convertView == null) {
                convertView = mLayoutInflater.inflate(R.layout.wallpaper_item, parent, false);
            }

            ImageView image = (ImageView) convertView.findViewById(R.id.wallpaper_image);

            int thumbRes = mThumbs.get(position);
            image.setImageResource(thumbRes);
            Drawable thumbDrawable = image.getDrawable();

            if (thumbDrawable == null) {
                Stereo3DLog.log(TAG, "Error decoding thumbnail resId=" + thumbRes
                                + " for wallpaper #" + position);
            } else {
                thumbDrawable.setDither(true);
            }

            return convertView;
        }
    }

    class WallpaperLoader extends AsyncTask<Integer, Void, Bitmap> {
        BitmapFactory.Options mOptions;

        WallpaperLoader() {
            mOptions = new BitmapFactory.Options();
            mOptions.inDither = false;
            mOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
        }

        protected Bitmap doInBackground(Integer... params) {
            if (isCancelled() || !isAdded()) {
                return null;
            }

            try {
                return BitmapFactory.decodeResource(getResources(),
                                                    mImages.get(params[0]), mOptions);
            } catch (OutOfMemoryError e) {
                return null;
            }
        }

        @Override
        protected void onPostExecute(Bitmap b) {
            if (b == null) {
                return;
            }

            if (isCancelled() || mOptions.mCancel) {
                b.recycle();
            } else {
                if (mBitmap != null) {
                    mBitmap.recycle();
                }

                View view = getView();

                if (view == null) {
                    mBitmap = null;
                    mWallpaperDrawable.setBitmap(null);
                } else {
                    mBitmap = b;
                    mWallpaperDrawable.setBitmap(b);
                    view.postInvalidate();
                }

                mLoader = null;
            }
        }

        void cancel() {
            mOptions.requestCancelDecode();
            super.cancel(true);
        }
    }

    static class WallpaperDrawable extends Drawable {
        Bitmap mBitmap;
        int mIntrinsicWidth;
        int mIntrinsicHeight;

        void setBitmap(Bitmap bitmap) {
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

            Stereo3DLog.log(TAG, "Bitmap width is " + mIntrinsicWidth + ", height is " + mIntrinsicHeight
                + ". Canvas width is " + width + ", height is " + height);

            // scale up the bitmap to make it cover the entire area
            float scaleW = width / (float)mIntrinsicWidth;
            float scaleH = height / (float)mIntrinsicHeight;

            if (scaleW > 1.0 || scaleH > 1.0) {

                Stereo3DLog.log(TAG, "Draw by scale size");

                float scale = scaleW > scaleH ? scaleW : scaleH;
                int scaledWidth = (int)(mIntrinsicWidth * scale);
                int scaledHeight = (int)(mIntrinsicHeight * scale);
                int x = (width - scaledWidth) / 2;
                int y = (height - scaledHeight) / 2;

                Bitmap scaledBitmap = Bitmap.createScaledBitmap(mBitmap, scaledWidth, scaledHeight, true);
                canvas.setDrawFilter(new PaintFlagsDrawFilter(0, Paint.ANTI_ALIAS_FLAG | Paint.FILTER_BITMAP_FLAG));
                canvas.drawBitmap(scaledBitmap, x, y, null);
                scaledBitmap.recycle();
                scaledBitmap = null;
            } else {
                Stereo3DLog.log(TAG, "Draw by original size");

                int x = (width - mIntrinsicWidth) / 2;
                int y = (height - mIntrinsicHeight) / 2;
                canvas.drawBitmap(mBitmap, x, y, null);
            }
        }

        @Override
        public int getOpacity() {
            return android.graphics.PixelFormat.OPAQUE;
        }

        @Override
        public void setAlpha(int arg0) {
            // TODO Auto-generated method stub
        }

        @Override
        public void setColorFilter(ColorFilter arg0) {
            // TODO Auto-generated method stub
        }
    }
}