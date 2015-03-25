package com.hissage.imageworker;

import android.content.Context;
import android.graphics.Bitmap;

import com.hissage.api.NmsContactApi;
import com.hissage.imagecache.NmsImageCache;
import com.hissage.util.log.NmsLog;

public class NmsContactAvatarWorker extends NmsImageWorker {

    private static final String TAG = "NmsContactAvatarWorker";

    private Context mContext;

    public NmsContactAvatarWorker(Context context, int placeHolderResId, NmsImageCache imageCache) {
        super(context, placeHolderResId, imageCache);
        if (context != null)
            mContext = context.getApplicationContext();
    }

    @Override
    protected Bitmap processBitmap(Object data) {
        if (data == null || !(data instanceof Short) || (Short) data <= 0) {
            NmsLog.error(TAG, "processBitmap. parm error.");
            return null;
        }

        return NmsContactApi.getInstance(mContext).getAvatarViaEngineContactId((Short) data);
    }

}
