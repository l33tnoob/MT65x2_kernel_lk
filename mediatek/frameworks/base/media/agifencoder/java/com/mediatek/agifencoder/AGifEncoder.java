/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.agifencoder;

//import com.mediatek.common.mpodecoder.IgifEncoder;

 
//import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.BufferedOutputStream;
import android.util.Log;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory.Options;
import android.net.Uri;
import android.content.ContentResolver;


public class AGifEncoder {
    private static final String TAG = "JAGifEncoder";
   
    private final int mNativeAGifEncoder;

    private int[] encodeSource = null; 
    private int fWidth ;
    private int fHeight ;
    private int fEncodeCount ;
    //private byte[] bsBuf = null;
    private OutputStream fStream = null;
    
    static {   
        System.loadLibrary("gifEncoder_jni");
    }
    
    private AGifEncoder(int nativeAGifEncoder, int width, int height) {
        if (nativeAGifEncoder == 0) {
            throw new RuntimeException("native movie creation failed");
        }
        mNativeAGifEncoder = nativeAGifEncoder; 
        fWidth = width;
        fHeight = height;
        fEncodeCount = 0;
        Log.i(TAG, "AGifEncoder mNatvie " + mNativeAGifEncoder + " native " + nativeAGifEncoder);
        //encodeSource = new int [width * height];
    }
    
    //public native boolean setWidth(int width);
    //public native boolean setHeight(int height);
    //public native boolean setOpaque(boolean isOpaque);
    
    // @hide
    public boolean setDuration(int duration){      
      return nativeSetDuration(mNativeAGifEncoder,duration);            
    }
    // @hide
    public int width(){
       return nativeWidth(mNativeAGifEncoder);
    }
    // @hide
    public int height(){
       return nativeHeight(mNativeAGifEncoder);
    }
    // @hide
    public int duration(){
       return nativeDuration(mNativeAGifEncoder);
    }
    
    private native boolean nativeSetDuration(int mNativeAGifEncoder, int duration);
    private native int nativeWidth(int mNativeAGifEncoder);
    private native int nativeHeight(int mNativeAGifEncoder);
    private native int nativeDuration(int mNativeAGifEncoder);
    

    /**
     * Number of bytes of temp storage we use for communicating between the
     * native compressor and the java OutputStream.
     */
    private final static int WORKING_COMPRESS_STORAGE = 2048;//4096;

    //optional
    private boolean setOutputStream( OutputStream stream ){
        if (stream == null) {
            throw new NullPointerException();
        }
        fStream = stream ;
        //bsBuf = new byte[WORKING_COMPRESS_STORAGE] ;
        Log.i(TAG, "setOutputStream, mNatvie " + mNativeAGifEncoder );

       return nativeSetOutputStream (mNativeAGifEncoder, fStream, new byte[WORKING_COMPRESS_STORAGE]) ;
    }
    // @hide
    public static AGifEncoder createAGifEncoder( int width, int height ){
        if (width == 0 || height ==0) {
            throw new NullPointerException();
        }
        //fStream = stream ;

       return nativeCreateAGifEncoder (width, height) ;
    }
    
    
    private static native AGifEncoder nativeCreateAGifEncoder (int width, int height) ;
    

    private static AGifEncoder encodeAGifStream( int width, int height, OutputStream stream ){
        if (stream == null || width == 0 || height ==0) {
            throw new NullPointerException();
        }
        //fStream = stream ;

       return nativeCreateEncoder (width, height, stream, new byte[WORKING_COMPRESS_STORAGE]) ;
    }
    // @hide
    public boolean encodeBitmap(Bitmap bitmap, OutputStream stream ){
        //int pointer = 0;

        if (bitmap != null) {
            
//            if (!bitmap.isMutable()) {
//                throw new IllegalStateException();
//            }
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            boolean ret = false ;

            Log.i(TAG, "encodeBitmap, w:" + fWidth + " h:" + fHeight + "stream:"+ stream);
            if (stream == null  ) {
                throw new NullPointerException();
            }   
            if( width == 0 || height ==0 || width != fWidth || height != fHeight){
               Log.i(TAG, "encodeBitmap, creation w:" + fWidth + " h:" + fHeight);
               throw new IllegalArgumentException("encode bitmap dimention not match with creation!!");
            }
            
            if(fEncodeCount == 0){
               fStream = stream ;
            }else if(fStream != stream){
               throw new IllegalArgumentException("encode bitmap dimention not match with creation!!");
            }

            //if(stream == null || fWidth == 0 || fHeight == 0){
            //  fWidth = width ;
            //  fHeight = height ;
            //}
            //if(encodeSource == null){
            //  encodeSource = new int [width * height];
            //}
            int[] encodePixels = new int [width * height];
                  

         

            bitmap.getPixels(encodePixels, 0, width, 0, 0, width, height); 
            
            ret = nativeEncodeBitmap(mNativeAGifEncoder, encodePixels, stream, new byte[WORKING_COMPRESS_STORAGE]); 
            if(ret == true){
              fEncodeCount += 1;   
            }
            return ret ;
            //return nativeEncodeBitmap(encodeSource, stream, new byte[WORKING_COMPRESS_STORAGE]); 
            
        }
        return false ;
              
    }
    //private static native boolean nativeEncodeBitmap(int bitmap);
    //private static native boolean nativeEncodeBitmap(int mNativeAGifEncoder,int[] pixels);
    //private static native boolean nativeEncodeBitmap(int[] pixels, OutputStream stream, byte[] tempStorage);
    private static native boolean nativeEncodeBitmap(int mNativeAGifEncoder,int[] pixels, OutputStream stream, byte[] tempStorage);



    /**
     * This method get total encode frame count of animation GIF 
     * @hide
     */
     
    // @hide     
    public int encodeFrameCount(){
        return nativeEncodeFrameCount();
    }
    private native int nativeEncodeFrameCount();

    /**
     * This method release all the Info stored for GIF.
     * After this method is call, AGifEncoder Object should no longer be used.
     * eg. mAGifEncoder.closeGif();
     *     mAGifEncoder = null;
     * @hide
     */
    public boolean close(OutputStream stream ){
      if (fStream == null || fStream != stream) {
          throw new NullPointerException();
      }           
      //return closeGif(fStream, new byte[WORKING_COMPRESS_STORAGE]);
      return nativeCloseStream(mNativeAGifEncoder, fStream, new byte[WORKING_COMPRESS_STORAGE]);
    }
    private native boolean nativeCloseStream(int mNativeAGifEncoder, OutputStream stream, byte[] tempStorage);
    private native boolean nativeCloseGif(OutputStream stream, byte[] tempStorage);


    private static native void nativeDestructor(int nativeAGifEncoder);
    
    private static native boolean nativeSetOutputStream(int mNativeAGifEncoder, OutputStream stream, byte[] tempStorage );
    
    private static native AGifEncoder nativeCreateEncoder( int wdith, int height, OutputStream stream, byte[] tempStorage );

    @Override
    protected void finalize() throws Throwable {
        try {
            nativeDestructor(mNativeAGifEncoder);
        } finally {
            super.finalize();
        }
    }
/*
//    private static AGifEncoder decodeTempStream(InputStream is) {
//        AGifEncoder moov = null;
//        try {
//            moov = decodeStream(is);
//            is.close();
//        }
//        catch (java.io.IOException e) {
//            //  do nothing.
//            //  If the exception happened on open, moov will be null.
//            //  If it happened on close, moov is still valid.
//            //
//        }
//        return moov;
//    }
*/    
}
