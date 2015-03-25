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


import android.os.ParcelFileDescriptor;

import android.content.Intent;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.res.Resources;

import android.database.Cursor;
import android.provider.MediaStore;
import android.provider.MediaStore.Images;
import android.media.MiniThumbFile;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

import android.net.Uri;
import com.mediatek.xlog.Xlog;
import com.mediatek.xlog.SXlog;

import java.io.*;

import java.lang.Runtime;
import java.lang.Process;
import java.io.IOException;

public class BipImage {

    private static final String TAG = "BipImage";

    public String DirName = null;           //char[256]
    public String FileName = null;          //char[256]
    public String ThumbnailFullPath = null; //char[256]
    public int ObjectSize = 0;
    public int AcceptableFileSize = 0;

    //public ImageDescriptor ImageDesc;
    public String Version = null;  // Image Descriptor version (exe) "1.0", etc)  char[10]
    public String Encoding = null;  // Image Encoding Type String (ex)"JPEG","GIF","BMP","WBMP","PNG", etc) char[30]
    public int Width = 0;
    public int Height = 0;
    public int Width2 = 0;
    public int Height2 = 0;
    public int Size = 0;
    public Transformation Transform;
    private static final String thumFilePath = "/data/@btmtk/profile/";
    private static final String thumFileName = "thumbnail.jpg";
    private CreateThumbnailThread mCreateThumbnailThread = null;


    private String getImageEncoding(String mimetype) {
        String encoding;

        if (mimetype.equals("image/jpeg")) {
            encoding = "JPEG";
        }
        else if (mimetype.equals("image/png")) {
            encoding = "PNG";
        }
        else if (mimetype.equals("image/gif")) {
            encoding = "GIF";
        }
        else if (mimetype.equals("image/bmp") || mimetype.equals("image/x-ms-bmp")) {
            encoding = "BMP";
        }
        else if (mimetype.equals("image/vnd.wap.wbmp")) {
            encoding = "WBMP";
        }
        else {
            Xlog.e(TAG, "Unsupport format");
            encoding = "unknow";
        }

        Xlog.v(TAG, "encoding: " + encoding);
        return encoding;
    }


    private int[] getImageDimension(Uri imageuri, ContentResolver cr) {

        ParcelFileDescriptor input = null;
        BitmapFactory.Options options = null;
        Bitmap b = null;
	int[] dimension = new int[2]; 
 
        try {
            input = cr.openFileDescriptor(imageuri, "r");
            options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
			if(input != null)
			{
				b = BitmapFactory.decodeFileDescriptor(input.getFileDescriptor(), null, options);
			}else{
				Xlog.e(TAG, "input != null");
				return dimension;
			}
            
            dimension[0] = options.outWidth; 
            dimension[1] = options.outHeight;
        } catch (FileNotFoundException ex) {
            dimension[0] = 0;
            dimension[1] = 0;
        } finally {
            try {
				if(input != null)
               		input.close();
            } catch (Throwable t) {
                // do nothing
            }
        }
        Xlog.v(TAG, "Dimension[0]: " + dimension[0]);
        Xlog.v(TAG, "Dimension[1]: " + dimension[1]);

        return dimension;
    } 


    private boolean createImageThumbnail(Uri imageuri, Context c, ContentResolver cr) {

        Xlog.v(TAG, "Thumbnail creating......");

	String source =  imageuri.getPathSegments().get(1);
	Long imageId = new Long(0);
	if(source.equals("images"))
        	imageId = new Long( imageuri.getPathSegments().get(3) );
	else if(source.equals("file"))
		imageId = new Long( imageuri.getPathSegments().get(2) );
	
        Xlog.v(TAG, "getSeg ID: " + imageId);  


        Bitmap bitmap = MediaStore.Images.Thumbnails.getThumbnail( cr,
                                                            imageId,
                                                            MediaStore.Images.Thumbnails.MICRO_KIND, //96*96
                                                            null);

        MiniThumbFile thumbFile = MiniThumbFile.instance( Uri.parse("content://media/external/images/thumbnails") );
        thumbFile.deactivate();



        if (null != bitmap) {
            Xlog.v(TAG, "getThumbnail success");
        } else {
            Xlog.e(TAG, "getThumbnail fail");
            //return false;
            Resources res = c.getResources();
            bitmap = BitmapFactory.decodeResource(res, R.drawable.default_thumbnail);    
        }

        //96*96 -> 160*120
        Xlog.v(TAG, "resize thumbnail......");
        final int width = bitmap.getWidth();
        final int height = bitmap.getHeight();
        int focusX = width / 2;
        int focusY = height / 2;

        int cropX;
        int cropY;
        int cropWidth;
        int cropHeight;
        float scaleFactor;

        //Horizontally constrained
        cropHeight = 120 * width / 160;
        cropY = Math.max(0, Math.min(focusY - cropHeight / 2, height - cropHeight));
        cropX = 0;
        cropWidth = width;
        scaleFactor = (float) 160 / width;

        final Bitmap finalBitmap = Bitmap.createBitmap(160, 120, Bitmap.Config.RGB_565);
        final Canvas canvas = new Canvas(finalBitmap);
        final Paint paint = new Paint();
        paint.setFilterBitmap(true);
        canvas.drawColor(0);
        canvas.drawBitmap(bitmap,
                          new Rect(cropX, cropY, cropX + cropWidth, cropY + cropHeight),
                          new Rect(0, 0, 160, 120),
                          paint);
        bitmap.recycle();

        //"/data/data/com.mediatek.bluetooth/files/thumbnail.jpg"
        Xlog.v(TAG, "compress and save thumbnail......");
        final ByteArrayOutputStream cacheOutput = new ByteArrayOutputStream(19200);

        try {
            finalBitmap.compress(Bitmap.CompressFormat.JPEG, 80, cacheOutput);

            //final FileOutputStream out = c.openFileOutput( "thumbnail.jpg", Context.MODE_PRIVATE );
            File thumbnailDir;
            File thumbnailFile;

            thumbnailDir = new File( thumFilePath );
            if( !thumbnailDir.exists() )
            {
                thumbnailDir.mkdirs();
            }

            thumbnailFile = new File(thumFilePath, thumFileName);
            if(thumbnailFile.exists())
            {
                thumbnailFile.delete();
            }
            thumbnailFile.createNewFile();
            
            FileOutputStream out = new FileOutputStream(thumbnailFile);
            cacheOutput.writeTo(out);
            cacheOutput.close();

            out.close();

            finalBitmap.recycle();
            
            try{
            	String cmd = "chmod 604 " + ThumbnailFullPath;	
              Xlog.i(TAG,"cmd="+cmd);
              Runtime rt = Runtime.getRuntime();
              Process proc = rt.exec(cmd);
              
              cmd = "chmod 777 " + thumFilePath;
              Xlog.i(TAG,"cmd="+cmd);
              rt.exec(cmd);
            }catch(IOException e){
              Xlog.e(TAG,"chmod fail!");
              e.printStackTrace();
              return false;
            }
        } catch (Exception e) {
            Xlog.e(TAG, "save thumbnail fail");
            return false;
        }

        return true;
    }

	private class CreateThumbnailThread extends Thread {
		public Uri imageuri;
		public Context c;
		public ContentResolver cr;

		public CreateThumbnailThread(Uri uri, Context context, ContentResolver contentResolver){
			imageuri = uri;
			c = context;
			cr = contentResolver;
		}
		
		@Override
		public void run() {
			if (createImageThumbnail(imageuri, c, cr)) {
				Xlog.v(TAG, "Create thumbnail success");
			}
			else {	
				Xlog.e(TAG, "Create thumbnail fail");
			}
		}
	}


    public BipImage(Uri imageuri, String filePath, String objectSize, String mime, Context c) {

        Version = "1.0";
        Xlog.v(TAG, "Image Descriptor Version: " + Version);

        String mimeType = null;
        int[] dimension = new int[2];

        ContentResolver cr = c.getContentResolver();

        Xlog.v(TAG, "File Path: " + filePath);
        FileName = filePath.substring(filePath.lastIndexOf('/')+1);
        Xlog.v(TAG, "File Name: " + FileName);
        DirName = filePath.substring(0, filePath.lastIndexOf('/'));
        Xlog.v(TAG, "Dir Name: " + DirName);

        ObjectSize = Integer.parseInt(objectSize);
        Size = ObjectSize;
        Xlog.v(TAG, "File Size: " + ObjectSize);

        Xlog.v(TAG, "File Mime: " + mime);
        Encoding = getImageEncoding(mime);
        Xlog.v(TAG, "Encoding: " + Encoding);


        dimension = getImageDimension(imageuri, cr);
        Width = dimension[0];
        Height = dimension[1];
        Xlog.v(TAG, "Width: " + Width);
        Xlog.v(TAG, "Height: " + Height);

        //ThumbnailFullPath = "/data/data/com.mediatek.bluetooth/files/thumbnail.jpg";
        ThumbnailFullPath = thumFilePath + thumFileName;
        Xlog.v(TAG, "ThumbnailFullPath: " + ThumbnailFullPath);
	/*
        if (createImageThumbnail(imageuri, c, cr)) {
            Xlog.v(TAG, "Create thumbnail success");
        }
        else {  
            Xlog.e(TAG, "Create thumbnail fail");
        }
        */
        mCreateThumbnailThread = new CreateThumbnailThread(imageuri, c, cr);
	mCreateThumbnailThread.start();
    }


    public BipImage(String dirname, String filename, String thumbnailfp, String objectsize,
                       String version, String encoding, int height, int width) {

        DirName = dirname;
        FileName = filename;
        ThumbnailFullPath = thumbnailfp;
        ObjectSize = Integer.parseInt(objectsize);

        Version = version;
        Encoding = encoding;
        Height = height;
        Width = width;
        Height2 = 0;
        Width2 = 0;
        Size = Integer.parseInt(objectsize);
    }
/*
    memcpy(BipObject.DirName, "/sdcard", 256);
    memcpy(BipObject.FileName, "adb.jpg", 256);

ng.parseLong(Height);

    BipObject.ObjectSize = 73220;

    memcpy(BipObject.ImageDesc.Version, "1.0", 10);
    memcpy(BipObject.ImageDesc.Encoding, "JPEG", 30);
    BipObject.ImageDesc.Height = 469;
    BipObject.ImageDesc.Width = 352;
    BipObject.ImageDesc.Height2 = 0;
    BipObject.ImageDesc.Width2 = 0;
    BipObject.ImageDesc.Size = 73220;
*/
}

class ImageDescriptor
{
    public String Version;   // Image Descriptor version (exe) "1.0", etc)  char[10]
    public String Encoding;  // Image Encoding Type String (ex)"JPEG","GIF","BMP","WBMP","PNG", etc) char[30]
    public int Width;
    public int Height;
    public int Width2;
    public int Height2;
    public int Size;
    public int Transform;
    //public Transformation Transform;
}

class ImageFormat
{
    public String Encoding; // Image Encoding Type String (ex)"JPEG","GIF","BMP","WBMP","PNG", etc)
    public int Width;
    public int Height;
    public int Width2;
    public int Height2;
    public int Size;
}

class Capability {
    public ImageDescriptor PreferFormat; // Image format prefer to receive
    public ImageFormat[] ImageFormats; // Image format can be retrieved by other devices
    public int NumImageFormats;

    public Capability(int numFormats){
        PreferFormat = new ImageDescriptor();
        
        NumImageFormats = numFormats;
        ImageFormats = new ImageFormat[numFormats];
        for(int i=0; i<NumImageFormats; i++)
            ImageFormats[i] = new ImageFormat();
    }

}

class AuthInfo {
    public boolean bAuth;
    public String UserId;
    public String Passwd;

    public AuthInfo(boolean auth, String uid, String pwd){
        bAuth = auth;
        UserId = uid;
        Passwd = pwd;
    }
}

//class AuthRes {
//}

enum Transformation
{
    BIP_TRANS_NONE,
    BIP_TRANS_STRETCH,
    BIP_TRANS_FILL,
    BIP_TRANS_CROP
}

