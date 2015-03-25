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

package com.mediatek.filemanager;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.text.TextUtils;
import android.view.ViewGroup;
import android.net.Uri;
import com.mediatek.filemanager.ext.DefaultIconExtension;
import com.mediatek.filemanager.ext.IIconExtension;
import com.mediatek.filemanager.service.FileManagerService;
import com.mediatek.filemanager.utils.DrmManager;
import com.mediatek.filemanager.utils.LogUtils;
import com.mediatek.filemanager.utils.OptionsUtils;
import com.mediatek.pluginmanager.Plugin;
import com.mediatek.pluginmanager.PluginManager;
import com.mediatek.drm.OmaDrmUtils;

import java.util.HashMap;


public final class IconManager {
    public static final String TAG = "IconManager";

    private static IconManager sInstance = new IconManager();
    /** the sdcard2 head image */
    // -------------- Bitmaps Cache for creating files ICON ---------------------

    private static final int OFFX = 4;
    /** Cache the default icons */
    protected HashMap<Integer, Bitmap> mDefIcons = null;
    /** Cache the sdcard2 icons, <all the icons has a sdcard2 head> */
    protected HashMap<Integer, Bitmap> mSdcard2Icons = null;
    private Resources mRes;
    protected Bitmap mIconsHead = null;

    private IIconExtension mExt = null;
    private int mCurrentDirection = 0;
    private boolean mDirectionChanged = false;
    /**
     * The map used to record custom icons.
     * The key is file type,the Value is drawable-id.
     * If want to add more file types ,just add it's file type & drawable-id to be show  in this Map! 
     */
    private static HashMap<Integer, Integer> sCustomDrableIdsMap = new HashMap<Integer, Integer>();

    private IconManager() {

    }

    /**
     * This method gets instance of IconManager
     * 
     * @return instance of IconManager
     */
    public static IconManager getInstance() {
        return sInstance;
    }

    /**
     * This method gets the drawable id based on the mimetype
     * 
     * @param mimeType the mimeType of a file/folder
     * @return the drawable icon id based on the mimetype
     */
    public static int getDrawableId(Context context,String mimeType) {
        if (TextUtils.isEmpty(mimeType)) {
            return R.drawable.fm_unknown;
        } else if (mimeType.startsWith("application/vnd.android.package-archive")) {
            // TODO change "application/vnd.android.package-archive" to static final string
            return R.drawable.fm_apk;
        } else if (mimeType.startsWith("application/zip")) {
            return R.drawable.fm_zip;
        } else if (mimeType.startsWith("application/ogg")) {
            return R.drawable.fm_audio;
        } else if (mimeType.startsWith("audio/")) {
            return R.drawable.fm_audio;
        } else if (mimeType.startsWith("image/")) {
            return R.drawable.fm_picture;
        } else if (mimeType.startsWith("text/")) {
            return R.drawable.fm_txt;
        } else if (mimeType.startsWith("video/")) {
            return R.drawable.fm_video;
        }
        return getCustomDrawableId(context, mimeType);
    }
    
    public static int getUnknownTypeDrawableId() {
        return R.drawable.fm_unknown;
    }
    
    /**
     * If the mime type not support by the default system,can customer the file icon here.
     * @param context
     * @param mimeType
     * @param fileUri
     * @return The resource id that use to curtome 
     */
    private static int getCustomDrawableId(Context context, String mimeType) {
        if (!OptionsUtils.isOP01Surported()) {
            // just supported on OP01
            return getUnknownTypeDrawableId();
        }

        int fileType = MediaFileManager.getFileTypeForMimeType(mimeType);

        if (!sCustomDrableIdsMap.containsKey(fileType)) {
            return getUnknownTypeDrawableId();
        }

        return sCustomDrableIdsMap.get(fileType);
    }

    /**
     * 
     * @param context
     * @param mimeType
     * @param fileInfo
     * @return true if have some APs support this mime type,false else. 
     */
    private static boolean isSupportedByCurrentSystem(Context context, String mimeType) {
        PackageManager packageManager = context.getPackageManager();
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setType(mimeType);
        ResolveInfo info = packageManager.resolveActivity(intent, PackageManager.MATCH_DEFAULT_ONLY);
        return info != null;
    }
    
    public static void updateCustomDrableMap(Context context) {
        sCustomDrableIdsMap.clear();

        // for excel mime type
        if (isSupportedByCurrentSystem(context, "application/vnd.ms-excel")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.spreadsheetml.template")) {
            sCustomDrableIdsMap.put(MediaFileManager.FILE_TYPE_MS_EXCEL, R.drawable.fm_excel);
            LogUtils.d(TAG, "add excel type drawable");
        }

        // for ppt mime type
        if (isSupportedByCurrentSystem(context, "application/mspowerpoint")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.presentationml.presentation")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.presentationml.template")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.presentationml.slideshow")) {
            sCustomDrableIdsMap.put(MediaFileManager.FILE_TYPE_MS_POWERPOINT, R.drawable.fm_ppt);
            LogUtils.d(TAG, "add ppt type drawable");
        }

        // for world mime type
        if (isSupportedByCurrentSystem(context, "application/msword")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.wordprocessingml.document")
                || isSupportedByCurrentSystem(context,
                        "application/vnd.openxmlformats-officedocument.wordprocessingml.template")) {
            sCustomDrableIdsMap.put(MediaFileManager.FILE_TYPE_MS_WORD, R.drawable.fm_word);
            LogUtils.d(TAG, "add world type drawable");
        }

        // for pdf mime type
        if (isSupportedByCurrentSystem(context, "application/pdf")) {
            sCustomDrableIdsMap.put(MediaFileManager.FILE_TYPE_PDF, R.drawable.fm_pdf);
            LogUtils.d(TAG, "add pdf type drawable");
        }
    }

    /**
     * This method gets icon from resources according to file's information.
     * 
     * @param res Resources to use
     * @param fileInfo information of file
     * @param service FileManagerService, which will provide function to get file's Mimetype
     * @return bitmap(icon), which responds the file
     */
    public Bitmap getIcon(Resources res, FileInfo fileInfo, FileManagerService service,int viewDirection) {
        if (mCurrentDirection != viewDirection) {
            mDirectionChanged = true;
            mCurrentDirection = viewDirection;
        }

        Bitmap icon = null;
        boolean isExternal = MountPointManager.getInstance().isExternalFile(fileInfo);
        LogUtils.d(TAG, "getIcon,isExternal =" + isExternal);
        if (fileInfo.isDirectory()) {
            icon = getFolderIcon(fileInfo, isExternal);
        } else {
            String mimeType = fileInfo.getFileMimeType(service);
            LogUtils.d(TAG, "getIcon imimeType =" + mimeType);
            int iconId = getDrawableId(service,mimeType);
            if (fileInfo.isDrmFile()) {
                int actionId = OmaDrmUtils.getMediaActionType(mimeType);
                LogUtils.d(TAG, "getIcon isDrmFile & actionId=" + actionId);
                if (actionId != DrmManager.ACTIONID_NOT_DRM) {
                    // try to get the DRM file icon.
                    icon = DrmManager.getInstance().overlayDrmIconSkew(res,
                            fileInfo.getFileAbsolutePath(), actionId, iconId);
                    if (icon != null && isExternal) {
                        icon = createExternalIcon(icon);
                    }
                }
            }
            if (icon == null) {
                icon = getFileIcon(iconId, isExternal);
            }
        }

        return icon;
    }

    private Bitmap getFileIcon(int iconId, boolean isExternal) {
        if (isExternal) {
            return getExternalIcon(iconId);
        } else {
            return getDefaultIcon(iconId);
        }
    }

    private Bitmap getFolderIcon(FileInfo fileInfo, boolean isExternal) {
        String path = fileInfo.getFileAbsolutePath();
        if (MountPointManager.getInstance().isInternalMountPath(path)) {
            return getDefaultIcon(R.drawable.phone_storage);
        } else if (MountPointManager.getInstance().isExternalMountPath(path)) {
            return getDefaultIcon(R.drawable.sdcard);
        } else if (mExt != null && mExt.isSystemFolder(path)) {
            Bitmap icon = mExt.getSystemFolderIcon(path);
            if (icon != null) {
                if (isExternal) {
                    return createExternalIcon(icon);
                } else {
                    return icon;
                }
            }
        } else if (OptionsUtils.isMtkHotKnotSupported() && fileInfo.getShowName().equalsIgnoreCase("HotKnot") && 
                (MountPointManager.getInstance().isInternalMountPath(fileInfo.getFile().getParent()) ||
                        MountPointManager.getInstance().isExternalMountPath(fileInfo.getFile().getParent()))) {
            /*add icon for HotKnot folder*/
            return getFileIcon(R.drawable.ic_hotknot_folder,isExternal);
        }
        return getFileIcon(R.drawable.fm_folder, isExternal);
    }

    /**
     * This method initializes variable mExt of IIconExtension type, and create system folder.
     * 
     * @param context Context to use
     * @param path create system folder under this path
     */
    public void init(Context context, String path) {
        mRes = context.getResources();
        try {
            mExt = (IIconExtension) PluginManager.createPluginObject(context,
                    IIconExtension.class.getName());
        } catch (Plugin.ObjectCreationException e) {
            mExt = new DefaultIconExtension();
        }
        mExt.createSystemFolder(path);
    }

    /**
     * This method checks weather certain file is system folder.
     * 
     * @param fileInfo certain file to be checked
     * @return true for system folder, and false for not system folder
     */
    public boolean isSystemFolder(FileInfo fileInfo) {
        if (fileInfo == null || mExt == null) {
            return false;
        }
        return mExt.isSystemFolder(fileInfo.getFileAbsolutePath());
    }

    /**
     * Get the sdcard2 icon . icon.
     * 
     * @param resId resource ID for external icon
     * @return external icon for certain item
     */
    public Bitmap getExternalIcon(int resId) {
        if (mSdcard2Icons == null) {
            mSdcard2Icons = new HashMap<Integer, Bitmap>();
        }
        if (mDirectionChanged) {
            if (mDefIcons != null) {
                mDefIcons.clear();
            }
            mSdcard2Icons.clear();
            mIconsHead = null;
            mDirectionChanged = false;
        }
        Bitmap icon = null;
        if (mSdcard2Icons.containsKey(resId)) {
            icon = mSdcard2Icons.get(resId);
        } else {
            icon = createExternalIcon(getDefaultIcon(resId));
            mSdcard2Icons.put(resId, icon);
        }
        return icon;
    }

    /**
     * Merge the {@link mIconsHead} with bitmap together to get the SDCard2 icon.
     * 
     * @param bitmap base icon for external icon
     * @return created external icon
     */
    public Bitmap createExternalIcon(Bitmap bitmap) {
        if (bitmap == null) {
            throw new IllegalArgumentException("parameter bitmap is null");
        }
        if (mIconsHead == null) {
            mIconsHead = BitmapFactory.decodeResource(mRes,
                    R.drawable.fm_sdcard2_header);
        }
        if (mCurrentDirection == ViewGroup.LAYOUT_DIRECTION_LTR) {
            int offx = mIconsHead.getWidth() / OFFX;
            int width = offx + bitmap.getWidth();
            int height = bitmap.getHeight();
            Bitmap icon = Bitmap.createBitmap(width, height, Config.ARGB_8888);
            Canvas c = new Canvas(icon);
            c.drawBitmap(bitmap, offx, 0, null);
            c.drawBitmap(mIconsHead, 0, 0, null);
            return icon;
        } else if (mCurrentDirection == ViewGroup.LAYOUT_DIRECTION_RTL) {
            int offx = mIconsHead.getWidth() / OFFX;
            int width = offx + bitmap.getWidth();
            int height = bitmap.getHeight();
            Bitmap icon = Bitmap.createBitmap(width, height, Config.ARGB_8888);
            Canvas c = new Canvas(icon);
            c.drawBitmap(bitmap, 0, 0, null);
            c.drawBitmap(mIconsHead, width-mIconsHead.getWidth(), 0, null);
            return icon;
        } else {
            LogUtils.d(TAG,"createExternalIcon, unknown direction...");
            return null;
        }
    }

    /**
     * Get the default bitmap and cache it in memory.
     * 
     * @param resId resource ID for default icon
     * @return default icon
     */
    public Bitmap getDefaultIcon(int resId) {
        if (mDefIcons == null) {
            mDefIcons = new HashMap<Integer, Bitmap>();
        }
        if (mDirectionChanged) {
            mDefIcons.clear();
            if (mSdcard2Icons != null) {
                mSdcard2Icons.clear();
            }
            mIconsHead = null;
            mDirectionChanged = false;
        }
        Bitmap icon = null;

        if (mDefIcons.containsKey(resId)) {
            icon = mDefIcons.get(resId);
        } else {
            icon = BitmapFactory.decodeResource(mRes, resId);
            if (icon == null) {
                throw new IllegalArgumentException(
                        "decodeResource()fail, or invalid resId");
            }
            mDefIcons.put(resId, icon);
        }
        return icon;
    }
}
