package com.hissage.vcard;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

import android.accounts.Account;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.os.PowerManager;

import com.hissage.jni.engineadapter;
import com.hissage.util.data.NmsConsts.HissageTag;
import com.hissage.util.log.NmsLog;

public class ImportVcardThread extends Thread {

    private static final boolean DO_PERFORMANCE_PROFILE = false;
    public static final String LOG_TAG = ImportVcardThread.class.getSimpleName();

    private ContentResolver mResolver;
    private VCardParser_V21 mVCardParser;
    private boolean mCanceled;
    private PowerManager.WakeLock mWakeLock;
    private String mCanonicalPath;
    // private ProgressDialog mProgressDialogForReadVCard;
    private Account mAccount;
    private List<VCardFile> mSelectedVCardFileList;
    private List<String> mErrorFileNameList;
    private Context context;
    int msgWhenFinish = 0;

    public ImportVcardThread(String canonicalPath, Context context, int msg) {
        mCanonicalPath = canonicalPath;
        context = context;
        mResolver = context.getContentResolver();
        msgWhenFinish = msg;
        init(context);
    }

    private void init(Context context) {
        mResolver = context.getContentResolver();
        PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = powerManager.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK
                | PowerManager.ON_AFTER_RELEASE, LOG_TAG);
        // mProgressDialogForReadVCard = new ProgressDialog(context);
        // mProgressDialogForReadVCard.setTitle("test ");
        // mProgressDialogForReadVCard.setMessage("vcard test");
        // mProgressDialogForReadVCard.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
    }

    @Override
    public void finalize() {
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
        }
    }

    @Override
    public void run() {
        boolean shouldCallFinish = true;
        mWakeLock.acquire();
        NmsContactObserver.unregisterContentObserver(false);
        // Some malicious vCard data may make this thread broken
        // (e.g. OutOfMemoryError).
        // Even in such cases, some should be done.
        try {
            if (mCanonicalPath != null) { // Read one file
                // mProgressDialogForReadVCard.setProgressNumberFormat("");
                // mProgressDialogForReadVCard.setProgress(0);

                // Count the number of VCard entries
                // mProgressDialogForReadVCard.setIndeterminate(true);
                long start;
                if (DO_PERFORMANCE_PROFILE) {
                    start = System.currentTimeMillis();
                }
                VCardEntryCounter counter = new VCardEntryCounter();
                VCardSourceDetector detector = new VCardSourceDetector();
                VCardBuilderCollection builderCollection = new VCardBuilderCollection(
                        Arrays.asList(counter, detector));

                boolean result;
                try {
                    result = readOneVCardFile(mCanonicalPath, VCardConfig.DEFAULT_CHARSET,
                            builderCollection, null, true, null);
                } catch (VCardNestedException e) {
                    try {
                        // Assume that VCardSourceDetector was able to
                        // detect the source.
                        // Try again with the detector.
                        result = readOneVCardFile(mCanonicalPath, VCardConfig.DEFAULT_CHARSET,
                                counter, detector, false, null);
                    } catch (VCardNestedException e2) {
                        result = false;
                        NmsLog.trace(HissageTag.vcard, "Must not reach here. " + e2);
                    }
                }
                if (DO_PERFORMANCE_PROFILE) {
                    long time = System.currentTimeMillis() - start;
                    NmsLog.trace(HissageTag.vcard,
                            "time for counting the number of vCard entries: " + time + " ms");
                }
                if (!result) {
                    shouldCallFinish = false;
                    // NmsMain.postMessage(new NmsMessage(msgWhenFinish));
                    if (mWakeLock != null && mWakeLock.isHeld()) {
                        mWakeLock.release();
                    }
                    return;
                }

                // mProgressDialogForReadVCard
                // .setProgressNumberFormat(getString(R.string.reading_vcard_contacts));
                // mProgressDialogForReadVCard.setIndeterminate(false);
                // mProgressDialogForReadVCard.setMax(counter.getCount());
                String charset = detector.getEstimatedCharset();
                doActuallyReadOneVCard(mCanonicalPath, null, charset, true, detector,
                        mErrorFileNameList);
            } else { // Read multiple files.
                // mProgressDialogForReadVCard
                // .setProgressNumberFormat(getString(R.string.reading_vcard_files));
                // mProgressDialogForReadVCard.setMax(mSelectedVCardFileList.size());
                // mProgressDialogForReadVCard.setProgress(0);

                for (VCardFile vcardFile : mSelectedVCardFileList) {
                    if (mCanceled) {
                        // NmsMain.postMessage(new NmsMessage(msgWhenFinish));
                        if (mWakeLock != null && mWakeLock.isHeld()) {
                            mWakeLock.release();
                        }
                        return;
                    }
                    String canonicalPath = vcardFile.getCanonicalPath();

                    VCardSourceDetector detector = new VCardSourceDetector();
                    try {
                        if (!readOneVCardFile(canonicalPath, VCardConfig.DEFAULT_CHARSET, detector,
                                null, true, mErrorFileNameList)) {
                            continue;
                        }
                    } catch (VCardNestedException e) {
                        NmsLog.nmsPrintStackTrace(e);
                    }
                    String charset = detector.getEstimatedCharset();
                    doActuallyReadOneVCard(canonicalPath, mAccount, charset, false, detector,
                            mErrorFileNameList);
                    // mProgressDialogForReadVCard.incrementProgressBy(1);
                }
            }
        } finally {
            mWakeLock.release();
            NmsContactObserver.registerContentObserver(context, true);
            // mProgressDialogForReadVCard.dismiss();
            // finish() is called via mCancelListener, which is used in
            // DialogDisplayer.
            if (shouldCallFinish) {
                if (!(mErrorFileNameList == null || mErrorFileNameList.isEmpty())) {
                    StringBuilder builder = new StringBuilder();
                    boolean first = true;
                    for (String fileName : mErrorFileNameList) {
                        if (first) {
                            first = false;
                        } else {
                            builder.append(", ");
                        }
                        builder.append(fileName);
                    }

                    // mHandler.post(new
                    // DialogDisplayer(getString(R.string.fail_reason_failed_to_read_files,
                    // builder
                    // .toString())));
                }
            }
        }
        
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
        }
        
        File file = new File(mCanonicalPath);
        if(file.exists()){
        	file.delete();
        }       
        if (msgWhenFinish == engineadapter.msgtype.NMS_ENG_MSG_UPDATE_VCARD.ordinal()) {
            try {
                NmsVcardUtils.nmsExportDirtyContactFromNative2File(context,
                        mCanonicalPath.replace("SUL", "CUL"));
            } catch (Exception e) {
                NmsLog.nmsPrintStackTrace(e);
            }
            engineadapter.get().nmsUploadCUL();
        } else if (engineadapter.msgtype.NMS_ENG_MSG_ADD_CONTACT_ACK.ordinal() == msgWhenFinish) {
            engineadapter.get().nmsSendMsgToEngine(
                    engineadapter.msgtype.NMS_ENG_MSG_ADD_CONTACT_ACK.ordinal(), null, 0);
        }
    }

    private boolean doActuallyReadOneVCard(String canonicalPath, Account account, String charset,
            boolean showEntryParseProgress, VCardSourceDetector detector,
            List<String> errorFileNameList) {
        VCardDataBuilder builder;
        int vcardType = VCardConfig.getVCardTypeFromString("ok");
        if (charset != null) {
            builder = new VCardDataBuilder(charset, charset, false, vcardType, mAccount);
        } else {
            charset = VCardConfig.DEFAULT_CHARSET;
            builder = new VCardDataBuilder(null, null, false, vcardType, mAccount);
        }
        builder.addEntryHandler(new EntryCommitter(mResolver));
        if (showEntryParseProgress) {
            // builder.addEntryHandler(new
            // ProgressShower(mProgressDialogForReadVCard, "test", context,
            // mHandler));
        }

        try {
            if (!readOneVCardFile(canonicalPath, charset, builder, detector, false, null)) {

                return false;
            }
        } catch (VCardNestedException e) {
            NmsLog.trace(HissageTag.vcard, "Never reach here.");
        }
        return true;
    }

    private boolean readOneVCardFile(String canonicalPath, String charset, VBuilder builder,
            VCardSourceDetector detector, boolean throwNestedException,
            List<String> errorFileNameList) throws VCardNestedException {
        FileInputStream is;
        try {
            is = new FileInputStream(canonicalPath);
            mVCardParser = new VCardParser_V21(detector);

            try {
                mVCardParser.parse(is, charset, builder, mCanceled);
            } catch (VCardVersionException e1) {
                NmsLog.nmsPrintStackTrace(e1);
                try {
                    is.close();
                } catch (IOException e) {
                    NmsLog.nmsPrintStackTrace(e);
                }
            } finally {
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }
                }
            }
        } catch (IOException e) {
            NmsLog.trace(HissageTag.vcard,
                    "IOException was emitted: " + NmsLog.nmsGetStactTrace(e));

            // mProgressDialogForReadVCard.dismiss();

            if (errorFileNameList != null) {
                errorFileNameList.add(canonicalPath);
            } else {
                // mHandler.post(new DialogDisplayer("exception" + ": " +
                // e.getLocalizedMessage()));
            }
            return false;
        } catch (VCardNotSupportedException e) {
            if ((e instanceof VCardNestedException) && throwNestedException) {
                throw (VCardNestedException) e;
            }
            if (errorFileNameList != null) {
                errorFileNameList.add(canonicalPath);
            } else {
                NmsLog.nmsPrintStackTrace(e);
            }
            return false;
        } catch (VCardException e) {
            NmsLog.nmsPrintStackTrace(e);
            if (errorFileNameList != null) {
                errorFileNameList.add(canonicalPath);
            } else {
                NmsLog.nmsPrintStackTrace(e);
            }
            return false;
        }
        return true;
    }

    public void cancel() {
        mCanceled = true;
        if (mVCardParser != null) {
            mVCardParser.cancel();
        }
    }

    public void onCancel(DialogInterface dialog) {
        cancel();
    }

}

class VCardFile {
    private String mName;
    private String mCanonicalPath;
    private long mLastModified;

    public VCardFile(String name, String canonicalPath, long lastModified) {
        mName = name;
        mCanonicalPath = canonicalPath;
        mLastModified = lastModified;
    }

    public String getName() {
        return mName;
    }

    public String getCanonicalPath() {
        return mCanonicalPath;
    }

    public long getLastModified() {
        return mLastModified;
    }
}