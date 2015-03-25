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

package com.mediatek.filemanager.utils;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import com.mediatek.drm.OmaDrmClient;
import com.mediatek.drm.OmaDrmUiUtils;
import com.mediatek.drm.OmaDrmStore;
import android.graphics.Bitmap;


/**
 * The class is defined for easily use DrmManagerClient.
 */
public final class DrmManager {
    private static final String TAG = "DrmManager";

    public static final String APP_DRM = "application/vnd.oma.drm";
    public static final String EXT_DRM_CONTENT = "dcf";

    public static final int ACTIONID_NOT_DRM = -1;
    public static final int ACTIONID_INVALID_DRM = -2;

    private OmaDrmClient mDrmManagerClient = null;

    private static DrmManager sInstance = new DrmManager();

    /**
     * Constructor for DrmManager.
     */
    private DrmManager() {
    }

    /**
     * Initial the DrmManagerClient.
     * 
     * @param context The context to use.
     */
    public void init(Context context) {
        if (OptionsUtils.isMtkDrmApp()) {
            if (mDrmManagerClient == null) {
                mDrmManagerClient = new OmaDrmClient(context);
            }
        }
    }

    /**
     * Get a DrmManager Object. init() must be called before using it.
     * 
     * @return a instance of DrmManager.
     */
    public static DrmManager getInstance() {
        return sInstance;
    }

    /**
     * This method gets Bitmap of DRM file. (Draw a little lock icon at right-down part over
     * original icon)
     * 
     * @param resources the resource to use
     * @param path absolute path of the DRM file
     * @param actionId action ID of the file, which is not unique for DRM file
     * @param iconId the ID of background icon, which the new icon draws on
     * @return Bitmap of the DRM file
     */
    public Bitmap overlayDrmIconSkew(Resources resources, String path, int actionId, int iconId) {
        if (mDrmManagerClient != null && OptionsUtils.isMtkDrmApp()) {
            return OmaDrmUiUtils.overlayDrmIconSkew(mDrmManagerClient, resources, path, actionId, iconId);
        } else {
            return null;
        }
    }

    /**
     * Get original mimeType of a file.
     * 
     * @param path The file's path.
     * @return original mimeType of the file.
     */
    public String getOriginalMimeType(String path) {
        if (mDrmManagerClient != null && OptionsUtils.isMtkDrmApp()) {
            String mimeType = mDrmManagerClient.getOriginalMimeType(path);
            if (mimeType == null) {
                LogUtils.w(TAG, "#getOriginalMimeType(),mDrmManagerClient.getOriginalMimeType(path) return null.path:"
                        + path);
                mimeType = "";
            }
            return mimeType;
        } else {
            return "";
        }
    }

    /**
     * This method check weather the rights-protected content has valid right to transfer.
     * 
     * @param path path to the rights-protected content.
     * @return true for having right to transfer, false for not having the right.
     */
    public boolean isRightsStatus(String path) {
        if (mDrmManagerClient != null && OptionsUtils.isMtkDrmApp()) {
            return mDrmManagerClient.checkRightsStatus(path, OmaDrmStore.Action.TRANSFER) 
                != OmaDrmStore.RightsStatus.RIGHTS_VALID;
        } else {
            return false;
        }
    }

    /**
     * This method checks DRM file's MIME type
     * 
     * @param path path to content
     * @return true for DRM known type, false for DrmStore.DrmObjectType.UNKNOWN.
     */
    public boolean checkDrmObjectType(String path) {
        if (mDrmManagerClient != null && OptionsUtils.isMtkDrmApp()) {
            return mDrmManagerClient.getDrmObjectType(path, null) 
                != OmaDrmStore.DrmObjectType.UNKNOWN;
        } else {
            return false;
        }
    }

    /**
     * This method create and show a Dialog, which display the DRM file's protection information.
     * 
     * @param activity activity, which the Dialog associated with
     * @param path path to content
     */
    public void showProtectionInfoDialog(Activity activity, String path) {
        if (mDrmManagerClient != null && OptionsUtils.isMtkDrmApp()) {
            OmaDrmUiUtils.showProtectionInfoDialog(activity, path);
        }
    }

    /**
     * This method release drm manager client
     */
    public void release() {
        if (mDrmManagerClient != null) {
            mDrmManagerClient.release();
            mDrmManagerClient = null;
            LogUtils.d(TAG,"release drm manager client.");
        }
    }
}
