/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.drm;

import android.content.ContentValues;
import android.content.Context;
import android.drm.DrmEvent;
import android.drm.DrmManagerClient;
import android.drm.DrmInfo;
import android.drm.DrmInfoRequest;
import android.drm.DrmInfoStatus;
import android.drm.DrmRights;

import android.net.Uri;
import android.util.Log;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

/**
 * The main programming interface for OMA DRM v1.0 feature: Mediatek's DRM framework proxy
 * An application dealing with OMA DRM v1.0 need to instantiate this class and
 * access proper API methods.
 */
public class OmaDrmClient {
    /**
     * Indicates that a request was successful or that no error occurred.
     */
    public static final int ERROR_NONE = DrmManagerClient.ERROR_NONE;
    /**
     * Indicates that an error occurred and the reason is not known.
     */
    public static final int ERROR_UNKNOWN = DrmManagerClient.ERROR_UNKNOWN;

    private static final String TAG = "OmaDrmClient";

    private DrmManagerClient mDrmManagerClient;
    private Context mContext;

    // the dialog array list to deal with dialog UI operation
    public static ArrayList<OmaDrmUiUtils.CustomAlertDialog> sSecureTimerDialogQueue =
        new ArrayList<OmaDrmUiUtils.CustomAlertDialog>();
    public static ArrayList<OmaDrmUiUtils.CustomAlertDialog> sConsumeDialogQueue =
        new ArrayList<OmaDrmUiUtils.CustomAlertDialog>();
    public static ArrayList<OmaDrmUiUtils.CustomAlertDialog> sProtectionInfoDialogQueue =
        new ArrayList<OmaDrmUiUtils.CustomAlertDialog>();
    public static ArrayList<OmaDrmUiUtils.CustomAlertDialog> sLicenseDialogQueue =
        new ArrayList<OmaDrmUiUtils.CustomAlertDialog>();

    /**
     * Creates a OmaDrmClient
     *
     * @param context Context of the caller.
     */
    public OmaDrmClient(Context context) {
        Log.d(TAG, "create OmaDrmClient instance");
        mContext = context;
        mDrmManagerClient = new DrmManagerClient(context);
    }

    protected void finalize() {
        Log.d(TAG, "finalize OmaDrmClient instance");
    }


    // M: @{
    // ALPS00772785, add release() function
    /**
     * Releases resources associated with the current session of DrmManagerClient.
     *
     * It is considered good practice to call this method when the {@link DrmManagerClient} object
     * is no longer needed in your application. After release() is called,
     * {@link DrmManagerClient} is no longer usable since it has lost all of its required resource.
     */
    public void release() {
        Log.d(TAG, "release OmaDrmClient instance");
        if (mDrmManagerClient != null) {
            mDrmManagerClient.release();
        }
    }
    // M: @}

    /**
     * Factory: get a newly created OmaDrmClient instance.
     *
     * @param context Context of the caller.
     * @return OmaDrmClient
     */
    public static OmaDrmClient newInstance(Context context) {
        Log.d(TAG, "new OmaDrmClient instance");
        return new OmaDrmClient(context);
    }

    /**
     * Get the internal reference to generic DrmManagerClient instance
     *
     * @return android.drm.DrmManagerClient
     */
    public DrmManagerClient getDrmClient() {
        return mDrmManagerClient;
    }

    /**
     * Get the application context where the DrmManagerClient instance was created
     *
     * @return The application Context
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Retrieves information about all the DRM plug-ins (agents) that are registered with
     * the DRM framework.
     *
     * @return A <code>String</code> array of DRM plug-in descriptions.
     */
    public String[] getAvailableDrmEngines() {
        return mDrmManagerClient.getAvailableDrmEngines();
    }

    /**
     * Retrieves constraint information for rights-protected content.
     *
     * @param path Path to the content from which you are retrieving DRM constraints.
     * @param action Action defined in {@link DrmStore.Action}.
     *
     * @return A {@link android.content.ContentValues} instance that contains
     * key-value pairs representing the constraints. Null in case of failure.
     * The keys are defined in {@link DrmStore.ConstraintsColumns}.
     */
    public ContentValues getConstraints(String path, int action) {
        return mDrmManagerClient.getConstraints(path, action);
    }

   /**
    * Retrieves metadata information for rights-protected content.
    *
    * @param path Path to the content from which you are retrieving metadata information.
    *
    * @return A {@link android.content.ContentValues} instance that contains
    * key-value pairs representing the metadata. Null in case of failure.
    */
    public ContentValues getMetadata(String path) {
        return mDrmManagerClient.getMetadata(path);
    }

    /**
     * Retrieves constraint information for rights-protected content.
     *
     * @param uri URI for the content from which you are retrieving DRM constraints.
     * @param action Action defined in {@link DrmStore.Action}.
     *
     * @return A {@link android.content.ContentValues} instance that contains
     * key-value pairs representing the constraints. Null in case of failure.
     */
    public ContentValues getConstraints(Uri uri, int action) {
        return getConstraints(uri, action);
    }

   /**
    * Retrieves metadata information for rights-protected content.
    *
    * @param uri URI for the content from which you are retrieving metadata information.
    *
    * @return A {@link android.content.ContentValues} instance that contains
    * key-value pairs representing the constraints. Null in case of failure.
    */
    public ContentValues getMetadata(Uri uri) {
        return mDrmManagerClient.getMetadata(uri);
    }

    /**
     * Saves rights to a specified path and associates that path with the content path.
     * 
     * <p class="note"><strong>Note:</strong> For OMA or WM-DRM, <code>rightsPath</code> and
     * <code>contentPath</code> can be null.</p>
     *
     * @param drmRights The {@link DrmRights} to be saved.
     * @param rightsPath File path where rights will be saved.
     * @param contentPath File path where content is saved.
     *
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     *
     * @throws IOException If the call failed to save rights information at the given
     * <code>rightsPath</code>.
     */
    public int saveRights(
            DrmRights drmRights, String rightsPath, String contentPath) throws IOException {
        return mDrmManagerClient.saveRights(drmRights, rightsPath, contentPath);
    }

    /**
     * Installs a new DRM plug-in (agent) at runtime.
     *
     * @param engineFilePath File path to the plug-in file to be installed.
     *
     * {@hide}
     */
    public void installDrmEngine(String engineFilePath) {
        mDrmManagerClient.installDrmEngine(engineFilePath);
    }

    /**
     * Checks whether the given MIME type or path can be handled.
     *
     * @param path Path of the content to be handled.
     * @param mimeType MIME type of the object to be handled.
     *
     * @return True if the given MIME type or path can be handled; false if they cannot be handled.
     */
    public boolean canHandle(String path, String mimeType) {
        return mDrmManagerClient.canHandle(path, mimeType);
    }

    /**
     * Checks whether the given MIME type or URI can be handled.
     *
     * @param uri URI for the content to be handled.
     * @param mimeType MIME type of the object to be handled
     *
     * @return True if the given MIME type or URI can be handled; false if they cannot be handled.
     */
    public boolean canHandle(Uri uri, String mimeType) {
        return mDrmManagerClient.canHandle(uri, mimeType);
    }

    /**
     * Processes the given DRM information based on the information type.
     *
     * @param drmInfo The {@link DrmInfo} to be processed.
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     */
    public int processDrmInfo(DrmInfo drmInfo) {
        return mDrmManagerClient.processDrmInfo(drmInfo);
    }

    /**
     * Retrieves information for registering, unregistering, or acquiring rights.
     *
     * @param drmInfoRequest The {@link DrmInfoRequest} that specifies the type of DRM
     * information being retrieved.
     *
     * @return A {@link DrmInfo} instance.
     */
    public DrmInfo acquireDrmInfo(DrmInfoRequest drmInfoRequest) {
        return mDrmManagerClient.acquireDrmInfo(drmInfoRequest);
    }

    /**
     * Processes a given {@link DrmInfoRequest} and returns the rights information asynchronously.
     *<p>
     * This is a utility method that consists of an
     * {@link #acquireDrmInfo(DrmInfoRequest) acquireDrmInfo()} and a
     * {@link #processDrmInfo(DrmInfo) processDrmInfo()} method call. This utility method can be 
     * used only if the selected DRM plug-in (agent) supports this sequence of calls. Some DRM
     * agents, such as OMA, do not support this utility method, in which case an application must
     * invoke {@link #acquireDrmInfo(DrmInfoRequest) acquireDrmInfo()} and
     * {@link #processDrmInfo(DrmInfo) processDrmInfo()} separately.
     *
     * @param drmInfoRequest The {@link DrmInfoRequest} used to acquire the rights.
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     */
    public int acquireRights(DrmInfoRequest drmInfoRequest) {
        return mDrmManagerClient.acquireRights(drmInfoRequest);
    }

    /**
     * Retrieves the type of rights-protected object (for example, content object, rights
     * object, and so on) using the specified path or MIME type. At least one parameter must
     * be specified to retrieve the DRM object type.
     *
     * @param path Path to the content or null.
     * @param mimeType MIME type of the content or null.
     * 
     * @return An <code>int</code> that corresponds to a {@link DrmStore.DrmObjectType}.
     */
    public int getDrmObjectType(String path, String mimeType) {
        return mDrmManagerClient.getDrmObjectType(path, mimeType);
    }

    /**
     * Retrieves the type of rights-protected object (for example, content object, rights
     * object, and so on) using the specified URI or MIME type. At least one parameter must
     * be specified to retrieve the DRM object type.
     *
     * @param uri URI for the content or null.
     * @param mimeType MIME type of the content or null.
     * 
     * @return An <code>int</code> that corresponds to a {@link DrmStore.DrmObjectType}.
     */
    public int getDrmObjectType(Uri uri, String mimeType) {
        return mDrmManagerClient.getDrmObjectType(uri, mimeType);
    }

    /**
     * Retrieves the MIME type embedded in the original content.
     *
     * @param path Path to the rights-protected content.
     *
     * @return The MIME type of the original content, such as <code>video/mpeg</code>.
     */
    public String getOriginalMimeType(String path) {
        return mDrmManagerClient.getOriginalMimeType(path);
    }

    /**
     * Retrieves the MIME type embedded in the original content.
     *
     * @param uri URI of the rights-protected content.
     *
     * @return MIME type of the original content, such as <code>video/mpeg</code>.
     */
    public String getOriginalMimeType(Uri uri) {
        return mDrmManagerClient.getOriginalMimeType(uri);
    }

    /**
     * Checks whether the given content has valid rights.
     *
     * @param path Path to the rights-protected content.
     *
     * @return An <code>int</code> representing the {@link DrmStore.RightsStatus} of the content.
     */
    public int checkRightsStatus(String path) {
        return mDrmManagerClient.checkRightsStatus(path);
    }

    /**
     * Check whether the given content has valid rights.
     *
     * @param uri URI of the rights-protected content.
     *
     * @return An <code>int</code> representing the {@link DrmStore.RightsStatus} of the content.
     */
    public int checkRightsStatus(Uri uri) {
        return mDrmManagerClient.checkRightsStatus(uri);
    }

    /**
     * Checks whether the given rights-protected content has valid rights for the specified
     * {@link DrmStore.Action}.
     *
     * @param path Path to the rights-protected content.
     * @param action The {@link DrmStore.Action} to perform.
     *
     * @return An <code>int</code> representing the {@link DrmStore.RightsStatus} of the content.
     */
    public int checkRightsStatus(String path, int action) {
        int result = mDrmManagerClient.checkRightsStatus(path, action);
        if (result == OmaDrmStore.RightsStatus.SECURE_TIMER_INVALID) {
            Log.d(TAG, "checkRightsStatus : secure timer indicates invalid state");
            result = OmaDrmStore.RightsStatus.RIGHTS_INVALID;
        }
        return result;
    }

    /**
     * Checks whether the given rights-protected content has valid rights for the specified
     * {@link DrmStore.Action}.
     *
     * @param uri URI for the rights-protected content.
     * @param action The {@link DrmStore.Action} to perform.
     *
     * @return An <code>int</code> representing the {@link DrmStore.RightsStatus} of the content.
     */
    public int checkRightsStatus(Uri uri, int action) {
        int result = mDrmManagerClient.checkRightsStatus(uri, action);
        if (result == OmaDrmStore.RightsStatus.SECURE_TIMER_INVALID) {
            Log.d(TAG, "checkRightsStatus : secure timer indicates invalid state");
            result = OmaDrmStore.RightsStatus.RIGHTS_INVALID;
        }
        return result;
    }

    /**
     * Removes the rights associated with the given rights-protected content.
     *
     * @param path Path to the rights-protected content.
     *
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     */
    public int removeRights(String path) {
        return mDrmManagerClient.removeRights(path);
    }

    /**
     * Removes the rights associated with the given rights-protected content.
     *
     * @param uri URI for the rights-protected content.
     *
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     */
    public int removeRights(Uri uri) {
        return mDrmManagerClient.removeRights(uri);
    }

    /**
     * Removes all the rights information of every DRM plug-in (agent) associated with
     * the DRM framework. Will be used during a master reset.
     *
     * @return ERROR_NONE for success; ERROR_UNKNOWN for failure.
     */
    public int removeAllRights() {
        return mDrmManagerClient.removeAllRights();
    }

    /**
     * Installing the drm message file (.dm).
     *
     * @param uri Uri of the downloaded protected content (FL, CD, FLSD) in .dm format
     * @return ERROR_NONE for success ERROR_UNKNOWN for failure
     */
    public int installDrmMsg(Uri uri) {
        Log.v(TAG, "installDrmMsg : " + uri);

        if (null == uri || Uri.EMPTY == uri) {
            Log.e(TAG, "installDrmMsg : Given uri is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }

        String path = null;
        try {
            path = OmaDrmUtils.convertUriToPath(mContext, uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException @installDrmMsg : " + e.getMessage());
            return DrmManagerClient.ERROR_UNKNOWN;
        }
        return installDrmMsg(path);
    }

    /**
     * Installing the drm message file (.dm).
     *
     * @param path Path of the downloaded protected content (FL, CD, FLSD) in .dm format
     * @return ERROR_NONE for success ERROR_UNKNOWN for failure
     */
    public int installDrmMsg(String path) {
        Log.v(TAG, "installDrmMsg : " + path);

        if (null == path || path.equals("")) {
            Log.e(TAG, "installDrmMsg : Given path is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }

        // constructs the request and process it by acquireDrmInfo
        DrmInfoRequest request =
            new DrmInfoRequest(OmaDrmStore.DrmRequestType.TYPE_SET_DRM_INFO,
                               OmaDrmStore.DrmObjectMime.MIME_DRM_MESSAGE);
        request.put(OmaDrmStore.DrmRequestKey.KEY_ACTION,
                    OmaDrmStore.DrmRequestAction.ACTION_INSTALL_DRM_MSG);
        request.put(OmaDrmStore.DrmRequestKey.KEY_DATA, path); // path

        DrmInfo info = mDrmManagerClient.acquireDrmInfo(request);

        // get message from returned DrmInfo
        byte[] data = info.getData();
        String message = "";
        if (null != data) {
            try {
                // the information shall be in format of ASCII string
                message = new String(data, "US-ASCII");
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unsupported encoding type of the returned DrmInfo data");
                message = "";
            }
        }
        Log.v(TAG, "installDrmMsg : >" + message);

        return OmaDrmStore.DrmRequestResult.RESULT_SUCCESS.equals(message) ?
                DrmManagerClient.ERROR_NONE : DrmManagerClient.ERROR_UNKNOWN;
    }

    /**
     * Consume the rights associated with the given protected content
     *
     * @param uri Uri of the protected content
     * @param action The action it performs to use the content
     * @return ERROR_NONE for success ERROR_UNKNOWN for failure
     */
    public int consumeRights(Uri uri, int action) {
        Log.v(TAG, "consumeRights: " + uri + " with action " + action);

        if (null == uri || Uri.EMPTY == uri) {
            Log.e(TAG, "consumeRights : Given uri is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }
        if (!OmaDrmStore.Action.isValid(action)) {
            Log.e(TAG, "consumeRights : Given action is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }

        String path = null;
        try {
            path = OmaDrmUtils.convertUriToPath(mContext, uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException @consumeRights : " + e.getMessage());
            return DrmManagerClient.ERROR_UNKNOWN;
        }
        return consumeRights(path, action);
    }

   /** 
     * Consume the rights associated with the given protected content
     *
     * @param path Path of the protected content
     * @param action The action it performs to use the content
     * @return ERROR_NONE for success ERROR_UNKNOWN for failure
     */
    public int consumeRights(String path, int action) {
        Log.v(TAG, "consumeRights : " + path + " with action " + action);

        if (null == path || path.equals("")) {
            Log.e(TAG, "consumeRights : Given path is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }
        if (!OmaDrmStore.Action.isValid(action)) {
            Log.e(TAG, "consumeRights : Given action is not valid");
            return DrmManagerClient.ERROR_UNKNOWN;
        }

        // constructs the request and process it by acquireDrmInfo
        DrmInfoRequest request =
            new DrmInfoRequest(OmaDrmStore.DrmRequestType.TYPE_SET_DRM_INFO,
                               OmaDrmStore.DrmObjectMime.MIME_DRM_CONTENT);
        request.put(OmaDrmStore.DrmRequestKey.KEY_ACTION,
                    OmaDrmStore.DrmRequestAction.ACTION_CONSUME_RIGHTS);
        request.put(OmaDrmStore.DrmRequestKey.KEY_DATA, path); // path
        request.put(OmaDrmStore.DrmRequestKey.KEY_DATA_EXTRA_1, String.valueOf(action)); // action

        DrmInfo info = mDrmManagerClient.acquireDrmInfo(request);

        // get message from returned DrmInfo
        byte[] data = info.getData();
        String message = "";
        if (null != data) {
            try {
                // the information shall be in format of ASCII string
                message = new String(data, "US-ASCII");
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unsupported encoding type of the returned DrmInfo data");
                message = "";
            }
        }
        Log.v(TAG, "consumeRights : >" + message);

        return OmaDrmStore.DrmRequestResult.RESULT_SUCCESS.equals(message) ?
                DrmManagerClient.ERROR_NONE : DrmManagerClient.ERROR_UNKNOWN;
    }

    /**
     * Get drm method from drm content
     *
     * @param uri Uri of the protected content
     * @return int OmaDrmStore.DrmMethod
     */
    public int getMethod(Uri uri) {
        Log.v(TAG, "getMethod : " + uri);

        ContentValues cv = mDrmManagerClient.getMetadata(uri);
        if (cv != null && cv.containsKey(OmaDrmStore.MetadataKey.META_KEY_METHOD)) {
            Integer m = cv.getAsInteger(OmaDrmStore.MetadataKey.META_KEY_METHOD);
            if (m != null) {
                return m.intValue();
            }
        }
        return OmaDrmStore.DrmMethod.METHOD_NONE;
    }

    /**
     * Get drm method from drm content
     *
     * @param path Path of the protected content
     * @return int OmaDrmStore.DrmMethod
     */
    public int getMethod(String path) {
        Log.v(TAG, "getMethod : " + path);

        ContentValues cv = mDrmManagerClient.getMetadata(path);
        if (cv != null && cv.containsKey(OmaDrmStore.MetadataKey.META_KEY_METHOD)) {
            Integer m = cv.getAsInteger(OmaDrmStore.MetadataKey.META_KEY_METHOD);
            if (m != null) {
                return m.intValue();
            }
        }
        return OmaDrmStore.DrmMethod.METHOD_NONE;
    }

    /**
     * Checks whether the given rights-protected content has valid rights for the specified
     * Action
     *
     * ATTENTION: would return valid & invalid & secure_timer_invalid,
                  please use this API before showing LicenseAcquisition Dialog
     *
     * @param uri Uri of the rights-protected content
     * @param action The Action to perform
     * @return OmaDrmStore.RightsStatus
     */
    public int checkRightsStatusForTap(Uri uri, int action) {
        Log.v(TAG, "checkRightsStatusForTap : " + uri + " with action " + action);
        int result = mDrmManagerClient.checkRightsStatus(uri, action);
        Log.v(TAG, "checkRightsStatusForTap : result " + result);
        return result;
    }

    /**
     * Checks whether the given rights-protected content has valid rights for the specified
     * Action
     *
     * ATTENTION: would return valid & invalid & secure_timer_invalid,
                  please use this API before showing LicenseAcquisition Dialog
     *
     * @param path Path of the rights-protected content
     * @param action The Action to perform
     * @return OmaDrmStore.RightsStatus
     */
    public int checkRightsStatusForTap(String path, int action) {
        Log.v(TAG, "checkRightsStatusForTap : " + path + " with action " + action);
        int result = mDrmManagerClient.checkRightsStatus(path, action);
        Log.v(TAG, "checkRightsStatusForTap : result " + result);
        return result;
    }

    /**
     * when the RO file was downloaded / received, rescan corresponding DRM file
     * this can be used by Download Provider or Drm Provider module to process
     * separate delivery (SD) scenario
     *
     * @param context The application context
     * @param rights The DrmRights object it has received
     * @param callback OnDrmScanCompletedListener. may be null
     * @return ERROR_NONE for success ERROR_UNKNOWN for failure
     */
    public int rescanDrmMediaFiles(Context context, DrmRights rights,
            OmaDrmUtils.OnDrmScanCompletedListener callback) {
        // first we get the content-id. register a OnEventListener
        mDrmManagerClient.setOnEventListener(new getCidListener(context, callback));

        // constructs the DrmInfo and process it with processDrmInfo
        DrmInfo info =
            new DrmInfo(OmaDrmStore.DrmRequestType.TYPE_GET_DRM_INFO,
                        rights.getData(), rights.getMimeType());
        info.put(OmaDrmStore.DrmRequestKey.KEY_ACTION,
                 OmaDrmStore.DrmRequestAction.ACTION_GET_CONTENT_ID);

        int result = mDrmManagerClient.processDrmInfo(info);
        Log.d(TAG, "OmaDrmClient#rescanDrmMediaFiles: > " + result);

        return result;
    }

    private class getCidListener implements DrmManagerClient.OnEventListener {
        private Context mContext = null;
        private OmaDrmUtils.OnDrmScanCompletedListener mCallback = null;

        public getCidListener(Context context,
                OmaDrmUtils.OnDrmScanCompletedListener callback) {
            mContext = context;
            mCallback = callback;
        }

        public void onEvent(DrmManagerClient client, DrmEvent event) {
            DrmInfoStatus status =
                (DrmInfoStatus)(event.getAttribute(DrmEvent.DRM_INFO_STATUS_OBJECT));
            String cid = OmaDrmUtils.getMsgFromInfoStatus(status);

            // then rescan the corresponding Drm media file(s)
            int result = OmaDrmUtils.rescanDrmMediaFiles(mContext, cid, mCallback);
            Log.d(TAG, "OmaDrmUtils.rescanDrmMediaFiles: > " + result);
        }
    }

}

