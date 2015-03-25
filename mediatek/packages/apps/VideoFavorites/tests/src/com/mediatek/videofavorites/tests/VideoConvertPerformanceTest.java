package com.mediatek.videofavorites.tests;

import com.mediatek.videofavorites.VideoFavoritesProvider;
import com.mediatek.videofavorites.VideoFavoritesProviderValues;
import com.mediatek.videofavorites.WidgetActionActivity;
import com.mediatek.videofavorites.WidgetAdapter;
import com.mediatek.xlog.Xlog;

import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.content.ContentValues;
import android.content.Context;
import android.content.IntentFilter;
import android.content.Intent;
import android.net.Uri;
import android.test.ActivityInstrumentationTestCase2;


import java.util.ArrayList;
import android.graphics.Rect;
import android.media.MediaMetadataRetriever;
import android.provider.MediaStore;
import android.provider.MediaStore.Video;
import android.os.SystemClock;
import java.io.File;
import java.lang.IllegalArgumentException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import com.mediatek.transcode.VideoTranscode;
import com.mediatek.videofavorites.R;
import com.mediatek.videofavorites.Storage;

public class VideoConvertPerformanceTest extends ActivityInstrumentationTestCase2<WidgetActionActivity> {
	
    private WidgetActionActivity mActivity;
    private static final String mInputPath = "/storage/sdcard0/[3D HD 1080p] SNSD- Run Devil Run MV[720x480].mp4";
    private static final String TAG = "PerformanceTest";
    private static final int ENCODE_WIDTH = 320;
    private static final int ENCODE_HEIGHT = 240;
    private static final String FILE_FORMAT = "_yyyyMMdd_HHmmss";   

    public VideoConvertPerformanceTest() {
        super(WidgetActionActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }
    
    public void test01VideoConvert(){
        
        String outputPath;
        Rect srcRect = new Rect();  
        boolean isValidVideo = getSourceVideoRect(mInputPath, srcRect); 
        long transcoderHandle = VideoTranscode.init();
        
        if (!isValidVideo) {
        	  Xlog.v(TAG, "input video file is valid");
            return;
        }
        
        long startTime = SystemClock.uptimeMillis();

        if (srcRect.width() <= ENCODE_WIDTH || srcRect.height() <= ENCODE_HEIGHT) {
            Xlog.v(TAG, "video is too small. no need to convert");	
            return;
        } else {
            outputPath = generateOutputPath(mInputPath);
            Xlog.v(TAG, "output file is: " + outputPath);
            Rect targetRect = getTargetRect(srcRect.width(), srcRect.height(), ENCODE_WIDTH,
                                            ENCODE_HEIGHT);
            Xlog.v(TAG, "srcRect: " + srcRect + " targetRect: " + targetRect);

            int result = VideoTranscode.transcode(transcoderHandle, mInputPath, outputPath,
                                                  (long) targetRect.width(), (long) targetRect.height(), (long)0,
                                                  (long)10000);

            Xlog.e(TAG, "transcode result: " + result);

            if (result != VideoTranscode.NO_ERROR) {
            }

        }
        long timecost = SystemClock.uptimeMillis() - startTime;

        Xlog.v(TAG, "transcode spend(ms):" + timecost); 
        
        long startTime2 = SystemClock.uptimeMillis();
        
        VideoTranscode.deinit(transcoderHandle); 
    }
    
        private boolean getSourceVideoRect(String filePath, Rect outRect) {

            String strWidth = null;
            String strHeight = null;
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            String hasVideo = null;

            File f = new File(filePath);
            if (f.length() == 0) {
                return false;
            }

            try {
                retriever.setDataSource(filePath);
                hasVideo = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_HAS_VIDEO);
                if (hasVideo == null) {
                    Xlog.e(TAG, "getSourceVideoRect, no videoTrack");
                    return false;
                }
                strWidth = retriever.extractMetadata(
                               MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH);
                strHeight = retriever.extractMetadata(
                                MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT);
            } catch (IllegalArgumentException ex) {
                // Assume this is a corrupt video file
                Xlog.e(TAG, "Exception:" + ex);
            } finally {
                retriever.release();
            }

            if (strWidth == null || strHeight == null) {
                Xlog.e(TAG, "invalid video width/height");
                return false;
            }

            int width = Integer.decode(strWidth).intValue();
            int height = Integer.decode(strHeight).intValue();
            if (width == 0 || height == 0) {
                Xlog.e(TAG, "video width/height is 0");
                return false;
            }
            outRect.set(0, 0, width, height);
            return true;
        }
        
        
        private String generateOutputPath(String inputName) {
            long dateTaken = System.currentTimeMillis();
            String postfix = createName(dateTaken);
            File inputFile = new File(inputName);

            prepareFolder(Storage.TRANSCODE_PATH);
            StringBuilder sb = new StringBuilder(Storage.TRANSCODE_PATH);
            sb.append(inputFile.getName());
            int i = sb.lastIndexOf(".");
            if (i == -1) {
                sb.append(postfix);
            } else {
                sb.insert(i, postfix);
            }
            return sb.toString();
        }
        
        private String createName(long dateTaken) {
            Date date = new Date(dateTaken);
            SimpleDateFormat dateFormat = new SimpleDateFormat(
                FILE_FORMAT, Locale.US);

            return dateFormat.format(date);
        }

        private void prepareFolder(String path) {
            File f = new File(path);
            if (f.exists()) {
                return;
            }

            if (!f.mkdirs()) {
                Xlog.e(TAG, "folder creation failed!");
            }
        }                

        private Rect getTargetRect(int srcWidth, int srcHeight, int maxWidth, int maxHeight) {
            float rSrc = (float) srcWidth / srcHeight;
            float rMax = (float) maxWidth / maxHeight;

            int targetWidth;
            int targetHeight;

            // crop and scale

            if (rSrc < rMax) {
                targetWidth = maxWidth;
                targetHeight = targetWidth * srcHeight / srcWidth;
            } else {
                targetHeight = maxHeight;
                targetWidth = targetHeight * srcWidth / srcHeight;
                // width must be the factor of 16, find closest but smallest factor
                // so hight won't larger than mHeight
                if (targetWidth % 16 != 0) {
                    targetWidth = (targetWidth - 15) >> 4 << 4;
                    targetHeight = targetWidth * srcHeight / srcWidth;
                }
            }

            return new Rect(0, 0, targetWidth, targetHeight);
        }    	
}