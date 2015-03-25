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


package com.mediatek.bluetooth.bpp;

import com.mediatek.bluetooth.R;


import android.os.Bundle;

import android.app.Activity;
import android.app.AlertDialog;
//import android.app.ActivityManager;

import android.content.Context;
import android.content.Intent;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.ComponentName;


import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDevicePicker;
import com.mediatek.bluetooth.BluetoothDevicePickerEx;

import android.widget.Toast;

import android.provider.MediaStore;
import android.database.Cursor;
import android.net.Uri;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.List;
import java.lang.Runtime;
import java.lang.Process;




public class BluetoothBppEntryActivity extends Activity {
    private static final String TAG = "BluetoothBppEntryActivity";

    private static final int FILTER_TYPE_PRINTER = 3;
    private static final String ACTION_PRINT = "mediatek.intent.action.PRINT";

    private static final int BLUETOOTH_DEVICE_REQUEST = 1;


    private static String[] mEntryError;

    private static Context mContext;
    private static BluetoothAdapter mAdapter;
    private static Uri mFileUri;
    private static String mFilePath;
    private static String mFileSize;
    private static String mMimeType;
    
    //private static ActivityManager mAm;
    private static boolean mReentry = false;
    private boolean mBack = false;
    private static int result = -1;
    private FileOperThread mFileOperThread = null;

    private class FileOperThread extends Thread {
	public FileOperThread(){
		}
		
	@Override
	public void run() {
		result = getFileInfo();
	}
    }

    private void getFileInfoMain(){
        if (mFileUri != null && mMimeType != null) {
            Xlog.v(TAG, "Get ACTION_SEND intent: Uri = " + mFileUri + "; mimetype = " + mMimeType);

            //int result = getFileInfo();
            
	    mFileOperThread = new FileOperThread();
	    mFileOperThread.start();
		
	    try {
	    	mFileOperThread.join();
		mFileOperThread = null;
	    } catch (InterruptedException e) {
		Xlog.e(TAG, "mFileOperThread close error.");
	    }
	    Xlog.v(TAG, "result = "+result);
            if (result != 0) {
                Toast.makeText(this, mEntryError[result], Toast.LENGTH_LONG).show();
                finish();
                return;
            }
        }
    }


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Xlog.v(TAG, "onCreate......");

        mReentry = false;


        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType(); // mime type can be also gotten from uri, i don't pass it to bpp manager

        // This will be around as long as this process is
        mContext = this.getApplicationContext();
        mMimeType = type;
        mFileUri = (Uri)intent.getParcelableExtra(Intent.EXTRA_STREAM);

        mEntryError = mContext.getResources().getStringArray(R.array.bt_bpp_entry_error);

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mAdapter == null) {
            Xlog.v(TAG, "bluetooth service is not started! ");
            finish();
            return;
        }


        if (mAdapter.isEnabled()){
            Xlog.v(TAG, "bluetooth service is available");
            getFileInfoMain();
            //if (startBppManager()) { 
            if (sendFileInfo() && result == 0) { 
                startDevicePicker();
            }

            finish();

        } else {
            Xlog.v(TAG, "bluetooth service is not available! ");
            Xlog.v(TAG, "turning on bluetooth......");

            Intent in = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(in, BLUETOOTH_DEVICE_REQUEST);
           
        }
		


        return;
    }



    @Override
    public void onStart() {
        Xlog.v(TAG, "onStart......");
        super.onStart();
    }
	
	@Override
    public void onResume() {
        Xlog.v(TAG, "onResume......");
        super.onResume();

        if(mReentry){
            finish();
        }
    }

	@Override
    public void onPause() {
        Xlog.v(TAG, "onPause......");
		mBack = true;
        mReentry = true;
        super.onPause();
    }

    @Override
    public void onStop() {
        Xlog.v(TAG, "onStop......");
        super.onStop();
    }


    @Override
    public void onDestroy() {
        Xlog.v(TAG, "onDestroy......");
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        Xlog.v(TAG, "onBackPressed......");
		if(!mBack){
			Xlog.v(TAG, "mBack is false");
        	super.onBackPressed();
		}
		else{
			Xlog.v(TAG, "mBack is true");
		}
    }



    @Override
    protected void onActivityResult( int requestCode, int resultCode, Intent data ){
        super.onActivityResult( requestCode, resultCode, data );

        if( requestCode == BLUETOOTH_DEVICE_REQUEST ){
            mReentry = false;


            if( Activity.RESULT_OK == resultCode ){
                // Bluetooth device is ready
		getFileInfoMain();
                
                if (sendFileInfo()&& result == 0) { 
		                startDevicePicker();
		            }
                //finish();
            }
            else {
                String message = "";
                if( data != null )
                {
                    //message = data.getStringExtra( BLUETOOTH_DEVICE_RESULT );
                }

                new AlertDialog.Builder(this)
                    .setTitle(R.string.app_name)
                    .setMessage(R.string.dialog_title_bpp_not_ready)
                    .setOnCancelListener(
                        new OnCancelListener() {
                            public void onCancel( DialogInterface dialog ){
                                BluetoothBppEntryActivity.this.finish();
                            }
                        }
                    )
                    .setNeutralButton(
                        R.string.dialog_bt_not_ready_confirm,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int which) {
                                BluetoothBppEntryActivity.this.finish();
                            }
                        }
                    ).show();
            }
        }
    }




    private int getFileInfo () {

        //URI can be also use to recognize the type 
        if ( mMimeType.equals("text/x-vcard")) {
            Xlog.v(TAG, "v-card");

            try {
                InputStream in =  mContext.getContentResolver().openInputStream( mFileUri );
                if ( null == in ) {
   	            Xlog.e(TAG, "input stream is null");
                    //return 3;
                    return 1;
                }
                //String filePath = "/data/@btmtk/profile/";
                //String fileName = "Contact.vcf";
                
                //File contactDir;
                //File contactFile;

                //contactDir = new File( filePath );
                //if( !contactDir.exists() )
                //{
                //    contactDir.mkdirs();
                //}

                //contactFile = new File(filePath, fileName);
                //if(contactFile.exists())
                //{
                //    contactFile.delete();
                //}
                //contactFile.createNewFile();

                FileOutputStream out = mContext.openFileOutput( "Contact.vcf", Context.MODE_WORLD_READABLE );
                //FileOutputStream out = new FileOutputStream(contactFile);

                int total = 0, read;
                byte[] buf = new byte[1024];

                while( ( read = in.read(buf) ) != -1 ){

                        out.write(buf, 0, read);
                        total += read;
                }
                in.close();
                out.close();
                
                //mFilePath = "/data/data/com.mediatek.bluetooth/files/Contact.vcf";
                //mFilePath = filePath + fileName;

                File path = null;
                path = mContext.getFileStreamPath("Contact.vcf");
                if (path != null) {
                    mFilePath = path.getAbsolutePath();
                }
   

                mMimeType = "text/x-vcard:3.0"; 
                mFileSize = Integer.toString(total);
                
                //try{
                //	String cmd = "chmod 604 " + mFilePath;	
                //	Xlog.i(TAG,"cmd="+cmd);
                //	Runtime rt = Runtime.getRuntime();
                //	Process proc = rt.exec(cmd);
                	
                //	cmd = "chmod 777 " + filePath;
              	//	Xlog.i(TAG,"cmd="+cmd);
              	//	rt.exec(cmd);
                //}catch(IOException e){
                //	Xlog.e(TAG,"chmod fail!");
                //	e.printStackTrace();
                //	return 1;
                //}

                Xlog.v(TAG, "File Path: " + mFilePath);
                Xlog.v(TAG, "File Size: " + mFileSize);
                Xlog.v(TAG, "File Mime: " + mMimeType);
                return 0;
            }
            catch( Exception ex ){
                Xlog.e( TAG, "create v-card file fail: " + ex.getMessage() );
                return 1;
            }

        }
        else {
            Xlog.v(TAG, "image");

            if( "content".equals(mFileUri.getScheme()) ) {
                if( MediaStore.AUTHORITY.equals(mFileUri.getAuthority()) ) {

                   String[] proj={
                      MediaStore.Images.Media.DATA,
                      MediaStore.Images.Media.MIME_TYPE,
                      MediaStore.Images.Media.SIZE};

                   Cursor cursor = managedQuery( mFileUri,   
                       proj, // Which columns to return   
                       null,       // WHERE clause; which rows to return (all rows)   
                       null,       // WHERE clause selection arguments (none)   
                       null); // Order-by clause (ascending by name)   

                    if ( null == cursor ) {
                       Xlog.e(TAG, "cursor is null");
                       //return 4;
                       return 2;
                    }  

                    int pathColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);   
                    int mimeColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.MIME_TYPE);   
                    int sizeColumnId = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE);   
                    //cursor.moveToFirst();
		    if(!cursor.moveToFirst()){						
				Xlog.e(TAG, "cursor.moveToFirst() is null");
				return 2;
		        }

                    mFilePath = cursor.getString(pathColumnId);
                    mMimeType = cursor.getString(mimeColumnId);
                    //mFileSize = cursor.getString(sizeColumnId); 
                }
            }
            else if( "file".equals(mFileUri.getScheme()) ) {

                mFilePath = mFileUri.getSchemeSpecificPart();
                mFilePath = mFilePath.substring(mFilePath.indexOf('/',2));

            }
            else
            {
                Xlog.e(TAG, "unhandled URI");
            }

            File f = new File(mFilePath);
            mFileSize = Long.toString(f.length());


            Xlog.v(TAG, "File Path: " + mFilePath);
            Xlog.v(TAG, "File Size: " + mFileSize);
            Xlog.v(TAG, "File Mime: " + mMimeType);

            if ( !mMimeType.equals("image/jpeg") &&
                 !mMimeType.equals("image/gif") &&
                 !mMimeType.equals("image/png") ) {
                return 2;
            } 
            else {
                return 0;
           } 
        }
    }

    private boolean sendFileInfo() {

        Intent intent = new Intent(BluetoothBppReceiver.ACTION_PASS_OBJECT);

        intent.putExtra(BluetoothBppManager.EXTRA_FILE_PATH, mFilePath);
        intent.putExtra(BluetoothBppManager.EXTRA_MIME_TYPE, mMimeType);
        intent.putExtra(BluetoothBppManager.EXTRA_FILE_SIZE, mFileSize);

        this.sendBroadcast(intent);
        return true;
    }

/*
    private boolean startBppManager() {
        Xlog.v(TAG, "Start Bpp Manager!");

        mAm = (ActivityManager)getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningServiceInfo> services = mAm.getRunningServices(100);
        final int NS = services != null ? services.size() : 0;

        for (int i=0; i<NS; i++) {
            ActivityManager.RunningServiceInfo si = services.get(i);
            if ( si.service.getClassName().equals("com.mediatek.bluetooth.bpp.BluetoothBppManager") ) {
                Toast.makeText(this, R.string.bt_bpp_reentry_error, Toast.LENGTH_LONG).show();
                return false;
            } 
        }

        Intent intent = new Intent(this, BluetoothBppManager.class);
        intent.putExtra("action", BluetoothBppManager.ACTION_PASS_OBJECT);

        intent.putExtra(BluetoothBppManager.EXTRA_FILE_PATH, mFilePath);
        intent.putExtra(BluetoothBppManager.EXTRA_MIME_TYPE, mMimeType);
        intent.putExtra(BluetoothBppManager.EXTRA_FILE_SIZE, mFileSize);

        this.startService(intent);
        return true;
    }
*/
    
    private void startDevicePicker(){
        Xlog.v(TAG, "Start Device Picker!");

        Intent in_toBDP = new Intent(BluetoothDevicePicker.ACTION_LAUNCH);
        in_toBDP.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        in_toBDP.putExtra(BluetoothDevicePicker.EXTRA_NEED_AUTH, false);
        in_toBDP.putExtra(BluetoothDevicePicker.EXTRA_FILTER_TYPE,BluetoothDevicePickerEx.FILTER_TYPE_PRINTER);
        in_toBDP.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_PACKAGE, "com.mediatek.bluetooth");
        in_toBDP.putExtra(BluetoothDevicePicker.EXTRA_LAUNCH_CLASS,BluetoothBppReceiver.class.getName());
        startActivity(in_toBDP);
    } 

}

