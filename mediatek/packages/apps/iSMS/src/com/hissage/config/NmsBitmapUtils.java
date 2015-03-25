package com.hissage.config;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import android.graphics.BitmapFactory.Options;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.media.ExifInterface;
import android.text.TextUtils;

import com.hissage.util.log.NmsLog;

public class NmsBitmapUtils {

    public static final int UNCONSTRAINED = -1;
    private static final String TAG = "NmsBitmapUtils";

    public static Options getOptions(String path) {
        Options options = new Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeFile(path, options);
        return options;
    }

    public static Options getOptions(InputStream is, int width, int height) {
        if (is == null || width <= 0 || height <= 0) {
            NmsLog.warn(TAG, "getOptions parm is error..");
            return null;
        }

        Options options = new Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(is, null, options);
        Rect r = getScreenRegion(width, height);
        int w = r.width();
        int h = r.height();
        int maxSize = (w >= h ? w : h);
        int inSimpleSize = computeSampleSize(options, maxSize, w * h);
        options.inSampleSize = inSimpleSize;
        options.inJustDecodeBounds = false;

        try {
            is.close();
            is = null;
        } catch (IOException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        }

        return options;
    }

    public static Bitmap getBitmapByInputStream(InputStream is, Options option) {
        if (is == null || option == null) {
            NmsLog.warn(TAG, "parm is error..");
            return null;
        }

        Bitmap bm = null;
        try {
            bm = BitmapFactory.decodeStream(is, null, option);
        } catch (java.lang.OutOfMemoryError e) {
            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error.");
        }

        try {
            is.close();
            is = null;
        } catch (IOException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        }

        return bm;
    }

    /**
     * Get bitmap
     * 
     * @param path
     * @param options
     * @return
     */
    public static Bitmap getBitmapByPath(String path, Options options, int width, int height) {
        if (TextUtils.isEmpty(path) || width <= 0 || height <= 0) {
            NmsLog.warn(TAG, "parm is error.");
            return null;
        }

        File file = new File(path);
        if (!file.exists()) {
            NmsLog.warn(TAG, "file is not exist.");
            return null;
        }
        FileInputStream in = null;
        try {
            in = new FileInputStream(file);
        } catch (FileNotFoundException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        }
        if (options != null) {
            Rect r = getScreenRegion(width, height);
            int w = r.width();
            int h = r.height();
            int maxSize = w > h ? w : h;
            int inSimpleSize = computeSampleSize(options, maxSize, w * h);
            options.inSampleSize = inSimpleSize;
            options.inJustDecodeBounds = false;
        }
        Bitmap bm = null;
        try {
            bm = BitmapFactory.decodeStream(in, null, options);
        } catch (java.lang.OutOfMemoryError e) {
            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
        }
        try {
            in.close();
        } catch (IOException e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        }
        return bm;
    }

    public static Bitmap getBitmapByPath(String path, Options options, int size) {
        return getBitmapByPath(path, options, size, size);
    }

    public static Bitmap decodeSampledBitmapFromResource(Resources res, int resId, int width,
            int height) {
        if (res == null || resId <= 0 || width <= 0 || height <= 0) {
            NmsLog.error(TAG, "parm is error");
            return null;
        }

        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(res, resId, options);

        if (options != null) {
            Rect r = getScreenRegion(width, height);
            int w = r.width();
            int h = r.height();
            int maxSize = w > h ? w : h;
            int inSimpleSize = computeSampleSize(options, maxSize, w * h);
            options.inSampleSize = inSimpleSize;
            options.inJustDecodeBounds = false;
        }

        options.inJustDecodeBounds = false;
        Bitmap bm = null;
        try {
            bm = BitmapFactory.decodeResource(res, resId, options);
        } catch (java.lang.OutOfMemoryError e) {
            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
        }
        return bm;
    }

    public static Bitmap decodeSampledBitmapFromResource(Resources res, int resId, int size) {
        return decodeSampledBitmapFromResource(res, resId, size, size);
    }

    private static Rect getScreenRegion(int width, int height) {
        return new Rect(0, 0, width, height);
    }

    /**
     * get inSampleSize.
     * 
     * @param options
     * @param minSideLength
     * @param maxNumOfPixels
     * @return
     */
    public static int computeSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        int initialSize = computeInitialSampleSize(options, minSideLength, maxNumOfPixels);

        int roundedSize;
        if (initialSize <= 8) {
            roundedSize = 1;
            while (roundedSize < initialSize) {
                roundedSize <<= 1;
            }
        } else {
            roundedSize = (initialSize + 7) / 8 * 8;
        }

        return roundedSize;
    }

    private static int computeInitialSampleSize(BitmapFactory.Options options, int minSideLength,
            int maxNumOfPixels) {
        double w = options.outWidth;
        double h = options.outHeight;

        int lowerBound = (maxNumOfPixels == UNCONSTRAINED) ? 1 : (int) Math.ceil(Math.sqrt(w * h
                / maxNumOfPixels));
        int upperBound = (minSideLength == UNCONSTRAINED) ? 128 : (int) Math.min(
                Math.floor(w / minSideLength), Math.floor(h / minSideLength));

        if (upperBound < lowerBound) {
            return lowerBound;
        }

        if ((maxNumOfPixels == UNCONSTRAINED) && (minSideLength == UNCONSTRAINED)) {
            return 1;
        } else if (minSideLength == UNCONSTRAINED) {
            return lowerBound;
        } else {
            return upperBound;
        }
    }

    public static byte[] resizeImgByMaxLength(String path, float maxLength) {
        int d = getExifOrientation(path);
        BitmapFactory.Options options = getOptions(path);

        int l = Math.max(options.outHeight, options.outWidth);
        int be = (int) (l / maxLength);
        if (be <= 0)
            be = 1;
        options.inSampleSize = be;
        options.inJustDecodeBounds = false;

        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeFile(path, options);
        } catch (java.lang.OutOfMemoryError e) {
            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
        }
        if (null == bitmap) {
            return null;
        }
        if (d != 0)
            bitmap = rotate(bitmap, d);

        String[] tempStrArry = path.split("\\.");
        String filePostfix = tempStrArry[tempStrArry.length - 1];
        CompressFormat formatType = null;
        if (filePostfix.equalsIgnoreCase("PNG")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else if (filePostfix.equalsIgnoreCase("JPG") || filePostfix.equalsIgnoreCase("JPEG")) {
            formatType = Bitmap.CompressFormat.JPEG;
        } else if (filePostfix.equalsIgnoreCase("GIF")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else if (filePostfix.equalsIgnoreCase("BMP")) {
            formatType = Bitmap.CompressFormat.PNG;
        } else {
            NmsLog.error(TAG, "Can't compress the image,because can't support the format:"
                    + filePostfix);
            return null;
        }

        int quality = 100;
        if (be == 1) {
            if (NmsCommonUtils.getFileSize(path) > 50 * 1024) {
                quality = 30;
            }
        } else {
            quality = 30;
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        bitmap.compress(formatType, quality, baos);
        final byte[] tempArry = baos.toByteArray();
        if (baos != null) {
            try {
                baos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            baos = null;
        }

        return tempArry;
    }

    public static byte[] resizeImgBySize(String path, int resize, boolean shrinkable) {
        NmsLog.trace(TAG, "request to resize image,path:" + path + " , size:" + resize);

        if (resize <= 1024) {
            NmsLog.error(TAG, "cancel to compress image,the resize is too small");
            return null;
        }

        File tempFile = new File(path);
        if (!tempFile.exists()) {
            NmsLog.error(TAG, "failed to find image file by the path:" + path);
            return null;
        }

        int fileSize = (int) tempFile.length();
        NmsLog.trace(TAG, "successful to find image file,length:" + fileSize);

        byte[] targetBytes;
        InputStream inputStream = null;
        ByteArrayOutputStream baos = null;
        Bitmap photo = null;
        String filePostfix = null;
        CompressFormat formatType = null;
        try {
            if (resize >= fileSize) {
                inputStream = new FileInputStream(tempFile);
                targetBytes = new byte[fileSize];
                if (inputStream.read(targetBytes) <= -1) {
                    NmsLog.error(TAG, "can't read the file to byet[]");
                } else {
                    NmsLog.trace(TAG, "successful to resize the image,after resize length is:"
                            + targetBytes.length);
                    return targetBytes;
                }
            } else {
                if (shrinkable) {
                    int multiple = fileSize / resize;
                    if (fileSize % resize >= 0
                            && fileSize % resize >= ((fileSize / multiple + 1) / 2)) {
                        multiple++;
                    }

                    if (multiple > 3) {
                        if (resize == 200 * 1024) {
                            multiple = 3;
                        } else if (resize == 100 * 1024) {
                            multiple = 6;
                        } else if (resize <= 50 * 1024) {
                            multiple = 10;
                        }
                    }

                    NmsLog.trace(TAG, "prepare to press sacle:" + multiple);
                    Options options = new Options();
                    options.inScaled = true;
                    options.inSampleSize = multiple;

                    int compressCount = 1;
                    do {
                        try {
                            photo = BitmapFactory.decodeFile(path, options);
                        } catch (java.lang.OutOfMemoryError e) {
                            NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
                        }
                        options.inSampleSize = multiple + compressCount;
                        NmsLog.trace(TAG, "try to encondw image " + compressCount + " times");
                        compressCount++;
                    } while (photo == null && compressCount <= 5);
                } else {
                    try {
                        photo = BitmapFactory.decodeFile(path, null);
                    } catch (java.lang.OutOfMemoryError e) {
                        NmsLog.error(TAG, "bitmap decode failed, catch outmemery error");
                    }
                }
                String[] tempStrArry = path.split("\\.");
                filePostfix = tempStrArry[tempStrArry.length - 1];
                tempStrArry = null;

                NmsLog.trace(TAG, "filePostfix:" + filePostfix);
                if (filePostfix.equals("PNG") || filePostfix.equals("png")) {
                    formatType = Bitmap.CompressFormat.PNG;
                } else if (filePostfix.equals("JPG") || filePostfix.equals("jpg")
                        || filePostfix.equals("JPEG") || filePostfix.equals("jpeg")) {
                    formatType = Bitmap.CompressFormat.JPEG;
                } else if (filePostfix.equalsIgnoreCase("GIF")) {
                    formatType = Bitmap.CompressFormat.PNG;
                } else if (filePostfix.equalsIgnoreCase("BMP")) {
                    formatType = Bitmap.CompressFormat.PNG;
                } else {
                    NmsLog.error(TAG, "Can't compress the image,because can't support the format:"
                            + filePostfix);
                    return null;
                }

                int quality = 100;
                while (quality > 0) {
                    baos = new ByteArrayOutputStream();
                    photo.compress(formatType, quality, baos);
                    final byte[] tempArry = baos.toByteArray();
                    NmsLog.trace(TAG, "successful to resize the image,after resize length is:"
                            + tempArry.length + " ,quality:" + quality);

                    if (tempArry.length <= resize) {
                        targetBytes = tempArry;
                        NmsLog.trace(TAG, "successful to resize the image,after resize length is:"
                                + targetBytes.length);
                        return targetBytes;
                    }
                    if (tempArry.length >= 1000000) {
                        quality = quality - 10;
                    } else if (tempArry.length >= 260000) {
                        quality = quality - 5;
                    } else {
                        quality = quality - 1;
                    }

                    if (baos != null) {
                        baos.close();
                        baos = null;
                    }
                }
                NmsLog.error(TAG, "can't compress the photo with the quality :" + quality);
            }
        } catch (Exception e) {
            NmsLog.error(TAG, "Exception ,when reading file:" + NmsLog.nmsGetStactTrace(e));
        } finally {
            targetBytes = null;
            try {
                filePostfix = null;
                formatType = null;
                if (inputStream != null) {
                    inputStream.close();
                    inputStream = null;
                }
                if (baos != null) {
                    baos.close();
                    baos = null;
                }
                if (photo != null) {
                    if (!photo.isRecycled()) {
                        photo.recycle();
                    }
                    photo = null;
                }
            } catch (Exception e) {
                NmsLog.error(
                        TAG,
                        "Exception,when recycel resource after compressing."
                                + NmsLog.nmsGetStactTrace(e));
            }
        }

        return null;
    }

    public static int getExifOrientation(String filepath) {
        int degree = 0;
        ExifInterface exif = null;

        try {
            exif = new ExifInterface(filepath);
        } catch (IOException ex) {
            NmsLog.trace(TAG, NmsLog.nmsGetStactTrace(ex));
        }

        if (exif != null) {
            int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, -1);
            if (orientation != -1) {
                switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    degree = 90;
                    break;

                case ExifInterface.ORIENTATION_ROTATE_180:
                    degree = 180;
                    break;

                case ExifInterface.ORIENTATION_ROTATE_270:
                    degree = 270;
                    break;
                default:
                    break;
                }
            }
        }

        return degree;
    }

    public static Bitmap rotate(Bitmap b, int degrees) {
        if (degrees != 0 && b != null) {
            Matrix m = new Matrix();
            m.setRotate(degrees, (float) b.getWidth() / 2, (float) b.getHeight() / 2);
            try {
                Bitmap b2 = Bitmap.createBitmap(b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                if (b != b2) {
                    b.recycle();
                    b = b2;
                }
            } catch (OutOfMemoryError ex) {
                NmsLog.error(TAG, "We have no memory to rotate. Return the original bitmap.");
            }
        }

        return b;
    }
}
