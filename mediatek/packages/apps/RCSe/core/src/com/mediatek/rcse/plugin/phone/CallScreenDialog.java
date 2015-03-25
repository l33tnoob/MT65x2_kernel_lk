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

package com.mediatek.rcse.plugin.phone;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.graphics.drawable.Drawable;
import android.view.View;
import android.widget.ListAdapter;

import com.mediatek.rcse.api.Logger;

/**
 * A common alert dialog class for rich call.Example: CallScreenDialog dialog =
 * new
 * CallScreenDialog(context);dialog.setTitle(title);dialog.setTitle.setMessage
 * (message
 * );dialog.setIcon(icon);dialog.setsetPositiveButton("ok",listener);dialog
 * .show();
 */
public class CallScreenDialog {
    private static final String TAG = "CallScreenDialog";
    private AlertDialog.Builder mBuilder = null;
    private AlertDialog mAlertDialog = null;
    private int mIcon = -1;

    public CallScreenDialog(Context context) {
        Logger.v(TAG, "CallScreenDialog(),context = " + context);
        mBuilder = new AlertDialog.Builder(context);
    }

    /**
     * Set ok button display string and listener
     * 
     * @param displayString
     * @param postiveListener A listener to respond ok button's click event
     */
    public void setPositiveButton(String displayString,
            DialogInterface.OnClickListener postiveListener) {
        Logger.v(TAG, "setPositiveButton(), displayString = " + displayString
                + ",postiveListener = " + postiveListener);
        mBuilder.setPositiveButton(displayString, postiveListener);
    }

    /**
     * Set cancel button display string and listener
     * 
     * @param displayString
     * @param negtiveListener A listener to respond cancel button's click event
     */
    public void setNegativeButton(String displayString,
            DialogInterface.OnClickListener negtiveListener) {
        Logger.v(TAG, "setPositiveButton(), displayString = " + displayString
                + ",negtiveListener = " + negtiveListener);

        mBuilder.setNegativeButton(displayString, negtiveListener);
    }

    public void setIcon(int iconId) {
        Logger.v(TAG, "setIcon(), iconId = " + iconId);
        mIcon = iconId;
    }

    public void setIcon(Drawable icon) {
        Logger.v(TAG, "setIcon()");
        mBuilder.setIcon(icon);
    }

    /**
     * Set the title will be show on the dialog
     * 
     * @param title The title will be show on the dialog
     */
    public void setTitle(String title) {
        mBuilder.setTitle(title);
    }

    /**
     * Set the content will be show on the dialog
     * 
     * @param message The content will be show on the dialog
     */
    public void setMessage(String message) {
        mBuilder.setMessage(message);
    }

    /**
     * Set content view.
     * 
     * @param view The view to be set.
     */
    public void setContent(View view) {
        mBuilder.setView(view);
    }

    /**
     * Show the dialog.
     */
    public void show() {
        Logger.v(TAG, "show Entry");
        mAlertDialog = null;
        mAlertDialog = mBuilder.create();
        if (mIcon != -1) {
            mAlertDialog.setIconAttribute(mIcon);
        }
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            dismissDialog();
        }
        mAlertDialog.show();
        Logger.v(TAG, "show Exit");
    }

    /**
     * Set cancelable
     * 
     * @param cancelable True if cancelable, else false.
     */
    public void setCancelable(boolean cancelable) {
        Logger.v(TAG, "setCancelable");
        mBuilder.setCancelable(cancelable);
    }

    /**
     * Dismiss the dialog.
     */
    public void dismissDialog() {
        Logger.v(TAG, "dismissDialog()");
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
        } else {
            Logger.w(TAG, "mAlertDialog is null");
        }
    }

    /**
     * Set single choice items.
     * 
     * @param adapter The adapter contains the items.
     * @param checkedItem The checked item.
     * @param listener The click listener for item.
     */
    public void setSingleChoiceItems(ListAdapter adapter, int checkedItem, OnClickListener listener) {
        Logger.v(TAG, "setSingleChoiceItems");
        mBuilder.setSingleChoiceItems(adapter, checkedItem, listener);
    }

    /**
     * set Cancel Listener
     * 
     * @param DialogInterface.OnCancelListener
     */
    public void setCancelListener(DialogInterface.OnCancelListener listener) {
        Logger.v(TAG, "setCancelListener Entry");
        mBuilder.setOnCancelListener(listener);
        Logger.v(TAG, "setCancelListener Exit");
    }
}
