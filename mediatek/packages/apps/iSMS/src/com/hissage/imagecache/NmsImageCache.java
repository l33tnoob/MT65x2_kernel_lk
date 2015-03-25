package com.hissage.imagecache;

import android.graphics.Bitmap;

public interface NmsImageCache {
    
    void addBitmapToCache(Object data, Bitmap bitmap);
    
    Bitmap getBitmapFromMemCache(Object data);
}
