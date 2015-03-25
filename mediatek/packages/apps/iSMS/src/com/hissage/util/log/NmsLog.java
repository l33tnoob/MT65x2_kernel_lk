package com.hissage.util.log;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Date;
import java.util.Random;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Environment;
import android.util.Log;

import com.hissage.config.NmsCommonUtils;
import com.hissage.config.NmsCustomUIConfig;
import com.hissage.jni.engineadapterforjni;
import com.hissage.struct.SNmsAssertAlertMsgData;
import com.hissage.struct.SNmsMsgType;
import com.hissage.util.data.NmsConsts;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.data.NmsConverter;

public final class NmsLog {

    public static final int LOG_TRACE = 0;
    public static final int LOG_WARNING = 1;
    public static final int LOG_ERROR = 2;
    public static final int LOG_KEYPATH = 3;
    public static final int LOG_NO_LOG = 4;
    private static int LOG_LEVEL = LOG_ERROR;
    public static boolean isRelease = false;
    public static File file = null;
    public static FileWriter fWriter = null;
    public static final String logpath = File.separator + NmsCustomUIConfig.ROOTDIRECTORY + "/Log/";
    public static final String cachepath = File.separator + NmsCustomUIConfig.ROOTDIRECTORY
            + "/Cache/";
    public static Context mContext = null;

    public synchronized static void init(Context c) {
        mContext = c;
        isRelease = (0 == (c.getApplicationInfo().flags &= ApplicationInfo.FLAG_DEBUGGABLE));

        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            try {
                String newLog = NmsCommonUtils.getSDCardPath(c) + logpath + "hesinelog.log";
                String bakLog = NmsCommonUtils.getSDCardPath(c) + logpath + "hesinelog-bak.log";
                file = new File(newLog);
                if (!file.getParentFile().exists()) {
                    file.getParentFile().mkdirs();
                }

                if (!file.exists()) {
                    file.createNewFile();
                } else {
                    File file_bak = new File(bakLog);
                    if (file_bak.exists())
                        file_bak.delete();
                    file.renameTo(file_bak);
                    file = null;
                    file = new File(newLog);
                    file.createNewFile();
                }
                if(fWriter != null){
                    fWriter.close();
                }
                fWriter = new FileWriter(file);

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void destroy() {
    	file = null;
    }

    public static String nmsGetStactTrace(Exception e) {
        if (null == e)
            return null;
        String ret = e.toString();
        StackTraceElement[] stack = e.getStackTrace();
        for (int i = 0; stack != null && i < stack.length; ++i) {
            ret += "\n" + stack[i].toString();
        }
        return ret;
    }

    public static void nmsPrintStackTrace(Exception e) {
        if (null == e)
            return;
        error(HissageTag.global, "Exception: " + e.toString());
        StackTraceElement[] stack = e.getStackTrace();
        for (int i = 0; stack != null && i < stack.length; ++i) {
            error(HissageTag.global, stack[i].toString());
        }
    }

    public static void error(String tag, String msg) {
        if (LOG_LEVEL <= LOG_ERROR) {
            Log.e(tag, "thread Id: " + Thread.currentThread().getId() + "  " + msg);
            if (file == null || !file.exists()) {
                return;
            }
            appendLog(file, tag + "\t" + "thread Id: " + Thread.currentThread().getId() + "  "
                    + msg, 0);
        }
    }

    public static void trace(String tag, String msg) {
        if (LOG_LEVEL <= LOG_TRACE) {
            Log.i(tag, "thread Id: " + Thread.currentThread().getId() + "  " + msg);
            if (file == null || !file.exists()) {
                return;
            }
            appendLog(file, tag + "\t" + "thread Id: " + Thread.currentThread().getId() + "  "
                    + msg, 1);
        }
    }

    public static void warn(String tag, String msg) {
        if (LOG_LEVEL <= LOG_WARNING) {
            Log.w(tag, "thread Id: " + Thread.currentThread().getId() + "  " + msg);
            if (file == null || !file.exists()) {
                return;
            }
            appendLog(file, tag + "\t" + "thread Id: " + Thread.currentThread().getId() + "  "
                    + msg, 2);
        }
    }

    public static void appendLog(File file, String content, int level) {
        try {
            if (file == null || !file.exists()) {
                return;
            }
            StringBuffer sb = new StringBuffer();
            sb.append(NmsConsts.SDF2.format(new Date()));
            sb.append("\t ");
            sb.append(level == 1 ? "i" : level == 2 ? "w" : "e");
            sb.append("\t");
            sb.append(content);
            sb.append("\r\n");
            fWriter.write(sb.toString());
            fWriter.flush();
        } catch (Exception e) {
            Log.e(HissageTag.global, "log output exception,maybe the log file is not exists,"
                    + nmsGetStactTrace(e));
        } finally {
                if (file != null && file.length() >= NmsCustomUIConfig.MAX_FILE_SIZE) {
                    init(mContext);
                    return;
                }
        }
    }

    public static void ndump(byte[] dat) {
        String s = NmsConverter.bytes2HexString(dat);
        trace(HissageTag.global, s);
    }

    // random function.
    public static int NmsRand() {
        Random generator = new Random();
        generator.setSeed(System.currentTimeMillis());
        int val = generator.nextInt();
        // if value is not a plus number. convert to plus.
        if (val < 0) {
            val = -val;
        }
        return val;
    }

    // C-Style memory compare function.
    public static int memcmp(byte[] b1, byte[] b2) {
        if (null == b1 || null == b2)
            return 1;
        if (b1.length != b2.length) {
            return 1;
        }
        for (int i = 0; i < b1.length; ++i) {
            if (b1[i] != b2[i]) {
                return 1;
            }
        }
        return 0;
    }

    // C-STYLE memory set function :).
    public static void memset(byte[] b) {
        for (int i = 0; i < b.length; ++i) {
            b[i] = 0;
        }
    }

    public static int memcpy(byte[] dest, byte[] src) {
        int i = 0;
        if (dest != null && src != null) {
            while ((i < dest.length) && (i < src.length)) {
                dest[i] = src[i];
                ++i;
            }
        }
        return i;
    }

    public static int strlen(byte[] p) {

        int n = 0;
        while (p[n] != 0) {
            ++n;
        }
        return n;
    }

    public static void nmsAssertException(Exception e) {
        nmsPrintStackTrace(e);
        if (isRelease) {
            return;
        }
        engineadapterforjni.nmsSendMsgToUI(SNmsMsgType.NMS_UI_MSG_ASSERT_ALERT,
                new SNmsAssertAlertMsgData(e.toString()));
    }

    // sleep interface.
    public static void NmsSleep(long millSeconds) {
        try {
            Thread.sleep(millSeconds);
        } catch (Exception e) {
            nmsPrintStackTrace(e);
        }
    }

    /* For test */
    public static long nmsTestGetJavaCurLogPos() {

        if (file == null || !file.exists())
            return 0;

        return file.length();
    }

    public static boolean nmsTestSearchInLog(long pos, String str) {
        boolean ret = false;

        if (file == null || !file.exists())
            return false;

        try {
            RandomAccessFile rFile;
            if (file.exists()) {
                int dataLen = str.length() * 2;
                if (dataLen < 2048)
                    dataLen = 2048;
                byte[] data = new byte[dataLen];
                int readLen = 0;

                rFile = new RandomAccessFile(file, "r");

                while (true) {
                    rFile.seek(pos);
                    readLen = rFile.read(data);

                    if (readLen == -1)
                        break;

                    String fileStr = new String(data, "UTF-8");
                    if (fileStr.indexOf(str) != -1) {
                        ret = true;
                        break;
                    }

                    if (readLen < dataLen)
                        break;

                    pos -= str.length() - 1;
                }

                rFile.close();
            }
        } catch (Exception e) {

        }

        return ret;
    }
    
    public static void nmsSetSystemLogPriority(int priority) {
        LOG_LEVEL = priority;
    }

    public static int nmsGetSystemLogPriority() {
        return LOG_LEVEL;
    }
}
