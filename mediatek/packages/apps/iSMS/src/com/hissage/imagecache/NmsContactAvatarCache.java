package com.hissage.imagecache;

import android.graphics.Bitmap;
import android.util.LruCache;

import com.hissage.config.NmsCommonUtils;
import com.hissage.util.log.NmsLog;

public class NmsContactAvatarCache implements NmsImageCache {

    private static final String TAG = "NmsContactAvatarCache";

    private static final int DEFAULT_MEM_CACHE_SIZE = 1024 * 1024 * 5; // 5MB

    private static NmsContactAvatarCache mInstance = null;
    private static LruCache<Short, Bitmap> mMemoryCache;

    private NmsContactAvatarCache() {
    }

    public synchronized static NmsContactAvatarCache getInstance() {
        if (mInstance == null) {
            mInstance = new NmsContactAvatarCache();
        }

        initMemoryCache();

        return mInstance;
    }

    public static void initMemoryCache() {
        if (mMemoryCache == null) {
            mMemoryCache = new LruCache<Short, Bitmap>(DEFAULT_MEM_CACHE_SIZE) {
                /**
                 * Measure item size in bytes rather than units which is more
                 * practical for a bitmap cache
                 */
                @Override
                protected int sizeOf(Short key, Bitmap bitmap) {
                    return NmsCommonUtils.getBitmapSize(bitmap);
                }
            };
        }
    }

    @Override
    public synchronized void addBitmapToCache(Object data, Bitmap bitmap) {
        if (data == null || !(data instanceof Short) || (Short) data <= 0 || bitmap == null) {
            NmsLog.error(TAG, "addBitmapToCache. parm error.");
            return;
        }

        if (mMemoryCache != null/* && mMemoryCache.get(data) == null */) {
            mMemoryCache.put((Short) data, bitmap);
        }

    }

    @Override
    public synchronized Bitmap getBitmapFromMemCache(Object data) {
        if (data == null || !(data instanceof Short) || (Short) data <= 0) {
            NmsLog.error(TAG, "getBitmapFromMemCache. parm error.");
            return null;
        }

        if (mMemoryCache != null) {
            final Bitmap memBitmap = mMemoryCache.get((Short) data);
            if (memBitmap != null) {
                return memBitmap;
            }
        }
        return null;
    }

    public synchronized void removeCache(short key) {
        if (mMemoryCache != null) {
            mMemoryCache.remove(key);
        }
    }

    public synchronized void clearCaches() {
        if (mMemoryCache != null) {
            mMemoryCache.evictAll();
        }
    }

}
