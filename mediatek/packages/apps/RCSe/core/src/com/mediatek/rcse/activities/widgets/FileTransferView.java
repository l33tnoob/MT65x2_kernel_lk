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
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.net.Uri;
import android.os.AsyncTask;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.webkit.MimeTypeMap;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.service.ApiManager;

import com.orangelabs.rcs.R;

import java.lang.ref.WeakReference;
import java.util.List;

public class FileTransferView extends RelativeLayout {

    private static final String TAG = "FileTransferView";
    private static final int INDEX_ZERO = 0;
    private static final String UNKNOWN_EXT_MIMETYPE = "unknown_ext_mimeType";
    private ImageView mFileIcon = null;
    private TextView mFileName = null;
    private AsyncGalleryView mImagePreview = null;

    public FileTransferView(Context context) {
        super(context);
    }

    public FileTransferView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public FileTransferView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    private void initLayout() {
        if (null == mFileIcon) {
            mFileIcon = (ImageView) findViewById(R.id.file_icon);
        }
        if (null == mFileName) {
            mFileName = (TextView) findViewById(R.id.file_name);
        }
        if (null == mImagePreview) {
            mImagePreview = (AsyncGalleryView) findViewById(R.id.file_transfer_preview);
        }
    }

    /**
     * @param filePath The path of the file.
     */
    public void setFile(final String filePath) {
        Logger.d(TAG, "setFile(), filePath is " + filePath);
        if (TextUtils.isEmpty(filePath)) {
            return;
        }
        initLayout();
        if (AsyncGalleryView.isPictureFile(filePath)) {
            Logger.d(TAG, "setFile(), the file is a picture");
            mFileIcon.setVisibility(View.GONE);
            mFileName.setVisibility(View.GONE);
            mImagePreview.setVisibility(View.VISIBLE);
            mImagePreview.setAsyncImage(filePath);
        } else {
            Logger.d(TAG, "setFile(), the file is not a picture");
            mFileIcon.setVisibility(View.VISIBLE);
            mFileName.setVisibility(View.VISIBLE);
            String fileName = ModelImpl.SentFileTransfer.extractFileNameFromPath(filePath);
            mFileName.setText(fileName);
            mImagePreview.setVisibility(View.GONE);
            new SetIconAsyncTask().execute(filePath, null, null);
        }
    }

    private Uri getFileNameUri(String filePath) {
        Logger.d(TAG, "getFileNameUri() entry, the filePath is " + filePath);
        if (TextUtils.isEmpty(filePath)) {
            return null;
        }
        Uri fileNameUri = null;
        if (!filePath.startsWith(AsyncGalleryView.FILE_SCHEMA)) {
            fileNameUri = Uri.parse(AsyncGalleryView.FILE_SCHEMA + filePath);
        } else {
            fileNameUri = Uri.parse(filePath);
        }
        Logger.d(TAG, "getFileNameUri() exit, the fileNameUri is " + fileNameUri);
        return fileNameUri;
    }

    private class SetIconAsyncTask extends AsyncTask<String, Void, Boolean> {
        private static final String TAG = "SetIconAsyncTask";
        private String mMimeType = null;
        private String mFilePath = null;
        private Bitmap mTempIcon = null;

        @Override
        protected Boolean doInBackground(String... params) {
            Logger.d(TAG, "doInBackground(), params[0] is " + params[0]);
            mFilePath = params[0];
            boolean isIconFound = false;
            String fileName = ModelImpl.SentFileTransfer.extractFileNameFromPath(mFilePath);
            mMimeType = MediaFile.getMimeTypeForFile(fileName);
            if (mMimeType == null) {
                mMimeType =
                        MimeTypeMap.getSingleton().getMimeTypeFromExtension(
                                getFileExtension(fileName));
            }
            if (mMimeType == null) {
                mMimeType = UNKNOWN_EXT_MIMETYPE;
                Logger.e(TAG, "doInBackground() ,mimeType is unknown");
            }
            WeakReference<Bitmap> iconBitmap = AsyncGalleryView.APP_ICON_MAP.get(mMimeType);
            if (null != iconBitmap && null != iconBitmap.get()) {
                mTempIcon = iconBitmap.get();
                isIconFound = true;
            } else {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(getFileNameUri(mFilePath), mMimeType);
                PackageManager packageManager = mContext.getPackageManager();
                List<ResolveInfo> list =
                        packageManager.queryIntentActivities(intent,
                                PackageManager.MATCH_DEFAULT_ONLY);
                int size = list.size();
                if (size == 0) {
                    Logger
                            .d(TAG,
                                    "doInBackground(), no icon found for this file type , use the default icon");
                } else {
                    Drawable icon = list.get(0).activityInfo.loadIcon(packageManager);
                    mTempIcon = ((BitmapDrawable) icon).getBitmap();
                    AsyncGalleryView.APP_ICON_MAP.put(mMimeType, new WeakReference<Bitmap>(
                            mTempIcon));
                    isIconFound = true;
                }
            }
            return isIconFound;
        }

        protected void onPostExecute(Boolean result) {
            Logger.d(TAG, "onPostExecute() ,result is " + result);
            if (result) {
                mFileIcon.setImageBitmap(mTempIcon);
            } else {
                mFileIcon.setImageResource(R.drawable.rcs_ic_ft_default_preview);
            }
            OnClickListener onClickListener = new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (null != mMimeType) {
                        Intent intent = new Intent(Intent.ACTION_VIEW);
                        intent.setDataAndType(getFileNameUri(mFilePath), mMimeType);
                        try {
                            mContext.startActivity(intent);
                        } catch (android.content.ActivityNotFoundException e) {
                            Toast.makeText(ApiManager.getInstance().getContext(),
                                    R.string.file_formats_not_support, Toast.LENGTH_SHORT).show();
                            e.printStackTrace();
                        }
                    } else {
                        Logger.e(TAG, "onPostExecute(), mMimeType is null");
                    }
                }
            };
            mFileName.setOnClickListener(onClickListener);
            mFileIcon.setOnClickListener(onClickListener);
        }

        private String getFileExtension(String fileName) {
            Logger.d(TAG, "getFileExtension() entry, the fileName is " + fileName);
            String extension = null;
            if (TextUtils.isEmpty(fileName)) {
                return null;
            }
            int lastDot = fileName.lastIndexOf(AsyncGalleryView.SEPRATOR);
            int fileLength = fileName.length();
            if ((lastDot > INDEX_ZERO) && (lastDot < fileLength - AsyncGalleryView.INDEX_OFFSET)) {
                extension =
                        fileName.substring(lastDot + AsyncGalleryView.INDEX_OFFSET).toLowerCase();
            }
            Logger.d(TAG, "getFileExtension() entry, the extension is " + extension);
            return extension;
        }
    }
}
