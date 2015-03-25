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

package com.mediatek.bluetooth.share;

import android.content.Context;
import android.database.Cursor;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.text.format.Formatter;
import android.view.View;
import android.widget.ImageView;
import android.widget.ResourceCursorAdapter;
import android.widget.TextView;

import com.mediatek.bluetooth.R;
import com.mediatek.bluetooth.share.BluetoothShareTask.Direction;

import java.util.Date;

public class BluetoothShareTabAdapter extends ResourceCursorAdapter {

    public BluetoothShareTabAdapter(Context context, int layout, Cursor c) {

        super(context, layout, c);
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {

        // create task from cursor
        BluetoothShareTask task = new BluetoothShareTask(cursor);

        // icon
        ImageView icon = (ImageView) view.findViewById(R.id.transfer_icon);
        if (task.getDirection() == Direction.in) {

            icon
                    .setImageResource((task.getState() == BluetoothShareTask.STATE_SUCCESS) ? 
                              R.drawable.bt_share_mgmt_in_success
                            : R.drawable.bt_share_mgmt_failure);
        } else {
            icon
                    .setImageResource((task.getState() == BluetoothShareTask.STATE_SUCCESS) ? 
                              R.drawable.bt_share_mgmt_out_success
                            : R.drawable.bt_share_mgmt_failure);
        }

        TextView textView;

        // file
        textView = (TextView) view.findViewById(R.id.transfer_file);
        String filename = task.getData();
        filename = (filename == null) ? "" : filename;
        textView.setText(filename);

        // peer device: peer_device
        textView = (TextView) view.findViewById(R.id.peer_device);
        String deviceName = task.getPeerName();
        deviceName = (deviceName == null) ? "" : deviceName;
        textView.setText(deviceName);

        // modified_date
        textView = (TextView) view.findViewById(R.id.modified_date);
        Date d = new Date(task.getModifiedDate());
        CharSequence modifiedDate = DateUtils.isToday(task.getModifiedDate()) ? DateFormat.getTimeFormat(context).format(d)
                : DateFormat.getDateFormat(context).format(d);
        textView.setText(modifiedDate);

        // transfer_info
        textView = (TextView) view.findViewById(R.id.transfer_info);
        textView.setText(Formatter.formatFileSize(context, task.getTotalBytes()));
    }
}
