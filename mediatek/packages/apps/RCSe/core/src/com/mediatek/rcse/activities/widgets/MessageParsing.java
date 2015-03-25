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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.MailTo;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Browser;
import android.text.style.URLSpan;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;

import com.orangelabs.rcs.R;

import java.util.ArrayList;

/**
 * Check the links in message text view. and ask user to pick the relevant
 * application.
 */
public class MessageParsing {
    private static final String TAG = "MessageParsing";
    private static final String TEL_PREFIX = "tel:";
    private static final String SMS_PREFIX = "smsto:";
    private static final String MAIL_PREFIX = "mailto:";
    private static final String RCSE_PREFIX = "rcseto:";
    private static final int NO_URL = 0;
    private static final int SINGLE_URL = 1;
    private static final String ADD_CONTACT = "addContacts";

    /**
     * Check for links. If none, do nothing; if number or more than one link,
     * ask user to pick one
     * 
     * @param messageView the Text view clicked.
     */
    public static void onChatMessageClick(View messageView) {
        Logger.d(TAG, "onChatMessageClick() entry ");
        URLSpan[] spans = ((TextView) messageView).getUrls();
        final Activity activity = (Activity) messageView.getContext();
        boolean hasTel = false;
        final ArrayList<String> urls = extractUris(spans);
        String url = null;
        int spanSize = spans.length;
        for (int i = 0; i < spanSize; i++) {
            url = urls.get(i);
            if (url.startsWith(TEL_PREFIX)) {
                hasTel = true;
                urls.add(RCSE_PREFIX + url.substring(TEL_PREFIX.length()));
                urls.add(SMS_PREFIX + url.substring(TEL_PREFIX.length()));
            }
        }
        if (spanSize == NO_URL) {
            Logger.d(TAG, "onChatMessageClick() there is no url just return");
            return;
        } else if (spanSize == SINGLE_URL && !hasTel) {
            Logger.d(TAG, "onChatMessageClick() size is 1 and not has tel");
            String urlSingle = urls.get(0);
            MultiAlertDialog browserAlertDialog = new MultiAlertDialog();
            browserAlertDialog.setActivity(activity);
            browserAlertDialog.setUrl(urlSingle);
            if (!urlSingle.startsWith(MAIL_PREFIX)) {
                Logger.d(TAG, "onChatMessageClick() size is 1 and start with browser");
                browserAlertDialog.show(activity.getFragmentManager(), MultiAlertDialog.TAG);
            } else {
                Logger.d(TAG, "onChatMessageClick() size is 1 and start with MAIL_PREFIX");
                startUrlRelatedActivity(urlSingle, activity);
            }
        } else {
            ArrayAdapter<String> adapter =
                    new ArrayAdapter<String>(activity, android.R.layout.select_dialog_item, urls) {
                        @Override
                        public View getView(int position, View convertView, ViewGroup parent) {
                            View v = super.getView(position, convertView, parent);
                            String url = getItem(position).toString();
                            try {
                                TextView tv = (TextView) v;
                                Drawable drawable = null;
                                if (url.startsWith(RCSE_PREFIX)) {
                                    Logger.d(TAG, "onChatMessageClick() it is start with rcse");
                                    drawable =
                                            activity.getApplicationContext().getResources()
                                                    .getDrawable(R.drawable.rcs_icon);
                                } else {
                                    drawable =
                                            activity.getPackageManager().getActivityIcon(
                                                    new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
                                }
                                if (drawable != null) {
                                    drawable.setBounds(0, 0, drawable.getIntrinsicHeight(),
                                            drawable.getIntrinsicHeight());
                                    tv.setCompoundDrawablePadding(10);
                                    tv.setCompoundDrawables(drawable, null, null, null);
                                } else {
                                    Logger.e(TAG, "onChatMessageClick() d is null");
                                }
                                if (url.startsWith(TEL_PREFIX)) {
                                    Logger.d(TAG,
                                            "onChatMessageClick() it is start with TEL_PREFIX");
                                    url =
                                            getItem(position).toString().substring(
                                                    TEL_PREFIX.length());
                                } else if (url.startsWith(SMS_PREFIX)) {
                                    Logger.d(TAG,
                                            "onChatMessageClick() it is start with SMS_PREFIX");
                                    url =
                                            getItem(position).toString().substring(
                                                    SMS_PREFIX.length());
                                } else if (url.startsWith(RCSE_PREFIX)) {
                                    Logger.d(TAG,
                                            "onChatMessageClick() it is start with RCSE_PREFIX");
                                    url =
                                            getItem(position).toString().substring(
                                                    RCSE_PREFIX.length());
                                } else if (url.startsWith(MAIL_PREFIX)) {
                                    Logger.d(TAG,
                                            "onChatMessageClick() it is start with MAIL_PREFIX");
                                    MailTo mt = MailTo.parse(url);
                                    url = mt.getTo();
                                }
                                tv.setText(url);
                            } catch (android.content.pm.PackageManager.NameNotFoundException e) {
                                e.printStackTrace();
                            }
                            return v;
                        }
                    };
            MultiAlertDialog mutiAlertDialog = new MultiAlertDialog();
            mutiAlertDialog.setActivity(activity);
            mutiAlertDialog.setAdapter(adapter);
            mutiAlertDialog.setUrls(urls);
            mutiAlertDialog.show(activity.getFragmentManager(), MultiAlertDialog.TAG);
        }
    }

    private static ArrayList<String> extractUris(URLSpan[] spans) {
        Logger.d(TAG, "extractUris() entry ");
        int size = spans.length;
        Logger.d(TAG, "extractUris() the Span size is " + size);
        ArrayList<String> container = new ArrayList<String>();
        for (int i = 0; i < size; i++) {
            container.add(spans[i].getURL());
        }
        return container;
    }

    /**
     * The MultiAlertDialog is used for two case: 1,only browser link, alert
     * browse dialog 2,number or more than one link, alert multi selected
     * dialog.
     */
    public static class MultiAlertDialog extends DialogFragment implements
            DialogInterface.OnClickListener {
        private static final String TAG = "MultiAlertDialog";
        private String mUrl = null;
        private ArrayAdapter<String> mAdapter = null;
        private ArrayList<String> mUrls = null;
        private Activity mActivity = null;

        @Override
        public void onCancel(DialogInterface dialog) {
            dismissAllowingStateLoss();
        }

        public MultiAlertDialog() {

        }

        public void setActivity(Activity activity) {
            mActivity = activity;
        }

        public void setUrl(String url) {
            mUrl = url;
        }

        public void setAdapter(ArrayAdapter<String> adapter) {
            mAdapter = adapter;
        }

        public void setUrls(ArrayList<String> urls) {
            mUrls = urls;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(mActivity);
            AlertDialog alertDialog = null;
            if (mAdapter != null) {
                Logger.d(TAG, "onCreateDialog() mAdapter is not null");
                dialogBuilder.setTitle(R.string.link_pick);
                dialogBuilder.setAdapter(mAdapter, mMultiClick);
                alertDialog = dialogBuilder.create();
                alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                        getString(android.R.string.cancel), this);
            } else {
                Logger.d(TAG, "onCreateDialog() mAdapter is null");
                dialogBuilder.setTitle(R.string.message);
                dialogBuilder.setMessage(R.string.browser_alert);
                alertDialog = dialogBuilder.create();
                alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,
                        getString(R.string.rcs_dialog_positive_button), this);
                alertDialog.setButton(DialogInterface.BUTTON_NEGATIVE,
                        getString(android.R.string.cancel), this);
            }
            return alertDialog;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                startUrlRelatedActivity(mUrl, mActivity);
            } else {
                dismissAllowingStateLoss();
            }
        }

        private DialogInterface.OnClickListener mMultiClick = new DialogInterface.OnClickListener() {
            @Override
            public final void onClick(DialogInterface dialog, int which) {
                if (which >= 0) {
                    String url = mUrls.get(which);
                    Uri uri = Uri.parse(url);
                    Intent intent = new Intent();
                    if (url.startsWith(RCSE_PREFIX)) {
                        Logger.d(TAG, "onClick() url start with RCSE_PREFIX");
                        Participant participant =
                                new Participant(url.substring(RCSE_PREFIX.length()), "");
                        ArrayList<Participant> participants = new ArrayList<Participant>();
                        participants.add(participant);
                        intent.putParcelableArrayListExtra(Participant.KEY_PARTICIPANT_LIST,
                                participants);
                        intent.putExtra(ADD_CONTACT, ChatMainActivity.VALUE_ADD_CONTACTS);
                        intent.setClass(mActivity, ChatScreenActivity.class);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    } else {
                        Logger.d(TAG, "onClick() url not start with RCSE_PREFIX");
                        intent = new Intent(Intent.ACTION_VIEW, uri);
                        intent.putExtra(Browser.EXTRA_APPLICATION_ID, mActivity.getPackageName());
                    }
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                    mActivity.startActivity(intent);
                }
                dialog.dismiss();
            }
        };
    }

    private static void startUrlRelatedActivity(String url, Activity activity) {
        Logger.d(TAG, "startUrlRelatedActivity() entry the url is " + url);
        Uri uri = Uri.parse(url);
        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
        intent.putExtra(Browser.EXTRA_APPLICATION_ID, activity.getPackageName());
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
        activity.startActivity(intent);
    }
}
