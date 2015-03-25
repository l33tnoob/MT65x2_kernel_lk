package com.hissage.imageworker;

import android.content.Context;
import android.graphics.Bitmap;
import android.media.ThumbnailUtils;
import android.provider.MediaStore.Video.Thumbnails;
import android.text.TextUtils;

import com.hissage.api.NmsiSMSApi;
import com.hissage.config.NmsBitmapUtils;
import com.hissage.imagecache.NmsImageCache;
import com.hissage.message.ip.NmsIpImageMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts.NmsIpMessageType;
import com.hissage.message.ip.NmsIpMessageConsts.NmsMessageProtocol;
import com.hissage.message.ip.NmsIpVideoMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.util.log.NmsLog;

public class NmsMessageMediaWorker extends NmsImageWorker {

    private static final String TAG = "NmsMessageMediaWorker";

    private int mImageSize;

    public NmsMessageMediaWorker(Context context, Bitmap placeHolderBitmap,
            NmsImageCache imageCache, int imageSize) {
        super(context, placeHolderBitmap, imageCache);
        mImageSize = imageSize;
    }

    @Override
    protected Bitmap processBitmap(Object data) {
        if (data == null || !(data instanceof Short) || (Short) data <= 0) {
            NmsLog.error(TAG, "processBitmap. parm error.");
            return null;
        }

        Bitmap thumb = null;
        NmsIpMessage ipMsg = NmsiSMSApi.nmsGetIpMsgInfoViaDbId((Short) data);
        if (ipMsg == null) {
            NmsLog.warn(TAG, "ipMsg is null");
            return null;
        }

        if (ipMsg.protocol == NmsMessageProtocol.MMS) {
            thumb = NmsSMSMMSManager.getInstance(mContext).getFirstImgFromMMS((int) ipMsg.id, true,
                    mImageSize, mImageSize);
        } else {
            if (ipMsg.type == NmsIpMessageType.PICTURE || ipMsg.type == NmsIpMessageType.SKETCH) {
                NmsIpImageMessage ipImageMsg = (NmsIpImageMessage) ipMsg;
                if (!TextUtils.isEmpty(ipImageMsg.thumbPath)) {
                    thumb = NmsBitmapUtils.getBitmapByPath(ipImageMsg.thumbPath,
                            NmsBitmapUtils.getOptions(ipImageMsg.thumbPath), mImageSize);
                }
                if (thumb == null && !TextUtils.isEmpty(ipImageMsg.path)) {
                    thumb = NmsBitmapUtils.getBitmapByPath(ipImageMsg.path,
                            NmsBitmapUtils.getOptions(ipImageMsg.path), mImageSize);
                }
            } else if (ipMsg.type == NmsIpMessageType.VIDEO) {
                NmsIpVideoMessage ipVideoMsg = (NmsIpVideoMessage) ipMsg;
                if (!TextUtils.isEmpty(ipVideoMsg.thumbPath)) {
                    thumb = NmsBitmapUtils.getBitmapByPath(ipVideoMsg.thumbPath,
                            NmsBitmapUtils.getOptions(ipVideoMsg.thumbPath), mImageSize);
                }
                if (thumb == null && !TextUtils.isEmpty(ipVideoMsg.path)) {
                    thumb = ThumbnailUtils.createVideoThumbnail(ipVideoMsg.path,
                            Thumbnails.MICRO_KIND);
                }
            } else {
                NmsLog.warn(TAG, "ipMsg.type == " + ipMsg.type + ". ignore!");
            }
        }

        return thumb;
    }

}
