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

package com.mediatek.rcse.activities.widgets;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.webkit.MimeTypeMap;
import android.widget.ImageView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ImageLoader;
import com.mediatek.rcse.service.ImageLoader.OnLoadImageFinishListener;

import com.orangelabs.rcs.R;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * A sub class of LinearLayout. It is simple to use. Example: AsyncGalleryView
 * ayncGalleryView =
 * (AsyncGalleryView)getActivity().findViewById(R.id.picture_view);
 * ayncGalleryView.setImagePath(fileName); ayncGalleryView.setTime("19:39:05");
 */
public class AsyncGalleryView extends ImageView implements OnClickListener {

    private static final String TAG = "AsyncGalleryView";
    private String mPictureNameString = null;
    private Uri mPictureNameUri = null;
    private String mMimeType = null;

    public static final String FILE_SCHEMA = "file://";

    private static final String[] PICTURE_FORMAT =
            {
                    "BMP", "PNG", "JPG", "JPEG", "TIFF", "PCX", "GIF", "DXF", "CGM", "CDR", "WMF", "EPS", "EMF", "PICT",
                    "PSD", "SWF", "SVG", "LIC", "TGA"
            };
    public static final String SEPRATOR = ".";
    public static final int INDEX_OFFSET = 1;
    public static final int INVALID_INDEX = -1;

    public static final Map<String, WeakReference<Bitmap>> APP_ICON_MAP =
            new ConcurrentHashMap<String, WeakReference<Bitmap>>();

    public AsyncGalleryView(Context context) {
        super(context);
    }

    public AsyncGalleryView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public AsyncGalleryView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    AsyncListener mCurrentListener = null;

    @Override
    public void setImageBitmap(Bitmap bm) {
        super.setImageBitmap(bm);
        onReset();
    }

    @Override
    public void setImageDrawable(Drawable drawable) {
        super.setImageDrawable(drawable);
        onReset();
    }

    @Override
    public void setImageResource(int resId) {
        super.setImageResource(resId);
        onReset();
    }

    public static boolean isPictureFile(String filePath) {
        if (null == filePath) {
            return false;
        }
        if (filePath.isEmpty()) {
            return false;
        }
        int index = filePath.lastIndexOf(SEPRATOR);
        if (index == INVALID_INDEX) {
            return false;
        } else {
            String fileFormat = filePath.substring(index + INDEX_OFFSET);
            if (null == fileFormat) {
                return false;
            }
            if (fileFormat.isEmpty()) {
                return false;
            }
            for (String format : PICTURE_FORMAT) {
                boolean isMatch = format.equalsIgnoreCase(fileFormat);
                if (isMatch) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * Set the image to display
     * 
     * @param filePath The file path that determined what used to display
     */
    public void setAsyncImage(String filePath) {
        Bitmap bitmap = ImageLoader.getImage(filePath);
        if (null != bitmap) {
            this.setImageBitmap(bitmap);
        } else {
            if (null != mCurrentListener && !mCurrentListener.mKey.equals(filePath)) {
                onReset();
            }
            mCurrentListener = new AsyncListener(filePath);
            ImageLoader.requestImage(filePath, mCurrentListener);
            setImageResource(R.drawable.default_image_preview);
        }
        setImagePath(filePath);
    }

    @Override
    public void setImageURI(Uri uri) {
        throw new RuntimeException(TAG + " doesn't support this method setImageURI() ");
    }

    private void onReset() {
        if (null != mCurrentListener) {
            mCurrentListener.destroy();
        } else {
            Logger.d(TAG, "onReset() mCurrentListener is null");
        }
    }

    private class AsyncListener implements OnLoadImageFinishListener {
        AsyncListener(Object key) {
            mKey = key;
        }

        private Object mKey = null;

        @Override
        public void onLoadImageFished(Bitmap image) {
            setImageBitmap(image);
            if (this == mCurrentListener) {
                mCurrentListener = null;
            }
        }

        public void destroy() {
            ImageLoader.interrupt(mKey);
        }
    }

    private void setImagePath(String fullFileName) {
        mPictureNameString = fullFileName;
        if (mPictureNameString != null) {
            if (!mPictureNameString.startsWith(FILE_SCHEMA)) {
                mPictureNameUri = Uri.parse(FILE_SCHEMA + mPictureNameString);
            } else {
                mPictureNameUri = Uri.parse(mPictureNameString);
            }
        } else {
            Logger.w(TAG, "mPictureNameString is null");
            return;
        }
        new ImageThumbnailAsyncTask().execute();
    }

    @Override
    public void onClick(View v) {
        Logger.v(TAG, "clicked then open the picture by gallery.");
        if (null != mMimeType) {
            if (isFileExist()) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(mPictureNameUri, mMimeType);
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                        | Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                mContext.startActivity(intent);
            } else {
                Toast.makeText(mContext, R.string.file_not_exist, Toast.LENGTH_SHORT).show();
            }
        } else {
            Logger.e(TAG, "onClick() mMimeType is null");
        }
    }

    private boolean isFileExist() {
        File file = new File(mPictureNameString);
        boolean isExist = file.exists();
        Logger.d(TAG, "isFileExist() file: " + mPictureNameString + ", isExist: " + isExist);
        return isExist;
    }

    /**
     * A helper class. It is used to produce thumbnail of a picture and show it
     * on the view.
     */
    private class ImageThumbnailAsyncTask extends AsyncTask<Void, Void, String> {
        private static final String TAG = "ImageThumbnailAsyncTask";

        @Override
        protected String doInBackground(Void... params) {
            return getMimeType(mPictureNameString);
        }

        protected void onPostExecute(String mimeType) {
            Logger.v(TAG, "onPostExecute(),mimeType = " + mimeType);
            if (null != mimeType) {
                mMimeType = mimeType;
                setOnClickListener(AsyncGalleryView.this);
            } else {
                Logger.e(TAG, "onPostExecute() mimeType is null");
            }
        }
    }

    /**
     * Get mimeType of a file with the file's full file name
     * 
     * @param fileName The file name
     * @return The mimeType of the file
     */
    public static String getMimeType(String fileName) {
        Logger.d(TAG, "getMimeType() entry with fileName = " + fileName);
        int pos = fileName.lastIndexOf(SEPRATOR);
        Logger.d(TAG, "getMimeType(): pos = " + pos);
        if (pos < 0 || pos > fileName.length()) {
            return null;
        }
        final String fileNameExtension = fileName.substring(pos);
        Logger.d(TAG, "getMimeType(), fileNameExtension = " + fileNameExtension);
        String extensionString =
                MimeTypeMap.getFileExtensionFromUrl(fileNameExtension.toLowerCase());
        MimeTypeMap mimeTypeMap = MimeTypeMap.getSingleton();
        String mimeType = mimeTypeMap.getMimeTypeFromExtension(extensionString);
        Logger.v(TAG, "getMimeType() mimeType = " + mimeType);
        return mimeType;
    }
}
