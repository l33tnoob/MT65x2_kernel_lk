/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.camera;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.FeatureHelpPage;
import com.mediatek.engineermode.R;
import com.mediatek.xlog.Xlog;

import java.util.Arrays;

/** Entry activity of EM camera. */
public class Camera extends Activity implements OnItemClickListener {

    private static final int DIALOG_AF = 1;
    // private static final int DIALOG_RAW_CAPTURE = 2;
    private static final int DIALOG_RAW_CAPTURE_MODE = 2;
    private static final int DIALOG_RAW_CAPTURE_TYPE = 3;
    private static final int DIALOG_ANTI_FLICKER = 4;
    private static final int DIALOG_SET_STEP = 5;
    private static final int DIALOG_ISO = 6;
    // private static final int ITEM_RAW_TYPE =2;
    private static final int ITEM_FLICKER = 3;
    private static final int ITEM_ISO = 4;
    private static final int ITEM_START_PREVIEW = 5;
    private static final int ITEM_HELP = 6;
    private static final String AUTO_STR = "0";
    private static final String CAMERA_PERFER_KEY = "camera_settings";
    private static final String AF_MODE = "AFMode";
    private static final String AF_STEP = "AFStep";
    private static final String RAW_CAPTURE_MODE = "RawCaptureMode";
    private static final String RAW_TYPE = "RawType";
    private static final String ANTI_FLICKER = "AntiFlicker";
    private static final String ISO_STR = "ISO";
    private int mMode = 0;
    private int mRawCaptureMode = 1;
    private int mRawCaptureType = 0;
    private String mAntiFlicker = "50";
    private String mISO = AUTO_STR;
    private int mStep = 1;
    private static final String TAG = "EM/Camera";

    private Intent mIntent = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Xlog.i(TAG, "Camera->onCreate()");
        setContentView(R.layout.camera);

        final ListView camereListView =
            (ListView) findViewById(R.id.ListView_Camera);
        final ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1,
                Arrays.asList(getResources().getStringArray(
                    R.array.camera_items)));
        camereListView.setAdapter(adapter);
        camereListView.setOnItemClickListener(this);
        setPreferencesTodefault();
    }

    /**
     * Click each camera setting item
     * 
     * @param arg0
     *            : selected item's adapter view
     * @param arg1
     *            : selected view
     * @param arg2
     *            : position of selected view
     * @param arg3
     *            : id of selected view
     */
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        switch (arg2) {
        case 0:
            showDialog(DIALOG_AF);
            break;
        case 1:
            showDialog(DIALOG_RAW_CAPTURE_MODE);
            break;
        case 2:
            showDialog(DIALOG_RAW_CAPTURE_TYPE);
            break;
        case ITEM_FLICKER:
            showDialog(DIALOG_ANTI_FLICKER);
            break;
        case ITEM_ISO:
            showDialog(DIALOG_ISO);
            break;
        case ITEM_START_PREVIEW:
            Xlog.v(TAG, "before start CameraPreview");
            if (null == mIntent) { // not select mode yet
                mIntent = new Intent();
            }
            mIntent.setClass(this, CameraPreview.class);
            final SharedPreferences preferences =
                this.getSharedPreferences(CAMERA_PERFER_KEY,
                    android.content.Context.MODE_PRIVATE);

            mIntent.putExtra(AF_MODE, preferences.getInt(AF_MODE, 0));
            Xlog.v(TAG, "AFMode has been set to "
                + mIntent.getIntExtra(AF_MODE, 0));
            mIntent.putExtra(AF_STEP, preferences.getInt(AF_STEP, 0));
            Xlog.v(TAG, "AFStep has been set to "
                + mIntent.getIntExtra(AF_STEP, 0));
            mIntent.putExtra(RAW_CAPTURE_MODE, preferences.getInt(
                RAW_CAPTURE_MODE, 1));
            Xlog.v(TAG, "RawCaptureMode has been set to "
                + mIntent.getIntExtra(RAW_CAPTURE_MODE, 1));
            mIntent.putExtra(RAW_TYPE, preferences.getInt(RAW_TYPE, 0));
            Xlog.v(TAG, "RawType has been set to "
                + mIntent.getIntExtra(RAW_TYPE, 0));
            mIntent.putExtra(ANTI_FLICKER, preferences.getString(ANTI_FLICKER,
                "50"));
            Xlog.v(TAG, "AntiFlicker has been set to "
                + mIntent.getStringExtra(ANTI_FLICKER));
            mIntent.putExtra(ISO_STR, preferences.getString(ISO_STR, AUTO_STR));
            Xlog.v(TAG, "ISO has been set to "
                + mIntent.getStringExtra(ISO_STR));

            this.startActivity(mIntent);
            Xlog.v(TAG, "after start CameraPreview ");
            mIntent = null; // in order to let user must select one mode before
            break;
        case ITEM_HELP:
            if (null == mIntent) { // not select mode yet
                mIntent = new Intent();
            }
            mIntent.setClass(this, FeatureHelpPage.class);
            mIntent.putExtra(FeatureHelpPage.HELP_TITLE_KEY, R.string.help);
            mIntent.putExtra(FeatureHelpPage.HELP_MESSAGE_KEY,
                R.string.camera_help_msg);
            try {
                startActivity(mIntent);
            } catch (ActivityNotFoundException e) {
                Xlog.e(TAG, "Start activity FeatureHelpPage error");
            }
            break;
        default:
            break;
        }
    }

    private void setPreferencesTodefault() {
        Xlog.v(TAG, "Camera->setPreferencesTodefault()");
        final SharedPreferences preferences =
            getSharedPreferences(CAMERA_PERFER_KEY,
                android.content.Context.MODE_PRIVATE);
        final SharedPreferences.Editor editor = preferences.edit();
        editor.putInt(AF_STEP, 0);
        editor.putInt(AF_MODE, 0);
        editor.putInt(RAW_CAPTURE_MODE, 1);
        editor.putInt(RAW_TYPE, 0);
        editor.putString(ANTI_FLICKER, "50");
        editor.putString(ISO_STR, AUTO_STR);
        editor.commit();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Builder builder;
        AlertDialog alertDlg;
        final SharedPreferences preferences =
            getSharedPreferences(CAMERA_PERFER_KEY,
                android.content.Context.MODE_PRIVATE);
        final SharedPreferences.Editor editor = preferences.edit();
        switch (id) {
        case DIALOG_SET_STEP:
            final String setStepItems[] = { "1", "2", "4" };
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.step_dailog_title);
            builder.setSingleChoiceItems(setStepItems, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mStep = 1 << whichButton;
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        editor.putInt(AF_STEP, mStep);
                        editor.commit();
                        Xlog.i(TAG, "AFStep : " + mStep);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;
        case DIALOG_AF:
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.af_mode_dailog_title);
            builder.setSingleChoiceItems(R.array.af_mode_dialog, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mMode = whichButton;
                        Xlog.i(TAG, "Set AF Route has choice " + mMode);
                        if (mMode == 1) {
                            showDialog(DIALOG_SET_STEP);
                        }
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        if (0 == mMode) { // Normal AF
                            mStep = 0;
                            editor.putInt(AF_STEP, mStep);
                        } else if (1 == mMode) {
                            Xlog.i(TAG, "1 == mMode");
                        } else {
                            mStep = 1;
                            editor.putInt(AF_STEP, mStep);
                        }
                        editor.putInt(AF_MODE, mMode);
                        editor.commit();
                        Xlog.i(TAG, "AF mode :" + mMode);
                        Xlog.i(TAG, "AF step :" + mStep);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;

        case DIALOG_RAW_CAPTURE_MODE:
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.capture_mode_dailog_title);
            builder.setSingleChoiceItems(R.array.capture_mode_dialog, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {

                        mRawCaptureMode = whichButton + 1;
                        Xlog.i(TAG, "Set Raw Capture Mode has choice "
                            + mRawCaptureMode);
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        editor.putInt(RAW_CAPTURE_MODE, mRawCaptureMode);
                        editor.commit();
                        Xlog.i(TAG, "Raw Capture mode :" + mRawCaptureMode);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;
        case DIALOG_RAW_CAPTURE_TYPE:
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.raw_type_dailog_title);
            builder.setSingleChoiceItems(R.array.raw_type_dialog, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {

                        mRawCaptureType = whichButton;
                        Xlog.i(TAG, "Set Raw Type has choice "
                            + mRawCaptureType);
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        editor.putInt(RAW_TYPE, mRawCaptureType);
                        editor.commit();
                        Xlog.i(TAG, "Raw Type :" + mRawCaptureType);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;
        case DIALOG_ANTI_FLICKER:
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.flicker_dailog_title);
            builder.setSingleChoiceItems(R.array.anti_flicker_dialog, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        if (whichButton == 0) {
                            mAntiFlicker = "50";
                        } else if (whichButton == 1) {
                            mAntiFlicker = "60";
                        }
                        Xlog.i(TAG, "Set Flicker has choice " + mAntiFlicker);
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {

                        editor.putString(ANTI_FLICKER, mAntiFlicker);
                        editor.commit();
                        Xlog.i(TAG, "intent's mAntiFlicker = " + mAntiFlicker);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;
        case DIALOG_ISO:
            final String isoItemsValue[] =
                { "0", "100", "150", "200", "300", "400", "600", "800",
                    "1600" };
            builder = new AlertDialog.Builder(Camera.this);
            builder.setTitle(R.string.iso_dailog_title);
            builder.setSingleChoiceItems(R.array.iso_dialog, 0,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {

                        if (whichButton >= 0
                            && whichButton <= isoItemsValue.length - 1) {
                            mISO = isoItemsValue[whichButton];
                        } else {
                            mISO = isoItemsValue[0];
                            Xlog.i(TAG,
                                "Out of Array length. Set mISO whichButton = "
                                    + whichButton);
                        }

                        Xlog.i(TAG, "Set mISO has choice " + mISO);
                    }
                });
            builder.setPositiveButton(android.R.string.ok,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        editor.putString(ISO_STR, mISO);
                        editor.commit();
                        Xlog.i(TAG, "intent's mISO = " + mISO);
                    }
                });
            alertDlg = builder.create();
            return alertDlg;

        default:
            return null;
        }
    }
}
