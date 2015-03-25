package com.hissage.imageworker;

import java.lang.ref.WeakReference;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.TransitionDrawable;
import android.os.AsyncTask;
import android.widget.ImageView;

import com.hissage.imagecache.NmsImageCache;
import com.hissage.util.log.NmsLog;

public abstract class NmsImageWorker {

    private static final String TAG = "NmsImageWorker";

    private static final int FADE_IN_TIME = 200;

    private NmsImageCache mImageCache;
    private Bitmap mPlaceHolderBitmap;
    private boolean mFadeInBitmap = false;
    private boolean mExitTasksEarly = false;

    protected Context mContext;

    protected NmsImageWorker(Context context, Bitmap placeHolderBitmap) {
        init(context, placeHolderBitmap, null);
    }

    protected NmsImageWorker(Context context, Bitmap placeHolderBitmap, NmsImageCache imageCache) {
        init(context, placeHolderBitmap, imageCache);
    }

    protected NmsImageWorker(Context context, int placeHolderResId) {
        init(context, BitmapFactory.decodeResource(context.getResources(), placeHolderResId), null);
    }

    protected NmsImageWorker(Context context, int placeHolderResId, NmsImageCache imageCache) {
        init(context, BitmapFactory.decodeResource(context.getResources(), placeHolderResId),
                imageCache);
    }

    private void init(Context context, Bitmap placeHolderBitmap, NmsImageCache imageCache) {
        if (context != null)
            mContext = context.getApplicationContext();
        mPlaceHolderBitmap = placeHolderBitmap;
        mImageCache = imageCache;
    }

    public void loadImage(Object data, ImageView imageView) {
        Bitmap bitmap = null;

        if (mImageCache != null) {
            bitmap = mImageCache.getBitmapFromMemCache(data);
        }

        if (bitmap != null) {
            imageView.setImageBitmap(bitmap);
        } else if (cancelPotentialWork(data, imageView)) {
            final BitmapWorkerTask task = new BitmapWorkerTask(imageView);
            final AsyncDrawable asyncDrawable = new AsyncDrawable(mContext.getResources(),
                    mPlaceHolderBitmap, task);
            imageView.setImageDrawable(asyncDrawable);
            task.execute(data);
        } else {
            NmsLog.trace(TAG, "The same work is already in progress.");
        }
    }

    public void setImageCache(NmsImageCache cacheCallback) {
        mImageCache = cacheCallback;
    }

    public NmsImageCache getImageCache() {
        return mImageCache;
    }

    public void setImageFadeIn(boolean fadeIn) {
        mFadeInBitmap = fadeIn;
    }

    public void setExitTasksEarly(boolean exitTasksEarly) {
        mExitTasksEarly = exitTasksEarly;
    }

    protected abstract Bitmap processBitmap(Object data);

    public static void cancelWork(ImageView imageView) {
        final BitmapWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);
        if (bitmapWorkerTask != null) {
            bitmapWorkerTask.cancel(true);
            // NmsLog.error(TAG, "cancelWork - cancelled work for " +
            // bitmapWorkerTask.data);
        }
    }

    public static boolean cancelPotentialWork(Object data, ImageView imageView) {
        final BitmapWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

        if (bitmapWorkerTask != null) {
            final Object bitmapData = bitmapWorkerTask.data;
            if (bitmapData == null || !bitmapData.equals(data)) {
                bitmapWorkerTask.cancel(true);
                // NmsLog.error(TAG, data + ", " + bitmapData);
            } else {
                // The same work is already in progress.
                return false;
            }
        }
        return true;
    }

    private static BitmapWorkerTask getBitmapWorkerTask(ImageView imageView) {
        if (imageView != null) {
            final Drawable drawable = imageView.getDrawable();
            if (drawable instanceof AsyncDrawable) {
                final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
                return asyncDrawable.getBitmapWorkerTask();
            }
        }
        return null;
    }

    private class BitmapWorkerTask extends AsyncTask<Object, Void, Bitmap> {
        private Object data;
        private final WeakReference<ImageView> imageViewReference;

        public BitmapWorkerTask(ImageView imageView) {
            imageViewReference = new WeakReference<ImageView>(imageView);
        }

        @Override
        protected Bitmap doInBackground(Object... params) {
            data = params[0];
            Bitmap bitmap = null;

            if (!isCancelled() && getAttachedImageView() != null && !mExitTasksEarly) {
                bitmap = processBitmap(params[0]);
            }

            if (bitmap != null && mImageCache != null) {
                mImageCache.addBitmapToCache(params[0], bitmap);
            }

            return bitmap;
        }

        @Override
        protected void onPostExecute(Bitmap result) {
            if (isCancelled() || mExitTasksEarly) {
                result = null;
            }

            final ImageView imageView = getAttachedImageView();
            if (result != null && imageView != null) {
                setImageBitmap(imageView, result);
            }
        }

        private ImageView getAttachedImageView() {
            final ImageView imageView = imageViewReference.get();
            final BitmapWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

            if (this == bitmapWorkerTask) {
                return imageView;
            }

            return null;
        }
    }

    private static class AsyncDrawable extends BitmapDrawable {
        private final WeakReference<BitmapWorkerTask> bitmapWorkerTaskReference;

        public AsyncDrawable(Resources res, Bitmap bitmap, BitmapWorkerTask bitmapWorkerTask) {
            super(res, bitmap);

            bitmapWorkerTaskReference = new WeakReference<BitmapWorkerTask>(bitmapWorkerTask);
        }

        public BitmapWorkerTask getBitmapWorkerTask() {
            return bitmapWorkerTaskReference.get();
        }
    }

    private void setImageBitmap(ImageView imageView, Bitmap bitmap) {
        if (mFadeInBitmap) {
            final TransitionDrawable td = new TransitionDrawable(new Drawable[] {
                    new ColorDrawable(android.R.color.transparent),
                    new BitmapDrawable(mContext.getResources(), bitmap) });
            imageView.setBackgroundDrawable(new BitmapDrawable(mContext.getResources(),
                    mPlaceHolderBitmap));
            imageView.setImageDrawable(td);
            td.startTransition(FADE_IN_TIME);
        } else {
            imageView.setImageBitmap(bitmap);
        }
    }
}
