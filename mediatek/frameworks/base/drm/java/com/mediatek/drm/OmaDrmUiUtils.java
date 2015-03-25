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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnDismissListener;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.graphics.Rect;
import android.net.Uri;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.text.SimpleDateFormat;

public class OmaDrmUiUtils {
    private static final String TAG = "OmaDrmUiUtils";
    private static final boolean OMA_DRM_FL_ONLY;

    static {
        // check system property tho determine if it has set Forward-Lock-Only
        String drmFLOnly = System.getProperty("drm.forwardlock.only", "no");
        OMA_DRM_FL_ONLY =
            drmFLOnly.equals("true") || drmFLOnly.equals("yes") || drmFLOnly.equals("1");
    }

    /**
     * Overlay back ground bitmap with front picture (with offset)
     * note that the OmaDrmClient instance shall be created in an Acitivty context
     *
     * @param client OmaDrmClient instance
     * @param bgdBmp Background bitmap to draw on
     * @param front Foreground drawable to draw
     * @return Bitmap New bitmap with overlayed drawable. null for failure
     */
    public static Bitmap overlayBitmapSkew(
            OmaDrmClient client, Bitmap bgdBmp, Drawable front) {
        if (null == bgdBmp || null == front || null == client) {
            Log.e(TAG, "overlayBitmapSkew : invalid parameters");
            return null;
        }

        // get DPI info and offset
        int offset = 0;
        Context context = client.getContext();
        if (context instanceof Activity) {
            DisplayMetrics metric = new DisplayMetrics();
            ((Activity)context).getWindowManager().getDefaultDisplay().getMetrics(metric);
            int densityDpi = metric.densityDpi;
            offset = 6 * DisplayMetrics.DENSITY_DEFAULT / densityDpi;
        } else {
            Log.e(TAG, "overlayBitmapSkew : not in Activity context @" + context);
        }

        Bitmap bMutable = Bitmap.createBitmap(bgdBmp.getWidth() + offset,
                                              bgdBmp.getHeight(),
                                              bgdBmp.getConfig());
        Canvas overlayCanvas = new Canvas(bMutable);
         // make sure the bitmap is valid otherwise we use an empty one
        if (!bgdBmp.isRecycled()) {
            overlayCanvas.drawBitmap(bgdBmp, 0, 0, null);
        }
        int overlayWidth = front.getIntrinsicWidth();
        int overlayHeight = front.getIntrinsicHeight();
        int left = bMutable.getWidth() - overlayWidth;
        int top = bMutable.getHeight() - overlayHeight;
        Rect newBounds = new Rect(left, top, left + overlayWidth, top + overlayHeight);
        front.setBounds(newBounds);
        front.draw(overlayCanvas);
        return bMutable;
    }

    /**
     * Overlay back ground bitmap with front picture (with offset)
     * note that the OmaDrmClient instance shall be created in an Acitivty context
     *
     * @param client OmaDrmClient instance
     * @param res The application resource
     * @param bgdBmp Background bitmap resource id
     * @param frontId Foregrond drawable resource id
     * @return Bitmap New bitmap with overlayed drawable
     */
    public static Bitmap overlayBitmapSkew(
            OmaDrmClient client, Resources res, int bgdBmpId, int frontId) {
        Bitmap bgdBmp = BitmapFactory.decodeResource(res, bgdBmpId);
        Drawable front = res.getDrawable(frontId);
        Bitmap bmp = overlayBitmapSkew(client, bgdBmp, front);
        if (bgdBmp != bmp && bgdBmp != null && !bgdBmp.isRecycled()) {
            bgdBmp.recycle();
            bgdBmp = null;
        }
        return bmp;
    }

    /**
     * Overlay back ground bitmap with front picture
     * note that the OmaDrmClient instance shall be created in an Acitivty context
     *
     * @param client OmaDrmClient instance
     * @param bgdBmp Background bitmap to draw on
     * @param front Foreground drawable to draw
     * @return Bitmap New bitmap with overlayed drawable
     */
    public static Bitmap overlayBitmap(
            OmaDrmClient client, Bitmap bgdBmp, Drawable front) {
        if (null == bgdBmp || null == front || null == client) {
            Log.e(TAG, "overlayBitmap : invalid parameters");
            return null;
        }

        Bitmap bMutable = Bitmap.createBitmap(bgdBmp.getWidth(),
                                              bgdBmp.getHeight(),
                                              bgdBmp.getConfig());
        Canvas overlayCanvas = new Canvas(bMutable);
         // make sure the bitmap is valid otherwise we use an empty one
        if (!bgdBmp.isRecycled()) {
            overlayCanvas.drawBitmap(bgdBmp, 0, 0, null);
        }
        int overlayWidth = front.getIntrinsicWidth();
        int overlayHeight = front.getIntrinsicHeight();
        int left = bgdBmp.getWidth() - overlayWidth;
        int top = bgdBmp.getHeight() - overlayHeight;
        Rect newBounds = new Rect(left, top, left + overlayWidth, top + overlayHeight);
        front.setBounds(newBounds);
        front.draw(overlayCanvas);
        return bMutable;
    }

    /**
     * Overlay back ground bitmap with front picture
     * note that the OmaDrmClient instance shall be created in an Acitivty context
     *
     * @param client OmaDrmClient instance
     * @param bgdBmp Background bitmap resource id
     * @param frontId Foregrond drawable resource id
     * @return Bitmap New bitmap with overlayed drawable
     */
    public static Bitmap overlayBitmap(
            OmaDrmClient client, Resources res, int bgdBmpId, int frontId) {
        Bitmap bgdBmp = BitmapFactory.decodeResource(res, bgdBmpId);
        Drawable front = res.getDrawable(frontId);
        Bitmap bmp = overlayBitmap(client, bgdBmp, front);
        if (bgdBmp != bmp && bgdBmp != null && !bgdBmp.isRecycled()) {
            bgdBmp.recycle();
            bgdBmp = null;
        }
        return bmp;
    }

    /**
     * Overlay a background with drm lock icon (with offset)
     *
     * @param client OmaDrmClient instance
     * @param res The application resource
     * @param path Path of drm protected content
     * @param action Action type of drm protected cotent
     * @param bgdBmp Background bitmap
     * @return Bitmap New bitmap with overlayed icon
     */
    public static Bitmap overlayDrmIconSkew(
            OmaDrmClient client, Resources res, String path, int action, Bitmap bgdBmp) {
        int method = client.getMethod(path);
        if (method == OmaDrmStore.DrmMethod.METHOD_NONE) {
            Log.d(TAG, "overlayDrmIconSkew : not drm type, no icon overlayed");
            return bgdBmp;
        }
        if (method == OmaDrmStore.DrmMethod.METHOD_FL) {
            Log.d(TAG, "overlayDrmIconSkew : method FL, no icon overlayed");
            return bgdBmp;
        }

        int rightsStatus = client.checkRightsStatus(path, action);
        int lockId = (rightsStatus == OmaDrmStore.RightsStatus.RIGHTS_VALID) ?
                com.mediatek.internal.R.drawable.drm_green_lock :
                com.mediatek.internal.R.drawable.drm_red_lock;

        Drawable front = res.getDrawable(lockId);
        return overlayBitmapSkew(client, bgdBmp, front);
    }

    /**
     * Overlay a background with drm lock icon (with offset)
     *
     * @param client OmaDrmClient instance
     * @param res The application resource
     * @param path Path of drm protected content
     * @param action Action type of drm protected cotent
     * @param bgdBmpId Background bitmap resource id
     * @return Bitmap New bitmap with overlayed icon
     */
    public static Bitmap overlayDrmIconSkew(
            OmaDrmClient client, Resources res, String path, int action, int bgdBmpId) {
        Bitmap bgdBmp = BitmapFactory.decodeResource(res, bgdBmpId);
        Bitmap bmp = overlayDrmIconSkew(client, res, path, action, bgdBmp);
        if (bgdBmp != bmp && bgdBmp != null && !bgdBmp.isRecycled()) {
            bgdBmp.recycle();
            bgdBmp = null;
        }
        return bmp;
    }

    /**
     * Overlay a background with drm lock icon
     *
     * @param client OmaDrmClient instance
     * @param res The application resource
     * @param path Path of drm protected content
     * @param action Action type of drm protected cotent
     * @param bgdBmp Background bitmap
     * @return Bitmap New bitmap with overlayed icon
     */
    public static Bitmap overlayDrmIcon(
            OmaDrmClient client, Resources res, String path, int action, Bitmap bgdBmp) {
        int method = client.getMethod(path);
        if (method == OmaDrmStore.DrmMethod.METHOD_NONE) {
            Log.d(TAG, "overlayDrmIcon : not drm type, no icon overlayed");
            return bgdBmp;
        }
        if (method == OmaDrmStore.DrmMethod.METHOD_FL) {
            Log.d(TAG, "overlayDrmIcon : method FL, no icon overlayed");
            return bgdBmp;
        }

        int rightsStatus = client.checkRightsStatus(path, action);
        int lockId = (rightsStatus == OmaDrmStore.RightsStatus.RIGHTS_VALID) ?
                com.mediatek.internal.R.drawable.drm_green_lock :
                com.mediatek.internal.R.drawable.drm_red_lock;

        Drawable front = res.getDrawable(lockId);
        return overlayBitmap(client, bgdBmp, front);
    }

    /**
     * Overlay a background with drm lock icon
     *
     * @param client OmaDrmClient instance
     * @param res The application resource
     * @param path Path of drm protected content
     * @param action Action type of drm protected cotent
     * @param bgdBmpId Background bitmap resource id
     * @return Bitmap New bitmap with overlayed icon
     */
    public static Bitmap overlayDrmIcon(
            OmaDrmClient client, Resources res, String path, int action, int bgdBmpId) {
        Bitmap bgdBmp = BitmapFactory.decodeResource(res, bgdBmpId);
        Bitmap bmp = overlayDrmIcon(client, res, path, action, bgdBmp);
        if (bgdBmp != bmp && bgdBmp != null && !bgdBmp.isRecycled()) {
            bgdBmp.recycle();
            bgdBmp = null;
        }
        return bmp;
    }

    /**
     * Listener for Drm dialogs.
     */
    public interface DrmOperationListener {
        /**
         * User's operation will continue the process.
         * App should continue the procedure when receiving this type.
         */
        int CONTINUE = 1;
        /**
         * User's operation will stop the process.
         * App should stop the procedure when receiving this type.
         */
        int STOP = 2;

        /**
         * operation callback.
         */
        void onOperated(int type);
    }

    /**
     * show a dialog indicating that the current operation can't be performed
     *   because the secure timer is in invalid state.
     * the dialog only have one button "ok"
     *
     * @param context Context should be an Activity context
     * @param clickListener OnClickListener should listen for DialogInterface.BUTTON_POSITIVE
     * @param dismissListener OnDismissListener should listen for dimiss
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showSecureTimerInvalidDialog(Context context,
            DialogInterface.OnClickListener clickListener,
            DialogInterface.OnDismissListener dismissListener) {
        Log.d(TAG, "showSecureTimerInvalidDialog @" + context);

        if (!(context instanceof Activity)) {
            Log.e(TAG, "showSecureTimerInvalidDialog : not an Acitivty context");
            return null;
        }

        Dialog result = validateCustomAlertDialog(OmaDrmClient.sSecureTimerDialogQueue, context);
        if (null != result) {
            return result;
        }

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sSecureTimerDialogQueue);
        Resources res = context.getResources();
        dialog.setButton(Dialog.BUTTON_POSITIVE,
                         res.getString(android.R.string.ok),
                         clickListener);
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_secure_timer_title);
        dialog.setMessage(res.getString(com.mediatek.internal.R.string.drm_secure_timer_message));
        dialog.setOnDismissListener(dismissListener);
        dialog.show();

        return dialog;
    }

    /**
     * show a dialog indicating that the operation will consume drm rights and
     *   prompts user to choose accept or cancel
     *
     * @param context Context should be an Activity context
     * @param listener OnClickListener should listen for DialogInterface.BUTTON_POSITIVE, BUTTON_NEGATIVE
     * @param dismissListener OnDismissListener should listen for dimiss
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showConsumeDialog(Context context,
            DialogInterface.OnClickListener listener,
            DialogInterface.OnDismissListener dismissListener) {
        Log.d(TAG, "showConsumeDialog @" + context);

        if (!(context instanceof Activity)) {
            Log.e(TAG, "showConsumeDialog : not an Acitivty context");
            return null;
        }

        checkCustomAlertDialog(OmaDrmClient.sLicenseDialogQueue, context);
        Dialog result = validateCustomAlertDialog(OmaDrmClient.sConsumeDialogQueue, context);
        if (null != result) {
            return result;
        }

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sConsumeDialogQueue);
        Resources res = context.getResources();
        dialog.setButton(Dialog.BUTTON_POSITIVE,
                         res.getString(android.R.string.ok),
                         listener);
        dialog.setButton(Dialog.BUTTON_NEGATIVE,
                         res.getString(android.R.string.cancel),
                         listener);
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_consume_title);
        dialog.setMessage(res.getString(com.mediatek.internal.R.string.drm_consume_message));
        dialog.setOnDismissListener(dismissListener);
        dialog.show();

        return dialog;
    }

    /**
     * show a dialog to display the protection information for protected content
     *
     * @param context Context should be an Activity context
     * @param uri Uri of the protected content
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showProtectionInfoDialog(Context context, Uri uri) {
        if (null == uri || Uri.EMPTY == uri) {
            Log.e(TAG, "showProtectionInfoDialog : Given uri is not valid");
            return null;
        }

        String path = null;
        try {
            path = OmaDrmUtils.convertUriToPath(context, uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException @showProtectionInfoDialog : " + e.getMessage());
            return null;
        }
        return showProtectionInfoDialog(context, path);
    }

    /**
     * show a dialog to display the protection information for protected content
     *
     * @param context Context should be an Activity context
     * @param path Path of the protected content
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showProtectionInfoDialog(final Context context, String path) {
        Log.d(TAG, "showProtectionInfoDialog : " + path + " @" + context);

        if (null == path || path.equals("")) {
            Log.e(TAG, "showProtectionInfoDialog : Given path is not valid");
            return null;
        }

        Dialog result = validateCustomAlertDialog(OmaDrmClient.sProtectionInfoDialogQueue, context);
        if (null != result) {
            return result;
        }

        final View scrollView =
            View.inflate(context,
                com.mediatek.internal.R.layout.drm_protectioninfoview, null);
        TextView fileNameView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_file_name_value);
        if (fileNameView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: fileNameView is null");
            return null;
        }

        int start = path.lastIndexOf("/");
        int end = path.lastIndexOf(".");
        String fileNameStr = path.substring(start + 1, end);
        fileNameView.setText(fileNameStr);

        TextView protectionInfoStatusView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_protection_status_value);
        if (protectionInfoStatusView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: protectionInfoStatusView is null");
            return null;
        }

        OmaDrmClient client = new OmaDrmClient(context);
        int rightsStatus = client.checkRightsStatus(path, OmaDrmStore.Action.TRANSFER);
        int canForward = ((rightsStatus == OmaDrmStore.RightsStatus.RIGHTS_VALID) ?
            com.mediatek.internal.R.string.drm_can_forward :
            com.mediatek.internal.R.string.drm_can_not_forward);
        protectionInfoStatusView.setText(canForward);

        TextView beginView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_begin);
        if (beginView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: beginView is null");
            return null;
        }
        TextView endView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_end);
        if (endView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: endView is null");
            return null;
        }
        TextView useLeftView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_use_left);
        if (useLeftView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: useLeftView is null");
            return null;
        }
        TextView beginValueView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_begin_value);
        if (beginValueView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: beginValueView is null");
            return null;
        }
        TextView endValueView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_end_value);
        if (endValueView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: endValueView is null");
            return null;
        }
        TextView useLeftValueView =
            (TextView)scrollView.findViewById(
                com.mediatek.internal.R.id.drm_use_left_value);
        if (useLeftValueView == null) {
            Log.e(TAG, "showProtectionInfoDialog : the TextView: useLeftValueView is null");
            return null;
        }

        String mime = client.getOriginalMimeType(path);
        if (null == mime) {
            Log.e(TAG, "showProtectionInfoDialog : failed to get the original mime type");
            return null;
        }

        CustomAlertDialog dialog =
            new CustomAlertDialog(context, OmaDrmClient.sProtectionInfoDialogQueue);
        Resources res = context.getResources();
        ContentValues values = client.getConstraints(path, 
                                   OmaDrmUtils.getMediaActionType(mime));
        if (values == null || values.size() == 0) {
            beginView.setText(com.mediatek.internal.R.string.drm_no_license);
            endView.setText("");
            useLeftView.setText("");

            ContentValues cv = client.getMetadata(path);
            String rightsIssuer = null;
            if (cv != null && cv.containsKey(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER)) {
                rightsIssuer = cv.getAsString(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER);
            }

            // the rights issuer is valid, we have a "renew" button in dialog
            final String rightsIssuerFinal = rightsIssuer;
            if (rightsIssuerFinal != null && !rightsIssuerFinal.isEmpty()) {
                dialog.setButton(Dialog.BUTTON_POSITIVE,
                    res.getString(com.mediatek.internal.R.string.drm_protectioninfo_renew),
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            //renew rights, start browser
                            Intent it = new Intent(Intent.ACTION_VIEW, Uri.parse(rightsIssuerFinal));
                            context.startActivity(it);
                            dialog.dismiss();
                        }
                    }
                );
            }
        } else {
            if (values.containsKey(OmaDrmStore.ConstraintsColumns.LICENSE_START_TIME)) {
                Long startL = values.getAsLong(
                                  OmaDrmStore.ConstraintsColumns.LICENSE_START_TIME);
                if (startL == null) {
                    Log.e(TAG, "showProtectionInfoDialog : startL is null");
                    return null;
                }
                if (startL == -1) {
                    beginValueView.setText(
                        com.mediatek.internal.R.string.drm_no_limitation);
                } else {
                    beginValueView.setText(toDateTimeString(startL));
                }
            } else {
                beginValueView.setText(com.mediatek.internal.R.string.drm_no_limitation);
            }

            if (values.containsKey(OmaDrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME)) {
                Long endL = values.getAsLong(
                                OmaDrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME);
                if (endL == null) {
                    Log.e(TAG, "showProtectionInfoDialog : endL is null");
                    return null;
                }
                if (endL == -1) {
                    endValueView.setText(
                        com.mediatek.internal.R.string.drm_no_limitation);
                } else {
                    endValueView.setText(toDateTimeString(endL));
                }
            } else {
                endValueView.setText(com.mediatek.internal.R.string.drm_no_limitation);
            }

            if (values.containsKey(OmaDrmStore.ConstraintsColumns.REMAINING_REPEAT_COUNT)
                    && values.containsKey(OmaDrmStore.ConstraintsColumns.MAX_REPEAT_COUNT)) {
                Long remainCount = values.getAsLong(
                                       OmaDrmStore.ConstraintsColumns.REMAINING_REPEAT_COUNT);
                if (remainCount == null) {
                    Log.e(TAG, "showProtectionInfoDialog : remainCount is null");
                    return null;
                }
                Long maxCount = values.getAsLong(
                                    OmaDrmStore.ConstraintsColumns.MAX_REPEAT_COUNT);
                if (maxCount == null) {
                    Log.e(TAG, "showProtectionInfoDialog() : maxCount is null");
                    return null;
                }
                if (remainCount == -1 || maxCount == -1) {
                    useLeftValueView.setText(
                        com.mediatek.internal.R.string.drm_no_limitation);
                } else {
                    useLeftValueView.setText(
                        remainCount.toString() + "/" + maxCount.toString());
                }
            } else {
                useLeftValueView.setText(com.mediatek.internal.R.string.drm_no_limitation);
            }
        }

        // we always have a "OK" button which does nothing.
        dialog.setButton(Dialog.BUTTON_NEUTRAL,
            res.getString(android.R.string.ok),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            }
        );
        dialog.setTitle(com.mediatek.internal.R.string.drm_protectioninfo_title);
        dialog.setView(scrollView);
        dialog.show();

        return dialog;
    }

    /**
     * Show license refresh dialog.
     *
     * @param client OmaDrmClient instance
     * @param context Context should be an Activity context
     * @param uri Uri of the protected content
     * @param dismissListener OnDismissListener
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showRefreshLicenseDialog(OmaDrmClient client, Context context,
            Uri uri, DialogInterface.OnDismissListener dismissListener) {
        Log.d(TAG, "showRefreshLicenseDialog : " + uri + " @" + context);

        if (OMA_DRM_FL_ONLY) {
            Log.d(TAG, "showRefreshLicenseDialog : Forward-lock-only is set.");
            return null;
        }
        if (null == uri || Uri.EMPTY == uri) {
            Log.e(TAG, "showRefreshLicenseDialog : Given uri is not valid");
            return null;
        }
        if (!(context instanceof Activity)) {
            Log.e(TAG, "showRefreshLicenseDialog : not an Activity context");
            return null;
        }

        String path = null;
        try {
            path = OmaDrmUtils.convertUriToPath(context, uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException @showRefreshLicenseDialog : " + e.getMessage());
            return null;
        }
        return showRefreshLicenseDialog(client, context, path, dismissListener);
    }

    /**
     * Show license refresh dialog.
     *
     * @param client OmaDrmClient instance
     * @param context Context should be an Activity context
     * @param path Path of the protected content
     * @param dismissListener OnDismissListener
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showRefreshLicenseDialog(OmaDrmClient client,
            final Context context, String path,
            DialogInterface.OnDismissListener dismissListener) {
        Log.d(TAG, "showRefreshLicenseDialog : " + path + " @" + context);

        if (OMA_DRM_FL_ONLY) {
            Log.d(TAG, "showRefreshLicenseDialog : Forward-lock-only is set.");
            return null;
        }
        if (null == path || path.equals("")) {
            Log.e(TAG, "showRefreshLicenseDialog : Given path is not valid");
            return null;
        }
        if (!(context instanceof Activity)) {
            Log.e(TAG, "showRefreshLicenseDialog : not an Activity context");
            return null;
        }

        checkCustomAlertDialog(OmaDrmClient.sConsumeDialogQueue, context);
        Dialog result = validateCustomAlertDialog(OmaDrmClient.sLicenseDialogQueue, context);
        if (null != result) {
            return result;
        }

        // try to get rights-issuer
        ContentValues cv = client.getMetadata(path);
        String rightsIssuer = null;
        if (cv != null && cv.containsKey(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER)) {
            rightsIssuer = cv.getAsString(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER);
        }

        // for Combined Delivery, there's no rights-issuer
        if (rightsIssuer == null || !rightsIssuer.startsWith("http")) {
            Toast.makeText(context,
                com.mediatek.internal.R.string.drm_toast_license_expired,
                Toast.LENGTH_LONG).show();
            return null;
        }

        // valid rights-issuer for SD/FLSD
        Resources res = context.getResources();
        String message =
            String.format(
                res.getString(com.mediatek.internal.R.string.drm_licenseacquisition_message),
                path);
        final String rightsIssuerFinal = rightsIssuer;

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sLicenseDialogQueue);
        // OK to launch browser and dismiss
        dialog.setButton(Dialog.BUTTON_POSITIVE,
            res.getString(com.mediatek.internal.R.string.drm_protectioninfo_renew),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    Log.d(TAG, "showRefreshLicenseDialog: start to refresh license");
                    Intent it =
                        new Intent(Intent.ACTION_VIEW, Uri.parse(rightsIssuerFinal));
                    context.startActivity(it);
                }
            }
        );
        // CANCEL to do nothing
        dialog.setButton(Dialog.BUTTON_NEGATIVE,
            res.getString(android.R.string.cancel),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    // nothing
                }
            }
        );
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_licenseacquisition_title);
        dialog.setMessage(message);
        dialog.setOnDismissListener(dismissListener);
        dialog.show();

        return dialog;
    }

    /**
     * Show license refresh dialog.
     *
     * @param client OmaDrmClient instance
     * @param context Context
     * @param uri Uri of the protected content
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showRefreshLicenseDialog(OmaDrmClient client,
            Context context, Uri uri) {
        return showRefreshLicenseDialog(client, context, uri, null);
    }

    /**
     * Show license refresh dialog.
     *
     * @param client OmaDrmClient instance
     * @param context Context
     * @param path Path of the protected content
     * @return Dialog The dialog being shown. may be null
     */
    public static Dialog showRefreshLicenseDialog(OmaDrmClient client,
            Context context, String path) {
        return showRefreshLicenseDialog(client, context, path, null);
    }


    /**
     * specific for VideoPlayer use
     */
    public static Dialog showConsumeRights(
            OmaDrmClient client, Context context,
            final DrmOperationListener listener) {
        Log.d(TAG, "showConsumeRights @" + context);

        checkCustomAlertDialog(OmaDrmClient.sLicenseDialogQueue, context);
        Dialog result = validateCustomAlertDialog(OmaDrmClient.sConsumeDialogQueue, context);
        if (null != result) {
            return result;
        }

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sConsumeDialogQueue);
        Resources res = context.getResources();
        // OK to continue
        dialog.setButton(Dialog.BUTTON_POSITIVE,
            res.getString(android.R.string.ok),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (listener != null) {
                        Log.d(TAG, "showConsumeRights: start to consume rights");
                        listener.onOperated(DrmOperationListener.CONTINUE);
                    }
                }
            }
        );
        // CANCEL to stop
        dialog.setButton(Dialog.BUTTON_NEGATIVE,
            res.getString(android.R.string.cancel),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    if (listener != null) {
                        Log.d(TAG, "showConsumeRights: cancel to consume rights");
                        listener.onOperated(DrmOperationListener.STOP);
                    }
                }
            }
        );
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_consume_title);
        dialog.setMessage(res.getString(com.mediatek.internal.R.string.drm_consume_message));
        // the "back" key shall also perform STOP
        dialog.setOnCancelListener(
            new OnCancelListener() {
                public void onCancel(DialogInterface dialog) {
                    if (listener != null) {
                        Log.d(TAG, "showConsumeRights: DrmOperationListener STOP");
                        listener.onOperated(DrmOperationListener.STOP);
                    }
                }
            }
        );
        dialog.show();

        return dialog;
    }

    /**
     * specific for VideoPlayer use
     */
    public static Dialog showSecureTimerInvalid(
            OmaDrmClient client, Context context,
            final DrmOperationListener listener) {
        Log.d(TAG, "showSecureTimerInvalid @" + context);

        Dialog result = validateCustomAlertDialog(OmaDrmClient.sSecureTimerDialogQueue, context);
        if (null != result) {
            return result;
        }

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sSecureTimerDialogQueue);
        Resources res = context.getResources();
        // OK to do nothing and dismiss
        dialog.setButton(Dialog.BUTTON_POSITIVE,
            res.getString(android.R.string.ok),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    // nothing
                }
            }
        );
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_secure_timer_title);
        dialog.setMessage(res.getString(com.mediatek.internal.R.string.drm_secure_timer_message));
        // when dismiss, cause the drm operation listener to STOP
        dialog.setOnDismissListener(
            new OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    if (listener != null) {
                        Log.d(TAG, "showSecureTimerInvalid : DrmOperationListener STOP");
                        listener.onOperated(DrmOperationListener.STOP);
                    }
                }
            }
        );
        dialog.show();

        return dialog;
    }

    /**
     * specific for VideoPlayer use
     */
    public static Dialog showRefreshLicense(
            OmaDrmClient client, Context context, Uri uri,
            DrmOperationListener listener) {
        if (OMA_DRM_FL_ONLY) {
            Log.d(TAG, "showRefreshLicense : Forward-lock-only is set");
            return null;
        }
        if (null == uri || Uri.EMPTY == uri) {
            Log.e(TAG, "showRefreshLicense : invalid uri");
            return null;
        }
        if (null == context || !(context instanceof Activity)) {
            Log.e(TAG, "showRefreshLicense : invalid context or not an Activity context");
            return null;
        }

        String path = null;
        try {
            path = OmaDrmUtils.convertUriToPath(context, uri);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumenException @showRefreshLicense : " + e.getMessage());
            return null;
        }
        return showRefreshLicense(client, context, path, listener);
    }

    /**
     * specific for VideoPlayer use
     */
    public static Dialog showRefreshLicense(
            OmaDrmClient client, final Context context, String path,
            final DrmOperationListener listener) {
        Log.d(TAG, "showRefreshLicense @" + context);

        if (OMA_DRM_FL_ONLY) {
            Log.d(TAG, "showRefreshLicense : Forward-lock-only is set");
            return null;
        }
        if (null == path || path.equals("")) {
            Log.e(TAG, "showRefreshLicense : invalid path");
            return null;
        }
        if (null == context || !(context instanceof Activity)) {
            Log.e(TAG, "showRefreshLicense : invalid context or not an Activity context");
            return null;
        }

        checkCustomAlertDialog(OmaDrmClient.sConsumeDialogQueue, context);
        Dialog result = validateCustomAlertDialog(OmaDrmClient.sLicenseDialogQueue, context);
        if (null != result) {
            return result;
        }

        // try to get right-issuer
        ContentValues cv = client.getMetadata(path);
        String rightsIssuer = null;
        if (cv != null && cv.containsKey(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER)) {
            rightsIssuer = cv.getAsString(OmaDrmStore.MetadataKey.META_KEY_RIGHTS_ISSUER);
        }

        // for Combined Delivery, there's no rights-issuer
        if (rightsIssuer == null || !rightsIssuer.startsWith("http")) {
            Toast.makeText(context,
                com.mediatek.internal.R.string.drm_toast_license_expired,
                Toast.LENGTH_LONG).show();
            if (listener != null) {
                listener.onOperated(DrmOperationListener.STOP);
            }
            return null;
        }

        // valid rights-issuer for SD/FLSD
        Resources res = context.getResources();
        String message =
            String.format(
                res.getString(com.mediatek.internal.R.string.drm_licenseacquisition_message),
                path);
        final String rightsIssuerFinal = rightsIssuer;

        CustomAlertDialog dialog = new CustomAlertDialog(context, OmaDrmClient.sLicenseDialogQueue);
        // OK to launch browser and dismiss
        dialog.setButton(Dialog.BUTTON_POSITIVE,
            res.getString(com.mediatek.internal.R.string.drm_protectioninfo_renew),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    Log.d(TAG, "showRefreshLicense: start to refresh license");
                    Intent it =
                        new Intent(Intent.ACTION_VIEW, Uri.parse(rightsIssuerFinal));
                    context.startActivity(it);
                }
            }
        );
        // CANCEL to do nothing and dismiss
        dialog.setButton(Dialog.BUTTON_NEGATIVE,
            res.getString(android.R.string.cancel),
            new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    // nothing
                }
            }
        );
        dialog.setIcon(android.R.drawable.ic_dialog_info);
        dialog.setTitle(com.mediatek.internal.R.string.drm_licenseacquisition_title);
        dialog.setMessage(message);
        // when dismiss, cause the drm operation listener to STOP
        dialog.setOnDismissListener(
            new OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    if (listener != null) {
                        Log.d(TAG, "showRefreshLicense: DrmOperationListener STOP");
                        listener.onOperated(DrmOperationListener.STOP);
                    }
                }
            }
        );
        dialog.show();

        return dialog;
    }

    /**
     * Convert seconds to date time string.
     *
     * @param sec The seconds count from 1970-1-1 00:00:00
     * @return String Date time string
     */
    private static String toDateTimeString(Long sec) {
        Date date = new Date(sec.longValue() * 1000L);
        SimpleDateFormat dateFormat = new SimpleDateFormat();
        String str = dateFormat.format(date);
        return str;
    }

    /*
     * The custom alert dialog which DRM dialogs would use.
     */
    static class CustomAlertDialog extends AlertDialog {

        private DialogInterface.OnDismissListener mDismissListener = null;
        private DialogInterface.OnShowListener mShowListener = null;
        private ArrayList<CustomAlertDialog> mQueue = null;
        private Context mContext = null;

        public CustomAlertDialog(Context context, ArrayList<CustomAlertDialog> queue) {
            super(context);
            mQueue = queue;
            mContext = context;

            super.setOnDismissListener(
                new DialogInterface.OnDismissListener() {
                    public void onDismiss(DialogInterface dialog) {
                        if (null != mDismissListener) {
                            Log.d(TAG, "CustomerAlertDialog: execute the original dismiss listener");
                            mDismissListener.onDismiss(dialog);
                        }
                        // remove this from list
                        if (null != mQueue) {
                            synchronized(mQueue) {
                                Log.d(TAG, "CustomerAlertDialog: remove this dialog from queue");
                                mQueue.remove(CustomAlertDialog.this);
                            }
                        }
                        mQueue = null;
                    }
                }
            );
            super.setOnShowListener(
                new DialogInterface.OnShowListener() {
                    public void onShow(DialogInterface dialog) {
                        if (null != mShowListener) {
                            Log.d(TAG, "CustomerAlertDialog: execute the original show listener");
                            mShowListener.onShow(dialog);
                        }
                        // remove this from list
                        if (null != mQueue) {
                            synchronized(mQueue) {
                                Log.d(TAG, "CustomerAlertDialog: add this dialog to queue");
                                mQueue.add(CustomAlertDialog.this);
                            }
                        }
                    }
                }
            );
        }

        @Override
        public void setOnDismissListener(DialogInterface.OnDismissListener listener) {
            mDismissListener = listener;
        }

        @Override
        public void setOnShowListener(DialogInterface.OnShowListener listener) {
            mShowListener = listener;
        }

        public final Context getCreatorContext() {
            return mContext;
        }
    };

    private static Dialog validateCustomAlertDialog(ArrayList<CustomAlertDialog> list, Context context) {
        Log.d(TAG, "validateCustomAlertDialog : validate existing dialog @" + context);
        Dialog result = null;
        // go through all the dialog in the list, and if the context matches,
        // return the dialog as existing available one
        synchronized(list) {
            Iterator<CustomAlertDialog> iter = list.iterator();
            while (iter.hasNext()) {
                CustomAlertDialog dialog = iter.next();
                Log.d(TAG, "validateCustomAlertDialog : existing dialog @" + dialog.getCreatorContext());
                if (dialog.getCreatorContext().equals(context)) {
                    Log.d(TAG, "validateCustomAlertDialog : context match, use this one");
                    result = dialog;
                    break;
                }
            }
        }
        return result;
    }

    private static void checkCustomAlertDialog(ArrayList<CustomAlertDialog> list, Context context) {
        Log.d(TAG, "checkCustomAlertDialog : check existing dialog @" + context);
        // go through all the dialog in the list, and if the context matches,
        // dismiss it.
        synchronized(list) {
            Iterator<CustomAlertDialog> iter = list.iterator();
            while (iter.hasNext()) {
                CustomAlertDialog dialog = iter.next();
                Log.d(TAG, "checkCustomAlertDialog : existing dialog @" + dialog.getCreatorContext());
                if (dialog.getCreatorContext().equals(context)) {
                    Log.d(TAG, "checkCustomAlertDialog : context match, dismiss it");
                    dialog.dismiss();
                    break;
                }
            }
        }
    }

}

