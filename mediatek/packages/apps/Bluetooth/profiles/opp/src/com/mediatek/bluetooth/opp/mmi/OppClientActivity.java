/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.bluetooth.opp.mmi;

import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Toast;

import com.mediatek.bluetooth.BluetoothShareGatewayActivity;
import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.opp.adp.OppConstants;
import com.mediatek.bluetooth.opp.adp.OppManager;
import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.util.BtLog;
import com.mediatek.bluetooth.util.MimeUtils;

import java.util.ArrayList;

/**
 * @author mtk01635 Object Push: ACTION_SEND or ACTION_SEND_MULTIPLE
 */
public class OppClientActivity extends Activity {

    protected static final String LINK_SHARE_URI_SCHEME = "btlink";

    ArrayList<Uri> mUris;

    boolean mIsHandover;

    BluetoothDevice mRemoteDevice;

    Thread mOppcCacheTaskThread = null;

    String mIntentType;

    public OppManager oppManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        OppLog.d("OppClientActivity.onCreate()[+]");

        super.onCreate(savedInstanceState);
        this.setVisible(false);

        this.oppManager = OppManager.getInstance(this);

        // get intent and action
        Intent intent = this.getIntent();
        String action = intent.getAction();

        // Object Push Request ( ACTION_SEND or ACTION_SEND_MULTIPLE )

        // create and save oppc push task(s)
        if (Intent.ACTION_SEND.equals(action) || OppConstants.ACTION_HANDOVER_SEND.equals(action)) {

	        String type = null;
            Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            mIntentType = intent.getType();
            CharSequence text = intent.getCharSequenceExtra(Intent.EXTRA_TEXT);
            if (uri == null && text != null) {

                // encode share content into Uri
                CharSequence subject = intent.getCharSequenceExtra(Intent.EXTRA_SUBJECT);
                String fragment = (subject == null) ? null : subject.toString();
                uri = Uri.fromParts(LINK_SHARE_URI_SCHEME, text.toString(), fragment);
				type = "text/html";
				BtLog.i( "link share set null type: intent mime-type[" + intent.getType()+ "]" );
            } else {
				
				if (uri != null) {
					type = OppClientActivity.this.getContentResolver().getType(uri);
				}
				if (type == null && uri != null) {
					type = MimeUtils.getMimeType(uri.getLastPathSegment());
				}
				
				type = (type == null) ? mIntentType : type;
			}	
					
            BluetoothShareTask newTask = this.newOppcTask(BluetoothShareTask.TYPE_OPPC_PUSH, type,
                    uri, OppConstants.ACTION_HANDOVER_SEND.equals(action));
            if (newTask == null) {

                // skip invalid task
                this.finish();
                return;
            }
            oppManager.oppcCacheTask(newTask);
            int toastResId = R.string.bt_oppc_push_toast_sending_file;

            // get remote device and submit tasks
            BluetoothDevice remoteDevice = intent
                    .getParcelableExtra(BluetoothShareGatewayActivity.EXTRA_DEVICE_ADDRESS);
            if (remoteDevice == null) {

                // TODO add toast for this situation
                OppLog
                        .e("null remote-device in SEND intent => cann't send object via bluetooth(OPP)");
            } else {
                // submit cached tasks
                oppManager.oppcSubmitTask(remoteDevice);

                String strDeviceName = remoteDevice.getName();
                if (null != strDeviceName) {
                    Toast.makeText(this, this.getString(toastResId, strDeviceName),
                        Toast.LENGTH_SHORT).show();
                } else {
                    OppLog.i("OppClientActivity getDeviceName return null");
                    Toast.makeText(this, this.getString(toastResId, this.getString(R.string.bt_oppc_push_toast_unknown_device)),
                        Toast.LENGTH_SHORT).show();
                }
                // notify user
            }
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action)
                || OppConstants.ACTION_HANDOVER_SEND_MULTIPLE.equals(action)) {

            mIsHandover = OppConstants.ACTION_HANDOVER_SEND_MULTIPLE.equals(action);

            mUris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            // get remote device and submit tasks
            mRemoteDevice = intent
                    .getParcelableExtra(BluetoothShareGatewayActivity.EXTRA_DEVICE_ADDRESS);
            mIntentType = intent.getType();

            // for ALPS00238849 to prevert ANR while share amount objects
            // new thread to get each object minetype and cache task
            mOppcCacheTaskThread = new Thread() {

                boolean mHasAny = false;

                @Override
                public void run() {

                    if (mUris != null) {

                        BtLog.i("share multi-object: mime-type[" + mIntentType + "]");

                        for (Uri uri : mUris) {

                            // we should use file-extension to get mime-type
                            // for MULTI_SEND case
                            String type = OppClientActivity.this.getContentResolver().getType(uri);
                            if (type == null && uri != null) {
                                type = MimeUtils.getMimeType(uri.getLastPathSegment());
                            }
                            type = (type == null) ? mIntentType : type;
                            // BtLog.i( "share mime-type[" + intentType +
                            // "] => [" + type + "]" );

                            if (type == null || uri == null) {
                                final String uriStr = OppClientActivity.this.getString(
                                        R.string.bt_oppc_push_toast_unsupported_uri, uri);
                                // Toast should be show in UI thread
                                OppClientActivity.this.runOnUiThread(new Runnable() {

                                    public void run() {
                                        // request cannot be processed
                                        Toast.makeText(OppClientActivity.this, uriStr,
                                                Toast.LENGTH_LONG).show();
                                    }
                                });

                                return;
                            }

                            // create task object
                            BluetoothShareTask newTask = new BluetoothShareTask(
                                    BluetoothShareTask.TYPE_OPPC_PUSH);
                            newTask.setMimeType(type);
                            newTask.setObjectUri(uri.toString());
                            newTask.setHandover(mIsHandover);

                            if (newTask == null) {

                                // skip invalid task
                                continue;
                            } else {
                                oppManager.oppcCacheTask(newTask);
                                mHasAny = true;
                            }
                        }
                    }
                    if (!mHasAny) {

                        OppLog.i("no valid oppc task => finish activity");
                        // this.finish();
                        return;
                    }

                    if (mRemoteDevice == null) {

                        // TODO add toast for this situation
                        OppLog
                                .e("null remote-device in SEND intent => cann't send object via bluetooth(OPP)");
                    } else {
                        // submit cached tasks
                        oppManager.oppcSubmitTask(mRemoteDevice);

                        // notify user, Toast run in thread
                        OppClientActivity.this.runOnUiThread(new Runnable() {

                            int mToastResId = R.string.bt_oppc_push_toast_sending_files;

                            public void run() {
                                String strDeviceName = mRemoteDevice.getName();
                                if (null != strDeviceName) {
                                    Toast.makeText(
                                        OppClientActivity.this,
                                        OppClientActivity.this.getString(mToastResId, strDeviceName), Toast.LENGTH_SHORT).show();
                                } else {
                                    Toast.makeText(
                                        OppClientActivity.this,
                                        OppClientActivity.this.getString(mToastResId, OppClientActivity.this.getString(R.string.bt_oppc_push_toast_unknown_device)),
                                            Toast.LENGTH_SHORT).show();
                                }
                            }
                        });

                    }
                }
            };

            mOppcCacheTaskThread.start();
        } else {
            OppLog.w("oppc unsupport action: " + action);
            // for business card pull / business card exchanges
        }

        this.finish();
    }

    /**
     * create OPP task
     *
     * @param type
     * @param mimeType
     * @param uri
     * @param isHandover
     * @return
     */
    private BluetoothShareTask newOppcTask(int type, String mimeType, Uri uri, boolean isHandover) {

        // check parameter and uri support
        if (mimeType == null || uri == null) {

            OppLog.w("invalid parameter for newOppcTask : mimetype[" + mimeType + "], uri[" + uri
                    + "]");
            OppLog.w("current request intent: [" + this.getIntent().toUri(Intent.URI_INTENT_SCHEME)
                    + "]");

            // request cannot be processed
            Toast.makeText(this, this.getString(R.string.bt_oppc_push_toast_unsupported_uri, uri),
                    Toast.LENGTH_LONG).show();
            return null;
        }

        // create task object
        BluetoothShareTask task = new BluetoothShareTask(type);
        task.setHandover(isHandover);
        task.setMimeType(mimeType);
        task.setObjectUri(uri.toString());
        return task;
    }
}
