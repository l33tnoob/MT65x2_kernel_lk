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

package com.mediatek.bluetooth.bip;

import com.mediatek.bluetooth.R;

import java.util.ArrayList;
import java.io.File;

import android.os.Bundle;

import android.app.Activity;
import android.content.Intent;
import android.content.Context;

import android.content.ContentResolver;
import android.content.ContentValues;

import android.database.Cursor;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;

import android.net.Uri;

import android.bluetooth.BluetoothDevice;

import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;
import android.widget.Toast;

import com.mediatek.bluetooth.BluetoothShareGatewayActivity;

import com.mediatek.bluetooth.share.BluetoothShareTask;
import com.mediatek.bluetooth.share.BluetoothShareTask.BluetoothShareTaskMetaData;



public class BipInitEntry{
    private static final String TAG = "BipInitEntry";

    private ContentResolver mContentResolver;
    private Context mContext;
    private InsertTaskThread mInsertTask = null;

    private synchronized void bipiInsert( Uri uri, BluetoothDevice remoteDevice){

        Xlog.v(TAG, "bipiInsert");
 

        ContentResolver cr = mContext.getContentResolver();
        String[] proj={ MediaStore.Images.Media.DATA,
                        MediaStore.Images.Media.MIME_TYPE,
                        MediaStore.Images.Media.SIZE};

        Cursor cursor = cr.query( uri,
                                  proj, // Which columns to return
                                  null,       // WHERE clause; which rows to return (all rows)
                                  null,       // WHERE clause selection arguments (none)
                                  null); // Order-by clause (ascending by name)
        int pathColumnId = 0, mimeColumnId = 0, sizeColumnId = 0;
        String filePath = null, objectSize = null, objectMime = null;
        ContentValues values = null;

        if ( null == cursor ) {
            Xlog.e(TAG, "cursor is null");
        }
        else {
			try{
	            pathColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
	            mimeColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.MIME_TYPE);
	            sizeColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE);
	            cursor.moveToFirst();

	            filePath = cursor.getString(pathColumnId);
	            Xlog.v(TAG, "File Path: " + filePath);

	            //objectSize = cursor.getString(sizeColumnId);
	            objectMime = cursor.getString(mimeColumnId);
	            Xlog.v(TAG, "File Mime: " + objectMime);

	            cursor.close();
			}catch(android.database.CursorIndexOutOfBoundsException e){
				Xlog.w(TAG, "CursorIndexOutOfBoundsException catched");
				Toast.makeText(mContext, mContext.getString(R.string.bt_bip_toast_connect_fail), Toast.LENGTH_LONG).show();
				return;
			}
           
		   File f = new File(filePath);
		   objectSize = Long.toString(f.length());
		   Xlog.v(TAG, "File Size: " + objectSize);
		   
            values = new ContentValues();

            //name, mime, size are updated during creating BipImage
            values.put( BluetoothShareTaskMetaData.TASK_TYPE, BluetoothShareTask.TYPE_BIPI_PUSH );
            values.put( BluetoothShareTaskMetaData.TASK_STATE, BluetoothShareTask.STATE_PENDING );
            values.put( BluetoothShareTaskMetaData.TASK_RESULT, 0 );

            values.put( BluetoothShareTaskMetaData.TASK_OBJECT_NAME, filePath.substring(filePath.lastIndexOf('/')+1) );
            values.put( BluetoothShareTaskMetaData.TASK_OBJECT_URI, uri.toString() );
            values.put( BluetoothShareTaskMetaData.TASK_OBJECT_FILE, filePath );
            values.put( BluetoothShareTaskMetaData.TASK_MIMETYPE, objectMime );
            values.put( BluetoothShareTaskMetaData.TASK_PEER_NAME, remoteDevice.getName());
            values.put( BluetoothShareTaskMetaData.TASK_PEER_ADDR, remoteDevice.getAddress() );
            values.put( BluetoothShareTaskMetaData.TASK_TOTAL_BYTES, f.length());
            values.put( BluetoothShareTaskMetaData.TASK_DONE_BYTES, 0 );

            mContentResolver.insert( BluetoothShareTaskMetaData.CONTENT_URI, values );
        }

        return;
    }


    public BipInitEntry(Context context, Intent intent) {
        Xlog.v(TAG, "BipInitEntry......");

        mContext = context;
        mContentResolver = mContext.getContentResolver();


        //Intent intent = this.getIntent();
        String action = intent.getAction();
        BluetoothDevice remoteDevice = intent.getParcelableExtra(BluetoothShareGatewayActivity.EXTRA_DEVICE_ADDRESS);
        Xlog.v(TAG, "Received BT device selected intent, bt device: " + remoteDevice);


        if ( null == action || null == remoteDevice ) {
            Xlog.e(TAG, "action is null or remoteDevce is null");
        }
        else if ( Intent.ACTION_SEND.equals(action) ) {
            Uri uri = intent.getParcelableExtra( Intent.EXTRA_STREAM);
            if ( null == uri ) {
                Xlog.e(TAG, "uri is null");
            }
            else {
                bipiInsert( uri, remoteDevice );
            }
        }
        else if ( Intent.ACTION_SEND_MULTIPLE.equals(action) ) {
            ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            if ( null == uris ) {
                Xlog.e(TAG, "uris is null");
            }
            else {
                mInsertTask = new InsertTaskThread(uris, remoteDevice);
		   mInsertTask.start();
            } 
        }
    }

	
	private class InsertTaskThread extends Thread {
		public ArrayList<Uri> uris;
		public BluetoothDevice remoteDevice;

		public InsertTaskThread(ArrayList<Uri> tmpUris, BluetoothDevice device){
			uris = tmpUris;
			remoteDevice = device;
		}
		
		@Override
		public void run() {
			int num = 0;
			for (Uri uri : uris) {
               			 Xlog.i(TAG, "num = "+num);
				bipiInsert( uri, remoteDevice );
				if(num<10){
					num++;
				}else{
					try{
						Thread.sleep(1000);
						num = 0;
					}catch(java.lang.InterruptedException e){
						Xlog.e(TAG, "Sleep error");
					}
				}
			}
		}
	}
	
}

