package com.hissage.config;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.URL;
import java.net.UnknownHostException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Enumeration;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.ProgressDialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.text.TextUtils;

import com.hissage.api.NmsIpMessageApiNative;
import com.hissage.contact.NmsContact;
import com.hissage.contact.NmsContact.NmsContactType;
import com.hissage.contact.NmsGroupChatContact;
import com.hissage.message.ip.NmsIpLocationMessage;
import com.hissage.message.ip.NmsIpMessage;
import com.hissage.message.ip.NmsIpMessageConsts;
import com.hissage.message.ip.NmsIpTextMessage;
import com.hissage.message.ip.NmsIpVCardMessage;
import com.hissage.message.smsmms.NmsSMSMMSManager;
import com.hissage.platfrom.NmsPlatformAdapter;
import com.hissage.struct.SNmsSimInfo;
import com.hissage.struct.SNmsSimInfo.NmsSimActivateStatus;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.data.NmsConverter;
import com.hissage.util.log.NmsLog;

public class NmsCommonUtils {

    private static final String TAG = "NmsCommonUtils";
    private static final long AVALIABLE_SPACE = 5 * 1024 * 1024 ;

    public static String getCachePath(Context c) {
        String path = null;
        String sdCardPath = getSDCardPath(c);
        if (!TextUtils.isEmpty(sdCardPath)) {
            path = sdCardPath + NmsLog.cachepath;
        }
        createNewDir(path);
        return path;
    }

    public static void createNewDir(String dir) {
        if (!getSDCardStatus()) {
            return;
        }
        if (null == dir) {
            return;
        }
        File f = new File(dir);
        if (!f.exists()) {
            String[] pathSeg = dir.split(File.separator);
            String path = "";
            for (String temp : pathSeg) {
                if (TextUtils.isEmpty(temp)) {
                    path += File.separator;
                    continue;
                } else {
                    path += temp + File.separator;
                }
                File tempPath = new File(path);
                if (tempPath.exists() && !tempPath.isDirectory()) {
                    tempPath.delete();
                }
                tempPath.mkdirs();
            }
        } else {
            if (!f.isDirectory()) {
                f.delete();
                f.mkdirs();
            }
        }
    }

    public static String getAudioCachePath(Context context) {
        String path = getSDCardPath(context) + File.separator + NmsCustomUIConfig.ROOTDIRECTORY
                + File.separator + "audio";
        createNewDir(path);
        return path;
    }

    public static String getPicCachePath(Context context) {
        String path = getSDCardPath(context) + File.separator + NmsCustomUIConfig.ROOTDIRECTORY
                + File.separator + "picture";
        createNewDir(path);
        return path;
    }

    public static String getVideoCachePath(Context context) {
        String path = getSDCardPath(context) + File.separator + NmsCustomUIConfig.ROOTDIRECTORY
                + File.separator + "video";
        createNewDir(path);
        return path;
    }

    public static String getVcardCachePath(Context context) {
        String path = getSDCardPath(context) + File.separator + NmsCustomUIConfig.ROOTDIRECTORY
                + File.separator + "vcard";
        createNewDir(path);
        return path;
    }

    public static String getVcalendarCachePath(Context context) {
        String path = getSDCardPath(context) + File.separator + NmsCustomUIConfig.ROOTDIRECTORY
                + File.separator + "calendar";
        createNewDir(path);
        return path;
    }

    public static String getMemPath(Context c) {
        return c.getFilesDir().getAbsolutePath();
    }

    public static String getSDCardPath(Context c) {
        File sdDir = null;
        String sdStatus = Environment.getExternalStorageState();

        if (TextUtils.isEmpty(sdStatus)) {
            return c.getFilesDir().getAbsolutePath();
        }

        boolean sdCardExist = sdStatus.equals(android.os.Environment.MEDIA_MOUNTED);

        if (sdCardExist) {
            sdDir = Environment.getExternalStorageDirectory();
            return sdDir.toString();
        }

        return c.getFilesDir().getAbsolutePath();
    }
    
    public static boolean getSDCardFullStatus() {
        long sizeSD = getSDcardAvailableSpace();
        long sizeData = getDataStorageAvailableSpace();

        NmsLog.trace(TAG, "getSDCardStatus(): sizeSD = " + sizeSD + " ; sizeData = " + sizeData);

        if (sizeSD <= AVALIABLE_SPACE && sizeData <= AVALIABLE_SPACE) {
            return true;
        }

        return false;
    }

    public static boolean getSDCardFullStatusEx() {
        long sizeSD = getSDcardAvailableSpace();
        NmsLog.trace(TAG, "getSDCardStatus(): sizeSD = " + sizeSD);
        if (sizeSD <= AVALIABLE_SPACE) {
            return true;
        }

        return false;
    }
    
    public static boolean getSysStoreFullstatus() {
        long sizeSys = getSysStorageAvailableSpace();
        NmsLog.trace(TAG, "getSysStoreFullstatus(): sizeSys = " + sizeSys);

        if (sizeSys < AVALIABLE_SPACE) {
            return true;
        }

        return false;

    }

    public static boolean getSDCardStatus() {
        boolean ret = false;
        String sdStatus = Environment.getExternalStorageState();
        NmsLog.trace(TAG, "=" + sdStatus + "=");
        if (sdStatus.equals(Environment.MEDIA_MOUNTED))
            ret = true;
        return ret;
    }

    // For default SDCard
    public static long getSDcardAvailableSpace() {
    	String state = Environment.getExternalStorageState();
    	if (Environment.MEDIA_MOUNTED.equals(state)) {
	        File path = Environment.getExternalStorageDirectory();
	        if(path == null) {
	            return 0;
	        }
	        StatFs stat = new StatFs(path.getPath());
	        long blockSize = stat.getBlockSize();
	        long availableBlocks = stat.getAvailableBlocks();
	
	        return availableBlocks * blockSize; //"Byte" 
    	}else{
    		return 0;
    	}
    }
    
    // For External SDcard
    public static long getExternalSdCardAvailableSpace() {
    	
    	Map<String, File> externalLocations = NmsExternalStorage.getAllStorageLocations();
        //File sdCard = externalLocations.get(NmsExternalStorage.SD_CARD);
        //File externalSdCard = externalLocations.get(NmsExternalStorage.EXTERNAL_SD_CARD);
    	String state = Environment.getExternalStorageState();
    	if (Environment.MEDIA_MOUNTED.equals(state)) {
	        //File path = Environment.getExternalStorageDirectory();
    		File externalSdCard = externalLocations.get(NmsExternalStorage.EXTERNAL_SD_CARD);
    		if(externalSdCard == null) {
    		    return 0;
    		}
	        StatFs stat = new StatFs(externalSdCard.getPath());
	        long blockSize = stat.getBlockSize();
	        long availableBlocks = stat.getAvailableBlocks();
	
	        return availableBlocks * blockSize; //"Byte" 
    	}else{
    		return 0;
    	}
    }
    
    public static String getExternalSdCardPath() {
        Map<String, File> externalLocations = NmsExternalStorage.getAllStorageLocations();
        //File sdCard = externalLocations.get(NmsExternalStorage.SD_CARD);
        //File externalSdCard = externalLocations.get(NmsExternalStorage.EXTERNAL_SD_CARD);
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            //File path = Environment.getExternalStorageDirectory();
            File externalSdCard = externalLocations.get(NmsExternalStorage.EXTERNAL_SD_CARD);
            return externalSdCard.getAbsolutePath() ;
        }
        
        return "";
    }

   
    public static long getSysStorageAvailableSpace() {
    	File path = Environment.getRootDirectory();
        StatFs stat = new StatFs(path.getPath());
        long blockSize = stat.getBlockSize();
        long availableBlocks = stat.getAvailableBlocks();

        return availableBlocks * blockSize; // "Byte"
    }
    
    public static long getDataStorageAvailableSpace() {
        File path = Environment.getDataDirectory();
        StatFs stat = new StatFs(path.getPath());
        long blockSize = stat.getBlockSize();
        long availableBlocks = stat.getAvailableBlocks();

        return availableBlocks * blockSize;
    }
    
    public static boolean isNumeric1(String str) {
        Pattern pattern = Pattern.compile("[0-9]*");
        return pattern.matcher(str).matches();
    }

    public static void delFolder(String folderPath) {
        try {
            delAllFile(folderPath);
            String filePath = folderPath;
            filePath = filePath.toString();
            java.io.File myFilePath = new java.io.File(filePath);
            myFilePath.delete();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean delAllFile(String path) {
        if (TextUtils.isEmpty(path))
            return false;
        boolean flag = false;
        File file = new File(path);
        if (!file.exists()) {
            return flag;
        }
        if (!file.isDirectory()) {
            return flag;
        }
        String[] tempList = file.list();
        File temp = null;
        for (int i = 0; i < tempList.length; i++) {
            if (path.endsWith(File.separator)) {
                temp = new File(path + tempList[i]);
            } else {
                temp = new File(path + File.separator + tempList[i]);
            }
            if (temp.isFile()) {
                temp.delete();
            }
            if (temp.isDirectory()) {
                delAllFile(path + "/" + tempList[i]);
                delFolder(path + "/" + tempList[i]);
                flag = true;
            }
        }
        return flag;
    }

    public static String setHesineMail(String number) {
        StringBuffer sb = new StringBuffer();
        if (!TextUtils.isEmpty(number)) {
            sb.append(number.startsWith("+") ? number.substring(1, number.length()) : number);
            sb.append("@hissage.com");
        }
        return sb.toString();
    }

    public static boolean isPic(String picPath) {
        if (TextUtils.isEmpty(picPath))
            return false;
        String ext = getFileExt(picPath).toLowerCase();
        return ext.equals(".jpg") || ext.equals(".jpeg") || ext.equals(".png");
    }

    public static byte[] readZeroTerminateBytes(DataInputStream dis) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte b = dis.readByte();
        DataOutputStream dos = new DataOutputStream(baos);
        while (b != 0) {
            dos.writeByte(b);
            b = dis.readByte();
        }
        return baos.toByteArray();
    }

    public static String readZeroTerminateBytesStr(DataInputStream dis) throws IOException {
        return NmsConverter.utf8Bytes2String(readZeroTerminateBytes(dis));
    }

    public static byte[] InputStream2ByteArray(InputStream in) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        int ch = -1;

        while ((ch = in.read()) != -1) {
            baos.write(ch);
        }

        byte[] data = baos.toByteArray();
        baos.close();

        return data;
    }

    public static byte[] bitmap2ByteArray(Bitmap bm) throws IOException {
        if (bm == null) {
            return null;
        }

        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        bm.compress(Bitmap.CompressFormat.PNG, 100, baos);

        byte[] data = baos.toByteArray();
        baos.close();

        return data;
    }

    public static Bitmap byteArray2Bitmap(byte[] data) {
        if (data == null) {
            return null;
        }

        if (data.length <= 0) {
            return null;
        }

        return BitmapFactory.decodeByteArray(data, 0, data.length);
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
                // We only recognize a subset of orientation tag values.
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
                // We have no memory to rotate. Return the original bitmap.
            }
        }

        return b;
    }

    public static Bitmap resizeImage(Bitmap bitmap, int w, int h, boolean needRecycle) {
        if (null == bitmap) {
            return null;
        }

        Bitmap BitmapOrg = bitmap;
        int width = BitmapOrg.getWidth();
        int height = BitmapOrg.getHeight();
        int newWidth = w;
        int newHeight = h;

        float scaleWidth = ((float) newWidth) / width;
        float scaleHeight = ((float) newHeight) / height;
        Matrix matrix = new Matrix();
        matrix.postScale(scaleWidth, scaleHeight);

        Bitmap resizedBitmap = Bitmap.createBitmap(BitmapOrg, 0, 0, width, height, matrix, true);
        if (needRecycle && !BitmapOrg.isRecycled() && (resizedBitmap != BitmapOrg)) {
            BitmapOrg.recycle();
        }
        return resizedBitmap;
    }

    public static Bitmap stitchBitmap(Bitmap one, Bitmap two, Bitmap three) {
        if (one == null || two == null || three == null) {
            NmsLog.error("stitchBitmap", "one/two/three is/are invalid!");
            return null;
        }

        if (one.getWidth() < 96 || one.getHeight() < 96) {
            one = resizeImage(one, 96, 96, false);
        }

        int block = one.getWidth() / 16;

        one = Bitmap.createBitmap(one, block * 4, 0, block * 9, one.getHeight());
        two = resizeImage(two, block * 7, one.getHeight() / 2, false);
        three = resizeImage(three, block * 7, one.getHeight() / 2, false);

        Bitmap newbmp = Bitmap.createBitmap(one.getWidth() + two.getWidth(), one.getHeight(),
                Config.ARGB_8888);
        Canvas canvas = new Canvas(newbmp);
        // canvas.drawColor(Color.WHITE);
        canvas.drawBitmap(one, 0, 0, null);
        canvas.drawBitmap(two, one.getWidth(), 0, null);
        canvas.drawBitmap(three, one.getWidth(), two.getHeight(), null);
        canvas.save(Canvas.ALL_SAVE_FLAG);
        canvas.restore();

        if (one != null)
            one.recycle();
        if (two != null)
            two.recycle();
        if (three != null)
            three.recycle();

        return newbmp;
    }

    public static boolean saveBitmap2file(Bitmap bmp, String filePath) throws FileNotFoundException {
        CompressFormat format = Bitmap.CompressFormat.PNG;
        int quality = 100;
        OutputStream stream = new FileOutputStream(filePath);

        return bmp.compress(format, quality, stream);
    }

    public static int getBitmapSize(Bitmap bitmap) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
            return bitmap.getByteCount();
        }
        // Pre HC-MR1
        return bitmap.getRowBytes() * bitmap.getHeight();
    }

    public static boolean validateEmail(String email) {
        boolean tag = true;
        final String pattern1 = "^([a-z0-9A-Z]+[-|\\.]?)+[a-z0-9A-Z]@([a-z0-9A-Z]+(-[a-z0-9A-Z]+)?\\.)+[a-zA-Z]{2,}$";
        final Pattern pattern = Pattern.compile(pattern1);
        final Matcher mat = pattern.matcher(email);
        if (!mat.find()) {
            tag = false;
        }
        return tag;
    }

    public static String getServerIp() throws Exception {
        InetAddress myServer = InetAddress.getByName("www.niu-xin.com");
        return myServer.getHostAddress();
    }

    public static String getLocalIp() {
        try {
            InetAddress localMachine = InetAddress.getLocalHost();
            return localMachine.getHostAddress();
        } catch (UnknownHostException e) {
            NmsLog.nmsPrintStackTrace(e);
            return null;
        }
    }

    public static String getPreLevelPath(String path) {
        if (path.endsWith("/")) {
            path = path.substring(0, path.length() - 1);
        }
        int start = path.lastIndexOf("/");
        if (start != -1) {
            return path.substring(0, start);
        }
        return path;
    }

    public static String getFileName(String pathandname, boolean isExt) {
        int start = pathandname.lastIndexOf("/");
        int end = pathandname.lastIndexOf(".");
        if (start != -1 && end != -1) {
            if (isExt) {
                String[] pars = pathandname.split("\\.");
                return pathandname.substring(start + 1, end) + "." + pars[pars.length - 1];
            } else {
                return pathandname.substring(start + 1, end);
            }
        } else {
            return null;
        }
    }

    public static String parseDate(String str) {
        try {
            Date date = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").parse(str);
            GregorianCalendar gc = (GregorianCalendar) Calendar.getInstance();
            gc.setTime(new Date());
            gc.set(Calendar.DAY_OF_MONTH, 1);
            if (date.getTime() - gc.getTime().getTime() > 0) {
                return new SimpleDateFormat("HH:mm").format(date);
            } else {
                return new SimpleDateFormat("MM��dd��").format(date);
            }
        } catch (ParseException e) {
            NmsLog.nmsPrintStackTrace(e);
        }
        return str;
    }

    public static boolean isHesineEmail(String email) {
        return (!TextUtils.isEmpty(email) && (email.contains("@hesine.com") || email
                .contains("@hissage.com")));
    }

    public static boolean validateBlog(String blog) {
        if (blog != null && blog.length() > 0) {
            if (blog != null && blog.length() > 0) {
                String[] pars = blog.split("@");
                if (pars.length > 1) {
                    String[] ps = pars[1].split("\\.");
                    if (ps.length == 1) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public static String getNumber(String linkman) {
        if (linkman != null && linkman.length() > 0) {
            if (linkman.split("<").length > 1) {
                String str = linkman.split("<")[1];
                if (str != null && str.length() > 0) {
                    return str.substring(0, str.length() - 1);
                }
            }
        }
        return linkman;
    }

    public static String getUserName(String user) {
        if (user != null && user.length() > 0) {
            if (user.split("<").length > 1) {
                String str = user.split("<")[0];
                if (str != null && str.length() > 0) {
                    if ((str.startsWith("\"") || str.startsWith("\'"))
                            && (str.endsWith("\"") || str.endsWith("\'")))
                        return str.substring(1, str.length() - 1);
                    else
                        return str;
                } else {
                    return user.split("<")[1];
                }
            }
        }
        return user;
    }

    public static String getUserAddr(String addr) {
        if (addr != null && addr.length() > 0) {
            if (addr.contains("<") && addr.contains(">")) {
                String str = addr.split("<")[1];
                if (str != null && str.length() > 0) {
                    return str.substring(0, str.length() - 1);
                }
            } else {
                return addr;
            }
        }
        return addr;
    }

    public static boolean isEquals(byte[] a, byte[] b) {
        if (a == null || b == null || a.length != b.length) {
            return false;
        }
        for (int i = 0; i < a.length; i++) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }

    public static boolean isExistsFile(String filepath) {
        try {
            if (TextUtils.isEmpty(filepath)) {
                return false;
            }
            File file = new File(filepath);
            return file.exists();
        } catch (Exception e) {
            // e.printStackTrace();
            NmsLog.error(TAG,
                    "the file is not exists file path is: " + filepath + NmsLog.nmsGetStactTrace(e));
            return false;
        }
    }

    public static int getFileSize(String filepath) {
        try {
            if (TextUtils.isEmpty(filepath)) {
                return -1;
            }
            File file = new File(filepath);
            return (int) file.length();
        } catch (Exception e) {
            NmsLog.nmsPrintStackTrace(e);
            return -1;
        }
    }

    public static String getLocalIpAddress() {
        try {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en
                    .hasMoreElements();) {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr
                        .hasMoreElements();) {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress()) {
                        return inetAddress.getHostAddress().toString();
                    }
                }
            }
        } catch (SocketException ex) {
            NmsLog.nmsPrintStackTrace(ex);
            return null;
        }
        return null;
    }

    public static String getFileExt(String filepath) {
        String ext = "";
        String[] tmp = filepath.split("\\.");
        if (tmp.length > 0) {
            ext = "." + tmp[tmp.length - 1];
        }
        return ext;
    }

    public static void copy(String src, String dest) {
        if (TextUtils.isEmpty(src) || TextUtils.isEmpty(dest)) {
            NmsLog.error(TAG, "copy failed. param is error.");
            return;
        }

        InputStream is = null;
        OutputStream os = null;

        File out = new File(dest);
        if (!out.getParentFile().exists()) {
            out.getParentFile().mkdirs();
        }

        try {
            is = new BufferedInputStream(new FileInputStream(src));
            os = new BufferedOutputStream(new FileOutputStream(dest));

            byte[] b = new byte[256];
            int len = 0;
            try {
                while ((len = is.read(b)) != -1) {
                    os.write(b, 0, len);

                }
                os.flush();
            } catch (IOException e) {

                NmsLog.nmsPrintStackTrace(e);
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {

                        NmsLog.nmsPrintStackTrace(e);
                    }
                }
            }
        } catch (FileNotFoundException e) {

            NmsLog.nmsPrintStackTrace(e);
        } finally {
            if (os != null) {
                try {
                    os.close();
                } catch (IOException e) {

                    NmsLog.nmsPrintStackTrace(e);
                }
            }
        }
    }

    public static void nmsStream2File(byte[] stream, String filepath) throws Exception {
        FileOutputStream outStream = null;
        try {
            File f = new File(filepath);
            if (!f.getParentFile().exists()) {
                f.getParentFile().mkdirs();
            }
            if (f.exists()) {
                f.delete();
            }
            f.createNewFile();
            outStream = new FileOutputStream(f);
            outStream.write(stream);
            outStream.flush();
        } catch (IOException e) {
            NmsLog.nmsPrintStackTrace(e);
            throw new RuntimeException(e.getMessage());
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                    outStream = null;
                } catch (IOException e) {
                    NmsLog.nmsPrintStackTrace(e);
                    throw new RuntimeException(e.getMessage());
                }
            }
        }
    }

    /*
     * public static boolean isPhoneNumberValid(String number) { boolean isValid
     * = false; if (number == null || number.length() <= 0) {//
     * NmsUtils.trace(HissageTag.stm, "isPhoneNumberValid, number is null");
     * return false; } number = number.replace("-", ""); number =
     * number.replace(" ", ""); number = number.replace("(", ""); number =
     * number.replace(")", ""); number = number.replace(".", ""); Pattern
     * pattern = Pattern.compile("^[+][0-9]{6,}|[0-9]{5,}$"); Matcher matcher =
     * pattern.matcher(number); isValid = matcher.matches(); return isValid;
     * 
     * }
     */
    public static boolean isPhoneNumberValid(String number) {
        boolean isValid = false;
        if (number == null || number.length() <= 0) {
            NmsLog.trace(TAG, "isPhoneNumberValid, number is null");
            return false;
        }
        Pattern PHONE = Pattern.compile( // sdd = space, dot, or dash
                "(\\+[0-9]+[\\- \\.]*)?" // +<digits><sdd>*
                        + "(\\([0-9]+\\)[\\- \\.]*)?" // (<digits>)<sdd>*
                        + "([0-9][0-9\\-  0-9\\+ \\.][0-9\\- 0-9\\+ \\.]+[0-9])");
        Matcher matcher = PHONE.matcher(number);
        isValid = matcher.matches();
        return isValid;
    }

    public static String formatGroupMembers(List<String> members) {
        if (members == null || members.isEmpty()) {
            return null;
        }

        String memberStr = "";
        boolean isValid = true;

        for (int i = 0; i < members.size(); ++i) {
            String member = members.get(i);
            if (member != null && member.length() <= NmsCustomUIConfig.PHONENUM_MAX_LENGTH
                    && isPhoneNumberValid(member)) {
                memberStr += NmsCommonUtils.nmsGetStandardPhoneNum(member) + ",";
            } else {
                NmsLog.error("formatGroupMembers", member + " is invalid!");
                isValid = false;
                memberStr = "";
                break;
            }
        }

        if (isValid) {
            memberStr = memberStr.substring(0, memberStr.lastIndexOf(","));
        }

        return memberStr;
    }

    public static int NmsGetSystemTime() {
        long millSeconds = System.currentTimeMillis();
        return (int) (millSeconds / 1000);
    }

    public static String getDate(long date) // added by luozheng for date
                                            // format;
    {
        Date dateData = new Date(date);
        SimpleDateFormat formater = new SimpleDateFormat("yyyy-MM-dd");
        return formater.format(dateData);
    }

    public static File getFileFromServer(String path, ProgressDialog pd) throws Exception {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            NmsLog.trace("SettingActivity", "The update url:" + path);
            URL url = new URL(path);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setReadTimeout(5000);
            conn.setConnectTimeout(5000);
            int max = conn.getContentLength();
            pd.setMax(max);
            InputStream is = conn.getInputStream();
            File file = new File(Environment.getExternalStorageDirectory(), "updata.apk");

            FileOutputStream fos = new FileOutputStream(file);
            BufferedInputStream bis = new BufferedInputStream(is);
            byte[] buffer = new byte[1024];
            int len;
            int total = 0;
            while ((len = bis.read(buffer)) != -1) {
                fos.write(buffer, 0, len);
                total += len;
                pd.setProgressNumberFormat(total + "B/" + max + "B");
                pd.setProgress(total);
            }
            fos.close();
            bis.close();
            is.close();
            conn.disconnect();
            return file;
        } else {
            return null;
        }
    }

    public static boolean deletefile(String filename) {
        if (!TextUtils.isEmpty(filename)) {
            File file = new File(filename);
            if (file.isDirectory()) {
                File[] subFiles = file.listFiles();
                if (subFiles != null && subFiles.length > 0) {
                    for (File f : subFiles) {
                        f.delete();
                    }
                }
                file.delete();
            } else {
                return file.delete();
            }
        }
        return false;
    }

    public static boolean isInvalidNum(String num) {
        if (TextUtils.isEmpty(num)) {
            return false;
        }

        if (num.length() == NmsGroupChatContact.NMS_CLIENT_GUID_LEN
                && num.indexOf(NmsGroupChatContact.NMS_GUID_PREFIX) == 0)
            return true;

        if (num.contains(",")) {
            String[] numList = num.split(",");
            for (String number : numList) {
                if (!NmsCommonUtils.isPhoneNumberValid(number)) {
                    return false;
                }
            }
        } else {
            if (!NmsCommonUtils.isPhoneNumberValid(num)) {
                return false;
            }
        }
        return true;
    }

    public static String getValidNum(String num) {
        if (TextUtils.isEmpty(num)) {
            return "";
        }

        if (num.length() == NmsGroupChatContact.NMS_CLIENT_GUID_LEN
                && num.indexOf(NmsGroupChatContact.NMS_GUID_PREFIX) == 0)
            return num;

        String tmp = "";
        if (num.contains(",")) {
            String[] numList = num.split(",");
            for (String number : numList) {
                if (TextUtils.isEmpty(tmp)) {
                    tmp = NmsCommonUtils.nmsGetStandardPhoneNum(number);
                } else {
                    tmp += "," + NmsCommonUtils.nmsGetStandardPhoneNum(number);
                }
            }
        } else {
            tmp = NmsCommonUtils.nmsGetStandardPhoneNum(num);
        }
        return tmp;
    }

    public static NmsIpMessage VCardToTextMsg(NmsIpVCardMessage msg) {
        NmsIpTextMessage tmp = new NmsIpTextMessage();
        tmp.from = "";
        tmp.to = msg.to;
        tmp.protocol = msg.protocol;
        tmp.status = msg.status;
        tmp.type = NmsIpMessageConsts.NmsIpMessageType.TEXT;
        tmp.body = ""; // vcardMsg2Str(msg);
        return tmp;
    }

    public static NmsIpTextMessage LocationToTextMsg(NmsIpLocationMessage msg) {
        NmsIpTextMessage tmp = new NmsIpTextMessage();
        tmp.from = "";
        tmp.to = msg.to;
        tmp.protocol = msg.protocol;
        tmp.status = msg.status;
        tmp.type = NmsIpMessageConsts.NmsIpMessageType.TEXT;
        tmp.body = NmsIpLocationMessage.locMsg2Str(msg);
        return tmp;
    }

    public static short getSetIMStatusContactId(String number) {
        if (TextUtils.isEmpty(number)) {
            return -1;
        }
        if (!NmsCommonUtils.isPhoneNumberValid(number)) {
            return -1;
        }
        NmsContact contact = NmsIpMessageApiNative.nmsGetContactInfoViaNumber(number);
        if (null == contact) {
            return -1;
        }
        /*
        if (!contact.getNumber().contains(number)) {
            return -1;
        } 
        if (contact.getType() != NmsContactType.HISSAGE_USER) {
            return -1;
        } */
        return contact.getId();
    }

    public static String nmsGetStandardPhoneNum(String strPhoneNumIn) {
        if (TextUtils.isEmpty(strPhoneNumIn)) {
            return "";
        }

        if (NmsGroupChatContact.isGroupChatContactNumber(strPhoneNumIn)) {
            return strPhoneNumIn;
        }
        
        if (strPhoneNumIn.contains("@") && strPhoneNumIn.contains(".")
                && (strPhoneNumIn.indexOf("@") < strPhoneNumIn.indexOf("."))){
            return strPhoneNumIn;
        }

        return strPhoneNumIn.replaceAll("[^+0-9]", "");
    }

    public static boolean isChinaCard(String imsi) {
        if (TextUtils.isEmpty(imsi)) {
            NmsLog.error(TAG, "imsi is empty");
            return false;
        }

        String mcc = null;

        try {
            mcc = imsi.substring(0, 3);
        } catch (Exception e) {
            NmsLog.error(TAG, NmsLog.nmsGetStactTrace(e));
        }

        if (mcc != null && mcc.equals("460")) {
            return true;
        } else {
            return false;
        }
    }

    public static short[] sysMsgIdsToEngineIds(long[] msgIds) {
        if (msgIds == null)
            return null;

        short engineIds[] = new short[msgIds.length];

        for (int i = 0; i < msgIds.length; i++) {
            short id = NmsSMSMMSManager.getInstance(null).getNmsRecordIDViaSysId(msgIds[i]);
            if (id < 0) {
                NmsLog.error(TAG, "convert sys msg id to engine id error, error id: " + msgIds[i]);
                return null;
            }
            engineIds[i] = id;
        }

        return engineIds;
    }

    // check if current sim have registered/activated
    public static boolean isCurrentSimcardActivated(Context context) {
        final int currentSimId = (int) NmsPlatformAdapter.getInstance(context).getCurrentSimId();
        if (currentSimId > 0) {
            SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(currentSimId);
            if (info != null) {
                if (info.status >= NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                    return true;
                }
            }
        }
        return false;
    }

    // check if current sim have enabled
    public static boolean isCurrentSimcardEnabled(Context context) {
        final int currentSimId = (int) NmsPlatformAdapter.getInstance(context).getCurrentSimId();
        if (currentSimId > 0) {
            SNmsSimInfo info = NmsIpMessageApiNative.nmsGetSimInfoViaSimId(currentSimId);
            if (info != null) {
                if (info.status == NmsSimActivateStatus.NMS_SIM_STATUS_ACTIVATED) {
                    return true;
                }
            }
        }
        return false;
    }

    public static boolean isNetworkReady(Context c) {

        ConnectivityManager connManager = (ConnectivityManager) c
                .getSystemService(Context.CONNECTIVITY_SERVICE);

        if (null == connManager) {
            NmsLog.error(HissageTag.global,
                    "connectivity manager is null when checking active network");
            return false;
        }

        NetworkInfo i = connManager.getActiveNetworkInfo();
        if (i == null) {
            NmsLog.error(HissageTag.global, "no active network when checking active network");
            return false;
        }

        if (!i.isConnected()) {
            NmsLog.error(HissageTag.global,
                    "current network is not connected when checking active network");
            return false;
        }

        if (!i.isAvailable()) {
            NmsLog.error(HissageTag.global,
                    "current network is not available when checking active network");
            return false;
        }
        return true;
    }
}
